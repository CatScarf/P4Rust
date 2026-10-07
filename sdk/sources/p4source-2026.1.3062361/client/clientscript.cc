/*
 * Copyright 1995, 2019 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# include <stdhdrs.h>

# ifdef HAS_EXTENSIONS

# include <clientapi.h>
# include <pathsys.h>
# include <strarray.h>
# include <rpc.h>
# include <client.h>

# include <deque>
# include <string>
# include <vector>
# include <regex>
# include <functional>

# include <dmextension.h>
# include <dmextension_c.h>
# include <dmextensiondata.h>
# include <dmextensiondata_c.h>
# include <clientscript.h>
# include <enviro.h>
# include <msgscript.h>

# include <map>
# include <p4_any.h>
# include <tuple>
# include <string>
# include <vector>
# include <p4_optional.h>
# include <functional>
# include <unordered_map>
# include <unordered_set>

# include <debug.h>

ClientScript::ClientScript( Client* c )
{
	if( const auto pe = c->GetEnviro()->Get( "P4EXTENSIONS" ) )
	    SetSearchPattern( pe );

	client = c;
}

ClientScript::~ClientScript()
{
}

bool ClientScript::CanLoad() const
{
	return !patterns.empty();
}

bool ClientScript::BuildCheck() const
{
	return true;
}

void ClientScript::SetClient( Client* c )
{
	client = c;
}

std::vector< std::unique_ptr< Extension > >& ClientScript::GetExts()
{
	return exts;
}

SCR_VERSION ClientScript::scrVerFromFileName( const char* file )
{
	// Script file names:
	//
	//   script.p4-extension  # A full Extension
	//   script.53.lua        # A loose script, $name.$scrVersion.$scrType
	//

	// First get the start of the string ".NN.lua"
	// (multiple dots might be in the file name)
	const char *ptr = nullptr;
	int countdots = 0;
	for( int i = strlen( file ) - 1; i >= 0; i-- )
	{
	    if( file[i] == '.' )
	        countdots++;
	    if( countdots == 2 )
	    {
	        ptr = &file[i];
	        break;
	    }
	}

	if( !ptr )
	    return P4SCRIPT_UNKNOWN;

	const std::string f( ptr );
	static std::regex rx( ".*?\\.(\\S+)\\.(\\S+)$",
	    std::regex::icase|std::regex::optimize|std::regex::ECMAScript );
	std::smatch ms;

	if( std::regex_match( f, ms, rx ) && ms.size() == 3 )
	    if( ms.str( 2 ) == "lua" && ms.str( 1 ) == "53" )
	        return P4SCRIPT_LUA_53;   

	return P4SCRIPT_UNKNOWN;
}

std::tuple< ClientScriptAction, int >
ClientScript::Run( const char* cmd, const char* func, ClientUser* u,
	           const bool noReplace, Error *e )

{
	// Record how many Extensions were run so callers have some context
	// they can use to do things like fallback to triggers, etc.
	int nRun = 0;

	for( const auto& ext : exts )
	{
	auto ecd = (ExtensionCallerDataC*)ext->GetECD();
	try {
	    // Ignore missing callbacks since they're not all mandatory.
	    if( !ext->fnExists( cmd ) )
	        continue;

	    nRun++;

	    ecd->ui = u;
	    // Extensions run from within a callback in the middle of a command
	    // don't set the function name so they don't clobber the context
	    // of where they were called from.
	    if( func )
	        ecd->func = func;
	    auto r = ext->RunCallBack( cmd, e );
	    ecd->ui = nullptr;

	    if( e->Test() || !r.has_value() )
	    {
	        StrBuf msg;
	        if( e->Test() )
	            e->Fmt( &msg );
	        else
	            msg << "no return from callback";
	        e->Set( MsgScript::ExtClientRuntimeFail )
	            << ecd->sourcePath.c_str() << msg;
	        return std::tuple< ClientScriptAction, int >
	                   ( ClientScriptAction::FAIL, nRun );
	    }

	    const ClientScriptAction ra =
	        static_cast< ClientScriptAction >( p4_std_any::any_cast< int >( r ) );

	    if( ra == ClientScriptAction::PASS )
	        continue;

	    if( ra == ClientScriptAction::FAIL )
	        e->Set( MsgScript::ExtClientCmdRejected )
	            << cmd << ecd->sourcePath.c_str();

	    if( noReplace && ra == ClientScriptAction::REPLACE )
	    {
	        StrBuf msg;
	        msg << "Client Extension can't return REPLACE for the " <<
	               cmd << " hook";
	        e->Set( MsgScript::ExtCodingErr ) << ecd->sourcePath.c_str()
	                                          << "0" << msg;
	        return std::tuple< ClientScriptAction, int >
	                   ( ClientScriptAction::FAIL, nRun );
	    }

	    if( !client->ExtensionsDebugHooksEnabled() &&
	        ra >= ClientScriptAction::PRE_DEBUG )
	        return std::tuple< ClientScriptAction, int >
	                   ( ClientScriptAction::UNKNOWN, nRun );

	    return std::tuple< ClientScriptAction, int >( ra, nRun );
	    }
	    catch( const std::exception& err )
	    {
	        ecd->ui = nullptr;
	        StrBuf msg;
	        msg << "Callback returned bad data: cb: '" << cmd
	            << "', err: " << err.what();
	        e->Set( MsgScript::ExtCodingErr ) << ecd->sourcePath.c_str()
	                                          << "0" << msg;
	        return std::tuple< ClientScriptAction, int >
	               ( ClientScriptAction::FAIL, nRun );
	    }
	}

	return std::tuple< ClientScriptAction, int >
	       ( ClientScriptAction::PASS, nRun );
}

std::vector< std::tuple< std::string, SCR_VERSION > >
ClientScript::FindLooseExts( const StrPtr &start, const bool search, Error* e )
{
	std::vector< std::tuple< std::string, SCR_VERSION > > list;

	if( patterns.empty() )
	    return list;

	auto p = PathSys::CreateUPtr();
	auto q = PathSys::CreateUPtr();
	auto f = FileSys::CreateUPtr( FileSysType( FST_TEXT|FST_L_CRLF ) );

# if defined(OS_NT)
	const auto charset = client->GetEnviro()->GetCharSet();

	if( charset )
	{
	    p->SetCharSet( charset );
	    q->SetCharSet( charset );
	    f->SetCharSetPriv( charset );
	}
# endif

	auto findExtslnPath = [&]( const char *match, const bool recurse )
	{
	    do {

	        e->Clear();
	        auto cdfs = FileSys::CreateUPtr( FST_DIRECTORY );
	        cdfs->Set( p->Text() );
	        std::unique_ptr< StrArray, std::function< void( StrArray* ) > >
	            dcs( cdfs->ScanDir( e ), [&]( StrArray* ptr ){ delete ptr; } );

	        if( !dcs )
	            break;

	        dcs->Sort( StrPtr::CaseFolding() );

	        for( int i = 0; i < dcs->Count() ; i++ )
	        {
	            const StrBuf* a = dcs->Get( i );

	            if( a->Contains( StrRef( match ) ) != a->Text() )
	                continue;

	            const StrPtr ap = *a;
	            q->SetLocal( *p, ap );
	            f->Set( *q );
	            f->Open( FOM_READ, e );

	            if( e->Test() )
	                continue;
/*
	            const std::string extSuffix( ".p4-extension" );
	            const std::string_view file( (*f)->Name() );
	            const auto isExt = file.rfind( extSuffix );

	            if( isExt != std::string::npos && isExt + extSuffix.length() == file.length() )
	                continue;
*/

	            const auto sv = scrVerFromFileName( f->Name() );

	            f->Close( e );

	            if( e->Test() )
	                return;

	            if( sv == P4SCRIPT_UNKNOWN )
	                continue;

	            list.push_back( std::tuple< std::string,
	                SCR_VERSION >( std::string( f->Name() ), sv ) );
	        }

	        if( !recurse )
	            break;
	    }
	    while( p->ToParent() );
	};

	for( const auto & vp : patterns )
	{
	    StrRef vpattern( vp.c_str() );
	    const bool rp = FileSys::IsRelative( vpattern );
	    const bool recurse = rp && search;

	    if( rp )
	        p->Set( start );
	    else
	    {
	        p->Set( vpattern );
	        p->ToParent();
	    }

	    // Skip any invalid path (those finishing with /)
	    if( vpattern.EndsWith( "/", 1 ) || vpattern.EndsWith( "\\", 1 ) )
	        continue;

	    const char *match = rp ? vpattern.Text()
	                       : 1 + strlen( p->Text() ) + vpattern.Text();

	    findExtslnPath( match, recurse );
	}

	e->Clear();

	return list;
}

