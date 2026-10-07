/*
 * Copyright 2018 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# define NEED_FILE
# include <stdhdrs.h>

# ifdef HAS_EXTENSIONS

# include <map>
# include <vector>
# include <deque>
# include <functional>
# include <fstream>
# include <iostream>
# include <thread>

/*
// For printf PRIu32
# define __STDC_FORMAT_MACROS
# include <inttypes.h>
// #include <cinttypes> ?
// printf("thz fn %" PRIu32 "\n", z->parent.maxRam );
*/

# include "p4script53.h"
# include "lua_cjson_redefines.h"

# include "clientapi.h"
# include "clientapilua.h"
# include "clientuserlua.h"
# include "filesyslua.h"
# include "errorlua.h"

# include <error.h>
# include <msgscript.h>
# include <debug.h>

# include <runcmd.h>
# include <enviro.h>
# include <strarray.h>
# include <pid.h>
# include <pathsys.h>
# include <datetime.h>

# include <p4error.h>
# include <p4result.h>
# include <clientuserp4lua.h>
# include <p4lua.h>
# include <p4mapmaker.h>

# include <p4script.h>
# include <debugextension.h>

int p4script::impl53::os_execute( void* Lv )
{
	auto lua = cast_sol_State( l );
	lua_State *L = lua->lua_state();
	const char *cmdLua = luaL_optstring( L, 1, NULL );
	StrBuf cmd = cmdLua;
	cmd.TrimBlanks();

	RunArgv args;
	RunCommand rcmd;
	Error e;
	int fds[ 2 ];
	fds[ 0 ] = fds[ 1 ] = -1;

# ifdef OS_NT
	Enviro env;
	const char* comSpec = env.Get( "ComSpec" );
	args.AddCmd( comSpec ? comSpec : "cmd" );
	args.AddArg( "/C" );
	args.AddArg( cmd.Text() );
# else
	args.AddCmd( "sh" );
	args.AddArg( "-c" );
	args.AddArg( cmd );
# endif

	rcmd.RunChild( args, ( RCO_AS_SHELL | RCO_USE_STDOUT ), fds, &e );

	bool failed = false;

	// Polling instead of blocking here like system() means we'll be
	// slower when spawning many short-lived commands.

	while( !rcmd.PollChild( 100 ) )
	{
	    if( parent.checkTime() )
	    {
	        realError.Set( MsgScript::ScriptMaxRunErr )
	            << "time"
	            << parent.fmtDuration( parent.maxTime ).c_str();

	        if( p4debug.GetLevel( DT_SCRIPT ) > 3 )
	            p4debug.printf( "SCRIPT p4/os_execute scriptCancelMsg"
	                            " block\n" );

	        failed = true;
	        parent.scriptCancelled = true;
	        rcmd.StopChild();
	        // The subprocess can still be hanging around at this point
	        // despite us sending a signal.  We don't want to stall here,
	        // so we just disassociate ourselves with it and continue on.
	        rcmd.SetAbandon();
	        luaL_error( L, "p4/os_execute" );
	        rcmd.WaitChild();
	        break;
	    }

	    std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
	}

	if( fds[ 0 ] > 0 )
	    close( fds[ 0 ] );

	if( fds[ 1 ] > 0 )
	    close( fds[ 1 ] );

	if( e.Test() )
	{
	    StrBuf fmt, msg;
	    e.Fmt( &fmt );
	    msg << "p4/os_execute: " << fmt;
	    return luaL_error( L, msg.Text() );
	}

	int status = failed ? 1 : 0;

	if( cmdLua != NULL )
	    return luaL_execresult( L, e.Test() || status );
	else
	{
	    lua_pushboolean( L, e.Test() || status );
	    return 1;
	}
}

bool p4script::impl53::timeBreakCb( void* Lv )
{
	return false;
}

static void debugHookShim( lua_State* L, lua_Debug* arv )
{
	void *ud = nullptr;
	lua_getallocf( L, &ud );

	auto z = static_cast< p4script* >( ud );
	z->debugCb( z, arv );
}

p4script::impl53::Debug::Debug() : pid( StrNum( Pid().GetID() ) )
{
	type = DEBUG::NONE;
	now = std::unique_ptr< DateTimeHighPrecision >
	          ( new DateTimeHighPrecision() );
}

