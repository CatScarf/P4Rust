/*
 * Copyright 1995, 2019 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

class MapApi;

namespace P4Lua {

class P4MapMaker;
typedef std::shared_ptr<P4MapMaker> P4MapMaker_t;

class P4MapMaker
{
    public:
	P4MapMaker();
	P4MapMaker( const P4MapMaker &m );

	~P4MapMaker();

	static void doBindings( sol::state* lua, sol::table& ns );

	static P4MapMaker_t Join( P4MapMaker l, P4MapMaker r );

	void		Insert( std::string m );
	void		Insert( std::string l, std::string r );

	void		Reverse();
	void		Clear();
	int		Count();
	bool		IsEmpty() { return Count() == 0; }
	sol::object	Translate( std::string p, int fwd, sol::this_state s );
	bool		Includes( std::string p );
	sol::table	Lhs( sol::this_state s );
	sol::table	Rhs( sol::this_state s );
	sol::table	ToArray( sol::this_state s );

    private:
	void		SplitMapping( const StrPtr &in, StrBuf &l, StrBuf &r );
	MapApi *	map;
};

}