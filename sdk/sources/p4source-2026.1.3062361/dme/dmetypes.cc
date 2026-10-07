/*
 * Copyright 2019 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# include <stdhdrs.h>
# include <strbuf.h>
# include <error.h>
# include <msgserver2.h>
# include <charman.h>
# include <dmetypes.h>

# include <ctype.h>


/*
 * DmtExtName - Extension name formatting
 * 
 * Format stored for
 *    DET_FULLNAME:    ExampleInc::ExtName
 *    DET_PREFIX:      ExampleInc
 *    DET_NAME:        ExtName
 */

bool
DmtExtName::ParseExt( const StrPtr &name, DmtExtType t, Error *e )
{
	const char *ptr = name.Text();
	const char *ptrc = strstr( ptr, "::" );
	
	// For FULLNAME, minimum length is 4 ("a::b") and first character cannot be a dash
	if( t == DET_FULLNAME && !ptrc ) 
	{
	    e->Set( MsgServer2::InvalidExtName ) << name;
	    return false;
	}

	// Filter invalid ("::" or "a::" or "::b")
	if( strlen(ptr) <= 3 && ptrc )
	{
	    e->Set( MsgServer2::InvalidExtName ) << name;
	    return false;
	}

	if( *ptr == '-' )
	{
	    e->Set( MsgServer2::InvalidExtName ) << name;
	    return false;
	}

	if( ptrc || t == DET_PREFIX )
	{
	    while( *ptr && *ptr != ':' )
	    {
	        if( !isAalnum( ptr ) )
	        {
	            e->Set( MsgServer2::InvalidExtName ) << name;
	            return false;
	        }
	        ++ptr;
	    }
	    if( *ptr == ':' )
	    {
	        // Then must be "::"
	        if( strncmp( ptr, "::", 2 ) )
	        {
	            e->Set( MsgServer2::InvalidExtName ) << name;
	            return false;
	        }
	        ptr += 2;
	    }
	}

	if( t == DET_PREFIX )
	{
	    // Cover cases where name is just ExampleInc (prefix) or
	    // where name is full (ExampleInc::ExtName)
	    ptr = name.Text();

	    if( ptrc )
	    {
	        int len = ptrc - ptr;
	        StrBuf::Set( ptr, len );
	        return true;
	    }

	    StrBuf::Set( name );
	    return true;
	}

	StrBuf ename( ptr );
	bool nameNumeric = true;

	while( *ptr )
	{
	    if( *ptr < '0' || *ptr > '9' )
	    {
	        nameNumeric = false;
	        break;
	    }
	    ++ptr;
	}
	
	if( nameNumeric || ename.Contains( StrRef( "/" ) ) ||
	    ename.Contains( StrRef( "..." ) ) )
	{
	    e->Set( MsgServer2::InvalidExtName ) << name;
	    return false;
	}

	StrBuf::Set( name );
	return true;
}


