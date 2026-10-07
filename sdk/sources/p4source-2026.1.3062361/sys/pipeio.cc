/*
 * Copyright 1995, 2023 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# define NEED_SOCKETPAIR

# include <stdhdrs.h>

# include <strbuf.h>
# include <error.h>

# include <pipeio.h>

#include "debug.h"
#include "filesys.h"
#include "tunable.h"

PipeIo::PipeIo(StrPtr path)
{
	pipe = FileSys::Create( FST_BINARY );
	pipe->Set( path );
	open = false;
}

PipeIo::~PipeIo()
{
	delete pipe;
}

void
PipeIo::Open( Error* e )
{
	pipe->Open( FOM_RW, e );
	if( !e->Test() )
	    open = true;
}

void
PipeIo::Write( const StrPtr &in, Error *e )
{
	if( open )
	    pipe->Write( in.Text(), in.Length(), e );
}

p4size_t
PipeIo::Read( StrBuf &out, Error *e )
{
	if( !open )
	    return 0;

	p4size_t size = (p4size_t)p4tunable.Get( P4TUNE_FILESYS_BUFSIZE );
	if( out.BufSize() < size )
	    out.Alloc( size - out.BufSize() );

	p4size_t read = pipe->Read( out.Text(), size, e );
	out.SetLength( read );
	return read;
}

p4size_t
PipeIo::Peek( Error *e )
{
	if( !open )
	    return 0;

	unsigned long readable = 0;

# ifdef OS_NT
	if( !PeekNamedPipe( pipe->GetFd(), 0, 0, 0, &readable, 0 ) )
	{
	    DWORD dwError = GetLastError();

	    if( dwError != ERROR_BROKEN_PIPE &&
	        dwError != ERROR_PIPE_NOT_CONNECTED &&
	        dwError != ERROR_INVALID_HANDLE )
	    {
	        e->Sys( "PeekNamedPipe", "stdio" );
	        return 0;
	    }
	}
# endif

	return readable;
}

void
PipeIo::Close( Error *e )
{
	if( open )
	    pipe->Close( e );
	open = false;
}

p4size_t
PipeIo::ReadLine( StrBuf &out, StrBuf &buf, Error *e )
{
	if( !open )
	    return 0;

	// If anything buffered, return it first.

	int len = 4096;
	char *c, *s;
	while ( ( s = buf.Text() ) && !( c = strchr( s, '\n' ) ) )
	{
	    // Read pipe into result buffer; no closing
	    int oldLen = buf.Length();
	    int space = buf.BufSize() - buf.Length();
	    int read = 0;
	    if( space > 0 )
	        read = pipe->Read( buf.Text() + oldLen, space, e );
	    else
	        read = pipe->Read( buf.Alloc(len), len, e );
	    buf.SetLength( read + oldLen );
	    if( read <= 0 )
	        buf << "\n";
	}

	out.Set( s, c - s );
	out.Terminate();
	StrBuf tmp = c + 1;
	buf = tmp;
	return out.Length();
}
