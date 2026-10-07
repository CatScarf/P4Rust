/*
 * Copyright 1995, 2000 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

/*
 * clientmain() - Perforce client program
 *
 * See below for usage.
 */

# ifdef _DEBUG
# define NEED_SIGNAL
# endif

# include <stdhdrs.h>

# if defined(OS_NT) && (_MSC_VER >= 1900) && defined(USE_WILDARGS)
# include <vcruntime_startup.h>
# endif

# include <debug.h>
# include <tunable.h>
# include "clientapi.h"
# include <strtable.h>
# include <strarray.h>
# include "clientmerge.h"
# include "clientusercolor.h"
# include <strops.h>
# include <splr.h>
# include <error.h>
# include <p4libs.h>
# include <errornum.h>
# include <options.h>
# include <rpc.h>
# include <pathsys.h>

# include "client.h"
# include "clientuserdbg.h"
# include "clientusermsh.h"
# include "clientaliases.h"

# include <ident.h>
# include <ticket.h>
# include <enviro.h>
# include <i18napi.h>
# include <charcvt.h>
# include <charset.h>
# include <diff.h>
# include <diffmerge.h>
# include <hostenv.h>
# include <errorlog.h>
# include <ignore.h>
# include <md5.h>
# include <msgscript.h>
# include <string>
# include <dmextension.h>
# include <dmextension_c.h>
# include <p4script.h>
# include "clientscript.h"

# include <msgclient.h>
# include <msgsupp.h>

# if defined(OS_NT) && (_MSC_VER >= 1900) && defined(USE_WILDARGS)
extern "C" errno_t __cdecl _p4_configure_narrow_argv(_crt_argv_mode const mode);
extern "C" errno_t __cdecl _p4_configure_wide_argv(_crt_argv_mode const mode);
# endif

/* Ident strings */

Ident ident = { 
	IdentMagic "P4" "/" ID_OS "/" ID_REL "/" ID_PATCH,
	 ID_Y "/" ID_M "/" ID_D 
};

/* Usage info */

ErrorId usage = { ErrorOf( 0, 0, E_FAILED, 0, 0 ), "p4 -h for usage." };

static const char long_usage[] =
"Usage:\n"
"\n"
"    p4 [ options ] command [ arg ... ]\n"
"\n"
"    Options:\n"
"	-b batchsize	specify a batchsize to use with -x (default 128)\n"
"	-h | -?		print this message\n"
"	-s		prepend message type to each line of server output\n"
"	-v level	debug modes\n"
"	-V		print client version\n"
"	-x file		read named files as xargs\n"
"	-G		format input/output as marshalled Python objects\n"
# if defined( HAS_CPP11 ) && !defined( HAS_BROKEN_CPP11 )
"	-Mj		format output as line-delimited JSON objects, with\n"
"			non-UTF8 characters replaced with U+FFFD.\n"
# endif
"	-z tag		format output as 'tagged' (like fstat)\n"
"	-I		show progress indicators\n"
"\n"
"	-c client	set client name (default $P4CLIENT)\n"
"	-C charset	set character set (default $P4CHARSET)\n"
"	-d dir		set current directory for relative paths\n"
"	-H host		set host name (default $P4HOST)\n"
"	-L language	set message language (default $P4LANGUAGE)\n"
"	-p port		set server port (default $P4PORT)\n"
"	-P password	set user's password (default $P4PASSWD)\n"
"	-q 		suppress all info messages\n"
"	-r retries	Retry command if network times out\n"
"	-Q charset	set command character set (default $P4COMMANDCHARSET)\n"
"	-u user		set user's username (default $P4USER)\n"
"\n"
"	--script	Run the named P4-Lua script\n"
"\n"
"    The P4 client 'p4' requires a valid P4 server network address 'P4PORT'\n"
"    for most operations, including its help system.\n"
"    Without an explicit P4PORT, the P4 client will use a default P4PORT of\n"
"   'perforce:1666'.  That is to say, the host is named 'perforce' and the port\n"
"    number is '1666'.\n"
"\n"
"    The P4 client accepts configuration via command-line options, P4CONFIG\n"
"    files, environmental variables and on Windows, the registry.\n"
"    Run 'p4 set' to list the client's current settings.\n"
"\n"
"    Run 'p4 help' to ask the P4 server for information on commands.\n"
"\n"
"    For administrators, see the P4 server's help output in 'p4d -h'.\n"
"\n"
"    For further information, visit the documentation at www.perforce.com.\n"
"\n";

static void
BadCharset()
{
	printf( "Character set must be one of:\n"
        "none, auto, utf8, utf8-bom, iso8859-1, shiftjis, eucjp, iso8859-15,\n"
        "iso8859-5, macosroman, winansi, koi8-r, cp949, cp1251,\n"
        "utf16, utf16-nobom, utf16le, utf16le-bom, utf16be,\n"
        "utf16be-bom, utf32, utf32-nobom, utf32le, utf32le-bom, utf32be,\n"
	"cp936, cp950, cp850, cp858, cp1253, iso8859-7, utf32be-bom,\n"
	"cp852, cp1250, or iso8859-2.\n"
        "Check P4CHARSET and your '-C' option.\n" );
}

/* Some forwards */

int clientMain( int argc, char **argv, int &uidebug, Error *e );
int clientMerger( int argc, char **argv, Error *e );
static int clientSet( int argc, char **argv, Options &, Error *e );
static int clientTickets( int argc, char **argv, Options &, Error *e );
static int clientIgnores( int argc, char **argv, Options &, Error *e );
int clientReplicate( int argc, char **argv, Options & );
int clientInit( int argc, char **argv, Options &, int, Error *e );
int clientSignPackage( int argc, char **argv, Options &, Error *e );
int clientInitHelp( int, Error *e );
int clientLegalHelp( Error *e );
int clientTrustHelp( Error *e );
int jtail( int, char **, Options &, Options &, const char * );
extern int commandChaining;

static char argv0[ 1024 ];

/* And now main... */

# if defined(OS_NT) && (_MSC_VER >= 1900) && defined(USE_WILDARGS)
extern "C" int __p4_argc;
extern "C" char** __p4_argv;
# endif

