/*
 * Copyright 1995, 2019 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# include <stdhdrs.h>

# ifdef HAS_EXTENSIONS

# include <string>
# include <sstream>
# include <vector>
# include <map>
# include <deque>

# include <clientapi.h>
# include <clientprog.h>
# include <strarray.h>
# include <spec.h>
# include <diff.h>
# include <msgclient.h>
# include <p4script.h>
# include <p4script53.h>

# include "p4luadebug.h"
# include "p4error.h"
# include "p4result.h"
# include "clientprogressp4lua.h"
# include "clientuserp4lua.h"
# include "specmgrp4lua.h"
# include "p4lua.h"

namespace P4Lua {

int
ClientTransferP4Lua::Transfer( ClientApi *client,
	                    ClientUser *ui,
	                    const char *cmd,
	                    StrArray &args,
	                    StrDict &pVars,
	                    int threads,
	                    Error *e )
{
	ClientApi child;
	StrRef var, val;

	for( int j = 0; pVars.GetVar( j++, var, val ); )
	    child.SetProtocol( var.Text(), val.Text() );

	child.SetProtocol( P4Tag::v_api, "99999" );
	child.SetProtocol( P4Tag::v_enableStreams, "" );
	child.SetProtocol( P4Tag::v_enableGraph, "" );
	child.SetProtocol( P4Tag::v_expandAndmaps, "" );

	child.SetPort( &client->GetPort() );
	child.SetUser( &client->GetUser() );
	child.SetClient( &client->GetClient() );

	if( client->GetPassword().Length() )
	    child.SetPassword( &client->GetPassword() );

	child.SetProtocolV( "tag" );
	child.SetProg( client->GetProg().Text() );

	child.Init( e );
	child.SetVersion( client->GetVersion().Text() );
	child.SetBreak( (ClientUserP4Lua *)ui );

	if( e->Test() )
	{
	    ui->HandleError( e );
	    return 1;
	}

	char** a = new char*[ args.Count() ];

	for( int j = 0; j < args.Count(); j++ )
	    a[ j ] = args.Get( j )->Text();

	child.SetArgv( args.Count(), a );
	child.Run( cmd, ui );

	delete[] a;
	child.Final( e );
	
	if( e->Test() )
	{
	    ui->HandleError( e );
	    return 1;
	}

	// Errors like MsgClient::ClobberFile are only detected like this.
	if( child.GetErrors() )
	    return 1;

	return 0;
}

ClientUserP4Lua::ClientUserP4Lua( P4Lua *p )
{
	parent = p;
	results = new P4Result();
	transfer = new ClientTransferP4Lua();
	specMgr = p->specMgr;
	luaState = 0;
	handler = 0;
	cProgress = 0;
	progress = 0;
	cOutputHandler = 0;
	mergeData = 0;
	mergeResult = 0;
	debug = 0;
	alive = 0;
	track = 0;
	luaState = 0;
}

ClientUserP4Lua::~ClientUserP4Lua()
{
	delete results;
}

void
ClientUserP4Lua::Reset()
{
	results->Reset();

	// Leave input alone.

	alive = 1;
}

void
ClientUserP4Lua::SetApiLevel( int l )
{
	apiLevel = l;
	results->SetApiLevel( l );
}

void
ClientUserP4Lua::Finished()
{
	input.clear();
}


/*
 * Handling of output
 */

void
ClientUserP4Lua::ProcessOutput( const char * method, std::string data )
{
	if( handler ) {
	    if( CallOutputMethod( method, data ) )
		results->AddOutput( data, *luaState );
	}
	else
	    results->AddOutput( data, *luaState );
}

void
ClientUserP4Lua::ProcessOutput( const char * method, sol::table data )
{
	if( handler ) {
	    if( CallOutputMethod( method, data ) )
		results->AddOutput( data, *luaState );
	}
	else
	    results->AddOutput( data, *luaState );
}

