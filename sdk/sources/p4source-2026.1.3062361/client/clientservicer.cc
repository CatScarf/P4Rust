/*
 * Copyright 1995, 2001 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# ifdef USE_EBCDIC
# define NEED_EBCDIC
# endif

# include <stdhdrs.h>

# include <strbuf.h>
# include <strdict.h>
# include <strops.h>
# include <strarray.h>
# include <strtable.h>
# include <error.h>
# include <runcmd.h>
# include <mapapi.h>
# include <handler.h>
# include <rpc.h>
# include <i18napi.h>
# include <charcvt.h>
# include <transdict.h>
# include <ignore.h>
# include <debug.h>
# include <tunable.h>
# include <vartree.h>
# include <inttree.h>

# include <p4tags.h>
# include <msgclient.h>
# include <msgsupp.h>

# include <filesys.h>
# include <pathsys.h>
# include <enviro.h>

# include <readfile.h>
# include <diffsp.h>
# include <diffan.h>
# include <diff.h>

# include <timer.h>
# include <progress.h>

# include "clientuser.h"
# include "client.h"
# include "clientprog.h"
# include "clientprogressreport.h"

# include "clientservice.h"
# include "clientaltsynchandler.h"

# ifdef HAS_CPP11
# include <thread>
# include <array>
# include <vector>
# endif

class StrStr
{
    public:
	StrStr( const char *name, const char *d ) :
	    fileName( name ), digest( d ) {}

	StrBuf fileName;
	StrBuf digest;
} ;

class DigestTree : public VVarTree
{
    public:
	DigestTree() {}
	virtual ~DigestTree() { Clear(); }

	virtual int Compare( const void *a, const void *b ) const
	{
	    const StrStr *aa = (const StrStr*)a;
	    const StrStr *bb = (const StrStr*)b;
	    return aa->fileName.XCompare( bb->fileName );
	}

	virtual void * Copy( const void *src ) const
	{
	    StrStr *s = ( StrStr* )src;
	    return new StrStr( s->fileName.Text(), s->digest.Text() );
	}

	virtual void Delete( void *a ) const { delete ( StrStr* )a; }
	virtual void Dump( void *a, StrBuf &buf ) const { /* do nothing */ }

} ;

struct StrSeq
{
	StrBuf fileName;
	Sequence *sequence;
	StrSeq( const char* name, Sequence* s = 0 ) :
	        fileName( name ), sequence( s ) {}
} ;

class SequenceTree : public VVarTree
{
    public:
	SequenceTree() {}
	virtual ~SequenceTree() {
	    // delete all sequences before removing all nodes
	    VarTreeNode *n = FirstNode();
	    while( n )
	    {
		StrSeq *s = ( StrSeq* )n->Value();
		delete s->sequence;
		n = n->Next();
	    }
	    Clear();
	}

	virtual int Compare( const void *a, const void *b ) const
	{
	    const StrSeq *aa = (const StrSeq*)a;
	    const StrSeq *bb = (const StrSeq*)b;
	    return aa->fileName.XCompare( bb->fileName );
	}

	virtual void * Copy( const void *src ) const
	{
	    StrSeq *s = ( StrSeq* )src;
	    return new StrSeq( s->fileName.Text(), s->sequence );
	}

	virtual void Delete( void *a ) const { delete ( StrSeq* )a; }
	virtual void Dump( void *a, StrBuf &buf ) const { /* do nothing */ }
} ;

/*
 * ReconcileHandle - handle reconcile's list of files to skip when adding
 */

class ReconcileHandle : public LastChance {
    public:
	static ReconcileHandle *
	GetOrCreate( Client *client, bool create, Error *e )
	{
	    StrRef handleName( "skipAdd" );
	    ReconcileHandle *h =
	        (ReconcileHandle *)client->handles.Get( &handleName );
	    if( !h && create )
	    {
	        h = new ReconcileHandle();
	        client->handles.Install( &handleName, h, e );

	        if( e->Test() )
	        {
	            delete h;
	            h = 0;
	        }
	    }
	    return h;
	}

	virtual ~ReconcileHandle() 
	{ 
	    delete pathArray;
	    delete progress;

	    ReportPerfStats();
	}

	void BeginStage( Client *client, const StrPtr& s, int progressType  )
	{
	    if( stage == s )
	        return;

	    stage = s;
	    delete progress;
	    progress = CreateClientProgressReport( client, progressType );
	    progress->Description( s );
	    if( progressType == CPT_FILES || progressType == CPT_DIRS )
	        progress->Units( progressType == CPT_FILES ? PRU_FILES
	                                                   : PRU_DIRS );
	    progress->DoReport( CPP_NORMAL );
	}

	void StageComplete( Client *client )
	{
	    if( progress )
	        progress->DoReport( CPP_DONE );
	}

	void Increment( Client *client, long inc )
	{
	    if( !progress )
	        return;

	    progress->Increment( inc );
	    if( progress->IsComplete() )
	        StageComplete( client );
	}

	void LogDigestTimer( P4INT64 elapseTime )
	{
	    digestCounter += 1;
	    digestTimer += elapseTime;
	}

	void LogSequenceTimer( P4INT64 elapseTime )
	{
	    sequenceCounter += 1;
	    sequenceTimer += elapseTime;
	}

	void LogDiffTimer( P4INT64 elapseTime )
	{
	    diffCounter += 1;
	    diffTimer += elapseTime;
	}

	Sequence *GetSequence( FileSys *f, const DiffFlags &flags, Error *e )
	{
	    StrSeq s( f->Name() );
	    StrSeq *c = ( StrSeq* )seqTree.Get( &s );
	    if( !c )
	    {
	        Timer timer;
	        timer.Start();

	        s.sequence = new Sequence( f, flags, e );
	        seqTree.Put( &s, e );
	        LogSequenceTimer( timer.Time() );

	        return s.sequence;
	    }
	    else
	    {
	        c->sequence->Reuse( f, e );
	        return c->sequence;
	    }
	}

	void SetMatch( int i )
	{
	    Error e;
	    matchedIndices.Insert( i, &e );
	}

	int AlreadyMatched( int i )
	{
	    return matchedIndices.Contains( i );
	}

	void GetDigest( FileSys *f, StrBuf &localDigest, Error *e )
	{
	    StrStr c( f->Name(), "" );
	    StrStr *d = ( StrStr* )digestTree.Get( &c );
	    if( d )
	    {
	        localDigest = d->digest;
	    }
	    else
	    {
	        Timer timer;
	        timer.Start();

	        f->Digest( &localDigest, e );
	        c.digest = localDigest;
	        digestTree.Put( &c, e );

	        LogDigestTimer( timer.Time() );
	    }
	}

	int delCount;
	StrArray *pathArray;
	ClientProgressReport *progress;

    private:
	// Progress reporting
	StrBuf stage;
	
