/*
 * Copyright 1995, 2019 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# include <stdhdrs.h>

# ifdef HAS_EXTENSIONS

# include <string>
# include <vector>
# include <map>
# include <deque>

# include <clientapi.h>
# include <enviro.h>
# include <hostenv.h>
# include <debug.h>
# include <i18napi.h>
# include <ignore.h>

# include <p4script.h>
# include <p4script53.h>

# include "p4error.h"
# include "p4result.h"
# include "p4luadebug.h"
# include "clientuserp4lua.h"
# include "specmgrp4lua.h"
# include "p4lua.h"

namespace P4Lua {

P4Lua::P4Lua()
{
	specMgr = new SpecMgrP4Lua();
	ui = new ClientUserP4Lua( this );
	client = new ClientApi();

	debug = 0;
	server2 = 0;
	depth = 0;
	exceptionLevel = 2;
	maxResults = 0;
	maxScanRows = 0;
	maxLockTime = 0;
	InitFlags();
	apiLevel = atoi( P4Tag::l_client );
	enviro = new Enviro;
	prog = "unnamed p4lua script";

	client->SetProtocol( "specstring", "" );

	//
	// Load any P4CONFIG file
	//
	HostEnv henv;
	StrBuf cwd;

	henv.GetCwd( cwd, enviro );
	if( cwd.Length() )
	    enviro->Config( cwd );

	//
	// Load the current ticket file. Start with the default, and then
	// override it if P4TICKETS is set.
	//
	const char *t;

	henv.GetTicketFile( ticketFile );

	if( (t = enviro->Get("P4TICKETS")) )
	       ticketFile = t;

	//
	// Load the current trust file. Start with the default, and then
	// override it if P4TRUST is set.
	//

	henv.GetTrustFile( trustFile );

	if( (t = enviro->Get("P4TRUST")) )
	    trustFile = t;

	//
	// Load the current P4CHARSET if set.
	//
	if( client->GetCharset().Length() )
	    SetCharset( client->GetCharset().Text() );
}

P4Lua::~P4Lua()
{
	if ( IsConnected() )
	{
	    Error e;
	    client->Final( &e );
	    // Ignore errors
	}
	delete enviro;
	delete ui;
	delete client;
	delete specMgr;
}

void
P4Lua::doBindings( void* luaV, void *nsV,
	std::vector< std::function< void( ClientApi& ) > >* ClientApiBindCfgs )
{
	// Note that the autoconnect() function is defined in
	// Extension::extImpl53::doBindings in dmextension53_s.cc so it
	// can be exposed only in the server.
	sol::table*  ns = static_cast< sol::table* >( nsV );

	ns->new_usertype< P4Lua >( "P4",
	    "new", sol::factories( 
	            [=]() {
	                auto ca = std::make_unique< P4Lua >( );
	                for( const auto& fn : *ClientApiBindCfgs )
	                    fn( *(ca->client) );
	                return ca;
	            },
	            [=]( sol::table pl ) {
	                auto ca = std::make_unique< P4Lua >( );
	                for( const auto& fn : *ClientApiBindCfgs )
	                    fn( *(ca->client) );
	                return ca;
	            } ),
	            
	    // Protocol options
	    "api_level",       sol::property(&P4Lua::GetApiLevel, &P4Lua::SetApiLevel),
	    "streams",         sol::property(&P4Lua::GetStreams, &P4Lua::SetStreams),
	    "tagged",          sol::property(&P4Lua::GetTagged, &P4Lua::SetTagged),
	    "track",           sol::property(&P4Lua::GetTrack, &P4Lua::SetTrack),
	    "graph",           sol::property(&P4Lua::GetGraph, &P4Lua::SetGraph),
	    "exception_level", sol::property(&P4Lua::GetExceptionLevel, &P4Lua::SetExceptionLevel),


	    // Perforce client settings.
	    "charset",         sol::property(&P4Lua::GetCharset, &P4Lua::SetCharset),
	    "client",          sol::property(&P4Lua::GetClient, &P4Lua::SetClient),
	    "enviro_file",     sol::property(&P4Lua::GetEnviroFile, &P4Lua::SetEnviroFile),
	    "host",            sol::property(&P4Lua::GetHost, &P4Lua::SetHost),
	    "ignore_file",     sol::property(&P4Lua::GetIgnoreFile, &P4Lua::SetIgnoreFile),
	    "language",        sol::property(&P4Lua::GetLanguage, &P4Lua::SetLanguage),
	    "p4config_file",   sol::readonly_property(&P4Lua::GetConfig),
	    "password",        sol::property(&P4Lua::GetPassword, &P4Lua::SetPassword),
	    "port",            sol::property(&P4Lua::GetPort, &P4Lua::SetPort),
	    "prog",            sol::property(&P4Lua::GetProg, &P4Lua::SetProg),
	    "protocol",        sol::overload( []( P4Lua& ca, const char* k )
	                                      { ca.SetProtocol( k ); },
	                                      []( P4Lua& ca, const char* k,
	                                                     const char* v )
	                                      { ca.SetProtocol( k, v ); } ),
	    "ticket_file",     sol::property(&P4Lua::GetTicketFile, &P4Lua::SetTicketFile),
	    "trust_file",      sol::property(&P4Lua::GetTrustFile, &P4Lua::SetTrustFile),
	    "user",            sol::property(&P4Lua::GetUser, &P4Lua::SetUser),
	    "version",         sol::property(&P4Lua::GetVersion, &P4Lua::SetVersion),

	    /* We can't support these setters as they could alter the
	       environment that the server is running in.
	    "cwd",             sol::property(&P4Lua::GetCwd, &P4Lua::SetCwd),
	    "env",             sol::property(&P4Lua::GetEnv, &P4Lua::SetEnv),
	    */
	    "cwd",             sol::readonly_property(&P4Lua::GetCwd),
	    "env",             sol::readonly_property(&P4Lua::GetEnv),

	    "maxresults",      sol::property(&P4Lua::GetMaxResults, &P4Lua::SetMaxResults),
	    "maxscanrows",     sol::property(&P4Lua::GetMaxScanRows, &P4Lua::SetMaxScanRows),
	    "maxlocktime",     sol::property(&P4Lua::GetMaxLockTime, &P4Lua::SetMaxLockTime),


	    "is_ignored",  &P4Lua::IsIgnored,

	    // Session Connect/Disconnect
	    "connect",     &P4Lua::Connect,
	    "is_connected",&P4Lua::Connected,
	    "disconnect",  &P4Lua::Disconnect,
	    "reset",       &P4Lua::Reset,

	    // Running commands - general purpose commands
	    "run",         &P4Lua::Run,

	    "input",       sol::writeonly_property(&P4Lua::SetInput),
	    "errors",      sol::readonly_property(&P4Lua::GetErrors),
	    "messages",    sol::readonly_property(&P4Lua::GetMessages),
	    "warnings",    sol::readonly_property(&P4Lua::GetWarnings),
	    "track_output",sol::readonly_property(&P4Lua::GetTrack),

	    "server_unicode",        sol::readonly_property(&P4Lua::ServerUnicode),
	    "server_level",          sol::readonly_property(&P4Lua::GetServerLevel),
	    "server_case_sensitive", sol::readonly_property(&P4Lua::ServerCaseSensitive),

	    // Spec parsing
	    "parse_spec",  &P4Lua::ParseSpec,
	    "format_spec", &P4Lua::FormatSpec,

	    // Debugging support
	    "debug",       sol::property(&P4Lua::GetDebug, &P4Lua::SetDebug)

	    /* ToDo: support handler callback objects
	    // Support for OutputHandler
	    "handler", sol::property(&P4Lua::GetHandler, &P4Lua::SetHandler),
	    

	    // Support for Progress API
	    "progress", sol::property(&P4Lua::GetProgress, &P4Lua::SetProgress)
	    */
	);
}

