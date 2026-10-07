// -*- mode: C++; tab-width: 4; -*-
// vi:ts=8 sw=4 noexpandtab autoindent

/*
 * NetTcpTransport
 *
 * Copyright 1995, 1996, 2011 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 * - previously part of nettcp.cc
 */

# define NEED_ERRNO
# define NEED_SIGNAL
# ifdef OS_NT
# define NEED_FILE
# endif
# define NEED_FCNTL
# define NEED_IOCTL
# define NEED_TYPES
# define NEED_SLEEP
# define NEED_SOCKET_IO

# ifdef OS_MPEIX
# define _SOCKET_SOURCE /* for sys/types.h */
# endif

# include <stdhdrs.h>
# include <ctype.h>

# include <error.h>
# include <strbuf.h>
# include "netaddrinfo.h"
# include <bitarray.h>
# include <debug.h>
# include <tunable.h>
# include <keepalive.h>
# include <msgrpc.h>
# include <timer.h>

# include "netportparser.h"
# include "netconnect.h"
# include "nettcpendpoint.h"
# include "nettcptransport.h"
# include "netselect.h"
# include "netport.h"
# include "netdebug.h"
# include "netutils.h"
# include "netsupport.h"

# ifdef OS_NT
#    ifndef SIO_KEEPALIVE_VALS
        struct tcp_keepalive {
          unsigned long onoff;
          unsigned long keepalivetime;
          unsigned long keepaliveinterval;
        };
#       define SIO_KEEPALIVE_VALS _WSAIOW(IOC_VENDOR,4)
#    else
#        include <mstcpip.h>
#    endif
# endif // OS_NT

# define PEEK_TIMEOUT 200 /* 200 milliseconds */

/*
 * Server:
 * p4tunables::list[] has values from the defns in debug.cc + loaded by
 * LoadConfig() at server startup (or admin restart).
 *
 * Client:
 * Socket setup is done in NetTcpEndPoint::Connect().
 */
NetTcpTransport::NetTcpTransport( int t, bool fromClient )
: isAccepted(fromClient)
, shutdownCalled(false)
, quickAck(false)
, afterReload(false)
{
	this->t = t;
	breakCallback = 0;
	lastRead = 0;
	maxWait = -1;
	selector = new NetTcpSelector( t );

	/*
	 * accept() or connect() have already completed,
	 * so switch to non-blocking mode for send/receive.
	 */
	SetSockBlocking( t, false );

	TRANSPORT_PRINTF( DEBUG_CONNECT,
		"NetTcpTransport %s connected to %s",
	        GetAddress( RAF_PORT )->Text(),
	        GetPeerAddress( RAF_PORT )->Text() );

	/*
	 * proxy (and maybe broker) don't call SetupSocket()
	 * on accepted sockets so we'll always do it,
	 * even though p4d will also do it later.
	 */
	SetupSocket();
	afterReload = true;
}

/*
 * Server:
 * Called from Rh::Run() in a child request-handler process
 * after the child proc has reloaded values via LoadConfig()
 * (so the relevant configurables may have changed since instantiation).
 *
 * Client:
 * Called from NetTcpEndPoint::Connect() after construction.
 *
 * For both client and server, socket setup is also done in the
 * transport constructor, but possibly with different config values.
 */
void
NetTcpTransport::SetupSocket()
{
	TRANSPORT_PRINTF( DEBUG_CONNECT, "NetTcpTransport::SetupSocket(fd=%d, reload=%d)", t, afterReload );
const int autotune = p4tunable.Get( P4TUNE_NET_AUTOTUNE );
TRANSPORT_PRINTF( DEBUG_CONNECT, "NetTcpTransport::SetupSocket(fd=%d, reload=%d, autotune=%d)", t, afterReload, autotune );
	SetupKeepAlives( t );
	NetUtils::SetupSocketSizes( t, afterReload );

	int nagle = p4tunable.Get( P4TUNE_NET_NAGLE );
	TRANSPORT_PRINTF( DEBUG_CONNECT,
	    "NetTcpTransport: reload=%d, nagle=%d", afterReload, nagle );
	SetNagle( nagle );

	quickAck = p4tunable.Get( P4TUNE_NET_QUICKACK );
	TRANSPORT_PRINTF( DEBUG_CONNECT,
	    "NetTcpTransport: reload=%d, quickAck=%d", afterReload, quickAck );
	SetQuickAck( t, quickAck );

	MoreSetupSocket();
}

// Subclasses can do additional setup here
void
NetTcpTransport::MoreSetupSocket()
{
	TRANSPORT_PRINTF( DEBUG_CONNECT, "NetTcpTransport::MoreSetupSocket(fd=%d, reload=%d)", t, afterReload );
}

NetTcpTransport::~NetTcpTransport()
{
	Close();

	delete selector;
}

