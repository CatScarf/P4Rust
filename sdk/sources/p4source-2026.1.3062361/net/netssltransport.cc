/**
 * @file netssltransport.cc
 *
 * @brief SSL driver for NetTransport
 *	NetSslTransport - a TCP with SSL subclass of NetTcpTransport
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
# define NEED_SOCKET_IO
# define NEED_SLEEP

# ifdef OS_MPEIX
# define _SOCKET_SOURCE /* for sys/types.h */
# endif

# define GET_MJR_SSL_VERSION(x) \
    (((x) >> 12) & 0xfffffL)

# include <stdhdrs.h>

# ifndef OS_NT
#   include <pthread.h>
#   if defined( OS_DARWIN ) || defined( OS_MACOSX )
#     include <Security/Security.h>
#     include <CoreFoundation/CoreFoundation.h>
#   endif
# else
#   include <wincrypt.h>
# endif // not OS_NT

# include <error.h>
# include <strbuf.h>
# include <md5.h>
# include "netaddrinfo.h"
# include <errorlog.h>
# include <debug.h>
# include <vararray.h>
# include <bitarray.h>
# include <tunable.h>

# include <enviro.h>
# include <msgrpc.h>
# include <timer.h>
# include "datetime.h"
# include "filesys.h"
# include "pathsys.h"

# include <keepalive.h>
# include "netutils.h"
# include "netsupport.h"
# include "netport.h"
# include "netportparser.h"
# include "netconnect.h"
# include "nettcpendpoint.h"
# include "nettcptransport.h"
# include "netselect.h"
# include "netdebug.h"


extern "C"
{
    // OpenSSL
# include <openssl/ssl.h>
# include <openssl/err.h>
# include <openssl/x509v3.h>
# include <openssl/opensslv.h>
# include <openssl/crypto.h>
}
# include "netsslcredentials.h"
# include "netsslendpoint.h"
# include "netssltransport.h"
# include "netsslmacros.h"

# if OPENSSL_VERSION_NUMBER < 0x10100000L
extern "C" {

static int
InitLockCallbacks( Error *e );
static int
ShutdownLockCallbacks( void );
static void
LockingFunction( int mode, int n, const char *file, int line );
static unsigned long
IdFunction( void );
static struct CRYPTO_dynlock_value *
DynCreateFunction( const char *file, int line );
static void
DynLockFunction( int mode, struct CRYPTO_dynlock_value *l, const char *file, int line );
static void
DynDestroyFunction( struct CRYPTO_dynlock_value *l, const char *file, int line );

}

// OPENSSL_free was exposed in OpenSSL 1.1.0 for public use.
// In earlier versions it's marked as library internal.
# ifdef OPENSSL_free
# undef OPENSSL_free
# endif
# define OPENSSL_free free

# endif // !OpenSSL 1.1

static int LoadCACerts( SSL_CTX *ctx, const char *caPath )
{
	int loaded = 0;
	Error e;
	FileSys *f = FileSys::Create( FST_BINARY );
	f->Set( caPath );
	int stat = f->Stat();
	delete f;
	if( (stat & FSF_EXISTS) && (stat & FSF_DIRECTORY) )
	{
	    ERR_clear_error();
	    loaded = SSL_CTX_load_verify_locations( ctx, 0, caPath );
	    const char *msg = "NetSslTransport::LoadSystemCACerts "
	                      "SSL_CTX_load_verify_locations path";
	    SSLCHECKERROR( &e, msg, MsgRpc::SslInit, fail );
	    if( p4debug.GetLevel( DT_SSL ) == 2 )
                p4debug.printf( msg );
	}
	else if( (stat & FSF_EXISTS) )
	{
	    ERR_clear_error();
	    loaded = SSL_CTX_load_verify_locations( ctx, caPath, 0 );
	    const char *msg = "NetSslTransport::LoadSystemCACerts "
	                      "SSL_CTX_load_verify_locations file";
	    SSLCHECKERROR( &e, msg, MsgRpc::SslInit, fail );
	    if( p4debug.GetLevel( DT_SSL ) == 2 )
                p4debug.printf( msg );
	}
fail:
	return loaded;
}

static void LoadSystemCACerts( SSL_CTX *ctx )
{
	Error e;
	StrBuf caPath = p4tunable.GetString( P4TUNE_SSL_CLIENT_CA_PATH );
	if( caPath.Length() && LoadCACerts( ctx, caPath.Text() ) )
	    return;

#ifdef OS_NT
	HCERTSTORE hStore = 0;
	PCCERT_CONTEXT pContext = 0;
	X509 *x509 = 0;
	X509_STORE *store = 0;
	int ok = 0;
	
	store = SSL_CTX_get_cert_store( ctx );
	if( !store )
	{
	    store = X509_STORE_new();
	    SSL_CTX_set_cert_store( ctx, store );
	}

	const char* locations[] = { "CA", "ROOT", "MY", 0 };
	for( int i = 0; locations[i]; i++ )
	{
	    hStore = CertOpenSystemStore( 0, locations[i] );

	    if( !hStore )
	       continue;

	    while((pContext = CertEnumCertificatesInStore( hStore, pContext )))
	    {
	        x509 = d2i_X509( NULL,
	                     (const unsigned char **)&pContext->pbCertEncoded,
	                     pContext->cbCertEncoded );
	        if( x509 )
	        {
	            ok = X509_STORE_add_cert( store, x509 );
	            char *str = X509_NAME_oneline(
	                X509_get_subject_name( x509 ), 0, 0 );
	            StrBuf msg = "NetSslTransport::LoadSystemCACerts "
	                         "SSL_CTX_set_options(";
	            msg << (str ? str : "null") << "): "
	                << (ok == 1 ? "success" : "failed");
	            OPENSSL_free( str );
	            SSLCHECKERROR( &e, msg.Text(), MsgRpc::SslInit, failLoad );
	            if( p4debug.GetLevel( DT_SSL ) == 2 && !e.Test() )
	                p4debug.printf( "%s Successfully called.\n", msg.Text() );
failLoad:
	            X509_free( x509 );
	        }
	    }

	    CertFreeCertificateContext( pContext );
	    CertCloseStore( hStore, 0 );
	    hStore = 0;
	}
#elif defined( OS_MACOSX )
	CFArrayRef certs = NULL;
	CFIndex count, index;
	X509 *x509;
	X509_STORE *store = 0;
	int ok = 0;

	store = SSL_CTX_get_cert_store( ctx );
	if( !store )
	{
	    store = X509_STORE_new();
	    SSL_CTX_set_cert_store( ctx, store );
	}
	
	SecTrustSettingsDomain domain;
	SecTrustSettingsDomain domains[] = {
	    kSecTrustSettingsDomainUser,
	    kSecTrustSettingsDomainAdmin,
	    kSecTrustSettingsDomainSystem
	};
	int domainCount = sizeof( domains ) / sizeof( domain );

	for( int d = 0; d < domainCount; d++ )
	{
	    domain = domains[d];
	    if( !SecTrustSettingsCopyCertificates( domain, &certs ) )
	    {
	        count = CFArrayGetCount( certs );
	        for( index = 0; index < count; index++ )
	        {
	            SecCertificateRef cert =
	                (SecCertificateRef)CFArrayGetValueAtIndex( certs,index );
	            if( CFGetTypeID( cert ) != SecCertificateGetTypeID() )
	                continue;
#if defined( OS_MACOSX104 ) || defined( OS_MACOSX105 )
	            CSSM_DATA data;
	            SecCertificateGetData( cert, &data );
	            const unsigned char *buf = data.Data;
	            x509 = d2i_X509( NULL, &buf, data.Length );
#else
	            CFDataRef data = SecCertificateCopyData( cert );
	            const unsigned char *buf = CFDataGetBytePtr( data );
	            x509 = d2i_X509( NULL, &buf, CFDataGetLength( data ) );
#endif
	            if( x509 )
	            {
	                ok = X509_STORE_add_cert( store, x509 );
	                char *str = X509_NAME_oneline(
	                    X509_get_subject_name( x509 ), 0, 0 );
	                StrBuf msg = "NetSslTransport::LoadSystemCACerts "
	                    "SSL_CTX_set_options(";
	                msg << (str ? str : "null") << "): "
	                    << (ok == 1 ? "success" : "failed");
	                OPENSSL_free( str );
	                SSLCHECKERROR( &e,msg.Text(),MsgRpc::SslInit,failLoad );
	                if( p4debug.GetLevel( DT_SSL ) == 2 && !e.Test() )
	                    p4debug.printf( "%s Successfully called.\n",
	                                    msg.Text() );
failLoad:
	                X509_free( x509 );
	            }
#if !defined( OS_MACOSX104 ) && !defined( OS_MACOSX105 )
	            CFRelease( data );
#endif
	        }
	        CFRelease( certs );
	    }
	}
#else
	// Every distro has it's own place for SSL certs
	const char* locations[] = {
	    "/etc/pki/tls/certs/ca-bundle.crt",
	    "/etc/ssl/certs/ca-certificates.crt",
	    "/etc/openssl/certs/ca-certificates.crt",
	    "/etc/ssl/ca-bundle.pem",
	    "/etc/ssl/cacert.pem",
	    "/etc/pki/tls/cacert.pem",
	    "/etc/pki/ca-trust/extracted/pem/tls-ca-bundle.pem",
	    "/etc/certs/ca-certificates.crt",
	    "/usr/local/share/certs/ca-root-nss.crt",
	    "/System/Library/OpenSSL/certs/",
	    "/etc/ssl/certs",
	    "/system/etc/security/cacerts",
	    "/usr/local/share/certs",
	    "/etc/pki/tls/certs",
	    "/etc/openssl/certs",
	    "/var/ssl/certs",
	    "/etc/ssl/cert.pem",
	    0
	};
	for( int i = 0; locations[i]; i++ )
	    if( LoadCACerts( ctx, locations[i] ) )
	        return;
#endif
}

////////////////////////////////////////////////////////////////////////////
//  Defines and Globals                                                   //
////////////////////////////////////////////////////////////////////////////

/*
 * Current Cypher Suite Configuration:
 * Primary:   AES256-SHA        SSLv3 Kx=RSA      Au=RSA  Enc=AES(256)      Mac=SHA1
 * Secondary: CAMELLIA256-SHA   SSLv3 Kx=RSA      Au=RSA  Enc=Camellia(256) Mac=SHA1
 */
# define SSL_PRIMARY_CIPHER_SUITE12 "ECDHE-RSA-AES128-GCM-SHA256"
# define SSL_PRIMARY_CIPHER_SUITE "AES256-SHA"
# define SSL_SECONDARY_CIPHER_SUITE "CAMELLIA256-SHA"

const char *P4SslVersionString = OPENSSL_VERSION_TEXT;
unsigned long sCompileVersion =  OPENSSL_VERSION_NUMBER;

unsigned long sVersion1_0_0 =  0x1000000f;
const char *sVerStr1_0_0 = "1.0.0";
# if OPENSSL_VERSION_NUMBER < 0x10100000L
# ifdef OS_NT
static HANDLE mutexArray[ CRYPTO_NUM_LOCKS ];
# else
static pthread_mutex_t mutexArray[ CRYPTO_NUM_LOCKS ];
# endif
# endif // !OpenSSL 1.1

typedef struct {
    int		value;
    const char	*name;
} SslErrorNames;

SslErrorNames	sslErrorNames[] = {
    {SSL_ERROR_NONE,			" (None)"},		//  0
    {SSL_ERROR_SSL,			" (SSL_Protocol)"},	//  1
    {SSL_ERROR_WANT_READ,		" (Want_Read)"},	//  2
    {SSL_ERROR_WANT_WRITE,		" (Want_Write)"},	//  3
    {SSL_ERROR_WANT_X509_LOOKUP,	" (Want_X509_Lookup)"},	//  4
    {SSL_ERROR_SYSCALL,			" (Syscall)"},		//  5
    {SSL_ERROR_ZERO_RETURN,		" (Zero_Return)"},	//  6
    {SSL_ERROR_WANT_CONNECT,		" (Want_Connect)"},	//  7
    {SSL_ERROR_WANT_ACCEPT,		" (Want_Accept)"},	//  8
# if OPENSSL_VERSION_NUMBER >= 0x10100000L
    {SSL_ERROR_WANT_ASYNC,		" (Want_Async)"},	//  9
# endif // OPENSSL >= 1.1.x
# if OPENSSL_VERSION_NUMBER >= 0x30000000L
    {SSL_ERROR_WANT_ASYNC_JOB,		" (Want_Async_Job)"},	// 10
    {SSL_ERROR_WANT_CLIENT_HELLO_CB,    " (Want_Client_Hello_CB"},// 11
    {SSL_ERROR_WANT_RETRY_VERIFY,	" (Want_Retry_Verify"},	// 12
# endif // OPENSSL >= 3.x.x
};

