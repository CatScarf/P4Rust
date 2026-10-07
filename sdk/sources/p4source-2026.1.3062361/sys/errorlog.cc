/*
 * Copyright 1995, 1996 Perforce Software.  All rights reserved.
 */

/*
 * ErrorLog.cc - report layered errors
 */

# define NEED_SYSLOG
# define NEED_FLOCK
# define NEED_DBGBREAK
# define NEED_CRITSEC
# define NEED_WIN32FIO
# define NEED_FILE

# include <stdhdrs.h>
# include <strbuf.h>
# include <error.h>
# include <errorlog.h>
# include <fdutil.h>
# include <filesys.h>

/*
 * ErrorLog::errorLog - file where errors are written, type punned
 * ErrorLog::errorTag - string attached to reported errors
 */

Error AssertError;
ErrorLog AssertLog;

ErrorLog::ErrorLog( ErrorLog *from )
	: hook(NULL), context(NULL)
{
	errorFsys = 0;
	errorTag = from->errorTag;
	logType = from->logType;

	// clone errorFsys

	if( from->errorFsys && from->logType == type_none )
	{
	    errorFsys = FileSys::Create( FST_ATEXT );
	    errorFsys->Set( from->errorFsys->Name() );
	    errorFsys->Perms( FPM_RW );
	}		

	// We will not inherit the critical section.  We also do not create
	// a new one, as only AssertLog will be using a critical section.
	vp_critsec = 0;
}

ErrorLog::~ErrorLog()
{
	delete errorFsys;
	errorFsys = 0;

# ifdef HAVE_CRITSEC
	if( vp_critsec )
	{
	    CRITICAL_SECTION *critsec;

	    critsec = (CRITICAL_SECTION *)vp_critsec;
	    DeleteCriticalSection( critsec );
	    delete critsec;
	    vp_critsec = 0;
	}
# endif
}

/*
 * Error::SysLog() - OS style System Logger entry point.
 */

void
ErrorLog::SysLog( const Error *e, int tagged, const char *et, const char *buf )
{
# if defined( HAVE_SYSLOG ) || defined( HAVE_EVENT_LOG )
	const char *errTag = errorTag;

	if( !errorTag )
	    init();

	if( et )
	    errTag = et;
# endif

# ifdef HAVE_SYSLOG
	// Default to LOG_DEBUG to maintain behavior from LogWrite.

	int level = LOG_DEBUG;

	// If severity defined, determine level accordingly.

	if( e )
	{
	    if( e->GetSeverity() != E_FATAL )
		level = LOG_WARNING;
	    else
		level = LOG_ERR;
	}

	/* char * cast for Lynx 4.0 */

	openlog( (char *)errTag, LOG_PID, LOG_DAEMON );
	if( tagged )
	    syslog( level, "%s: %s", e->FmtSeverity(), buf );
	else
	    syslog( LOG_WARNING, "%s", buf );
	closelog();
# endif

# ifdef HAVE_EVENT_LOG
	// Log a warning unless exiting, then log an error.
	// (can exit on E_FAILED, through ::Abort)

	WORD level = EVENTLOG_INFORMATION_TYPE;

	if( e )
	{
	    if( e->GetSeverity() != E_FATAL )
		level = EVENTLOG_WARNING_TYPE;
	    else
		level = EVENTLOG_ERROR_TYPE;
	}

	// Log the event to the Windows Application Event log.
	// We don't configure our event IDs in the registry or have
	// an event message resource.  Windows will preface our entry
	// with a silly warning message.

	HANDLE hEventSource = 
	    RegisterEventSource( NULL, errTag );

	LPTSTR  lpszStrings[1] = { (LPTSTR)buf };

	if( hEventSource != NULL )
	{
	    ReportEvent( 
		    hEventSource,	// handle of event source
		    level,		// event type
		    0,			// event category, (doesn't matter)
		    0,			// event ID
		    NULL,		// current user's SID, (let it default)
		    1,			// strings in lpszStrings
		    0,			// no bytes of raw data
		    (LPCTSTR *)lpszStrings,	// array of strings
		    NULL );		// no raw data

	    DeregisterEventSource(hEventSource);
	}
# endif // HAVE_EVENT_LOG

	return;
}

void
ErrorLog::init()
{
	logType = type_stderr;
	errorTag = "Error";
	errorFsys = 0;
	vp_critsec = 0;
}

