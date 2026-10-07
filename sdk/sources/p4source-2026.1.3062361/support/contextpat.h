/*
 * Copyright 2022 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

class ContextPat
{
    public:
	ContextPat(unsigned int mask, unsigned int value);
	virtual ~ContextPat();
	static unsigned int is_set(unsigned int mask);
    private:
	unsigned int current;
	ContextPat *last;
};


enum ContextPatOpts
{
	// Used to force the FILE_FLAG_OVERLAPPED bit off
	// when opening a db file in the checkpoint
	// generating code for Windows.
	FORCE_OVERLAP_OFF = 0x01
};
