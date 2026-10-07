/*
 * Copyright 1995, 2003 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

/*
 * fileio -- FileIO, FileIOBinary, FileIOAppend methods
 *
 * Hacked to hell for OS implementations.
 */

# define NEED_FLOCK
# define NEED_FSYNC
# define NEED_FCNTL
# define NEED_FILE
# define NEED_STAT
# define NEED_UTIME
# define NEED_UTIMES
# define NEED_TIME_HP
# define NEED_ERRNO
# define NEED_READLINK
# define NEED_WIN32FIO
# define NEED_XATTRS

# include <stdhdrs.h>

# include <error.h>
# include <errornum.h>
# include <debug.h>
# include <tunable.h>
# include <msgsupp.h>
# include <strbuf.h>
# include <strarray.h>
# include <strdict.h>
# include <strtable.h>
# include <datetime.h>
# include <i18napi.h>
# include <charcvt.h>
# include <largefile.h>
# include <fdutil.h>
# include <md5.h>

# include <msgos.h>

# include "filesys.h"
# include <pathsys.h>
# include "fileio.h"

// We need to know the user's umask so that Chmod() doesn't give
// away permissions beyond the umask.

int global_umask = -1;

# ifndef O_BINARY
# define O_BINARY 0
# endif

# ifndef O_CLOEXEC
# define O_CLOEXEC 0
# endif

const FileIOBinary::OpenMode FileIOBinary::openModes[4] = {
	{
	"open for read",
		O_RDONLY|O_BINARY|O_CLOEXEC,  // bflags
		O_RDONLY|O_BINARY|O_CLOEXEC,  // aflags
		0,                            // stdio descriptor
	},

	{
	"open for write",
		O_WRONLY|O_CREAT|O_TRUNC|O_BINARY|O_CLOEXEC,
		O_WRONLY|O_CREAT|O_APPEND|O_CLOEXEC,
		1
	},

	{
	"open for read/write",
		O_RDWR|O_CREAT|O_BINARY|O_CLOEXEC,
		O_RDWR|O_CREAT|O_APPEND|O_CLOEXEC,
		1
	},

	{
	"open for untranslated write",
		O_WRONLY|O_CREAT|O_TRUNC|O_BINARY|O_CLOEXEC,
		O_WRONLY|O_CREAT|O_APPEND|O_BINARY|O_CLOEXEC,
		1
	}
} ;

FileIO::FileIO()
{
	// Get the umask if we don't know it already.

	if( global_umask < 0 )
	{
	    global_umask = umask( 0 );
	    (void)umask( global_umask );
	}
}

void
FileIO::DepotSize( offL_t &len, Error *e )
{
}

int
directoryHasMultipleObjects( StrBuf dir, StrArray *dirContents, Error *e )
{
	if( dirContents == 0 || dirContents->Count() == 0 )
	    return 0;

	if( dirContents->Count() > 1)
	    return 1;

	
	int ret = 0;

	PathSys *path = PathSys::Create();
	path->SetLocal( dir, *( dirContents->Get( 0 ) ) );

	FileSys *f = FileSys::Create( FST_TEXT );
	f->Set( *path );

	if( f->Stat() & FSF_DIRECTORY )
	{
	    //Check this sub directory contents
	    StrArray *ua = f->ScanDir( e );

	    // Recurse
	    ret = directoryHasMultipleObjects( *path, ua, e );

	    delete ua;

	    if( e->Test() )
	    {
	        delete f;
	        delete path;
	        return 1;
	    }
	}

	delete f;
	delete path;
	return ret;
}

void
FileIO::RenameSourceSubstrInTargetSubdir( StrBuf &currentName,
	                                  FileSys *target,
	                                  Error *e )
{
	// Only source should be a directory substring of target
# if defined( OS_NT )
	if( path.Length() > target->Path()->Length() ||
	    *( target->Name() + path.Length() ) != '\\' )
	    return;
# else
	if( path.Length() > target->Path()->Length() ||
	    *( target->Name() + path.Length() ) != '/' )
	    return;
#endif

	// Target should start with source sub string

	if( strstr( target->Name(), Name() ) != target->Name() )
	    return;

	// This a workaround to handle a rename function deficiency,
	// which cannot work with an old name which is a directory substring
	// of the new name.

	// Name() is a substring of target->Name(), starts target->Name()
	// and so Name() must be a directory for the target.

	// Create a temporary name, rename path to temp name
	char buf[50];
	TempName( buf );
	currentName.Append( buf );

	if( OsRename( Path(), &currentName, target ) != 0 )
	{
	    e->Set( MsgSupp::RenameTempFailed ) << currentName.Text();
	    return;
	}

	// If the old name directory doesn't exist, make it now.
	MkDir( *(target->Path()), e );
	if( e->Test() )
	{
	    e->Set( MsgSupp::RenameMkdirFailed ) << Name();
	    return;
	}
}

