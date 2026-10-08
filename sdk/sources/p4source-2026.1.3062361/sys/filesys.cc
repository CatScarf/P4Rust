/*
 * Copyright 1995, 1996 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# define NEED_FILE
# define NEED_CHDIR
# define NEED_WIN32FIO

# include <stdhdrs.h>
# include <charman.h>

# include <i18napi.h>
# include <charcvt.h>
# include <charset.h>

# include <error.h>
# include <debug.h>
# include <tunable.h>
# include <strbuf.h>
# include <strdict.h>
# include <strtable.h>
# include <strops.h>
# include <md5.h>
# include <datetime.h>

# include "pathsys.h"
# include "filesys.h"
# include "macfile.h"
# include "fileio.h"
# include "signaler.h"
# include "enviro.h"
# include "errno.h"
# include "hostenv.h"
# include "pressuremonitor.h"
# include "sysinfo.h"
# define USE_ERRNO

# include <msgos.h>

bool P4FileSysCreateOnIntr = true;

# ifdef OS_NT
// Indicates atomic rename can occur.
int p4WinAtomicRename=0;
# endif

void
FileSysCleanup( FileSys *f )
{
	if( f->IsDeleteOnClose() )
	    f->Cleanup();
}

FileSys *
FileSys::Create( FileSysType t, FileSysBuffer *buf )
{
	FileSys *f;
	LineType lt;

	// Pull the LineType out of the FileSysType.

	switch( t & FST_L_MASK )
	{
	case FST_L_LOCAL: 	lt = LineTypeLocal; break;
	case FST_L_LF:		lt = LineTypeRaw; break;
	case FST_L_CR:		lt = LineTypeCr; break;
	case FST_L_CRLF:	lt = LineTypeCrLf; break;
	case FST_L_LFCRLF:	lt = LineTypeLfcrlf; break;
	default: 		lt = LineTypeLocal; 
	}

	// Apple's an odd mod: it's a mod, but totally supplants
	// the text/binary type.  So include it in the switch.
	// The other mods don't affect the base type.

	switch( t & ( FST_MASK | FST_M_APPLE | FST_M_APPEND ) )
	{
	case FST_TEXT:		
		f = new FileIOBuffer( lt ); 
		break;
	case FST_UNICODE:
		f = new FileIOUnicode( lt ); 
		break;

	case FST_BINARY:
	    if( t & FST_C_MASK )
		f = new FileIOCompress;
	    else
		f = new FileIOBinary;
	    break;

	case FST_RESOURCE:
		f = new FileIOResource;
		break;

	case FST_APPLETEXT:
	case FST_APPLEFILE:
		f = new FileIOApple;
		break;

	case FST_SYMLINK:
	        if( SymlinksSupported() )
	            f = new FileIOSymlink;
	        else
	            f = new FileIOBinary;
		break;

	case FST_ATEXT:
		f = new FileIOAppend( lt );
		break;

	case FST_UTF8:
		f = new FileIOUTF8( lt );
		break;

	case FST_UTF16:
		f = new FileIOUTF16( lt );
		break;

	case FST_EMPTY:
	        f = new FileIOEmpty;
	        break;

	case FST_DIRECTORY:
	        f = new FileIODir;
	        break;
	
	default:
		return NULL;
	}

	// Replace the type.

	f->type = t;

	// Insert the delegate buffer
	f->delegate = buf;

	// Arrange for temps to blow on exit.
	if( P4FileSysCreateOnIntr )
	    signaler.OnIntr( (SignalFunc)FileSysCleanup, f );

	return f;
}

FileSys::FileSys()
{
	// start off with permission bits as the umask;

	mode = FOM_READ;
	perms = FPM_RO;
	modTime = 0;
	sizeHint = 0;
	checksum = 0;
	cacheHint = 0;
	preserveCWD = 0;

	allowStatCache = 0;
	// PR_002 Start
	statSnapshotFlags = -1;
	// PR_002 End
	statCacheValid = 0;
	cachedModTime = 0;
	cachedSize = 0;

	charSet = GlobalCharSet::Get();
	content_charSet = GlobalCharSet::Get();
	delegate = 0;

	type = FST_TEXT;

# ifdef OS_NT
	LFN = 0;
# endif

	isTemp = 0;

# ifdef OS_CYGWIN
	// cygwin 1.3.12: rmdir() exits(!) if passwd CWD.
	preserveCWD = 1;
# endif
}

FileSys::~FileSys()
{
	// Once gone, we don't need to delete it
	// on interrupt anymore.

	if( P4FileSysCreateOnIntr )
	    signaler.DeleteOnIntr( this );

	// If there's a delegate, it may be deleted now
	// Or not, depending on what the caller needs

	if( delegate )
	    delegate->Release();
}

int
FileSys::BufferSize()
{
	return p4tunable.Get( P4TUNE_FILESYS_BUFSIZE );
}

void
FileSys::Cleanup()
{
	Error e;
	Close( &e );
	if( IsDeleteOnClose() )
	    Unlink();
}

# ifdef OS_NT
void
FileSys::SetLFN( const StrPtr &name )
{
	int lfn_val = p4tunable.Get( P4TUNE_FILESYS_WINDOWS_LFN );
	if( lfn_val )
	{
	    int extended = IsExtended( name );

	    // For LFN nt_wname will convert from ANSI to UNICODE.
	    //
	    // Use GetCwdbyCS as it determines UTF8 or ANSI.  It
	    // also provides a better representation of cwd length.
	    if( extended )
	    {
		// The caller passed a device-prefixed path ("\\?\" or
		// "\\.\").  Enable LFN regardless of length so the
		// MAX_PATH guards accept it, and skip the length/UNC
		// classification - nt_wname detects the existing prefix
		// and must not prefix it again.
		LFN = LFN_ENABLED;
	    }
	    else if( IsRelative( name ) )
	    {
		StrBuf cwd;
		HostEnv::GetCwdbyCS( cwd, GlobalCharSet::Get());

		// If the absolute path is over 255, use LFN support.
		// If the tunable is 10, LFN is always on for Testing.
		if( lfn_val == 10 || ( cwd.Length() + name.Length() > 255 ) )
		    LFN = LFN_ENABLED;
	    }
	    else
	    {
		// Here the path is already absolute, assume a drive spec.
		if( lfn_val == 10 || ( name.Length() > 255 ) )
		    LFN = LFN_ENABLED;
	    }

	    // LFN_UNCPATH drives nt_wname's "\\?\UNC" prefix assembly, so
	    // it must never be set for an already-extended path: rename
	    // ORs the flags of two FileSys objects and applies them to
	    // both names, and a leaked UNCPATH bit would corrupt a plain
	    // peer path into "\\?\UNC<path-minus-one-slash>".
	    if( LFN && !extended )
	    {
		// If UNC path, later we use "\\?\UNC" instead of "\\?\"
		if( IsUNC( name ) )
		    LFN |= LFN_UNCPATH;
	    }
	}

	// If DOUNICODE, This flag will cause nt_wname to convert
	// from UTF8 to UNICODE.
	if( DOUNICODE )
	    LFN |= LFN_UTF8;
}
# endif // OS_NT

// Validate if the atomic configurable is supported.

void
FileSys::CheckForAtomicRename( Error *e )
{
# ifdef OS_NT
	if( !SystemInfo::CheckForAtomicRename() )
	{
	    e->Set( MsgOs::NoAtomicRename );
	    return;
	}
# endif // OS_NT
}


// Atomic rename is only supported for Visual Studio 2017 and newer.
// The top level decision for atomic rename is made here.
// This can be called very early on start up.  Logging a message
// on those early calls is not possible.  An error message can be
// logged when the tunable is set.

void
FileSys::SetAtomicRename()
{
# if (_MSC_VER > 1900)
	Error e;

	CheckForAtomicRename( &e );

	if( !e.Test() )
	{
	    if( p4WinAtomicRename )
	        LFN |= LFN_ATOMIC_RENAME;
	}
# endif
}

void
FileSys::Set( const StrPtr &name )
{
# if defined(OS_NT) && (_MSC_VER >= 1800)
	if( name.Text()[0] != '-' )
	    SetLFN( name );
# endif
	path.Set( name );
	statCacheValid = 0;
	// PR_002 Start
	statSnapshotFlags = -1;
	// PR_002 End
}

void
FileSys::Set( const StrPtr &name, Error *e )
{
# ifdef OS_NT
# if (_MSC_VER >= 1800)
	if( name.Text()[0] != '-' )
	    SetLFN( name );
# endif
	if( !LFN )
	{
	    // For a relative path, this does not take into account p4root.
	    int maxLen = 260; // MAX_PATH from <windows.h>
	    if( e && name.Text() && name.Length() > maxLen )
		e->Set( MsgOs::NameTooLong ) << name <<
		    StrNum( (int)name.Length() ) << StrNum( maxLen );
	}
# endif
	path.Set( name );
	statCacheValid = 0;
	// PR_002 Start
	statSnapshotFlags = -1;
	// PR_002 End
}

void
FileSys::SetDigest( MD5 *m )
{
	checksum = m;
}

MD5 *
FileSys::GetDigest( )
{
	return checksum;
}

int
FileSys::DoIndirectWrites()
{
	// No indirect if were doing buffer stuff
	if( delegate )
	    return 0;

# if defined( OS_MACOSX ) && OS_VER < 1010
	return ( ( type & FST_MASK ) == FST_SYMLINK );
# else
	return 1;
# endif
}

void
FileSys::Translator( CharSetCvt * )
{
}

void
FileSys::LowerCasePath()
{
	if( CharSetApi::isUnicode( (CharSetApi::CharSet)GetCharSetPriv()) )
	{
	    StrBuf res;

	    if( CharSetCvt::Utf8Fold( &path, &res ) == 0 )
	    {
		path = res;
		return;
	    }
	    // if Utf8 fold failed... fall through...
	}
	StrOps::Lower( path );
}

/*
 * FileSys::GetFd()
 * FileSys::GetSize()
 * FileSys::GetCurrentSize()
 * FileSys::Seek()
 *
 * Non-functional stubs.
 */

