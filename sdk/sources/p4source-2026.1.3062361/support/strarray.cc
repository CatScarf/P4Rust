/*
 * Copyright 1995, 1996 Perforce Software.  All rights reserved.
 */

# include <stdhdrs.h>
# include <strbuf.h>
# include <error.h>
# include <debug.h>
# include <vararray.h>
# include <strarray.h>
# include <msgdb.h>

# include <new>

# define DEBUG_RECORDS  ( p4debug.GetLevel( DT_RECORDS ) >= 4 )
# define DEBUG_EXTEND   ( p4debug.GetLevel( DT_RECORDS ) >= 5 )

/*
 * strarray.cc - a 0 based array of StrBufs
 */

class StrVarArray : public VVarArray {

    public:

	/**
	 * StrVarArray::StrVarArray -
	 * Constructor
	 */
	StrVarArray( )
	    : VVarArray(),
	    caseFolding( 0 )
	{
	}

	/**
	 * StrVarArray::StrVarArray -
	 * Constructor. Allows caller to preallocate space if populating an 
	 * array of known size.
	 *
	 * @param[in] max - Specific maximum size to allocate for this new 
	 *                  array.
	 */
	StrVarArray( int max ) 
	    : VVarArray( max ),
	    caseFolding(0)
	{
	}

	void SetCaseFolding( int c ) 
	{
	    caseFolding = c;
	}

	virtual int Compare( const void *a, const void *b ) const 
	{
	    return caseFolding 
		    ? ((StrBuf *)a)->XCompare( *(StrBuf *)b )
		    : ((StrBuf *)a)->CCompare( *(StrBuf *)b );
	}

	virtual void	Destroy( void * ) const {}

	int caseFolding;

} ;

/**
 * StrArray::StrArray -
 * Constructor
 */
StrArray::StrArray()
    : array(new StrVarArray)
{
}

/**
 * StrArray::StrArray -
 * Constructor. Allows caller to preallocate space if populating an array of
 * known size.
 *
 * @param[in] max - Specific maximum size to allocate for this new array.
 */
StrArray::StrArray( int max )
    : array(new StrVarArray(max))
{

}

StrArray::~StrArray()
{
	for( int i = 0; i < array->Count(); i++ )
	    delete (StrBuf *)array->Get(i);

	delete array;
}

void
StrArray::Clear()
{
	for( int i = 0; i < array->Count(); i++ )
	    delete (StrBuf *)array->Get(i);
	array->Clear();
}

const StrBuf *
StrArray::Get( int i ) const
{
	return (StrBuf *)array->Get(i);
}

StrBuf *
StrArray::Edit( int i )
{
	return (StrBuf *)array->Edit(i);
}

void
StrArray::Remove( int i )
{
	if( array->Get( i ) )
	{
	    delete Edit( i );
	    array->Remove( i );
	}
}

StrBuf *
StrArray::Put()
{
	return (StrBuf *)array->Put( new StrBuf );
}

int
StrArray::Count() const
{
	return array->Count();
}

void
StrArray::Sort( int caseFolding )
{
	array->SetCaseFolding( caseFolding );
	array->Sort();
}

int
StrArray::Search( const StrBuf *key )
{
	// Ye old binary search.
	// "Binary search routines are never written right the first time
	// around" - Robert G Sheldon.
	// This one has been fixed _3_ times since it was written.

	// Search until we converge on the first record >= the key.
	// Zero based indexing; 'hi' is one beyond the last valid record.

	int lo = 0;
	int hi = Count();

	for(;;)
	{
	    int index = ( lo + hi ) / 2;

	    // lo <= hi
	    // if index == hi then lo == hi
	    // if index == lo then hi == lo or hi == lo + 1

	    // Return if we've converged.

	    if( lo == hi )
		return index;

	    int cmp = array->Compare( key, Get( index ) );

	    // If cmp == 0 we treat it as cmp < 0 -- we're positioning
	    // before the first matching record.

	    // If cmp > 0 && index == lo we're converging one past this
	    // current record.  Set lo = hi so that we'll return index = hi.

	    if( cmp <= 0 )
		hi = index;
	    else if( index != lo )
		lo = index;
	    else
		lo = hi;
	}
}

