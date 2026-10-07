/**
 * @file netsslendpoint.cc
 *
 * @brief SSL driver for NetEndPoint
 *	NetSslEndPoint - a TCP with SSL subclass of NetTcpEndPoint
 *
 * Threading: underlying SSL library contains threading
 *
 * @invariants:
 *
 * Copyright (c) 2011 Perforce Software
 * Confidential.  All Rights Reserved.
 * @author Wendy Heffner
 *
 * Creation Date: August 19, 2011
 */

/**
 * NOTE:
 * The following file only defined if USE_SSL true.
 * The setting of this definition is controlled by
 * the Jamrules file.  If the jam build is specified
 * with -sSSL=yes this class will be defined.
 */
# ifdef USE_SSL

# define NEED_ERRNO
# define NEED_SIGNAL
# ifdef OS_NT
# define NEED_FILE
# endif
# define NEED_FCNTL
# define NEED_IOCTL
# define NEED_TYPES

# ifdef OS_MPEIX
# define _SOCKET_SOURCE /* for sys/types.h */
# endif

// Only partial Smart Heap instrumentation.
//
# ifdef MEM_DEBUG
# undef DEFINE_NEW_MACRO
# endif

# include <stdhdrs.h>
# include <error.h>
# include <strbuf.h>
# include "netaddrinfo.h"

extern "C"
{ // OpenSSL

# include <openssl/bio.h>
# include <openssl/ssl.h>
# include <openssl/err.h>

}

# include <errorlog.h>
# include <debug.h>
# include <vararray.h>
# include <bitarray.h>
# include <tunable.h>
# include <enviro.h>
# include <filesys.h>
# include <pathsys.h>

# include <keepalive.h>
# include "netsupport.h"
# include "netport.h"

# include "netportparser.h"
# include "netconnect.h"
# include "nettcpendpoint.h"
# include "nettcptransport.h"
# include "netsslcredentials.h"
# include "netsslendpoint.h"
# include "netssltransport.h"
# include "netsslmacros.h"
# include "netselect.h"
# include "netdebug.h"
# include "netutils.h"
# include <msgrpc.h>

#include <memory>
using namespace std;

////////////////////////////////////////////////////////////////////////////
//  NetSslEndPoint                                                        //
////////////////////////////////////////////////////////////////////////////

/*
 * We're processing `p4 admin restart` or `kill -HUP`
 * - the admin might have changed `certificate.txt`
 *   so ensure that we'll re-read the credentials.
 */
void
NetSslEndPoint::NotifyRestarting()
{
	if( serverCredentials )
	{
	    // claim ownership of cert and key so that
	    // "delete serverCredentials" will also delete the cert and key
	    serverCredentials->SetOwnCert( true );
	    serverCredentials->SetOwnKey( true );
	}

	NetSslTransport::NotifyRestarting();
}

void
NetSslEndPoint::Listen( Error *e )
{
	isAccepted = false;
	if ( !serverCredentials )
	{
	    serverCredentials = new NetSslCredentials();
	    serverCredentials->ReadCredentials( e );
	    if( e->Test() )
	    {
		delete serverCredentials;
		serverCredentials = NULL;
		return;
	    }
	    X509 *cert = serverCredentials->GetCertificate();
	    serverCredentials->CheckCertChainOrder( cert, true, e );
	    if( e->Test() ) {
		p4debug.printf( "NetSslEndPoint::Listen() - invalid certificate chain\n" );
		/*
		 * We should return here (thus making the connection attempt fail)
		 * but I'm afraid that we'd break some customers
		 * who are running with not-quite-valid certs.
		 */
		//return;
	    }
	}
	NetTcpEndPoint::Listen(e);
}

/**
 * NetSslEndPoint::ListenCheck
 *
 * @brief Method stubbed out in ssl version of endpoint
 *
 * @param e, Error pointer to hand back any error state
 */
void
NetSslEndPoint::ListenCheck( Error *e )
{
	/*
	 * This operation should never be performed on an
	 * SSL endpoint.  We do not want to allow the
	 * NetTcpEndPoint::ListenCheck to be used
	 * if ssl.
	 */
	e->Set( MsgRpc::SslInvalid ) << GetPortParser().String().Text();
	return;
}

// Called from NetTcpEndPoint::SocketSetup() to do additional setup
void
NetSslEndPoint::MoreSocketSetup( int fd, AddrType type, Error *e )
{
	TRANSPORT_PRINTF( DEBUG_CONNECT, "NetSslEndPoint::MoreSocketSetup(%d)", fd );

	// Let our parent do whatever extra setup it deems appropriate.
	NetTcpEndPoint::MoreSocketSetup( fd, type, e );

	/*
	 * Disable the Nagle algorithm unless it's specifically requested;
	 * it isn't needed with SSL and interacts badly with it.
	 * Note that NetTcpEndPoint::MoreSocketSetup() might have enabled it.
	 */
	SetNagle( fd );
}

/*
 * enable/disable the Nagle algorithm, ie:
 * - 0: set TCP_NODELAY (disable Nagle)
 * - 1: clear TCP_NODELAY (enable Nagle)
 * - 2: for SSL: like 0 => Nagle disabled, but for TCP like 1 => Nagle enabled
 * - default: 2 (for backwards compatibility; set to 0 if internal testing shows no problems)
 *  = TODO: remove this compatibility hack when we're convinced
 *    that we don't need it
 */