const char *
P4Lua::GetEnv( const char *v)
{
	return enviro->Get( v );
}

void
P4Lua::SetEnviroFile( const char *c )
{
	enviro->SetEnviroFile(c);
}

const char *
P4Lua::GetEnviroFile()
{
	return enviro->GetEnviroFile()->Text();
}

void
P4Lua::SetApiLevel( int level )
{
	StrBuf	b;
	b << level;
	apiLevel = level;
	client->SetProtocol( "api", b.Text() );
	ui->SetApiLevel( level );
}

int
P4Lua::SetCharset( const char *c )
{
	StrRef cs_none( "none" );

	if( P4LUA_COMMANDS )
	    fprintf( stderr, "[P4] Setting charset: %s\n", c );

	if( c && cs_none != c )
	{
	    CharSetApi::CharSet cs = CharSetApi::Lookup( c );
	    if( cs < 0 )
	    {
		StrBuf	m;
		m = "Unknown or unsupported charset: ";
		m.Append( c );
		//Except( "P4#charset=", m.Text() );
	    }
	    CharSetApi::CharSet utf8 = CharSetApi::Lookup( "utf8" );
	    client->SetTrans( utf8, cs, utf8, utf8 );

	    client->SetCharset( c );
	}
	else
	{
	    // Disables automatic unicode detection if called
	    // prior to init (2014.2)
	    client->SetTrans( 0 );
	}
	return 1;
}