int
main( int argc, char **argv )
{
	int w_argc = 0, n_argc = 0;

# if defined(OS_NT) && (_MSC_VER >= 1900) && defined(USE_WILDARGS)

	// Do argument processing twice in order to catch issues where
	// the win32 *A(NSI) functions do best-fit mapping of characters,
	// potentially causing problems like a single argument appearing as
	// two.  The *W(ide) API does not do best-fit mapping, so it is
	// run first to get the actual number of arguments, then followed
	// by the narrow call that we actually use.  We don't just use the
	// wide version and call it a day in order to preserve backwards
	// compatibility. 

	_p4_configure_wide_argv(_crt_argv_expanded_arguments);
	w_argc = __p4_argc;
	_p4_configure_narrow_argv(_crt_argv_expanded_arguments);
	n_argc = __p4_argc;

# define argc __p4_argc
# define argv __p4_argv
# endif

	/* Error reporting handle */

	Error e;

	AssertLog.SetTag( "Perforce client" );

	P4Libraries::Initialize( P4LIBRARIES_INIT_ALL, &e );
	AssertLog.Abort( &e );

	if( n_argc != w_argc )
	{
	    e.Set( MsgSupp::AmbiguousArgs );
	    AssertLog.Abort( &e );
	}

	strncpy( argv0, argv[0], sizeof( argv0 ) - 1 );

	int uidebug = 0;
	int ret = clientMain( --argc, ++argv, uidebug, &e );

	AssertLog.Report( &e );

	e.Clear();
	P4Libraries::Shutdown( P4LIBRARIES_INIT_ALL, &e );
	AssertLog.Abort( &e );

	if( uidebug )
	    printf( "exit: %d\n", ret );

# if defined(OS_NT) && (_MSC_VER >= 1900) && defined(USE_WILDARGS)
# undef argc
# undef argv
# endif

# ifdef OS_VMS
	exit( ret );
# endif

	return ret;
}

void
setVarsAndArgs( ClientApi &client, int argc, char **argv, Options &opts)
{
	// Set the version used in server log and monitor output
	StrBuf v;
	v << ID_REL "/" << ID_OS << "/" << ID_PATCH;

	StrPtr *s, *version = NULL;

	// Set any special -z var=value's, for debug
	// Use -zfunc=something to override default func

	for( int i = 0; ( s = opts.GetValue( 'z', i ) ); i++ )
	{
	    if( strstr( s->Text(), "prog=" ) == s->Text() )
	        client.SetProg( s->Text() + 5 );
	    if( strstr( s->Text(), "version=" ) == s->Text() )
	        version = s;
	    client.SetVarV( s->Text() );
	}

	client.SetVersion( version ? version->Text() + 8
	                           : v.Text() );
	client.SetArgv( argc - 1, argv + 1 );
}

static int
ErrorIncludesRPCSubsystem( Error *e )
{
	for( int eno = e->GetErrorCount(); eno > 0; eno-- )
	{
	    if( e->GetId( eno - 1 )->Subsystem() == ES_RPC )
	        return 1;
	}
	return 0;
}

static int clientLongOpts[] = { Options::Client,
	                   Options::Batchsize, Options::User,
	                   Options::Host, Options::Charset,
	                   Options::Help, Options::Port, Options::Password,
	                   Options::CmdCharset, Options::Retries,
	                   Options::Quiet, Options::Progress,
	                   Options::MessageType, Options::Directory,
	                   Options::Variable, Options::Xargs, 
	                   Options::Aliases, Options::Field,
	                   Options::Color, Options::Script,
	                   Options::ScriptMaxMem, Options::ScriptMaxTime,
	                   Options::NoScript, Options::ScriptLang,
	                   Options::ScriptLangVersion, Options::ScriptAPIVersion,
	                   Options::ScriptEnableDbg,
#ifdef _DEBUG
	                   Options::DebugBreak,
#endif
	                   0 };

static const char *clientOptFlags =
	"?b:c:C:d:eE:F:GRhH:M:p:P:l:L:qQ:r#sI?u:v:Vx:z:Z:"; 

int
clientMain( int argc, char **argv, int &uidebug, Error *e )
{
	/* Arg processing */

	// Parse up options

	int argcAlias = argc;
	char **argvAlias = argv;

	Options opts;
	clientParseOptions( opts, argc, argv, e );

	if( e->Test() )
	    return 1;

	for( int i = 0; StrPtr *s = opts.GetValue( 'v', i ); i++ )
	    p4debug.SetLevel( s->Text() );

	int result;
	Enviro enviro;
	ClientApi client( &enviro );

	if( clientPrepareEnv( client, opts, enviro ) )
	    return 1;

	if( ClientAliases::ProcessAliases( argcAlias, argvAlias, argc, argv,
	                                   opts, enviro, client, result, e ) )
	    return result;

	return clientRunCommand( argc, argv, opts, 0, enviro, client,
	                         uidebug, e );
}

void
clientParseOptions( Options &opts, int &argc, StrPtr *&argv, Error *e )
{
	opts.ParseLong( argc, argv, clientOptFlags, clientLongOpts,
		OPT_ANY, usage, e );
}

void
clientParseOptions( Options &opts, int &argc, char **&argv, Error *e )
{
	opts.ParseLong( argc, argv, clientOptFlags, clientLongOpts,
		OPT_ANY, usage, e );

	
#ifdef _DEBUG
	if( opts[Options::DebugBreak] )
	{
# ifdef OS_NT
	    __debugbreak();
# else
	    raise( SIGTRAP );
# endif
	}
#endif
}

void
clientSetVariables( ClientApi &client, Options &opts )
{
	StrPtr *s;
	if( ( s = opts[ 'c' ] ) ) client.SetClient( s );
	if( ( s = opts[ 'H' ] ) ) client.SetHost( s );
	if( ( s = opts[ 'L' ] ) ) client.SetLanguage( s );
	if( ( s = opts[ 'd' ] ) ) client.SetCwd( s );
	if( ( s = opts[ 'u' ] ) ) client.SetUser( s );
	if( ( s = opts[ 'p' ] ) ) client.SetPort( s );
}

