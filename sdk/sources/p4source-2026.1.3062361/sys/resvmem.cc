/*
 * Copyright 2001 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# include <stdhdrs.h>
# include <resvmem.h>

# ifdef OS_LINUX

#   include <sys/resource.h>

# endif

/*
 * Set the process stack size 
 *
 * this has only been adaquatly tested under linux, so
 * its only implemented there...
 *
 * Really only works on 64 bit platforms, as the stack
 * on the 32 bit is set to the max allowed...
 *
 * this won't reduce the stack, or set it above the max
 * because this seemed unreliable durring testing.
 */
int SetStackSize( int size )
{

# ifdef OS_LINUX
	if( size )
	{
	    long ssize;
	    struct rlimit limInfo;

	    if( size == 0 )
		return ( -1 );

	    ssize = 1024L * size ;

	    getrlimit( RLIMIT_STACK, &limInfo );

	    if( ssize > limInfo.rlim_max )
		ssize = limInfo.rlim_max;

	    limInfo.rlim_cur = ssize;
	    
	    setrlimit( RLIMIT_STACK, &limInfo );
 	}
# endif
	return ( 0 );
}
