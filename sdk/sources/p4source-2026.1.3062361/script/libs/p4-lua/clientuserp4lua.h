/*
 * Copyright 1995, 2019 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

namespace P4Lua {

class P4Lua;
class P4Result;
class SpecMgrP4Lua;

class ClientTransferP4Lua : public ClientTransfer {
    public:
	virtual int	Transfer( ClientApi *client,
	                    ClientUser *ui,
	                    const char *cmd,
	                    StrArray &args,
	                    StrDict &pVars,
	                    int threads,
	                    Error *e );
} ;

class ClientUserP4Lua : public ClientUser, public KeepAlive
{
    public:
	ClientUserP4Lua( P4Lua *parent );
	~ClientUserP4Lua();

	// Client User methods overridden here
	void OutputText(const char *data, int length);
	void Message(Error *e);
	void HandleError(Error *e);
	void OutputStat(StrDict *values);
	void OutputBinary(const char *data, int length);
	void InputData(StrBuf *strbuf, Error *e);
	void Diff(FileSys *f1, FileSys *f2, int doPage, char *diffFlags, Error *e);
	void Prompt(const StrPtr &msg, StrBuf &rsp, int noEcho, Error *e);

	int Resolve(ClientMerge *m, Error *e);
	int Resolve(ClientResolveA *m, int preview, Error *e);

	ClientProgress* CreateProgress( int type );
	int ProgressIndicator();

	void Finished();

	// Local methods
	bool SetInput(sol::object i, sol::this_state s);
	void SetCommand(const char *c)	{ cmd = c; }
	void SetApiLevel(int l);
	void SetTrack(bool t)		{ track = t; }

	P4Result& GetResults()		{ return *results; }
	int ErrorCount()		{ return results->ErrorCount(); };
	void Reset();

	// Debugging support
	void SetDebug(int d)		{ debug = d; }

	// Handler support
	void SetHandler(void* handler);
	void* GetHandler()		{ return handler; }

	//	Progress API support
	void SetProgress( void* p );
	void* GetProgress()		{ return progress; }

	// override from KeepAlive
	virtual int IsAlive()		{ return alive; }

	void SetState( sol::this_state *s ) { luaState = s; }

    private:
	void ProcessOutput(const char * method, std::string data);
	void ProcessOutput(const char * method, sol::table data);
	void ProcessMessage(Error * e);
	bool CallOutputMethod(const char * method, std::string data);
	bool CallOutputMethod(const char * method, sol::table data);

	StrBuf cmd;
	SpecMgrP4Lua *specMgr;
	P4Result *results;
	std::deque<sol::object> input;
	void* mergeData;
	void* mergeResult;
	void* handler;
	void* cOutputHandler;
	void* progress;
	void* cProgress;
	int debug;
	int apiLevel;
	int alive;
	bool track;
	P4Lua *parent;
	sol::this_state *luaState;
} ;

}