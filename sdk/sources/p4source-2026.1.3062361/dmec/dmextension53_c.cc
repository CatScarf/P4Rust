/*
 * Copyright 1995, 2018 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# include <stdhdrs.h>

# ifdef HAS_EXTENSIONS

# include <clientapi.h>
# include <msgscript.h>
# include "dmextension.h"
# include "dmextension_c.h"
# include <p4script53.h>
# include <rpc.h>
# include <client.h>
# include <clientscript.h>
# include <clientapilua.h>

ExtensionClient::ExtensionClient( const SCR_VERSION v, const int apiVersion,
	                p4_std_optional::optional<
	                std::unique_ptr< ExtensionCallerData > > ecd, Error* e )
: Extension( v, apiVersion, std::move( ecd ), e, v != P4SCRIPT_LUA_53 )
{
	if( v != P4SCRIPT_LUA_53 )
	    return;

	ExtensionClient::extImpl53client* ec =
	    new ExtensionClient::extImpl53client( *this, e );
	std::unique_ptr< Extension::extImpl53 > ecu( ec );
	rhePimpl = std::move( ecu );
}

ExtensionClient::~ExtensionClient()
{
}

ExtensionClient::extImpl53client::extImpl53client( Extension& p, Error* e )
: extImpl53( p, e )
{
	doBindings( e );
}

ExtensionClient::extImpl53client::~extImpl53client()
{
}

void ExtensionClient::DisableExtensionBinding()
{
	((ExtensionClient::extImpl53client*)&(*rhePimpl))->DisableExtensionBinding();
}

void Extension::extImpl53::doBindings( Error* e )
{
}

void ExtensionClient::extImpl53client::doBindings( Error* e )
{
	auto lua = cast_sol_State( parent.GetImpl()->getState() );

	sol::table ns_hc = (*lua)[ "Helix" ][ "Core" ];
	sol::table ns = ns_hc.create_named( "Client" );

	ns.new_enum( "Action",
	    "FAIL"   , ClientScriptAction::FAIL,
	    "PASS"   , ClientScriptAction::PASS,
	    "REPLACE", ClientScriptAction::REPLACE,
	    "PRE_DEBUG", ClientScriptAction::PRE_DEBUG,
	    "ABORT", ClientScriptAction::ABORT,
	    "EARLY_RETURN", ClientScriptAction::EARLY_RETURN
	);

	ns[ "ClientMsg" ] = [ this ]( const char* msg )
	{
	    auto ecd = (ExtensionCallerDataC*)parent.GetECD();
	    Error e;
	    e.Set( MsgScript::ExtClientMsg ) << ecd->sourcePath.c_str() << msg;
	    ecd->ui->Message( &e );
	};

	ns[ "ClientError" ] = [ this ]( const char* msg )
	{
	    auto ecd = (ExtensionCallerDataC*)parent.GetECD();
	    Error e;
	    e.Set( MsgScript::ExtClientError ) << ecd->sourcePath.c_str()
	                                       << msg;
	    ecd->ui->HandleError( &e );
	};

	ns[ "ClientPrompt" ] = [ this ]( const char* msg, const bool noEcho )
	{
	    auto ecd = (ExtensionCallerDataC*)parent.GetECD();
	    Error e, ep;
	    // TODO: It would be good to filter any control characters
	    //       out of this now.
	    ep.Set( MsgScript::ExtClientPrompt ) << ecd->sourcePath.c_str() << msg;
	    StrBuf rsp;
	    // TODO: noEcho isn't working?
	    ecd->ui->Prompt( &ep, rsp, noEcho, &e );
	    return std::string( rsp.Text() );
	};

	ns[ "GetVar" ] = [ this ]( const std::string k, sol::this_state L )
	{
	    auto ecd = (ExtensionCallerDataC*)parent.GetECD();
	    sol::variadic_results r;

	    if( k == "sourcePath" )
	        r.push_back( { L, sol::in_place_type< std::string >,
	                       ecd->sourcePath } );
	    else
	    if( k == "client" )
	        r.push_back( { L, sol::in_place_type< std::string >,
	            std::string( ecd->client->GetClient().Text() ) } );
	    else
	    if( k == "cwd" )
	        r.push_back( { L, sol::in_place_type< std::string >,
	            std::string( ecd->client->GetCwd().Text() ) } );
	    else
	    if( k == "port" )
	        r.push_back( { L, sol::in_place_type< std::string >,
	            std::string( ecd->client->GetPort().Text() ) } );
	    else
	    if( k == "user" )
	        r.push_back( { L, sol::in_place_type< std::string >,
	            std::string( ecd->client->GetUser().Text() ) } );
	    else
	    if( k == "func" )
	        r.push_back( { L, sol::in_place_type< std::string >,
	                       ecd->func } );
	    else
	    if( k == "argc" )
	        r.push_back( { L, sol::in_place_type< int >,
	            ecd->client->GetSendArgc() } );
	    else
	    if( k == "argv" )
	    {
	        std::vector< std::string > as;

	        for( int i = 0; auto arg = ecd->client->GetSendArgv( i ); i++ )
	            as.emplace_back( std::string( arg->Text() ) );

	        r.push_back( { L, sol::in_place_type< std::vector< std::string > >,
	                     sol::as_table( as ) } );
	    }
	    else
	    if( k == "ticket" )
	        r.push_back( { L, sol::in_place_type< std::string >,
	            std::string( ecd->client->GetPassword().Text() ) } );
	    else
	    if( k == "zerosync" )
	    {
	        if( const auto zs = ecd->client->GetVar( "zerosync" ) )
	            r.push_back( { L, sol::in_place_type< std::string >,
	                           zs->Text() } );
	        else
	            r.push_back( { L, sol::in_place, sol::lua_nil } );
	    }
	    else
	        r.push_back( { L, sol::in_place, sol::lua_nil } );

	    return r;
	};

	ns_hc[ "P4API" ][ "ClientApi" ][ "EnableExtensions" ] = []( ClientApiLua& ca ) -> bool
	{
	    Error e;
	    ca.EnableExtensions( &e );
	    return e.Test();
	};

	ns_hc[ "P4API" ][ "ClientApi" ][ "DisableExtensions" ] = []( ClientApiLua& ca )
	{
	    ca.DisableExtensions();
	};

	((ExtensionCallerDataC*)parent.GetECD())->loginSSO =
	    [this]( StrBuf& data, StrBuf& result, int argc,
	            std::vector< std::string > argv,
	            std::unordered_map< std::string, std::string > ssodict,
	            Error *e )
	{
	    auto l = cast_sol_State( parent.GetImpl()->getState() );

	    try {
	        // XXX: does this need to handle binary?
	        auto r = (*l)[ "loginSSO" ]( argc, argv, ssodict,
	                                     std::string( data.Text() ) );

	        if( !r.valid() )
	        {
	            sol::error err = r;
	            StrBuf msg;
	            msg << "loginSSO" << ": " << err.what();
	            e->Set( MsgScript::ScriptRuntimeError ) << parent.getImplName()
	                                                    << msg;
	            return false;
	        }

	        const bool ret = r.get< bool >( 0 );

	        if( ret )
	            result = r.get< std::string >( 1 ).c_str();

	        return ret;
	    } catch( const std::exception& err )
	    { 
	        StrBuf msg;
	        msg << "loginSSO" << ": " << err.what();
	        e->Set( MsgScript::ScriptRuntimeError ) << parent.getImplName()
	                                                << msg;
	    }

	    return false;
	};
}

void ExtensionClient::extImpl53client::DisableExtensionBinding()
{
	auto lua = cast_sol_State( parent.GetImpl()->getState() );
	sol::table ns = (*lua)[ "Helix" ][ "Core" ][ "P4API" ];

	ns[ "ClientApi" ][ "EnableExtensions"  ] = nullptr;
	ns[ "ClientApi" ][ "DisableExtensions" ] = nullptr;
}

# endif

