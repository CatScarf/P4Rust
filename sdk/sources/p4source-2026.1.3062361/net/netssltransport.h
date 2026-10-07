/**
 * @file netssltransport.h
 *
 * @brief SSL driver for NetTransport (connection use and close)
 *	NetSslTransport - a TCP subclass of NetTcpTransport
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

/*
 * These headers are required to be included before
 * including netssltransport.h
 *
 * extern "C" {	// OpenSSL
 *
 * # include "openssl/bio.h"
 * # include "openssl/ssl.h"
 * # include "openssl/err.h"
 *
 * }
 *
 * # include "netsupport.h"
 * # include "netport.h"
 * # include "netaddrinfo.h"
 * # include "netportparser.h"
 * # include "netconnect.h"
 * # include "nettcptransport.h"
 * # include "netsslcredentials.h"
 */

# ifdef USE_SSL

////////////////////////////////////////////////////////////////////////////
//  OpenSSL Dynamic Locking Callback Functions                            //
////////////////////////////////////////////////////////////////////////////

// Dynamic locking code only being used on NT in 2012.1, will be used on
// other platforms in 2012.2 (by that time I will include pthreads to the
// HPUX build). In 2012.1 HPUX has many compile errors for pthreads.
//
// OpenSSL 1.1 handles these internally so are no longer required

# if OPENSSL_VERSION_NUMBER < 0x10100000L
# ifndef OS_HPUX

# ifdef OS_NT
struct CRYPTO_dynlock_value
{
    HANDLE mutex;
};
# else
struct CRYPTO_dynlock_value
{
    pthread_mutex_t mutex;
};
# endif // OS_NT

# endif // OS_HPUX
# endif // !OpenSSL 1.1

////////////////////////////////////////////////////////////////////////////
//  Class NetSslTransport                                                 //
////////////////////////////////////////////////////////////////////////////

class NetSslTransport : public NetTcpTransport
{

    public:
	NetSslTransport( int t, bool fromClient );
	NetSslTransport( int t, bool fromClient, NetSslCredentials *cred );
	virtual ~NetSslTransport();

	virtual void	SetupSocket();
	virtual void	MoreSetupSocket();
	virtual void    SetNagle( int nagle );
	void            ValidateCredentials( Error *e );
	bool            CheckCtxErrors( const char *msg, Error *e );
	void            ClientMismatch( Error *e );
	void            DoHandshake( Error *e );
	void            Close();
	int             SendOrReceive( NetIoPtrs &io, Error *se, Error *re );
	void            GetEncryptionType(StrBuf &value)
	                { value.Set( cipherSuite ); }
	void            GetPeerFingerprint(StrBuf &value);
	NetSslCredentials *GetPeerCredentials()
	                { return credentials; }
	void		ReleaseCreds();

	static void	NotifyRestarting();


    private:
	SSL_CTX *	CreateAndInitializeSslContext(const char *conntype);
	void            SslClientInit( Error *e );
	void            SslServerInit( StrPtr *hostname, Error *e );
	void		ValidateRuntimeVsCompiletimeSSLVersion( Error *e );
	void            GetVersionString( StrBuf &sb, unsigned long version );

	// These two endpoint method need access to the Init methods
	friend NetTransport *NetSslEndPoint::Accept( KeepAlive *, Error *e );
	friend NetTransport *NetSslEndPoint::Connect( Error *e );



	static bool     VerifyKeyFile( const char *path );
	bool            SslHandshake( Error *e );

	static unsigned long  sCompileVersion;
	static SSL_CTX *sServerCtx;
	static SSL_CTX *sClientCtx;
	BIO *           bio;
	SSL *           ssl;
	StrBuf          cipherSuite;
	bool            clientNotSsl;
	bool		ownsCreds;
	NetSslCredentials *credentials;
	StrBuf          customCipherList;
	StrBuf          customCipherSuites;
	static bool     sIsRestarting;
} ;

# endif // USE_SSL
