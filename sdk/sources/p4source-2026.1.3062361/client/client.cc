/*
 * Copyright 1995, 1996 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# include <stdhdrs.h>

# include <debug.h>
# include <tunable.h>
# include <strbuf.h>
# include <strdict.h>
# include <strtree.h>
# include <strarray.h>
# include <strtable.h>
# include <error.h>
# include <i18napi.h>
# include <charcvt.h>
# include <transdict.h>
# include <options.h>
# include <handler.h>
# include <runcmd.h>
# include <rpc.h>
# include <ident.h>
# include <enviro.h>
# include <ignore.h>
# include <filesys.h>
# include <signaler.h>
# include <maptable.h>

# include <msgclient.h>
# include <msgserver.h>
# include <msgscript.h>
# include <msgrpc.h>
# include <msgsupp.h>
# include <p4tags.h>
# include <netportparser.h>
# include <dmextension.h>

# include "clientuser.h"
# include "clientusernull.h"
# include "clientservice.h"
# include "clientmerge.h"
# include "clientscript.h"
# include "client.h"

#include "clientaltsynchandler.h"

void clientTrust( Client *, Error * );

Ident p4api_ident = {
	        IdentMagic "P4API" "/" ID_OS "/" ID_REL "/" ID_PATCH,
		         ID_Y "/" ID_M "/" ID_D
};

Client::Client( Enviro *e ) : Rpc( &service )
{
	// Use the builtin ClientMerger until SetMerger is called.

	fromTransDialog = toTransDialog = NULL;
	translated = this;
	transfname = this;
	errors = 0;
	fatals = 0;
	is_unicode = 0;
	unknownUnicode = 1;
	content_charset = 0;
	output_charset = 0;
	transErr.Clear();
	ignore = new Ignore;
	lowerTag = upperTag = 0;
	authenticated = 0;
	ignoreList = 0;
	pubKeyChecked = 0;
	hostprotoset = 0;
	syncTime = 0;
	ownCwd = 1;
	fstatPartial = 0;
	extraVars = 0;

	protocolXfiles = -1;
	protocolNocase = 0;
	protocolSecurity = 0;
	protocolUnicode = 0;
	protocolClientExts = 0;

	if( e )
	{
	    enviro = e;
	    ownEnviro = 0;
	}
	else
	{
	    enviro = new Enviro;
	    ownEnviro = 1;
	}

	/*
	 * If P4CONFIG is set, we'll need to load the environment
	 * from the .p4 file in the directory path.
	 */

	if( ownEnviro )
	    enviro->Config( GetCwd() );

	// Our standard implementation.
	// See msgs/p4tagl.cc for l_client setting.

	service.Dispatcher( clientDispatch );

	service.SetProtocol( P4Tag::v_cmpfile ); // has clientCompareFile #1737
	if( ClientAltSyncHandler::IsSupported() )
	    service.SetProtocol( P4Tag::v_altSync );
	// clientCheckFile returns v_digest when v_sendDigest is set.
	// Always announced -- the server-side path only triggers when the
	// server itself sets v_sendDigest, so API consumers that extend
	// clientCheckFile see no behavior change.
	service.SetProtocol( P4Tag::v_calculateDigest );
	service.SetProtocol( P4Tag::v_client, P4Tag::l_client );
	apiVer = atoi( P4Tag::l_client );
	apiSet = 0;

	buildInfo = p4api_ident.GetIdent();

	finalized = false;
	initialized = false;

	argv = new StrArray;

	exts = new ClientScript( this );
	extsEnabled = false;
	extsDebugHooksEnabled = false;
	ownExts = true;

	gCharSetCvtCache = new CharSetCvtCache;

	sendClientTotal = 0;
	recvClientTotal = 0;
	sendClientBytes = 0;
	recvClientBytes = 0;

	systemIgnoreMap = 0;
	enviroGen = 0;
}

Client::~Client()
{
	CleanupTrans();
	delete gCharSetCvtCache;
	if( ownEnviro )
	    delete enviro;
	delete fstatPartial;
	delete ignore;
	delete extraVars;
	delete argv;
	if( ownExts )
	    delete exts;
	delete systemIgnoreMap;
}

