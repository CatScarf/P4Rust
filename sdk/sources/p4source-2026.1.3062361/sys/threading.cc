/*
 * Copyright 1995, 1996 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

/*
 * Threader.cc -- handle multiple users at the same time
 */

# define NEED_SIGNAL
# define NEED_FORK
# define NEED_SLEEP
# define NEED_ERRNO
# define NEED_THREADS
# define NEED_FCNTL

# include <stdhdrs.h>
# include <pid.h>

# include <strbuf.h>
# include <error.h>
# include <errorlog.h>
# include <msgserver.h>
# include <datetime.h>
# include <threading.h>
# include <debug.h>
# include <tunable.h>

# ifdef HAS_RPMALLOC
# include <rpmalloc.h>
# endif

# ifdef USE_SSL
extern "C"
{
# include "openssl/crypto.h"
}
# endif

/*
 * Regarding GetThreadCount:
 *
 * Different threaders maintain their thread counts in different ways. The
 * implementations all try to keep an accurate thread count, but it is
 * possible that the thread count could be inaccurate. Callers of this method
 * should try to use the information primarily for monitoring and diagnosis,
 * and for performance analysis.
 *
 * Currently, GetThreadCount is used for:
 * - server.maxcommands,
 * - thread high-water-mark tracing in the server log
 */

/*
 *
 * Threader - single Threader
 *
 */

Threader::~Threader()
{
}

void
Threader::Launch( Thread *t )
{
	threadCount++;
	t->Run();
	threadCount--;
	delete t;
}

void
Threader::Cancel()
{
	cancelled = 1;
	process->Cancel();
}

void
Threader::Restart()
{
	restarted = 1;
	process->Cancel();
}

void
Threader::NoDowngrade()
{
	canDowngrade = 0;
}

void
Threader::Quiesce()
{
	// no special work for single threading
}

void
Threader::Reap()
{
	// no special termination for single threading
}

# ifndef OS_NT
void
Threader::Cleanup( pid_t pid )
{
	if( process )
	    process->Cleanup( pid );
}
# endif

int
Threader::GetThreadCount()
{
	return threadCount;
}

/*
*
* MultiThreader - threading on NT, using (uh) threads
*
*/

# ifdef OS_NT

# define HAVE_MULTITHREADER

# define WIN32_LEAN_AND_MEAN
# include <windows.h>
# include <process.h>
# include "ntthdlist.h"

#ifdef HAS_CPP11

# include <atomic>
# include <random>

typedef std::atomic< int > THR_COUNT;

// Per-thread pointer to the thread element in global list.
P4MT std::atomic< P4INT64 >* _P4_NT_ThreadCurrentMemBytes = 0;

#endif

// Globally-visible instance, used by 'p4 admin resource-monitor'.
NtThreadList *_P4_NT_ThreadList = 0;

class MultiThreader : public Threader {

    private:

	const bool useProcessorGroups;
	const int dbgLevel;
#ifdef HAS_CPP11
	const int maxGroups;
	int groupToCores[ 64 ]; // maxGroups
	THR_COUNT groupToCurThreads[ 64 ]; // maxGroups
	int overflowMap[ 64 * 512 ]; // maxGroups * maxCores
	int nGroups = 0, curGroup = 0;
	HANDLE curProcess = 0;

	std::ranlux24_base rng; // Fastest generator.
	std::uniform_int_distribution< int > coreMap;

#endif