void ClientScript::SetSearchPath( const char* where )
{
	path = where;
}

void ClientScript::SetSearchPattern( const char *what )
{
	std::string extpattern( what );
	std::size_t posnext = 0;
	std::size_t posstart = 0;

	while( posstart < extpattern.length() )
	{
	    posnext = extpattern.find( ',', posstart );
	    if( posnext != std::string::npos )
	    {
	        // First get rid of leading spaces
	        while( extpattern.data()[posstart] == ' ' && ( posstart < posnext ) )
	            posstart++;

	        if( posstart < posnext )
	        {
	            // Get rid of trailing spaces
	            std::size_t posend = posnext - 1;
	            while( extpattern.data()[posend] == ' ' && ( posstart < posend ) )
	                posend--;
	            if( posstart <= posend )
	                patterns.push_back( extpattern.substr( posstart, posend - posstart + 1) );
	        }

	        posstart = posnext + 1;
	    }
	    else
	    {
	        if( posstart > 0 )
	        {
	            // First get rid of leading spaces
	            while( extpattern.data()[posstart] == ' ' && ( posstart < extpattern.length() ) )
	                posstart++;
	        }
	        patterns.push_back( extpattern.substr( posstart ) );
	        posstart = extpattern.length();
	    }
	}
}

void ClientScript::LoadScripts( const bool search, Error* e )
{
	const StrPtr& sp = path.Length() ? path : client->GetCwd();
	auto list = FindLooseExts( sp, search, e );

	if( e->Test() )
	    return;

	for( const auto& le : list )
	{
	   const std::string& name = std::get< 0 >( le );
	   const SCR_VERSION    sv = std::get< 1 >( le );

	   switch( sv )
	   {
	   case P4SCRIPT_LUA_53:
	   {
	       std::unique_ptr< ExtensionCallerData >
	           ecdC( new ExtensionCallerDataC );
	       auto ecd = (ExtensionCallerDataC*)ecdC.get();

	       ecd->sourcePath = name;
	       ecd->client = client;

	       auto ext =
	           std::make_unique< ExtensionClient >( P4SCRIPT_LUA_53, 20191,
	                                                std::move( ecdC ), e );

	       if( e->Test() )
	           return;

	       std::string c_port = client->GetPort().Text();
	       std::string c_language = client->GetLanguage().Text();
	       std::string c_host = client->GetHost().Text();
	       std::string c_user = client->GetUser().Text();
	       std::string c_version = client->GetVersion().Text();
	       std::string c_charset = client->GetCharset().Text();
	       std::string c_client = client->GetClient().Text();

	       auto fn = [=]( ClientApi& ca )
	       {
	            ca.SetPort( c_port.c_str() );
	            ca.SetLanguage( c_language.c_str() );
	            ca.SetHost( c_host.c_str() );
	            ca.SetUser( c_user.c_str() );
	            ca.SetVersion( c_version.c_str() );
	            ca.SetCharset( c_charset.c_str() );
	            ca.SetClient( c_client.c_str() );
	       };

	       ext->ConfigBinding( P4SCRIPT_CLIENTAPI,
	                p4_std_any::p4_any
	                ( static_cast< std::function< void( ClientApi& ) > >( fn ) ), e );

	       // Prevent recursive Extension activation.
	       ext->DisableExtensionBinding();

	       ext->LoadFile( name.c_str(), e );

	       if( e->Test() )
	           return;

	       exts.emplace_back( std::move( ext ) );
	       break;
	   }

	   default:
	       StrBuf msg;
	       msg << "Found unknown script type '" << name.c_str() << "'";
	       e->Set( MsgScript::DevErr ) << msg;
	  }
	}
}

# else
# endif