void
Client::Init( Error *e )
{
	finalized = false;

	// Set up address and connect.
	errors = 0;

	// re-add host and port in protocol message
	hostprotoset = 0;

	// unicode setup if possible
	if( unknownUnicode )
	    SetupUnicode( e );

	if( GetEVar( P4Tag::v_ipaddr ) && GetEVar( P4Tag::v_svrname ) )
	    SetProtocolDynamic( P4Tag::v_ipaddr, 
	                        GetEVar( P4Tag::v_ipaddr )->Text() );

# ifdef USE_CDC
	if( p4tunable.Get( P4TUNE_NET_DELTA_TRANSFER_MINSIZE ) )
	    service.SetProtocol( P4Tag::v_chunking );
# endif

	if( !e->Test() )
	    service.SetEndpoint( GetPort().Text(), e );

	if( !e->Test() )
	    Connect( e );

	if( !e->Test() )
	{
	    initialized = true;

	    DoHandshake( e );	// no-op if not ssl

	    if( !e->Test() && ( unknownUnicode ||
	        ( extsEnabled && exts->CanLoad() ) ) )
	    {
		ClientUserNULL cnull( e );
		
		// discover unicode server status

		SetVar( P4Tag::v_prog, GetProg() );

		const bool enabled = extsEnabled;
		const int authed = authenticated;
		// Ensure that output from the first real command on this
		// connection comes before the any pre-command Extension output.
		authenticated = 0;
		extsEnabled = false;

		Run( "discover", &cnull );

		authenticated = authed;
		extsEnabled = enabled;

		// bad command is expected when connecting to
		// servers before 2014.2
		if( e->CheckIds( MsgServer::BadCommand ) )
		{
		    e->Clear();
		    errors = 0;
		}

		// on trust errors, ignore them, and ignore the
		// unicode detection result
		if( e->CheckIds( MsgRpc::HostKeyMismatch ) ||
		    e->CheckIds( MsgRpc::HostKeyUnknown ) ||
		    e->CheckIds( MsgRpc::SslCertBad ) ||
		    e->CheckIds( MsgRpc::SslCertBadChain ) )
		{
		    e->Clear();
		    errors = 0;
		}
		else
		{
		if( !e->Test() )
		{
		    if( extsEnabled && protocolClientExts )
		        exts->LoadScripts( true, e );
		    if( unknownUnicode )
		        LearnUnicode( e );
		}
		}
		if( e->Test() )
		    (void) Final( e );
	    }

	    // don't bother testing for error since caller will handle

	    return;
	}

	e->Set( MsgClient::Connect );
}

void
Client::Run( const char *func, ClientUser *u )
{
	if( finalized )
	{
	    Error e;
	    e.Set( MsgClient::DevErr ) << "Run() after Final()";
	    u->HandleError( &e );
	    SetError();
	    return;
	}

	// Run the command async, and then wait.

	// In a sync operation we mask Control-C. This allows the
	// management of when we terminate the client sync operation
	// to maintain consistency between the Client and Server.
	// If the user issues a Control-C, we set a flag but we do
	// not act upon the Control-C right away.  Multiple levels
	// of masking is ok.  The isIntr could not be used for this
	// as it is used in other places which make that incompatible.
	// A new isMaskedIntr has been created.

	int mask = 0;

	if( func && strcmp( func, "sync" ) == 0 )
	{
	    mask++;
	    signaler.Mask();
	}

	RunTag( func, u );

	if( mask )
	    signaler.Unmask();

	WaitTag();
}