	void InitProcessorInfo()
	{
#ifdef HAS_CPP11
	    curProcess = GetCurrentProcess();

	    nGroups = GetActiveProcessorGroupCount();

	    for( int i = 0; i < nGroups; i++ )
	        groupToCores[ i ] = GetActiveProcessorCount( i );

	    // Initialize the mapping.
	    for( int i = 0; i < maxGroups; i++ )
	        groupToCurThreads[ i ] = 0;

	    PROCESSOR_NUMBER pn;
	    GetCurrentProcessorNumberEx( &pn );
	    curGroup = pn.Group;

	    // If group 0 has 8 CPUs and group 1 has 4, overflowMap looks like
	    // 0 0 0 0 0 0 0 0 1 1 1 1.  Then, a random number chosen in that
	    // range will schedule the thread to a group proportionally to its
	    // size.

	    int p = 0;

	    for( int i = 0; i < nGroups ; i++ )
	        for( int j = 0; j < groupToCores[ i ]; j++ )
	            overflowMap[ p++ ] = i;

	    // Now that the range is known, reset the distribution.
	    std::uniform_int_distribution< int >::param_type
	        pt( 0, p - 1 );
	    coreMap.param( pt );

	    if( dbgLevel >= 1 )
	    {
	        p4debug.printf( "nGroups %d\ncurGroup %d\n", nGroups, curGroup );
	        for( int i = 0; i < nGroups; i++ )
	            p4debug.printf( "groupToCores %d %d\n", i, groupToCores[ i ] );
	    }

	    if( dbgLevel >= 9 )
	    {
	        p4debug.printf( "overflowMap (p %d): ",p );
	        for( int i = 0; i < p; i++ )
	            p4debug.printf( "%d ", overflowMap[ i ] );
	        p4debug.printf( "\n" );
	    }
#endif
	}

	int DistributeThreads()
	{
#ifdef HAS_CPP11
	    // Note that there's a race between the groupToCurThreads
	    // comparison and the increment in this function.  It's ok
	    // though since we're going to at worst get an additional
	    // thread scheduled on a group before we start spilling into
	    // the next group.

	    if( nGroups == 1 )
	        return curGroup;

	    // Prefer our own processor group so as to avoid any cross-group
	    // overhead such as the CPU cache coherency protocol.

	    if( groupToCurThreads[ curGroup ] < groupToCores[ curGroup ] )
	    {
	        groupToCurThreads[ curGroup ]++;
	        return curGroup;
	    }

	    for( int g = 0; g < nGroups; g++ )
	        if( groupToCurThreads[ g ] < groupToCores[ g ] )
	        {
	            groupToCurThreads[ g ]++;
	            return g;
	        }

	    // All groups have at least one thread assigned to each core,
	    // so now we start assigning threads to groups randomly,
	    // biased towards the size of the group.

	    const int rnd = coreMap( rng );
	    const int group = overflowMap[ rnd ];
	    groupToCurThreads[ group ]++;

	    return group;
#else
	    return 0;
#endif
	}

	struct NtThreadProcParam
	{
	    Thread *thread;
	    MultiThreader* ths;

	    NtThreadProcParam( MultiThreader* ths, Thread *t )
	    {
	        this->ths = ths;
	        thread = t;
	    }
	};