const char *
GetSslErrorName( int err )
{
    if( err < SSL_ERROR_NONE || err >= sizeof( sslErrorNames )/sizeof( sslErrorNames[0] ) )
	return "(unknown)";

    return sslErrorNames[err].name;
}

////////////////////////////////////////////////////////////////////////////
//  MutexLocker                                                           //
////////////////////////////////////////////////////////////////////////////
# ifdef OS_NT
# define ONE_SECOND 1000
class MutexLocker
{
    public:
	MutexLocker( HANDLE &mutex );
	~MutexLocker();

    private:
	DWORD   retval;
	HANDLE &mutex;
};

MutexLocker::MutexLocker( HANDLE &theLock )
    : mutex(theLock)
{
	retval = WaitForSingleObject( mutex, ONE_SECOND );
	if( retval != WAIT_OBJECT_0 )
	    DEBUGPRINT( SSLDEBUG_ERROR,
		    "Unable to acquire lock on windows." );
}

MutexLocker::~MutexLocker()
{
	if( retval == WAIT_OBJECT_0 )
	    ReleaseMutex( mutex );
}

# endif // OS_NT

////////////////////////////////////////////////////////////////////////////
//  SSLKEYLOGFILE support                                                 //
////////////////////////////////////////////////////////////////////////////
# if OPENSSL_VERSION_NUMBER >= 0x10100000L
MT_STATIC FileSys *sslKeyLogFile = 0;
void P4SSLKeyLog( const SSL *ssl, const char *line )
{
	Error e;
	if( !sslKeyLogFile )
	{
	    StrBuf file = p4tunable.GetString( P4TUNE_SSL_KEYLOG_FILE );
	    if( !file.Length() )
	        return;
	    sslKeyLogFile = FileSys::Create( FST_ATEXT );
	    sslKeyLogFile->Set( file );
	    sslKeyLogFile->Perms( FPM_RW );
	}

	sslKeyLogFile->Open( FOM_WRITE, &e );
	if( !e.Test() )
	{
	    StrBuf data( line );
	    data << "\n";
	    sslKeyLogFile->Write( data, &e );
	    sslKeyLogFile->Close( &e );
	}

	if( e.Test() )
	    AssertLog.Report( &e );
}
#endif

////////////////////////////////////////////////////////////////////////////
//  Global                                                                //
////////////////////////////////////////////////////////////////////////////
# ifdef OS_NT
HANDLE                    sClientMutex = CreateMutex( NULL, FALSE, NULL );
HANDLE                    sServerMutex = CreateMutex( NULL, FALSE, NULL );
# endif // OS_NT

/**
 * MillisecondDifference
 *
 * @brief Utility to find the difference between two DateTimeHighPrecision
 * Note: Assumes that the difference is bounded by at most one second
 * since this is used to measure passage of time in a select with a 1/2 sec
 * timeout.
 *
 * @param lop left operator in subtraction
 * @param rop right operator in subtraction
 * @return count of milliseconds difference
 */
int
MillisecondDifference(const DateTimeHighPrecision &lop,
		      const DateTimeHighPrecision &rop)
{
    int sec = lop.Seconds() - rop.Seconds();
    int leftMs = (sec * 1000) + (lop.Nanos() / 1000000);
    int rightMs = (rop.Nanos() / 1000000);
    return (leftMs - rightMs);
}

////////////////////////////////////////////////////////////////////////////
//  NetSslTransport - STATIC MEMBERS                                      //
////////////////////////////////////////////////////////////////////////////
SSL_CTX *NetSslTransport::sServerCtx = NULL;
SSL_CTX *NetSslTransport::sClientCtx = NULL;
bool	NetSslTransport::sIsRestarting = false;

/**
 * NetSslTransport::constructor
 *
 * @brief constructor
 * @invariant assures that bio and ssl are NULL prior to use
 *
 * @param int socket
 * @param bool flag indicating if server or client side
 *
 * Server:
 * p4tunables::list[] has values from the defns in debug.cc + loaded by
 * LoadConfig() at server startup (or admin restart).
 *
 * Client:
 * Socket setup is done in NetSslEndPoint::Connect().
 */
NetSslTransport::NetSslTransport( int t, bool fromClient )
    : NetTcpTransport( t, fromClient ), credentials(NULL)
{
	this->bio = NULL;
	this->ssl = NULL;
	this->clientNotSsl = false;
	this->ownsCreds = true;
	this->credentials = new NetSslCredentials;

	/*
	 * proxy (and maybe broker) don't call SetupSocket()
	 * on accepted sockets so we'll always do it,
	 * even though p4d will also do it later.
	 */
	SetupSocket();
}

// Called for servers from NetSslEndPoint::Accept()
// NB: NetSslEndPoint owns the passed-in cred
NetSslTransport::NetSslTransport( int t, bool fromClient,
				NetSslCredentials *cred )
    : NetTcpTransport( t, fromClient ), credentials( cred )
{
	this->bio = NULL;
	this->ssl = NULL;
	this->clientNotSsl = false;
	this->ownsCreds = false;

	if( !credentials )
	{
	    /*
	     * It's an error to pass in NULL; this will prevent a crash,
	     * but p4d won't accept connections without valid
	     * credentials from NetSslEndPoint.
	     */
	    credentials = new NetSslCredentials;
	    this->ownsCreds = true;
	    p4debug.printf( "NetSslTransport ctor: NULL credentials given\n" );
	}

	/*
	 * proxy (and maybe broker) don't call SetupSocket()
	 * on accepted sockets so we'll always do it,
	 * even though p4d will also do it later.
	 */
	SetupSocket();
}

NetSslTransport::~NetSslTransport()
{
	/*
	 * NOTE: Never "delete credentials;"
	 *
	 * "credentials" points either to the
	 * NetSslTransport::myCredentials object (which will get
	 * auto-deleted when this transport object is deleted),
	 * or to the credentials owned by a NetSslEndPoint object
	 * (which will get deleted when the NetSslEndPoint object
	 * is deleted).
	 */

	Close();
	ReleaseCreds();

	if( ssl )
	{
	    SSL_free( ssl );
	    ssl = NULL;
	}
}

void
NetSslTransport::ReleaseCreds()
{
	credentials->ReleaseCredentials( true, true );
	if( ownsCreds )
	{
	    delete credentials;
	    credentials = NULL;
	}
}

/*
 * Server:
 * Called from Rh::Run() in a child request-handler process
 * after the child proc has reloaded values via LoadConfig()
 * (so the relevant configurables may have changed since instantiation).
 * Socket setup is done in Rh::Run() via Rpc::TransportSetupSocket().
 *
 * Client:
 * Socket setup is done in NetSslEndPoint::Connect().
 *
 * For both client and server, socket setup is also done in the
 * transport constructor, but possibly with different config values.
 */
void
NetSslTransport::SetupSocket()
{
	TRANSPORT_PRINTF( DEBUG_CONNECT, "NetSslTransport::SetupSocket(fd=%d, reload=%d)", t, afterReload );
	this->NetTcpTransport::SetupSocket(); // base class setup
}

// SSL-specific setup, called from NetTcpTransport::SetupSocket()
void
NetSslTransport::MoreSetupSocket()
{
	TRANSPORT_PRINTF( DEBUG_CONNECT, "NetSslTransport::MoreSetupSocket(fd=%d, reload=%d)", t, afterReload );
	this->NetTcpTransport::MoreSetupSocket(); // parent might add code later

	cipherSuite.Set( "encrypted" );
	customCipherList = p4tunable.GetString( P4TUNE_SSL_CIPHER_LIST );
	if( !isAccepted && p4tunable.IsSet( P4TUNE_SSL_CLIENT_CIPHER_LIST ) )
	    customCipherList = p4tunable.GetString( P4TUNE_SSL_CLIENT_CIPHER_LIST );
	customCipherSuites = p4tunable.GetString( P4TUNE_SSL_CIPHER_SUITES );
	if( !isAccepted && p4tunable.IsSet( P4TUNE_SSL_CLIENT_CIPHER_SUITES ) )
	    customCipherSuites = p4tunable.GetString( P4TUNE_SSL_CLIENT_CIPHER_SUITES );

	int nagle = p4tunable.Get( P4TUNE_NET_NAGLE );
	SetNagle( nagle );
}

// MS Visual Studio didn't implement snprintf until VS 2015.  Sigh.
# ifdef _MSC_VER
  #define SNPRINTF1(buf, len, msg, arg1)	sprintf( buf, msg, arg1 )
  #define SNPRINTF2(buf, len, msg, arg1, arg2)	sprintf( buf, msg, arg1, arg2 )
# else
  #define SNPRINTF1(buf, len, msg, arg1)	snprintf( buf, len, msg, arg1 )
  #define SNPRINTF2(buf, len, msg, arg1, arg2)	snprintf( buf, len, msg, arg1, arg2 )
# endif

