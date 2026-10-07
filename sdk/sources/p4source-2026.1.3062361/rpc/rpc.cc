/*
 * Copyright 1995, 1996 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

/*
 * Rpc.cc - remote procedure service
 *
 * The general flow of the client/server protocol:
 *
 * 	The client sends a message containing the user's request to the 
 *	server and then dispatches, i.e. reads and executes requests from
 *	the server until the server sends the "release" message.
 *
 *	The server initially dispatches, i.e. reads and executes a
 *	request from the user.  The final act of the request should
 *	either be to release the client (send the "release" message), 
 *	or send a message to the client instructing it to invoke
 *	(eventually) another server request.  In the end, the final
 *	message sent to the client should be "release".
 */

# define NEED_SLEEP

# include <memory>
# include <stdhdrs.h>

# include <signaler.h>

# include <debug.h>
# include <tunable.h>
# include <strbuf.h>
# include <strops.h>
# include <strdict.h>
# include <strarray.h>
# include <strtable.h>
# include <error.h>
# include <errorlog.h>
# include <tracker.h>
# include <timer.h>
# include <md5.h>
# include <ticket.h>

# include <keepalive.h>
# include <netportparser.h>
# include <netconnect.h>
# include <netbuffer.h>

# include <rpcdebug.h>
# include <msgrpc.h>
// PR_001 Start
#include "../../../../native/sdk_hooks.h"
// PR_001 End
# include <p4tags.h>

# ifdef USE_SSL
extern "C" {	// OpenSSL

# include "openssl/bio.h"
# include "openssl/ssl.h"
# include "openssl/err.h"

}
# endif //USE_SSL
# include "enviro.h"
# include "filesys.h"
# include "pathsys.h"
# include "rpc.h"
# include "rpcbuffer.h"
# include "rpctrans.h"
# include "rpcdispatch.h"
# include "rpcdebug.h"
# include "rpcservice.h"
# include "netsslcredentials.h"

P4INT64 lastSSLCertModTime = 0;
P4INT64 lastSSLKeyModTime = 0;

const char *RpcTypeNames[] = {
	"",		//    RPC_CLIENT = 0,
	"rmt: ",	//    RPC_REMOTE,
	"px: ",		//    RPC_PX,
	"",		//    RPC_RH,
	"bgd: ",	//    RPC_BACKGROUND,
	"pxc: ",	//    RPC_PXCLIENT,
	"rpl: ",	//    RPC_RPL,
	"bs: ",		//    RPC_BROKER,
	"bc: ",		//    RPC_BROKER_TO_SERVER,
	"rauth: ",	//    RPC_RMTAUTH,
	"sbx: ",        //    RPC_SANDBOX,
	"x3: ",         //    RPC_X3,
	"ukn: ",        //    RPC_UNKNOWN
	"dmrpc: ",      //    RPC_DMRPC
	0
};

RpcService::RpcService()
{
	// Initialize our members.

	dispatcher = new RpcDispatcher;
	protoSendBuffer = new RpcSendBuffer;
	endPoint = 0;
	openFlag = RPC_CONNECT;

	// Load up core dispatch routines
	Dispatcher( rpcServices );
}

RpcService::~RpcService()
{
	delete dispatcher;
	delete endPoint;
	delete protoSendBuffer;
}

void
RpcService::NotifyRestarting()
{
    if( endPoint )
	endPoint->NotifyRestarting();
}

/**
 * RpcService::SetEndpoint
 *
 * @brief Creates an communication's endpoint for either listening or connecting.
 *
 * @param  address for socket
 * @param  error structure pointer
 * @see NetEndPoint::Create
 */
void
RpcService::SetEndpoint( const char *addr, Error *e )
{
	delete endPoint;
	endPoint = NetEndPoint::Create( addr, e );
	endPointAddr = addr;
}

/*
 * RpcService::Dispatcher() - add another list of function to call on received RPC
 */

void		
RpcService::Dispatcher( const RpcDispatch *dispatch )
{
	dispatcher->Add( dispatch );
}

void
RpcService::AddAltDispatcher()
{
	dispatcher->AddAltDispatcher();
}

void
RpcService::Listen( Error *e )
{
	// initialize listen

	openFlag = RPC_LISTEN;
	endPoint->Listen( e );

	if( e->Test() )
	{
	    // Failed open -- clear open flag so connect bails, too.

	    e->Set( MsgRpc::Listen ) << endPoint->GetAddress();
	    openFlag = RPC_NOOPEN;
	    return;
	}
}

void
RpcService::GetHost( StrPtr *peerAddress, StrBuf &hostBuf, Error *e )
{
	hostBuf.Clear();
	NetEndPoint *ep = NetEndPoint::Create( peerAddress->Text(), e );
	if( !e->Test() )
	    hostBuf.Set( ep->GetPrintableHost() );
	delete ep;
	return;
}

StrPtr *
RpcService::GetListenAddress( int raf_flags )
{
    return endPoint ? endPoint->GetListenAddress( raf_flags ) : 0;
}

bool
RpcService::MakeAddrListenable( const StrPtr &oldAddr, StrBuf &newAddr, Error *e )
{
	StrBuf host( NetPortParser( oldAddr ).Host() );

	if( host.Length() )
	{
	    Error le;
	    RpcService checkService;

	    checkService.SetEndpoint( oldAddr.Text(), &le );
	    if( !le.Test() ) checkService.ListenCheck( &le );

	    if( le.Test() )
	    {
	        /*
	         * Unable to listen with the (likely foreign) host in
	         * the address; removing the host is likely to make
	         * the address more listenable.
	         */
	        StrOps::Replace( newAddr, oldAddr, host, StrRef( "" ) );
	        return true;
	    }
	}

	return false;	// nothing changed to make address any more listenable
}

void
RpcService::ListenCheck( Error *e )
{
	endPoint->ListenCheck( e );
}

int
RpcService::CheaterCheck( const char *port )
{
	return endPoint->CheaterCheck( port );
}

void
RpcService::Unlisten()
{
	if( endPoint )
	    endPoint->Unlisten();
}

void
RpcService::GetExpiration( StrBuf &buf )
{
	if( endPoint )
	    endPoint->GetExpiration( buf );
}

