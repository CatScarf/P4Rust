/*
 * Copyright 1995, 2021 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

/*
 * serverid.h - Read/Write the serverid to the server.id file.
 *
 */

void ValidateSvrID ( const StrPtr &, Error * );

class ServerIDAccess {

    public:
	ServerIDAccess():serverIDFile( "server.id" ){ }
	~ServerIDAccess(){ }

	int		HasServerID( );
	void		ReadServerID( Error * );
	void		WriteServerID( const StrPtr &, Error * );
	void		SetSIDMessage( StrPtr *msg );
	StrBuf		GetServerID() const { return serverID; }
	StrBuf		GetSIDMessage() const  { return sIDMessage; }

    private:
	StrBuf		serverID;
	StrBuf		sIDMessage;
	const char	*serverIDFile;
};
