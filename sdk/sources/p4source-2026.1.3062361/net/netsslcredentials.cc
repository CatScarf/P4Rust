/**
 * @file netsslcredentials.cc
 *
 * @brief Wrapper for getting or generating server cert and private key.
 *
 * Note: this code is derived from the OpenSSL demo program selfsign.c
 *
 * Threading: underlying SSL library contains threading
 *
 * @invariants:
 *
 * Copyright (c) 2011 Perforce Software
 * Confidential.  All Rights Reserved.
 * @author Wendy Heffner
 *
 * Creation Date: October 31, 2011
 */

# ifdef USE_SSL

# define NEED_GETUID
# define NEED_STAT
# define NEED_ERRNO

# include <stdhdrs.h>
# include <strbuf.h>
# include <vararray.h>
# include <intarray.h>
# include <error.h>
# include <errorlog.h>
# include <enviro.h>
# include <debug.h>
# include <pathsys.h>
# include <filesys.h>
# include <fileio.h>
# include <hostenv.h>
# include <utils.h>

#include "netportipv6.h"
#include "netport.h"
#include "netutils.h"

# include <msgrpc.h>
# include <msgos.h>

extern "C"
{ // OpenSSL
# include <openssl/err.h>
# include <openssl/x509v3.h>
# include <openssl/ssl.h>
# include <openssl/crypto.h>
# if OPENSSL_VERSION_NUMBER >= 0x30000000L
# include <openssl/evp.h>
# include <openssl/rsa.h>
# endif
}
# include <stdio.h>
# include "netdebug.h"
# include "netsslmacros.h"
# include "netsslcredentials.h"

////////////////////////////////////////////////////////////////////////////
//  NetSslCredentials - Globals and Defines                               //
////////////////////////////////////////////////////////////////////////////
# define SSL_CONFIGFILE        (const char*)"config.txt"
# define SSL_KEYFILE           (const char*)"privatekey.txt"
# define SSL_CERTFILE          (const char*)"certificate.txt"

# define SSL_X509_NUMBITS      4096
# define SSL_X509_VERSION      2
# define SSL_X509_SERIALNUM    01
# define SSL_X509_NOTBEFORE    0
# define SSL_X509_NOTAFTER     ((long) 730 )  // expires in two years
# define SSL_X509_DAY          ((long) 60 * 60 * 24 )
# define SSL_X509_MAX_SECONDS  2147483647 /*((1 << 31) - 1) */
// Fields:
// C = ISO3166 two character country code
// ST = state or province
// L = Locality; generally means city
// O = Organization - Company Name
// OU = Organization Unit - division or unit
// CN = CommonName - entity name e.g. www.example.com
// Example Values:
// C=MY,ST=another state,L=another city,O=my company,OU=certs,
// CN=www.example.com
# define SSL_X509_C      (const char*)"US"
# define SSL_X509_ST     (const char*)"CA"
# define SSL_X509_L      (const char*)"Alameda"
# define SSL_X509_O      (const char*)"Perforce Autogen Cert"

////////////////////////////////////////////////////////////////////////////
// Static Callback Function Implementation                                //
////////////////////////////////////////////////////////////////////////////

static void
PrintNodes( const char *name, STACK_OF(X509_POLICY_NODE) *nodes, BIO *bio,
            const char *sep )
{
	X509_POLICY_NODE *node;
	int i;

	BIO_printf( bio, "%s Policies:", name );
	if( nodes )
	{
	    BIO_puts( bio, sep );
	    for( i = 0; i < sk_X509_POLICY_NODE_num( nodes ); i++ )
	    {
	        node = sk_X509_POLICY_NODE_value( nodes, i );
	        X509_POLICY_NODE_print( bio, node, 2 );
	    }
	}
	else
	    BIO_printf( bio, " <empty>%s", sep );
}

static void
PrintPolicies( X509_STORE_CTX *ctx, BIO *bio, const char *sep )
{
	if( !ctx )
	    return;

	X509_POLICY_TREE *tree = X509_STORE_CTX_get0_policy_tree( ctx );
	int explicit_policy = X509_STORE_CTX_get_explicit_policy( ctx );

	BIO_printf( bio, "Require explicit Policy: %s%s",
	            explicit_policy ? "True" : "False", sep );

	PrintNodes( "Authority", X509_policy_tree_get0_policies( tree ),
	            bio, sep );
	PrintNodes( "User", X509_policy_tree_get0_user_policies( tree ),
	            bio, sep );
}

static void
PrintCertificateSubject( const char *msg, int depth, X509 *cert, const char *sep = "\n" )
{
	BIO *bio = BIO_new( BIO_s_mem() );
	BIO_printf( bio, "+" );
	if( depth >= 0 )
	    BIO_printf( bio, " #%d", depth );
	if( msg )
	    BIO_printf( bio, " %s", msg );
	if( msg || (depth >= 0) )
	    BIO_printf( bio, ": " );
	if( cert )
	{
	    BIO_printf( bio, "Subject: " );
	    X509_NAME_print_ex( bio, X509_get_subject_name(cert), 0,
	                        XN_FLAG_ONELINE );
	}
	else
	{
	    BIO_printf( bio, "<no cert>" );
	}

	char	*data = NULL;
	long	datalen = 0;

	datalen = BIO_get_mem_data( bio, &data );
	StrBuf objS;
	if( datalen )
	    objS.Set(data, datalen);
	objS.Terminate();
	p4debug.printf( "%s%s", objS.Text(), sep );

	BIO_free_all( bio );
}

static void
PrintCertificateAltSubject(
	const char *msg,
	int depth,
	X509 *cert,
	const char *sep = "\n",
	BIO *bio = NULL)
{
	if( !cert )
	    return;

	bool show_msg = true;
	int msglen = strlen(msg) + 1;

	if( msg || (depth >= 0) )
	    BIO_printf( bio, ": " );

	GENERAL_NAMES *names = NULL;
	unsigned char *utf8 = NULL;
	BIO *mybio = NULL;

	if( !bio )
	    bio = mybio = BIO_new( BIO_s_mem() );

	do
	{
	    void *vnames = X509_get_ext_d2i( cert, NID_subject_alt_name, 0, 0 );
	    if( !vnames )
		break;

	    names = static_cast<GENERAL_NAMES *>(vnames);
	    int count = sk_GENERAL_NAME_num( names );

	    for( int i = 0; i < count; ++i )
	    {
		GENERAL_NAME* entry = sk_GENERAL_NAME_value( names, i );
		if( !entry )
		    continue;

		// should we also handle GEN_IPADD (IP address)?
		switch( entry->type )
		{
		case GEN_DNS: // DNS name
		    {
			int len1 = 0;
			int len2 = -1;

			len1 = ASN1_STRING_to_UTF8( &utf8, entry->d.dNSName );
			if( utf8 )
			    len2 = (int)strlen( (const char*)utf8 );

			// Skip this entry if the lengths don't match
			if( utf8 && len1 && len2 && (len1 == len2))
			{
			    BIO_printf( bio, "+ #%d/%c", depth, 'a'+i );

			    if( show_msg )
			    {
				BIO_printf( bio, " %s", msg );
				show_msg = false;
			    }
			    else
			    {
				BIO_printf( bio, "%*s", msglen, " " );
			    }
			    BIO_printf( bio, " : DNS:%s%s", utf8, sep );
			}

			if( utf8 )
			{
			    OPENSSL_free( utf8 );
			    utf8 = NULL;
			}
		    }
		    break;
		case GEN_IPADD:
		    {
			StrBuf buf;
			NetUtils::IpBytesToStr( entry->d.ip->data, entry->d.ip->length > 4, buf );
			BIO_printf( bio, "+ #%d/%c", depth, 'a'+i );

			if( show_msg )
			{
			    BIO_printf( bio, " %s", msg );
			    show_msg = false;
			}
			else
			{
			    BIO_printf( bio, "%*s", msglen, " " );
			}
			BIO_printf( bio, " : IP:%s%s", buf.Text(), sep );
		    }
		    break;
		}
	    }

	    sk_GENERAL_NAME_pop_free( names, GENERAL_NAME_free );
	} while (0);

	(void) BIO_flush(bio);
	char	*data = NULL;
	long	datalen = 0;

	datalen = BIO_get_mem_data( bio, &data );
	StrBuf objS;
	if( datalen )
	    objS.Set(data, datalen);
	objS.Terminate();
	p4debug.printf( "%s%s", objS.Text(), sep );

	if( mybio )
	    BIO_free_all( mybio );
}