p4script::impl53::Debug::~Debug()
{
	if( !log )
	    return;

	Error eIgnore;

	buf.Clear();
	now->Now();
	LogHeader( buf );
	buf << "End of script\n\n";
	log->Write( buf, &eIgnore );

	log->Close( &eIgnore );
}

void p4script::impl53::Debug::SetDebug( const DEBUG type, const StrBuf* id,
	                                const StrBuf& path, Error* e )
{
	this->type = type;
	this->id = id;

	if( type == DEBUG::TRACING )
	{
	    auto file = PathSys::CreateUPtr();
	    file->SetLocal( path, StrRef( ".p4-debug-tracing.txt" ) );
	    log = FileSys::CreateUPtr( FST_ATEXT );
	    log->Set( file->Text() );
	    log->Open( FOM_RW, e );

	    buf.Clear();
	    now->Now();
	    LogHeader( buf );
	    buf << "Start of script\n";
	    log->Write( buf, e );
	}
}

void p4script::impl53::Debug::LogHeader( StrBuf& buf )
{
	buf << now->Seconds() << "." << now->Nanos() << " " << pid << " "
	    << id->Text() << " ";
}

bool p4script::impl53::Debug::TraceCB( void* _L, void* arv, Error* e )
{
	lua_State* L = static_cast< lua_State* >( _L );
	lua_Debug* info = static_cast< lua_Debug* >( arv );

	lua_getinfo( L, "nSl", info );

	if( strcmp( info->what, "Lua" ) )
	    return true;

	const char* srcFile = info->source + 1;

	// Don't attempt to trace into non-source modules.
	if( strstr( srcFile, "Internal/" ) == srcFile )
	    return true;

	if( info->source && info->source[ 0 ] == '@' &&
	    lines.find( srcFile ) == lines.cend() )
	{
	    std::ifstream is( srcFile );

	    if( is.fail() )
	    {
	        StrBuf msg;
	        msg << "p4script::impl53::Debug::TraceCB() could not open "
	               "source file '" << srcFile << "': " << strerror( errno );
	        e->Set( MsgScript::GenericFatal ) << msg;
	        return false;
	    }

	    std::vector< std::string > fc;

	    for( std::string line; std::getline( is, line );
	         fc.emplace_back( std::move( line ) ) )
	        ;

	    lines[ srcFile ] = std::move( fc );
	}

	const auto& srcLines = std::get< 1 >( *(lines.find( srcFile ) ) );

	int lineNo = -1;

	switch( info->event )
	{
	    case LUA_HOOKCALL:
	        lineNo = info->linedefined;
	        level++;
	        break;
	    case LUA_HOOKRET:
	        // This isn't called on tailcall.
	        lineNo = info->lastlinedefined;
	        level--;
	        break;
	    case LUA_HOOKLINE:
	        lineNo = info->currentline;
	        break;
	    case LUA_HOOKTAILCALL:
	        lineNo = info->linedefined;
	        level++;
	        break;
	}

	if( lineNo == -1 )
	   return true;

	now->Now();
	buf.Clear();

	if( lastFile != srcFile )
	{
	    lastFile = srcFile;
	    LogHeader( buf );
	    buf << lastFile.c_str() << "\n";
	}

	LogHeader( buf );

	char lineNoBuf[ 16 ];
	memset( lineNoBuf, '\0', 16 * sizeof( char ) );
	snprintf( lineNoBuf, 16, "%5d ", lineNo );
	buf << lineNoBuf;

	if( level <= 0 )
	    level = 1;

	for( int i = 0; i < level; i++ )
	    buf << ">>> ";

	buf << " " << ( ( lineNo > srcLines.size() )
	    ? std::string( "unknown" ) : srcLines[ lineNo - 1 ] ).c_str();

	buf << "\n";

	if( info->event == LUA_HOOKRET )
	    level--;

	log->Write( buf, e );

	if( e->Test() )
	    return false;

	return true;
}

p4script::DEBUG p4script::impl53::Debug::GetType() const
{
	return type;
}

