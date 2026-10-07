/*
 * Copyright 1995, 1996 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# define NEED_GETPID
# define NEED_SIGNAL
# include "stdhdrs.h"
# include "pid.h"

# ifdef OS_NT

int
Pid::GetID()
{
	return GetCurrentThreadId();
}

int
Pid::GetProcID()
{
	return GetCurrentProcessId();
}

int
Pid::CheckID( int id )
{
	DWORD threadId = id;
	HANDLE threadHandle;

	threadHandle = OpenThread( THREAD_QUERY_INFORMATION | SYNCHRONIZE,
	                           0, threadId );

	if( !threadHandle )
	    return 0;

	DWORD retval = WaitForSingleObject(threadHandle, 0);

	CloseHandle( threadHandle );

	if ( retval == WAIT_OBJECT_0 )	// thread has ended
		return 0;

	return 1;
}

# else

int
Pid::GetID()
{
	return getpid();
}

int
Pid::GetProcID()
{
	return GetID();
}

int 
Pid::CheckID( int id )
{
	return( kill( id, 0 ) == 0 );
}

# endif