	// Performance tracking
	P4INT64 digestCounter;
	P4INT64 digestTimer;
	P4INT64 sequenceCounter;
	P4INT64 sequenceTimer;
	P4INT64 diffCounter;
	P4INT64 diffTimer;
	
	DigestTree digestTree;
	SequenceTree seqTree;
	P4INT64Tree matchedIndices;

	ReconcileHandle()
	{ 
	    pathArray = new StrArray;
	    delCount = 0;
	    progress = 0;

	    digestCounter = 0;
	    digestTimer = 0;
	    sequenceCounter = 0;
	    sequenceTimer = 0;
	    diffCounter = 0;
	    diffTimer = 0;
	}

	ClientProgressReport *
	CreateClientProgressReport( Client *client, int progressType )
	{
	    ClientProgress *indicator;
	    indicator =  client->GetUi()->CreateProgress( progressType );
	    return new ClientProgressReport( indicator );
	}

	void ReportPerfStats()
	{
	    if( !p4debug.IsSet( DT_PERF ) )
	        return;

	    if( !digestCounter && !sequenceCounter && !diffCounter )
	        return;

	    p4debug.printf( "Reconcile performance stats:\n" );
	    p4debug.printf( "Digest/Sequence/Diff count+time(ms):\n");
	    p4debug.printf( "%d+%d %d+%d %d+%d\n",
	                    digestCounter, digestTimer,
	                    sequenceCounter, sequenceTimer,
	                    diffCounter, diffTimer );
	}
} ;

/*
 * SendDir - utility method used by clientTraverseShort to decide if a
 *	     filename should be output as a file or as a directory (status -s)
 */
int
SendDir( PathSys *fileName, StrPtr *cwd, StrArray *dirs, int &idx, int skip )
{
	int isDir = 0;

	// Skip printing file in current directory and just report subdirectory

	if( skip )
	{
	    fileName->SetLocal( *cwd, StrRef( "..." ) );
	    return 1;
	}

	// If file is in the current directory: isDirs is unset so that our
	// caller will send back the original file.

	fileName->ToParent();

	if( !fileName->SCompare( *cwd ) )
	    return isDir;

	// Set path to the directory under cwd containing this file.
	// 'dirs' is the list of dirs in cwd on workspace.

	for( ; idx < dirs->Count() && !isDir; idx++ )
	{
	    if( fileName->IsUnderRoot( *dirs->Get( idx ) ) )
	    {
		fileName->SetLocal( *dirs->Get(idx), StrRef( "..." ));
		++isDir;
	    }
	}

	return isDir;
}

void
clientReconcileFlush( Client *client, Error *e )
{
	// Delete the client's reconcile handle

	ReconcileHandle *recHandle =
	    ReconcileHandle::GetOrCreate( client, false, e );

	if( recHandle )
	    delete recHandle;
}

void convertSlash( char *pathSet )
{
	while( *pathSet != '\0' )
	{
	    if( *pathSet == '\\' )
	        *pathSet = '/';
	    pathSet++;
	}
}

/*
 * clientReconcileEdit() -- "inquire" about file, for 'p4 reconcile'
 *
 * This routine performs clientCheckFile's scenario 1 checking, but
 * also saves the list of files that are in the depot so they can be
 * compared to the list of files on the client when reconciling later for add.
 *
 */

void
clientReconcileEdit( Client *client, Error *e )
{
	client->NewHandler();
	StrPtr *clientType = client->GetVar( P4Tag::v_type );
	StrPtr *digest = client->GetVar( P4Tag::v_digest );
	StrPtr *digestType = client->GetVar( P4Tag::v_digestType );
	StrPtr *confirm = client->GetVar( P4Tag::v_confirm, e );
	StrPtr *fileSize = client->GetVar( P4Tag::v_fileSize );
	StrPtr *submitTime = client->GetVar( P4Tag::v_time );
	StrPtr *count = client->GetVar( P4Tag::v_count );

	if( e->Test() && !e->IsFatal() )
	{
	    client->OutputError( e );
	    return;
	}

	const char *status = "exists";
	const char *ntype = clientType ? clientType->Text() : "text";

	// For adding files,  checkSize is a maximum (or use alt type)
	// For flush,  checkSize is an optimization check on binary files.

	offL_t checkSize = fileSize ? fileSize->Atoi64() : 0;

	/*
	 * If we do know the type, we want to know if it's missing.
	 * If it isn't missing and a digest is given, we want to know if
	 * it is the same.
	*/

	FileSys *f = ClientSvc::File( client, e );

	if( e->Test() || !f )
	    return;
	int statVal = f->Stat();

	// Save the list of depot files. We'll diff it against the list of all
	// files on client to find files to add in clientReconcileAdd

	ReconcileHandle *recHandle =
	    ReconcileHandle::GetOrCreate( client, true, e );
	if( e->Test() )
	    return;

	recHandle->BeginStage( client, StrRef( "Reconcile Edit" ), CPT_FILES );
	if( count )
	    recHandle->progress->Total( count->Atoi64() );

	if( AltSyncCheckFile( client, confirm, status, ntype, e ) )
	{
	    if( !strcmp( status, "missing" ) )
	        recHandle->delCount++;
	    else
	        recHandle->pathArray->Put()->Set( f->Name() );
	    return;
	}
	if( e->Test() )
	    return;

	if( !( statVal & ( FSF_SYMLINK|FSF_EXISTS ) ) )
	{
	    status = "missing";
	    recHandle->delCount++;
	} 
	else if ( ( !( statVal & FSF_SYMLINK ) && ( f->IsSymlink() ) ) 
                || ( ( statVal & FSF_SYMLINK ) && !( f->IsSymlink() ) ) ) 
	{
	    recHandle->pathArray->Put()->Set( f->Name() );
	}
	else if( digest )
	{
	    recHandle->pathArray->Put()->Set( f->Name() );
	    if( digestType )
	    {
		StrBuf localDigest;
		FileDigestType digType = FS_DIGEST_UNKNOWN;
		if( !digestType->Compare( StrRef( P4Tag::v_digestTypeMD5 ) ) )
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

		f->ComputeDigest( digType, &localDigest, e );
		if( !e->Test() && !localDigest.XCompare( *digest ) )
		    status = "same";

	    } 
	    else if( !checkSize || checkSize == f->GetSize() )
	    {
		StrBuf localDigest;
		f->Translator( ClientSvc::XCharset( client, FromClient ) );
		int modTime = f->StatModTime();

		// Bypass expensive digest computation with -m unless the
		// local file's timestamp is different from the server's.

		if( !submitTime ||
		    ( submitTime && ( modTime != submitTime->Atoi()) ) )
		{
		    f->Digest( &localDigest, e );

		    if( !e->Test() && !localDigest.XCompare( *digest ) )
		    {
			status = "same";
			client->SetVar( P4Tag::v_time, modTime );
		    }
		}
		else if( submitTime )
		    status = "same";
	    }

	    // If we can't read the file (for some reason -- wrong type?)
	    // we consider the files different.

	    e->Clear();
	}

	delete f;

	recHandle->Increment( client, 1 );

	// tell the server 

	client->SetVar( P4Tag::v_type, ntype );
	client->SetVar( P4Tag::v_status, status );
	client->Confirm( confirm );

	// Report non-fatal error and clear it.

	client->OutputError( e );
}

