/*
 * Copyright 1995, 2023 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# include <stdhdrs.h>

# include <sqlite3.h>
# include <curl/curl.h>

// p4libs non-stub wrappers

extern void p4_sqlite3_initialize()
{
	sqlite3_initialize();
}

extern void p4_sqlite3_shutdown()
{
	sqlite3_shutdown();
}

extern void p4_curl_global_init_mem( curl_malloc_callback m,
                             curl_free_callback f, curl_realloc_callback r,
                             curl_strdup_callback s, curl_calloc_callback c )
{
	curl_global_init_mem( CURL_GLOBAL_ALL, m, f, r, s, c );
}

extern void p4_curl_global_cleanup()
{
	curl_global_cleanup();
}
