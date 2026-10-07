/*
 * Copyright 2025 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# include <stdhdrs.h>
# include <charman.h>
# include <debug.h>

# include <strbuf.h>
# include <strdict.h>
# include <strtree.h>

# include <error.h>
# include <errorlog.h>

# include "spec.h"
# include <msgdb.h>
# include <msgsupp.h>
# include <msgserver2.h>
# include <p4tags.h>
# include <datetime.h>

# if defined( HAS_CPP11 ) && !defined( HAS_BROKEN_CPP11 )
#define HAS_JSON
# include <json.hpp>
# include <strops.h>
using json = nlohmann::json;
# endif

# include "specdatajson.h"


#ifdef HAS_JSON
class  JsonExtraTags : public StrDict {

public:
    /**
     * JsonExtraTags::JsonExtraTags -
     * Constructor
     *
     * @param[in] inJsonObj - Reference to a JSON object that will be populate
     *	                      with extra tag data
     */
    JsonExtraTags( JsonSpecData& inSpecData, json& inJsonObj )
	: jsonSpecData( inSpecData ),
	  jsonObj( inJsonObj )
    {
	extraTagLen = strlen( P4Tag::v_extraTag );
    }

protected:
    /**
     * JsonExtraTags::VGetVar -
     * Always returns null. This is a write-only class.
     *
     * @param[in] var - Dictionary key
     * @return StrPtr* - Always returns NULL
     */
    StrPtr* VGetVar( const StrPtr& var )
    {
	// This is a write-only operation... no need to return anything
	// from this.
	return NULL;
    }

    /**
     * JsonExtraTags::VSetVarTyped -
     * Template helper that writes a typed value into the JSON object under
     * the given key.
     *
     * @param[in] var - Dictionary key; entries starting with 'ExtraTag' are
     *                  silently ignored.
     * @param[in] val - Typed value to store; must be serialisable by
     *                  nlohmann::json.
     */
    template<typename T>
    void VSetVarTyped( const StrPtr& var, const T& val )
    {
	// var's that start with 'ExtraTag' are defining metadata about
	// tags not defined in the specification definition. Eg:
	//  extraTag0: firmerThanParent
	//  extraTagType0: word
	//  firmerThanParent: someValue
	// We do not want to save that extra metadata to the JSON, but will
	// instead need to figure out how to include that information in
	// the specification.
	if( !var.StartsWith( P4Tag::v_extraTag, extraTagLen ) )
	{
	    const StrPtr& varName = jsonSpecData.GetFieldName( var );
	    jsonObj[ varName.Text() ] = val;
	}
    }

    /**
     * JsonExtraTags::VSetVar -
     * Only saves values _not_ starting with 'ExtraTag'
     *
     * @param[in] var - Dictionary key
     * @param[in] val - Dictionary value for key
     */
    void VSetVar( const StrPtr& var, const StrPtr& val )
    {
	VSetVarTyped( var, val.Text() );
    }

    /**
     * JsonExtraTags::VSetVar -
     * Only saves values _not_ starting with 'ExtraTag'
     *
     * @param[in] var - Dictionary key
     * @param[in] val - Dictionary value for key
     */
    void VSetVar( const StrPtr& var, int val )
    {
	VSetVarTyped( var, val );
    }

    /**
     * JsonExtraTags::VSetVar -
     * Only saves values _not_ starting with 'ExtraTag'
     *
     * @param[in] var - Dictionary key
     * @param[in] val - Dictionary value for key
     */
    void VSetVar( const StrPtr& var, bool val )
    {
	VSetVarTyped( var, val );
    }

# ifdef HAVE_INT64
    /**
     * JsonExtraTags::VSetVar -
     * Only saves values _not_ starting with 'ExtraTag'
     *
     * @param[in] var - Dictionary key
     * @param[in] val - Dictionary value for key
     */
    void VSetVar( const StrPtr& var, long val )
    {
	VSetVarTyped( var, val );
    }

    /**
     * JsonExtraTags::VSetVar -
     * Only saves values _not_ starting with 'ExtraTag'
     *
     * @param[in] var - Dictionary key
     * @param[in] val - Dictionary value for key
     */
    void VSetVar( const StrPtr& var, P4INT64 val )
    {
	VSetVarTyped( var, val );
    }
