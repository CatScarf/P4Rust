/*
 * Copyright 1995, 2003 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

/*
 * fileiozip.cc - FileIOGzip/FileIOGunzip methods
 */

# include <stdhdrs.h>

# include <error.h>
# include <strbuf.h>
# include <gzip.h>
# include <msgsupp.h>

# include "filesys.h"
# include "fileio.h"

FileIOCompress::~FileIOCompress()
{
	Cleanup();
	delete gzip;
	delete gzbuf;
}

void
FileIOCompress::Open( FileOpenMode mode, Error *e )
{
	switch ( GetType() & FST_C_MASK )
	{
	case FST_C_GZIP:
	    compMode = FIOC_GZIP;
	    break;
	case FST_C_GUNZIP:
	    compMode = FIOC_GUNZIP;
	    break;
	default:
	    compMode = FIOC_PASS;
	}

	if( compMode != FIOC_PASS )
	{
	    gzip = new Gzip;
	    gzbuf = new StrFixed( BufferSize() );

	    // Read expects is/ie; write os/oe.

	    gzip->is = gzbuf->Text();
	    gzip->ie = gzbuf->Text();
	    gzip->os = gzbuf->Text();
	    gzip->oe = gzbuf->Text() + gzbuf->Length();
	}

	pos = 0;
	size = -1;

	FileIOBinary::Open( mode, e );

	if( e->Test() )
	{
	    delete gzip;
	    gzip = 0;
	    delete gzbuf;
	    gzbuf = 0;
	}
}

void
FileIOCompress::Write( const char *buf, int len, Error *e )
{
	switch ( compMode )
	{
	case FIOC_PASS:
	    FileIOBinary::Write( buf, len, e );
	    break;

	case FIOC_GZIP:
	    // Write( 0, 0 ) means flush (from Close()).
	    // Don't be fooled by buf, 0.

	    if( buf && !len ) 
		return;

	    gzip->is = buf;
	    gzip->ie = buf + len;

	    // Flush output if full and Compress input
	    // util Compress() indicates input exhausted

	    do if( gzip->OutputFull() )
	       {
		   FileIOBinary::Write( gzbuf->Text(),
			gzip->os - gzbuf->Text(), e );
		   gzip->os = gzbuf->Text();
	       }
	    while( !e->Test() && gzip->Compress( e ) && !gzip->InputEmpty() );
	    break;
	case FIOC_GUNZIP:
	    gzip->is = buf;
	    gzip->ie = buf + len;

	    // Flush output if full and Compress input
	    // util Compress() indicates input exhausted

	    do if( gzip->OutputFull() )
	    {
	        if( loop )
	            loop->WriteLoop( gzbuf->Text(),
	                             gzip->os - gzbuf->Text(), 0, e );
	        else
	            FileIOBinary::Write( gzbuf->Text(),
	                                 gzip->os - gzbuf->Text(), e );

	        gzip->os = gzbuf->Text();
	    }
	    while( !e->Test() && gzip->Uncompress( e ) && !gzip->InputEmpty() );
	    break;
	}
}

void
FileIOCompress::WriteThrough( const char *buf, int len, Error *e )
{
	FileIOBinary::Write( buf, len, e );
}

int
FileIOCompress::Read( char *buf, int len, Error *e )
{
	int done;
	int res;

	switch ( compMode )
	{
	case FIOC_PASS:
	    return FileIOBinary::Read( buf, len, e );

	case FIOC_GZIP:
	    gzip->os = buf;
	    gzip->oe = buf + len;

	    do if( gzip->InputEmpty() )
	       {
		   int l = FileIOBinary::Read( gzbuf->Text(), gzbuf->Length(), e );
		   if( !l )
		       e->Set( E_FAILED, "Unexpected end of file" );
		   gzip->is = gzbuf->Text();
		   gzip->ie = gzbuf->Text() + l;
	       }
	    while( !e->Test() && gzip->Uncompress( e ) && !gzip->OutputFull() );

	    res = gzip->os - buf;
	    pos += res;
	    return res;
	case FIOC_GUNZIP:
	    gzip->os = buf;
	    gzip->oe = buf + len;
	    done = 0;

	    do if( gzip->InputEmpty() && !done )
	       {
		   int l = FileIOBinary::Read( gzbuf->Text(), gzbuf->Length(), e );
		   gzip->is = l ? gzbuf->Text() : 0;
		   gzip->ie = gzbuf->Text() + l;
		   done |= !l;
	       }
	    while( !e->Test() && gzip->Compress( e ) && !gzip->OutputFull() );

	    res = gzip->os - buf;
	    pos += res;
	    return res;
	}

	e->Sys( "read", Name() );
	return -1;
}

