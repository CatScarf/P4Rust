/*
 * Copyright 1995, 2021 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# ifndef P4_STRCHECK_H
# define P4_STRCHECK_H

enum DmtStringChecks {
	DSC_PLAIN       = 0x0000, // allow anything but wildcards and revs
	DSC_CLEAN       = 0x0001, // turn spaces into _
	DSC_EMPTY       = 0x0002, // allow empty strings
	DSC_SLASH       = 0x0004, // allow /'s
	DSC_REL         = 0x0008, // allow relative paths (., ..)
	DSC_REV         = 0x0010, // allow rev indicators
	DSC_WILD        = 0x0020, // allow wildcards
	DSC_DASHNUM     = 0x0040, // allow -xxx and pure numbers
	DSC_ANY         = 0x0080,  // allow anything (but limit size)
	DSC_NOPERCENT   = 0x0100, // don't allow %%
	DSC_NOCOMMA     = 0x0200, // don't allow ,
	DSC_NOTAPERCENT = 0x0400, // don't allow % 
	DSC_NNEGNUM     = 0x0800, // allow pure numbers >= 0
	DSC_EMBEDNUL    = 0x1000, // allow embedded null bytes in value
	DSC_NOWHITESPACE= 0x2000, // don't allow whitespace
	DSC_NOEQUALS    = 0x4000  // don't allow =
} ;

# endif
