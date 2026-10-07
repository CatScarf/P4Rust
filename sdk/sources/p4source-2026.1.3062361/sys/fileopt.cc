/*
 * Copyright 1995, 2023 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

/*
 * fileopt.cc -- OS/filesystem-specific part of FileSys (as opposed to filegen.cc)
 */

# include <stdhdrs.h>
# include <strbuf.h>
# include <error.h>
# include <filesys.h>

void FileSys::CopyRange( offL_t offIn, size_t len,
	                 FileSys *targetFile, offL_t offOut, Error *e )
{
	if( len == 0 )
	    return;

	size_t cpLen = len; // Remainder size to copy normally.

	// Error, or all-done.

	if( cpLen == 0 || e->Test() )
	    return;

	// Copy the remainder of the requested range manually.

	const offL_t posIn  = offIn  + len - cpLen;
	const offL_t posOut = offOut + len - cpLen;

	Seek( posIn, e );

	if( e->Test() )
	    return;

	targetFile->Seek( posOut, e );

	if( e->Test() )
	    return;

	const P4INT64 bs = BufferSize();
	StrFixed buf( bs );

	while( !e->Test() && cpLen )
	{
	    const size_t l = Read( buf.Text(), cpLen > bs ? bs : cpLen, e );

	    if( !l || e->Test() )
		break;

	    targetFile->Write( buf.Text(), l, e );
	    cpLen -= l;
	}
}
