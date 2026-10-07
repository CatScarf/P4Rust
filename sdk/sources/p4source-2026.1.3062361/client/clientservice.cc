/*
 * Copyright 1995, 2001 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# ifdef USE_EBCDIC
# define NEED_EBCDIC
# endif

# include <stdhdrs.h>

# ifdef HAS_EXTENSIONS
# include <vector>
# endif

#include <stddef.h>

# include <strbuf.h>
# include <strdict.h>
# include <strops.h>
# include <strarray.h>
# include <strtable.h>
# include <strtree.h>
# include <error.h>
# include <mapapi.h>
# include <runcmd.h>
# include <handler.h>
# include <rpc.h>
# include <md5.h>
# include <mangle.h>
# include <i18napi.h>
# include <charcvt.h>
# include <transdict.h>
# include <debug.h>
# include <datetime.h>
# include <tunable.h>
# include <ignore.h>
# include <timer.h>
# include <progress.h>

# ifdef USE_CDC
# include <blake3digester.h>
# include <vararray.h>
# include <intarray.h>
# include <vartree.h>
# include <strintstree.h>
# include <chunkmap.h>
# endif

# include <p4tags.h>

# include <filesys.h>
# include <pathsys.h>
# include <fileio.h>
# include <enviro.h>
# include <ticket.h>

# include <msgscript.h>
# include <dmextension.h>
# include <dmextension_c.h>

# include "clientmerge.h"
# include "clientresolvea.h"
# include "clientuser.h"

# include <msgclient.h>
# include <msgsupp.h>
# include <msgrpc.h>

# include "clientservice.h"
# include "clientscript.h"
# include "client.h"
# include "clientprog.h"
# include "clientprogressreport.h"
# include "clientaltsynchandler.h"

# define SSOMAXLENGTH 131072    // max sso message 128k

ClientFile::ClientFile( FileSys *fs )
{
	file = fs;
	indirectFile = 0;
	isDiff = 0;
	checksum = 0;
	matchDict = 0;
	progress = 0;

# ifdef USE_CDC
	chunkOffsetTree = 0;
# endif
}

ClientFile::~ClientFile()
{
	if( file )
	    file->SetDigest( NULL );
	delete file;
	delete indirectFile;
	delete checksum;
	delete matchDict;
	delete progress;

# ifdef USE_CDC
	delete chunkOffsetTree;
# endif
}

/*
 * ProgressHandle - progress indicator handle
 */

class ProgressHandle : public LastChance {
    public:
	ProgressHandle( ClientProgress *p ) : progress( p ) {}
	~ProgressHandle() { delete progress; }

	ClientProgress *progress;
};

/*
 * ClientVarHandle - arbitrary large variable handle
 */

class ClientVarHandle : public LastChance
{
    public:
	ClientVarHandle( offL_t size, int parts, Error *e ) :
	    parts( parts ),
	    partsRecved( 0 ),
	    bytes( size ),
	    bytesRecved( 0 )
	{
	    deleteOnRelease = 1;

	    // Allocate the space for the variable
	    // If this is over a certain size, we should probably use a tmpfile
	    // Might need to catch and handle too out-of-memory errors?

	    if( size > 0xFFFFFFFF )
	    {
		// Bigger than 32bit range!
		e->Set( MsgClient::DataOutOfBounds );
		SetError( e );
		return;
	    }

	    memStore.Alloc( size );
	}

	~ClientVarHandle()
	{
	}

	void Append( int part, offL_t offset, const StrPtr *data, Error *e )
	{
	    // Silent on prior error
	    if( IsError() )
		return;

	    if( partsRecved > parts ||
		part > parts ||
		data->Length() + offset > bytes )
	    {
		e->Set( MsgClient::DataOutOfBounds );
		SetError( e );
		return;
	    }

	    memcpy( memStore.Text() + offset, data->Text(), data->Length() );
	    bytesRecved += data->Length();
	    partsRecved += 1;
	}

	StrBuf	    memStore;

    private:
	int	    parts;
	int	    partsRecved;
	offL_t	    bytes;
	offL_t	    bytesRecved;

} ;

FileSysType 
LookupType( const StrPtr *type, Error *e )
{
	if( !type ) return FST_TEXT;

	if( type->Length() > 3 )
	{
	    e->Set( MsgClient::BadFiletype ) << type;

	    // This shouldn't happen, but in at least 1 place the server
	    // sends "binary+F"
	    if( !strncmp( "binary", type->Text(), 6 ) )
	        return FST_BINARY;
	}

	/*
	 * For types see DmtFileType::ClientPart() in dmtypes.h.
	 *
	 * For backward and forward compatibility the following bits 
	 * have be reserved (*type - '0'):
	 *
	 * 	0x01: generally text(0)/binary(1)
	 *	0x02: executable
	 *
	 * NB prior to 99.1 the 'unknowns' didn't acknowledge the
	 * exec bit, and were just plain files.
	 *
	 * NB 2000.2 appletext is no different from applefile: its
	 * sole purpose is to support 'diff -t' on applefile.
	 */

	// FileSysType - Can't do |'s on enums, so it's an int for now.

	int t;

	// fileType [ lineType [ uncompress ] ]

	int tf = 0;
	int tl = 0;
	int tu = 0;

	switch( type->Length() )
	{
	default:
	case 3: tu = StrOps::XtoO( (*type)[2] );
	case 2: tl = StrOps::XtoO( (*type)[1] );
	case 1: tf = StrOps::XtoO( (*type)[0] );
	case 0: /* nothing??? */;
	}
	
	if( tu > 1 )
	    e->Set( MsgClient::BadUncompressFlag ) << tu << type;
	if( tl > 4 )
	    e->Set( MsgClient::BadLineEndingFlag ) << tl << type;

	// Map '[ uncompress ] fileType' into FileSysType.

	switch( tf )
	{

	    // Normal.

	case 0x000: t = FST_TEXT;			break; 
	case 0x001: t = FST_BINARY;			break; 
	case 0x002: t = FST_TEXT 	| FST_M_EXEC; 	break; 
	case 0x003: t = FST_BINARY 	| FST_M_EXEC;	break; 
	case 0x004: t = FST_SYMLINK;			break;
	case 0x005: t = FST_RESOURCE;			break; 
	case 0x006: t = FST_SYMLINK 	| FST_M_EXEC;	break;	// disallowed
	case 0x007: t = FST_RESOURCE 	| FST_M_EXEC;	break;  // disallowed
	case 0x008: t = FST_UNICODE;			break;	// 2001.1
	case 0x009: t = FST_RTEXT;			break;	// 99.1
	case 0x00A: t = FST_UNICODE 	| FST_M_EXEC;	break;	// 2001.1
	case 0x00B: t = FST_RTEXT 	| FST_M_EXEC;	break;	// 99.1
	case 0x00C: t = FST_APPLETEXT;			break;	// 2000.2
	case 0x00D: t = FST_APPLEFILE;			break;	// 99.2 
	case 0x00E: t = FST_APPLETEXT 	| FST_M_EXEC;	break;	// 2000.2
	case 0x00F: t = FST_APPLEFILE 	| FST_M_EXEC;	break;	// 99.2 
	case 0x014: t = FST_UTF8;			break;	// 2015.1
	case 0x016: t = FST_UTF8 	| FST_M_EXEC;	break;	// 2015.1
	case 0x018: t = FST_UTF16;			break;	// 2007.2
	case 0x01A: t = FST_UTF16 	| FST_M_EXEC;	break;	// 2007.2

	    // Stop-gap.

	default:   t = FST_BINARY;			break;

	}

	if( tu )
	    t |= FST_C_GUNZIP;

	// And the lineType
	// 0x0 -> FST_L_LOCAL == 0, so don't bother.

	// If linetype already set,  don't | in the linetype bits.
	// 
	// The only way that linetype will be set after setting base
	// type is if FST_RTEXT has been selected (this is a binary file
	// being treated as text, usually for the purpose of diff/resolve).
	//
	// Note:  we can't overwrite the linetype because there is
	// a danger that we will write a file to the client workspace
	// with line-end translations,  but when submit happens it
	// will be treated as binary (adding extra characters to the
	// file stored by the server).

	if( !(t & FST_L_MASK) )
	{
	    switch( tl )
	    {
	    case 0x1: t |= FST_L_LF; break;
	    case 0x2: t |= FST_L_CR; break;
	    case 0x3: t |= FST_L_CRLF; break;
	    case 0x4: t |= FST_L_LFCRLF; break;
	    }
	}

	return FileSysType( t );
}

CharSetCvt *
ClientSvc::XCharset( Client *client, XDir d )
{
	CharSetCvt::CharSet trans_charset =
	    (CharSetCvt::CharSet) client->ContentCharset();

	switch( d )
	{
	case FromClient:
	    return CharSetCvt::FindCachedCvt( client->gCharSetCvtCache,
	                                      trans_charset,CharSetCvt::UTF_8 );
	    break;
	case FromServer:
	    return CharSetCvt::FindCachedCvt( client->gCharSetCvtCache,
	                                     CharSetCvt::UTF_8, trans_charset );
	    break;
	}

	return 0;
}

bool
ClientSvc::SizeMatchesDepot( FileSysType t, CharSetCvt *cvt )
{
	// Byte-changing translation comes from three places: the line-end bits,
	// the charset converter, and the type-internal converter that
	// FileIOUnicode/UTF8/UTF16 install on Open() (see fileiouni.cc), which
	// cvt does not show.  LineTypeLocal == LineTypeRaw resolves at compile
	// time per platform.

	FileSysType base = (FileSysType)( t & FST_MASK );
	FileSysType lf   = (FileSysType)( t & FST_L_MASK );

	// Apple and resource types read through a fork-combining stream, so
	// their stored size is not the on-disk count whatever their line-end
	// bits say, and their base nibble reads as ordinary text or binary.

	if( ( t & FST_M_APPLE ) || base == FST_RESOURCE )
	    return false;

	// IsUnicode() and IsTextual() are instance methods; asked here of the
	// bare type.

	int isUnicode = base == FST_UNICODE || base == FST_UTF8 ||
	                base == FST_UTF16;
	int isTextual = isUnicode || base == FST_TEXT;

	return !isTextual
	    || ( !isUnicode &&
	         !cvt &&
	         ( lf == FST_L_LF ||
	           ( lf == FST_L_LOCAL && LineTypeLocal == LineTypeRaw ) ) );
}

FileSys *
ClientSvc::FileFromPath( Client *client, const char *vName, Error *e )
{
	return FileFromPath( client, vName, P4Tag::v_type, e );
}

FileSys *
ClientSvc::FileFromPath( Client *client, const char *vName, const char *vType,
	                 Error *e )
{
	StrPtr *clientPath = client->transfname->GetVar( vName, e );
	StrPtr *clientType = vType ? client->GetVar( vType ) : 0;
	StrPtr *utf8bom = client->GetVar( P4Tag::v_utf8bom );

	if( e->Test() )
	    return 0;

	FileSysType type = LookupType( clientType, e );
	if( e->Test() && client->CheckFileType() )
	    return 0;
	else
	    e->Clear();

	return ClientSvc::FileFromPathNoVar( client, clientPath, type,
	                                     utf8bom, e );
}

FileSys *
ClientSvc::FileFromPathNoVar( Client *client, const StrPtr *clientPath,
	                      const FileSysType type,
	                      const StrPtr *utf8bom, Error *e )
{
	FileSys *f = client->GetUi()->File( type );

	f->SetContentCharSetPriv( client->ContentCharset() );
	f->Set( *clientPath, e );
	if( e->Test() )
	{
	    delete f;
	    client->OutputError( e );
	    return 0;
	}

	if( !CheckFilePath( client, f, e ) )
	{
	    client->OutputError( e );
	    delete f;
	    return 0;
	}

	// If we've been instructed to override the BOM on UTF8, do it now
	if( (type & FST_MASK) == FST_UTF8 && utf8bom && utf8bom->IsNumeric() )
	{
	    int b = utf8bom->Atoi();
	    int c = (int)CharSetApi::UTF_8_BOM;
# ifdef OS_NT
	    if( b == 0 )
	        c = (int)CharSetApi::UTF_8;
# else
	    if( b != 1 ) // meaning either 0 or 2
	        c = (int)CharSetApi::UTF_8;
# endif
	    f->SetContentCharSetPriv( c );
	}

	return f;
}

int
ClientSvc::CheckFilePath( Client *client, FileSys *f, Error *e,
	                  int skipClientPath )
{
	if( client->CheckSystemIgnore( f->Name() ) )
	{
	    e->Set( MsgClient::PathProtected ) << f->Name();
	    return 0;
	}

	if( !skipClientPath &&
	    !f->IsUnderPath( client->GetClientPath() ) &&
	    !f->IsUnderPath( client->GetTempPath() ) )
	{
	    e->Set( MsgClient::NotUnderPath )
	        << f->Name() << client->GetClientPath();
	    return 0;
	}
	return 1;
}

FileSys *
ClientSvc::File( Client *client, Error *e )
{
	return FileFromPath( client, P4Tag::v_path, e );
}

/*
 * Client Service -- top half 
 */

void
Client::OutputError( Error *e )
{
	if( e->Test() )
	{
	    // Note error so we can adjust exit()
	    // Format and output

	    SetError();
	    GetUi()->HandleError( e );
	    e->Clear();
	    ClearSecretKey();
	    ClearPBuf();
	}
}

// Find the number of entries in given directory
int clientDirectoryEntryCount( StrPtr *dir, Error *e )
{
	PathSys *path = PathSys::Create();
	path->Set( dir );
	FileSys *d = FileSys::Create( FST_TEXT );
	d->Set( *path);

	int uaCount = 0;

	if( d->Stat() & FSF_DIRECTORY )
	{
	    StrArray *ua = d->ScanDir( e );

	    if( ua )
	        uaCount = ua->Count();

	    delete ua;
	}

	delete d;
	delete path;

	return uaCount;
}


// Client Service -- bottom half
// File support

// clientBailoutFile - called if handle is never deleted

// If set, we will not write client side files during a sync.
MT_STATIC int client_nullsync;

FileDigestType
clientFileDigestType( StrPtr *digestType )
{
	FileDigestType digType = FS_DIGEST_UNKNOWN;

	if( !digestType->Compare(
			StrRef( P4Tag::v_digestTypeMD5 ) ) )
	    digType = FS_DIGEST_MD5;
	else if( !digestType->Compare(
			StrRef( P4Tag::v_digestTypeGitText ) ) )
	    digType = FS_DIGEST_GIT_TEXT_SHA1;
	else if( !digestType->Compare(
			StrRef( P4Tag::v_digestTypeGitBinary ) ) )
	    digType = FS_DIGEST_GIT_BINARY_SHA1;
	else if( !digestType->Compare(
			StrRef( P4Tag::v_digestTypeSHA256 ) ) )
	    digType = FS_DIGEST_SHA256;

	return digType;
}

void
clientOpenFile( Client *client, Error *e )
{
	++client->recvClientTotal;
	if( (client_nullsync = p4tunable.Get( P4TUNE_FILESYS_CLIENT_NULLSYNC) ) )
	    return;

	ClientFile *f;
	FileSys *fs = 0;

	client->NewHandler();
	StrPtr *clientPath = client->transfname->GetVar( P4Tag::v_path, e );
	StrPtr *clientHandle = client->GetVar( P4Tag::v_handle, e );
	StrPtr *modTime = client->GetVar( P4Tag::v_time );
	StrPtr *noclobber = client->GetVar( P4Tag::v_noclobber );
	StrPtr *sizeHint = client->GetVar( P4Tag::v_fileSize );
	StrPtr *svrSize = client->GetVar( P4Tag::v_serverSize );
	StrPtr *perms = client->GetVar( P4Tag::v_perms );
	StrPtr *func = client->GetVar( P4Tag::v_func, e );
	StrPtr *diffFlags = client->GetVar( P4Tag::v_diffFlags );
	StrPtr *digest = client->GetVar( P4Tag::v_digest );
	StrPtr *digestType = client->GetVar( P4Tag::v_digestType );
	bool do_checksum = false;

	if( noclobber && *noclobber == P4Tag::v_false )
	    noclobber = 0;

	// clear syncTime

	client->SetSyncTime( 0 );

	if( e->Test() )
	{
	    if( !e->IsFatal() )
	    {
		f = new ClientFile( NULL );
		client->handles.Install( clientHandle, f, e );
		goto bail;
	    }
	    return;
	}

	if ( digestType )
	{
	    // Requires check on digest if present, hence
	    // before opening the file, check if it exist
	    if ( FileSys::FileExists( clientPath->Text() ) )
	    {
		do_checksum = true;
	    }
	}

	// Create file object for writing.
	// Set binary/text/etc file type

	fs = ClientSvc::File( client, e );
	f = new ClientFile( fs );

	if( !fs )
	    e->Set( MsgClient::FileOpenError );
	if( e->Test() )
	{
	    f->SetError( e );
	    e->Clear();
	}

	// Create handle.
	// Toss file if we can't save handle.

	client->handles.Install( clientHandle, f, e );

	if( e->Test() )
	{
	    delete f;
	    return;
	}
	if( f->IsError() )
	    return;

	// Clear previous errors set by a failed refresh (Bug 30647).
	// Ignore return value.

	if( *clientHandle == "sync" )
	    client->handles.AnyErrors( clientHandle );

	// Prepare write/diff, as appropriate.
	// Before 97.2, the default was to clobber files and there wasn't
	// a noclobber var, so now the lack of noclobber means clobber.
	// Clobbering means writing an already writeable file.

	if( *func == P4Tag::c_OpenDiff || *func == P4Tag::c_OpenMatch )
	{
	    // Set up to be a diff
	    // We save the real name as the altFile and
	    // replace the real name with a temp file.

	    f->isDiff = 1;
	    f->file->SetDeleteOnClose();
	    f->diffName.Set( *clientPath );

	    // Save diffFlags for diff operation.

	    if( diffFlags )
		f->diffFlags.Set( diffFlags );

	    // Make temp dir 

	    f->file->MakeGlobalTemp();

	    // N-way diff?  Save rpc vars for the close..

	    if( *func == P4Tag::c_OpenMatch )
		clientOpenMatch( client, f, e );
	}
	else
	{
	    // Handle noclobber.
	    if ( do_checksum )
	    {
		StrBuf localDigest;
		FileDigestType digType;

		digType = clientFileDigestType( digestType );

		fs->ComputeDigest( digType, &localDigest, e );
		if( e->Test() || localDigest.XCompare( *digest ) ) 
		{
		    e->Set( MsgClient::NoModifiedFile ) 
			<< "update"
			<< f->file->Name();
		    goto bail;
		}
	    }

	    int statFlags = f->file->Stat();

	    if( noclobber &&
		( statFlags & (FSF_WRITEABLE|FSF_SYMLINK) ) == FSF_WRITEABLE )
	    {
		e->Set( MsgClient::ClobberFile ) << f->file->Name();
		goto bail;
	    }

	    // If the target file exists and is not a special file
	    // i.e. a device, we write a temp file and
	    // arrange for Close() to rename it into place.

	    if( ( statFlags & ( FSF_SYMLINK | FSF_EXISTS ) )
		  && ! ( statFlags & FSF_SPECIAL )
		  && f->file->DoIndirectWrites() )
	    {
		f->indirectFile = f->file;

		f->file = client->GetUi()->File( f->file->GetType() );
		f->file->SetContentCharSetPriv(
		    f->file->GetContentCharSetPriv() );
		f->file->MakeLocalTemp( f->indirectFile->Name() );
		f->file->SetDeleteOnClose();
	    }
	    else if( statFlags & FSF_SYMLINK )
	    {
		// This case is the target is an existing symlink
		// and the new file can not do indirect writes, but
		// we do need to replace the symlink...
		// so... delete the symlink and just
		// transfer normally...

		// XXX maybe we should move the symlink to
		// a temp name and move it back if the transfer
		// fails?

		f->file->Unlink( e );

		if( e->Test() )
		{
		    goto bail;
		}

		// delete the file if transfer fails

		f->file->SetDeleteOnClose();
	    }
	    else if( ( statFlags & FSF_EXISTS ) )
	    {
		f->file->Chmod2( FPM_RW, e );
		// Ignore chmod: we'll get error at open
		e->Clear();
	    }
	    else
	    {
		// create directory path when writing a (new file)
		// We do this after checking to see if the file
		// exists, because the fstat() of Stat() will
		// trigger the automounter, while the access() of
		// MkDir() will not (we think).

		f->file->MkDir( e );

		if( e->Test() )
		{
		    e->Set( MsgClient::MkDir ) << f->file->Name();
		    goto bail;
		}

		// If we are writing a file that doesn't exist,
		// we'll want to delete it if we fail to write it
		// fully.

		f->file->SetDeleteOnClose();
	    }

	    // Set closing permissions and mod time.
	    // Done after indirectFile shuffle.

	    if( perms && *perms == "rw" )
		f->file->Perms( FPM_RW );

	    if( modTime )
		f->file->ModTime( modTime );

	    // Pre-allocate space,  only NT fileio will use this

	    if( sizeHint )
	        f->file->SetSizeHint( sizeHint->Atoi64() );

	    // If the server told us how much it's going to send,
	    // then we have give a process bar.
	    // If we got a size hint, we can use that too

	    ClientProgress *indicator;

	    if( svrSize && ( indicator =
                  client->GetUi()->CreateProgress( CPT_RECVFILE,
                                                   svrSize->Atoi64() ) ) )
	    {
	        f->progress = new ClientProgressReport( indicator );
	        f->progress->Description( *clientPath );
	        f->progress->Units( PRU_KBYTES );
	        f->progress->Total( (long)( svrSize->Atoi64() / 1024 ) );
	    }
	}

	// Actually open file

	f->file->Open( FOM_WRITE, e );

	f->symTarget.Clear();

	// 2010.2 can verify transfered contents from server to client, for
	// non-textual filetypes updates are performed by FileIOBinary::Write()
	// note:  do this after the open (because open does a write on NT).

	if( !digestType && digest && p4tunable.Get( P4TUNE_LBR_VERIFY_OUT ) 
	           && !f->file->IsSymlink() )
	{
	    f->serverDigest.Set( digest );
	    f->checksum = new MD5;
	    if( ( !f->file->IsTextual() || (f->file->GetType() & FST_C_MASK) ) &&
	        !( f->file->GetType() & FST_M_APPLE ) &&
	        !( f->file->GetType() == FST_RESOURCE ) )
	        f->file->SetDigest( f->checksum );
	}

	// Character set translations
	f->file->Translator( ClientSvc::XCharset( client, FromServer ) );

	// If anything went wrong, we mark the handle so that
	// the rest of the write protocol is honored (but ignored).

    bail:
	// Mark handle with any error
	// Report non-fatal error and clear it.

	f->SetError( e );
	client->OutputError( e );
}

