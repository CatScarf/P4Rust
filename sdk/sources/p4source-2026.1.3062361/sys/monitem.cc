/*
 * Copyright 1995, 2020 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# define NEED_WINDOWSH

# include "stdhdrs.h"

# include <strbuf.h>
# include <error.h>
# include <errorlog.h>
# include <debug.h>

# include "filesys.h"

# ifdef HAS_CPP11
# include <atomic>
typedef	std::atomic_int64_t	p4_aint64_t;
# else
// NOTE: This should be atomic, but isn't.  Things will compile at least.
typedef volatile P4INT64	p4_aint64_t;
# endif

# include "memfile.h"
# include "monitem.h"

inline int align( int v, int a ) { return (v + a - 1) & ~(a - 1); }

MonItem *MonItem::first = NULL;

MonItem::MonItem( const char *n, int f )
    : name(n), data(NULL), flags(f)
{
	// Add item to hash table of items
	// should assert that the named item is not already known
	next = first;
	first = this;
}

MonItem::~MonItem()
{
	// remove from hash table
}

// NOTE: Copy and assignment intentially not implemented to prevent
// automatic generation

StrRef
MonItem::Name() const
{
	return StrRef( name );
}

MonItem *
MonItem::Find( const char *n )
{
	if( !*n )
	    return NULL;

	for( MonItem *p = first; p; p = p->next )
	{
	    if( !strcmp( n, p->name ) )
	        return p;
	}
	return NULL;
}

int
MonItem::DataSize() const
{
	return 0;
}

int
MonItem::Alignment() const
{
	return 0;
}

int
MonItem::ItemId() const
{
	return MIT_NONE;
}

MonItem *
MonItem::TypeFactory( void *d )
{
	int *p = (int *)d;
	int  t = p[0];
	int  f = p[1];
	const char *n = (const char *)(2 + p);
	MonItem *i = NULL;

	switch( t )
	{
	case MIT_INTMAX:
	    i = new MonIntMax( n, f );
	    break;
	case MIT_INT:
	    i = new MonInteger( n, f );
	    break;
	case MIT_NONE:
	    i = new MonItem( n, f );
	    break;
	default:
	    break;
	}

	if( i )
	    i->data = n + align( strlen( n ) + 1, i->Alignment() );

	return i;
}

int
MonItem::ItemSize() const
{
	return align( strlen(name) + 1, Alignment() )
	    + DataSize()
	    + ( 2 * sizeof(int) );
}

void
MonItem::Initialize()
{
}

void
MonItem::Display( StrBuf *b ) const
{
	b->UAppend( "unknown type" );
}

MonInteger::MonInteger( const char *n, int f, P4INT64 init )
    : MonItem( n, f ), initialValue( init )
{
}

MonInteger::~MonInteger()
{
}

int
MonInteger::DataSize() const
{
	return sizeof(p4_aint64_t);
}

int
MonInteger::Alignment() const
{
	return sizeof(p4_aint64_t);
}

void
MonInteger::Initialize()
{
	*(p4_aint64_t *)Data() = 0;
}

int
MonInteger::ItemId() const
{
	return MIT_INT;
}

void
MonInteger::NewValue()
{
}

void
MonInteger::Set( P4INT64 l )
{
	if( Active() )
	{
	    *(p4_aint64_t *)Data() = l;
	    NewValue();
	}
}

void
MonInteger::Increment()
{
	if( Active() )
	{
	    ++*(p4_aint64_t *)Data();
	    NewValue();
	}
}

void
MonInteger::Increment( P4INT64 l )
{
	if( Active() )
	{
	    *(p4_aint64_t *)Data() += l;
	    NewValue();
	}
}

void
MonInteger::Decrement()
{
	if( Active() )
	{
	    --*(p4_aint64_t *)Data();
	    NewValue();
	}
}

void
MonInteger::Decrement( P4INT64 l )
{
	if( Active() )
	{
	    *(p4_aint64_t *)Data() -= l;
	    NewValue();
	}
}

P4INT64
MonInteger::Value() const
{
	if( Active() )
	    return *(p4_aint64_t const *)Data();
	return initialValue;
}

void
MonInteger::Display( StrBuf *b ) const
{
	if( Active() )
	{
	    StrNum n( Value() );
	    b->UAppend( &n );
	}
	else
	    b->UAppend( "inactive" );
}

int
MonIntMax::DataSize() const
{
	return 2 * sizeof(p4_aint64_t);
}

void
MonIntMax::Initialize()
{
	((p4_aint64_t *)Data())[0] = initialValue;
	// -1 should be max neg int for int64
	((p4_aint64_t *)Data())[1] = -1;
}

void
MonIntMax::NewValue()
{
	// NOTE: because these are volatile, we try to minimize operations
	// on them, i.e. reading once into non-volatile variables if possible
	// compilers do not optimize access to volatile variables

	p4_aint64_t &v = ((p4_aint64_t *)Data())[0];
	p4_aint64_t &t = ((p4_aint64_t *)Data())[1];

# ifdef HAS_CPP11
	int64_t vv = v;
	int64_t c = t;
	while( c < vv && !t.compare_exchange_strong( c, vv ) )
	{
	}
# else
	// non-atomic implementation
	P4INT64 vv = v;
	if( t < vv )
	    t = vv;
# endif
}

int
MonIntMax::ItemId() const
{
	return MIT_INTMAX;
}

P4INT64
MonIntMax::MaxValue() const
{
	if( Active() )
	    return ((p4_aint64_t const *)Data())[1];
	return -1;
}

void
MonIntMax::Display( StrBuf *b ) const
{
	if( Active() )
	{
	    MonInteger::Display( b );
	    b->UAppend( " max " );
	    StrNum n( MaxValue() );
	    b->UAppend( &n );
	}
	else
	{
	    b->UAppend( "inactive" );
	}
}

MemItems::MemItems()
    : mem( 0 ), mlen( 0 ), mf( 0 )
{
}

bool
MemItems::Init( const char *fname )
{
	delete mf;
	mem = 0;
	if (mlen == 0)
	    mlen = ComputeSize();

	// Create shared memory
	mf = new MemFile;
	mf->Open( fname, mlen * 2 );	// twice size for new and old items

	if (mf->Addr() != NULL)
	{
	    mem = (char *)mf->Addr();
	    mlen = mf->Size();
	    LoadMem( mem );
	    Init();
	}

	return mf->Addr() != NULL;
}

bool
MemItems::HardInit( const char *fname )
{
	delete mf;
	mem = 0;
	if (mlen == 0)
	    mlen = ComputeSize();

	// Create shared memory
	mf = new MemFile;
	mf->Open( fname, mlen );

	if (mf->Addr() != NULL)
	{
	    mem = (char *)mf->Addr();
	    mlen = mf->Size();
	    Init();
	}

	return mf->Addr() != NULL;
}

bool
MemItems::Load( const char *fname, Error *e )
{
	// open existing shared memory
	delete mf;
	mf = new MemFile( fname, e );
	if( e->Test() )
	    return false;

	mem = (char *)mf->Addr();
	mlen = mf->Size();

	if( mem != NULL )
	    LoadMem( mem );

	return mem != NULL;
}

long
MemItems::ComputeSize()
{
	// compute size
	long sz = 0; // really memory header space
	for( MonItem *i = MonItem::first; i; i = i->next )
	    sz += i->ItemSize();
	sz += sizeof(int);
	return sz;
}

void
MemItems::Init()
{
	// Making a new memory thing
	if( mem == NULL )
	{
	    mlen = ComputeSize();
	    mem = new char[mlen];
	}

	int *w = (int *)mem;

	// HEADER Stuff here
	for( MonItem *i = MonItem::first; i; i = i->next )
	{
	    if( i->data == NULL )
	    {
	        w[0] = i->ItemId();
	        w[1] = i->flags;
	        char *s = (char *)(2 + w);
	        strcpy( s, i->name );
	        s += align( strlen( i->name )+1, i->Alignment() );
	        i->data = s;
	        w = (int *)(s + i->DataSize());
	        i->Initialize();
	    }
	}

	mem = (char *)w;

	*w = -1;
}

void
MemItems::LoadMem( void *m )
{
	mem = (char *)m;

	// Magic number and header stuff
	while( *(int *)mem > 0 )
	{
	    const char *n = (const char *)(2 + (int *)mem);
	    MonItem *f = MonItem::Find( n );
	    if( !f )
	        f = MonItem::TypeFactory( mem );

	    f->data = n + align( strlen( n )+1, f->Alignment() );
	    mem += f->ItemSize();
	}
}

MemItems::~MemItems()
{
	delete mf;
}

void
MemItems::Dump( const char *f, Error *e )
{
	FileSys *file = FileSys::Create( FST_BINARY );
	Error e1;
	if( !e )
	    e = &e1;

	file->Set( f );
	file->Open( FOM_WRITE, e );
	if( !e->Test() )
	    file->Write( mem, mlen, e );
	if( !e->Test() )
	    file->Close( e );
	delete file;
	
	if( e->Test() && e != &e1)
	    AssertLog.Report( e );
}

void
MemItems::Display()
{
	for( MonItem *i = MonItem::first; i; i = i->next )
	{
	    StrBuf b;
	    printf( "%s (flags %o) ", i->Name().Text(), i->Flags() );
	    i->Display( &b );
	    b.Terminate();
	    printf( "%s\n", b.Text() );
	}
}