	static unsigned int __stdcall NtThreadProc( void *param )
	{
# ifdef HAS_RPMALLOC
	    rpmalloc_thread_initialize();
# endif

	    NtThreadProcParam* data = static_cast< NtThreadProcParam* >( param );
	    Thread *t = data->thread;
#ifdef HAS_CPP11
	    MultiThreader* ths = data->ths;
#endif
	    delete data;

#ifdef HAS_CPP11
	    GROUP_AFFINITY aff;

	    if( ths->useProcessorGroups )
	    {
	        // Note that without initializing the reserved parts
	        // of this, setting the afinity will error out with
	        // ERROR_INVALID_PARAMETER.
	        memset( &aff, 0, sizeof( GROUP_AFFINITY ) );
	        aff.Mask = 0;
	        aff.Group = ths->DistributeThreads();
	        auto ct = GetCurrentThread();

	        if( !SetThreadGroupAffinity( ct, &aff, nullptr ) )
	        {
	            ths->groupToCurThreads[ ths->curGroup ]++;
	            p4debug.printf( "Failed to set affinity to group %d %d\n",
	                            aff.Group, GetLastError() );
	            // Affinity set failure - just run the thread as-is.
	        }

	        if( ths->dbgLevel > 2 )
	        {
	            PROCESSOR_NUMBER pnN;
	            GetCurrentProcessorNumberEx( &pnN );

	             p4debug.printf( "Thread %d distributed to %d, running on "
	                 "group %d CPU %d total %d\n", GetCurrentThreadId(),
	                 aff.Group, pnN.Group, pnN.Number,
	                 ths->groupToCurThreads[ aff.Group ].load() );
	        }
	    }
#endif
	    // Since we are running in the same address space as parent
	    // process we don't want to call DisService.
	    
#ifdef HAS_CPP11
	    NtThreadList::ThreadInfo* ti =
#endif
	        _P4_NT_ThreadList->AddThread( t, GetCurrentThreadId() );
#ifdef HAS_CPP11
	    _P4_NT_ThreadCurrentMemBytes = &ti->mem;
#endif

	    t->Run();

	    if( !_P4_NT_ThreadList->RemoveThread( t ) )
	    {
	        Error e;
	        char msg[128];

	        sprintf( msg, "Can't remove thread entry %d",
	                 GetCurrentThreadId() );
	        e.Set( E_FATAL, msg );
	        AssertLog.Report( &e );
	    }

#ifdef HAS_CPP11
	    if( ths->useProcessorGroups )
	        ths->groupToCurThreads[ aff.Group ]--;
#endif

	    delete t;

# ifdef USE_SSL
# if OPENSSL_VERSION_NUMBER >= 0x10100000L
	    OPENSSL_thread_stop();
# endif
# endif // USE_SSL

# ifdef HAS_RPMALLOC
	    rpmalloc_thread_finalize();
# endif

	    _endthreadex(0);

	    return 0;
	}

    public:

#ifdef HAS_CPP11
	MultiThreader( ThreadMode tmb )
	    : useProcessorGroups(
	          p4tunable.Get( P4TUNE_SYS_THREADING_GROUPS ) ),
	      dbgLevel( p4debug.GetLevel( DT_THREAD ) ), maxGroups( 64 ),
	      coreMap( 0, 0 )
# else
	MultiThreader( ThreadMode tmb )
	    : useProcessorGroups(
	          p4tunable.Get( P4TUNE_SYS_THREADING_GROUPS ) ),
	      dbgLevel( p4debug.GetLevel( DT_THREAD ) )
# endif
	{
	    if( useProcessorGroups )
	        InitProcessorInfo();
	    delete _P4_NT_ThreadList;
	    _P4_NT_ThreadList = new NtThreadList();
	}

	void Launch( Thread *t )
	{
	    NtThreadProcParam* data = new NtThreadProcParam( this, t );

	    unsigned int ThreadId;

	    HANDLE h = (HANDLE)_beginthreadex(
		    NULL,
		    0,
		    &MultiThreader::NtThreadProc,
		    (void*)data,
		    0,
		    &ThreadId );

	    if( !h )
	    {
		Error e;

		e.Sys( "_beginthreadex()", "NtThreadProc" );
		e.Set( E_FATAL, "Can't create thread" );
		AssertLog.Report( &e );

		delete t;
		return;
	    }

	    CloseHandle( h );
	}

	// By now threads have been notified to self terminate.
	// We wait 15 seconds for the thread count to go to 0.
	// In practice most threads will terminate immediately.
	// Any hold out threads will have to be suspended in Reap.
	// Windows "net stop" can timeout if we take too long.
	//
	// We don't do anything to the processor group tracking
	// since we assume this object isn't going to be used again.

	void Quiesce()
	{
	    Error e;
	    DateTime date;
	    StrBuf dt;

	    date.SetNow();
	    date.Fmt( dt.Alloc( DateTimeBufSize ) );

	    e.Set( MsgServer::Quiescing ) 
		<< dt.Text()
		<< Pid().GetProcID()
		<< GetThreadCount();
	    AssertLog.Report( &e );

	    int retries = 0;
	    while( ! _P4_NT_ThreadList->Empty() && retries++ < 15 )
		sleep( 1 );
	    if( retries < 15 )
		return;

	    e.Clear();
	    e.Set( MsgServer::QuiesceFailed )
		<< dt.Text()
		<< Pid().GetProcID();
	    AssertLog.Report( &e );
	}