static void
PrintCertificateIssuer( const char *msg, int depth, X509 *cert, const char *sep = "\n" )
{
	BIO *bio = BIO_new( BIO_s_mem() );
	BIO_printf( bio, "+" );
	if( depth >= 0 )
	    BIO_printf( bio, " #%d", depth );
	if( msg )
	    BIO_printf( bio, " %s", msg );
	if( msg || (depth >= 0) )
	    BIO_printf( bio, ": " );
	if( cert )
	{
	    BIO_printf( bio, "Issuer:  " );
	    X509_NAME_print_ex( bio, X509_get_issuer_name(cert), 0,
	                        XN_FLAG_ONELINE );
	}
	else
	{
	    BIO_printf( bio, "<no cert>" );
	}

	char	*data = NULL;
	long	datalen = 0;

	datalen = BIO_get_mem_data( bio, &data );
	StrBuf objS;
	if( datalen )
	    objS.Set(data, datalen);
	objS.Terminate();
	p4debug.printf( "%s%s", objS.Text(), sep );

	BIO_free_all( bio );
}

static void
PrintCertificateSubject( int depth, X509 *cert, BIO *bio, const char *sep )
{
	BIO_printf( bio , "depth=%d ", depth );
	if( cert != NULL )
	{
	    X509_NAME_print_ex( bio, X509_get_subject_name(cert), 0,
	                        XN_FLAG_ONELINE );
	    BIO_puts( bio, sep );
	}
	else
	    BIO_printf( bio, " <no cert>%s", sep );
}

static void
PrintCertificateAltSubject( int depth, X509 *cert, BIO *bio, const char *sep )
{
	BIO_printf( bio , "depth=%d ", depth );
	if( cert == NULL )
	{
	    BIO_printf( bio, " <no cert>%s", sep );
	    return;
	}
	PrintCertificateAltSubject( "", depth, cert, sep, NULL );
}

static void
PrintCertificateError( X509 *cert, int err, X509_STORE_CTX *ctx, BIO *bio,
                       const char *sep )
{
	switch (err)
	{
	case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
	case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
	    BIO_puts( bio, "issuer= " );
	    X509_NAME_print_ex( bio, X509_get_issuer_name( cert ),
	                        0, XN_FLAG_ONELINE );
	    BIO_puts( bio, sep );
	    break;
	case X509_V_ERR_CERT_NOT_YET_VALID:
	case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
	    BIO_printf( bio, "notBefore=" );
# if OPENSSL_VERSION_NUMBER < 0x10100000L
	    ASN1_TIME_print( bio, X509_get_notBefore( cert ) );
# else
	    ASN1_TIME_print( bio, X509_get0_notBefore( cert ) );
# endif
	    BIO_puts( bio, sep );
	    break;
	case X509_V_ERR_CERT_HAS_EXPIRED:
	case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
	    BIO_printf( bio, "notAfter=" );
# if OPENSSL_VERSION_NUMBER < 0x10100000L
	    ASN1_TIME_print( bio, X509_get_notAfter( cert ) );
# else
	    ASN1_TIME_print( bio, X509_get0_notAfter( cert ) );
# endif
	    BIO_puts( bio, sep );
	    break;
	case X509_V_ERR_NO_EXPLICIT_POLICY:
	    PrintPolicies( ctx, bio, sep );
	    break;
	default:
	    break;
	}
}

static int ossl_verify_cb_idx;

# if OPENSSL_VERSION_NUMBER < 0x10100000L

static int ossl_verify_cb_idx_once = 0;

int GetSSLVerifyCbIdx()
{
	if( !ossl_verify_cb_idx_once )
	{
	    // Potential race condition here, but this "should" still be
	    // gracefully handled. Ideally noone's using OpenSSL 1.0.2...
	    int newIdx = X509_STORE_CTX_get_ex_new_index( 0, 0, 0, 0, 0 );
	    if( !ossl_verify_cb_idx_once++ )
	        ossl_verify_cb_idx_once = newIdx;
	}

	return ossl_verify_cb_idx;
}

# else // OpenSSL >=1.1

static CRYPTO_ONCE ossl_verify_cb_idx_once = CRYPTO_ONCE_STATIC_INIT;

void SetSSLVerifyCbIdx()
{
	ossl_verify_cb_idx = X509_STORE_CTX_get_ex_new_index( 0, 0, 0, 0, 0 );
}

int GetSSLVerifyCbIdx()
{
	CRYPTO_THREAD_run_once(&ossl_verify_cb_idx_once, SetSSLVerifyCbIdx);
	return ossl_verify_cb_idx;
}
# endif // OpenSSL >=1.1

int
verify_callback(int ok, X509_STORE_CTX *ctx)
{
	NetSslCredentials *credentials = (NetSslCredentials *)
	    X509_STORE_CTX_get_ex_data( ctx, GetSSLVerifyCbIdx() );
	if( !credentials )
	    return ok;

	X509 *err_cert = X509_STORE_CTX_get_current_cert( ctx );
	int err = X509_STORE_CTX_get_error( ctx );
	int depth = X509_STORE_CTX_get_error_depth( ctx );

	credentials->SetCertValid( depth, err, err_cert );
	if( !SSLDEBUG_CERT )
	    return ok;
	
	BIO *bio = BIO_new( BIO_s_mem() );
	PrintCertificateSubject( depth, err_cert, bio, "\n" );

	if( !ok )
	    BIO_printf( bio, "verify error:num=%d:%s\n", err,
	                X509_verify_cert_error_string( err ) );
	
	PrintCertificateError( err_cert, err, ctx, bio, "\n" );

	if( err == X509_V_OK && ok == 2 )
	    PrintPolicies( ctx, bio, "\n" );

	if( ok )
	    BIO_printf( bio, "verify return:%d\n", ok );

	char	*data = NULL;
	long	datalen = 0;

	datalen = BIO_get_mem_data( bio, &data );
	StrBuf objS;
	if( datalen )
	    objS.Set( data, datalen );
	objS.Terminate();
	p4debug.printf( objS.Text() );

	BIO_free_all( bio );

	return ok;
}

# if OPENSSL_VERSION_NUMBER < 0x30000000
static void
Callback( int code, int arg, void *cb_arg )
{
	if ( !SSLDEBUG_FUNCTION )
	    return;

	if( code == 0 )
	    p4debug.printf(".");
	if( code == 1 )
	    p4debug.printf("+");
	if( code == 2 )
	    p4debug.printf("*");
	if( code == 3 )
	    p4debug.printf("\n");
}
# endif

////////////////////////////////////////////////////////////////////////////
//  NetSslCredentials - Methods                                           //
////////////////////////////////////////////////////////////////////////////

