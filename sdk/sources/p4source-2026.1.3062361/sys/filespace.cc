/*
 * Copyright 1995, 1996 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# define NEED_SLEEP
# define NEED_STAT
# define NEED_STATFS
# define NEED_STATVFS

# include <stdhdrs.h>

# include <error.h>
# include <strbuf.h>
# include <strops.h>
# include <debug.h>
# include <tunable.h>

# include "filesys.h"
# include "pathsys.h"

/*
 * FileSys::GetDiskSpace -- fill in details about disk space usage.
 */

DiskSpaceInfo::DiskSpaceInfo()
{
	this->fsType = new StrBuf();
	this->mountpoint = new StrBuf();
	blockSize = totalBytes = usedBytes = freeBytes = 0;
	pctUsed = 0;
}

DiskSpaceInfo::~DiskSpaceInfo()
{
	delete this->fsType;
	delete this->mountpoint;
}

void
FileSys::GetDiskSpace( DiskSpaceInfo *info, Error *e )
{
	info->fsType->Set( "unknown" );
	info->mountpoint->Set( "unknown" );

# ifdef OS_NT
	char buffer[1024];
	char *lpp;
	int boff = 0;

	if( !GetFullPathName( Name(), sizeof( buffer ), buffer, &lpp ) )
	{
	    e->Sys( "GetFullPathName", Name() );
	    return;
	}
	if( lpp )
	    *lpp = '\0';

	// Walk up to get a real file
	PathSys *ps = PathSys::Create();
	Set( buffer );
	ps->Set( Name() );
	while( !(Stat() & FSF_EXISTS) && ps->ToParent() )
	    Set( ps->Text() );
	delete ps;

	// Resolve symlinks
	HANDLE hFile = CreateFile( Name(), // file to open
	                           0,      // don't need to read
	                           0,      // don't need to share
	                           NULL,   // default security
	                           OPEN_EXISTING, // existing file only
	                           FILE_FLAG_BACKUP_SEMANTICS, // open dirs
	                           NULL ); // no attr. template

	if( hFile != INVALID_HANDLE_VALUE )
	{
	    DWORD dwRet = GetFinalPathNameByHandle( hFile,
	                                            buffer, sizeof( buffer ),
	                                            VOLUME_NAME_DOS );
	    if( dwRet < sizeof( buffer ) )
	    {
	        if( IsExtended( StrRef( buffer ) ) )
	            boff = 4;
	        Set( buffer + boff );
	        CloseHandle( hFile );
	    }
	    else
	    {
	        e->Sys( "GetFinalPathNameByHandle", Name() );
	        CloseHandle( hFile );
	        return;
	    }
	}
	else
	{
	    e->Sys( "CreateFile", Name() );
	    CloseHandle( hFile );
	    return;
	}

	char *which = Name();
	if( !strncmp( Name(), "UNC\\", 4 ) )
	{
	    // Network mounts turn into UNC paths
	    StrBuf tmp( "\\\\" );
	    tmp.Append( Name() + 4 );
	    if( !tmp.EndsWith( "\\", 1 ) )
	        tmp << "\\";
	    Set( tmp );
	}
	else if( buffer[1 + boff] == ':' )
	{
	    // buffer "should" only have the drive letter a colon and a slash
	    // but no point in not making absolutely certain
	    buffer[2 + boff] = '\\';
	    buffer[3 + boff] = '\0';
	    which = buffer + boff;
	}

	ULARGE_INTEGER freeBytesAvailable;
	ULARGE_INTEGER totalNumberOfBytes;
	ULARGE_INTEGER totalNumberOfFreeBytes;

	if( !GetDiskFreeSpaceEx( Name(),
				&freeBytesAvailable,
				&totalNumberOfBytes,
				&totalNumberOfFreeBytes ) )
	{
	    e->Sys( "GetDiskFreeSpaceEx", Name() );
	    return;
	}
	info->blockSize = -1;
	info->freeBytes = freeBytesAvailable.QuadPart;
	info->totalBytes= totalNumberOfBytes.QuadPart;

	char vName[1024];
	char fsName[1024];
	if( !GetVolumeInformation( which, 
	                           vName, sizeof( vName ),
	                           (LPDWORD)0, (LPDWORD)0, (LPDWORD)0,
	                           fsName, sizeof( fsName) ) )
	{
	    e->Sys( "GetVolumeInformation", which );
	    return;
	}

	info->fsType->Set( fsName );
	info->mountpoint->Set( which );
	StrOps::Lower( *info->mountpoint );
	info->usedBytes = info->totalBytes - info->freeBytes;
	double usage = 1.0;
	if( info->totalBytes > 0 )
	    usage = (double)info->usedBytes / (double)info->totalBytes;
	info->pctUsed = (int)( usage * 100 );
# else
	if( !strchr( Name(), '/' ) )
	{
	    StrBuf nm;
	    nm << "./" << Name();
	    Set( nm );
	}
	PathSys *ps = PathSys::Create();
	ps->Set( Name() );
	ps->ToParent();
	Set( ps->Text() );
	while( !( Stat() & FSF_EXISTS ) && ps->ToParent() )
		Set( ps->Text() );
	delete ps;
	struct statvfs df;

	if( statvfs( Name(), &df ) == -1 )
	{
	    e->Sys( "statvfs", Name() );
	    return;
	}

	info->blockSize = df.f_frsize;
	info->freeBytes  = (P4INT64) ( (double) df.f_frsize * df.f_bfree  );
	info->totalBytes = (P4INT64) ( (double) df.f_frsize * df.f_blocks );
	info->usedBytes  = info->totalBytes - info->freeBytes;
	info->freeBytes  = (P4INT64) ( (double) df.f_frsize * df.f_bavail );
	double usage = 1.0;
	if( info->totalBytes > 0 )
	    usage = (double)info->usedBytes /
	            (double)(info->usedBytes + info->freeBytes);
	info->pctUsed = (int)( usage * 100 );
	
	// Note that used + free may not equal total, and also note that
	// used/total does not match pctUsed. This is because
	// the filesystem may also have 'reserved' space, which is
	// available only to privileged users, and we are attemptint
	// to return the non-privileged information, like df does.

# ifdef HAVE_STATVFS_BASETYPE
	info->fsType->Set( df.f_basetype );
# endif

# endif // !OS_NT

# ifdef HAVE_STATFS
	struct statfs sys_fs;

	if( statfs( Name(), &sys_fs ) == -1 )
	{
	    e->Sys( "statfs", Name() );
	    return;
	}

# ifdef HAVE_STATFS_FSTYPENAME
	info->fsType->Set( sys_fs.f_fstypename );
	info->mountpoint->Set( sys_fs.f_mntonname );
# else
	switch( sys_fs.f_type )
	{
	case 0x6969: 
	    info->fsType->Set( "nfs" );
	    break;
	case 0xEF53: 
	    info->fsType->Set( "ext2" );
	    break;
	case 0x58465342: 
	    info->fsType->Set( "xfs" );
	    break;
	case 0x1021994: 
	    info->fsType->Set( "tmpfs" );
	    break;
	case 0x858458f6: 
	    info->fsType->Set( "ramfs" );
	    break;
	case 0x00011954: 
	    info->fsType->Set( "ufs" );
	    break;
	case 0x52654973: 
	    info->fsType->Set( "reiserfs" );
	    break;
	default:
	    info->fsType->Set( StrNum( (P4INT64) sys_fs.f_type ) );
	    break;
	}
# endif // !HAVE_STATFS_FSTYPENAME

# endif // HAVE_STATFS

# ifdef OS_LINUX
	// On Linux we get the device ID from stat, which we can look up
	// from /proc/self/mountinfo

	struct stat st;
	if( stat( Name(), &st ) )
	{
	    e->Sys( "stat", Name() );
	    return;
	}
	
	StrBuf mountId;
	mountId << major(st.st_dev) << ":" << minor(st.st_dev);

	StrBuf mounts;
	FileSys *mpf = FileSys::Create( FST_TEXT );
	mpf->Set( "/proc/self/mountinfo" );
	if( mpf->Stat() & FSF_EXISTS )
	    mpf->ReadFile( &mounts, e );
	delete mpf;
	if( e->Test() )
	{
	    e->Sys( "FileSys::ReadFile", "/proc/self/mountinfo" );
	    return;
	}
	
	char *p = mounts.Text();
	char *n = p;
	while( p && ( n = strchr( p, '\n' ) ) )
	{
	    StrBuf line(StrRef(p, n-p));

	    StrBuf tmp;
	    char* words[12];
	    int w = StrOps::Words(tmp, line.Text(), words, 12, ' ');
	    if( w > 4 && !strcmp( words[2], mountId.Text() ) )
	    {
	        info->mountpoint->Set( words[4] );
	        for( int i = 6; i < w; i++ )
	            if( !strcmp( words[i - 1], "-" ) )
	            {
	                info->fsType->Set( words[i] );
	                break;
	            }

	        // We've got the best response we can expect: return now
	        break;
	    }
	    p = n + 1;
	}
# endif
}