void
P4Lua::SetCwd( const char *c )
{
	client->SetCwd( c );
	enviro->Config( StrRef( c ) );
}

void
P4Lua::SetTicketFile( const char *p )
{
	client->SetTicketFile( p );
	ticketFile = p;
}

void
P4Lua::SetTrustFile( const char *p )
{
	client->SetTrustFile( p );
	trustFile = p;
}

void
P4Lua::SetDebug( int d )
{
	debug = d;
	ui->SetDebug( d );
	specMgr->SetDebug( d );

	if( P4LUA_RPC )
	    p4debug.SetLevel( "rpc=5" );
	else
	    p4debug.SetLevel( "rpc=0" );

	if( P4LUA_SSL )
	    p4debug.SetLevel( "ssl=3" );
	else
	    p4debug.SetLevel( "ssl=0" );
}

void
P4Lua::SetProtocol( const char *var )
{
	client->SetProtocol( var );
}

void
P4Lua::SetProtocol( const char *var, const char *val )
{
	client->SetProtocol( var, val );
}

bool
P4Lua::SetEnv( const char* var, const char* val, sol::this_state s )
{
	Error    e;

	client->GetEnviro()->Set( var, val, &e );
	if( e.Test() && exceptionLevel )
	{
	    StrBuf buf;
	    e.Fmt( buf, EF_PLAIN );
	    std::string err = "P4#set_env - ";
	    err.append( buf.Text() );
	    luaL_error( s, err.c_str() );
	}

	if( e.Test() )
	    return false;

	// Fixes an issue on OS X where the next enviro->Get doesn't return the
	// cached value
	client->GetEnviro()->Reload();

	return true;
}

//
// connect to the Perforce server.
//

bool
P4Lua::Connect( sol::this_state s )
{
	if( P4LUA_COMMANDS )
	    fprintf( stderr, "[P4] Connecting to Perforce\n" );

	if( IsConnected() )
	{
	    if( exceptionLevel > 1)
	    {
	        luaL_error( s, "P4#connect - Perforce client already connected!" );
		return false;
	    }
	    return true;
	}

	return ConnectOrReconnect( s );
}

bool
P4Lua::ConnectOrReconnect( sol::this_state s )
{
	if ( IsTrackMode() )
	    client->SetProtocol( "track", "" );

	Error e;

	ResetFlags();
	client->Init( &e );

	if( e.Test() )
	    ui->GetResults().AddMessage( &e, s );

	if( e.Test() && exceptionLevel )
	{
	    StrBuf buf;
	    e.Fmt( buf, EF_PLAIN );
	    std::string err = "P4#connect - ";
	    err.append( buf.Text() );
	    luaL_error( s, err.c_str() );
	}

	if( e.Test() )
	    return false;

	 // If a handler is defined, reset the break functionality
	// for the KeepAlive function

	if( ui->GetHandler() )
	{
	    client->SetBreak( ui );
	}

	SetConnected();
	return true;
}