/*
 * We want fd to be blocking before connect() or accept()
 * so that we don't need to loop waiting for the connect to complete,
 * and then make it non-blocking to do I/O.
 *
 * Call SetSockBlocking( fd, true ) to make fd blocking.
 * Call SetSockBlocking( fd, false ) to make fd non-blocking.
 */
void
NetTcpTransport::SetSockBlocking( int fd, bool blocking )
{
	/*
	 * SendOrReceive() likes non-blocking I/O.
	 * Without it, it's just synchronous.
	 */

	int rslt = 0;

# ifdef OS_NT
	u_long val = !blocking;
	rslt = ioctlsocket( fd, FIONBIO, &val );
	if( rslt < 0 ) // ie, SOCKET_ERROR (-1)
	{
	    int errnum = WSAGetLastError();
	    StrBuf errbuf;
	    Error::StrError( errbuf, errnum );

	    p4debug.printf(
	        "NetTcpTransport::SetSockBlocking: FIONBIO failed, error=\"%s\" (%d)\n",
	        errbuf.Text(), errnum );
	}
# else
	int flags = fcntl( fd, F_GETFL, 0 );
	if( flags == -1 )
	{
	    int errnum = errno;
	    StrBuf errbuf;
	    Error::StrError( errbuf, errnum );

	    p4debug.printf(
	        "NetTcpTransport::SetSockBlocking: F_GETFL failed, error=\"%s\" (%d)\n",
	        errbuf.Text(), errnum );

	    return;
	}

	if( blocking )
	    rslt = fcntl( fd, F_SETFL, flags & ~O_NONBLOCK );
	else
	    rslt = fcntl( fd, F_SETFL, flags | O_NONBLOCK );

	if( rslt < 0 )
	{
	    int errnum = errno;
	    StrBuf errbuf;
	    Error::StrError( errbuf );

	    p4debug.printf(
		"NetTcpTransport::SetSockBlocking: F_SETFL failed, error=\"%s\" (%d)\n",
		errbuf.Text(), errnum );
	}
# endif
}

/*
 * enable/disable Quick ACK
 * - 0: do nothing [Quick ACK auto-disables after each read() or recv()]
 * - 1: set TCP_QUICKACK (enable Quick ACK)
 * - default: 0 (for backwards compatibility)
 */
void
NetTcpTransport::SetQuickAck( int fd, bool mode )
{
	TRANSPORT_PRINTF( DEBUG_INFO,
	    "NetTcpTransport::SetQuickAck(fd=%d, TCP_QUICKACK, enable=%d, reload=%d)",
		fd, mode, afterReload );

	NetUtils::SetQuickAck( fd, mode );
}

void
NetTcpTransport::SetQuickAck( int fd )
{
	SetQuickAck( fd, quickAck );
}

void
NetTcpTransport::SetQuickAck()
{
# if defined(OS_NT)
    	/*
	 * On Windows QuickACK is sticky so there's no need to set it again;
	 * if you need to change it, call one of the two other methods.
	 * We log it here anyway so that "socket-nagle-quickack.t"
	 * can count the calls.
	 */

# else
	SetQuickAck( t, quickAck );
# endif
}

# ifdef OS_NT
/*
 * Set the keepalive parameters on Windows
 * - enable/disable keepalives can be set via setsockopt
 * - On Windows Vista and later the keepalive count is fixed at 10
 * - both the idle time and the interval time must be set in the same call
 * - this routine sets only idle and interval, and only if both are positive
 * - quietly return success if the default values (both 0) are used
 * - else complain and return failure if either are negative
 */
bool
NetTcpTransport::SetWin32KeepAlives(
	int		socket,
	const long	ka_idlesecs,
	const long	ka_intvlsecs)
{
	// default values -- don't set, don't complain, and return success
	if( (ka_idlesecs == 0) && (ka_intvlsecs == 0) )
	{
	    return true;
	}

	// if either are non-positive, complain and return failure without setting
	if( (ka_idlesecs <= 0) || (ka_intvlsecs <= 0) )
	{
	    TRANSPORT_PRINTF( DEBUG_CONNECT,
		"NetTcpTransport: not setting TCP keepalive idle = %d secs, interval = %d secs"
		" because both must be positive", ka_idlesecs, ka_intvlsecs );
	    return false;
	}

	DWORD	retcnt;
	struct tcp_keepalive
		keepalive_opts;

	int	ka_idle_msecs = ka_idlesecs * 1000;
	int	ka_intvl_msecs = ka_intvlsecs * 1000;

	// count is fixed to 10
	keepalive_opts.onoff = 1;
	keepalive_opts.keepalivetime = ka_idle_msecs;
	keepalive_opts.keepaliveinterval = ka_intvl_msecs;

	TRANSPORT_PRINTF( DEBUG_CONNECT,
	    "NetTcpTransport: setting TCP keepalive idle = %d secs, interval = %d secs",
		ka_idlesecs, ka_intvlsecs );

	int rv = WSAIoctl( socket, SIO_KEEPALIVE_VALS, &keepalive_opts,
		sizeof(keepalive_opts), NULL, 0, &retcnt, NULL, NULL );
	if( rv )	// error
	{
	    StrBuf errnum;
	    Error::StrNetError( errnum );
	    p4debug.printf(
	    	"NetTcpTransport WSAIoctl(idle=%d, interval=%d) failed, error = %s\n",
		keepalive_opts.keepalivetime, keepalive_opts.keepaliveinterval, errnum.Text() );
	}

	return rv == 0;
}
# endif // OS_NT

