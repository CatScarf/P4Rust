/*
 * Copyright 1995, 2019 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# include <stdhdrs.h>

# ifdef HAS_EXTENSIONS

# include <string>
# include <vector>

# include <clientapi.h>
# include <mapapi.h>
# include <debug.h>
# include <p4script.h>
# include <p4script53.h>

# include "p4luadebug.h"
# include "p4mapmaker.h"

namespace P4Lua {

P4MapMaker::P4MapMaker()
{
	map = new MapApi;
}

P4MapMaker::~P4MapMaker()
{
	delete map;
}

P4MapMaker::P4MapMaker( const P4MapMaker &m )
{
	StrBuf	l, r;
	const StrPtr *s;
	MapType	t;
	int 	i;

	map = new MapApi;
	for( i = 0; i < m.map->Count(); i++ )
	{
	    s = m.map->GetLeft( i );
	    if( !s ) break;
	    l = *s;

	    s = m.map->GetRight( i );
	    if( !s ) break;
	    r = *s;

	    t = m.map->GetType( i );

	    map->Insert( l, r, t );
	}
}

void
P4MapMaker::doBindings( sol::state* lua, sol::table& ns )
{
	ns.new_usertype< P4MapMaker >( "Map",
	    sol::constructors<P4MapMaker(), P4MapMaker(P4MapMaker)>(),
	    "join",      sol::overload( [] ( P4MapMaker& m, P4MapMaker& r )
	                                { return P4MapMaker::Join( m, r ); },
	                                [] ( P4MapMaker& m, P4MapMaker& l,
	                                                    P4MapMaker& r )
	                                { return P4MapMaker::Join( l, r ); }),
	    "insert",    sol::overload( []( P4MapMaker& m, std::string l )
	                                { m.Insert( l ); },
	                                []( P4MapMaker& m, std::string l,
	                                                   std::string r )
	                                { m.Insert( l, r ); } ),
	    "clear",     &P4MapMaker::Clear,
	    "count",     &P4MapMaker::Count,
	    "isempty",   &P4MapMaker::IsEmpty,
	    "translate", sol::overload( [] ( P4MapMaker& m, std::string l,
	                                     sol::this_state s )
	                                { return m.Translate( l, 1, s ); },
	                                [] ( P4MapMaker& m, std::string l,
	                                     bool f, sol::this_state s )
	                                { return m.Translate( l, f, s ); } ),
	    "reverse",   &P4MapMaker::Reverse,
	    "includes",  &P4MapMaker::Includes,
	    "lhs",       &P4MapMaker::Lhs,
	    "rhs",       &P4MapMaker::Rhs,
	    "to_a",      &P4MapMaker::ToArray
	);
}

P4MapMaker_t
P4MapMaker::Join( P4MapMaker l, P4MapMaker r )
{
	P4MapMaker_t m( new P4MapMaker() );
	delete m->map;

	m->map = MapApi::Join( l.map, r.map );
	return m;
}

void
P4MapMaker::Insert( std::string m )
{
	StrBuf  in;
	StrBuf  lbuf;
	StrBuf  r;
	StrRef  l;
	MapType t = MapInclude;

	in = m.c_str();
	SplitMapping( in, lbuf, r );

	l = lbuf.Text();

	// Look for mapType in lhs only. 
	if( l[ 0 ] == '-' )
	{
	    l += 1;
	    t = MapExclude;
	}
	else if( l[ 0 ] == '+' )
	{
	    l += 1;
	    t = MapOverlay;
	}
	else if( l[ 0 ] == '&' )
	{
	    l += 1;
	    t = MapOneToMany;
	}

	map->Insert( l, r, t );
}


void
P4MapMaker::Insert( std::string l, std::string r )
{
	StrBuf  left;
	StrBuf  right;
	StrBuf *dest = &left;
	int     quoted = 0;
	int     index = 0;

	const char *p;
	MapType	t = MapInclude;

	p = l.c_str();
	for( ; ; )
	{
	    for( index = 0; *p; p++ )
	    {
	        switch( *p )
	        {
	        case '"':
	            quoted = !quoted;
	            break;

	        case ' ':
	        case '\t':
	            // Embedded whitespace ok; leading not.
	            if( quoted || index )
	            {
	                dest->Extend( *p );
	                index++;
	            }
	            break;

	        case '-':
	            if( !index )
	                t = MapExclude;
	            else 
	                dest->Extend( *p );
	            index++;
	            break;
	        
	        case '+':
	            if( !index )
	                t = MapOverlay;
	            else 
	                dest->Extend( *p );
	            index++;
	            break;

	        case '&':
	            if( !index )
	                t = MapOneToMany;
	            else 
	                dest->Extend( *p );
	            index++;
	            break;

	        default:
	            dest->Extend( *p );
	            index++;
	        }
	    }

	    if( dest == &right )
	        break;

	    dest = &right;
	    p = r.c_str();
	    quoted = 0;
	}
	left.Terminate();
	right.Terminate();

	map->Insert( left, right, t );
}

int
P4MapMaker::Count()
{
	return map->Count();
}

void
P4MapMaker::Clear()
{
	map->Clear();
}

void
P4MapMaker::Reverse()
{
	MapApi       *nmap = new MapApi;
	const StrPtr *l;
	const StrPtr *r;
	MapType       t;

	for( int i = 0; i < map->Count(); i++ )
	{
	    l = map->GetLeft( i );
	    r = map->GetRight( i );
	    t = map->GetType( i );

	    nmap->Insert( *r, *l, t );
	}

	delete map;
	map = nmap;
}

sol::object
P4MapMaker::Translate( std::string p, int fwd, sol::this_state s )
{
	StrBuf from;
	StrBuf to;
	MapDir dir = MapLeftRight;

	if( !fwd )
	    dir = MapRightLeft;

	from = p.c_str();
	if( map->Translate( from, to, dir ) )
	    return sol::make_object( s, std::string( to.Text(), to.Length() ) );
	return sol::lua_nil;
}

bool
P4MapMaker::Includes( std::string p )
{
	StrBuf from;
	StrBuf to;
	from = p.c_str();
	if( map->Translate( from, to, MapLeftRight ) )
	    return true;
	if( map->Translate( from, to, MapRightLeft ) )
	    return true;
	return false;
}

sol::table
P4MapMaker::Lhs( sol::this_state state )
{
	sol::table      a( state, sol::create );
	StrBuf          s;
	const StrPtr   *l;
	MapType         t;
	int             quote;

	for( int i = 0; i < map->Count(); i++ )
	{
	    s.Clear();
	    quote = 0;

	    l = map->GetLeft( i );
	    t = map->GetType( i );

	    if( l->Contains( StrRef( " " ) ) )
	    {
	        quote++;
	        s << "\"";
	    }

	    switch( t )
	    {
	    case MapInclude:
	        break;
	    case MapExclude:
	        s << "-";
	        break;
	    case MapOverlay:
	        s << "+";
	    case MapOneToMany:
	        s << "&";
	    };

	    s << l->Text();
	    if( quote ) s << "\"";

	    a.add( std::string( s.Text(), s.Length() ) );
	}
	return a;
}

sol::table
P4MapMaker::Rhs( sol::this_state state )
{
	sol::table      a( state, sol::create );
	StrBuf          s;
	const StrPtr   *r;
	int             quote;

	for( int i = 0; i < map->Count(); i++ )
	{
	    s.Clear();
	    quote = 0;

	    r = map->GetRight( i );

	    if( r->Contains( StrRef( " " ) ) )
	    {
	        quote++;
	        s << "\"";
	    }

	    s << r->Text();
	    if( quote ) s << "\"";

	    a.add( std::string( s.Text(), s.Length() ) );
	}
	return a;
}

sol::table
P4MapMaker::ToArray( sol::this_state state )
{
	sol::table      a( state, sol::create );
	StrBuf          s;
	const StrPtr   *l;
	const StrPtr   *r;
	MapType         t;
	int             quote;

	for( int i = 0; i < map->Count(); i++ )
	{
	    s.Clear();
	    quote = 0;

	    l = map->GetLeft( i );
	    r = map->GetRight( i );
	    t = map->GetType( i );

	    if( l->Contains( StrRef( " " ) ) ||
	        r->Contains( StrRef( " " ) ) )
	    {
	        quote++;
	        s << "\"";
	    }

	    switch( t )
	    {
	    case MapInclude:
	        break;
	    case MapExclude:
	        s << "-";
	        break;
	    case MapOverlay:
	        s << "+";
	    case MapOneToMany:
	        s << "&";
	    };

	    s << l->Text();

	    if( quote ) s << "\" \"";
	    else s << " ";

	    s << r->Text();
	    if( quote ) s << "\"";

	    a.add( std::string( s.Text(), s.Length() ) );
	}
	return a;
}

//
// Take a single string containing either a half-map, or both halves of
// a mapping and split it in two. If there's only one half of a mapping in
// the input, then l, and r are set to the same value as 'in'. If 'in'
// contains two halves, then they are split.
//
void
P4MapMaker::SplitMapping( const StrPtr &in, StrBuf &l, StrBuf &r )
{
	char   *pos;
	int     quoted = 0;
	int     split = 0;
	StrBuf *dest = &l;

	pos = in.Text();

	l.Clear();
	r.Clear();

	while( *pos )
	{
	    switch( *pos )
	    {
	    case '"':
	        quoted = !quoted;
	        break;

	    case ' ':
	        if( !quoted && !split )
	        {
	            // whitespace in the middle. skip it, and start updating
	            // the destination
	            split = 1;
	            dest->Terminate();
	            dest = &r;
	        }
	        else if( !quoted )
	        {
	            // Trailing space on rhs. ignore
	        }
	        else
	        {
	            // Embedded space
	            dest->Extend( *pos );
	        }
	        break;

	    default:
	        dest->Extend( *pos );
	    }
	    pos++;
	}
	l.Terminate();
	r.Terminate();

	if( !r.Length() )
	    r = l;
}

}

# endif
