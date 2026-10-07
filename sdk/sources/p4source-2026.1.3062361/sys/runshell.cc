/*
 * Copyright 1995, 2019 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

/*
 * RunShell() -- Use the shell to 'run' commands/URLs *NT only*
 */

# include <windows.h>
# include <objbase.h>
# include <shellapi.h>

bool
DoRunShell( const char *cmd )
{
	SHELLEXECUTEINFO ShExecInfo;
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = 0;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = NULL;
	ShExecInfo.lpFile = cmd;
	ShExecInfo.lpParameters = NULL;
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_MAXIMIZE;
	ShExecInfo.hInstApp = NULL;
	CoInitializeEx( NULL, COINIT_APARTMENTTHREADED|COINIT_DISABLE_OLE1DDE);
	return ShellExecuteExA( &ShExecInfo );
}