void
clientWriteFile( Client *client, Error *e )
{
	StrPtr *clientHandle = client->GetVar( P4Tag::v_handle, e );
	StrPtr *data = client->GetVar( P4Tag::v_data, e );

	if( data )
	    client->recvClientBytes += data->Length();

	if( client_nullsync )
	    return;

	if( e->Test() )
	    return;

	// Get handle.

	ClientFile *f = (ClientFile *)client->handles.Get( clientHandle, e );

	if( e->Test() || f->IsError() )
	    return;

	if( f->serverDigest.Length() && 
	  ( ( f->file->IsTextual() && !( f->file->GetType() & FST_C_MASK ) ) ||
	    ( f->file->GetType() & FST_M_APPLE ) ||
	    ( f->file->GetType() == FST_RESOURCE ) ) )
	    f->checksum->Update( *data );


	// Write data.
	// On failure, mark handle with error.

# ifdef USE_EBCDIC
	// Translate back!
	if( !f->file->IsTextual() )
	    __etoa_l( data->Text(), data->Length() );
# endif

	f->file->Write( data, e );

	if( !e->Test() && f->file->IsSymlink() && data->Length() )
	    f->symTarget << data;

	if( f->progress )
	    f->progress->Increment( data->Length() / 1024,
	                            e->Test() ? CPP_FAILDONE : CPP_NORMAL );
	
	// Mark handle with any error
	// Report non-fatal error and clear it.

	f->SetError( e );
	client->OutputError( e );
}

# ifdef USE_CDC

class StrIntTree : public VVarTree
{
    public:
	class StrInt
	{
	    public:
	        StrInt( const char *k, int i )
	        {
	            key = new StrBuf( k );
	            index = i;
	        }

	        StrInt( const StrInt& other )
	        {
	            key = new StrBuf( *other.key );
	            index = other.index;
	        }

	        ~StrInt()
	        {
	            delete key;
	        }

	        StrBuf *key;

	        // Because of the VVarTree::Copy() method we cannot
	        // put FileSys* as the payload or the temp file would
	        // be deleted when it's moved. We use an index into
	        // an array of FileSys* instead.

	        int index;
	} ;

	StrIntTree() {}
	virtual ~StrIntTree()
	{
	    Clear();
	}

	virtual int Compare( const void *a, const void *b ) const
	{
	    const StrInt &t1 = *(const StrInt *)a;
	    const StrInt &t2 = *(const StrInt *)b;
	    return t1.key->Compare( *t2.key );
	}

	virtual void * Copy( const void *src ) const
	{
	    return new StrInt( *(StrInt*)src );
	}

	virtual void Delete( void *a ) const
	{
	    delete (StrInt *)a;
	}

	virtual void Dump( void *a, StrBuf &buf ) const
	{
	}
} ;

class ClientTempFiles : public LastChance
{
    private:
	// Creating instances of this class is not allowed.
	// Use the static method to get a handle of this class.

	ClientTempFiles() {}

    public:
	static ClientTempFiles *
	GetHandle( Client *client, Error* e, bool create )
	{
	    const StrRef tempFileHandleName = StrRef( "clientTempFiles" );
	    ClientTempFiles *clientTempFiles = ( ClientTempFiles * )
	                client->handles.Get( &tempFileHandleName, e );
	    if( !clientTempFiles && create )
	    {
	        Error err;

	        if( e )
	            e->Clear();
	        else
	            e = &err;

	        clientTempFiles = new ClientTempFiles;
	        client->handles.Install( &tempFileHandleName, clientTempFiles, e );
	        if( e->Test() )
	        {
	            delete clientTempFiles;
	            clientTempFiles = 0;
	        }
	    }
	    return clientTempFiles;
	}

	virtual ~ClientTempFiles()
	{
	    int n = tempFiles.Count();
	    for( int i = 0; i < n; i++ )
	    {
	        delete ( FileSys* )tempFiles.Get( i );
	    }
	}

	FileSys *
	Put( const char *key, Error *e )
	{
	    int n = tempFiles.Count();
	    StrIntTree::StrInt s( key, n );
	    FileSys * ftemp = FileSys::CreateGlobalTemp( FST_BINARY );
	    if( ftemp )
	    {
	        indices.Put( &s, e );
	        if( e->Test() )
	        {
	            delete ftemp;
	            ftemp = 0;
	        }
	        else
	            tempFiles.Put( ftemp );
	    }
	    return ftemp;
	}

	FileSys *
	GetRemove( const char *key )
	{
	    StrIntTree::StrInt s( key, 0 );
	    StrIntTree::StrInt *r = ( StrIntTree::StrInt* )indices.Get( &s );
	    if( r )
	        // Replace the temp file with NULL assuming
	        // the caller will delete it after use
	        return ( FileSys* )tempFiles.Replace( r->index, 0 );

	    return 0;
	}

    private:
	VarArray tempFiles;
	StrIntTree indices;
} ;

class CDCStats : public LastChance
{
    private:
	int elapse_time;
	
	int fileCount;
	P4INT64 fileSize;

	int chunkMapCount;
	P4INT64 chunkMapSize;

	int chunkCount;
	P4INT64 chunkSize;

	CDCStats()
	{
	    elapse_time = 0;
	    fileCount = 0;
	    fileSize = 0;
	    chunkMapCount = 0;
	    chunkMapSize = 0;
	    chunkCount = 0;
	    chunkSize = 0;
	}

    public:
	static CDCStats *
	GetHandle( Client *client, Error* e )
	{
	    const StrRef cdcStatsHandleName = StrRef( "cdcStats" );
	    CDCStats *cdcStats = ( CDCStats * )
	                client->handles.Get( &cdcStatsHandleName, e );
	    if( !cdcStats )
	    {
	        e->Clear();
	        cdcStats = new CDCStats;
	        client->handles.Install( &cdcStatsHandleName, cdcStats, e );
	        if( e->Test() )
	        {
	            delete cdcStats;
	            cdcStats = 0;
	        }
	    }
	    return cdcStats;
	}

	class CDCTimer : public Timer
	{
	    private:
	        CDCStats *cdcStats;

	    public:
	        CDCTimer( CDCStats* stats ) :
	            cdcStats ( stats )
	        {
	            Start();
	        }

	        ~CDCTimer()
	        {
	            if( cdcStats )
	                cdcStats->LogTime( Time() );
	        }
	} ;

	~CDCStats()
	{
	    if( p4debug.GetLevel( DT_DLTXFER ) >= DL_INFO )
	    {
	        p4debug.printf( "Delta Transfer stats:\n" );
	        p4debug.printf( "files/chunkmaps/chunks count+bytes:\n");
	        p4debug.printf( "%d+%d %d+%d %d+%d\n",
	                        fileCount, fileSize,
	                        chunkMapCount, chunkMapSize,
	                        chunkCount, chunkSize );
	        P4INT64 delta = fileSize - ( chunkMapSize + chunkSize );
	        p4debug.printf( "%d %s bytes transferred\n",
	                        delta > 0 ? delta : -delta,
	                        delta > 0 ? "fewer" : "more" );

	        StrBuf msg = "Processing time: ";
	        msg << StrMs( elapse_time ) << "s\n";
	        p4debug.printf( msg.Text() );
	    }
	}

	void LogTime( int time )
	{
	    elapse_time += time;
	}

	void LogFile( P4INT64 size )
	{
	    fileCount += 1;
	    fileSize += size;
	}

	void LogChunkMap( P4INT64 size )
	{
	    chunkMapCount += 1;
	    chunkMapSize += size;
	}

	void LogChunk( P4INT64 size )
	{
	    chunkCount += 1;
	    chunkSize += size;
	}
} ;

static bool
clientChunkMapInternal( Client* client, StrPtr *clientPath,
	StrPtr *index, StrPtr *confirm, ChunkMap &cm,
	StrBuf& status, Error *e )
{
	// We don't have the file size for sync but still need to
	// honor the signal to diable CDC locally

	const int minSize = p4tunable.Get( P4TUNE_NET_DELTA_TRANSFER_MINSIZE );
	const P4INT64 cdcThreshold = p4tunable.Get( P4TUNE_NET_DELTA_TRANSFER_THRESHOLD );

	if( !minSize || !cdcThreshold )
	{
	    status = "Delta Transfer disabled";
	    return false;
	}

	if( !FileSys::FileExists( clientPath->Text() ) )
	{
	    status = "missing";
	    return false;
	}

	ClientTempFiles* tempFiles = ClientTempFiles::GetHandle( client, e, true );
	if( e->Test() || !tempFiles )
	{
	    delete tempFiles;
	    return false;
	}

	StrBuf verify_hash;
	cm.GetVerifyHash( verify_hash );
	if( !verify_hash.Length() )
	{
	    status = "Empty chunk map veryfy hash";
	    return false;
	}
	
	FileSys* ftemp = tempFiles->Put( clientPath->Text(), e );
	if( e->Test() || !ftemp )
	    return false;

	cm.Write( ftemp->Path(), e );
	if( e->Test() )
	    return false;

	// local chunkmap

	ChunkMap cmHave;
	cmHave.Create( clientPath, e );
	if( e->Test() )
	    return false;

	VVarArray *cmDiff = cm.Diff( cmHave, e );
	if( e->Test() || !cmDiff )
	    return false;

	P4INT64 nChunksToSend = cmDiff->Count();
	if( nChunksToSend )
	{
	    if( !cm.BelowThreshold( nChunksToSend, &status ) )
	    {
	        delete cmDiff;
	        return false;
	    }

	    const ChunkMap::Chunk *c =
	            (const ChunkMap::Chunk *)cmDiff->Get( 0 );
	    const int batch = FileSys::BufferSize() / c->hash.Length();

	    MD5 digest;
	    for( int i = 0; i < nChunksToSend; )
	    {
	        StrBuf *m = client->MakeVar( P4Tag::v_data );
	        
	        int b = 0;
	        for( ; b < batch && i < nChunksToSend; b++, i++ )
	        {
	            c = (const ChunkMap::Chunk *)cmDiff->Get( i );
	            m->Append( c->hash.Text() );
	            digest.Update( c->hash );
	        }

	        if( !b )
	            break; // should not happen

	        if( i == nChunksToSend )
	        {
	            // send digest to signal completion of transfer

	            StrBuf dbuf;
	            digest.Final( dbuf );
	            client->SetVar( P4Tag::v_digest, dbuf );
	        }

	        client->SetVar( P4Tag::v_total, nChunksToSend );
	        client->SetVar( P4Tag::v_count, b );
	        client->SetVar( P4Tag::v_status, "exists" );
	        client->SetVar( P4Tag::v_index, index );
	        client->Confirm( confirm );
	    }
	}
	else
	{
	    client->SetVar(P4Tag::v_status, "same" );
	    client->SetVar( P4Tag::v_index, index );
	    client->Confirm( confirm );
	}
	delete cmDiff;

	return true;
}

static ChunkMap *
clientReceiveChunkMap( Client *client, Error *e )
{
	// We need either chunkMapHandle or chunkMap

	StrPtr *smHandle = client->GetVar( P4Tag::v_chunkMapHandle );
	StrPtr *smBuf = client->GetVar( P4Tag::v_chunkMap );
	if( !smHandle && !smBuf )
	    client->GetVar( P4Tag::v_chunkMap, e ); // Trigger error on neither

	if( e->Test() )
	    return 0;

	ClientVarHandle *smVar = 0;
	if( smHandle )
	{
	    smVar = (ClientVarHandle *) client->handles.Get( smHandle, e );

	    if( e->Test() )
	        return 0;
	}
	
	// Use either chunkMapHandle or chunkMap
	// If we got both chunkMapHandle and chunkMap, use chunkMap

	ChunkMap *cm = new ChunkMap( smBuf ? smBuf : &smVar->memStore, e );

	if( e->Test() )
	{
	    delete cm;
	    cm = 0;
	}

	// Free-up this memory now in case it's big, so we don't hold
	// it for the file transfer.  Note that the 'sm' var is now invalid.

	delete smVar;

	return cm;
}

static void
clientChunkMap( Client *client, Error *e )
{
	// On a non-Unicode server, client->transfname is the client itself.
	// Calling client-transfname->GetVar() will retrieve the path name
	// un-translated and un-cached. However, on a Unicode server,
	// transfname is TransDict which translates the path and caches the
	// translated name using the var name (P4Tag::v_xxx) as key. If a
	// method is called to process multiple files on a Unicode server
	// without clearing the cache, all subsequent calls will get the first
	// cached path.
	// 
	// Because this is the first client method for processing a file for
	// delta transfer, we need to call client->NewHandler() to reset the
	// translation cache.

	client->NewHandler();
	StrPtr *clientPath = client->transfname->GetVar( P4Tag::v_path, e );
	StrPtr *index = client->GetVar( P4Tag::v_index, e );
	StrPtr *confirm = client->GetVar( P4Tag::v_confirm );

	if( e->Test() )
	    return;

	if( p4debug.GetLevel( DT_DLTXFER ) == DL_DEBUG )
	    p4debug.printf( "clientChunkMap: %s\n", clientPath->Text() );

	CDCStats *cdcStats = 0;
	if( p4debug.GetLevel( DT_DLTXFER ) > DL_NONE )
	{
	    cdcStats = CDCStats::GetHandle( client, e );
	    if( e->Test() || !cdcStats )
	        return;
	}
	CDCStats::CDCTimer cdcTimer( cdcStats );

	ChunkMap *cm = clientReceiveChunkMap( client, e );
	if( e->Test() || !cm )
	    return;

	if( cdcStats )
	    cdcStats->LogChunkMap( cm->GetBuf()->Length() );

	StrBuf status;
	if( !clientChunkMapInternal( client,
	    clientPath, index, confirm, *cm, status, e ) )
	{
	    // Send back status or error so server can
	    // fall back to sending whole file

	    if( !status.Length() )
	    {
	        if( e->Test() )
	            e->Fmt( &status );
	        else
	            status = "Client error";
	    }
	    client->SetVar( P4Tag::v_index, index );
	    client->SetVar(P4Tag::v_status, status );
	    client->Confirm( confirm );
	}
	delete cm;
}

static void
clientWriteFileChunks( Client *client, Error *e )
{
	StrIntsTree::StrInts *co = 0;

	StrPtr *clientHandle = client->GetVar( P4Tag::v_handle, e );
	StrPtr *clientPath = client->transfname->GetVar( P4Tag::v_path, e );
	ClientFile *f = (ClientFile *)client->handles.Get( clientHandle, e );

	if( e->Test() || !f || f->IsError() )
	{
	    f->SetError( e );
	    client->OutputError( e );
	    return;
	}

	CDCStats *cdcStats = 0;
	if( p4debug.GetLevel( DT_DLTXFER ) > DL_NONE )
	{
	    cdcStats = CDCStats::GetHandle( client, e );
	    if( e->Test() || !cdcStats )
	    {
	        f->SetError( e );
	        client->OutputError( e );
	        return;
	    }
	}
	CDCStats::CDCTimer cdcTimer( cdcStats );

	StrPtr *hash = client->GetVar( P4Tag::v_hash );
	StrPtr *data = client->GetVar( P4Tag::v_data );
	if( !hash || !data )
	{
	    // local file copy

	    if( f->indirectFile )
	    {
	        FileSysUPtr in = FileSys::CreateUPtr( FST_BINARY );
	        in->Set( *f->indirectFile->Path() );
	        in->Open( FOM_READ, e );

	        P4INT64 fileSize = 0;

	        StrFixed buf( FileSys::BufferSize() );
	        while( !e->Test() )
	        {
	            int l = in->Read( buf.Text(), buf.Length(), e );
	            if( !l || e->Test() )
	                break;

	            f->file->Write( buf.Text(), l, e );
	            fileSize += l;
	        }
	        in->Close( e );

	        if( cdcStats )
	            cdcStats->LogFile( fileSize );
	    }
	    f->SetError( e );
	    client->OutputError( e );
	    return;
	}

	if( cdcStats )
	    cdcStats->LogChunk( data->Length() );

	client->recvClientBytes += data->Length();

	if( client_nullsync || !f->indirectFile )
	    goto end;

	if( !f->chunkOffsetTree )
	{
	    // First call: fill with chunks found in local file

	    ClientTempFiles* tempFiles = ClientTempFiles::GetHandle( client, 0, false );
	    if( !tempFiles )
	        goto end;

	    ChunkMap cm;
	    FileSys* ftemp = tempFiles->GetRemove( clientPath->Text() );
	    if( ftemp )
	    {
	        cm.Read( ftemp->Path(), e );
	        // Remove the temp file after use to avoid
	        // having too many files during operation
	        delete ftemp;

	        if( e->Test() )
	            goto end;
	    }
	    else
	        goto end;

	    if( cdcStats )
	        cdcStats->LogFile( cm.GetFileSize() );

	    f->chunkOffsetTree = new StrIntsTree;

	    // create the local chunkmap again

	    ChunkMap cmHave;
	    cmHave.Create( f->indirectFile->Path(), e );
	    if( e->Test() )
	        goto end;

	    FileSysUPtr in = FileSys::CreateUPtr( FST_BINARY );
	    in->Set( *f->indirectFile->Path() );
	    in->Open( FOM_READ, e );

	    ChunkMap::Chunk c;
	    VVarTree *haveTree = cmHave.AsVTree( e );
	    while( haveTree && !e->Test() &&
	           cm.GetNextChunk( c ) )
	    {
	        ChunkMap::Chunk *cc = 0;
	        cc = (ChunkMap::Chunk*)haveTree->Get( &c );
	        if( cc )
	        {
	            // chunk is found in local chunk map

	            in->CopyRange( cc->offset, cc->size,
	                           f->file, c.offset, e );
	        }
	        else
	        {
	            // save and fill later with chunks from server

	            co = f->chunkOffsetTree->Get( c.hash.Text() );
	            if( !co )
	            {
	                co = f->chunkOffsetTree->Put( c.hash.Text(), e );
	                if( !co || e->Test() )
	                    // break here even though e is tested in the loop
	                    break;
	            }
	            co->Put( c.offset );
	        }
	    }
	    in->Close( e );
	    delete haveTree;
	}

	// assemble the chunks here

	co = f->chunkOffsetTree->Get( hash->Text() );
	if( co )
	{
	    for( int i = 0; i < co->count && !e->Test(); i++ )
	    {
	        f->file->Seek( (P4INT64)co->ints[ i ], e );
	        f->file->Write( data, e );
	    }
	}

	// Mark handle with any error
	// Report non-fatal error and clear it.
end:
	f->SetError( e );
	client->OutputError( e );
}

