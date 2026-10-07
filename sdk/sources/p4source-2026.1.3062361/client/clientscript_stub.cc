/*
 * Copyright 1995, 2019 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# include <stdhdrs.h>
# include <error.h>
# include <strbuf.h>
# include <filesys.h>
# include <pathsys.h>
# include <strdict.h>
# include <strarray.h>
# include <handler.h>
# include <rpc.h>
# include <client.h>

# ifdef HAS_EXTENSIONS

# include <string>
# include <vector>
# include <regex>

# include <dmextension.h>
# include <dmextension_c.h>
# include <dmextensiondata.h>
# include <dmextensiondata_c.h>
# include <clientscript.h>
# include <enviro.h>

ClientScript::ClientScript( Client* c )
{
	(void)client; // Silence -Wunused
}

ClientScript::~ClientScript()
{
}

bool ClientScript::CanLoad() const
{
	return false;
}

bool ClientScript::BuildCheck() const
{
	return false;
}

std::vector< std::unique_ptr< Extension > >& ClientScript::GetExts()
{
	return exts;
}

void ClientScript::LoadScripts( const bool search, Error* e )
{
}

std::tuple< ClientScriptAction, int >
ClientScript::Run( const char* cmd, const char* func,
	           ClientUser *u, const bool noReplace, Error *e )
{
	return std::tuple< ClientScriptAction, int >
	       ( ClientScriptAction::PASS, 0 );
}

SCR_VERSION ClientScript::scrVerFromFileName( const char* file )
{
	return P4SCRIPT_UNKNOWN;
}

std::vector< std::tuple< std::string, SCR_VERSION > >
ClientScript::FindLooseExts( const StrPtr& start, const bool search, Error* e )
{
	return std::vector< std::tuple< std::string, SCR_VERSION > >();
}

void ClientScript::SetClient( Client* c ) {}
void ClientScript::SetSearchPath( const char* where ) {}
void ClientScript::SetSearchPattern( const char* what ) {}

extern void p4_sqlite3_shutdown() {}
extern void p4_sqlite3_initialize() {}

typedef void *(*p4_malloc_callback)(size_t size);
typedef void (*p4_free_callback)(void *ptr);
typedef void *(*p4_realloc_callback)(void *ptr, size_t size);
typedef char *(*p4_strdup_callback)(const char *str);
typedef void *(*p4_calloc_callback)(size_t nmemb, size_t size);
extern void p4_curl_global_init_mem( p4_malloc_callback m,
                             p4_free_callback f, p4_realloc_callback r,
                             p4_strdup_callback s, p4_calloc_callback c ) {}
extern void p4_curl_global_cleanup() {}

# else // HAS_EXTENSIONS

// This is only here to prevent linker warnings with empty archive files.

void __p4_clientscript_stub_empty_placeholder() {}

# endif