void p4script::impl53::debugCb( void* Lv, void* arv )
{
	auto lua = cast_sol_State( l );
	lua_State *L = lua->lua_state();
	const lua_Debug* info = static_cast< lua_Debug* >( arv );

	if( debug.GetType() == DEBUG::TRACING &&
	    info->event != LUA_HOOKCOUNT &&
	    !debug.TraceCB( L, arv, &realError ) )
	{
	    parent.scriptCancelled = true;
	    luaL_error( L, "debugHook" );
	    return;
	}

	if( info->event == LUA_HOOKCOUNT &&
	    !parent.scriptCancelled && parent.checkTime() )
	{
	    realError.Set( MsgScript::ScriptMaxRunErr )
	        << "time"
	        << parent.fmtDuration( parent.maxTime ).c_str();

	    if( p4debug.GetLevel( DT_SCRIPT ) > 3 )
	        p4debug.printf( "SCRIPT p4script::impl53::debugHook "
	                        "scriptCancelMsg block\n" );
	    parent.scriptCancelled = true;
	    luaL_error( L, "debugHook" );
	}
}

void p4script::impl53::SetRealError( const Error* e )
{
	const bool isExit = realError.CheckId( MsgScript::OsExitRealError );

	for( const auto& fn : parent.LuaBindCfgs )
	    if( isExit && !fn( SCR_BINDING_LUA_OPTS::OS_EXIT ) )
	        return;
	
	realError.Clear();
	realError = *e;
	realError.Snap();
}

void* p4script::impl53::allocator( void *ud, void *ptr, size_t osize,
	                           size_t nsize )
{
	auto a = static_cast< p4script* >( ud );
	auto z = static_cast< p4script::impl53* >( a->pimpl.get() );

	if( !a->scriptCancelled && a->checkTime() )
	{
	    z->realError.Set( MsgScript::ScriptMaxRunErr )
	        << "time"
	        << a->fmtDuration( a->maxTime ).c_str();

	    if( p4debug.GetLevel( DT_SCRIPT ) > 3 )
	        p4debug.printf( "SCRIPT p4script::impl53::allocator "
	                        "scriptCancel block\n" );
	    a->scriptCancelled = true;
	    return nullptr;
	}

	a->curMem += nsize - osize;

	if( nsize == 0 ) // a free is always allowed
	{
	    P4_FREE( ptr );
	    return nullptr;
	}

	if( !a->scriptCancelled && a->checkMem() && !z->realError.Test() )
	{
	    z->realError.Set( MsgScript::ScriptMaxRunErr )
	        << "memory"
	        << z->parent.fmtMem( z->parent.maxMem ).c_str()
	        << "bytes";

	    z->parent.scriptCancelled = true;

	    if( p4debug.GetLevel( DT_SCRIPT ) > 3 )
	        p4debug.printf( "SCRIPT p4script::impl53::allocator "
	                        "checkMem block\n" );

	    return NULL;
	}

	return P4_REALLOC( ptr, nsize );
}

const char* p4script::impl53::getImplName() const
{
	return implName;
}

const char* p4script::impl53::getImplVersion() const
{
	return implVersion;
}

p4script::impl53::impl53( p4script& p, Error *e ) : impl( p, e )
{
	if( e->Test() )
	    e->Set( MsgScript::DoNotBlameTheScript );

	parent.beginTime();

	auto lua = new sol::state( nullptr, allocator, &p );

	if( !lua )
	{
	    e->Set( MsgScript::ScriptRuntimeError )
	        << "Couldn't create interpreter - Out of memory?";
	    return;
	}
	lua_State *L = lua->lua_state();
	lua_sethook( L, (lua_Hook)debugHookShim, LUA_MASKCOUNT, maxInsStep );
	sol::set_default_state( L );

	lua->open_libraries( sol::lib::base, sol::lib::package,
	                     sol::lib::coroutine, sol::lib::string,
	                     sol::lib::os, sol::lib::math, sol::lib::table,
	                    /*no debug*/ sol::lib::bit32, sol::lib::io,
	                    /* no ffi/jit*/ sol::lib::count );
	l = lua;
	doBindings();
}

p4script::impl53::~impl53()
{
	delete cast_sol_State( l );
}

void* p4script::impl53::getState()
{
	return l;
}

bool p4script::impl53::SupportsDebugType( const DEBUG type ) const
{
	return type <= DEBUG::TRACING;
}