void
NetTcpTransport::SetupKeepAlives( int t )
{
# ifdef SO_KEEPALIVE
	const SOCKOPT_T one = 1;

	// turn on tcp keepalives unless user disabled it (in which case turn them off)
	int ka_disable = p4tunable.Get( P4TUNE_NET_KEEPALIVE_DISABLE );
	if( ka_disable )
	{
	    const SOCKOPT_T zero = 0;

	    TRANSPORT_PRINT( DEBUG_CONNECT, "NetTcpTransport: disabling TCP keepalives" );

	    do_setsockopt( "NetTcpTransport", t, SOL_SOCKET, SO_KEEPALIVE, &zero, sizeof( zero ) );
	}
	else
	{
	    TRANSPORT_PRINT( DEBUG_CONNECT, "NetTcpTransport: enabling TCP keepalives" );

	    do_setsockopt( "NetTcpTransport", t, SOL_SOCKET, SO_KEEPALIVE, &one, sizeof( one ) );
# ifdef OS_NT
	    const long ka_idlesecs = p4tunable.Get( P4TUNE_NET_KEEPALIVE_IDLE );
	    const long ka_intvlsecs = p4tunable.Get( P4TUNE_NET_KEEPALIVE_INTERVAL );
	    SetWin32KeepAlives( t, ka_idlesecs, ka_intvlsecs );
# else // OS_NT

	    // set tcp keepalive count if user requested
	    const SOCKOPT_T ka_keepcount = p4tunable.Get( P4TUNE_NET_KEEPALIVE_COUNT );
	    if( ka_keepcount )
	    {
// some systems (eg, Darwin 10.5.x) support keepalive but not the tuning parameters!
# if defined(SOL_TCP) && defined(TCP_KEEPCNT)
		TRANSPORT_PRINTF( DEBUG_CONNECT,
			"NetTcpTransport: setting TCP keepalive count = %d", ka_keepcount );
		do_setsockopt( "NetTcpTransport", t, SOL_TCP, TCP_KEEPCNT, &ka_keepcount, sizeof( ka_keepcount ) );
# else
		TRANSPORT_PRINT( DEBUG_CONNECT,
			"NetTcpTransport: this system does not support setting TCP keepalive count" );
# endif // TCP_KEEPCNT
	    }

	    // set tcp keepalive idle time (in seconds) if user requested
	    const SOCKOPT_T ka_idlesecs = p4tunable.Get( P4TUNE_NET_KEEPALIVE_IDLE );
	    if( ka_idlesecs )
	    {
# if defined(SOL_TCP) && defined(TCP_KEEPIDLE)
		TRANSPORT_PRINTF( DEBUG_CONNECT,
			"NetTcpTransport: setting TCP keepalive idle secs = %d", ka_idlesecs );
		do_setsockopt( "NetTcpTransport", t, SOL_TCP, TCP_KEEPIDLE, &ka_idlesecs, sizeof( ka_idlesecs ) );
# else
		TRANSPORT_PRINT( DEBUG_CONNECT,
			"NetTcpTransport: this system does not support setting TCP keepalive idle time" );
# endif // TCP_KEEPIDLE
	    }

	    // set tcp keepalive interval time (in seconds) if user requested
	    const int ka_intvlsecs = p4tunable.Get( P4TUNE_NET_KEEPALIVE_INTERVAL );
	    if( ka_intvlsecs )
	    {
# if defined(SOL_TCP) && defined(TCP_KEEPINTVL)
		TRANSPORT_PRINTF( DEBUG_CONNECT,
			"NetTcpTransport: setting TCP keepalive interval secs = %d", ka_intvlsecs );
		do_setsockopt( "NetTcpTransport", t, SOL_TCP, TCP_KEEPINTVL, &ka_intvlsecs, sizeof( ka_intvlsecs ) );
# else
		TRANSPORT_PRINT( DEBUG_CONNECT,
			"NetTcpTransport: this system does not support setting TCP keepalive interval" );
# endif // TCP_KEEPINTVL
	    }
# endif // OS_NT
	}
# else
	TRANSPORT_PRINTF( DEBUG_CONNECT,
		"NetTcpTransport: this system does not support TCP keepalive packets" );
# endif // SO_KEEPALIVE
}