void
Client::RunTag( const char *func, ClientUser *u )
{
	if( finalized || !initialized )
	{
	    static const char* ef = "RunTag() after Final()";
	    static const char* ei = "RunTag() before Init()";
	    Error e;
	    e.Set( MsgClient::DevErr ) << ( finalized ? ef : ei );
	    u->HandleError( &e );
	    SetError();
	    return;
	}

	Error e;

	// for assembla... job067346 - late protocol set
	// we set this late so that an application can SetHost
	// after creating the Client object and init.
	if( !hostprotoset )
	{
	    hostprotoset = 1;
	    if( GetInitRoot().Length() == 0 )
		SetProtocolDynamic( P4Tag::v_host, GetHost() );
	    SetProtocolDynamic( P4Tag::v_port, GetPort() );
	}

	// Warning: WaitTag() calls Dispatch() which may call Invoke().

	// Fill this tag slot
	// Below we garantee it's empty.

	tags[ upperTag ] = u;

	// Cheesy access to RPC vars, a la OutputStat
	// Equally cheesy access to environment

	if( u )
	{
	    u->SetVarList( this );
	    u->SetEnviro( enviro );

	    if( output_charset )
		u->SetOutputCharset( output_charset );
	}

	if( func && *func == 't' && !strcmp( func, "trust" ) )
	{
	    Loopback( &e );

	    clientTrust( this, &e );

	    if( e.Test() && u )
	        u->Message( &e );

	    return;
	}

	if( !pubKeyChecked )
	{
	    CheckKnownHost( &e, GetTrustFile() );
	    if( e.Test() )
	    {
		if( u )
		{
		    e << "p4 trust";
		    u->Message( &e );
		}
		SetError();
		Clear();
		return;
	    }
	    pubKeyChecked = 1;
	}

	SetVar( P4Tag::v_prog, GetProg() );

	// Formulate up function name.
	// Set user/client/os/cwd
	// Invoke client's request.

	StrBuf s;
	s << "user-" << ( func ? func : "help" );

	GetEnv();

# ifdef HAS_EXTENSIONS

	ClientScriptAction rPre = ClientScriptAction::FAIL;
	int nRunPre = 0;

	std::tie( rPre, nRunPre ) =
	    ExtensionsEnabled() ? exts->Run( "preCommand", func, u, false, &e )
	                        : std::tuple< ClientScriptAction, int >
	                          ( ClientScriptAction::PASS, 0 );

	if( e.Test() || rPre == ClientScriptAction::FAIL )
	{
	    SetError();
	    SetFatal();
	    if( u )
	        u->Message( &e );
	    return;
	}

	if( rPre == ClientScriptAction::REPLACE )
	    return;

# endif

	Invoke( s.Text() );

	// Advance upper tag, and ensure the new slot is empty.

	int nextTag = ( upperTag + 1 ) % ClientTags;

	if( nextTag == lowerTag )
	    WaitTag( tags[ lowerTag ] );

	upperTag = nextTag;

	// Until successful dispatch (after init or setting user/passwd) we 
	// process one command at a time, so that they crypto goes through.

	if( !authenticated )
	    WaitTag();

# ifdef HAS_EXTENSIONS

	ClientScriptAction rPost = ClientScriptAction::FAIL;
	int nRunPost = 0;

	std::tie( rPost, nRunPost ) =
	    ExtensionsEnabled() ? exts->Run( "postCommand", func, u, true, &e )
	                        : std::tuple< ClientScriptAction, int >
	                          ( ClientScriptAction::PASS, 0 );

	if( e.Test() || rPost == ClientScriptAction::FAIL )
	{
	    SetError();
	    SetFatal();
	    if( u )
	        u->Message( &e );
	}
# endif
}

void
Client::WaitTag( ClientUser *u )
{
	if( finalized )
	{
	    Error e;
	    e.Set( MsgClient::DevErr ) << "WaitTag() after Final()";
	    u->HandleError( &e );
	    SetError();
	    return;
	}

	// While any RunTag() requests are outstanding, Dispatch().
	// Dispatch() returns exactly once for each Invoke().

	while( lowerTag != upperTag )
	{
	    Dispatch();

	    authenticated = 1;

	    // lowerTag is done; signal that
	    // and bump it.  If that was the
	    // tag we were waiting for, bail.

	    ClientUser *ui = tags[ lowerTag ];

	    if( Dropped() && !IoError()->CheckId( MsgRpc::Break ) )
		ui->Message( IoError() );

	    ui->Finished();

	    lowerTag = ( lowerTag + 1 ) % ClientTags;

	    if( u == ui )
		break;
	}
}

int
Client::Final( Error *e )
{
	finalized = true;
	initialized = false;

	// Dispatch returns when the other end is waiting for another
	// request.  We tell it to go away with ReleaseFinal.
	// Then we disconnect.

	ReleaseFinal();
	Disconnect();
	
	// Propagate any IO errors

	if( !e->Test() )
	    *e = *IoError();

	// Return non-zero if any errors have been detected.
	// Note that the server still sends some errors back on
	// stdout, which we don't detect.

	return e->Test() || GetErrors();
}

