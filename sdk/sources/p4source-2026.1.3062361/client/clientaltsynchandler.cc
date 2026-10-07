/*
 * Copyright 1995, 2023 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# define NEED_FILE

# include <stdhdrs.h>

# include <debug.h>
# include <strbuf.h>
# include <error.h>
# include <handler.h>
# include <runcmd.h>
# include <strdict.h>
# include <strtable.h>
# include <strtree.h>
# include <strops.h>

# include <filesys.h>
# include <pipeio.h>

# include <rpc.h>

# include <p4tags.h>
# include <msgclient.h>

# include "client.h"
# include "clientuser.h"
# include "clientaltsynchandler.h"

# if defined( HAS_CPP11 ) && !defined( HAS_BROKEN_CPP11 )
# include <string>
# include <vector>
# include <unordered_map>
# include <json.hpp>
using json = nlohmann::json;
# define HAVE_JSON
# endif

# ifdef _DEBUG
# include <errorlog.h>
# endif


# ifdef HAVE_JSON
int
ClientAltSyncHandler::IsSupported()
{
	return 1;
}

ClientAltSyncHandler *
ClientAltSyncHandler::GetAltSyncHandler( Client *client, Error *e )
{

	static StrRef handleName( "altSyncHandle" );
	ClientAltSyncHandler *handle =
	    (ClientAltSyncHandler *)client->handles.Get( &handleName );
	if( handle )
	    return handle;
	
	const StrPtr altSync = client->GetAltSyncTrigger();
	if( altSync == "unset" )
	    return 0;

	handle = new ClientAltSyncHandler( client );
	client->handles.Install( &handleName, handle, e );
	if( e->Test() )
	{
	    delete handle;
	    return 0;
	}
	return handle;
}

static void
JsonObjToDict( json obj, StrDict *dict )
{
	for( auto it : obj.items() )
	{
	    if( it.value().is_string() )
	        dict->SetVar( it.key().c_str(),
	                      it.value().get<std::string>().c_str() );
	    else if( it.value().is_number_integer())
	        dict->SetVar( it.key().c_str(),
	                      StrNum( it.value().get<std::int64_t>() ) );
	    else if( it.value().is_boolean() )
	        dict->SetVar( it.key().c_str(),
	                      it.value().get<bool>() ? P4Tag::v_true
	                                             : P4Tag::v_false );
	}
}

int
ClientAltSyncHandler::AltSync( Error *e, StrDict *results, bool *pass )
{
	if( results )
	    results->Clear();
	if( pass )
	    *pass = false;

	StrPtr *cmd = client->GetVar( P4Tag::v_altSync, e );

	if( !e->Test() && !IsAlive() )
	    Start( e );
	
	if( e->Test() )
	    return -1;

	StrBufDict altSyncDict;
	FillDict( altSyncDict );

	StrBuf input, output;
	json jd;
	StrRef var, val;
	for( int j = 0; altSyncDict.GetVar( j, var, val ); j++ )
	{
	    if( val.IsNumeric() && !( val.Length() > 1 && val[0] == '0' ) )
	        jd[var.Text()] = val.Atoi64();
	    else if( val == P4Tag::v_true )
	        jd[var.Text()] = true;
	    else if( val == P4Tag::v_false )
	        jd[var.Text()] = false;
	    else
	        jd[var.Text()] = val.Text();
	}

	input.Append( jd.dump( -1, ' ', false,
	                       json::error_handler_t::replace ).c_str() );
	input << "\n";

	pipe ? pipe->Write( input, e ) : rc.Write( input, e );

	int res = 1;

	if( pipe ? pipe->ReadLine( output, readBuf, e ) <= 0
	         : rc.ReadLine( output, readBuf, e ) <= 0 )
	{
	    res = End( e );

	    if( e->Test() )
	    {
	        client->GetUi()->Message( e );
	        e->Clear();
	    }
	    return res;
	}

	StrOps::StripNewline( output );

	json ret = json::parse( output.Text(), nullptr, false );
	if( ret.is_discarded() )
	{
	    // Invalid JSON
	    e->Set( MsgClient::AltSyncBadJSON ) << output;
	}
	else if( !ret.contains("result") ||
	         !( ret["result"].is_number_integer() || // 0 or 1
	            ret["result"].is_boolean() ||        // true or false
	            ret["result"].is_string() ) )      //success/error/pass
	    e->Set( MsgClient::AltSyncNoResult ) << output;
	else
	{
	    if( ret["result"].is_string() )
	    {
	        if( ret["result"].get<std::string>() == "error" )
	            res = 1;
	        else if( ret["result"].get<std::string>() == "success")
	            res = 0;
	        else if( ret["result"].get<std::string>() == "pass" )
	        {
	            if( pass )
	                *pass = true;
	            res = 0;
	        }
	        else
	            e->Set( MsgClient::AltSyncBadResult ) << output;
	    }
	    else
	    {
	        res = ret["result"].is_boolean() ?
	             (ret["result"].get<bool>() ? 0 : 1) :
	              ret["result"].get<int>();

	        // If result wasn't textual, handle <altSync>=pass
	        if( pass && ret.contains(cmd->Text()) &&
	            ret[cmd->Text()].is_string() &&
	            ret[cmd->Text()].get<std::string>() == "pass")
	            *pass = true;
	    }

	    // Handle errors
	    if( !e->Test() && res && ret.contains( "error" ) )
	    {
	        std::string error = ret[ "error" ].get<std::string>();
	        StrBuf errStr = error.c_str();
	        e->Set( MsgClient::AltSyncErr ) << errStr;
	    }
	    else if( !e->Test() && results && ret.is_object() )
	    {
	        // Get response
	        JsonObjToDict( ret, results );
	    }
	}
	readBuf = "";

	if( !e->Test() && !res && *cmd == P4Tag::v_check )
	{
	    // Successful setup, cache anything useful
	    StrPtr *root = client->GetVar( P4Tag::v_clientRoot );
	    if( root )
	        clientRoot = *root;
	}

	if( e->Test() )
	{
	    client->GetUi()->Message( e );
	    e->Clear();
	    return res;
	}

	return res;
}

# else // !HAVE_JSON

int
ClientAltSyncHandler::IsSupported()
{
	return 0;
}

ClientAltSyncHandler *
ClientAltSyncHandler::GetAltSyncHandler( Client *client, Error *e )
{
	e->Set( MsgClient::AltSyncNotSupported );
	return 0;
}

int
ClientAltSyncHandler::AltSync( Error *e, StrDict *results, bool *pass )
{
	e->Set( MsgClient::AltSyncNotSupported );
	return 1;
}

# endif

void
ClientAltSyncHandler::Start( Error *e )
{
	if( started )
	    End( e );

	if( e->Test() )
	    return;

	StrBufDict altSyncDict;
	FillDict( altSyncDict );

	// Allow the trigger to know if it's talking through a broker or
	// proxy since the serverAddress will be different in this case.

	altSyncDict.SetVar( "P4PORT", client->GetPort() );
	altSyncDict.SetVar( "P4CLIENT", client->GetClient() );

	const StrPtr altSync = client->GetAltSyncTrigger();

	if( altSync.StartsWith( "pipe:", 5 ) )
	{
	    pipe = new PipeIo( StrRef( altSync.Text() + 5 ) );
	    pipe->Open( e );
	}
	else
	{
	    RunArgs cmd;
	    StrOps::Expand( cmd.SetBuf(), altSync, altSyncDict );
	    rc.Run( cmd, e );
	}

	// If we failed to start, make sure we're in a clean state
	if( e->Test() )
	{
	    e->Set( MsgClient::AltSyncFailStart );

	    Error ignore;
	    if( pipe )
	    {
	        pipe->Close( &ignore );
	        delete pipe;
	        pipe = 0;
	    }
	    else
	        rc.StopChild();
	}

	started = 1;
}

int
ClientAltSyncHandler::IsAlive()
{
	Error e;
	return started ? pipe ? pipe->Peek( &e ) || !e.Test()
	               : !rc.PollChild( 1 )
	               : 0;
}

int
ClientAltSyncHandler::End( Error *e )
{
	if( !started )
	    return 0;

	if( IsAlive() )
	{
	    // Hardcode the shutdown string rather than have to spin up 
	    static StrRef quit( "{\"altSync\":\"quit\"}\n" );
#if defined(_DEBUG) && defined(HAVE_JSON)
	    // In development, check that it's valid
	    json p = json::parse( quit.Text(), nullptr, false );
	    if( p.is_discarded() )
	    {
	        e->Set( MsgClient::AltSyncBadJSON ) << quit;
	        AssertLog.Abort( e );
	    }
#endif
	    pipe ? pipe->Write( quit, e ) : rc.Write( quit, e );
	}

	int res;
	if( pipe )
	{
	    pipe->Close( e );
	    res = e->Test();
	    delete pipe;
	    pipe = 0;
	}
	else
	    res = rc.WaitChild();

	started = 0;
	readBuf = "";
	return res;
}

void
ClientAltSyncHandler::FillDict( StrDict &dict )
{
	StrRef var, val;
	StrPtr *asVars = client->GetVar( P4Tag::v_altSyncVars );
	StrBufTree copyVars;
	if( asVars )
	{
	    StrBuf tmp;
	    char *vars[128];
	    int count = StrOps::Words( tmp, asVars->Text(), vars, 128, ',' );
	    for( int i = 0; i < count; i++ )
	        copyVars.SetVar( vars[i] );
	}

	for( int i = 0; client->GetVar( i, var, val ); i++ )
	{
	    if( asVars && !copyVars.GetVar( var ) )
	        continue;

	    // Belt and braces
	    if( var == P4Tag::v_func ||
	        var == P4Tag::v_func2 ||
	        var == P4Tag::v_state ||
	        var == P4Tag::v_handle ||
	        var == P4Tag::v_decline ||
	        var == P4Tag::v_confirm ||
	        var == P4Tag::v_message ||
	        var == P4Tag::v_message2 )
	        continue;

	    if( var.EndsWith( "Rec", 3 ) || var.EndsWith( "Rec2", 4 ) )
	        continue;

	    dict.SetVar( var, val );
	}

	dict.SetVar( P4Tag::v_port, client->GetPort() );
	dict.SetVar( P4Tag::v_client, client->GetClient() );
	dict.SetVar( P4Tag::v_user, client->GetUser() );
	dict.SetVar( P4Tag::v_server, client->protocolServer );
}