NetSslCredentials::NetSslCredentials(bool isTest)
  : privateKey( NULL ),
    certificate( NULL ),
    fingerprint(),
    certC( SSL_X509_C ),
    certCN(),
    certST( SSL_X509_ST ),
    certL( SSL_X509_L ),
    certO( SSL_X509_O )
{
	ownCert = true;
	ownKey = true;
	certEX = SSL_X509_NOTAFTER;
	certSV = SSL_X509_NOTBEFORE;
	certUNITS = SSL_X509_DAY;
	chain = new VarArray;
	verify = new IntArray( 5, -1 );

	if ( !isTest )
	{
	    HostEnv h;
	    Enviro   enviro;
	    char *sslDirStr = NULL;

	    h.GetHost( certCN );

	    const StrPtr *cachedServerName = enviro.GetCachedServerName();
	    if ( cachedServerName )
	    {
		enviro.BeServer( cachedServerName );
	    }

	    sslDirStr = enviro.Get( "P4SSLDIR" );
	    if ( sslDirStr && sslDirStr[0] != '\0' )
	    {
		sslDir = sslDirStr;
	    }
	}
	else
	{
	    sslDir.Set("/tmp/4kssldir");
	    certCN.Set("TestHost");
	}
}

#ifdef USE_CREDS_COPY_CTOR
/*
 * See the comment in netsslcredentials.h for why this is removed.
 * - If we want to change this to use deep copy then we could try
 *   X509_dup(cert), X509_check_purpose(copied_cert, -1, 0)
 *   but this would require investigation.
 */
NetSslCredentials::NetSslCredentials( NetSslCredentials &rhs)
  : privateKey (rhs.privateKey),
    certificate(rhs.certificate),
    fingerprint(rhs.fingerprint),
    certC(rhs.certC),
    certCN(rhs.certCN),
    certST(rhs.certST),
    certL(rhs.certL),
    certO(rhs.certO),
    certEX(rhs.certEX),
    ownKey(false),
    ownCert(false),
    sslDir(rhs.sslDir)
{
	chain = new VarArray;
	for( int i = 0; i < rhs.chain->Count(); i++ )
	    chain->Put( rhs.chain->Get( i ) );
	verify = new IntArray( 5, -1 );
}
#endif // USE_CREDS_COPY_CTOR

NetSslCredentials::~NetSslCredentials()
{
	ReleaseCredentials( true, true );

	delete chain;
	delete verify;
}

void
NetSslCredentials::ClearChain( bool freeChain )
{
	if( ownCert && freeChain )
	{
	    for( int i = 0; i < chain->Count(); i++ )
		X509_free( (X509 *)chain->Get( i ) );
	}

	if( freeChain )
	    chain->Clear();
}

void
NetSslCredentials::ReleaseCredentials( bool freeCert, bool freeKey )
{
	ClearChain( true );

	if ( freeCert && ownCert && certificate )
	{
	    X509_free( certificate );
	    certificate = NULL;
	}
	ownCert = false;

	if ( freeKey && ownKey && privateKey )
	{
	    EVP_PKEY_free( privateKey );
	    privateKey = NULL;
	}
	ownKey = false;
}

#ifdef USE_CREDS_ASSIGN_OPER
/*
 * See the comment in netsslcredentials.h for why this is removed.
 * - If we want to change this to use deep copy then we could try
 *   X509_dup(cert), X509_check_purpose(copied_cert, -1, 0)
 *   but this would require investigation.
 */
NetSslCredentials &
NetSslCredentials::operator =( NetSslCredentials &rhs )
{
	ReleaseCredentials( true, true );

	privateKey = rhs.privateKey;
	certificate = rhs.certificate;

	for( int i = 0; i < chain->Count(); i++ )
	    X509_free( (X509 *)chain->Get( i ) );
	chain->Clear();

	for( int i = 0; i < rhs.chain->Count(); i++ )
	    chain->Put( rhs.chain->Get( i ) );

	fingerprint = rhs.fingerprint;
	certC = rhs.certC;
	certCN = rhs.certCN;
	certST = rhs.certST;
	certL = rhs.certL;
	certO = rhs.certO;
	certEX = rhs.certEX;
	ownKey = false;
	ownCert = false;
	sslDir = rhs.sslDir;
	return *this;
}
#endif // USE_CREDS_ASSIGN_OPER

void
NetSslCredentials::HaveCredentials( Error *e )
{
	if( !privateKey || !certificate || (fingerprint.Length() == 0) )
	    e->Set(MsgRpc::SslNoCredentials);
}

void
NetSslCredentials::ReadCredentials( Error *e )
{
	FILE *fp = NULL;
	X509 *chainCert = 0;
	PathSys *keyFile = PathSys::Create();
	PathSys *certFile = PathSys::Create();

	GetCredentialFilepaths( keyFile, certFile, e);

	// Validate that P4SSLDIR exists and is a directory
	ValidateSslDir( e );
	P4CHECKERROR( e, "NetSslCredentials::ReadCredentials ValidateSslDir", fail );
	ValidateCredentialFiles( e );
	P4CHECKERROR( e, "NetSslCredentials::ReadCredentials ValidateCredentialFiles", fail );

	// read in private key
	fp = fopen( keyFile->Text(), "r" );
	if( fp == NULL ) {
	    e->Net( "fopen", strerror(errno) );
	    goto failSetError;
	}

	if ( privateKey && ownKey )
	{
	    EVP_PKEY_free( privateKey );
	}
	privateKey = PEM_read_PrivateKey(fp, NULL, NULL, NULL );
	ownKey = true;
	SSLNULLHANDLER( privateKey, e, "NetSslCredentials::ReadCredentials PEM_read_PrivateKey", failSetError );

	// verify that RSA key
# if OPENSSL_VERSION_NUMBER < 0x10100000L
	if (privateKey->type != EVP_PKEY_RSA)
# else // OpenSSL >=1.1
	if( !( EVP_PKEY_base_id( privateKey ) == EVP_PKEY_RSA ||
	       EVP_PKEY_base_id( privateKey ) == EVP_PKEY_EC ||
	       EVP_PKEY_base_id( privateKey ) == EVP_PKEY_ED25519 ||
	       EVP_PKEY_base_id( privateKey ) == EVP_PKEY_ED448 ) )
# endif
	{
	    e->Set( MsgRpc::SslKeyNotRSA );
	    goto fail;
	}
	fclose( fp );
	fp = NULL;

	// read in certificate
	if( SSLDEBUG_CERT && certificate )
	    p4debug.printf( "NetSslCredentials::ReadCredentials cert='%s'\n", certFile->Text() );
	fp  = fopen( certFile->Text(), "r" );
	if( fp == NULL ) {
	    e->Net( "fopen", strerror(errno) );
	    goto failSetError;
	}

	if ( ownCert && certificate )
	{
	    X509_free( certificate );
	}

	// we assume that the first cert is the subject (ie, server) cert
	certificate = PEM_read_X509( fp, NULL, 0, NULL );
	ownCert = true;
	SSLNULLHANDLER( certificate, e, "NetSslCredentials::ReadCredentials PEM_read_X509", failSetError );

	ValidateCertDateRange( certificate, e );
	P4CHECKERROR( e, "NetSslCredentials::ReadCredentials ValidateCertDateRange", fail );
	// Note: the cert chain must not contain the server cert

	/*
	 * Reset the chain before adding new certs.
	 *
	 * If we're restarting, then 'certificates.txt'
	 * or 'privatekey.txt' might have changed since
	 * we originally loaded the chain and we don't
	 * want to leave the old certificates in the chain.
	 *
	 * If the certificates haven't changed then we still
	 * clear the chain because otherwise the chain will
	 * contain another copy of our chain certificates.
	 * Duplicate certs in the chain is not a good idea,
	 * violates the requirement that each cert is signed
	 * by the next certificate, and can cause OpenSSL
	 * handshakes to fail (typically with a null parameter
	 * error).
	 */

	for( int i = 0; i < chain->Count(); i++ )
	    X509_free( (X509 *)chain->Get( i ) );
	chain->Clear();

	while( (chainCert = PEM_read_X509( fp, NULL, 0, NULL ) ) )
	{
	    ValidateCertDateRange( chainCert, e );
	    P4CHECKERROR( e, "NetSslCredentials::ReadCredentials ValidateCertDateRange (chain)", fail );

	    chain->Put( chainCert );
	}
	e->Clear();

	GetFingerprintFromCert( e );
	if( e->Test() ) {
	    goto fail;
	}

	fclose( fp );
	delete keyFile;
	delete certFile;
	return;

failSetError:
	e->Set( MsgRpc::SslBadKeyFile );
fail:
	if( fp ) {
	    fclose( fp );
	}
	ReleaseCredentials( true, true );
	delete keyFile;
	delete certFile;
	if( chainCert )
	    X509_free( chainCert );
	return;
}

