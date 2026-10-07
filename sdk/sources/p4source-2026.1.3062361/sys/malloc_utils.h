/*
 * Copyright 1995, 2019 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

class MallocUtils
{
    public:

	static char*	MemMgrVersion( StrBuf *buf );
	static void	PrintInfo();

	// Currently-executing thread peak committed memory.
	// Return units are in bytes.
	static P4INT64	ThreadPeakMemBytes();
	// Same, but returns in unit of megabytes.
	static P4INT64	PeakThreadMegaBytes();

	// Currently-executing thread current committed memory.
	// Return units are in bytes.
	static P4INT64	ThreadCurrentMemBytes();
	// Same, but returns in unit of megabytes.
	static P4INT64	CurrentThreadMegaBytes();

	// Process-long peak (e.g. between separate commands on a connection).
	static P4INT64	PeakThreadMaxMegaBytes();
	// Zero the process-long peak.
	static void	ResetThreadStatsPeak();

	static void	FreeThreadMemoryCache();
  
	static int	Trackable( int level, const P4INT64 mem );

    private:

# ifdef HAS_MIMALLOC
	static void*	GetMimallocStatsForCurrentThread();
# endif
} ;