int
clientTraverseShort( Client *client, StrPtr *cwd, const char *dir, int traverse,
		    int noIgnore, int initial, int skipCheck, int skipCurrent,
		    MapApi *map, StrArray *files, StrArray *dirs, int &idx,
		    StrArray *depotFiles, int &ddx, const char *config,
		    ClientProgressReport* progress, Error *e )
{
	// Variant of clientTraverseDirs that computes the files to be
	// added during traversal of directories instead of at the end,
	// and returns directories and files rather than all files.
	// This is used by 'status -s'.

	if( progress )
	    progress->Increment( 1 );

	// Scan the directory.

	FileSys *f = client->GetUi()->File( FST_BINARY );

	// Opt into FileSys stat caching so the per-file Stat() in the loop
	// below stamps mtime+size onto the FileSys, and the follow-up
	// StatModTime() / GetSize() inside SetDigestOrType reuse those
	// values instead of doing a second CreateFile or nt_stat.  Cache is
	// invalidated on every Set().

	f->EnableStatCache();
	f->SetContentCharSetPriv( client->content_charset );
	f->Set( StrRef( dir ) );
	int fstat = f->Stat();

	Ignore *ignore = client->GetIgnore();
	StrPtr ignored = client->GetIgnoreFile();
	StrBuf from;
	StrBuf to;
	const char *fileName;
	int found = 0;

	// Use server case-sensitivity rules to find files to add
	// from.SetCaseFolding( client->protocolNocase );

	// With unicode server and client using character set, we need
	// to send files back as utf8.

	if( client != client->translated )
	{
	    CharSetCvt *cvt = ( (TransDict *)client->transfname )->ToCvt();
	    fileName = cvt->FastCvt( f->Name(), strlen(f->Name()), 0 );
	    if( !fileName )
		fileName = f->Name();
	}
	else
	    fileName = f->Name();

	// If this is a file, not a directory, and not to be ignored,
	// save the filename 

	if( !( fstat & FSF_DIRECTORY ) )
	{
	    if( ( fstat & FSF_EXISTS ) || ( fstat & FSF_SYMLINK ) )
	    {
		if( noIgnore || 
	           !ignore->Reject( StrRef(f->Name()), ignored, config ) )
		{
		    files->Put()->Set( fileName );
		    found = 1;
		}
	    }
	    delete f;
	    return found;
	}

	// If this is a symlink to a directory, and not to be ignored,
	// return filename and quit

	if( ( fstat & FSF_SYMLINK ) && ( fstat & FSF_DIRECTORY ) )
	{
	    if( noIgnore || 
	        !ignore->Reject( StrRef(f->Name()), ignored, config ) )
	    {
		files->Put()->Set( fileName );
		found = 1;
	    }
	    delete f;
	    return found;
	}

	// This is a directory to be scanned.

	StrArray *ua = f->ScanDir( e );

	if( e->Test() )
	{
	    // report error but keep moving

	    delete f;
	    client->OutputError( e );
	    return 0;
	}

	// PathSys for concatenating dir and local path,
	// to get full path

	PathSys *p = PathSys::Create();
	p->SetCharSet( f->GetCharSetPriv() );

	// Attach path delimiter to dirs so Sort() works correctly, and also to
	// save relevant Stat() information.

	StrArray *a = new StrArray();
	StrBuf dirDelim;
	StrBuf symDelim;

#ifdef OS_NT
	dirDelim.Set( "\\" );
	symDelim.Set( "\\\\" );
#else
	dirDelim.Set( "/" );
	symDelim.Set( "//" );
#endif

	// Now files & dirs at this level from ScanDir() will be sorted
	// in the same order as the depotFiles array. And delimiters tell us
	// the Stat() of those files.

	for( int i = 0; i < ua->Count(); i++ )
	{
	    p->SetLocal( StrRef( dir ), *ua->Get(i) );
	    f->Set( *p );
	    int stat = f->Stat();
	    StrBuf out;

	    if( ( stat & FSF_DIRECTORY ) && !( stat & FSF_SYMLINK ) )
		out << ua->Get( i ) << dirDelim;
	    else if( ( stat & FSF_DIRECTORY ) && ( stat & FSF_SYMLINK ) )
		out << ua->Get( i ) << symDelim;
	    else if( ( stat & FSF_EXISTS ) || ( stat & FSF_SYMLINK ) )
		out << ua->Get(i);
	    else
		continue;

	    a->Put()->Set( out );
	}
	a->Sort( !StrBuf::CaseUsage() );
	delete ua;

	// If directory is unknown to p4, we don't need to check that files
	// are in depot (they aren't), so just return after the first file
	// is found and bypass checking. 

	int doSkipCheck = skipCheck;

	int dddx = 0;
	int matched;
	StrArray *depotDirs = new StrArray();

	// First time through we save depot dirs

	if( initial )
	{
	    StrPtr *ddir;
	    for( int j=0; ( ddir=client->GetVar( StrRef("depotDirs"), j) ); j++)
	    {
		StrBuf dirD;
		dirD << ddir << dirDelim;
		depotDirs->Put()->Set( dirD );
	    }
	    depotDirs->Sort( !StrBuf::CaseUsage() );
	}

	// For each directory entry.

	for( int i = 0; i < a->Count(); i++ )
	{
	    // Strip delimiters and use the hints to determine stat

	    int isDir = 0;
	    int isSymDir = 0;
	    StrBuf fName;
	    if( a->Get(i)->EndsWith( symDelim.Text(), 2 ) )
	    {
		++isDir;
		++isSymDir;
		fName.Set( a->Get(i)->Text(), a->Get(i)->Length() - 2 );
	    }
	    else if( a->Get(i)->EndsWith( dirDelim.Text(), 1 ) )
	    {
		++isDir;
		fName.Set( a->Get(i)->Text(), a->Get(i)->Length() - 1 );
	    }
	    else
		fName.Set( a->Get(i) );
		
	    // Check mapping, ignore files before sending file or symlink back

	    p->SetLocal( StrRef( dir ), fName );
	    f->Set( *p );
	    int checkFile = 0;

	    if( client != client->translated )
	    {
		CharSetCvt *cvt = ( (TransDict *)client->transfname )->ToCvt();
		fileName = cvt->FastCvt( f->Name(), strlen(f->Name()) );
		if( !fileName )
		    fileName = f->Name();
	    }
	    else
		fileName = f->Name();

	    if( isDir )
	    {
		if( isSymDir )
		{
		    from.Set( fileName );
		    from << "/";

#ifdef OS_NT
	            convertSlash( from.Text() );
#endif

	            if( client->protocolNocase != StrBuf::CaseUsage() )
	            {
	                from.SetCaseFolding( client->protocolNocase );
	                matched = map->Translate( from, to, MapLeftRight );
	                from.SetCaseFolding( !client->protocolNocase );
	            }
	            else
	                matched = map->Translate( from, to, MapLeftRight );

		    if( !matched )
			continue;

		    if( noIgnore || 
	                !ignore->Reject( StrRef(f->Name()), ignored, config ) )
		    {
			if( doSkipCheck )
			{
			    p->Set( fileName );
			    (void)SendDir( p, cwd, dirs, idx, skipCurrent );
			    files->Put()->Set( p );
			    found = 1;
			    break;
			}
			else
			   ++checkFile;
		    }
		}
		else if( traverse )
		{
		    if( initial )
		    {
			dirs->Put()->Set( fileName );
			int foundOne = 0;
			int l;

			// If this directory is unknown to the depot, we don't
			// need to compare against depot files. 

			for( ; dddx < depotDirs->Count() && !foundOne; dddx++)
			{
			    StrBuf fName;
			    fName << fileName << dirDelim;
			    const StrPtr *ddir = depotDirs->Get( dddx );
			    p->SetLocal( *cwd, *ddir );
			    l =  fName.SCompare( *p );
			    if( !l )
				++foundOne;
			    else if( l < 0 )
				break;
			}
			skipCheck = !foundOne ? 1 : 0;
		    }

		    found = clientTraverseShort( client, cwd, f->Name(),
						traverse, noIgnore, 0,
						skipCheck, skipCurrent, map,
						files, dirs, idx, depotFiles,
						ddx, config, progress, e );

		    // Stop traversing directories when we have a file to
		    // to add, unless we are at the top and need to check
		    // for files in the current directory.

		    if( found && !initial )
			break;
		    else if( found && initial && !skipCurrent )
			found = 0;
		    if( found )
			break;
		}
	    }
	    else
	    {
		from.Set( fileName );
		
#ifdef OS_NT
	        convertSlash( from.Text() );
#endif

	        if( client->protocolNocase != StrBuf::CaseUsage() )
	        {
	            from.SetCaseFolding( client->protocolNocase );
	            matched = map->Translate( from, to, MapLeftRight );
	            from.SetCaseFolding( !client->protocolNocase );
	        }
	        else
	            matched = map->Translate( from, to, MapLeftRight );

		if( !matched )
		    continue;

		if( noIgnore || 
	            !ignore->Reject( StrRef(f->Name()), ignored, config ) )
		{
		    if( doSkipCheck )
		    {
			p->Set( fileName );
			(void)SendDir( p, cwd, dirs, idx, skipCurrent );
			files->Put()->Set( p );
			found = 1;
			break;
		    }
		    else
			++checkFile;
		}
	    }

	    // See if file is in depot and if not, either set the file
	    // or directory to be reported back to the server.

	    if( checkFile )
	    {
		int l = 0;
		int finished = 0;
		while ( !finished )
		{
		    if( ddx >= depotFiles->Count())
			l = -1;
		    else
			l = StrRef( fileName ).SCompare( *depotFiles->Get(ddx));

		    if( !l )
		    {
			++ddx;
			++finished;
		    }
		    else if( l < 0 )
		    {
			p->Set( fileName );
			if( initial && skipCurrent )
			{
			    p->ToParent();
			    p->SetLocal( *p, StrRef("...") );
			    files->Put()->Set( p );
			}
			else if( SendDir( p, cwd, dirs, idx, skipCurrent ) )
			    files->Put()->Set( p );
			else
			    files->Put()->Set( fileName );
			found = 1;
			break;
		    }
		    else
			++ddx;
		}
		if( ( !initial || skipCurrent ) && found )
		    break;
	    }
	}

	delete p;
	delete a;
	delete f;
	delete depotDirs;

	return found;
}

