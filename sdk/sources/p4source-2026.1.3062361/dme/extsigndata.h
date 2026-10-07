/*
 * Copyright 1995, 2020 Perforce Software.  All rights reserved.
 */

/*
 * extsigndata.h
 *
 */

class ExtSignData {
 
    public:
	                ExtSignData( const StrPtr *pkgDir, Error *e );
 
	bool            BuildSigFile( const char *sigFile, 
	                              const char *certFile, 
	                              const char *privKey,
	                              Error *e );

	bool            VerifySigFile( const char *sigFile, 
	                               const char *certFile,
	                               Error *e );

	void            AddFile( const StrBuf &pkgDir, const char *file,
	                         Error *e );
 
	std::vector< std::unordered_map< std::string, std::string > >&
	                GetFiles();

    private:
 
	bool            SignData( const char *certFile,
	                          const char *privKey,
	                          const StrBuf *buffer,
	                          StrBuf &sig64, Error *e );

	bool            VerifyData( const char *certFile,
	                            const StrBuf &signedData, StrBuf &sig64,
	                            Error *e ) const;

	StrBuf          HashFile( const char *fname, Error *e ) const;

	bool            BuildSigList( StrBuf *sig, StrBuf *jout, Error *e );

	std::vector< std::unordered_map< std::string, std::string > > files;
} ;