StrPtr *
NetTcpTransport::GetPeerAddress( int raf_flags )
{
	if( raf_flags & RAF_REQ )
	{
	    peerAddr.Set( raf_flags & RAF_PORT ? GetPortParser().HostPort()
	                                       : GetPortParser().Host() );
	    return &peerAddr;
	}

	GetPeerAddress( t, raf_flags, peerAddr );
	return &peerAddr;
}

void
NetTcpTransport::GetAddress( int t, int raf_flags, StrBuf &myAddr )
{
	struct sockaddr_storage addr;
	struct sockaddr *saddrp = reinterpret_cast<struct sockaddr *>(&addr);
	TYPE_SOCKLEN addrlen = sizeof addr;

	if( getsockname( t, saddrp, &addrlen ) < 0 || addrlen > sizeof addr )
	{
	    myAddr.Set( "unknown" );
	}
	else
	{
	    NetUtils::GetAddress( addr.ss_family, saddrp, raf_flags, myAddr );
	}
}

void
NetTcpTransport::GetPeerAddress( int t, int raf_flags, StrBuf &peerAddr )
{
	struct sockaddr_storage addr;
	struct sockaddr *saddrp = reinterpret_cast<struct sockaddr *>(&addr);
	TYPE_SOCKLEN addrlen = sizeof addr;

	if( getpeername( t, saddrp, &addrlen ) < 0 || addrlen > sizeof addr )
	{
	    if( addrlen > sizeof addr )
	    {
		DEBUGPRINT( DEBUG_CONNECT,
			"Unable to get peer address since addrlen > sizeof addr.");
	    }
	    else
	    {
		StrBuf buf;
		Error::StrNetError( buf );
		DEBUGPRINTF( DEBUG_CONNECT,
			"Unable to get peer address: %s", buf.Text());
	    }
	    peerAddr.Set( "unknown" );
	}
	else
	{
	    NetUtils::GetAddress( addr.ss_family, saddrp, raf_flags, peerAddr );
	}
}

int
NetTcpTransport::GetPortNum( int t )
{
	struct sockaddr_storage addr;
	struct sockaddr *saddrp = reinterpret_cast<struct sockaddr *>(&addr);
	TYPE_SOCKLEN addrlen = sizeof addr;

	if( getsockname( t, saddrp, &addrlen ) < 0 || addrlen > sizeof addr )
	{
	    StrBuf buf;
	    Error::StrNetError( buf );
	    DEBUGPRINTF( DEBUG_CONNECT,
		    "Unable to get sockname: %s", buf.Text());
	    return -1;
	}
	else
	{
	    return NetUtils::GetInPort( saddrp );
	}
}

bool
NetTcpTransport::IsSockIPv6( int t )
{
	struct sockaddr_storage addr;
	struct sockaddr *saddrp = reinterpret_cast<struct sockaddr *>(&addr);
	TYPE_SOCKLEN addrlen = sizeof addr;

	if( getsockname( t, saddrp, &addrlen ) < 0 || addrlen > sizeof addr )
	{
	    StrBuf buf;
	    Error::StrNetError( buf );
	    DEBUGPRINTF( DEBUG_CONNECT,
		    "Unable to get sockname: %s", buf.Text());
	    return false;
	}
	else
	{
	    return NetUtils::IsAddrIPv6( saddrp );
	}
}

void
NetTcpTransport::Send( const char *buffer, int length, Error *e )
{
	NetIoPtrs io;

	io.sendPtr = (char *)buffer;
	io.sendEnd = (char *)buffer + length;
	io.recvPtr = 0;
	io.recvEnd = 0;

	while( io.sendPtr != io.sendEnd )
	    if( !NetTcpTransport::SendOrReceive( io, e, e ) )
	        return;
}

int
NetTcpTransport::Receive( char *buffer, int length, Error *e )
{
	NetIoPtrs io;

	io.recvPtr = buffer;
	io.recvEnd = buffer + length;
	io.sendPtr = 0;
	io.sendEnd = 0;

	if( NetTcpTransport::SendOrReceive( io, e, e ) )
	    return io.recvPtr - buffer;

	return e->Test() ? -1 : 0;
}

/*
 * NetTcpTransport::SendOrReceive() - send or receive data as ready
 *
 * If data to write and no room to read, block writing.
 * If room to read and no data to write, block reading.
 * If both data to write and room to read, block until one is ready.
 *
 * If neither data to write nor room to read, don't call this!
 *
 * Brain-dead version of NetTcpSelector::Select() indicates both
 * read/write are ready.  To avoid blocking on read, we only do so
 * if not instructed to write.
 *
 * Returns 1 if either data read or written.
 * Returns 0 if neither, meaning EOF or error.
 */