static void
SetDigestOrType( Client *client, FileSys *f, const char *fileName,
	         StrArray *files, StrArray *sizes, StrArray *times,
	         StrArray *digests, StrArray *types, Error *e )
{
	files->Put()->Set( fileName );
	times->Put()->Set( StrNum( f->StatModTime() ) );

	// When neither digest nor type is requested, take the 24.2-style
	// lean walk -- stat-based size only, no per-file open.
	// The server gates v_sendDigest, v_sendFileSize, and v_sendType
	// together, so reaching this path means either a flush/move-against-
	// old-client (all three set, current behavior below) or the deferred
	// non-doMoves case (none set, this branch).

	if( !digests && !types )
	{
	    if( sizes )
	        sizes->Put()->Set( StrNum( f->GetSize() ) );

	    return;
	}

	// Get the correct type for the digest and file size

	FileSysType t = f->CheckType();
	FileSys *f2 = client->GetUi()->File( t );
	CharSetCvt *cvt = ClientSvc::XCharset( client, FromClient );

	if( f2 )
	{
	    f2->SetContentCharSetPriv( client->content_charset );
	    f2->Set( fileName );
	    f2->Translator( cvt );
	}

	// When no digest is requested, we may be able to skip the full
	// Open+Read of the file and use GetSize() (stat-based) for the size.
	// clientCheckFile asks the same question, so the rule is shared.

	StrBuf localDigest;
	FileSys* fs = f2 ? f2 : f; // In case we failed to resolve the type
	bool sizeMatchesDepot =
	    ClientSvc::SizeMatchesDepot( fs->GetType(), cvt );

	offL_t fsize;
	if( digests )
	    fsize = fs->Digest( &localDigest, e );
	else if( sizeMatchesDepot )
	    fsize = fs->GetSize();
	else
	    fsize = fs->Digest( 0, e );

	if( e->Test() )
	{
	    // We "should" just be marking the file for add, so if we encounter
	    // an error here, we won't be able to match it to a delete (yet)
	    // but we might as well record its presence.
	    // Suppress the error and pad the arrays
	    e->Clear();
	    if( digests )
	        digests->Put();
	    if( types )
	        types->Put();
	    if( sizes )
	        sizes->Put();
	    return;
	}

	if( digests )
	    digests->Put()->Set( localDigest );

	if( types )
	{
	    // Beware of empty files

	    Error msg;

	    // fs rather than f2: FileSys::Create() has no case for FST_CANTTELL
	    // and returns null, and a file that was deliberately not read is
	    // readable enough that the Digest above succeeds and nothing else
	    // catches the null.  Scan -1 matches the CheckType() above.

	    const char *ntype = clientCheckFileType( fs, t,
	                                             client->protocolXfiles,
	                                             1, -1, 0, 0, 0, &msg );
	    if( !ntype )
	        client->SetError();

	    types->Put()->Set( ntype ? ntype : "" );
	}
	delete f2;

	if( sizes )
	    sizes->Put()->Set( StrNum( fsize ) );
}

