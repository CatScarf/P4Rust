/*
 * Copyright 1995, 2009 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# include <stdhdrs.h>
# include <strbuf.h>
# include <intarray.h>
# include "jnltrack.h"

# include <ctype.h>

# include "debug.h"

JnlTracker::JnlTracker()
    : pids( 10 ), ckp( 0 )
{
	Clear();
}

void
JnlTracker::Clear()
{
	npids = maxpids = consistentcnt = markcnt[0] = markcnt[1] = 0;
	scanstate = 0;
	sawJTrailer = 0;
	txCount = 0;
}

int
JnlTracker::Marker( long pid, Kind kind, bool skipEmpty )
{
	++markcnt[ kind ];
	bool found = false;
	int debug = p4debug.GetLevel( DT_RPL );

	for ( int i = 0; i < npids; ++i )
	{
	    if( pids[i] == pid )
	    {
		found = true;
		if( kind == END )
		{
		    // existing end
		    if( i < --npids )
			pids[ i ] = pids[ npids ];
		    goto done;
		}
		// existing middle
		return 0;
	    }
	}

	// If the only record processed so far is the mx marker, skip the pid
	if( kind == MIDDLE && txCount <= 1 && debug >= 6)
	{
	    StrBuf log;
	    log << "Jnl Replay Tracker: Pid " << pid
	        << " in-progress and empty: " << (skipEmpty ? "" : "not ")
	        << "skipping";
	    p4debug.Event();
	    p4debug.printf( "%s\n", log.Text() );
	}

	if( kind == MIDDLE && txCount <= 1 && skipEmpty )
	    return 0;

	if( kind == MIDDLE )
	{
	    // new middle
	    pids[ npids++ ] = pid;
	    if( npids > maxpids )
		maxpids = npids;
	    return 0;
	}

	// new end
	// 0 if not consistent, 1 if consistent
 done:
	if( npids )
	{
	    if( maxpids < npids + 1 )
		maxpids = npids + 1;

	    if( debug >= 5 )
	    {
		StrBuf log;
		log << "Jnl Replay Tracker: Pid " << pid << " complete, "
		    << npids << " remaining: ";
		for( int j = 0; j < npids; j++ )
		    log << (j ? ", " : "") << pids[j];
		p4debug.Event();
		p4debug.printf( "%s\n", log.Text() );
	    }

	    return 0;
	}
	if( !maxpids )
	    maxpids = 1;
	++consistentcnt;

	if( debug >= 5 )
	{
	    StrBuf log;
	    log << "Jnl Replay Tracker: Pid " << pid << " complete, 0 remaining";
	    if( !found && txCount <= 1 )
	        log << " (no work to commit)";
	    p4debug.Event();
	    p4debug.printf( "%s\n", log.Text() );
	}
	return 1;
}

void
JnlTracker::ScanJournal( const char *p, int l, const char **xact )
{
	while( l-- )
	{
	    int	c = *p++;

	    switch( scanstate )
	    {
	    case 0:
		// looking for string start at start of line
		// start
		if( c == '@' )
		    scanstate = 1;
		break;
	    case 1:
		// starting string at start of line
		scanstate = 4;
		if( c == 'm' )
		{
		    scankind = MIDDLE;
		    break;
		}
		else if( c == 'e' )
		{
		    scankind = END;
		    break;
		}
		else if( c == 'n' )
		{
		    // starting a journal note
		    scanstate = 9;
		    break;
		}
		// fall through...
	    instring:
		// in a string, need to end it
		scanstate = 2;
		// fall through...
	    case 2:
		// instring
		if( c == '@' )
		    scanstate = 3;
		break;
	    outstring:
		// out of that string
		scanstate = 3;
		// fall through...
	    case 3:
		// outstring
		if( c == '@' )
		    scanstate = 2;
		else if( c == '\n' )
		    scanstate = 0;
		break;
	    case 4:
		if( c == 'x' )
		    scanstate = 5;
		else
		    goto instring;
		break;
	    case 5:
		if( c == '@' )
		    scanstate = 6;
		else
		    scanstate = 2;
		break;
	    case 6:
		if( isdigit( c ) )
		{
		    scanpid = c - '0';
		    scanstate = 7;
		}
		else if( c != ' ' )
		    goto outstring;
		break;
	    case 7:
		if( isdigit( c ) )
		    scanpid = scanpid * 10 + c - '0';
		else
		{
		    if( c == ' ' && Marker( scanpid, scankind ) && xact )
			scanstate = 8;
		    else
			scanstate = 3;
		}
		break;
	    case 8:
		if( c == '\n' )
		{
		    scanstate = 0;
		    *xact = p;
		}
		break;
	    case 9:
		// starting a journal note
		if( c == 'x' )
		    scanstate = 10;
		else
		    goto instring;
		break;
	    case 10:
		if( c == '@' )
		    scanstate = 11;
		else
		    scanstate = 2;
		break;
	    case 11:
		// is this a server restart note
		if( c == ' ' )
		    break;
		if( c == '7' )
		{
		    Restart();	    // yes! server restart note
		    if( xact )
		    {
			scanstate = 8;
			break;
		    }
		}
		else if( c == '3' )
		{
		    scanstate = 12;
		    break;
		}
		else if( ckp && c == '1' )
		{
		    scanstate = 12;
		    break;
		}
		goto outstring;
	    case 12:
		if( c == ' ' )
		{
		    sawJTrailer = 1;
		}
		goto outstring;
	    }
	}
}

int
JnlTracker::SawJournalTrailer()
{
	return sawJTrailer;
}

