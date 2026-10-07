/*
 * Copyright 1995, 2024 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

/*
 * blake3digester.h -- BLAKE3 cryptographic hash function interface
 */

/*
 * BLAKE3 - compute BLAKE3 checksum given a bunch of blocks of data
 */

// This define is a different spelling of the internal BLAKE3 codes
// equivalent.  This one exists so there's no BLAKE3 library leakage
// when this header is imported.

#define BLAKE3_BUFFLEN 32

class BLAKE3
{
    public:
	static void	Digest( const char *p, int n,
			        unsigned char* digest );

			 BLAKE3();
			~BLAKE3();

	// Copy is unimplemented.
# ifdef HAS_CPP11
	BLAKE3&		operator=( const BLAKE3& other ) = delete;
# endif
	
	void		Update( const StrPtr &buf );
	void		Update( const char *buf, int len );
	void		Final( StrBuf &output );
	void 		Final( unsigned char digest[ BLAKE3_BUFFLEN ] );

    private:

	void*		hasher;
};