void
clientTraverseDirs( Client *client, const char *dir, int traverse, int noIgnore,
		    int getDigests, int getTypes, MapApi *map, StrArray *files,
		    StrArray *sizes, StrArray *times, StrArray *digests, StrArray *types,
		    int &hasIndex, StrArray *hasList, const char *config, 
		    ClientProgressReport* progress, Error *e )
{
	// Return all files in dir, and optionally traverse dirs in dir,
	// while checking each file against map before returning it

	if( progress )
	    progress->Increment( 1 );

	// Scan the directory.

	FileSys *f = client->GetUi()->File( FST_BINARY );

	// Opt into FileSys stat caching so the per-file Stat() in the loop
	// below stamps mtime+size onto the FileSys, and the follow-up
	// StatModTime() / GetSize() inside SetDigestOrType reuse those
	// values instead of doing a second CreateFile or nt_stat.  Cache is
	// invalidated on every Set().

	f->EnableStatCache();
	f->SetContentCharSetPriv( client->content_charset );
	f->Set( StrRef( dir ) );
	int fstat = f->Stat();

	Ignore *ignore = client->GetIgnore();
	StrPtr ignored = client->GetIgnoreFile();
	StrBuf from;
	StrBuf to;
	const char *fileName;
	StrBuf localDigest;

	// With unicode server and client using character set, we need
	// to send files back as utf8.

	if( client != client->translated )
	{
	    CharSetCvt *cvt = ( (TransDict *)client->transfname )->ToCvt();
	    fileName = cvt->FastCvt( f->Name(), strlen(f->Name()), 0 );
	    if( !fileName )
	        fileName = f->Name();
	}
	else
	    fileName = f->Name();

	// If this is a file, not a directory, and not to be ignored,
	// save the filename 

	if( !( fstat & FSF_DIRECTORY ) )
	{
	    if( ( fstat & FSF_EXISTS ) || ( fstat & FSF_SYMLINK ) )
	    {
	        if( noIgnore || 
	            !ignore->Reject( StrRef(f->Name()), ignored, config ) )
	        {
	            SetDigestOrType( client, f, fileName,
	                             files, sizes, times,
	                             getDigests ? digests : 0,
	                             getTypes ? types : 0, e );
	        }
	    }
	    delete f;
	    return;
	}

	// If this is a symlink to a directory, and not to be ignored,
	// return filename and quit

	if( ( fstat & FSF_SYMLINK ) && ( fstat & FSF_DIRECTORY ) )
	{
	    if( noIgnore || 
	        !ignore->Reject( StrRef(f->Name()), ignored, config ) )
	    {
	        SetDigestOrType( client, f, fileName,
	                         files, sizes, times,
	                         getDigests ? digests : 0,
	                         getTypes ? types : 0, e );
	    }
	    delete f;
	    return;
	}

	// Directory might be ignored,  bail

	if( !noIgnore && 
	    ignore->RejectDir( StrRef( f->Name() ), ignored, config ) )
	{
	    delete f;
	    return;
	}

	// This is a directory to be scanned.

	StrArray *a = f->ScanDir( e );

	if( e->Test() )
	{
	    // report error but keep moving

	    delete f;
	    client->OutputError( e );
	    return;
	}

	// Sort in case sensitivity of client
	a->Sort( !StrBuf::CaseUsage() );

	// PathSys for concatenating dir and local path,
	// to get full path

	PathSys *p = PathSys::Create();
	p->SetCharSet( f->GetCharSetPriv() );
	int matched;

	// For each directory entry.

	for( int i = 0; i < a->Count(); i++ )
	{
	    // Check mapping, ignore files before sending file or symlink back

	    p->SetLocal( StrRef( dir ), *a->Get(i) );
	    f->Set( *p );

	    if( client != client->translated )
	    {
	        CharSetCvt *cvt = ( (TransDict *)client->transfname )->ToCvt();
	        fileName = cvt->FastCvt( f->Name(), strlen(f->Name()) );
	        if( !fileName )
	            fileName = f->Name();
	    }
	    else
	        fileName = f->Name();

	    // Do compare with array list (skip files if possible)
	    int cmp = -1;

	    while( hasList && hasIndex < hasList->Count() )
	    {
	        cmp = f->Path()->SCompare( *hasList->Get( hasIndex ) );

	        if( cmp < 0 )
	            break;

	        hasIndex++;

	        if( cmp == 0 )
	            break;
	    }

	    // Don't stat if we matched a file from the edit list

	    if( cmp == 0 )
	        continue;

	    int stat = f->Stat();

	    if( stat & FSF_DIRECTORY )
	    {
	        if( stat & FSF_SYMLINK )
	        {
	            from.Set( fileName );
	            from << "/";

#ifdef OS_NT
	        convertSlash( from.Text() );
#endif

	            if( client->protocolNocase != StrBuf::CaseUsage() )
	            {
	                from.SetCaseFolding( client->protocolNocase );
	                matched = map->Translate( from, to, MapLeftRight );
	                from.SetCaseFolding( !client->protocolNocase );
	            }
	            else
	                matched = map->Translate( from, to, MapLeftRight );

	            if( !matched )
	                continue;

	            if( noIgnore || 
	                !ignore->Reject( StrRef(f->Name()), ignored, config ) )
	            {
	                SetDigestOrType( client, f, fileName,
	                                 files, sizes, times,
	                                 getDigests ? digests : 0,
	                                 getTypes ? types : 0, e );
	            }
	        }
	        else if( traverse )
	            clientTraverseDirs( client, f->Name(), traverse, noIgnore,
	                                getDigests, getTypes, map, files, sizes, times,
	                                digests, types, hasIndex, hasList, 
	                                config, progress, e );
	    }
	    else if( ( stat & FSF_EXISTS ) || ( stat & FSF_SYMLINK ) )
	    {
	        from.Set( fileName );

#ifdef OS_NT
	        convertSlash( from.Text() );
#endif
	        if( client->protocolNocase != StrBuf::CaseUsage() )
	        {
	            from.SetCaseFolding( client->protocolNocase );
	            matched = map->Translate( from, to, MapLeftRight );
	            from.SetCaseFolding( !client->protocolNocase );
	        }
	        else
	            matched = map->Translate( from, to, MapLeftRight );

	        if( !matched )
	            continue;

	        if( noIgnore || 
	            !ignore->Reject( StrRef(f->Name()), ignored, config ) )
	        {
	            SetDigestOrType( client, f, fileName,
	                             files, sizes, times,
	                             getDigests ? digests : 0,
	                             getTypes ? types : 0, e );
	        }
	    }
	}

	delete p;
	delete a;
	delete f;
}