	// Locks have been taken, threads should not proceed further.
	// Continue with the restart if there are no hold out threads,
	// otherwise suspend any hold out threads and shutdown.
	//
	// We don't do anything to the processor group tracking
	// since we assume this object isn't going to be used again.

	void Reap()
	{
	    if( _P4_NT_ThreadList->Empty() )
		return;

	    if( restarted && canDowngrade )
	    {
		Error e;
		DateTime date;
		StrBuf dt;

		date.SetNow();
		date.Fmt( dt.Alloc( DateTimeBufSize ) );

		e.Set( MsgServer::ReDowngrade ) 
		    << dt.Text()
		    << Pid().GetProcID();
		AssertLog.Report( &e );

	        restarted = 0;
	        cancelled = 1;
	    }

	    _P4_NT_ThreadList->SuspendThreads();
	}

	int GetThreadCount()
	{
	    return _P4_NT_ThreadList->GetThreadCount();
	}

	int IsSingle() { return 0; }
} ;

# endif

# ifdef OS_BEOS

# define HAVE_MULTITHREADER

# include <OS.h>

static status_t
BeOSRunBinder( void *param )
{
	Thread *t = (Thread *)param;

	// Since we are running in the same address space as parent
	// process we don't want to call DisService.

	t->Run();
	delete t;

	return B_OK;
}

class MultiThreader : public Threader {

    public:

	MultiThreader( ThreadMode tmb )
	{
	}

	void Launch( Thread *t )
	{
	    thread_id threadID;

	    threadID = spawn_thread(
			BeOSRunBinder, "p4d task",
			B_NORMAL_PRIORITY,
		    	(void *)t);

	    if( threadID < 0 )
	    {
		// Create thread failed;
		t->Run();
		delete t;
		return;
	    }

	    resume_thread(threadID);
	}

	int GetThreadCount() { return -1; } // not implemented on BEOS

	int IsSingle() { return 0; }
} ;

# endif /* OS_BEOS */

/*
 *
 * MultiThreader - multi Threader on God-fearing UNIX
 *
 * Everything is easy in the forking scheme, except termination.
 *
 * Launching is done by forking.  We catch SIGCHILD just to do the
 * requisite wait() calls to reap the child's exit status.
 *
 * Termination is orchestrated by the parent.  If a child's Cancel()
 * is called, it makes itself immune from SIGTERM and then send SIGTERM
 * to its parent.  The parent, on SIGTERM, resends SIGTERM to its process
 * group, killing everyone.  Why doesn't the child just send SIGTERM to
 * the process group?  Not sure yet.
 *
 * Restart is like termination, but it uses SIGHUP, not SIGTERM.
 */

# if defined( SIGCHLD ) && \
    !defined( OS_BEOS ) && \
    !defined( OS_AS400 ) && \
    !defined( OS_VMS )

# define HAVE_MULTITHREADER
# define HAVE_SIGHUP_HANDLER

/* For MVS, which must have different C/C++ linkage */
/* This precludes making them static, alas */

extern "C" void HandleSigChld( int flag );
extern "C" void HandleSigTerm( int flag );
extern "C" void HandleSigHup( int flag );

#ifdef HAS_VALGRIND
extern "C" void HandleSigValgrind( int flag );
# endif

void
HandleSigChld( int flag );

void
HandleSigTerm( int flag )
{
	// The child just plain exits on sigterm
	// The parent invokes the global stopping logic.

	Threading::Cancel();
}

void
HandleSigHup( int flag )
{
	Threading::Restart();
}

#ifdef HAS_VALGRIND

