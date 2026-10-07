/*
 * Copyright 1995, 1996 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# define NEED_FCNTL
# define NEED_FILE
# define NEED_STAT
# define NEED_UTIME
# define NEED_ERRNO
# define NEED_SLEEP
# define NEED_CHDIR
# define NEED_WIN32FIO

# include <stdhdrs.h>

# include <error.h>
# include <errornum.h>
# include <errorlog.h>
# include <strbuf.h>
# include <strdict.h>
# include <strtable.h>
# include <strarray.h>
# include <debug.h>
# include <tunable.h>
# include <md5.h>
# include <datetime.h>
# include <charset.h>
# include <i18napi.h>
# include <charcvt.h>
# include <fdutil.h>
# include <largefile.h>
# include <msgos.h>

# include <share.h>
# include <mbstring.h>

# include "hostenv.h"
# include "filesys.h"
# include "pathsys.h"
# include "fileio.h"

# include "pressuremonitor.h"
# include "sysinfo.h"

extern int global_umask;

# define utimbufL _utimbuf

# define DOUNICODE	( CharSetApi::isUnicode((CharSetApi::CharSet)GetCharSetPriv()) )

extern int nt_atrename( StrPtr *, StrPtr *, int, int );

#define LF 10           // line feed
#define CR 13           // carriage return
#define CTRLZ 26        // ctrl-z means eof for text

#define FH_ERROR		-1
#define FH_CONTINUE		0
#define FH_DELETE_PENDING	1
#define FH_READ_ONLY		2

// The REPARSE_DATA_BUFFER is part of the "Windows Driver Kit" according to
// the MSDN docs, so for the time being we just copy the structure here:
//
// For MinGW builds, the mingw x86 version grouped the DDK into the
// winnt.h header.  The newer mingw-w64 is more like Visual Studio
// in that you must include ddk/ntifs.h for the reparse structure.
// We defined the reparse structure only for OS_NT and OS_MINGW64.
//
# if defined( OS_MINGW64 ) == defined( OS_MINGW )
typedef struct _REPARSE_DATA_BUFFER {
	ULONG  ReparseTag;
	USHORT ReparseDataLength;
	USHORT Reserved;
	union {
		// if ReparseTag == IO_REPARSE_TAG_SYMLINK:
		struct {
			USHORT SubstituteNameOffset;
			USHORT SubstituteNameLength;
			USHORT PrintNameOffset;
			USHORT PrintNameLength;
			ULONG  Flags;
			WCHAR  PathBuffer[1];
		} SymbolicLinkReparseBuffer;

		// if ReparseTag == IO_REPARSE_TAG_MOUNT_POINT:
		struct {
			USHORT SubstituteNameOffset;
			USHORT SubstituteNameLength;
			USHORT PrintNameOffset;
			USHORT PrintNameLength;
			WCHAR  PathBuffer[1];
		} MountPointReparseBuffer;
		
		struct {
			UCHAR DataBuffer[1];
		} GenericReparseBuffer;
	} ;
} REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;
# endif

// We also include a handful of relevant magic constants from the device
// driver development kit here:
//
#ifndef FSCTL_GET_REPARSE_POINT
// define FSCTL_GET_REPARSE_POINT     CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 42, METHOD_BUFFERED, FILE_ANY_ACCESS)
# define FSCTL_GET_REPARSE_POINT 0x000900A8
# endif
# ifndef IO_REPARSE_TAG_SYMLINK
# define IO_REPARSE_TAG_SYMLINK 0xA000000CL
# endif
# ifndef S_ISDIR
# define S_ISDIR(m) (((m)&S_IFMT)==S_IFDIR)
# endif
# ifndef SYMBOLIC_LINK_FLAG_DIRECTORY 
# define SYMBOLIC_LINK_FLAG_DIRECTORY 1
# endif
# ifndef MAXIMUM_REPARSE_DATA_BUFFER_SIZE
# define MAXIMUM_REPARSE_DATA_BUFFER_SIZE ( 16 * 1024 )
# endif

typedef BOOLEAN (WINAPI *CreateSymbolicLinkAProc)(LPCSTR,LPCSTR,DWORD);
typedef BOOLEAN (WINAPI *CreateSymbolicLinkWProc)(LPCWSTR,LPCWSTR,DWORD);

static CreateSymbolicLinkAProc CreateSymbolicLinkA_func = 0;
static CreateSymbolicLinkWProc CreateSymbolicLinkW_func = 0;
static int functionHandlesLoaded = 0;

#ifndef SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE
#define SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE    (0x2)
#endif

// Handle the Unicode and LFN file name translation.
// Caller to nt_wname() must free the memory through nt_free_wname().
//
void
nt_free_wname( const wchar_t *wname )
{
	delete [] (char *)wname;
}

// This function is used for both Unicode mode and Long File Name support.
// If this function is called, it is assumed we have Unicode mode, or LFN
// or both, otherwise this function will not be called.
//
// If Unicode mode, we are converting from UTF8 to UNICODE.
// If LFN, we are converting from ANSI to UNICODE.
// If Unicode mode and LFN, we are converting from UTF8 to UNICODE.
//
const wchar_t *
nt_wname( StrPtr *fname, int lfn, int *newlen )
{
	wchar_t *wname;
	int len = 0;
	StrBuf lfname;
	const char *filename;
	int fnamelen;

	// We want one of these two long filename forms.
	//   non-UNC  -> \\?\c:\path
	//   UNC      -> \\?\UNC\host\share\path
	// Derive UNC-ness from the actual path, not from the lfn flags,
	// because rename ORs the flags of two FileSys objects and a
	// stale LFN_UNCPATH bit from a UNC peer would corrupt a plain
	// drive-letter path into "\\?\UNC<drive-path>".
	//
	if( lfn & LFN_ENABLED )
	{
	    // The caller may pass a device-prefixed path ("\\?\" or
	    // "\\.\").  Prefixing it again would corrupt it (e.g.
	    // "\\?\UNC\?\c:\path" for a misclassified drive path), so take
	    // it as is; the separator normalization below still applies.
	    int extended = FileSys::IsExtended( *fname );
	    int isunc = !extended && FileSys::IsUNC( *fname );

	    if( extended )
		lfname.Set( *fname );
	    else if( isunc )
		lfname.Set( "\\\\?\\UNC" );
	    else
		lfname.Set( "\\\\?\\" );

	    // LFN requires a full pathname.
	    if( !extended && FileSys::IsRelative( *fname ) )
	    {
		int cs = GlobalCharSet::Get();
		StrBuf cwd;
		HostEnv::GetCwdbyCS( cwd, cs );

		PathSys *p = PathSys::Create();
		p->SetCharSet( cs );
		p->SetLocal( cwd, *fname );
		lfname.Append( p->Text() );
		delete p;
	    }
	    else if( !extended )
	    {
		// In the UNC case, fname will have two leading back slashes.
		// Use an offset to remove one leading slash.
		// \\?\UNC\\host\share\path -> \\?\UNC\host\share\path
		if( isunc )
		    lfname.Append( &(fname->Text()[1]) );
		else
		    lfname.Append( fname->Text() );
	    }

	    // The LFN escape bypasses the Win32 API nicety checks, force '\'.
	    for(int i=0 ; i < lfname.Length(); ++i )
	    {
		if( lfname.Text()[i] == '/' )
		    lfname.Text()[i] = '\\';
	    }

	    // LFN adjustments to fname.
	    filename = lfname.Text();
	    fnamelen = lfname.Length();
	}
	else
	{
	    // Pass incoming fname through.
	    filename = fname->Text();
	    fnamelen = fname->Length();
	}

	if( lfn & LFN_UTF8 )
	{
	    CharSetCvtUTF816 cvt;

	    // This is converting from UTF8 to UNICODE.
	    //
	    wname = (wchar_t *)cvt.CvtBuffer( filename, fnamelen, &len );

	    // No error structure, instead return a NULL.
	    if ( cvt.LastErr() != CharSetCvt::NONE )
	    {
		if( wname )
		    nt_free_wname( wname );
		return NULL;
	    }
	}
	else
	{
	    // This is converting from ANSI to UNICODE.
	    //
	    // first determine the buffer size needed for conversion
	    //
	    len = MultiByteToWideChar (
				CP_ACP,
				0,	// Use default flags
				filename,
				-1,	// filename is null terminated
				0,
				0 );

	    if( len == 0 )
	    {
		// Report an error?
		return NULL;
	    }

	    wname = (wchar_t *)new char[(len+1)*sizeof(wchar_t)];

	    // perform the actual conversion using the active code page
	    //
	    len = MultiByteToWideChar (
				CP_ACP,
				0,	// Use default flags
				filename,
				-1,	// filename is null terminated
				wname,
				len );

	    // No error structure, instead return a NULL.
	    if( len == 0 )
	    {
		if( wname )
		    nt_free_wname( wname );
		return NULL;
	    }
	}

	if( newlen != NULL )
	    *newlen = len;

	return wname;
}

// Determine if the handle is pointing to a file which has been made
// read only or is in a delete pending state.
//
// Return values,
//	FH_ERROR         -1 - something went wrong.
//	FH_CONTINUE       0 - file operation can continue.
//	FH_DELETE_PENDING 1 - file was unlinked, now in delete pending.
//	FH_READ_ONLY      2 - file is now read only.
//
int
nt_check_fh (
	HANDLE fh
	)
{
	FILE_STANDARD_INFO fsi;

	// Test for delete pending.
	//
	memset( &fsi, 0, sizeof(fsi) );
	BOOL bResult = GetFileInformationByHandleEx(
	                        fh,
	                        FileStandardInfo,
	                        &fsi,
	                        sizeof(fsi) );

	if( ! bResult )
	    return FH_ERROR;

	if( fsi.DeletePending )
	    return FH_DELETE_PENDING;

	FILE_BASIC_INFO fbi;

	memset( &fbi, 0, sizeof(fbi) );
	bResult = GetFileInformationByHandleEx(
	                        fh,
	                        FileBasicInfo,
	                        &fbi,
	                        sizeof(fbi) );

	if( ! bResult )
	    return FH_ERROR;

	if( fbi.FileAttributes & FILE_ATTRIBUTE_READONLY )
	    return FH_READ_ONLY;

	return FH_CONTINUE;
}

time_t
nt_convtime( SYSTEMTIME *systime )
{
	struct tm u_tm;
	time_t t;

	// Do the converstion twice.  First time gets the TZ from
	// the systime.  Second time we have the correct TZ to
	// produce the correct time_t.
	//

	u_tm.tm_sec   = systime->wSecond;
	u_tm.tm_min   = systime->wMinute; u_tm.tm_hour  = systime->wHour;
	u_tm.tm_mday  = systime->wDay;
	u_tm.tm_mon   = systime->wMonth - 1;
	u_tm.tm_year  = systime->wYear - 1900;
	u_tm.tm_wday  = 0;
	u_tm.tm_yday  = 0;
	u_tm.tm_isdst = 0;

	t = mktime( &u_tm );

	u_tm.tm_sec   = systime->wSecond;
	u_tm.tm_min   = systime->wMinute;
	u_tm.tm_hour  = systime->wHour;
	u_tm.tm_mday  = systime->wDay;
	u_tm.tm_mon   = systime->wMonth - 1;
	u_tm.tm_year  = systime->wYear - 1900;
	u_tm.tm_wday  = 0;
	u_tm.tm_yday  = 0;

	t = mktime( &u_tm );

	return t;
}

// This function is only for LFN support.
// This function does not handle wild cards, neither does the MS version.
// We do not fabricate c:/ or //host/share/
// This only handles absolute path names with a drive spec.
// Mostly taken from VS vc/crt/src/stat.c
//
// Limit this function to the VS2013 compiler.
//
int
nt_wstati64( const wchar_t *wname, struct statbL *sb )
{
# if (_MSC_VER >= 1800)
	HANDLE findhandle;
	WIN32_FIND_DATAW findbuf;

	errno = 0;

	findhandle = FindFirstFileExW (
			wname,
			FindExInfoStandard,
			&findbuf,
			FindExSearchNameMatch,
			NULL, 0);

	// File does not exist.
	if( findhandle == INVALID_HANDLE_VALUE )
	{
	    errno = ENOENT;
	    return -1;
	}

	FindClose( findhandle );

	if( (findbuf.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) &&
	    (findbuf.dwReserved0 == IO_REPARSE_TAG_SYMLINK) )
	{
	    int fd = -1;
	    errno_t e;
	    int oflag = _O_RDONLY;
	    int ret=0;

	    if( findbuf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		oflag |= _O_OBTAIN_DIR;

	    e = _wsopen_s( &fd, wname, oflag, _SH_DENYNO, 0 );
	    if( e != 0 || fd == -1 )
		return -1;

	    ret = _fstati64( fd, sb );

	    close( fd );
	    return ret;
	}

	// Sort out file times.
	//
	SYSTEMTIME SysTime;

	// Range testing on SystemTimeToTzSpecificLocalTime()
	// Lowest date which will convert correctly
	//   quad=0x430e234000, highpart=0x43, lowpart=0xe234000
	//   quad=288000000000, highpart=67, lowpart=237191168
	// Highest date which will convert correctly
	//   quad=0x7fff35f4f06c8000, highpart=0x7fff35f4, lowpart=0xf06c8000
	//   quad=9223149888000000000, highpart=2147431924, lowpart=4033642496
	//
	// Range testing on FileTimeToSystemTime()
	//   Lowest is quad=0
	//   Highest is quad=0x8000000000000000
	//
	// Both failure conditions return this error,
	//   WinAPI - ERROR_INVALID_PARAMETER
	//   CRT - EINVAL

	if( findbuf.ftLastWriteTime.dwLowDateTime ||
	    findbuf.ftLastWriteTime.dwHighDateTime )
	{
	    if( !FileTimeToSystemTime( &findbuf.ftLastWriteTime, &SysTime ) ||
		!SystemTimeToTzSpecificLocalTime( NULL, &SysTime, &SysTime ) )
	    {
		errno = EINVAL;
		return -1;
	    }
	    sb->st_mtime = nt_convtime( &SysTime );
	}
	if( findbuf.ftLastAccessTime.dwLowDateTime ||
	    findbuf.ftLastAccessTime.dwHighDateTime )
        {
	    if( !FileTimeToSystemTime( &findbuf.ftLastAccessTime, &SysTime ) ||
		!SystemTimeToTzSpecificLocalTime( NULL, &SysTime, &SysTime ) )
	    {
		errno = EINVAL;
		return -1;
	    }
	    sb->st_atime = nt_convtime( &SysTime );
	}
	if( findbuf.ftCreationTime.dwLowDateTime ||
	    findbuf.ftCreationTime.dwHighDateTime )
        {
	    if( !FileTimeToSystemTime( &findbuf.ftCreationTime, &SysTime ) ||
		!SystemTimeToTzSpecificLocalTime( NULL, &SysTime, &SysTime ) )
	    {
		errno = EINVAL;
		return -1;
	    }
	    sb->st_ctime = nt_convtime( &SysTime );
	}

	// A=0, B=1, etc.
	//
	const wchar_t *p;
	if( p = wcschr(wname, L':') )
	    sb->st_rdev = sb->st_dev = (_dev_t)(_mbctolower(*--p) - 0x61);
	else
	    sb->st_rdev = sb->st_dev = 0;

	// Sort out the Unix style file modes.
	//
	unsigned short uxmode = 0;

	// Watch out, a directory can have FILE_ATTRIBUTE_ARCHIVE set.
	//
	if( findbuf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
	    uxmode |= _S_IFDIR|_S_IEXEC;
	else
	if( findbuf.dwFileAttributes & FILE_ATTRIBUTE_NORMAL ||
		findbuf.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE )
	{
	    uxmode |= _S_IFREG;
	}

	if( findbuf.dwFileAttributes & FILE_ATTRIBUTE_READONLY )
	    uxmode |= _S_IREAD;
	else
	    uxmode |= _S_IREAD|_S_IWRITE;

	// The correct way to determine if a file is executable is to
	// use Access Control Lists, ACLs.  This is nasty stuff and you
	// can dive in a bit by using the icacls command line tool.
	// Apparently Cygwin does a better job with ACLs, hence job032715.
	// The code below is basically a MS hack to simulate S_IEXEC.
	//
	if( p = wcsrchr(wname, L'.') )
	{
	    if( _wcsicmp(p, L".exe") == 0 ||
		_wcsicmp(p, L".cmd") == 0 ||
		_wcsicmp(p, L".bat") == 0 ||
		_wcsicmp(p, L".com") == 0 )
		    uxmode |= _S_IEXEC;
	}
	uxmode |= (uxmode & 0700) >> 3;
	uxmode |= (uxmode & 0700) >> 6;

	sb->st_mode = uxmode;

	// You can use GetFileInformationByHandle() to get the hardlink
	// count.  We don't have a file handle here.  We do the same
	// thing as MS, just set st_nlink to 1.
	//
	sb->st_nlink = 1;

	// 64bit file size.
	//
	sb->st_size = ((__int64)(findbuf.nFileSizeHigh)) * (0x100000000i64) +
                    (__int64)(findbuf.nFileSizeLow);

	// Windows doesn't really have a uid or gid.  Using ACLs it is
	// possible to come up with these numbers.  Although they will not
	// be in the ranges as you have on Unix.  So we do the smae thing
	// as MS, and assign them to 0.  You can get a file ID by using
	// GetFileInformationByHandle(), it is a 64bit value.
	//
	sb->st_uid = sb->st_gid = sb->st_ino = 0;

	return 0;
# else
	return -1;
# endif
}

int
ntw_islink( StrPtr *fname, DWORD *dwFlags, int lfn )
{
	DWORD fileAttributes;
	const wchar_t *wname;

	wname = nt_wname( fname, lfn, NULL );
	if( !wname )
	    return -1;

	fileAttributes = GetFileAttributesW( wname );
	if( fileAttributes == INVALID_FILE_ATTRIBUTES )
	{
	    nt_free_wname( wname );
	    return -1;
	}

	if( !(fileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) )
	{
	    nt_free_wname( wname );
	    return 0;
	}

	if( dwFlags )
	{
	    *dwFlags = 0;
	    if( fileAttributes & FILE_ATTRIBUTE_DIRECTORY )
	        *dwFlags = SYMBOLIC_LINK_FLAG_DIRECTORY;
	}


	HANDLE fH;
	WIN32_FIND_DATAW findFileDataW;

	fH = FindFirstFileW( wname, &findFileDataW );
	nt_free_wname( wname );

	if( fH == INVALID_HANDLE_VALUE )
	    return -1;
	FindClose( fH );
	if( findFileDataW.dwReserved0 == IO_REPARSE_TAG_SYMLINK ||
	    findFileDataW.dwReserved0 == IO_REPARSE_TAG_MOUNT_POINT )
	        return 1;

	return 0;
}

// Atomic rename is same device only.  If the devices differ
// fall back to the old copy truncate code.
//
int
nt_volumechk( char *src, char *dst )
{
	char src_vol = '\0';
	char dst_vol = '\0';
	char cwd_vol = '\0';

	// For now we do not support atomic rename on UNC paths.
	//
	// Yuck, both / and \ work for UNC paths.
	//
	if( src[0] && (
	    ( src[0] == '\\' && src[1] == '\\' ) ||
	    ( src[0] == '/'  && src[1] == '/' ) ) )
	    return 0;
	if( dst[0] && (
	    ( dst[0] == '\\' && dst[1] == '\\' ) ||
	    ( dst[0] == '/'  && dst[1] == '/' ) ) )
	    return 0;

	if( src[0] && src[1] == ':' )
	    src_vol = src[0];
	if( dst[0] && dst[1] == ':' )
	    dst_vol = dst[0];

	// Both on same volume.
	//
	if( src_vol == '\0' && dst_vol == '\0' )
	    return 1;

	// Both still on same volume.
	//
	if( src_vol != '\0' && dst_vol != '\0' && src_vol == dst_vol )
	    return 1;

	// We need more information, use CWD.
	//
	int cs = GlobalCharSet::Get();
	StrBuf cwd;
	HostEnv::GetCwdbyCS( cwd, cs );

	char *cwdstr = cwd.Text();

	// We assume cwd_vol will always be able to be set, otherwise we 
	// have larger problems.  CWD is bad.
	//
	if( cwdstr[0] && cwdstr[1] == ':' )
	    cwd_vol = cwdstr[0];

	// Use cwd_vol in place of src_vol.
	//
	if( src_vol == '\0' )
	{
	    if( dst_vol == cwd_vol )
	        return 1;
	    else
	        return 0;
	}
	// Use cwd_vol in place of dst_vol.
	//
	if( dst_vol == '\0' )
	{
	    if( src_vol == cwd_vol )
	        return 1;
	    else
	        return 0;
	}

	return 0;
}

int
nt_islink( StrPtr *fname, DWORD *dwFlags, int dounicode, int lfn )
{
	// Allow unicode to fall through.
	if( dounicode || lfn )
	{
	    int ret;
	    if( (ret = ntw_islink( fname, dwFlags, lfn )) >= 0 ||
	        lfn & LFN_ENABLED )
	            return ret;
	}

	DWORD fileAttributes = GetFileAttributes( fname->Text() );
	if( fileAttributes == INVALID_FILE_ATTRIBUTES )
	    return -1;

	if( dwFlags )
	    if( fileAttributes & FILE_ATTRIBUTE_DIRECTORY )
	        *dwFlags = SYMBOLIC_LINK_FLAG_DIRECTORY;

	if( fileAttributes & FILE_ATTRIBUTE_REPARSE_POINT )
	{
	    WIN32_FIND_DATA findFileData;
	    HANDLE fH = FindFirstFile( fname->Text(), &findFileData );
	    if( fH == INVALID_HANDLE_VALUE )
	        return -1;
	    FindClose( fH );
	    if( findFileData.dwReserved0 == IO_REPARSE_TAG_SYMLINK     ||
	        findFileData.dwReserved0 == IO_REPARSE_TAG_MOUNT_POINT )
	        return 1;
	}
	return 0;
}

// Open the file in Unicode mode, hand control back to nt_readlink().
// Return the file handle.
//
HANDLE
ntw_readlink( StrPtr *name, StrBuf &targetBuf, int lfn )
{
	HANDLE fH;
	const wchar_t *wname;

	wname = nt_wname( name, lfn, NULL );
	if( !wname )
	    return INVALID_HANDLE_VALUE;

	fH = CreateFileW( wname,
	                    GENERIC_READ, FILE_SHARE_READ,
	                    0, OPEN_EXISTING,
	                    (FILE_FLAG_BACKUP_SEMANTICS|
	                        FILE_FLAG_OPEN_REPARSE_POINT),
	                    0);
	nt_free_wname( wname );

	return fH;
}

// Reads what the symlink points to, puts the data into targetBuf.
// Returns the number of bytes read.
//
int
nt_readlink( StrPtr *name, StrBuf &targetBuf, int dounicode, int lfn )
{
	HANDLE fH = INVALID_HANDLE_VALUE;

	// Allow unicode to fall through.
	if( dounicode || lfn )
	{
	    fH = ntw_readlink( name, targetBuf, lfn );
	    if( fH == INVALID_HANDLE_VALUE && lfn & LFN_ENABLED )
	        return -1;
	}
	if( fH == INVALID_HANDLE_VALUE )
	{
	    fH = CreateFile( name->Text(), GENERIC_READ, FILE_SHARE_READ,
	        0, OPEN_EXISTING,
	        (FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OPEN_REPARSE_POINT), 0);
	}
	if( fH == INVALID_HANDLE_VALUE )
	    return -1;

	// If the extra memory allocated at the end of REPARSE_DATA_BUFFER
	// is not large enough, the code ERROR_MORE_DATA is returned.
	//
	// The MS docs for DeviceIoControl() indicate that when the
	// error code ERROR_MORE_DATA is returned, "Your application
	// should call DeviceIoControl again with the same operation,
	// specifying a new starting point".
	//
	// MS confirmed that this comment is not valid when using
	// DeviceIoControl() for collecting the symlink target.  The
	// extra memory must be enough for the symlink target.  Also
	// DeviceIoControl() will not tell you the required buffer size.
	// MS admits that the error code ERROR_INSUFFICIENT_BUFFER
	// would have been a better return code.
	//
	// So we allocate the maximum allowed extra memory at the end
	// of the REPARSE_DATA_BUFFER, 16k.  This equates to a maximum
	// of 4096 in length for a Windows symlink target.  Testing
	// symlink creation with a mixture of targets confirms this.
	// 
	REPARSE_DATA_BUFFER *reparseBuffer;
	// The REPARSE_DATA_BUFFER size and room for the symlink target.
	DWORD struct_siz = sizeof(REPARSE_DATA_BUFFER) +
	                        MAXIMUM_REPARSE_DATA_BUFFER_SIZE;
	reparseBuffer = (REPARSE_DATA_BUFFER *) malloc( struct_siz );
	reparseBuffer->ReparseDataLength = MAXIMUM_REPARSE_DATA_BUFFER_SIZE;
	DWORD returnedLength = 0;
	DWORD result = DeviceIoControl( fH, FSCTL_GET_REPARSE_POINT, 0, 0,
	                reparseBuffer, struct_siz, &returnedLength, 0 );
	CloseHandle( fH );
	if( !result )
	{
	    free( reparseBuffer );
	    return -1;
	}

	int len, off;
	WCHAR *wp;

	// This is low-level device driver and file system filter data
	// structures, so we tread gently. By observation, the substitute
	// name and the print name are similar, but the substitute name,
	// particularly for junctions, seems to often point to the so-called
	// "non-parsed string", which starts "\??\". I haven't found any
	// docs about that magic string prefix, and have been successfully
	// using the PrintName representation instead.
	//
	if( reparseBuffer->ReparseTag == IO_REPARSE_TAG_SYMLINK )
	{
	    len = reparseBuffer->SymbolicLinkReparseBuffer.PrintNameLength;
	    off = reparseBuffer->SymbolicLinkReparseBuffer.PrintNameOffset;
	    wp  = reparseBuffer->SymbolicLinkReparseBuffer.PathBuffer;
	}
	else if( reparseBuffer->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT )
	{
	    len = reparseBuffer->MountPointReparseBuffer.PrintNameLength;
	    off = reparseBuffer->MountPointReparseBuffer.PrintNameOffset;
	    wp  = reparseBuffer->MountPointReparseBuffer.PathBuffer;
	}
	else
	{
	    free( reparseBuffer );
	    return -1;
	}

	len = len / sizeof(WCHAR);
	off = off / sizeof(WCHAR);
	wp += off;

	int retlen = len;
	targetBuf.Alloc( len );
	char *o = targetBuf.Text();
	while( len-- )
	{
	    char c = *wp++;
	    // Use forward slashes, storing in Unix format.
	    *o++ = c == '\\' ? '/' : c;
	}
	*o = 0;
	targetBuf.SetLength();
	free( reparseBuffer );
	return retlen;
}

FD_TYPE
nt_getStdHandle( int std_desc, int flags )
{
	HANDLE std_fh;

	if( std_desc < 0 || std_desc > 2 )
	    return FD_ERR;

	std_fh = (HANDLE)_get_osfhandle( std_desc );

	// If we are a Windows Service, we want to error on a bad handle.

	if( std_fh == INVALID_HANDLE_VALUE2 )
	    return FD_ERR;

	FD_TYPE fd = new struct P4_FD;

	fd->flags = flags;
	fd->mode = 0;
	fd->lock = 0;
	fd->dounicode = 0;
	fd->fdFlags = FD_IsSTD;
	fd->fh = std_fh;
	fd->ptr = NULL;
	fd->rcv = 0;
	if( fd->flags & O_BINARY )
	{
	    // Binary reads and writes go directly into caller's buffer.

	    fd->iobuf_siz = 0;
	    fd->iobuf = NULL;
	}
	else
	{
	    // Internal buffers for CR LF translations.

	    fd->iobuf_siz = FileSys::BufferSize() * 2;
	    fd->iobuf = new unsigned char [fd->iobuf_siz];
	}

	return fd;
}

// Perform the ansi or ascii conversion to unicode and use the wide
// character Win32 APIs from the very start.
//
// We are not supporting _O_*TEXT* modes or special devices.
//
HANDLE
ntw_open( StrPtr *fname, int flags, int mode, int dounicode, int lfn )
{
	// Process the Posix flags into Win32 native actions.
	// (Modeling our code off of open.c in VS vc/crt/src.)
	//
	HANDLE osfh;                    /* OS handle of opened file */
	DWORD fileaccess;               /* OS file access (requested) */
	DWORD fileshare;                /* OS file sharing mode */
	DWORD filecreate = 0;           /* OS method of opening/creating */
	DWORD fileattribflags;          /* OS file attributes and flags */
	SECURITY_ATTRIBUTES SecurityAttributes;
	int newlen = 0;
	const wchar_t *wname;

	// UTF8 is set into the lfn flag.
	//
	wname = nt_wname( fname, lfn, &newlen );
	if( !wname )
	    return INVALID_HANDLE_VALUE;

	// Length check for unicode.
	// If LFN and Unicode are always set, this can be removed.
	if( !(lfn & LFN_ENABLED) && newlen > ( MAX_PATH * 2 ) )
	{
	    nt_free_wname( wname );
	    SetLastError( ERROR_BUFFER_OVERFLOW );
	    return INVALID_HANDLE_VALUE;
	}

	// Establish the default security attributes.
	//
	SecurityAttributes.nLength = sizeof( SecurityAttributes );
	SecurityAttributes.lpSecurityDescriptor = NULL;

	// All files on Windows are set to no inherit.
	//
	SecurityAttributes.bInheritHandle = FALSE;

	// Process Posix flags into file access for CreateFile.
	//
	switch( flags & (_O_RDONLY | _O_WRONLY | _O_RDWR) )
	{
	    case _O_RDONLY:	// read access
	        fileaccess = GENERIC_READ;
	        break;

	    case _O_WRONLY:	// write access
	        if (flags & _O_APPEND)
	        {
	            // Read access is added so that a Ctrl-Z EOF might
	            // be removed later.
	            fileaccess = GENERIC_READ | GENERIC_WRITE;
	        }
	        else
	            fileaccess = GENERIC_WRITE;
	        break;

	    case _O_RDWR:	// read and write access
	        fileaccess = GENERIC_READ | GENERIC_WRITE;
	        break;

	    default:		// error, bad flags
	        SetLastError( ERROR_BAD_ARGUMENTS );
	        return INVALID_HANDLE_VALUE;

	}

	// We always use a mode which shares the file.
	// Also Posix defaults to _SH_DENYNO.
	//
	fileshare = FILE_SHARE_READ | FILE_SHARE_WRITE;

	// Allow delete and rename on an open file for Windows.
	// The delete or rename is only allowed once.
	//
	if( lfn & LFN_ATOMIC_RENAME )
	{
	    fileshare |= FILE_SHARE_DELETE;
	    if( fileaccess & GENERIC_WRITE )
	        fileaccess |= DELETE;
	}

	// Process Posix open/create method into CreateFile disposition.
	//
	switch( flags & (_O_CREAT | _O_EXCL | _O_TRUNC) ) {
	    case 0:
	    case _O_EXCL:		// ignore EXCL w/o CREAT
	        filecreate = OPEN_EXISTING;
	        break;

	    case _O_CREAT:
	        filecreate = OPEN_ALWAYS;
	        break;

	    case _O_CREAT | _O_EXCL:
	    case _O_CREAT | _O_TRUNC | _O_EXCL:
	        filecreate = CREATE_NEW;
	        break;

	    case _O_TRUNC:
	    case _O_TRUNC | _O_EXCL:	// ignore EXCL w/o CREAT
	        filecreate = TRUNCATE_EXISTING;
	        break;

	    case _O_CREAT | _O_TRUNC:
	        filecreate = CREATE_ALWAYS;
	        break;

	    default:		// error, bad flags
	        SetLastError( ERROR_BAD_ARGUMENTS );
	        return INVALID_HANDLE_VALUE;
        }

	// Process Posix attributes and flags into CreateFile attributes.
	//
	// default
	fileattribflags = FILE_ATTRIBUTE_NORMAL;

	if( flags & _O_CREAT )
	{
	    if ( !((mode & ~global_umask) & _S_IWRITE) )
	        fileattribflags = FILE_ATTRIBUTE_READONLY;
	}

