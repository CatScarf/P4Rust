/*
 * Copyright 1995, 2024 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

/*
 * Digest - General Digest interface
 */


# include <stdhdrs.h>
# include <strbuf.h>
# include <error.h>
# include <digest.h>
# include <strops.h>
# include <msgsupp.h>
# include <openssl/evp.h>

# ifdef USE_CDC
# include "../blake3/blake3.h"
# endif

typedef const EVP_MD * (*MD_THING)();

static struct MD_TYPE {
	const char	*name;
	// These flags are here to warn or error about insecure use
	short		insecure_flags;
	MD_THING	md;
} mdtable[] = {
    { "none", 0, NULL },
    { "md5", DIG_SECURITY|DIG_INTEGRITY, EVP_md5 },	// very insecure
    { "sha1", DIG_SECURITY, EVP_sha1 },	// insecure
    { "sha256", 0, EVP_sha256 },
    { "sha224", 0, EVP_sha224 },
    { "sha384", 0, EVP_sha384 },
    { "sha512", 0, EVP_sha512 },
    { "sha512_224", 0, EVP_sha512_224 },
    { "sha512_256", 0, EVP_sha512_256 },
    { "sha3_224", 0, EVP_sha3_224 },
    { "sha3_256", 0, EVP_sha3_256 },
    { "sha3_384", 0, EVP_sha3_384 },
    { "sha3_512", 0, EVP_sha3_512 },
    { "blake3", 0, NULL }, // This has special handling
    { NULL, 0, NULL }
};

void
Digester::Final( StrBuf &buf )
{
	StrBuf tmp;

	// final in binary
	FinalBinary( tmp );

	// convert to hex
	buf.Clear();
	StrOps::OtoX( tmp, buf );
}

void
Digester::FinalLower( StrBuf &buf )
{
	StrBuf tmp;

	// final in binary
	FinalBinary( tmp );

	// convert to hex
	buf.Clear();
	StrOps::OtoXlower( tmp, buf );
}

int
Digester::Simple( const StrPtr &data, StrBuf &result )
{
	p4size_t s = Length();

	result.Clear();
	return Simple( (const char *)data.Value(), data.Length(),
			(unsigned char *)result.Alloc(s), s);
}

class OpenSSLDigester : public Digester {
    public:
	OpenSSLDigester(enum DigestAlg);
	virtual		~OpenSSLDigester();
	virtual int	Length();	// length of result in bytes

	virtual void	Init( Error *e = NULL );
	virtual void	Update( const char *buf, p4size_t len );
# ifdef notyet
	virtual Digest	*Final();
# endif
	virtual void	FinalBinary(StrBuf &);

	// quick small data digests - returns 0 or length of result
	virtual int	Simple( const char *p, p4size_t n,
				unsigned char *digest, p4size_t digestlen );
    private:
	void		Clear();

	EVP_MD_CTX	*ctx;
	const EVP_MD	*md;
};

OpenSSLDigester::OpenSSLDigester(enum DigestAlg alg)
    : ctx( NULL )
{
	// bounds check at higher level
	md = mdtable[(int)alg].md();
	Init( NULL );
}

OpenSSLDigester::~OpenSSLDigester()
{
	Clear();
}

void
OpenSSLDigester::Clear()
{
	if ( ctx )
	    EVP_MD_CTX_free( ctx );
}

int
OpenSSLDigester::Length()
{
	if ( md )
	    return EVP_MD_size( md );
	return 0;
}

void
OpenSSLDigester::Init( Error *e )
{
	Clear();
	ctx = EVP_MD_CTX_new();
	if ( ctx )
	    EVP_DigestInit_ex( ctx, md, 0 );
}

void
OpenSSLDigester::Update( const char *buf, p4size_t len )
{
	if ( ctx )
	    EVP_DigestUpdate( ctx, buf, len );
}

# ifdef notyet
Digest *
OpenSSLDigester::Final()
{
	if ( ctx )
	{
	    unsigned int s = Length();
	    // make holder class
	    Digest *dig = NULL;
	    EVP_DigestFinal_ex(ctx, dig->Buffer(), &s);
	}
	return NULL;
}
# endif