# endif

    /**
     * JsonExtraTags::VGetCount -
     * Returns the current size of the embedded jsonObject.
     *
     * @return int - Current size of the embedded jsonObject.
     */
    int VGetCount()
    {
	return jsonObj.size();
    }

private:
    JsonSpecData&           jsonSpecData;
    json&                   jsonObj;
    size_t                  extraTagLen;
};
#endif // HAS_JSON

/**
 * JsonSpecData::JsonSpecData -
 * Constructor
 *
 * @param[in] inSpecData - Type specific SpecData object. This takes ownership
 *	                   of the SpecData object and ensures it gets deleted
 */
JsonSpecData::JsonSpecData( SpecData* inSpecData )
	:
#ifdef HAS_JSON
	  jsonData(json::object()),
	  jsonExtraTags( new JsonExtraTags(*this, jsonData ) ),
#else
	  jsonExtraTags( NULL ),
#endif
	  srcSpecData( inSpecData ),
	  arrayOffset( 0 )
{
}

/**
 * JsonSpecData::~JsonSpecData -
 * Destructor
 */
JsonSpecData::~JsonSpecData()
{
	delete jsonExtraTags;
	delete srcSpecData;
}

/**
 * JsonSpecData::Set -
 * Sets data onto the json object, ideally with appropriate typing
 *
 * @param[in] sd    - SpecElem with type information
 * @param[in] x     - Array index for the wv
 * @param[in] wv    - List of text values, value we need is typically at index
 *	              0. Last element in list is a null value.
 * @param[out] e    - Overall status of the operation
 */
void
JsonSpecData::Set( SpecElem* sd, int x, const char** wv, Error* e )
{
#ifndef HAS_JSON
	e->Set( MsgSupp::JsonNotEnabled );
#else
	const JsonSpecHandling* srcSpecWithJson =
	    dynamic_cast<const JsonSpecHandling*>( srcSpecData );
	if( srcSpecWithJson &&
	    srcSpecWithJson->SetJsonField( sd, x, wv, jsonData, e ))
	{
	    // We have defined custom handling for whatever this was.
	    return; 
	}

	json& dstObj = jsonData;
	if( !srcSpecData->IsSystemField( *sd ) )
	{
	    dstObj = jsonData[ "customFields" ];
	}

	const StrPtr& fieldName = GetFieldName( *sd );

	if( sd->IsList() )
	{
	    CheckResetArrayOffset( fieldName );

	    // The wv represents some kind of a list, possibly a list of words
	    // or possibly just a new line.
	    if( sd->IsWords() )
	    {
	        json wordArray = json::array();
	        const char** tmp = wv;
	        while( *tmp != 0 )
	        {
	            wordArray.push_back( *tmp );
	            tmp++;
	        }
	        SetWordListData( fieldName.Text(), x, wordArray );
	    }
	    else
	    {
	        json& jArray = GetArray( fieldName.Text() );
	        jArray.push_back( *wv );
	    }
	}
	else if( sd->IsDate() )
	{
	    // The wv is a date value, render it as ISO8601
	    JsonSpecData::SetISO8601Date( jsonData, fieldName.Text(), *wv, *e );

	}
	else if( sd->IsLine() && sd->values.Length() > 0 )
	{
	    // This primarily is for 'Option' lines
	    StrBuf b;
	    char* words[ 10 ];
	    int nWords = StrOps::Words( b, *wv, words, 10 );

	    jsonData[ fieldName.Text() ] = json::array();
	    for( int i = 0; i < nWords; ++i )
	    {
	        jsonData[ fieldName.Text() ].push_back( words[ i ] );
	    }
	}
	else
	{
	    // srcSpecData didn't have any special handling, and we don't
	    // have any errors, so lets just save this value as text.
	    jsonData[ fieldName.Text() ] = *wv;
	}
#endif // HAS_JSON
}

