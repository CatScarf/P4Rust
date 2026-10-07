/*
 * Copyright 1995, 2019 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# include <stdhdrs.h>

# ifdef HAS_EXTENSIONS

# include <string>
# include <map>

# include <clientapi.h>
# include <p4script.h>
# include <p4script53.h>

# include "p4luadebug.h"
# include "p4error.h"

namespace P4Lua {

P4Error::P4Error( const Error &other )
{
	this->debug = 0;
	error = other;
}

void
P4Error::doBindings( sol::state* lua, sol::table& ns )
{
	ns.new_usertype< P4Error >( "Message",
	    "msgid",     &P4Error::GetId,
	    "generic",   &P4Error::GetGeneric,
	    "serverity", &P4Error::GetSeverity,
	    "inspect",   &P4Error::Inspect,
	    "to_s",      &P4Error::GetText
	);
}

int
P4Error::GetId()
{
	ErrorId *id = error.GetId( 0 );
	if( !id )
	    return 0;
	return id->UniqueCode();
}

int
P4Error::GetGeneric()
{
	return error.GetGeneric();
}

int
P4Error::GetSeverity()
{
	return error.GetSeverity();
}

std::string
P4Error::GetText()
{
	StrBuf t;
	error.Fmt( t, EF_PLAIN );
	return std::string( t.Text(), t.Length() );
}

sol::table
P4Error::GetDict( sol::this_state s )
{
	sol::table hash( s, sol::create );
	StrRef key, val;
	for( int i = 0; error.GetDict()->GetVar( i, key, val ) != 0; i++ ) {
	     hash.set( std::string( key.Text(), key.Length() ),
	               std::string( val.Text(), val.Length() ) );
	}
	return hash;
}

std::string
P4Error::Inspect()
{
	StrBuf a;
	StrBuf b;

	error.Fmt( a, EF_PLAIN );
	b << "[";
	b << "Gen:" << error.GetGeneric();
	b << "/Sev:" << error.GetSeverity();
	b << "]: ";
	b << a;
	return std::string( b.Text(), b.Length() );
}

}

# endif