void
NetSslCredentials::GenerateCredentials( Error *e  )
{
	PathSys *keyFile = PathSys::Create();
	PathSys *certFile = PathSys::Create();

	GetCredentialFilepaths( keyFile, certFile, e);
	P4CHECKERROR( e, "NetSslCredentials::GenerateCredentials GetCredentialsFiles", fail );

	ValidateSslDir( e );
	P4CHECKERROR( e, "NetSslCredentials::GenerateCredentials ValidateSslDir", fail );

	// Validate that serverKey.txt and serverCert.txt do not exist in P4SSLDIR
	if( FileSys::FileExists( keyFile->Text() ) || FileSys::FileExists( certFile->Text() ) )
	{
	    e->Set( MsgRpc::SslDirHasCreds );
	    goto fail;
	}

	ParseConfig( e );
	P4CHECKERROR( e, "NetSslCredentials::GenerateCredentials ParseConfig", fail );

	MakeSslCredentials( e );
	P4CHECKERROR( e, "NetSslCredentials::GenerateCredentials MakeSslCredentials", fail );

	WriteCredentials(  keyFile, certFile, e );
	P4CHECKERROR( e, "NetSslCredentials::GenerateCredentials WriteCredentials", fail );

	ownCert = true;
	ownKey = true;

fail:
	ReleaseCredentials( true, true );
	delete keyFile;
	delete certFile;
}

void
NetSslCredentials::ValidateSslDir( Error * e)
{

	if ( sslDir.Length() == 0 )
	{
	    e->Set( MsgRpc::SslBadDir );
	    return;
	}

	// Validate that P4SSLDIR exists and is a directory
	FileSys *f = FileSys::Create( FST_BINARY );
	f->Set( sslDir );

	if( (f->Stat() & (FSF_EXISTS | FSF_DIRECTORY)) !=
		(FSF_EXISTS | FSF_DIRECTORY) )
	{
	    e->Set( MsgRpc::SslBadDir );
	    goto fail;
	}

	// Validate that P4SSLDIR permission is 700 or 500
	if( (! f->HasOnlyPerm( FPM_RWXO )) && (! f->HasOnlyPerm( FPM_RXO )) )
	{
	    e->Set( MsgRpc::SslBadFsSecurity );
	    goto fail;
	}

	// Validate that dir is owned by same owner as p4d
	CompareDirUid( e );
	P4CHECKERROR( e, "NetSslCredentials::ValidateSslDir CompareDirUid", fail );

fail:
	delete f;
	return;
}


void
NetSslCredentials::ValidateCredentialFiles( Error *e )
{
	FileSys *f = NULL;
	PathSys *keyFile = PathSys::Create();
	PathSys *certFile = PathSys::Create();

	GetCredentialFilepaths( keyFile, certFile, e);
	if( e->Test() )
	    goto fail;

	// Validate that serverKey.txt and serverCert.txt exist in P4SSLDIR
	if( !( FileSys::FileExists( keyFile->Text() ) && FileSys::FileExists( certFile->Text() )))
	{
	    e->Set( MsgRpc::SslBadKeyFile );
	    goto fail;
	}

	// Validate that files are owned by same owner as p4d
	CompareFileUids( e );
	P4CHECKERROR( e, "NetSslCredentials::ValidateCredentialFiles CompareFileUids", fail );

	// Validate that serverKey.txt and serverCert.txt permissions are 600 or 400
	f = FileSys::Create( FST_BINARY );
	f->Set( keyFile->Text() );
	if( (! f->HasOnlyPerm( FPM_RWO )) && (! f->HasOnlyPerm( FPM_ROO )) )
	{
	    e->Set( MsgRpc::SslBadFsSecurity );
	    goto fail;
	}
	f->Set( certFile->Text() );
	if( ! f->HasOnlyPerm( FPM_RWO ) && ! f->HasOnlyPerm( FPM_ROO ) )
	{
	    e->Set( MsgRpc::SslBadFsSecurity );
	    goto fail;
	}

fail:
	if( f )
	    delete f;
	delete keyFile;
	delete certFile;
	return;
}

void
NetSslCredentials::ValidateCertDateRange( X509 *cert, Error *e )
{
	time_t *ptime = NULL;
	int i;

# if OPENSSL_VERSION_NUMBER < 0x10100000L
	i = X509_cmp_time( X509_get_notBefore( cert ), ptime );
# else
	i = X509_cmp_time( X509_get0_notBefore( cert ), ptime );
# endif
	if( i >= 0 )
	    goto fail;

# if OPENSSL_VERSION_NUMBER < 0x10100000L
	i = X509_cmp_time( X509_get_notAfter( cert ), ptime );
# else
	i = X509_cmp_time( X509_get0_notAfter( cert ), ptime );
# endif
	if( i <= 0 )
	    goto fail;

	return;

fail:
	e->Set( MsgRpc::SslCertBadDates );
}

void
NetSslCredentials::GetExpiration( StrBuf &buf )
{
	Error e;
	if( !certificate ) {
	    buf.Clear();
	    return;
	}

	BUF_MEM *bufMemPtr = NULL;
	int retVal = 0;

	BIO *bio = BIO_new( BIO_s_mem() );
	SSLNULLHANDLER( bio, &e, "NetSslCredentials::GetExpiration BIO_new", fail );

# if OPENSSL_VERSION_NUMBER < 0x10100000L
	retVal = ASN1_TIME_print(bio,X509_get_notAfter(certificate));
# else
	retVal = ASN1_TIME_print(bio,X509_get0_notAfter(certificate));
# endif
	SSLHANDLEFAIL( retVal, &e, "NetSslCredentials::GetExpiration BIO_get_mem_ptr",
		MsgRpc::SslFailGetExpire,
		failCleanBIO );

	retVal = BIO_get_mem_ptr( bio, &bufMemPtr );
	SSLHANDLEFAIL( retVal, &e, "NetSslCredentials::GetExpiration BIO_get_mem_ptr",
	    MsgRpc::SslFailGetExpire,
	    failCleanBIO );


	buf.Set( bufMemPtr->data, bufMemPtr->length );
	buf.Terminate();
	BIO_free_all( bio );
	return;

failCleanBIO:
	BIO_free_all( bio );

fail:
	buf.Clear();
	return;
}