int
NetTcpTransport::SendOrReceive( NetIoPtrs &io, Error *se, Error *re )
{
    	// if data is waiting to be read, don't let a read error stop us
	bool wasReadError = re->Test();	// remember the read error
	int doRead = io.recvPtr != io.recvEnd && (!wasReadError || selector->Peek());
	int doWrite = io.sendPtr != io.sendEnd && !se->Test();

	int dataReady;
	int maxwait = GetMaxWait();
	int autotune = p4tunable.Get( P4TUNE_NET_AUTOTUNE );
	int doGoAround = 0;
	int retCode = 0;
	Timer waitTime;
	if( t < 0 )
	{
	    // Socket has been closed don't try to use
	    return 0;
	}
	if( maxwait )
	{
	    waitTime.Start();
	}

	int readable;
	int writable;

	// Flush can call us with nothing to do when the connection
	// gets broken.

	if( !doRead && !doWrite )
	    return 0;

	// 500 is .5 seconds.
	const int timeout = !maxwait || maxwait > 500 ? 500 : maxwait;

	do
	{
	    readable = doRead;
	    writable = doWrite;

	    int tv = -1;
	    if( ( readable && breakCallback ) || maxwait )
	    {
	        tv = timeout;
	        if( breakCallback )
	        {
	            tv = breakCallback->PollMs();
	            if( tv < 1 )
	                tv = timeout;
	        }
	        if( tv < 1 ) // in case of overflow, be sane
	            tv = timeout;
	    }

	    if( ( dataReady = selector->Select( readable, writable, tv ) ) < 0 )
	    {
	        re->Sys( "select", "socket" );
	        return 0;
	    }

	    if( !dataReady && maxwait && waitTime.Time() >= maxwait )
	    {
	        lastRead = 0;
	        re->Set( MsgRpc::MaxWait ) <<
	            ( doRead ? "receive" : "send" ) << ( maxwait / 1000 );
	        return 0;
	    }

	    // Before checking for data do the callback isalive test.
	    // If the user defined IsAlive() call returns a zero
	    // value then the user wants this request to terminate.

	    if( doRead && breakCallback && !breakCallback->IsAlive() )
	    {
	        lastRead = 0;
	        re->Set( MsgRpc::Break );
	        return 0;
	    }

	    if( !writable && !readable )
	        continue;

	    if( autotune && readable && writable )
	    {
	        doGoAround = 1;
	        writable = 0;
	    }

	    // Write what we can; read what we can

	    if( writable )
	    {
		goAround:
	        int l = write( t, io.sendPtr, io.sendEnd - io.sendPtr );

	        if( l > 0 )
	            TRANSPORT_PRINTF( DEBUG_TRANS, 
	                "NetTcpTransport send %d bytes", l );

	        if( l > 0 )
	        {
	            lastRead = 0;
	            io.sendPtr += l;
	            if( autotune && !readable )
	                return 1;
	            retCode = 1;
	        }

	        if( l < 0 )
	        {
# ifdef OS_NT
	            int	errornum = WSAGetLastError();

	            // don't use switch, values might be the same
                    if( ( errornum == WSAEWOULDBLOCK ) || 
	                ( errornum == WSATRY_AGAIN ) || 
	                ( errornum == WSAEINTR) ) continue;
# else
	            // don't use switch, values might be the same
	            if( ( errno == EWOULDBLOCK ) || 
	                ( errno == EAGAIN ) || 
	                ( errno == EINTR ) ) continue;
# endif // OS_NT

	            se->Net( "write", "socket" );
	            se->Set( MsgRpc::TcpSend );
	        }
	    }

	    if( readable )
	    {
	        int l = read( t, io.recvPtr, io.recvEnd - io.recvPtr );

	        if( l > 0 )
	            TRANSPORT_PRINTF( DEBUG_TRANS, 
	                "NetTcpTransport recv %d bytes", l );

		if( l >= 0 )
		    SetQuickAck();

	        if( l > 0 )
	        {
	            /*
	             * probably we'll never be able to read the FIN if there
	             * isn't data waiting already
	             */
	            lastRead = (wasReadError ? selector->Peek() : 1);
	            io.recvPtr += l;
                    if( doGoAround )
	            {
	                readable = 0;
	                doGoAround = 0;
	                goto goAround;
	            }
	            retCode = 1;
	        }

	        if( l < 0 )
	        {
	            if( doGoAround )
	            {
	                readable = 0;
	                doGoAround = 0;
	                goto goAround;
	            }
# ifdef OS_NT
	            int errornum = WSAGetLastError();
	            // don't use switch, values might be the same
	            if( ( errornum == WSAEWOULDBLOCK ) || 
	                ( errornum == WSATRY_AGAIN ) || 
	                ( errornum == WSAEINTR ) ) continue;
# else
		    // don't use switch, values might be the same
		    if( ( errno == EWOULDBLOCK ) || 
	                ( errno == EAGAIN ) || 
	                ( errno == EINTR ) ) continue;
# endif // OS_NT

	            re->Net( "read", "socket" );
	            re->Set( MsgRpc::TcpRecv );
	        }
	    }

	    return retCode;
	} while( !retCode );

	return retCode;
}