int
clientPrepareEnv( ClientApi &client, Options &opts, Enviro &enviro )
{
	// Misc -E environment updates

	StrPtr *s;
	StrBufDict envList;
	StrRef var, val;
	for ( int i = 0 ; ( s = opts.GetValue( 'E', i ) ) ; i++ )
	    envList.SetVarV( s->Text() );

	// Apply these updates before and after each Config().
	// Before to make sure we read the right P4CONFIG, and after
	// to make sure that -E vars override P4CONFIG vars.

	// Create client rpc handle.

	for ( int i = 0 ; envList.GetVar( i, var, val ) ; i++ )
	    client.GetEnviro()->Update( var.Text(), val.Text() );

	// Locale setting

	// Early setting of cwd to find right P4CONFIG file...
	if( ( s = opts[ 'd' ] ) )
	    client.SetCwd( s );

	for ( int i = 0 ; envList.GetVar( i, var, val ) ; i++ )
	    client.GetEnviro()->Update( var.Text(), val.Text() );

	for ( int i = 0 ; envList.GetVar( i, var, val ) ; i++ )
	    enviro.Update( var.Text(), val.Text() );

	enviro.Config( client.GetCwd() );

	for ( int i = 0 ; envList.GetVar( i, var, val ) ; i++ )
	    enviro.Update( var.Text(), val.Text() );

	const char *lc;

	if( ( s = opts[ 'l' ] ) || ( s = opts[ 'C' ] ) )
	{
	    lc = s->Text();
	}
	else
	{
	    lc = enviro.Get( "P4CHARSET" );
	}

	if( lc )
	{
	    CharSetCvt::CharSet cs = CharSetCvt::Lookup( lc );

	    if( cs == CharSetApi::CSLOOKUP_ERROR )
	    {
		// bad charset specification
		BadCharset();
		return 1;
	    }
	    else
	    {
		CharSetCvt::CharSet ws = cs;

		if( ( s = opts[ 'Q' ] ) ||
		    ( lc = enviro.Get( "P4COMMANDCHARSET" ) ) )
		{
		    if( s )
			lc = s->Text();
		    cs = CharSetCvt::Lookup( lc, &enviro );
		    if( (int)cs == -1 )
		    {
			printf( "P4COMMANDCHARSET unknown\n" );
			return 1;
		    }
		}
		if( CharSetApi::Granularity( cs ) != 1 )
		{
		    printf( "p4 can not support a wide charset unless\n"
			    "P4COMMANDCHARSET is set to another charset.\n"
			    "Attempting to discover a good charset.\n" );
		    cs = CharSetCvt::Discover( &enviro );
		    if( cs == -1 )
		    {
			printf( "Could not discover a good charset.\n" );
			return 1;
		    }
		}

		client.SetTrans( cs, ws, cs, cs );

		for ( int i = 0 ; envList.GetVar( i, var, val ) ; i++ )
		     client.GetEnviro()->Update( var.Text(), val.Text() );
	    }
	}
	return 0;
}