void
NetSslCredentials::ParseConfig( Error *e )
{
	StrBuf line, var, value;
	StrRef configFile( SSL_CONFIGFILE );
	PathSys *p = PathSys::Create();
	FileSys *f = FileSys::Create( FileSysType( FST_TEXT|FST_L_CRLF ) );

	// Create full pathname to certificate config file
	p->SetLocal( sslDir, configFile );
	f->Set( *p );
	f->Open( FOM_READ, e );
	delete p;

	// If file doesn't exist then keep defaults and just return
	if( e->Test() )
	{
	    DEBUGPRINT( SSLDEBUG_FUNCTION, "NetSslCredentials::ParseConfig - config.txt file not found in P4SSLDIR." );
	    e->Clear();
	    delete f;
	    return;
	}

	while( f->ReadLine( &line, e ) )
	{
	    line.TruncateBlanks();

	    char *equals = strchr( line.Text(), '=' );
	    if( !equals ) continue;

	    var.Set( line.Text(), equals - line.Text() );
	    var.TrimBlanks();

	    if(  var.Text()[0] == '#' )  continue;

	    value.Set( equals + 1 );
	    value.TrimBlanks();

	    DEBUGPRINTF( SSLDEBUG_FUNCTION,
		    "NetSslCredentials::ParseConfig name=%s, value=%s", var.Text(), value.Text() );

	    if ( var == "C")
	    	certC = value;
	    else if ( var == "CN")
	    	certCN = value;
	    else if ( var == "ST")
	    	certST = value;
	    else if ( var == "L")
	    	certL = value;
	    else if ( var == "O")
	    	certO = value;
	    else if ( var == "EX")
	    {
		int expire = StrBuf::Atoi(value.Text());
		if( expire > 0 )
		{
		    certEX = expire;
		}
		else
		{
		    e->Set( MsgRpc::SslCfgExpire ) << value;
	            f->Close( e );
	            delete f;
		    return;
		}
	    }
	    else if ( var == "SV")
	    {
		int start = StrBuf::Atoi(value.Text());
		certSV = start;
	    }
	    else if ( var == "UNITS")
		if( value == "secs" )
		{
		    certUNITS = 1;
		}
		else if ( value == "mins")
		{
		    certUNITS = 60;
		}
		else if ( value == "hours")
		{
		    certUNITS = 3600;
		} else if ( value == "days")
		    ; // do nothing
		else
		{
		    e->Set( MsgRpc::SslCfgUnits ) << value;
	            f->Close( e );
	            delete f;
		    return;
		}
	    else
	    {
		DEBUGPRINTF( SSLDEBUG_ERROR,
			"Certificate configuration file option \"%s\" unknown.", var.Text() );
	    }
	}
	if( (SSL_X509_MAX_SECONDS / certUNITS) < certEX ) {
	    e->Set( MsgRpc::SslCfgExpire ) << value;
	    f->Close( e );
	    delete f;
	    return;
	}

	f->Close( e );
	delete f;
}

void
NetSslCredentials::WriteCredentials(  PathSys *keyFile, PathSys *certFile, Error * e )
{
	FILE *fp = NULL;
	int retVal = 0;
	FileSys *fsKey = FileSys::Create( FST_TEXT );
	FileSys *fsCert = FileSys::Create( FST_TEXT );

	// write out in private key
	fp = fopen( keyFile->Text(), "w" );
	if( fp == NULL ) {
	    e->Net( "fopen", strerror(errno) );
	    goto fail;
	}
	retVal = PEM_write_PrivateKey(fp, privateKey, NULL, NULL, 0, 0, NULL );
	SSLHANDLEFAIL( retVal, e, "NetSslCredentials::WriteCredentials PEM_write_PrivateKey", MsgRpc::SslCertGen, fail );
	fclose( fp );
	fsKey->Set(*keyFile);
	fsKey->Chmod(FPM_RWO, e);

	// read in certificate
	fp  = fopen( certFile->Text(), "w" );
	if( fp == NULL )
	{
	    e->Net( "fopen", strerror(errno) );
	    e->Set( MsgRpc::SslCertGen );
	    goto fail;
	}
	retVal = PEM_write_X509(fp, certificate );
	SSLHANDLEFAIL( retVal, e, "NetSslCredentials::WriteCredentials PEM_write_X509", MsgRpc::SslCertGen, fail );
	fclose( fp );
	fp = NULL;
	fsCert->Set(*certFile);
	fsCert->Chmod(FPM_RWO, e);

fail:
	if( fp )
	    fclose( fp );
	delete fsKey;
	delete fsCert;
	return;
}
void
NetSslCredentials::CompareFileUids( Error *e )
{
# ifdef HAVE_GETUID
	uid_t keyOwner = 0;
	uid_t certOwner = 0;
	uid_t dirOwner = 0;
	uid_t currUsr = geteuid();
	PathSys *keyFile = PathSys::Create();
	PathSys *certFile = PathSys::Create();
	FileSys *f = FileSys::Create( FST_BINARY );

	GetCredentialFilepaths( keyFile, certFile, e);
	P4CHECKERROR( e, "NetSslCredentials::CompareUids GetCredentialsFiles", fail );

	f->Set( keyFile->Text() );
	keyOwner = f->GetOwner();
	if( currUsr != keyOwner )
	{
	    e->Set( MsgRpc::SslCredsBadOwner );
	    goto fail;
	}

	f->Set( certFile->Text() );
	certOwner = f->GetOwner();
	if( currUsr != certOwner )
	{
	    e->Set( MsgRpc::SslCredsBadOwner );
	    goto fail;
	}

	f->Set( sslDir );
	dirOwner = f->GetOwner();
	if( currUsr != dirOwner )
	    e->Set( MsgRpc::SslCredsBadOwner );

fail:
	delete f;
	delete keyFile;
	delete certFile;
	return;

# endif // HAVE_GETUID
}

void
NetSslCredentials::CompareDirUid( Error *e )
{
# ifdef HAVE_GETUID

	uid_t dirOwner = 0;
	uid_t currUsr = geteuid();
	FileSys *f = FileSys::Create( FST_BINARY );

	f->Set( sslDir );
	dirOwner = f->GetOwner();
	if( currUsr != dirOwner )
	    e->Set( MsgRpc::SslCredsBadOwner );
	delete f;

# endif // HAVE_GETUID
}

void
NetSslCredentials::GetCredentialFilepaths( PathSys *keyFile, PathSys *certFile, Error * e )
{
	StrRef certFilename( SSL_CERTFILE );
	StrRef keyFilename( SSL_KEYFILE );

	keyFile->SetLocal(sslDir, keyFilename );
	certFile->SetLocal(sslDir, certFilename );
}