void Client::ConditionalInvokeStats( Error* e )
{
	if( !statCallback.Length() )
	    return;

	if( sendClientTotal > 0 )
	     this->SetVar( P4Tag::v_sendFileCount, StrNum( sendClientTotal ) );

	if( recvClientTotal > 0 )
	    this->SetVar( P4Tag::v_recvFileCount, StrNum( recvClientTotal ) );

	if( sendClientBytes > 0 )
	    this->SetVar( P4Tag::v_sendFileBytes, StrNum( sendClientBytes ) );

	if( recvClientBytes > 0 )
	    this->SetVar( P4Tag::v_recvFileBytes, StrNum( recvClientBytes ) );

	// don't invoke if no data to send
	if( sendClientTotal || recvClientTotal ||
	    sendClientBytes || recvClientBytes )
	    this->Invoke( statCallback.Text() );

	sendClientTotal = 0;
	recvClientTotal = 0;
	sendClientBytes = 0;
	recvClientBytes = 0;
}

void
Client::GetEnv()
{
	/*
	 * Load up into RPC variables:
	 *
	 *	client - this client's name
	 *	cwd - the current working directory
	 *	host - this client's host name (for host locking)
	 *	language - preferred language for error messages
	 *	os - the name of the OS (for calculating paths)
	 *	user - the user's name
	 */

	const StrPtr &lang = GetLanguage();
	const StrPtr &initroot = GetInitRoot();

	translated->SetVar( P4Tag::v_client, GetClient() );
	transfname->SetVar( P4Tag::v_cwd, GetCwd() );
	if( transErr.Test() && translated != transfname )
	    translated->SetVar( P4Tag::v_cwd, GetCwd() );
	if( initroot.Length() )
	{
	    transfname->SetVar( P4Tag::v_initroot, initroot );
	    if( transErr.Test() && translated != transfname )
	        translated->SetVar( P4Tag::v_initroot, initroot );
	}
	else
	    SetVar( P4Tag::v_host, GetHost() );
	if( lang.Length() ) translated->SetVar( P4Tag::v_language, lang );
	SetVar( P4Tag::v_os, GetOs() );
	SetVar( P4Tag::v_locale, GetLocale() );
	translated->SetVar( P4Tag::v_user, GetUser() );

	/*
	 *	unicode - set if all messages are in unicode
	 */

	if( is_unicode )
	{
	    SetVar( P4Tag::v_unicode );
	    SetVar( P4Tag::v_charset, content_charset );
	}
	else // no charset was defined
	{
	    int cs = GuessCharset();
	    if( cs != 0 )
	        SetVar( P4Tag::v_charset, cs );
	}

	SetVar( P4Tag::v_utf8bom, p4tunable.Get( P4TUNE_FILESYS_UTF8BOM ) );

	SetVar( P4Tag::v_clientCase, StrBuf::CaseUsage() );

	int i = GetUi()->ProgressIndicator();
	if( i )
	    SetVar( P4Tag::v_progress, i );
}

void
Client::Confirm( const StrPtr *confirm )
{
	// pre 99.1 servers need GetEnv(), too, because they
	// reestablished DmCaller context each time. 

	if( protocolServer < 6 )
	    GetEnv();

	CopyVars();

	Invoke( confirm->Text() );
}

void
Client::NewHandler()
{
	if (translated != this)
	{
		((TransDict *)translated)->Clear();
	}
	if (transfname != this && transfname != translated)
	{
		((TransDict *)transfname)->Clear();
	}
}

const StrBuf*
Client::GetSendArgv( const int i )
{
	return argv->Get( i );
}

int
Client::GetSendArgc()
{
	return argv->Count();
}

void
Client::SetArgv( int ac, char* const *av )
{
	argv->Clear();

	if( translated != this )
	{
	    for( int i = 0; i < ac; i++ )
	    {
	        int translen = 0;
	        const char *transbuf =
	            ((TransDict*)translated)->ToCvt()->FastCvt( av[ i ],
	                strlen( av[ i ] ), &translen );
	        if( transbuf )
	            argv->Put()->Set( transbuf );
	        else
	            argv->Put()->Set( "?" );
	    }

	    translated->SetArgv( ac, av );
	}
	else
	{
	    for( int i = 0; i < ac; i++ )
	        argv->Put()->Set( av[ i ] );

	    Rpc::SetArgv(ac, av);
	}
}

void
Client::EnableExtensions( Error* e )
{
	if( !exts->BuildCheck() )
	{
	    e->Set( MsgScript::ExtScriptNotInBuild );
	    return;
	}

	extsEnabled = true;
}

void
Client::DisableExtensions()
{
	extsEnabled = false;
}

bool
Client::ExtensionsEnabled()
{
	return extsEnabled;
}