void
FileIO::RenameTargetSubStrSubdirInSource( StrBuf &currentName, 
	                                  FileSys *target, 
	                                  Error *e )
{
	if( !( target->Stat() & FSF_DIRECTORY ) ||
	         !path.Contains( *( target->Path() ) ) )
	    return;

	// In this case, the target is directory subpath 
	// of the source.  Rename the source file if 
	// the target directory only has one entry.

	StrArray *ua = target->ScanDir( e );

	int dirHasMoreThanOne = 
	    directoryHasMultipleObjects( target->Name(), ua, e );

	delete ua;

	if( e->Test() )
	{
	    e->Set( MsgSupp::RenameDirSearchFailed ) << target->Name();
	    return;
	}

	if( dirHasMoreThanOne )
	{
	    e->Set( MsgSupp::RenameDirNotEmpty ) << target->Name();
	    return;
	}

	//reset the currentname to a temporary name in the directory
	char buf[50];
	TempName( buf );
	currentName = target->Name();
	currentName.Append( buf );

	//Move the source name to the temporary name
	if( OsRename( Path(), &currentName, target ) != 0 )
	{
	    e->Set( MsgSupp::RenameTempFailed ) << currentName.Text();
	    return;
	}

	//Remove the target name directory
	RmDir( *( Path() ), e );
	if( e->Test() )
	{
	    e->Set( MsgSupp::RenameRmdirFailed ) << Name();
	    return;
	}
}

# if !defined( OS_VMS ) && !defined( OS_NT )

int
FileIO::OsRename( StrPtr *source, StrPtr *target, FileSys *origTarget )
{
	// Parameter origTarget used by windows version.
	return rename( source->Text(), target->Text() );
}

void
FileIO::Rename( FileSys *target, Error *e )
{
	// yeech - must not exist to rename

# if defined(OS_OS2) || defined(OS_AS400)
	target->Unlink( 0 );
# endif

# if defined( OS_MACOSX ) || defined( OS_DARWIN )
	// On mac, deal with a possible immutable *uchg) flag

	struct stat sb;

	if( stat( Name(), &sb ) >= 0 && ( sb.st_flags & UF_IMMUTABLE ) )
	    chflags( Name(), sb.st_flags & ~UF_IMMUTABLE );
	if( stat( target->Name(), &sb ) >= 0 && ( sb.st_flags & UF_IMMUTABLE ) )
	    chflags( target->Name(), sb.st_flags & ~UF_IMMUTABLE );
# endif
	// Run on all other UNIX variants, including Mac OS X

	if( rename( Name(), target->Name() ) < 0 )
	{
	    if( Path()->Contains( *( target->Path() ) ) || 
	        target->Path()->Contains( *( Path( ) ) ) )
	    {
	        // Either target is a substring (directory) of source,
	        // or source is a substring of target (target has a
	        // directory subpath which is the as source) 
	        // or source or target has a component which is not a directory.

	        // Try moving the current name to a temporary name, and then trying again.
	        StrBuf currentName;
	        currentName = Name();

	        if( path.Length() < target->Path()->Length() )
	        {
	            // source is a substring of the target
	            RenameSourceSubstrInTargetSubdir( currentName, target, e );
	        }
	        else
	        {
	            // target is a substring of the source
	            RenameTargetSubStrSubdirInSource( currentName, target, e );
	        }

	        if( e->Test() )
	            return;

	        if( rename( currentName.Text(), target->Name() ) < 0 )
	        {
	            e->Sys( "rename", target->Name() );
	            return;
	        }
	    }
	    else
	    {
	        e->Sys( "rename", target->Name() );
	        return;
	    }
	}

	// source file has been deleted,  clear the flag

	ClearDeleteOnClose();
}

void
FileIO::ChmodTime( P4INT64 modTime, Error *e )
{
# ifdef HAVE_UTIME
	struct utimbuf t;
	DateTime now;

	now.SetNow();
	t.actime = DateTime::Localize( now.Value() );
	t.modtime = DateTime::Localize( modTime );

	if( utime( Name(), &t ) < 0 )
	    e->Sys( "utime", Name() );
# endif // HAVE_UTIME
}