int
clientRunCommand(
	int argc,
	char **argv,
	Options &opts,
	ClientUser *callerUI,
	Enviro &enviro,
	ClientApi &client,
	int &uidebug,
	Error *e )
{
	StrPtr *s;
	int result;
	int restarted = 0;
	P4DebugConfig debugHelper;


	// Blast out message.

	if( opts[ 'h' ] || opts[ '?' ] )
	{
	    printf( "%s", long_usage );
	    return 0;
	}

	if( opts[ 'V' ] )
	{
	    StrBuf s;
	    ident.GetMessage( &s );
	    printf( "%s", s.Text() );
	    return 0;
	}

	// Debugging 

	if( opts[ 's' ] || opts[ 'e' ] )
	    ++uidebug;

	if( p4debug.GetLevel( DT_TIME ) >= 1 )
	    debugHelper.Install();

	// Always explicitly disable it in unoptimized builds.
#ifdef USE_OPTIMIZED_ZLIB
	if( p4tunable.Get( P4TUNE_ZLIB_DISABLE_OPTIM ) )
#endif
	    P4Libraries::DisableZlibOptimization();

	// If we recognize the command (merge3, set), do it now */

	if( argc && !strcmp( argv[0], "merge3" ) )
	{
	    return clientMerger( argc - 1, argv + 1, e );
	}
	else if( argc && !strcmp( argv[0], "set" ) )
	{
	    return clientSet( argc - 1, argv + 1, opts, e );
	}
	else if( argc && !strcmp( argv[0], "tickets" ) )
	{
	    return clientTickets( argc - 1, argv + 1, opts, e );
	}
	else if( argc && !strcmp( argv[0], "init" ) )
	{
	    return clientInit( argc - 1, argv + 1, opts, 0, e );
	}
	else if( argc && !strcmp( argv[0], "clone" ) )
	{
	    return clientInit( argc - 1, argv + 1, opts, 1, e );
	}
	else if( argc && !strcmp( argv[0], "ignores" ) )
	{
	    return clientIgnores( argc - 1, argv + 1, opts, e );
	}
	else if( argc && !strcmp( argv[0], "replicate" ) )
	{
	    return clientReplicate( argc - 1, argv + 1, opts );
	}
	else if( argc > 1 && !strcmp( argv[0], "help" ) )
	{
	    if( !strcmp( argv[1], "clone" ) )
	        return clientInitHelp( 1, e );
	    else if( !strcmp( argv[1], "init" ) )
	        return clientInitHelp( 0, e );
	    else if( !strcmp( argv[1], "trust" ) )
	        return clientTrustHelp( e );
	    else if( argc == 3 && !strcmp( argv[2], "legal" ) &&
	             ( !strcmp( argv[1], "-l" ) ||
	               !strcmp( argv[1], "--local" ) ) )
	        return clientLegalHelp( e );
	}
# ifdef HAS_CPP11
	else if( argc >= 1 && !strcmp( argv[0], "extension" ) )
	{
	    Error eIgnore;
	    const int longOpts[] = { Options::PkgExtension, 0 };
	    Options opts;
	    int eargc = argc-1;
	    char **eargv = argv;
	    eargv++;

	    opts.ParseLong( eargc, eargv, "", longOpts, OPT_ANY,
	                    MsgClient::Fatal, &eIgnore );

	    if( opts[ Options::PkgExtension ] )
	        return clientSignPackage( argc -1, argv + 1, opts, e );
	}
# endif // HAS_CPP11

	// Get command line overrides of user, client, cwd, port
	
	clientSetVariables( client, opts );
	if( ( s = opts[ 'P' ] ) )
	{
	    client.SetPassword( s );
	    memset( s->Text(), '\0', s->Length() );
	}
	StrBuf argv0B( argv0 );
	client.SetExecutable( &argv0B );

	// Specify a batch size

	int batchSize = opts[ 'b' ] ? opts[ 'b' ]->Atoi() : 128;
	if( batchSize < 1 ) 
	    batchSize = 1;

	// Any protocol settings?

	for( int i = 0; ( s = opts.GetValue( 'Z', i ) ); i++ )
	    client.SetProtocolV( s->Text() );

	// -G or -R implies -Z tag -Z sendspec

	if( opts[ 'G' ] || opts[ 'R' ] || opts[ Options::Field ] )
	{
	    client.SetProtocol( P4Tag::v_tag );
	    client.SetProtocol( P4Tag::v_specstring );
	}

	// -M implies -Z sendspec
	if( opts[ 'M' ] )
	    client.SetProtocol( P4Tag::v_specstring );

	// Set default api value high,  client floats with server output
	// This high value also lets the server know that the client can 
	// handle streams (i.e. don't change this).
	// Ignore above,  unfortunately there are other api's using the
	// magic 99999,  although there is server support to prevent this
	// we shall enable streams here for older servers.
	// The commandline client just prints to console, so the many-to-many
	// relationship of &-maps wont break it.

	client.SetProtocol( P4Tag::v_api, "99999" );
	client.SetProtocol( P4Tag::v_enableStreams );
	client.SetProtocol( P4Tag::v_enableGraph );
	client.SetProtocol( P4Tag::v_expandAndmaps );

	// Set the client program name
	client.SetProg( "p4" );

	if( !opts[ Options::NoScript ] )
	{
	    Error eIgnore;
	    client.EnableExtensions( &eIgnore );
	}

	if( opts[ Options::ScriptEnableDbg ] )
	    client.EnableDebugHooks();

	if( ( s = opts[ Options::Script ] ) )
	{
	    const StrPtr* script = s;
# ifdef HAS_EXTENSIONS
	    Client client1;
	    client1.SetArgv( argc, argv );
	    ClientUser ui;
	    std::unique_ptr< ExtensionCallerData >
	        ecdC( new ExtensionCallerDataC );
	    auto ecd = (ExtensionCallerDataC*)ecdC.get();
	    ecd->sourcePath = script->Text();
	    int apiVersion = 99999;
	    if( ( s = opts[ Options::ScriptAPIVersion ] ) )
	        apiVersion = s->Atoi();
	    ecd->apiVersion = apiVersion;
	    ecd->client = &client1;
	    ecd->ui = &ui;

	    StrBuf lang = "Lua";

	    SCR_VERSION dv = ClientScript::scrVerFromFileName( script->Text() );
	    SCR_VERSION v = dv == P4SCRIPT_UNKNOWN ? P4SCRIPT_LUA_53 : dv;

	    if( ( ( s = opts[ Options::ScriptLang ] ) ) )
	    {
	        if( s->CCompare( StrRef( "lua"  ) ) )
	        {
	            e->Set( MsgScript::ScriptLangUnknown ) << s;
	            return 1;
	        }
	        else
	            lang = *s;
	    }

	    if( ( s = opts[ Options::ScriptLangVersion ] ) )
	        if( *s != "53" )
	        {
	            e->Set( MsgScript::ScriptLangVerUnknown ) << lang << s;
	            return 1;
	        }

	    ExtensionClient scr( v, apiVersion, std::move( ecdC ), e );
# else
	    ExtensionClient scr( P4SCRIPT_UNKNOWN, NULL, e );
# endif
	    if( e->Test() )
	        return 1;
	    if( ( s = opts[ Options::ScriptMaxMem ] ) )
	        scr.SetMaxMem( (scriptMem_t)s->Atoi64() );
	    if( ( s = opts[ Options::ScriptMaxTime ] ) )
	        scr.SetMaxTime( s->Atoi64() );
	    s = opts[ Options::Script ];

	    std::string c_port = client.GetPort().Text();
	    std::string c_language = client.GetLanguage().Text();
	    std::string c_host = client.GetHost().Text();
	    std::string c_user = client.GetUser().Text();
	    std::string c_version = client.GetVersion().Text();
	    std::string c_charset = client.GetCharset().Text();
	    std::string c_client = client.GetClient().Text();

# ifdef HAS_EXTENSIONS
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
	    scr.ConfigBinding( P4SCRIPT_CLIENTAPI,
	                       p4_std_any::p4_any
	        ( static_cast< std::function< void( ClientApi& ) > >( fn ) ), e );
# endif

	    if( (*script)[ 0 ] == '-' )
	    {
	        StrBuf txt;
# ifdef HAS_EXTENSIONS
	        auto in = FileSys::CreateUPtr( FST_TEXT );
	        in->Set( "-" );
	        in->Open( FOM_READ, e );

	        if( e->Test() )
	            return 1;

	        in->ReadWhole( &txt, e );

	        if( e->Test() )
	            return 1;
# endif
	        return !scr.doStr( txt.Text(), e ) || e->Test();
	    }

	    return !scr.doFile( script->Text(), e ) || e->Test();
	}


    restart:

	// Connect.

	client.Init( e );

	if( e->Test() )
	    return 1;

	// Normal client -- will connect to server.
	// -s means use debug display mode.

	ClientUser *ui;

	if( !( ui = callerUI ) ) {

	if( opts[ 's' ] )
	    ui = new ClientUserDebug( client.GetAPI() );
	else if( opts[ 'e' ] )
	    ui = new ClientUserDebugMsg( client.GetAPI() );
	else if( opts[ 'F' ] )
	    ui = new ClientUserFmt( opts[ 'F' ], client.GetAPI() );
	else if( opts[ Options::Field ] )
	    ui = new ClientUserMunge( opts, client.GetAPI() );
	else if( opts[ 'G' ] )
	    ui = new ClientUserPython( client.GetAPI() );
	else if( opts[ 'R' ] )
	    ui = new ClientUserRuby( client.GetAPI() );
	else if( ( s = opts[ 'M' ] ) && ( *s == "g" ) )
	    ui = new ClientUserPython( client.GetAPI() );
# if defined( HAS_CPP11 ) && !defined( HAS_BROKEN_CPP11 )
	else if( ( s = opts[ 'M' ] ) && ( *s == "j" ) )
	    ui = new ClientUserJSON( client.GetAPI() );
# endif
	else if( ( s = opts[ 'M' ] ) && ( *s == "r" ) )
	    ui = new ClientUserRuby( client.GetAPI() );
	else if( ( s = opts[ 'M' ] ) && ( *s == "p" ) )
	    ui = new ClientUserPhp( client.GetAPI() );
	else if( ( s = opts[ 'I' ] ) && *s == "debug" )
	    ui = new ClientUserDebugProgress( 1, client.GetAPI() );
	else if( opts[ 'I' ] )
	    ui = new ClientUserProgress( 1, client.GetAPI() );
	else if( enviro.Get( "P4COLORS" ) && strlen(enviro.Get( "P4COLORS" )) )
	    ui = new ClientUserColor( opts[ Options::Color ] ? 1 : 0, 1,
	                              client.GetAPI() );
	else
	    ui = new ClientUser( 1, client.GetAPI() );

	if( opts[ 'q' ] )
	    ui->SetQuiet();

	}

	// Invoke operation in server:
	//	if not -x, bundle up argv and send it on down.
	//	if -x, bundle up argv and a few lines from the file.

	StrPtr *xargsName;

	if( !( xargsName = opts[ 'x' ] ) )
	{
	    // Normal invocation.

	    setVarsAndArgs( client, argc, argv, opts );
	    Error *terr = client.GetTransError();
	    if( terr->Test() )
	    {
	        e->Merge( *terr );
	        // No point running command, lost a command argument.
	        if( !callerUI )
	            delete ui;
	        return 1;
	    }
	    
	    if( ui->CanAutoLoginPrompt() )
		client.SetVarV( P4Tag::v_autoLogin );

	    client.Run( argv[0], ui );
	}
	else if( argc == 1 && !strcmp( argv[0], "run" ) )
	{
	    // p4 -x cmdfile run: serially run multiple commands, same client
	    commandChaining = 1;
	    StrBuf s;
	    FileSys *xf = FileSys::Create( FST_TEXT );

	    xf->Set( *xargsName );
	    xf->Open( FOM_READ, e );

	    if( e->Test() )
	    {
		delete xf;
	        if( !callerUI )
	            delete ui;
		return 1;
	    }

	    while( !e->Test() && !client.Dropped() && !client.GetFatals() )
	    {
		int eof = xf->ReadLine( &s, e );
		if( eof < 0 )
		{
	            e->Set( MsgClient::LineTooLong ) << xf->BufferSize();
		    break;
		}
		else if ( eof > 0 )
		{
	            StrBuf	wordBuffer;
	            char	*words[2048];
		    int 	nwords;
		    
		    nwords = StrOps::Words( wordBuffer, s.Text(), words,
				    ( sizeof( words ) / sizeof( words[0] ) ) );
		    
		    if( !nwords )
	                continue; //blank line in file

	            setVarsAndArgs( client, nwords, words, opts );
	            Error *terr = client.GetTransError();
	            if( terr->Test() )
	            {
	                e->Merge( *terr );
	                if( !callerUI )
	                    delete ui;
	                return 1;
	            }
		    client.Run( words[0], ui );
		    fflush( stdout );
		    fflush( stderr );
		}
		else
		{
		    break;
		}
	    }

	    xf->Close( e );
	    delete xf;
	}
	else if( (*xargsName) != "<" )       
	{
	    // -x [file] invocation

	    StrBuf s;
	    FileSys *xf = FileSys::Create( FST_TEXT );
	    int eof = 0;
	    int count = 0;

	    // Open -x file

	    xf->Set( *xargsName );
	    xf->Open( FOM_READ, e );

	    if( e->Test() )
	    {
		delete xf;
		if( !callerUI )
		    delete ui;
		return 1;
	    }

	    // In blocks of batchSize args.

	    while( !e->Test() && !client.Dropped() && !eof )
	    {
		// If more...

		if( !( eof = !xf->ReadLine( &s, e ) ) )
		{
		    // Set user's (constant) args at beginning of block

		    if( !count )
	            {
	                setVarsAndArgs( client, argc, argv, opts );
	                Error *terr = client.GetTransError();
	                if( terr->Test() )
	                {
	                    e->Merge( *terr );
	                    if( !callerUI )
	                        delete ui;
	                    return 1;
	                }
	            }
		    // Set this arg.

		    client.SetVar( "", &s );
	            Error *terr = client.GetTransError();
	            if( terr->Test() )
	            {
	                e->Merge( *terr );
	                if( !callerUI )
	                    delete ui;
	                return 1;
	            }
		}

		// Dispatch

		if( eof ? count : ++count == batchSize )
		{
		    client.Run( argv[0], ui );
		    count = 0;
		}
	    }

	    // Seal off xargs file.

	    xf->Close( e );
	    delete xf;
	}
	else                         // -x - from an alias chain
	{
	    StrBuf s;
	    int eof = 0;
	    int count = 0;

	    StrBuf data;
	    ui->InputData( &data, e );
	    StrPtrLineReader splr( &data );

	    while( !e->Test() && !client.Dropped() && !eof )
	    {
		if( !( eof = !splr.GetLine( &s ) ) )
		{
		    if( !count )
	            {
	                setVarsAndArgs( client, argc, argv, opts );
	                Error *terr = client.GetTransError();
	                if( terr->Test() )
	                {
	                    e->Merge( *terr );
	                    if( !callerUI )
	                        delete ui;
	                    return 1;
	                }
	            }
		    client.SetVar( "", &s );
	            Error *terr = client.GetTransError();
	            if( terr->Test() )
	            {
	                e->Merge( *terr );
	                if( !callerUI )
	                    delete ui;
	                return 1;
	            }
		}

		if( eof ? count : ++count == batchSize )
		{
		    client.Run( argv[0], ui );
		    count = 0;
		}
	    }
	}

	// Since we only did sync Client::Run() calls, the ui should
	// no longer be necessary.

	if( !callerUI )
	    delete ui;

	// Close and clean.

	result = client.Final( e );

	if( result && ErrorIncludesRPCSubsystem( e ) &&
	    opts[ 'r' ] && ( restarted++ < opts[ 'r' ]->Atoi() ) )
	{
	    e->Clear();
	    goto restart;
	}

	return result;
}

