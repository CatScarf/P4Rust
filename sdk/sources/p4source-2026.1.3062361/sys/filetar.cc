/*
 * Copyright 2026 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# include <stdhdrs.h>
# include <error.h>
# include <errorlog.h>
# include <strbuf.h>
# include <filesys.h>
# include <fileio.h>
# include <filestrbuf.h>
# include <vararray.h>
# include <debug.h>
# include <gzip.h>
# include "filetar.h"

TarCallback::TarCallback( const char *path, const char *name, P4INT64 mtime,
	                  FileSys *input )
{
	Initialize( path, name, mtime );
	source = input;
}

TarCallback::TarCallback( const char *path, const char *name, P4INT64 mtime, Lbr *lbr,
	                  TarCbFunc_t i, TarCbDelFunc_t d )
{
	Initialize( path, name, mtime );
	this->lbr = lbr;
	this->lbrinit = i;
	this->lbrdel = d;
	source = 0;
}

TarCallback::TarCallback( const char *path, const char *name, P4INT64 mtime )
{
	Initialize( path, name, mtime );
	source = new FileStrPtr( &payload );
}

void
TarCallback::Initialize( const char *path, const char *name, P4INT64 mtime )
{
	this->path.Set( path );
	this->name.Set( name );
	this->mtime = mtime;
	state = Init;
	fillcnt = 0;
	padcnt = 0;
	headercnt = 0;
	tarpos = 0;
	late = 0;
	paysize = 0;
	lbr = 0;
	lbrinit = 0;
	lbrdel = 0;
}

TarCallback::~TarCallback()
{
	if( lbrdel )
	    lbrdel( this );
	delete source;
}

void
TarCallback::dochksum( tarheader *th )
{
	int sum = 0;
	unsigned char *cptr = ( unsigned char * ) th;
	for( int i = 0; i < sizeof( tarheader ); i++ )
	    sum += *cptr++;

	sprintf( th->chksum, "%07o", sum );
}

void
TarCallback::SetHdrPtr( const char *th, char *hdrs )
{
	late = th;
	headers = hdrs;
}

int
TarCallback::Read( char *buf, int len, Error *e )
{
	char *where = buf;
	int left = len;

	while( left > 0 )
	{
	    if( state == Init )
	    {
	        if( paysize )
	            paycnt = paysize;
	        else
	        {
	            if( lbrinit )
	            {
	                lbrinit( this, e );
	                if( e->Test() )
	                    return -1;
	            }
	            else
	            {
	                source->Open(FOM_READ, e );
	                if( e->Test() )
	                    return -1;
	            }
	            paycnt = source->GetSize();
	        }
	        headercnt = SetTarHeader();
	        if( !headercnt )
	        {
	            e->Set( E_FAILED, "The extended tar header is too big" );
	            return -1;
	        }
	        P4INT64 rsize = paycnt + TARSZ - 1;
	        rsize &= ~(TARSZ - 1);
	        fillcnt = rsize - paycnt;

	        int request = headercnt;
	        if( request > len )
	            request = len;
	        memcpy( where, headers, request );
	        where += request;
	        left -= request;
	        headercnt -= request;
	        if( headercnt )
	        {
	            tarpos = headers + request;
	            state = Header;
	        }
	        else
	        {
	            state = Payload;
	        }
	        continue;
	    }
	    if( state == Header )
	    {
	        int request = headercnt;
	        if( request > len )
	            request = len;
	        memcpy(where, tarpos, request);
	        where += request;
	        left -= request;
	        headercnt -= request;
	        if( headercnt )
	            tarpos += request;
	        else
	        {
	            state = Payload;
	        }
	        continue;
	    }
	    if( state == Payload )
	    {
	        int request = left;
	        if( paycnt < request )
	            request = paycnt;
	        int n = 0;
	        if( request )
	        {
	            n = source->Read( where, request, e );
	            if( n < 0 || e->Test() )
	                return -1;
	        }
	        if( n == 0 )
	        {
	            if( paycnt )
	            {
	                padcnt = paycnt;
	                state = Pad;
	            }
	            else
	            {
	                state = Fill;
	            }
	            continue;
	        }
	        else
	        {
	            where += n;
	            paycnt -= n;
	            left -= n;
	        }
	        continue;
	    }
	    if( state == Pad )
	    {
	        if( padcnt )
	        {
	            int request = padcnt;
	            if( request > left )
	                request = left;
	            memset(where, '\0', request );
	            where += request;
	            padcnt -= request;
	            left -= request;
	        }
	        if( !padcnt )
	            state = Fill;
	        continue;
	    }
	    if( state == Fill )
	    {
	        if( fillcnt )
	        {
	            int request = fillcnt;
	            if( request > left )
	                request = left;
	            memset( where, '\0', request );
	            where += request;
	            left -= request;
	            fillcnt -= request;
	        }
	        if( !fillcnt )
	            state = Done;
	        continue;
	    }
	    if( state == Done )
	    {
	        break;
	    }
	}
	if( state == Done )
	{
	    source->Close( e );
	    if( e->Test() )
	        return -1;
	}

	return  len - left;
}

static const struct {
    tarheader th;
    char pad[ TARSZ - sizeof( tarheader ) ];
} late_template = {
    {
	/* name */     { 0 },
	/* mode */     "0000444",
	/* uid */      "0000000",
	/* gid */      "0000000",
	/* size */     { 0 },
	/* mtime */    "00000000000",
	/* chksum */   { ' ',' ',' ',' ',' ',' ',' ',' ' },
	/* typeflag */ { '0' },
	/* linkname */ { 0 },
	/* magic */    "ustar",
	/* version */  { '0','0' },
	/* uname */    "perforce",
	/* gname */    "perforce",
	/* devmajor */ "0000000",
	/* devminor */ "0000000",
	/* prefix */   { 0 },
    },
    { 0 },
};
// The extended tag names.
static const char *T_size  = "size";
static const char *T_path  = "path";
static const char *T_mtime = "mtime";