/**
 * JsonSpecData::SetComment -
 * Sets comments in the appropriate location on the stored JSON object.
 *
 * @param[in] sd  - SpecElem with type information
 * @param[in] x   - Array index provided by Spec::Parse, see UpdateArrayOffset
 * @param[in] val - Comment value
 * @param[in] nl  - 1 = new line comment, 0 = inline comment
 * @param[out] e  - Overall status of the operation
 */
void
JsonSpecData::SetComment( SpecElem* sd, int x, const StrPtr* val, int nl, 
	                  Error* e )
{
#ifndef HAS_JSON
	e->Set( MsgSupp::JsonNotEnabled );
#else
	const StrPtr& fieldName = GetFieldName( *sd );

	if( sd->IsList() )
	{
	    CheckResetArrayOffset( fieldName );

	    if( sd->IsWords() )
	    {
	        UpdateArrayOffset( nl );

	        SetWordListComment( fieldName.Text(),
	                            x,
	                            val->Text() );


	    }
	    else
	    {
	        json& jArray = GetArray( fieldName.Text() );
	        jArray.push_back( val->Text() );
	    }
	}
	else
	{
	    StrBuf name;
	    name << fieldName << "Comment";

	    jsonData[ name.Text() ] = val->Text();

	}
#endif // HAS_JSON
}

/**
 * JsonSpecData::Finalize -
 * Spec::Parse has been building up a json object, we need to render that
 * object to a text value. This function serializes the jsonData to text.
 *
 * @param[in] dict - Where to send the serialized data
 * @param[in] keyName - Name of the key to set on the provided dict
 * @param[out] e - Overall status of the operation
 */
void
JsonSpecData::Finalize( StrDict& dict, const char* keyName, Error& e )
{
#ifdef HAS_JSON
    try 
    {
	dict.SetVar( keyName, jsonData.dump().c_str() );
    }
    catch( nlohmann::json::type_error& ex )
    {
	e.Set( MsgSupp::JsonSerializationFailed )
	    << ex.what();
    }
#endif
}


/**
* JsonSpecData::SetExtraTag -
* JSON-aware override of SpecData::SetExtraTag for string values. This is
* needed to properly reserialize dates to ISO8601
*
* @param[out] rh    - Output string dictionary to set the data on.
* @param[in]  name  - Name of the extra tag field.
* @param[in]  index - (unused) Index of the extra tag field.
* @param[in]  type  - Data type hint for the field (e.g. "date").
* @param[in]  value - Text value of the field.
*/
void
JsonSpecData::SetExtraTag( StrDict& rh, const StrRef& name, const int, 
	                   const StrRef& type, const StrRef& value ) const
{
	// In JSON a date field should be written as ISO8601, not a display
	// format.
	if( type.Compare( StrRef( "date" ) ) == 0 )
	{
	    // Parse the date
	    Error e;
	    DateTime parsed( value.Text(), &e );

	    // This really, really should not error out. We are parsing a date
	    // that the server itself has just rendered into a text.
	    if( e.Test() )
	    {
	        AssertLog.Report( &e );
	        rh.SetVar( name, value );
	    }
	    else
	    {
	        // Serialize the date as ISO8601
	        char bufTime[ 32 ];
	        parsed.FmtISO8601( bufTime );
	        rh.SetVar( name.Text(), bufTime );
	    }
	}
	else
	{
	    // All other types are passed through to the output.
	    rh.SetVar( name, value );
	}
}

/**
 * JsonSpecData::SetWordListComment -
 * Word lists can be created either by Set or SetComment. If Set created the
 * word list first, then this will merge in the comment to the existing
 * object
 *
 * @param[in out] fieldName - Name of the word list array
 * @param[in out] index     - Index provided by SetComment
 * @param[in out] comment   - Comment value to set
 */
