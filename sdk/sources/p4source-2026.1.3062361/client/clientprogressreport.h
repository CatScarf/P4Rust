/*
 * Copyright 2025 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

 class ClientProgressReport : public ProgressReport {
    public:
	ClientProgressReport( ClientProgress *p ) : cp( p ) {}
	virtual ~ClientProgressReport()
	{
	    if( needfinal )
	        DoReport( CPP_FAILDONE );

	    delete cp;
	}

	virtual void DoReport( int flag )
	{
	    if( !cp )
	        return;

	    if( fieldChanged & ( CP_DESC|CP_UNITS) )
	        cp->Description( &description, units );

	    if( fieldChanged & CP_TOTAL )
	        cp->Total( total );

	    if( fieldChanged & CP_POS )
	        cp->Update( position );

	    fieldChanged = 0;
	    if( flag == CPP_DONE || flag == CPP_FAILDONE )
	    {
	        cp->Done( flag == CPP_FAILDONE );
	        needfinal = 0;
	    }
	}

	bool IsComplete()
	{
	    return total && position == total;
	}

    protected:
	ClientProgress *cp;
} ;