void
RpcService::SetProtocol( const char *var, const StrRef &value )
{
	protoSendBuffer->SetVar( StrRef( (char*)var ), value );
}

void
RpcService::SetProtocolV( const char *arg )
{
	StrBuf s;
	const char *p;

	if( ( p = strchr( arg, '=' ) ) )
	{
	    s.Set( arg, p - arg );
	    protoSendBuffer->SetVar( s, StrRef( (char *)p + 1 ) );
	}
	else
	{
	    protoSendBuffer->SetVar( StrRef( (char *)arg ), StrRef::Null() );
	}
}
const StrBuf
RpcService::GetMyQualifiedP4Port( StrBuf &serverSpecAddr, Error &e ) const
{
	StrBuf result;
	if( endPoint )
	{
	    result = endPoint->GetPortParser().GetQualifiedP4Port( serverSpecAddr, e);
	}
	else
	{
	    e.Set( MsgRpc::BadP4Port ) << "no endpoint";
	}
	return result;
}

void
RpcService::GetMyFingerprint( StrBuf &value )
{
	endPoint->GetMyFingerprint( value );
}

int
RpcService::IsSingle()
{
	return endPoint ? endPoint->IsSingle() : 0;
}

int
RpcService::IsSSL()
{
	return endPoint ? endPoint->IsSSL() : 0;
}

static void RpcCleanup( Rpc *r )
{
	r->FlushTransport();
	r->ShutdownTransport();
}
/*
 * Rpc
 */

Rpc::Rpc( RpcService *s )
{
	recvBuffering = 0;

	service = s;
	transport = 0;
	forward = 0;

	sendBuffer = new RpcSendBuffer;
	recvBuffer = new RpcRecvBuffer;
	protoDynamic = new StrBufDict;

	duplexFsend = 0;
	duplexFrecv = 0;
	duplexRsend = 0;
	duplexRrecv = 0;

	dispatchDepth = 0;
	endDispatch = 0;
	suspendDispatch = 0;
	priorityDispatch = 0;

	protocolSent = 0;
	protocolServer = 0;

	rpc_hi_mark_rev = 
	rpc_hi_mark_fwd = p4tunable.Get( P4TUNE_RPC_HIMARK );
	rpc_lo_mark = p4tunable.Get( P4TUNE_RPC_LOWMARK );

	TrackStart();

	timer = new Timer;

	keep = 0;

	sendDirectTotal = 0;
	recvDirectTotal = 0;
	sendDirectBytes = 0;
	recvDirectBytes = 0;
}

Rpc::~Rpc( void )
{
	signaler.DeleteOnIntr( this );
	Disconnect();

	delete sendBuffer;
	delete recvBuffer;
	delete protoDynamic;
	delete timer;
}

void
Rpc::DoHandshake( Error *e )
{
	if( transport )
	    transport->DoHandshake(e);
}

void 
Rpc::ClientMismatch( Error *e )
{
	if( transport )
	    transport->ClientMismatch( e );
}

void 
Rpc::SetMaxWait( const int maxWait )
{
	if( transport )
	    transport->SetMaxWait( maxWait );
}

void
Rpc::SetProtocolDynamic( const char *var, const StrRef &value )
{
	protoDynamic->ReplaceVar( var, value.Text() );
}

void
Rpc::ClearProtocolDynamic( const char *var )
{
	protoDynamic->RemoveVar( var );
}

void
Rpc::Connect( Error *e )
{
	// Don't allow double connects.

	if( transport )
	{
	    e->Set( MsgRpc::Reconn );
	    return;
	}

	// If an operation fails, cruft can be left in the sendBuffer.
	// Since we may use this buffer over for the next connection, we
	// need to clear it now.

	sendBuffer->Clear();

	duplexFsend = 0;
	duplexFrecv = 0;
	duplexRsend = 0;
	duplexRrecv = 0;
	dispatchDepth = 0;
	endDispatch = 0;
	protocolSent = 0;
	re.Clear();
	se.Clear();

	// Create the transport layer, 
	// Cleanup code is in Disconnect().

	NetTransport *t = 0;

	switch( service->openFlag )
	{
	case RPC_LISTEN:   
	    t = service->endPoint->Accept( keep, e );
	    break;

	case RPC_CONNECT:  
	    t = service->endPoint->Connect( e );
	    break;

	default:    	    
	    e->Set( MsgRpc::Unconn );
	}

	if( e->Test() )
	{
	    if( t )
		delete t;

	    // connect errors leave send & receive errors
	    // so as to intercept Invoke() and Dispatch()
	    re = *e;
	    se = *e;
	    return;
	}

	// Pass transport onto the buffering routines.

	transport = new RpcTransport( t );

	if( keep )
	    transport->SetBreak( keep );

	// If rpc.himark tuned beyond net.bufsize, must tell NetBuffer.

	transport->SetBufferSizes( rpc_hi_mark_fwd, rpc_hi_mark_rev );

	if( service->openFlag == RPC_CONNECT )
	    signaler.OnIntr( (SignalFunc)RpcCleanup, this );
}

void
Rpc::GetEncryptionType( StrBuf &value )
{ 
	if( transport )
	    transport->GetEncryptionType( value );
}


void
Rpc::GetPeerFingerprint( StrBuf &value )
{
	if( transport )
	    transport->GetPeerFingerprint( value );
}

NetSslCredentials *
Rpc::GetPeerCredentials()
{
	return transport ? transport->GetPeerCredentials() : 0;
}

void
Rpc::GetExpiration(StrBuf &value)
{
    if( service )
	service->GetExpiration( value );
}

