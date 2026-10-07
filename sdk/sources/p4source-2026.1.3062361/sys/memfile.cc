/*
 * Copyright 1995, 2020 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# define NEED_WINDOWSH
# define NEED_MMAP
# define NEED_FCNTL
# define NEED_FILE

# include "stdhdrs.h"

# include <strbuf.h>
# include <error.h>

# include "memfile.h"

# ifdef OS_NT

# ifdef OS_NTX86
# define MEMFILE_READ GENERIC_READ|GENERIC_WRITE
# define MEMPAGE_READ PAGE_READWRITE
# define MEMMAP_READ FILE_MAP_WRITE
# else
# define MEMFILE_READ GENERIC_READ
# define MEMPAGE_READ PAGE_READONLY
# define MEMMAP_READ FILE_MAP_READ
# endif

void
MemFile::Open( const char *fn, Error *e )
{
	if( base )
	    Close();
	
	f = CreateFile( fn, MEMFILE_READ,
	                    FILE_SHARE_READ|FILE_SHARE_WRITE,
	                    NULL, OPEN_EXISTING,
	                    FILE_ATTRIBUTE_NORMAL|FILE_FLAG_RANDOM_ACCESS,
	                    NULL );
	
	if( f != INVALID_HANDLE_VALUE )
	{
	    fm = CreateFileMapping( f, NULL, MEMPAGE_READ, 0, 0, NULL );
	    if( fm != INVALID_HANDLE_VALUE )
	    {
	        base = MapViewOfFile( fm, MEMMAP_READ, 0, 0, 0 );
	        if( !base )
	        {
	            e->Sys( "MapViewOfFile", fn );
	            CloseHandle( fm );
	            CloseHandle( f );
	        }
	        else
	            sz = GetFileSize( f, NULL );
	    }
	    else
	    {
	        e->Sys( "CreateFileMapping", fn );
	        CloseHandle( f );
	    }
	}
	else
	    e->Sys( "CreateFile", fn );
}

void
MemFile::Open( const char *fn, unsigned long lsz )
{
	if( base )
	    Close();
	
	f = CreateFile( fn, GENERIC_READ|GENERIC_WRITE,
	                    FILE_SHARE_READ|FILE_SHARE_WRITE, NULL,
	                    OPEN_ALWAYS,
	                    FILE_ATTRIBUTE_NORMAL|FILE_FLAG_RANDOM_ACCESS,
	                    NULL );
	
	if( f != INVALID_HANDLE_VALUE )
	{
	    GetFileSize( f, &sz );
	    if( lsz > sz )
	    {
	        SetFilePointer( f, lsz, 0, FILE_BEGIN );
	        SetEndOfFile( f );
	        sz = lsz;
	    }

	    fm = CreateFileMapping( f, NULL, PAGE_READWRITE, 0, 0, NULL );
	    if( fm != INVALID_HANDLE_VALUE )
	    {
	        base = MapViewOfFile( fm, FILE_MAP_WRITE, 0, 0, 0 );
	        if( !base )
	        {
	            CloseHandle( fm );
	            CloseHandle( f);
	        }
	    }
	    else
	        CloseHandle( f );
	}
}

void
MemFile::Close()
{
	if( base )
	{
	    UnmapViewOfFile( base );
	    CloseHandle( fm );
	    CloseHandle( f );
	    base = 0;
	    sz = 0;
	}
}

# else // OS_NT

void
MemFile::Open( const char *fn, Error *e )
{
	f = ::open( fn, O_RDONLY );
	if( f >= 0 )
	{
	    fcntl( f, F_SETFD, 1 );
	    sz = lseek( f, 0l, SEEK_END );
	    base = mmap( NULL, sz, PROT_READ, MAP_SHARED, f, 0 );
	    if( base == (void *)-1 )
	    {
	        e->Sys( "mmap", fn );
	        base = 0;
	        ::close(f);
	        sz = 0;
	    }
	}
	else
	    e->Sys( "open", fn );
}

void
MemFile::Open( const char *fn, unsigned long lsz )
{
	f = ::open( fn, O_RDWR|O_CREAT, 0666 );
	if( f >= 0 )
	{
	    fcntl( f, F_SETFD, 1 );
	    sz = lseek( f, 0l, SEEK_END );
	    if( lsz > sz )
	    {
	        ftruncate( f, lsz );
	        sz = lsz;
	    }
	    base = mmap( NULL, sz, PROT_READ|PROT_WRITE, MAP_SHARED, f, 0 );
	    if( base == (void *)-1 )
	    {
	        base = 0;
	        ::close(f);
	        sz = 0;
	    }
	}
}

void
MemFile::Close()
{
	if( base )
	{
	    munmap( base, sz );
	    ::close( f );
	    base = 0;
	    sz = 0;
	}
}

# endif // !OS_NT
