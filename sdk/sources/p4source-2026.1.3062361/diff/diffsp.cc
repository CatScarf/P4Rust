/*
 * Copyright 1997 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 *
 * Diff code written by James Strickland, May 1997.
 */

# include <new>

# define NEED_TYPES

#include <stdhdrs.h>
#include <error.h>
#include <strbuf.h>
#include <readfile.h>

#include "diff.h"
#include "diffsp.h"
#include "diffsr.h"

/*
 * Sequence - a file as an abstract sequence of elements
 *
 * Sequence just manages the element list.  The Sequencers do
 * the actual file reading/hashing/comparing.
 */

/*
 * Sequence::Sequence() - open file and hash lines
 */

Sequence::Sequence( FileSys *f, const DiffFlags &flags, Error *e )
{
	line = 0;
	lineCount = 0;
	lineMax = 0;
	reallocCount = 0; 
	sequencer = 0;

	readfile = new ReadFile;

	// Build list of line hashes.

	switch( flags.sequence )
	{
	case DiffFlags::Line:	sequencer = new LineReader; break;
	case DiffFlags::Word:	sequencer = new WordReader; break;
	case DiffFlags::WClass:	sequencer = new WClassReader; break;
	case DiffFlags::DashL:	sequencer = new DifflReader; break;
	case DiffFlags::DashB:	sequencer = new DiffbReader; break;
	case DiffFlags::DashW:	sequencer = new DiffwReader; break;
	}

	// We open, ~Sequence() closes.

	sequencer->A = this;
	sequencer->src = readfile;
	readfile->Open( f, e );

	if( e->Test() )
	    return;

	// allocate initial space

	GrowLineBuf( e ); 

	if( e->Test() )
	    return;

	line[0].offset = 0 ;
	line[1].offset = 0 ;

	// Load lines

	sequencer->Load( e );
}

Sequence::Sequence( const Sequence &other, const DiffFlags &flags )
{
	lineCount = other.lineCount;
	lineMax = other.lineMax;
	line = new VarInfo[ lineMax ];
	memcpy( line, other.line, sizeof( VarInfo ) * lineMax );

	reallocCount = 0;
	sequencer = 0;

	readfile = new ReadFile;

	switch( flags.sequence )
	{
	case DiffFlags::Line:	sequencer = new LineReader; break;
	case DiffFlags::Word:	sequencer = new WordReader; break;
	case DiffFlags::WClass:	sequencer = new WClassReader; break;
	case DiffFlags::DashL:	sequencer = new DifflReader; break;
	case DiffFlags::DashB:	sequencer = new DiffbReader; break;
	case DiffFlags::DashW:	sequencer = new DiffwReader; break;
	}

	sequencer->A = this;
	sequencer->src = readfile;
	// Must call Reuse() before using this instance
}

void
Sequence::Release()
{
	readfile->Close();
}

void
Sequence::Reuse( FileSys *f, Error *e )
{
	readfile->Open( f, e );
}

/*
 * Sequence::~Sequence() - close file
 */

Sequence::~Sequence()
{
	delete sequencer;
	delete readfile;
	if( line ) delete[] line;
}

/*
 * Sequence::StoreLine() - add hash/offset to list
 */

void 
Sequence::StoreLine( HashVal HashValue, Error *e )
{
	// we need to store indices from 0..n+1
	// give us more space!

	if( lineCount+1 >= lineMax ) 
	    GrowLineBuf( e ); 

	if( e->Test() )
	    return;

	line[ lineCount ].hash = HashValue;
	line[ lineCount+1 ].offset  = readfile->Tell();

	++lineCount;
}

/*
 * Sequence::GrowLineBuf() - grow hash/offset list table
 */

void 
Sequence::GrowLineBuf( Error *e )
{
	const LineNo prevLineMax = lineMax;
	LineNo CharsPerLine;

	/*
	 * We try to gues right.
	 *
	 * First: size in bytes/"guess" of # of chars per line
	 * Second: size in bytes/average # of chars per line
	 * After that: just double each time
	 */

	switch( reallocCount++ ) 
	{
	case 0: 
	    // allocate initial space: 32 = numCharsPerLine guess
	    lineMax = 200 + readfile->Size() / 32;
	    break;

	case 1: 
	    // reallocate based on actual number of characters per line
	    CharsPerLine = Off( lineCount ) / lineCount;
	    lineMax = readfile->Size() / 10 * 13 / CharsPerLine;
	    break;

	default:
	    lineMax *= 2;
	    break;
	}

	try
	{
	    if( line )
	    {
	        VarInfo *lt = new VarInfo[ lineMax ];
	        memcpy( lt, line, sizeof( VarInfo ) * prevLineMax );
	        delete[] line;
	        line = lt;
	    }
	    else
	        line = new VarInfo[ lineMax ];
	} catch( const std::bad_alloc& ex )
	{
	    e->Sys( "new", "out of memory" );
	}
}

/*
 * Sequence::CopyLines() - copy given lines into buffer
 */

int
Sequence::CopyLines( 
	LineNo &l,
	LineNo m,
	char *buf,
	int len,
	LineType lineType )
{
	// Don't go past the end of the file!

	if( Lines() < m )
	    m = Lines();

	// Copy what we can

	len = readfile->Textcpy( buf, len, LengthLeft( m - 1 ), lineType );

	// Did we finish?

	if( !LengthLeft( m - 1 ) )
	    l = m;

	return len;
}

/*
 * Sequence::Dump() - copy given lines into FILE
 *   returns true if the last line ends with a newline,
 *   flase if not.
 */

bool
Sequence::Dump( FILE *out, LineNo l, LineNo m, LineType lineType )
{
	int len;
	int llen = 0;
	char buf[ 1024 ];

	while( ( len = CopyLines( l, m, buf, sizeof( buf ), lineType ) ) )
	{
		fwrite( buf, 1, len, out );
		llen = len;
	}

	return( ( llen > 0 && buf[ llen - 1 ] != '\n' ) ? false : true );


}