int
NetTcpTransport::DuplexReady()
{
	/*
	 * Always return true (1) unless net.autotune is set
	 * to the new value (2), which will return false (0).
	 *
	 * The (new and undoc) net.autotune value of 2
	 * behaves like net.autotune=1 did prior to job110437
	 * (returns !net.autotune).
	 * This is very useful for testing, as well as providing
	 * a way to revert behavior in the field if this change
	 * causes trouble for a customer.
	 *
	 * Prior to this change, DuplexReady() always returned false
	 * if net.autotune was enabled, which makes Rpc::Dispatch() 2nd-level
	 * Dispatch() calls do nothing (if not flushing or handling errors);
	 * it also makes RpcMulti::Dispatch() calls not dispatch callbacks.
	 * As a result, under some circumstances (the details of which I don't
	 * fully understand but seem to require submitting much data and many
	 * files), Rpc and/or MultiRpc stop dispatching callbacks and
	 * the data transfer stalls forever.
	 *
	 * This change enables Rpc and MultiRpc to dispatch callbacks
	 * when net.autotune is enabled, preventing these stalls.
	 */

	int	autotune = p4tunable.Get( P4TUNE_NET_AUTOTUNE );

	return autotune < 2 ? 1 : 0;
}

int
NetTcpTransport::GetSendBuffering()
{
	int sz = 4096;

# ifdef SO_SNDBUF
	TYPE_SOCKLEN rsz = sizeof( sz );

	if( ::getsockopt( t, SOL_SOCKET, SO_SNDBUF, (char *)&sz, &rsz ) < 0 )
	    sz = 4096;
# endif

# ifdef OS_LINUX
	// Linux says one thing but means another, reserving 1/4th
	// the space for internal use.

	sz = sz * 3 / 4;
# endif

# ifdef OS_DARWIN
	// Darwin's window-size adaptive mechanism returns inflated values
	// in getsockopt() from what sysctl reports in send and recv spaces,
	// at least until net.inet.tcp.sockthreshold sockets are open.
	// Our technique of filling the pipe can cause dead-lock in
	// extreme cases, so we manually avoid Darwin's mechanism here for
	// the send buffer.

	if( sz > DARWIN_MAX  )
	    sz = DARWIN_MAX;
# endif

# ifdef SO_SNDLOWAT
	// FreeBSD and Darwin (at least) will block the sending process
	// when writes are smaller than both the low water mark and space
	// available, so don't use that last bit of space.

	int sl;

	if( ::getsockopt( t, SOL_SOCKET, SO_SNDLOWAT, (char *)&sl, &rsz ) == 0 )
	    sz -= sl;
# endif

	return sz;
}

int
NetTcpTransport::GetRecvBuffering()
{
	int sz = 4096;

# ifdef SO_RCVBUF
	TYPE_SOCKLEN rsz = sizeof( sz );

	if( ::getsockopt( t, SOL_SOCKET, SO_RCVBUF, (char *)&sz, &rsz ) < 0 )
	    sz =  4096;
# endif

# ifdef OS_LINUX
	sz = sz * 3 / 4;
# endif

	return sz;
}

void
NetTcpTransport::Close( void )
{
	if( t < 0 )
	    return;

	TRANSPORT_PRINTF( DEBUG_CONNECT, "NetTcpTransport %s closing %s",
	        GetAddress( RAF_PORT )->Text(),
	        GetPeerAddress( RAF_PORT )->Text() );

	// Avoid TIME_WAIT on the server by reading the EOF after
	// the last message sent by the client.  Getting the EOF
	// means we've received the TH_FIN packet, which means we
	// don't have to send our own on close().  He who sends
	// a TH_FIN goes into the 2 minute TIME_WAIT imposed by TCP.

	TRANSPORT_PRINTF( DEBUG_CONNECT,
		"NetTcpTransport lastRead=%d", lastRead );

	if( lastRead )
	{
	    int r = 1;
	    int w = 0;
	    char buf[1];
	    // Only wait a second by default, since it's possible we'll
	    // be in a state where the EOF never comes.
	    const int max = p4tunable.Get( P4TUNE_NET_MAXCLOSEWAIT );

	    if( selector->Select( r, w, max ) >= 0 && r )
	    {
		int n = read( t, buf, 1 );
		if( n < 0 )
		{
		    StrBuf errbuf;

		    Error::StrNetError( errbuf );
		    TRANSPORT_PRINTF( SSLDEBUG_ERROR, "NetTcpTransport::Close: read of FIN packet failed (ignored): %s", errbuf.Text() );
		}
		SetQuickAck();
	    }
	}

	if( DEBUG_INFO )
	{
		StrBuf b;

		if( GetInfo( &b ) )
			p4debug.printf( "tcp info: %s", b.Text() );
	}

	CloseSocket();
}

