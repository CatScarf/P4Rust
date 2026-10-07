/*
 * Copyright 2005 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

/*
 * GlobalCharSet -- a static charSet index across the process.
 */

# include <stdhdrs.h>
# include <charset.h>

int GlobalCharSet::globCharSet = 0;
P4MT int GlobalCharSet::globCharSetAlt = 0;
P4MT bool GlobalCharSet::globCharSetUseAlt = false;

void GlobalCharSet::Set( int cs )
{
	if( globCharSetUseAlt )
	    globCharSetAlt = cs;
	else
	    globCharSet = cs;
}

int GlobalCharSet::Get()
{
	return globCharSetUseAlt ? globCharSetAlt : globCharSet;
}

void GlobalCharSet::UseAlt( const bool val )
{
	globCharSetUseAlt = val;
	globCharSetAlt = globCharSet;
}
