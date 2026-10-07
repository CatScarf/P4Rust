/*
 * Copyright 2019 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# include <stdhdrs.h>

# ifdef HAS_EXTENSIONS

# include <limits>
# include <error.h>
# include <clientapi.h>
# include <strarray.h>
# include <p4script53.h>
# include <clientuserlua.h>
# include <clientapilua.h>
# include <filesyslua.h>
# include <map>
# include <memory>

extern bool solfnCheck( const sol::protected_function_result& r,
	                const char* impl, const char* where, Error* e );

extern void solExcpESet( const sol::error& err, const char* impl,
	                 const char* where, Error* e );

int ClientTransferLua::Transfer( ClientApi* client, ClientUser* ui,
	                         const char* cmd, StrArray& args,
	                         StrDict& pVars, int threads, Error* e )
{
	ClientApi child;
	StrRef var, val;

	for( int j = 0; pVars.GetVar( j++, var, val ); )
	    child.SetProtocol( var.Text(), val.Text() );

	child.SetProtocol( P4Tag::v_api, "99999" );
	child.SetProtocol( P4Tag::v_enableStreams, "" );
	child.SetProtocol( P4Tag::v_enableGraph, "" );
	child.SetProtocol( P4Tag::v_expandAndmaps, "" );

	child.SetPort( &client->GetPort() );
	child.SetUser( &client->GetUser() );
	child.SetClient( &client->GetClient() );

	if( client->GetPassword().Length() )
	    child.SetPassword( &client->GetPassword() );

	child.SetProtocolV( "tag" );
	child.SetProg( client->GetProg().Text() );

	child.Init( e );
	child.SetVersion( client->GetVersion().Text() );
	child.SetBreak( (ClientUserLua*)ui );

	if( e->Test() )
	{
	    ui->HandleError( e );
	    return 1;
	}

	char** a = new char*[ args.Count() ];

	for( int j = 0; j < args.Count(); j++ )
	    a[ j ] = args.Get( j )->Text();

	child.SetArgv( args.Count(), a );
	child.Run( cmd, ui );

	delete[] a;
	child.Final( e );
	
	if( e->Test() )
	{
	    ui->HandleError( e );
	    return 1;
	}

	// Errors like MsgClient::ClobberFile are only detected like this.
	if( child.GetErrors() )
	    return 1;

	return 0;
}

ClientUserLua::ClientUserLua( const char* impl, const int apiVersion )
	: ClientUser(), impl( impl ), apiVersion( apiVersion )
{

}

ClientUserLua::~ClientUserLua()
{
}

void ClientUserLua::doBindings( sol::state* lua, sol::table& ns,
	std::vector< std::function< void( ClientUserLua& ) > >* ClientUserBindCfgs,
	const char* implName, const int apiVersion )
{
	ns.new_usertype< ClientUserLua >( "ClientUser",
	    "new", sol::factories( [=]()
	        {
	            auto cu = std::make_unique< ClientUserLua >( implName,
	                                                         apiVersion );
	            for( const auto& fn : *ClientUserBindCfgs )
	                fn( *cu );
	            return cu;

	        } ),
	    "OutputError" , &ClientUserLua::fOutputError,
	    "HandleError" , &ClientUserLua::fHandleError,
	    "Message"     , &ClientUserLua::fMessage,
	    "InputData"   , &ClientUserLua::fInputData,
	    "OutputInfo"  , &ClientUserLua::fOutputInfo,
	    "OutputText"  , &ClientUserLua::fOutputText,
	    "OutputBinary", &ClientUserLua::fOutputBinary,
	    "OutputStat"  , &ClientUserLua::fOutputStat,
	    "Prompt"      , &ClientUserLua::fPrompt,
	    "File"        , &ClientUserLua::fFile,
	    "Edit"        , &ClientUserLua::fEdit,
	    "data"        , &ClientUserLua::data,
	    sol::base_classes, sol::bases< ClientUser, KeepAlive >()
	);
}

void ClientUserLua::DisableParallelTransfer()
{
	delete transfer;
	transfer = new ClientTransferLua();
}

int ClientUserLua::IsAlive()
{
	return 1; // TODO: put something better in here
}

void ClientUserLua::Message( Error* err )
{
	if( !fMessage )
	{
	    ClientUser::Message( err );
	    return;
	}

	auto x = std::make_unique< Error >( Error() );
	*x.get() = *err;
	x.get()->Snap();
	auto r = apiVersion == 1 ? fMessage( std::move( x ) )
	                         : fMessage( this, std::move( x ) );

	Error e;
	solfnCheck( r, impl, "ClientUserLua::Message", &e );
}

void ClientUserLua::InputData( StrBuf *strbuf, Error *e )
{
	if( !fInputData )
	{
	    ClientUser::InputData( strbuf, e );
	    return;
	}

	auto es = std::make_shared< Error >();
	Error* esp = es.get();

	auto r = apiVersion == 1 ? fInputData( es ) : fInputData( this, es );

	if( esp->Test() )
	    e->Merge( *esp );

	if( solfnCheck( r, impl, "ClientUserLua::InputData", e ) )
	    return;

	try {
	    std::string buf = r.get< std::string >( 0 );
	    strbuf->Set( buf.c_str() );
	    return;
	} catch( const sol::error& err )
	{ solExcpESet( err, impl, "ClientUserLua::InputData", e ); }
}

void ClientUserLua::Edit( FileSys *f1, Error *e )
{
	if( !fEdit )
	{
	    ClientUser::Edit( f1, e );
	    return;
	}

	auto es = std::make_shared< Error >();
	Error* esp = es.get();

	auto r = apiVersion == 1 ? fEdit( f1->Name(), es )
	                         : fEdit( this, f1->Name(), es );

	if( esp->Test() )
	    e->Merge( *esp );

	solfnCheck( r, impl, "ClientUserLua::Edit", e );
}

void ClientUserLua::Prompt( Error *err, StrBuf &rsp, int noEcho, Error *e )
{
	if( !fPrompt )
	{
	    ClientUser::Prompt( err, rsp, noEcho, e );
	    return;
	}

	std::string s( rsp.Text() );
	auto ers = std::make_shared< Error >();
	*ers.get() = *err;
	ers.get()->Snap();
	auto es = std::make_shared< Error >();
	Error* esp = es.get();

	auto r = apiVersion == 1 ? fPrompt( ers, s, noEcho, es )
	                         : fPrompt( this, ers, s, noEcho, es );

	if( esp->Test() )
	    e->Merge( *esp );

	if( solfnCheck( r, impl, "ClientUserLua::Prompt", e ) )
	    return;

	try {
	    std::string buf = r.get< std::string >( 0 );
	    rsp.Set( buf.c_str() );
	    return;
	} catch( const sol::error& err )
	{ solExcpESet( err, impl, "ClientUserLua::Prompt", e ); }
}

void ClientUserLua::ErrorPause( char *errBuf, Error *e )
{
	if( !fErrorPause )
	{
	    ClientUser::ErrorPause( errBuf, e );
	    return;
	}

	auto es = std::make_shared< Error >();
	Error* esp = es.get();

	auto r = apiVersion == 1 ? fErrorPause( errBuf, es )
	                         : fErrorPause( this, errBuf, es );

	if( esp->Test() )
	    e->Merge( *esp );

	solfnCheck( r, impl, "ClientUserLua::ErrorPause", e );
}

void ClientUserLua::OutputError( const char *err )
{
	if( !fOutputError )
	{
	    ClientUser::OutputError( err );
	    return;
	}

	auto r = apiVersion == 1 ? fOutputError( err )
	                         : fOutputError( this, err );
	Error e;
	solfnCheck( r, impl, "ClientUserLua::OutputError", &e );
}

void ClientUserLua::HandleError( Error *err )
{
	if( !fHandleError )
	{
	    ClientUser::HandleError( err );
	    return;
	}

	auto x = std::make_unique< Error >( Error() );
	*x.get() = *err;
	x.get()->Snap();

	auto r = apiVersion == 1 ? fHandleError( std::move( x ) )
	                         : fHandleError( this, std::move( x ) );

	Error e;
	solfnCheck( r, impl, "ClientUserLua::HandleError", &e );
}

void ClientUserLua::OutputInfo( char level, const char *data )
{
	if( !fOutputInfo )
	{
	    ClientUser::OutputInfo( level, data );
	    return;
	}

	auto r = apiVersion == 1 ? fOutputInfo( level, data )
	                         : fOutputInfo( this, level, data );
	Error e;
	solfnCheck( r, impl, "ClientUserLua::OutputInfo", &e );
}

void ClientUserLua::OutputText( const char *data, int length )
{
	if( !fOutputText )
	{
	    ClientUser::OutputText( data, length );
	    return;
	}

	auto r = apiVersion == 1 ? fOutputText( data, length )
	                         : fOutputText( this, data, length );
	Error e;
	solfnCheck( r, impl, "ClientUserLua::OutputText", &e );
}

void ClientUserLua::OutputBinary( const char *data, int length )
{
	if( !fOutputBinary )
	{
	    ClientUser::OutputBinary( data, length );
	    return;
	}

	auto r = apiVersion == 1 ? fOutputBinary( data, length )
	                         : fOutputBinary( this, data, length );
	Error e;
	solfnCheck( r, impl, "ClientUserLua::OutputBinary", &e );
}

void ClientUserLua::OutputStat( StrDict *dict )
{
	if( !fOutputStat )
	{
	    ClientUser::OutputStat( dict );
	    return;
	}

	std::map< std::string, std::string > m;
	StrRef var, val;

	for( int i = 0; dict->GetVar( i, var, val ); i++ )
	    if( strcmp( var.Text(), "func" ) && 
	        strcmp( var.Text(), "specFormatted" ) &&
	        strcmp( var.Text(), "altArg" ) )
	{
	    m[ std::string( var.Text() ) ] = std::string( val.Text() );
	}

	auto r = apiVersion == 1 ? fOutputStat( std::move( m ) )
	                         : fOutputStat( this, std::move( m ) );

	Error e;
	solfnCheck( r, impl, "ClientUserLua::OutputStat", &e );
}

FileSys* ClientUserLua::File( FileSysType type )
{
	if( !fFile )
	    return FileSys::Create( type );

	auto r = apiVersion == 1 ? fFile( type ) : fFile( this, type );
	Error e;

	if( solfnCheck( r, impl, "ClientUserLua::File", &e ) )
	    return nullptr;

	try {
	    return r.get< std::unique_ptr< FileSysLua > >().release();
	} catch( const sol::error& err )
	{ solExcpESet( err, impl, "ClientUserLua::File", &e ); }

	return nullptr;
}

# endif
