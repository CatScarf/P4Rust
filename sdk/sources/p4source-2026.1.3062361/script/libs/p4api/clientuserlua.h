/*
 * Copyright 2019 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

class ClientUserLua : public ClientUser, public KeepAlive
{
	public:

	     ClientUserLua( const char* impl, const int apiVersion );
	    ~ClientUserLua();

	    static void doBindings( sol::state* lua, sol::table& ns,
	    std::vector< std::function< void( ClientUserLua& ) > >* ClientUserBindCfgs,
	    const char* implName, const int apiVersion );

	    void DisableParallelTransfer();

	    void InputData( StrBuf* strbuf, Error* e );

	    void OutputText( const char* data, int length );
	    void OutputBinary( const char* data, int length );
	    void OutputInfo( char level, const char* data );
	    void OutputStat( StrDict* dict );
	    void HandleError( Error* err );
	    void OutputError( const char* err );
	    void Message( Error* err );
	    void Edit( FileSys* f1, Error* e );

	    void Prompt( Error* err, StrBuf& rsp, int noEcho, Error* e );
	    void ErrorPause( char* errBuf, Error* e );
	    FileSys* File( FileSysType type );

	    int IsAlive();

	    sol::protected_function fInputData, fOutputText, fOutputBinary,
	                            fOutputInfo, fOutputStat, fHandleError,
	                            fOutputError, fMessage, fEdit, fPrompt,
	                            fErrorPause, fFile;
	    sol::table data;
	    const char* impl;
	    const int apiVersion;
};

class ClientTransferLua : public ClientTransfer {
    public:
	int Transfer( ClientApi* client, ClientUser* ui, const char* cmd,
	              StrArray& args, StrDict& pVars, int threads, Error* e );
} ;