void
clientReconcileAdd( Client *client, Error *e )
{
	/*
	 * Reconcile add confirm
	 *
	 * Scans the directory (local syntax) and returns
	 * files in the directory using the full path.  This
	 * differs from clientScanDir in that it returns full
	 * paths, supports traversing subdirectories, and checks
	 * against a mapTable.
	 */

	client->NewHandler();
	StrPtr *dir = client->transfname->GetVar( P4Tag::v_dir, e );
	StrPtr *confirm = client->GetVar( P4Tag::v_confirm, e );
	StrPtr *traverse = client->GetVar( "traverse" );
	StrPtr *summary = client->GetVar( "summary" );
	StrPtr *skipIgnore = client->GetVar( "skipIgnore" );
	StrPtr *skipCurrent = client->GetVar( "skipCurrent" );
	StrPtr *sendFileSize = client->GetVar( P4Tag::v_sendFileSize );
	StrPtr *sendDigest = client->GetVar( P4Tag::v_sendDigest );
	StrPtr *sendType = client->GetVar( P4Tag::v_sendType );
	StrPtr *sendTime = client->GetVar( "sendTime" );
	StrPtr *mapItem;

	if( e->Test() )
	    return;

	MapApi *map = new MapApi;
	StrArray *files = new StrArray();
	StrArray *sizes = new StrArray();
	StrArray *times = new StrArray();
	StrArray *dirs = new StrArray();
	StrArray *depotFiles = new StrArray();
	StrArray *digests = new StrArray();
	StrArray *types = new StrArray();

	// Construct a MapTable object from the strings passed in by server

	for( int i = 0; ( mapItem = client->GetVar( StrRef("mapTable"), i ) ); i++)
	{
	    MapType m;
	    int j;
	    char *c = mapItem->Text();
	    switch( *c )
	    {
	    case '-': m = MapExclude;   j = 1; break;
	    case '+': m = MapOverlay;   j = 1; break;
	    // Special case for &: we want the client to send all
	    // potentially mapped files
	    case '&': m = MapInclude;   j = 1; break;
	    default:  m = MapInclude;   j = 0; break;
	    }
#ifdef OS_NT 
	    convertSlash( c );
#endif
	    map->Insert( StrRef( c+j ), StrRef( c+j ), m );
	}

	// If we have a list of files we know are in the depot already,
	// filter them out of our list of files to add. For -s option,
	// we need to have this list of depot files for computing files
	// and directories to add (even if it is an empty list).

	ReconcileHandle *recHandle =
	    ReconcileHandle::GetOrCreate( client, true, e );
	if( e->Test() )
	{
	    delete files;
	    delete sizes;
	    delete times;
	    delete dirs;
	    delete depotFiles;
	    delete digests;
	    delete types;
	    delete map;
	    return;
	}
	recHandle->pathArray->Sort( !StrBuf::CaseUsage() );
	recHandle->BeginStage( client, StrRef( "Reconcile Add" ), CPT_DIRS );

	// status -s also needs the list of files opened for add appended
	// to the list of depot files.

	if( summary != 0 )
	{
	    const StrPtr *dfile;
	    for( int j=0; ( dfile=client->GetVar( StrRef("depotFiles"), j) ); j++)
	        depotFiles->Put()->Set( dfile );
	    for( int j=0; ( dfile=recHandle->pathArray->Get(j) ); j++ )
	        depotFiles->Put()->Set( dfile );
	    depotFiles->Sort( !StrBuf::CaseUsage() );
	}

	// status -s will output files in the current directory and paths
	// rather than all of the files individually. Compare against depot
	// files early so we can abort traversal early if we can.

	int hasIndex = 0;
	const char *config = client->GetEnviro()->Get( "P4CONFIG" );

	if( summary != 0 )
	{
	    int idx = 0;
	    int ddx = 0;
	    (void)clientTraverseShort( client, dir, dir->Text(), traverse != 0,
	                               skipIgnore != 0, 1, 0, skipCurrent != 0,
	                               map, files, dirs, idx,
	                               depotFiles, ddx, config, recHandle->progress, e );
	}
	else
	    clientTraverseDirs( client, dir->Text(), traverse != 0,
	                        skipIgnore != 0, sendDigest != 0,
	                        sendType != 0, map, files, sizes, times,
	                        digests, types, hasIndex, recHandle->pathArray,
	                        config, recHandle->progress, e );
	delete map;

	// Compare list of files on client with list of files in the depot
	// if we have this list from ReconcileEdit. Skip this comparison
	// if summary because it was done already.

	if( !summary )
	{
	    int i1 = 0, i2 = 0, i0 = 0, l = 0;

	    while( i1 < files->Count() )
	    {
	        if( i2 >= recHandle->pathArray->Count())
	            l = -1;
	        else
	            l = files->Get( i1 )->SCompare( 
	                *recHandle->pathArray->Get( i2 ) );

	        if( !l )
	        {
	            ++i1;
	            ++i2;
	        }
	        else if( l < 0 )
	        {
	            client->SetVar( P4Tag::v_file, i0, *files->Get( i1 ) );

	            if( ( sendFileSize && ( i1 < sizes->Count() ) ) ||
	                ( !sendDigest && recHandle->delCount ) )
	            {
	                // 2025.1 server always requests fileSize but older servers
	                // did not so the client has to guess when to send based on
	                // 1) !sendDigest: not doing flush and 
	                // 2) delCount != 0: need for move match

	                client->SetVar( P4Tag::v_fileSize, 
	                                i0, *sizes->Get( i1 ) );
	            }

	            if( sendDigest )
	                client->SetVar( P4Tag::v_digest, i0, *digests->Get(i1) );
	            if( sendType )
	                client->SetVar( P4Tag::v_type, i0, *types->Get(i1) );
	            if( sendTime )
	                client->SetVar( P4Tag::v_time, i0, *times->Get(i1) );
	            ++i0;
	            ++i1;
	        }
	        else
	        {
	            ++i2;
	        }

	        if( !( ( i1 + 1 ) % 1000 ) )
	        {
	            client->Confirm( confirm );
	            i0 = 0;
	        }
	    }
	}
	else
	{
	    int i0 = 0;

	    for( int j = 0; j < files->Count(); j++ )
	    {
	        client->SetVar( P4Tag::v_file, i0++, *files->Get(j) );

	        if( !( ( j + 1 ) % 1000 ) )
	        {
	            client->Confirm( confirm );
	            i0 = 0;
	        }
	    }
	}

	recHandle->StageComplete( client );

	client->Confirm( confirm );
	delete files;
	delete sizes;
	delete times;
	delete dirs;
	delete depotFiles;
	delete digests;
	delete types;
}

