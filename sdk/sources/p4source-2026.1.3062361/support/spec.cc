/*
 * Copyright 1995, 1996 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

/*
 * spec.cc -- parsing and formatting of Specs according to their definitions
 */

# include <stdhdrs.h>

# include <strbuf.h>
# include <strops.h>
# include <strdict.h>
# include <vararray.h>
# include <intarray.h>
# include <error.h>
# include <errorlog.h>
# include <debug.h>

# include "specchar.h"
# include <msgdb.h>
# include <msgdm2.h>
# include "specparse.h"

# include "p4tags.h"
# include "datetime.h"

# include "spec.h"

/*
 * Spec
 */

Spec::Spec()
{
	elems = new VarArray;
	comment = StrRef::Null();
}

Spec::Spec( const char *string, const char *cmt, Error *e )
{
	elems = new VarArray;
	comment.Set( (char *)cmt );

	// We used to abort on an error here, but that also caused
	// the server to exit on windows. Now we just pass up the error.

	StrRef p;
	p.Set( (char *)string );
	Decode( &p, e );
}

Spec::~Spec()
{
	for( int i = 0; i < elems->Count(); i++ )
	    delete (SpecElem *)elems->Get(i);
	delete elems;
}

int 
Spec::Count()
{
	return elems->Count();
}

SpecElem * 
Spec::Get( int i )
{
	return (SpecElem *)elems->Get( i );
}

SpecElem *
Spec::Find( int code, Error *e, const StrPtr *fixedTag )
{
	int tryFixed = fixedTag && fixedTag->Length();

	for( int i = 0; i < elems->Count(); i++ )
	{
	    SpecElem *de = (SpecElem *)elems->Get(i);

	    if( ( code || !tryFixed ) && de->code == code )
	       return de;
	    else if( tryFixed && de->fixed == *fixedTag )
	       return de;
	}

	if( e )
	    e->Set( MsgDb::FieldBadIndex );

	return 0;
}

SpecElem *
Spec::Find( const StrPtr &tag, Error *e )
{
	for( int i = 0; i < elems->Count(); i++ )
	{
	    SpecElem *de = (SpecElem *)elems->Get(i);

	    if( !de->tag.CCompare( tag ) )
	        return de;
	}

	if( e )
	    e->Set( MsgDb::FieldUnknown ) << tag;

	return 0;
}

void
Spec::Dump( const char *msg )
{
	if( p4debug.GetLevel( DT_SPEC ) < 5 )
	    return;

	p4debug.printf("Spec::Dump %s\n", msg);
	if ( ! elems->Count() )
	{
	    p4debug.printf("Spec::Dump .... No elems\n");
	    return;

	}
	for( int i = 0; i < elems->Count(); i++ )
	{
	    SpecElem *de = (SpecElem *)elems->Get(i);
	    p4debug.printf("Spec::Dump .... elem  %d code=%d tag=%s fixed=%s\n",i,
	            de->code, de->tag.Text(), de->fixed.Text());
	}
}

/*
 * Spec::Encode() -- encode a spec definition in a transmittable string
 */

void
Spec::Encode( StrBuf *string )
{
	string->Clear();

	for( int i = 0; i < elems->Count(); i++ )
	    ((SpecElem *)elems->Get(i))->Encode( string, i );
}

/*
 * Spec::Decode() -- decode a spec definition from a string
 */

void
Spec::Decode( StrPtr *string, Error *e )
{
	// Since Decode() mangles the buffer, we have our own copy.
	// This just appends to the definition list!

	StrRef r;
	decoderBuffer.Set( string );
	r.Set( &decoderBuffer );

	while( !e->Test() && *r.Text() )
	    Add( StrRef( "tag" ) )->Decode( &r, e );
}

/*
 * Spec::EncodeFieldMapToString() -- extract fieldmap from spec elements
 *         and encode to StrBrf.
 */