void
ClientUserP4Lua::ProcessMessage( Error * e )
{
	if( handler ) {
	    int s = e->GetSeverity();

	    if( s == E_EMPTY || s == E_INFO ) {
		// info messages should be send to outputInfo
		// not outputError, or untagged output looks
		// very strange indeed

		StrBuf m;
		e->Fmt( &m, EF_PLAIN );
		std::string s = m.Text();

		if( CallOutputMethod( "outputInfo", s ) )
		    results->AddOutput( s, *luaState );
	    }
	    else {	    
		StrBuf m;
		e->Fmt( &m, EF_PLAIN );
		std::string s = m.Text();

		if( CallOutputMethod( "outputMessage", s ) )
		    results->AddMessage( e, *luaState );
	    }
	}
	else
	    results->AddMessage( e, *luaState );
}

bool
ClientUserP4Lua::CallOutputMethod(const char * method, std::string data)
{
	int answer = 0;

	if (P4LUA_COMMANDS) fprintf(stderr, "[P4] CallOutputMethod\n");

	// some wild hacks to satisfy the rb_protect method
	/*
	VALUE args[3] = { handler, (VALUE) rb_intern(method), data };

	VALUE result = rb_protect(CallMethod, (VALUE) args, &excepted);

	if (excepted) { // exception thrown
		alive = 0;
	} else {
		int a = NUM2INT(result);

		if (P4CPP_COMMANDS)
			fprintf(stderr, "[P4] CallOutputMethod returned %d\n", a);

		if (a & CANCEL) {
			if (P4CPP_COMMANDS)
				fprintf(stderr, "[P4] CallOutputMethod cancelled\n");
			alive = 0;
		}
		answer = a & HANDLED;
	}*/

	return (answer == 0);
}

bool
ClientUserP4Lua::CallOutputMethod(const char * method, sol::table data)
{
	int answer = 0;

	if (P4LUA_COMMANDS) fprintf(stderr, "[P4] CallOutputMethod\n");

	// some wild hacks to satisfy the rb_protect method
	/*
	VALUE args[3] = { handler, (VALUE) rb_intern(method), data };

	VALUE result = rb_protect(CallMethod, (VALUE) args, &excepted);

	if (excepted) { // exception thrown
		alive = 0;
	} else {
		int a = NUM2INT(result);

		if (P4CPP_COMMANDS)
			fprintf(stderr, "[P4] CallOutputMethod returned %d\n", a);

		if (a & CANCEL) {
			if (P4CPP_COMMANDS)
				fprintf(stderr, "[P4] CallOutputMethod cancelled\n");
			alive = 0;
		}
		answer = a & HANDLED;
	}*/

	return (answer == 0);
}


/*
 * Very little should use this. Most output arrives via
 * Message() these days, but -Ztrack output, and a few older
 * anachronisms might take this route.
 */
void
ClientUserP4Lua::OutputText( const char *data, int length )
{
	std::string data2(data, length);
	if( track && length > 4 && data[0] == '-' && data[1] == '-'
	    && data[2] == '-' && data[3] == ' ' ) {
	    int p = 4;
	    for( int i = 4; i < length; ++i ) {
		if( data[i] == '\n' ) {
		    if( i > p ) {
			std::string s(data + p, i - p);
			results->AddTrack( s );
			p = i + 5;
		    }
		    else {
			// this was not track data after all,
			// try to rollback the damage done
			ProcessOutput( "outputText",
			    std::string( data, length ) );
			results->DeleteTrack();
			return;
		    }
		}
	    }
	}
	else
	    ProcessOutput( "outputText", std::string( data, length ) );
}

void
ClientUserP4Lua::Message( Error *e )
{
	ProcessMessage( e );
}

void
ClientUserP4Lua::HandleError( Error *e )
{
	ProcessMessage( e );
	if( e->CheckIds( MsgClient::Fatal ) )
	{
	    parent->client->Final( e );
	    parent->ClearConnected();
	}
}

void
ClientUserP4Lua::OutputBinary( const char *data, int length )
{
	//
	// Binary is just stored in a string. Since the char * version of
	// P4Result::AddOutput() assumes it can strlen() to find the length,
	// we'll make the String object here.
	//
	ProcessOutput( "outputBinary", std::string( data, length ) );
}

