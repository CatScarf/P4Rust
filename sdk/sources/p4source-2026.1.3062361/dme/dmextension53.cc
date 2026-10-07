/*
 * Copyright 1995, 2018 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# include <stdhdrs.h>

# ifdef HAS_EXTENSIONS

# include <error.h>
# include <strbuf.h>
# include <msgdm.h>
# include "dmextension.h"
# include <p4script53.h>

Extension::extImpl53::extImpl53( Extension& p, Error *e )
: extImpl( p, e )
{
	if( parent.ecd )
	    doBindings( e );
}

Extension::extImpl53::~extImpl53()
{
}

p4_std_any::p4_any
Extension::extImpl53::RunCallBack( const char* name, Error* e )
{
	auto ret = parent.doScriptFn( name, e );

	if( e->Test() )
	{
	    if( parent.ecd )
	        parent.GetECD()->SetExtExecError();

	    return {};
	}

	if( !ret.has_value() )
	    return {};

	sol::object ro;

	try {
	    ro = p4_std_any::any_cast< sol::object >( ret );
	}
	catch( const std::exception& err )
	{
	    StrBuf msg;
	    msg << "Extension::extImpl53::RunCallBack: " << err.what();
	    e->Set( MsgDm::DevErr ) << msg;
	    return {};
	}

	if( ro.is< std::map< std::string, std::string > >() )
	{
	    return p4_std_any::p4_any
	    ( ro.as< sol::as_table_t< std::unordered_map< std::string, std::string > > >() );
	}

	if( ro.is< bool >() )
	    return p4_std_any::p4_any( ro.as< bool >() );

	// E.g. enums
	if( ro.is< int >() )
	    return p4_std_any::p4_any( ro.as< int >() );

	if( ro.is< std::string >() )
	    return p4_std_any::p4_any( ro.as< std::string >() );

	return {};
}

# endif