# ifdef ALLOW_FILENAMES_TO_BE_CASE_SENSITIVE

	// This Posix flag enables CreateFileW to open a file in case
	// sensitive mode.  It is also necessary to set a Kernel registry
	// value to enable NTFS case sensitivity.  A reboot is required.
	// HKLM/SYSTEM/CurrentControlSet/Control/Kernel
	//    obcasesensitive = 0
	// The docs say FILE_ATTRIBUTE_NORMAL is used by itself.  A
	// unit test has shown this is not the case.
	//
	if( lfn & LFN_CSENSITIVE )
	    fileattribflags |= FILE_FLAG_POSIX_SEMANTICS;

# endif

	// Set temporary file (delete-on-close) attribute if requested.
	//
	if( flags & _O_TEMPORARY )
	{
	    fileattribflags |= FILE_FLAG_DELETE_ON_CLOSE;
	    fileaccess |= DELETE;
	    fileshare |= FILE_SHARE_DELETE;
	}

	// Set temporary file (delay-flush-to-disk) attribute if requested.
	//
	if( flags & _O_SHORT_LIVED )
	    fileattribflags |= FILE_ATTRIBUTE_TEMPORARY;
	
# if (_MSC_VER >= 1800)
	// Set directory access attribute if requested.
	//
	if( flags & _O_OBTAIN_DIR )
	    fileattribflags |= FILE_FLAG_BACKUP_SEMANTICS;