void
Client::SetExtension( ClientScript* cs, Error* e, const bool callerOwns )
{
	EnableExtensions( e );

	if( e->Test() )
	    return;

	if( ownExts )
	    delete exts;

	ownExts = !callerOwns;
	exts = cs;
}

void
Client::EnableDebugHooks()
{
	extsDebugHooksEnabled = true;
}

bool
Client::ExtensionsDebugHooksEnabled() const
{
	return extsDebugHooksEnabled;
}

void
Client::VSetVar( const StrPtr &var, const StrPtr &val )
{
	if (translated != this)
		translated->RemoveVar(var.Text());
	// careful about the order here
	Rpc::VSetVar(var, val);
}

void
Client::SetProg( const char *prog )
{
	programName.Set( prog );
}

void
Client::SetProg( const StrPtr *prog )
{
	programName.Set( prog );
}

void
Client::SetVersion( const char *version )
{
	programVersion.Set( version );
	SetVar( P4Tag::v_version, version );
}

void
Client::SetVersion( const StrPtr *version )
{
	programVersion.Set( version );
	SetVar( P4Tag::v_version, version );
}

StrPtr *
Client::GetProtocol( const StrPtr &var )
{
	// NB: result only valid until next GetProtocol call.

	if( var == P4Tag::v_server2 || var == P4Tag::v_server )
	    protocolBuf.Set( protocolServer );
	else if( var == P4Tag::v_nocase && protocolNocase )
	    protocolBuf.Set( protocolNocase );
	else if( var == P4Tag::v_security )
	    protocolBuf.Set( protocolSecurity );
	else if( var == P4Tag::v_unicode )
	    protocolBuf.Set( protocolUnicode );
	else if( var == P4Tag::v_extensionsEnabled )
	    protocolBuf.Set( protocolClientExts );
	else return 0;

	return &protocolBuf;
}

void
Client::SetupUnicode( Error *e )
{
	const char *charsetVal = GetCharset().Text();

	if( *charsetVal )
	    LateUnicodeSetup( charsetVal, e );
}

void
Client::LearnUnicode( Error *e )
{
	const char *value = "none";

	if( protocolUnicode )
	    value = "auto";

	SetCharset( value );

	if( charsetVar.Length() )
	    enviro->Set( charsetVar.Text(), value, e );

	// ignore any errors trying to save the charset
	e->Clear();
	errors = 0;

	LateUnicodeSetup( value, e );
}

void
Client::LateUnicodeSetup( const char *value, Error *e )
{
	unknownUnicode = 0;

	int cs = CharSetApi::Lookup( value );

	if( cs >= 0 )
	    SetTrans( cs );
	else if( e )
	    e->Set( MsgClient::UnknownCharset ) << value;
}

void
Client::SetEnviroFile( const char *c )
{
	enviro->SetEnviroFile( c );
	enviro->Reload();
}

const StrPtr *
Client::GetEnviroFile()
{
	return enviro->GetEnviroFile();
}

void
Client::FstatPartialAppend( StrDict *part )
{
	if( !fstatPartial )
	    fstatPartial = new StrBufTreeIdx;

	StrRef var, val;

	for( int i = 0; part->GetVar( i, var, val ); i++ )
	    fstatPartial->SetVar( var, val );
}

void
Client::FstatPartialClear()
{
	delete fstatPartial;
	fstatPartial = 0;
}

void
Client::SetProtocol( const char *p, const char *v )
{
	if( !apiSet && !strcmp( P4Tag::v_api, p ) )
	{
	    apiVer = atoi( v );
	    apiSet = 1;
	}
	service.SetProtocol( p, v );
}

void
Client::SetProtocolV( const char *arg )
{
	const char *p;
	if( !apiSet &&
	    ( p = strchr( arg, '=' ) ) &&
	    !strncmp( P4Tag::v_api, arg, p - arg ) )
	{
	    apiVer = atoi( p + 1 );
	    apiSet = 1;
	}
	service.SetProtocolV( arg );
}

StrPtr *
Client::GetEVar( const char *k )
{
	StrRef key( k );
	return GetEVar( &key );
}

StrPtr *
Client::GetEVar( const StrPtr *k )
{
	if( !extraVars )
	    return NULL;

	return extraVars->GetVar( *k );
}

void
Client::SetEVar( const StrPtr *k, const StrPtr *v )
{
	if( !extraVars )
	    extraVars = new StrBufDict;

	extraVars->ReplaceVar( *k, *v );
}

Error *
Client::GetTransError()
{
	return &transErr;
}
