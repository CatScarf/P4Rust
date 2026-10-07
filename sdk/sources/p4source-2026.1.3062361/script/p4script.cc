/*
 * Copyright 2018 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

#include <iomanip>
#include <sstream>

// For doFile()
#include <stdhdrs.h>
#include <error.h>
#include <clientapi.h>

#include <p4script.h>
#include <msgscript.h>
#include <msgdm.h>
#include <debug.h>
#include <charset.h>

# ifdef HAS_EXTENSIONS

p4script::impl::impl( p4script& p, Error *e )
: parent( p )
{
}

p4script::impl::~impl()
{
}

bool p4script::impl::doFile( const char *name, Error *e )
{
	return false;
}

bool p4script::impl::doStr( const char *buf, Error *e )
{
	return false;
}

const char* p4script::impl::getImplName() const
{
	return "no scripting engine";
}

const char* p4script::impl::getImplVersion() const
{
	return "no scripting engine version";
}

p4script::impl* p4script::GetImpl()
{
	return pimpl.get();
}

bool p4script::DebugStrToEnum( const char* type, DEBUG& ret )
{
	StrRef t( type );

	if( t == "none" )
	{
	    ret = DEBUG::NONE;
	    return true;
	}

	if( t == "tracing" )
	{
	    ret = DEBUG::TRACING;
	    return true;
	}

	if( t == "crystal_ball" )
	{
	    ret = DEBUG::CRYSTAL_BALL;
	    return true;
	}

	return false;
}

bool p4script::DebugEnumToStr( const DEBUG type, std::string& ret )
{
	switch( type )
	{
	    case DEBUG::NONE:
	        ret = "none";
	    case DEBUG::TRACING:
	        ret = "tracing";
	    case DEBUG::CRYSTAL_BALL:
	        // Testing placeholder.
	        ret = "crystal_ball";
	    default:
	        return false;
	}

	return true;
}

bool p4script::SupportsDebugType( const DEBUG type ) const
{
	return pimpl->SupportsDebugType( type );
}

void p4script::SetDebug( const DEBUG type, const StrBuf* id,
	                 const StrBuf& path, Error* e )
{
	pimpl->SetDebug( type, id, path, e );
}

int p4script::os_execute( void* Lv )
{
	switch( ((p4script*) Lv)->scriptType )
	{
	    case P4SCRIPT_LUA_53:
	        return ((p4script*)Lv)->pimpl->os_execute( Lv );
	        break;
	    default:
	        p4debug.printf( "p4script::os_execute() bad case!\n" );
	        return 1;
	}
}

bool p4script::timeBreakCb( void* Lv )
{
	switch( static_cast< p4script* >( Lv )->scriptType )
	{
	    case P4SCRIPT_LUA_53:
	        return ((p4script*)Lv)->pimpl->timeBreakCb( Lv );
	        break;
	    default:
	        p4debug.printf( "p4script::timeBreakCb() bad case!\n" );
	        return 1;
	}
}

void p4script::debugCb( void* Lv, void* arv )
{
	switch( static_cast< p4script* >( Lv )->scriptType )
	{
	    case P4SCRIPT_LUA_53:
	        ((p4script*)Lv)->pimpl->debugCb( Lv, arv );
	        return;
	        break;
	    default:
	        p4debug.printf( "p4script::debugCb() bad case!\n" );
	        return;
	}
}

void p4script::SetRealError( void* Lv, const Error* e )
{
	switch( static_cast< p4script* >( Lv )->scriptType )
	{
	    case P4SCRIPT_LUA_53:
	        ((p4script*)Lv)->pimpl->SetRealError( e );
	        return;
	        break;
	    default:
	        p4debug.printf( "p4script::SetRealError() bad case!\n" );
	        return;
	}
}

p4script::~p4script()
{
	pimpl.reset();
}

p4script::p4script( const SCR_VERSION v, const int apiVersion, Error *e )
  : apiVersion( apiVersion ), scriptType( v )
{
	switch( v )
	{
	    case P4SCRIPT_LUA_53:
	        pimpl = std::make_unique< p4script::impl53 >( *this, e );
	        break;
	    default:
	        StrBuf msg = "p4script::p4script() bad case!";
	        e->Set( MsgDm::DevErr ) << msg;
	        return;
	}
}

void p4script::SetMaxTime( const uint32_t max )
{
	maxTime = (std::chrono::seconds)max;
}

void p4script::SetMaxMem( const scriptMem_t max )
{
	maxMem = max * 1048576; // megabytes -> bytes
}

void p4script::ConfigBinding( const SCR_BINDING_LIBNAME lib, p4_std_any::p4_any cfg,
	                      Error* e )
{
	try
	{
	switch( lib )
	{
	    case P4SCRIPT_CLIENTAPI:
	        ClientApiBindCfgs.push_back(
	            p4_std_any::any_cast< std::function< void( ClientApi& ) >
	                         >( cfg ) );
	        break;
	    case P4SCRIPT_CLIENTUSER:
	        ClientUserBindCfgs.push_back(
	            p4_std_any::any_cast< std::function< void( ClientUserLua& ) >
	                         >( cfg ) );
	        break;
	    case P4SCRIPT_LUA53:
	        LuaBindCfgs.push_back(
	            p4_std_any::any_cast< std::function< bool( SCR_BINDING_LUA_OPTS ) >
	                         >( cfg ) );
	        break;

	    default:
	        StrBuf msg = "p4script::ConfigBinding() bad case!";
	        e->Set( MsgDm::DevErr ) << msg;
	        return;
	}
	}
	catch( const std::exception& err )
	{
	   StrBuf msg = "p4script::ConfigBinding() error! ";
	   msg << err.what();
	   e->Set( MsgDm::DevErr ) << msg;
	   return;
	}
}

bool p4script::checkMem() const
{
	return maxMem ? curMem >= maxMem : false;
}

bool p4script::checkTime()
{
	addTime();

	return maxTime.count() ? curTime >= maxTime : false;
}

void p4script::beginTime()
{
	startTime = Now();
}

void p4script::addTime()
{
	curTime += scriptTime_t( Now() - startTime );
	beginTime();
}

scriptTimeInc_t p4script::Now() const
{
	return scriptClock_t::now();
}

bool p4script::doFile( const char *name, Error *e )
{
	if( e->Test() )
	{
	    e->Set( MsgScript::DoNotBlameTheScript );
	    return false;
	}

	GlobalCharSet::UseAlt( true );
	const bool r = pimpl->doFile( name, e );
	GlobalCharSet::UseAlt( false );

	return r;
}

bool p4script::doStr( const char *buf, Error *e )
{
	if( e->Test() )
	{
	    e->Set( MsgScript::DoNotBlameTheScript );
	    return false;
	}

	GlobalCharSet::UseAlt( true );
	const bool r = pimpl->doStr( buf, e );
	GlobalCharSet::UseAlt( false );

	return r;
}

p4_std_any::p4_any p4script::doScriptFn( const char* name, Error* e )
{
	if( e->Test() )
	{
	    e->Set( MsgScript::DoNotBlameTheScript );
	    return {};
	}

	GlobalCharSet::UseAlt( true );
	auto r = pimpl->doScriptFn( name, e );
	GlobalCharSet::UseAlt( false );

	return r;
}

bool p4script::fnExists( const char* name )
{
	return pimpl->fnExists( name );
}

std::string p4script::fmtDuration( const scriptTime_t &dur ) const
{
	std::stringstream buf;

	auto millis = std::chrono::duration_cast
	              < std::chrono::milliseconds >( dur ).count();

	int hours = ( millis / ( 1000 * 60 * 60 ) );
	int mins  = ( millis / ( 1000 * 60 ) ) % 60;
	int secs  = ( millis / 1000) % 60;

	buf << std::setfill( '0' ) << std::setw( 2 ) << hours << ":";
	buf << std::setfill( '0' ) << std::setw( 2 ) << mins  << ":";
	buf << std::setfill( '0' ) << std::setw( 2 ) << secs        ;

	return buf.str();
}

std::string p4script::fmtMem( const scriptMem_t &mem ) const
{
	std::stringstream buf;
	buf << mem;
	return buf.str();
}

std::string p4script::getElapsedTime() const
{
	return fmtDuration( curTime );
}

int p4script::getAPIVersion() const
{
	return apiVersion;
}

const char* p4script::getImplName() const
{
	return pimpl->getImplName();
}

const char* p4script::getImplVersion() const
{
	return pimpl->getImplVersion();
}

# else // HAS_EXTENSIONS

# include <stdio.h>

p4script::p4script( const SCR_VERSION v, Error *e )
{
	fprintf( stderr, "Warning: scripting support is not "
	                 "compiled into this program.\n" );
}

p4script::~p4script() {}

bool p4script::doFile( const char *name, Error *e )
{
	return false;
}

void p4script::SetMaxTime( const unsigned int max )
{
}

void p4script::SetMaxMem( const scriptMem_t max )
{
}

# endif