void HandleSigValgrind( int signo )
{
	/*
	    When a process running under Valgrind is interrupted by a signal
	    and there's no signal handler for it installed (via signal()),
	    there's a chance that Valgrind will report in-flight memory that is
	    as-of-yet not freed as a leak.  This is technically true since
	    the delivery of an unhandled signal (like SIGHUP/SIGTERM) causes
	    the process to immediately leave whatever it was doing and go
	    exit, not giving destructors or other control flow a chance to
	    do the right thing.

	    This however is not a very useful report for us since we told the
	    process to do this and we don't care what it was busy with.  One
	    might be tempted to fix this with the Valgrind client API, asking it
	    to stop reporting errors once we've received a signal, ala: 

	        if( RUNNING_ON_VALGRIND )
	        {
	            VALGRIND_PRINTF( "This is HandleSigValgrind\n" );
	            VALGRIND_DISABLE_ERROR_REPORTING;
	        }

	        signal( signo, SIG_DFL );
	        kill( Pid().GetProcID(), signo );

	    But it turns out that that's unnecessary, and simply having a
	    signal handler at all, is enough.  So, in the end all we need
	    to do is exit:
	*/

	/*
	 * We should call _exit(0) here rather than exit(0)
	 * because _exit() is async-signal-safe
	 * but exit() is not.
	 *
	 * However, this code runs only under valgrind,
	 * so not in production, flushing streams
	 * might avoid some bogus valgrind reports,
	 * and it's probably safe to do, so we'll
	 * continue to call exit() rather than _exit().
	 */
	exit( 0 );
}
# endif

class MultiThreader : public Threader {

    public:

	MultiThreader( ThreadMode tmb )
	{
	    // Daemon?  Parent forks and exits, leaving child to
	    // call the shots.

	    if( IsDaemon( tmb ) )
	    {
		if( tmb == TmbDaemonSafe )
		{
		    // close stdio file operations
		    close(0);
		    close(1);
		    close(2);
		    int fd = open( "/dev/null", O_RDWR );
		    if( fd == 0 )
		    {
			// must be fd 1 and 2
			(void)(dup(fd)+1);
			(void)(dup(fd)+1);
		    }
		}
		if( fork() > 0 )
		    exit( 0 );
	    }

	    MultiThreader::sharedThreadCount = 0;

	    // become the leader of our process group
	    // This way we can kill everyone with one stroke.

	    setpgid( 0, getpid() );

	    // We'll catch this once

	    signal( SIGTERM, HandleSigTerm );
	    signal( SIGHUP, HandleSigHup );

# if !defined ( OS_AIX41 ) && !defined ( OS_AIX43 ) && !defined ( OS_AIX53 )
	    if( InstallSigChldHandler() == -1 )
	    {
	        p4debug.Event();
	        p4debug.printf(
	            "MultiThreader(): sigaction(SIGCHLD) failed, errno=%d\n",
	            errno );
	    }
# endif
	}

	// Installs HandleSigChld via sigaction(), whose disposition (unlike
	// signal()'s) persists across delivery on every target platform
	// (job135814 / P4-27887). Called from the constructor and again at the
	// top of every Launch(), since a forked child resets SIGCHLD to SIG_DFL
	// below before running.
	static int InstallSigChldHandler()
	{
	    struct sigaction sa;
	    memset( &sa, 0, sizeof( sa ) );
	    sa.sa_handler = HandleSigChld;
	    sigemptyset( &sa.sa_mask );
	    sa.sa_flags = SA_RESTART;
	    return sigaction( SIGCHLD, &sa, 0 );
	}

