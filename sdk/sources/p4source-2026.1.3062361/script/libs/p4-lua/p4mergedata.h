/*
 * Copyright 1995, 2019 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

namespace P4Lua {

class P4MergeData
{
    public:
    P4MergeData( ClientUser *ui, ClientMerge *m, StrPtr &hint, void* info );
    P4MergeData( ClientUser *ui, ClientResolveA *m, StrPtr &hint, void* info );

    void  SetDebug( int d )	{ debug = d;	}

    //	Content resolve
    std::string	GetYourName();
    std::string	GetTheirName();
    std::string	GetBaseName();

    std::string	GetYourPath();
    std::string	GetTheirPath();
    std::string	GetBasePath();
    std::string	GetResultPath();

    bool	RunMergeTool();

    //	What type of resolve is it?
    bool	GetActionResolveStatus();
    bool	GetContentResolveStatus();

    //	Action Resolve
    void *	GetMergeInfo();

    std::string	GetMergeAction();
    std::string	GetYoursAction();
    std::string	GetTheirAction();
    std::string	GetType();


    std::string	GetString();
    std::string	GetMergeHint();


    // Invalidate our merger and actionMerger objects as they do not survive
    // beyond the life of a resolve while this object itself might well do so,
    // particularly in the case of an exception raised from within the block.
    void	Invalidate();


    private:
    int				debug;
    ClientUser *	ui;
    StrBuf			hint;
    ClientMerge *	merger;
    ClientResolveA*	actionmerger;
    StrBuf			yours;
    StrBuf			theirs;
    StrBuf			base;

    void *			info;

};

}