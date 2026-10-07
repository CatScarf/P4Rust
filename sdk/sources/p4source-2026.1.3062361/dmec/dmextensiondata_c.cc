/*
 * Copyright 1995, 2019 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# include <charman.h>

# include <stdhdrs.h>
# include <strbuf.h>
# include <error.h>
# include <filesys.h>

# include "dmextensiondata.h"
# include "dmextensiondata_c.h"

# ifdef HAS_EXTENSIONS

# include <unordered_set>

ExtensionDataClient::ExtensionDataClient( const StrBuf& archiveFile,
	                   p4_std_optional::optional< StrBuf > unzipPath, Error* e )
	: ExtensionData( archiveFile, unzipPath, e )
{
}

ExtensionDataClient::ExtensionDataClient( const StrBuf& depotFile,
	                                  const int& depotRev,
	                                  const StrBuf& srvExtsDir,
	                   const p4_std_optional::optional< StrBuf > archiveFile,
	                                  Error* e )
	: ExtensionData( depotFile, depotRev, srvExtsDir, archiveFile, e )
{
}

# endif