	// Blocks SIGCHLD around the sharedThreadCount read-modify-write so
	// HandleSigChld's decrement can't interleave and get lost (job135814 /
	// P4-27887). Prefers pthread_sigmask() over sigprocmask(), since
	// Launch() may run in a forked child with a live MicroThread pool.
	static void AdjustThreadCount( int delta )
	{
	    sigset_t chldMask, oldMask;
	    sigemptyset( &chldMask );
	    sigaddset( &chldMask, SIGCHLD );

# ifdef HAVE_PTHREAD
	    int err = pthread_sigmask( SIG_BLOCK, &chldMask, &oldMask );
	    if( err != 0 )
	    {
	        p4debug.Event();
	        p4debug.printf(
	            "AdjustThreadCount(): pthread_sigmask(SIG_BLOCK) failed, "
	            "err=%d\n", err );
	        sharedThreadCount += delta;
	        return;
	    }

	    sharedThreadCount += delta;

	    err = pthread_sigmask( SIG_SETMASK, &oldMask, 0 );
	    if( err != 0 )
	    {
	        p4debug.Event();
	        p4debug.printf(
	            "AdjustThreadCount(): pthread_sigmask(SIG_SETMASK) failed, "
	            "err=%d\n", err );
	    }
# else
	    if( sigprocmask( SIG_BLOCK, &chldMask, &oldMask ) == -1 )
	    {
	        p4debug.Event();
	        p4debug.printf(
	            "AdjustThreadCount(): sigprocmask(SIG_BLOCK) failed, "
	            "errno=%d\n", errno );
	        sharedThreadCount += delta;
	        return;
	    }

	    sharedThreadCount += delta;

	    if( sigprocmask( SIG_SETMASK, &oldMask, 0 ) == -1 )
	    {
	        p4debug.Event();
	        p4debug.printf(
	            "AdjustThreadCount(): sigprocmask(SIG_SETMASK) failed, "
	            "errno=%d\n", errno );
	    }
# endif
	}

	void Launch( Thread *t )
	{
	    // Some OS's allow you to ignore the signal and will reap
	    // children automatically.  This is good for AIX, because
	    // manual reaping just doesn't seem to work. Note that due to
	    // this, AIX doesn't provide a valid GetThreadCount() value.

# if defined ( OS_AIX41 ) || defined ( OS_AIX43 ) || defined ( OS_AIX53 )
	    signal( SIGCHLD, SIG_IGN );
# else
	    if( InstallSigChldHandler() == -1 )
	    {
	        p4debug.Event();
	        p4debug.printf(
	            "Launch(): sigaction(SIGCHLD) failed, errno=%d\n",
	            errno );
	    }

	    AdjustThreadCount( 1 );
# endif

# if defined ( OS_LINUX26 )
	    //
	    // Reap any dead children
	    // (works around Linux kernel bug fixed in 2.6.11).
	    //
	    int status;
	    pid_t pid;
	    /*
	     * Note: Changed waitpid code below from waiting on all child processes
	     * (-1) to waiting on processes in process group (0). This change protects
	     * the child process p4zk which detaches itself from the parent's process
	     * group. When p4d is restarting we want to kill all child processes but
	     * keep the p4zk process going. (If invoking shutdown, p4zk will notice
	     * when connection to p4d closes and will exit)
	     */
	    while( ( pid = waitpid( (pid_t)0, &status, WNOHANG ) ) > 0 )
	    {
	        if( WIFSIGNALED( status ) )
	        {
	            Error e;
	            e.Set( E_FATAL,
	                "Process %pid% exited on a signal %signal%!" );
	            e << pid << WTERMSIG(status);
	            AssertLog.Report( &e );
	        }
	        AdjustThreadCount( -1 );
	    }
# endif

	    switch( fork() )
	    {
	    case -1:
		/*
		 * Fork failed.  We could bail out here, but since we have
		 * the wherewithall to run the request, we'll just go ahead
		 * and do it, hoping that the crisis will abate.
		 */
		{
		    Error e;
		    e.Set( E_FATAL, "fork() failed,  single threading!\n");
		    AssertLog.Report( &e );
		}

		t->Run();
	        AdjustThreadCount( -1 );
		delete t;
		break;

	    case 0:
		/*
		 * The child.  Note it so that we know to handle Stop() by
		 * sending a signal to our parent.  To be nice, we drop the
		 * listen socket needed by the parent, using Unlisten().
		 * We then run the request, delete the handler, and exit.
		 *
		 * Revert SIGCHLD here, as on OSF it will spoil RunCmd().
		 */

#ifdef HAS_VALGRIND
		// Install handlers to explicitly exit(), which helps Valgrind
		// avoid printing unwanted reports about in-flight allocations
		// that it sees as leaking due to the signal-interrupted program.
		signal( SIGTERM, HandleSigValgrind );
		signal( SIGHUP, HandleSigValgrind );
# endif
		process->Child();

#ifdef HAS_VALGRIND
		// Leave the Valgrind handlers in for the duration of the child.
# else
		signal( SIGTERM, SIG_DFL );
		signal( SIGHUP, SIG_DFL );
#endif
		signal( SIGCHLD, SIG_DFL );

		t->Run();
		delete t;
		exit(0);
		break;

	    default:
		/*
		 * The parent.  Delete the handler, which will close the
		 * child connection, and return so that we can go back
		 * to servicing further requests.
		 */

		delete t;
		break;
	    }
	}

# ifdef OS_SUNOS
// sunos requires an argument
# define getpgrp() getpgrp(0)
# endif

# if defined ( OS_AIX41 ) || ( OS_AIX43 ) || ( OS_AIX53 )
// Dumbo getpgrp() returns garbage on AIX41.
# define getpgrp() getpgid(0)
# endif

