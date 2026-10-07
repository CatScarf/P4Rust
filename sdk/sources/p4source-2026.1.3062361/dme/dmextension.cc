/*
 * Copyright 1995, 2018 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# include <stdhdrs.h>
# include <strbuf.h>
# include <strdict.h>
# include <strtable.h>
# include <error.h>
# include <rpc.h>
# include <debug.h>
# include <msgscript.h>

# include <filesys.h>
# include "dmextension.h"

# ifdef HAS_EXTENSIONS

ExtensionCallerData::~ExtensionCallerData()
{
}

Extension::Extension( const SCR_VERSION v, const int apiVersion,
	              p4_std_optional::optional< std::unique_ptr< ExtensionCallerData > > ecd,
	              Error* e, const bool alloc )
: p4script( v, apiVersion, e ), ecd( std::move( ecd ) )
{
	switch( v )
	{
	    case P4SCRIPT_LUA_53:
	        rhePimpl =
	            std::make_unique< Extension::extImpl53 >( *this, e );
	        break;
	    default:
	        StrBuf msg = "Extension::Extension() bad case!";
	        e->Set( MsgScript::DevErr ) << msg;
	        return;
	}
};

Extension::~Extension()
{
}

void Extension::LoadFile( const char* file, Error *e )
{
	doFile( file, e );
}

void Extension::doBindings( Error* e )
{
	rhePimpl->doBindings( e );
}

p4_std_any::p4_any Extension::RunCallBack( const char* name, Error* e )
{
	return rhePimpl->RunCallBack( name, e );
}

Extension::extImpl::extImpl( Extension& p, Error *e )
: parent( p )
{
}

Extension::extImpl::~extImpl()
{
}

# endif
