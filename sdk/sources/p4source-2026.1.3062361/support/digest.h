/*
 * Copyright 1995, 2024 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */


/*
 * Digest - General Digest interface
 */

enum DigestAlg { NODIGEST = 0, MD5DIG, SHA1, SHA256, SHA224, SHA384, SHA512,
    SHA512_224, SHA512_256, SHA3_224, SHA3_256, SHA3_384, SHA3_512,
    BLAKE3,
    ENDDIGEST };

// Digester uses requirements
# define DIG_RANDOM	1
# define DIG_INTEGRITY	2
# define DIG_SECURITY	4

# ifdef notyet

// The Digest class is a generic container class for a digest and the context
// of the algorithm used to create it, with utility functions to abstract away
// from needing per-algorithm classes

class Digest {
    public:
	static Digest	*ImportEx( const unsigned char *bytes, unsigned int len );
	static Digest	*Import( enum DigestAlg alg, const unsigned char *bytes, unsigned int len );
	// diable copy, assignment?
	virtual		~Digest() = 0;
	virtual void	Clear();
	virtual bool	Compare(const Digest &) const;

	virtual void	Fmt( StrBuf &buf ) const;
	virtual bool	IsSet() const;
	virtual unsigned int Length() const;
	virtual Digest	*Clone() const;

	unsigned int	ExportEx( unsigned char *bytes, unsigned int len ) const;
	unsigned int	Export( enum DigestAlg alg, unsigned char *bytes, unsigned int len ) const;
	unsigned int	LengthEx() const;
	unsigned int	Length( enum DigestAlg alg );
	bool		operator ==( const Digest & ) const;
	virtual enum DigestAlg DigestAlg() const;
};

# endif // notyet

// This Digester class is a mostly abstract class with multiple
// implementations privately specified in digest.cc

class Digester {
    public:
	static Digester	*Create(
		enum DigestAlg = NODIGEST,
		short usage = 0,	// 
		Error * = 0 );

	static enum DigestAlg Lookup(
	    const StrPtr &name,
	    enum DigestAlg defalg = SHA256,
	    int requirements = 0,
	    Error * = 0 );

	virtual		~Digester() {};

	virtual int	Length() = 0;	// length of result in bytes

	virtual void	Init( Error *e = NULL ) = 0;
	void		Update( const StrPtr &buf )
	    { Update( buf.Value(), buf.Length() ); }
	virtual void	Update( const char *buf, p4size_t len ) = 0;
# ifdef notyet
	virtual Digest	*Final() = 0;
# endif
	// Finailize digest as binary in a StrBuf
	virtual void	FinalBinary(StrBuf &) = 0;

	// Finailize digest as a hex string in a StrBuf
	virtual void	Final(StrBuf &);
	virtual void	FinalLower(StrBuf &);

	// quick small data digests - returns 0 or length of result
	virtual int	Simple( const char *p, p4size_t n,
				unsigned char *digest, p4size_t digestlen ) = 0;
	virtual int	Simple( const StrPtr &data, StrBuf &result );
};