//
// Disconnect session
//

bool
P4Lua::Disconnect( sol::this_state s )
{
	if ( P4LUA_COMMANDS )
	    fprintf( stderr, "[P4] Disconnect\n" );

	if ( !IsConnected() )
	{
	    // Clear the specdef cache.
	    specMgr->Reset();
	    ResetFlags();
	    if( exceptionLevel > 1)
	    {
		luaL_error( s, "P4#disconnect - not connected" );
		return false;
	    }
	    return true;
	}
	Error e;
	client->Final( &e );
	ResetFlags();

	// Clear the specdef cache.
	specMgr->Reset();
	return true;
}


//
// Test whether or not connected
//
bool
P4Lua::Connected( sol::this_state s )
{
	if( IsConnected() && !client->Dropped() )
	    return true;
	else if( IsConnected() )
	    Disconnect( s );
	return false;
}

void
P4Lua::SetTagged( bool enable )
{
	if( enable )
	    SetTag();
	else
	    ClearTag();
}

bool
P4Lua::SetTrack( bool enable, sol::this_state s )
{
	if ( IsConnected() ) {
	    if( exceptionLevel )
	    {
		luaL_error( s, "P4.track - Can't change performance tracking once you've connected.");
	    }
	    return false;
	}
	else if ( enable ) {
	    SetTrackMode();
	    ui->SetTrack(true);
	}
	else {
	    ClearTrackMode();
	    ui->SetTrack(false);
	}
	return true;
}

void
P4Lua::SetStreams( bool enable )
{
	if ( enable )
	   SetStreamsMode();
	else
	   ClearStreamsMode();
}

void
P4Lua::SetGraph( bool enable )
{
	if ( enable )
	   SetGraphMode();
	else
	   ClearGraphMode();
}

sol::object
P4Lua::GetServerLevel( sol::this_state s )
{
	if( !IsConnected() )
	{
	    luaL_error( s, "P4.server_level - Not connected to a Perforce Server." );
	    return sol::make_object( s, false );
	}
	if( !IsCmdRun() )
	    Run( "info", sol::variadic_args(), s );
	return sol::make_object( s, server2 );
}

bool
P4Lua::ServerCaseSensitive( sol::this_state s )
{
	if( !IsConnected() )
	{
	    luaL_error( s, "P4.server_case_sensitive - Not connected to a Perforce Server.");
	    return false;
	}
	if( !IsCmdRun() )
	    Run( "info", sol::variadic_args(), s );
	return !IsCaseFold();
}

bool
P4Lua::ServerUnicode( sol::this_state s )
{
	if( !IsConnected() )
	{
	    luaL_error( s, "P4.server_unicode - Not connected to a Perforce Server.");
	    return false;
	}
	if( !IsCmdRun() )
	    Run( "info", sol::variadic_args(), s );
	return IsUnicode();
    }


// Check if the supplied path falls within the view of the ignore file
bool
P4Lua::IsIgnored( const char *path )
{
	Ignore *ignore = client->GetIgnore();
	if( !ignore ) return 0;

	StrRef p( path );
	return ignore->Reject( p, client->GetIgnoreFile() );
}

void
P4Lua::Reset()
{
	ui->Reset();
}


//
// Run returns the results of the command. If the client has not been
// connected, then an exception is raised but errors from Perforce
// commands are returned via the Errors() and ErrorCount() interfaces
// and not via exceptions because one failure in a command applied to many
// files would interrupt processing of all the other files if an exception
// is raised.

