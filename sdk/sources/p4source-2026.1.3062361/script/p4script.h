/*
 * Copyright 2018 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# ifndef P4SCRIPT_H
# define P4SCRIPT_H

enum SCR_VERSION
{
	P4SCRIPT_UNKNOWN,
	P4SCRIPT_LUA_53
};

enum SCR_BINDING_LIBNAME
{
	P4SCRIPT_CLIENTAPI,
	P4SCRIPT_CLIENTUSER,
	P4SCRIPT_LUA53
};

# ifdef HAS_EXTENSIONS

enum class SCR_BINDING_LUA_OPTS
{
	OS_EXIT
};

# include <chrono>
# include <future>
# include <p4_any.h>
# include <vector>
# include <functional>
# include <unordered_map>

using scriptClock_t   = std::chrono::steady_clock;
using scriptTime_t    = scriptClock_t::duration;
using scriptTimeInc_t = std::chrono::time_point< scriptClock_t >;
using scriptMem_t     = uint32_t;

class p4scriptImpl53;
class ClientApi;
class ClientApiLua;
class ClientUserLua;
class FileSys;
class DateTimeHighPrecision;
class StrBuf;
class Error;

class p4script
{
	public:

	    virtual ~p4script();
	     p4script( const SCR_VERSION v, const int apiVersion, Error *e );

	    void SetMaxTime( const uint32_t max );
	    // Limit memory to N megabytes.
	    void SetMaxMem ( const scriptMem_t max );

	    enum class DEBUG
	    {
	        // These must be kept in their existing order.
	        NONE,
	        TRACING,
	        // The next type goes here.
	        CRYSTAL_BALL // For testing.
	    };

	    static bool DebugStrToEnum( const char* type, DEBUG& ret );
	    static bool DebugEnumToStr( const DEBUG type, std::string& ret );
	    // Enable a particular debugging mode.
	    bool SupportsDebugType( const DEBUG type ) const;
	    void SetDebug( const DEBUG type, const StrBuf* id,
	                   const StrBuf& path, Error* e );

	    void ConfigBinding( const SCR_BINDING_LIBNAME lib, p4_std_any::p4_any cfg,
	                        Error* e );

	    // Execute the named file.
	    bool doFile( const char *name, Error *e );
	    // Execute the supplied script code. 
	    bool doStr ( const char *s, Error *e );
	    // Call a script function.
	    p4_std_any::p4_any doScriptFn( const char* name, Error* e );
	    // Check if a function exists.
	    bool fnExists( const char* name );

	    std::string getElapsedTime() const;

	    int getAPIVersion() const;
	    const char* getImplName() const;
	    const char* getImplVersion() const;

protected:

	    class impl;
	    class impl53;

	    std::unique_ptr< impl > pimpl;

private:
	    // Which API to expose to scripts.
	    const int apiVersion;

	    // Script memory allocation count (bytes).
	    scriptMem_t curMem{};

	    // Maximum allowed allocation before aborting script execution.
	    scriptMem_t maxMem{};

	    // Has the script used too much?
	    bool checkMem() const;

	    // Time spent executing the script, and the maximum allowed.
	    scriptTime_t curTime{}, maxTime{};

	    scriptTimeInc_t startTime{};

	    // Has the script run too long?
	    bool checkTime();

	    void beginTime();
	    void   addTime();

	    scriptTimeInc_t Now() const;

	    std::string fmtDuration( const scriptTime_t &dur ) const;
	    std::string fmtMem     ( const scriptMem_t  &mem ) const;

	    // Once the scriptCancelMsg is received, we set this so that
	    // we only return failure to the Lua runtime once.  It's possible
	    // to crash the runtime if we return errors multiple times in the
	    // memory allocator.
	    bool scriptCancelled = false;

	    const SCR_VERSION scriptType;

	    std::vector< std::function< void( ClientApi & ) > >
	        ClientApiBindCfgs;

	    std::vector< std::function< void( ClientUserLua& ) > >
	        ClientUserBindCfgs;

	    std::vector< std::function< bool( SCR_BINDING_LUA_OPTS opts ) > >
	        LuaBindCfgs;

public:

	    // These are not intended to be public, but are so out of
	    // necessity for the implementation.

	    // os_execute() is a non-blocking version of the Lua function.
	    static int os_execute( void* Lv );
	    // Callback for use in non-blocking versions of Lua functions.
	    static bool timeBreakCb( void* Lv );
	    // Callback for use with the Lua debug callback.
	    static void debugCb( void* Lv, void* arv );
	    // Used for cases like os.exit() where we want to call some script
	    // code that produces an error, but the real cause is different.
	    static void SetRealError( void* Lv, const Error* e );

	    p4script::impl* GetImpl();

} ;

class p4script::impl
{
	public:

	             impl( p4script& p, Error *e );
	    virtual ~impl();

	    virtual bool SupportsDebugType( const DEBUG type ) const = 0;
	    virtual void SetDebug( const DEBUG type, const StrBuf* id,
	                           const StrBuf& path, Error* e ) = 0;

	    virtual bool doFile( const char *name, Error *e );
	    virtual bool doStr ( const char *buf , Error *e );
	    virtual p4_std_any::p4_any doScriptFn( const char* name,
	                                                  Error* e ) = 0;
	    virtual bool fnExists( const char* name ) = 0;

	    virtual const char* getImplName() const;
	    virtual const char* getImplVersion() const;

	    virtual int os_execute( void* Lv ) = 0;
	    virtual bool timeBreakCb( void* Lv ) = 0;
	    virtual void debugCb( void* Lv, void* arv ) = 0;
	    virtual void SetRealError( const Error* e ) = 0;

	    virtual void* getState() = 0;

	protected:

	    friend class p4script;
	    friend class Extension;
	    p4script& parent;

};

class p4script::impl53 : public p4script::impl
{
	public:

	     impl53( p4script& p, Error *e );
	    ~impl53();

	    bool SupportsDebugType( const DEBUG type ) const;
	    void SetDebug( const DEBUG type, const StrBuf* id,
	                   const StrBuf& path, Error* e );

	    bool doFile( const char *name, Error *e );
	    bool doStr ( const char *buf , Error *e );
	    p4_std_any::p4_any doScriptFn( const char* name, Error* e );
	    bool fnExists( const char* name );

	    const char* getImplName() const;
	    const char* getImplVersion() const;

	    int os_execute( void* Lv );
	    bool timeBreakCb( void* Lv );
	    void debugCb( void* Lv, void* arv );
	    void SetRealError( const Error* e );

	    void* getState();

	protected:

	    virtual void doBindings();

	private:

	    class Debug
	    {
	        public:
	            Debug();
	           ~Debug();

	            void SetDebug( const DEBUG type, const StrBuf* id,
	                           const StrBuf& path, Error* e );
	            bool TraceCB( void* _L, void* arv, Error* e );

	            DEBUG GetType() const;

	        private:

	            // Format is: SECONDS.NANOS PID ID MESSAGE
	            void LogHeader( StrBuf& buf );

	            DEBUG type = DEBUG::NONE;
	            // Output buffer for log content.
	            StrBuf buf;
	            // 'id' is the string after the PID, e.g. the command UUID.
	            const StrBuf pid, *id;
	            std::unique_ptr< DateTimeHighPrecision > now;
	            std::unique_ptr< FileSys > log;
	            // Stack depth for indentation.
	            int level = 0;
	            // When to emit the source file header.
	            std::string lastFile;
	            // Source script file content.
	            std::unordered_map< std::string,
	                                std::vector< std::string > > lines;
	    };

	    Debug debug;

	    static void*
	    allocator( void *ud, void *ptr, size_t osize, size_t nsize );

	    bool doCode( const char *data, const bool isStr, Error *e );

	    // Break script execution every N instructions to give us a
	    // chance to abort.  This number was pulled out of a bag as
	    // appearing not too long or short (on the assumption that
	    // breaking too often would be inefficient) while allowing
	    // our checks to occur often enough.
	    const uint32_t maxInsStep = 32768;

	    // A separate error that we set to distinguish between Lua
	    // errors we caused (we interrupted the script) and ones we
	    // didn't.
	    Error realError{};

	    // Pointer to the script runtime.
	    void *l{};

	    const char* implName    = "Lua";
	    const char* implVersion = "5.3.6";
};

# else // HAS_EXTENSIONS

typedef unsigned int scriptMem_t;

class p4script
{
	public:

	    ~p4script();
	     p4script( const SCR_VERSION v, Error *e );

	    void SetMaxTime( const unsigned int max );
	    void SetMaxMem ( const scriptMem_t max );

	    bool doFile( const char *name, Error *e );

};

# endif // HAS_EXTENSIONS
# endif // P4SCRIPT_H