# else

// stubs

static void
clientChunkMap( Client *, Error * )
{
}

static void
clientWriteFileChunks( Client *, Error * )
{
}

# endif

void
clientCloseFile( Client *client, Error *e )
{
	if( client_nullsync )
	    return;

	StrPtr *clientHandle = client->GetVar( P4Tag::v_handle, e );
	StrPtr *func = client->GetVar( P4Tag::v_func, e );
	StrPtr *commit = client->GetVar( P4Tag::v_commit );

	if( e->Test() )
	    return;

	// Get handle.

	ClientFile *f = (ClientFile *)client->handles.Get( clientHandle, e );

	if( e->Test() )
	    return;

	// Check for illegal symlinks
	//
	// Block symlinks outside the workspace if filesys.restictsymlinks=1
	// and P4CLIENTROOT or DVCS are in use.
	// Block symlinks to P4SYSTEMIGNORE even when filesys.restictsymlinks=0.

	if( f->file && ( f->file->GetType() & FST_SYMLINK ) )
	{
	    PathSys *path = PathSys::Create();
	    FileSys *f2 = FileSys::Create( FST_BINARY );
	    StrBuf basePath;
	    char *p;
	    if( ( p = strchr( f->symTarget.Text(), '\n' ) ) )
	    {
	        f->symTarget.SetEnd( p );
	        f->symTarget.Terminate();
	    }
	    path->SetLocal( client->GetCwd(), StrRef( f->file->Name() ) );
	    path->ToParent();
	    basePath = *path;
	    path->SetLocal( basePath, f->symTarget );
	    f2->Set( *path );
	    ClientSvc::CheckFilePath( client, f2, e,
	                ( !p4tunable.Get( P4TUNE_FILESYS_RESTRICTSYMLINKS ) ||
	                  !client->GetClientPath().Length() ) );
	    delete f2;
	    delete path;
	}

	if( !e->Test() && !f->IsError() && f->file )
	{
	    const offL_t fTell = f->file->Tell();
	    const offL_t fSH = f->file->GetSizeHint();

	    if( fSH && fTell && fSH > fTell )
	        f->file->Truncate( fTell, e );
	}

	// Close file, and then diff/rename as appropriate.

	if( f->file )
	    f->file->Close( e );

	// Stat file and record syncTime

	if( f->file )
	{
	    int modTime = f->file->GetModTime();
	    if( modTime ) client->SetSyncTime( modTime );
	             else client->SetSyncTime( f->file->StatModTime() );
	}

	if( !e->Test() && !f->IsError() && f->serverDigest.Length() && commit )
	{
	    StrBuf clientDigest;
	    f->checksum->Final( clientDigest );

	    if( f->serverDigest != clientDigest )
		e->Set( MsgClient::DigestMisMatch ) << f->file->Name()
		                                    << clientDigest
		                                    << f->serverDigest;
	}

	if( e->Test() || f->IsError() )
	{
	    // nothing
	}
	else if( f->isDiff )
	{
	    if( *func == P4Tag::c_CloseMatch )
	    {
		// Pass off control to clientFindMatch.
		// Don't delete handle yet, clientAckMatch needs it.

		clientCloseMatch( client, f, e );
		return;
	    }
	    FileSys *f2 = client->GetUi()->File( f->file->GetType() );
	    f2->SetContentCharSetPriv(
		f->file->GetContentCharSetPriv() );
	    f2->Set( f->diffName );
	    client->GetUi()->Diff( f->file, f2, 0, f->diffFlags.Text(), e );
	    delete f2;
	}
	else if( !commit )
	{
	    // nothing
	}
	else if( f->indirectFile )
	{
	    // rename to actual target

	    f->file->Rename( f->indirectFile, e );

	    // If rename worked, no longer need to delete temp.

	    if( !e->Test() )
		f->file->ClearDeleteOnClose();
	}
	else
	{
	    // just close actual target
	    f->file->ClearDeleteOnClose();
	}

	// Progress complete
	if( f->progress )
	    f->progress->Increment( 0, e->Test() || f->IsError() ? CPP_FAILDONE
	                                                         : CPP_DONE );

	// Handle remembers if any error occurred 
	// Report non-fatal error and clear it.

	f->SetError( e );
	client->OutputError( e );

	delete f;
}

void
clientMoveFile( Client *client, Error *e )
{
	// Move file, clientPath is old,  targetPath is new

	client->NewHandler();
	StrPtr *clientPath = client->transfname->GetVar( P4Tag::v_path, e );
	StrPtr *targetPath = client->transfname->GetVar( P4Tag::v_path2, e );
	StrPtr *confirm = client->GetVar( P4Tag::v_confirm, e );
	StrPtr *rmdir = client->GetVar( P4Tag::v_rmdir );
	StrPtr *doForce = client->GetVar( P4Tag::v_force );
	StrPtr *perm = client->GetVar( P4Tag::v_perm );

	if( e->Test() )
	    return;

	if( rmdir && *rmdir == P4Tag::v_false )
	    rmdir = 0;

	FileSys *s = ClientSvc::File( client, e );

	if( e->Test() || !s )
	    return;

	// stat the source file

	if( !( s->Stat() & ( FSF_SYMLINK|FSF_EXISTS ) ) )
	{
	    e->Set( MsgClient::NoSuchFile ) << clientPath;
	    client->OutputError( e );
	    delete s;
	    return;
	}

	// set the source Perms, so they are reflected in the target
	// 2011.1+ server may request that existing RO perms be preserved.
	
	if( !perm || ( s->Stat() & FSF_WRITEABLE ) )
	    s->Perms( FPM_RW );

	// check target file does not exist

	FileSys *t = ClientSvc::FileFromPath( client, P4Tag::v_path2, e );
	if( e->Test() || !t )
	    return;

	int targetIsSubdir = 0;
	if( ( t->Stat() & ( FSF_DIRECTORY ) ) &&
	    ( clientPath->Contains( *targetPath ) ||
	      targetPath->Contains( *clientPath ) ) )
	{
	    // Either clientPath = A and targetPath = A/A
	    // or clientPath = A/A and targetPath = A

	    //targetPath is a directory subpath of clientPath
	    StrArray *ua = t->ScanDir( e );

	    int uaCount = 0;

	    if( ua )
	        uaCount = ua->Count();

	    delete ua;

	    if( uaCount == 1)
	        //client file is the only object in target dir
	        targetIsSubdir = 1;
	    else if( uaCount > 1 )
	        e->Set( MsgClient::DirectoryNotEmpty ) << targetPath;

	}

	// Target file exists,  could be a case change,  allow this only if
	// the server is case sensitive.

	if( !e->Test() && ( t->Stat() & ( FSF_SYMLINK|FSF_EXISTS ) ) &&
	    ( client->protocolNocase || clientPath->Compare( *targetPath ) ) &&
	    !doForce && !targetIsSubdir )
	    e->Set( MsgClient::FileExists ) << targetPath;

	if( !e->Test() )
	    t->MkDir( e );

	if( !e->Test() )
	    s->Rename( t, e );

	// ok, now zonk the source directory, ignoring errors

	if( !e->Test() && rmdir )
	    s->RmDir();

	delete s;
	delete t;

	if( e->Test() )
	    client->OutputError( e );
	else
	    client->Confirm( confirm );
}

void
clientDeleteFile( Client *client, Error *e )
{
	client->NewHandler();
	StrPtr *clientPath = client->GetVar( P4Tag::v_path, e );
	StrPtr *noclobber = client->GetVar( P4Tag::v_noclobber );
	StrPtr *clientHandle = client->GetVar( P4Tag::v_handle );
	StrPtr *rmdir = client->GetVar( P4Tag::v_rmdir );
	StrPtr *revertmovermdir = client->GetVar( P4Tag::v_revertmovermdir );
	StrPtr *digest = client->GetVar( P4Tag::v_digest );
	StrPtr *digestType = client->GetVar( P4Tag::v_digestType );
	StrPtr *confirm = client->GetVar( P4Tag::v_confirm );
	StrPtr *altsync = client->GetVar( P4Tag::v_altSync );

	if( noclobber && *noclobber == P4Tag::v_false )
	    noclobber = 0;

	if( rmdir && *rmdir == P4Tag::v_false )
	    rmdir = 0;

	ClientProgress *indicator;
	ProgressReport *progress = 0;

	if( ( indicator = client->GetUi()->CreateProgress( CPT_DELFILE, 0 ) ) )
	{
	    progress = new ClientProgressReport( indicator );
	    progress->Description( *clientPath );
	    progress->Units( PRU_FILES );
	    progress->Total( 1 );
	}

	FileSys *f = 0;
	int stat = 0;

	// clear syncTime

	client->SetSyncTime( 0 );

	if( e->Test() && !e->IsFatal() )
	{
	    client->OutputError( e );
	    goto end;
	}

	f = ClientSvc::File( client, e );

	if( e->Test() || !f )
	{
	    client->OutputError( e );
	    goto end;
	}

	// stat the file

	stat = f->Stat();

	// Don't try to unlink a directory. It won't work, and it will be
	// confusing. Worse, it might mess up directory permissions.
	//
	if( (stat & (FSF_EXISTS | FSF_DIRECTORY | FSF_SYMLINK)) ==
	            (FSF_EXISTS | FSF_DIRECTORY) )
	{
	    delete f;
	    goto end;
	}

	// Don't delete modified files noclobber allwrite (digestType set)
	if( digestType )
	{
	    StrBuf localDigest;
	    FileDigestType digType;

	    digType = clientFileDigestType( digestType );

	    f->ComputeDigest( digType, &localDigest, e );
	    if( e->Test() || localDigest.XCompare( *digest ) )
	    {
		LastChance l;
		client->handles.Install( clientHandle, &l, e );
		l.SetError();

		e->Set( MsgClient::NoModifiedFile ) 
		    << "delete"
		    << f->Name();
		client->OutputError( e );
		delete f;
		goto end;
	    }
	}

	// Don't clobber poor file
	// noclobber, handle new to 99.1
	// be safe about clientHandle being set

	if( noclobber && clientHandle &&
		( stat & ( FSF_WRITEABLE | FSF_SYMLINK ) ) == FSF_WRITEABLE )
	{
	    LastChance l;
	    client->handles.Install( clientHandle, &l, e );
	    l.SetError();

	    e->Set( MsgClient::ClobberFile ) << f->Name();
	    client->OutputError( e );
	    delete f;
	    goto end;
	}

	// In the case of reverting a file that was moved to a directory of the
	// the same name, check first that the directory only contains the file.
	// If not, return an error that the directory is not empty. A more
	// descriptive error will be given in clientCheckFile.
	if( revertmovermdir )
	{
	    int uaCount = clientDirectoryEntryCount( revertmovermdir, e );
	    if( e->Test() )
	    {
	        client->OutputError( e );
	        delete f;
	        goto end;
	    }

	    if( uaCount > 1 )
	    {
	        if( clientHandle )
	        {
	            LastChance l;
	            client->handles.Install( clientHandle, &l, e );
	            l.SetError();
	        }

	        e->Set( MsgClient::DirectoryNotEmpty ) << revertmovermdir;
	        client->OutputError( e );
	        delete f;
	        goto end;
	    }

	}

	f->Unlink( e );

	// If unlink returned an error and the file existed before
	// the unlink attempt, then it failed.  Except it could be
	// an applefile, in that case one of the forks could have
	// been previously removed, if its applefile, stat it again.

	if( e->Test() && clientHandle && ( f->GetType() & FST_M_APPLE ) )
	    stat = f->Stat();

	if( e->Test() && clientHandle && ( stat & FSF_EXISTS ) )
	{
	    LastChance l;
	    client->handles.Install( clientHandle, &l, e );
	    l.SetError();

	    client->OutputError( e );

	    // Some platforms change the file mode to writeable so that
	    // unlink can work, make sure thats changed back.

	    if( !( stat & FSF_WRITEABLE ) )
	        f->Chmod( FPM_RO, e );

	    delete f;
	    goto end;
	}

	// Clear unlink error of non-existent file.

	e->Clear();

	// ok, now zonk the directory, ignoring errors

	if( rmdir || revertmovermdir )
	{
	    if( rmdir && *rmdir == "preserveCWD" )
	        f->PreserveCWD();

	    if( altsync )
	    {
	        ClientAltSyncHandler *handler =
	            ClientAltSyncHandler::GetAltSyncHandler( client, e );
	
	        if( !e->Test() )
	            f->PreserveRoot( handler->GetClientRoot() );
	    }

	    f->RmDir();
	}

	delete f;

end:
	if( progress )
	    progress->Position( e->Test() ? 0 : 1,
	                        e->Test() ? CPP_FAILDONE : CPP_DONE );

	// Ack fallthough

	if( confirm )
	    clientAck( client, e );

	delete progress;
}

void
clientChmodFile( Client *client, Error *e )
{
	client->NewHandler();
	StrPtr *perms = client->GetVar( P4Tag::v_perms, e );
	StrPtr *modTime = client->GetVar( P4Tag::v_time );
	StrPtr *confirm = client->GetVar( P4Tag::v_confirm );

	FileSys *f = 0;

	if( e->Test() && !e->IsFatal() )
	{
	    client->OutputError( e );
	    goto end;
	}

	f = ClientSvc::File( client, e );

	if( e->Test() || !f )
	{
	    client->OutputError( e );
	    goto end;
	}

	// Set mode and time.  Mode is sensitive to exec bit.
	// Don't try and set the time if the file is not writable.
	// If its not writeable then its time was not changed anyhow,
	// this also avoids an access error.
	//

	if( modTime && ( f->Stat() & FSF_WRITEABLE ) )
	{
	    f->ModTime( modTime );
	    f->ChmodTime( e );
	}

	if( !e->Test() )
	    f->Chmod2( perms->Text(), e );

	delete f;

	// Report non-fatal error and clear it.

	if( e->Test() )
	    client->OutputError( e );

end:
	// Ack fallthough

	if( confirm )
	    clientAck( client, e );
}

void
clientConvertFile( Client *client, Error *e )
{
	StrPtr *clientPath = client->transfname->GetVar( P4Tag::v_path, e );
	StrPtr *perms      = client->GetVar( P4Tag::v_perms, e );
	StrPtr *fromCS     = client->GetVar( StrRef( P4Tag::v_charset ), 1 );
	StrPtr *toCS       = client->GetVar( StrRef( P4Tag::v_charset ), 2 );

	if( !fromCS || !toCS )
	    e->Set( MsgSupp::NoParm ) << P4Tag::v_charset;

	if( e->Test() )
	    return;

	int size = FileSys::BufferSize();
	StrBuf bu, bt;
	char *b = bu.Alloc( size );
	int l, statFlags;

	FileSys *f = 0;
	FileSys *t = 0;

	CharSetCvt::CharSet cs1 = CharSetCvt::Lookup( fromCS->Text() );
	CharSetCvt::CharSet cs2 = CharSetCvt::Lookup( toCS->Text() );
	if( cs2 == CharSetApi::CSLOOKUP_ERROR ||
	    cs1 == CharSetApi::CSLOOKUP_ERROR )
	    goto convertFileFinish;

	f = ClientSvc::File( client, e );
	f->SetContentCharSetPriv( cs1 );
	if( e->Test() )
	    goto convertFileFinish;

	statFlags = f->Stat();
	if( !( statFlags & FSF_EXISTS  ) ||
	     ( statFlags & FSF_SYMLINK )  )
	{
	    e->Set( MsgClient::FileOpenError );
	    goto convertFileFinish;
	}

	t = client->GetUi()->File( f->GetType() );
	t->MakeLocalTemp( f->Name() );
	t->SetContentCharSetPriv( cs2 );

	f->Open( FOM_READ, e );
	f->Translator( CharSetCvt::FindCachedCvt( client->gCharSetCvtCache, cs1,
	                                          CharSetCvt::UTF_8 ) );

	t->Open( FOM_WRITE, e );
	t->Translator( CharSetCvt::FindCachedCvt( client->gCharSetCvtCache,
	                                          CharSetCvt::UTF_8, cs2 ) );

	if( e->Test() )
	    goto convertFileFinish;

	while( ( l = f->Read( b, size, e ) ) && !e->GetErrorCount() )
	    t->Write( b, l, e );

	// Translation errors are info, and FileSys::Close clears
	// info messages, so we need to trap those here since we
	// don't want to go ahead with a mangled file.

	if( e->GetErrorCount() )
	{
	    e->Set( MsgSupp::ConvertFailed )
		    << clientPath
		    << fromCS
		    << toCS;
	    client->OutputError( e );
	    f->Close( e );
	    t->Close( e );
	    t->Unlink( e );
	    delete f;
	    delete t;
	    return;
	}

	f->Close( e );
	t->Close( e );

	if( e->Test() )
	{
	    t->Unlink( e );
	    goto convertFileFinish;
	}

	t->Rename( f, e );
	f->Chmod( perms->Text(), e );

convertFileFinish:
	if( e->GetErrorCount() )
	{
	    e->Set( MsgSupp::ConvertFailed )
		    << clientPath
		    << fromCS
		    << toCS;
	    client->OutputError( e );
	}
	delete f;
	delete t;
}

/*
 * clientCheckFile() -- "inquire" about file, returning info to server
 *
 * This routine, for compatibility purposes, has several modes.
 *
 * 1.	If clientType is set, we know the type and we're checking to see
 *	if the file exists and (if digest is set) if the file has the same
 *	fingerprint.  We return this in "status" with a value of "missing",
 *	"exists", or "same".  This starts around version 1742.
 *
 * 2.	If clientType is unset, we're looking for the type of the file,
 *	and we'll return it in "type".  This is sort of overloaded, 'cause
 *	it can also get set with pseudo-types like "missing".  In this
 *	case, we use the "xfiles" protocol check to make sure we don't
 *	return something the server doesn't expect.
 *
 *	- files unset: return text, binary.
 * 	- files >= 0: also return xtext, xbinary.
 *	- files >= 1: also return symlink.
 *	- files >= 2; also return resource (mac resource file).  
 *	- files >= 3; also return ubinary
 *	- files >= 4; also return apple
 *
 *	If forceType is set, we'll use that in preference over what
 *	we've discovered.  We still check the file (to make sure they're
 *	not adding a directory, and so they get to right warning if
 *	they add an empty file), but we'll just override that back to
 *	the (typemap's) forceType.
 *
 * We map empty/missing/unreadable into forceType/"text".
 */

enum ctAction { 
	OK, 	// use forceType/the discovered type
	ASS, 	// missing/unreadable/empty: assume it is forceType/text
	SUBST,	// server can't handle it: substitute altType
	CHKSZ,  // size is to big for default, use alternate
	CANT 	// just can't be added
};

/*
 * Handle the varieties of the protocol.
 * Don't ever "missing", "empty", or "unreadable" 
 * Don't send "resource" unless xfiles >= 2
 * Don't send "special", "symlink", or "directory" unless xfiles >= 1
 * Don't send "xtext" or "xbinary" unless xfiles >= 0
 */