void
clientExactMatch( Client *client, Error *e )
{
	// Compare existing digest to list of
	// new client files, return match, or not.

	// Args:
	// type     = existing file type (clientpart)
	// digest   = existing file digest
	// fileSize = existing file size
	// charSet  = existing file charset
	// toFileN  = new file local path
	// indexN   = new file index
	// confirm  = return callback
	//
	// Return:
	// toFile   = exact match
	// index    = exact match

	client->NewHandler();
	StrPtr *digest = client->GetVar( P4Tag::v_digest );
	StrPtr *confirm = client->GetVar( P4Tag::v_confirm, e );
	StrPtr *count = client->GetVar( P4Tag::v_count );

	if( e->Test() )
	    return;

	ReconcileHandle *recHandle =
	    ReconcileHandle::GetOrCreate( client, true, e );
	if( e->Test() )
	    return;

	recHandle->BeginStage( client, StrRef( "Matching digest" ), CPT_FILES );
	if( count )
	    recHandle->progress->Total( count->Atoi64() );

	StrPtr *matchFile = 0;
	FileSys *f = 0;

	for( int i = 0; 
	     ( matchFile = client->GetVar( StrRef( P4Tag::v_toFile ), i ) );
	     i++ )
	{
	    StrPtr *matchIndex = client->GetVar( StrRef(P4Tag::v_index), i );
	    if( matchIndex &&
	        recHandle->AlreadyMatched( matchIndex->Atoi() ) )
	        continue;

	    delete f;

	    StrVarName path = StrVarName( StrRef( P4Tag::v_toFile ), i );
	    f = ClientSvc::FileFromPath( client, path.Text(), e );

	    // If we encounter a problem with a file, we just don't return
	    // it as a match.  No need to blat out lots of errors.

	    if( e->Test() || !f )
	    {
		e->Clear();
		continue;
	    }

	    int statVal = f->Stat();

	    // Skip files that are symlinks when we
	    // aren't looking for symlinks.

	    if( ( !( statVal & ( FSF_SYMLINK|FSF_EXISTS ) ) )
		|| ( !( statVal & FSF_SYMLINK ) && ( f->IsSymlink() ) )  
                || ( ( statVal & FSF_SYMLINK ) && !( f->IsSymlink() ) ) )
		continue;

	    if( !digest )
	        continue;

	    StrBuf localDigest;
	    f->Translator( ClientSvc::XCharset( client, FromClient ) );
	    recHandle->GetDigest( f, localDigest, e );

	    if( e->Test() )
	    {
		e->Clear();
		continue;
	    }

	    if( !localDigest.XCompare( *digest ) )
	    {
	        // found exact match

	        client->SetVar( P4Tag::v_toFile, matchFile );
	        client->SetVar( P4Tag::v_index, matchIndex );
	        recHandle->SetMatch( matchIndex->Atoi() );
	        recHandle->Increment( client, 1 );
	        break;
	    }

	    // keep the cursor spinning

	    recHandle->Increment( client, 0 );
	}
	delete f;

	client->Confirm( confirm );
}

void
clientOpenMatch( Client *client, ClientFile *f, Error *e )
{
	// Follow on from clientOpenFile, not called by server directly.

	// Grab RPC vars and attach them to the file handle so that
	// clientCloseMatch can use them for N-way diffing.

	StrPtr *fromFile = client->GetVar( P4Tag::v_fromFile, e );
	StrPtr *key	 = client->GetVar( P4Tag::v_key, e );
	StrPtr *flags    = client->GetVar( P4Tag::v_diffFlags );
	if( e->Test() )
	    return;
	
	f->matchDict = new StrBufDict;
	f->matchDict->SetVar( P4Tag::v_fromFile, fromFile );
	f->matchDict->SetVar( P4Tag::v_key, key );
	if( flags )
	    f->matchDict->SetVar( P4Tag::v_diffFlags, flags );
	for( int i = 0 ; ; i++ )
	{
	    StrPtr *index = client->GetVar( 
		    StrRef( P4Tag::v_index ),  i );
	    StrPtr *file  = client->GetVar( 
		    StrRef( P4Tag::v_toFile ), i );
	    if( !index || !file )
		break;
	    f->matchDict->SetVar( 
		   StrRef( P4Tag::v_index ),  i, *index );
	    f->matchDict->SetVar( 
		   StrRef( P4Tag::v_toFile ), i, *file );
	}

	StrPtr *matchlines = client->GetVar( P4Tag::v_matchlines );
	if( matchlines )
	    f->matchDict->SetVar( P4Tag::v_matchlines, matchlines );

	StrPtr *threads = client->GetVar( P4Tag::v_threads );
	if( threads )
	    f->matchDict->SetVar( P4Tag::v_threads, threads );

	StrPtr *count = client->GetVar( P4Tag::v_count );
	if( count )
	    f->matchDict->SetVar( P4Tag::v_count, count );
}

static int
DiffMatchFiles( Sequence &s1,
	        FileSys *f2, Sequence *s2 )
{
	int same = 0;

	DiffAnalyze diff( &s1, s2 );
	for( Snake *s = diff.GetSnake() ; s ; s = s->next )
	    same += ( s->u - s->x );

	s2->Release();
	delete f2;

	return same;
}

# ifdef HAS_CPP11
static void
DiffMatchFilesAsync( FileSys *f1, const Sequence *s1,
	             FileSys *f2, Sequence *s2,
	             const DiffFlags &flags, int *res )
{
	*res = 0;
	Sequence s( *s1, flags );

	Error e;
	s.Reuse( f1, &e );
	if( !e.Test() )
	{
	    *res = DiffMatchFiles( s, f2, s2 );
	    s.Release();
	}
	delete f1;
}
# endif