# endif

	// Set sequential or random access attribute if requested.
	//
	if( flags & _O_SEQUENTIAL )
	    fileattribflags |= FILE_FLAG_SEQUENTIAL_SCAN;
	else if( flags & _O_RANDOM )
	    fileattribflags |= FILE_FLAG_RANDOM_ACCESS;

	osfh = CreateFileW (
	                wname,
	                fileaccess,
	                fileshare,
	                &SecurityAttributes,
	                filecreate,
	                fileattribflags,
	                NULL ) ;


	// The only file we open with shared delete is the journal file with
	// filesys.atomic.rename set to 1. If the journal file is concurrently
	// opened without shared delete, a sharing violation will occur.  If
	// the journal is rotated with atomic rename, any open handles will
	// follow the rename to the file journal.N.  If the journal.N file is
	// opened without shared delete, a sharing violation will occur.  This
	// second instance occurred in case 810393.
	//
	// If we have a sharing violation, we assume the file open was attempted
	// without shared delete and another handle already has this file open
	// with shared delete.  Add the shared delete flag and try the open
	// again.
	//
	if( osfh == INVALID_HANDLE_VALUE &&
	    GetLastError() == ERROR_SHARING_VIOLATION )
	{
	    SetLastError( ERROR_SUCCESS );

	    fileshare |= FILE_SHARE_DELETE;
	    if( fileaccess & GENERIC_WRITE )
	        fileaccess |= DELETE;

	    osfh = CreateFileW (
	                    wname,
	                    fileaccess,
	                    fileshare,
	                    &SecurityAttributes,
	                    filecreate,
	                    fileattribflags,
	                    NULL ) ;
	}

	nt_free_wname( wname );

	if( osfh == INVALID_HANDLE_VALUE )
	    return INVALID_HANDLE_VALUE;

	// Binary files don't need any legacy EOF handling.

	if( flags & _O_BINARY )
	    return osfh;

	// _O_TEXT only, from here on out.
	// In code above, if O_APPEND, GENERIC_READ is added.
	// This will cover the case of text and append needing to
	// remove the Ctrl-Z EOF.
	//
	// When reading a text file, we also read into the buffer the
	// EOF Ctrl-Z, with the idea we are maintaining the file's content.

	if( (fileaccess & GENERIC_READ) && (fileaccess & GENERIC_WRITE) )
	{
	    // MS: We have a text mode file.  If it ends in CTRL-Z, we wish to
	    // remove the CTRL-Z character, so that appending will work.
	    // We do this by seeking to the end of file, reading the last
	    // byte, and shortening the file if it is a CTRL-Z.

	    BOOL bRet;
	    LARGE_INTEGER offset;

	    offset.QuadPart = -1;
	    bRet = SetFilePointerEx (
	                osfh,
	                offset,
	                (PLARGE_INTEGER)NULL,
	                FILE_END );
	    if( ! bRet )
	    {
	        // MS: OS error -- should ignore negative seek error,
	        // since that means we had a zero-length file.
	        // All other errors cause a failure.
	        if( GetLastError() != ERROR_NEGATIVE_SEEK )
	        {
	            CloseHandle (osfh);
	            return INVALID_HANDLE_VALUE;
	        }
	    }
	    else
	    {
	        // Seek was OK, read the last char in file. The last
	        // char is a CTRL-Z if and only if _read returns 0
	        // and ch ends up with a CTRL-Z.

	        wchar_t ch = 0;
	        DWORD bytes_read;
	        BOOL success = FALSE;

	        if( ::ReadFile(osfh, &ch, (DWORD)1, &bytes_read, NULL )
	            && ch == CTRLZ)
	        {
	            // MS: read was OK and we got CTRL-Z! Wipe it out!
	            success =
	                SetFilePointerEx( osfh, offset, NULL, FILE_END ) &&
	                SetEndOfFile( osfh );

	            if( !success )
	            {
	                CloseHandle (osfh);
	                return INVALID_HANDLE_VALUE;
	            }
	        }

	        // now rewind the file to the beginning
	        offset.QuadPart = 0;
	        bRet = SetFilePointerEx (
	                        osfh,
	                        offset,
	                        (PLARGE_INTEGER)NULL,
	                        FILE_BEGIN );
	        if( ! bRet )
	        {
	            CloseHandle (osfh);
	            return INVALID_HANDLE_VALUE;
	        }
	    }
	}

	return osfh;
}

// Open the handle without the FD_TYPE book keeping.
//
HANDLE
nt_open2( StrPtr *fname, int flags, int mode, int dounicode, int lfn )
{
	HANDLE osfh;

	while( true )
	{
	    osfh = ntw_open( fname, flags, mode, dounicode, lfn );

	    if( osfh == INVALID_HANDLE_VALUE &&
	        GetLastError() != ERROR_BAD_ARGUMENTS )
	    {
	        // Claer LFN_UTF8 so that the MS Unicode conversion is used
	        // instead of our CVT class in nt_wname().  This is the only
	        // significant difference as compared to the Posix ::open().
	        //
	        int lfn_fallback = lfn & ~LFN_UTF8;

	        osfh = ntw_open( fname, flags, mode, dounicode, lfn_fallback );
	    }

	    if ( GetLastError() == ERROR_PIPE_BUSY )
	    {
	        if( WaitNamedPipe( fname->Text(), NMPWAIT_USE_DEFAULT_WAIT ) )
	        {
	            SetLastError( ERROR_SUCCESS );
	            continue;
	        }
	        else
                    break;   // timeout, don't try again
	    }
	    else
	        break;
	}

	return osfh ;
}

// Open the handle with FD_TYPE book keeping.
//
FD_TYPE
nt_open( StrPtr *fname, int flags, int mode, int dounicode, int lfn )
{
	HANDLE osfh;

	osfh = nt_open2( fname, flags, mode, dounicode, lfn );

	// System error already set in nt_open2.
	//
	if( osfh == INVALID_HANDLE_VALUE )
	    return FD_ERR;

	FD_TYPE fd = new struct P4_FD;

	fd->flags = flags;
	fd->mode = mode;
	fd->lock = 0;
	fd->dounicode = dounicode;
	fd->lfn = lfn;
	fd->fdFlags = 0;
	fd->fh = osfh;
	fd->ptr = NULL;
	fd->rcv = 0;
	if( fd->flags & O_BINARY )
	{
	    // Binary reads and writes go directly into caller's buffer.

	    fd->iobuf_siz = 0;
	    fd->iobuf = NULL;
	}
	else
	{
	    // Internal buffers for CR LF transalations.

	    fd->iobuf_siz = FileSys::BufferSize() * 2;
	    fd->iobuf = new unsigned char [fd->iobuf_siz];
	}

	return fd;
}

// Close the handle without the FD_TYPE book keeping.
// This function is intended internal only.
//
static void
nt_close2(FD_TYPE fd)
{
	// Close handle if it is not a standard handle.

	// A NULL check on fd is performed in nt_close().
	// nt_close() frees the fd structure.

	if( !(fd->fdFlags & FD_IsSTD) )
	    if( fd->fh != INVALID_HANDLE_VALUE )
	    {
	        CloseHandle( fd->fh );
	        fd->fh = INVALID_HANDLE_VALUE;
	    }
}

// Close the handle with the FD_TYPE book keeping.
//
int
nt_close(FD_TYPE fd)
{
	// Treat a double close or failed open as not an error.
	//
	if( fd == FD_ERR )
	    return 0;

	nt_close2( fd );

	if( fd->iobuf != NULL )
	{
	    delete []fd->iobuf;
	    fd->iobuf = NULL;
	    fd->iobuf_siz = 0;
	}
	delete fd;
	fd = FD_INIT;

	return 0;
}

// Close and reopen on the given journal related file handle.
// This can occur after a journal atomic rename.
//
// Return values,
//  success =  0
//  failure =  1
int
nt_reopen( StrPtr *fname, FD_TYPE fd )
{
	if( fd == FD_ERR )
	{
	    SetLastError( ERROR_INVALID_HANDLE );
	    return 1;
	}

	if( fd->fdFlags & FD_LOCKED )
	    if( lockFileByHandle( fd->fh, LOCKF_UN) < 0 )
	        return 1;

	nt_close2( fd );

	fd->fh = nt_open2( fname,
	                    fd->flags, fd->mode, fd->dounicode, fd->lfn );
	if( fd->fh == INVALID_HANDLE_VALUE )
	    return 1;

	if( fd->fdFlags & FD_LOCKED )
	    if( lockFileByHandle( fd->fh, fd->lock) < 0 )
	        return 1;

	return 0;
}

// Used when a file has been unlinked and there are handles
// still open on the target file.
//
// Return values,
//  success =  0
//  failure =  1
int
nt_reopen_wait( StrPtr *fname, FD_TYPE fd )
{
	if( fd == FD_ERR )
	{
	    SetLastError( ERROR_INVALID_HANDLE );
	    return 1;
	}

	int count;
	int renameMax  = p4tunable.Get( P4TUNE_SYS_RENAME_MAX );
	int renameWait = p4tunable.Get( P4TUNE_SYS_RENAME_WAIT );

	for( count=0; count < renameMax; ++count )
	{
	    msleep( renameWait );

	    // If this check fails, we try the open anyway, that will
	    // return the appropriate error.
	    //
	    if( nt_check_fh (fd->fh) != FH_DELETE_PENDING )
	        break;
	}

	if( count >= renameMax )
	    return 1;

	return nt_reopen( fname, fd );
}