/*
 * This call is here just for the cases where you want to signal
 * the other end of the connection that you're done writing,
 * but don't necessarily want to close the connection yet.
 *
 * It's harmless to call it just before calling `Close()`,
 * or to call it multiple times, but it's still required
 * to call `Close()` or to delete the transport object, which
 * calls `Close()` for you.
 *
 * Note that `Close()` will call `shutdown()` for you
 * so normally you don't need to call `Shutdown()` yourself.
 */
void
NetTcpTransport::Shutdown( Error *re, Error *se )
{
	Shutdown();
}

/*
 * On linux, calling shutdown() on a socket means subsequent shutdown()
 * calls on the same socket are ignored, and receiving a FIN after
 * already having received a FIN does nothing.
 *
 * On other platforms (notably Windows), I don't know if that's true,
 * so we ensure that we don't call shutdown() twice on our own socket.
 */
void
NetTcpTransport::Shutdown()
{
    TRANSPORT_PRINTF( DEBUG_CONNECT,
	"*** NetTcpTransport::Shutdown(): t=%d, shutdownCalled=%d, %s <--> %s",
	t,
	shutdownCalled,
	GetAddress( RAF_PORT )->Text(),
	GetPeerAddress( RAF_PORT )->Text() );

	if( !shutdownCalled && (t >= 0) )
	{
	    // we want the client to initiate shutdown
	    if( !IsAccepted() )
	    {
		TRANSPORT_PRINTF( DEBUG_CONNECT,
		    "NetTcpTransport shutting down connection: %s <--> %s",
		    GetAddress( RAF_PORT )->Text(),
		    GetPeerAddress( RAF_PORT )->Text() );

		shutdownCalled = true;

#if defined(SHUT_WR) // POSIX
		shutdown( t, SHUT_WR );
#else // Windows
		shutdown( t, SD_SEND );
#endif
	    }
	}
}

void
NetTcpTransport::CloseSocket()
{
	if( t >= 0 )
	{
	    Shutdown();
	    close( t );
	    t = -1;
	}
}

void
NetTcpTransport::SetNagle( int nagle )
{
	TRANSPORT_PRINTF( DEBUG_CONNECT,
	    "NetTcpTransport::SetNagle(fd=%d, reload=%d, enable=%d)",
	    t, afterReload, nagle );

	NetUtils::SetNagle( t, nagle );
}

int
NetTcpTransport::IsAlive()
{
	int readable = 1;
	int writeable = 0;

	if( selector->Select( readable, writeable, 0 ) < 0 )
	    return 0;

	// All good if no EOF waiting

	return !readable || selector->Peek();
}

void
NetTcpTransport::ClientMismatch( Error *e )
{
    switch( CheckForHandshake( t ) )
    {
    case PeekSSL:
        // this is a non-ssl connection
        // this is a ssl connection and we are a cleartext server
        e->Net( "accept", "socket" );
        e->Set( MsgRpc::TcpPeerSsl );
        CloseSocket();
	break;
    case PeekTimeout:
    default:
	break;
    }
}

/*
 * Don't pass in "errno"; pass "GetLastSockError()" instead
 * (or call the no-arg version which will do it for you)
 * so that it works on Windows as well.
 * [static]
 */
bool
NetTcpTransport::IsRetryError( int err )
{
#ifdef OS_NT
	if( err == WSAEWOULDBLOCK || err == WSATRY_AGAIN || err ==  WSAEINTR )
	    return true;
#else
	if( err == EWOULDBLOCK || err == EAGAIN || err == EINTR )
	    return true;
#endif

	return false;
}

/*
 * PEEK_TIMEOUT is hard-coded; should we consider making it a tunable?
 * This would allow customers to tune their Peek timeout settings
 * (eg, if their clients are typically slow to send their first packet)
 * and make it easier to test our handling of Peek timeouts.
 * The main use of Peek() is to give better error messages in case
 * of SSL client to plain-text server (or vice versa) connection attempts,
 * rather than the somewhat unclear error messages that would occur
 * without this check.
 *
 * We might also consider using selector->Select() in our wait loop
 * in order to reduce CPU usage.
 */
