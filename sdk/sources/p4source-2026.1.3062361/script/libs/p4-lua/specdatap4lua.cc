/*
 * Copyright 1995, 2019 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# include <stdhdrs.h>

# ifdef HAS_EXTENSIONS

# include <string>
# include <map>
# include <vector>

# include <clientapi.h>
# include <i18napi.h>
# include <spec.h>
# include <debug.h>

# include <p4script.h>
# include <p4script53.h>

# include "p4luadebug.h"
# include "specdatap4lua.h"

namespace P4Lua {
	
StrPtr *
SpecDataP4Lua::GetLine( SpecElem *sd, int x, const char **cmt )
{
	*cmt = 0;
	std::string key;
	std::string val;

	key = std::string( sd->tag.Text(), sd->tag.Length() );

	if( !sd->IsList() )
	{
	    sol::optional<std::string> s = hash[key];
	    if( s ) {
	        last = s->c_str();
	        return &last;
	    }
	    return 0;
	}
	
	sol::optional<sol::table> t = hash[key];
	if( !t )
	    return 0;

	sol::optional<std::string> s = (*t)[ x + 1 ];
	if( s ) {
	    last = s->c_str();
	    return &last;
	}
	return 0;
}

void	
SpecDataP4Lua::SetLine( SpecElem *sd, int x, const StrPtr *v, Error *e )
{
	std::string  key;
	std::string val;
	std::map<int, std::string> ary;
	std::map<std::string, std::map<int, std::string>>::iterator it;
	StrBuf t;

	key = std::string( sd->tag.Text(), sd->tag.Length() );
	val = std::string( v->Text(), v->Length() );

	if( sd->IsList() )
	{
	    sol::table o;
	    const sol::object& obj = hash.get< sol::object >( key );

	    if( obj.get_type() == sol::type::lua_nil )
	    {
	        o = sol::table( *luaState, sol::create );
	        hash.set( key, o );
	    }
	    else
	        o = obj;
	    o.set(x + 1, val);
	}
	else
	{
	    hash.set( key, val );
	}
	return;
}

}

# endif
