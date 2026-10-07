/*
 * Copyright 1995, 1996 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

/*
 * intarray.h -- a cheesy, zero-based, automatically resized int array
 */

class IntArray {

    public:
		IntArray( int nInts = 4, int def = 0 )
		{
		    ints = 0;
		    lInts = 0;
		    count = 0;
		    this->def = def;
		    ReAlloc( nInts );
		}

		~IntArray()
		{
		    delete []ints;
		}

	int& operator []( int x ) 
		{
		    // keep track of count
		    if( ( x + 1 ) > count )
		        count = x + 1;

		    // geometric growth
		    if( x >= lInts ) ReAlloc( x * 3 / 2 );
		    return ints[x];
		}

	// Return unreferenced value
	int Get( int x ) 
		{
		    return (*this)[x];
		}

	void Set( int x, int value ) 
		{
		    int& index = (*this)[x];
		    index = value;
		}

	void	SetCount( int ct ) { count = ct; }
	int	Find( int v );
	bool	Contains( int v ) { return Find( v ) != -1; }
	void	Reset()
		{
		    for( int i = 0; i < lInts; i++ ) ints[i] = def;
		}
	void	Sort();

	int	Count() { return count; }

    private:

	void	ReAlloc( int nInts )
		{
		    int i = 0;
		    int *old = ints;
		    ints = new int[nInts];
		    for( ; i < lInts; i++ ) ints[i] = old[i];
		    for( ; i < nInts; i++ ) ints[i] = def;
		    lInts = nInts;
		    delete []old;
		}

	int	lInts;
	int	*ints;
	int	count; // only used by Find()/Contains()
	int	def; // default value
};

class P4INT64Array {

    public:
		P4INT64Array( int nInts = 4, P4INT64 def = 0 )
		{
		    ints = 0;
		    lInts = 0;
		    count = 0;
		    this->def = def;
		    ReAlloc( nInts );
		}

		~P4INT64Array()
		{
		    delete []ints;
		}

	P4INT64& operator []( int x ) 
		{
		    // keep track of count
		    if( ( x + 1 ) > count )
		        count = x + 1;

		    // geometric growth
		    if( x >= lInts ) ReAlloc( x * 3 / 2 );
		    return ints[x];
		}

	P4INT64 Get( int x ) const
		{
		    return ints[ x ];
		}

	void	SetCount( int ct ) { count = ct; }
	int	Find( P4INT64 v );
	bool	Contains( P4INT64 v ) { return Find( v ) != -1; }
	void	Reset()
		{
		    for( int i = 0; i < lInts; i++ ) ints[i] = def;
		}

	int	Count() { return count; }

    private:

	void	ReAlloc( int nInts )
		{
		    int i = 0;
		    P4INT64 *old = ints;
		    ints = new P4INT64[nInts];
		    for( ; i < lInts; i++ ) ints[i] = old[i];
		    for( ; i < nInts; i++ ) ints[i] = def;
		    lInts = nInts;
		    delete []old;
		}

	int	lInts;
	P4INT64	*ints;
	int	count; // only used by Find()/Contains()
	P4INT64	def; // default value
};