int
clientMerger( int argc, char **argv, Error *e )
{
	// Parse up options

	Options opts;
	int longOpts[] = { Options::DiffFlags, Options::BinaryAsText,
	                   Options::Verbose, 0 };
	opts.ParseLong( argc, argv, "d:rtvx:", longOpts, OPT_THREE, usage, e );

	if( e->Test() )
	    return 1;

	int rcsstyle = opts[ 'r' ] != 0;
	int showall = opts[ 'v' ] != 0;
        StrPtr *checkMerge = opts[ 'x' ];

	/* Run as 3-way diff for merging */

	FileSys *f1 = FileSys::Create( FST_BINARY );
	FileSys *f2 = FileSys::Create( FST_BINARY );
	FileSys *f3 = FileSys::Create( FST_BINARY );

	f1->Set( argv[0] );
	f2->Set( argv[1] );
	f3->Set( argv[2] );

	StrBuf baseDigest;
	StrBuf l1Digest;
	StrBuf l2Digest;

	MD5 baseMD5;
	MD5 resultMD5;
	MD5 l1MD5;
	MD5 l2MD5;
	
	if( checkMerge )
	{
	    f1->Digest( &baseDigest, e );
	    f2->Digest( &l1Digest, e );
	    f3->Digest( &l2Digest, e );

	    e->Clear();
	}

	DiffFlags flags( opts[ 'd' ] );
	LineType lineType = opts[ 't' ] ? LineTypeRaw : LineTypeLocal;

	DiffMerge *m = new DiffMerge( f1, f2, f3, flags, lineType, e );
	AssertLog.Abort( e );

	StrBuf sBuf;
	char buf[4096];
	int oldbits = 0, bits;
	int len;

	for( ; ( bits = m->Read( buf, sizeof( buf ), &len ) ); oldbits = bits )
	{
	    /* Merge check */

	    if( checkMerge )
	    {
	        sBuf.Clear();
	        sBuf << buf;
	        sBuf.SetLength( len );

		if( bits & SEL_BASE )
	        {
	            if( *checkMerge == "base" ) 
		        fwrite( buf, 1, len, stdout );
	            baseMD5.Update( sBuf );
	        }
		if( bits & SEL_LEG1)
	        {
	            if( *checkMerge == "leg1" ) 
		        fwrite( buf, 1, len, stdout );
	            l1MD5.Update( sBuf );
	        }
		if( bits & SEL_LEG2 )
		{
	            if( *checkMerge == "leg2" ) 
		        fwrite( buf, 1, len, stdout );
	            l2MD5.Update( sBuf );
		}
	        if( bits & SEL_RSLT || bits == ( SEL_BASE | SEL_CONF ) )
	        {
	            if( *checkMerge == "result" ) 
		        fwrite( buf, 1, len, stdout );
	           resultMD5.Update( sBuf );
	        }

	        continue; 
	    }

	    /* Display tag */

	    if( !oldbits || oldbits == bits )
	    {
		/* OK */;
	    }
	    else if( !rcsstyle )
	    {
		printf( ">>>> %s\n", m->BitNames( bits ) );
	    }
	    else if( showall || 
		     ( bits & SEL_CONF ) || 
		     ( bits == SEL_ALL && ( oldbits & SEL_CONF ) ) )
	    {
		switch( bits )
		{
		case SEL_ALL:
		    printf( "<<<<\n" );
		    break;

		case SEL_BASE:
		case SEL_BASE|SEL_CONF:
		case SEL_BASE|SEL_LEG1:
		case SEL_BASE|SEL_LEG2:
		    printf( ">>>> %s\n", m->BitNames( bits ) );
		    break;

		case SEL_LEG1|SEL_RSLT|SEL_CONF:
		    /* fall through */

		case SEL_LEG2|SEL_RSLT|SEL_CONF:
		case SEL_LEG1|SEL_RSLT:
		case SEL_LEG2|SEL_RSLT:
		case SEL_LEG1|SEL_LEG2|SEL_RSLT:
		    printf( "==== %s\n", m->BitNames( bits ) );
		    break;
		}
	    }

	    /* Display data */

	    if( !rcsstyle || showall || 
	        bits & SEL_RSLT || 
		bits == ( SEL_BASE | SEL_CONF ) )
	    {
		fwrite( buf, 1, len, stdout );
	    }
	}

	if( checkMerge && *checkMerge == "check" )
	{
	    StrBuf baseFinal;
	    StrBuf l1Final;
	    StrBuf l2Final;
	    StrBuf resultFinal;

	    resultMD5.Final( resultFinal );
	    baseMD5.Final( baseFinal );
	    l1MD5.Final( l1Final );
	    l2MD5.Final( l2Final );

	    if( baseDigest.Compare( baseFinal ) )
	        printf("BAD (base)\n");
	    if( l1Digest.Compare( l1Final ) )
	        printf("BAD (leg1)\n");
	    if( l2Digest.Compare( l2Final ) )
	        printf("BAD (leg2)\n");

	    if( ( !baseDigest.Compare( l1Digest ) &&
	           l2Digest.Compare( resultFinal ) )
	      ||( !baseDigest.Compare( l2Digest ) &&
	           l1Digest.Compare( resultFinal ) ) )
	        printf("BAD (safe-merge)\n");
	}

	delete m;
	delete f1;
	delete f2;
	delete f3;

	return 0;
}

