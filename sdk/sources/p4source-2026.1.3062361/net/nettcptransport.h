// -*- mode: C++; tab-width: 4; -*-
// vi:ts=8 sw=4 noexpandtab autoindent

/*
 * Copyright 1995, 1996 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 * - previously part of nettcp.h
 */

/*
 * nettcptransport.h - TCP driver for NetTransport
 *
 * Classes Defined:
 *
 *    NetTcpTransport - a TCP subclass of NetTransport
 */

class KeepAlive;
class NetTcpSelector;

class NetTcpTransport : public NetTransport {

    public:
			NetTcpTransport( int t, bool fromClient );
			~NetTcpTransport();

	virtual void	SetupSocket();
	virtual void	MoreSetupSocket();
	virtual void	SetSockBlocking( int fd, bool blocking );
	virtual void	SetupKeepAlives( int t );

	virtual void	SetNagle( int nagle );
	virtual void	SetQuickAck( int fd, bool mode );
	virtual void	SetQuickAck( int fd ); // use cached net.quickack
	virtual void	SetQuickAck();

	void		Send( const char *buffer, int length, Error *e );
	int		Receive( char *buffer, int length, Error *e );
	void		Close();
	virtual bool	IsAccepted()
			{
			    return isAccepted;
			}
	void		ClientMismatch( Error *e );
	int		SendOrReceive( NetIoPtrs &io, Error *se, Error *re );
	int		DuplexReady();

	bool		HasAddress() { return true; }
	StrPtr	*	GetAddress( int raf_flags )
			{
			    GetAddress( t, raf_flags, myAddr );
			    return &myAddr;
			}

	StrPtr	*	GetPeerAddress( int raf_flags );

	int		GetPortNum()
			{
			    return GetPortNum( t );
			}

	bool		IsSockIPv6()
			{
			    return IsSockIPv6( t );
			}

	static void	GetAddress( int t, int raf_flags, StrBuf &addr );
	static void	GetPeerAddress( int t, int raf_flags, StrBuf &addr );
	static int	GetPortNum( int t );
	static bool	IsSockIPv6( int t );
	static bool	IsRetryError( int err );


	int		IsAlive();

	void		SetBreak( KeepAlive *b ) { breakCallback = b; }
	void		CloseSocket();
	virtual void	Shutdown( Error *re, Error *se );
	void		Shutdown();

	int		GetSendBuffering();
	int		GetRecvBuffering();
	const NetPortParser &GetPortParser() const;
	void            SetPortParser(const NetPortParser &portParser);

	int             GetMaxWait();
	void            SetMaxWait( const int maxWait );

	int		GetFd() { return t; }
	int		GetInfo( StrBuf * );

    protected:
#ifdef OS_NT
	bool		SetWin32KeepAlives(
			    int		socket,
			    const long	ka_idlesecs,
			    const long	ka_intvlsecs);
#endif // OS_NT
	int 		Peek( int fd, char *buffer, int length );

	int		t;
	KeepAlive	*breakCallback;
	int		lastRead;    // to avoid server TIME_WAIT
	NetTcpSelector	*selector;
	bool             isAccepted;
	bool		shutdownCalled;

    private:
	StrBuf		myAddr;
	StrBuf		peerAddr;
	NetPortParser   portParser;
	int             maxWait;	// in ms

    protected:
	bool		quickAck;	// cached net.quickack
	bool		afterReload;	// configurables have been reloaded
} ;

