/*
 * Copyright 1995, 2023 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# define NEED_WINDOWSH
# define NEED_ERRNO
# include <stdhdrs.h>

# ifdef HAS_CPP11

# include <error.h>
# include <errorlog.h>
# include <strbuf.h>
# include <debug.h>
# include <tunable.h>
# include <strdict.h>
# include <strtable.h>
# include <filesys.h>
# include <filestrbuf.h>
# include <fblreader.h>
# include <hostcfg.h>
# include <runcmd.h>
# include <pid.h>
# include <datetime.h>

# define __STDC_FORMAT_MACROS
# include <inttypes.h>

# include "pressuremonitor.h"
# include "sysinfo.h"

# include <ctype.h>

////////////////////////////////////////////////////////////////////////////////
// Linux code:

# ifdef OS_LINUX

static void slurpFile( const StrBuf& path, StrBuf& out, Error* e )
{
	FileSysUPtr f = FileSys::CreateUPtr( FST_TEXT );
	f->Set( path );
	f->Open( FOM_READ, e );
	if( e->Test() )
	    return;
	f->ReadWhole( &out, e );
}

// Return k/v pairs for files under '/proc', such as 'meminfo'.

static void parseProcFile( StrBuf& data, StrBufDict& out, Error* e )
{
	// key:<tab><arbitrary data>
	// E.g.:
	// VmPeak:	 5866208 kB
	// Groups:	4 24 27 30 46 112 124 1000 
	// State:	R (running)
	// PPid:	1

	FileSysBufferedLineReader fb( new FileStrPtr( &data ) );
	StrBuf line;

	while( fb.ReadLine( &line, e ) && !e->Test() )
	{
	    const char* kOffset = strchr( line.Text(), ':' );

	    if( !kOffset )
	        return; // error?

	    StrBuf k, v;
	    k.Set( line.Text(), kOffset - line.Text() );

	    // "k: v"
	    //    ^  <- vOffset
	    int vOffset = kOffset - line.Text() + 1;

	    while( isspace( line[ vOffset ] ) && vOffset < line.Length() )
	        vOffset++;

	    v.Set( line.Text() + vOffset );

	    out.SetVar( k, v );
	}
}

// Separate the value from the unit for a line from a file under /proc.

static void parseProcNumericUnit( const StrBuf& in, StrBuf& val, StrBuf& unit )
{
	// E.g. '2048 kB' -> '2048', 'kB'
	const char* offset = strchr( in.Text(), ' ' );

	if( !offset )
	    return; // error?

	val.Set( in.Text(), offset - in.Text() );
	unit.Set( offset + 1 );
}

// Collect the necessary memory fields from "/proc/meminfo", synthesizing
// "MemAvailable" if it's not present.

static void collectCurrentMemInfoLinux( StrBufDict& output, Error* e )
{
	// https://docs.kernel.org/filesystems/proc.html
	const StrBuf mFile( "/proc/meminfo" );
	StrBuf memData;

	slurpFile( mFile, memData, e );

	if( e->Test() )
	    return;

	StrBufDict memDict;

	parseProcFile( memData, memDict, e );

	if( e->Test() )
	    return;

	const char* const ks[ 6 ] = { "MemTotal", "MemFree", "MemAvailable",
	                              "SwapTotal", "SwapFree", NULL };

	for( int i = 0; ks[ i ]; i++ )
	{
	    const char* const k = ks[ i ];
	    StrBuf v, u;
	    StrPtr* b = memDict.GetVar( k );
	    if( !b ) // E.g. MemAvailable was not present in RHEL < 7.
	        continue;

	    parseProcNumericUnit( *b, v, u );
	    if( u == "kB" )
	        v = StrNum( v.Atoi64() * 1024 );
	    // Err on non 'kB' units?
	    // All Linux versions so far use kB, so it probably won't change.
	    output.SetVar( k, v );
	}

	// Historical Linux versions didn't have this:

	if( !memDict.GetVar( "MemAvailable" ) )
	{
	    // NB:  this assumes the units are in kB.
	    P4INT64 memFree = memDict.GetVar( "MemFree" )->Atoi64() * 1024;
	    P4INT64 buffers = memDict.GetVar( "Buffers" )->Atoi64() * 1024;
	    P4INT64 cached = memDict.GetVar( "Cached" )->Atoi64() * 1024;

	    output.SetVar( "MemAvailable",
	        StrNum( memFree + buffers + cached ) );
	}
}

// This struct represents a single Linux Cgroup V2 pressure file line.

struct CgroupV2PressureInfoLine
{
	float avg10 = 0, avg60 = 0, avg300 = 0;
	uint64_t total = 0;

	void Dump( StrBuf& buf ) const
	{
	    char tBuf[ 1024 ];
	    memset( tBuf, '\0', 1024 );
	    sprintf( tBuf, "avg10 %f, avg60 %f, avg300 %f total %" PRIu64,
	             avg10, avg60, avg300, total );
	    buf = tBuf;
	}
};

// This struct represents a single Linux Cgroup V2 pressure file, which
// is composed of one or lines of text.  There are versions of Linux where
// the CPU pressure file only has a single 'some' line since 'full' is
// not applicable to CPU pressure.  Others have both lines for CPU.
//
//   cat /proc/pressure/cpu
//   some avg10=0.00 avg60=0.00 avg300=0.00 total=1116177576
//
//   cat /proc/pressure/memory
//   some avg10=0.00 avg60=0.00 avg300=0.00 total=8470643
//   full avg10=0.00 avg60=0.00 avg300=0.00 total=6808329

struct CgroupV2PressureInfo
{
	CgroupV2PressureInfoLine some, full;
};

static CgroupV2PressureInfo
parseCgroupV2PressureFile( const StrBuf& fileName, Error* e )
{
	// cat /proc/pressure/memory
	// some avg10=0.00 avg60=2.23 avg300=11.01 total=2110152573
	// full avg10=0.00 avg60=2.07 avg300=10.21 total=1972817062
	//
	// Note that the cpu pressure file only reports the 'some' field.

	CgroupV2PressureInfo psi;

	auto parse = []( const char* line, CgroupV2PressureInfoLine& info )
	{
	    // Skip over the leading 'some'/'full' word.
	    const char* pos = strchr( line, ' ' );
	    return sscanf( pos + 1,
	                   "avg10=%f avg60=%f avg300=%f total=%" PRIu64,
	                   &info.avg10, &info.avg60, &info.avg300, &info.total
	                 );
	};

	auto f = FileSys::CreateUPtr( FST_TEXT );

	f->Set( fileName );
	f->Open( FOM_READ, e );

	if( e->Test() )
	{
	    e->Sys( "parseCgroupV2PressureFile open", fileName.Text() );
	    return {};
	}

	StrBuf buf;

	// Read the 'some' line.
	f->ReadLine( &buf, e );

	if( e->Test() )
	{
	    e->Sys( "parseCgroupV2PressureFile readline 0", fileName.Text() );
	    return {};
	}

	(void) parse( buf.Text(), psi.some );

	buf.Clear();

	// Read the 'full' line.
	f->ReadLine( &buf, e );

	if( e->Test() )
	{
	    e->Sys( "parseCgroupV2PressureFile readline 1", fileName.Text() );
	    return {};
	}

	(void) parse( buf.Text(), psi.full );

	return psi;
}
# endif // !OS_NT

////////////////////////////////////////////////////////////////////////////////
// Windows code:

# ifdef OS_NT

static void collectCurrentMemInfoWin( StrBufDict& output, Error* e )
{
	// https://learn.microsoft.com/en-us/windows/win32/api/sysinfoapi/ns-sysinfoapi-memorystatusex

	// Note that the size of these fields is ull vs the P4INT64
	// that we cast it to.

	MEMORYSTATUSEX ms;

	ms.dwLength = sizeof( ms );
	GlobalMemoryStatusEx( &ms );

	output.SetVar( "MemTotal", (P4INT64)ms.ullTotalPhys );
	output.SetVar( "MemFree",
	    (P4INT64)( ms.ullTotalPhys - ms.ullAvailPhys ) );
	output.SetVar( "MemAvailable", (P4INT64)ms.ullAvailPhys );
	output.SetVar( "SwapTotal", (P4INT64)ms.ullTotalPageFile );
	output.SetVar( "SwapFree", (P4INT64)ms.ullAvailPageFile );

	// This is Windows-only.
	output.SetVar( "MemoryLoad", (P4INT64)ms.dwMemoryLoad );

	// TotalVirtual and AvailVirtual are unused.
}

# endif // OS_NT

////////////////////////////////////////////////////////////////////////////////
// SystemInfo members:

SystemInfo::SystemInfo()
{
}

void SystemInfo::ResetResults()
{
	memPressurePct = PressureMonitor::Type::LOW;
	memPressurePSI = PressureMonitor::Type::LOW;
	cpuPressurePSI = PressureMonitor::Type::LOW;

	pauseRateMemMedmPct = 0;
	pauseRateMemHighPct = 0;
	pauseRateMemMedmPSI = 0;
	pauseRateMemHighPSI = 0;
	pauseRateCPUPSI = 0;
}

bool SystemInfo::MemoryPressureHigh()
{
	return memPressurePct == PressureMonitor::Type::HIGH ||
	       memPressurePSI == PressureMonitor::Type::HIGH;
}

bool SystemInfo::MemoryPressureMedium()
{
	return ( memPressurePct == PressureMonitor::Type::MEDIUM ||
	         memPressurePSI == PressureMonitor::Type::MEDIUM ) &&
	       ( memPressurePct != PressureMonitor::Type::HIGH &&
	         memPressurePSI != PressureMonitor::Type::HIGH );
}

bool SystemInfo::MemoryPressureLow()
{
	return memPressurePct == PressureMonitor::Type::LOW &&
	       memPressurePSI == PressureMonitor::Type::LOW;
}

bool SystemInfo::CPUPressureHigh()
{
	return cpuPressurePSI == PressureMonitor::Type::HIGH;
}

void SystemInfo::SetMemTuningsPct( const double aKp, const double aKi,
	                           const double aKd,
	                           const double cKp, const double cKi,
	                           const double cKd )
{
	if( !calc_mem_medm_cfg )
	    return;

	// There's only tuning available for the medium level because
	// the high level is mostly used for time-averaging the time
	// spent there (and the default tuning ought to suffice).

	calc_mem_medm_cfg->SetTunings( aKp, aKi, aKd, cKp, cKi, cKd );
}

void SystemInfo::SetMemTuningsPSI( const double aKp, const double aKi,
	                           const double aKd,
	                           const double cKp, const double cKi,
	                           const double cKd )
{
	if( !calc_mem_medm_psi )
	    return;

	calc_mem_medm_psi->SetTunings( aKp, aKi, aKd, cKp, cKi, cKd );
}

void SystemInfo::SetCPUTuningsPSI( const double aKp, const double aKi,
	                           const double aKd,
	                           const double cKp, const double cKi,
	                           const double cKd )
{
	if( !calc_cpu_high_psi )
	    return;

	calc_cpu_high_psi->SetTunings( aKp, aKi, aKd, cKp, cKi, cKd );
}

P4INT64 SystemInfo::MediumMemoryBytes( StrBufDict& rs )
{
	const P4INT64 memTotal = rs.GetVar( "MemTotal" )->Atoi64();
	const float mediumPct =
	    p4tunable.Get( P4TUNE_SYS_PRESSURE_MEM_MEDIUM ) * .01;

	return (P4INT64)( memTotal * mediumPct );
}

void SystemInfo::CollectCurrentMemInfoPct( StrBufDict& rs,
	                                   const P4INT64 svrCachedMem,
	                                   Error* e )
{
	rs.Clear();

# ifdef OS_NT
	collectCurrentMemInfoWin( rs, e );
# elif OS_LINUX
	collectCurrentMemInfoLinux( rs, e );
# endif

	if( e->Test() )
	{
	    memPressurePct = PressureMonitor::Type::LOW;
	    usedPct = 0;
	    return;
	}

	// On Windows, the server is a single process and the memory manager
	// caches a lot of memory when threads exit (as opposed to unix where
	// forked process memory is immediately given back to the OS).  In order
	// to maintain the memory cache (for performance) and to not count the
	// cached memory against the OS free amount (since it's ours to use),
	// it is added to the amount the OS says is available.  Note that this
	// is mixing measures in a way - the OS available number is the amount
	// of memory the OS says can be used without swapping - the cached
	// memory in the server is not guaranteed to be out of swap.
	// Incorporating the cached memory is also important because it keeps
	// the server from artificially hovering close to the medium pausing
	// threshold.

	const P4INT64 memAvail = rs.GetVar( "MemAvailable" )->Atoi64();
	const P4INT64 memTotal = rs.GetVar( "MemTotal"     )->Atoi64();

	const float memAvailPct = (float) ( memAvail + svrCachedMem ) /
	                          (float) memTotal;

	usedPct = 1 - memAvailPct;

	if( p4tunable.Get( DT_RESOURCE ) > 4 )
	    p4debug.printf( "CollectCurrentMemInfo "
	                    "memTotal %lld memAvail %lld svrCachedMem %lld "
	                    "usedPct %.2f\n",
	                    memTotal, memAvail, svrCachedMem, usedPct );

	return;
}

void SystemInfo::InitPressureMonitorHighMemPSI( Error* e )
{
	// Initialize the PSI monitor, if supported.  There may be
	// callers of this class that don't need/want this, so do
	// it outside of the constructor.

	if( !( HostCfg::CanPSI() && !pm_mem_high ) )
	    return;

	// todo: have the windows one treat medium as low?
	pm_mem_high = PressureMonitor::MkPressureMonitor(
	                  PressureMonitor::Resource::MEMORY,
	                  PressureMonitor::Type::HIGH, e );

	// If for some reason the monitor failed to initialize,
	// just ignore it and use the configurables.

	if( e->Test() )
	{
	    AssertLog.Report( e );
	    e->Clear();
	    pm_mem_high.reset();
	    return;
	}

	if( p4tunable.Get( DT_RESOURCE ) > 7 )
	    p4debug.printf( "InitPressureMonitorHighMemPSI success\n" );
}

////////////////////////////////////////////////////////////////////////////////
// Code to assign the low/medium/high labels for memory pressure.

void SystemInfo::CalculateResourcePressureMemoryByPct( StrBufDict& rs,
	                                               Error* e )
{
	const double highVal = calc_mem_high_cfg->GetInput();

	if( pauseRateMemMedmPct == 0 )
	    memPressurePct = PressureMonitor::Type::LOW;

	if( pauseRateMemMedmPct > 0 && highVal < calc_mem_high_cfg_setpoint )
	    memPressurePct = PressureMonitor::Type::MEDIUM;

	if( highVal >= calc_mem_high_cfg_setpoint )
	    memPressurePct = PressureMonitor::Type::HIGH;

	if( p4tunable.Get( DT_RESOURCE ) > 5 )
	    p4debug.printf( "CalculateResourcePressureMemoryByPct "
	                    "m %d, hp %d, hv %.2f, r %d\n",
	                    pauseRateMemMedmPct, pauseRateMemHighPct, highVal,
	                    (int)memPressurePct );
}

void SystemInfo::CalculateResourcePressureMemoryByPSI( Error* e )
{
	if( !HostCfg::CanPSI() )
	    return;

	InitPressureMonitorHighMemPSI( e );

	if( e->Test() )
	{
	    memPressurePSI = PressureMonitor::Type::LOW;
	    return;
	}

	if( pm_mem_high && pm_mem_high->Exists() )
	{
	    memPressurePSI = PressureMonitor::Type::HIGH;

	    if( p4tunable.Get( DT_RESOURCE ) > 5 )
	        p4debug.printf( "CalculateResourcePressureMemory "
	                         "pressure monitor high\n" );
	    return;
	}
	else
	    memPressurePSI = PressureMonitor::Type::LOW;

# ifdef OS_LINUX

	const double highVal = calc_mem_high_psi->GetInput();

	if( pauseRateMemMedmPSI == 0 )
	    memPressurePSI = PressureMonitor::Type::LOW;

	if( pauseRateMemMedmPSI > 0 && highVal < calc_mem_high_psi_setpoint )
	    memPressurePSI = PressureMonitor::Type::MEDIUM;

	if( highVal >= calc_mem_high_psi_setpoint )
	    memPressurePSI = PressureMonitor::Type::HIGH;

	if( p4tunable.Get( DT_RESOURCE ) > 5 )
	    p4debug.printf( "CalculateResourcePressureMemoryByPSI "
	                    "m %d, hp %d, hv %.2f, r %d\n",
	                    pauseRateMemMedmPSI, pauseRateMemHighPSI, highVal,
	                    (int)memPressurePSI );
# endif
}

////////////////////////////////////////////////////////////////////////////////

int SystemInfo::CalculatePauseRateForMemByPct( Error* e )
{
	if( !calc_mem_medm_cfg )
	{
	    calc_mem_medm_cfg = std::make_unique< PauseCalcMemPcts >
	                        ( &calc_mem_medm_cfg_setpoint, "mem pct medm" );
	    calc_mem_high_cfg = std::make_unique< PauseCalcMemPcts >
	                        ( &calc_mem_high_cfg_setpoint, "mem pct high" );
	}

	int highPct = p4tunable.Get( P4TUNE_SYS_PRESSURE_MEM_HIGH   ),
	    medmPct = p4tunable.Get( P4TUNE_SYS_PRESSURE_MEM_MEDIUM );

	// High and medium should both be unset if one is, so if either are
	// unset, just auto-set both to avoid odd cases.

	if( medmPct >= highPct )
	{
	    // Use the defaults in this case.
	    highPct =
	        p4tunable.GetOriginalValue( P4TUNE_SYS_PRESSURE_MEM_HIGH );
	    medmPct =
	        p4tunable.GetOriginalValue( P4TUNE_SYS_PRESSURE_MEM_MEDIUM );
	}

	int medm_dur =
	    p4tunable.Get( P4TUNE_SYS_PRESSURE_MEM_MEDIUM_DURATION ) / 100,
	    high_dur =
	    p4tunable.Get( P4TUNE_SYS_PRESSURE_MEM_HIGH_DURATION ) / 100;

	if( high_dur == 0 )
	    high_dur = 1;

	if( medm_dur == 0 )
	    medm_dur = 1;

	calc_mem_medm_cfg_setpoint = medmPct;
	calc_mem_medm_cfg->SetWindowSize( medm_dur );
	calc_mem_high_cfg_setpoint = highPct;
	calc_mem_high_cfg->SetWindowSize( high_dur );

	// Expects that CollectCurrentMemInfoPct() was already called
	// to update usedPct.

	calc_mem_medm_cfg->SetInput( (uint64_t)( usedPct * 100 ) );
	calc_mem_high_cfg->SetInput( (uint64_t)( usedPct * 100 ) );

	pauseRateMemMedmPct = calc_mem_medm_cfg->Compute();
	pauseRateMemHighPct = calc_mem_high_cfg->Compute();

	if( p4tunable.Get( DT_RESOURCE ) > 5 )
	    p4debug.printf( "SystemInfo::CalculatePauseRateForMemByPct "
	                    "medm setpoint %f, high setpoint %f, "
	                    "medm dur %d, high dur %d, usedPct %f,"
	                    " medm pct %d, high pct %d\n",
	                    calc_mem_medm_cfg_setpoint,
	                    calc_mem_high_cfg_setpoint, medm_dur, high_dur,
	                    usedPct,
	                    pauseRateMemMedmPct, pauseRateMemHighPct );

	return pauseRateMemMedmPct;
}

// Note that there's nothing really Windows-specific here, but there's currently
// only a Windows implementation of the PressureMonitor, so until there are more
// we'll just do this there.

int SystemInfo::CalculatePauseRateForMemWindowsPSI( Error* e )
{
	// Note that there's no configuration here - so no tunables.

	InitPressureMonitorHighMemPSI( e );

	if( e->Test() )
	    return 0;

# ifdef OS_NT
	// No shades of gray on Windows.  It only emits this when it's
	// really at the end, like less than 256 MB free.

	if( pm_mem_high && pm_mem_high->Exists() )
	    return 100;
# endif
	return 0;
}

int SystemInfo::CalculatePauseRateForMemLinuxPSI( Error* e )
{
# ifdef OS_LINUX
	if( !calc_mem_high_psi )
	{
	    calc_mem_high_psi = std::make_unique< PauseCalcLinuxPSI >
	        ( &calc_mem_high_psi_setpoint, "mem psi high" );
	    calc_mem_medm_psi = std::make_unique< PauseCalcLinuxPSI >
	        ( &calc_mem_medm_psi_setpoint, "mem psi medm" );
	}

	int high_dur =
	    p4tunable.Get( P4TUNE_SYS_PRESSURE_OS_MEM_HIGH_DURATION ) / 100,
	    medm_dur =
	    p4tunable.Get( P4TUNE_SYS_PRESSURE_OS_MEM_MEDIUM_DURATION ) / 100;

	if( high_dur == 0 )
	    high_dur = 1;

	if( medm_dur == 0 )
	    medm_dur = 1;

	int highPct = p4tunable.Get( P4TUNE_SYS_PRESSURE_OS_MEM_HIGH ),
	    medmPct = p4tunable.Get( P4TUNE_SYS_PRESSURE_OS_MEM_MEDIUM );

	if( medmPct >= highPct )
	{
	    // Use the defaults in this case.
	    highPct =
	        p4tunable.GetOriginalValue( P4TUNE_SYS_PRESSURE_OS_MEM_HIGH );
	    medmPct =
	        p4tunable.GetOriginalValue( P4TUNE_SYS_PRESSURE_OS_MEM_MEDIUM );
	}

	calc_mem_high_psi_setpoint = highPct;
	calc_mem_high_psi->SetWindowSize( high_dur );
	calc_mem_medm_psi_setpoint = medmPct;
	calc_mem_medm_psi->SetWindowSize( medm_dur );

	const StrBuf mem = HostCfg::GetGlobalCGroupV2MemFile();
	const auto info = parseCgroupV2PressureFile( mem, e );

	// If something went wrong, return a 0% pause level.

	if( e->Test() )
	    return 0;

	calc_mem_high_psi->SetInput( info.some.total );
	calc_mem_medm_psi->SetInput( info.some.total );

	pauseRateMemMedmPSI = calc_mem_medm_psi->Compute();
	pauseRateMemHighPSI = calc_mem_high_psi->Compute();

	if( p4tunable.Get( DT_RESOURCE ) > 5 )
	    p4debug.printf( "SystemInfo::CalculatePauseRateForMemLinuxPSI"
	                    " avg10 %f, medm pct %d, high pct %d, medm dur %d,"
	                    " high dur %d, total %" PRIu64 "\n",
	                    info.some.avg10, pauseRateMemMedmPSI,
	                    pauseRateMemHighPSI, medm_dur, high_dur,
	                    info.some.total );

	return pauseRateMemMedmPSI;
# else
	return 0;
# endif
}

int SystemInfo::CalculatePauseRateForMemByPSI( Error* e )
{
	if( !HostCfg::CanPSI() )
	    return 0;

# ifdef OS_NT
	return CalculatePauseRateForMemWindowsPSI( e );
# endif
# ifdef OS_LINUX
	return CalculatePauseRateForMemLinuxPSI( e );
# endif

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// All CPU-related code.

void SystemInfo::CalculateResourcePressureCPUByPSI( Error* e )
{
	if( !HostCfg::CanPSI() )
	    return;

	if( pauseRateCPUPSI > 0 )
	    cpuPressurePSI = PressureMonitor::Type::HIGH;
	else
	    cpuPressurePSI = PressureMonitor::Type::LOW;

	return;
}

int SystemInfo::CalculatePauseRateForCPUByPSI( Error* e )
{
	if( !HostCfg::CanPSI() )
	    return 0;

	pauseRateCPUPSI = CalculatePauseRateForCPULinuxPSI( e );

	return pauseRateCPUPSI;
}

int SystemInfo::CalculatePauseRateForCPULinuxPSI( Error* e )
{
# ifdef OS_LINUX
	if( !calc_cpu_high_psi )
	    calc_cpu_high_psi = std::make_unique< PauseCalcLinuxPSI >
	                       ( &calc_cpu_high_psi_setpoint, "cpu psi" );

	int high_dur =
	    p4tunable.Get( P4TUNE_SYS_PRESSURE_OS_CPU_HIGH_DURATION ) / 100;

	if( high_dur == 0 )
	    high_dur = 1;

	calc_cpu_high_psi_setpoint =
	    p4tunable.Get( P4TUNE_SYS_PRESSURE_OS_CPU_HIGH );
	calc_cpu_high_psi->SetWindowSize( high_dur );

	const StrBuf cpu = HostCfg::GetGlobalCGroupV2CPUFile();
	const auto info = parseCgroupV2PressureFile( cpu, e );

	// If something went wrong, return a 0% pause level.

	if( e->Test() )
	    return 0;

	if( p4tunable.Get( DT_RESOURCE ) > 5 )
	{
	    StrBuf someBuf, fullBuf;
	    info.some.Dump( someBuf );
	    info.full.Dump( fullBuf );

	    p4debug.printf( "SystemInfo::CalculatePauseRateForCPULinuxPSI() "
	                    "some %s, full %s\n", someBuf.Text(),
	                                          fullBuf.Text() );
	}

	calc_cpu_high_psi->SetInput( info.some.total );

	pauseRateCPUPSI = calc_cpu_high_psi->Compute();

	return pauseRateCPUPSI;
# else
	return 0;
# endif
}

# endif // HAS_CPP11
