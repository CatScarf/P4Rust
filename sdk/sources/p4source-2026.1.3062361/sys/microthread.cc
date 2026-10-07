/*
 * Copyright 1995, 2022 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

/*
 * MicroThead.cc -- Fine Task Threading
 */

// NOTE if we don't have threads, this code is not tested
# define NEED_THREAD
# define NEED_SIGNAL
# define NEED_THREADS
# include <stdhdrs.h>
# include <error.h>
# include <debug.h>
# include <vararray.h>

# include <microthread.h>

# ifdef HAS_CPP11
# include <system_error>
# endif

# ifdef THREAD_CPP_DEBUG
// THREAD_CPP_DEBUG is C++ thread specific stuff which doesn't fit
// into the usual Perforce debug facilities.
// thread ids are only printable via operator <<
// Do not normally build this since we do not want debug output on stdout
# include <iostream>
# endif

# define DEBUG_ASSERT ( p4debug.GetLevel( DT_THREAD ) >= 1 )
# define DEBUG_THREAD ( p4debug.GetLevel( DT_THREAD ) >= 4 )
# define DEBUG_POOL ( p4debug.GetLevel( DT_THREAD ) >= 2 )

MicroThread::MicroThread()
    : started( false ), running( false ), joined( false ), postrun( false ),
	waiton( false ), pool( NULL )
{
	dbgconfig = P4DebugConfig::ThreadClone();
}

MicroThread::~MicroThread()
{
# ifdef HAVE_THREAD
	mymutex.lock();
	DEBUGPRINTF( DEBUG_THREAD,
	             "MT destructor start %d, running %d, joined %d",
	             started, running, joined);

	if( started && !joined )
	{
	    DEBUGPRINT( DEBUG_THREAD, "waiting in destructor");
	    mymutex.unlock();
	    Wait();
	    mymutex.lock();
	}

# ifdef THREAD_CPP_DEBUG
	std::cout << "destructor thread id " << mythread.get_id() << std::endl;
# endif
	mymutex.unlock();
# endif
	delete dbgconfig;
}

bool
MicroThread::Started()
{
	int st;
# ifdef HAVE_THREAD
	mymutex.lock();
# endif
	st = started;
# ifdef HAVE_THREAD
	mymutex.unlock();
# endif
	return st;
}

int
MicroThread::RunWork()
{
	int ret = 0;
# ifdef HAVE_THREAD
	mymutex.lock();
	started = true;
	running = true;

	try {
	    std::thread t( &MicroThread::WorkLaunch, this );
	    mythread = std::move( t );
	}
	catch( const std::system_error &ex )
	{
	    ret = 1;
	    p4debug.printf( "Failed to create new thread\n");
	}
	mymutex.unlock();
# else
	started = true;
	running = true;

	WorkLaunch( this );
# endif
	return ret;
}

void
MicroThread::Wait()
{
# ifdef HAVE_THREAD

	DEBUGPRINTF( DEBUG_THREAD, "thread waiting joined %d, waiton %d", joined, waiton );

	mymutex.lock();

	if( !joined && !waiton )
	{
	    waiton = true;
	    joined = true;
	    if( mythread.joinable() )
	    {
	        DEBUGPRINTF( DEBUG_THREAD, "real join running %d started %d", running, started );
# ifdef THREAD_CPP_DEBUG
	        std::cout << "joining " << mythread.get_id()
	                  << " vs " << std::this_thread::get_id() << std::endl;
# endif
	        mymutex.unlock();
	        mythread.join();
	        mymutex.lock();
	    }
	    DEBUGPRINT( DEBUG_THREAD, "joined" );
	}

	// assert not running
	running = false; // in case of an unclean exit

	mymutex.unlock();

	if( ! postrun )
	{
	    PostWork();
	    postrun = true;
	}
	DEBUGPRINT( DEBUG_THREAD, "thread done waiting" );

# endif
}

/*
 * This function is the top level of threads here.
 */

