/*
 * Copyright 1995, 2009 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

class JnlTracker {

    public:
	JnlTracker();

	enum Kind { MIDDLE, END };

	// Returns 1 if at a consistency point, 0 otherwise
	int	Marker( long pid, Kind kind, bool skipEmpty = false );

	int	MarkerCount( Kind kind ) const
		    { return markcnt[ kind ]; }

	int	ConsistentCount() const { return consistentcnt; }

	int	MaxCount() const { return maxpids; }

	int	XactCount() const { return npids; }

	int	AtStartOfRecord() const { return scanstate == 0; }

	void	ScanJournal( const char *, int, const char **xact = 0 );

	void	Clear();

	void	Restart() { npids = 0; txCount = 0; }

	int	SawJournalTrailer();

	void	SetCheckpoint() { ckp = 1; }
	
	void	TxTick() { txCount++; }
	void	TxReset() { txCount = 0; }
	int	TxCount() { return txCount; }

    private:
	IntArray	pids;
	int		npids;
	int		markcnt[2];
	int		consistentcnt;
	int		sawJTrailer;
	int		maxpids;
	int		ckp;

	// for journal scanning
	int		scanstate;
	int		scanpid;
	Kind		scankind;

	// for transaction record counting
	int		txCount;
};