void
clientCloseMatch( Client *client, ClientFile *f1, Error *e )
{
	// Follow on from clientCloseFile, not called by server directly.

	// Compare temp file to existing client files.  Figure out the
	// best match, along with a quantitative measure of how good
	// the match was (lines matched vs total lines).  Stash it
	// in the handle so clientAckMatch can report it back.

	if( !f1->matchDict )
	{
	    e->Set( MsgSupp::NoParm ) << "clientCloseMatch";
	    return;
	}

	StrPtr *fname;
	FileSys *f2 = 0;
	DiffFlags flags;
	if( StrPtr* diffFlags = f1->matchDict->GetVar( P4Tag::v_diffFlags ) )
	    flags.Init( diffFlags );

	StrPtr *matchlines = f1->matchDict->GetVar( P4Tag::v_matchlines );
	int matchPct = matchlines ? matchlines->Atoi() : 0;

	int bestNum = 0;
	int bestSame = 0; 
	int totalLines = 0;

	ReconcileHandle *recHandle =
	    ReconcileHandle::GetOrCreate( client, true, e );
	if( e->Test() )
	    return;

	recHandle->BeginStage( client, StrRef( "Matching content" ), CPT_FILES );
	StrPtr *count = f1->matchDict->GetVar( P4Tag::v_count );
	if( count )
	    recHandle->progress->Total( count->Atoi64() );

	Timer t; t.Start();
	Sequence s1( f1->file, flags, e );
	recHandle->LogSequenceTimer( t.Time() );

	// If matchlines is not sent by the server
	// both linesLower and linesUpper will be zero

	int linesLower = ( matchPct * s1.Lines() ) / 100;
	int linesUpper = matchPct ? ( ( s1.Lines() * 100 ) / matchPct ) : 0;

# ifdef HAS_CPP11
	const int maxThreads = 32;
	StrPtr *strThreads = f1->matchDict->GetVar( P4Tag::v_threads );
	int threads = strThreads ? strThreads->Atoi() : 1;
	if( threads > maxThreads )
	    threads = maxThreads;

	int c = 0; // thread counter for setting
	int r = 0; // thread counter for retrieval
	std::array< int, maxThreads > res;
	std::vector< std::thread > ts;
	std::vector< int > indices;
# endif

	// Beware of duplicates in the list of files
	// Use a DigestTree to ensure uniqueness

	DigestTree fileSet;

	for( int i = 0; 
	     ( fname = f1->matchDict->GetVar( StrRef( P4Tag::v_toFile ), i ) );
	     i++ )
	{
	    StrPtr *matchIndex = f1->matchDict->GetVar( StrRef( P4Tag::v_index ), i );
	    if( matchIndex &&
	        recHandle->AlreadyMatched( matchIndex->Atoi() ) )
	        continue;

	    StrStr cf( fname->Text(), "" );
	    if( fileSet.Get( &cf ) )
	        continue;
	    else
	        fileSet.Put( &cf, e );

	    f2 = client->GetUi()->File( f1->file->GetType() );
	    f2->SetContentCharSetPriv( f1->file->GetContentCharSetPriv() );
	    f2->Set( *fname );

	    if( e->Test() || !f2 )
	    {
		// don't care
		e->Clear();
		continue;
	    }

	    Sequence *s2 = recHandle->GetSequence( f2, flags, e );
	    totalLines = s1.Lines();

	    if ( e->Test() )
	    {
		// still don't care
		e->Clear();
		continue;
	    }

	    // Skip the optimization if linesLower or linesUpper is 0
	    
	    if( ( !linesLower || !linesUpper ) ||
	        ( s2->Lines() >= linesLower && s2->Lines() <= linesUpper ) )
	    {
	        Timer timer;
	        timer.Start();

# ifdef HAS_CPP11
	        if( threads > 1 )
	        {
	            if( ts.size() == threads )
	            {
	                // Once all threads have started, we wait
	                // for the 1st/oldest thread to finish before
	                // starting a new thread and put it at the end.
	                // At any time, newer threads alway come after
	                // older threads in the vector ts.

	                if( r == threads )
	                    r -= threads;

	                ts.front().join();
	                int same = res[ r++ ];
	                if( same > bestSame )
	                {
	                    bestNum = indices.front();
	                    bestSame = same;
	                }
	                ts.erase( ts.begin() );
	                indices.erase( indices.begin() );
	            }
	            if( c == threads )
	                c -= threads;

	            FileSys *f1c = 0;
	            f1c = client->GetUi()->File( f1->file->GetType() );
	            f1c->SetContentCharSetPriv( f1->file->GetContentCharSetPriv() );
	            f1c->Set( f1->file->Name() );
	            ts.emplace_back( DiffMatchFilesAsync,
	                                 f1c, &s1, f2, s2, flags,
	                                 &res[ c++ ] );
	            indices.push_back( i );
	        }
	        else
	        {
	            int same = DiffMatchFiles( s1, f2, s2 );
	            if( same > bestSame )
	            {
	                bestNum = i;
	                bestSame = same;
	            }
	        }
# else
	        int same = DiffMatchFiles( s1, f2, s2 );
	        if( same > bestSame )
	        {
	            bestNum = i;
	            bestSame = same;
	        }
# endif
	        recHandle->LogDiffTimer( timer.Time() );
	    }
	    else
	    {
	        s2->Release();
	        delete f2;
	    }
	    recHandle->Increment( client, 0 );
	}

# ifdef HAS_CPP11
	if( ts.size() )
	{
	    Timer timer;
	    timer.Start();
	    for( int t = 0; t < ts.size(); t++ )
	    {
	        if( r == threads )
	            r -= threads;

	        ts[ t ].join();
	        int same = res[ r++ ];
	        if( same > bestSame )
	        {
	            bestNum = indices[ t ];
	            bestSame = same;
	        }
	    }
	    ts.clear();
	    indices.clear();
	    recHandle->LogDiffTimer( timer.Time() );
	}
# endif
	f1->file->Close( e );
	recHandle->Increment( client, 1 );

	if( bestSame )
	{
	    f1->matchDict->SetVar( P4Tag::v_index,
	        f1->matchDict->GetVar( StrRef( P4Tag::v_index ), bestNum ) );
	    f1->matchDict->SetVar( P4Tag::v_toFile, 
	        f1->matchDict->GetVar( StrRef( P4Tag::v_toFile ), bestNum ) );

	    f1->matchDict->SetVar( P4Tag::v_lower, bestSame );
	    f1->matchDict->SetVar( P4Tag::v_upper, totalLines );
	}

	// clientAckMatch will send this back
}

void
clientAckMatch( Client *client, Error *e )
{
	StrPtr *handle = client->GetVar( P4Tag::v_handle, e );
	StrPtr *confirm = client->GetVar( P4Tag::v_confirm, e );
	
	if( e->Test() )
	    return;

	// Get handle.

	ClientFile *f = (ClientFile *)client->handles.Get( handle, e );

	if( e->Test() )
	    return;

	// Fire everything back.

	StrPtr *fromFile = f->matchDict->GetVar( P4Tag::v_fromFile );
	StrPtr *key	 = f->matchDict->GetVar( P4Tag::v_key );
	StrPtr *toFile   = f->matchDict->GetVar( P4Tag::v_toFile );
	StrPtr *index    = f->matchDict->GetVar( P4Tag::v_index );
	StrPtr *lower    = f->matchDict->GetVar( P4Tag::v_lower );
	StrPtr *upper    = f->matchDict->GetVar( P4Tag::v_upper );

	if( fromFile && key )
	{
	    client->SetVar( P4Tag::v_fromFile,	fromFile );
	    client->SetVar( P4Tag::v_key,	key );
	}
	else
	{
	    e->Set( MsgSupp::NoParm ) << "fromFile/key";
	    return;
	}
	if( toFile && index && lower && upper )
	{
	    client->SetVar( P4Tag::v_toFile, toFile );
	    client->SetVar( P4Tag::v_index,  index );
	    client->SetVar( P4Tag::v_lower,  lower );
	    client->SetVar( P4Tag::v_upper,  upper );

	    Error err;
	    ReconcileHandle *recHandle =
	        ReconcileHandle::GetOrCreate( client, false, &err );
	    StrPtr *matchlines = f->matchDict->GetVar( P4Tag::v_matchlines );
	    if( recHandle && matchlines )
	    {
	        int matchPct = ( 100 * lower->Atoi() ) / upper->Atoi();
	        if( matchPct >= matchlines->Atoi() )
	        {
	            recHandle->SetMatch( index->Atoi() );
	        }
	    }
	}

	client->Confirm( confirm );
	delete f;	    
}