void
OpenSSLDigester::FinalBinary(StrBuf &buf)
{
	if ( ctx )
	{
	    buf.Clear();
	    unsigned int s = Length();
	    EVP_DigestFinal_ex(ctx, (unsigned char *)buf.Alloc(s), &s);
	    buf.SetLength(s);
	}
}

int
OpenSSLDigester::Simple( const char *p, p4size_t n,
			 unsigned char *digest, p4size_t dlen )
{
	if ( ctx )
	{
	    int l = Length();
	    if ( l == 0 || dlen < l )
		return 0;
	    EVP_DigestUpdate( ctx, p, n );
	    EVP_DigestFinal_ex( ctx, digest, &dlen );
	    EVP_DigestInit_ex( ctx, md, 0 );
	    return l;
	}
	return 0;
}

# ifdef USE_CDC

class Blake3Private : public Digester {
    public:
	Blake3Private();
	virtual		~Blake3Private();
	virtual int	Length();	// length of result in bytes

	virtual void	Init( Error *e = NULL );
	virtual void	Update( const char *buf, p4size_t len );
# ifdef notyet
	virtual Digest	*Final();
# endif
	virtual void	FinalBinary(StrBuf &);

	// quick small data digests - returns 0 or length of result
	virtual int	Simple( const char *p, p4size_t n,
				unsigned char *digest, p4size_t digestlen );
    private:
	void		Clear();

	blake3_hasher	b3;
};

Blake3Private::Blake3Private()
{
	Init();
}

Blake3Private::~Blake3Private()
{
}

int
Blake3Private::Length()
{
	return BLAKE3_OUT_LEN;
}

void
Blake3Private::Init(Error *e)
{
	blake3_hasher_init(&b3);
}

void
Blake3Private::Update( const char *buf, p4size_t len )
{
	blake3_hasher_update( &b3, buf, len );
}

# ifdef notyet
Digest *
Blake3Private::Final()
{
	return NULL;
}
# endif

void
Blake3Private::FinalBinary(StrBuf &b)
{
	b.Clear();
	blake3_hasher_finalize( &b3, (uint8_t *)b.Alloc(Length()), Length() );
}

int
Blake3Private::Simple( const char *p, p4size_t n,
			 unsigned char *digest, p4size_t dlen )
{
	int l = Length();
	if ( l == 0 || dlen < l )
	    return 0;
	blake3_hasher_update( &b3, p, n );
	blake3_hasher_finalize( &b3, digest, dlen );
	blake3_hasher_init( &b3 );

	return l;
}

# endif

Digester *
Digester::Create( enum DigestAlg alg, short, Error * )
{
	if ((int)alg <= NODIGEST || (int)alg >= ENDDIGEST)
	    return NULL;

	// error and secure usage checking

	if (alg == BLAKE3) {
# ifdef USE_CDC
	    return new Blake3Private();
# else
	    return NULL;
# endif
	}

	return new OpenSSLDigester(alg);
}

enum DigestAlg
Digester::Lookup( const StrPtr &name, enum DigestAlg defalg,
	int requirements, Error *e )
{
	enum DigestAlg retdig = NODIGEST;

	if ( name.Length() > 0 )
	{
	    int i = 0;
	    for ( MD_TYPE *mtp = mdtable; mtp->name; ++mtp, ++i )
	    {
		if ( StrPtr::CCompare( name.Text(), mtp->name ) == 0 ) {
		    retdig = (enum DigestAlg)i;
		    if ( requirements & mtp->insecure_flags )
		    {
			// considered insecure
			e->Set( MsgSupp::DigestAlgWeak ) << mtp->name;
		    }
		    break;
		}
	    }

# ifndef USE_CDC
	    // if we do not have blake3 then error
	    if ( retdig == BLAKE3 )
		retdig = NODIGEST;
# endif

	    if ( retdig == NODIGEST )
	    {
		// not found, using default
		retdig = defalg;
		e->Set( MsgSupp::DigestAlgNotFound ) << name
		     << mdtable[(int)retdig].name;
	    }

	} else
	    retdig = defalg;

	return retdig;
}
