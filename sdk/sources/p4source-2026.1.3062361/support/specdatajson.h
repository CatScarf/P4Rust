/*
 * Copyright 2025 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# if defined( HAS_CPP11 ) && !defined( HAS_BROKEN_CPP11 )
#define HAS_JSON
# include <json.hpp>
using json = nlohmann::json;
# endif

class JsonSpecElemNameLookup {
public:
    virtual ~JsonSpecElemNameLookup() {}

    /**
     * JsonSpecElemNameLookup::GetElemNameByCode -
     *	Maps an integer field code to its JSON name.
     *
     * @param[in] code       - Integer field code to look up.
     * @return const StrPtr& - JSON name, or StrRef::Null() if not found.
     */
    virtual const StrPtr& GetElemNameByCode( int code ) const = 0;

    /**
     * JsonSpecElemNameLookup::TranslateElemName -
     *	Maps a spec field name string to its JSON equivalent.
     *
     * @param[in] srcName    - Spec field name to translate.
     * @return const StrPtr& - JSON name, or srcName if no mapping exists.
     */
    virtual const StrPtr& TranslateElemName( const StrPtr& srcName ) = 0;
};

class JsonSpecHandling {
public:
    virtual ~JsonSpecHandling() {}

#ifdef HAS_JSON
    /**
     * JsonSpecHandling::SetJsonField -
     * Specific SpecData implementations can override this to handle specific
     * fields. For example, ChangeSpecData can know that CHANGE_SPEC_CHANGE is
     * typically numeric and it can set a json field as such rather than
     * setting a text value for that numeric field.
     *
     * @param[in] sd    - SpecDataElement that is currently being parsed.
     * @param[in] x     - Index of this element in an array (if in an array)
     * @param[in] wv    - Null terminated array of text values to parse for 
     *                    this element.
     * @param[in/out] ioJson - Reference to the JSON object to set data on
     * @param[out] e    - Overall status of this operation
     * @return bool     - Returns true if a JSON field was set from the spec 
     *                    data. Returns false otherwise.
     */
    virtual bool        SetJsonField( SpecElem* sd, int x, const char** wv,
	                              json& ioJson, Error* e ) const = 0;
#endif // HAS_JSON

    const StrPtr&       GetJsonFieldName( const SpecElem& sd ) const;
    const StrPtr&       GetJsonFieldName( const StrPtr& srcName );

protected:
    /**
     * JsonSpecHandling::GetJsonNameLookup -
     *	Returns the const name lookup table for this spec type.
     *
     * @return const JsonSpecElemNameLookup& - The name lookup table.
     */
    virtual const JsonSpecElemNameLookup&   GetJsonNameLookup() const = 0;

    /**
     * JsonSpecHandling::GetJsonNameLookupNonConst -
     *	Returns a mutable reference to the name lookup table for this spec type.
     *	Required for non-const operations such as TranslateElemName.
     *
     * @return JsonSpecElemNameLookup& - The mutable name lookup table.
     */
    virtual JsonSpecElemNameLookup&         GetJsonNameLookupNonConst() = 0;
};

 // This class exists to translate between the text based forms && JSON data
 // structures. In order for this to properly function, HAS_JSON must be
 // defined.
class JsonSpecData : public SpecData {

public:
    JsonSpecData( SpecData* srcSpecData );
    virtual ~JsonSpecData();

# if defined( HAS_CPP11 ) && !defined( HAS_BROKEN_CPP11 )
    // Ensure this class can't be copied, moved, or assigned to avoid
    // potentially leaking the srcSpecData
    JsonSpecData( const JsonSpecData& ) = delete;
    JsonSpecData( JsonSpecData&& ) = delete;
    JsonSpecData& operator=( const JsonSpecData& ) = delete;
    JsonSpecData& operator=( JsonSpecData&& ) = delete;
#endif

    virtual void        Set( SpecElem* sd, int x, const char** wv, Error* e );
    virtual void        SetComment( SpecElem* sd,
	                            int x,
	                            const StrPtr* val,
	                            int nl,
	                            Error* e );

    void                Finalize( StrDict& dict, const char* keyName, 
				  Error& e );

    StrDict*            GetExtraTagDict() const { return jsonExtraTags; }

    virtual void        SetExtraTag( StrDict& rh,
	                             const StrRef& name,
	                             const int index,
	                             const StrRef& type,
	                             const StrRef& value ) const;

    const SpecData&     GetSrcSpecData() const { return *srcSpecData; }

#ifdef HAS_JSON
    static json&        GetArray( json& jsonObj, const char* key );
    static void         SetISO8601Date( json& jsonObj,
	                                const char* keyName,
	                                const char* dateFromFmt,
	                                Error& e );
#endif // HAS_JSON

    static const StrPtr& GetFieldName( const SpecData& sd, 
	                               const SpecElem& se );
    static const StrPtr& GetFieldName( SpecData& sd, 
	                               const StrPtr& srcName );
    const StrPtr&        GetFieldName( const SpecElem& se ) const;
    const StrPtr&        GetFieldName( const StrPtr& srcName );
private:

    void                SetWordListComment( const char* fieldName,
	                                    const int index,
	                                    const char* comment );
    void                UpdateArrayOffset( const int nl );
    void                CheckResetArrayOffset( const StrPtr& curArray );

#ifdef HAS_JSON
    json&               GetArray( const char* key );
    void                SetWordListData( const char* fieldName,
	                                 const int index,
	                                 json& dataArray );
    int	                GetAdjustedIndex( const json& array,
	                                  const int index ) const;

    // This holds the JSON data we are building up or parsing.
    json                jsonData;
#endif // HAS_JSON

    // If we support JSON data, this is set to an instance of JsonExtraTags
    // that will collect the extra tag information, it is NULL otherwise.
    StrDict*            jsonExtraTags;

    // We own this reference to the original spec data object and must ensure
    // it gets properly cleaned up.
    SpecData*           srcSpecData;

    // This 'arrayName' & 'arrayOffset' exists because of a very long 
    // standing issue in Spec::Parse when encountering inline comments it
    // increments the 'index' of the array even though an inline comment
    // shouldn't result in a new index. See UpdateArrayOffset for more
    // details.
    StrBuf              arrayName;
    int	                arrayOffset;
};