void
NetSslCredentials::MakeSslCredentials( Error *e )
{
	if ( privateKey && certificate )
	{
	    return;
	}

	int       retval;
# if OPENSSL_VERSION_NUMBER < 0x30000000L
	RSA       *rsa = NULL;
# else
	EVP_PKEY_CTX *ctx = NULL;
# endif
	X509_NAME *name = NULL;
# if OPENSSL_VERSION_NUMBER >= 0x10100000L && \
     OPENSSL_VERSION_NUMBER < 0x30000000L
	BIGNUM    *bne = NULL;
	BN_GENCB  *callback = NULL;
# endif

	if( privateKey && ownKey )
	    EVP_PKEY_free( privateKey );
	if( ( privateKey = EVP_PKEY_new()) == NULL )
	{
	    e->Net( "EVP_PKEY_new", "failed" );
	    e->Set( MsgRpc::SslCertGen );
	    goto fail;
	}
	ownKey = true;

	if ( certificate && ownCert )
	    X509_free( certificate );
	certificate = X509_new();
	ownCert = true;
	SSLHANDLEFAIL( certificate, e, "X509_new", MsgRpc::SslCertGen, fail );

# if OPENSSL_VERSION_NUMBER < 0x10100000L
	rsa = RSA_generate_key( SSL_X509_NUMBITS, RSA_F4, Callback, NULL );
	SSLHANDLEFAIL( rsa, e, "RSA_generate_key", MsgRpc::SslCertGen, fail );
# elif OPENSSL_VERSION_NUMBER < 0x30000000L
	rsa = RSA_new();
	bne = BN_new();
	retval = BN_set_word(bne, RSA_F4);
	SSLHANDLEFAIL( retval, e, "BN_set_word", MsgRpc::SslCertGen, fail );
	callback = BN_GENCB_new();
	SSLHANDLEFAIL( callback, e, "BN_GENCB_new", MsgRpc::SslCertGen, fail );
	BN_GENCB_set_old( callback, Callback, NULL );
	retval = RSA_generate_key_ex( rsa, SSL_X509_NUMBITS, bne, callback);
	BN_free( bne );
	BN_GENCB_free( callback );
	bne = NULL;
	SSLHANDLEFAIL( retval, e, "RSA_generate_key_ex", MsgRpc::SslCertGen, fail );
# else
	ctx = EVP_PKEY_CTX_new_id( EVP_PKEY_RSA, NULL );
	SSLHANDLEFAIL( ctx, e, "EVP_PKEY_CTX_new_id", MsgRpc::SslCertGen, fail );
	retval = EVP_PKEY_keygen_init( ctx );
	SSLHANDLEFAIL( retval, e, "EVP_PKEY_keygen_init", MsgRpc::SslCertGen, fail );
	retval = EVP_PKEY_CTX_set_rsa_keygen_bits( ctx, SSL_X509_NUMBITS );
	SSLHANDLEFAIL( retval, e, "EVP_PKEY_CTX_set_rsa_keygen_bits", MsgRpc::SslCertGen, fail );
	retval = EVP_PKEY_keygen( ctx, &privateKey );
	EVP_PKEY_CTX_free( ctx );
	SSLHANDLEFAIL( retval, e, "EVP_PKEY_keygen", MsgRpc::SslCertGen, fail );
# endif

# if OPENSSL_VERSION_NUMBER < 0x30000000L
	retval = EVP_PKEY_assign_RSA( privateKey, rsa );
	SSLHANDLEFAIL( retval, e, "EVP_PKEY_assign_RSA", MsgRpc::SslCertGen, fail );
# endif

	X509_set_version( certificate, SSL_X509_VERSION );
	ASN1_INTEGER_set( X509_get_serialNumber( certificate ), SSL_X509_SERIALNUM );
# if OPENSSL_VERSION_NUMBER < 0x10100000L
	X509_gmtime_adj( X509_get_notBefore( certificate ), certSV * SSL_X509_DAY );
	X509_gmtime_adj( X509_get_notAfter( certificate ), certEX * certUNITS );
# else
	X509_gmtime_adj( X509_getm_notBefore( certificate ), certSV * SSL_X509_DAY );
	X509_gmtime_adj( X509_getm_notAfter( certificate ), certEX * certUNITS );
# endif
	X509_set_pubkey( certificate, privateKey );

	name = X509_get_subject_name( certificate );
	retval =
	    X509_NAME_add_entry_by_txt( name, "C", MBSTRING_ASC, (unsigned char*)certC.Text(),
					-1,
					-1,
					0 );

	SSLHANDLEFAIL( retval, e, "X509_NAME_add_entry_by_txt for \"C\"",
		       MsgRpc::SslCertGen,
		       fail );

	retval = X509_NAME_add_entry_by_txt( name, "ST", MBSTRING_ASC, (unsigned char*)certST.Text(),
					     -1, -1,
					     0 );
	SSLHANDLEFAIL( retval, e, "X509_NAME_add_entry_by_txt for \"ST\"",
		       MsgRpc::SslCertGen,
		       fail );
	retval =
	    X509_NAME_add_entry_by_txt( name, "L", MBSTRING_ASC, (unsigned char*)certL.Text(),
					-1,
					-1,
					0 );

	SSLHANDLEFAIL( retval, e, "X509_NAME_add_entry_by_txt for \"L\"",
		       MsgRpc::SslCertGen,
		       fail );
	retval =
	    X509_NAME_add_entry_by_txt( name, "O", MBSTRING_ASC, (unsigned char*)certO.Text(),
					-1,
					-1,
					0 );

	SSLHANDLEFAIL( retval, e, "X509_NAME_add_entry_by_txt for \"O\"",
		       MsgRpc::SslCertGen,
		       fail );

	DEBUGPRINTF( SSLDEBUG_FUNCTION, "Setting CN to Hostname: %s", certCN.Text())
	    retval =
	    X509_NAME_add_entry_by_txt( name, "CN", MBSTRING_ASC, (unsigned char*)certCN.Text(),
					-1, -1,
					0 );
	SSLHANDLEFAIL( retval, e, "X509_NAME_add_entry_by_txt for \"CN\": ",
		       MsgRpc::SslCertGen,
		       fail );

	/* Its self signed so set the issuer name to be the same as the
	 * subject.
	 */
	X509_set_issuer_name( certificate, name );

	if( !X509_sign( certificate, privateKey, EVP_sha256() ) )
	{
	    e->Net( "EVP_PKEY_new", "failed" );
	    e->Set( MsgRpc::SslCertGen );
	    goto fail;
	}

	return;

fail:

	if (certificate)
	{
	    X509_free( certificate );
	    certificate = NULL;
	}

	if (privateKey)
	{
	    EVP_PKEY_free( privateKey );
	    privateKey = NULL;
	}

# if OPENSSL_VERSION_NUMBER >= 0x10100000L && \
     OPENSSL_VERSION_NUMBER < 0x30000000L
	if( bne )
	    BN_free( bne );
	if( callback )
	    BN_GENCB_free( callback );
# endif
}

void
NetSslCredentials::GetFingerprintFromCert( Error *e )
{
	int retval = 1;
	int i = 0;
	unsigned int n = 0;
	BIO *bio = NULL;
	BUF_MEM *bufMemPtr = NULL;
	unsigned char *asn1pubKey = NULL;
	unsigned char md[EVP_MAX_MD_SIZE];
	const EVP_MD *fdig = EVP_sha1();
	unsigned char *ptr = NULL;
	unsigned int len;

	if( !certificate )
	{
	    e->Set( MsgRpc::SslNoCredentials );
	    return;
	}

	bio = BIO_new(BIO_s_mem());
	SSLNULLHANDLER( bio, e, "GetFingerprintFromCert BIO_new", fail );

	len = i2d_X509_PUBKEY( X509_get_X509_PUBKEY( certificate ), NULL );
	if( (len == 0) || (len > 20480) ) // 20KB should be enough for anyone
	{
	    SSLHANDLEFAIL( false, e, "GetFingerprintFromCert cert zero or too big",
		MsgRpc::SslGetPubKey,
		failCleanBIO );
	}

	ptr = asn1pubKey = new unsigned char[len];
	if( !ptr )
	{
	    SSLHANDLEFAIL( false, e, "GetFingerprintFromCert new asn1pubKey",
		MsgRpc::SslGetPubKey,
		failCleanBIO );
	}

	i2d_X509_PUBKEY( X509_get_X509_PUBKEY( certificate ), &ptr );
	if( ptr - asn1pubKey != len )
	{
	    SSLHANDLEFAIL( false, e, "GetFingerprintFromCert OVERRUN",
		MsgRpc::SslGetPubKey,
		failCleanBIO );
	}
	EVP_Digest( asn1pubKey, len, md, &n, fdig, NULL );

	DEBUGPRINTF( SSLDEBUG_FUNCTION, "pubkey len is: %d", len );
	DEBUGPRINTF( SSLDEBUG_FUNCTION, "digest len is: %u", n );

	n--;  // loop through all but last one
	for (i=0; i<(int)n; i++)
	{
	    BIO_printf(bio,"%02X:",md[i]);
	}
	// last one do not add a colon
	BIO_printf(bio,"%02X",md[n]);

	retval = BIO_get_mem_ptr( bio, &bufMemPtr );
	SSLHANDLEFAIL( retval, e, "GetFingerprintFromCert BIO_get_mem_ptr",
		MsgRpc::SslGetPubKey,
		failCleanBIO );

	fingerprint.Set( bufMemPtr->data, bufMemPtr->length );
	fingerprint.Terminate();
	DEBUGPRINTF( SSLDEBUG_FUNCTION, "GetFingerprintFromCert Fingerprint is: %s", fingerprint.Text() );


failCleanBIO:
	BIO_free_all(bio);
	if( asn1pubKey )
	    delete []asn1pubKey;
fail:
	/* nothing to clean up */
	return;
}

