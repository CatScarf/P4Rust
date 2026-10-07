/*
 * Copyright 1995, 2024 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# include <stdhdrs.h>
# include <error.h>
# include <strbuf.h>
# include <strops.h>
# include <blake3.h>
# include "blake3digester.h"

void
BLAKE3::Digest( const char *p, int n, unsigned char* digest )
{
	blake3_hasher hasher;
	blake3_hasher_init( &hasher );
	blake3_hasher_update( &hasher, p, n );
	blake3_hasher_finalize( &hasher, digest, BLAKE3_BUFFLEN );
}

BLAKE3::BLAKE3()
{
# if defined(_DEBUG)
	// BLAKE3_BUFFLEN is ours, BLAKE3_OUT_LEN is BLAKE3's.
	static_assert( BLAKE3_BUFFLEN == BLAKE3_OUT_LEN,
	               "BLAKE3_BUFFLEN size definition wrong." );
# endif

	hasher = (blake3_hasher*) new blake3_hasher;
	blake3_hasher_init( (blake3_hasher*) hasher );
}

BLAKE3::~BLAKE3()
{
	delete (blake3_hasher*) hasher;
}

void
BLAKE3::Update( const StrPtr &buf )
{
	blake3_hasher_update( (blake3_hasher*) hasher, buf.Text(),
	                      buf.Length() );
}

void
BLAKE3::Update( const char *buf, int len )
{
    blake3_hasher_update( (blake3_hasher*) hasher, buf, len );
}

void
BLAKE3::Final( StrBuf &output )
{
	output.Clear();
	unsigned char digest[ BLAKE3_OUT_LEN ];
	blake3_hasher_finalize( (blake3_hasher*) hasher, digest,
	                        BLAKE3_OUT_LEN );
	StrOps::OtoX( digest, BLAKE3_OUT_LEN, output );
}

void
BLAKE3::Final( unsigned char digest[ BLAKE3_BUFFLEN ] )
{
	blake3_hasher_finalize( (blake3_hasher*) hasher, digest,
	                        BLAKE3_OUT_LEN );
}
