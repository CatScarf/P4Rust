/*
 * Copyright 1995, 2019 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

namespace P4Lua {

class P4Result
{
    public:
			P4Result();

	// Setting
	void		AddOutput( sol::object v );
	void		AddOutput( std::string v, sol::this_state s );
	void		AddOutput( std::map<std::string, std::string> v,
			           sol::this_state s );
	void		AddOutput( sol::table v, sol::this_state s );
	void		AddMessage( Error *e, sol::this_state s );
	void		AddTrack( const char *msg );
	void		AddTrack( std::string t );
	void 		DeleteTrack();

	// Getting
	sol::table	GetOutput( sol::this_state s );
	sol::table	GetErrors( sol::this_state s );
	sol::table	GetWarnings( sol::this_state s );
	sol::table	GetMessages( sol::this_state s );
	sol::table	GetTrack( sol::this_state s );

	// Get errors/warnings as a formatted string
	void		FmtErrors( std::string buf );
	void		FmtWarnings( std::string buf );

	// Set API level for backwards compatibility
	void		SetApiLevel( int l )	{ apiLevel = l; }

	// Testing
	int		ErrorCount();
	int		WarningCount();

	// Clear previous results
	void		Reset();

    private:
	void		Fmt( const char *label, std::vector<std::string> ary,
			     std::string buf );
	std::string	FmtMessage( Error *e );

	std::vector<sol::object>	output;
	std::vector<std::string>	warnings;
	std::vector<std::string>	errors;
	std::vector<P4Error_t>		messages;
	std::vector<std::string>	track;
	int				apiLevel;
};

}