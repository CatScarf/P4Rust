
// -*- mode: C++; tab-width: 8; -*-
// vi:ts=8 sw=4 noexpandtab autoindent

/**
 * utils.cc
 *
 * Copyright 2017 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */


// global utility funciton

/**
 * ISHex
 * - return true if c is a valid character for a hex value, 
 * - otherwise return false.
 */

#include "utils.h"

bool
IsHex( char c )
{
	if( ( c >= '0' && c <= '9' ) ||
	    ( c >= 'a' && c <= 'f' ) ||
	    ( c >= 'A' && c <= 'F' ) )
	    return true;
	else
	    return false;
} // IsHex

/**
 * FindRevspec
 * - return a pointer to the beginning of a revspec in a string
 * - return 0 if no revspec is present
 */

const char *
FindRevspec( const char *p, int n )
{
	for( int i = 0; i < n; i++, p++ )
	{
	    if( *p == '#' || *p == '@' )
	        return p;
	}
	return 0;
}