void
JsonSpecData::SetWordListComment( const char* fieldName,
	                          const int index,
	                          const char* comment )
{
#ifdef HAS_JSON
	json& wordArray = GetArray( fieldName );
	int adjustedIndex = GetAdjustedIndex( wordArray, index );

	// If the wordArray size is less than, or equal to the index, 
	// this is a new value that hasn't been seen before.
	if( wordArray.size() == adjustedIndex )
	{
	    json newObj = json::object();
	    newObj[ "comment" ] = comment;
	    wordArray.push_back( newObj );
	}
	else
	{
	    // Else... we should have the object created already, and now we are going to
	    // set the comment on it.
	    wordArray[ adjustedIndex ][ "comment" ] = comment;
	}
#endif
}

/**
 * JsonSpecData::UpdateArrayOffset -
 * This exists because the Spec::Parse function has a bug where even if we are
 * NOT a new line comment, it increments the 'index' of the array. So we need
 * to ensure we set data based on the _real_ array index as opposed to the fake
 * one.
 *
 * I would prefer to fix the core issue in Spec::Parse, however it appears that
 * other code on the server is already hacking around the issue and an attempt
 * to 'fix the glitch' resulted in a cascade of other code changes. So for now
 * we live with the problem.
 *
 * This function updates an 'arrayOffset' value that keeps track how out of
 * sync Spec::Parse is from reality.
 *
 * Other functions that use or update the arrayOffset include:
 * - GetAdjustedIndex
 * - CheckResetArrayOffset
 *
 * @param[in] nl - Is this a new line? If so, Spec:Parse incremented the array
 *	           index even when there was new array value.
 */
void
JsonSpecData::UpdateArrayOffset( const int nl )
{
	if( !nl )
	{
	    // Note: This needs to happen before SetWordListComment
	    //       because SetWordList* uses the arrayOffset
	    // Note2: We can get away with using a single arrayOffset because
	    //    Spec::Parse fully processes a single array at a time.
	    //    CheckResetArrayOffset monitors to see if we switch arrays
	    //    and if we do it resets the arrayOffset back to 0.
	    //    If at some future date Spec::Parse starts processing arrays
	    //    in parallel or swapping back & forth between arrays, then we
	    //    will need to add more thorough offset tracking, maybe a map
	    //    of Name -> offset.
	    arrayOffset--;
	}
}


/**
 * JsonSpecData::CheckResetArrayOffset -
 * See UpdateArrayOffset for a detailed description of why this exists.
 *
 * This function monitors for the current array being processed changing, on
 * an array change it resets the arrayOffset to 0.
 *
 * @param[in] curArray - Tag name of the current array being processed
 */
void
JsonSpecData::CheckResetArrayOffset( const StrPtr& curArray )
{
	if( curArray.Compare( arrayName ) != 0 )
	{
	    arrayOffset = 0;
	    arrayName.Set( curArray );
	}
}

/**
 * JsonSpecData::GetFieldName -
 *  Resolves the JSON output name for a spec field.
 *
 * @param[in] se    - SpecElem to resolve the JSON name for.
 * @return StrPtr&  - JSON field name to use for output.
 */
const StrPtr&
JsonSpecData::GetFieldName( const SpecElem& se ) const
{
	return GetFieldName( GetSrcSpecData(), se );
}

/**
 * JsonSpecData::GetFieldName -
 *  Translates srcName to its JSON equivalent via JsonSpecHandling if
 *  the srcSpecData supports it.
 *  If no translation is found, returns srcName.
 *
 * @param[in] srcName    - Spec field name to translate.
 * @return const StrPtr& - JSON field name, or srcName if no mapping exists.
 */
const StrPtr&
JsonSpecData::GetFieldName( const StrPtr& srcName )
{
	return GetFieldName( *srcSpecData, srcName );
}

/**
 * JsonSpecData::GetFieldName -
 * Resolves the JSON output name for a spec field.
 *
 * @param[in] sd        - SpecData to query for JsonSpecHandling support.
 * @param[in] se        - SpecElem whose JSON name is being resolved.
 * @return const StrPtr& - JSON key name for the SpecElem
 */