void
NetSslCredentials::SetCertValid( int depth, int err, X509 *cert )
{
	if( !cert )
	    return;

	// Stash the error value
	(*verify)[depth] = err;

	// Populate the chain
	while( depth > chain->Count() )
	    chain->Put( NULL );

	if( depth != 0 )
	    chain->Replace( depth - 1, cert) ;
}

X509 *
NetSslCredentials::GetCertificate() const
{
	return certificate;
}

X509 *
NetSslCredentials::GetChain( int i ) const
{
	return (X509 *)chain->Get( i );
}

const StrPtr *
NetSslCredentials::GetFingerprint() const
{
	return &fingerprint;
}

EVP_PKEY *
NetSslCredentials::GetPrivateKey() const
{
	return privateKey;
}

void 
NetSslCredentials::SetCertificate( X509 *cert, stack_st_X509 *certChain,
	X509_STORE *store, Error *e )
{
	if( !cert )
	{
	    e->Set( MsgRpc::SslNoCredentials );
	    return;
	}

	verify->Reset();
	if( ownCert )
	{
	    for( int i = 0; i < chain->Count(); i++ )
		X509_free( (X509 *)chain->Get( i ) );
	}

	for( int i = 0; i < chain->Count(); i++ )
	    X509_free( (X509 *)chain->Get( i ) );
	chain->Clear();
	ReleaseCredentials( true, true );

	this->certificate = cert;
	this->ownCert = false;
	ValidateCertDateRange( cert, e );
	if( e->Test() )
	{
	    this->certificate = NULL;
	    return;
	}
	GetFingerprintFromCert( e );
	if( e->Test() )
	{
	    this->certificate = NULL;
	    this->fingerprint.Clear();
	    return;
	}

	if( !store )
	    return;

	X509_STORE_CTX *csc = X509_STORE_CTX_new();
	if( X509_STORE_CTX_init( csc, store, cert, certChain ) )
	{
	    X509_STORE_CTX_set_purpose(csc, X509_PURPOSE_ANY);
	    X509_STORE_CTX_set_verify_cb( csc, verify_callback );
	    X509_STORE_CTX_set_flags( csc, 0 );
	    X509_STORE_CTX_set_ex_data( csc, GetSSLVerifyCbIdx(), this );
	    X509_verify_cert( csc );
	}
	X509_STORE_CTX_free( csc );
}

int 
NetSslCredentials::IsSelfSigned()
{
	return (*verify)[0] != -1 && (*verify)[1] == -1;
}

/*
 * Check that the server cert is issued by the first cert in the chain
 * and that each cert in the chain is issued by the following cert
 * (if any).
 * Note that the first cert in 'certificate.txt' is taken as the server
 * cert and the rest of them comprise the chain.
 */
int
NetSslCredentials::CheckCertChainOrder( X509 *cert, bool alwaysShow, Error *e )
{
    int	rslt = 0;

    if( !cert )
    {
	p4debug.printf( "NetSslCredentials::CheckCertChainOrder : server cert is NULL\n" );
	e->Set( MsgRpc::SslCertBadChain );

	return 1;
    }

    if( SSLDEBUG_CERT )
    {
	PrintCertificateSubject( "server cert", 0, cert );
	PrintCertificateIssuer( "           ", 0, cert );
	PrintCertificateAltSubject( "server alt subject", 0, cert );
    }

    X509 *prevCert = cert;

    for( unsigned int idx = 0; idx < chain->Count(); ++idx )
    {
	X509 *itemCert = (X509 *) chain->Get( idx );

	if( !itemCert )
	{
	    p4debug.printf( "Warning: Skipping NULL X.509 certificate #%d in chain\n", idx );
	    continue;
	}

	if( SSLDEBUG_CERT )
	{
	    PrintCertificateSubject( "chain  cert", idx+1, itemCert );
	    PrintCertificateIssuer( "           ", idx+1, itemCert );
	    PrintCertificateAltSubject( "server alt subject", idx+1, itemCert );
	}

	X509_NAME *prevIssuerNameX509 = X509_get_issuer_name( prevCert );
	X509_NAME *curSubjNameX509 = X509_get_subject_name( itemCert );

	bool	mismatch = X509_NAME_cmp(prevIssuerNameX509, curSubjNameX509);
	if( alwaysShow || mismatch )
	{
	    char *prevIssuerName = X509_NAME_oneline( prevIssuerNameX509, NULL, 0 );
	    char *curSubjName = X509_NAME_oneline( curSubjNameX509, NULL, 0 );

	    if( mismatch )
	    {
		X509_NAME *prevSubjNameX509 = X509_get_subject_name( prevCert );
		char *prevSubjName = X509_NAME_oneline( prevSubjNameX509, NULL, 0 );

		p4debug.printf( "* Incorrect cert chain: cert #%d '%s' was signed by '%s' but the next cert was #%d '%s'.\n"
		    "* Each certificate in the certificate file must be"
		    " signed by the following certificate (if any).\n",
		    idx, prevSubjName, prevIssuerName, idx+1, curSubjName );
		e->Set( MsgRpc::SslCertBadChain );
		rslt = 1;
		OPENSSL_free( prevSubjName );
	    }
	    OPENSSL_free( prevIssuerName );
	    OPENSSL_free( curSubjName );
	}

	prevCert = itemCert;
    }

    return rslt;
}

void 
NetSslCredentials::ValidateChain( bool criticalOnly, Error *e )
{
	if( SSLDEBUG_CERT )
	    p4debug.printf(
	        "NetSslCredentials::ValidateChain checking for verify errors\n"
	    );

	int depth = 0;
	int err;
	while( !e->Test() &&
	    ( ( err = (*verify)[depth++] ) != -1 ||
	      depth <= chain->Count() ) )
	{
	    if( err == X509_V_OK || err == -1 )
	        continue;

	    bool critical = true;
	    if( err == X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT ||
	        err == X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT ||
	        err == X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN ||
	        err == X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY ||
	        err == X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE ||
	        err == X509_V_ERR_CERT_CHAIN_TOO_LONG ||
	        err == X509_V_ERR_INVALID_CA ||
	        err == X509_V_ERR_INVALID_PURPOSE )
	        critical = false;

	    char sslError[256];
	    char func[] = "NetSslCredentials::ValidateChain X509_verify_cert";
	    ERR_error_string_n( err, sslError, 256 );
	    const char* vErr = X509_verify_cert_error_string( err );

	    BIO *bio = BIO_new( BIO_s_mem() );
	    X509* cert = depth == 1 ? certificate
	                            : (X509*)chain->Get( depth - 2 );
	    if( !cert )
	    {
		p4debug.printf( "Warning: Skipping NULL X.509 certificate #%d in chain\n",
			(depth == 1 ? -1 : depth-2) );
		continue;
	    }

	    PrintCertificateSubject( depth - 1, cert, bio, " " );
	    PrintCertificateAltSubject( depth - 1, cert, bio, " " );
	    PrintCertificateError( cert, err, 0, bio, " " );
	    BUF_MEM *bufMemPtr = 0;
	    BIO_get_mem_ptr( bio, &bufMemPtr );
	    StrBuf objS;
	    objS.Set( bufMemPtr->data, bufMemPtr->length );
	    objS.Terminate();
	    BIO_free_all( bio );

	    if( criticalOnly && !critical )
	    {
	        if( SSLDEBUG_CERT )
	            p4debug.printf( "%s ignoring failure: %s: %s %s\n",
	                            func, sslError, vErr, objS.Text() );
	        continue;
	    }

	    e->Set( MsgOs::Net2 ) << func << sslError << vErr << objS;
	    e->Set( critical ? MsgRpc::SslCertBad : MsgRpc::SslCertBadChain );
    }
}

