/*
 * Copyright 1995, 2000 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

/*
 * NtThreadList - Class supporting Threader::Reap for NT
 *
 * Public methods:
 *
 *	NtThreadList::NtThreadList()
 *	NtThreadList::~NtThreadList()
 *
 *	NtThreadList::AddThread()
 *	NtThreadList::RemoveThread()
 *	NtThreadList::GetThread()
 */

#ifndef NTTHDLIST_H__
#define NTTHDLIST_H__

# if defined( OS_NT ) && defined( HAS_CPP11 )
#include <atomic>
# endif

class NtThreadList
{
    public:

    struct ThreadInfo
    {
# if defined( OS_NT ) && defined( HAS_CPP11 )
	    std::atomic< P4INT64 >	mem;
# endif
    };

		    NtThreadList();
    	virtual	    ~NtThreadList();

	ThreadInfo*	AddThread( void *k, DWORD tid );
	int		RemoveThread( void *k );
	void		SuspendThreads();
	int		Empty();
	int		GetThreadCount();
	P4INT64		TotalMem();

    private:

    struct NtThreadEntry {
	    void                 *key;
	    DWORD                 tid;
	    ThreadInfo            info;
	    struct NtThreadEntry *next;
	    struct NtThreadEntry *prev;
    } ;

	NtThreadEntry		*head;
	CRITICAL_SECTION	section;
	int			listSize;
};

#endif // NTTHDLIST_H__