void
ClientUserP4Lua::OutputStat( StrDict *values )
{
	StrPtr * spec = values->GetVar( "specdef" );
	StrPtr * data = values->GetVar( "data" );
	StrPtr * sf = values->GetVar( "specFormatted" );
	StrDict * dict = values;
	SpecDataTable specData;
	Error e;

	//
	// Determine whether or not the data we've got contains a spec in one form
	// or another. 2000.1 -> 2005.1 servers supplied the form in a data variable
	// and we use the spec variable to parse the form. 2005.2 and later servers
	// supply the spec ready-parsed but set the 'specFormatted' variable to tell
	// the client what's going on. Either way, we need the specdef variable set
	// to enable spec parsing.
	//
	int isspec = spec && ( sf || data );

	//
	// Save the spec definition for later
	//
	if( spec ) specMgr->AddSpecDef( cmd.Text(), spec->Text() );

	//
	// Parse any form supplied in the 'data' variable and convert it into a
	// dictionary.
	//
	if( spec && data ) {
	    // 2000.1 -> 2005.1 server's handle tagged form output by supplying the form
	    // as text in the 'data' variable. We need to convert it to a dictionary
	    // using the supplied spec.

	    // Parse the form. Use the ParseNoValid() interface to prevent
	    // errors caused by the use of invalid defaults for select items in
	    // jobspecs.

	    Spec s( spec->Text(), "", &e );
	    if( !e.Test() )
		s.ParseNoValid( data->Text(), &specData, &e );
	    if( e.Test() ) {
		HandleError( &e );
		return;
	    }
	    dict = specData.Dict();
	}

	//
	// If what we've got is a parsed form, then we'll convert it to a P4::Spec
	// object. Otherwise it's a plain hash.
	//

	if( isspec )
	    ProcessOutput( "outputStat", specMgr->StrDictToSpec( dict, spec, *luaState ) );
	else
	    ProcessOutput( "outputStat", specMgr->StrDictToHash( dict, *luaState ) );
}

/*
 * Diff support for Lua API. Since the Diff class only writes its output
 * to files, we run the requested diff putting the output into a temporary
 * file. Then we read the file in and add its contents line by line to the
 * results->
 */

void
ClientUserP4Lua::Diff(
	FileSys *f1,
	FileSys *f2,
	int doPage,
	char *diffFlags,
	Error *e )
{

	//
	// Duck binary files. Much the same as ClientUser::Diff, we just
	// put the output into Lua space rather than stdout.
	//
	if( !f1->IsTextual() || !f2->IsTextual() ) {
	    if( f1->Compare( f2, e ) )
		results->AddOutput( "(... files differ ...)", *luaState );
	    return;
	}

	// Time to diff the two text files. Need to ensure that the
	// files are in binary mode, so we have to create new FileSys
	// objects to do this.

	FileSys *f1_bin = FileSys::Create( FST_BINARY );
	FileSys *f2_bin = FileSys::Create( FST_BINARY );
	FileSys *t = FileSys::CreateGlobalTemp( f1->GetType() );

	f1_bin->Set( f1->Name() );
	f2_bin->Set( f2->Name() );

	{
	    //
	    // In its own block to make sure that the diff object is deleted
	    // before we delete the FileSys objects.
	    //
#ifndef OS_NEXT
	    ::
#endif
	    Diff d;

	    d.SetInput( f1_bin, f2_bin, diffFlags, e );
	    if( !e->Test() ) d.SetOutput( t->Name(), e );
	    if( !e->Test() ) d.DiffWithFlags( diffFlags );
	    d.CloseOutput( e );

	    // OK, now we have the diff output, read it in and add it to
	    // the output.
	    if( !e->Test() ) t->Open( FOM_READ, e );
	    if( !e->Test() ) {
		StrBuf b;
		while( t->ReadLine( &b, e ) )
		    results->AddOutput( std::string( b.Text(), b.Length() ), *luaState );
	    }
	}

	delete t;
	delete f1_bin;
	delete f2_bin;

	if( e->Test() )
	    HandleError( e );
}

/*
 * convert input from the user into a form digestible to Perforce. This
 * involves either (a) converting any supplied hash to a Perforce form, or
 * (b) skipping whatever isn't a string or table.
 */
void
ClientUserP4Lua::InputData( StrBuf *strbuf, Error *e )
{
	if( P4LUA_CALLS )
	    fprintf( stderr, "[P4] InputData(). Using supplied input\n" );

	strbuf->Clear();

	while( input.size() )
	{
	    sol::object i = input[0];
	    input.pop_front();

	    if( i.get_type() == sol::type::table )
	    {
	        StrPtr *specDef = varList->GetVar( "specdef" );
	        specMgr->AddSpecDef( cmd.Text(), specDef->Text() );
	        specMgr->SpecToString( cmd.Text(), i.as<sol::table>(), *strbuf, e, *luaState );
	        return;
	    }

	    if( i.get_type() == sol::type::string )
	    {
	        strbuf->Set( i.as<std::string>().c_str() );
	        return;
	    }
	}

	e->Set( E_FAILED, "No user-input supplied." );
}