/*
 * Rpc::SetHiMark() - move rpc_hi_mark beyond 2000 default
 *
 *	If we get reliable send/receive TCP buffer sizes from the client
 *	(they often differ from each other and from the server's), we
 *	can feel confident about moving our himark off the default 2000.
 *
 *	The thinking as of Feb 2009 is that one direction of a TCP
 *	connection can have data outstanding that amounts to the receiver's
 *	SO_RCVBUF.  Many systems can take more -- we tried the sum of
 *	the sender's SO_SNDBUF and the receiver's SO_RCVBUF -- but some
 *	systems can't.  Solaris, for example, starts to close its window
 *	when it receives more than SO_RCVBUF, keeps accepting data until
 *	the window is 0, and won't reopen it until the application reads
 *	all outstanding data.  Weird and unfortunate, because we generally
 *	read in small chunks.
 *
 *	Sometimes we're metering client->server data, because the
 *	server->client side is clogged with file data (sync uses
 *	InvokeDuplex()).  Other times we're metering server->client data,
 *	because the client->server side is clogged (resolve, submit use
 *	InvokeDuplexRev()).  Since the server and client may have different
 *	SO_RCVBUF sizes, and we want to make the most of the connection,
 *	we keep separate rpc_hi_mark_fwd and rpc_hi_mark_rev limits.  We
 *	miss out when we meter data but neither side is clogged (diff,
 *	open, revert).  Our high mark could be somewhere around the sum
 *	of the SO_RCVBUFs for the client and server in this case, since
 *	the metered data could wait in either buffer.   But we're not
 *	yet that sophisticated.
 *
 *	Note that TCP limits its window size to the minimum of the
 *	sender's SO_SNDBUF and receiver's SO_RCVBUF, and that limits the
 *	amount of data travelling on the network in that direction.  If
 *	our himark were lower than the window size, we would artificially
 *	limit that amount of data further.  Thus we'd like the himark
 *	high.  Fortunately, we seem safe in making it SO_RCVBUF.
 *
 *	A SO_SNDBUF larger than the SO_RCVBUF (and thus the TCP window)
 *	should have no impact.  A SO_RCVBUF larger than the SO_SNDBUF
 *	(and thus a himark larger than the TCP window) is beneficial in
 *	the case where we're metering data on one side but the other
 *	side is not clogged.  It allows more data to be in transit on
 *	the other side of the connection.
 */

void
Rpc::SetHiMark( int sndbuf, int rcvbuf )
{
	if( !transport )
	    return;

	// For testing and emergencies setting rpc.himark overrides all.
	if( p4tunable.IsSet( P4TUNE_RPC_HIMARK ) )
	    return;

	const int minHiMark = p4tunable.Get( P4TUNE_RPC_HIMARK );

	rpc_hi_mark_fwd = transport->GetRecvBuffering();
	rpc_hi_mark_rev = rcvbuf;

	// Since Invoke() needs to Dispatch() _before_ we hit
	// the himark, we reduce the himark by lomark, and hope that
	// no single duplex message is larger than lomark.

	rpc_hi_mark_fwd -= rpc_lo_mark;
	rpc_hi_mark_rev -= rpc_lo_mark;

	if( rpc_hi_mark_fwd < minHiMark ) rpc_hi_mark_fwd = minHiMark;
	if( rpc_hi_mark_rev < minHiMark ) rpc_hi_mark_rev = minHiMark;

	// Put a stake through the heart of himark hanging: set our
	// NetBuffer buffer sizes to handling any outstanding data,
	// in case the lying cheating OS (linux, solaris) renegs
	// on its reported TCP send or receive space.

	transport->SetBufferSizes( rpc_hi_mark_fwd, rpc_hi_mark_rev );

	RPC_DBG_PRINTF( DEBUG_CONNECT,
		"Rpc himark: snd+rcv server %d+%d client %d+%d = %d/%d",
		    transport->GetSendBuffering(), 
		    transport->GetRecvBuffering(), 
		    sndbuf, rcvbuf, 
		    rpc_hi_mark_fwd,
		    rpc_hi_mark_rev );
}

void
Rpc::Disconnect()
{
	if( !transport )
	    return;

	transport->Flush( &se );
	transport->Close();

	delete transport;
	transport = 0;
}

StrPtr *
Rpc::GetAddress( int raf_flags )
{
	return transport ? transport->GetAddress( raf_flags ) : 0;
}

bool
Rpc::HasAddress()
{
	return transport ? transport->HasAddress() : false;
}

StrPtr *
Rpc::GetPeerAddress( int raf_flags )
{
	return transport ? transport->GetPeerAddress( raf_flags ) : 0;
}

int
Rpc::GetPortNum()
{
	return transport ? transport->GetPortNum() : -1;
}

bool
Rpc::IsSockIPv6()
{
	return transport ? transport->IsSockIPv6() : false;
}

StrBuf *
Rpc::MakeVar( const char *var )
{
	return sendBuffer->MakeVar( StrRef( (char *)var ) );
}

void		
Rpc::VSetVar( const StrPtr &var, const StrPtr &value )
{
	sendBuffer->SetVar( var, value );
}

void
Rpc::Release()
{
	this->Invoke( P4Tag::p_release );
}

void
Rpc::ReleaseFinal()
{
	this->Invoke( P4Tag::p_release2 );
}

StrPtr *
Rpc::VGetVar( const StrPtr &var )
{
	return recvBuffer->GetVar( var );
}

int
Rpc::VGetVarX( int x, StrRef &var, StrRef &val )
{
	return recvBuffer->GetVar( x, var, val );
}

void
Rpc::VRemoveVar( const StrPtr &var )
{
	recvBuffer->RemoveVar( var );
}

void
Rpc::VClear()
{
	sendBuffer->Clear();
}

int
Rpc::VGetCount()
{
	return recvBuffer->GetCount();
}

void
Rpc::VReset()
{
	sendBuffer->Reset();
}

StrPtr *
Rpc::GetArgi( int i )
{
	return i < recvBuffer->GetArgc() ? &recvBuffer->GetArgv()[i] : 0;
}

StrPtr *
Rpc::GetArgi( int i, Error *e )
{
	StrPtr *s = GetArgi( i );

	if( !s )
	    e->Set( MsgRpc::NoPoss );

	return s;
}

int
Rpc::GetArgc()
{
	return recvBuffer->GetArgc();
}

StrPtr *
Rpc::GetArgv()
{
	return recvBuffer->GetArgv();
}

void
Rpc::CopyVars()
{
	sendBuffer->CopyVars( recvBuffer );
}

KeepAlive *
Rpc::GetKeepAlive()
{
	return transport;
}

void
Rpc::SetBreak( KeepAlive *breakCallback )
{
	keep = breakCallback;

	if( transport )
	    transport->SetBreak( breakCallback );
}

KeepAlive *
Rpc::GetBreak()
{
    return keep;
}

/*
 * Invoke()
 * InvokeDuplex()
 * InvokeDuplexRev()
 * FlushDuplex()
 * InvokeFlush()
 * InvokeOne()
 * GotFlushed()
 */

