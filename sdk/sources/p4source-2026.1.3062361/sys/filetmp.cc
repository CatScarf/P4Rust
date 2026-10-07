/*
 * Copyright 1995, 1996 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# define NEED_SLEEP

# include <stdhdrs.h>
# include <pid.h>

# ifdef HAS_CPP11
# include <string>
# include <sstream>
# include <thread>
# endif

# if defined(OS_OS2) || defined(OS_NT)
const char *lclTemp = "t%dt%dt%d.tmp";
const char *gblTemp = ".";
# define HAVE_TEMP
# endif // OS_OS2 || OS_NT

# ifdef OS_VMS
# ifdef VI_WORKS_WITH_DIR
const char *lclTemp = "t%d%d.tmp";
const char *gblTemp = "sys$scratch:";
# else
const char *lclTemp = "t%d%d.tmp";
const char *gblTemp = "";
# define HAVE_TEMP
# endif
# endif

# ifndef HAVE_TEMP
# ifdef HAS_CPP11
const char *lclTemp = "tmp.%d.%s.%d";
const char *gblTemp = "/tmp";
# else
const char *lclTemp = "tmp.%d.%d";
const char *gblTemp = "/tmp";
# endif
# endif

# include <error.h>
# include <strbuf.h>
# include <random.h>
# include <debug.h>
# include <tunable.h>

# include "enviro.h"
# include "pathsys.h"
# include "filesys.h"
# include "datetime.h"
# include "msgsupp.h"

/*
 * FileSys::TempName() - create a temp name in target buffer
 */

void
FileSys::TempName( char *buf )
{
	// Format temp file name

	MT_STATIC int count = 0;

# ifdef OS_OS2
	const int maxTemp = 1000;
# else
	const int maxTemp = p4tunable.Get( P4TUNE_FILESYS_MAXTMP );
# endif

	count = ( count + Random::Integer( 1, 100 ) ) % maxTemp;

# if defined(OS_OS2) || defined(OS_NT)
	sprintf( buf, lclTemp, Pid().GetProcID(), Pid().GetID(), count );
# else // OS_OS2 || OS_NT

# ifdef HAS_CPP11
	std::stringstream ss;
	ss << std::this_thread::get_id();
	sprintf( buf, lclTemp, Pid().GetProcID(), ss.str().c_str(), count );
# else
	sprintf( buf, lclTemp, Pid().GetID(), count );
# endif

# endif // OS_OS2 || OS_NT
}

/*
 * FileSys::MakeLocalTemp() - turn file path into a temp file in same dir
 */

void
FileSys::MakeLocalTemp( char *file )
{
	// Make tempname

	char buf[50];

	PathSys *p = PathSys::Create();
	p->SetCharSet( GetCharSetPriv() );

	int cnt = p4tunable.Get( P4TUNE_FILESYS_MAKETMP );

	do
	{
	    TempName( buf );

	    // Append it to path

	    p->Set( file );
	    p->ToParent();
	    p->SetLocal( *p, StrRef( buf ) );

	    Set( *p );
	} while ( ( Stat() & ( FSF_SYMLINK | FSF_EXISTS ) ) && --cnt > 0 );

	delete p;
}

/*
 * FileSys::MakeGlobalTemp() - create a temp file path in temp dir
 */

void
FileSys::MakeGlobalTemp()
{
	const char *tdir;

	// Use $TEMP or $TMP if given.

	Enviro enviro;
	enviro.SetCharSet( GetCharSetPriv() );

	if( 
# ifdef OS_INTERIX
	    !( tdir = enviro.Get( "TMPDIR" ) ) &&
# endif
	    !( tdir = enviro.Get( "TEMP" ) ) && 
	    !( tdir = enviro.Get( "TMP" ) ) )
		tdir = gblTemp;

	// Make tempname
	char buf[50];

	PathSys *p = PathSys::Create();
	p->SetCharSet( GetCharSetPriv() );

	int cnt = 10;

	do
	{
	    TempName( buf );

	    // Append to global temp dir

	    p->Set( tdir );
	    p->SetLocal( *p, StrRef( buf ) );

	    Set( *p );
	} while ( ( Stat() & ( FSF_SYMLINK | FSF_EXISTS ) ) && --cnt > 0 );

	delete p;
}

// This creates a lock file ment to be very temporary
FileSys *
FileSys::CreateLock( FileSys *f, Error *e )
{
	int t;
	StrBuf nameit;
	DateTime nowtime, locktime;

	// We can get called with 'e' set already, which
	// confuses the result of the Open() call below
	// and causes a lock failure. Guard
	// against this by using a local 'e'.

	FileSys *lockFile = Create( (FileSysType)( FST_BINARY|FST_M_EXCL ) );

	nameit.Set( f->Path() );
	nameit.UAppend( ".lck" );

	lockFile->Set( nameit );

	int lcnt = p4tunable.Get( P4TUNE_FILESYS_LOCKTRY );

	while( --lcnt > 0 )
	{
	    if( ( t = lockFile->StatModTime() ) > 0 )
	    {
		// file exists
		locktime.Set( t );
		nowtime.SetNow();
		if( nowtime.Compare( locktime ) > p4tunable.Get( P4TUNE_FILESYS_LOCKDELAY ) )
		{
		    Error locale;
		    // lock file old
		    lockFile->Unlink( &locale );
		    if( locale.Test() )
		    {
			e->Merge( locale );
			delete lockFile;
			return NULL;
		    }
		}
		else
		{
		    // lock file exists, reasonably new, we should wait
		    sleep( 1 );
		}
	    }
	    else
	    {
		// file does not seem to exist, try create
		Error locale;
		lockFile->Open( FOM_WRITE, &locale );

		if( !locale.Test() )
		{
		    lockFile->SetDeleteOnClose();
		    return lockFile;
		}
		// if open failed, we try again
	        if( lockFile->RetryCreate() )
	            sleep( 1 );
	        else
	        {
	            StrBuf einfo;
	            locale.Fmt( einfo, 0 );
	            e->Set( MsgSupp::FatalLockError ) <<
	                nameit << einfo;
	            delete lockFile;
	            return NULL;
	        }
	    }
	}

	// too many retries, error
	// set error
	e->Set( MsgSupp::TooManyLockTrys ) << nameit;
	delete lockFile;
	return NULL;
}