const struct ctTable {

	FileSysType 	checkType;
	int		xlevel;
	ctAction	action[2];
	const char	*type;
	const char	*altType;
	const char	*cmpType;

} checkTable[] = {
	
	{ FST_TEXT,	0, { OK, CHKSZ },   	"text", "text", "ctext" },
	{ FST_XTEXT,	0, { SUBST, CHKSZ },	"xtext", "text", "text+Cx" },
	{ FST_BINARY,	0, { OK, OK },		"binary", "binary", 0 },
	{ FST_XBINARY,	0, { SUBST, OK },	"xbinary", "binary", 0 },
	{ FST_APPLEFILE,4, { SUBST, OK },	"apple", "binary", 0 },
	{ FST_XAPPLEFILE,4, { SUBST, OK },	"apple+x", "binary", 0 },
	{ FST_CBINARY,	3, { SUBST, OK },	"ubinary", "binary", 0 },
	{ FST_SYMLINK,	1, { CANT, OK },	"symlink", 0, 0 },
	{ FST_RESOURCE,	2, { CANT, OK },	"resource", 0, 0 },
	{ FST_SPECIAL,	-1, { CANT, CANT },	"special", 0, 0 },
	{ FST_DIRECTORY,-1, { CANT, CANT },	"directory", 0, 0 },
	{ FST_MISSING,	-1, { ASS, ASS },	"missing", "text", 0 },
	{ FST_CANTTELL,	-1, { ASS, ASS },	"unreadable", "text", 0 },
	{ FST_EMPTY,	-1, { ASS, ASS },	"empty", "text", 0 },
	{ FST_UNICODE,	5, { SUBST, CHKSZ },	"unicode", "text", "unicode+C" },
	{ FST_XUNICODE,	5, { SUBST, CHKSZ },	"xunicode", "text","xunicode+C" },
	{ FST_UTF16,	6, { SUBST, CHKSZ },	"utf16", "binary","utf16+C" },
	{ FST_XUTF16,	6, { SUBST, CHKSZ },	"xutf16", "binary", "xutf16+C" },
	{ FST_UTF8,	7, { SUBST, CHKSZ },	"utf8", "text","utf8+C" },
	{ FST_XUTF8,	7, { SUBST, CHKSZ },	"xutf8", "text", "xutf8+C" },
	{ FST_TEXT,	0, { OK, OK },	0, 0, 0 }
} ;

static void clientCheckFileGraph( Client *client, Error *e );

const char *
clientCheckFileType( FileSys *f, FileSysType t, int clientProtocolFiles,
	             int checkSize, int scan, StrPtr *wildType,
	             StrPtr *forceType, StrPtr *msgType, Error *e )
{
	const ctTable *c;

	for( c = checkTable; c->type; c++ )
	    if( t == c->checkType )
	        break;

	if( !c->type )
	    c = checkTable;

	const char *ntype = "text";

	switch( c->action[ clientProtocolFiles >= c->xlevel ] )
	{
	case OK:
	    // Symlinks keep their detected type unless filesys.symlink.typemap
	    // is set, in which case the typemap forceType takes precedence.
	    if( forceType && ( t != FST_SYMLINK ||
	        p4tunable.Get( P4TUNE_FILESYS_SYMLINK_TYPEMAP ) ) )
	        ntype = forceType->Text();
	    else
	        ntype = c->type;
	    break;

	case CHKSZ:
	    // If server sends a maximum size for file, check it
	    if( forceType )
	        ntype = forceType->Text();
	    else if( checkSize )
	        ntype = c->cmpType;
	    else
	        ntype = c->type;
	    break;

	case ASS:
	    // Use the altType, saying we're assuming it
	    ntype = forceType ? forceType->Text() : c->altType;

	    // FST_CANTTELL covers a file we could not read and a file we chose
	    // not to read, and only the scan size tells them apart.  The second
	    // gets its own wording rather than being called unreadable.

	    if( wildType )
	        e->Set( MsgClient::CheckFileAssumeWild )
	                << f->Name() << c->type << ntype << wildType;
	    else if( t == FST_CANTTELL && FileSys::BinaryScanSize( scan ) <= 0 )
	        e->Set( MsgClient::CheckFileNoScan ) << f->Name()
	                << ( msgType ? msgType->Text() : ntype );
	    else
	        e->Set( MsgClient::CheckFileAssume ) << f->Name()
	                << c->type << ( msgType ? msgType->Text() : ntype );
	    break;

	case SUBST:
	    // Substitute altType for type
	    ntype = c->altType;

	    e->Set( MsgClient::CheckFileSubst )
	            << f->Name() << c->altType << c->type;
	    break;

	case CANT:
	    // Just can't do it
	    e->Set( MsgClient::CheckFileCant )
	            << f->Name() << c->type;
	    return 0;
	}

	return ntype;
}

void
clientCheckFile( Client *client, Error *e )
{
	client->NewHandler();
	StrPtr *clientPath = client->transfname->GetVar( P4Tag::v_path, e );
	StrPtr *clientType = client->GetVar( P4Tag::v_type );
	StrPtr *wildType = client->GetVar( P4Tag::v_type2 );
	StrPtr *msgType = client->GetVar( P4Tag::v_type3 );
	StrPtr *forceType = client->GetVar( P4Tag::v_forceType );
	StrPtr *forceTypeBits = client->GetVar( P4Tag::v_forceTypeBits );
	StrPtr *digest = client->GetVar( P4Tag::v_digest );
	StrPtr *digestType = client->GetVar( P4Tag::v_digestType );
	StrPtr *confirm = client->GetVar( P4Tag::v_confirm, e );
	StrPtr *fileSize = client->GetVar( P4Tag::v_fileSize );
	StrPtr *sendFileSize = client->GetVar( P4Tag::v_sendFileSize );
	StrPtr *sendDigest = client->GetVar( P4Tag::v_sendDigest );
	StrPtr *scanSize = client->GetVar( P4Tag::v_scanSize );
	StrPtr *ignore = client->GetVar( P4Tag::v_ignore );
	StrPtr *checkLinks = client->GetVar( P4Tag::v_checkLinks );
	StrPtr *checkLinksNs = client->GetVar( P4Tag::v_checkLinksN );
	const int checkLinksN = checkLinksNs ? checkLinksNs->Atoi() : 0;

	if( !sendFileSize && fileSize )
	    client->RemoveVar( P4Tag::v_fileSize );

	StrPtr *revertmovecheck = client->GetVar( P4Tag::v_revertmovecheck );
	if( revertmovecheck )
	{
	    // Reverting a moved file, the original name is now a directory.
	    // Check that the directory is empty.
	    int uaCount = clientDirectoryEntryCount( clientPath, e );
	    if( e->Test() )
	    {
	        client->OutputError( e );
	        return;
	    }

	    if( uaCount > 0 )
	    {
	        // The directory is not empty.  This probably occurs because
	        // another file besides the moved file was put in the directory.
	        Error msg;
	        StrPtr *revertmovedirnotempty = 
	            client->GetVar( P4Tag::v_revertmovedirnotempty );
	        if( revertmovedirnotempty ) 
	        {
	            // The operation was a preview of a revert, 
	            // so clientDeleteFile was never called.
	            // Spit out the directory not empty message so that
	            // the preview has the same messaging.
	            msg.Set( MsgClient::DirectoryNotEmpty ) << 
	                revertmovedirnotempty->Text();
	            client->GetUi()->Message( &msg );
	            client->SetError();
	            msg.Clear();
	        }

	        msg.Set( MsgClient::CantRevertDirectoryNotEmpty) <<
	                 revertmovecheck->Text() <<
	                 clientPath->Text() <<
	                 clientPath->Text();
	        client->GetUi()->Message( &msg );
	        client->SetError();

	        return;
	    }
	}

	if( e->Test() && !e->IsFatal() )
	{
	    client->OutputError( e );
	    return;
	}

	if( !clientPath && ( checkLinks || ignore || !clientType ) )
	    return;

	if( digest && digestType )
	{
	    clientCheckFileGraph( client, e );
	    return;
	}

	const char *status = "exists";
	const char *ntype = clientType ? clientType->Text() : "text";

	if( AltSyncCheckFile( client, confirm, status, ntype, e ) ||
	    e->Test() )
	    return;

	// For adding files,  checkSize is a maximum (or use alt type)
	// For flush,  checkSize is an optimization check on binary files.

	offL_t checkSize = fileSize ? fileSize->Atoi64() : 0;

	// Check for symbolic link in path

	if( checkLinks )
	{
	    PathSys *ps = PathSys::Create();
	    FileSys *fs = client->GetUi()->File( FST_BINARY );
	    ps->Set( clientPath );

	    // Don't allow opening a file for add if it is a symlink to
	    // a directory.  job092324 said this was too restrictive, so
	    // only do it if filesys.checklinks < 3.

	    fs->Set( *ps );
	    if( ( fs->Stat() & FSF_SYMLINK ) && checkLinksN < 3 )
	    {
		FileSys *fl = new FileIOSymlink;
		fl->Set( fs->Name() );
		fl->Open( FOM_READ, e );
		if( fl->Stat() & FSF_DIRECTORY )
		{
		    Error msg;
		    msg.Set( MsgClient::CheckFileBadPath )
			     << clientPath << fs->Name();
		    client->GetUi()->Message( &msg );
		    client->SetError();
		    delete fl;
		    delete fs;
		    delete ps;
		    return;
		}
		delete fl;
	    }

	    while( ps->ToParent() )
	    {
	        fs->Set( *ps );
	        if( fs->Stat() & FSF_SYMLINK )
	        {
	            Error msg;
	            msg.Set( MsgClient::CheckFileBadPath )
		             << clientPath << fs->Name();
	            client->GetUi()->Message( &msg );
	            client->SetError();
	            delete fs;
	            delete ps;
	            return;
	        }

	        // Don't walk above clientroot

	        if( !ps->Compare( *checkLinks ) )
	            break;
	    }
	    delete fs;
	    delete ps;
	}

	// 2012.1 server asks the client to do ignore checks (on add), in the
	// case of forced client type, ignore == ack, do quick confirm

	if( ignore )
	{
	    if( client->GetIgnore()->Reject( *clientPath, 
	        client->GetIgnoreFile(), 
	        client->GetEnviro()->Get( "P4CONFIG" ) ) )
	    {
	        Error msg;
	        msg.Set( MsgClient::CheckFileCant )
		         << clientPath->Text() << "ignored";
	        client->GetUi()->Message( &msg );
	        client->SetError();
	        return;
	    }

	    // No client-type check from add,  just ack
	    // Blank confirm, just return.

	    if( *ignore == P4Tag::c_Ack )
	    {
	        if( confirm && confirm->Length() )
	            client->Confirm( confirm );
	        return;
	    }
	}

	/*
	 * Behavior is split depending on whether clientType is set.
	 */

	if( clientType )
	{
	    /*
	     * If we do know the type, we want to know if it's missing.
	     * If it isn't missing and a digest is given, we want to know if
	     * it is the same (requires a real ft+le+cmp value)
	     */

	    FileSys *f = ClientSvc::FileFromPath( client, P4Tag::v_path,
	                                          ( digest || sendDigest )
	                                              ? P4Tag::v_type : 0,
	                                          e );

	    if( e->Test() || !f )
		return;
	    int statVal = f->Stat();

	    if( !( statVal & ( FSF_SYMLINK|FSF_EXISTS ) ) )
	    {
		status = "missing";
	    } 
	    else if ( ( !( statVal & FSF_SYMLINK ) && ( f->IsSymlink() )
						   && f->SymlinksSupported() )
	  	    || ( ( statVal & FSF_SYMLINK ) && !( f->IsSymlink() ) ) ) 
	    {
	        ; /* do nothing */
	    }
	    // serverSize is the byte count f->Digest() returned -- the
	    // translated (LF-normalized) size, matching the server's
	    // storage record.  f->GetSize() would be the on-disk byte
	    // count and mismatch for Windows text files (CRLF).

	    offL_t serverSize = -1;

	    if( digest || sendDigest )
	    {
	        if( !checkSize || checkSize == f->GetSize() )
	        {
		    StrBuf localDigest;

                    f->Translator( ClientSvc::XCharset( client, FromClient ) );

		    serverSize = f->Digest( &localDigest, e );

		    if( !e->Test() )
		    {
		        if( digest && !localDigest.XCompare( *digest ) )
		            status = "same";
		        if( sendDigest )
		            client->SetVar( P4Tag::v_digest, localDigest );
		    }
		    else
		    {
		        // Partial read; fall back to GetSize below.

		        serverSize = -1;
		    }
	        }

		// If we can't read the file (for some reason -- wrong type?)
		// we consider the files different.

		e->Clear();
	    }

	    if( sendFileSize )
	        client->SetVar( P4Tag::v_fileSize,
	                        StrNum( serverSize >= 0
	                                ? serverSize : f->GetSize() ) );

	    delete f;
	}
	else
	{
	    /*
	     * If we don't know the type, that's what we're trying to find out.
	     *
	     * This mode (!clientType) is compatible with pre-1742 servers.
	     *
	     * Set type to binary, as OS2 won't even read binary chars
	     * from a file opened as text.
	     */

	    int scan = -1;
	    if( scanSize )
	        scan = scanSize->Atoi();

	    FileSys *f = client->GetUi()->File( FST_BINARY );
	    f->SetContentCharSetPriv( client->ContentCharset() );
	    f->Set( *clientPath );

	    // Every content-derived row of checkTable resolves to forceType at
	    // xfiles 7 or better, and what is left -- symlink, directory,
	    // special, empty, unreadable -- comes from stat(), so there is
	    // nothing left for the content to decide.  A wildcard type is
	    // overlaid onto the detected type, so it does not qualify.
	    //
	    // 7 is the highest xlevel in checkTable; a new content-derived row
	    // above it has to raise this too.

	    FileSysType t;

	    if( forceType && forceTypeBits && !wildType &&
	        client->protocolXfiles >= 7 )
	        t = f->CheckTypeStat( 1 );
	    else
	        t = f->CheckType( scan );

	    offL_t size = f->GetSize();

	    if( sendFileSize )
	    {
	        // Size against the type the file will be stored as, which is
	        // forceTypeBits when the typemap supplied one and the detected
	        // type otherwise.

	        FileSysType st = t;

	        // A detected symlink keeps its own type unless
	        // filesys.symlink.typemap lets the typemap win, so sizing has
	        // to follow the same rule the reported type does.

	        if( forceTypeBits &&
	            ( t != FST_SYMLINK ||
	              p4tunable.Get( P4TUNE_FILESYS_SYMLINK_TYPEMAP ) ) )
	        {
	            Error te;
	            FileSysType ft = LookupType( forceTypeBits, &te );

	            if( !te.Test() )
	                st = ft;
	        }

	        // These are the stat() verdicts that name something with no
	        // content to read: no file, a directory or other non-file, no
	        // bytes, or bytes this client could not or was told not to
	        // look at.  A translating stored type would otherwise send the
	        // sizing read at them, where it can only fail, and the failure
	        // is not the user's: the add proceeds, and
	        // clientCheckFileType() below reports the condition in its own
	        // words.  stat()'s count is what a client without
	        // forceTypeBits sends for these anyway.

	        FileSysType tbase = (FileSysType)( t & FST_MASK );

	        int noContent = tbase == FST_MISSING   ||
	                        tbase == FST_EMPTY     ||
	                        tbase == FST_CANTTELL  ||
	                        tbase == FST_DIRECTORY ||
	                        tbase == FST_SPECIAL;

	        // Ask before building the FileSys, so the common case costs no
	        // allocation per file.  A symlink stores its link target rather
	        // than the bytes it points at, so it always reads.

	        if( !noContent &&
	            ( ( st & FST_MASK ) == FST_SYMLINK ||
	              !ClientSvc::SizeMatchesDepot( st,
	                  ClientSvc::XCharset( client, FromClient ) ) ) )
	        {
	            FileSys *f2 = client->GetUi()->File( st );

	            if( f2 )
	            {
	                f2->SetContentCharSetPriv( client->ContentCharset() );
	                f2->Set( *clientPath );

	                // A local Error: Digest() neither clears the one it is
	                // given nor reads with one already set, and e can
	                // arrive here already set.

	                Error de;

	                size = f2->Digest( 0, &de );

	                // A typemap type its content does not fit makes the
	                // read fail where the detected type would not.  That
	                // is an artifact of the type, so the truncated count
	                // and the error are both dropped.  Any other failure
	                // is real and propagates as before.

	                if( de.Test() )
	                {
	                    if( de.CheckId( MsgSupp::NoTrans ) ||
	                        de.CheckId( MsgSupp::PartialChar ) )
	                        size = f->GetSize();
	                    else
	                        e->Merge( de );
	                }

	                delete f2;
	            }
	        }

	        client->SetVar( P4Tag::v_fileSize, StrNum( size ) );
	    }

	    Error msg;
	    ntype = clientCheckFileType( f, t, client->protocolXfiles,
	                                 fileSize && size > checkSize,
	                                 scan, wildType, forceType, msgType,
	                                 &msg );
	    if( msg.GetSeverity() )
	        client->GetUi()->Message( &msg );
	    delete f;
	    if( !ntype )
	    {
	        client->SetError();
	        return;
	    }
	}

	// set the charset here?

	client->SetVar( P4Tag::v_type, ntype );
	client->SetVar( P4Tag::v_status, status );
	client->Confirm( confirm );

	// Report non-fatal error and clear it.

	client->OutputError( e );
}

void
clientCheckFileGraph( Client *client, Error *e )
{
	StrPtr *clientType = client->GetVar( P4Tag::v_type, e );
	StrPtr *digest = client->GetVar( P4Tag::v_digest, e );
	StrPtr *digestType = client->GetVar( P4Tag::v_digestType, e );
	StrPtr *confirm = client->GetVar( P4Tag::v_confirm, e );

	if( e->Test() && !e->IsFatal() )
	{
	    client->OutputError( e );
	    return;
	}

	FileDigestType digType;
	if( !digestType->Compare(
	                 StrRef( P4Tag::v_digestTypeMD5 ) ) )
	    digType = FS_DIGEST_MD5;
	else if( !digestType->Compare(
	                 StrRef( P4Tag::v_digestTypeGitText ) ) )
	    digType = FS_DIGEST_GIT_TEXT_SHA1;
	else if( !digestType->Compare(
	                 StrRef( P4Tag::v_digestTypeGitBinary ) ) )
	    digType = FS_DIGEST_GIT_BINARY_SHA1;
	else if( !digestType->Compare(
	                 StrRef( P4Tag::v_digestTypeSHA256 ) ) )
	    digType = FS_DIGEST_SHA256;

	const char *status = "exists";

	FileSys *f = ClientSvc::File( client, e );

	if( e->Test() || !f )
	    return;
	int statVal = f->Stat();

	if( !( statVal & (FSF_SYMLINK|FSF_EXISTS) ) )
	{
	    status = "missing";
	} 
	else
	{
	    StrBuf localDigest;

	    f->ComputeDigest( digType, &localDigest, e );

	    if( !e->Test() && !localDigest.XCompare( *digest ) )
	        status = "same";

	    e->Clear();
	}

	delete f;

	client->SetVar( P4Tag::v_type, clientType->Text() );
	client->SetVar( P4Tag::v_status, status );
	client->Confirm( confirm );

	client->OutputError( e );
}

// Integ support