FD_PTR
FileSys::GetFd()
{
	return FD_ERR;
}

int
FileSys::GetOwner()
{
	return 0;
}

void
FileSys::StatModTimeHP(DateTimeHighPrecision *modTime)
{
	*modTime = DateTimeHighPrecision();
}

bool
FileSys::HasOnlyPerm( FilePerm perms )
{
	return true;
}

offL_t
FileSys::GetSize()
{
	return 0;
}

offL_t
FileSys::GetCurrentSize()
{
	return 0;
}

int
FileSys::LinkCount()
{
	return -1;
}

void
FileSys::Seek( offL_t offset, Error * )
{
}

offL_t
FileSys::Tell()
{
	return 0;
}

/*
 * FileSys::Perms() - translate permission
 */

FilePerm
FileSys::Perm( const char *perms )
{
	return strcmp( perms, "rw" ) ? FPM_RO : FPM_RW;
}

static int ContainsTraversal( const char *p )
{
	while( p && *p )
	{
	    if( p[0] == '.' && p[1] == '.' &&
		( p[2] == '\0' || p[2] == '/'
# ifdef OS_NT
		|| p[2] == '\\'
# endif
		    ) )
	        return 1;
	    do {
		if( *++p == '/'
# ifdef OS_NT
		    || *p == '\\'
# endif
		    )
		{
		    ++p;
		    break;
		}
	    } while( *p != '\0' );
	}
	return 0;
}


