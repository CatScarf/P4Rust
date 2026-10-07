/*
 * Copyright 1995, 2022 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

// Do not define NEED_WINDOWSH, we have to customize _WIN32_WINNT
// before including windows.h

# define NEED_FILE
# define NEED_CHMOD
# define NEED_WIN32FIO

// Must have a newer _WIN32_WINNT definition for this API.
//
# ifdef _WIN32_WINNT
# undef _WIN32_WINNT
# define _WIN32_WINNT 0xA01
# endif
# ifdef WINVER
# undef WINVER
# define WINVER 0xA01
# endif

// Prevent conflict with our rpc.h file.
//
# ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
# endif

# include <windows.h>

# include <stdhdrs.h>

# include <strbuf.h>
# include <debug.h>
# include <i18napi.h>
# include <charcvt.h>

# include <share.h>
# include <mbstring.h>

# include "filesys.h"
# include "fileio.h"
# include "ntthdlist.h"

// Atomic rename is only supported if building with VS2017 or newer.
// Otherwise the nt_atrename function is stubbed to return an error.
// This shold only occur if there is a coding error.

# if (_MSC_VER > 1900)

// Imports from fileiont.cc
//
extern void nt_free_wname(const wchar_t *wname);
extern const wchar_t * nt_wname(StrPtr *fname, int lfn, int *newlen);
extern FD_TYPE nt_open(StrPtr *fname, int flags, int mode, int dounicode, int lfn);
extern int nt_close(FD_TYPE fd);


// This can be found in WinBase.h
// For VS2017
// c:\Program Files (x86)\Windows Kits\10\Include\10.0.17763.0\um
//
#ifdef FOR_REFERENCE
	typedef struct _FILE_RENAME_INFO {
	#if (_WIN32_WINNT >= _WIN32_WINNT_WIN10_RS1)
	    union {
	        BOOLEAN ReplaceIfExists;  // FileRenameInfo
	        DWORD Flags;              // FileRenameInfoEx
	    } DUMMYUNIONNAME;
	#else
	    BOOLEAN ReplaceIfExists;
	#endif
	    HANDLE RootDirectory;
	    DWORD FileNameLength;
	    WCHAR FileName[1];
	} FILE_RENAME_INFO, *PFILE_RENAME_INFO;
#endif


// Implement atomic rename.  This handles both unicode and lfn filenames.
// If needed, the nt_wname call prepends the "//?/".
//
// This can also handle changing the case of the filename without removing
// the target.
//
// For this to be atomic, must use VS2017 and newer, the union as shown
// below and FileRenameInfoEx.  Also Windows 10 or Windows Server 2019.
//
// Return values
//   success =  0
//   failure =  1
int
nt_atrename( StrPtr *srcname, StrPtr *dstname, int dounicode, int lfn )
{
	FD_TYPE srcfd;

	int bits = FileIOBinary::GetModeAflags( FOM_RW );

	// Unicode filenames only here.
	// nt_open() sets the system error.
	srcfd = nt_open( srcname, bits, PERM_0666, dounicode, lfn );
	if( srcfd == FD_ERR )
	    return 1;

	// Wide character version of destination file.
	const wchar_t *wdstname = nt_wname( dstname, lfn, NULL );
	if( !wdstname )
	    return -1;

	// FILE_RENAME_INFO EOS already covered, FileName[1].
	DWORD wdstnameSiz = wcslen(wdstname) * sizeof(wchar_t);
	size_t friSiz = sizeof(FILE_RENAME_INFO) + wdstnameSiz;

	FILE_RENAME_INFO *fri = (FILE_RENAME_INFO *)malloc(friSiz);
	memset( fri, 0, friSiz );

# ifdef NOT_ATOMIC
	// As described by Niall Douglas
	//
	// Not all compilers support the union, VS2015 and newer.
	//
	fri->ReplaceIfExists = TRUE;
# else
	fri->Flags = (FILE_RENAME_FLAG_REPLACE_IF_EXISTS |
	                FILE_RENAME_FLAG_POSIX_SEMANTICS);
# endif

	fri->RootDirectory = NULL;

	fri->FileNameLength = wdstnameSiz;
	memcpy( fri->FileName, wdstname, wdstnameSiz );

	// Return value, 0 on failure, non zero otherwise.
	//
	BOOL bSFIRet = SetFileInformationByHandle(
	                srcfd->fh,
# ifdef NOT_ATOMIC
	                FileRenameInfo,
# else
	                FileRenameInfoEx,
# endif
	                fri,
	                friSiz );

	nt_free_wname( wdstname );

	nt_close( srcfd );

	if( !bSFIRet )
	    return 1;

	return 0;
}

# else

int
nt_atrename( StrPtr *srcname, StrPtr *dstname, int dounicode, int lfn )
{
	return -1;
}

# endif

