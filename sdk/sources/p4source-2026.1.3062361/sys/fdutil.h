/*
 * Copyright 1995, 1996 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

extern "C" {

#ifdef OS_NT
int lockFileByHandle( HANDLE h, int flag, int (*cb)(void *) = 0, void *arg = 0 );
#endif
int lockFile( FD_TYPE fd, int flag, int (*cb)(void *) = 0, void *arg = 0 );
FD_TYPE checkFd( FD_TYPE fd );
void checkStdio( FD_TYPE fd = FD_INIT );

}


enum lockFileFlags {

	LOCKF_UN = 0,		// unlock
	LOCKF_SH = 1,		// shared
	LOCKF_EX = 2,		// exclusive
	LOCKF_SH_NB = 3,	// shared (no block)
	LOCKF_EX_NB = 4		// exclusive (no block)

} ;