void
Rpc::Invoke( const char *opName )
{
	// If there are any outstanding requests sent via InvokeDuplexRev,
	// we have to assume the return pipe is clogged, and thus meter all
	// data sent.

	if( duplexRrecv )
	    InvokeDuplex( opName );
	else
	    InvokeOne( opName );
}

void
Rpc::InvokeDuplex( const char *opName )
{
	// This data makes a loop: meter it, so we know to dispatch for it.

	int sz = InvokeOne( opName );
	duplexFrecv += sz;
	duplexFsend += sz;
	Dispatch( DfDuplex, service->dispatcher );
}

void
Rpc::InvokeDuplexPlus( const char *opName, int extra )
{
	// This data makes a loop: meter it, so we know to dispatch for it.
	// We know the return will have some extra content, supply that 
	// so we can add it in to better estimate the return

	int sz = InvokeOne( opName ) + extra;
	duplexFrecv += sz;
	duplexFsend += sz;
	Dispatch( DfDuplex, service->dispatcher );
}

void
Rpc::InvokeDuplexRev( const char *opName )
{
	// This data only goes forward, but causes the return pipe to fill.
	// Meter it, but also turn on duplexRrecv so that all Invokes are
	// metered until this message is flushed..

	++duplexRrecv;
	++duplexRsend;
	InvokeDuplex( opName );
}

void
Rpc::FlushDuplex()
{
	// Flush any outstanding duplex data in the pipe.
	// Do nothing if there is no duplex data expected back

	if( duplexFrecv > 0 )
	{
	    // Proxies and brokers expect a flush1 message
	    // (with himark = 0) when flushing the channel,
	    // so we have to tell Dispatch() to send one.
	    // DfFlush moves the lomark to 0, and ensuring
	    // duplexFsend > 0 forces a flush1 message.

	    ++duplexFrecv;
	    ++duplexFsend;

	    Dispatch( DfFlush, service->dispatcher );
	}
}

void
Rpc::InvokeOver( const char *opName )
{
	// This data makes a loop, but we specifically don't flush yet:
	// we return so that the outer Dispatch() can take over.
	// Otherwise, we'd needlessly nest Dispatch() calls, and in
	// one case ('submit') we'd try to nest too deeply.

	int sz = InvokeOne( opName );
	duplexFrecv += sz;
	duplexFsend += sz;
	Dispatch( DfOver, service->dispatcher );
}

int		
Rpc::InvokeOne( const char *opName )
{
	// Don't pile errors

	if( se.Test() || re.Test() || !transport )
	{
	    sendBuffer->Clear();
	    return 0;
	}

	int delay = p4tunable.Get( P4TUNE_RPC_DELAY );

	// Send off protocol if not yet sent.
	// If we're using RpcForward we want to allow it
	// to send its own protocol message.

	if( !protocolSent && strcmp( opName, P4Tag::p_protocol ) )
	{
	    RpcSendBuffer buf;
	    int sz = transport->GetSendBuffering();
	    int rz = transport->GetRecvBuffering();

	    buf.CopyBuffer( service->protoSendBuffer );
	    
	    int i = 0;
	    StrRef var, val;
	    while( protoDynamic->GetVar( i++, var, val ) )
	        buf.SetVar( var, val );

	    buf.SetVar( StrRef( P4Tag::v_sndbuf ), StrNum( sz ) );
	    buf.SetVar( StrRef( P4Tag::v_rcvbuf ), StrNum( rz ) );
	    if( p4tunable.Get( P4TUNE_NET_AUTOTUNE ) )
		buf.SetVar( P4Tag::v_autoTune, StrRef( "1" ) );
	    buf.SetVar( StrRef( P4Tag::v_func ), StrRef( P4Tag::p_protocol ) );

	    RPC_DBG_PRINT( DEBUG_FUNCTION,
		    "Rpc invoking protocol" );

	    timer->Start();

	    if( delay )
	    {
	        p4debug.Event();
	        p4debug.printf(
	            "Delaying RPC send by %dms configured with 'rpc.delay'\n",
	            delay );
	        msleep( delay );
	    }

	    transport->Send( buf.GetBuffer(), &re, &se );

	    sendTime += timer->Time();
	}

	protocolSent = 1;

	// Set func=opName variable.

	SetVar( P4Tag::v_func, opName );

	// Tracking

	RPC_DBG_PRINTF( DEBUG_FUNCTION,
		"Rpc invoking %s", opName );

	// Send the buffer to peer

	timer->Start();
	
	if( delay )
	{
	    p4debug.Event();
	    p4debug.printf(
	        "Delaying RPC send by %dms configured with 'rpc.delay'\n",
	        delay );
	    msleep( delay );
	}

	transport->Send( sendBuffer->GetBuffer(), &re, &se );

	if( se.Test() && se.CheckId( MsgRpc::TooBig ) )
	{
	    AssertLog.Report( &se );

	    sendBuffer->Clear();
	    StrBufDict errorDict;
	    se.Marshall1( errorDict );
	    se.Clear();

	    int j = 0;
	    StrRef var, val;
	    while( errorDict.GetVar( j++, var, val ) )
	        sendBuffer->SetVar( var, val );

	    sendBuffer->SetVar( P4Tag::v_func, StrRef( "client-Message" ) );
	    transport->Send( sendBuffer->GetBuffer(), &re, &se );
	}

	// time tracking
	sendTime += timer->Time();

	if( se.Test() )
	    return 0;

	// Clear for next call.
	// Get size of buffer so we know how full the pipe is.
	// We must include RpcTransport's overhead.

	int sz = sendBuffer->GetBufferSize() + transport->SendOverhead(); 

	sendBuffer->Clear();

	// tracking

	sendCount++;
	sendBytes += sz;

	return sz;
}

void
Rpc::GotFlushed()
{
	StrPtr *fseq = GetVar( P4Tag::v_fseq );
	StrPtr *rseq = GetVar( P4Tag::v_rseq );

	if( fseq )
	    duplexFrecv -= fseq->Atoi();

	if( rseq )
	    duplexRrecv -= rseq->Atoi();
}