int
NetTcpTransport::Peek( int fd, char *buffer, int length )
{
		int count = 0;
		int retval = -1;

# ifdef OS_NT
		// Set to blocking on windows for peek
		SetSockBlocking( t, true );
		retval = recv( fd, buffer, length, MSG_PEEK );
		// Set back to non blocking, @#!%$#@ stupid windows....
		SetSockBlocking( t, false );
# else
		retval = recv( fd, buffer, length, MSG_PEEK );
		// lengthened timeout because found out that timed out early with VPN
		while( (retval == -1) && IsRetryError(errno) && (count < PEEK_TIMEOUT) )
		{
			// parent process closing socket can make
			// resource temporarily unavailable.
			msleep(1);
			retval = recv( fd, buffer, length, MSG_PEEK );
			count++;
		}
# endif

		if( retval == -1 && count < PEEK_TIMEOUT )
		{
		    TRANSPORT_PRINTF( SSLDEBUG_ERROR,
			    "Peek error is: %d", GetLastSockError());
		}
		return retval;
}

const NetPortParser &
NetTcpTransport::GetPortParser() const
{
    return portParser;
}

void NetTcpTransport::SetPortParser(const NetPortParser &portParser)
{
    this->portParser = portParser;
}

int
NetTcpTransport::GetMaxWait()
{
	if( maxWait < 0 )
	{
	    maxWait = p4tunable.Get( P4TUNE_NET_MAXWAIT ) * 1000;
	}

	return maxWait;
}

void
NetTcpTransport::SetMaxWait( const int maxWait )
{
	this->maxWait = maxWait;
}

int
NetTcpTransport::GetInfo( StrBuf *b )
{
# ifdef TCP_INFO
		if( !b )
			return 0;
# define TD( x ) *b << #x << " " << tinfo.tcpi_ ## x << "\t";

		struct tcp_info tinfo;
		socklen_t sl = sizeof tinfo;

		if( ::getsockopt( t, IPPROTO_TCP, TCP_INFO, (void *)&tinfo, &sl ) >= 0 )
		{
# ifdef OS_FREEBSD
		    b->UAppend( "options" );
# else
		    *b << "retransmits " << (int)tinfo.tcpi_retransmits << "\t";
		    *b << "probes " << (int)tinfo.tcpi_probes << "\t";
		    *b << "backoff " << (int)tinfo.tcpi_backoff << "\noptions";
# endif
			if( tinfo.tcpi_options & TCPI_OPT_TIMESTAMPS )
				*b << " timestamps";
			if( tinfo.tcpi_options & TCPI_OPT_SACK )
				*b << " sack";
			if( tinfo.tcpi_options & TCPI_OPT_WSCALE )
				*b << " wscale";
			if( tinfo.tcpi_options & TCPI_OPT_ECN )
				*b << " ecn";
# ifdef TCPI_OPT_ECN_SEEN
			if( tinfo.tcpi_options & TCPI_OPT_ECN_SEEN )
				*b << " ecn_seen";
# endif
# ifdef TCPI_OPT_SYN_DATA
			if( tinfo.tcpi_options & TCPI_OPT_SYN_DATA )
				*b << " syn_data";
# endif
# ifdef TCPI_OPT_TOE
			if( tinfo.tcpi_options & TCPI_OPT_TOE )
				*b << " toe";
# endif
		    *b << "\nsscale " << (int)tinfo.tcpi_snd_wscale << "\t";
		    *b << "rscale " << (int)tinfo.tcpi_rcv_wscale << "\n";

			TD( rto );
# ifdef OS_LINUX
			TD( ato );
# endif
			TD( snd_mss );
		    *b << "rcv_mss " << tinfo.tcpi_rcv_mss << "\n";

# ifdef OS_LINUX
			TD( unacked );
			TD( sacked );
			TD( lost );
			TD( retrans );
		    *b << "fackets " << tinfo.tcpi_fackets << "\n";

			TD( last_data_sent );
# endif
			TD( last_data_recv );
# ifdef OS_LINUX
			TD( last_ack_recv );

			b->Extend( '\n' );

			TD( pmtu );
			TD( rcv_ssthresh );
# endif
			TD( rtt );
			TD( rttvar );
			b->Extend( '\n' );
			TD( snd_ssthresh );
			TD( snd_cwnd );
# ifdef OS_LINUX
			TD( advmss );
			TD( reordering );

			b->Extend( '\n' );

			//			TD( rcv_rtt );
			//		    p4debug.printf( "total retrans %u\n", tinfo.tcpi_total_retrans );
# endif
# ifdef OS_FREEBSD
			TD( rcv_space );
			TD( snd_wnd );
			TD( snd_bwnd );
			TD( snd_nxt );
			TD( rcv_nxt );
			b->Extend( '\n' );
			TD( toe_tid );
			TD( snd_rexmitpack );
			TD( rcv_ooopack );
			TD( snd_zerowin );
			b->Extend( '\n' );
# endif
			b->Terminate();
			return 1;
		}
# endif
		return 0;
}