void
FileIO::ChmodTimeHP( const DateTimeHighPrecision &modTime, Error *e )
{
	DateTimeHighPrecision now;

	now.Now();

# if defined(HAVE_UTIMENSAT)
	struct timespec tv[2];

	tv[0].tv_sec = DateTime::Localize( now.Seconds() );
	tv[0].tv_nsec = now.Nanos();
	tv[1].tv_sec = DateTime::Localize( modTime.Seconds() );
	tv[1].tv_nsec = modTime.Nanos();

	if( utimensat( AT_FDCWD, Name(), tv, 0 ) < 0 )
	    e->Sys( "utimensat", Name() );
# elif defined(HAVE_UTIMES)
	struct timeval	tv[2];

	tv[0].tv_sec = DateTime::Localize( now.Seconds() );
	tv[0].tv_usec = now.Nanos() / 1000;
	tv[1].tv_sec = DateTime::Localize( modTime.Seconds() );
	tv[1].tv_usec = modTime.Nanos() / 1000;

	if( utimes( Name(), tv ) < 0 )
	    e->Sys( "utimes", Name() );
# else
	ChmodTime( modtime.Seconds(), e )
# endif
}

# endif // !OS_VMS && !OS_NT


void
FileIO::ChmodTime( Error *e )
{
	if( modTime )
	    ChmodTime( modTime, e );
}

# if !defined( OS_NT )

void
FileIO::Truncate( offL_t offset, Error *e )
{
	// Don't bother if non-existent.

	if( !( Stat() & FSF_EXISTS ) )
	    return;

# ifdef HAVE_TRUNCATE
	if( truncate( Name(), offset ) >= 0 )
	    return;
# endif // HAVE_TRUNCATE

	e->Sys( "truncate", Name() );
}

void
FileIO::Truncate( Error *e )
{
	// Don't bother if non-existent.

	if( !( Stat() & FSF_EXISTS ) )
	    return;

	// Try truncate first; if that fails (as it will on secure NCR's),
	// then open O_TRUNC.

# ifdef HAVE_TRUNCATE
	if( truncate( Name(), 0 ) >= 0 )
	    return;
# endif // HAVE_TRUNCATE

	int fd;

# if !defined ( OS_MACOSX ) || OS_VER >= 1010
	if( ( fd = checkFd( openL( Name(), O_WRONLY|O_TRUNC, PERM_0666 ) ) ) >= 0 )
# else
	if( ( fd = checkFd( openL( Name(), O_WRONLY|O_TRUNC ) ) ) >= 0 )
# endif // OS_MACOSX
	{
	    close( fd );
	    return;
	}

	e->Sys( "truncate", Name() );
}

# endif // !defined( OS_NT )

/*
 * FileIO::Stat() - return flags if file exists
 */

# ifndef OS_NT

int
FileIO::Stat()
{
	// Stat & check for missing, special

	int flags = 0;
	struct statbL sb;

# ifdef HAVE_SYMLINKS
	// With symlinks, we first stat the link
	// and if it is a link, then stat the actual file.
	// A symlink without an underlying file appears
	// as FSF_SYMLINK.  With an underlying file
	// as FSF_SYMLINK|FSF_EXISTS.

	if( lstatL( Name(), &sb ) < 0 )
	    return flags;

	if( S_ISLNK( sb.st_mode ) )
	    flags |= FSF_SYMLINK;

	if( S_ISLNK( sb.st_mode ) && statL( Name(), &sb ) < 0 )
	    return flags;
# else
	// No symlinks: just stat the file.

	if( statL( Name(), &sb ) < 0 )
	    return flags;
# endif

	flags |= FSF_EXISTS;

	if( sb.st_mode & S_IWUSR ) flags |= FSF_WRITEABLE;
	if( sb.st_mode & S_IXUSR ) flags |= FSF_EXECUTABLE;
	if( S_ISDIR( sb.st_mode ) ) flags |= FSF_DIRECTORY;
	if( !S_ISREG( sb.st_mode ) ) flags |= FSF_SPECIAL;
	if( !sb.st_size ) flags |= FSF_EMPTY;

	// Cache mtime+size from this stat so a subsequent StatModTime() or
	// GetSize() can skip a redundant statL.  Skip symlinks: lstatL above
	// returns the link's stats and a later by-path statL would resolve
	// to the target's stats; don't let the cache disagree.

	if( allowStatCache && !( flags & FSF_SYMLINK ) )
	{
	    cachedModTime = sb.st_mtime;
	    cachedSize    = sb.st_size;
	    statCacheValid = 1;
	}

# if defined ( OS_DARWIN ) || defined( OS_MACOSX )
	// If the immutable bit is set, we can't write this file
	// Chmod() will unset the immutable bit if it needs to.
	//
	if( sb.st_flags & UF_IMMUTABLE ) flags &= ~FSF_WRITEABLE;
# endif

	return flags;
}

# endif

