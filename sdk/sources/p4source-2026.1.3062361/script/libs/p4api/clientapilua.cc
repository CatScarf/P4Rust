/*
 * Copyright 2018 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# include <stdhdrs.h>

# ifdef HAS_EXTENSIONS

# include <limits>

# include <p4script53.h>
# include <clientapi.h>
# include <error.h>
# include <debug.h>
# include <p4script.h>

# include "clientapilua.h"

ClientApiLua::ClientApiLua( const void* l )
{
}

ClientApiLua::~ClientApiLua()
{
}

void ClientApiLua::doBindings( void* luaV, void* nsV,
	std::vector< std::function< void( ClientApi& ) > >* ClientApiBindCfgs )
{
	sol::state* lua = static_cast< sol::state* >( luaV );
	sol::table*  ns = static_cast< sol::table* >(  nsV );

	ns->new_usertype< ClientApiLua >( "ClientApi",
	   "new", sol::factories( [=]() {
	              auto ca = std::make_unique< ClientApiLua >( lua );
	              for( const auto& fn : *ClientApiBindCfgs )
	                  fn( *ca );
	              return ca;
		} ),
	    "Init"       , &ClientApiLua::fInit,
	    "Final"      , &ClientApiLua::fFinal,
	    "Run"        , &ClientApiLua::fRun,
	    "SetClient"  , &ClientApiLua::fSetClient,
	    "SetHost"    , &ClientApiLua::fSetHost,
	    "SetUser"    , &ClientApiLua::fSetUser,
	    "SetPort"    , &ClientApiLua::fSetPort,
	    "SetVersion" , &ClientApiLua::fSetVersion,
	    "SetProg"    , &ClientApiLua::fSetProg,
	    "SetVar"     , &ClientApiLua::fSetVar,
	    "SetProtocol", sol::overload( []( ClientApiLua& ca, const char* k )
	                                  { ca.SetProtocol( k ); },
	                                  []( ClientApiLua& ca, const char* k,
	                                                        const char* v )
	                                  { ca.SetProtocol( k, v ); } ),
	    "SetPassword", &ClientApiLua::fSetPassword,
	    "GetPort"    , &ClientApiLua::fGetPort,
	    "GetUser"    , &ClientApiLua::fGetUser,
	    "IsUnicode"  , &ClientApi::IsUnicode,
	    "SetTrans"   , &ClientApi::SetTrans,
	    "Null"       , &ClientApiLua::Null
	);
}

void ClientApiLua::restrictAPI( const int max )
{
	apiMax = max;
}

void ClientApiLua::fRun( const char *func, ClientUser *ui )
{
	ClientApi::Run( func, ui );
}

void ClientApiLua::fSetVar( const char *var, const char *value )
{
	ClientApi::SetVar( var, value );
}

void ClientApiLua::fSetVersion( const char *v )
{
	ClientApi::SetVersion( v );
}

void ClientApiLua::fSetUser( const char *u )
{
	ClientApi::SetUser( u );
}

void ClientApiLua::fSetPassword( const char *p )
{
	ClientApi::SetPassword( p );
}

void ClientApiLua::fSetPort( const char *p )
{
	ClientApi::SetPort( p );
}

void ClientApiLua::fSetProg( const char *p )
{
	ClientApi::SetProg( p );
}

void ClientApiLua::fSetProtocol( const char *p, const char *v )
{
	if( apiMax && StrRef( p ) == P4Tag::v_api &&
	    StrRef( v ).Atoi() >= apiMax )
	{
	    ClientApi::SetProtocol( p, StrNum( apiMax ).Text() );
	    return;
	}

	ClientApi::SetProtocol( p, v );
}

void ClientApiLua::fSetClient( const char *c )
{
	ClientApi::SetClient( c );
}

void ClientApiLua::fSetHost( const char *h )
{
	ClientApi::SetHost( h );
}

const char* ClientApiLua::fGetPort()
{
	return ClientApi::GetPort().Text();
}

const char* ClientApiLua::fGetUser()
{
	return ClientApi::GetUser().Text();
}

const char* ClientApiLua::Null() const
{
	return StrRef::Null().Text();
}

int ClientApiLua::fInit( Error &e )
{
	ClientApi::Init( &e );
	return e.Test();
}

int ClientApiLua::fFinal()
{
	Error e;
	ClientApi::Final( &e );
	return e.Test();
}

# endif
