/*
 * Copyright 1995, 2023 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# define NEED_WINDOWSH
# include <stdhdrs.h>

# ifdef HAS_CPP11

# include <error.h>
# include <errorlog.h>
# include <strbuf.h>
# include <debug.h>
# include <tunable.h>
# include <datetime.h>
# include <strdict.h>
# include <strtable.h>

# define __STDC_FORMAT_MACROS
# include <inttypes.h>

# include "pressuremonitor.h"

# include <PID_v1.h>
# include <PID_v1.cpp>
# include <algorithm> 

////////////////////////////////////////////////////////////////////////////////

PressureMonitor::~PressureMonitor()
{
}

# ifdef OS_NT

# include <memoryapi.h>

PressureMonitorWin::PressureMonitorWin( PressureMonitor::Type t, Error* e )
	: h( 0 )
{
	// https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-creatememoryresourcenotification
	h = CreateMemoryResourceNotification( LowMemoryResourceNotification );

	if( h == NULL )
	{
	    StrBuf buf, msg;
	    e->StrError( buf ); // Get GetLastError()'s text.
	    msg << "PressureMonitorWin: " << buf;
	    e->Set( E_FATAL, msg.Text() );
	}
}

PressureMonitorWin::~PressureMonitorWin()
{
	// Ignores the return.

	if( h )
	    CloseHandle( h );
}

bool PressureMonitorWin::Exists() const
{
	if( !h )
	    return false;

	BOOL exists = false;
	// https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-querymemoryresourcenotification
	const BOOL r = QueryMemoryResourceNotification( h, &exists );

	return ( !r || !exists ) ? false : true;
}

# endif // OS_NT

std::unique_ptr< PressureMonitor >
PressureMonitor::MkPressureMonitor( PressureMonitor::Resource r,
	                            PressureMonitor::Type t,
	                            Error* e )
{
	if( r == PressureMonitor::Resource::CPU )
	{
	    // Nothing for Windows.
	    return {};
	}

	// Memory monitor.

# if defined( OS_NT )
	return std::make_unique< PressureMonitorWin >( t, e );
# endif

	return {};
}

////////////////////////////////////////////////////////////////////////////////

void MovingAverage::SetSize( const int s )
{
	if( values && values->capacity() == s )
	    return;

	values.reset( new std::vector< uint64_t >( s, 0 ) );
}

double MovingAverage::Calculate( const uint64_t value )
{
	total += value;
	const size_t capacity = values->capacity();

	if( nValues >= capacity )
	{
	    uint64_t& oldest = (*values)[ nValues++ % capacity ];
	    total -= oldest;
	    oldest = value;
	}
	else
	    (*values)[ nValues++ ] = value;

	return (double)total / (double)(std::min)( nValues, capacity );
}

////////////////////////////////////////////////////////////////////////////////

PauseRateCalculatorByResource::PauseRateCalculatorByResource( double* setpoint,
	                                                      const char* tag )
	: type( tag )
{
	this->setpoint = setpoint;

	pid = new PID( &input, &output, setpoint, aggKp, aggKi, aggKd,
	               PID_P_ON_E, PID_REVERSE );

	// Range of percentages.
	((PID*)pid)->SetOutputLimits( 0, 100 );
	((PID*)pid)->SetMode( PID_AUTOMATIC );

	avg.SetSize( 1 );
}


PauseRateCalculatorByResource::~PauseRateCalculatorByResource()
{
	delete (PID*) pid;
}

void PauseRateCalculatorByResource::SetTunings( const double aKp,
	                                        const double aKi,
	                                        const double aKd,
	                                        const double cKp,
	                                        const double cKi,
	                                        const double cKd )
{
	aggKp = aKp;
	aggKi = aKi;
	aggKd = aKd;

	consKp = cKp;
	consKi = cKi;
	consKd = cKd;

	((PID*)pid)->SetTunings( aggKp, aggKi, aggKd );
}

void PauseRateCalculatorByResource::SetWindowSize( const int wSize )
{
	avg.SetSize( wSize );
}

void PauseRateCalculatorByResource::SetInput( const uint64_t in )
{
	p4debug.printf( "Wrong SetInput()!\n" );
	abort();
}

int PauseRateCalculatorByResource::Compute()
{
	p4debug.printf( "Wrong Compute()!\n" );
	abort();
	return 0;
}

double PauseRateCalculatorByResource::GetInput() const
{
	return input;
}

void PauseRateCalculatorByResource::Initialize()
{
	if( initialized )
	    return;

	initialized = true;

	/*
	  The point of calling SetMode is to call PID::Initialize(),
	  which sets PID::lastInput, which when set to the current
	  input value, prevents the controller from seeing a zero->currentInput
	  change in input, causing it output a non-zero result the
	  second time it's called.  The end result was that the server,
	  immediately after starting, would issue a single 'under pressure'
	  message.  The next call to PID::Compute() would set PID::lastInput
	  and all would be fine afterwards.
	*/

	((PID*)pid)->SetMode( PID_MANUAL );
	((PID*)pid)->SetMode( PID_AUTOMATIC );
}