/*
 * Rpc::Dispatch() - dispatch incoming RPC's until 'release' received
 * Rpc::DispatchOne() - just dispatch from the current buffer
 */

void		
Rpc::Dispatch( DispatchFlag flag, RpcDispatcher *dispatcher )
{
	// Don't nest more than once: we only allow a
	// Dispatch()/InvokeDuplex()/FlushDuplex() combo.

	if( dispatchDepth > 1 )
	    return;
	
	// If we're containing this dispatch, don't increment the depth.
	// We only do this when we know that the original dispatcher is
	// effectivly paused, waiting for this to complete.

	if( flag != DfContain )
	    ++dispatchDepth;

	RPC_DBG_PRINTF( DEBUG_FLOW,
	        ">>> Dispatch(%d%s) %d/%d %d/%d %d",
	        dispatchDepth, flag == DfContain ? "+" : "",
	        duplexFsend, duplexFrecv, 
	        duplexRsend, duplexRrecv, flag );

	// Use server's recv buffer size as himark for InvokeDuplex()
	// Use client's recv buffer size as himark for InvokeDuplexRev()

	int loMark = rpc_lo_mark;
	int hiMark = duplexRrecv ? rpc_hi_mark_rev : rpc_hi_mark_fwd;

	// Flushing means eveything goes.
	// Complete: lo = N/A, hi = N/A
	// Duplex: lo = LO, hi = HI
	// Flush: lo = 0, hi = 0
	// Over: lo = 0, hi = HI

	if( flag != DfDuplex ) loMark = 0;
	if( flag == DfFlush ) hiMark = 0;

	// Push the recvBuffer, in case we are nesting Dispatch().

# ifdef HAS_CPP11
	// Make sure this gets cleaned up during an exception.
	std::unique_ptr< RpcRecvBuffer > savRecvBuffer =
	    std::unique_ptr< RpcRecvBuffer >( recvBuffer );
	recvBuffer = 0;
# else
	RpcRecvBuffer *savRecvBuffer = recvBuffer;
	recvBuffer = 0;
# endif

	// Receive (dispatching) until told to stop.

	while( !endDispatch )
	{
	    if( re.Test() && ( !transport || !transport->RecvReady() ) )
	        break;

	    // If more than loMark in pipe, send a marker.
	    // We flush always if Dispatch() called from FlushDuplex()
	    // or if Invoking from such a Dispatch().

	    if( duplexFsend > loMark && !se.Test() )
	    {
		// Send a flush1 through for metering flow control.

		// Note: hardcode the size of the flush1 message
		// As of 2008.1 its 46 bytes (round up to 60).

	        const int flushMessageSize = 60;

	        RPC_DBG_PRINTF( DEBUG_FLOW,
	        	"Rpc flush %d bytes", duplexFsend );

		SetVar( P4Tag::v_himark, loMark ? hiMark : 0 );
		if( flag == DfFlush )
		    SetVar( P4Tag::v_flushHard );

		duplexFrecv += flushMessageSize;
		duplexFsend += flushMessageSize;

		if( duplexFsend ) SetVar( P4Tag::v_fseq, duplexFsend );
		if( duplexRsend ) SetVar( P4Tag::v_rseq, duplexRsend );

		duplexFsend = 0;
		duplexRsend = 0;

		InvokeOne( P4Tag::p_flush1 );
	    }

	    // If top level Dispatch(), go forever.
	    // If 2nd level Dispatch(), get below hiMark.
	    // If flushing, go until all flush2's received.
	    // If error sending, go until receive error.

	    else if( flag == DfComplete ||
		     ( flag == DfDuplex && DuplexDispatchReady( hiMark ) ) ||
		     ( flag == DfFlush && duplexFrecv ) ||
		     ( flag == DfContain && !le.Test() ) ||
		     se.Test() )
	    {
		if( !recvBuffer )
		    recvBuffer = new RpcRecvBuffer;

		// PR_001 Start
		p4rust::Pump(this, false);
		while (p4rust::Pending(this) && transport && !transport->RecvReady())
		    p4rust::Pump(this, true);
		// PR_001 End
		DispatchOne( dispatcher, flag == DfContain );

	        // pack and potentially resize buffer
	        if( transport && p4tunable.Get( P4TUNE_NET_AUTOTUNE ) )
	            transport->ResizeBuffer();
	    }
	    else break;
	}

	// Pop recvBuffer.

	delete recvBuffer;

# ifdef HAS_CPP11
	recvBuffer = savRecvBuffer.release();
# else
	recvBuffer = savRecvBuffer;
# endif

	RPC_DBG_PRINTF( DEBUG_FLOW,
	        "<<< Dispatch(%d%s) %d/%d %d/%d %d",
	        dispatchDepth, flag == DfContain ? "+" : "",
	        duplexFsend, duplexFrecv,
	        duplexRsend, duplexRrecv, flag );

	if( flag == DfContain || !--dispatchDepth )
	    endDispatch = 0;
}


void
Rpc::DispatchAndRunCallback( const StrPtr& func, bool passError /*= false */ )
{
	DispatchAndRunCallback( service->dispatcher, func, passError );
}


void
Rpc::DispatchAndRunCallback( const RpcDispatcher* dispatcher, 
	                     const StrPtr& func, 
	                     bool passError )
{
	// Find the registered function as given with 'func'.
	// If no such function is found, call 'funcHandler'.
	// If any error occurs, call 'errorHandler'.
	// If no 'errorHandler' just report the error and return.
	const RpcDispatch* disp = dispatcher->Find( func.Text() );

	if( !disp && !( disp = dispatcher->Find( P4Tag::p_funcHandler ) ) )
	{
	    ue.Set( MsgRpc::UnReg ) << func;
	    goto error;
	}

	// Invoke requested function.

	RunCallback( disp, ue );

	// Take a copy of the errors

	le = ue;

	// If we have just finished processing an ack, now is a good
	// time to process any masked Control-C's.
	//
	// We don't need to actually check if we are in a sync here.  If
	// we are not, the non-masked Control-C will terminate the client
	// process. If we are in a sync, the Control-C will be masked and
	// will not be processed until this condition is checked.
	//

	if( signaler.IsMaskedIntr() )
	{
	    if( strcmp( func.Text(), P4Tag::c_Ack ) == 0 )
	    {
	        endDispatch = 1;
	        return;
	    }
	}

	// If an error occurred, we'll call the caller's errorHandle
	// function.  If the error was fatal, we'll tack on some tracing
	// information.

	if( !ue.Test() )
	    return;

	if( ue.IsFatal() )
	    ue.Set( MsgRpc::Operat ) << disp->opName;

    error:
	// If a user error occurred, invoke errorHandler to deal with it.
	// In this case, the Error is actually passed in to the dispatched 
	// function, rather than being just an output parameter.

	// The exception is if we are running as a nested Dispatch that will
	// pass this error to its parent (then this will be done by the parent)

	if( passError )
	    return;

	if( ( disp = dispatcher->Find( P4Tag::p_errorHandler ) ) )
	{
	    RunCallback( disp, ue );
	    return;
	}

	AssertLog.Report( &ue );

}