int
clientTickets( int argc, char **argv, Options &global_opts, Error *e )
{
	const char *c;
	HostEnv h;
	StrBuf cwd;
	StrBuf ticketfile;
	StrBuf output;

	// Parse up options

	Options opts;
	int longOpts[] = { Options::Charset, Options::CmdCharset, 0 };
	opts.ParseLong( argc, argv, "C:l:Q:", longOpts, OPT_ANY, usage, e );

	if( e->Test() )
	    return 1;

	if( global_opts[ 'd' ] )
	    cwd.Set( global_opts[ 'd' ] );
	else
	    h.GetCwd( cwd );
	Enviro enviro;
	enviro.Config( cwd );
	const char *lc;
	StrPtr *s;
	CharSetCvt::CharSet cs = CharSetCvt::NOCONV;

	if( ( s = opts[ 'l' ] ) || ( s = opts[ 'C' ] ) )
	{
	    lc = s->Text();
	}
	else
	{
	    lc = enviro.Get( "P4CHARSET" );
	}

	if( lc )
	{
	    cs = CharSetCvt::Lookup( lc );

	    if( (int)cs == -1 )
	    {
		// bad charset specification
		BadCharset();
		return 1;
	    }
	    else
	    {
		if( ( s = opts[ 'Q' ] ) ||
		    ( lc = enviro.Get( "P4COMMANDCHARSET" ) ) )
		{
		    if( s )
			lc = s->Text();
		    cs = CharSetCvt::Lookup( lc );
		    if( (int)cs == -1 )
		    {
			printf( "P4COMMANDCHARSET unknown\n" );
			return 1;
		    }
		}
		if( CharSetApi::Granularity( cs ) != 1 )
		{
		    printf( "p4 can not support a wide charset unless\n"
			    "P4COMMANDCHARSET is set to another charset.\n" );
		    return 1;
		}
		if( cs )
		{
		    GlobalCharSet::Set( cs );
		    enviro.SetCharSet( cs );
	            if( !global_opts[ 'd' ] )
		        h.GetCwd( cwd, &enviro );
		    enviro.Config( cwd );
		}
	    }
	}

	if( ( c = enviro.Get( "P4TICKETS" ) ) )
	    ticketfile.Set( c );
	else
	    h.GetTicketFile( ticketfile );

	Ticket t( &ticketfile );
	t.List( output );

	if( cs )
	{
	    // i18n mode - need to translate from utf8 to P4CHARSET

	    CharSetCvt *converter = CharSetCvt::FindCvt( CharSetCvt::UTF_8, cs );
	    if( converter )
	    {
		c = converter->FastCvtQues( output.Text(), output.Length() );
		printf( "%s", c ? c : output.Text() );
		delete converter;
	    }
	}
	else
	    printf( "%s", output.Text() );
	
	return 0;
}