// Return -1 on error, or bytes read.
//
int
nt_read( FD_TYPE fd, const void *buf, unsigned len )
{
	BOOL bRet;
	DWORD l;

	if( fd == FD_ERR )
	{
	    SetLastError( ERROR_INVALID_HANDLE );
	    return -1;
	}

	if( fd->flags & O_BINARY )
	{
	    // Special case, when reading on EOF, we look for a
	    // broken pipe and return 0.  This matches the behavior
	    // of the Posix read function.

	    if( ::ReadFile( fd->fh, (LPVOID)buf, len, &l, NULL ) == FALSE )
	    {
	        DWORD err = GetLastError();
	        if( err != ERROR_BROKEN_PIPE )
	            return -1;
	    }
	    return l;
	}

	// Read logic: read whole lines that end in \r, and
	// arrange so that a following \n translates the \r
	// into a \n and the \n is dropped.

	// soaknl: we saw a \r, skip this \n

	unsigned char *wrk_buf = (unsigned char *)buf;
	int wrk_len = len;
	int soaknl = 0;

	// This code was borrowed from FileIOBuffer::Read().  This code
	// has been pared down to just the CR/LF case.  This code could
	// be made simpler by removing the use of memccpy().  MS has
	// deprecated this function.  Memccpy function is not optimized.
	//
	while( wrk_len || soaknl )
	{
	    // Nothing in the buffer?

	    if( fd->rcv == 0 )
	    {
	       fd->ptr = fd->iobuf;

	       // We only read into iobuf limiting to len, translating
	       // from iobuf into buf should always be safe.

	       bRet = ::ReadFile( fd->fh, fd->iobuf, len,
	                            &(fd->rcv), NULL );
	       if( bRet == FALSE )
	           return -1;

	       if( fd->rcv == 0 )
	           break;
	    }

	    // Skipping \n because we saw a \r?

	    if( soaknl )
	    {
	       if( *(fd->ptr) == '\n' )
	           ++(fd->ptr), --(fd->rcv), wrk_buf[-1] = '\n';
	       soaknl = 0;
	    }

	    // Trim avail to what's needed
	    // Fill user buffer; stop at \r

	    unsigned char *p;
	    int l = fd->rcv < wrk_len ? fd->rcv : wrk_len ;

	    // Copy to next \r.  If we hit one, arrange so that
	    // if we see \n the next time through (when we know
	    // there'll be data in the buffer), we translate this
	    // \r to a \n and drop the subsequent \n.
	    // LFCRLF reads CRLF.

	    if( ( p = (unsigned char *)memccpy( wrk_buf, fd->ptr, '\r', l ) ) )
	    {
	       l = p - wrk_buf;
	       soaknl = 1;
	    }

	    fd->ptr += l;
	    fd->rcv -= l;
	    wrk_buf += l;
	    wrk_len -= l;
	}

	return len - wrk_len;
}

// Return -1 on error, or bytes written.
//
int
nt_write ( StrPtr *fname, FD_TYPE fd, const void *buf, unsigned cnt )
{
	if( fd == FD_ERR )
	{
	    SetLastError( ERROR_INVALID_HANDLE );
	    return -1;
	}

	int lfcount = 0;	// count of line feeds
	int charcount = 0;	// count of chars written so far
	DWORD written;		// count of chars written on this write
	DWORD lasterr = 0;	// win32 error

	// nothing to do, just return
	if( cnt == 0 )
	    return 0;

	if( !(fd->fdFlags & FD_IsSTD) && fd->lfn & LFN_ATOMIC_RENAME )
	{
	    switch ( nt_check_fh( fd->fh ) )
	    {
	        case FH_ERROR:
	            return -1;

	        // Continue, do not need to reopen the handle.
	        //
	        case FH_CONTINUE:
	            break;

	        // Any open handle on an unlinked file blocks the purge
	        // of the directory entry.  There is no way around a
	        // delete pending until all handles are closed on the
	        // file which was unlinked.
	        //
	        // We make an attempt to wait out the delete pending.
	        //
	        case FH_DELETE_PENDING:
	            if( nt_reopen_wait( fname, fd ) )
	                return -1;
	            break;

	        case FH_READ_ONLY:
	            if( nt_reopen( fname, fd ) )
	                return -1;
	            break;
	    }
	}

	if( fd->flags & O_APPEND )
	{
	    // We are appending, seek to the end of the file.
	    // Return an error if the seek fails.
	    //
	    LARGE_INTEGER offset;
	    offset.QuadPart = 0;
	    if( SetFilePointerEx (
	              fd->fh,
	              offset,
	              (PLARGE_INTEGER)NULL,
	              FILE_END ) == 0)
	    {
	       // Caller will report the error.
	       return -1;
	    }
	}

	if( fd->flags & O_BINARY )
	{
	    // binary mode, no translation

	    if ( ::WriteFile( fd->fh, buf, cnt, &written, NULL ) == FALSE )
	       return -1;
	    return written;
	}
	else
	{
	    // text mode, translate LF's to CR/LF's on output

	    lasterr = 0;		// no win32 error yet

	    char ch;			// current character
	    unsigned char *q = NULL;
	    char *p = (char *)buf;	// beginning of input buffer

	    while( (unsigned)(p - (char *)buf) < cnt )
	    {
	       q = fd->iobuf;	// start at beginning of iobuf

	       // fill the lf buf, except maybe last char

	       while ( q - fd->iobuf < fd->iobuf_siz - 1 &&
	              (unsigned)(p - (char *)buf) < cnt )
	       {
	           ch = *p++;
	           if ( ch == LF )
	           {
	              ++lfcount;
	              *q++ = CR;
	           }
	           *q++ = ch;
	       }

	       // write the lf buf and update total
	       if( ::WriteFile( fd->fh,
	                     fd->iobuf,
	                     (int)(q - fd->iobuf),
	                     (LPDWORD)&written,
	              	NULL) == FALSE )
	       {
	           lasterr = GetLastError();
	           break;
	       }

	       charcount += written;
	       if( written < q - fd->iobuf )
	           break;
	    }
	}

	if( charcount == 0 )
	{
	    // If nothing was written, first check for a win32 error,
	    // otherwise we return -1, let the caller report the error.
	    // Unless a device and first char was CTRL-Z

	    if( lasterr != 0 )
	       return -1;
	    else if( *(char *)buf == CTRLZ )
	       return 0;
	    else
	       return -1;
	}
	else
	{
	    // return adjusted bytes written
	    return charcount - lfcount;
	}
}

static int
ntw_stat( StrPtr *fname, struct statbL *sb, int lfn )
{
	int ret;
	int newlen = 0;
	const wchar_t *wname;

	wname = nt_wname( fname, lfn, &newlen );
	if ( !wname )
	    return -1;

	// Length check for unicode.
	// If LFN and Unicode are combined, this can be removed.
	if( !(lfn & LFN_ENABLED) && newlen > ( MAX_PATH * 2 ) )
	{
	    nt_free_wname( wname );
	    SetLastError( ERROR_BUFFER_OVERFLOW );
	    return -1;
	}

	if( lfn & LFN_ENABLED )
	    ret = nt_wstati64( wname, sb );  // LFN
	else
	    ret = _wstati64( wname, sb );   // Unicode
	nt_free_wname( wname );
	return ret;
}

static int
nt_stat( StrPtr *fname, struct statbL *sb, int dounicode, int lfn )
{
	// Allow unicode to fall through.
	if( dounicode || lfn )
	{
	    int ret;
	    if( (ret = ntw_stat( fname, sb, lfn ) ) >= 0 ||
	        lfn & LFN_ENABLED )
	            return ret;
	}

	if( fname->Length() > MAX_PATH )
	{
	    SetLastError( ERROR_BUFFER_OVERFLOW );
	    return -1;
	}

	return ::_stati64( fname->Text(), sb );
}

static int
ntw_unlink( StrPtr *fname, int lfn )
{
	DWORD dwFlags = 0;
	const wchar_t *wname=NULL;

	wname = nt_wname( fname, lfn, NULL );
	if ( !wname )
	    return -1;

	if( ntw_islink( fname, &dwFlags, lfn ) >= 0 )
	{
	    if( dwFlags == SYMBOLIC_LINK_FLAG_DIRECTORY )
	    {
	        BOOL bRet = RemoveDirectoryW( wname );
	        nt_free_wname( wname );
	        return bRet ? 0 : -1;
	    }
	    else
	    {
	        int ret = _wunlink( wname );
	        nt_free_wname( wname );
	        return ret;
	    }
	}
	else
	    nt_free_wname( wname );
	return -1;
}

static int
nt_unlink( StrPtr *fname, int dounicode, int lfn )
{
	DWORD dwFlags = 0;

	// Allow unicode to fall through.
	if( dounicode || lfn )
	{
	    int ret;
	    if( (ret = ntw_unlink( fname, lfn )) >= 0 ||
	        lfn & LFN_ENABLED )
	    	    return ret;
	}

	// no error returned if directory is not removed.
	if( nt_islink( fname, &dwFlags, dounicode, lfn ) > 0 &&
	    dwFlags == SYMBOLIC_LINK_FLAG_DIRECTORY &&
	    RemoveDirectory( fname->Text() ) )
	        return 0;

	return ::_unlink( fname->Text() );
}

static HANDLE
nt_openDirOrFileHandleW( StrPtr *fname, DWORD flags, int lfn )
{
	HANDLE fH;
	const wchar_t *wname;

	wname = nt_wname( fname, lfn, NULL );
	if( !wname )
	    return INVALID_HANDLE_VALUE;

	fH = CreateFileW( wname,
	        FILE_WRITE_ATTRIBUTES,
	        ( FILE_SHARE_READ | FILE_SHARE_WRITE ),
	        NULL,
	        OPEN_EXISTING,
	        FILE_ATTRIBUTE_NORMAL,
	        NULL);

	nt_free_wname( wname );
	return fH;
}
static HANDLE
nt_openHandleW( StrPtr *fname, int lfn )
{
	return nt_openDirOrFileHandleW( fname, FILE_ATTRIBUTE_NORMAL, lfn );
}
static HANDLE
nt_openDirHandleW( StrPtr *fname, int lfn )
{
	return nt_openDirOrFileHandleW( fname,
	        ( FILE_FLAG_BACKUP_SEMANTICS | FILE_ATTRIBUTE_NORMAL ), lfn );
}

static HANDLE
nt_openDirOrFileHandle( const char *fname, DWORD flags )
{
	return CreateFile( fname,
	        FILE_WRITE_ATTRIBUTES,
	        ( FILE_SHARE_READ | FILE_SHARE_WRITE ),
	        NULL,
	        OPEN_EXISTING,
	        flags,
	        NULL);
}
static HANDLE
nt_openHandle( const char *fname )
{
	return nt_openDirOrFileHandle( fname, FILE_ATTRIBUTE_NORMAL );
}
static HANDLE
nt_openDirHandle( const char *fname )
{
	return nt_openDirOrFileHandle( fname,
	        ( FILE_FLAG_BACKUP_SEMANTICS | FILE_ATTRIBUTE_NORMAL ) );
}

// This code corrected a DST datetime problem, job039200
// As of Visual Studio 2013, the problem has been fixed.
// Keep this fix as we still build with older Visual Studios.
//
// msec is in milliseconds.
//
static int
nt_convertToFileTime( time_t t32, int msec, FILETIME *ft)
{
	SYSTEMTIME st;
	struct tm *u_tm;

	u_tm = ::gmtime( &t32 );

	if( !u_tm )
	    return -1;

	st.wMilliseconds = msec;
	st.wDayOfWeek = 0;
	st.wSecond = u_tm->tm_sec;
	st.wMinute = u_tm->tm_min;
	st.wHour   = u_tm->tm_hour;
	st.wDay    = u_tm->tm_mday;
	st.wMonth  = u_tm->tm_mon + 1;
	st.wYear   = u_tm->tm_year + 1900;

	SystemTimeToFileTime( &st, ft );

	return 0;
}

// msec is in milliseconds.
//
static int
nt_setFileTimes( HANDLE hFile, time_t t32, int msec )
{
	FILETIME ft;
	int result;

	if( hFile == INVALID_HANDLE_VALUE || t32 == -1 ||
	        nt_convertToFileTime( t32, msec, &ft ) )
	    return -1;
	result = SetFileTime( hFile, (LPFILETIME)0, (LPFILETIME)0, &ft ) != 0
	        ? 0 : -1 ;
	CloseHandle( hFile );
	return result;
}

// msec is in milliseconds.
//
static int
ntw_utime( StrPtr *fname, struct utimbufL *ut, int msec, int lfn )
{
	const wchar_t *wname;
	int ret;

	wname = nt_wname( fname, lfn, NULL );
	if( !wname )
	    return -1;

	ret = nt_setFileTimes( nt_openHandleW( fname, lfn ),
	                        ut->modtime, msec );
	nt_free_wname( wname );
	return ret;
}

// msec is in milliseconds.
//
static int
nt_utime( StrPtr *fname, struct utimbufL *ut, int msec, int dounicode, int lfn)
{
	// Allow unicode to fall through.
	if( dounicode || lfn )
	{
	    int ret;
	    if ( (ret = ntw_utime( fname, ut, msec, lfn ) ) >= 0 ||
	        lfn & LFN_ENABLED )
	            return ret;
	}

	return nt_setFileTimes( nt_openHandle( fname->Text() ),
	                	ut->modtime, msec );
}

// This function returns 0 for success and -1 for failure.
// (Windows file attribute return TRUE for success and FALSE for failure.)
// On Windows, S_IEXEC is determined by the file extension.
// The error condition is collected from GetLastError() in the caller.
//
static int
nt_chmod( StrPtr *fname, int m, int dounicode, int lfn )
{
	const wchar_t *wname;
	WIN32_FILE_ATTRIBUTE_DATA attr_data;

	wname = nt_wname( fname, lfn, NULL );
	if( !wname )
	    return -1;

	if (!GetFileAttributesExW(wname, GetFileExInfoStandard, (void*) &attr_data))
	{
	    nt_free_wname( wname );
	    return -1;
	}

	if (m & _S_IWRITE)
	{
	    // clear read only bit
	    attr_data.dwFileAttributes &= ~FILE_ATTRIBUTE_READONLY;
	}
	else
	{
	    // set read only bit
	    attr_data.dwFileAttributes |= FILE_ATTRIBUTE_READONLY;
	}

	// set new attribute
	if (!SetFileAttributesW(wname, attr_data.dwFileAttributes))
	{
	    nt_free_wname( wname );
	    return -1;
	}

	nt_free_wname( wname );
	return 0;
}

