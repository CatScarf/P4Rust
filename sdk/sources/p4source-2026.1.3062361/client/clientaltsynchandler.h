/*
 * Copyright 1995, 2023 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

class PipeIo;

class ClientAltSyncHandler : public LastChance {
    public:
	static int	IsSupported();
	static ClientAltSyncHandler *
			GetAltSyncHandler( Client *client, Error *e );
	

			ClientAltSyncHandler( Client *c ) : started( 0 ),
			                                    client( c ),
			                                    pipe(0) {}
			~ClientAltSyncHandler() { Error e; End( &e ); }
	void		Start( Error *e );
	int		IsAlive();
	int		End( Error *e );
	int		AltSync( Error *e, StrDict *results = 0,
			         bool *pass = 0 );

	StrPtr		GetClientRoot() { return clientRoot; }

    private:
	void		FillDict( StrDict &dict );

	int		started;
	Client		*client;
	RunCommandIo	rc;
	PipeIo		*pipe;
	StrBuf		readBuf;

	StrBuf		clientRoot;

} ;
