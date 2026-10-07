/*
 * Copyright 1995, 2023 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# define NEED_WINDOWSH
# define NEED_ERRNO
# include <stdhdrs.h>

# ifdef HAS_CPP11

# include <debug.h>
# include <tunable.h>
# include <strbuf.h>
# include <strops.h>
# include <error.h>
# include <filesys.h>
# include <datetime.h>
# include <proc_utils.h>
# include <pressuremonitor.h>
# include <sysinfo.h>
# include "hostcfg.h"

# if defined( OS_LINUX )
# include <sys/prctl.h>
# endif

////////////////////////////////////////////////////////////////////////////////

bool HostCfg::hasOOMAdj = false;
bool HostCfg::canOOMAdj = false;
bool HostCfg::tryOOMAdj = true;
// Set a reasonable default for this in case for some odd reason we
// failed to read the real value normally.
StrBuf HostCfg::initialOOMScoreAdj = "0";
// E.g. "-1000\n".
StrBuf HostCfg::hiddenOOMScoreAdj;
StrBuf HostCfg::oomScoreAdjFile = "/proc/self/oom_score_adj";

////////////////////////////////////////////////////////////////////////////////

bool HostCfg::hasCGroups = false;
bool HostCfg::hasCGroupsV1 = false;
bool HostCfg::hasCGroupsV2 = false;
bool HostCfg::isCGroupsV1 = false;
bool HostCfg::isCGroupsV2 = false;
bool HostCfg::hasPSI = false;
bool HostCfg::canPSI = false;

std::vector< HostCfg::cgInfo > HostCfg::cgInfos;

StrBuf HostCfg::processCGroupFile = "/proc/self/cgroup";
bool HostCfg::canUseCGroupFile = false;

////////////////////////////////////////////////////////////////////////////////

HostCfg::HostCfg()
{
}

void HostCfg::Discover( Error* e )
{
# if defined( OS_LINUX )
	DiscoverOOM();
	DiscoverCGroup( e );
	DiscoverPSI();
# else
	DiscoverPSI(); // Windows
# endif
}

void HostCfg::Dump( StrBuf& buf )
{
	StrBuf hiddenScoreBuf;
	// We want the output all on a single line and to preserve the
	// existence of the newline in this variable.
	StrOps::EncodeNonPrintable( hiddenOOMScoreAdj, hiddenScoreBuf );

	buf << "HostCfg::Dump(): "
	    << "OOMScoreAdjFile() '"      <<       OOMScoreAdjFile() << "', "
	    << "tryOOMAdj "               <<               tryOOMAdj << ", "
	    << "HasOOMScoreAdj() "        <<        HasOOMScoreAdj() << ", "
	    << "CanOOMScoreAdj() "        <<        CanOOMScoreAdj() << ", "
	    << "hiddenOOMScoreAdj "       <<          hiddenScoreBuf << ", "
	    << "GetInitialOOMScoreAdj() " << GetInitialOOMScoreAdj() << ", "
	    << "hasCGroups "              <<              hasCGroups << ", "
	    << "hasCGroupsV1 "            <<            hasCGroupsV1 << ", "
	    << "hasCGroupsV2 "            <<            hasCGroupsV2 << ", "
	    << "isCGroupsV1 "             <<             isCGroupsV1 << ", "
	    << "isCGroupsV2 "             <<             isCGroupsV2 << ", "
	    << "hasPSI "                  <<                  hasPSI << ", "
	    << "canPSI "                  <<                  canPSI << ", "
	    << "processCGroupFile '"      <<       processCGroupFile << "' "
	    << "cgInfos ";

	for( const auto& t : cgInfos )
	    buf << "[ '" << std::get< 0 >( t ) << "', '"
	        << std::get< 1 >( t ) << "', '"
	        << std::get< 2 >( t ) << "' ] ";

	buf << "\n";
}

////////////////////////////////////////////////////////////////////////////////

const void HostCfg::GetCurrentOOMScoreAdj( StrBuf& buf, Error* e ) const
{
	FileSysUPtr osa = FileSys::CreateUPtr( FST_TEXT );
	osa->Set( OOMScoreAdjFile()->Text() );
	osa->ReadFile( &buf, e );
}

void HostCfg::DiscoverOOM()
{
	if( !tryOOMAdj )
	    return;

	// Errors are thrown away here since this function is just
	// supposed to say if we can or can't manipulate the OOM
	// adjustment file.

	Error eIgnore;
	Error* e = &eIgnore;

	FileSysUPtr osa = FileSys::CreateUPtr( FST_TEXT );
	osa->Set( OOMScoreAdjFile()->Text() );

	const int oomStat = osa->Stat();

	if( !( oomStat & FSF_EXISTS ) )
	    return;

	// The file at least exists.

	hasOOMAdj = true;

	// If it's not writeable, we won't try anything else.

	if( !( oomStat & FSF_WRITEABLE ) )
	   return;

	StrBuf buf;
	osa->ReadFile( &buf, e );

	if( e->Test() )
	    return;

	StrOps::StripNewline( buf );

	initialOOMScoreAdj = buf;

	// Now check if the Capabilities are set correctly.

	OSProcessUtils pu;

	// Note that the test hide actually enables the proection temporarily.

	if( !pu.TestHideFromOOMKiller() )
	    return;

	canOOMAdj = true;

	return;
}

const StrPtr* HostCfg::OOMScoreAdjFile()
{
	return &oomScoreAdjFile;
}

const StrPtr* HostCfg::GetInitialOOMScoreAdj()
{
	return &initialOOMScoreAdj;
}

const StrPtr* HostCfg::GetHiddenOOMScoreAdj()
{
	return &hiddenOOMScoreAdj;
}

bool HostCfg::HasOOMScoreAdj()
{
	return hasOOMAdj;
}

bool HostCfg::CanOOMScoreAdj()
{
	return canOOMAdj;
}

void HostCfg::ConfigureOOMAdjust( const int score, Error* e )
{
# if defined( OS_LINUX )
	if( score == 0 )
	{
	    tryOOMAdj = false;
	    return;
	}

	hiddenOOMScoreAdj << score << "\n";

	// Making the process dumpable is a prerequisite to
	// being able to write to /proc/self/oom_score_adj
	// as a non-root user.  The other is to have this set:
	// "setcap 'CAP_SYS_RESOURCE=+ep' p4d"

	const int prrc = prctl( PR_SET_DUMPABLE, 1, 0, 0, 0 );

	if( prrc == -1 )
	{
	    e->Sys( "HostCfg::ConfigureOOMAdjust prctl ", strerror( errno ) );
	    return;
	}
# endif
}

////////////////////////////////////////////////////////////////////////////////

void HostCfg::DiscoverPSI()
{
# if defined( OS_LINUX )

	if( !HasCGroups() )
	    return;

	// We only support the global pressure file for now.  In a
	// mixed-configuration setup, the V2 pressure file may be present
	// and usable when the rest is detected as V1.

	StrBuf path = GetGlobalCGroupV2MemFile();

	hasPSI = FileSys::FileExists( path.Text() );

# elif OS_NT
	// All supported versions of Windows have the APIs for this.
	hasPSI = true;
# endif

	if( !hasPSI )
	    return;

	// Now actually try to open and use the monitoring.  Can fail
	// due to permissions (e.g. the defaults on the memory file).

	Error ee;
	Error* e = &ee;

# ifdef OS_NT
	auto pm = PressureMonitor::MkPressureMonitor(
	              PressureMonitor::Resource::MEMORY,
	              PressureMonitor::Type::HIGH, e );
# endif

# ifdef OS_LINUX

	// See if the memory pressure file is readable.
	// Assume if it is, that the CPU file also is.

	FileSysUPtr memFile = FileSys::CreateUPtr( FST_TEXT );
	memFile->Set( path );
	memFile->Open( FOM_READ, e );

	if( e->Test() )
	    return;

	StrBuf buf;
	memFile->ReadWhole( &buf, e );

# endif

	if( e->Test() )
	    return;

	canPSI = true;
}

void HostCfg::DiscoverCGroup( Error* e )
{
# if defined( OS_LINUX )
	hasCGroups = FileSys::FileExists( processCGroupFile.Text() );
	hasCGroupsV1 = FileSys::FileExists( "/sys/fs/cgroup/memory" );
	hasCGroupsV2 = FileSys::FileExists(
	                   "/sys/fs/cgroup/cgroup.controllers" );
	isCGroupsV1 = hasCGroupsV1 && !hasCGroupsV2;
	isCGroupsV2 = hasCGroupsV2 && !hasCGroupsV1;

	if( hasCGroups )
	    ParseCGroupDef( e );

	if( e->Test() )
	{
	    e->Sys( "HostCfg::Inspect", "parse cgroup def" );
	    return;
	}
# endif // OS_LINUX
}

bool HostCfg::HasCGroups()
{
	return hasCGroups;
}

bool HostCfg::HasCGroupV1()
{
	return hasCGroupsV1;
}

bool HostCfg::HasCGroupV2()
{
	return hasCGroupsV2;
}

bool HostCfg::IsCGroupV1()
{
	return isCGroupsV1;
}

bool HostCfg::IsCGroupV2()
{
	return isCGroupsV2;
}

void HostCfg::ParseCGroupDef( Error* e )
{
/*
	v1

	    cat /proc/self/cgroup
	    11:blkio:/user.slice
	    10:hugetlb:/
	    9:memory:/user.slice
	    8:cpuset:/
	    7:cpu,cpuacct:/user.slice
	    6:devices:/user.slice
	    5:perf_event:/
	    4:pids:/user.slice/user-1000.slice
	    3:net_cls,net_prio:/
	    2:freezer:/
	    1:name=systemd:/user.slice/user-1000.slice/session-9427.scope

	v2

	    cat /proc/self/cgroup
	    0::/user.slice/user-1000.slice/session-1.scope
*/

	FileSysUPtr f = FileSys::CreateUPtr( FST_TEXT );
	f->Set( processCGroupFile );
	f->Open( FOM_READ, e );

	if( e->Test() )
	{
	    f->Close( e );
	    return;
	}

	// hierarchy:subsystems:group

	StrBuf line;

	while( f->ReadLine( &line, e ) && !e->Test() )
	{
	    char* eoh = strchr( line.Text(), ':' );

	    if( !eoh )
	    {
	        e->Sys( "HostCfg::ParseCGroupDef eoh: ", line.Text() );
	        return;
	    }

	    *eoh = '\0';
	    char* eos = strchr( eoh + 1, ':' );

	    if( !eos )
	    {
	        e->Sys( "HostCfg::ParseCGroupDef eos: ", line.Text() );
	        return;
	    }

	    *eos = '\0';
	    cgInfos.emplace_back(
	        std::make_tuple( line.Text(), eoh + 1, eos + 1 ) );
	}
};