void
MicroThread::WorkLaunch( void * w )
{
	// SHOULD THERE BE A MIMALLOC SPECIAL CALL HERE???

	MicroThread *t = (MicroThread *)w;

# if defined( HAVE_PTHREAD ) && defined( SIGCHLD )
	// Block SIGCHLD for the life of every pool worker (job135814 /
	// P4-27887), so it can only land on the thread that adjusts
	// sharedThreadCount.
	sigset_t chldMask;
	sigemptyset( &chldMask );
	sigaddset( &chldMask, SIGCHLD );
	pthread_sigmask( SIG_BLOCK, &chldMask, 0 );
# endif

# ifdef THREAD_CPP_DEBUG
	t->mymutex.lock();
	// make sure that t->mythread is valid for debug output.
	t->mymutex.unlock();
# endif
	// setup debug file
	if ( t->dbgconfig )
	    t->dbgconfig->Install();

	DEBUGPRINT( DEBUG_THREAD, "Fresh Start" );
# ifdef THREAD_CPP_DEBUG
	std::cout << "starting thread id " << t->mythread.get_id() << std::endl;

	int i = 0;
# endif
	int haveerr = 0;
	int skiponerror = t->pool->IsSkipOnError();

	do {
	    // If we have an error and we are skipping on errors
	    // don't call the Work() function.
	    if( ! ( haveerr && skiponerror ) )
	    {
	        t->Work();
	        if( t->err.Test() )
	            haveerr = 1;
	    }

# ifdef THREAD_CPP_DEBUG
	    ++i;
# endif

# ifdef HAVE_THREAD
	    t->mymutex.lock();
# endif

	    if( t->waiton || t->pool == NULL )
	    {
	        t->running = false;
# ifdef HAVE_THREAD
	        t->mymutex.unlock();
# endif
	        break;
	    }

	    MicroThread *next = t->pool->NextWork();
	    if( next )
	    {
	        // reinstall debug config for this new thread
	        if ( next->dbgconfig )
	            next->dbgconfig->Install();

	        t->waiton = true;
	        DEBUGPRINT( DEBUG_THREAD, "Switching Start" );
	        // move the thread object to new work
	        t->joined = true;	// because no thread object anymore
# ifdef THREAD_CPP_DEBUG
	        std::cout << "old thread id " << t->mythread.get_id() << std::endl;
# endif
# ifdef HAVE_THREAD
	        next->mythread = std::move( t->mythread );
# endif

# ifdef THREAD_CPP_DEBUG
	        std::cout << "switched thread id " << next->mythread.get_id() << std::endl;
# endif
	    }
	    t->running = false;
# ifdef THREAD_CPP_DEBUG
	    if( !next )
	        std::cout << "final ending thread id " << t->mythread.get_id() << std::endl;
# endif
# ifdef HAVE_THREAD
	    t->mymutex.unlock();
# endif
	    t = next;
	} while( t );
# ifdef THREAD_CPP_DEBUG
	std::cout << "Thread ends " << i << " works id " << std::this_thread::get_id() << std::endl;
	if ( t )
	    std::cout << "ending thread id " << t->mythread.get_id() << std::endl;
# endif

	// End of the thread - SHOULD THERE BE A MIMALLOC CALL HERE???
}

/*
 * Work
 * This virtual call is the body of work to be done in a seporate thread
 */

void
MicroThread::Work()
{
}

/*
 * PostWork
 * This virtual call is made in from the thread which waits for work to finish
 */

void
MicroThread::PostWork()
{
}

void
MicroThreadMutex::GetMutex()
{
# ifdef HAVE_THREAD
	mutex.lock();
# endif
}

void
MicroThreadMutex::ReleaseMutex()
{
# ifdef HAVE_THREAD
	mutex.unlock();
# endif
}

void
MicroThreadPool::ThreadLimit( int n )
{
# ifdef OS_LINUXX86
	 // 32 bit linux runs out of memory with too many threads
	 // so we silently make sure we never have too many.
	if( n > 20 )
	    n = 20;
# endif
	activeLimit = n;
}


void
MicroThreadPool::AddThread( MicroThread * t )
{
	t->SetPool( this );
	poolMutex.GetMutex();
	DEBUGPRINTF( DEBUG_POOL, "add thread limit %d, count %d", activeLimit, active.Count());
	if( activeLimit <= 0 || active.Count() < activeLimit )
	{
	    poolMutex.ReleaseMutex();
	    int failed = t->RunWork();
	    poolMutex.GetMutex();
	    if( failed )
	    {
	        int acount = active.Count();
	        // decrement the thread limit on the pool
	        // to discourage trying to create
	        // more threads, but not if it
	        // would disable ever crerating a
	        // new thread.
	        if ( acount > 1 )
	            activeLimit = acount - 1;
	        waiting.Put( (void *)t );
	    }
	    else
	        active.Put( (void *)t );
	}
	else
	    waiting.Put( (void *)t );
	poolMutex.ReleaseMutex();
}

