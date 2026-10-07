/*
 * Copyright 1995, 2020 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

// This class provides a means to poll for frequently-changing data and
// to dump the state of a collection of reporting commands.

class SystemInfo
{
	public:

# ifdef HAS_CPP11

	    // The pressure monitors can't be copied, so don't try.
	    SystemInfo& operator=( const SystemInfo& ) = delete;
	    SystemInfo( const SystemInfo& ) = delete;

	    SystemInfo();

# endif // HAS_CPP11
	    ////////////////////////////////////////

	    // Trawl the system, running a variety of reporting commands
	    // about system configuration.
	    static void Collect( StrBufDict& output, Error* e );

	    ////////////////////////////////////////

# ifdef OS_NT
	    static int WindowsVersionInfo( DWORD &major, DWORD &minor,
	        DWORD &build, WORD &ptype );

	    static int CheckForAtomicRename();
	    static int CheckForDevSymlink();
# endif

	    ////////////////////////////////////////
# ifdef HAS_CPP11

	    void SetMemTuningsPct( const double aKp, const double aKi,
	                           const double aKd,
	                           const double cKp, const double cKi,
	                           const double cKd );

	    void SetMemTuningsPSI( const double aKp, const double aKi,
	                           const double aKd,
	                           const double cKp, const double cKi,
	                           const double cKd );

	    void SetCPUTuningsPSI( const double aKp, const double aKi,
	                           const double aKd,
	                           const double cKp, const double cKi,
	                           const double cKd );

	    // Sample OS free resources for percentage-based calculations.
	    void CollectCurrentMemInfoPct( StrBufDict& rs,
	                                   const P4INT64 svrCachedMem,
	                                   Error* e );

	    // These make the low/medium/high designations.
	    void CalculateResourcePressureMemoryByPct( StrBufDict& rs,
	                                               Error* e );
	    void CalculateResourcePressureMemoryByPSI( Error* e );
	    void CalculateResourcePressureCPUByPSI( Error* e );

	    // low/medium/high are labels assigned to pressure states, and
	    // are used to guide command-handling behavior.
	    // CalculateResourcePressure*() must be called prior to these.
	    bool MemoryPressureHigh();
	    bool MemoryPressureMedium();
	    bool MemoryPressureLow();
	    bool CPUPressureHigh();

	    P4INT64 MediumMemoryBytes( StrBufDict& rs );

	    int CalculatePauseRateForCPUByPSI( Error* e );
	    int CalculatePauseRateForMemByPct( Error* e );
	    int CalculatePauseRateForMemByPSI( Error* e );

# endif // HAS_CPP11

	private:

# ifdef HAS_CPP11

	    void InitPressureMonitorHighMemPSI( Error* e );

	    int CalculatePauseRateForCPULinuxPSI( Error* e );
	    int CalculatePauseRateForMemLinuxPSI( Error* e );
	    int CalculatePauseRateForMemWindowsPSI( Error* e );

	    // Return the pressure and pause rates to their initial settings.
	    void ResetResults();

	    // These are the current labeled (rather than a numeric range)
	    // states for each type.

	    PressureMonitor::Type
	        memPressurePct = PressureMonitor::Type::LOW,
	        memPressurePSI = PressureMonitor::Type::LOW,
	        cpuPressurePSI = PressureMonitor::Type::LOW;

	    std::unique_ptr< PressureMonitor > pm_mem_high;

	    // The PID controller in the PauseRateCalculatorByResource object
	    // captures the setpoint via a pointer, so we own the storage here.
	    // The value is updated each time the pause rate is requested,
	    // since the caller will be re-loading the configuration
	    // periodically.

	    double calc_cpu_high_psi_setpoint = 0,
	           calc_mem_medm_psi_setpoint  = 0,
	           calc_mem_high_psi_setpoint = 0,
	           calc_mem_medm_cfg_setpoint  = 0,
	           calc_mem_high_cfg_setpoint = 0;

	    // The the time-averaging of the PauseRateCalculatorByResource
	    // instances is driven by the values of the *DURATION configurables
	    // in milliseconds and the tick rate of the callers (currently
	    // a hard-coded 100ms).

	    // PSI-based calculators.

	    std::unique_ptr< PauseRateCalculatorByResource > calc_cpu_high_psi;

	    std::unique_ptr< PauseRateCalculatorByResource > calc_mem_high_psi,
	                                                     calc_mem_medm_psi;

	    // Percentage-based calculator.

	    std::unique_ptr< PauseRateCalculatorByResource > calc_mem_medm_cfg,
	                                                     calc_mem_high_cfg;

	    // Percentage of memory used in the system.  Input for
	    // calc_mem_medm_cfg.

	    double usedPct = 0;

	    // The end result - backpressure percentages driven by system load.

	    int pauseRateCPUPSI = 0,
	        pauseRateMemMedmPct = 0, pauseRateMemHighPct = 0,
	        pauseRateMemMedmPSI = 0, pauseRateMemHighPSI = 0;

# endif // HAS_CPP11
} ;