void
Rpc::TransportSetupSocket()
{
	if( transport )
	    transport->SetupSocket();
}

NO_SANITIZE_UNDEFINED
void
Rpc::RunCallback( const RpcDispatch *disp, Error &ue )
{
	(*disp->function)( this, &ue );
}

void
Rpc::DispatchOne( RpcDispatcher *dispatcher, bool passError )
{
	StrPtr *func;

	// Flush any data due the other end: we can't expect
	// anything if the other end is awaiting us!

	// Note that a broken send pipe doesn't stop dispatching:
	// we want what's in the receive pipe (they may be important
	// acks), so we read until the receive pipe is broken too.

	// Receive sender's buffer and then parse the variables out.
	
	timer->Start();

	int delay = p4tunable.Get( P4TUNE_RPC_DELAY );
	if( delay )
	{
	    p4debug.Event();
	    p4debug.printf(
	        "Delaying RPC receive by %dms configured with 'rpc.delay'\n",
	        delay );
	    msleep( delay );
	}

	int sz = transport->Receive( recvBuffer->GetBuffer(), &re, &se );

	// time tracking
	recvTime += timer->Time();

	if( sz <= 0 )
	{
	    // EOF doesn't set re, so we will.

	    if( !re.Test() )
		re.Set( MsgRpc::Closed );

	    return;
	}

	// tracking

	recvCount++;
	recvBytes += recvBuffer->GetBufferSize();

	Error e;
	recvBuffer->Parse( &e );

	if( e.Test() )
	{
	    re = e;
	    return;
	}

	// Find the function to dispatch.  The protocol mandates that
	// this must be set: how else do we know what to do with the 
	// buffer?

	func = GetVar( P4Tag::v_func, &e );

	if( e.Test() )
	{
	    re = e;
	    return;
	}

	RPC_DBG_PRINTF( DEBUG_FUNCTION,
		"Rpc dispatch %s", func->Text() );


	ue.Clear();

	DispatchAndRunCallback( dispatcher, *func, passError );
}

int
Rpc::DuplexDispatchReady( int hiMark )
{
	return ( duplexFrecv > hiMark &&
			( priorityDispatch || transport->DuplexReady() ) );
}

void
Rpc::Loopback( Error *e )
{
	recvBuffer->CopyBuffer( sendBuffer->GetBuffer() );
	recvBuffer->Parse( e );
	sendBuffer->Clear();
}

int
Rpc::Active()
{
	return !endDispatch &&
	    ( !ReadErrors() || ( transport && transport->RecvReady() ) );
}

int
Rpc::DispatchReady()
{
	return !endDispatch && transport && transport->DuplexReady();
}

int
Rpc::SuspendDispatch( int v )
{
	int r = suspendDispatch;
	suspendDispatch = v;
	return r;
}

int
Rpc::PriorityDispatch( int v )
{
	int r = priorityDispatch;
	priorityDispatch = v;
	return r;
}

/*
 * Rpc::StartCompression() -- initiate full link compression
 * Rpc::GotRecvCompress() -- turn on recv half compression
 * Rpc::GotSendCompress() -- turn on send half compression
 *
 * Notes on compression.
 *
 * Both ends must support the compression protocol.  Use "client" >= 6
 * or "server2" >= "6" ? protocol level to be sure.
 *
 * Compression can be turned on any time; in practice, it happens
 * after protocol exchange has happened (otherwise, how do you know?).
 * To orchestrate this, the follow sequence is obeyed:
 *
 * 1.  StartCompression() send "compress1" to the other end, and then turns 
 *     on send compression.   All data sent afterwards are compressed.  
 *
 * 2.  When the other end receives "compress1", it sends back "compress2"
 *     and then turns on both send and receieve compression.
 *
 * 3.  When this end receives "compress2", it turns on receive compression.
 */

void
Rpc::StartCompression( Error *e )
{
	// send the "compress1" flag, then compress the send link.
	// When we get "compress2", we'll compress the recv link.

	Invoke( P4Tag::p_compress1 );
	transport->SendCompression( e );
}

void
Rpc::GotSendCompressed(  Error *e )
{
	transport->SendCompression( e );
}

void
Rpc::GotRecvCompressed(  Error *e )
{
	transport->RecvCompression( e );
}

void
Rpc::FlushTransport()
{
	if( transport )
	    transport->Flush( &se );
}

void
Rpc::ShutdownTransport()
{
	if( transport )
	    transport->Shutdown( &se );
}

int	
Rpc::GetRecvBuffering() 
{
	if( transport )
	    return transport->GetRecvBuffering();
	return 0;
}


/*
 * Performance tracking
 *
 * Rpc::TrackStart() - reset tracking counters
 * Rpc::Trackable() - is any track data interesting?
 * Rpc::TrackReport() - report interesting track data
 */

void
Rpc::TrackStart()
{
	sendCount = 0;
	sendBytes = 0;
	recvCount = 0;
	recvBytes = 0;
	sendTime = 0;
	recvTime = 0;

	sendDirectTotal = 0;
	recvDirectTotal = 0;
	sendDirectBytes = 0;
	recvDirectBytes = 0;
}