/*
 * In a script we don't really want the user to see a prompt, so we
 * (ab)use the SetInput() function to allow the caller to supply the
 * answer before the question is asked.
 */
void
ClientUserP4Lua::Prompt( const StrPtr &msg, StrBuf &rsp, int noEcho, Error *e )
{
	if( P4LUA_CALLS )
	    fprintf( stderr, "[P4] Prompt(): %s\n", msg.Text() );

	InputData( &rsp, e );
}

/*
 * Do a resolve. We implement a resolve by calling a block.
 */
int
ClientUserP4Lua::Resolve( ClientMerge *m, Error *e )
{
	if( P4LUA_CALLS )
	    fprintf( stderr, "[P4] Resolve()\n" );

	return CMS_QUIT;

	/* XXX: Port me!
	//
	// If LuaExcept is non-zero, we should skip any further
	//	resolves
	//
	if( LuaExcept ) return CMS_QUIT;
	//
	// If no block has been passed, default to using the merger's resolve
	//
	if( !rb_block_given_p() ) return m->Resolve( e );

	//
	// First detect what the merger thinks the result ought to be
	//
	StrBuf t;
	MergeStatus autoMerge = m->AutoResolve( CMF_FORCE );

	// Now convert that to a string;
	switch( autoMerge ) {
	case CMS_QUIT:
	    t = "q";
	    break;
	case CMS_SKIP:
	    t = "s";
	    break;
	case CMS_MERGED:
	    t = "am";
	    break;
	case CMS_EDIT:
	    t = "e";
	    break;
	case CMS_YOURS:
	    t = "ay";
	    break;
	case CMS_THEIRS:
	    t = "at";
	    break;
	}

	mergeData = MkMergeInfo( m, t );

	void* r;
	StrBuf reply;

	//
	// Call the block using rb_protect to make sure that if the
	// block raises any exceptions we trap them here. We don't want
	// some random longjmp() trashing the Perforce connection. If an
	// exception is raised, we'll abort the merge.
	//
	r = rb_protect( rb_yield, mergeData, &LuaExcept );

	// Make sure the pointers held by the mergeData object are 
	// invalidated. This makes sure we can't dereference a pointer to
	// something that no longer exists if the mergeData object lives
	// longer than this resolve (e.g. exception in block) and its to_s
	// method gets called
	ID invalidate = rb_intern( "invalidate" );
	rb_funcall( mergeData, invalidate, 0 );

	if( LuaExcept ) return CMS_QUIT;
	reply = StringValuePtr( r );

	if( reply == "ay" )
	    return CMS_YOURS;
	else if( reply == "at" )
	    return CMS_THEIRS;
	else if( reply == "am" )
	    return CMS_MERGED;
	else if( reply == "ae" )
	    return CMS_EDIT;
	else if( reply == "s" )
	    return CMS_SKIP;
	else if( reply == "q" )
	    return CMS_QUIT;
	else {
	    StrBuf msg = "[P4] Invalid 'p4 resolve' response: ";
	    msg << reply;
	    rb_warn( "%s", msg.Text() );
	}

	return CMS_QUIT;
	*/
}