sol::object
P4Lua::Run( const char* cmd, const sol::variadic_args& args, sol::this_state s )
{
	// Save the entire command string for our error messages. Makes it
	// easy to see where a script has gone wrong.

	StrBuf	cmdString;
	cmdString << "\"p4 " << cmd;
	for( int i = 0; i < args.size(); i++ )
	    cmdString << " " << args[i].get< std::string >().c_str();
	cmdString << "\"";

	if ( P4LUA_COMMANDS )
	    fprintf( stderr, "[P4] Executing %s\n", cmdString.Text()  );

	if ( depth )
	{
	    //rb_warn( "Can't execute nested Perforce commands." );
	    return sol::make_object( s, false );
	}

	// Clear out any results from the previous command
	ui->Reset();

	if( !IsConnected() && exceptionLevel )
	{
	    luaL_error( s, "P4#run - not connected." );
	    return sol::make_object( s, false );
	}

	if( !IsConnected() )
	    return sol::make_object( s, false );

	// Tell the UI which command we're running.
	ui->SetCommand( cmd );


	std::vector<std::vector<char>> vstrings;
	std::vector<char*> cstrings;
	cstrings.reserve(args.size());
	for( size_t i = 0; i < args.size(); ++i )
	{
	    std::string s = args[i].get< std::string >();
	    vstrings.emplace_back(s.begin(), s.end());
	    vstrings.back().push_back('\0');
	    cstrings.push_back(vstrings.back().data());
	}

	depth++;
	ui->SetState( &s );
	RunCmd( cmd, (int)cstrings.size(), (char**)cstrings.data() );
	depth--;

	if( ui->GetHandler() ) {
	    if( client->Dropped() && !ui->IsAlive() ) {
		Disconnect( s );
		ConnectOrReconnect( s );
	    }
	}

	//ui->RaiseRubyException();

	P4Result &results = ui->GetResults();

	if( results.ErrorCount() && exceptionLevel )
	{
	    std::string err = "P4#run - Errors during command execution: ";
	    err.append( cmdString.Text() );
	    luaL_error( s, err.c_str() );
	    return sol::make_object( s, false );
	}

	if ( results.WarningCount() && exceptionLevel > 1 )
	{
	    std::string err = "P4#run - Warnings during command execution: ";
	    err.append( cmdString.Text() );
	    luaL_error( s, err.c_str() );
	    return sol::make_object( s, false );
	}

	return results.GetOutput( s );
}


void
P4Lua::RunCmd( const char *cmd, int argc, char * const *argv )
{
	client->SetProg( &prog );
	if( version.Length() )
	    client->SetVersion( &version );

	if( IsTag() )
	    client->SetVar( "tag" );

	if ( GetStreams() && apiLevel > 69 )
	    client->SetVar( "enableStreams", "" );

	if ( GetGraph() && apiLevel > 81 )
	    client->SetVar( "enableGraph", "" );

	// If maxresults or maxscanrows is set, enforce them now
	if( maxResults  )	client->SetVar( "maxResults",  maxResults  );
	if( maxScanRows )	client->SetVar( "maxScanRows", maxScanRows );
	if( maxLockTime )	client->SetVar( "maxLockTime", maxLockTime );

	//	If progress is set, set progress var.
	if( ui->GetProgress() ) client->SetVar( P4Tag::v_progress, 1 );

	client->SetArgv( argc, argv );
	client->Run( cmd, ui );

	// Can only read the protocol block *after* a command has been run.
	// Do this once only.
	if( !IsCmdRun() )
	{
	    StrPtr *s = 0;
	    if ( (s = client->GetProtocol(P4Tag::v_server2)) )
		server2 = s->Atoi();

	    if( (s = client->GetProtocol(P4Tag::v_unicode)) )
		if( s->Atoi() )
		    SetUnicode();

	    if( (s = client->GetProtocol(P4Tag::v_nocase)) )
		SetCaseFold();
	}
	SetCmdRun();
}


//
// Parses a string supplied by the user into a hash. To do this we need
// the specstring from the server. We try to cache those as we see them,
// but the user may not have executed any commands to allow us to cache
// them so we may have to fetch the spec first.
//