int
StrArray::GetIndex( const StrBuf *key )
{
	int index = Search( key );
	const StrBuf *r = Get( index );

	if( !r || array->Compare( key, r ) )
	    return -1;

	return index;
}

const StrBuf *
StrArray::Find( const StrBuf *key )
{
	// Handy wrapper for Search() for when you just expect one record.
	// Return one matching record if found, otherwise return 0.

	int index = Search( key );
	const StrBuf *r = Get( index );

	if( r && array->Compare( key, r ) )
	    return 0;

	return r;
}

void
StrArray::Copy( const StrArray *other )
{
	for( int i = 0; i < other->Count(); i++ )
	    Put()->Set( other->Get( i ) );
}

/*
 * StrPtrArray -- an array of StrPtrs
 */

StrPtrArray::StrPtrArray()
{
	tabVal = 0;
	tabSize = 0;
	tabLength = 0;
}

/**
 * StrPtrArray::StrPtrArray -
 * Creates a StrPtrArray and pre-allocates storage for the specified number of
 * StrPtrs. Space is allocated without any additional padding, so the first 
 * time someone tries to add more values than the specified size, we will need 
 * to re-allocate the array.
 * 
 * This is optimized for scenarios when you are creating a StrPtrArray and you
 * know exactly how many values will be placed into that array.
 *
 * @param[in] size - Number of StrPtrs to pre-allocate space for.
 */
StrPtrArray::StrPtrArray( int size )
	: tabVal(0),
	  tabSize(0),
	  tabLength(0)
{
	Extend( size );
}

StrPtrArray::~StrPtrArray()
{
	delete []tabVal;
}

/**
 * StrPtrArray::Put -
 * Puts the provided StrPtr into the array, growing the array storage size
 * as needed.
 *
 * @param[in] val - Value to store into the array
 * @param[out] e  - Optional err object. If the 'Put' fails, and this error
 *		    was provided, it will be set to an appropriate message
 * @return bool - Returns true if the value was stored into the array, false
 *	          if we failed to extend the array to accommodate the new value.
 */
bool
StrPtrArray::Put( const StrPtr &val, Error* err /* = 0 */)
{
	if( tabLength == tabSize )
	{
	    // Realloc with spare room

	    // grow geometrically, please
	    int newSize = (tabSize + 50) * 3 / 2;
	    if( !Extend( newSize, err ) ) return false;
	}

	tabVal[ tabLength++ ] = val;

	return true;
}

/**
 * StrPtrArray::Extend -
 * If the 'newSize' is larger than the 'tabSize', this will allocate enough
 * internal storage to hold 'newSize' StrRefs. It will then transfer all data
 * from the old array to the new array.
 * 
 * If the allocation fails due to an out-of-memory error, this will return
 * false.
 *
 * @param[in] newSize - New array size to allocate in memory.
 * @param[out] e  - Optional err object. If the 'Extend' fails, and this error
 *		    was provided, it will be set to an appropriate message
 * @return bool - True if the memory allocation was successful, false otherwise.
 */
bool
StrPtrArray::Extend( int newSize, Error* e /* = 0 */ )
{
	if( newSize > tabSize )
	{
	    // Attempt to allocate the new array, return false on failure
	    StrRef* newtabVal = 0;
	    try { 
		newtabVal = new StrRef[ newSize ];
	    } 
	    catch( const std::bad_alloc& ex ) 
	    { 
		if( e ) e->Set( MsgDb::MaxMemory );
		return false; 
	    }

	    if( tabVal )
	    {
	        for( int i = 0; i < tabSize; i++ )
	            newtabVal[ i ] = tabVal[ i ];

	        delete[]tabVal;
	    }

	    tabVal = newtabVal;
	    tabSize = newSize;

	    if( DEBUG_EXTEND )
	        p4debug.printf( "StrPtrArray extend %d\n", tabSize );
	}

	return true;
}