# if !defined( OS_NT )
int
FileIO::GetOwner()
{
	int uid = 0;
	struct statbL sb;

# ifdef HAVE_SYMLINKS
	// With symlinks, we first stat the link
	// and if it is a link, then stat the actual file.
	// A symlink without an underlying file appears
	// as FSF_SYMLINK.  With an underlying file
	// as FSF_SYMLINK|FSF_EXISTS.

	if( lstatL( Name(), &sb ) < 0 )
	    return uid;

	if( S_ISLNK( sb.st_mode ) && statL( Name(), &sb ) < 0 )
	    return uid;
# else
	// No symlinks: just stat the file.

	if( statL( Name(), &sb ) < 0 )
	    return uid;
# endif

	return sb.st_uid;
}
# endif

# if !defined( OS_NT )
bool
FileIO::HasOnlyPerm( FilePerm perms )
{
	struct statbL sb;
	mode_t modeBits = 0;

	if( statL( Name(), &sb ) < 0 )
	    return false;

	switch (perms)
	{
	case FPM_RO:
	    modeBits = PERM_0222;
	    break;
	case FPM_RW:
	    modeBits = PERM_0666;
	    break;
	case FPM_ROO:
	    modeBits = PERM_0400;
	    break;
	case FPM_RXO:
	    modeBits = PERM_0500;
	    break;
	case FPM_RWO:
	    modeBits = PERM_0600;
	    break;
	case FPM_RWXO:
	    modeBits = PERM_0700;
	    break;
	}
	/*
	 * In this case we want an exact match of permissions
	 * We don't want to "and" to a mask, since we also want
	 * to verify that the other bits are off.
	 */
	if( (sb.st_mode & PERMSMASK) == modeBits )
		return true;

	return false;
}
# endif // not defined OS_NT

# if !defined( OS_NT )
P4INT64
FileIO::StatAccessTime()
{
	struct statbL sb;

	if( statL( Name(), &sb ) < 0 )
	    return 0;

	return DateTime::Centralize( sb.st_atime );
}

P4INT64
FileIO::StatModTime()
{
	// Reuse the mtime stamped on the FileSys by the most recent Stat()
	// when the lean-walk caller has enabled the cache.

	if( allowStatCache && statCacheValid )
	    return DateTime::Centralize( cachedModTime );

	struct statbL sb;

	if( statL( Name(), &sb ) < 0 )
	    return 0;

	return DateTime::Centralize( sb.st_mtime );
}

void
FileIO::StatModTimeHP(DateTimeHighPrecision *modTime)
{
	struct statbL sb;

	if( statL( Name(), &sb ) < 0 )
	{
	    *modTime = DateTimeHighPrecision();
	    return;
	}

	P4INT64	seconds = DateTime::Centralize( sb.st_mtime );
	int	nanosecs = 0;

// nanosecond support for stat is a bit of a portability mess
#if defined(OS_LINUX) || defined(OS_FREEBSD)
  #if defined(__GLIBC__) && defined(__GLIBC_PREREQ)
    #if defined(_BSD_SOURCE) || defined(_SVID_SOURCE) \
	|| (__GLIBC_PREREQ(2, 12) \
	    && ((_POSIX_C_SOURCE >= 200809L) || (_XOPEN_SOURCE >= 700)))
	nanosecs = sb.st_mtim.tv_nsec;
    #else
	nanosecs = sb.st_mtimensec;
    #endif
  #else   
	nanosecs = sb.st_mtim.tv_nsec;
  #endif
#elif defined(OS_MACOSX) && OS_VER < 1010
	/*
	 * HFS+ stores timestamps in 1-second resolution
	 * so nanosecs will always be zero, but maybe
	 * someone will run on a filesystem that does support
	 * finer-grained timestamps (eg, ext4).
	 */
	nanosecs = sb.st_mtimespec.tv_nsec;
#endif

	*modTime = DateTimeHighPrecision( seconds, nanosecs );
}

# endif

/*
 * FileIO::Unlink() - remove single file (error optional)
 */

# if !defined( OS_VMS ) && !defined( OS_NT )

void
FileIO::Unlink( Error *e )
{

# if defined(OS_OS2) || defined(OS_DARWIN) || defined(OS_MACOSX)

	// yeech - must be writable to remove
	// even on Darwin because the uchg flag might be set
	// but only for real files, otherwise the chmod
	// will propagate to the target file.

	Chmod( FPM_RW, 0 );

# endif

	if( *Name() && unlink( Name() ) < 0 && e )
	    e->Sys( "unlink", Name() );
}

void
FileIO::UnlinkNoRetry( Error *e )
{
	Unlink( e );
}

