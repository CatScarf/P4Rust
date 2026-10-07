/*
 * Copyright 1995, 2020 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# include <stdlib.h>
# include "malloc_override.h"

# ifdef HAS_MALLOC_OVERRIDE

#  ifdef USE_MIMALLOC

# undef mi_malloc
# undef mi_calloc
# undef mi_realloc
# undef mi_free

void *mi_malloc( size_t size ) noexcept
{
	return malloc( size );
}

void *mi_calloc( size_t nmemb, size_t size ) noexcept
{
	return calloc( nmemb, size );
}

void *mi_realloc( void *ptr, size_t size ) noexcept
{
	return realloc( ptr, size );
}

void mi_free( void *ptr ) noexcept
{
	free( ptr );
}

#  endif // USE_MIMALLOC

# ifdef USE_SMARTHEAP
#  if (_MSC_VER >= 1900)

# undef SH_malloc
# undef SH_calloc
# undef SH_realloc
# undef SH_free

void *SH_malloc( size_t size )
{
	return malloc( size );
}

void *SH_calloc( size_t nmemb, size_t size )
{
	return calloc( nmemb, size );
}

void *SH_realloc( void *ptr, size_t size )
{
	return realloc( ptr, size );
}

void SH_free( void *ptr )
{
	free( ptr );
}

#  endif // _MSC_VER < 1900
# endif // USE_SMARTHEAP

# else // HAS_MALLOC_OVERRIDE

# include <stdhdrs.h>
# include <strbuf.h>
# include "malloc_utils.h"

char * MallocUtils::MemMgrVersion( StrBuf *buf )
{
	*buf = "standard library manager";
	return buf->Text();
}

P4INT64 MallocUtils::ThreadPeakMemBytes() { return 0; };
P4INT64 MallocUtils::ThreadCurrentMemBytes() { return 0; };
P4INT64 MallocUtils::CurrentThreadMegaBytes() { return 0; };
P4INT64 MallocUtils::PeakThreadMaxMegaBytes() { return 0; };
P4INT64 MallocUtils::PeakThreadMegaBytes() { return 0; };
void MallocUtils::ResetThreadStatsPeak() {};
void MallocUtils::FreeThreadMemoryCache() {};

# endif