SSL_CTX *
NetSslTransport::CreateAndInitializeSslContext( const char *conntypename )
{
    char	msgbuf[128];
    size_t	bufsize = sizeof( msgbuf ) - 1;

    SNPRINTF1( msgbuf, bufsize,
	       "NetSslTransport::Ssl%sInit - Initializing CTX structure.",
	       conntypename );
    TRANSPORT_PRINT_VAR( SSLDEBUG_FUNCTION, msgbuf );

    /*
     * In OpenSSL v1.1.0 we can use
     *	TLS_method()
     *	SSL_CTX_set_min_proto_version()
     *	SSL_CTX_set_max_proto_version()
     * instead of the deprecated
     * 	SSLv23_method()
     * and the not-recommended
     *	SSL_CTX_set_options( SSL_OP_NO_xxx )
     * but we still build with OpenSSL v1.0.x, so for now we'll stay
     * with the old way of doing things.
     *
     * See https://www.openssl.org/docs/man1.1.0/ssl/SSLv23_method.html
     * Note:
     *	"Clients should avoid creating "holes" in the set of protocols they support.
     *	When disabling a protocol, make sure that you also disable either all
     *	previous or all subsequent protocol versions. In clients, when a protocol
     *	version is disabled without disabling all previous protocol versions,
     *	the effect is to also disable all subsequent protocol versions."
     */

     /*
      * Start by allowing all protocol versions
      */

    ERR_clear_error();
    SSL_CTX	*ctxp = SSL_CTX_new( SSLv23_method() );
    SNPRINTF1( msgbuf, bufsize, "NetSslTransport::Ssl%sInit SSL_CTX_new", conntypename );
    TRANSPORT_PRINT_VAR( SSLDEBUG_FUNCTION, msgbuf );

    ERR_clear_error();
    SSL_CTX_set_mode(
	ctxp,
	SSL_MODE_ENABLE_PARTIAL_WRITE | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER );
    SNPRINTF1( msgbuf, bufsize, "NetSslTransport::Ssl%sInit SSL_CTX_set_mode", conntypename );
    SSLLOGFUNCTION( msgbuf );

    /*
     * Now disable SSLv2 and SSLv3 (but still allow TLSv1.0 and later)
     */

    ERR_clear_error();
    SSL_CTX_set_options( ctxp, SSL_OP_NO_SSLv2 );
    SNPRINTF1( msgbuf, bufsize, "NetSslTransport::Ssl%sInit SSL_CTX_set_options(NO_SSLv2)", conntypename );
    SSLLOGFUNCTION( msgbuf );

    ERR_clear_error();
    SSL_CTX_set_options( ctxp, SSL_OP_NO_SSLv3 );
    SNPRINTF1( msgbuf, bufsize, "NetSslTransport::Ssl%sInit SSL_CTX_set_options(NO_SSLv3)", conntypename );
    SSLLOGFUNCTION( msgbuf );

    /*
     * Let the customer disable any protocols they don't want.
     * Allow only TLS versions in the (closed) range [tlsmin, tlsmax].
     */
    int	tlsmin = p4tunable.Get( P4TUNE_SSL_TLS_VERSION_MIN );
    int	tlsmax = p4tunable.Get( P4TUNE_SSL_TLS_VERSION_MAX );

    if( !strcmp( conntypename, "Client" ) )
    {
	/*
	 * Clients have a second min/max
	 * Setting ssl.tls.version.min/max overrides the client
	 * default, unless that is also set
	 */
	if( p4tunable.IsSet( P4TUNE_SSL_CLIENT_TLS_VERSION_MIN ) ||
	    !p4tunable.IsSet( P4TUNE_SSL_TLS_VERSION_MIN ) )
	    tlsmin = p4tunable.Get( P4TUNE_SSL_CLIENT_TLS_VERSION_MIN );

	if( p4tunable.IsSet( P4TUNE_SSL_CLIENT_TLS_VERSION_MAX ) ||
	    !p4tunable.IsSet( P4TUNE_SSL_TLS_VERSION_MAX ) )
	    tlsmax = p4tunable.Get( P4TUNE_SSL_CLIENT_TLS_VERSION_MAX );
    }

    struct TlsVersion {
	int		value;		// tunable value
	int		proto;		// OpenSSL protocol version value
	const char	*name;		// OpenSSL protocol version name
    };

    /*
     * Expand this table if and when new TLS protocol versions are available.
     * This table *must* be ordered by the "value" field
     * and terminated by a "value" of 0.
     */
    static TlsVersion	tlsVersions[] = {
	{ 10,	SSL_OP_NO_TLSv1,	"NO_TLSv1.0" },
	{ 11,	SSL_OP_NO_TLSv1_1,	"NO_TLSv1.1" },
	{ 12,	SSL_OP_NO_TLSv1_2,	"NO_TLSv1.2" },
# if OPENSSL_VERSION_NUMBER >= 0x10100000L
	{ 13,	SSL_OP_NO_TLSv1_3,	"NO_TLSv1.3" },
# endif
	{ 0,	0,			NULL}
    };

    /*
     * Pin the value to the legal range.
     * Update this code if new values are added to tlsVersions[].
     */
    if( tlsmin < 10 )
	tlsmin = 10;
    if( tlsmin > 13 )
	tlsmin = 13;
    
    if( tlsmax < 10 )
	tlsmax = 10;
    if( tlsmax > 13 )
	tlsmax = 13;

    // Max can't exceed min
    if( tlsmax < tlsmin )
	tlsmax = tlsmin;

    if( SSLDEBUG_FUNCTION )
    {
	p4debug.printf( "NetSslTransport::Ssl%sInit tlsmin=%d, tlsmax=%d\n",
			conntypename, tlsmin, tlsmax );
    }

# if OPENSSL_VERSION_NUMBER >= 0x30000000L
    // If using lower than TLS1.2, we need to enable lower security
    if( tlsmin < 12 || tlsmax < 12 )
    {
	ERR_clear_error();
	SSL_CTX_set_security_level( ctxp, 0 );
	SNPRINTF1( msgbuf, bufsize,
	    "NetSslTransport::Ssl%sInit SSL_CTX_set_security_level(0)",
	    conntypename );
	SSLLOGFUNCTION( msgbuf );
    }
# endif

    // disallow protocols below the requested minimum
    for( TlsVersion *vp = tlsVersions; vp->value; vp++ )
    {
	if( vp->value < tlsmin )
	{
	    ERR_clear_error();
	    SSL_CTX_set_options( ctxp, vp->proto );
	    SNPRINTF2( msgbuf, bufsize,
		       "NetSslTransport::Ssl%sInit SSL_CTX_set_options(%s)",
		       conntypename, vp->name );
	    SSLLOGFUNCTION( msgbuf );
	}
    }

    // disallow protocols above the requested maximum
    for( TlsVersion *vp = tlsVersions; vp->value; vp++ )
    {
	if( vp->value > tlsmax )
	{
	    ERR_clear_error();
	    SSL_CTX_set_options( ctxp, vp->proto );
	    SNPRINTF2( msgbuf, bufsize,
		       "NetSslTransport::Ssl%sInit SSL_CTX_set_options(%s)",
		       conntypename, vp->name );
	    SSLLOGFUNCTION( msgbuf );
	}
    }

#ifdef SSL_OP_NO_ENCRYPT_THEN_MAC
    if( !p4tunable.Get( P4TUNE_SSL_ENABLE_ETM ) )
    {
	ERR_clear_error();
	SSL_CTX_set_options( ctxp, SSL_OP_NO_ENCRYPT_THEN_MAC );
	SNPRINTF2( msgbuf, bufsize,
		       "NetSslTransport::Ssl%sInit SSL_CTX_set_options(%s)",
		       conntypename,"SSL_OP_NO_ENCRYPT_THEN_MAC" );
	SSLLOGFUNCTION( msgbuf );
    }
#endif

# if OPENSSL_VERSION_NUMBER >= 0x10100000L
    if( p4tunable.GetString( P4TUNE_SSL_KEYLOG_FILE ).Length() )
    {
	ERR_clear_error();
	SSL_CTX_set_keylog_callback( ctxp, P4SSLKeyLog );
	SNPRINTF1( msgbuf, bufsize,
	           "NetSslTransport::Ssl%sInit SSL_CTX_set_keylog_callback()",
	           conntypename );
	SSLLOGFUNCTION( msgbuf );
    }
# endif

    return ctxp;
}

/**
 * NetSslTransport::SslClientInit
 *
 * @brief static method to initialize the SSL client-side CTX structure
 *
 * @param error structure
 * @return none
 */

void
NetSslTransport::SslClientInit( Error *e )
{
	if( sClientCtx )
	    return;

# ifdef OS_NT
	/*
	 * Windows multi-threaded so must synchronize
	 * CTX creation, since we only want one.
	 */
	MutexLocker locker( sClientMutex );
	/*
	 * Now that we have the lock see if any
	 * thread created the CTX while we were
	 * waiting.
	 */
# endif // OS_NT

	if( !sClientCtx )
	{
# if OPENSSL_VERSION_NUMBER < 0x10100000L
	    // Silence -Wunused;
	    (void)&InitLockCallbacks;
	    (void)&ShutdownLockCallbacks;
# endif

# ifdef OS_NT
# if OPENSSL_VERSION_NUMBER < 0x10100000L
	    InitLockCallbacks( e );
	    if( e->Test())
		return;
# endif // !OpenSSL 1.1
# endif // OS_NT

	    ValidateRuntimeVsCompiletimeSSLVersion( e );
	    if( e->Test() )
	    {
		TRANSPORT_PRINT( SSLDEBUG_ERROR,
			"Version mismatch between compile OpenSSL version and runtime OpenSSL version." );
		return;
	    }

# if OPENSSL_VERSION_NUMBER < 0x10100000L
	    /*
	     * Added due to job084753: Swarm is a web app that reuses processes.
	     * For some reason in this environment the SSL error stack is not removed
	     * when the process is reused so we need to explicitly remove any previous
	     * state prior to setting up a new SSL Context.
	     *
	     * Deprecated in OpenSSL 1.1.0
	     */
	    ERR_remove_thread_state( NULL );
	    // probably cannot check for error return from this call :-)

	    // As of OpenSSL 1.1.0, SSL_load_error_strings() is not needed
	    ERR_clear_error();
	    SSL_load_error_strings();
	    SSLCHECKERROR( e,
	        "NetSslTransport::SslClientInit SSL_load_error_strings",
	         MsgRpc::SslInit,
	          fail );
# endif
# if OPENSSL_VERSION_NUMBER < 0x30000000L
	    // As of OpenSSL 3, ERR_load_*_strings() are not needed

	    // don't continue if we have already failed
	    if( CheckCtxErrors( "SslClientInit", e ) )
		goto fail;

	    ERR_load_BIO_strings();
	    SSLCHECKERROR( e,
	        "NetSslTransport::SslClientInit ERR_load_BIO_strings",
	        MsgRpc::SslInit,
	        fail );
# endif
# if OPENSSL_VERSION_NUMBER < 0x10100000L
	    // As of OpenSSL 1.1.0, SSL_library_init() is not needed
	    ERR_clear_error();
	    if ( !SSL_library_init() )
	    {
	        // executable not compiled supporting SSL
	        // need to link with open SSL libraries
	        e->Set( MsgRpc::SslNoSsl );
	        return;
	    }
	    SSLCHECKERROR( e, "NetSslTransport::SslClientInit SSL_library_init",
	        MsgRpc::SslInit,
	        fail );
# endif

	    // WSAstartup code in NetTcpEndPoint constructor

	    /*
	     * Allow TLSv1.0 and later but disable SSLv2 and SSLv3
	     * - Allow customers to further filter TLS protocol versions
	     */
	    if( SSLDEBUG_FUNCTION )
		p4debug.printf( "NetSslTransport::SslClientInit(): Creating client SSL context\n" );
	    if( (sClientCtx = CreateAndInitializeSslContext( "Client" )) == NULL )
		goto fail;

	    LoadSystemCACerts( sClientCtx );
	}
	return;

fail:
	e->Set( MsgRpc::SslCtx ) << "the connecting client";
	return;
}

void
NetSslTransport::GetPeerFingerprint( StrBuf &value )
{
	if( !isAccepted && credentials->GetFingerprint() &&
		credentials->GetFingerprint()->Length() )
	    value.Set( credentials->GetFingerprint()->Text() );
	else
	    value.Clear();
}

/*
 * We're processing `p4 admin restart` or `kill -HUP`
 * - the admin might have changed 'certificate.txt'
 *   and/or 'privatekey.txt' so set sIsRestarting
 *   to ensure that we'll re-read the credentials
 *   when SslServerInit is next called.
 *
 * [static]
 */
void
NetSslTransport::NotifyRestarting()
{
	sIsRestarting = true;
}


/**
 * NetSslTransport::SslServerInit
 *
 * @brief One time initialization for server side of connection for SSL.
 * @invariant Assures that the server-side SSL CTX structure has been initialized.
 *
 * @param error structure
 * @return nothing
 */