void
NetSslEndPoint::SetNagle( int fd, int mode )
{
	if( mode == 2 )
	    mode = 0; // backwards compatibility: Nagle normally disabled for SSL

	TRANSPORT_PRINTF( DEBUG_CONNECT,
	    "NetSslEndPoint::SetNagle(fd=%d, mode=%d)",
	    fd, mode );

	NetUtils::SetNagle( fd, mode );
}

void
NetSslEndPoint::SetNagle( int fd )
{
	int mode = p4tunable.Get( P4TUNE_NET_NAGLE );
	SetNagle( fd, mode );
}

/**
 * NetSslEndPoint::Accept
 *
 * @brief ssl endpoint version of accept, verifies that client request coming in is via ssl
 *
 * @param error structure
 * @return a NetSslTransport
 */
NetTransport *
NetSslEndPoint::Accept( KeepAlive *, Error *e )
{
	NetSslTransport *  sslTransport = NULL;
	TYPE_SOCKLEN       lpeer;
	struct sockaddr_storage
	                   peer;
	int                t;

	TRANSPORT_PRINTF( SSLDEBUG_TRANS, "NetSslEndpoint accept on %d", s );

	lpeer = sizeof peer;

	// Loop accepting, as it gets interrupted (by SIGCHILD) on
	// some platforms (MachTen, but not FreeBSD).

	while( ( t = accept( s, (struct sockaddr *) &peer, &lpeer )) < 0 )
	{
#ifdef OS_NT
	    if( GetLastSockError() != WSAEINTR )
#else
	    if( GetLastSockError() != EINTR )
#endif // OS_NT
	    {
		e->Net( "accept", "socket" );
		goto fail;
	    }
	}

	/*
	 * Set up our accepted socket because we didn't call
	 * CreateSocket(), so we haven't set it up yet.
	 */
	SetupSocket( t, GetSocketFamily(t), AT_LISTEN, e );

	sslTransport = new NetSslTransport( t, true, serverCredentials );

	if(sslTransport)
	{
	    sslTransport->SetPortParser(GetPortParser());
	    /*
	     * Lazy initialization: If no SSL context has been created
	     * for the server side of the connection then do it
	     * now.
	     */
	    StrPtr *hostname = GetListenAddress( RAF_NAME );
	    sslTransport->SslServerInit( hostname, e );
	}
	return sslTransport;

fail:
	{
	    int	errnum = GetLastSockError();
	    StrBuf errBuf;

	    Error::StrError( errBuf, errnum );
#ifdef OS_NT
	    bool isClosedFdErr = (errnum == WSAEBADF);
#else
	    bool isClosedFdErr = (errnum == EBADF);
#endif // OS_NT

	    // isClosedFdErr will be true on restart or shutdown
	    if( !isClosedFdErr )
	    {
		DEBUGPRINTF( SSLDEBUG_ERROR,
		    "NetSslEndpoint::Accept(): In fail error code: error=%d (\"%s\")",
		    errnum, errBuf.Text() );

		StrBuf	errMsg = GetPortParser().String();
		errMsg << " : ";
		errMsg << errBuf;
		e->Set( MsgRpc::SslAccept ) << errMsg.Text();
	    }
	}

	return 0;
}

/**
 * NetSslEndPoint::Connect
 *
 * @brief performs a ssl endpoint connect and returns
 * a NetSslTransport for the new connection
 *
 * @param Error structure
 * @return NetSslTransport for the new connection
 */
NetTransport *
NetSslEndPoint::Connect( Error *e )
{
	int                t;
	NetSslTransport *  sslTransport = NULL;

	// Set up addresses

	/* Configure socket */
	if( ( t = BindOrConnect( AT_CONNECT, e )) < 0 )
	{
	    TRANSPORT_PRINT( SSLDEBUG_ERROR,
		    "NetSslEndpoint::Connect In fail error code." );
	    return 0;
	}

	TRANSPORT_PRINTF( SSLDEBUG_TRANS,
		"NetSslEndpoint setup connect socket on %d", t );

# ifdef SIGPIPE
	signal( SIGPIPE, SIG_IGN );
# endif

	sslTransport = new NetSslTransport( t, false );
	if(sslTransport)
	{
	    sslTransport->SetPortParser(GetPortParser());
	    /*
	     * Lazy initialization: If no SSL context has been created
	     * for the client side of the connection then do it
	     * now.
	     */
	    sslTransport->SslClientInit( e );
	}
	return sslTransport;

}


void
NetSslEndPoint::GetMyFingerprint( StrBuf &value )
{
	if( serverCredentials && serverCredentials->GetFingerprint() &&
		serverCredentials->GetFingerprint()->Length() )
	    value.Set( serverCredentials->GetFingerprint()->Text() );
	else
	    value.Clear();
}

void
NetSslEndPoint::GetExpiration( StrBuf &buf )
{
	if (serverCredentials)
	    serverCredentials->GetExpiration( buf );
	else
	    buf.Clear();
}

# endif //USE_SSL