void
Spec::EncodeFieldMapToString( StrBuf *s,  Error *e )
{
	for( int i = 0; i < elems->Count(); i++ )
	{
	    SpecElem *se = (SpecElem *)elems->Get(i);
	    {
	        StrBuf  c;
	        c.Clear();
	        c << se->code;
	        *s << se->tag << ":" << c << ";";
	    }
	}
	if ( elems->Count() )
	    *s << ";";
}
/*
 * Spec::ExtractFieldMapToDict() -- extract the fieldmap from spec elements 
 *          and encode to a StrDict. tag => code 
 */

void
Spec::ExtractFieldMapToDict( StrDict *map,  Error *e, int skipAuto )
{

	for( int i = 0; i < elems->Count(); i++ )
	{
	    SpecElem *se = (SpecElem *)elems->Get(i);
	    if ( skipAuto && !se->code )    // 0 means AutoId
	        continue;
	    {
	        StrBuf  c;
	        c.Clear();
	        c << se->code;
	        map->SetVar( se->tag.Text(), c.Text() );
	    }
	}
}
/*
 * Spec::Add() - create a single new specElem
 */

SpecElem *
Spec::Add( const StrPtr &tag )
{
	SpecElem *de = new SpecElem;

	de->index = elems->Count();
	de->tag.Set( &tag );
	de->code = de->index;
	de->type = SDT_WORD;
	de->opt = SDO_OPTIONAL;
	de->nWords = 1;
	de->maxLength = 0;
	de->fmt = SDF_NORMAL;
	de->seq = 0;
	de->open = SDO_NOTOPEN;
	de->maxWords = 0;	// used for Streams

	elems->Put( de );

	return de;
}

/*
 * Spec::Add() - create a single new specElem copied from a SpecElem
 */

SpecElem *
Spec::Add( const SpecElem *se, int atIndex, Error *e )
{
	SpecElem *de = new SpecElem;
	int count = elems->Count();

	de->index = elems->Count();
	de->type = se->type;
	de->tag.Set( &se->tag );
	de->fixed.Set( &se->fixed );
	de->presets.Set( &se->presets );
	de->values.Set( &se->values );
	de->code = se->code;
	de->subCode.Set( &se->subCode );
	de->opt  = se->opt;
	de->nWords = se->nWords;
	de->maxLength = se->maxLength;
	de->fmt  = se->fmt;
	de->seq  = se->seq;
	de->open = se->open;
	de->maxWords = se->maxWords;	// used for Streams

	if ( atIndex >= count )
	{
	    elems->Put( de );    // add new slot at end.
	    return de ;
	}

	elems->Put( 0 );    // add new slot at end.

	int i = count-1;      // orignal count
	for ( ; i >= atIndex; i-- ) 
	    elems->Replace( i+1 , elems->Get(i) );

	elems->Replace( atIndex ,de );
	return de;
}

/*
 * Spec::Parse() -- parse a spec string into SpecData
 */