////////////////////////////////////////////////////////////////////////////////

PauseCalcLinuxPSI::PauseCalcLinuxPSI( double* setpoint, const char* tag )
	: PauseRateCalculatorByResource( setpoint, tag )
{
	aggKp = 10;
	aggKi = 10;
	aggKd =  0;

	consKp = 1;
	consKi = 5;
	consKd = 0.1;

	((PID*)pid)->SetTunings( aggKp, aggKi, aggKd, PID_P_ON_M );
}

void PauseCalcLinuxPSI::SetInput( const uint64_t total )
{
	// The 'total' argument is the current PSI 'total' field:
	// some avg10=0.00 avg60=0.00 avg300=0.02 total=3824517857
	//
	// The 'tdiff' variable is the difference between the current state
	// and the previous one.

	// Zero-check for the initial state, to prevent a negative number.
	// Also check for wraparound of the OS counter.

	const uint64_t tdiff = ( lastTotal == 0 ||
	                         ( (double)total - (double)lastTotal ) < 0 )
	                       ? 0 : total - lastTotal;

	lastTotal = total;
	instanceInput = (uint64_t)( 100 * (double) tdiff / 100000 );

	// The input range is from 0-~100000, where the high end is equal to
	// a 100% stall rate.  The value can actually go up a bit from that,
	// but it will be computed as a 100% pause rate regardless, so it
	// doesn't matter.

	// Scale the input to the 0-100 range.
	input = 100 * (double) avg.Calculate( tdiff ) / 100000;
}

int PauseCalcLinuxPSI::Compute()
{
	Initialize();

	/*
	    Because there's a delay between newly-unpaused commands leaving the
	    pausing loop and when they start processing where they'll begin
	    driving up CPU usage, reduce the rate of change when system load is
	    trending downwards so as to let fewer commands in (giving time for
	    their effect to be seen).  This smooths out the load.

	    When load is going up, use the aggressive settings to cap things off
	    fast.
	*/

	bool cons = false;

	if( instanceInput < *setpoint || instanceInput < lastOutput )
	{
	    cons = true;
	    ((PID*)pid)->SetTunings( consKp, consKi, consKd );
	}
	else
	    ((PID*)pid)->SetTunings( aggKp, aggKi, aggKd );

	const double realInput = input;

	const bool r = ((PID*)pid)->Compute();

	lastOutput = output;

	if( p4tunable.Get( DT_RESOURCE ) > 5 )
	{
	    p4debug.Event();
	    p4debug.printf( "PauseCalcLinuxPSI::Compute(%s) "
	                    "input %f, tdiff %" PRIu64 ", output %f, "
	                    "setpoint %f, cons %d\n",
	                    type, /*input*/realInput, instanceInput, output,
	                    *setpoint, cons );
	}

	// If r is false Compute() isn't ready, so return a zero.
	return r ? (int)output : 0;
}

////////////////////////////////////////////////////////////////////////////////

PauseCalcMemPcts::PauseCalcMemPcts( double* setpoint, const char* tag )
	: PauseRateCalculatorByResource( setpoint, tag )
{
	aggKp = 10.0;
	aggKi = 10.0;
	aggKd = 10;

	((PID*)pid)->SetTunings( aggKp, aggKi, aggKd );
}

void PauseCalcMemPcts::SetWindowSize( const int wSize )
{
	avg.SetSize( wSize );
}

void PauseCalcMemPcts::SetInput( const uint64_t usedPct )
{
	input = avg.Calculate( usedPct );
}

int PauseCalcMemPcts::Compute()
{
	Initialize();

	const bool r = ((PID*)pid)->Compute();

	if( p4tunable.Get( DT_RESOURCE ) > 5 )
	{
	    p4debug.Event();
	    p4debug.printf( "PauseCalcMemPcts::Compute(%s) "
	                    "input %f, output %f, setpoint %f\n",
	                    type, input, output, *setpoint );
	}

	return r ? (int)output : 0;
}

# endif // HAS_CPP11

////////////////////////////////////////////////////////////////////////////////
