/*
 * Copyright 1995, 2023 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# ifdef HAS_CPP11

# include <memory>
# include <vector>
# include <cstdint>

// This class monitors the specified OS resource for a particular state,
// E.g. high CPU usage or low RAM pressure.  It exists to compliment the
// other means of getting OS resource pressure feedback (such as polling
// a file or API calls to get raw stats) in that this provides an object
// with a longer lifetime that can be used to get push notifications from
// the OS, or see transitions between pressure states (neither of which
// are possible in the pure sampling-based approach).

class PressureMonitor
{
    public:

	enum class Resource
	{
	    CPU, MEMORY
	};

	enum class Type
	{
	    // NB: The meanings of LOW/HIGH are reversed on Windows vs Linux.
	    //     There is also no MEDIUM on Windows.
	    LOW, MEDIUM, HIGH
	};

	// The underlying file handles can't be copied, so don't try.
	PressureMonitor& operator=( const PressureMonitor& ) = delete;
	PressureMonitor( const PressureMonitor& ) = delete;
	PressureMonitor() = default;

	virtual	~PressureMonitor();

	virtual	bool	Exists() const = 0;

	static std::unique_ptr< PressureMonitor >
	    MkPressureMonitor( PressureMonitor::Resource r,
	                       PressureMonitor::Type t,
	                       Error* e );
};

class PressureMonitorWin final : public PressureMonitor
{
    public:

	 PressureMonitorWin( PressureMonitor::Type t, Error* e );
	~PressureMonitorWin();

	bool	Exists() const;

    private:

# ifdef OS_NT
	HANDLE h;
# endif

} ;

////////////////////////////////////////////////////////////////////////////////

class MovingAverage
{
    public:

	// This must be called before the object is in a valid state.
	void SetSize( const int s );

	double Calculate( const uint64_t value );

    private:

	std::unique_ptr< std::vector< uint64_t > > values;
	size_t nValues = 0;
	uint64_t total = 0;
};

class PauseRateCalculatorByResource
{
    public:

			 PauseRateCalculatorByResource( double* setpoint,
	                                                const char* tag );
	virtual	~PauseRateCalculatorByResource();

	virtual	void	SetTunings( const double aKp, const double aKi,
	                                    const double aKd,
	                                    const double cKp, const double cKi,
	                                    const double cKd );
	virtual	void	SetWindowSize( const int wSize );
	virtual	void	SetInput( const uint64_t in );
	virtual	int	Compute();

	double	GetInput() const;

    protected:

	// Post-construction setup of the controller.  Necessary to call after
	// the first input has been received and before the first Compute().
	void		Initialize();
	bool		initialized = false;

	// Descriptive tag for logging.
	const char* type = nullptr;

	void* pid = nullptr;

	// Different resources have distinct configuration needs.  These are
	// expected to be set by the subclasses.
	double aggKp = 0, aggKi = 0, aggKd = 0;
	double consKp = 0, consKi = 0, consKd = 0;

	double output = 0, input = 0, *setpoint = nullptr;

	// The time scale this covers is determined by the tick rate of
	// this class's caller.
	MovingAverage avg;
};

// This monitors cgroups v2 /proc/pressure/* files.

class PauseCalcLinuxPSI final : public PauseRateCalculatorByResource
{
    public:

	PauseCalcLinuxPSI( double* setpoint, const char* tag );

	void	SetInput( const uint64_t total );
	int	Compute();

    private:

	double lastOutput = 0;
	uint64_t instanceInput = 0, lastTotal = 0;
};

// This monitors the whole-system memory availability.

class PauseCalcMemPcts final : public PauseRateCalculatorByResource
{
    public:

	PauseCalcMemPcts( double* setpoint, const char* tag );

	void	SetWindowSize( const int wSize );
	void	SetInput( const uint64_t usedPct );
	int	Compute();
};

# endif // HAS_CPP11