const StrPtr&
JsonSpecData::GetFieldName( const SpecData& sd, const SpecElem& se )
{
	// Next, lets see if we have setup this source specification to
	// know how to translate field codes to fixed field names.
	if( const JsonSpecHandling* jsh = dynamic_cast<const JsonSpecHandling*>( &sd ) )
	{
	    const StrPtr& result = jsh->GetJsonFieldName( se );
	    if( result.Length() > 0 )
	    {
	        return result;
	    }
	}

	// Hitting this point means one of a few possible situations:
	// 1. There isn't any official JSON handling for this spec type because
	//    it hasn't been added to the REST API yet.
	// 2. There is a new system field and someone forgot to add it to the
	//    appropriate JsonSpecHandling implementation.
	// 3. This is a customer defined field that we know nothing about.

	if( sd.IsSystemField(se) )
	{
	    // We are in either 1 or 2, so we are going to report a warning
	    // The t4 tests can check for this warning and fail the tests
	    // if we see it.
	    Error e;
	    e.Set( MsgSupp::NoJsonKeyTranslation ) << se.tag.Text();
	    AssertLog.Report( &e );
	}

	// If this is a _new_ system  field, then 'fixed' will probably be set
	// and we can at least return a constant field name for it, although
	// it is likely the case will not be camelCase.
	if( se.fixed.Length() > 0 )
	{
	    return se.fixed;
	}

	// Finally, as a last ditch fallback we will just return the tag name
	// we have for the field.
	return se.tag;
}

/**
 * JsonSpecData::GetFieldName -
 *  Translates srcName to its JSON equivalent via JsonSpecHandling if the
 *  SpecData object supports it.
 *  If no translation is found, returns srcName.
 *
 * @param[in] sd      - SpecData to query for JsonSpecHandling support.
 * @param[in] srcName - Spec field name to translate.
 * @return const StrPtr& - JSON name, or srcName if no mapping exists.
 */
const StrPtr&
JsonSpecData::GetFieldName( SpecData& sd, const StrPtr& srcName )
{
	// Next, lets see if we have setup this source specification to
	// know how to translate field codes to fixed field names.
	if( JsonSpecHandling* jsh = dynamic_cast<JsonSpecHandling*>( &sd ) )
	{
	    const StrPtr& result = jsh->GetJsonFieldName( srcName );
	    if( result.Length() > 0 )
	    {
	        return result;
	    }
	}
	return srcName;
}

#ifdef HAS_JSON

/**
 * JsonSpecData::GetArray -
 * Returns a reference to an array with the provided key name. If an array
 * does not exist, it creates one first.
 *
 * @param[in] key - Name of the array to return a reference too
 * @return json - Returns a reference to a json array
 */
json&
JsonSpecData::GetArray( const char* key )
{
	return JsonSpecData::GetArray( jsonData, key );
}

/**
 * JsonSpecData::GetArray -
 * Static overload that retrieves (or creates & retrieves) the JSON array 
 * stored under 'key' within the provided JSON object.
 *
 * @param[in,out] jsonObj - JSON object to look up or create the array in.
 * @param[in]     key     - Property name of the array within jsonObj.
 * @return json& - Reference to the (possibly newly created) JSON array.
 */
json&
JsonSpecData::GetArray( json& jsonObj, const char* key )
{
	if( !jsonObj[ key ].is_array() )
	{
	    jsonObj[ key ] = json::array();
	}

	return jsonObj[ key ];
}

/**
 * JsonSpecData::SetISO8601Date -
 * Parses a date string produced by DateTime::Fmt (e.g. "2026/03/02 16:10:45")
 * and writes it to the given JSON object as an ISO8601 string
 * (e.g. "2026-03-02T16:10:45Z").
 *
 * @param[out] jsonObj      - JSON object to set the property on.
 * @param[in]  keyName      - Name of the JSON property to set.
 * @param[in]  dateFromFmt  - Date string in DateTime::Fmt format,
 *                            e.g. "2026/03/02 16:10:45".
 * @param[out] e            - Receives any parse error; if set, no property
 *                            is written.
 */
