/*
 * Copyright 1995, 2020 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# define NEED_WINDOWSH

# include <stdhdrs.h>

# include <error.h>
# include <strbuf.h>
# include <strdict.h>
# include <strtable.h>
# include "pressuremonitor.h"
# include "sysinfo.h"
# include "runcmd.h"
# include "pid.h"

# ifdef HAS_CPP11

# include <vector>
# include <regex>

static void run( StrBufDict& output, const std::vector< const char* > cmd,
	         const std::vector< std::string >& masks, Error* e )
{
	RunCommandIo io;
	RunArgv args;
	StrBuf name, out;

	for( const auto& a : cmd )
	{
	    args << a;
	    name << a << " ";
	}

	char* end = name.Text() + name.Length() - 1;
	*( end ) = '\0'; // Omit the trailing space.
	name.SetEnd( end );

	io.Run( args, out, e );

	if( e->Test() )
	    e->Fmt( &out );

	// Mask-out anything that might be sensitive, like passwords.

	for( const auto& m : masks )
	{
	    std::string re;
	    // Capture the leading newline so it's part of the replacement.
	    re += "((^|\n)";
	    re += m;
	    re += ").*";
	    out = std::regex_replace( out.Text(),
	                              std::regex( re ), "$1MASKED" ).c_str();
	}

	output.SetVar( name, out );

	// Prevent the error from being attached to all output.
	e->Clear();
}


void SystemInfo::Collect( StrBufDict& output, Error* e )
{
	// Not all commands are available everywhere, and for simplicity of
	// implementation, we don't try to minimize output (read: there will
	// be duplication in places).

	output.Clear();

	StrNum pid( Pid().GetID() );

	// Unix is =-separated and Windows is whitepace.
	const std::vector< std::string > masks = { "P4PASSWD=", "P4PASSWD\\s+" };

# ifdef OS_LINUX
# define FOUND_PLATFORM

	const std::vector< std::vector< const char* > > cmds =
	    { { "hostnamectl" },
	      { "lsb_release", "-a" },
	      { "sh", "-c", "cat /etc/*elease" },
	      { "sh", "-c", "cat /proc/swaps" },
	      { "sh", "-c", "cat /proc/cpuinfo" },
	      { "sh", "-c", "cat /proc/meminfo" },
	      { "sh", "-c", "ls -1 /dev/disk/by-id/" },
	      { "sh", "-c", "xargs -0 -L1 -a /proc/self/environ" },
	      { "sh", "-c", "cat /proc/sys/kernel/core_pattern" },
	      { "grep", "docker\\|lxc", "/proc/1/cgroup" },
	      { "systemd-detect-virt" },
	      { "mount" },
	      { "lshw" },
	      { "lspci" },
	      { "lsusb" },
	      { "free", "-m" },
	      { "sysctl", "-a" },
	      { "sh", "-c", "ulimit -a" },
	      { "timedatectl" },
	      { "sh", "-c", "cat /etc/fstab" },
	      { "sh", "-c", "cat /sys/kernel/mm/redhat_transparent_hugepage/enabled" },
	      { "sh", "-c", "cat /sys/kernel/mm/redhat_transparent_hugepage/defrag" },
	      { "sh", "-c", "cat /sys/kernel/mm/redhat_transparent_hugepage/khugepaged/defrag" },
	      { "sh", "-c", "cat /sys/kernel/mm/transparent_hugepage/enabled" },
	      { "sh", "-c", "cat /sys/kernel/mm/transparent_hugepage/defrag" },
	      { "sh", "-c", "cat /sys/kernel/mm/transparent_hugepage/khugepaged/defrag" },
	      { "sh", "-c", "cat /sys/class/dmi/id/sys_vendor" },
	      { "sh", "-c", "cat /sys/class/dmi/id/bios_*" },
	      { "sh", "-c", "cat /sys/class/dmi/id/board_vendor /sys/class/dmi/id/board_name  /sys/class/dmi/id/board_version" },
	      { "p4", "set" },
	      { "sh", "-c", "uptime" },
	      { "sh", "-c", "whoami" },
	      { "sh", "-c", "uname -a" },
	      { "sh", "-c", "id" },
	      { "sh", "-c", "cat /proc/self/cgroup" }, // What the current cgroup for this process is.
	      // Will say 'cgroup', or 'cgroup2', or both.  This duplicates the
	      // unfiltered 'mount' output, but is easier to eyeball.
	      { "sh", "-c", "mount | grep '^cgroup' | awk '{print $1}' | uniq" },
	      // Exists if v2
	      { "sh", "-c", "ls /sys/fs/cgroup/cgroup.controllers" },
	      { "sh", "-c", "grep PSI /boot/config-$(uname --kernel-release)" },
	      // Kernel command-line.  Needs psi=1 if kernel compiled with
	      // CONFIG_PSI_DEFAULT_DISABLED=y
	      { "sh", "-c", "cat /proc/cmdline" },
	      // This is the default path for a cgroupv2 systemd-managed distro.
	      { "sh", "-c", "cat /sys/fs/cgroup/user.slice/user-$(id -u).slice/user@$(id -u).service/cgroup.controllers" },
	      { "sh", "-c", "ls /sys/fs/cgroup/user.slice/user-$(id -u).slice/user@$(id -u).service/" },
	      { "sh", "-c", "cat /sys/fs/cgroup/user.slice/user-$(id -u).slice/user@$(id -u).service/memory.pressure" },
	      // system-wide
	      { "sh", "-c", "tail /proc/pressure/*" },
	    };

# endif // OS_LINUX

# ifdef OS_NT
# define FOUND_PLATFORM
	
	const std::vector< std::vector< const char* > > cmds =
	    // https://docs.microsoft.com/en-us/powershell/scripting/samples/collecting-information-about-computers?view=powershell-7
	    { { "powershell", "-Command", "Get-ComputerInfo" }, // PS 5.1
	      { "powershell", "-Command", "Get-WMIObject Win32_OperatingSystem | Select Name, version, servicepackmajorversion, BuildNumber, CSName, OSArchitecture, OperatingSystemSKU, Caption, InstallDate" },
	      { "powershell", "-Command", "Get-HotFix | Format-List" },
	      { "powershell", "-Command", "Get-TimeZone" },
	      { "powershell", "-Command", "Get-CimInstance -ClassName Win32_BIOS" },
	      { "powershell", "-Command", "Get-CimInstance -ClassName Win32_Processor | Format-List *" },
	      { "powershell", "-Command", "Get-CimInstance -ClassName Win32_Baseboard" },
	      { "powershell", "-Command", "Get-CimInstance -ClassName Win32_PhysicalMemory" },
	      { "powershell", "-Command", "Get-CimInstance -ClassName Win32_LogicalDisk" },
	      { "powershell", "-Command", "Get-WmiObject -Class Win32_TemperatureProbe" },
	      { "powershell", "-Command", "Get-WmiObject win32_LogicalDisk | Select Name,Caption,Compressed,BlockSize,Availability,DeviceID,MediaType,DriveType,FreeSpace,FileSystem,VolumeName,SystemName | format-list" },
	      { "powershell", "-Command", "Get-CimInstance -ClassName Win32_ComputerSystem | Format-List *" },
	      { "powershell", "-Command", "Get-Childitem -Path Env:* | Sort-Object" },
	      { "systeminfo" },
	      { "powershell", "-Command", "Get-Date | Format-List *" },
	      { "powershell", "-Command", "[TimeZoneInfo]::Local" },
	      { "p4", "set" },
	    };

# endif // OS_NT

# if defined( OS_DARWIN ) || defined( OS_MACOSX )
# define FOUND_PLATFORM
	const std::vector< std::vector< const char* > > cmds;
	output.SetVar( "error", "Function not available on Darwin." );
# endif // OS_DARWIN OS_MACOSX

# ifdef FOUND_PLATFORM
	for( const auto& cmd : cmds )
	    run( output, cmd, masks, e );
# endif // FOUND_PLATFORM
}

# else // HAS_CPP11

void SystemInfo::Collect( StrBufDict& output, Error* e )
{
}

# endif // HAS_CPP11

# ifdef OS_NT

#include <winuser.h>

static int AtomicRenameSupported = -1;
static int DevSymlinkSupported = -1;

// Using the Powershell is very slow.  Call the Windows APIs
// directly to qualify the Windows Version.
//
// Operating System       Version   Build
// ---------------------------------------
// Windows Server 2022     10.0     20348
// Windows 11              10.0     22000
// Windows Server 2019     10.0     19042
// Windows 10              10.0     10240-19044
// Windows Server 2016     10.0     14393
// Windows Server 2012 R2   6.3     9600
// Windows 8.1              6.3     9600
// Windows Server 2012      6.2     9200
// Windows 8                6.2     9200
// Windows Server 2008 R2   6.1     7601
// Windows 7                6.1     7601
// Windows Server 2008      6.0     6001
// Windows Vista            6.0     6000
// Windows Server 2003 R2   5.2     3718
// Windows Server 2003      5.2     1218
// Windows Home Server      5.2     3790
// Windows XP Pro x64       5.2     3790
// Windows XP               5.1     2600
// Windows 2000             5.0     2195
//

// Return values,
//  0 - failure
//  1 - success
//
int
SystemInfo::WindowsVersionInfo( DWORD &major, DWORD &minor, DWORD &build,
	WORD &ptype )
{
	OSVERSIONINFOEX osi;

	ZeroMemory( &osi, sizeof(OSVERSIONINFOEX) );

	// (Requires Windows 2000 or later)
	//
	osi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if( ! GetVersionEx( (OSVERSIONINFO *) &osi ) )
	    return 0;

	major = osi.dwMajorVersion;
	minor = osi.dwMinorVersion;
	build = osi.dwBuildNumber;
	ptype = osi.wProductType;

	return 1;
}

// Return values,
//  0 - atomic rename not supported
//  1 - atomic rename supported
//
int
SystemInfo::CheckForAtomicRename( )
{
	DWORD major=0;
	DWORD minor=0;
	DWORD build=0;
	WORD ptype=0;

	if( AtomicRenameSupported >= 0)
	    return AtomicRenameSupported;

	if( ! WindowsVersionInfo( major, minor, build, ptype ) )
	{
	    AtomicRenameSupported = 0;
	    return 0;
	}

	// Disable for Windows 2003 and earlier.
	//
	if( major <= 6 )
	{
	    AtomicRenameSupported = 0;
	    return 0;
	}

	if( major == 10 )
	{
	    if( minor == 0 )
	    {
	        if( ptype == VER_NT_WORKSTATION )
	        {
	            if( build < 10240 )
	            {
	                // Earlier then Windows 10
	                AtomicRenameSupported = 0;
	            }
	            if( build >= 19044 )
	            {
	                // Windows 10, 11
	                AtomicRenameSupported = 1;
	            }
	        }
	        else
	        {
	            if( build >= 17763 )
	            {
	                // Windows Server 2020, build 20348
	                // Windows Server 2019, build 17763
	                AtomicRenameSupported = 1;
	            }
	            else if( build >= 14393 )
	            {
	                // Windows Server 2016, build 9200
	                // Allow 2016 for EC testing.
	                AtomicRenameSupported = 1;
	            }
	            else
	            {
	                // Earlier then Windows 2016 Server
	                AtomicRenameSupported = 0;
	            }
	        }
	    }
	}

	return AtomicRenameSupported;
}


// Return values,
//  0 - dev symlink not supported
//  1 - dev symlink supported
//
int
SystemInfo::CheckForDevSymlink()
{
	DWORD major=0;
	DWORD minor=0;
	DWORD build=0;
	WORD ptype=0;

	if( DevSymlinkSupported >= 0)
	    return DevSymlinkSupported;

	// Enable for Win10+
	DevSymlinkSupported = 
	    WindowsVersionInfo( major, minor, build, ptype ) &&
	    major >= 10;

	return DevSymlinkSupported;
}

# endif // OS_NT