bool
FileSys::IsRelative( const StrPtr &path )
{
	const char *p = path.Text();

	if( *p == '/' )
	    return false;
#ifdef OS_NT
	if( *p == '\\' )
	    return false;
	if( p[0] && p[1] == ':' )
	    return false;
#endif
	return true;
}

#ifdef OS_NT
/*
 * FileSys::IsExtended() - path begins with a Win32 device namespace
 * prefix, "\\?\" or "\\.\" (separators may be forward slashes; the OS
 * normalizes those forms).  Such a path must reach the OS as-is: it is
 * never relative, it is not a UNC host form, and assembling another
 * prefix in front of it produces a name NtOpenFile rejects (see
 * SetLFN and nt_wname).
 */

bool
FileSys::IsExtended( const StrPtr &path )
{
	const char *p = path.Text();

	return ( p[0] == '/' || p[0] == '\\' ) &&
	       ( p[1] == '/' || p[1] == '\\' ) &&
	       ( p[2] == '?' || p[2] == '.' ) &&
	       ( p[3] == '/' || p[3] == '\\' );
}

bool
FileSys::IsUNC( const StrPtr &path )
{
	const char *p = path.Text();

	// At a minimum we are looking for "\\host\share".
	if( (p[0] == '/' || p[0] == '\\') && (p[1] == '/' || p[1] == '\\') )
	{
	    // Device prefixes are not UNC hosts.  Only the extended form
	    // "\\?\UNC\host\share" names a UNC path.
	    if( IsExtended( path ) )
		return p[2] == '?' && !strnicmp( &p[4], "UNC", 3 ) &&
		       ( p[7] == '/' || p[7] == '\\' );

	    if( strchr( &(p[2]), '/' ) || strchr( &(p[2]), '\\' ) )
		return true;
	}
	return false;
}
#endif