static int
ntw_setattr( StrPtr *fname, int m, int lfn )
{
	const wchar_t *wname;
	int ret;

	wname = nt_wname( fname, lfn, NULL );
	if( !wname )
	    return -1;

	ret = SetFileAttributesW( wname, m ) ? 1 : 0 ;
	nt_free_wname( wname );
	return ret;
}

static int
nt_setattr( StrPtr *fname, int m, int dounicode, int lfn )
{
	// Allow unicode to fall through.
	if( dounicode || lfn )
	{
	    int ret;
	    if ((ret = ntw_setattr( fname, m, lfn )) >= 0 ||
	        lfn & LFN_ENABLED )
	            return ret;
	}

	return SetFileAttributesA( fname->Text(), m ) ? 1 : 0 ;
}

typedef DWORD ACCESS_MASK;

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _OBJECT_ATTRIBUTES {
    ULONG Length;
    HANDLE RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG Attributes;
    PVOID SecurityDescriptor;        // Points to type SECURITY_DESCRIPTOR
    PVOID SecurityQualityOfService;  // Points to type SECURITY_QUALITY_OF_SERVICE
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

typedef struct _FILE_GET_EA_INFORMATION
{
    ULONG NextEntryOffset;
    UCHAR EaNameLength;
    CHAR EaName[1];
} FILE_GET_EA_INFORMATION, *PFILE_GET_EA_INFORMATION;

typedef struct _FILE_FULL_EA_INFORMATION {
    ULONG  NextEntryOffset;
    UCHAR  Flags;
    UCHAR  EaNameLength;
    USHORT EaValueLength;
    CHAR   EaName[1];
} FILE_FULL_EA_INFORMATION, *PFILE_FULL_EA_INFORMATION;

typedef struct _IO_STATUS_BLOCK {
    union {
        long Status;
        PVOID    Pointer;
    };
    ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

typedef struct _RTL_RELATIVE_NAME_U
{
	UNICODE_STRING RelativeName;
	HANDLE ContainingDirectory;
	PVOID CurDirRef;
} RTL_RELATIVE_NAME_U, *PRTL_RELATIVE_NAME_U;

typedef void (NTAPI *RtlInitUnicodeStringPtr)(
	PUNICODE_STRING         DestinationString,
	PCWSTR SourceString );

typedef ULONG (NTAPI *NtQueryEaFilePtr) (
	HANDLE FileHandle,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID Buffer,
	ULONG Length,
	BOOLEAN ReturnSingleEntry,
	PVOID EaList,
	ULONG EaListLength,
	PULONG EaIndex,
	BOOLEAN RestartScan );

typedef ULONG (NTAPI *NtOpenFilePtr) (
	PHANDLE FileHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	PIO_STATUS_BLOCK IoStatusBlock,
	ULONG ShareAccess,
	ULONG OpenOptions );

typedef ULONG (NTAPI *NtSetEaFilePtr)(
	HANDLE FileHandle,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID EaBuffer,
	ULONG EaBufferSize );

typedef ULONG (NTAPI *NtClosePtr)( HANDLE FileHandle );

typedef ULONG (NTAPI *RtlDosPathNameToNtPathName_UPtr)(
	PWSTR DosFileName,
	PUNICODE_STRING NtFileName,
	PWSTR* FilePart,
	PRTL_RELATIVE_NAME_U RelativeName );

typedef void (NTAPI *RtlFreeUnicodeStringPtr)(
	PUNICODE_STRING UnicodeString );

typedef ULONG (NTAPI *RtlNtStatusToDosErrorPtr)(
	ULONG Status );

#define NT_SUCCESS(Status) (((long)(Status)) >= 0)

#define InitializeObjectAttributes(p,n,a,r,s) { \
  (p)->Length = sizeof(OBJECT_ATTRIBUTES); \
  (p)->RootDirectory = (r); \
  (p)->Attributes = (a); \
  (p)->ObjectName = (n); \
  (p)->SecurityDescriptor = (s); \
  (p)->SecurityQualityOfService = NULL; \
}

// Share flags

#define FILE_SHARE_READ                         0x00000001
#define FILE_SHARE_WRITE                        0x00000002
#define FILE_SHARE_DELETE                       0x00000004
#define FILE_SHARE_VALID_FLAGS                  0x00000007

// Open options

#define FILE_DIRECTORY_FILE                     0x00000001
#define FILE_WRITE_THROUGH                      0x00000002
#define FILE_SEQUENTIAL_ONLY                    0x00000004
#define FILE_NO_INTERMEDIATE_BUFFERING          0x00000008

#define FILE_SYNCHRONOUS_IO_ALERT               0x00000010
#define FILE_SYNCHRONOUS_IO_NONALERT            0x00000020
#define FILE_NON_DIRECTORY_FILE                 0x00000040
#define FILE_CREATE_TREE_CONNECTION             0x00000080

#define FILE_COMPLETE_IF_OPLOCKED               0x00000100
#define FILE_NO_EA_KNOWLEDGE                    0x00000200
#define FILE_OPEN_REMOTE_INSTANCE               0x00000400
#define FILE_RANDOM_ACCESS                      0x00000800

#define FILE_DELETE_ON_CLOSE                    0x00001000
#define FILE_OPEN_BY_FILE_ID                    0x00002000
#define FILE_OPEN_FOR_BACKUP_INTENT             0x00004000
#define FILE_NO_COMPRESSION                     0x00008000

// Errors

#define STATUS_NO_EAS_ON_FILE                   0xC0000052L
#define STATUS_NONEXISTENT_EA_ENTRY             0xC0000051L
#define STATUS_BUFFER_OVERFLOW                  0x80000005L


static HMODULE ntdll = 0;
static NtSetEaFilePtr NtSetEaFile = 0;
static NtOpenFilePtr NtOpenFile = 0;
static NtClosePtr NtClose = 0;
static RtlDosPathNameToNtPathName_UPtr RtlDosPathNameToNtPathName_U = 0;
static RtlFreeUnicodeStringPtr RtlFreeUnicodeString = 0;
static NtQueryEaFilePtr NtQueryEaFile = 0;
static RtlNtStatusToDosErrorPtr RtlNtStatusToDosError = 0;

long
ntw_getea( StrPtr *fname,  StrBufDict *eaList, int lfn )
{
	// Globally runtime load the DDK function pointers

	if( !ntdll )
	    ntdll = GetModuleHandle( "ntdll.dll" );
	if( !ntdll )
	    return -1;

	if( !NtQueryEaFile )
	    NtQueryEaFile = (NtQueryEaFilePtr)
	                              GetProcAddress( ntdll, "NtQueryEaFile" );
	if( !NtQueryEaFile )
	    return -1;

	if( !NtOpenFile )
	    NtOpenFile = (NtOpenFilePtr)GetProcAddress( ntdll, "NtOpenFile" );
	if( !NtOpenFile )
	    return -1;

	if( !NtClose )
	    NtClose = (NtClosePtr)GetProcAddress( ntdll, "NtClose" );
	if( !NtClose )
	    return -1;

	if( !RtlDosPathNameToNtPathName_U )
	    RtlDosPathNameToNtPathName_U = (RtlDosPathNameToNtPathName_UPtr)
	               GetProcAddress( ntdll, "RtlDosPathNameToNtPathName_U" );
	if( !RtlDosPathNameToNtPathName_U )
	    return -1;

	if( !RtlFreeUnicodeString )
	    RtlFreeUnicodeString = (RtlFreeUnicodeStringPtr)
	                       GetProcAddress( ntdll, "RtlFreeUnicodeString" );
	if( !RtlFreeUnicodeString )
	    return -1;

	if( !RtlNtStatusToDosError )
	    RtlNtStatusToDosError = (RtlNtStatusToDosErrorPtr)
	                      GetProcAddress( ntdll, "RtlNtStatusToDosError" );
	if( !RtlNtStatusToDosError )
	    return -1;


	// First off, we need the UNC file path
	const wchar_t *wname = nt_wname( fname, lfn, NULL );
	if( !wname )
	    return -1;

	// Now we need it driver dev style
	// This means using the NT Object path convention: /??/
	// See https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-even/c1550f98-a1ce-426a-9991-7509e7c3787c

	UNICODE_STRING upath;
	RtlDosPathNameToNtPathName_U( (wchar_t *)wname, &upath, 0, 0 );

	OBJECT_ATTRIBUTES attr;
	InitializeObjectAttributes( &attr, &upath, 0, 0, 0 );

	IO_STATUS_BLOCK io;
	HANDLE h;

	unsigned long status = NtOpenFile( &h,
	                          FILE_READ_EA,
	                          &attr, &io,
	                          FILE_SHARE_VALID_FLAGS,
	                          0 );
	nt_free_wname( wname );
	RtlFreeUnicodeString( &upath );

	if( !NT_SUCCESS( status ) )
	    return RtlNtStatusToDosError( status );

	bool restartScan = true;
	char buf[4096];
	do {
	    status = NtQueryEaFile( h, &io, buf, 4096, false, 0, 0, 0,
	                            restartScan );
	    if( !NT_SUCCESS( status ) )
	    {
	        switch (status)
	        {
	        case STATUS_NONEXISTENT_EA_ENTRY:
	        case STATUS_NO_EAS_ON_FILE:
	            NtClose( h );
	            return 0;
	        default:
	            NtClose( h );
	            return RtlNtStatusToDosError( status );
	        }
	    }
	    
	    PFILE_FULL_EA_INFORMATION ea = (PFILE_FULL_EA_INFORMATION)buf;
	    do
	    {
	        if (ea->EaValueLength > 0 )
	            eaList->SetVar( StrRef(ea->EaName),
	                            StrRef( ea->EaName + ea->EaNameLength + 1,
	                                    ea->EaValueLength ) );
	
	    } while( (ea = (FILE_FULL_EA_INFORMATION *)(ea->NextEntryOffset ?
	                        ((char *) ea + ea->NextEntryOffset) : NULL)) );

	    restartScan = false;
	} while( status == STATUS_BUFFER_OVERFLOW );

	NtClose( h );
	return 0;
}

long
nt_getea( StrPtr *fname, StrBufDict *eaList, int dounicode, int lfn )
{
	// We have to use wchar names, so force LFN if we're not unicode
	return ntw_getea( fname, eaList,
	                  lfn ? lfn : !dounicode ? LFN_ENABLED : 0 );
}

long
ntw_setea( StrPtr *fname, StrDict *vals, int lfn ) 
{
	if( !vals->GetCount() )
	    return 0;

	// Globally runtime load the DDK function pointers

	if( !ntdll )
	    ntdll = GetModuleHandle( "ntdll.dll" );
	if( !ntdll )
	    return -1;

	if( !NtSetEaFile )
	    NtSetEaFile = (NtSetEaFilePtr)
	                                GetProcAddress( ntdll, "NtSetEaFile" );
	if( !NtSetEaFile )
	    return -1;

	if( !NtOpenFile )
	    NtOpenFile = (NtOpenFilePtr)GetProcAddress( ntdll, "NtOpenFile" );
	if( !NtOpenFile )
	    return -1;

	if( !NtClose )
	    NtClose = (NtClosePtr)GetProcAddress( ntdll, "NtClose" );
	if( !NtClose )
	    return -1;

	if( !RtlDosPathNameToNtPathName_U )
	    RtlDosPathNameToNtPathName_U = (RtlDosPathNameToNtPathName_UPtr)
	               GetProcAddress( ntdll, "RtlDosPathNameToNtPathName_U" );
	if( !RtlDosPathNameToNtPathName_U )
	    return -1;

	if( !RtlFreeUnicodeString )
	    RtlFreeUnicodeString = (RtlFreeUnicodeStringPtr)
	                       GetProcAddress( ntdll, "RtlFreeUnicodeString" );
	if( !RtlFreeUnicodeString )
	    return -1;
	
	if( !RtlNtStatusToDosError )
	    RtlNtStatusToDosError = (RtlNtStatusToDosErrorPtr)
	                      GetProcAddress( ntdll, "RtlNtStatusToDosError" );
	if( !RtlNtStatusToDosError )
	    return -1;

	// First off, we need the UNC file path
	const wchar_t *wname = nt_wname( fname, lfn, NULL );
	if( !wname )
	    return -1;

	/*
	// Now we need it driver dev style
	// This means using the NT Object path convention: /??/
	// See https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-even/c1550f98-a1ce-426a-9991-7509e7c3787c
	*/

	UNICODE_STRING upath;
	RtlDosPathNameToNtPathName_U( (wchar_t *)wname, &upath, 0, 0 );

	OBJECT_ATTRIBUTES attr;
	InitializeObjectAttributes( &attr, &upath, 0, 0, 0 );

	IO_STATUS_BLOCK io;
	HANDLE h;

	long status = NtOpenFile( &h,
	                          READ_CONTROL | FILE_READ_EA | FILE_WRITE_EA,
	                          &attr, &io,
	                          FILE_SHARE_VALID_FLAGS,
	                          FILE_OPEN_FOR_BACKUP_INTENT );

	nt_free_wname( wname );
	RtlFreeUnicodeString( &upath );

	if( !NT_SUCCESS( status ) )
	    return RtlNtStatusToDosError( status );

	size_t size = 0;
	StrDictIterator *iter = vals->GetIterator();
	StrRef name, val;
	int align = sizeof( ULONG );
	while( iter->Get( name, val ) )
	{
	    size += sizeof( FILE_FULL_EA_INFORMATION )
	         + name.Length() + val.Length();
	    size += size % align ? align - (size % align) : 0;
	    iter->Next();
	}
	char* buf = new char[size];
	memset( buf, 0, size );

	iter->Reset();

	PFILE_FULL_EA_INFORMATION ea = (PFILE_FULL_EA_INFORMATION)buf;
	size_t end = 0;
	while( iter->Get( name, val ) )
	{
	    ea->NextEntryOffset = end;
	    if( end )
	    {
	        ea = (PFILE_FULL_EA_INFORMATION)((char *)ea + end);
	        ea->NextEntryOffset = 0;
	        end = 0;
	    }
	    ea->Flags = 0;
	    ea->EaNameLength = name.Length();
	    memcpy( ea->EaName, name.Text(), name.Length() + 1 );
	    end = ea->EaName - (char *)ea + ea->EaNameLength + 1;

	    if( !val.Length() )
	        ea->EaValueLength = 0;
	    else
	    {
	        ea->EaValueLength = val.Length();
	        memcpy( ea->EaName + ea->EaNameLength + 1, val.Text(),
	                val.Length() );
	        end += val.Length();
	    }
	    end += end % align ? align - (end % align) : 0;
	    iter->Next();
	}

	int ret = NtSetEaFile( h, &io, buf, size );
	delete[] buf;
	NtClose( h );

	if( !NT_SUCCESS( ret ) )
	    return RtlNtStatusToDosError( ret );
	return ret;
}

long
ntw_setea( StrPtr *fname, StrPtr *name, StrPtr *val, int lfn )
{
	// We have to use wchar names, so force LFN if we're not unicode
	StrPtrDict vals;
	vals.SetVar( name->Text(), val ? val->Text() : "" );
	return ntw_setea( fname, &vals, lfn );
}

long
nt_setea( StrPtr *fname, StrPtr *name, StrPtr *val, int dounicode, int lfn )
{
	// We have to use wchar names, so force LFN if we're not unicode
	return ntw_setea( fname, name, val,
	                  lfn ? lfn : !dounicode ? LFN_ENABLED : 0 );
}

long
nt_setea( StrPtr *fname, StrDict *vals, int dounicode, int lfn )
{
	// We have to use wchar names, so force LFN if we're not unicode
	return ntw_setea( fname, vals,
	                  lfn ? lfn : !dounicode ? LFN_ENABLED : 0 );
}

// Return values
//   success =  0
//   failure =  1
//
static int
ntw_rename( StrPtr *fname, StrPtr *nname, int lfn )
{
	const wchar_t *wname;
	const wchar_t *wnname;
	int ret;

	wname = nt_wname( fname, lfn, NULL );
	if( !wname )
	    return -1;

	wnname = nt_wname( nname, lfn, NULL );
	if( !wnname )
	{
	    nt_free_wname( wname );
	    return -1;
	}

	// MoveFileEx returns 0 on failure, 1 for success.
	//
	// allow moving to a different volume

	ret = !MoveFileExW(wname, wnname, MOVEFILE_COPY_ALLOWED);

	nt_free_wname( wname );
	nt_free_wname( wnname );
	return ret;
}

// Return values
//  success =  0
//  failure =  1
//
static int
nt_rename( StrPtr *fname, StrPtr *nname, int dounicode, int lfn )
{
	// Allow unicode to fall through.
	// LFN does not fall through.
	if( dounicode || lfn )
	{
	    int ret;
	    if( (ret=ntw_rename( fname, nname, lfn )) == 0 ||
	        lfn & LFN_ENABLED )
	            return ret;
	}

	// rename calls wrename, which calls MoveFileExW
	// MoveFileEx returns 0 on failure, 1 for success.
	// rename returns !MoveFileEx
	//

	return ::rename( fname->Text(), nname->Text() );
}

// This code is only used on the client.
// Target must be absolute, can not chdir for LFN.
// Must call this function through nt_makelink().
int
ntw_makelink( StrBuf &target, StrPtr *name, DWORD dwFlags, int lfn )
{
	int result = -1;

	// For the symlink target we do not want a LFN path.
	const wchar_t *wtarget = nt_wname( &target, 0, NULL );
	if( !wtarget )
	    return -1;
	const wchar_t *wname = nt_wname( name, lfn, NULL );
	if( !wname )
	{
	    nt_free_wname( wtarget );
	    return -1;
	}

	if( (*CreateSymbolicLinkW_func)( wname, wtarget, dwFlags ) )
	    result = 0;

	nt_free_wname( wtarget );
	nt_free_wname( wname );

	return result;
}

// This code is only used on the client.
int
nt_makelink( StrBuf &target, StrPtr *name, int dounicode, int lfn )
{
	int result = -1;
	StrBuf n_tgt;
	StrBuf abs_tgt;

	if( !FileSys::SymlinksSupported() )
	    return result;

	// Copy and normalize the target of the symlink for Windows.
	char *symlink = target.Text();
	n_tgt.Set( target.Text() );
	char *p = n_tgt.Text();
	while( *symlink )
	{
	    if( *symlink == '/' ) *p = '\\';
	    p++;
	    symlink++;
	}
	*p = '\0';

	// Create an absolute target for the stat().
	if( FileSys::IsRelative( n_tgt ) )
	{
	    PathSys *pth = PathSys::Create();
	    pth->Set( name->Text() );
	    pth->ToParent();
	    pth->SetLocal( StrRef( pth->Text() ), n_tgt );
	    abs_tgt.Set( pth->Text() );
	}
	else
	    abs_tgt.Set( n_tgt.Text() );

	struct statbL sb;
	DWORD dwFlags = 0;
	if( SystemInfo::CheckForDevSymlink() )
	    dwFlags |= SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;

	// Try to stat the target of the symlink, directory or file.
	// If the stat fails, we assume a file symlink.
	if( nt_stat( &abs_tgt, &sb, dounicode, lfn ) >= 0 )
	{
	    if( S_ISDIR( sb.st_mode ) )
	        dwFlags |= SYMBOLIC_LINK_FLAG_DIRECTORY;
	}

	// Allow unicode to fall through.
	// Using target maintains relative symlinks.
	if( dounicode || lfn )
	{
	    int ret;
	    if( (ret = ntw_makelink( n_tgt, name, dwFlags, lfn )) >= 0 ||
	        lfn & LFN_ENABLED )
	            return ret;
	}

	if( (*CreateSymbolicLinkA_func)(name->Text(), n_tgt.Text(), dwFlags) )
	    result = 0;

	return result;
}

int
FileIO::OsRename( StrPtr *source, StrPtr *target, FileSys *origTarget )
{
	return nt_rename( source, target, DOUNICODE, 
	                  LFN | origTarget->GetLFN() );
}

void
FileIO::Rename( FileSys *target, Error *e )
{
	static int (*nt_rename_func)( StrPtr *, StrPtr *, int , int );

	// The LFN_ATOMIC_RENAME flag determines atomicity.
	// Right now this is only for journal files.
	// And only if built with VS2017 and newer.
	// If older then VS2017, nt_atrename is stubbed to be an error.
	// Which should only happen if there is a coding error.

	if( LFN & LFN_ATOMIC_RENAME )
	    nt_rename_func = &nt_atrename;
	else
	    nt_rename_func = &nt_rename;

	// On VMS and Novelle, the source must be writable (deletable, 
	// actually) for the rename() to unlink it.  So we give it write 
	// perm first and then revert it to original perms after.

	Chmod( FPM_RW, e );

	// Don't unlink the target unless the source exists,
	// as our rename isn't atomic (like on UNIX) and some
	// stumblebum user may have removed the source file.

	if( e->Test() )
	    return;

	// Remember original perms of target so we can reset on failure.

	int targetStat = target->Stat();
	FilePerm oldPerms =
	            ( targetStat & FSF_WRITEABLE ) ? FPM_RW : FPM_RO;

	// The atomic rename returns a different error code then
	// the regular rename API when renaming into a directory.
	// There is no point in going further, this will fail.
	// Also, we need to make errors look the same for t4.
	//
	// ... errmsg open for write: journal.6: Access is denied.

	if( LFN & LFN_ATOMIC_RENAME )
	    if( targetStat & FSF_DIRECTORY )
	    {
	        StrBuf b;
	        b << "open for write " << target->Name();
	        SetLastError( ERROR_ACCESS_DENIED );
	        e->Sys( "rename", b.Text() );
	        return;
	    }

	// One customer (in Iceland) wanted this for IRIX as well.
	// You need if you are you running NFS aginst NT as well
	// if you are running on NT.  Gag me!
	//
	// To support case-changing a file,  rename needs to NOT
	// unlink the file in this case, this is mainly client support.
	
	const StrPtr *targetPath = target->Path();

	// If we are using the atomic rename, we do not need to
	// unlink the target.

	if( !(LFN & LFN_ATOMIC_RENAME) &&
	      ( Path()->Length() != targetPath->Length() ||
	        Path()->Compare( *targetPath ) ) )
	{
	    target->Unlink( 0 ); // yeech - must not exist to rename
	}

	// Return values for both nt_rename and nt_atrename.
	//  success =  0
	//  failure =  1

	if( nt_rename_func( Path(), target->Path(), DOUNICODE,
	               LFN|target->GetLFN() ) )
	{
	    int ret = 1;
	    int renameMax  = p4tunable.Get( P4TUNE_SYS_RENAME_MAX );
	    int renameWait = p4tunable.Get( P4TUNE_SYS_RENAME_WAIT );

	    StrBuf currentName;
	    currentName = Name();

	    if( currentName.Contains( *targetPath ) || 
	        targetPath->Contains( currentName ) )
	    {
	        // Either target is a substring (directory) of source,
	        // or source is a substring of target (target has a
	        // directory subpath which is the as source) 
	        // or source or target has a component which is not a directory.

	        // Try moving the current name to a temporary name, and then trying again.

	        RenameSourceSubstrInTargetSubdir( currentName, target, e );

	        if( e->Test() )
	            return;

	        RenameTargetSubStrSubdirInSource( currentName, target, e );

	        if( e->Test() )
	            return;

	        ret = nt_rename_func( &currentName, target->Path(), DOUNICODE,
	               LFN|target->GetLFN() );
	    }

	    // If atomic rename fails, trying again will not make a difference.
	    //
	    if( ret && !(LFN & LFN_ATOMIC_RENAME) )
	    {
	        // nasty hack coming up.
	        // one customer is suffering from a rename() problem
	        // that requires more diagnostics,  so we will retry 
	        // the rename() 10 times with 1 second interval and
	        // log any failure.

	        for( int i=0; i < renameMax; ++i )
	        {
	            msleep( renameWait );

	            target->Unlink( 0 );

	            ret = nt_rename_func( &currentName, target->Path(), DOUNICODE,
	                             LFN|target->GetLFN() );

	            if( !ret )
	                break;
	        }
	    }

	    if( ret )
	    {
	        StrBuf b;
	        b << "failed to rename " << target->Name()
	          << " after " << StrNum( renameMax ) << " attempts";
	        e->Sys( "rename", b.Text() );

	        // failed, restore original target perms. 

	        target->Perms( oldPerms );
	        target->Chmod( e );
	        return;
	    }
	}

	// reset the target to our perms

	target->Perms( perms );
	target->Chmod( e );

	// source file has been deleted,  clear the flag
	ClearDeleteOnClose();
}

/*
 * FileIO::Unlink() - remove single file (error optional)
 */

void
FileIO::Unlink( Error *e )
{
	if( *Name() )
	{
	    // yeech - must be writable to remove
	    nt_chmod( Path(), PERM_0666  & ~global_umask, DOUNICODE, LFN );

	    if( nt_unlink( Path(), DOUNICODE, LFN ) < 0)
	    {
	        // Special handling for momentarily busy executable file.
	        // All other errors will not invoke retries.
	        if( errno == EACCES )
	        {
	            int renameMax  = p4tunable.Get( P4TUNE_SYS_RENAME_MAX );
	            int renameWait = p4tunable.Get( P4TUNE_SYS_RENAME_WAIT );

	            for( int i=0; i < renameMax; ++i )
	            {
	                msleep( renameWait );

	                if( nt_unlink( Path(), DOUNICODE, LFN ) >= 0 )
	                    return;
	            }
	        }

	        if( e && ! e->Test() )
	            e->Sys( "unlink", Name() );
	    }
	}
}

void
FileIO::UnlinkNoRetry( Error *e )
{
	if( *Name() )
	{
	    // yeech - must be writable to remove
	    nt_chmod( Path(), PERM_0666  & ~global_umask, DOUNICODE, LFN );

	    if( nt_unlink( Path(), DOUNICODE, LFN ) < 0)
	    {
	        if( e && ! e->Test() )
	            e->Sys( "unlink", Name() );
	    }
	}
}

// Caller must free the memory.
wchar_t *
FileIO::UnicodeName( StrBuf *fname, int lfn )
{
	wchar_t *ret;

	ret = (wchar_t *)nt_wname( fname, lfn, NULL );
	if( !ret )
	    return NULL;

	return ret;
}

void
FileIO::ChmodTime( P4INT64 modTime, Error *e )
{
	struct utimbufL t;

	t.actime = 0; // This is ignored by nt_utime
	t.modtime = DateTime::Localize( modTime );

	if( nt_utime( Path(), &t, 0, DOUNICODE, LFN ) < 0 )
	    e->Sys( "utime", Name() );
}

void
FileIO::ChmodTimeHP( const DateTimeHighPrecision &modTime, Error *e )
{
	struct utimbufL t;

	t.actime = 0; // This is ignored by nt_utime
	t.modtime = DateTime::Localize( modTime.Seconds() );

	if( nt_utime( Path(), &t, modTime.Nanos() / 1000000, DOUNICODE, LFN ) < 0 )
	    e->Sys( "utime", Name() );
}

void
FileIO::Truncate( offL_t offset, Error *e )
{
	// Don't bother if non-existent.

	if( !( Stat() & FSF_EXISTS ) )
	    return;

	int success = 1;

# ifdef HAVE_TRUNCATE

	HANDLE hFile;

	if( DOUNICODE || LFN )
	{
	    const wchar_t *wname;
	    wname = nt_wname( Path(), LFN, NULL );
	    if( !wname )
	    {
	        e->Sys( "truncate", Name() );
	        return;
	    }
	    hFile = CreateFileW( wname, GENERIC_WRITE, FILE_SHARE_WRITE,
	                        NULL, OPEN_EXISTING,
	                        FILE_ATTRIBUTE_NORMAL, NULL );
	    nt_free_wname( wname );
	}
	else
	{
	    hFile = CreateFile( Name(), GENERIC_WRITE, FILE_SHARE_WRITE,
	                        NULL, OPEN_EXISTING,
	                        FILE_ATTRIBUTE_NORMAL, NULL );
	}

	if (hFile == INVALID_HANDLE_VALUE)
	{
	    e->Sys( "truncate", Name() );
	    return;
	}
	
	LARGE_INTEGER offset_li;
	offset_li.QuadPart = offset;

	success = SetFilePointerEx( hFile, offset_li, 0, FILE_BEGIN ) &&
	          SetEndOfFile( hFile );
	                
	CloseHandle( hFile );

# endif // HAVE_TRUNCATE

	if( !success )
	    e->Sys( "truncate", Name() );
}

void
FileIO::Truncate( Error *e )
{
	// Don't bother if non-existent.

	if( !( Stat() & FSF_EXISTS ) )
	    return;

	// Try truncate first; if that fails (as it will on secure NCR's),
	// then open O_TRUNC.
	
	FD_TYPE fd;
	// checkFd() just returns on Windows, fd value doesn't matter.
	//
	fd = checkFd( nt_open( Path(), O_WRONLY|O_TRUNC, PERM_0666,
	                        DOUNICODE, LFN ) );
	if( fd != FD_ERR )
	{
	    if( nt_close( fd ) < 0 )
	        e->Sys( "close", Name() );
	    return;
	}

	e->Sys( "truncate", Name() );
}

/*
 * FileIO::Stat() - return flags if file exists
 */

int
FileIO::Stat()
{
	// Stat & check for missing, special

	int flags = 0;
	struct statbL sb;
	StrBuf abs_tgt;
	int islink = 0;

	if( FileSys::SymlinksSupported() &&
	    nt_islink( Path(), NULL, DOUNICODE, LFN ) > 0 )
	{
	    StrBuf linkTarget;
	    // The StrBuf allocation is done in nt_readlink().
	    if( nt_readlink( Path(), linkTarget, DOUNICODE, LFN ) < 0 )
	        return flags;
	    flags = FSF_SYMLINK;

	    // Create an absolute path for the symlink target.
	    if( FileSys::IsRelative( linkTarget ) )
	    {
	        PathSys *pth = PathSys::Create();
	        pth->Set( Name() );
	        pth->ToParent();
	        // SetLocal doesn't work with too many ../../
	        // as it doesn't check the number of directories
	        // in root. So do a simple append.
	        abs_tgt.Set( pth->Text() );
	        delete pth;
	        abs_tgt << "/" << linkTarget;
	    }
	    else
	        abs_tgt.Set( linkTarget );

	    islink = 1;
	}

	if( nt_stat( islink ? &abs_tgt :
		Path(), &sb, DOUNICODE, LFN ) < 0 )
	    return flags;

	flags |= FSF_EXISTS;

	if( sb.st_mode & S_IWUSR ) flags |= FSF_WRITEABLE;
	if( sb.st_mode & S_IXUSR ) flags |= FSF_EXECUTABLE;
	if( S_ISDIR( sb.st_mode ) ) flags |= FSF_DIRECTORY;
	if( !S_ISREG( sb.st_mode ) ) flags |= FSF_SPECIAL;
	if( !sb.st_size ) flags |= FSF_EMPTY;

	// Cache mtime+size from this nt_stat so a subsequent StatModTime()
	// or GetSize() can skip a redundant CreateFile or nt_stat.
	// Symlinks resolve to different mtimes via the handle vs nt_stat
	// paths, so we don't cache them.

	if( allowStatCache && !islink )
	{
	    cachedModTime = sb.st_mtime;
	    cachedSize    = sb.st_size;
	    statCacheValid = 1;
	}

	return flags;
}

int
FileIO::GetOwner()
{
	int uid = 0;
	struct statbL sb;
	StrBuf abs_tgt;

	if( FileSys::SymlinksSupported() &&
	    nt_islink( Path(), NULL, DOUNICODE, LFN ) > 0 )
	{
	    StrBuf linkTarget;
	    // The StrBuf allocation is done in nt_readlink().
	    if( nt_readlink( Path(), linkTarget, DOUNICODE, LFN ) < 0 )
	        return uid;

	    // Create an absolute path for the target.
	    if( FileSys::IsRelative( *Path() ) )
	    {
	        PathSys *pth = PathSys::Create();
	        pth->Set( Name() );
	        pth->ToParent();
	        pth->SetLocal( StrRef( pth->Text() ), linkTarget );
	        abs_tgt.Set( pth->Text() );
	    }
	    else
	        abs_tgt.Set( Name() );

	    if( nt_stat( &abs_tgt, &sb, DOUNICODE, LFN ) >= 0 )
	        uid = sb.st_uid;
	    return uid;
	}

	if( nt_stat( Path(), &sb, DOUNICODE, LFN ) >= 0 )
	    uid = sb.st_uid;
	return uid;
}

bool
FileIO::HasOnlyPerm( FilePerm perms )
{
# ifdef false
	/*
	 * This code does not work on windows since the
	 * windows does not handle the notion of group and world
	 * permissions in the same way unix does.  Brent is looking
	 * into seeing if there is a way to assure security on
	 * the credentials directory and file.  For now commented out.
	 */
	struct statbL sb;
	int modeBits = 0;

	if( nt_stat( Path(), &sb, DOUNICODE, LFN ) < 0 )
	    return false;

	switch (perms)
	{
	case FPM_RO:
	    modeBits = PERM_0222;
	    break;
	case FPM_RW:
	    modeBits = PERM_0666;
	    break;
	case FPM_ROO:
	    modeBits = PERM_0400;
	    break;
	case FPM_RXO:
	    modeBits = PERM_0500;
	    break;
	case FPM_RWO:
	    modeBits = PERM_0600;
	    break;
	case FPM_RWXO:
	    modeBits = PERM_0700;
	    break;
	}
	/*
	 * In this case we want an exact match of permissions
	 * We don't want to "and" to a mask, since we also want
	 * to verify that the other bits are off.
	 */
	if( (sb.st_mode & PERMSMASK) == modeBits )
	        return true;

	return false;
# else
	return true;
# endif //ifdef 0
}

enum StatType{ 
	CREATE_TIME,
	ACCESS_TIME,
	MODIFY_TIME
};	

# ifdef OS_MINGW
static P4INT64
nt_getLastStatTime( HANDLE hFile, StatType statType, int &msec )
{
	// Convert file timestamp to local time, then to time_t.
	// This is because MINGW doesn't have _mkgmtime, but does have mktime.
	SYSTEMTIME st;
	SYSTEMTIME stUTC;
	struct tm u_tm;
	FILETIME cTime, aTime, mTime;
	BOOL bRet;

	if (hFile == INVALID_HANDLE_VALUE)
	    return -1;
	// Avoid leaking the handle.
	bRet = GetFileTime( hFile, &cTime, &aTime, &mTime );
	CloseHandle( hFile );
	if( !bRet )
	    return -1;

        switch( statType )
	{
	    case CREATE_TIME: 
		FileTimeToSystemTime( &cTime, &stUTC );
	        break;
	    case ACCESS_TIME: 
		FileTimeToSystemTime( &aTime, &stUTC );
	        break;
	    case MODIFY_TIME: 
		FileTimeToSystemTime( &mTime, &stUTC );
	        break;
	    default:
	        return -1;
	}

	SystemTimeToTzSpecificLocalTime( NULL, &stUTC, &st );
	
	msec = st.wMilliseconds;

	u_tm.tm_sec   = st.wSecond;
	u_tm.tm_min   = st.wMinute;
	u_tm.tm_hour  = st.wHour;
	u_tm.tm_mday  = st.wDay;
	u_tm.tm_mon   = st.wMonth - 1;
	u_tm.tm_year  = st.wYear - 1900;
	u_tm.tm_wday  = 0;
	u_tm.tm_yday  = 0;
	u_tm.tm_isdst = 0;

	return ( DateTime::Centralize( ::mktime( &u_tm ) ) );
}

# else

// This code corrected a DST datetime problem, job039200
// As of Visual Studio 2013, the problem has been fixed.
// We must keep this fix since we still build with older Visual Studios.
//
// msec is in milliseconds.
//
static P4INT64
nt_getLastStatTime( HANDLE hFile, StatType statType, int &msec )
{
	SYSTEMTIME st;
	struct tm u_tm;
	FILETIME cTime, aTime, mTime;
	BOOL bRet;

	if (hFile == INVALID_HANDLE_VALUE)
	    return -1;
	// Avoid leaking the handle.
	bRet = GetFileTime( hFile, &cTime, &aTime, &mTime );
	CloseHandle( hFile );
	if( !bRet )
	    return -1;

        switch( statType )
	{
	    case CREATE_TIME: 
		FileTimeToSystemTime( &cTime, &st );
	        break;
	    case ACCESS_TIME: 
		FileTimeToSystemTime( &aTime, &st );
	        break;
	    case MODIFY_TIME: 
		FileTimeToSystemTime( &mTime, &st );
	        break;
	    default:
	        return -1;
	}

	
	msec = st.wMilliseconds;

	u_tm.tm_sec   = st.wSecond;
	u_tm.tm_min   = st.wMinute;
	u_tm.tm_hour  = st.wHour;
	u_tm.tm_mday  = st.wDay;
	u_tm.tm_mon   = st.wMonth - 1;
	u_tm.tm_year  = st.wYear - 1900;
	u_tm.tm_wday  = 0;
	u_tm.tm_yday  = 0;
	u_tm.tm_isdst = 0;

	return ( DateTime::Centralize( ::_mkgmtime( &u_tm ) ) );
}

# endif

P4INT64
FileIO::StatAccessTime()
{
	HANDLE fH;
	StrPtr *fname = Path();
	int msecs = 0;

	if( DOUNICODE || LFN )
	{
	    // nt_openHandleW() does the unicode filename translation.
	    if( nt_islink( fname, NULL, DOUNICODE, LFN ) > 0 )
	        fH = nt_openHandleW( fname, LFN );
	    else
	        fH = nt_openDirHandleW( fname, LFN );
	    if( fH != INVALID_HANDLE_VALUE )
	        return nt_getLastStatTime( fH, ACCESS_TIME, msecs );

	    // We know LFN can not fall through and succeed.
	    // Unicode case continues to fall through.
	    if( LFN )
	        return -1;
	}

	if( nt_islink( fname, NULL, DOUNICODE, LFN ) > 0 )
	    fH = nt_openDirHandle( fname->Text() );
	else
	    fH = nt_openHandle( fname->Text() );
	return nt_getLastStatTime( fH, ACCESS_TIME, msecs );
}

P4INT64
FileIO::StatModTime()
{
	// Skip the per-file CreateFile when the lean-walk caller
	// (clientservicer.cc SetDigestOrType) has already stamped this from
	// the preceding Stat().  Cache is per-FileSys, invalidated on Set().

	if( allowStatCache && statCacheValid )
	    return cachedModTime;

	HANDLE fH;
	StrPtr *fname = Path();
	int msecs = 0;

	if( DOUNICODE || LFN )
	{
	    // nt_openHandleW() does the unicode filename translation.
	    if( nt_islink( fname, NULL, DOUNICODE, LFN ) > 0 )
	        fH = nt_openHandleW( fname, LFN );
	    else
	        fH = nt_openDirHandleW( fname, LFN );
	    if( fH != INVALID_HANDLE_VALUE )
	        return nt_getLastStatTime( fH, MODIFY_TIME, msecs );

	    // We know LFN can not fall through and succeed.
	    // Unicode case continues to fall through.
	    if( LFN )
	        return -1;
	}

	if( nt_islink( fname, NULL, DOUNICODE, LFN ) > 0 )
	    fH = nt_openDirHandle( fname->Text() );
	else
	    fH = nt_openHandle( fname->Text() );
	return nt_getLastStatTime( fH, MODIFY_TIME, msecs );
}

void
FileIO::StatModTimeHP(DateTimeHighPrecision *modTime)
{
	HANDLE fH;
	StrPtr *fname = Path();

	time_t	seconds;
	int	msecs = 0;

	if( DOUNICODE || LFN )
	{
	    // nt_openHandleW() does the unicode filename translation.
	    if( nt_islink( fname, NULL, DOUNICODE, LFN ) > 0 )
	        fH = nt_openHandleW( fname, LFN );
	    else
	        fH = nt_openDirHandleW( fname, LFN );

	    if( fH != INVALID_HANDLE_VALUE )
	    {
	        seconds = nt_getLastStatTime( fH, MODIFY_TIME, msecs );
	        *modTime = DateTimeHighPrecision( seconds, msecs * 1000000 );
	        return;
	    }

	    // We know LFN can not fall through and succeed.
	    // Unicode case continues to fall through.
	    if( LFN )
	    {
	        *modTime = DateTimeHighPrecision();
	        return;
	    }
	}

	if( nt_islink( fname, NULL, DOUNICODE, LFN ) > 0 )
	    fH = nt_openDirHandle( fname->Text() );
	else
	    fH = nt_openHandle( fname->Text() );
	
	seconds = nt_getLastStatTime( fH, MODIFY_TIME, msecs );
	*modTime = DateTimeHighPrecision( seconds, msecs * 1000000 );
}

void
FileIO::Chmod( FilePerm perms, Error *e )
{
	// Don't set perms on symlinks

	if( ( GetType() & FST_MASK ) == FST_SYMLINK )
	    return;

	// Permissions for readonly/readwrite, exec vs no exec

	int bits = IsExec() ? PERM_0777 : PERM_0666;

	switch( perms )
	{
	case FPM_RO: bits &= ~PERM_0222; break;
	case FPM_ROO: bits &= ~PERM_0266; break;
	case FPM_RWO: bits = PERM_0600; break; // for key file, set exactly to rwo
	case FPM_RXO: bits = PERM_0500; break;
	case FPM_RWXO: bits = PERM_0700; break;
	}

	if( nt_chmod( Path(), bits & ~global_umask, DOUNICODE, LFN ) >= 0 )
	    return;

	// Can be called with e==0 to ignore error.

	if( e )
	    e->Sys( "chmod", Name() );
}

void
FileIO::SetAttribute( FileSysAttr attrs, Error *e )
{
	int flags = 0;

	if( ( attrs & FSA_HIDDEN ) == FSA_HIDDEN )
	    flags |= FILE_ATTRIBUTE_HIDDEN;


	if( nt_setattr( Path(), flags, DOUNICODE, LFN ) >= 0 )
	    return;

	// Can be called with e==0 to ignore error.

	if( e )
	    e->Sys( "SetFileAttribute", Name() );
}

void
FileIO::SetExtendedAttribute( StrPtr *name, StrPtr *val, Error *e )
{
	long res = nt_setea( Path(), name, val, DOUNICODE, LFN );
	if( !res )
	    return;

	SetLastError( res );

	// Can be called with e==0 to ignore error.
	if( e )
	    e->Sys( "SetExtendedAttribute", Name() );
}

void
FileIO::SetExtendedAttributes( StrDict *vals, Error *e )
{
	long res = nt_setea( Path(), vals, DOUNICODE, LFN );
	if( !res )
	    return;

	SetLastError( res );

	// Can be called with e==0 to ignore error.
	if( e )
	    e->Sys( "SetExtendedAttributes", Name() );
}

void
FileIO::GetExtendedAttribute( StrPtr *name, StrBuf *val, Error *e )
{
	val->Clear();
	StrBufDict attrs;
	StrPtr *tmp = 0;
	GetExtendedAttributes( &attrs, e );
	if( !e->Test() )
	    tmp = attrs.GetVar( name->Text(), e );
	if( !e->Test() && tmp )
	    *val = *tmp;
}

void
FileIO::GetExtendedAttributes( StrBufDict *attrs, Error *e )
{
	long res = nt_getea( Path(), attrs, DOUNICODE, LFN );
	if( !res )
	    return;

	SetLastError( res );

	// Can be called with e==0 to ignore error.

	if( e )
	    e->Sys( "GetExtendedAttributes", Name() );
}

void
FileIOBinary::Open( FileOpenMode mode, Error *e )
{
	this->lastOSError = 0;
	// Save mode for write, close

	this->mode = mode;

	// Get bits for (binary) open
	// bflags is always O_BINARY

	int bits = openModes[ mode ].bflags;

	// Reset the isStd flag

	isStd = 0;

	// Handle exclusive open (must not already exist)

# ifdef O_EXCL
	// Set O_EXCL to ensure we create the file when we open it.

	if( GetType() & FST_M_EXCL )
	    bits |= O_EXCL;
# else
	// No O_EXCL: we'll manually check if file already exists.
	// Not atomic, but what can one do?

	if( GetType() & FST_M_EXCL && Stat() & FSF_EXISTS )
	{
	    e->Set( E_FAILED, "file exists" );

	    // if file is set delete on close unset that because we
	    // didn't create the file...
	    ClearDeleteOnClose();
	    return;
	}
# endif // O_EXCL

	// open delegate buffer, stdin/stdout or real file

	if( delegate )
	{
	    delegate->Open( Path(), mode, e );
	}
	else if( Name()[0] == '-' && !Name()[1] )
	{
	    // we do raw output: flush stdout
	    // for nice mixing of messages.

	    if( mode == FOM_WRITE )
	        fflush( stdout );

	    fd = nt_getStdHandle(openModes[ mode ].standard, bits);
	    if( fd == FD_ERR )
	        e->Sys( openModes[ mode ].modeName, Name() );

	    // checkStdio() just returns on Windows, fd doesn't matter.
	    //
	    checkStdio( (FD_TYPE)fd );
	    isStd = 1;
	}
	else
	{
	    // checkFd() just returns on Windows, fd doesn't matter.
	    //
	    if( (fd = checkFd( nt_open( Path(), bits, PERM_0666,
	        DOUNICODE, LFN ) ) ) == FD_ERR)
	    {
	        this->lastOSError = ::GetLastError();
	        e->Sys( openModes[ mode ].modeName, Name() );
# ifdef O_EXCL
	        // if we failed to create the file probably due to the
	        // file already existing (O_EXCL)
	        // then unset delete on close because we didn't create it...
	        if( ( bits & (O_EXCL|O_CREAT) ) == (O_EXCL|O_CREAT) )
	            ClearDeleteOnClose();
# endif
	    }
	}


	if( e->Test() )
	    return;

	// Do we need to preallocate (fragmentation ?)

	offL_t sizeOffSet = GetSizeHint();

	if( delegate )
	{
	    delegate->SizeHint( sizeOffSet );
	}
	else if( sizeOffSet )
	{
	    FileIOBinary::Seek( sizeOffSet - (offL_t)1, e );

	    if( !e->Test() )
	    {
	        char endFile = 0;
	        FileIOBinary::Write( &endFile, 1, e );
	        FileIOBinary::Seek( (offL_t)0, e );
	    }
	}
}

void
FileIOBinary::Close( Error *e )
{
	if( delegate )
	{
	    delegate->Close( e );
	    return;
	}

	if( isStd )
	    return;

	if( fd == FD_ERR )
	{
# ifdef PPOSSIBLE_FUTURE_ERROR_REPORT
	    // A failed open with out error checking or double
		// close can land us here.  Right now just return
		// without reporting an error.
		//
	    SetLastError( ERROR_INVALID_HANDLE );
	    e->Sys( "Close", Name() );
# endif
	    return;
	}

	if( ( GetType() & FST_M_SYNC ) )
	    Fsync( e );

	if( nt_close( (FD_TYPE)fd ) < 0 )
	    e->Sys( "close", Name() );

	fd = FD_INIT;

	if( mode == FOM_WRITE && modTime )
	    ChmodTime( modTime, e );

	if( mode == FOM_WRITE )
	    Chmod( perms, e );
}


void
FileIOBinary::Write( const char *buf, int len, Error *e )
{
	if( delegate )
	{
	    delegate->Write( buf, len, e );

	    if( checksum && !e->Test() )
	        checksum->Update( StrRef( buf, len ) );

	    return;
	}

	if( fd == FD_ERR )
	{
	    SetLastError( ERROR_INVALID_HANDLE );
	    e->Sys( "Write", Name() );
	    return;
	}

	// Raw, unbuffered write

	int l;

	if( ( l = nt_write( Path(), (FD_TYPE)fd, buf, len ) ) < 0 )
	    e->Sys( "write", Name() );
	else
	    tellpos += l;

	if( checksum && l > 0 )
	    checksum->Update( StrRef( buf, l ) );
}

int
FileIOBinary::Read( char *buf, int len, Error *e )
{
	if( delegate )
	    return delegate->Read( buf, len, e );

	// Raw, unbuffered read

	if( fd == FD_ERR )
	{
	    SetLastError( ERROR_INVALID_HANDLE );
	    e->Sys( "Read", Name() );
	    return -1;
	}

	int l;

	if( ( l = nt_read( (FD_TYPE)fd, buf, len ) ) < 0 )
	    e->Sys( "read", Name() );
	else
	    tellpos += l;

	if ( checksum && l > 0 )
	    checksum->Update( StrRef( buf, l ) );

	return l;
}

// Return 1 if it make sense to retry a file create
// operation. Used in sys/filetmp.cc.
int
FileIOBinary::RetryCreate()
{
	if( lastOSError == ERROR_FILE_EXISTS ||
	    // We can't tell the difference between
	    // a DELETE PENDING and a real permission
	    // failure. So we have to retry.
	    // Brain dead.
	    lastOSError == ERROR_ACCESS_DENIED )
	    return 1;
	return 0;
}

offL_t
FileIOBinary::GetSize()
{
	// Reuse the size stamped on the FileSys by the most recent Stat()
	// in the lean-walk path; otherwise fall back to a fresh nt_stat.

	if( allowStatCache && statCacheValid )
	    return cachedSize;

	struct _stati64 sb;

	if( nt_stat( Path(), &sb, DOUNICODE, LFN ) < 0 )
	    return -1;

	return sb.st_size;
}

// Comparing lseek origin to SetFilePointer dwMoveMethod
// lseek	SetFilePointer
// SEEK_SET=0	FILE_BEGIN=0
// SEEK_CUR=1	FILE_CURRENT=1
// SEEK_END=2	FILE_END=2
// They match, so we can use the Posix definitions unchanged.
//
void
FileIOBinary::Seek( offL_t offset, Error *e )
{
	if( delegate )
	{
	    delegate->Seek( offset, e );
	    return;
	}

	if( fd == FD_ERR )
	{
	    SetLastError( ERROR_INVALID_HANDLE );
	    e->Sys( "Seek", Name() );
	    return;
	}

	LARGE_INTEGER offset_in;
	LARGE_INTEGER offset_out;

	// Always seek from the start of the file.

	offset_in.QuadPart = offset;
	if( SetFilePointerEx (
	        ((FD_TYPE)fd)->fh,
	        offset_in,
	        &offset_out,
	        FILE_BEGIN ) == 0)
	{
	    e->Sys( "Seek", Name() );
	}

	tellpos = (offL_t)offset_out.QuadPart;
}

// Not supported on NT.
int
FileIOBinary::LinkCount()
{
	return -1;
}

void
FileIOAppend::Open( FileOpenMode mode, Error *e )
{
	// Save mode for write, close

	this->mode = mode;
	
	// aflags for write, read/write are O_TEXT, otherwise O_BINARY

	int bits = openModes[ mode ].aflags;

	// Reset the isStd flag

	isStd = 0;

	// open stdin/stdout or real file
	if( delegate )
	{
	    delegate->Open( Path(), mode, e );
	}
	else if( Name()[0] == '-' && !Name()[1] )
	{
	    fd = nt_getStdHandle(openModes[ mode ].standard, bits);
	    if( fd == FD_ERR )
	        e->Sys( openModes[ mode ].modeName, Name() );

	    checkStdio( (FD_TYPE)fd );
	    isStd = 1;
	}
	else
	{
	    // checkFd() just returns on Windows, fd value doesn't matter.
	    //
	    if ( ( fd = checkFd( nt_open( Path(), bits,
	                        PERM_0666, DOUNICODE, LFN ) ) ) == FD_ERR )
	    {
	        e->Sys( openModes[ mode ].modeName, Name() );
	    }
	}
}

// Should work with unicode and LFN.
offL_t
FileIOAppend::GetSize( )
{
	offL_t s = 0;

	if( fd == FD_ERR )
	{
	    SetLastError( ERROR_INVALID_HANDLE );
	    return -1;
	}

	if( !lockFile( (FD_TYPE)fd, LOCKF_SH ) )
	{
	    BY_HANDLE_FILE_INFORMATION bhfi;

	    if( GetFileInformationByHandle( ((FD_TYPE)fd)->fh, &bhfi ) )
	        s = ((offL_t)(bhfi.nFileSizeHigh)) * (0x100000000LL) +
	            (offL_t)(bhfi.nFileSizeLow);

	    lockFile( (FD_TYPE)fd, LOCKF_UN );
	}
	else
	    s = FileIOBinary::GetSize();

	return s;
}

offL_t
FileIOAppend::GetCurrentSize()
{
	// This function returns the current size of the file.  If a rename
	// has occured and the file handle has followed the rename, then the
	// size must be collected by using the path and not the handle.
	//

	if( LFN & LFN_ATOMIC_RENAME )
	    return FileIOBinary::GetSize();
	else
	    return GetSize();
}

void
FileIOAppend::Write( const char *buf, int len, Error *e )
{
	if( delegate )
	{
	    delegate->Write( buf, len, e );
	    return;
	}

	if( fd == FD_ERR )
	{
	    SetLastError( ERROR_INVALID_HANDLE );
	    e->Sys( "Write", Name() );
	    return;
	}

	// We do an unbuffered write here to guarantee the atomicity
	// of the write.  Stdio might break it up into chunks, whereas
	// write() is supposed to keep it whole.

	if( lockFile( (FD_TYPE)fd, LOCKF_EX ) < 0 )
	{
	    e->Sys( "lock", Name() );
	    return;
	}

	FileIOBinary::Write( buf, len, e );

	if( lockFile( (FD_TYPE)fd, LOCKF_UN ) < 0 )
	{
	    e->Sys( "unlock", Name() );
	    return;
	}
}

void
FileIOAppend::Rename( FileSys *target, Error *e )
{
	int fallback=0;

	// File may be open, so to rename we copy 
	// and truncate FileIOAppend files on NT.
	//
	// But if a FileIOAppend file on Windows is ever instead renamed
	// using rename() semantics, the FileIOAppend::GetCurrentSize()
	// function on Windows might need changed so that it returns
	// the correct size of a current file by path, not of a recently
	// rename()'d file that still happens to be open on this->fd.

	// Check for cross volume rename.  If the Rename is cross volume
	// an atomic rename can not be used.  Revert to old bahavior.
	//
	int volchk = nt_volumechk( Name(), target->Name() );

	// The caller determines if an atomic rename is used.
	// Right now this is just for journal operations.

	if( volchk && (LFN & LFN_ATOMIC_RENAME) )
	{
	    FileIO::Rename( target, e );

	    // Report the non-fatal error.
	    if( e->Test() )
	    {
	       // Clear out previous rename errors, they should be moot.
	        e->Clear();

	        e->Set( MsgOs::AtomicRenameFailed ) << target->Name();
	        AssertLog.Report( e );
	        e->Clear();

	        fallback++;
	    }
	}
	else
	    fallback++;

	if( fallback )
	{
	    if( p4debug.GetLevel( DT_SERVER ) >= 4 )
	        p4debug.printf(
	            "Rename failed, doing copy/truncate instead: %s %s %d\n",
	            Name(), target->Name(), volchk );

	    Copy( target, FPM_RO, e );

	    if( e->Test() )
	        return;

	    Truncate( e );
	}
}

// Initialize both multibyte and wide char operations.
int
FileSys::SymlinksSupported()
{
	if( !functionHandlesLoaded)
	{
	    functionHandlesLoaded = 1;

	    CreateSymbolicLinkA_func = (CreateSymbolicLinkAProc)
	            GetProcAddress(
	                GetModuleHandle("kernel32.dll"),
	                "CreateSymbolicLinkA");

	    CreateSymbolicLinkW_func = (CreateSymbolicLinkWProc)
	            GetProcAddress(
	                GetModuleHandle("kernel32.dll"),
	                "CreateSymbolicLinkW");

	    if( CreateSymbolicLinkA_func != 0 &&
	        CreateSymbolicLinkW_func != 0 )
	    {
	        const char *tempdir = getenv("TEMP");
	        if( !tempdir )
	        {
	            CreateSymbolicLinkA_func = 0;
	            CreateSymbolicLinkW_func = 0;
	            return 0;
	        }
	        StrBuf testLink;
	        StrBuf testTarget;
	        testLink << tempdir << "\\p4_test_symlink";
	        testTarget << tempdir << "\\p4_test_target";
	        nt_chmod( &testLink, PERM_0666  & ~global_umask, 0, 0 );
	        nt_unlink( &testLink, 0, 0 );
	        int result = nt_makelink( testTarget, &testLink, 0, 0 );
	        nt_unlink( &testLink, 0, 0 );
	        if( result < 0 )
	        {
	            CreateSymbolicLinkA_func = 0;
	            CreateSymbolicLinkW_func = 0;
	        }
	    }
	}
	return CreateSymbolicLinkA_func != 0;
}