void
ErrorLog::EnableCritSec()
{
# ifdef HAVE_CRITSEC
	if( vp_critsec == NULL )
	{
	    CRITICAL_SECTION *critsec;

	    critsec = new CRITICAL_SECTION;
	    InitializeCriticalSection( critsec );
	    vp_critsec = (void *)critsec;
	}
# endif
}

void
ErrorLog::Report( const Error *e, int reportFlags )
{
	if( e->GetSeverity() == E_EMPTY )
	    return;

	int tagged = reportFlags & REPORT_TAGGED;
	int hooked = reportFlags & REPORT_HOOKED;
	int stdio  = reportFlags & REPORT_STDIO;

	if( !errorTag )
	    init();

	StrBuf buf;

	e->Fmt( buf, tagged ? EF_INDENT | EF_NEWLINE : EF_NEWLINE );

# if defined( HAVE_SYSLOG ) || defined( HAVE_EVENT_LOG )
	if ( logType == type_syslog )
	{
	    SysLog( e, tagged, NULL, buf.Text() );
	    if( stdio )
	        StdioWrite( buf );
	    return;
	}
# endif
	if( tagged )
	{
	    StrBuf out;

	    out.Set( errorTag );
	    out.Extend( ' ' );
	    out.Append( e->FmtSeverity() );
	    out.Extend( ':' );
	    out.Extend( '\n' );
	    out.Append( &buf );
	    LogWrite( out, stdio );
	}
	else
	    LogWrite( buf, stdio );

	if( hook && hooked )
	    (*hook)( context, e );
}

void
ErrorLog::LogWrite( const StrPtr &s, int stdio )
{
	int skipFile = 0;
# if defined( HAVE_SYSLOG ) || defined( HAVE_EVENT_LOG )
	if ( logType == type_syslog )
	{
	    // Pass e=NULL and tagged=0 to maintain previous bahavior,
	    // see SysLog for details.

	    SysLog( NULL, 0, NULL, s.Text() );
	    skipFile = 1;
	    if( !stdio )
	        return;
	}
# endif

	if( errorFsys && !skipFile )
	{
	    Error tmpe;

# ifdef HAVE_CRITSEC
	    if( vp_critsec )
	    {
		CRITICAL_SECTION *critsec;

		critsec = (CRITICAL_SECTION *)vp_critsec;
		EnterCriticalSection( critsec );
	    }
# endif

	    errorFsys->Open( FOM_WRITE, &tmpe );
	    if( !tmpe.Test() )
	    {
		errorFsys->Write( &s, &tmpe );
		errorFsys->Close( &tmpe );
	    }
	    if( tmpe.Test() )
	    {
		// An error was encountered when
		// attempting to write to the log.

# if defined( HAVE_SYSLOG ) || defined( HAVE_EVENT_LOG )

		// Write to syslog or the event log the original
		// message that was to be written to the log.
		SysLog( NULL, 0, NULL, s.Text() );

		// Write to syslog or the event log the error that was
		// encountered when attempting to write to the log.
		StrBuf buf;
		tmpe.Fmt( &buf );
		SysLog( &tmpe, 1, NULL, buf.Text() );

# endif

# ifndef OS_NT

		// Write to stderr the error that was encountered when
		// attempting to write to the log. stderr should be
		// well-defined on platforms other than Windows.
		ErrorLog tmpeel;
		tmpeel.SetTag( errorTag );
		tmpeel.Report( &tmpe );

# endif
	    }

# ifdef HAVE_CRITSEC
	    if( vp_critsec )
	    {
		CRITICAL_SECTION *critsec;

		critsec = (CRITICAL_SECTION *)vp_critsec;
		LeaveCriticalSection( critsec );
	    }
# endif
	    
	    if( !stdio )
	        return;
	}

	// Output to stderr if logType is stderr or if we're aborting
	// But not if logType is stdout, just do stdout for now
	if( logType == type_stdout || logType == type_stderr || stdio )
	    StdioWrite( s, logType != type_stdout );

	// If stdout is being captured, output to stderr too
	if( logType == type_stdout && stdio &&
	    ( !isatty( fileno( stdout ) ) || !isatty( fileno( stderr ) ) ) )
	    StdioWrite( s, 1 );
}