void
clientActionResolve( Client *client, Error *e )
{
	// So as to be 100% translatable, action resolve gets
	// all of its strings sent as Error objects from the server.

	// should always get these
	StrPtr *actionType	= client->GetVar( P4Tag::v_rActionType, e );
	StrPtr *autoResult	= client->GetVar( P4Tag::v_rAutoResult, e );

	// should get preview or confirm/decline
	StrPtr *preview		= client->GetVar( P4Tag::v_preview );
	StrPtr *confirm		= client->GetVar( P4Tag::v_confirm );
	StrPtr *decline		= client->GetVar( P4Tag::v_decline );

	// should definitely get at least one of these
	StrPtr *actionMerge	= client->GetVar( P4Tag::v_rActionMerge );
	StrPtr *actionTheirs	= client->GetVar( P4Tag::v_rActionTheirs );
	StrPtr *actionYours	= client->GetVar( P4Tag::v_rActionYours );

	// CLI UI strings -- might not always need/get these?
	StrPtr *optAuto		= client->GetVar( P4Tag::v_rOptAuto );
	StrPtr *optHelp		= client->GetVar( P4Tag::v_rOptHelp );
	StrPtr *optMerge	= client->GetVar( P4Tag::v_rOptMerge );
	StrPtr *optSkip		= client->GetVar( P4Tag::v_rOptSkip );
	StrPtr *optTheirs	= client->GetVar( P4Tag::v_rOptTheirs );
	StrPtr *optYours	= client->GetVar( P4Tag::v_rOptYours );
	StrPtr *promptMerge	= client->GetVar( P4Tag::v_rPromptMerge );
	StrPtr *promptTheirs	= client->GetVar( P4Tag::v_rPromptTheirs );
	StrPtr *promptYours	= client->GetVar( P4Tag::v_rPromptYours );
	StrPtr *promptType	= client->GetVar( P4Tag::v_rPromptType );
	StrPtr *userError	= client->GetVar( P4Tag::v_rUserError );
	StrPtr *userHelp	= client->GetVar( P4Tag::v_rUserHelp );
	StrPtr *userPrompt	= client->GetVar( P4Tag::v_rUserPrompt );

	StrPtr *moveReaddIntegConflictIgnored =
	    client->GetVar( P4Tag::v_rMoveReaddIntegConflictIgnored );
	StrPtr *moveReaddIntegConflictSkip =
	    client->GetVar( P4Tag::v_rMoveReaddIntegConflictSkip );

	if ( !e->Test() && !preview && ( !confirm || !decline ) )
	    e->Set( MsgSupp::NoParm ) << "confirm/decline";

	if ( e->Test() || !actionType->Length() )
	{
	    client->OutputError( e );
	    return; 
	}

	Error   mActionType, mActionMerge, mActionTheirs, mActionYours,
		mOptAuto, mOptHelp, mOptMerge, mOptSkip, mOptTheirs,
		mOptYours, mPromptMerge, mPromptTheirs,	mPromptYours, 
		mPromptType, mUserError, mUserHelp, mUserPrompt,
		mMoveReaddIntegConflictIgnored, mMoveReaddIntegConflictSkip;

				mActionType.	UnMarshall2( *actionType );
	if ( actionMerge )	mActionMerge.	UnMarshall2( *actionMerge );
	if ( actionTheirs )	mActionTheirs.  UnMarshall2( *actionTheirs );
	if ( actionYours )	mActionYours.	UnMarshall2( *actionYours );
	if ( optAuto )		mOptAuto.	UnMarshall2( *optAuto );
	if ( optHelp )		mOptHelp.	UnMarshall2( *optHelp );
	if ( optMerge )		mOptMerge.	UnMarshall2( *optMerge );
	if ( optSkip )		mOptSkip.	UnMarshall2( *optSkip );
	if ( optTheirs )	mOptTheirs.	UnMarshall2( *optTheirs );
	if ( optYours )		mOptYours.	UnMarshall2( *optYours );
	if ( promptMerge )	mPromptMerge.	UnMarshall2( *promptMerge );
	if ( promptTheirs )	mPromptTheirs.	UnMarshall2( *promptTheirs );
	if ( promptYours )	mPromptYours.	UnMarshall2( *promptYours );
	if ( promptType )	mPromptType.	UnMarshall2( *promptType );
	if ( userError )	mUserError.	UnMarshall2( *userError );
	if ( userHelp )		mUserHelp.	UnMarshall2( *userHelp );
	if ( userPrompt )	mUserPrompt.	UnMarshall2( *userPrompt );

	if ( moveReaddIntegConflictIgnored )
	    mMoveReaddIntegConflictIgnored.UnMarshall2( 
	        *moveReaddIntegConflictIgnored );

	if ( moveReaddIntegConflictSkip )
	    mMoveReaddIntegConflictSkip.UnMarshall2( 
	        *moveReaddIntegConflictSkip );

	// set up ClientResolveA object

	ClientResolveA resolve( client->GetUi() );

	MergeStatus suggest;
	if ( !autoResult )
	    suggest = CMS_SKIP;
	else if ( *autoResult == P4Tag::v_rOptTheirs )
	    suggest = CMS_THEIRS;
	else if ( *autoResult == P4Tag::v_rOptMerge )
	    suggest = CMS_MERGED;
	else if ( *autoResult == P4Tag::v_rOptYours )
	    suggest = CMS_YOURS;
	else
	    suggest = CMS_SKIP;
	resolve.SetAuto( suggest );

	resolve.SetType       ( mActionType );
	resolve.SetMergeAction( mActionMerge );
	resolve.SetTheirAction( mActionTheirs );
	resolve.SetYoursAction( mActionYours );
	resolve.SetAutoOpt    (	mOptAuto );
	resolve.SetHelpOpt    (	mOptHelp );
	resolve.SetMergeOpt   (	mOptMerge );
	resolve.SetSkipOpt    (	mOptSkip );
	resolve.SetTheirOpt   (	mOptTheirs );
	resolve.SetYoursOpt   (	mOptYours );
	resolve.SetMergePrompt( mPromptMerge );
	resolve.SetTheirPrompt( mPromptTheirs );
	resolve.SetYoursPrompt( mPromptYours );
	resolve.SetTypePrompt ( mPromptType );
	resolve.SetUsageError (	mUserError );
	resolve.SetHelp       (	mUserHelp );
	resolve.SetPrompt     (	mUserPrompt );

	if ( moveReaddIntegConflictIgnored )
	    resolve.SetMoveReaddIntegConflictIgnored (
	        mMoveReaddIntegConflictIgnored );

	if ( moveReaddIntegConflictSkip )
	    resolve.SetMoveReaddIntegConflictSkip(
	        mMoveReaddIntegConflictSkip );

	// ask ClientUser to do the resolve

	int userResult = client->GetUi()->Resolve( &resolve, !!preview, e );

	if ( e->Test() )
	{
	    client->GetUi()->Message( e );
	    e->Clear();
	    userResult = CMS_QUIT;
	}

	if ( preview )
	    return;

	// back to the server with the result

	switch( userResult )
	{
	case CMS_THEIRS:
	    client->SetVar( P4Tag::v_rUserResult, P4Tag::v_rOptTheirs );
	    break;
	case CMS_YOURS:
	    client->SetVar( P4Tag::v_rUserResult, P4Tag::v_rOptYours );
	    break;
	case CMS_MERGED:
	    client->SetVar( P4Tag::v_rUserResult, P4Tag::v_rOptMerge );
	    break;
	case CMS_SKIP:
	default:
	    client->SetVar( P4Tag::v_rUserResult, P4Tag::v_rOptSkip );
	    confirm = decline;
	    break;
	}

	client->Confirm( confirm );
}

void
clientOpenMerge( Client *client, Error *e )
{
	client->NewHandler();
	StrPtr *clientPath = client->transfname->GetVar( P4Tag::v_path, e );
	StrPtr *clientHandle = client->GetVar( P4Tag::v_handle, e );
	StrPtr *func = client->GetVar( P4Tag::v_func, e );
	StrPtr *clientType = client->GetVar( P4Tag::v_type );
	StrPtr *resultType = client->GetVar( P4Tag::v_type2 );
	StrPtr *theirType = client->GetVar( P4Tag::v_type3 );
	StrPtr *baseType = client->GetVar( P4Tag::v_type4 );
	StrPtr *showAll = client->GetVar( P4Tag::v_showAll );
	StrPtr *diffFlags = client->GetVar( P4Tag::v_diffFlags );
	StrPtr *noBase = client->GetVar( P4Tag::v_noBase );
	StrPtr *digest = client->GetVar( P4Tag::v_digest );
	StrPtr *modTime = client->GetVar( P4Tag::v_theirTime );

	FileSys *s = ClientSvc::File( client, e ); // For P4CLIENTPATH verification

	if( e->Test() || !s )
	{
	    delete s;
	    if ( !e->IsFatal() )
		client->OutputError( e );
	    return;
	}

	delete s;
	s = 0;

	// very old servers do not send result type
	if( !resultType )
	    resultType = clientType;

	// old servers do not send theirs/base types
	if ( !theirType )
	    theirType = resultType;
	if ( !baseType )
	    baseType = clientType;

	// Create integrator
	// Note that noBase comes from pre-12354 servers.

	MergeType mt;

	if( !strcmp( func->Text(), P4Tag::c_OpenMerge2 ) )
	    mt = CMT_BINARY;
	else if( noBase )
	    mt = CMT_2WAY;
	else
	    mt = CMT_3WAY;

	// Set binary/text/etc file type - mostly for exec bit

	FileSysType type = LookupType( clientType, e );
	FileSysType rType = LookupType( resultType, e );
	FileSysType tType = LookupType( theirType, e );
	FileSysType bType = LookupType( baseType, e );

	if( e->Test() && client->CheckFileType() )
	    return;
	else
	    e->Clear();

	ClientMerge *merge = ClientMerge::Create
		( client->GetUi(), type, rType, tType, bType, mt );

	// Pass on showAll/diff flag.

	if( showAll )
	    merge->SetShowAll();

	if( diffFlags )
	    merge->SetDiffFlags( diffFlags );

	// Save digest for binary resolve, 2003.1 sends base digest.

	if( client->protocolServer >= 16 && digest )
	    merge->CopyDigest( digest, e );

	if( modTime )
	    merge->SetTheirModTime( modTime );

	// Create handle
	// If that fails, we have to toss the integrator

	client->handles.Install( clientHandle, merge, e );

	if( e->Test() )
	{
	    delete merge;
	    return;
	}

	// Start integration.
	// If that fails, we mark the handle so the rest of the
	// protocol is honored (but ignored).

	// You would think these names should be translated
	// but not really... These names are used as markers
	// in the generation of merge files so we actually
	// want these in UTF-8 because the merge code will
	// translated them if the type is unicode.
	// If the file type is text then we do want these
	// translated...  This is really confusing...  Sorry...
	StrDict *usedict = client;
	if( ( type & FST_MASK ) != FST_UNICODE )
	    usedict = client->transfname;
	merge->SetNames( 
		usedict->GetVar( P4Tag::v_baseName ),
		usedict->GetVar( P4Tag::v_theirName ),
		usedict->GetVar( P4Tag::v_yourName ) );

	merge->Open( clientPath, e, ClientSvc::XCharset( client, FromServer ),
			client->ContentCharset() );

	// Mark handle with any error
	// Report non-fatal error and clear it.

	merge->SetError( e );
	client->OutputError( e );
}

void
clientWriteMerge( Client *client, Error *e )
{
	StrPtr *clientHandle = client->GetVar( P4Tag::v_handle, e );
	StrPtr *data = client->GetVar( P4Tag::v_data, e );
	StrPtr *bits = client->GetVar( P4Tag::v_bits );
	ClientMerge *merge;

	if( e->Test() )
	    return;

	// Get handle.

	merge = (ClientMerge *)client->handles.Get( clientHandle, e );

	if( e->Test() || merge->IsError() )
	    return;

	// Write data.

	merge->Write( data, bits, e );

	// Mark handle with any error
	// Report non-fatal error and clear it.

	merge->SetError( e );
	client->OutputError( e );
}

void
clientCloseMerge( Client *client, Error *e )
{
	StrPtr *clientHandle = client->GetVar( P4Tag::v_handle, e );
	StrPtr *mergeConfirm = client->GetVar( P4Tag::v_mergeConfirm );
	StrPtr *mergeDecline = client->GetVar( P4Tag::v_mergeDecline );
	StrPtr *mergePerms = client->GetVar( P4Tag::v_mergePerms );
	StrPtr *mergeAuto = client->GetVar( P4Tag::v_mergeAuto );
	const StrPtr *resultDigest;
	ClientMerge *merge;
	int manualMerge = 0;

	if( e->Test() )
	    return;

	// Get handle.

	merge = (ClientMerge *)client->handles.Get( clientHandle, e );

	if( e->Test() )
	    return;

	// Close & resolve.

	merge->Close( e );

	// Mark handle with any error

	merge->SetError( e );

	// Accept or decline integration depending on what Resolve() said.

	// If mergeConfirm is null, it means sender detected error and
	// doesn't want us to proceed.

	if( merge->IsError() )
	{
	    mergeConfirm = mergeDecline;
	}
	else while( mergeConfirm )
	{
	    MergeStatus stat;

	    // Make user's file writeable for the duration of the resolve.
	    // We only do this if mergePerms is set for two reasons: 1) then
	    // we can be sure to revert the file to the proper perms below,
	    // 2) only servers which set mergePerms are smart enough to leave
	    // files opened for integ r/o to begin with.

	    if( mergePerms )
		merge->Chmod( "rw", e );

	    // Do automatic merge if requested.
	    // 'auto' means skip conflicts.
	    // 'force' means accept conflicts.

	    if( mergeAuto && *mergeAuto == "safe" )
	    {
		stat = merge->AutoResolve( CMF_SAFE );
	    }
	    else if( mergeAuto && *mergeAuto == "force" )
	    {
		stat = merge->AutoResolve( CMF_FORCE );
	    }
	    else if( mergeAuto && *mergeAuto == "auto" )
	    {
		stat = merge->AutoResolve( CMF_AUTO );
	    }
	    else // manual merge
	    {
		stat = (MergeStatus)client->GetUi()->Resolve( merge, e );
	        manualMerge = 1;
	    }

	    // server2 < 11 can't take CMS_EDIT.

	    if( stat == CMS_EDIT && client->protocolServer < 11 )
		stat = CMS_MERGED;

	    // Formulate response to server according to user's action.
	    // If the target file was updated

	    switch( stat )
	    {
	    case CMS_QUIT:	// user wants to quit
		mergeConfirm = mergeDecline;
		break;

	    case CMS_SKIP:	// skip the integration record
		mergeConfirm = mergeDecline;
		break;

	    case CMS_MERGED: // accepted merged theirs and yours
		resultDigest = merge->GetMergeDigest();
		if( resultDigest )
		    client->SetVar( P4Tag::v_digest, resultDigest );
		client->SetVar( P4Tag::v_mergeHow, "merged" );
		break;

	    case CMS_EDIT: // accepted edited merge
		client->SetVar( P4Tag::v_mergeHow, "edit" );
		break;

	    case CMS_THEIRS: // accepted theirs
	    {
		resultDigest = merge->GetTheirDigest();
		if( resultDigest )
		    client->SetVar( P4Tag::v_digest, resultDigest );
		client->SetVar( P4Tag::v_mergeHow, "theirs" );
		const char *forced = "no";
		if( merge->GetYourChunks() > 0 ||
		    merge->GetConflictChunks() > 0 )
		    forced = "yes";
		else if( merge->GetTheirChunks() > 0 )
		    forced = "theirs";
		client->SetVar( P4Tag::v_force, forced );
		break;
	    }
	    case CMS_YOURS:	// accepted yours
		resultDigest = merge->GetYourDigest();
		if( resultDigest )
		    client->SetVar( P4Tag::v_digest, resultDigest );
		client->SetVar( P4Tag::v_mergeHow, "yours" );
		break;
	    }

	    // Move selected file into position

	    if( !e->Test() )
		merge->Select( stat, e );

	    // Rename could fail because yours file is busy, retry

	    if( e->Test() && manualMerge && mergeConfirm != mergeDecline )
	    {
	        client->RemoveVar( P4Tag::v_mergeHow );
	        client->OutputError( e );
	        e->Clear();
	        continue;
	    }

	    // Revert permissions

	    if( !e->Test() && mergePerms )
		merge->Chmod( mergePerms->Text(), e );

	    // bail if errors in select or chmod

	    if( e->Test() )
		mergeConfirm = mergeDecline;

	    break;
	}

	// Confirm resolve.

	if( mergeConfirm )
	    client->Confirm( mergeConfirm );

	// Handle remembers if any error occurred 
	// Report non-fatal error and clear it.

	merge->SetError( e );
	client->OutputError( e );

	delete merge;
}

# ifdef HAS_EXTENSIONS

// Helper to perform actions or influence flow control in chunking
// callbacks during testing.

static ClientScriptAction chunkSendDebugHook( Client* client,
	                                      const char* cbName,
	                                      const char* fnName, Error* e )
{
	if( !( client->ExtensionsEnabled() &&
	    client->ExtensionsDebugHooksEnabled() ) )
	    return ClientScriptAction::PASS;

	ClientScript* exts = client->GetExtensions();

	const auto [ r, nRun ] =
	    exts->Run( cbName, fnName, client->GetUi(), true, e );

	if( e->Test() || r == ClientScriptAction::FAIL )
	{
	    if( !e->IsFatal() )
	        client->OutputError( e );
	    return ClientScriptAction::FAIL;
	}

	return r;
}

# endif // HAS_EXTENSIONS

# ifdef USE_CDC

static void
clientSendFileChunkMap( Client *client, ProgressReport **progress, FileSys *f,
	                MD5* md5, offL_t& filesize, ChunkMap& cm,
	                const StrPtr* writeMap, Error *e )
{
# ifdef HAS_EXTENSIONS
	if( chunkSendDebugHook( client, "ClientSendFileChunkMapBegin",
	                        "clientSendFileChunkMap", e ) ==
	    ClientScriptAction::EARLY_RETURN )
	    return;
# endif

	StrPtr* depotFile   = client->GetVar( P4Tag::v_depotFile  , e );
	StrPtr* depotRev    = client->GetVar( P4Tag::v_depotRev   , e );
	StrPtr* depotChange = client->GetVar( P4Tag::v_depotChange, e );
	StrPtr* lbrChange   = client->GetVar( P4Tag::v_lbrChange  , e );
	StrPtr* token       = client->GetVar( P4Tag::v_chunkToken , e );
	StrPtr* index       = client->GetVar( P4Tag::v_index      , e );

	if( e->Test() )
	    return;

	// Force binary-mode for no translation.  Note that this is the
	// second time the file is opened, since clientSendFile() has one
	// too.

	const StrRef fn( f->Name() );
	FileSys* fs = ClientSvc::FileFromPathNoVar( client, &fn, FST_BINARY,
	                                            nullptr, e );
	if( e->Test() )
	    return;

	fs->Open( FOM_READ, e );

	if( e->Test() )
	{
	    delete fs;
	    return;
	}

	// This reads the whole file, and will often times be I/O-bound.

	// If the caller needs an MD5 of the content, slowly generate it here
	// since the file is already being read.  A progress report probably
	// isn't necessary here since there already is one in the network
	// transfer.

	cm.Create( fs, e, md5 );

	fs->Close( e );
	delete fs;

	if( e->Test() )
	    return;

	// Send the client chunk map back to server to store with new archive.
	// A chunkmap, while generally small, has no strict upper bound on
	// size, so in order to avoid bumping into the maximum RPC message
	// size, we stream the chunk map content to the server across multiple
	// messages.

	const StrPtr* cmBuf = cm.GetBuf();
	const P4INT64 mapLen = cmBuf->Length();
	const int bSize = fs->BufferSize();

	MD5 digest;
	digest.Update( *cmBuf );

	for( P4INT64 i = 0; i < mapLen; i += bSize )
	{
	    const int len = ( i + bSize <= mapLen ) ? bSize : mapLen - i;
	    StrBuf *bu = client->MakeVar( P4Tag::v_data );
	    char *b = bu->BlockAlloc( len );
	    memcpy( b, cmBuf->Text() + i, len );

	    client->SetVar( P4Tag::v_depotFile  , depotFile );
	    client->SetVar( P4Tag::v_depotRev   , depotRev );
	    client->SetVar( P4Tag::v_depotChange, depotChange );
	    client->SetVar( P4Tag::v_lbrChange  , lbrChange );
	    client->SetVar( P4Tag::v_chunkToken , token );
	    client->SetVar( P4Tag::v_index      , index );

	    // Only send the file size at the end, so the server can use it to
	    // know when the map has been completely transferred and can verify

	    if( (i + len) == mapLen )
	    {
	        client->SetVar( P4Tag::v_fileSize, StrNum( filesize ) );
	        StrBuf dbuf;
	        digest.Final( dbuf );
	        client->SetVar( P4Tag::v_digest, dbuf );
	    }

	    client->Invoke( writeMap->Text() );
	}
}

static VVarArray *
clientGetChunksToSend( Client *client, ChunkMap& cm, Error *e )
{
	ChunkMap *sm = clientReceiveChunkMap( client, e );

	if( e->Test() || !sm )
	    return 0;

	// Find out which chunks are only on the client.

	VVarArray *dm = cm.Diff( *sm, e );

	delete sm;

	if( e->Test() )
	{
	    delete dm;
	    return 0;
	}
	return dm;
}

