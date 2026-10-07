/*
 * Copyright 1995, 2019 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# include <stdhdrs.h>

# ifdef HAS_EXTENSIONS

# include <vector>
# include <string>
# include <map>

# include <clientapi.h>

# include <p4script.h>
# include <p4script53.h>

# include "p4error.h"
# include "p4result.h"

namespace P4Lua {
	
P4Result::P4Result()
{
	apiLevel = atoi( P4Tag::l_client );
}

void
P4Result::Reset()
{
	output.clear();
	warnings.clear();
	errors.clear();
	messages.clear();
	track.clear();
}

//
// Direct output - not via a message of any kind. For example,
// binary output.
//
void
P4Result::AddOutput( sol::object v )
{
	output.push_back( v );
}

void
P4Result::AddOutput( std::string v, sol::this_state s )
{
	output.push_back( sol::object( s, sol::in_place, v ) );
}

void
P4Result::AddOutput( std::map<std::string, std::string> v, sol::this_state s )
{
	output.push_back( sol::object( s, sol::in_place, sol::as_table( v ) ) );
}

void
P4Result::AddOutput( sol::table v, sol::this_state s )
{
	output.push_back( sol::object( s, sol::in_place, v ) );
}

/*
 * Main distribution of output to the user. This method sorts the
 * output into groups: output, warnings, errors, and sends all output
 * (regardless of severity) to the messages array.
 */
void
P4Result::AddMessage( Error *e, sol::this_state s )
{
	int sev;
	sev = e->GetSeverity();

	// 
	// Empty and informational messages are pushed out as output as nothing
	// worthy of error handling has occurred. Warnings go into the warnings
	// list and the rest are lumped together as errors.
	//

	if ( sev == E_EMPTY || sev == E_INFO )
	    AddOutput( FmtMessage( e ), s );
	else if ( sev == E_WARN )
	    warnings.push_back( FmtMessage( e ) );
	else
	    errors.push_back( FmtMessage( e ) );

	//
	// Whatever severity, format the message into the messages array, wrapped
	// up in a P4::Message object.
	//
	messages.push_back( std::shared_ptr<P4Error>( new P4Error( *e ) ) );
}

sol::table
P4Result::GetOutput( sol::this_state s )
{
	sol::table ret( s, sol::create );
	for( int i = 0; i < output.size(); i++ )
	    ret.add( output[i] );
	return ret;
}

sol::table
P4Result::GetErrors( sol::this_state s )
{
	sol::table ret( s, sol::create );
	for( int i = 0; i < errors.size(); i++ )
	    ret.add( errors[i] );
	return ret;
}

sol::table
P4Result::GetWarnings( sol::this_state s )
{
	sol::table ret( s, sol::create );
	for( int i = 0; i < warnings.size(); i++ )
	    ret.add( warnings[i] );
	return ret;
}

sol::table
P4Result::GetTrack( sol::this_state s )
{
	sol::table ret( s, sol::create );
	for( int i = 0; i < track.size(); i++ )
	    ret.add( track[i] );
	return ret;
}

sol::table
P4Result::GetMessages( sol::this_state s )
{
	sol::table ret( s, sol::create );
	for( int i = 0; i < messages.size(); i++ )
	    ret.add( messages[i] );
	return ret;
}

void
P4Result::AddTrack( const char *msg )
{
	track.push_back( msg );
}

void
P4Result::DeleteTrack()
{
	track.clear();
}

void
P4Result::AddTrack( std::string t )
{
	track.push_back( t );
}

int
P4Result::ErrorCount()
{
	return errors.size();
}

int
P4Result::WarningCount()
{
	return warnings.size();
}

void
P4Result::FmtErrors( std::string buf )
{
	Fmt( "[Error]: ", errors, buf );
}

void
P4Result::FmtWarnings( std::string buf )
{
	Fmt( "[Warning]: ", warnings, buf );
}

void
P4Result::Fmt( const char *label, std::vector<std::string> ary, std::string buf )
{
	buf.clear();
	// If the array is empty, then we just return
	if( ! ary.size() ) return;

	buf.append( ary.at( 0 ) );
	for( int i = 1; i < ary.size(); i++ )
	{
	    buf.append( "\n\t" );
	    buf.append( ary.at( i ) );
	}
}

std::string
P4Result::FmtMessage( Error *e )
{
	StrBuf t;
	e->Fmt( t, EF_PLAIN );
	return  t.Text();
}

}

# endif