bool
FileSys::MakePathWriteable( const StrPtr &oldFile, StrBuf &newFile, Error *e )
{
	StrBuf strippedFile;

	PathSys *parent = PathSys::Create();
	parent->Set( oldFile );
	parent->ToParent( &strippedFile );

	if( !parent->Length() )
	{
	    delete parent;
	    return false;
	}
	delete parent;

	FileSys *f = FileSys::Create( FST_BINARY );
	f->Set( oldFile.Text() );
	int exists = f->Stat() & FSF_EXISTS;

	Error tmpE;
	f->Open( FOM_RW, &tmpE );
	f->Close( &tmpE );

	/*
	 * The open/close causes a file that doesn't exist to be created.
	 * If it didn't exist before, we should delete it now.
	 */
	if( !exists )
	    f->Unlink();
	delete f;

	if( !tmpE.Test() )
	    return false;

	/*
	 * File oldFile is not writeable. Removing the absolute or relative
	 * path component might help, so set the stripped value in newFile.
	 */
	newFile.Set( strippedFile );
	return true;
}

static int UnderRootCheck( const char *name, const char *root, int rootLen )
{
	int result;
	PathSys *p = PathSys::Create();
	p->Set( name );
	StrBuf r;
	if( *root == '.' )
	{
	    HostEnv h;
	    Enviro e;
	    StrBuf b;
	    h.GetCwd( b, &e );
	    r << b << StrRef( root + 1, rootLen - 1 );
	}
	else
	    r.Set( root, rootLen );

	result = p->IsUnderRoot( r );

	delete p;

	return result;
}

/**
 * FileExists
 *
 * @brief Verify that the file exists
 *
 * @param  filepath
 * @return boolean true if exists false otherwise
 * @throws
 */
bool
FileSys::FileExists( const char *filepath )
{
	if( !filepath )
	    return false;

	FileSys *f = FileSys::Create( FST_BINARY );
	f->Set( filepath );
	if( f->Stat() & FSF_EXISTS )
	{
	    delete f;
	    return true;
	}
	delete f;
	return false;
}

// NeedMkDir - Checks if the parent directory exists
bool
FileSys::NeedMkDir()
{
	PathSys *path = PathSys::Create();
	path->Set( Path() );
	path->ToParent();
	bool res = ! FileSys::FileExists( path->Text() );
	delete path;
	return res;
}

// Enforces P4CLIENTPATH restrictions
int
FileSys::IsUnderPath( const StrPtr &roots )
{
	if( roots.Length() == 0 )
	    return 1;

	HostEnv h;
	Enviro e;
	StrBuf b;
	const char *n = Name();
	if( ContainsTraversal( n ) )
	    return 0;
	if( IsRelative( StrRef( n ) ) )
	{
	    h.GetCwd( b, &e );
	    n = b.Text();
	}

	char listSep = ';';
	const char *p = roots.Text();
	const char *s = p;
	while( *p )
	{
	    if( *p == listSep )
	    {
	        if( p != s )
	        {
	            if( UnderRootCheck( n, s, p - s ) )
	                return 1;
	        }
	        s = p + 1;
	    }
	    p++;
	}
	if( p != s )
	    return UnderRootCheck( n, s, p - s );
	return 0;
}

void
FileSys::DepotSize( offL_t &len, Error *e )
{
	len = (offL_t) -1;
}

# ifdef HAS_CPP11

FileSysUPtr FileSys::CreateUPtr( FileSysType type )
{
	return FileSysUPtr( Create( type ) );
}

FileSysUPtr FileSys::CreateGlobalTempUPtr( FileSysType type )
{
	return FileSysUPtr( CreateGlobalTemp( type ) );
}

# endif
