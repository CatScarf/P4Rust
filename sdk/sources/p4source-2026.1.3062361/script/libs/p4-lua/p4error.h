/*
 * Copyright 1995, 2019 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

namespace P4Lua {
	
class P4Error
{
    public:
	// Construct by copying another error object
	P4Error( const Error &other );

	static void doBindings( sol::state* lua, sol::table& ns );
    
	void  SetDebug( int d )			{ debug = d;	}

	int	GetId();
	int	GetGeneric();
	int	GetSeverity();
	std::string	GetText();
	sol::table	GetDict( sol::this_state s );
	std::string	Inspect();

    private:
	Error		error;
	int		debug;
};

typedef std::shared_ptr<P4Error> P4Error_t;

}