void
FileIO::Chmod( FilePerm perms, Error *e )
{
	// Don't set perms on symlinks

	if( ( GetType() & FST_MASK ) == FST_SYMLINK )
	    return;

	// Permissions for readonly/readwrite, exec vs no exec

	int bits = IsExec() ? PERM_0777 : PERM_0666;

	switch( perms )
	{
	case FPM_RO: bits &= ~PERM_0222; break;
	case FPM_ROO: bits &= ~PERM_0266; break;
	case FPM_RWO: bits = PERM_0600; break; // for key file, set exactly to rwo
	case FPM_RXO: bits = PERM_0500; break;
	case FPM_RWXO: bits = PERM_0700; break;
	}

	if( chmod( Name(), bits & ~global_umask ) >= 0 )
	    return;

# if defined( OS_MACOSX ) || defined( OS_DARWIN )

	// On mac, try unlocking the immutable bit and chmoding again.

	struct stat sb;

	if( stat( Name(), &sb ) >= 0 &&
	    chflags( Name(), sb.st_flags & ~UF_IMMUTABLE ) >= 0 &&
	    chmod( Name(), bits & ~global_umask ) >= 0 )
		return;
# endif

	// Can be called with e==0 to ignore error.

	if( e )
	    e->Sys( "chmod", Name() );
}

# endif /* !OS_VMS && !OS_NT */

/*
 * FileIO::SetExtendedAttribute() - set/remove extended attr (error optional)
 * FileIO::SetExtendedAttributes() - set/remove extended attr (error optional)
 * FileIO::GetExtendedAttribute() - get extended attr (error optional)
 * FileIO::GetExtendedAttributes() - get all extended attrs (error optional)
 */

# ifdef HAVE_XATTRS

# if defined( OS_MACOSX ) || defined( OS_DARWIN )

ssize_t
getxattr( const char *path, const char *name, void *value, size_t sizes )
{
	return getxattr( path, name, value, sizes, 0, 0 );
}

ssize_t
listxattr( const char *path, char *namebuf, size_t size )
{
	return listxattr( path, namebuf, size, 0 );
}

int
setxattr( const char *path, const char *name, void *value, size_t size,
          int flags )
{
	return setxattr( path, name, value, size, 0, flags );
}

int
removexattr( const char *path, const char *name )
{
	return removexattr( path, name, 0 );
}

# endif // OS_MACOSX || OS_DARWIN

# if defined( OS_FREEBSD )

ssize_t
getxattr( const char *path, const char *name, void *value, size_t sizes )
{
	return extattr_get_file( path, EXTATTR_NAMESPACE_USER,
	    name, value, sizes );
}

ssize_t
listxattr( const char *path, char *namebuf, size_t size )
{
	return extattr_list_file( path, EXTATTR_NAMESPACE_USER,
	    namebuf, size );
}

int
setxattr( const char *path, const char *name, void *value, size_t size,
          int flags )
{
	return extattr_set_file( path, EXTATTR_NAMESPACE_USER,
	    name, value, size );
}

int
removexattr( const char *path, const char *name )
{
	return extattr_delete_file( path, EXTATTR_NAMESPACE_USER, name );
}

# endif // OS_FREEBSD

void
FileIO::SetExtendedAttribute( StrPtr *name, StrPtr *val, Error *e )
{
	if( !val || !val->Length() )
	{
	    if( removexattr( Name(), name->Text() ) >= 0 )
	        return;

	    // Can be called with e==0 to ignore error.

	    if( e )
	        e->Sys( "removexattr", Name() );

	    return;
	}
	
	if( setxattr( Name(), name->Text(),
	              val->Text(), val->Length(), 0 ) >= 0 )
	    return;

	// Can be called with e==0 to ignore error.

	if( e )
	    e->Sys( "setxattr", Name() );
}

void
FileIO::SetExtendedAttributes( StrDict *vals, Error *e )
{
	StrDictIterator *iter = vals->GetIterator();
	StrRef name, val;
	while( iter->Get( name, val ) && ( !e || !e->Test() ) )
	{
	    SetExtendedAttribute( &name, &val, e );
	    iter->Next();
	}
}

void
FileIO::GetExtendedAttribute( StrPtr *name, StrBuf *val, Error *e )
{
	val->Clear();

	StrBuf v;
	p4size_t len = 1024;
	v.Alloc( len );
	int ret;
	while( ( ret = getxattr( Name(), name->Text(), v.Text(), len ) ) )
	{
	    if( ret >= 0 )
	    {
	        val->Set( v.Text(), ret );
	        return;
	    }

	    if( errno == ERANGE )
	    {
	        len *= 2;
	        v.Alloc( len );
	    }
	    else
	        break;
	}

	// Can be called with e==0 to ignore error.

	if( e )
	    e->Sys( "getxattr", Name() );
}

