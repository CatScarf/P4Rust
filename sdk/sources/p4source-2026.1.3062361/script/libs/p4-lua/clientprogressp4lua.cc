/*
 * Copyright 1995, 2019 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# include <string>

# include <clientapi.h>
# include <clientprog.h>
# include <clientprogressp4lua.h>

namespace P4Lua {

ClientProgressP4Lua::ClientProgressP4Lua(void *prog, int type)
{
	progress = prog;
	//progress->Init( type );
}

ClientProgressP4Lua::~ClientProgressP4Lua()
{
}

void
ClientProgressP4Lua::Description(const StrPtr *d, int units)
{
	std::string desc = d->Text();
	//progress->Description(desc, units);
}

void
ClientProgressP4Lua::Total(long total)
{
	//progress->Total( total );
}

int
ClientProgressP4Lua::Update(long position)
{
	return  0;// !progress->Update( position );
}

void
ClientProgressP4Lua::Done(int failed)
{
	//progress->Done( failed != 0 );
}

}