void p4script::impl53::SetDebug( const DEBUG type, const StrBuf* id,
	                         const StrBuf& path, Error* e )
{
	auto lua = cast_sol_State( l );
	lua_State *L = lua->lua_state();

	switch( type )
	{
	    case DEBUG::NONE:
	        break;
	    case DEBUG::TRACING:
	    {
	        // The current implementation doesn't have an off switch.

	        const int mask = LUA_MASKCOUNT | LUA_MASKCALL | LUA_MASKRET |
	                         LUA_MASKLINE;
	        lua_sethook( L, (lua_Hook)debugHookShim, mask, maxInsStep );

	        debug.SetDebug( type, id, path, e );

	        break;
	    }
	    case DEBUG::CRYSTAL_BALL:
	        break;
	    default:
	        break;
	}
}

bool p4script::impl53::doFile( const char *name, Error *e )
{
	return doCode( name, false, e );
}

bool p4script::impl53::doStr( const char *buf, Error *e )
{
	return doCode( buf, true, e );
}

bool p4script::impl53::doCode( const char *data, const bool isStr, Error *e )
{
	if( e->Test() )
	{
	    e->Set( MsgScript::DoNotBlameTheScript );
	    return false;
	}

	bool ret = true;
	auto lua = cast_sol_State( l );

	parent.beginTime();

	try
	{
	    isStr ? lua->safe_script( data ) : lua->safe_script_file( data );
	}
	catch( const sol::error& err )
	{
	    if( realError.Test() )
	    {
	         if( !realError.CheckId( MsgScript::OsExitRealError ) )
	         {
	             *e = realError;
	             e->Snap();
	          }

	         realError.Clear();
	    }
	    else
	        e->Set( MsgScript::ScriptRuntimeError )
	            << getImplName() << err.what();

	    ret = false;
	}

	parent.addTime();

	return ret;
}

p4_std_any::p4_any
p4script::impl53::doScriptFn( const char* name, Error* e )
{
	DEBUGPRINTF( EXTS_INFO, "Executing script function '%s'.", name );

	if( e->Test() )
	{
	    e->Set( MsgScript::DoNotBlameTheScript );
	    return {};
	}

	auto lua = cast_sol_State( l );

	parent.beginTime();

	try {
	    auto multi = lua->get< sol::protected_function >( name );
	    auto mret = multi();

	    if( !mret.valid() )
	    {
	        sol::error err = mret;

	        if( realError.Test() )
	        {
	             *e = realError;
	             e->Snap();
	             realError.Clear();
	        }
	        else
	            e->Set( MsgScript::ScriptRuntimeError )
	                << getImplName() << err.what();

	        return {};
	    }

	    return p4_std_any::p4_any( static_cast< sol::object >( mret ) );
	}
	catch( const sol::error& err )
	{
	    if( realError.Test() )
	    {
	         *e = realError;
	         e->Snap();
	         realError.Clear();
	    }
	    e->Set( MsgScript::ScriptRuntimeError )
	        << getImplName() << err.what();

	    return {};
	}

	parent.addTime();

	return {};
}

bool p4script::impl53::fnExists( const char* name )
{
	auto lua = cast_sol_State( l );

	const sol::object fn = (*lua)[ name ];
	const sol::type type = fn.get_type();

	return type == sol::type::function;
}

bool solfnCheck( const sol::protected_function_result& r,
	        const char* impl, const char* where, Error* e )
{
	if( r.valid() && !e->Test() )
	    return false;

	// Can't call the what() below on valid data.
	if( r.valid() )
	    return true;

	sol::error err = r;
	StrBuf msg;
	msg << where << ": " << err.what();
	e->Set( MsgScript::ScriptRuntimeError ) << impl << msg;

	return true;
}

void solExcpESet( const sol::error& err, const char* impl,
	          const char* where, Error* e )
{
	StrBuf msg;
	msg << where << ": " << err.what();
	e->Set( MsgScript::ScriptRuntimeError ) << impl << msg;
}

extern int luaopen_cjson( lua_State *l );
extern int luaopen_cjson_safe( lua_State *l );
extern int luaopen_lsqlite3( lua_State *L );
extern int luaopen_lcurl( lua_State *L );
extern int luaopen_lcurl_safe( lua_State *L );