void
NetSslTransport::SslServerInit( StrPtr *hostname, Error *e )
{
	// if we are not restarting and have already initialized
	// then we're done
	if( !sIsRestarting && sServerCtx )
	    return;

	bool wasRestarting = sIsRestarting;
	X509 *chainCert = 0;

# ifdef OS_NT
	/*
	 * Windows multi-threaded so must synchronize
	 * CTX creation, since we only want one.
	 */
	MutexLocker locker( sServerMutex );
	/*
	 * Now that we have the lock see if any
	 * thread created the CTX while we were
	 * waiting.
	 */
# endif // OS_NT

	if( sIsRestarting )
	{
	    // toss credentials, etc, so that we (re)start clean
	    if( ssl )
	    {
		SSL_free( ssl );
		ssl = NULL;
	    }

	    if( sServerCtx )
	    {
		SSL_CTX_free( sServerCtx );
		sServerCtx = NULL;
	    }

	    credentials->ReleaseCredentials( true, true );
	    sIsRestarting = false;
	}

	// already initialized
	if( sServerCtx )
	    return;

	/*
	 * If we're restarting then we've already performed
	 * our one-time initialization at startup.
	 * All of this setup is only for OpenSSL before v3.0,
	 * and so is not currently needed.
	 */
	if( !wasRestarting )
	{
# ifdef OS_NT
# if OPENSSL_VERSION_NUMBER < 0x10100000L
	    InitLockCallbacks( e );
	    if( e->Test())
		return;
# endif // !OpenSSL 1.1
# endif // OS_NT

# if OPENSSL_VERSION_NUMBER < 0x10100000L
	    /*
	     * Added due to job084753: Swarm is a web app that reuses processes.
	     * For some reason in this environment the SSL error stack is not removed
	     * when the process is reused so we need to explicitly remove any previous
	     * state prior to setting up a new SSL Context.
	     *
	     * Deprecated in OpenSSL 1.1.0
	     */
	    ERR_remove_thread_state( NULL );
	    // probably cannot check for error return from this call :-)

	    // As of OpenSSL 1.1.0, SSL_load_error_strings() is not needed
	    ERR_clear_error();
	    SSL_load_error_strings();
	    SSLCHECKERROR( e,
		"NetSslTransport::SslClientInit SSL_load_error_strings",
		MsgRpc::SslInit,
		fail );
# endif
# if OPENSSL_VERSION_NUMBER < 0x30000000L
	    // As of OpenSSL 3, ERR_load_*_strings() are not needed

	    // don't continue if we a havelready failed
	    if( CheckCtxErrors( "SslServerInit", e ) )
		goto fail;

	    ERR_load_BIO_strings();
	    SSLCHECKERROR( e,
		"NetSslTransport::SslServerInit ERR_load_BIO_strings",
		MsgRpc::SslInit,
		fail );
# endif
# if OPENSSL_VERSION_NUMBER < 0x10100000L
	    // As of OpenSSL 1.1.0, SSL_library_init() is not needed
	    ERR_clear_error();
	    if( !SSL_library_init() )
	    {
		// executable not compiled supporting SSL
		// need to link with open SSL libraries
		e->Set( MsgRpc::SslNoSsl );
		return;
	    }
	    SSLCHECKERROR( e, "NetSslTransport::SslClientInit SSL_library_init",
		MsgRpc::SslInit,
		fail );
# endif
	} // !wasRestarting

	/* Set up cert and key:
	 * Note that we have already verified in RpcService::Listen
	 * that sslKey and sslCert exist in P4SSLDIR and are valid.
	 * Since we are doing lazy init of the SslCtx the
	 * files could have been deleted before hitting this code.  We will do
	 * another verify check here when we finally load the credentials
	 * for the first read/write.
	 */
	credentials->ReadCredentials( e );
	P4CHECKERROR( e, "NetSslTransport::SslServerInit ReadCredentials", fail );

	/*
	 * Allow TLSv1.0 and later but disable SSLv2 and SSLv3
	 * - Allow customers to further filter TLS protocol versions
	 */
	if( SSLDEBUG_FUNCTION )
	    p4debug.printf( "NetSslTransport::SslServerInit(): Creating server SSL context\n" );
	if( (sServerCtx = CreateAndInitializeSslContext( "Server" )) == NULL )
	    goto fail;

	ERR_clear_error();
	SSL_CTX_use_PrivateKey( sServerCtx, credentials->GetPrivateKey() );
	SSLLOGFUNCTION(
	    "NetSslTransport::SslServerInit SSL_CTX_use_PrivateKey" );
	credentials->SetOwnKey( false );
	ownsCreds = false;

	/*
	 * Note: if want key passphrase protected then need to implement
	 * a callback function to supply the passphrase:
	 *     int passwd_cb( char *buf, int size, int flag, void *userdata );
	 *
	 * Alternatively strip the passphrase protection off the key via
	 *     cp server.key server.key.org
	 *     openssl [rsa|dsa] -in server.key.org -out server.key
	 */
	ERR_clear_error();
	SSL_CTX_use_certificate( sServerCtx, credentials->GetCertificate() );
	SSLLOGFUNCTION(
	    "NetSslTransport::SslServerInit SSL_CTX_use_certificate" );
	credentials->SetOwnCert( false );

	/*
	 * If we have a chain, add those certs to the context
	 */
	for( int i = 0; (chainCert = credentials->GetChain( i++ )); )
	{
	    ERR_clear_error();
	    SSL_CTX_add_extra_chain_cert( sServerCtx, chainCert );
	    SSLLOGFUNCTION(
	       "NetSslTransport::SslServerInit SSL_CTX_add_extra_chain_cert" );
	}

	/*
	 * Set context to not verify certificate authentication with CA.
	 */
	ERR_clear_error();
	SSL_CTX_set_verify( sServerCtx, SSL_VERIFY_NONE, NULL /* no callback */);
	SSLLOGFUNCTION(
	    "NetSslTransport::SslServerInit SSL_CTX_set_verify server ctx" );

	/*
	 * NOTE: The way this code is written there is no CA check on the cert.
	 * If want to do this then SSL_CTX_load_verify_locations( sServerCtx, NULL, sslCACert );
	 * The certificate authority certificate directory must be hashed:
	 *     c_rehash /path/to/certfolder
	 */

	return;

fail:
	// clean up to get a fresh start for a subsequent call
	if( ssl )
	{
	    SSL_free( ssl );
	    ssl = NULL;
	}

	if( sServerCtx )
	{
	    SSL_CTX_free( sServerCtx );
	    sServerCtx = NULL;
	}

	credentials->ReleaseCredentials( true, true );
	sIsRestarting = wasRestarting;
	e->Set( MsgRpc::SslCtx ) << "the accepting server";
}

/**
 * NetSslTransport::DoHandshake
 *
 * @brief transport level method to invoke SSL handshake and provide debugging output
 * @invariant if SSL and CTX structures are non-null handshake has completed successfully
 *
 * @param error structure
 * @return none
 */
void
NetSslTransport::DoHandshake( Error *e )
{
	if( ssl )
	    return;

	/*
	 * Lazy initialization: If no SSL context has been created for
	 * this type of connection (server side or client side) then do it
	 * now, then use it to create an SSL structure for this connection.
	 */
	if( this->isAccepted )
	{
	    ERR_clear_error();
	    if( sServerCtx == NULL )
	    {
		if( SSLDEBUG_FUNCTION )
		    p4debug.printf( "NetSslTransport::DoHandshake(): Re-creating server SSL context\n" );
		sServerCtx = CreateAndInitializeSslContext( "Server" );
		SSLNULLHANDLER( sServerCtx, e, "NetSslTransport::DoHandshake SSL_new", fail );
	    }
	    ssl = SSL_new( sServerCtx );
	    SSLNULLHANDLER( ssl, e, "NetSslTransport::DoHandshake SSL_new", fail );
	    if( customCipherList.Length() )
	    {
		ERR_clear_error();
		SSL_set_cipher_list( ssl, customCipherList.Text() );
		SSLLOGFUNCTION( "NetSslTransport::DoHandshake SSL_set_cipher_list custom" );
	    }
	    else if ( p4tunable.Get( P4TUNE_SSL_SECONDARY_SUITE ) )
	    {
		ERR_clear_error();
		SSL_set_cipher_list( ssl, SSL_SECONDARY_CIPHER_SUITE );
		SSLLOGFUNCTION( "NetSslTransport::DoHandshake SSL_set_cipher_list secondary" );
	    }
	    else if ( p4tunable.Get( P4TUNE_SSL_TLS_VERSION_MIN ) < 12 )
	    {
		StrBuf cipherSuite;
		cipherSuite << SSL_PRIMARY_CIPHER_SUITE12 << ":"
			    << SSL_PRIMARY_CIPHER_SUITE;
		ERR_clear_error();
		SSL_set_cipher_list( ssl, cipherSuite.Text() );
		SSLLOGFUNCTION( "NetSslTransport::DoHandshake SSL_set_cipher_list primary+legacy" );
	    }
	    else
	    {
		ERR_clear_error();
		SSL_set_cipher_list( ssl, SSL_PRIMARY_CIPHER_SUITE12 );
		SSLLOGFUNCTION( "NetSslTransport::DoHandshake SSL_set_cipher_list primary" );
	    }

# if OPENSSL_VERSION_NUMBER >= 0x10100000L
	    // TLS 1.3 can send session-resumption info after the main handshake, which
	    // will cause us to hang, so since we don't use sessions, disable that.
	    ERR_clear_error();
	    SSL_set_num_tickets( ssl, 0 );
	    SSLLOGFUNCTION( "NetSslTransport::DoHandshake SSL_set_num_tickets" );
	    
	    if( customCipherSuites.Length() )
	    {
		ERR_clear_error();
		SSL_set_ciphersuites( ssl, customCipherSuites.Text() );
		SSLLOGFUNCTION( "NetSslTransport::DoHandshake SSL_set_ciphersuites custom" );
	    }
# endif
	}
	else
	{
	    ERR_clear_error();
	    ssl = SSL_new( sClientCtx );
	    SSLNULLHANDLER( ssl, e, "NetSslTransport::DoHandshake SSL_new", fail );

	    if( customCipherList.Length() )
	    {
		ERR_clear_error();
		SSL_set_cipher_list( ssl, customCipherList.Text() );
		SSLLOGFUNCTION( "NetSslTransport::DoHandshake SSL_set_cipher_list custom" );
	    }
	    else
	    {
		StrBuf suites;
		suites << SSL_PRIMARY_CIPHER_SUITE12 << ":"
		       << "HIGH:"
		       << SSL_PRIMARY_CIPHER_SUITE << ":"
		       << SSL_SECONDARY_CIPHER_SUITE;
		ERR_clear_error();
		SSL_set_cipher_list( ssl, suites.Text() );
		SSLLOGFUNCTION( "NetSslTransport::DoHandshake SSL_set_cipher_list primary+secondary+high" );
	    }

# if OPENSSL_VERSION_NUMBER >= 0x10100000L
	    if( customCipherSuites.Length() )
	    {
		ERR_clear_error();
		SSL_set_ciphersuites( ssl, customCipherSuites.Text() );
		SSLLOGFUNCTION( "NetSslTransport::DoHandshake SSL_set_ciphersuites custom" );
	    }
# endif

	    // SNI support
	    const char * hostname = GetPortParser().Host().Text();
	    // SSL_set_tlsext_host_name() requires a non-empty hostname
	    if( !hostname || !*hostname )
	    	hostname = "localhost";
	    ERR_clear_error();
	    SSL_set_tlsext_host_name( ssl, hostname );
	    StrBuf msg = "NetSslTransport::DoHandshake SSL_set_tlsext_host_name: \"";
	    msg << hostname << "\"";
	    SSLLOGFUNCTION( msg.Text() );
	}

	/*
	 * If debugging output configured, dump prioritized list of
	 * supported cipher-suites.
	 */
	if( SSLDEBUG_CERT )
	{
	    int priority = 0;
	    bool shouldContinue = true;
	    p4debug.printf( "List of Cipher Suites supported:\n" );

	    while( shouldContinue )
	    {
		const char *cipherStr = SSL_get_cipher_list( ssl, priority );
		priority++;
		shouldContinue = (cipherStr) ? true : false;
		if( shouldContinue )
		{
		    p4debug.printf( "  Priority %d: %s\n", priority, cipherStr );
		}
	    }
	}

	/* Create a bio for this new socket */
	bio = BIO_new_socket( t, BIO_NOCLOSE );
	SSLNULLHANDLER( bio, e, "NetSslTransport::DoHandshake BIO_new_socket",
                    fail );

	/**
	 * Hooks the BIO up with the SSL *. Note once this is called then the
	 * SSL * owns the BIOs and will cleanup their allocation in an SSL_free
	 * if the reference count becomes zero.
	 */
	SSL_set_bio( ssl, bio, bio );
	SSLLOGFUNCTION( "NetSslTransport::DoHandshake SSL_set_bio" );

	/*
	 * Blocking mode disables our DoS prevention
	 * but might be more reliable under heavy connection load.
	 */
	if( p4tunable.Get( P4TUNE_SSL_HANDSHAKE_BLOCKING ) )
	{
	    SetSockBlocking( t, true );
	    bool result = SslHandshake(e);
	    SetSockBlocking( t, false );

	    if( !result )
		goto fail;
	}
	else
	{
	    if( !SslHandshake(e) )
		goto fail;
	}

	if( !isAccepted )
	{
# if OPENSSL_VERSION_NUMBER < 0x30000000L
	    X509 *serverCert = SSL_get_peer_certificate( ssl );
# else
	    X509 *serverCert = SSL_get1_peer_certificate( ssl );
# endif
	    stack_st_X509 *serverCertChain = SSL_get_peer_cert_chain( ssl );
	    X509_STORE *store = SSL_CTX_get_cert_store( SSL_get_SSL_CTX( ssl ) );
	    credentials->SetCertificate( serverCert, serverCertChain, store, e );

	    if ( e->Test() )
	    {
		X509_free( serverCert );
		if( SSLDEBUG_CERT )
		    p4debug.printf( "(a) connect: Credential error" );
		goto failNoRead;
	    }

	    SSLLOGFUNCTION( credentials->GetFingerprint()->Text() );
	    if( SSLDEBUG_CONNECT )
	        p4debug.printf(
	            "NetSslTransport::DoHandshake %s certificate received\n",
	            credentials->IsSelfSigned() ? "self-signed" : "chain" );

	    if( SSLDEBUG_CERT )
	    {
		char *str = NULL;
		// print out CERT info from server

		p4debug.printf( "Server certificate:\n" );
		str = X509_NAME_oneline( X509_get_subject_name( serverCert ), 0,
				     0 );
		SSLNULLHANDLER( str, e, "connect X509_get_subject_name", fail );
		p4debug.printf( "\t subject: %s\n", str );
		OPENSSL_free( str );
		str = X509_NAME_oneline( X509_get_issuer_name( serverCert ), 0,
				     0 );
		SSLNULLHANDLER( str, e, "connect X509_get_issuer_name", fail );
		p4debug.printf( "\t issuer: %s\n", str );
		OPENSSL_free( str );

	    }
	    X509_free( serverCert );
	    SSLLOGFUNCTION( "X509_free" );
	}

	return;

	/* Error Handling */
fail:
	lastRead = 1;

failNoRead:
	TRANSPORT_PRINT( SSLDEBUG_ERROR,
		"NetSslTransport::DoHandshake In fail error code." );
	if( ssl )
	{
	    ERR_clear_error();
	    SSL_free( ssl );
	    SSLLOGFUNCTION( "NetSslTransport::DoHandshake SSL_free" );
	    bio = NULL;
	    ssl = NULL;
	}
	// CloseSocket();
	if( isAccepted )
	{
	    TRANSPORT_PRINT( SSLDEBUG_ERROR,
		"NetSslTransport::DoHandshake failed on server side.");
	    if( !e->Test() )
		e->Set( MsgRpc::SslAcceptFailed ) << GetPortParser().String() << "";
	}
	else
	{
	    TRANSPORT_PRINT( SSLDEBUG_ERROR,
		    "NetSslTransport::DoHandshake failed on client side.");
	    if( !e->Test() )
	    {
		e->Set( MsgRpc::SslConnectFailed )
		    << GetPortParser().String() << "" << "";
	    }
	}
}