	void Cancel()
	{
	    int pgrp = getpgrp();

	    if( pgrp != getpid() )
	    {
		// The child really has no control over what's going
		// on, so it just sends a SIGTERM to the parent to tell
		// it to shut down.  We want this child to exit nicely, so
		// we block the SIGTERM the parent will send _us_.

		if( signal( SIGTERM, SIG_IGN ) == SIG_ERR )
		{
		    p4debug.Event();
		    p4debug.printf( "MultiThreader::Cancel(): signal(SIGTERM, SIG_IGN) failed, errno=%d\n", errno );
		}
		if( kill(pgrp, SIGTERM) == -1 )
		{
		    p4debug.Event();
		    p4debug.printf( "MultiThreader::Cancel(): kill(SIGTERM) failed, errno=%d\n", errno );
		}
	    }
	    else
	    {
		// The parent knows what to do:

		Threader::Cancel();
	    }
	}

	void Restart()
	{
	    int pgrp = getpgrp();

	    if( pgrp != getpid() )
	    {
		// Send SIGHUP to parent to tell it to restart.
		// Just as for Cancel(), we want the child to exit nicely.

		if( signal( SIGHUP, SIG_IGN ) == SIG_ERR )
		{
		    p4debug.Event();
		    p4debug.printf( "MultiThreader::Restart(): signal(SIGHUP, SIG_IGN) failed, errno=%d\n", errno );
		}
		if( signal( SIGTERM, SIG_IGN ) == SIG_ERR )
		{
		    p4debug.Event();
		    p4debug.printf( "MultiThreader::Restart(): signal(SIGTERM, SIG_IGN) failed, errno=%d\n", errno );
		}
		if( kill( pgrp, SIGHUP ) == -1 )
		{
		    p4debug.Event();
		    p4debug.printf( "MultiThreader::Cancel(): kill(SIGHUP) failed, errno=%d\n", errno );
		}
	    }
	    else
	    {
		Threader::Restart();
	    }
	}

	void Quiesce()
	{
	    // Someday we might handle the sub-processes here.
	}

	void Reap()
	{
	    // Kill all child processes.
	    // 1. Block SIGTERM so #2 doesn't get us.
	    // 2. Kill everyone in our process group.
	    // 3. Wait for them to die.

	    signal( SIGTERM, SIG_IGN );

	    kill( 0, SIGTERM );

	    int status;
	    /*
	     * Note: Changed waitpid code below from waiting on all child processes
	     * (-1) to waiting on processes in process group (0). This change protects
	     * the child process p4zk which detaches itself from the parent's process
	     * group. When p4d is restarting we want to kill all child processes but
	     * keep the p4zk process going. (If invoking shutdown, p4zk will notice
	     * when connection to p4d closes and will exit)
	     *
	     * We now signal SIGTERM periodically in order to encourage our
	     * children to die; this avoids a deadlock during shutdown or restart
	     * when the parent has locked db.* but a child is trying to lock one
	     * of them.  The parent was waiting for the child to die, and the child
	     * was waiting for the parent to release the lock.
	     */
	    int pid = 0;
	    int error = 0;

	    do
	    {
		errno = 0;
		pid = waitpid( (pid_t)0, &status, WNOHANG );
		error = errno;
		if( pid == 0 )	// sleep only if no child exited this time around
		{
		    kill( 0, SIGTERM );		// die! die! my darling!
		    msleep( 50 );
		}
	    } while( (pid >= 0) || (error != ECHILD) );
	}