void
Spec::Parse( const char *buf, SpecData *data, Error *e, int validate )
{
	StrBuf tag;
	StrBuf value;
	SpecParseReturn r;

	// For the uniq & required check

	IntArray counts( elems->Count() );

	// Fire up the tokenizer

	SpecParse parser( buf );

	for( ;; )
	{
	    // main loop: parse TAG & values

	    r = parser.GetToken( 0, &tag, e );

	    // toss comment outside of a tag

	    if( r == SR_COMMENT || r == SR_COMMENT_NL )
	        continue;

	    if( r != SR_TAG )
	        break;

	    SpecElem *de;

	    // Look up tag in Spec Definition.

	    if( !( de = Find( tag, e ) ) )
	        break;

	    // Count the values
	    int vcount = 0;

	    // Read each value: either a block of text or individual line.

	    while( !e->Test() )
	    {
	        // Create a space for the text.

	        r = parser.GetToken( de->IsText(), &value, e );

	        if( r == SR_COMMENT )
	        {
	            data->SetComment( de, counts[ de->index ]++, &value, 0, e );
	            continue;
	        }

	        if( r == SR_COMMENT_NL )
	        {
	            data->SetComment( de, counts[ de->index ]++, &value, 1, e );
	            continue;
	        }

	        if( r != SR_VALUE )
	            break;

	        vcount++;

	        // Prevent duplicate tags...

	        if( counts[ de->index ] && !de->IsList() )
	        {
	            e->Set( MsgDb::Field2Many ) << de->tag;
	            break;
	        }

	        // Check against limited values

	        if( validate && !de->CheckValue( value ) )
	        {
	            e->Set( MsgDb::FieldBadVal ) << de->tag << de->values;
	            break;
	        }

	        // Call SpecData to actually dispose of the data
	        // XXX No word count check!

	        data->SetLine( de, counts[ de->index ]++, &value, e );
	        if( e->CheckId( MsgDb::FieldUnknown ) )
	        {
	            // Set an informative message.
	            StrBuf field( *e->GetDict()->GetVar("field") );
	            e->Clear();
	            e->Set( MsgDm2::SpecMissingBuiltin ) << field;
	            e->Snap();
	        }

	        // Only one text block per tag

	        if( de->IsText() )
	            break;
	    }

	    if( !vcount && de->IsList() && de->AllowEmpty() )
	        counts[ de->index ]++;

	    if( e->Test() || r == SR_EOS )
	        break;
	}

	if( e->Test() )
	    parser.ErrorLine( e );

	// Check that required fields are present

	if( !e->Test() && validate )
	{
	    int i;
	    SpecElem *de;

	    for( i = 0; ( de = (SpecElem *)elems->Get(i) ); i++ )
	        if( de->IsRequired() && !counts[ de->index ] )
	    {
	        e->Set( MsgDb::FieldMissing ) << de->tag;
	        break;
	    }
	}
}

/*
 * Spec::Format() -- turn SpecData into a spec string
 */

void
Spec::Format( SpecData *data, StrBuf *s )
{
	s->Clear();

	const StrRef nl("\n");
	const StrRef nltab("\n\t");
	// Comment at the front

	*s << comment;

	for( int i = 0; i < elems->Count(); i++ )
	{
	    SpecElem *d = (SpecElem *)elems->Get(i);
	    const char *c = 0;
	    StrPtr *v;
	    StrBuf addTabs; 

	    // if no data for this line, don't display it
	    // Special case for DEFAULT: we'll display a blank entry
	    // for the user (we don't put in the default value, tho).

	    if( !( v = data->GetLine( d, 0, &c ) ) &&
	        !( d->opt == SDO_DEFAULT || d->opt == SDO_EMPTY ) )
	        continue;

	    // Blanks between sections

	    if( s->Length() )
	        *s << "\n";

	    // Output format depends on type

	    int j = 0;

	    switch( d->type )
	    {
	    case SDT_WORD:	// single line, N words
	    case SDT_SELECT:	// SDT_WORD from a list of words
	    case SDT_LINE:	// single line of text (arbitrary words)
	    case SDT_DATE:	// SDT_LINE that is a date

	        // Tag: value

	        *s << d->tag << ":";
	        if( v ) *s << "\t" << v;
	        if( c ) *s << "\t# " << c;
	        *s << "\n";
	        break;

	    case SDT_WLIST:	// multiple lines, N words
	    case SDT_LLIST:	// multiple lines of text (arbitrary words)

	        // Tag:\n\tvalue1\n\tvalue2...

	        *s << d->tag << ":\n";

	        // This code is changed in p19.2 and forward ported to 20.1.
	        // For these servers, specific SpecMgrs handle
	        // the data strings SpecElems of type SDT_LLIST/SDT_WLIST.
	        //
	        // Custom spec SDT_LLIST/SDT_WLIST fields invoke this code.
	        // job102389 revealed that tabs were not inserted in the
	        // output form for multiline SDT_LLIST/SDT_WLIST elems.
	        //
	        // The 'while()' section below appears to expect
	        // SpecData->GetLine() to be used iteratively to return
	        // '\n' separated substrings. It does not, and 
	        // and instead returns the entire string.
	        //
	        // It is a larger task to fulfill that expectation, so 
	        // explicitly insert tab after newline for SDT_LLIST/SDT_WLIST.
	        // Were GetLine() to be fixed, the following fix is a NOOP.

	        if( v && ( d->type == SDT_LLIST || d->type == SDT_WLIST ) &&
	            v->Contains( nl ) && !v->Contains( nltab ) )
	        {
	            StrOps::Replace( addTabs, *v, nl, nltab );
	            v = &addTabs;
	        }


	        while( v )
	        {
	            // 2016.2 Handle comments:  
	            // Comments can be
	            // (1) Single line ( v->Length() == 0 )
	            // (2) Appended ( v->Length() > 0 && fmt:C )

	            while( v && !c && !v->Length() )
	                v = data->GetLine( d, ++j, &c );
	            if( !v )
	                break;

	            *s << "\t" << v;

	            if( c && v->Length() && ( d->fmt == SDF_COMMENT ) )
	                *s << "\t##" << c;
	            else if( c && v->Length() ) 
	                *s << "\t# " << c;
	            else if( c ) 
	                *s << "##" << c;

	            *s << "\n";

	            v = data->GetLine( d, ++j, &c );
	        }
	        break;

	    case SDT_TEXT:	// block of text,
	    case SDT_BULK:	// SDT_TEXT not indexed

	        // Tag:\n\tblock

	        *s << d->tag << ":\n";
	        if( v ) StrOps::Indent( *s, *v );
	        break;
	    }
	}
}