int
ClientUserP4Lua::Resolve( ClientResolveA *m, int preview, Error *e )
{
	if( P4LUA_CALLS )
	    fprintf( stderr, "[P4] Resolve(Action)\n" );

	return CMS_QUIT;

	/* XXX: port me
	//
	// If LuaExcept is non-zero, we should skip any further
	//	resolves
	//
	if( LuaExcept ) return CMS_QUIT;

	//
	// If no block has been passed, default to using the merger's resolve
	//
	if( !rb_block_given_p() ) return m->Resolve( 0, e );

	StrBuf t;
	MergeStatus autoMerge = m->AutoResolve( CMF_FORCE );

	// Now convert that to a string;
	switch( autoMerge ) {
	case CMS_QUIT:
	    t = "q";
	    break;
	case CMS_SKIP:
	    t = "s";
	    break;
	case CMS_MERGED:
	    t = "am";
	    break;
	case CMS_EDIT:
	    t = "e";
	    break;
	case CMS_YOURS:
	    t = "ay";
	    break;
	case CMS_THEIRS:
	    t = "at";
	    break;
	default:
	    StrBuf msg = "[P4] Unknown automerge result encountered: ";
	    msg << autoMerge;
	    t = "q";
	    break;
	}

	mergeData = MkActionMergeInfo( m, t );

	void* r;
	StrBuf reply;

	//
	// Call the block using rb_protect to make sure that if the
	// block raises any exceptions we trap them here. We don't want
	// some random longjmp() trashing the Perforce connection. If an
	// exception is raised, we'll abort the merge.
	//
	r = rb_protect( rb_yield, mergeData, &LuaExcept );
	if( LuaExcept ) return CMS_QUIT;

	reply = StringValuePtr( r );

	if( reply == "ay" )
	    return CMS_YOURS;
	else if( reply == "at" )
	    return CMS_THEIRS;
	else if( reply == "am" )
	    return CMS_MERGED;
	else if( reply == "ae" )
	    return CMS_EDIT;
	else if( reply == "s" )
	    return CMS_SKIP;
	else if( reply == "q" )
	    return CMS_QUIT;
	else {
	    StrBuf msg = "[P4] Invalid 'p4 resolve' response: ";
	    msg << reply;
	    rb_warn( "%s", msg.Text() );
	}
	return CMS_QUIT;
	*/
}

/*
 * Return the ClientProgress.
 */
ClientProgress *
ClientUserP4Lua::CreateProgress( int type )
{
	if( P4LUA_CALLS )
	    fprintf( stderr, "[P4] CreateProgress()\n" );
	return progress ? new ClientProgressP4Lua( progress, type ) : 0;
}

/*
 * Simple method to check if a progress indicator has been
 * registered to this ClientUser.
 */
int
ClientUserP4Lua::ProgressIndicator()
{
	if( P4LUA_CALLS )
	    fprintf( stderr, "[P4] ProgressIndicator()\n" );
	int result = progress != nullptr;
	return result;
}

/*
 * Accept input from Lua and convert to a StrBuf for Perforce
 * purposes.  We just save what we're given here because we may not
 * have the specdef available to parse it with at this time.
 */
bool
ClientUserP4Lua::SetInput( sol::object i, sol::this_state lState )
{
	if( P4LUA_CALLS )
	    fprintf( stderr, "[P4] SetInput()\n" );

	if( i.get_type() == sol::type::string ) {
	    std::string s = i.as<std::string>();
	    std::stringstream stream( s );
	    std::string l;
	    while( std::getline( stream, l, '\n' ) )
	    {
	        input.push_back( sol::make_object( lState, l ) );
	    }
	    return true;
	}
	input.push_back( i );
	return true;
}

/*
 * Set the Handler object. Double-check that it is either nil or
 * an instance of OutputHandler to avoid future problems
 */

void
ClientUserP4Lua::SetHandler( void* h )
{
	if( P4LUA_CALLS )
	    fprintf( stderr, "[P4] SetHandler()\n" );

	/*
	if( Qnil != h && Qfalse == rb_obj_is_kind_of( h, cOutputHandler ) ) {
	    rb_raise( eP4, "Handler needs to be an instance of P4::OutputHandler" );
	    return Qfalse;
	}

	handler = h;
	alive = 1; // ensure that we don't drop out after the next call

	return Qtrue;
	*/
}

/*
 * Set a ClientProgress for the current ClientUser.
 */
void
ClientUserP4Lua::SetProgress( void* p )
{
	if( P4LUA_CALLS )
	    fprintf( stderr, "[P4] SetProgress()\n" );
	/*
	//	Check that p is a kind_of P4::Progress and if it isn't
	//	raise an error.
	void* res = rb_obj_is_kind_of( p, cProgress );
	if( ( p != Qnil ) && ( res == Qfalse ) ) {
	    rb_raise( eP4, "Progress must be of type P4::Progress" );
	    return Qfalse;
	}

	progress = p;
	alive = 1;
	return Qtrue;
	*/
}

}

# endif