void
FileIO::GetExtendedAttributes( StrBufDict *attrs, Error *e )
{
	attrs->Clear();

	StrBuf list;
	p4size_t len = 1024;
	list.Alloc( len );
	int ret;
	while( ( ret = listxattr( Name(), list.Text(), len ) ) )
	{
	    if( ret >= 0 )
	    {
	        char *buf = list.Text();
	        while( ret > 0 )
	        {
	            StrRef name( buf );
	            StrBuf tmp;
	            Error t;
	            GetExtendedAttribute( &name, &tmp, &t );
	            if( !t.Test() )
	                attrs->SetVar( name, tmp );

	            int l = strlen( buf ) + 1;
	            buf += l;
	            ret -= l;
	        }
	        return;
	    }

	    if( errno == ERANGE )
	    {
	        len *= 2;
	        list.Alloc( len );
	    }
	    else
	        break;
	}

	// No extended attributes
	if( ret == 0 )
	    return;

	// Can be called with e==0 to ignore error.

	if( e )
	    e->Sys( "listxattr", Name() );
}

# elif !defined( HAVE_XATTRS ) && !defined( OS_NT )

// Extended attribute support is not implemented on this platform
// Use no-op stubs

void
FileIO::SetExtendedAttribute( StrPtr *name, StrPtr *val, Error *e )
{
}

void
FileIO::SetExtendedAttributes( StrDict *vals, Error *e )
{
}

void
FileIO::GetExtendedAttribute( StrPtr *name, StrBuf *val, Error *e )
{
	val->Clear();
}

void
FileIO::GetExtendedAttributes( StrBufDict *attrs, Error *e )
{
	attrs->Clear();
}

# endif // !HAVE_XATTRS && !OS_NT

/*
 * FileIoText Support
 * FileIOBinary support
 */

FileIOBinary::~FileIOBinary()
{
	Cleanup();
}

# if !defined( OS_NT )

void
FileIOBinary::Open( FileOpenMode mode, Error *e )
{

	this->lastOSError = 0;

	// Save mode for write, close

	this->mode = mode;

	// Get bits for (binary) open

	int bits = openModes[ mode ].bflags;

	// Reset the isStd flag

	isStd = 0;

	// Handle exclusive open (must not already exist)

# ifdef O_EXCL
	// Set O_EXCL to ensure we create the file when we open it.

	if( GetType() & FST_M_EXCL )
	    bits |= O_EXCL;
# else
	// No O_EXCL: we'll manually check if file already exists.
	// Not atomic, but what can one do?

	if( ( GetType() & FST_M_EXCL ) && ( Stat() & FSF_EXISTS ) )
	{
	    e->Set( E_FAILED, "file exists" );

	    // if file is set delete on close unset that because we
	    // didn't create the file...
	    ClearDeleteOnClose();
	    return;
	}
# endif // O_EXCL

	// open delegate buffer, stdin/stdout or real file

	if( delegate )
	{
	    delegate->Open( Path(), mode, e );
	}
	else if( Name()[0] == '-' && !Name()[1] )
	{
	    // we do raw output: flush stdout
	    // for nice mixing of messages.

	    if( mode == FOM_WRITE )
		fflush( stdout );

	    fd = openModes[ mode ].standard;
	    checkStdio( fd );
	    isStd = 1;
	}
	else if( ( fd = checkFd( openL( Name(), bits, PERM_0666 ) ) ) < 0 )
	{
	    this->lastOSError = errno;
	    e->Sys( openModes[ mode ].modeName, Name() );
# ifdef O_EXCL
	    // if we failed to create the file probably due to the
	    // file already existing (O_EXCL)
	    // then unset delete on close because we didn't create it...
	    if( ( bits & (O_EXCL|O_CREAT) ) == (O_EXCL|O_CREAT) )
		ClearDeleteOnClose();
# endif
	}

}

// Return 1 if it make sense to retry a file create
// operation. Used in sys/filetmp.cc.
int
FileIOBinary::RetryCreate()
{
	if( lastOSError == EEXIST )
	    return 1;
	return 0;
}

void
FileIOBinary::Close( Error *e )
{
	if( delegate )
	{
	    delegate->Close( e );
	    return;
	}

	if( isStd || fd < 0 )
	    return;

	if( ( GetType() & FST_M_SYNC ) )
	    Fsync( e );

# ifdef OS_LINUX
	if( cacheHint && p4tunable.Get( P4TUNE_FILESYS_CACHEHINT ) )
	  (void) posix_fadvise64( fd, 0, 0, POSIX_FADV_DONTNEED );
# endif

	if( close( fd ) < 0 )
	    e->Sys( "close", Name() );

	fd = -1;

	if( mode == FOM_WRITE && modTime )
	    ChmodTime( modTime, e );

	if( mode == FOM_WRITE )
	    Chmod( perms, e );
}

# endif // !OS_NT

