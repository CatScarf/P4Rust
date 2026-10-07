/*
 * Copyright 2025 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

// Maps a string key to a collection of integers

class StrIntsTree : public VVarTree
{
    public:
	class StrInts
	{
	    public:
	        StrInts() : count( 0 ) {}
	        StrInts( const char *s ) : count( 0 ), key( s ) {}
	        ~StrInts() {}

	        int count;
	        P4INT64Array ints;
	        StrBuf key;

	        void
	        Put( P4INT64 i )
	        {
	            ints[ count++ ] = i;
	        }
	} ;

	StrIntsTree() {}
	virtual ~StrIntsTree()
	{
	    Clear();
	}

	virtual int Compare( const void *a, const void *b ) const
	{
	    const StrInts *ca = (const StrInts *)a;
	    const StrInts *cb = (const StrInts *)b;
	    return ca->key.XCompare( cb->key );
	    
	}
	
	virtual void *Copy( const void *src ) const
	{
	    return CopyOver( new StrInts, src );
	}
	
	virtual void Delete( void *a ) const
	{
	    delete ( StrInts* )a;
	}

	virtual void Dump( void *a, StrBuf &buf ) const
	{
	}

	StrInts *Get( const char* key )
	{
	    StrInts o( key );
	    return ( StrInts* )VVarTree::Get( &o );
	}

	StrInts *Put( const char* key, Error *e )
	{
	    StrInts o( key );
	    return ( StrInts* )VVarTree::Put( &o, e );
	}

    protected:
	virtual void *CopyOver( void *tgt, const void *src ) const
	{
	    StrInts *ca = (StrInts *)tgt;
	    const StrInts *cb = (const StrInts *)src;

	    ca->key = cb->key;
	    for( int i = 0; i < cb->count; i++ )
	        ca->Put( cb->ints.Get( i ) );

	    return ca;
	}
} ;
