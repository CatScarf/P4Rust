/**
 * @file netsslcredentials.h
 *
 * @brief Wrapper for getting or generating server cert and private key.
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

/*
 * These headers are required to be included before
 * including netsslcredentials.h
 *
 * extern "C" {	// OpenSSL
 *
 * # include <openssl/pem.h>
 * # include <openssl/conf.h>
 * # include <openssl/x509v3.h>
 * # ifndef OPENSSL_NO_ENGINE
 * # include <openssl/engine.h>
 * # endif
 *
 * }
 *
 */
# ifdef USE_SSL

class VarArray;
class IntArray;
class PathSys;
class DateTime;

class NetSslCredentials
{
public:
		static void ValidateCertDateRange( X509 *cert, Error *e );
	
		NetSslCredentials( bool isTest = false );
# ifdef HAS_CPP11
		/*
		 * delete the copy ctor and assignment op because
		 * 1) we don't use either of them
		 * 2) they each just do a shallow copy, we don't yet
		 *    do a good enough job of tracking and handling shared
		 *    ownership, and we haven't tested them.
		 */
		NetSslCredentials( NetSslCredentials &rhs) = delete;
		NetSslCredentials &operator =( NetSslCredentials &rhs ) = delete;
# endif
		~NetSslCredentials();
		void ReadCredentials( Error *e );
		void ClearChain( bool freeChain );
		void ReleaseCredentials( bool freeCert = false, bool freeKey = false );
		void GenerateCredentials( Error *e );
		void ValidateSslDir( Error * e);
		void ValidateCredentialFiles( Error *e );
		void GetExpiration( StrBuf &buf );
		void HaveCredentials( Error *e );
		void ValidateChain( bool criticalOnly, Error *e );
		int CheckCertChainOrder( X509 *cert, bool alwaysShow, Error *e );

		void ValidateSubject( StrPtr *name, StrPtr *ip, Error *e );

		// Getters and Setters
		X509 *GetCertificate() const;
		X509 *GetChain( int i ) const;
		const StrPtr *GetFingerprint() const;
		EVP_PKEY *GetPrivateKey() const;
		void SetCertificate( X509 *certificate,
		                     stack_st_X509 *certChain,
		                     X509_STORE *store, Error *e );
		void SetOwnCert(bool ownCert);
		void SetOwnKey(bool ownKey);
		void SetCertC(StrBuf &certC);
		void SetCertCN(StrBuf &certCN);
		void SetCertL(StrBuf &certL);
		void SetCertO(StrBuf &certO);
		void SetCertST(StrBuf &certSt);
		void SetSslDir(StrPtr *sslDir);
		void GetCredentialFilepaths( PathSys *keyFile,
		                             PathSys *certFile, Error * e );
		void GetFingerprintFromCert( Error *e );
		int IsSelfSigned();
		void SetCertValid( int depth, int error, X509 *cert );

		void GetModTimes( P4INT64 *keyFile, P4INT64 *certFile,
		                  Error *e );

private:
		// Utilities
		void ParseConfig( Error *e );
		void WriteCredentials( PathSys *keyFile, PathSys *certFile,
		                       Error * e );
		void MakeSslCredentials( Error *e );
		void CompareFileUids( Error *e );
		void CompareDirUid( Error *e );

		EVP_PKEY *privateKey;
		X509 *certificate;
		VarArray *chain;
		IntArray *verify;
		StrBuf fingerprint;
		StrBuf certC;
		StrBuf certCN;
		StrBuf certST;
		StrBuf certL;
		StrBuf certO;
		int certEX;
		int certSV;
		int certUNITS;
		bool ownKey;
		bool ownCert;
		StrBuf sslDir;
};

# endif // USE_SSL