void
FileIOBinary::Fsync( Error *e )
{
# ifdef HAVE_FSYNC
	if( fd >= 0 && fsync( fd ) < 0 )
	    e->Sys( "fsync", Name() );
# endif
}

# if !defined( OS_NT )

void
FileIOBinary::Write( const char *buf, int len, Error *e )
{
	if( delegate )
	{
	    delegate->Write( buf, len, e );

	    if( checksum && !e->Test() )
	        checksum->Update( StrRef( buf, len ) );

	    return;
	}

	// Raw, unbuffered write

	int l;

	if( ( l = write( fd, buf, len ) ) < 0 )
	    e->Sys( "write", Name() );
	else
	    tellpos += l;

	if( checksum && l > 0 )
	    checksum->Update( StrRef( buf, l ) );
}

int
FileIOBinary::Read( char *buf, int len, Error *e )
{
	if( delegate )
	    return delegate->Read( buf, len, e );

	// Raw, unbuffered read

	int l;

	if( ( l = read( fd, buf, len ) ) < 0 )
	    e->Sys( "read", Name() );
	else
	    tellpos += l;

	if( checksum && l > 0 )
	    checksum->Update( StrRef( buf, l ) );

	return l;
}

offL_t
FileIOBinary::GetSize()
{
	// When the file isn't open, reuse the size from the most recent
	// Stat() if the cache is enabled.  An open fd may have grown since
	// then, so let the fstatL path run live.

	if( fd < 0 && allowStatCache && statCacheValid )
	    return cachedSize;

	struct statbL sb;

	if( fd >= 0 && fstatL( fd, &sb ) < 0 )
	    return -1;
	if( fd < 0 && statL( Name(), &sb ) < 0 )
	    return -1;

	return sb.st_size;
}

void
FileIOBinary::Seek( offL_t offset, Error *e )
{
	if( delegate )
	{
	    delegate->Seek( offset, e );
	    return;
	}

	if( lseekL( fd, offset, 0 ) == -1 )
	    e->Sys( "seek", Name() );
	tellpos = offset;
}

// Return the links in the filesystem to this file, or -1
// on an error.
int
FileIOBinary::LinkCount()
{
	struct statbL sb;
	if( fd < 0 || fstatL( fd, &sb ) < 0 )
	    return -1;
	return sb.st_nlink;
}


# endif // !OS_NT

offL_t
FileIOBinary::Tell()
{
	return tellpos;
}

void
FileIOBinary::DepotSize( offL_t &len, Error *e )
{
	len = GetSize();
}


FileIODir::FileIODir()
{
}

FileIODir::~FileIODir()
{
	Cleanup();
}

void
FileIODir::Open( FileOpenMode mode, Error *e )
{
}

void
FileIODir::Write( const char *buf, int len, Error *e )
{
}

int
FileIODir::Read( char *buf, int len, Error *e )
{
	return 0;
}

void
FileIODir::Close( Error *e )
{
}

FD_PTR
FileIODir::GetFd()
{
	return FD_ERR;
}

offL_t
FileIODir::GetSize()
{
	return 0;
}

void
FileIODir::Seek( offL_t offset, Error *e )
{
}

offL_t
FileIODir::Tell()
{
	return 0;
}

void
FileIODir::Fsync( Error *e )
{
}

void
FileIODir::Cleanup()
{
	Error e;

	if( IsDeleteOnClose() && Path()->Length() )
	    PurgeDir( Path()->Text(), &e );
}

/*
 * FileIOAppend support
 *
 * On UNIX, FileIOAppend::Write() and Rename() obey a special
 * protocol to provide for atomic rotating of files that may
 * be open by active processes.
 *
 * Rename() opens & locks the file, renames it, and then changes
 * the permission on the renamed file to read-only.
 *
 * Write() locks and then fstats the file, to see if it is still
 * writeable.  If not, it assumes it has been renamed, so it closes
 * and reopens the original file (which should always be writeable),
 * and then relocks it.  It can then write and unlock.
 *
 * There is the possibility that rapid fire Rename() calls may get
 * between Write()'s reopen for write and its lock.  In this odd case,
 * we retry 10 times.
 */

FileIOAppend::~FileIOAppend()
{
	// Need to set mode to read so it doesn't try to chmod the
	// file in the destructor

	mode = FOM_READ;
}

# if !defined( OS_NT )

void
FileIOAppend::Open( FileOpenMode mode, Error *e )
{
	// Save mode for write, close

	this->mode = mode;

	// Reset the isStd flag

	isStd = 0;

	// open stdin/stdout or real file

	if( Name()[0] == '-' && !Name()[1] )
	{
	    fd = openModes[ mode ].standard;
            checkStdio( fd );
	    isStd = 1;
	}
	else if( ( fd = checkFd( openL( Name(),
			 openModes[ mode ].aflags,
			 PERM_0666 ) ) ) < 0 )
	{
	    e->Sys( openModes[ mode ].modeName, Name() );
	    ClearDeleteOnClose();
	}

	// Clear send/receive buffers

	rcv = snd = 0;
}

