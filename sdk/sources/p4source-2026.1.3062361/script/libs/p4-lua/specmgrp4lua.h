/*
 * Copyright 1995, 2019 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

class StrBufDict;
class StrDict;
class StrBuf;
class StrPtr;
class Error;

namespace P4Lua {

class SpecMgrP4Lua {
    public:
		SpecMgrP4Lua();
		~SpecMgrP4Lua();
	void	SetDebug( int i )	{ debug = i; 	}

	// Clear the spec cache and revert to internal defaults
	void	Reset();

	// Add a spec to the cache
	void	AddSpecDef( const char *type, std::string specDef );
	void	AddSpecDef( const char *type, const char * specDef );

	// Check that a type of spec is known.
	int	HaveSpecDef( const char *type );

	//
	// Parse routine: converts strings into Ruby P4::Spec objects.
	//
	sol::table	StringToSpec( const char *type, const char *spec, Error *e, sol::this_state );

	//
	// Format routine. updates a StrBuf object with the form; 
	// that can then be converted to a Ruby string where required. 
	//
	void	SpecToString(const char *type, sol::table hash, StrBuf &b, Error *e, sol::this_state luaState);

	//
	// Convert a Perforce StrDict into a Ruby hash. Used when we're 
	// parsing tagged output that is NOT a spec. e.g. output of
	// fstat etc.
	//
	sol::table	StrDictToHash( StrDict *dict, sol::this_state luaState );
	sol::table	StrDictToHash( StrDict *dict, sol::table hash, sol::this_state );

	// 
	// Convert a Perforce StrDict into a P4::Spec object. This is for
	// 2005.2 and later servers where the forms are supplied pre-parsed
	// into a dictionary - we just need to convert them. The specDef
	// argument tells us what type of spec we're converting.
	//
	sol::table	StrDictToSpec( StrDict *dict, StrPtr *specDef, sol::this_state );


	//
	// Return a list of the fields in a given type of spec. Return Qnil
	// if the spec type is not known.
	//
	sol::table	SpecFields( const char *type, Error *e, sol::this_state );

    private:

	void	SplitKey( const std::string key, std::string &base, StrBuf &index );
	void	InsertItem( sol::table hash, const std::string var, const std::string val, sol::this_state );
	sol::table	SpecFields( std::string specDef, sol::this_state  );

    private:
	int		debug;
	std::map<std::string, std::string>	specs;
};

}