void
Spec::Format( SpecData *data, StrDict *dict )
{
	// just dump each element into a dictionary
	// tag name is var name
	// may be multiple vars with same tag

	for( int i = 0; i < elems->Count(); i++ )
	{
	    SpecElem *d = (SpecElem *)elems->Get(i);
	    const char *c = 0;
	    StrPtr *v;
	    int j;

	    // if no data for this line, don't display it

	    if( d->IsList() )
	    {
	        for( j = 0; ( v = data->GetLine( d, j, &c ) ); j++ ) 
	            dict->SetVar( d->tag, j, *v );
	    }
	    else
	    {
	        if( ( v = data->GetLine( d, 0, &c ) ) )
	            dict->SetVar( d->tag, *v );
	    }
	}
}

/**
 * SpecData::SetExtraTag -
 * Records an extra (non-schema) tag and its string value in the output
 * dictionary 'rh'. 
 *
 * @param[out] rh    - Output string dictionary to write the tag into.
 * @param[in]  name  - Name (key) of the extra tag field.
 * @param[in]  index - Zero-based position in the extraTag index arrays;
 *                     -1 to exclude extraTag# and extraTagType# from output.
 * @param[in]  type  - Type hint string for the field (e.g. "word", "date").
 * @param[in]  value - String value to associate with the tag.
 */
void
SpecData::SetExtraTag( StrDict& rh, const StrRef& name, const int index, 
	               const StrRef& type, const StrRef& value ) const
{
	SetExtraTagCommon( rh, name, index, type );
	rh.SetVar( name, value );
}

/**
 * SpecData::SetExtraTag -
 * Bool overload.  Records an extra (non-schema) tag and its boolean value
 * in the output dictionary 'rh'.
 *
 * @param[out] rh    - Output string dictionary to write the tag into.
 * @param[in]  name  - Name (key) of the extra tag field.
 * @param[in]  index - Zero-based position in the extraTag index arrays;
 *                     -1 to exclude extraTag# and extraTagType# from output.
 * @param[in]  type  - Type hint string for the field.
 * @param[in]  value - Boolean value to associate with the tag.
 */