	int GetThreadCount()
	{
# if defined ( OS_AIX41 ) || ( OS_AIX43 ) || ( OS_AIX53 )
	    // Since SIGCHLD doesn't work on AIX, thread count is unavailable.
	    return -1;
# else
	    return sharedThreadCount;
# endif
	}

	int IsSingle() { return 0; }

	static volatile sig_atomic_t sharedThreadCount;
} ; 

volatile sig_atomic_t MultiThreader::sharedThreadCount = 0;

void
HandleSigChld( int flag )
{
	int status;
	pid_t pid;

# if defined(OS_CYGWIN) || defined(OS_LINUX26)
	// Cygwin (2.95.3) doesn't restore errno on return from interrupt.
	// This leads accept() to return ECHILD rather than EINTR.

	int save_errno = errno;
# endif
	/*
	 * Note: Changed waitpid code below from waiting on all child processes
	 * (-1) to waiting on processes in process group (0). This change protects
	 * the child process p4zk which detaches itself from the parent's process
	 * group. When p4d is restarting we want to kill all child processes but
	 * keep the p4zk process going. (If invoking shutdown, p4zk will notice
	 * when connection to p4d closes and will exit)
	 */
	while( ( pid = waitpid( (pid_t)0, &status, WNOHANG ) ) > 0 )
	{
	    MultiThreader::sharedThreadCount--;

	    if( WIFSIGNALED( status ) )
	    {
		Error e;
	        if( WTERMSIG( status ) != SIGTERM )
	        {
	            e.Set( E_FATAL,
	                   "Process %pid% exited on a signal %signal%!" );
# ifndef OS_NT
	            Threading::Cleanup( pid );
# endif
	        }
	        else
		    e.Set( E_INFO,
	                   "Process %pid% terminated normally during server shutdown." );

		e << pid << WTERMSIG(status);
		AssertLog.ReportNoHook( &e );
	    }
	}

# if defined(OS_CYGWIN) || defined(OS_LINUX26)
	errno = save_errno;
# endif
}

# endif /* UNIX */

# ifndef HAVE_MULTITHREADER

class MultiThreader : public Threader
{
    public:
	MultiThreader( ThreadMode tmb )
	{
	}
} ;

# endif

# ifdef HAVE_PTHREAD

/*
 * This is a minimal MultiThreader implementation based on posix threads.
 *
 * It is by no means complete and not suitable for use with the perforce
 * server until it handles termination, thread count, etc.
 *
 * Also, the variable qualifier MT_STATIC needs to be worked on.
 * Not all supported platforms can handle that.
 *
 */

class PosixThreader : public Threader {
    public:
	PosixThreader( ThreadMode )
	{
	}

	void Launch( Thread *t )
	{
		pthread_t newthread;

		pthread_create( &newthread, NULL, starter, (void *)t );

		pthread_detach( newthread );
	}

	int IsSingle() { return 0; }

    private:

	static void *starter( void *v )
	{
	    Thread *t = (Thread *)v;

	    t->Run();

	    delete t;

	    return NULL;
	}

};

# else
typedef MultiThreader PosixThreader;
# endif /* HAVE_PTHREAD */

/*
 *
 * Threader -- generic top level threading
 *
 */

Threader *Threading::current = 0;

Threading::Threading( ThreadMode tmb, Process *p )
{
	switch( tmb )
	{
	case TmbSingle:

# ifdef HAVE_SIGHUP_HANDLER
	    signal( SIGHUP, HandleSigHup );
# endif
	    threader = new Threader;
	    break;
	case TmbMulti:
	case TmbDaemon:
	case TmbDaemonSafe:
	    threader = new MultiThreader( tmb );
	    break;
	case TmbThreads:
	    threader = new PosixThreader( tmb );
	    break;
	}

	threader->process = p;
	current = threader;
}

Thread::~Thread() {}

Process::~Process() {}