int
TarCallback::SetTarHeader()
{
	// See if we need to use an extended header first.
	//const char *T_linkname = "linkname";
	int extended = 0;
	char *next = headers;
	char *bad = headers + HDRSZ - TARSZ;
	// 11 octal digits (8589934591)
	if( paycnt > 077777777777 || name.Length() >= 100 ||
	    path.Length() >= 155 || mtime > 077777777777 )
	{
	    // The extended header.
	    memcpy( next, late, TARSZ );
	    next += TARSZ;
	    tarheader *th = (tarheader *)headers;
	    th->typeflag[0] = 'x';

	    // The size tag
	    int taglen = calc_tag_len( strlen( T_size ), vsize( paycnt ) );
	    if ( next + taglen >= bad )
	        return 0;
	    sprintf(next, "%d %s=%lld\n", taglen, T_size, paycnt );
	    next += taglen;

	    // The file pathname tag
	    StrBuf newpath;
	    if( path.Length() )
	        newpath << path << "/" << name;
	    else
	        newpath << name;
	    taglen = calc_tag_len( strlen( T_path ), newpath.Length() );
	    if ( next + taglen >= bad )
	        return 0;
	    sprintf(next, "%d %s=%s\n", taglen, T_path, newpath.Text() );
	    next += taglen;

	    // The mtime tag
	    taglen = calc_tag_len( strlen( T_mtime ), vsize( mtime ) );
	    if ( next + taglen >= bad )
	        return 0;
	    sprintf(next, "%d %s=%lld\n", taglen, T_mtime, mtime );
	    next += taglen;

	    // round up to the next block size.
	    int bytes = next - headers;
	    int newsize = (bytes + TARSZ - 1) & ~(TARSZ - 1);
	    memset(next, 0, newsize - bytes);
	    next += newsize - bytes;
	    extended = next - headers;

	    // the size of the extended blocks
	    //
	    sprintf( th->size, "%011llo",
	              (P4INT64) bytes - TARSZ );
	    dochksum( th );
	}
	// The standard header
	memcpy( next, late, TARSZ );
	// patch it up
	tarheader *th = (tarheader *) next;
	if( !extended )
	{
	    strncpy( th->name, name.Text(), 100 );
	    sprintf( th->size, "%011llo", paycnt );
	    sprintf( th->mtime, "%011llo", mtime );
	    strncpy( th->prefix, path.Text(), 155 );
	}
	dochksum( th );
	extended += TARSZ;
	return extended;
}