int
clientSet( int argc, char **argv, Options &global_opts, Error *e )
{
	/* p4 set - manipulate env vars */

	StrBuf var;

	// Parse up options

	Options opts;
	int longOpts[] = { Options::System, Options::Service,
	                   Options::Charset, Options::CmdCharset, 
			   Options::Quiet, 0 };
	opts.ParseLong( argc, argv, "sS:C:l:Q:q", longOpts, OPT_ANY, usage, e );

	if( e->Test() )
	    return 1;

	// Blast out message.

	Enviro enviro;
	HostEnv h;
	StrBuf cwd;

	if( opts[ 's' ] )
	    enviro.BeServer();

	if( opts[ 'S' ] )
	{
	    int serviceExists = enviro.BeServer( opts[ 'S' ], !argc );
	    if( !serviceExists )
	    {
	        printf( "Perforce service '%s' does not exist.\n",
	                opts[ 'S' ]->Text() );
	        return 1;
	    }
	}
	    
	if( global_opts[ 'd' ] )
	    cwd.Set( global_opts[ 'd' ] );
	else
	    h.GetCwd( cwd, &enviro );

	StrBuf charsetVar;

	const char *lc;
	StrPtr *s;
	enviro.LoadConfig( cwd, argc == 0 );

	if( ( s = global_opts[ 'p' ] ) )
	    lc = s->Text();
	else if( ! ( lc = enviro.Get( "P4PORT" ) ) )
	    lc = "perforce:1666";

	charsetVar.Set( "P4_" );
	if( strchr( lc, '=' ) )
	{
	    // If the port contains an equals we replace it
	    // since environment names can not have an equals
	    StrBuf tmp( lc );

	    StrOps::Sub( tmp, '=', '@' );
	    charsetVar.Append( &tmp );
	}
	else
	    charsetVar.Append( lc );
	charsetVar.Append( "_CHARSET" );

	if( ( s = opts[ 'l' ] ) || ( s = opts[ 'C' ] ) ||
			 ( s = global_opts[ 'C' ] ) )
	{
	    lc = s->Text();
	}
	else
	{
	    lc = enviro.Get( "P4CHARSET" );
	    if( !lc )
		lc = enviro.Get( charsetVar.Text() );
	}

	if( lc )
	{
	    CharSetCvt::CharSet cs = CharSetCvt::Lookup( lc );

	    if( (int)cs == -1 )
	    {
		// bad charset specification
		BadCharset();
		printf(	"Continuing as if not set.\n" );
	    }
	    else
	    {
		if( ( s = opts[ 'Q' ] ) ||
		    ( s = global_opts[ 'Q' ] ) ||
		    ( lc = enviro.Get( "P4COMMANDCHARSET" ) ) )
		{
		    if( s )
			lc = s->Text();
		    cs = CharSetCvt::Lookup( lc, &enviro );
		    if( (int)cs == -1 )
		    {
			printf( "P4COMMANDCHARSET unknown\n" );
			return 1;
		    }
		}
		if( CharSetApi::Granularity( cs ) != 1 )
		{
		    printf( "p4 can not support a wide charset unless\n"
			    "P4COMMANDCHARSET is set to another charset.\n"
			    "Attempting to discover a good charset.\n" );
		    cs = CharSetCvt::Discover( &enviro );
		    if( cs == -1 )
		    {
			printf( "Could not discover a good charset.\n" );
			return 1;
		    }
		}
		if( cs )
		{
		    GlobalCharSet::Set( cs );
		    enviro.SetCharSet( cs );
	            if( !global_opts[ 'd' ] )
		        h.GetCwd( cwd, &enviro );
		    enviro.Config( cwd );
		}
	    }
	}

	if( !argc )
	{
	    enviro.List( opts[ 'q' ] ? 1 : 0 );

	    enviro.Print( charsetVar.Text(), opts[ 'q' ] ? 1 : 0 );

	    int o = 0;
	    while( p4tunable.GetName( o ) )
	    {
	        if( p4tunable.IsSet( o ) )
	            printf( "%s: %d\n",
	                    p4tunable.GetName( o ), p4tunable.Get( o ) );
	        o++;
	    }
	}
	else for( ; argc; argc--, argv++ )
	{
	    char *equals;

	    if( ( equals = strchr( *argv, '=' ) ) )
	    {
		var.Set( *argv, equals - *argv );
		++equals;
		if( ( var.SCompare( StrRef( "P4CHARSET" ) ) == 0 ||
			var.SCompare( StrRef( "P4COMMANDCHARSET" ) ) == 0 ||
			 var.EndsWith( "_CHARSET", 8 ) )
		    && *equals )
		{
		    CharSetApi::CharSet cs = CharSetCvt::Lookup( equals );
		    if( (int)cs == -1 )
		    {
			printf( "Will not set %s to an invalid value.\n",
			    var.Text() );
			continue;
		    }
		    if( CharSetApi::Granularity( cs ) != 1 )
		    {
			lc = enviro.Get( "P4COMMANDCHARSET" );

			if( !lc || 
			    -1 == (int)( cs = CharSetCvt::Lookup( lc ) ) ||
			    CharSetApi::Granularity( cs ) != 1 )
			    printf( "p4 can not support a wide charset unless\n"
			        "P4COMMANDCHARSET is set to another charset.\n" );
			return 1;
		    }
		}
		enviro.Set( var.Text(), equals, e );
		AssertLog.Report( e );
		e->Clear();
	    }
	    else
	    {
		enviro.Print( *argv, opts[ 'q' ] ? 1 : 0 );
	    }
	}

	return 0;
}

