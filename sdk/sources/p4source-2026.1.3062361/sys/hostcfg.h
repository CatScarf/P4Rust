/*
 * Copyright 1995, 2023 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

// Collect/report information regarding the static (mostly)
// configuration of the host OS/machine or current process.

# ifdef HAS_CPP11

# include <tuple>
# include <vector>

# endif

class HostCfg
{
    public:

	HostCfg();

	// Look at the OS configuration and initialize getters.
	void	Discover( Error* e );

	static	void	Dump( StrBuf& buf );

	////////////////////////////////////////

	// Linux-specific:

	// Path to file, e.g. "/proc/self/oom_score_adj".  Could
	// potentially differ if /proc is mounted somewhere else.
	static	const	StrPtr*	OOMScoreAdjFile();
	// The OOM score adjustment file is present and writable.
	static	bool	HasOOMScoreAdj();
	// This process has the correct Capabilities and is able to
	// perform the necessary manipulations to the OOM score file.
	static	bool	CanOOMScoreAdj();
	// The value of the OOM score adjustment on process load.
	static	const	StrPtr*	GetInitialOOMScoreAdj();
	// The value to replace the default with to hide from the OOM killer.
	static	const	StrPtr*	GetHiddenOOMScoreAdj();
	// Return the content of the OOM score adjustment file as it is now.
	const	void	GetCurrentOOMScoreAdj( StrBuf& buf, Error* e ) const;
	// Perform process setup operations, optionally disabling OOM adjustment
	void	ConfigureOOMAdjust( const int score, Error* e );

	////////////////////////////////////////

	// Linux-specific:

	// Prefer using HasCGroupV2() to IsCGroupV2() to accommodate
	// the situation where both cgroup versions are present.

	static	bool	HasCGroups();
	static	bool	HasCGroupV1();
	static	bool	HasCGroupV2();
	static	bool	IsCGroupV1();
	static	bool	IsCGroupV2();

	static	StrBuf	GetProcessCGroupV1MemoryDir();
	static	StrBuf	GetProcessCGroupV2Dir();
	static	StrBuf	GetProcessCGroupV2CPUFile();
	static	StrBuf	GetProcessCGroupV2MemFile();
	static	StrBuf	GetGlobalCGroupV1MemFile();
	static	StrBuf	GetGlobalCGroupV2MemFile();
	static	StrBuf	GetGlobalCGroupV2CPUFile();

	////////////////////////////////////////

	// If some type of PSI support is visible.
	static	bool	HasPSI();
	// If some type of PSI support is usable.
	static	bool	CanPSI();

    private:

	// This must be called after DiscoverCGroup().
	// This only says if any global PSI is available, not all of them
	// or the cgroup-specific one a process may be in.
	void	DiscoverPSI();

	////////////////////////////////////////

	void	DiscoverOOM();

	static	StrBuf	oomScoreAdjFile;
	static	bool	hasOOMAdj, canOOMAdj, tryOOMAdj;
	static	StrBuf	initialOOMScoreAdj, hiddenOOMScoreAdj;

	////////////////////////////////////////

	void	DiscoverCGroup( Error* e );
	void	ParseCGroupDef( Error* e );
	static StrBuf	GetProcessCGroupV1XDir( const char* );

	static bool	hasCGroups, hasCGroupsV1,
			hasCGroupsV2, isCGroupsV1, isCGroupsV2, hasPSI, canPSI;

# ifdef HAS_CPP11
	using cgInfo = std::tuple< StrBuf, StrBuf, StrBuf >;
	static std::vector< cgInfo >	cgInfos;
# endif

	static StrBuf processCGroupFile;
	static bool canUseCGroupFile;
} ;