void
NetSslCredentials::ValidateSubject( StrPtr *name, StrPtr *ip, Error *e )
{
	if( !certificate || ( !name && !ip ) )
	    return;

	if( SSLDEBUG_CERT )
	    p4debug.printf(
	      "NetSslCredentials::ValidateSubject checking for subject match\n"
	    );

	StrBuf cn;
	int len = X509_NAME_get_text_by_NID(
	              X509_get_subject_name( certificate ),
	              NID_commonName, cn.Alloc( 256 ), 256 );
	if( len >= 0 )
	{
	    cn.SetLength( len );
	    cn.Terminate();
	}

	if( name && !name->CCompare( cn ) )
	{
	    if( SSLDEBUG_CERT )
	        p4debug.printf(
	            "NetSslCredentials::ValidateSubject matched: CN == %s\n",
	            name->Text() );
	    return;
	}
	if( ip && !ip->CCompare( cn ) )
	{
	    if( SSLDEBUG_CERT )
	        p4debug.printf(
	            "NetSslCredentials::ValidateSubject matched: CN == %s\n",
	            ip->Text() );
	    return;
	}
	
	char *dot;
	if( cn.StartsWith( "*.", 2 ) &&
	    ( dot = strchr( name->Text(), '.' ) ) &&
	    !strcmp( cn.Text() + 1, dot ) )
	{
	    if( SSLDEBUG_CERT )
	        p4debug.printf(
	           "NetSslCredentials::ValidateSubject matched: "
	           "Wildcard CN (%s) == %s\n",
	           cn.Text(), name->Text() );
	    return;
	}

	int matched = 0;
	STACK_OF(GENERAL_NAME) *sans =
	    (STACK_OF(GENERAL_NAME) *)X509_get_ext_d2i( certificate,
	                                                NID_subject_alt_name,
	                                                0, 0 );
	if( sans != NULL )
	{
	    int sanc = sk_GENERAL_NAME_num( sans );
	    for( int i = 0; !e->Test() && !matched && i < sanc; i++ )
	    {
	        const GENERAL_NAME *gn = sk_GENERAL_NAME_value( sans, i );
	        if( gn->type == GEN_DNS && name )
	        {
	            // Current name is a DNS name, let's check it
	            const char *dnsName =
# if OPENSSL_VERSION_NUMBER < 0x10100000L
	                (const char *)ASN1_STRING_data( gn->d.dNSName );
# else
	                (const char *)ASN1_STRING_get0_data( gn->d.dNSName );
# endif
	            if( !dnsName )
	                continue;

	            // Make sure there isn't an embedded NUL character
	            // in the DNS name
	            if( ASN1_STRING_length( gn->d.dNSName ) !=
	                strlen( dnsName ) )
	            {
	                if( SSLDEBUG_ERROR )
	                    p4debug.printf(
	                        "NetSslCredentials::ValidateSubject "
	                        "SAN length (%d) doesn't match string "
	                        "length (%d)!\n",
	                        ASN1_STRING_length( gn->d.dNSName ),
	                        strlen( dnsName ));
	                e->Set( MsgRpc::SslCertMalformed );
	                break;
	            }

	            if( !name->CCompare( StrRef( dnsName ) ) )
	            {
	                if( SSLDEBUG_CERT )
	                    p4debug.printf(
	                        "NetSslCredentials::ValidateSubject matched: "
	                        "SAN == %s\n",
	                        name->Text() );
	                matched++;
	                break;
	            }
	        }
	        else if( gn->type == GEN_IPADD && ip )
	        {
	            const unsigned char* ipAddr = gn->d.iPAddress->data;
	            if( !ipAddr )
	                continue;

	            StrBuf tmp;
	            if( gn->d.iPAddress->length == 4 )
	            {
	                NetUtils::IpBytesToStr( gn->d.iPAddress->data, 0,
	                                        tmp );
	                if( !ip->CCompare( tmp ) )
	                {
	                    if( SSLDEBUG_CERT )
	                        p4debug.printf(
	                           "NetSslCredentials::ValidateSubject "
	                           "matched: SAN == %s\n",
	                           ip->Text() );
	                    matched++;
	                    break;
	                }
	            }
	            else if( gn->d.iPAddress->length == 16 )
	            {
	                NetUtils::IpBytesToStr( gn->d.iPAddress->data, 1,
	                                        tmp );
	                if( !ip->CCompare( tmp ) )
	                {
	                    if( SSLDEBUG_CERT )
	                        p4debug.printf(
	                            "NetSslCredentials::ValidateSubject "
	                            "matched: SAN == %s\n",
	                            ip->Text() );
	                    matched++;
	                    break;
	                }
	            }
	            else
	            {
	                if( SSLDEBUG_ERROR )
	                    p4debug.printf(
	                        "NetSslCredentials::ValidateSubject "
	                        "Bad sized IP Address in cert: %s\n",
	                        gn->d.iPAddress->length );
	                e->Set( MsgRpc::SslCertMalformed );
	                break;
	            }
	        }
	    }
	    sk_GENERAL_NAME_pop_free( sans, GENERAL_NAME_free );

	    if( matched || e->Test() )
	        return;
	}
	
	e->Set( MsgRpc::SslCertBadSubject ) << cn << name;
}

void 
NetSslCredentials::SetOwnCert( bool ownCert )
{
	this->ownCert = ownCert;
}

void 
NetSslCredentials::SetOwnKey( bool ownKey )
{
	this->ownKey = ownKey;
}

void 
NetSslCredentials::SetCertC( StrBuf &certC )
{
	this->certC = certC;
}

void 
NetSslCredentials::SetCertCN( StrBuf &certCN )
{
	this->certCN = certCN;
}

void 
NetSslCredentials::SetCertL( StrBuf &certL )
{
	this->certL = certL;
}

void 
NetSslCredentials::SetCertO( StrBuf &certO )
{
	this->certO = certO;
}

void NetSslCredentials::SetCertST( StrBuf &certST )
{
	this->certST = certST;
}

void NetSslCredentials::SetSslDir( StrPtr *sslDir )
{
	this->sslDir = *sslDir;
}


void
NetSslCredentials::GetModTimes( P4INT64 *keyFile, P4INT64 *certFile, Error *e )
{
	PathSys *cert = PathSys::Create();
	PathSys *key = PathSys::Create();
	FileSys *f = FileSys::Create( FST_BINARY );
	GetCredentialFilepaths( key, cert, e );
	if( !e->Test() && keyFile )
	{
	    f->Set( key->Text() );
	    *keyFile = f->StatModTime();
	}
	if( !e->Test() && certFile )
	{
	    f->Set( cert->Text() );
	    *certFile = f->StatModTime();
	}
	delete cert;
	delete key;
	delete f;
}

# endif // USE_SSL