void
Rpc::TrackStart( RpcTrack *track )
{
	track->trackable = 0;
	track->sendCount = 0;
	track->sendBytes = 0;
	track->recvCount = 0;
	track->recvBytes = 0;
	track->rpc_hi_mark_fwd = 0;
	track->rpc_hi_mark_rev = 0;
	track->sendTime = 0;
	track->recvTime = 0;
	track->sendError.Clear();
	track->recvError.Clear();
	track->duplexFrecv = 0;
	track->duplexRrecv = 0;

	track->sendDirectTotal = 0;
	track->recvDirectTotal = 0;
	track->sendDirectBytes = 0;
	track->recvDirectBytes = 0;
}

int
Rpc::Trackable( int level )
{
	Tracker t( level );

	return t.Over( TT_RPC_ERRORS, se.Test() || re.Test() ) ||
	       t.Over( TT_RPC_MSGS, sendCount + recvCount ) ||
	       t.Over( TT_RPC_MBYTES, ( sendBytes + recvBytes ) / 1024 / 1024 ) ||
	       t.Over( TT_RPC_MBYTES, ( sendDirectBytes + recvDirectBytes )
		       / 1024 / 1024 );
}

int
Rpc::Trackable( int level, RpcTrack *track )
{
	if( !track )
	    return 0;

	Tracker t( level );

	return t.Over( TT_RPC_ERRORS, track->sendError.Test() ||
	                              track->recvError.Test() ) ||
	       t.Over( TT_RPC_MSGS, track->sendCount + track->recvCount ) ||
	       t.Over( TT_RPC_MBYTES, ( track->sendBytes + track->recvBytes )
	                                / 1024 / 1024 ) ||
	       t.Over( TT_RPC_MBYTES, ( track->sendDirectBytes +
	           track->recvDirectBytes) / 1024 / 1024 );
}

void
Rpc::TrackReport( int level, StrBuf &out )
{
	if( !Trackable( level ) )
	    return;

	out 
	    << "--- rpc msgs/size in+out "
	    << StrNum( recvCount ) << "+"
	    << StrNum( sendCount ) << "/"
	    << recvBytes / 1024 / 1024 << "mb+"
	    << sendBytes / 1024 / 1024 << "mb "
	    << "himarks " 
	    << rpc_hi_mark_fwd << "/" 
	    << rpc_hi_mark_rev << " snd/rcv "
	    << StrMs( sendTime ) << "s/"
	    << StrMs( recvTime ) << "s\n";

	out
	    << "--- filetotals (svr) send/recv files+bytes "
	    << sendDirectTotal << "+"
	    << StrNum( sendDirectBytes / 1024 / 1024) << "mb/"
	    << recvDirectTotal << "+"
	    << StrNum( recvDirectBytes / 1024 / 1024 ) << "mb\n";
	

	if( !se.Test() && !re.Test() )
	    return;

	out << "--- rpc ";

	if( se.Test() ) out << "send ";
	if( re.Test() ) out << "receive ";

	out << "errors, duplexing F/R " 
	    << duplexFrecv << "/"
	    << duplexRrecv << "\n";
}

void
Rpc::TrackReport( int level, const char *tag, RpcTrack *track, StrBuf &out )
{
	if( !track )
	    return;
	
	if( !Trackable( level, track ) )
	    return;

	out 
	    << "--- rpc (" << tag << ") msgs/size in+out "
	    << StrNum( track->recvCount ) << "+"
	    << StrNum( track->sendCount ) << "/"
	    << track->recvBytes / 1024 / 1024 << "mb+"
	    << track->sendBytes / 1024 / 1024 << "mb "
	    << "himarks " 
	    << track->rpc_hi_mark_fwd << "/" 
	    << track->rpc_hi_mark_rev << " snd/rcv "
	    << StrMs( track->sendTime ) << "s/"
	    << StrMs( track->recvTime ) << "s\n";

	out
	    << "--- filetotals (svr) send/recv files+bytes "
	    << track->sendDirectTotal << "+"
	    << StrNum( track->sendDirectBytes / 1024 / 1024 ) << "mb/"
	    << track->recvDirectTotal << "+"
	    << StrNum( track->recvDirectBytes / 1024 / 1024 ) << "mb\n";

	if( !track->sendError.Test() && !track->recvError.Test() )
	    return;

	out << "--- rpc ";

	if( track->sendError.Test() ) out << "send ";
	if( track->recvError.Test() ) out << "receive ";

	out << "errors, duplexing F/R " 
	    << track->duplexFrecv << "/"
	    << track->duplexRrecv << "\n";
}

void
Rpc::GetTrack( int level, RpcTrack *track )
{
	track->trackable = Trackable( level );
	if( !track->trackable )
	    return;
	ForceGetTrack( track );
}

void
Rpc::ForceGetTrack( RpcTrack *track )
{
	if( !track )
	    return;
	track->recvCount = recvCount;
	track->sendCount = sendCount;
	track->recvBytes = recvBytes;
	track->sendBytes = sendBytes;
	track->rpc_hi_mark_fwd = rpc_hi_mark_fwd;
	track->rpc_hi_mark_rev = rpc_hi_mark_rev;
	track->recvTime = recvTime;
	track->sendTime = sendTime;
	if( se.Test() ) track->sendError = se;
	if( re.Test() ) track->recvError = re;
	if( se.Test() || re.Test() )
	{
	    track->duplexFrecv = duplexFrecv;
	    track->duplexRrecv = duplexRrecv;
	}
	else
	    track->duplexFrecv = track->duplexRrecv = 0;

	track->sendDirectTotal = sendDirectTotal;
	track->recvDirectTotal = recvDirectTotal;
	track->sendDirectBytes = sendDirectBytes;
	track->recvDirectBytes = recvDirectBytes;
}

void
Rpc::AddTrack( RpcTrack *track )
{
	if( !track )
	    return;

	track->recvCount += recvCount;
	track->sendCount += sendCount;
	track->recvBytes += recvBytes;
	track->sendBytes += sendBytes;
	if( rpc_hi_mark_fwd > track->rpc_hi_mark_fwd )
	    track->rpc_hi_mark_fwd += rpc_hi_mark_fwd;
	if( rpc_hi_mark_rev > track->rpc_hi_mark_rev )
	    track->rpc_hi_mark_rev += rpc_hi_mark_rev;
	track->recvTime += recvTime;
	track->sendTime += sendTime;
	if( se.Test() ) track->sendError.Merge( se );
	if( re.Test() ) track->recvError.Merge( re );
	if( se.Test() || re.Test() )
	{
	    track->duplexFrecv += duplexFrecv;
	    track->duplexRrecv += duplexRrecv;
	}

	track->sendDirectTotal += sendDirectTotal;
	track->recvDirectTotal += recvDirectTotal;
	track->sendDirectBytes += sendDirectBytes;
	track->recvDirectBytes += recvDirectBytes;
}