void
FileIOCompress::Close( Error *e )
{
	// Flush & clear gzip 

	switch( compMode )
	{
	case FIOC_GZIP:

	    if( gzip && mode == FOM_WRITE )
	    {
		Write( 0, 0, e );
		FileIOBinary::Write( gzbuf->Text(),
				     gzip->os - gzbuf->Text(), e );
	    }
	    break;

	case FIOC_GUNZIP:
	    // Flush & clear gzip

	    if( gzip && mode == FOM_WRITE )
	    {
	        const P4INT64 pending = gzip->os - gzbuf->Text();
	        if( pending > 0 &&
	            pending <= (int)gzbuf->Length() )
	        {
	            if( loop )
	                loop->WriteLoop( gzbuf->Text(), (int)pending, 1, e );
	            else
	                FileIOBinary::Write( gzbuf->Text(), (int)pending, e );
	        }
	        else if( pending != 0 )
	        {
	            // Impossible gzip buffer state: fail rather than flush a
	            // truncated file.
	            if( !e->Test() )
	                e->Set( MsgSupp::InflateBadState )
	                    << (int)pending << gzbuf->Length();
	        }
	    }
	    break;

	case FIOC_PASS:
	    break;
	}

	delete gzip;
	gzip = 0;
	delete gzbuf;
	gzbuf = 0;

	// Rest of normal close

	FileIOBinary::Close( e );
}

void
FileIOCompress::Seek( offL_t offset, Error *e )
{
	if( compMode == FIOC_PASS )
	    FileIOBinary::Seek( offset, e );
	else if( offset - 1 > pos )
	{
	    // Read onto offset
	    while( offset - pos - 1 > 0 )
	    {
	        StrBuf buf = StrFixed( BufferSize() );
	        offL_t diff = offset - pos - 1;
	        if( diff > buf.Length() )
	            diff = buf.Length();

	        if( !Read( buf.Text(), diff, e ) )
	            break;
	    }
	}
	// else set error? The FileSys stub method is quiet on this
}

offL_t
FileIOCompress::GetRealSize()
{
	if( size != -1 )
	    return size;
	
	if( compMode == FIOC_PASS )
	{
	    size = FileIOBinary::GetSize();
	    return size;
	}

	if( FileIOBinary::GetSize() == -1 )
	    return -1; // Not really a file (stdin?)

	Error e;
	int done = 0;

	Gzip gzip2;
	StrFixed gzbuf2( BufferSize() );
	StrFixed buf( BufferSize() );


	// Read expects is/ie; write os/oe.

	gzip2.is = gzbuf2.Text();
	gzip2.ie = gzbuf2.Text();

	FileIOBinary zfile;
	zfile.Set( FileIOBinary::Path()->Text() );
	zfile.Open( FOM_READ, &e );

	if( e.Test() )
	    return -1; // Failed to re-open
	
	offL_t comp = 0;
	offL_t res = 1;
	while( res )
	{
	    gzip2.os = buf.Text();
	    gzip2.oe = buf.Text() + buf.Length();

	    switch( compMode )
	    {
	    case FIOC_GZIP:

		do if( gzip2.InputEmpty() )
		{
		    int l = zfile.Read( gzbuf2.Text(), gzbuf2.Length(), &e );
		    if( !l )
			e.Set( E_FAILED, "Unexpected end of file" );
		    gzip2.is = gzbuf2.Text();
		    gzip2.ie = gzbuf2.Text() + l;
		}
		while( !e.Test() && gzip2.Uncompress( &e ) && !gzip2.OutputFull() );

		comp += res = gzip2.os - buf.Text();
		break;

	    case FIOC_GUNZIP:

		do if( gzip2.InputEmpty() && !done )
		{
		    int l = zfile.Read( gzbuf2.Text(), gzbuf2.Length(), &e );
		    gzip2.is = l ? gzbuf2.Text() : 0;
		    gzip2.ie = gzbuf2.Text() + l;
		    done |= !l;
		}
		while( !e.Test() && gzip2.Compress( &e ) && !gzip2.OutputFull() );

		comp += res = gzip2.os - buf.Text();
		break;
	    }
	}
	size = comp;

	return size;
}
