/*
 * Copyright 2019 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# include <stdhdrs.h>

# ifdef HAS_EXTENSIONS

# include <limits>
# include <p4script53.h>
# include <strbuf.h>
# include <error.h>
# include <errorlua.h>

void ErrorLua::doBindings( sol::state* lua, sol::table& ns )
{
	ns.new_enum( "ErrorSeverity",
	    "E_EMPTY" , ErrorSeverity::E_EMPTY,
	    "E_INFO"  , ErrorSeverity::E_INFO,
	    "E_WARN"  , ErrorSeverity::E_WARN,
	    "E_FAILED", ErrorSeverity::E_FAILED,
	    "E_FATAL" , ErrorSeverity::E_FATAL
	);

	ns.new_usertype< ErrorId >( "ErrorId",
	    "code", []( ErrorId* eid ) { return eid->code; },
	    "fmt", []( ErrorId* eid ) { return std::string( eid->fmt ); },
	    "SubCode", &ErrorId::SubCode,
	    "SubSystem", &ErrorId::Subsystem,
	    "Generic", &ErrorId::Generic,
	    "ArgCount", &ErrorId::ArgCount,
	    "Severity", &ErrorId::Severity,
	    "UniqueCode", &ErrorId::UniqueCode
	);

	ns.new_usertype< Error >( "Error",
	    "clone", []( Error* e )
	    {
	        auto e1 = std::make_unique< Error >();
	        *e1 = *e;
	        e1->Snap();
	        return e1;
	    },
	    "Test" , []( Error* e ) -> bool { return e->Test(); },
	    "Fmt"  , []( Error* e )
	    {
	        StrBuf fmt;
	        e->Fmt( &fmt );
	        return std::string( fmt.Text() );
	    },
	    sol::meta_function::to_string, []( Error* e )
	    {
	        StrBuf fmt;
	        e->Fmt( &fmt );
	        return std::string( fmt.Text() );
	    },
	    "Clear", &Error::Clear,
	    "IsInfo", &Error::IsInfo,
	    "IsWarning", &Error::IsWarning,
	    "IsError", &Error::IsError,
	    "IsFatal", &Error::IsFatal,
	    "GetSeverity", &Error::GetSeverity,
	    "FmtSeverity", []( Error* e )
	    { std::string fmt = e->FmtSeverity(); return fmt; },
	    "GetGeneric", &Error::GetGeneric,
	    "Set", sol::overload( []( Error* e, ErrorSeverity s, const char *fmt ){ e->Set( s, fmt ); } ),
	    "GetErrorCount", &Error::GetErrorCount,
	    "GetId", &Error::GetId,
	    "CheckId", &Error::CheckId,
	    "CheckIds", &Error::CheckIds,
	    "Snap", &Error::Snap,
	    "Dump", &Error::Dump
	);
}

# endif