// Given the length of the key and value in a tag line,
// compute the full length once its composed.
int
TarCallback::calc_tag_len( int keylen, int vallen )
{
	int base = keylen + vallen + 3; // space '=' '\n'
	base += tag_fudge( base );
	return base;
}

// Fix up the length of the tag, dealing with the fact that
// the length field has to be included in the calculation.
int
TarCallback::tag_fudge( int base )
{
	if( base < 9 )
	    return 1;
	if( base < 98 )
	    return 2;
	if( base < 997 )
	    return 3;
	if( base < 9996 )
	    return 4;
	if( base < 99995 )
	    return 5;
	if( base < 999994 )
	    return 6;
	// If its too big, make sure we fail.
	return HDRSZ;
}

// Number of digits when converted into a base 10 string.
int
TarCallback::vsize( P4INT64 val )
{
	int ret = 1;
	while( val /= 10 )
	    ret++;
	return ret;
}

FileTar::FileTar( int compressed )
{
	if( compressed )
	{
	    gzip = new Gzip;
	    gzbuf = new StrFixed( COMPSZ );
	    gzip->is = gzbuf->Text();
	    gzip->ie = gzbuf->Text();
	}
	else
	{
	    gzip = 0;
	    gzbuf = 0;
	}
	headers = new StrFixed( HDRSZ );
	cbidx = 0;
	trailcnt = 0;
	done = 0;
}

FileTar::~FileTar()
{
	for( int i = 0; i < cblist.Count(); i++ )
	{
	    TarCallback *cb = (TarCallback *) cblist.Get( i );
	    delete cb;
	}
	delete gzip;
	delete gzbuf;
	delete headers;
}

int
FileTar::comp_read( char *buf, int len, Error *e )
{
	int res;
	int done = 0;
	gzip->os = buf;
	gzip->oe = buf + len;

	do
	{
	    if( gzip->InputEmpty() && !done )
	    {
	        int l = real_read( gzbuf->Text(), gzbuf->Length(), e );
	        gzip->is = l ? gzbuf->Text() : 0;
	        gzip->ie = gzbuf->Text() + l;
	        done |= !l;
	    }
	}
	while( !e->Test() && gzip->Compress( e ) && !gzip->OutputFull() );

	res = gzip->os - buf;
	return res;
}

int
FileTar::Read( char *buf, int len, Error *e )
{
	if( gzip )
	    return comp_read( buf, len, e);
	else
	    return real_read( buf, len, e);
}

int
FileTar::real_read( char *buf, int len, Error *e )
{
	char *work = buf; // where to write next
	int left = len; // number of byes still needing to be satified

	while( left > 0 )
	{
	    if( done )
	    {
	        if( trailcnt )
	        {
	            int request = trailcnt;
	            if( request > left )
	                request = left;
	            memset(work, 0, request );
	            work += request;
	            trailcnt -= request;
	            left -= request;
	            if ( trailcnt )
	                continue;
	        }
	        break;
	    }
	    if( cbidx >= cblist.Count() )
	    {
	        // Add the trailer.
	        done = 1;
	        trailcnt = TARSZ * 2;
	        continue;
	    }

	    TarCallback *cb = (TarCallback *) cblist.Get( cbidx );
	    if( cb->State() == Done )
	    {
	        cbidx++;
	        continue;
	    }
	    int n;
	    n = cb->Read( work, left, e );
	    if( e->Test() || n < 0 )
	        return -1;
	    work += n;
	    left -= n;
	}
	return len - left;
}

void
FileTar::AddCallback( TarCallback *cb )
{
	cb->SetHdrPtr( (const char *)&late_template, headers->Text() );
	cblist.Put( (void *) cb );
}