offL_t
FileIOAppend::GetSize()
{
	offL_t s = 0;

	if( !lockFile( fd, LOCKF_SH ) )
	{
	    s = FileIOBinary::GetSize();

	    lockFile( fd, LOCKF_UN );
	}
	else
	    s = FileIOBinary::GetSize();

	return s;
}

offL_t
FileIOAppend::GetCurrentSize()
{
	offL_t s;

	// Get the size of the current file (by path), not of a recently
	// rename()'d file that still happens to be open on this->fd.

	FileSys *f = FileSys::Create( FST_BINARY );
	if( f )
	{
	    f->Set( path );
	    s = f->GetSize();
	    delete f;
	}
	else
	{
	    s = -1;
	}

	return s;
}

void
FileIOAppend::Write( const char *buf, int len, Error *e )
{
	// Lock and check for writeability.
	// If made read-only by Rename(), close and reopen.
	// If file keeps becomming read-only, give up after
	// trying a while.

	int tries = 10;

	while( --tries )
	{
	    struct statbL sb;

	    if( lockFile( fd, LOCKF_EX ) < 0 )
	    {
		e->Sys( "Write() lock", Name() );
		return;
	    }

	    if( fstatL( fd, &sb ) < 0 )
	    {
		e->Sys( "Write() fstat", Name() );

		if( lockFile( fd, LOCKF_UN ) < 0 )
		    e->Sys( "Write() unlock", Name() );

		return;
	    }

	    if( sb.st_mode & S_IWUSR )
		break;

	    if( close( fd ) < 0 )
	    {
		e->Sys( "Write() close", Name() );

		if( lockFile( fd, LOCKF_UN ) < 0 )
		    e->Sys( "Write() unlock", Name() );

		return;
	    }

	    Open( mode, e );

	    if( e->Test() )
		return;
	}

	if( !tries )
	{
	    e->Set( E_FAILED, "Tired of waiting for %file% to be writeable." )
		<< Name();
	    return;
	}

	FileIOBinary::Write( buf, len, e );

	if( lockFile( fd, LOCKF_UN ) < 0 )
	{
	    e->Sys( "Write() unlock", Name() );
	    return;
	}
}

void
FileIOAppend::Rename( FileSys *target, Error *e )
{
	// Open, lock for write, rename, chmod to RO, and close

	Open( FOM_WRITE, e );

	if( e->Test() )
	    return;

	if( lockFile( fd, LOCKF_EX ) < 0 )
	{
	    e->Sys( "Rename() lock", Name() );
	    Close( e );
	    return;
	}

	if( rename( Name(), target->Name() ) < 0 )
	{
	    // Uh oh -- we can't rename.  Maybe cross device?
	    // Do the dumb way: copy/truncate.

	    if( p4debug.GetLevel( DT_SERVER ) >= 4 )
	        p4debug.printf(
	            "Rename failed, doing copy/truncate instead: %s %s %s\n",
	            Name(), target->Name(), strerror( errno ) );

	    mode = FOM_READ;

	    if( lockFile( fd, LOCKF_UN ) < 0 )
	    {
		e->Sys( "Rename() UNLOCK for copying", Name() );
		// try to keep going
	    }

	    Close( e );
	    Copy( target, FPM_RO, e );

	    if( e->Test() )
		return;

	    Truncate( e );
	    return;
	}

	target->Chmod( FPM_RO, e );

	if( lockFile( fd, LOCKF_UN ) < 0 )
	{
	    e->Sys( "Rename() UNLOCK", Name() );
	    // let this fall info the next e->Test
	}

	// Need to set mode to read so it doesn't try to chmod the
	// file in the destructor

	mode = FOM_READ;

	if( e->Test() )
	{
	    Close( e );
	    return;
	}

	struct statbL sb;

	if( fstatL( fd, &sb ) < 0 )
	{
	    e->Sys( "Rename() fstat", Name() );
	    Close( e );
	    return;
	}

	if( sb.st_mode & S_IWUSR )
	{
	    P4INT64 bigINode = sb.st_ino;
	    P4INT64 bigMode  = sb.st_mode;
	    e->Set( MsgOs::ChmodBetrayal ) <<
	            Name() << target->Name() <<
	            StrNum( bigMode ) << StrNum( bigINode );
	    Close( e );
	    return;
	}

	Close( e );
}

# endif

void
FileIOAppend::Close( Error *e )
{
	mode = FOM_READ;

	FileIOBuffer::Close( e );
}