static ErrorId replicateUsage = { ErrorOf( 0, 0, 0, 0, 0),
	"p4 [ -p port ][ -u user ][ -P password ][ -C charset ] replicate\n"
	"\t   [ -j token ][ -s statefile ][ -i interval ][ -k -x -R ]\n"
	"\t   [ -J prefix ][ -T tables excluded ][ -o output ][ command ]\n"
};

int
clientReplicate( int ac, char **av, Options &preops )
{
	Error e;

	AssertLog.SetTag( "replicate" );

	Options opts;

	int longOpts[] = { Options::JournalPrefix, Options::Repeat,
	                   Options::OutputFile, 0 };
	opts.ParseLong( ac, av, "xkj:i:J:s:o:T:R", longOpts,
	                OPT_ANY, replicateUsage, &e );

	if( e.Test() )
	{
	    AssertLog.Report( &e );
	    return 1;
	}

	return jtail( ac, av, preops, opts, "replicate" );
}

int
clientIgnores( int argc, char **argv, Options &global_opts, Error *e )
{
	HostEnv h;
	Ignore i;
	StrBuf cwd;
	StrBuf path;
	StrBuf ignorefile;

	// Parse up options

	Options opts;
	int longOpts[] = { 0 };
	opts.ParseLong( argc, argv, "vi", longOpts, OPT_ANY, usage, e );

	if( e->Test() )
	    return 1;
	
	if( opts['i'] && !argc )
	{
	    printf( "At least one file path must provided.\n" );
	    return 1;
	}
	if( !opts['i'] && argc > 1 )
	{
	    printf( "Only one path may be provided.\n" );
	    return 1;
	}
	
	if( global_opts[ 'd' ] )
	    cwd.Set( global_opts[ 'd' ] );
	else
	    h.GetCwd( cwd );

	Enviro enviro;
	enviro.Config( cwd );

	// A little bit of work to handle -E P4IGNORE

	StrBufDict envList;
	StrRef var, val;
	StrPtr* s;
	for ( int i = 0 ; ( s = global_opts.GetValue( 'E', i ) ) ; i++ )
	    envList.SetVarV( s->Text() );

	for ( int i = 0 ; envList.GetVar( i, var, val ) ; i++ )
	    enviro.Update( var.Text(), val.Text() );

	{
	    // Use the client defaulting logic
	    Client client( &enviro );
	    ignorefile = client.GetIgnoreFile();
	}

	// Collect some info about our target file

	FileSys *f = FileSys::Create( FST_BINARY );
	PathSys *p = PathSys::Create();

	if( argc-- )
	    f->Set( argv++[0] );
	else
	    f->Set( cwd );

	int exists = f->Stat();
	path.Set( f->Path() );
	p->SetLocal( cwd, path );
	path.Set( p->Text() );


	if( !opts['i'] )
	{
	    delete f;
	    delete p;

	    // Just list P4IGNORE rules in play at the path provided

	    // Add an extra level for good measure
	    if( ( exists &= FSF_DIRECTORY ) )
	        path << "/*";

	    StrArray output;
	    i.List( path, ignorefile, enviro.Get( "P4CONFIG" ), &output );

	    char *l;
	    for( int j = 0; j < output.Count(); ++j )
	    {
	        l = output.Get( j )->Text();
	        if( !opts[ 'v' ] &&
	            ( !strncmp( l, "#FILE ", 6 ) ||
	              !strncmp( l, "#LINE ", 6 ) ) )
	            continue;

	        printf( "%s\n", l );
	    }

	    return 0;
	}

	// Check if each provided path would be ignored

	char* configName = enviro.Get( "P4CONFIG" );
	while( path.Length() )
	{
	    StrBuf line;
	    int res;

	    // Check if the target is ignored

	    if( ( exists &= FSF_DIRECTORY ) )
	        res = i.RejectDir( path, ignorefile, configName, &line);
	    else
	        res = i.Reject( path, ignorefile, configName, &line );


	    if( opts['v'] && line.Length() )   // verbose, with reason
	        printf( "%s %s by %s\n",
	                path.Text(),
	                res ? "ignored" :"not ignored",
	                line.Text() );
	    else if( opts['v'] && !res )       // not ignored (verbose)
	        printf( "%s not ignored\n", path.Text() );
	    else if( res )
	        printf( "%s ignored\n", path.Text() );


	    // Move onto the next one

	    if( argc-- )
	    {
	        f->Set( argv++[0] );
	        path.Set( f->Path() );
	        p->SetLocal( cwd, path );
	        path.Set( p->Text() );
	    }
	    else
	        path.Clear();
	}
	
	delete f;
	delete p;

	return 0;
}