void
Rpc::CheckKnownHost( Error *e, const StrRef & trustfile )
{
	StrBuf	pubkey;
	GetPeerFingerprint( pubkey );
	
	// if not ssl we are done
	if( !pubkey.Length() )
	    return;

	NetSslCredentials *creds = GetPeerCredentials();
	int sslLevel = p4tunable.Get( P4TUNE_SSL_CLIENT_CERT_VALIDATE );

	StrPtr *peer = GetPeerAddress( RAF_PORT );
	StrBuf peerIpPort = *peer;

	RPC_DBG_PRINTF( DEBUG_CONNECT,
		"Checking host %s pubkey %s",
		peer->Text(), pubkey.Text() );

	// Round 1: P4TRUST (IP based)

	StrRef dummyuser( "**++**" );
	StrRef altuser( "++++++" );
	StrBuf trustkey;
	int doReplace = 0;
	char *keystr;

	{
	    Ticket hostfile( &trustfile );
	    keystr = hostfile.GetTicket( *peer, dummyuser );
	    if( keystr )
	    {
	        if( pubkey == keystr )
	            return;
	        trustkey.Set( keystr );
	    }
	}
	{
	    Ticket hostfile( &trustfile );
	    keystr = hostfile.GetTicket( *peer, altuser );
	    if( keystr && pubkey == keystr )
	        doReplace = 1;
	}
	if( doReplace )
	{
	    {
	        Ticket hostfile( &trustfile );
	        hostfile.ReplaceTicket( *peer, dummyuser, pubkey, e );
	    }
	    if( !e->Test() )
	    {
	        Ticket hostfile( &trustfile );
	        hostfile.DeleteTicket( *peer, altuser, e );
	    }
	    return;
	}

	if( trustkey.Length() )
	{
	    // Got a bad hit
	    e->Set( MsgRpc::HostKeyMismatch ) << *peer << pubkey;
	    return;
	}

	// Round 2: P4TRUST (FQDN based)

	peer = GetPeerAddress( RAF_REQ | RAF_PORT );
	StrBuf peerDNSPort = *peer;

	{
	    Ticket hostfile( &trustfile );
	    keystr = hostfile.GetTicket( *peer, dummyuser );
	    if( keystr )
	    {
	        if( pubkey == keystr )
	            return;
	        trustkey.Set( keystr );
	    }
	}
	{
	    Ticket hostfile( &trustfile );
	    keystr = hostfile.GetTicket( *peer, altuser );
	    if( keystr && pubkey == keystr )
	        doReplace = 1;
	}
	if( doReplace )
	{
	    {
	        Ticket hostfile( &trustfile );
	        hostfile.ReplaceTicket( *peer, dummyuser, pubkey, e );
	    }
	    if( !e->Test() )
	    {
	        Ticket hostfile( &trustfile );
	        hostfile.DeleteTicket( *peer, altuser, e );
	    }
	    return;
	}

	if( trustkey.Length() )
	{
	    // Got a bad hit
	    e->Set( MsgRpc::HostKeyMismatch ) << *peer << pubkey;
	    return;
	}

	// Round 3: Cert validity

	// sslLevel == 0, use P4TRUST: strict mode
	if( creds && sslLevel && !creds->IsSelfSigned() )
	{
	    peer = GetPeerAddress();
	    StrBuf peerIp = *peer;
	    peer = GetPeerAddress( RAF_REQ );
	    StrBuf peerDNS = *peer;

	    RPC_DBG_PRINTF( DEBUG_CONNECT,
	            "Checking host %s/%s cert chain",
	            peerIp.Text(), peerDNS.Text() );

	    // sslLevel < 2, chain must be valid
	    creds->ValidateChain( sslLevel != 1, e );
	    if( e->IsFatal() )
	        return;

	    // sslLevel >= 1, cert must be for the target server
	    creds->ValidateSubject( &peerDNS, &peerIp, e );

	    if( !e->Test() )
	        return;
	}

	e->Set( MsgRpc::HostKeyUnknown ) << peerIpPort << pubkey;
}

int
Rpc::GetInfo( StrBuf *b )
{
	return transport->GetInfo( b );
}

void
RpcUtility::Generate(RpcUtilityType type,
		     Error *e )
{
# ifdef USE_SSL
	NetSslCredentials credentials;

	switch( type )
	{
	case Generate_Credentials:
	    credentials.GenerateCredentials( e );
	    break;
	case Generate_Fingerprint:
	    credentials.ReadCredentials( e );
	    if ( !e->Test() )
	    {
		const StrPtr *fingerprint = credentials.GetFingerprint();
		if( fingerprint )
		    printf("Fingerprint: %s\n", fingerprint->Text());
	    }
	    break;
	}
	return;
# else
	e->Set( MsgRpc::SslNoSsl );
# endif //USE_SSL

}

void
RpcUtility::GetModTimes( P4INT64 *keyFile, P4INT64 *certFile, Error *e )
{
# ifdef USE_SSL
	NetSslCredentials credentials;
	credentials.GetModTimes( keyFile, certFile, e );
# else
	e->Set( MsgRpc::SslNoSsl );
# endif //USE_SSL
}

bool
RpcUtility::CheckCertsChanged( Error *e )
{
	P4INT64 keyTime = 0;
	P4INT64 certTime = 0;
	RpcUtility::GetModTimes( &keyTime, &certTime, e );
	
	if( ( keyTime <= 0 || keyTime <= lastSSLKeyModTime ) &&
	    ( certTime <= 0 || certTime <= lastSSLCertModTime ) )
	    return false;
	
	lastSSLKeyModTime = keyTime;
	lastSSLCertModTime = certTime;

	return true;
}

void
RpcUtility::CheckCertsChanged( RpcService *service, Error *e )
{
	// Check if the cert files have been modified (if SSL)
	if( !service->IsSSL() || !CheckCertsChanged( e ))
	    return;

	// Trigger cert reload
	service->NotifyRestarting();
}