void
ErrorLog::StdioWrite( const StrPtr &s, int err )
{
	// Under a Windows Service, stderr is not valid.  The values
	// returned by GetStdHandle for STD_*_HANDLE are also invalid.
	// See fdutil.cc for further details of the state of descriptors
	// and handles when running as a Windows Service.  Also see
	// stdhdrs.h for the definition of INVALID_HANDLE_VALUE2.
	//
	//   Unix logType defaults to type_stderr.
	//   Windows Services must call UnsetLogType().
	//

# ifdef OS_NT
	HANDLE flog=GetStdHandle(STD_ERROR_HANDLE);
	DWORD written;

	if( !err )
	    flog = GetStdHandle(STD_OUTPUT_HANDLE);

	if( flog != INVALID_HANDLE_VALUE2 && flog != 0x0 )
	{
	    // Using the STD_OUTPUT_HANDLE or STD_ERROR_HANDLE
	    // in the non Windows Service case is essentiall
	    // output to the ConDrv Console device.  In this
	    // type of output you can not lock, unlock or flush.
	    //
	    ::WriteFile( flog, s.Text(), s.Length(), &written, NULL );
	}
# if defined( HAVE_EVENT_LOG )
	else
	{
	    // Write to syslog or the event log the original
	    // message that was to be written to the log.
	    SysLog( NULL, 0, NULL, s.Text() );

	    // Write to syslog or the event log the error that was
	    // encountered when attempting to write to the log.
	    Error tmpe;
	    StrBuf buf;
	    tmpe.Fmt( &buf );
	    SysLog( &tmpe, 1, NULL, buf.Text() );
	}
# endif
# else // OS_NT
	FILE *flog=stderr;

	if( !err )
	    flog = stdout;

	// lock the file exclusive for this append,  some platforms
	// don't do append correctly (you know who you are!)

	int fd = fileno( flog );
	lockFile( fd, LOCKF_EX );

	fputs( s.Text(), flog );

	/* Flush even if stderr, for NT's buffered stderr. */

	fflush( flog );

	lockFile( fd, LOCKF_UN );
# endif // ! OS_NT
}

/*
 * Error::Abort() - blurt out an error and exit
 */

void
ErrorLog::Abort( const Error *e )
{
	if( !e->Test() )
	    return;

	ReportAbort( e );

# ifdef HAVE_DBGBREAK
	if( IsDebuggerPresent() )
	{
	    char msg[128];

	    // Notice p4diag this is a Process abort event.
	    sprintf (msg, "event: Process Abort");
	    OutputDebugString(msg);

	    // Under p4diag, create a strack trace and mini dump.
	    DebugBreak();
	}
# endif // HAVE_DBGBREAK

	exit(-1);
}

/*
 * ErrorLog::SetLog() - redirect Abort() and Report() to named file
 */

void
ErrorLog::SetLog( const char *file )
{
	// Redirect to syslog, stdout or stderr?

	if( !strcmp( file, "syslog" ) )
	{
	    logType = type_syslog;
	    return;
	}
	else if( !strcmp( file, "stdout" ) )
	{
	    logType = type_stdout;
	    return;
	}
	else if( !strcmp( file, "stderr" ) )
	{
	    logType = type_stderr;
	    return;
	}

	FileSys *fs = FileSys::Create( FST_ATEXT );
	Error e;

	fs->Set( file );

	fs->Perms( FPM_RW );

	fs->MkDir( &e );
	if( !e.Test() )
	    fs->Open( FOM_WRITE, &e );

	if( e.Test() )
	    AssertLog.Report( &e );
	else
	    UnsetLogType();

	fs->Close( &e );

	if( errorFsys )
	    delete errorFsys;

	errorFsys = fs;
}

void
ErrorLog::Rename( const char *file, Error *e )
{
	FileSys *fs = FileSys::Create( FST_ATEXT );
	fs->Set( file );

	errorFsys->Rename( fs, e );

	delete fs;
}

offL_t
ErrorLog::Size()
{
	if( !errorFsys )
	    return 0;

	Error e;
	errorFsys->Open( FOM_READ, &e );

	if( e.Test() )
	    return 0;

	offL_t size = errorFsys->GetSize();

	errorFsys->Close( &e );

	return size;
}

const char *
ErrorLog::Name()
{
	if( !errorFsys )
	    return 0;

	return errorFsys->Name();
}

const char *
ErrorLog::GetTag()
{
	if( !errorTag || !strcmp( errorTag, "Error" ) )
	    return 0;

	return errorTag;
}