void
JsonSpecData::SetISO8601Date( json& jsonObj, const char* keyName, 
	                      const char* dateFromFmt, Error& e )
{
    DateTime dt( dateFromFmt, &e );
    if( !e.Test() )
    {
	char bufTime[ 32 ];
	dt.FmtISO8601( bufTime );
	jsonObj[ keyName ] = bufTime;
    }
    else 
    {
	jsonObj[ keyName ] = dateFromFmt;
    }
}

/**
 * JsonSpecData::SetWordListData -
 * Word lists can be created either by Set or SetComment. If SetComment created
 * the word list first, then this will merge in the 'data' to the existing
 * object.
 *
 * @param[in] fieldName - Name of the word list array
 * @param[in] index     - Index provided by Set
 * @param[in] dataArray - JSON array of data
 */
void
JsonSpecData::SetWordListData( const char* fieldName,
	                       const int index,
	                       json& dataArray )
{
	json& wordArray = GetArray( fieldName );
	int adjustedIndex = GetAdjustedIndex( wordArray, index );

	// If the wordArray size is equal to the index, this is a new value
	// that hasn't been seen before.
	if( wordArray.size() == adjustedIndex )
	{
	    json newObj = json::object();
	    newObj[ "data" ] = dataArray;
	    wordArray.push_back( newObj );
	}
	else
	{
	    // Else... we should have the object created already, and now we 
	    // are going to set the data on it.
	    wordArray[ adjustedIndex ][ "data" ] = dataArray;
	}
}

/**
 * JsonSpecData::GetAdjustedIndex -
 * See UpdateArrayOffset for a detailed description of why this exists.
 *
 * This function returns the adjusted index using the current arrayOffset.
 *
 * It also performs a sanity check to make sure that the adjusted index is at
 * most one more than the current size of the array
 *
 * @param[in] array - Array we are getting an adjusted index for, this is only
 *	              used as a sanity check to ensure we are not generating
 *	              an invalid adjusted index
 * @param[in] index - Index provided by Spec::Parse
 * @return int      - Actual adjusted index based on the arrayOffset
 */
int
JsonSpecData::GetAdjustedIndex( const json& array, const int index ) const
{
	int adjustedIndex = index + arrayOffset;

	// The new, adjusted index must be either inside the array's bounds
	// or at most 1 higher (indicating a new value). This should not be
	// able to happen unless there is a bug, hence the DevErr & report
	if( adjustedIndex < 0 ||
	    adjustedIndex > array.size() )
	{
	    Error e;
	    e.Set( MsgDb::DevErr ) <<
	        "Adjusted JSON array index out of bounds.";
	    AssertLog.Report( &e );

	    // Clamp the adjusted index to either the front or end of the array.
	    if( adjustedIndex < 0 )
	    {
	        adjustedIndex = 0;
	    }
	    else
	    {
	        adjustedIndex = array.size();
	    }
	}

	return adjustedIndex;
}

#endif // HAS_JSON

/**
 * JsonSpecHandling::GetJsonFieldName -
 *  Returns the JSON name for a field by its integer code
 *
 * @param[in] sd         - SpecElem whose code is used for the lookup.
 * @return const StrPtr& - JSON field name, or StrRef::Null() if not found.
 */
const StrPtr&
JsonSpecHandling::GetJsonFieldName( const SpecElem& sd ) const
{
	return GetJsonNameLookup().GetElemNameByCode( sd.code );
}

/**
 * JsonSpecHandling::GetJsonFieldName -
 *  Translates a spec field string name to its JSON equivalent
 *
 * @param[in] srcName    - Spec field name to translate.
 * @return const StrPtr& - JSON name, or srcName itself if no mapping exists.
 */
const StrPtr&
JsonSpecHandling::GetJsonFieldName( const StrPtr& srcName )
{
	return GetJsonNameLookupNonConst().TranslateElemName( srcName );
}