static void
clientSendFileChunked( Client *client, ProgressReport **progress, FileSys *f,
	               offL_t& filesize, VarArray *dm,
	               const StrPtr *clientPath, const StrPtr *handle,
	               const StrPtr *write, Error *e )
{
# ifdef HAS_EXTENSIONS
	chunkSendDebugHook( client, "ClientSendFileChunkedBegin",
	                            "clientSendFileChunked", e );
# endif

	if( !dm )
	    return;
	
	StrPtr* depotFile   = client->GetVar( P4Tag::v_depotFile   , e );
	StrPtr* depotRev    = client->GetVar( P4Tag::v_depotRev    , e );
	StrPtr* depotChange = client->GetVar( P4Tag::v_depotChange , e );
	StrPtr* lbrChange   = client->GetVar( P4Tag::v_lbrChange   , e );
	StrPtr* token       = client->GetVar( P4Tag::v_chunkToken  , e );
	StrPtr* index       = client->GetVar( P4Tag::v_index       , e );

	if( e->Test() )
	    return;

	int n = dm->Count();

	ClientProgress *indicator = nullptr;
	if( ( indicator = client->GetUi()->CreateProgress( CPT_SENDFILE, n ) ) )
	{
	    *progress = new ClientProgressReport( indicator );
	    (*progress)->Description( *clientPath );
	    (*progress)->Units( PRU_DELTAS );
	    (*progress)->Total( dm->Count() );
	}

	// Now do the truffle shuffle and send new chunks back to server.

	// Note that for the case where there is zero overlap between the
	// prior revision and the client revision, we'll have to read the
	// whole client file twice - once for chunking and once for
	// transmission.

	for( int i = 0; i < n; i++ )
	{
# ifdef HAS_EXTENSIONS
	    if( chunkSendDebugHook( client, "ClientSendFileChunkedLoop",
	                            "clientSendFileChunked", e ) ==
	        ClientScriptAction::EARLY_RETURN )
	    {
	        return;
	    }
# endif

	    if( client->Dropped() )
	    {
	        return;
	    }

	    const ChunkMap::Chunk *chunk = (ChunkMap::Chunk *)dm->Get( i );

	    f->Seek( chunk->offset, e );

	    if( e->Test() )
	    {
	        return;
	    }

	    StrBuf *bu = client->MakeVar( P4Tag::v_data );
	    char *b = bu->BlockAlloc( chunk->size );
	    const int l = f->Read( b, chunk->size, e );

	    if( e->Test() )
	    {
	        return;
	    }

	    client->SetVar( P4Tag::v_depotFile, depotFile );
	    client->SetVar( P4Tag::v_depotRev, depotRev );
	    client->SetVar( P4Tag::v_depotChange, depotChange );
	    client->SetVar( P4Tag::v_lbrChange, lbrChange );
	    client->SetVar( P4Tag::v_offset, chunk->offset );
	    client->SetVar( P4Tag::v_size, (int)chunk->size );
	    client->SetVar( P4Tag::v_hash, chunk->hash );
	    client->SetVar( P4Tag::v_hashType, 0 );
	    client->SetVar( P4Tag::v_compression, 0 );
	    client->SetVar( P4Tag::v_chunkToken, token );
	    client->SetVar( P4Tag::v_handle, handle );
	    client->SetVar( P4Tag::v_index, index );
	    client->Invoke( write->Text() );
	    // todo: add RPC::PriorityDispatch( 1 ) here like in lbrWriteFile?

	    client->sendClientBytes += l;

	    if( *progress )
	        (*progress)->Position( i, e->Test() ? CPP_FAILDONE
	                                            : CPP_NORMAL );
	}

	if( *progress )
	    (*progress)->Position( n, CPP_DONE );
}

# endif // USE_CDC

void
clientSendFileWhole( Client *client, ProgressReport **progress, FileSys *f,
	             MD5* md5, offL_t& len, const offL_t filesize,
	             const int sendDigest, const StrPtr *handle,
	             const StrPtr *write, const StrPtr *clientPath, Error *e )
{
	ClientProgress *indicator = NULL;

	if( ( indicator = client->GetUi()->CreateProgress( CPT_SENDFILE,
	                                                   filesize ) ) )
	{
	    *progress = new ClientProgressReport( indicator );
	    (*progress)->Description( *clientPath );
	    (*progress)->Units( PRU_KBYTES );
	    (*progress)->Total( filesize / 1024 );
	}

	const int size = FileSys::BufferSize();

	while( !client->Dropped() )
	{
		StrBuf *bu = client->MakeVar( P4Tag::v_data );
		char *b = bu->Alloc( size );
		int l = f->Read( b, size, e );

		if( e->Test() )
		{
		    if( *progress )
			(*progress)->Increment( 0, CPP_FAILDONE );
		    bu->SetEnd( b );
		    break;
		}

# ifdef USE_EBCDIC
		// Pre un-Translate!
		if( !f->IsTextual() )
		    __atoe_l( b, l );
# endif

		bu->SetEnd( b + l );

		len += l;

		if( *progress )
		{
		    if( l )
			(*progress)->Position( len / 1024, CPP_NORMAL );
		    else
			(*progress)->Position( filesize / 1024, CPP_DONE );
		}

		if( !l )
		    break;

		if( sendDigest )
		{
#ifdef USE_EBCDIC
		    __etoa_l( b, l );
#endif
		    if( md5 )
		        md5->Update( StrRef( b, l ) );
#ifdef USE_EBCDIC
		    __atoe_l( b, l );
#endif
		}

		client->sendClientBytes += l;
		client->SetVar( P4Tag::v_handle, handle );
		client->Invoke( write->Text() );
	}
}

void
clientSendFile( Client *client, Error *e )
{
	client->NewHandler();
	StrPtr *clientPath = client->transfname->GetVar( P4Tag::v_path, e );
	StrPtr *perms = client->GetVar( P4Tag::v_perms );
	StrPtr *handle = client->GetVar( P4Tag::v_handle, e );
	StrPtr *open = client->GetVar( P4Tag::v_open, e );
	StrPtr *write = client->GetVar( P4Tag::v_write, e );
# ifdef USE_CDC
	StrPtr *chunkWrite = client->GetVar( P4Tag::v_chunkWrite );
# endif
	StrPtr *chunkMapWrite = client->GetVar( P4Tag::v_chunkMapWrite );
	StrPtr *confirm = client->GetVar( P4Tag::v_confirm, e );
	StrPtr *decline = client->GetVar( P4Tag::v_decline, e );
	StrPtr *serverDigest = client->GetVar( "serverDigest" );
	StrPtr *digestType = client->GetVar( P4Tag::v_digestType );
	StrPtr *pendingDigest = client->GetVar( "pendingDigest" );
	StrPtr *revertUnchanged = client->GetVar( P4Tag::v_revertUnchanged );
	StrPtr *depotTime = client->GetVar( P4Tag::v_depotTime );
	StrPtr *reopen = client->GetVar( P4Tag::v_reopen );
	StrPtr *skipDigestCheck = client->GetVar( "skipDigestCheck" );
	StrPtr *index = client->GetVar( P4Tag::v_index );

	if( e->Test() && !e->IsFatal() )
	{
	    client->OutputError( e );
	    return;
	}

	FileSys *f = ClientSvc::File( client, e );

	if( e->Test() )
	    return;

	if( !f )
	{
	    client->Confirm( open );
	    client->Confirm( decline );
	    return;
	}

	// 2016.2 submit + reopen chmod +RW upon success

	int chmodReopen = ( client->protocolServer >= 42 );

	// 2014.2 submit --forcenoretransfer skips file transfer to call 
	// dmsubmitfile directly, only fixing up file perms as needed.

	if( skipDigestCheck )
	{
	    client->SetVar( P4Tag::v_status, "same" );
	    client->SetVar( P4Tag::v_digest, skipDigestCheck );
	    client->Confirm( confirm );

	    // Ignore failure to chmod file since the file may not exist

	    Error te;

	    if( !chmodReopen && perms )
		f->Chmod2( perms->Text(), &te );
	    else if( chmodReopen )
	    {
		if( perms && !reopen )
		    f->Chmod2( perms->Text(), &te );
		else if( reopen )
		    f->Chmod2( FPM_RW, e );
	    }
	    delete f;
	    return;
	}

	// 2000.1 started sending modTime
	// 2003.2 wants a digest
	// 2005.1 sends filesize

	int modTime = f->StatModTime();
	int sendDigest = ( client->protocolServer >= 17 );
	int sendFileSize = ( client->protocolServer >= 19 );

	StrBuf digest;
	offL_t len = 0;
	offL_t filesize = 0;
	MD5 md5;

	// 2006.2 server, sends digest for 'p4 submit -R'
	// 2010.2 server @257282 sends digest for 'p4 resolve' of integ
	// 2014.1 server sends digest for shelve -a leaveunchanged
	// 2017.2 server sends sha1 for submit and digestType

	if( digestType )
	{
	    FileDigestType digType;
	    digType = clientFileDigestType(digestType);

	    StrBuf localDigest;
	    f->ComputeDigest( digType, &localDigest, e );
	    if( !e->Test() && !localDigest.XCompare( *serverDigest ) )
	    {
	        client->SetVar( P4Tag::v_status, "same" );
	        client->SetVar( P4Tag::v_sha, &localDigest );
	        client->Confirm( confirm );
	        // We differ from classic here as
	        // with 'submitunchanged' in the submitoptions set we
	        // don't send a digest.
	        if( !e->Test() && perms && revertUnchanged )
	        {
	            if( depotTime && ( f->Stat() & FSF_WRITEABLE ) )
	            {
	                // Refresh modtime from depot rev, the same
	                // way clientChmodFile() does

	                f->ModTime( depotTime );
	                f->ChmodTime( e );
	            }

	            if( !e->Test() )
	                f->Chmod2( perms->Text(), e );
	        }
	        delete f;
	        return;
	    }
	}
	else if( serverDigest || pendingDigest )
	{
	    StrBuf localDigest;
	    f->Translator( ClientSvc::XCharset( client, FromClient ) );
	    f->Digest( &localDigest, e );

	    if( !e->Test() && ( ( serverDigest &&
				  !localDigest.XCompare( *serverDigest ) ) ||
			        ( pendingDigest &&
				  !localDigest.XCompare( *pendingDigest ) ) ) )
	    {
	        client->SetVar( P4Tag::v_status, "same" );
	        client->SetVar( P4Tag::v_digest, &localDigest );
	        client->Confirm( confirm );
	        if( !e->Test() && perms && revertUnchanged )
		{
		    if( depotTime && ( f->Stat() & FSF_WRITEABLE ) )
		    {
			// Refresh modtime from depot rev, the same
			// way clientChmodFile() does

			f->ModTime( depotTime );
			f->ChmodTime( e );
		    }

		    if( !e->Test() )
			f->Chmod2( perms->Text(), e );
		}
	        delete f;
	        return;
	    }
	}

	// pre-2003.2 server, send modTime on open

	if( modTime && !sendDigest )
	    client->SetVar( P4Tag::v_time, modTime );

	// open source file, even if file open fails.

	f->Open( FOM_READ, e );

	if( !e->Test() )
	{
	    // This is just an estimate. The actual file size is computed below

	    filesize = f->GetSize();
	    client->SetVar( P4Tag::v_fileSize, StrNum( filesize ) );
	}

	// Send open (with modTime, and possibly estimated file size)
	++client->sendClientTotal;
	client->Confirm( open );

	ProgressReport *progress = NULL;

# ifdef USE_CDC
	const int minSize = p4tunable.Get( P4TUNE_NET_DELTA_TRANSFER_MINSIZE );
	const P4INT64 cdcThreshold = p4tunable.Get( P4TUNE_NET_DELTA_TRANSFER_THRESHOLD );

	// Declare variables before first goto
	bool doChunkingTransfer = chunkWrite && chunkMapWrite &&
	    minSize && filesize >= minSize && cdcThreshold;

	ChunkMap cm;

	// If the chunk map was sent as a handle but failed, fallback
	StrPtr *cmHandle;
	if( doChunkingTransfer &&
	    ( cmHandle = client->GetVar( P4Tag::v_chunkMapHandle ) ) )
	{
	    ClientVarHandle *smVar =
	        (ClientVarHandle *)client->handles.Get( cmHandle, e );
	    if( e->Test() )
	        goto bail;

	    if( smVar->IsError() )
	        doChunkingTransfer = false;
	}
	else if( doChunkingTransfer && !client->GetVar( P4Tag::v_chunkMap ) )
	    doChunkingTransfer = false;

	// If the file doesn't qualify for CDC skip the chunkMap generation
	if( ( chunkMapWrite && filesize < minSize ) || !cdcThreshold )
	{
	    chunkMapWrite = 0;
	    doChunkingTransfer = false;
	}
# else
	const bool doChunkingTransfer = false;
# endif

	if( e->Test() ) 
	    goto bail;

	f->Translator( ClientSvc::XCharset( client, FromClient ) );

	// send data, as long as no rpc error

# ifdef USE_CDC
	if( chunkMapWrite )
	{
	    clientSendFileChunkMap( client, &progress, f, &md5, filesize, cm,
	                            chunkMapWrite, e );
	    if( e->Test() )
	        goto bail;
	}

	// If the file qualifies for chunked transfer and we have computed a
	// chunkMap and we have a prior chunkMap, only send the deltas

	if( doChunkingTransfer )
	{
	    VVarArray *dm = clientGetChunksToSend( client, cm, e );
	    P4INT64 nTotalChunks = cm.ChunkCount();
	    P4INT64 nChunksToSend = dm ? dm->Count() : nTotalChunks;

	    if( cm.BelowThreshold( nChunksToSend ) )
	    {
	        clientSendFileChunked( client, &progress, f, filesize, dm,
	                               clientPath, handle, chunkWrite, e );
	        // The on-disk size doesn't lie in the binary+F case.
	        len = filesize;
	    }
	    else
	        doChunkingTransfer = false;

	    delete dm;
	}

	if( !doChunkingTransfer )
# endif // USE_CDC
	    clientSendFileWhole( client, &progress, f,
	                         chunkMapWrite ? NULL : &md5, len, filesize,
	                         sendDigest, handle, write, clientPath, e );

	f->Close( e );

	// Chmod according to perms.
	// 2000.1 server does this to avoid a separate 
	// client-ChmodFile call.
	// 2007.2 server will send reopen flag, because it had to
	// send the permissions bit for revertUnchanged.

	if( !e->Test() && perms && !reopen )
	    f->Chmod2( perms->Text(), e );
	else if( !e->Test() && chmodReopen && reopen )
	    f->Chmod2( FPM_RW, e );

    bail:
	// close it down
	// On any failure we'll send the decline so that the server
	// knows something went wrong.

	delete f;
	delete progress;

	if( sendDigest )
	{
	    // 2005.1 server, send filesize

	    if( sendFileSize )
	        client->SetVar( P4Tag::v_fileSize, StrNum( len ) );

	    // 2003.2 server, send digest and modTime together

	    md5.Final( digest );
	    client->SetVar( P4Tag::v_digest, &digest );

	    if( modTime )
	        client->SetVar( P4Tag::v_time, modTime );
	}

	if( doChunkingTransfer )
	{
	    client->SetVar( P4Tag::v_chunking );
	    if( index )
	        client->SetVar( P4Tag::v_index, index );
	}

	client->Confirm( e->Test() ? decline : confirm );

	// Report non-fatal error and clear it.

	client->OutputError( e );
}

// Spec support

void
clientEditData( Client *client, Error *e )
{
	StrPtr *spec = client->GetVar( P4Tag::v_data, e );
	StrPtr *confirm = client->GetVar( P4Tag::v_confirm );
	StrPtr *decline = client->GetVar( P4Tag::v_decline );
	StrPtr *compare = client->GetVar( P4Tag::v_compare );
	StrBuf newSpec;

	if( e->Test() )
	    return;

	// this does the work of FileSys::CreateGlobalTemp but
	// uses the client user object's FileSys create method
	FileSys *f = client->GetUi()->File( FST_UNICODE );
	f->SetContentCharSetPriv( client->content_charset );
	f->SetDeleteOnClose();
	f->MakeGlobalTemp();

	if( confirm )
	    f->Perms( FPM_RWO );

	/* Set different translators between write/read */
	f->Translator( client->fromTransDialog );

	if( !e->Test() )	f->WriteFile( spec, e );
	if( !e->Test() )	client->GetUi()->Edit( f, e );

	f->Translator( client->toTransDialog );

	if( !e->Test() )	f->ReadFile( &newSpec, e );

	delete f;

	if( e->Test() )
	    confirm = decline;

	// send the confirmation

	if( confirm )
	{
	    // If asked to compare old vs new, do so.

	    if( compare )
		client->SetVar( P4Tag::v_compare, newSpec == *spec ? "same" : "diff" );

	    client->SetVar( P4Tag::v_data, &newSpec );

	    client->Confirm( confirm );
	}

	// Report non-fatal error and clear it.

	client->OutputError( e );
}

void
clientInputData( Client *client, Error *e )
{
	client->NewHandler();
	StrPtr *confirm = client->GetVar( P4Tag::v_confirm, e );

	if( e->Test() )
	    return;

	StrBuf newSpec;

	client->GetUi()->InputData( &newSpec, e );

	// send the confirmation

	client->translated->SetVar( P4Tag::v_data, &newSpec );

	client->Confirm( confirm );
}


//
// clientPrompt
// clientErrorPause
// clientHandleError
// clientOutputError
// clientOutputText
// clientOutputInfo
// clientOutputData
// 