/**
 * NetSslTransport::SslHandshake
 *
 * @brief low level method to invoke ssl handshake and handle asynchronous behavior of
 * the BIO structure.
 *
 * @param error structure
 * @return bool true = success, false = fail
 */
bool
NetSslTransport::SslHandshake( Error *e )
{
	int       readable = 0;
	int       writable = 0;
	int       counter = 0;
	int       sslClientTimeoutMs
	    = p4tunable.Get( P4TUNE_SSL_CLIENT_TIMEOUT ) * 1000;
	DateTimeHighPrecision dtBeforeSelect, dtAfterSelect;
	int maxwait = GetMaxWait();
	if( maxwait && maxwait > sslClientTimeoutMs )
	    sslClientTimeoutMs = maxwait;

	static const int kCounterLimit = 10;

	/* select timeout */
	const int tv = !maxwait || maxwait > HALF_SECOND ?
	    HALF_SECOND : maxwait;

	enum Status
	{
	    ST_FAILED,
	    ST_RUNNING,
	    ST_SUCCESS
	};

	Status status = ST_RUNNING;
	bool done = false;
	bool showDetailedSslError = true;
	const char *errorReason = "";
	long errErrorNum = SSL_ERROR_NONE;
	int errorRet = 0;

	do
	{
	    /*
	     * We must clear the error queue before any SSL_*() call
	     * if afterwards we're going to check for errors
	     * with ERR_get_error() or SSL_get_error().
	     */
	    ERR_clear_error();
	    errorReason = "";
	    showDetailedSslError = true;

	    // Perform an SSL handshake.
	    int ret = 0;

	    if( isAccepted )
		ret = SSL_accept( ssl );
	    else
		ret = SSL_connect( ssl );

	    errErrorNum = ERR_get_error();
	    done = SSL_is_init_finished( ssl );

	    int errnum = GetLastSockError();

	    // for handshake debugging, print the SSL state info if not finished
	    if( !done && SSLDEBUG_CONNECT )
	    {
		int state = SSL_get_state( ssl );

		// Don't log EAGAIN; that's way too verbose.
		if( !IsRetryError( errnum ) )
		{
		    StrBuf errBuf;

		    Error::StrError( errBuf, errnum );
		    TRANSPORT_PRINTF( SSLDEBUG_CONNECT,
			"(0) SslHandshake ret=%d, errno=%d (%s), state=%d (%s) [%s], finished=%s",
			ret, errnum, errBuf.Text(), state, SSL_state_string(ssl),
			SSL_state_string_long(ssl), (done ? "true" : "false") );
		}
	    }

	    if( done != (ret == 1) )
	    {
		// I think that done == true iff ret == 1, so we should never get here
		// but it's not well documented, so I'll report it here
		// if they're ever not in sync
		p4debug.printf( "(0a) SslHandshake: done=%d, ret=%d\n", done, ret );
	    }

	    if( done || (ret == 1) )
		return true; // success!

	    switch( errorRet = SSL_get_error( ssl, ret ) )
	    {
	    case SSL_ERROR_NONE:
		TRANSPORT_PRINTF( SSLDEBUG_CONNECT,
		    "(1) NetSslTransport::SslHandshake protocol=%s",
		    SSL_get_version( ssl ) );
		status = ST_SUCCESS;
		break;

	    case SSL_ERROR_ZERO_RETURN:
		{
		    /* connection closed */
		    if( isAccepted )
		    {
			TRANSPORT_PRINT( SSLDEBUG_ERROR, "(2a) SSL_accept returned SSL_ERROR_ZERO_RETURN" );
		    }
		    else
		    {
			TRANSPORT_PRINT( SSLDEBUG_ERROR, "(2b) SSL_connect returned SSL_ERROR_ZERO_RETURN" );
		    }

		    char sslErrorBuf[256];

		    ERR_error_string_n( errErrorNum, sslErrorBuf, sizeof( sslErrorBuf ) );
		    TRANSPORT_PRINTF( SSLDEBUG_ERROR, "Handshake Failed: %s", sslErrorBuf );

		    /*
		     * Mismatched TLS version in 3.0 is 0xA00042E:
		     * 0xA000000 means ERR_LIB_SSL (which is 20 [err.h])
		     * 0x42E is  SSL_R_TLSV1_ALERT_PROTOCOL_VERSION
		     *   [1070 in ssl.h]
		     * 0xA00042E is
		     *   ERR_REASON(SSL_R_TLSV1_ALERT_PROTOCOL_VERSION,
		     *       "tlsv1 alert protocol version")
		     * and ERR_REASON(reason) is
		     *   ERR_PACK(ERR_LIB_SSL,0,reason)
		     *   [in sslerr.c]
		     * See ssl_err.[hc] for details.
		     *
		     * So we could have a separate SslProtocolGenericError
		     * ErrorId and use SslProtocolError only for this
		     * specific error, but there's little penalty
		     * for always using SslProtocolError, and the
		     * "fix TLS settings" clause might apply for other
		     * cases as well, so I'm leaving this code disabled
		     * (and I haven't defined SslGenericProtocolError).
		     */

#ifdef USE_SSL_USE_GENERIC_PROTOCOL_ERROR
		    if( ERR_GET_LIB( errErrorNum ) == ERR_LIB_SSL &&
			ERR_GET_REASON( errErrorNum )
				== SSL_R_TLSV1_ALERT_PROTOCOL_VERSION )
		    {
			e->Set( MsgRpc::SslProtocolError )
			    << GetPortParser().String()
			    << sslErrorBuf;
		    }
		    else
		    {
			/*
			 * If you enable this code, you must
			 * define SslProtocolGenericError
			 * - like SslProtocolError but delete
			 *   " (fix the TLS settings)"
			 */
			e->Set( MsgRpc::SslProtocolGenericError )
			    << GetPortParser().String()
			    << sslErrorBuf;
		    }
#else
		    e->Set( MsgRpc::SslProtocolError )
			<< GetPortParser().String()
			<< sslErrorBuf;
#endif
		    e->Set( MsgRpc::SslCloseEOF )
			<< GetPeerAddress( RAF_PORT )->Text();

		    status = ST_FAILED;
		    errorReason = "(connection closed)";
		    /*
		     * SslProtocolError contains the low-level SSL
		     * error detail, so don't add that detail to the
		     * generic SslConnectFailed/SslAcceptFailed error.
		     */
		    showDetailedSslError = false;
		}
		break;

	    case SSL_ERROR_WANT_READ:
		readable = 1;
		status = ST_RUNNING;
		TRANSPORT_PRINTF( SSLDEBUG_FUNCTION,
		    "(3a) NetSslTransport::SslHandshake WANT_READ ret=%d", ret );
		break;

	    case SSL_ERROR_WANT_WRITE:
		writable = 1;
		status = ST_RUNNING;
		TRANSPORT_PRINTF( SSLDEBUG_FUNCTION,
		    "(3b) NetSslTransport::SslHandshake WANT_WRITE ret=%d", ret );
		break;

	    case SSL_ERROR_SYSCALL:
		errorReason = "(syscall failed)";
		if( (ret == -1) && IsRetryError(errnum) )
		{
		    bool want_read = SSL_want_read( ssl );
		    bool want_write = SSL_want_write( ssl );

		    status = ST_RUNNING;

		    if( want_read || want_write )
		    {
			if( want_read )
			{
			    readable = 1;
			}
			if( want_write )
			{
			    writable = 1;
			}
		    }
		    else
		    {
			status = ST_FAILED;
			errorReason = "(syscall failed: not readable or writable)";
			TRANSPORT_PRINTF( SSLDEBUG_ERROR,
			    "(4) NetSslTransport::SslHandshake SYSCALL %s ret=%d",
			    errorReason, ret );
		    }
		}
		else
		{
		    status = ST_FAILED;
		    errorReason = "(syscall failed: no retry)";

		    char sslErrorBuf[256] = "";

		    ERR_error_string_n( errErrorNum, sslErrorBuf, sizeof(sslErrorBuf) );
		    TRANSPORT_PRINTF( SSLDEBUG_ERROR,
			"(4a) NetSslTransport::SslHandshake %s %s ret=%d",
			errorReason, sslErrorBuf, ret );

		    /*
		     * I don't understand why these errors are reported
		     * as SYSCALL errors, but we'll handle them specially
		     * and add a SslProtocolError to let the user know
		     * that they should to fix their TLS settings.
		     *
		     * See the comments for case SSL_ERROR_ZERO_RETURN
		     * for a discussion of these OpenSSL macros.
		     */
		    if( ERR_GET_LIB( errErrorNum ) == ERR_LIB_SSL )
		    {
			switch( ERR_GET_REASON( errErrorNum ) )
			{
			    case SSL_R_UNSUPPORTED_PROTOCOL:
			    case SSL_R_NO_PROTOCOLS_AVAILABLE:
				e->Set( MsgRpc::SslProtocolError )
				    << GetPortParser().String()
				    << sslErrorBuf;
				errorReason = "(TLS protocol)";
				/*
				 * SslProtocolError contains the low-level SSL
				 * error detail, so don't add that detail to the
				 * generic SslConnectFailed/SslAcceptFailed error.
				 */
				showDetailedSslError = false;
				break;
			    default:
				break;
			}
		    }
		}
		break;

	    case SSL_ERROR_SSL:
		/*
		 * underlying protocol error;
		 * dump error to debug output
		 */
		{
		    if( errErrorNum )
		    {
			// buffer for ssl protocol errors
			char sslErrorBuf[256];
			ERR_error_string_n( errErrorNum, sslErrorBuf, sizeof( sslErrorBuf ) );
			TRANSPORT_PRINTF( SSLDEBUG_ERROR,
			    "(5) SSL Handshake Failed: %s", sslErrorBuf );
			StrBuf errbuf;
			errbuf.Set( sslErrorBuf );
			errbuf.Append( " {" );
			errbuf.Append( StrNum( errErrorNum ).Text() );
			errbuf.Append( "}" );
			e->Set( MsgRpc::SslProtocolError )
			    << GetPortParser().String()
			    << errbuf;
		    }
		    else
		    {
			e->Set( MsgRpc::SslProtocolError )
			    << GetPortParser().String()
			    << "";
		    }

		    status = ST_FAILED;
		    errorReason = "(SSL protocol error)";
		}
		// [[fallthrough]]; // requires C++17

		// these all fall through as well
	    case SSL_ERROR_WANT_X509_LOOKUP:
# if OPENSSL_VERSION_NUMBER >= 0x10100000L
	    case SSL_ERROR_WANT_ASYNC:
# endif // OPENSSL >= 1.1.x
# if OPENSSL_VERSION_NUMBER >= 0x30000000L
	    case SSL_ERROR_WANT_ASYNC_JOB:
	    case SSL_ERROR_WANT_CLIENT_HELLO_CB:
	    case SSL_ERROR_WANT_RETRY_VERIFY:
# endif // OPENSSL >= 3.x.x

	    default:
		{
		    StrBuf errBuf;

		    if( errorRet == SSL_ERROR_SSL )
		    {
			errBuf.Append( " (SSL protocol error)" );
		    }
		    else if( errErrorNum )
		    {
			char sslErrorBuf[256];

			ERR_error_string_n( errErrorNum, sslErrorBuf, sizeof( sslErrorBuf ) );
			errBuf.Append( "{" );
			errBuf.Append( GetSslErrorName( errorRet ) );
			errBuf.Append( ": " );
			errBuf.Append( sslErrorBuf );
			errBuf.Append( ": " );
			errBuf.Append( StrNum( errErrorNum ).Text() );
			errBuf.Append( "}" );
		    }
		    else if( Error::IsNetError() )
		    {
			StrBuf tmp;
			Error::StrNetError( tmp );
			errBuf.Append( " (" );
			errBuf.Append( &tmp );
			errBuf.Append( ")" );
		    }
		    else
		    {
			StrBuf tmp;
			Error::StrError( tmp );
			errBuf.Append( " (" );
			errBuf.Append( &tmp );
			errBuf.Append( ")" );
		    }

		    if( errErrorNum )
		    {
			char sslErrorBuf2[256];

			ERR_error_string_n( errErrorNum, sslErrorBuf2, sizeof( sslErrorBuf2 ) );
			errBuf.Append( " [" );
			errBuf.Append( sslErrorBuf2 );
			errBuf.Append( " {" );
			errBuf.Append( StrNum( errErrorNum ).Text() );
			errBuf.Append( "}]" );
		    }

		    if( isAccepted )
		    {
			TRANSPORT_PRINTF( SSLDEBUG_ERROR,
			    "(6a) NetSslTransport::SslHandshake failed on server side: %d %s, errno=%d",
			    errorRet, errBuf.Text(), errnum );
			e->Set( MsgRpc::SslAcceptFailed ) << errorReason << errBuf;
		    }
		    else
		    {
			TRANSPORT_PRINTF( SSLDEBUG_ERROR,
			    "(6b) NetSslTransport::SslHandshake failed on client side: %d %s, errno=%d",
			    errorRet, errBuf.Text(), errnum );
			e->Set( MsgRpc::SslConnectFailed )
			    << GetPortParser().String() << errorReason << errBuf;
		    }
		    return false;
		}
		break;

	    case SSL_ERROR_WANT_CONNECT:
	    case SSL_ERROR_WANT_ACCEPT:
		status = ST_RUNNING;
		break;
	    } // end switch - last SSL error

	    if( status == ST_RUNNING )
	    {
		/*
		 * Changed to prevent a bad guy forming a DOS attack on
		 * Linux by running nc against a p4d.  Previous code
		 * executed a tight loop calling SSL_accept and erroring
		 * out with a WANT_WRITE and a EAGAIN error. We stop this
		 * by sleeping for a millisecond before trying again.
		 * Some OSs return EWOULDBLOCK under this same condition.
		 *
		 * Note POSIX 2008 spec prevents the select call in this
		 * case statement from returning prematurely with EAGAIN,
		 * but Ubuntu seems to follow an earlier version of the
		 * spec. It appears that Mac OS X is complient since this
		 * problem does not exist on that platform.
		 */

		dtBeforeSelect.Now();
		int selectRet =
			selector->Select( readable, writable, tv );
		dtAfterSelect.Now();
		int elapsed = MillisecondDifference( dtAfterSelect, dtBeforeSelect );
		counter += elapsed;

		TRANSPORT_PRINTF( SSLDEBUG_CONNECT,
		    "(7) NetSslTransport::SslHandshake select=%dms, tv=%dms, counter=%dms, timeout=%dms)",
		    elapsed, tv, counter, sslClientTimeoutMs );

		if( selectRet < 0 )
		{
		    e->Sys( "select", "socket" );
		    return false;
		}

		if( counter > kCounterLimit )
		{
		    if( counter > sslClientTimeoutMs )
		    {
			const char *who = (isAccepted ? "server" : "client");
			TRANSPORT_PRINTF( SSLDEBUG_ERROR,
			    "(7a) NetSslTransport::SslHandshake failed on %s side: %d (timeout after %dms, timeout=%dms)",
			    who, errorRet, counter, sslClientTimeoutMs);

			if( isAccepted )
			    e->Set( MsgRpc::SslAcceptTimeout ) << GetPortParser().String();
			else
			    e->Set( MsgRpc::SslConnectTimeout ) << GetPortParser().String();

			return false;
		    }

		    TRANSPORT_PRINTF( SSLDEBUG_CONNECT,
			"(7b) NetSslTransport::SslHandshake sleeping: limit=%d, counter=%d, timeout=%dms",
			kCounterLimit, counter, sslClientTimeoutMs );

		    msleep(1);
		    counter++;
		}
		else
		{
		    TRANSPORT_PRINT( SSLDEBUG_FUNCTION,
			"(7c) NetSslTransport::SslHandshake WANT_READ with EAGAIN or EWOULDBLOCK");
		}
	    }
	} while( status == ST_RUNNING && !done );

	if( status == ST_FAILED )
	{
	    /*
	     * Either it was a real error or we gave up retrying.
	     * Check the underlying SSL ERR for info;
	     * if none then report StrNetError().
	     */
	    if( errErrorNum != SSL_ERROR_NONE )
	    {
		char errErrorStr[256] = "";

		// don't add the details if we've already done that
		if( showDetailedSslError )
		{
		    ERR_error_string_n( errErrorNum, errErrorStr,
			    sizeof( errErrorStr ) );
		}

		if( isAccepted )
		{
		    TRANSPORT_PRINTF( SSLDEBUG_ERROR,
			"(8a) SslHandshake failed on server side: SSL_accept error %s: {%s}",
			errorReason, errErrorStr );
		    e->Set( MsgRpc::SslAcceptFailed ) << errorReason << errErrorStr;
		}
		else
		{
		    TRANSPORT_PRINTF( SSLDEBUG_ERROR,
			"(8b) SslHandshake failed on client side: SSL_connect error %s: <%s>",
			errorReason, errErrorStr );
		    e->Set( MsgRpc::SslConnectFailed )
			<< GetPortParser().String()
			<< errorReason
			<< errErrorStr;
		}
	    }
	    else
	    {
		StrBuf buf;

		Error::StrNetError( buf );

		if( isAccepted )
		{
		    e->Net( "accept", buf.Text() );
		    TRANSPORT_PRINTF( SSLDEBUG_ERROR,
			"(8c) SslHandshake: SSL_accept error %s: <%s>",
			errorReason,
			buf.Text() );
		    e->Set( MsgRpc::SslAcceptFailed ) << errorReason << buf;
		}
		else
		{
		    e->Net( "connect", buf.Text() );
		    TRANSPORT_PRINTF( SSLDEBUG_ERROR,
			"(8d) SslHandshake: SSL_connect error %s: <%s>",
			errorReason,
			buf.Text() );
		    e->Set( MsgRpc::SslConnectFailed )
			<< GetPortParser().String()
			<< errorReason
			<< buf;
		}
	    }

	    return false;
	}

	return true;
}