void
MicroThreadPool::WaitAll( Error *e )
{
	int dbg_assert = DEBUG_ASSERT;
	int dbg_pool = DEBUG_POOL;
	poolMutex.GetMutex();
	while( active.Count() > 0 )
	{
	    DEBUGPRINTF( dbg_pool, "Wait All count %d\n", active.Count() );

	    MicroThread *t = (MicroThread *)active.Get( 0 );

	    if( dbg_assert && ! t->Started() )
	        DEBUGPRINT( dbg_assert, "NOT STARTED THREAD WAIT!" );

	    active.Remove( 0 );

	    DEBUGPRINTF( dbg_pool, "Wait All removed count %d", active.Count() );

	    poolMutex.ReleaseMutex();
	    t->Wait();
	    if( e && t->ErrorObj().Test() )
	        e->Merge( t->ErrorObj(), 1 ); // Skip any dups
	    delete t;
	    StartWaiting();
	    poolMutex.GetMutex();
	}
	poolMutex.ReleaseMutex();
}

void
MicroThreadPool::Reap( Error *e )
{
	DEBUGPRINT( DEBUG_POOL, "in reap" );
	poolMutex.GetMutex();
	for( int i = 0; i < active.Count(); i++ )
	{
	    MicroThread *t = (MicroThread *)active.Get( i );

	    if( !t->Running() )
	    {
	        DEBUGPRINT( DEBUG_POOL, "reap waiting" );
	        active.Remove( i );
	        poolMutex.ReleaseMutex();
	        t->Wait();
	        DEBUGPRINT( DEBUG_POOL, "reap done wait" );
	        if( e )
	            e->Merge( t->ErrorObj() );
	        delete t;
	        i = 0;
	        poolMutex.GetMutex();
	    }
	}
	poolMutex.ReleaseMutex();
	StartWaiting();
	DEBUGPRINT( DEBUG_POOL, "out reap" );
}

bool
MicroThreadPool::AnyActive()
{
	DEBUGPRINT(DEBUG_POOL, "in any active");
	poolMutex.GetMutex();
	bool any = waiting.Count() > 0 || active.Count() > 0;
	poolMutex.ReleaseMutex();
	return any;
}

void
MicroThreadPool::StartWaiting()
{
	poolMutex.GetMutex();
	while( waiting.Count() > 0 &&
	       ( activeLimit <= 0 || active.Count() < activeLimit ) )
	{
	    MicroThread *t = (MicroThread *)waiting.Get( 0 );

	    waiting.Remove( 0 );
	    active.Put( (void *)t );
	    poolMutex.ReleaseMutex();
	    t->RunWork();
	    poolMutex.GetMutex();
	}
	poolMutex.ReleaseMutex();
}

MicroThread *
MicroThreadPool::NextWork()
{
	MicroThread *next = NULL;

	poolMutex.GetMutex();
	if( waiting.Count() > 0 ) {
	    next = (MicroThread *)waiting.Get( 0 );
	    waiting.Remove( 0 );
	    active.Put( (void *)next );

	    next->started = true;
	    next->running = true;
	}
	poolMutex.ReleaseMutex();
	return next;
}

int
MicroThreadPool::GetThreadLimit()
{
	int ret;
	poolMutex.GetMutex();
	ret  = activeLimit;
	poolMutex.ReleaseMutex();
	return ret;
}

// When nthreads is 0, we need to provide a reasonable
// guess as we can't use '0' to divide up the work into chunks.
int
ThreadGuess(int nthreads)
{
	if( nthreads )
	    return nthreads;

	int tnum = 0;

# if defined( HAVE_THREAD ) && defined( HAS_CPP11 )
	tnum = std::thread::hardware_concurrency();
# endif
	if( ! tnum )
	    tnum = 20;
	return tnum;
}