# include "libs/lua-curlv3/lua/cURL/impl/curl.lua.c"
# include "libs/lua-curlv3/lua/cURL/safe.lua.c"
# include "libs/lua-curlv3/lua/cURL/utils.lua.c"
# include "libs/lua-curlv3/lua/curl.lua.c"
# include "libs/argparse/argparse.lua.cc"

static int loadInlineLuaModule( lua_State *L )
{
	const StrRef name( luaL_checkstring( L, 1 ) );
	StrBuf filename = "@Internal/";
	filename << name;

	const auto fn = [&]( const char* data )
	{
	    if( luaL_loadbuffer( L, data, strlen( data ),
	                         filename.Text() ) == LUA_OK )
	    {
	        lua_pushstring( L, filename.Text() );
	        return 2;
	    }

	    return luaL_error( L, "module loading error '%s' from"
	                          " file '%s':\n\t%s",
	                           lua_tostring( L, 1 ), filename.Text(),
	                          lua_tostring( L, -1 ) );
	};

	if( name == "cURL" )
	    return fn( curl_str );

	if( name == "cURL.safe" )
	    return fn( curlsafe_str );

	if( name == "cURL.utils" )
	    return fn( curlutils_str );

	if( name == "cURL.impl.cURL" )
	    return fn( curlimpl_str );

	if( name == "argparse" )
	    return fn( argparse_str );

	return 1;
}

void p4script::impl53::doBindings()
{
	auto lua = cast_sol_State( l );

	luaL_requiref( lua->lua_state(), "cjson", luaopen_cjson, 1 );
	luaL_requiref( lua->lua_state(), "cjson.safe", luaopen_cjson_safe, 1 );

	// LuaSQLite3 - normally its import for the statically-linked version
	// is 'lsqlite3complete', but that's a bit long so we just call it by
	// the normal name.
	luaL_requiref( lua->lua_state(), "lsqlite3", luaopen_lsqlite3, 1 );

	luaL_requiref( lua->lua_state(), "lcurl", luaopen_lcurl, 1 );
	luaL_requiref( lua->lua_state(), "lcurl.safe", luaopen_lcurl_safe, 1 );

	sol::table searchers = (*lua)[ "package" ][ "searchers" ];
	searchers.add( loadInlineLuaModule );

	sol::table ns = lua->create_named_table( "Helix" )
	    .create_named( "Core" ).create_named( "P4API" );

	ErrorLua::doBindings( lua, ns );

	ClientUserLua::doBindings( lua, ns, &parent.ClientUserBindCfgs,
	                           getImplName(), parent.apiVersion );

	FileSysLua::doBindings( lua, ns, getImplName(), parent.apiVersion );

	ClientApiLua::doBindings( lua, &ns, &parent.ClientApiBindCfgs );

	if( parent.apiVersion == 1 )
	{
	    sol::table ns182 = lua->create_named_table( "Perforce" );

	    // Namespace Perforce -> Helix.Core.P4API.
	    ns182[ "Error"  ] = ns[ "Error"  ];
	    ns182[ "ErrorSeverity"  ] = ns[ "ErrorSeverity"  ];

	    // Class name changes.
	    ns182[ "ClientApiLua"  ] = ns[ "ClientApi"  ];
	    ns182[ "ClientUserLua" ] = ns[ "ClientUser" ];
	    ns182[ "FileSysLua"    ] = ns[ "FileSys"    ];
	}
	
	// P4Lua uses P4 namespace

	sol::table ns2 = lua->create_named_table( "P4" );

	P4Lua::P4Lua::doBindings( lua, &ns2, &parent.ClientApiBindCfgs );
	P4Lua::P4MapMaker::doBindings( lua, ns2 );
	P4Lua::P4Error::doBindings( lua, ns2 );
}

# else

/*

// Diagnostics to see why we ended up here.

#ifdef OS_NT

#define STRING2(x) #x
#define STRING(x) STRING2(x)

#pragma message("_MSC_VER")
#pragma message(STRING(_MSC_VER))
#pragma message("__cplusplus")
#pragma message(STRING(__cplusplus))

#endif // OS_NT

error

*/

# endif // HAS_EXTENSIONS
