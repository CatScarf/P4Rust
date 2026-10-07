/*
 * Copyright 1995, 2022 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# define NEED_ERRNO
# define NEED_WINDOWSH
# define NEED_PSAPIH
# include <stdhdrs.h>

# ifdef HAS_CPP11

# include <strbuf.h>
# include <error.h>
# include <debug.h>
# include <filesys.h>
# include <hostcfg.h>

# include "proc_utils.h"

OSProcessUtils::OSProcessUtils() : isHidden( false )
{
}

OSProcessUtils::~OSProcessUtils()
{
	Error eIgnore;
	ResetOOMKillerStatus( &eIgnore );
}

void OSProcessUtils::WriteOOMFile( const StrPtr* buf, Error* e ) const
{
#ifdef OS_LINUX
	// This doesn't use FileSys since FileSys::Close() insists upon doing
	// a chmod(), which we don't want because it isn't permitted.

	FILE* fh = fopen( HostCfg::OOMScoreAdjFile()->Text(), "w" );

	if( !fh )
	{
	    e->Sys( "OSProcessUtils::WriteOOMFile fopen: ", strerror( errno ) );
	    return;
	}

	// Note that this won't work under strace, and that the return code
	// strace reports is not what we get here - strace might say
	// '4426 write(4, "-999\n", 5) = -1 EACCES (Permission denied)'
	// but we get 1 returned here.

	const int fwrc = fwrite( buf->Text(), buf->Length(), 1, fh );

	if( fwrc != 1 )
	{
	    StrBuf msg;
	    msg << buf->Length() << " " << fwrc << ", '"
	        << strerror( errno ) << "'";
	    e->Sys( "OSProcessUtils::WriteOOMFile write ", msg.Text() );
	}

	const int fcrc = fclose( fh );

	if( fcrc )
	    e->Sys( "OSProcessUtils::WriteOOMFile close: ", strerror( errno ) );
#endif
}

void OSProcessUtils::ReadOOMFile( StrBuf& buf, Error* e ) const
{
	FileSysUPtr osa = FileSys::CreateUPtr( FST_TEXT );
	osa->Set( HostCfg::OOMScoreAdjFile()->Text() );
	osa->ReadFile( &buf, e );

	if( e->Test() )
	   e->Sys( "OSProcessUtils::ReadOOMFile", "" );
}

bool OSProcessUtils::HideFromOOMKiller( Error* e, const bool verify = false )
{
	if( !HostCfg::CanOOMScoreAdj() || isHidden )
	    return true;

	return HideFromOOMKillerRaw( e, verify );
}

bool OSProcessUtils::HideFromOOMKillerRaw( Error* e, const bool verify = false )
{
	// This has to include the trailing newline since one is
	// included on reads, and in order to compare the two
	// for verification they must be be formatted the same.

	const StrPtr* v( HostCfg::GetHiddenOOMScoreAdj() );

	WriteOOMFile( v, e );

	// If it failed, still say it's hidden since there are probably
	// more calls to this function coming and this will keep them
	// all from retrying.  Doing this causes ResetOOMKillerStatus()
	// to run, but that's better and more likely to pass.

	isHidden = true;

	if( e->Test() )
	    return false;

	if( verify )
	{
	    StrBuf buf;
	    ReadOOMFile( buf, e );

	    if( e->Test() )
	    {
	        e->Sys( "OSProcessUtils::HideFromOOMKiller verify read", "" );
	        return false;
	    }

	    if( buf != *v )
	    {
	        p4debug.printf( "OSProcessUtils::HideFromOOMKiller bad"
	                        " oom_score_adj write: wrote '%s', read"
	                        " '%s'\n", v->Text(), buf.Text() );
	        return false;
	    }
	}

	return true;
}

bool OSProcessUtils::TestHideFromOOMKiller()
{
	// Note that if HideFromOOMKiller's default value is raised
	// from -1000 (the minimum allowed value) and the process
	// default value is lower than HideFromOOMKiller's setting,
	// that this check is reduced in strength from "we can set the
	// whole allowed range", to "we are allowed to set some values".
	// That's probably ok since the value we set is not parameterized
	// anywhere.

	// Note that this function actually enables the proection temporarily.

	Error eIgnore;

	const bool r = HideFromOOMKillerRaw( &eIgnore, true );
	ResetOOMKillerStatus( &eIgnore );
	return r;
}

void OSProcessUtils::ResetOOMKillerStatus( Error* e )
{
	if( !isHidden )
	    return;

	WriteOOMFile( HostCfg::GetInitialOOMScoreAdj(), e );
	// If resetting failed, oh well.
	isHidden = false;
}

# ifdef OS_NT

// Return values,
//  0 - fetch of file path failed.
//  1 - fetch of file path succeeded.
//
int
OSProcessUtils::GetCurrentExecutablePath( StrBuf *exename, Error *e )
{
	HANDLE hProcess;
	DWORD ret;

	hProcess = GetCurrentProcess();

	ret = GetModuleFileNameEx (
	            hProcess,
	            NULL,
	            exename->Text(),
	            exename->Length()
	);
	if( ret == 0 )
	{
	    e->Sys( "GetCurrentExecutablePath", "GetModuleFileNameEx" );
	    return 0;
	}

	return 1;
}

# endif

# endif // HAS_CPP11