StrBuf HostCfg::GetProcessCGroupV1XDir( const char* subsystem )
{
	StrBuf path = "/sys/fs/cgroup";

	for( const auto& t : cgInfos )
	    if( std::get< 1 >( t ) == subsystem )
	    {
	        path << "/" << subsystem << std::get< 2 >( t );
	        return path;
	    }

	// Default to the whole-system file.
	path << subsystem << "/";
	return path;
}

StrBuf HostCfg::GetProcessCGroupV1MemoryDir()
{
	return GetProcessCGroupV1XDir( "memory" );
}

StrBuf HostCfg::GetGlobalCGroupV1MemFile()
{
	return StrBuf( "/sys/fs/cgroup/memory/memory.pressure_level" );
}

StrBuf HostCfg::GetProcessCGroupV2Dir()
{
	StrBuf path = "/sys/fs/cgroup";
	// /proc/self/cgroup should only contain the following in V2:
	// 0::/user.slice/user-1000.slice/session-1.scope
	path << std::get< 2 >( cgInfos.at( 0 ) );
	return path;
}

StrBuf HostCfg::GetProcessCGroupV2CPUFile()
{
	StrBuf path = GetProcessCGroupV2Dir();
	path << "/cpu.pressure";
	return path;
}

StrBuf HostCfg::GetProcessCGroupV2MemFile()
{
	StrBuf path = GetProcessCGroupV2Dir();
	path << "/memory.pressure";
	return path;
}

StrBuf HostCfg::GetGlobalCGroupV2CPUFile()
{
	StrBuf path;
	path << "/proc/pressure/cpu";
	return path;
}

StrBuf HostCfg::GetGlobalCGroupV2MemFile()
{
	StrBuf path;
	path << "/proc/pressure/memory";
	return path;
}

bool HostCfg::HasPSI()
{
	return hasPSI;
}

bool HostCfg::CanPSI()
{
	return canPSI;
}

////////////////////////////////////////////////////////////////////////////////

# endif // CPP_11