void
SpecData::SetExtraTag( StrDict& rh, const StrRef& name, const int index, 
	               const StrRef& type, const bool value ) const
{
	SetExtraTagCommon( rh, name, index, type );
	rh.SetVar( name.Text(), value );
}

/**
 * SpecData::SetExtraTag -
 * Integer overload.  Records an extra (non-schema) tag and its int value
 * in the output dictionary 'rh'.
 *
 * @param[out] rh    - Output string dictionary to write the tag into.
 * @param[in]  name  - Name (key) of the extra tag field.
 * @param[in]  index - Zero-based position in the extraTag index arrays;
 *                     -1 to exclude extraTag# and extraTagType# from output.
 * @param[in]  type  - Type hint string for the field.
 * @param[in]  value - Integer value to associate with the tag.
 */
void
SpecData::SetExtraTag( StrDict& rh, const StrRef& name, const int index, 
	               const StrRef& type, const int value ) const
{
	SetExtraTagCommon( rh, name, index, type );
	rh.SetVar( name.Text(), value );
}

# ifdef HAVE_INT64
/**
 * SpecData::SetExtraTag -
 * Long integer overload.  Records an extra (non-schema) tag and its long
 * value in the output dictionary 'rh'.
 *
 * @param[out] rh    - Output string dictionary to write the tag into.
 * @param[in]  name  - Name (key) of the extra tag field.
 * @param[in]  index - Zero-based position in the extraTag index arrays;
 *                     -1 to exclude extraTag# and extraTagType# from output.
 * @param[in]  type  - Type hint string for the field.
 * @param[in]  value - Long integer value to associate with the tag.
 */
void
SpecData::SetExtraTag( StrDict& rh, const StrRef& name, const int index, 
	               const StrRef& type, const long value ) const
{
	SetExtraTagCommon( rh, name, index, type );
	rh.SetVar( name.Text(), value );
}

/**
 * SpecData::SetExtraTag -
 * P4INT64 overload.  Records an extra (non-schema) tag and its 64-bit
 * integer value in the output dictionary 'rh'.
 *
 * @param[out] rh    - Output string dictionary to write the tag into.
 * @param[in]  name  - Name (key) of the extra tag field.
 * @param[in]  index - Zero-based position in the extraTag index arrays;
 *                     -1 to exclude extraTag# and extraTagType# from output.
 * @param[in]  type  - Type hint string for the field.
 * @param[in]  value - 64-bit integer value to associate with the tag.
 */
void
SpecData::SetExtraTag( StrDict& rh, const StrRef& name, const int index, 
	               const StrRef& type, const P4INT64 value ) const
{
	SetExtraTagCommon( rh, name, index, type );
	rh.SetVar( name.Text(), value );
}
# endif

/**
 * SpecData::SetExtraTagCommon -
 * Shared helper used by all SetExtraTag overloads.  Consolidates handling
 * of extraTag# and extraTagType#.
 *
 * @param[out] rh    - Output string dictionary to write the metadata into.
 * @param[in]  name  - Name of the extra tag to register.
 * @param[in]  index - Zero-based slot in the extraTag index arrays;
 *                     -1 to exclude extraTag# and extraTagType# from output.
 * @param[in]  type  - Type hint string for the tag (e.g. "word", "date").
 */
void
SpecData::SetExtraTagCommon( StrDict& rh, const StrRef& name, 
	                     const int index, const StrRef& type ) const
{
	if( index >= 0 )
	{
	    rh.SetVar( P4Tag::v_extraTag, index, name );
	    rh.SetVar( P4Tag::v_extraTagType, index, type );
	}
}

/**
 * SpecData::IsSystemField -
 * Returns true when the given spec element should be treated as a system
 * field.
 *
 * @param[in] sd - Spec element to test.
 * @return bool  - true if 'sd' is a system field; false otherwise.
 */
bool
SpecData::IsSystemField( const SpecElem& sd ) const
{
	return sd.code <= GetMaxSystemSpecCode() || sd.fixed.Length() > 0;
}