/*
 * Copyright 2025 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# if INTPTR_MAX == INT64_MAX

class P4INT64Tree : public VVarTree
{
    public:
	        P4INT64Tree() {}
	virtual ~P4INT64Tree() { Clear(); }

	int Contains( P4INT64 i ) const
	{
	    VarTreeNode *n = Find( ( const void* )i );
	    return n && ( P4INT64 )n->Value() == i;
	}

	void Insert( P4INT64 i, Error *e ) { Put( ( void * )i, e ); }

    protected:
	virtual int Compare( const void *a, const void *b ) const
	{
	    return ( P4INT64 )a - ( P4INT64 )b;
	}
	virtual void * Copy( const void *src ) const
	{
	    return ( void* )src;
	}
	virtual void   Delete( void *a ) const {}
	virtual void   Dump( void *a, StrBuf &buf ) const { buf << ( P4INT64 )a; }
} ;

# else

class P4INT64Tree : public VVarTree
{
    public:
	        P4INT64Tree() {}
	virtual ~P4INT64Tree() { Clear(); }

	int Contains( P4INT64 i ) const
	{
	    VarTreeNode *n = Find( ( const void* )&i );
	    return n && *( P4INT64* )n->Value() == i;
	}

	void Insert( P4INT64 i, Error *e ) { Put( ( void * )&i, e ); }

    protected:
	virtual int Compare( const void *a, const void *b ) const
	{
	    return *( P4INT64* )a - *( P4INT64* )b;
	}
	virtual void * Copy( const void *src ) const
	{
	    P4INT64 *p = new P4INT64;
	    *p = *( const P4INT64* )src;
	    return p;
	}
	virtual void   Delete( void *a ) const { delete ( P4INT64* )a; }
	virtual void   Dump( void *a, StrBuf &buf ) const { buf << *( P4INT64* )a; }
} ;

# endif