void
clientPrompt( Client *client, Error *e )
{
	client->FstatPartialClear();
	client->NewHandler();
	Error e1;
	Error rcvErr;
	StrBuf resp;

	StrPtr *data = client->translated->GetVar( P4Tag::v_data, &e1 );
	StrPtr *confirm = client->GetVar( P4Tag::v_confirm, e );
	StrPtr *truncate = client->GetVar( P4Tag::v_truncate );
	StrPtr *noecho = client->GetVar( P4Tag::v_noecho );
	StrPtr *noprompt = client->GetVar( P4Tag::v_noprompt );
	StrPtr *digest = client->GetVar( P4Tag::v_digest );
	StrPtr *mangle = client->GetVar( P4Tag::v_mangle );
	StrPtr *user = client->GetVar( P4Tag::v_user );

	StrDict *errorDict = client;
	if( !e->Test() && !data )
	{
	    if( client != client->translated )
	        errorDict = ((TransDict *)client->translated)
	            ->CreateErrorOutputDict();
	    rcvErr.UnMarshall1( *errorDict );
	}

	if( !e->Test() && !rcvErr.GetErrorCount() && e1.Test() )
	    *e = e1;

	if( e->Test() )
	{
	    if ( !e->IsFatal() )
		client->OutputError( e );
	    if( client != errorDict )
	        delete errorDict;
	    return;
	}

	if( noprompt )
	    resp = *client->GetPBuf();
	else if( data )
	    client->GetUi()->Prompt( *data, resp, noecho != 0, e );
	else
	    client->GetUi()->Prompt( &rcvErr, resp, noecho != 0, e );

	client->SetPBuf( resp );

	if( e->Test() )
	{
	    if( client != errorDict )
	        delete errorDict;
	    return;
	}

	// if digest set, we'll send the md5 of the response
	// otherwise, we'll send the raw thing.
	// The server accepts an md5 of the password in leiu
	// of the actual thing.

	// New to 2003.2:
	//
	// Depending on the server security level the server can
	// set the mangle variable to indicate that the return value
	// should be encrypted and not hashed.
	//
	// For added security when asked for the old password the
	// digest string will now contain a token which the password
	// md5 should be md5'd with.
	// 
	// Currently unicode clients will not be able to have their
	// password complexity checked.  Because of limitations on the
	// mangle algorithm input to digest or mangle will be truncated
	// to 16 characters.
	// 
	// note: scrambling the ticket does not provide robust security.
	// Sites with strong security requirement should use SSL support
	// added in 2012.1

	if( ( mangle || digest ) && resp.Length() )
	{
	    MD5 md5;
# ifdef USE_EBCDIC
	    __etoa_l( data->Text(), data->Length() );
	    __etoa_l( resp.Text(), resp.Length() );
# endif
	    if( client != client->translated )
	    {
		int newl = 0;
		const char *t = ((TransDict *)client->translated)->ToCvt()->
		    FastCvt( resp.Text(), resp.Length(), &newl );
		if (t)
		    resp.Set(t, newl);
	    }

	    // truncate to 16 characters

	    if( truncate && resp.Length() > 16 )
	        resp.SetLength( 16 );

	    if( digest )
	    {
	        md5.Update( resp );
	        md5.Final( resp );
#ifdef USE_EBCDIC
		// Back to ascii for redigestion.
		__etoa_l( resp.Text(), resp.Length() );
#endif

		// 2005.2 server can use the oldpassword MD5 as part of
		// the encryption key, only the client and server will
		// know this.

		if( client->protocolServer >= 20 )
		    client->SetSecretKey( resp );

	        // 2003.2 server will send a hashed token to avoid
	        // putting on the wire a string that can be used by
	        // an old client to authenticate.

	        if( digest->Length() )
	        {
	            MD5 md5;

#ifdef USE_EBCDIC
		    // To ASCII for digesting
		    __etoa_l( digest->Text(), digest->Length() );
#endif
	            md5.Update( resp );
	            md5.Update( *digest );
	            md5.Final( resp );
#ifdef USE_EBCDIC
		    // XXX: The digest is in ASCII, but when it gets sent back 
		    // to the server, we need it in EBCDIC so that the RPC 
		    // layer can convert it back to ASCII.
		    __atoe_l( digest->Text(), digest->Length() );
#endif
	        }

		StrPtr *daddr = client->GetPeerAddress( RAF_PORT );

		if( daddr )
		{
		    client->SetVar( P4Tag::v_daddr, *daddr );

		    if( client->protocolServer >= 29 )
		    {
			MD5 md5;

			if( daddr )
			{
			    md5.Update( resp );
			    md5.Update( *daddr );
			    md5.Final( resp );
			}
		    }
		}

	        client->SetVar( P4Tag::v_data, resp );
	    }
	    else
	    {
	        // Create a key which the server can duplicate to unmangle
	        // the password:

	        Mangle m;
	        StrBuf key;                
		StrPtr *secretKey=0;
	        StrPtr *data = &resp;

		if( client->protocolServer >= 20 )
		    secretKey = client->GetSecretKey();

#ifdef USE_EBCDIC
		__etoa_l( mangle->Text(), mangle->Length() );
		__etoa_l( user->Text(), user->Length() );
#endif
	        md5.Update( *mangle );               
		md5.Update( *user );     

#ifdef USE_EBCDIC
		__atoe_l( mangle->Text(), mangle->Length() );
		__atoe_l( user->Text(), user->Length() );
#endif
		// 2005.2 add salt to the key

		if( secretKey && secretKey->Length() )
		{
		    md5.Update( *secretKey );
		    if( client->GetVar( P4Tag::v_data2 ) )
		        client->ClearSecretKey();
		}

		md5.Final( key );                    
#ifdef USE_EBCDIC
		__etoa_l( key.Text(), key.Length() );
#endif
	        mangle = &key;

	        m.In( *data, *mangle, key, e );     

	        if( e->Test() )
	        {
	            if( client != errorDict )
	                delete errorDict;
	            return;
	        }

	        client->SetVar( P4Tag::v_data, key );
	    }
	}
	else
	{
	    // Drop support for really old password/login exchange

	    StrBuf promptStr;
	    if( data )
	        promptStr << *data;
	    else
	        rcvErr.Fmt( promptStr, 0 );
	    StrOps::Lower( promptStr );

	    if( resp.Length() && 
	      ( noecho || promptStr.Contains( StrRef( "pass" ) ) ) )
	    {
	        MD5 md5;
	        md5.Update( resp );
	        md5.Final( resp );
	    }

	    client->translated->SetVar( P4Tag::v_data, resp );
	}

	if( client != errorDict )
            delete errorDict;

	client->Confirm( confirm );
}

void
clientSyncTrigger( Client *client, Error *e )
{
	client->GetVar( "zerosync", e );

	if( e->Test() )
	{
	    if ( !e->IsFatal() )
		client->OutputError( e );
	    return;
	}

# ifdef HAS_EXTENSIONS

	ClientScript* exts = client->GetExtensions();

	ClientScriptAction r = ClientScriptAction::FAIL;
	int nRun = 0;

	std::tie( r, nRun ) = client->ExtensionsEnabled()
	    ? exts->Run( "zeroSync", nullptr, client->GetUi(),
	                 true, e )
	    : std::tuple< ClientScriptAction, int >
	      ( ClientScriptAction::PASS, 0 );

	if( e->Test() || r == ClientScriptAction::FAIL )
	{
	    if( !e->IsFatal() )
	        client->OutputError( e );
	    return;
	}

	// If there weren't any Extensions that handled this, fallback to
	// a trigger.

	if( nRun > 0 )
	    return;

# endif

	const StrPtr *syncTrigger = &client->GetSyncTrigger();

	if( !strcmp( syncTrigger->Text(), "unset" ) )
	    return;

	// execute syncUpdater command   (shiv trigger)

	RunCommandIo *rc = new RunCommandIo;
	StrBuf result;

	RunArgs cmd;
	StrOps::Expand( cmd.SetBuf(), *syncTrigger, *client );

	rc->Run( cmd, result, e );
	delete rc;
}


void
clientSingleSignon( Client *client, Error *e )
{
	StrPtr *confirm = client->GetVar( P4Tag::v_confirm, e );

	if( e->Test() )
	{
	    if ( !e->IsFatal() )
		client->OutputError( e );
	    return;
	}

	StrBufDict SSODict;
	StrRef var, val;
#ifdef HAS_EXTENSIONS
	std::unordered_map< std::string, std::string > ssodict;
#endif

	for( int i = 0; client->GetVar( i, var, val ); i++ )
	{
	    if( var == P4Tag::v_func   ||
	        var == P4Tag::v_func2  ||
	        var == P4Tag::v_state  ||
	        var == P4Tag::v_confirm )
	        continue;

	    SSODict.SetVar( var, val );
#ifdef HAS_EXTENSIONS
	    ssodict[ std::string( var.Text() ) ] = std::string( val.Text() );
#endif
	}

	// Allow the trigger to know if it's talking through a broker or
	// proxy since the serverAddress will be different in this case.
	
	SSODict.SetVar( "P4PORT", client->GetPort() );

	if ( !SSODict.GetVar( "ssoArgs" ) )
	    SSODict.SetVar( "ssoArgs", "" );
	if ( !SSODict.GetVar( "data" ) )
	    SSODict.SetVar( "data", "" );

	ClientSSO *handler = 0;
	if( ( handler = client->GetUi()->GetSSOHandler() ) )
	{
	    StrBuf resBuf;

	    ClientSSOStatus result = handler->Authorize( SSODict,
	                                                 SSOMAXLENGTH,
	                                                 resBuf );

	    // Backwards server compatibility

	    if( client->protocolServer < 46 && result == CSS_EXIT)
	    {
	        result = CSS_FAIL;
	        resBuf = "Client aborted SSO";
	    }

	    switch( result )
	    {
	    case CSS_PASS:  client->SetVar( P4Tag::v_status, "pass"  ); break;
	    case CSS_FAIL:  client->SetVar( P4Tag::v_status, "fail"  ); break;
	    case CSS_EXIT:  client->SetVar( P4Tag::v_status, "exit"  ); break;
	    case CSS_UNSET: client->SetVar( P4Tag::v_status, "unset" ); break;
	    case CSS_SKIP:  goto BuiltInSSO;
	    }

	    if( result == CSS_EXIT || result == CSS_UNSET)
	    {
	        client->SetVar( P4Tag::v_sso );
	        client->Confirm( confirm );
	        return;
	    }

	    // truncate to 128K max length

	    if( resBuf.Length() > SSOMAXLENGTH )
	    {
	        resBuf.SetLength( SSOMAXLENGTH );
	        resBuf.Terminate();
	    }

	    client->SetVar( P4Tag::v_sso, resBuf );

	    client->Confirm( confirm );
	    return;
	}

BuiltInSSO:

	const StrPtr *loginSSO = &client->GetLoginSSO();

	if( !strcmp( loginSSO->Text(), "unset" ) )
	{
	    client->SetVar( P4Tag::v_status, "unset" );
	    client->SetVar( P4Tag::v_sso );
	    client->Confirm( confirm );
	    return;
	}

	// Execute P4LOGINSSO command - first see if it's an Extension,
	// then fallback to a trigger.

# ifdef HAS_EXTENSIONS

	if( client->ExtensionsEnabled() )
	{
	    StrBuf cmdBuffer;
	    char* words[ 50 ];
	    int nWords = 0;

	    nWords = StrOps::WordsQ( cmdBuffer, loginSSO->Text(), words,
	        ( sizeof( words ) / sizeof( words[ 0 ] ) ), e );

	    if( !nWords )
	    {
	        client->SetVar( P4Tag::v_status, "fail" );
	        client->SetVar( P4Tag::v_sso, "Bad P4EXTENSION:  no words" );
	        client->Confirm( confirm );
	        return;
	    }

	    int argc = nWords;
	    std::vector< std::string > argv;

	    for( int i = 1; i < nWords; i++ )
	        argv.emplace_back( std::string( words[ i ] ) );

	    ClientScript exts( client );

	    exts.SetSearchPattern( words[ 0 ] );
	    exts.LoadScripts( false, e );

	    if( e->Test() )
	        return;

	    // We'll have a match if the file pointed to P4LOGINSSO matches
	    // a known script type.

	    auto& extList = exts.GetExts();

	    if( extList.size() )
	    {
	        auto& ext = exts.GetExts()[ 0 ];
	        StrBuf data = *SSODict.GetVar( "data" ), result;

	        const bool ret = ((ExtensionCallerDataC*)(*ext).GetECD())
	            ->loginSSO( data, result, argc, std::move( argv ),
	                    std::move( ssodict ), e );

	        const char* status = ( ret || !e->Test() ) ? "pass" : "fail";
	        client->SetVar( P4Tag::v_status, status );

	        if( result.Length() > SSOMAXLENGTH )
	        {
	            result.SetLength( SSOMAXLENGTH );
	            result.Terminate();
	        }

	        client->SetVar( P4Tag::v_sso, result );
	        client->Confirm( confirm );
	        return;
	    }
	}
#endif

	// Extensions not supported, enabled or run - use a trigger.

	RunCommandIo rc;
	StrBuf result;
	RunArgs cmd;
	StrBufDict fields;
	StrPtr *data;
	StrBuf input;

	// data may be specified as an placeholder or sent to stdin
	StrOps::Expand(cmd.SetBuf(), *loginSSO, fields, &fields);
	if( !fields.GetVar( "data" ) &&
	    ( data = SSODict.GetVar( "data" ) ) )
	{
	    input = *data;
	    SSODict.RemoveVar( "data" );
	}

	StrOps::Expand( cmd.SetBuf(), *loginSSO, SSODict );

	if( rc.Run( cmd, input, result, e ) || e->Test() )
	    client->SetVar( P4Tag::v_status, "fail" );
	else
	    client->SetVar( P4Tag::v_status, "pass" );

	// truncate to 128K max length

	if( result.Length() > SSOMAXLENGTH )
	{
	    result.SetLength( SSOMAXLENGTH );
	    result.Terminate();
	}

	client->SetVar( P4Tag::v_sso, result );
	client->Confirm( confirm );
}

void
clientErrorPause( Client *client, Error *e )
{
	client->FstatPartialClear();
	client->NewHandler();
	StrPtr *data = client->translated->GetVar( P4Tag::v_data, e );

	if( e->Test() )
	    return;

	client->GetUi()->ErrorPause( data->Text(), e );
}

void
clientHandleError( Client *client, Error *e )
{
	client->FstatPartialClear();
	client->NewHandler();
	StrPtr *data = client->translated->GetVar( P4Tag::v_data, e );

	if( e->Test() )
	{
	    if ( !e->IsFatal() )
		client->OutputError( e );
	    return;
	}

	// Unmarshall error object

	Error rcvErr;

	rcvErr.UnMarshall0( *data );

	if( rcvErr.IsError() )
	    client->SetError();

	client->GetUi()->HandleError( &rcvErr );
	client->ClearSecretKey();
	client->ClearPBuf();
}

void
clientMessage( Client *client, Error *e )
{
	client->FstatPartialClear();
	client->NewHandler();

	/* Unpack the error and hand it to the user. */

	Error rcvErr;
	StrDict *errorDict = client;
	if( client != client->translated )
	{
	    errorDict = ((TransDict *)client->translated)
		->CreateErrorOutputDict();
	}
	rcvErr.UnMarshall1( *errorDict );

	if( rcvErr.IsError() )
	    client->SetError();

	client->GetUi()->Message( &rcvErr );

	if( rcvErr.Test() )
	{
	    client->ClearSecretKey();
	    client->ClearPBuf();
	}

	// MsgDm::DomainSave
	ErrorId clientUpdate = { ErrorOf( 6, 6370, 1, 0, 2 ), "" };

	// zerosync client shiv trigger

	if( rcvErr.CheckId( clientUpdate ) )
	{
	    StrPtr *syncClient = client->GetVar( "zerosync" );

	    if( syncClient )
	    {
	        Error te;  //ignore error
	        clientSyncTrigger( client, &te );

	        if( te.Test() )
	            client->GetUi()->Message( &te );
	    }
	}

	if( errorDict != client )
	    delete errorDict;
}

void
clientOutputError( Client *client, Error *e )
{
	client->FstatPartialClear();
	client->NewHandler();
	StrPtr *data = client->translated->GetVar( P4Tag::v_data, e );
	StrPtr *warn = client->GetVar( P4Tag::v_warning );

	// Note error so we can adjust exit()

	if( !warn )
	    client->SetError();

	if( e->Test() )
	{
	    if ( !e->IsFatal() )
		client->OutputError( e );
	    return;
	}

	client->GetUi()->OutputError( data->Text() );
}

void
clientOutputInfo( Client *client, Error *e )
{
	client->FstatPartialClear();
	client->NewHandler();
	StrPtr *data = client->translated->GetVar( P4Tag::v_data, e );
	StrPtr *level = client->GetVar( P4Tag::v_level );
	int lev = level ? *level->Text() : '0';

	if( e->Test() )
	{
	    if ( !e->IsFatal() )
		client->OutputError( e );
	    return;
	}

	client->GetUi()->OutputInfo( lev, data->Text() );
}

void
clientOutputText( Client *client, Error *e )
{
	client->FstatPartialClear();
	client->NewHandler();
	StrPtr *trans = client->GetVar( P4Tag::v_trans );
	StrPtr *data;

	if ( trans && *trans == "no" )
	    data = client->GetVar( P4Tag::v_data, e );
	else
	    data = client->translated->GetVar( P4Tag::v_data, e );

	if( e->Test() )
	{
	    if ( !e->IsFatal() )
		client->OutputError( e );
	    return;
	}

	client->GetUi()->OutputText( data->Text(), data->Length() );
}

void
clientOutputBinary( Client *client, Error *e )
{
	client->FstatPartialClear();
	StrPtr *data = client->GetVar( P4Tag::v_data, e );

	if( e->Test() )
	    return;

	client->GetUi()->OutputBinary( data->Text(), data->Length() );
}


void
clientProgress( Client *client, Error *e )
{
	client->NewHandler();

	StrPtr *handle = client->GetVar( P4Tag::v_handle, e );

	if( e->Test() )
	    return;

	ProgressHandle *progh = (ProgressHandle *)client->handles.Get( handle );
	ClientProgress *prog;
	StrPtr *val;

	if( progh )
	    prog = progh->progress;
	else
	{
	    val = client->GetVar( "type", e );

	    if( e->Test() )
	        return;

	    prog = client->GetUi()->CreateProgress( val->Atoi() );
	    if( !prog )
		return;
	}

	val = client->GetVar( "desc" );
	if( val )
	    prog->Description( val, client->GetVar( "units" )->Atoi() );

	val = client->GetVar( "total" );
	if( val )
	    prog->Total( val->Atoi() );

	val = client->GetVar( "update" );
	if( val )
	    prog->Update( val->Atoi() ); // cancel error XXX

	val = client->GetVar( "done" );
	if( val )
	{
	    prog->Done( val->Atoi() ? CPP_FAILDONE : CPP_DONE );
	    if( progh )
		delete progh;
	    else
		delete prog;
	}
	else if( !progh )
	    client->handles.Install( handle, new ProgressHandle( prog ), e );
}

//
// clientFstatInfo
// clientFstatPartial
//

void
clientFstatInfo( Client *client, Error *e )
{
	// Rpc has a StrDict interface
	client->NewHandler();

	// XXX hmmm... since we potentially have different translations
	// which one should we choose

	// Append the final fstat partial to the existing partials
	client->FstatPartialAppend( client->translated );

	client->GetUi()->OutputStat( client->fstatPartial );
	client->FstatPartialClear();
}

void
clientFstatPartial( Client *client, Error *e )
{
	// Rpc has a StrDict interface
	client->NewHandler();

	// XXX hmmm... since we potentially have different translations
	// which one should we choose

	// Append the partial to the existing partials
	client->FstatPartialAppend( client->translated );

	if( client->GetUi()->OutputStatPartial( client->fstatPartial ) )
	    client->FstatPartialClear();
}

//
// clientAck
//

void
clientAck( Client *client, Error *e )
{
	StrPtr *confirm = client->GetVar( P4Tag::v_confirm, e );
	StrPtr *decline = client->GetVar( P4Tag::v_decline );
	StrPtr *handle = client->GetVar( P4Tag::v_handle ); 

	if( e->Test() )
	    return;

	// Ack decline if handle indicates failure.
	// Ack confirm if no handle or handle shows success.

	if( handle && client->handles.AnyErrors( handle ) )
	{
	    confirm = decline;
	}
	else
	{
	    // no errors, if syncTime is set, send it

	    if( client->GetSyncTime() )
	        client->SetVar( "syncTime", client->GetSyncTime() );
	}

	// clear syncTime

	client->SetSyncTime( 0 );

	if( confirm )
	    client->Confirm( confirm );
}


//
void
clientPing( Client *client, Error *e )
{
	StrPtr *payloadSize = client->GetVar( P4Tag::v_fileSize);
	StrPtr *timer = client->GetVar( P4Tag::v_time );
	StrPtr *clientMessageBuf = client->GetVar( P4Tag::v_fileSize );
	StrPtr *serverMessageBuf = client->GetVar( P4Tag::v_value );
        StrPtr *acksBuf = client->GetVar( P4Tag::v_blockCount );
        StrPtr *secsBuf = client->GetVar( P4Tag::v_token );
        StrPtr *taggedFlag = client->GetVar( P4Tag::v_tag );

	if( e->Test() )
	    return;

	if ( payloadSize ) 
	{
	    int size = payloadSize->Atoi();
	    if( size > 1000000 )
	        size = 1000000;

	    StrBuf sbuf;
	    sbuf.Alloc( size );
	    sbuf.Fill( "b" );
	    sbuf.Terminate();
	    client->SetVar( P4Tag::v_desc, sbuf );
	}
	client->SetVar( P4Tag::v_fileSize, clientMessageBuf );
	client->SetVar( P4Tag::v_value, serverMessageBuf );
	client->SetVar( P4Tag::v_blockCount, acksBuf );
	client->SetVar( P4Tag::v_token, secsBuf );
	client->SetVar( P4Tag::v_tag, taggedFlag );
	if ( timer )
	    client->SetVar(  P4Tag::v_time, timer );
	client->Invoke( "dm-Ping" );
}


//
// clientCrypto - encrypt the string with the user's password
//