sol::object
P4Lua::ParseSpec( const char * type, const char *form, sol::this_state luaState )
{
	if ( !specMgr->HaveSpecDef( type ) )
	{
	    if( exceptionLevel )
	    {
		StrBuf m;
		m = "P4#parse_spec - No spec definition for ";
		m.Append( type );
		m.Append( " objects." );
		luaL_error( luaState, m.Text() );
	    }
	    return sol::make_object( luaState, false );
;
	}

	// Got a specdef so now we can attempt to parse it.
	Error e;
	sol::table v = specMgr->StringToSpec( type, form, &e, luaState );

	if ( e.Test() )
	{
	    if( exceptionLevel )
	    {
		StrBuf buf;
		e.Fmt( buf, EF_PLAIN );
		std::string err = "P4#parse_spec - ";
		err.append( buf.Text() );
		luaL_error( luaState, err.c_str() );
	    }
	    return sol::make_object( luaState, false );
	}
	return v;
}


//
// Converts a hash supplied by the user into a string using the specstring
// from the server. We may have to fetch the specstring first.
//

sol::object
P4Lua::FormatSpec( const char * type, sol::table hash, sol::this_state luaState )
{
	if ( !specMgr->HaveSpecDef( type ) )
	{
	    if( exceptionLevel )
	    {
		StrBuf m;
		m = "P4#format_spec - No spec definition for ";
		m.Append( type );
		m.Append( " objects." );
		luaL_error( luaState, m.Text() );
	    }
	    return sol::make_object( luaState, false );
	}

	// Got a specdef so now we can attempt to convert.
	StrBuf	buf;
	Error	e;

	specMgr->SpecToString( type, hash, buf, &e, luaState );
	if( !e.Test() )
	{
	    return sol::make_object(luaState, std::string( buf.Text() ) );
	}

	if( exceptionLevel )
	{
	    StrBuf m;
	    m = "Error converting hash to a string.";
	    if( e.Test() ) e.Fmt( m, EF_PLAIN );
	    std::string err = "P4#format_spec - ";
	    err.append( m.Text() );
	    luaL_error( luaState, err.c_str() );
	}
	return sol::make_object( luaState, false );
}

//
// Returns a hash whose keys contain the names of the fields in a spec of the
// specified type. Not yet exposed to Ruby clients, but may be in future.
//
sol::object
P4Lua::SpecFields( const char * type, sol::this_state luaState )
{
	if ( !specMgr->HaveSpecDef( type ) )
	{
	    if( exceptionLevel )
	    {
		StrBuf m;
		m = "P4#spec_fields - No spec definition for ";
		m.Append( type );
		m.Append( " objects." );
		luaL_error( luaState, m.Text() );
	    }
	    return sol::make_object( luaState, false );
	}

	StrBuf buf;
	Error e;
	sol::table v = specMgr->SpecFields( type, &e, luaState );

	if( !e.Test() )
	    return v;

	if( exceptionLevel )
	{
	    StrBuf m;
	    m = "Error converting hash to a string.";
	    if( e.Test() ) e.Fmt( m, EF_PLAIN );
	    std::string err = "P4#format_spec - ";
	    err.append( m.Text() );
	    luaL_error( luaState, err.c_str() );
	}
	return sol::make_object( luaState, false );
}

//
// Raises an exception or returns false on bad input
//

bool
P4Lua::SetInput( sol::object input, sol::this_state s )
{
	if ( P4LUA_COMMANDS )
	    fprintf( stderr, "[P4] Received input for next command\n" );

	if ( !ui->SetInput( input, s ) )
	{
	    if ( exceptionLevel )
		luaL_error( s, "P4#input - Error parsing supplied data." );
	    return false;
	}
	return true;
}

//
// Sets the handler and connects the SetBreak feature
//
void
P4Lua::SetHandler( void* handler )
{
	if ( P4LUA_COMMANDS )
	    fprintf( stderr, "[P4] Received handler object\n" );

	ui->SetHandler( handler );

	if( !handler )
	    client->SetBreak(NULL);
	else
	    client->SetBreak( ui );
}

void
P4Lua::SetProgress( void* progress )
{
	if ( P4LUA_COMMANDS )
	    fprintf( stderr, "[P4] Received progress object\n" );

	return ui->SetProgress( progress );
}

}

# endif