/*
 * NetSslTransport::SendOrReceive() - send or receive data as ready
 *
 * If data to write and no room to read, block writing.
 * If room to read and no data to write, block reading.
 * If both data to write and room to read, block until one is ready.
 *
 * If neither data to write nor room to read, don't call this!
 *
 * Brain-dead version of NetSslSelector::Select() indicates both
 * read/write are ready.  To avoid blocking on read, we only do so
 * if not instructed to write.
 *
 * Returns 1 if either data read or written.
 * Returns 0 if neither, meaning EOF or error.
 */

int
NetSslTransport::SendOrReceive( NetIoPtrs &io, Error *se, Error *re )
{
	// Hacky solution to allow us to send out a cleartext error
	// message to the client if they are attempting a cleartext
	// connection to our SSL server.
	if( clientNotSsl )
	{
	    int retval = NetTcpTransport::SendOrReceive( io, se, re );
	    return retval;
	}
	if( t < 0 )
	{
	    TRANSPORT_PRINT( SSLDEBUG_ERROR,"NetSslTransport::SendOrReceive connection "
		    "closed, returning w/o doing anything." );
	    // Socket has been closed don't try to use
	    return 0;
	}
	StrBuf buf;
	int  doRead = 0;
	int  doWrite = 0;

	int dataReady;
	int maxwait = GetMaxWait();
	Timer waitTime;

	int  readable = 0;
	int  writable = 0;

	/* flags to mark all the combinations of why we're blocking */
	bool read_waiton_write = false;
	bool read_waiton_read = false;
	bool write_waiton_write = false;
	bool write_waiton_read = false;

	/* select timeout */
	int  tv;

	/* ssl status */
	int  sslError;
	int  sslPending;
	long errErrorNum = 0;
	char errErrorStr[256];

	// Lazy call of handshake code.
	if( !ssl )
	{
	    DoHandshake( se );
	    if( se->Test() )
		goto end;
	}

	// don't count the SSL handshake as part of the read/write time
	if( maxwait )
	{
	    waitTime.Start();
	}

	for ( ;; )
	{
	    doRead = io.recvPtr != io.recvEnd && !re->Test();
	    doWrite = io.sendPtr != io.sendEnd && !se->Test();

	    if( !doRead && !doWrite )
	    {
		// cannot read or write
		return 0;
	    }

	    /*
	     * If we enter this loop and are supposed to read AND
	     * there is something already in the SSL read buffers
	     * then don't wait in the select call for something
	     * to appear in the kernel buffers.
	     */
	    sslPending = SSL_pending( ssl );

	    /*
	     * Complex logic to check OS level buffers for
	     * read or write:
	     * For Write:
	     * - if entered this function to write then always check
	     * - if entered to read but handshake is blocking operation
	     *   then write before continue writing
	     * DO NOT check write otherwise since will cause tight loop
	     * burning CPU on interactive commands.
	     *
	     * For Read:
	     * - if entered this function to read then always check
	     * - if entered to write but handshake is blocking operation
	     *   then read before continue writing
	     */
	    readable =
		( doRead || write_waiton_read || read_waiton_read ) ? 1 : 0;
	    writable =
		( doWrite || write_waiton_write || read_waiton_write ) ? 1 : 0;

	    if (readable && sslPending)
		tv = 0;
	    else if( (readable && breakCallback) || maxwait )
	    {
		tv = !maxwait || maxwait > HALF_SECOND ? HALF_SECOND : maxwait;
		if( breakCallback )
		{
		    int p = breakCallback->PollMs();
		    if( p >= 1 )
			tv = p;
		}
	    }
	    else
		tv = -1;


	    if( ( dataReady = selector->Select( readable, writable, tv ) < 0 ) )
	    {
		re->Sys( "select", "socket" );
		return 0;
	    }

	    if( !dataReady && maxwait && waitTime.Time() >= maxwait )
	    {
	        lastRead = 0;
	        TRANSPORT_PRINT( SSLDEBUG_ERROR, "SSL SendOrReceive maxwait expired." );
	        if( doRead )
	            re->Set( MsgRpc::MaxWait ) << "receive" << ( maxwait / 1000 );
	        else
	            se->Set( MsgRpc::MaxWait ) << "send" << ( maxwait / 1000 );
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

	    if( SSLDEBUG_BUFFER )
	    {
		DateTimeHighPrecision dt;
		char buftime[ DTHighPrecisionBufSize ];

		dt.Now();
		dt.Fmt( buftime );

		p4debug.printf(
		    "State status:      time: %s\n"
		    "\tsslPending         %d - is something in the SSL read buffer?\n"
		    "\treadable           %d - is something in the OS read buffer?\n"
		    "\twritable           %d - is there available room OS write buffer?\n"
		    "\tdoRead             %d - we have room in P4rpc read buffer\n"
		    "\tdoWrite            %d - we have stuff to write in P4rpc write buffer\n"
		    "\twrite_waiton_write %d - ssl write buffer not available, try again when net net write buffer ready\n"
		    "\twrite_waiton_read  %d - ssl write buffer not available due to handshake, try again when net read buffer ready\n"
		    "\tread_waiton_write  %d - ssl read buffer not available due to handshake, try again when net write buffer ready\n"
		    "\tread_waiton_read   %d - ssl read buffer not available, try again when net read buffer ready\n",
		    buftime, sslPending, readable, writable, doRead, doWrite,
		    write_waiton_write, write_waiton_read, read_waiton_write,
		    read_waiton_read );
	    }

	    /* This "if" statement reads data. It will only be entered if
	     * the following conditions are all true:
	     * 1. we're not in the middle of a write
	     * 2. there's space left in the recv buffer
	     * 3. the kernel write buffer is available signaling that a handshake
	     *    write has completed (this write had been blocking our read, and
	     *    now we are free to continue it), or either SSL or OS says we can
	     *    read regardless of whether we're blocking for availability to read.
	     */
	    if( !( write_waiton_read || write_waiton_write ) && doRead
            && ( sslPending || readable || ( writable && read_waiton_write )) )
	    {
		/* clear the flags since we'll set them based on the I/O call's
		 * return
		 */
		read_waiton_read = false;
		read_waiton_write = false;

		/* read into the buffer after the current position */

		ERR_clear_error();
		int l = SSL_read( ssl, io.recvPtr, io.recvEnd - io.recvPtr );
		int errnum = GetLastSockError();
		errErrorNum = ERR_get_error();
		SSLLOGFUNC( "NetSslTransport::SendOrReceive SSL_read", errErrorNum );

		if( l >= 0 )
		    SetQuickAck();

		switch ( sslError = SSL_get_error( ssl, l ) )
		{
		case SSL_ERROR_NONE:
		    /*
		     * no errors occurred.  update the new buffer size and signal
		     * that "have data" by returning 1.
		     */
		    lastRead = 1;

		    if( l > 0 )
		    {
			TRANSPORT_PRINTF( SSLDEBUG_TRANS,
			    "NetSslTransport::SendOrReceive recv %d bytes",
			    l );
			io.recvPtr += l;
			return 1;
		    }
		    break;
		case SSL_ERROR_ZERO_RETURN:
		    /* connection closed */
		    TRANSPORT_PRINT( SSLDEBUG_ERROR, "SSL_read returned SSL_ERROR_ZERO_RETURN" );
		    goto end;
		    break;
		case SSL_ERROR_WANT_READ:
		    /*
		     * we need to retry the read after kernel buffer available for
		     * reading
		     */
		    TRANSPORT_PRINT( SSLDEBUG_ERROR, "SSL_read returned SSL_ERROR_WANT_READ" );
		    read_waiton_read = true;
		    break;
		case SSL_ERROR_WANT_WRITE:
		    /*
		     * we need to retry the read after kernel buffer is available for
		     * writing
		     */
		    TRANSPORT_PRINT( SSLDEBUG_ERROR, "SSL_read returned SSL_ERROR_WANT_WRITE" );
		    read_waiton_write = true;
		    break;
		case SSL_ERROR_SYSCALL:
		    if( IsRetryError( errnum ) )
		    {
			// Not a permanent error, so try again.
			break;
		    }

		    /*
		     * An I/O error occurred; check underlying SSL ERR for info
		     * if none then report StrNetError()
		     */
		    if ( errErrorNum == 0 )
		    {
			if( l == 0 )
			{
			    re->Net( "read", "SSL_read encountered an EOF." );
			    TRANSPORT_PRINT( SSLDEBUG_ERROR, "SSL_read encountered an EOF." );
			}
			else if ( l < 0 )
			{
			    Error::StrNetError( buf );
			    re->Net( "read", buf.Text() );
			    TRANSPORT_PRINTF( SSLDEBUG_ERROR, "SSL_read encountered a system error: %s", buf.Text() );
			}
			else
			{
			    TRANSPORT_PRINT( SSLDEBUG_FUNCTION,
				    "SSL_read claims SSL_ERROR_SYSCALL but returns data." );
			    lastRead = 1;
			    if( l > 0 )
			    {
				TRANSPORT_PRINTF( SSLDEBUG_TRANS,
					"NetSslTransport::SendOrReceive recv %d bytes\n",
					l );
				io.recvPtr += l;
				SetQuickAck();
			    }
			}
			return 1;
		    }
		    else
		    {
			ERR_error_string_n( errErrorNum, errErrorStr, sizeof( errErrorStr ) );
			re->Net( "read", errErrorStr );
			TRANSPORT_PRINTF( SSLDEBUG_ERROR,
				"SSL_read encountered a syscall ERR: %s", errErrorStr );
		    }
		    re->Set( MsgRpc::SslRecv );
		    goto end;
		    break;
		case SSL_ERROR_SSL:
		    /* underlying protocol error dump error to
		     * debug output
		     */

		    {
			if( errErrorNum )
			{
			    // buffer for ssl protocol errors
			    char sslErrorBuf[256];
			    ERR_error_string_n( errErrorNum, sslErrorBuf, sizeof( sslErrorBuf ) );
			    TRANSPORT_PRINTF( SSLDEBUG_ERROR, "SSL protocol failed during write: %s", sslErrorBuf );
			    re->Set( MsgRpc::SslProtocolError )
				<< GetPortParser().String()
				<< sslErrorBuf;
			}
			else
			{
			    re->Set( MsgRpc::SslProtocolError )
				<< GetPortParser().String()
				<< "";
			}
		    }
		    // [[fallthrough]]; // requires C++17
		default:
		    if( l == 0 )
		    {
			if( doWrite )
			{
			    // Error: connection was closed but we had stuff to write
			    re->Net( "read", "socket" );
			    re->Set( MsgRpc::SslRecv );
			}
			TRANSPORT_PRINT( SSLDEBUG_FUNCTION,
			    "SSL_read attempted on closed connection." );
			/*
			 * Connection closed but did not shutdown SSL cleanly.
			 * The p4 proxy will do this if the command issued by
			 * its client does not require forwarding to the p4d server.
			 */
			goto end;
		    }

		    /* ERROR */
		    re->Net( "read", "socket" );
		    re->Set( MsgRpc::SslRecv );
		    TRANSPORT_PRINTF( SSLDEBUG_ERROR,
			    "SSL_read returned unknown error: %d",
                                  sslError );
		    goto end;
		}
	    }

	    /* This "if" statement writes data. It will only be entered if
	     * the following conditions are all true:
	     * 1. we're not in the middle of a read
	     * 2. there's room in the buffer
	     * 3. the kernel read buffer is available signaling that a handshake
	     *    read has completed (this rad had been blocking our write, and
	     *    now we are free to continue it), or OS says we can write regardless
	     *    of whether we're blocking for availability to write.
	     */
	    if( !( read_waiton_write || read_waiton_read ) && doWrite
            && ( writable || ( readable && write_waiton_read )) )
	    {
		/* clear the flags */
		write_waiton_read = false;
		write_waiton_write = false;

		ERR_clear_error();
		/* perform the write from the start of the buffer */
		int l = SSL_write( ssl, io.sendPtr, io.sendEnd - io.sendPtr );
		int errnum = GetLastSockError();
		errErrorNum = ERR_get_error();
		SSLLOGFUNC( "NetSslTransport::SendOrReceive SSL_write", errErrorNum );

		switch ( sslError = SSL_get_error( ssl, l ) )
		{
		case SSL_ERROR_NONE:
		    /*
		     * no errors occurred.  update the new buffer size and signal
		     * that "have data" by returning 1.
		     */
		    lastRead = 0;

		    if( l > 0 )
		    {
			TRANSPORT_PRINTF( SSLDEBUG_TRANS,
				"NetSslTransport send %d bytes\n", l );
			io.sendPtr += l;
			return 1;
		    }
		    break;
		case SSL_ERROR_ZERO_RETURN:
		    /* connection closed */
		    TRANSPORT_PRINT( SSLDEBUG_ERROR,
			    "SSL_write returned SSL_ERROR_ZERO_RETURN" );
		    goto end;
		case SSL_ERROR_WANT_READ:
		    /*
		     * we need to retry the write after A is available for
		     * reading
		     */
		    TRANSPORT_PRINT( SSLDEBUG_ERROR,
			    "SSL_write returned SSL_ERROR_WANT_READ" );
		    write_waiton_read = true;
		    break;
		case SSL_ERROR_WANT_WRITE:
		    /*
		     * we need to retry the write after A is available for
		     * writing
		     */
		    TRANSPORT_PRINT( SSLDEBUG_ERROR,
			    "SSL_write returned SSL_ERROR_WANT_WRITE" );
		    write_waiton_write = true;
		    break;
		case SSL_ERROR_SYSCALL:
		    if( IsRetryError( errnum ) )
		    {
			// Not a permanent error, so try again.
			break;
		    }

		    /*
		     * An I/O error occurred; check underlying SSL ERR for info
		     * if none then report StrNetError()
		     */
	            if ( errErrorNum == 0 )
	            {
	        	if( l == 0 )
	        	{
	        	    se->Net( "write", "SSL_write encountered an EOF." );
	        	    TRANSPORT_PRINT( SSLDEBUG_ERROR, "SSL_write encountered an EOF." );
	        	}
	        	else if ( l < 0 )
	        	{
	        	    Error::StrNetError( buf );
	        	    se->Net( "write", buf.Text() );
	        	    TRANSPORT_PRINTF( SSLDEBUG_ERROR,
	        		    "SSL_write encountered a system error: %s", buf.Text() );
	        	}
	        	else
	        	{
	        	    TRANSPORT_PRINT( SSLDEBUG_ERROR,
	        		    "SSL_write claims SSL_ERROR_SYSCALL but returns data." );

			    lastRead = 0;
			    if( l > 0 )
			    {
				TRANSPORT_PRINTF( SSLDEBUG_TRANS, "NetSslTransport send %d bytes", l );
				io.sendPtr += l;
			    }
			}
			return 1;
	            }
	            else
	            {
	        	ERR_error_string_n( errErrorNum, errErrorStr, sizeof( errErrorStr ) );
	        	se->Net( "write", errErrorStr );
	        	TRANSPORT_PRINTF( SSLDEBUG_ERROR,
	        		"SSL_write encountered a syscall ERR: %s", errErrorStr );
	            }
	            se->Set( MsgRpc::SslSend );
	            goto end;
		    break;
		case SSL_ERROR_SSL:
		    /* underlying protocol error; dump error to
		     * debug output
		     */

		    {
			if( errErrorNum )
			{
			    // buffer for ssl protocol errors
			    char sslErrorBuf[256];
			    ERR_error_string_n( errErrorNum, sslErrorBuf, sizeof( sslErrorBuf ) );
			    TRANSPORT_PRINTF( SSLDEBUG_ERROR, "SSL write failed: %s", sslErrorBuf );
			    se->Set( MsgRpc::SslProtocolError )
				<< GetPortParser().String()
				<< sslErrorBuf;
			}
			else
			{
			    se->Set( MsgRpc::SslProtocolError )
				<< GetPortParser().String()
				<< "";
			}
		    }
		    // [[fallthrough]]; // requires C++17
		default:
		    if( l == 0 )
		    {
			/*
			 * Connection closed but did not shutdown SSL cleanly.
			 * The p4 proxy will do this if the command issued by
			 * its client does not require forwarding to the p4d server.
			 */
			TRANSPORT_PRINT( SSLDEBUG_FUNCTION,
			    "SSL_write attempted on closed connection." );
			goto end;
		    }

		    /* ERROR */
		    se->Net( "write", "socket" );
		    se->Set( MsgRpc::SslSend );
		    TRANSPORT_PRINTF( SSLDEBUG_ERROR,
			    "SSL_write returned unknown error: %d",
                                  sslError );
		    goto end;
		}
	    }
	}

end:
	return 0;
}


/**
 * NetSslTransport::Close
 *
 * @brief handles teardown of the SSL connection and deletion of the
 * of the SSL and BIO structures, closes socket
 * @invariant SSL and BIO are NULL if the connection is closed
 *
 * @return none
 */
void
NetSslTransport::Close( void )
{
	if( t < 0 )
	    return;

	TRANSPORT_PRINTF( SSLDEBUG_CONNECT, "NetSslTransport %s closing %s",
                           GetAddress( RAF_PORT )->Text(),
                           GetPeerAddress( RAF_PORT )->Text() );

	if( ssl )
	{
	    if( SSL_get_shutdown( ssl ) & SSL_RECEIVED_SHUTDOWN )
	    {
		// clean shutdown
		ERR_clear_error();
		SSL_shutdown( ssl );
		SSLLOGFUNCTION( "NetSslTransport::Close SSL_shutdown" );
	    }
	    else
	    {
		/*
		 * An error is causing this shutdown, so we remove session
		 * from cache.
		 */
		ERR_clear_error();
		SSL_clear( ssl );
		SSLLOGFUNCTION( "NetSslTransport::Close SSL_clear" );
	    }
	    ERR_clear_error();
	    BIO_pop( bio );
	    SSLLOGFUNCTION( "NetSslTransport::Close BIO_pop" );
	    SSL_free( ssl );
	    SSLLOGFUNCTION( "NetSslTransport::Close SSL_free" );
	}

	bio = NULL;
	ssl = NULL;

	// Avoid TIME_WAIT on the server by reading the EOF after
	// the last message sent by the client.  Getting the EOF
	// means we've received the TH_FIN packet, which means we
	// don't have to send our own on close().  He who sends
	// a TH_FIN goes into the 2 minute TIME_WAIT imposed by TCP.

	TRANSPORT_PRINTF( SSLDEBUG_TRANS, "NetSslTransport lastRead=%d", lastRead );

	// Only wait in select a second by default, since it's possible
	// we'll be in a state where the EOF never comes.
	const int max = p4tunable.Get( P4TUNE_NET_MAXCLOSEWAIT );

	if( lastRead )
	{
	    int  r = 1;
	    int  w = 0;
	    char buf[1];

	    if( selector->Select( r, w, max ) >= 0 && r )
	    {
		int n = read( t, buf, 1 );
		if( n < 0 )
		{
		    StrBuf errbuf;

		    Error::StrNetError( errbuf );
		    TRANSPORT_PRINTF( SSLDEBUG_ERROR, "NetSslTransport::Close: read of FIN packet failed (ignored): %s", errbuf.Text() );
		}
		SetQuickAck();
	    }
	}

	CloseSocket();
}

/*
 * Like NetTcpTransport::SetNagle()
 * - but we change 2 to 0 to disable Nagle for compatibility
 *   with existing behavior.
 * - TODO: remove this compatibility hack when we're convinced
 *    that we don't need it
 */
void
NetSslTransport::SetNagle( int nagle )
{
	TRANSPORT_PRINTF( DEBUG_CONNECT,
	    "NetSslTransport::SetNagle(fd=%d, reload=%d, enable=%d)",
		t, afterReload, nagle );

	if( nagle == 2 )
	    nagle = 0; // backwards compatibility: Nagle normally disabled for SSL

	NetUtils::SetNagle( t, nagle );
}

/*
 * Fetch, remove, and report SSL errors, but skip the spurious ones
 * generated by our credential fetches.
 *
 * NetSslCredentials::ReadCredentials() calls PEM_read_X509()
 * in a loop and will generate the "no start line" error when it reads
 * an EOF after the last certificate in each certificate file.
 *
 * Returns true iff there were any non-ignored errors.
 */
bool
NetSslTransport::CheckCtxErrors( const char *msg, Error *e )
{
	long errErrorNum = 0;
	bool ret = false;

	while( ( errErrorNum = ERR_get_error() ) != 0 )
	{
	    if( ERR_GET_LIB( errErrorNum ) != ERR_LIB_PEM ||
		ERR_GET_REASON( errErrorNum ) != PEM_R_NO_START_LINE )
	    {
		char sslErrorBuf[256];

		ERR_error_string_n( errErrorNum, sslErrorBuf,
		    sizeof( sslErrorBuf ) );

		DEBUGPRINTF( SSLDEBUG_CERT, "%s: %s",
		    msg, sslErrorBuf );

		e->Set( MsgRpc::SslCtx ) << sslErrorBuf;

		ret = true;
	    }
	}

	return ret;
}

void
NetSslTransport::ClientMismatch( Error *e )
{
	switch( CheckForHandshake( t ) )
	{
	case PeekCleartext:
	    TRANSPORT_PRINT( SSLDEBUG_ERROR,
	        "Handshake peek appears not to be for SSL." );
	    // this is a non-ssl connection
	    e->Set( MsgRpc::SslCleartext );
	    clientNotSsl = true;
	    break;
	case PeekTimeout:
	    /*
	     * This is not an error; the client is just slow
	     * sending the Client Hello request.
	     * If we set an error here then Rh::Run will abort
	     * the connection, so we'll just log the timeout;
	     * all will proceed normally later, after the client
	     * sends the Client Hello.
	     *
	     * If the client doesn't send a Client Hello
	     * (eg, it's making a clear-text connection
	     * but also is slow sending its first packet)
	     * then the SSL Handshake will fail with an unclear
	     * error (probably "wrong version number")
	     * rather than our user-friendly message
	     * from the PeekClearText case above.
	     *
	     * Note that the p4 client normally sends the Client Hello
	     * within 20-50 milliseconds after the 3-way TCP connection
	     * handshake completes, but if p4 is run under valgrind
	     * then it can take 2-4 seconds.
	     */
	    TRANSPORT_PRINTF( SSLDEBUG_ERROR,
		"Handshake peek from %s timed out.\n",
	        GetPeerAddress( RAF_PORT )->Text() );
	    break;
	default:
	    break;
	}
}

void
NetSslTransport::ValidateRuntimeVsCompiletimeSSLVersion( Error *e )
{
	StrBuf sb;
# if OPENSSL_VERSION_NUMBER < 0x10100000L
	GetVersionString( sb, SSLeay() );
# else
	GetVersionString( sb, OpenSSL_version_num() );
# endif
	TRANSPORT_PRINTF( SSLDEBUG_ERROR,
		"OpenSSL runtime version %s", sb.Text() );
	sb.Clear();
	GetVersionString( sb, OPENSSL_VERSION_NUMBER );
	TRANSPORT_PRINTF( SSLDEBUG_ERROR,
		"OpenSSL compile version %s", sb.Text() );

# if OPENSSL_VERSION_NUMBER < 0x10100000L
	if ( GET_MJR_SSL_VERSION( SSLeay() ) <
# else
	if( GET_MJR_SSL_VERSION( OpenSSL_version_num() ) <
# endif
	    GET_MJR_SSL_VERSION( sVersion1_0_0 ) )
	{
	    e->Set( MsgRpc::SslLibMismatch ) << sVerStr1_0_0;
	}
}

void
NetSslTransport::GetVersionString( StrBuf &sb, unsigned long version )
{
	/* Version numbering explanation from OpenSSL openssl/opensslv.h file:
	 *
	 * Numeric release version identifier:
	 * MNNFFPPS: major minor fix patch status
	 * The status nibble has one of the values 0 for development, 1 to e for betas
	 * 1 to 14, and f for release.  The patch level is exactly that.
	 * For example:
	 * 0.9.3-dev          0x00903000
	 * 0.9.3-beta1        0x00903001
	 * 0.9.3-beta2-dev    0x00903002
	 * 0.9.3-beta2        0x00903002 (same as ...beta2-dev)
	 * 0.9.3              0x0090300f
	 * 0.9.3a             0x0090301f
	 * 0.9.4              0x0090400f
	 * 1.2.3z             0x102031af
	 */
	unsigned long M = (version >> 28) & ~0xfffffff0L;
	unsigned long N = (version >> 20) & ~0xffffff00L;
	unsigned long P = (version >> 12) & ~0xffffff00L;
	sb << M << "." << N << "." << P;
}

////////////////////////////////////////////////////////////////////////////
//  OpenSSL Dynamic Locking Callback Functions                            //
////////////////////////////////////////////////////////////////////////////
// Dynamic locking code only being used on NT in 2012.1, will be used on
// other platforms in 2012.2 (by that time I will include pthreads to the
// HPUX build). In 2012.1 HPUX has many compile errors for pthreads.
//
// OpenSSL 1.1 handles these internally so are no longer required

#if OPENSSL_VERSION_NUMBER < 0x10100000L
#ifndef OS_HPUX

static void LockingFunction( int mode, int n, const char *file, int line )
{
# ifdef OS_NT
    if( mode & CRYPTO_LOCK )
    {
	WaitForSingleObject( mutexArray[n], INFINITE );
    }
    else
    {
	ReleaseMutex( mutexArray[n] );
    }
# else
    if( mode & CRYPTO_LOCK )
    {
	pthread_mutex_lock( &mutexArray[n] );
    }
    else
    {
	pthread_mutex_unlock( &mutexArray[n] );
    }
# endif // OS_NT
}

/**
 * OpenSSL uniq id function.
 *
 * @return    thread id
 */
static unsigned long IdFunction( void )
{
# ifdef OS_NT
    return ((unsigned long) GetCurrentThreadId());
# else
    return ((unsigned long) pthread_self());
# endif
}

/**
 * OpenSSL allocate and initialize dynamic crypto lock.
 *
 * @param    file    source file name
 * @param    line    source file line number
 */
static struct CRYPTO_dynlock_value *
DynCreateFunction( const char *file, int line )
{
    struct CRYPTO_dynlock_value *value;

    value = (struct CRYPTO_dynlock_value *) malloc(
	    sizeof( struct CRYPTO_dynlock_value ) );
    if( !value )
    {
	goto err;
    }
# ifdef OS_NT
    value->mutex = CreateMutex( NULL, FALSE, NULL );
# else
    pthread_mutex_init( &value->mutex, NULL );
# endif // OS_NT
    return value;

    err: return (NULL);
}

/**
 * OpenSSL dynamic locking function.
 *
 * @param    mode    lock mode
 * @param    l        lock structure pointer
 * @param    file    source file name
 * @param    line    source file line number
 * @return    none
 */
static void DynLockFunction(
        int mode,
        struct CRYPTO_dynlock_value *l,
        const char *file,
        int line )
{
# ifdef OS_NT
    if( mode & CRYPTO_LOCK )
    {
	WaitForSingleObject( l->mutex, INFINITE );
    }
    else
    {
	ReleaseMutex( l->mutex );
    }
# else
    if( mode & CRYPTO_LOCK )
    {
	pthread_mutex_lock( &l->mutex );
    }
    else
    {
	pthread_mutex_unlock( &l->mutex );
    }
# endif // OS_NT
}

/**
 * OpenSSL destroy dynamic crypto lock.
 *
 * @param    l        lock structure pointer
 * @param    file    source file name
 * @param    line    source file line number
 * @return    none
 */

static void DynDestroyFunction(
        struct CRYPTO_dynlock_value *l,
        const char *file,
        int line )
{
# ifdef OS_NT
    CloseHandle( l->mutex );
# else
    pthread_mutex_destroy( &l->mutex );
# endif // OS_NT
    free( l );
}

static int InitLockCallbacks( Error *e )
{
	for ( int i = 0; i < CRYPTO_num_locks(); i++ )
	{
# ifdef OS_NT
	    mutexArray[i] = CreateMutex( NULL, FALSE, NULL );
# else
	    pthread_mutex_init( &mutexArray[i], NULL );
# endif // OS_NT
	}
	/* static locks callbacks */
	CRYPTO_set_locking_callback( LockingFunction );
	CRYPTO_set_id_callback( IdFunction );
	/* dynamic locks callbacks */
	CRYPTO_set_dynlock_create_callback( DynCreateFunction );
	CRYPTO_set_dynlock_lock_callback( DynLockFunction );
	CRYPTO_set_dynlock_destroy_callback( DynDestroyFunction );

	return (0);
}

/**
 * Cleanup TLS library.
 *
 * @return    0
 */
static int ShutdownLockCallbacks( void )
{
    CRYPTO_set_dynlock_create_callback( NULL );
    CRYPTO_set_dynlock_lock_callback( NULL );
    CRYPTO_set_dynlock_destroy_callback( NULL );

    CRYPTO_set_locking_callback( NULL );
    CRYPTO_set_id_callback( NULL );

    for ( int i = 0; i < CRYPTO_num_locks(); i++ )
    {
# ifdef OS_NT
	CloseHandle( mutexArray[i] );
# else
	pthread_mutex_destroy( &mutexArray[i] );
# endif // OS_NT
    }

    return (0);
}

# endif // !OS_HPUX
# endif // !OpenSSL 1.1
# endif //USE_SSL