void
clientCrypto( Client *client, Error *e )
{
	StrPtr *confirm = client->GetVar( P4Tag::v_confirm, e );
	StrPtr *token = client->GetVar( P4Tag::v_token, e );
	StrPtr *truncate = client->GetVar( P4Tag::v_truncate );
	StrPtr *serverID = client->GetVar( P4Tag::v_serverAddress );
	StrPtr *usrName = client->GetVar( P4Tag::v_user );

	// Get these from the extra variable stash
	StrPtr *ipAddr = client->GetEVar( P4Tag::v_ipaddr );
	StrPtr *svrName = client->GetEVar( P4Tag::v_svrname );
	StrPtr *svcPass = client->GetEVar( P4Tag::v_password );
	StrPtr *svrPort = client->GetEVar( P4Tag::v_port );
	StrPtr *daddr = svrPort;
	StrPtr *sid = client->GetEVar( P4Tag::v_serverID );
	StrPtr *stype = client->GetEVar( P4Tag::v_serverType );
	StrPtr *sver = client->GetEVar( P4Tag::v_serverVersion );

	if( e->Test() )
	    return;

	int pxAuth = ( ipAddr && svrName ) ? 1 : 0;

	StrBuf u;
	if( usrName )
	{
	    u = *usrName;
	    if( client->protocolNocase )
	        StrOps::Lower( u );
	}

	StrBuf s;
	if( svrName )
	{
	    s = *svrName;
	    if( client->protocolNocase )
	        StrOps::Lower( s );
	}

	// Set the normalized server address (2007.2) 

	client->SetServerID( serverID ? serverID->Text() : "" );

	if( !daddr || !pxAuth )
	    daddr = client->GetPeerAddress( RAF_PORT );
	if( daddr )
	    client->SetVar( P4Tag::v_daddr, *daddr );

	// If no password, just send back an empty string
	// If a password, send the digest of the token and digested password.
	// Yes, the password itself must be digested first.

	StrBuf result;
	const StrPtr &password  = client->GetPassword( usrName ? &u : 0 );
	const StrPtr &password2 = client->GetPassword2();

	if( pxAuth )
	    client->SetVar( P4Tag::v_caddr, *ipAddr );

	if( !password.Length() )
	{
	    client->SetVar( P4Tag::v_token, &result );
	    if( !pxAuth )
	    {
	        client->Invoke( confirm->Text() );
	        return;
	    }
	}

	// send 2 passwords (token,token2) if both ticket and P4PASSWD
	// are set (2007.2).

	int max = !password.Length() ? 0 :
	          password2.Length() && password != password2 ? 2 : 1;

	for( int i = 0; i < max; ++i )
	{
	    // Digest of password
	    // If password already digested, use it straight.

	    result = ( i == 0 ) ? password : password2;

	    if( !StrOps::IsDigest( result ) )
	    {
		MD5 md5;
# ifdef USE_EBCDIC
		__etoa_l( result.Text(), result.Length() );
# endif
		if( client != client->translated )
		{
		    int newl = 0;
		    const char *t = ((TransDict *)client->translated)->
			ToCvt()->FastCvt( result.Text(),
					  result.Length(), &newl );
		    if (t)
			result.Set(t, newl);
		}
		else
		{
		    // New to 2003.2, truncate password to 16 characters,
		    // if password was strength checked.

		    if( truncate && result.Length() > 16 )
		        result.SetLength( 16 );
		}

		md5.Update( result );
		md5.Final( result );
	    }

	    // Digest of token and digested password.

	    {
		MD5 md5;

# ifdef USE_EBCDIC
		__etoa_l( token->Text(), token->Length() );
		__etoa_l( result.Text(), result.Length() );
# endif

		md5.Update( *token );
		md5.Update( result );
		md5.Final( result );
	    }

	    if( client->protocolServer >= 29 )
	    {
		// 2010.1 or later...

		if( daddr )
		{
		    MD5 md5;

		    md5.Update( result );
		    md5.Update( *daddr );
		    md5.Final( result );
		}
	    }

	    if( i == 0 ) client->SetVar( P4Tag::v_token, &result );
	            else client->SetVar( P4Tag::v_token2, &result );
	}

	if( pxAuth )
	{
	    const StrPtr &svcTicket = svcPass ? *svcPass
	                          : client->GetPassword( svrName ? &s : 0, 1 );
	    StrPtr *daddr0 = client->GetPeerAddress( RAF_PORT );
	    if( daddr0 )
	    {
		StrBuf phash;
		MD5 md5;
		if( svrName->Length() )
		{
		    md5.Update( *svrName );
		    client->SetVar( P4Tag::v_svrname, 0, *svrName );
		}
		if( svcTicket.Length() )
		    md5.Update( svcTicket );
		md5.Update( *token );

		md5.Update( *daddr0 );
		client->SetVar( P4Tag::v_daddr, 0, *daddr0 );

		md5.Final( phash );
		client->SetVar( P4Tag::v_dhash, 0, phash );

		if( stype )
		    client->SetVar( P4Tag::v_serverType, 0, *stype );
		if( sid )
		    client->SetVar( P4Tag::v_serverID, 0, *sid );
		if( sver )
		    client->SetVar( P4Tag::v_serverVersion, 0, *sver );
		if( svrPort )
		    client->SetVar( P4Tag::v_laddr, *svrPort );
	    }
	}

	client->Invoke( confirm->Text() );
}

//
// clientSetPassword - set the password on the client
//

void
clientSetPassword( Client *client, Error *e )
{
	client->NewHandler();
	StrPtr *data = client->GetVar( P4Tag::v_data, e );
	StrPtr *serverID = client->GetVar( P4Tag::v_serverAddress );
	StrPtr *noprompt = client->GetVar( P4Tag::v_noprompt );

	if( e->Test() )
	    return;

	// In 2004.1 'p4 login' will also send the username along
	// this is required when a user is impersonating another 
	// through the P4USER or -u options.  Since this has come
	// from the server and is used as a key to replace/delete 
	// a ticket in the ticketfile, we must translate it.

	StrPtr *user = client->GetVar( P4Tag::v_user );
	int sameUser = !user || !user->Compare( client->GetUser() );

	// DVCS can change the user on the fly, so if that happens, accept it

	if( client->GetVar( P4Tag::v_userChanged ) )
	    sameUser = 1;

	// In 2003.2 the new command 'p4 login' also calls
	// SetPassword,  check to see if v_data2 has been set
	// to show that this is a callback from 'login' and 
	// not password.

	StrPtr *data2 = client->GetVar( P4Tag::v_data2 );

	// Its possible they have a 3.2 server and 4.1 client, instead
	// of spitting out the instructions on how to set configuration
	// environment we use "******" to represent any user.

	StrRef userWild( "******" );
	StrBuf ticket;
	StrBuf u;

	// In 2010.2 tickets are XOR'd with encrypted tokens

	StrPtr *token = client->GetVar( P4Tag::v_digest ); 

	if( token )
	{
	    StrBuf secretKey;
	    StrBuf token2;
	    Mangle sk;

	    if( sameUser && client->GetSecretKey()->Length())
	    {
	        secretKey << client->GetSecretKey();
	    }
	    else
	    {
	        secretKey << client->GetPassword();
	        if( !StrOps::IsDigest( secretKey ) )
	        {
	            MD5 md5;
		    md5.Update( secretKey );
		    md5.Final( secretKey );
	        }
	    }

	    ticket << data;

	    sk.InMD5( *token, secretKey, token2, e );
	    sk.XOR( ticket, token2, e );

	    if( e->Test() )
	        return;

	    data = &ticket;
	}

	// Clear secret key - cms bug

	client->ClearSecretKey();

	if( noprompt )
	    client->ClearPBuf();

	if( client->GetVar( P4Tag::v_output ) ) // print
	{
	    Error msg;
	    msg.Set( MsgClient::LoginPrintTicket ) << data;
	    client->GetUi()->Message( &msg );
	    return;
	}

	// Downcase the username when setting/updating the ticket file
	// from a case insensitive server.

	if( user && client->protocolNocase )
	{
	    u = *user;
	    StrOps::Lower( u );
	    user = &u;
	}

	if( !user )
	    user = &userWild;

	// 2007.2 server sends the serverAddress as the identifier which
	// can be used as the key to find the ticket file entry.
	// If the server issues a "logout" then also clean up any other
	// entries for this server by also logging out with the old
	// key P4PORT.

	if( data2 && !strcmp( data2->Text(), "login" ) )
	{
	    Ticket t( &client->GetTicketFile() );
	    const StrPtr *port = serverID ? serverID : &client->GetPort();
	    t.ReplaceTicket( *port, *user, *data, e );
	    client->SetTicket( port, user, data );
	    return;
	}

	if( data2 && !strcmp( data2->Text(), "logout" ) )
	{
	    Ticket t( &client->GetTicketFile() );

	    if( serverID )
	    {
	        const StrPtr *port = serverID;
	        t.DeleteTicket( *port, *user, e );
	    }

	    if( !e->Test() )
	    {
		const StrPtr *port = &client->GetPort();
		t.DeleteTicket( *port, *user, e );
	    }
	    return;
	}

	// Pre 2004.1 server,  still support windows registry
	// stuff - (this does nothing on UNIX).

	client->DefinePassword( data->Text(), e );

	// Zap any registry errors
	e->Clear();
}

//
// clientProtocol - handle server's 'protocol' message
//

void
clientProtocol( Client *client, Error *e )
{
	StrPtr *s;

	if( ( s = client->GetVar( P4Tag::v_xfiles ) ) )
	    client->protocolXfiles = s->Atoi();

	if( ( s = client->GetVar( P4Tag::v_server2 ) ) || 
	    ( s = client->GetVar( P4Tag::v_server ) ) )
	    client->protocolServer = s->Atoi();

	if( ( s = client->GetVar( P4Tag::v_security ) ) )
	    client->protocolSecurity = s->Atoi();

	client->protocolNocase = client->GetVar( P4Tag::v_nocase ) != 0;

	client->protocolUnicode = client->GetVar( P4Tag::v_unicode ) != 0;

	if( ( s = client->GetVar( P4Tag::v_extensionsEnabled ) ) )
	    client->protocolClientExts = s->Atoi();
	else
	    client->protocolClientExts = 1;

	if( ( s = client->GetVar( P4Tag::v_clientStatsFunc ) ) )
	    client->statCallback.Set( s );
	else
	    client->statCallback.Clear();
}

//
// clientFatalError - handle protocol errors
// called when above functions return prematurely.
//

void
clientFatalError( Client *client, Error *e )
{
	// add our "fatal error" message
	// Note error so we can adjust exit()
	// Output it via the UI
	// release ourselves from Dispatch()
	// XXX Woe be to tagged calls: not thought through!

	e->Set( MsgClient::Fatal );
	client->SetError();
	client->SetFatal();
	client->GetUi()->HandleError( e );
	client->ClearSecretKey();
	client->ClearPBuf();
	client->GotReleased();
}

void
clientOpenUrl( Client *client, Error *e )
{
	StrPtr *url = client->GetVar( P4Tag::v_url, e );

	// This code path is only valid if we're really targeting a URL

	StrRef http( "http://" );
	StrRef https( "https://" );

	if( !e->Test() &&
	    ( http.SCompareN( *url ) &&
	      https.SCompareN( *url ) ) )
	    e->Set( MsgClient::InvalidUrl );

	if( e->Test() )
	{
	    if ( !e->IsFatal() )
	        client->OutputError( e );
	    return;
	}

	client->GetUi()->HandleUrl( url );
}

void
clientWriteVarPartial( Client *client, Error *e )
{
	const StrPtr* handle = client->GetVar( P4Tag::v_handle, e );
	const StrPtr* data = client->GetVar( P4Tag::v_data, e );
	const StrPtr* offset = client->GetVar( P4Tag::v_offset, e );
	const StrPtr* size = client->GetVar( P4Tag::v_size, e );
	const StrPtr* part = client->GetVar( P4Tag::v_sequence, e );
	const StrPtr* parts = client->GetVar( P4Tag::v_count, e );

	if( e->Test() )
	    return;

	ClientVarHandle *v = (ClientVarHandle *)client->handles.Get( handle );
	if( !v )
	{
	    v = new ClientVarHandle( size->Atoi64(), parts->Atoi(), e );
	    client->handles.Install( handle, v, e );
	}
	
	if( e->Test() )
	    return;

	v->Append( part->Atoi(), offset->Atoi64(), data, e );
}

static const RpcDispatch *
GetClientRpcFunction( StrPtr *func, Error *e )
{
	if( !func )
	    return 0;

	int i = 0;
	while( clientDispatch[i].opName )
	{
	    if( !strcmp( func->Text(), clientDispatch[i].opName ) )
	        return &clientDispatch[i];

	    i++;
	}

	e->Set( MsgRpc::UnReg ) << func;
	return 0;
}

void
clientAltSync( Client *client, Error *e )
{
	client->NewHandler();
	StrPtr *altSync = client->GetVar( P4Tag::v_altSync, e );
	StrPtr *confirm = client->GetVar( P4Tag::v_confirm );
	StrPtr *decline = client->GetVar( P4Tag::v_decline );
	StrPtr *asResults = client->GetVar( P4Tag::v_altSyncResults );
	const RpcDispatch *passFunc =
	    GetClientRpcFunction( client->GetVar( P4Tag::v_passFunc ), e );

	if( e->Test() )
	    return;

	ClientAltSyncHandler *handler =
	    ClientAltSyncHandler::GetAltSyncHandler( client, e );
	if( e->Test() )
	    return;

	if( !handler )
	{
	    client->SetVar( P4Tag::v_status, "unset" );
	    if( decline ? decline : confirm )
	        client->Confirm( decline ? decline : confirm );
	    return;
	}

	bool pass = false;
	StrBufTree results;
	if( handler->AltSync( e, asResults ? &results : 0, &pass ) ||
	    e->Test() )
	{
	    if( e->Test() )
	    {
	        client->GetUi()->HandleError( e );
	        e->Clear();
	    }
	    client->SetVar( P4Tag::v_status, "fail" );
	    return;
	}
	else
	{
	    // Handle passthrough cases
	    if( pass && passFunc )
	    {
	        (*passFunc->function)( client, e );
	        return;
	    }
	    if( pass )
	    {
	        e->Set( MsgClient::AltSyncUnhandledPass ) << altSync;
	        client->OutputError( e );
	        client->SetVar( P4Tag::v_status, "fail" );
	        if( confirm )
	            clientAck( client, e );
	        return;
	    }

	    if( asResults )
	    {
	        StrPtr *val;
	        StrBuf tmp;
	        char *vars[128];
	        int count = StrOps::Words( tmp, asResults->Text(),
	                                   vars, 128, ',' );
	        for( int i = 0; i < count; i++ )
	        {
	            if( ( val = results.GetVar( vars[i] ) ) )
	                client->SetVar( vars[i], val );
	            else
	            {
	                // if it ends in a *, that means we need to iterate 0..N
	                int l = strlen( vars[i] ) - 1;
	                if( vars[i][l] == '*' )
	                {
	                    // ignore the *
	                    StrBuf nm;
	                    nm.Set( vars[i], l );
	                    int n = 0;
	                    while( ( val = results.GetVar( nm, n ) ) )
	                        client->SetVar( nm, n++, *val );
	                }
	            }
	        }
	    }

	    client->SetVar( P4Tag::v_status, "pass" );
	}

	// Ack fallthough

	if( confirm )
	    clientAck( client, e );
}


int
AltSyncCheckFile( Client *client, StrPtr *confirm, const char *status,
                  const char *ntype, Error *e )
{
	if( !client->GetVar( P4Tag::v_altSync ) )
	    return 0;

	// Ask altSync if the file is virtual

	ClientAltSyncHandler *handler =
	    ClientAltSyncHandler::GetAltSyncHandler( client, e );
	if( e->Test() )
	    return 1;

	StrBufDict results;
	if( handler && !handler->AltSync( e, &results ) )
	{
	    StrPtr *dres = results.GetVar( "checkFile" );
	    status = dres && *dres == "same"    ? "same" :
	             dres && *dres == "missing" ? "missing" :
	             dres && *dres == "virtual" ? "virtual" :
	                                          "exists";
	    if( dres && *dres != "exists" )
	    {
	        client->SetVar( P4Tag::v_type, ntype );
	        client->SetVar( P4Tag::v_status, status );
	        client->Confirm( confirm );
	        return 1;
	    }
	}

	return 0;
}

void clientRelease( Client* client, Error* e )
{
	// Note - this overrides rpcservice.cc :: RpcServerRelease 

	if( client->statCallback.Length() )
	    client->ConditionalInvokeStats( e );

	client->GotReleased();
}

void clientReceiveFiles( Client *client, Error *e );

/*
 * clientDispatch - dispatch table
 */

const RpcDispatch clientDispatch[] = {
	{ P4Tag::c_OpenFile,	RpcCallback(clientOpenFile) },
	{ P4Tag::c_OpenDiff,	RpcCallback(clientOpenFile) },
	{ P4Tag::c_OpenMatch,	RpcCallback(clientOpenFile) },
	{ P4Tag::c_WriteFile,	RpcCallback(clientWriteFile) },
	
	{ P4Tag::c_ChunkMap,	RpcCallback(clientChunkMap) },
	{ P4Tag::c_WriteFileChunks,RpcCallback(clientWriteFileChunks) },

	{ P4Tag::c_WriteDiff,	RpcCallback(clientWriteFile) },
	{ P4Tag::c_WriteMatch,	RpcCallback(clientWriteFile) },
	{ P4Tag::c_CloseFile,	RpcCallback(clientCloseFile) },
	{ P4Tag::c_CloseDiff,	RpcCallback(clientCloseFile) },
	{ P4Tag::c_CloseMatch,	RpcCallback(clientCloseFile) },
	{ P4Tag::c_AckMatch,	RpcCallback(clientAckMatch) },

	{ P4Tag::c_DeleteFile,	RpcCallback(clientDeleteFile) },
	{ P4Tag::c_ChmodFile,	RpcCallback(clientChmodFile) },
	{ P4Tag::c_CheckFile,	RpcCallback(clientCheckFile) },
	{ P4Tag::c_ConvertFile,	RpcCallback(clientConvertFile) },
	{ P4Tag::c_ReconcileEdit,RpcCallback(clientReconcileEdit) },
	{ P4Tag::c_MoveFile,	RpcCallback(clientMoveFile) },

	{ P4Tag::c_ActionResolve, RpcCallback(clientActionResolve) },

	{ P4Tag::c_OpenMerge2,	RpcCallback(clientOpenMerge) },
	{ P4Tag::c_OpenMerge3,	RpcCallback(clientOpenMerge) },
	{ P4Tag::c_WriteMerge,	RpcCallback(clientWriteMerge) },
	{ P4Tag::c_CloseMerge,	RpcCallback(clientCloseMerge) },

	{ P4Tag::c_ReceiveFiles,RpcCallback(clientReceiveFiles) },
	{ P4Tag::c_SendFile,	RpcCallback(clientSendFile) },
	{ P4Tag::c_EditData,	RpcCallback(clientEditData) },
	{ P4Tag::c_InputData,	RpcCallback(clientInputData) },
	{ P4Tag::c_ReconcileAdd,RpcCallback(clientReconcileAdd) },
	{ P4Tag::c_ReconcileFlush,RpcCallback(clientReconcileFlush) },
	{ P4Tag::c_ExactMatch,    RpcCallback(clientExactMatch) },

	{ P4Tag::c_Prompt,	RpcCallback(clientPrompt) },
	{ P4Tag::c_Progress,	RpcCallback(clientProgress) },
	{ P4Tag::c_ErrorPause,	RpcCallback(clientErrorPause) },
	{ P4Tag::c_HandleError,	RpcCallback(clientHandleError) },
	{ P4Tag::c_Message,	RpcCallback(clientMessage) },
	{ P4Tag::c_OutputError,	RpcCallback(clientOutputError) },
	{ P4Tag::c_OutputInfo,	RpcCallback(clientOutputInfo) },
	{ P4Tag::c_OutputData,	RpcCallback(clientOutputInfo) },
	{ P4Tag::c_OutputText,	RpcCallback(clientOutputText) },
	{ P4Tag::c_OutputBinary,RpcCallback(clientOutputBinary) },
	{ P4Tag::c_FstatInfo,	RpcCallback(clientFstatInfo) },
	{ P4Tag::c_FstatPartial,RpcCallback(clientFstatPartial) },
	{ P4Tag::c_OpenUrl,	RpcCallback(clientOpenUrl) },
	{ P4Tag::c_WriteVarPartial, RpcCallback(clientWriteVarPartial) },

	{ P4Tag::c_AltSync,	RpcCallback(clientAltSync) },

	{ P4Tag::c_Ack,		RpcCallback(clientAck) },
	{ P4Tag::c_Ping,	RpcCallback(clientPing) },

	{ P4Tag::c_Crypto,	RpcCallback(clientCrypto) },
	{ P4Tag::c_SetPassword,	RpcCallback(clientSetPassword) },
	{ P4Tag::c_SSO,		RpcCallback(clientSingleSignon) },

	{ P4Tag::p_release,	RpcCallback(clientRelease) },

	{ P4Tag::p_protocol,	RpcCallback(clientProtocol) },
	{ P4Tag::p_errorHandler,RpcCallback(clientFatalError) },


	{ 0, 0 }
} ;
