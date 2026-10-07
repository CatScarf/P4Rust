/*
 * Copyright 1995, 2019 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

namespace P4Lua {

class SpecDataP4Lua : public SpecData
{
    public:
	SpecDataP4Lua( sol::table h, sol::this_state *s )
	{
	    hash = h;
	    luaState = s;
	}

	virtual StrPtr *GetLine( SpecElem *sd, int x, const char **cmt );
	virtual void	SetLine( SpecElem *sd, int x, const StrPtr *val,
			         Error *e );

    private:
	sol::this_state *luaState;
	sol::table	hash;
	StrBuf		last;
};

}
