/*
 * Copyright 1995, 2019 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# include <stdhdrs.h>

# ifdef HAS_EXTENSIONS

# include <string>
# include <vector>
# include <deque>
# include <map>

# include <clientapi.h>
# include <i18napi.h>
# include <strtable.h>
# include <spec.h>
# include <p4script.h>
# include <p4script53.h>

# include "p4error.h"
# include "p4result.h"
# include "p4luadebug.h"
# include "clientuserp4lua.h"
# include "p4mergedata.h"

namespace P4Lua {

P4MergeData::P4MergeData(ClientUser *ui, ClientMerge *m, StrPtr &hint,
	void* info)
{
	this->debug = 0;
	this->actionmerger = 0;
	this->ui = ui;
	this->merger = m;
	this->hint = hint;
	this->info = info;

	// Extract (forcibly) the paths from the RPC buffer.
	StrPtr *t;
	if ((t = ui->varList->GetVar("baseName"))) base = t->Text();
	if ((t = ui->varList->GetVar("yourName"))) yours = t->Text();
	if ((t = ui->varList->GetVar("theirName"))) theirs = t->Text();

}

P4MergeData::P4MergeData(ClientUser *ui, ClientResolveA *m, StrPtr &hint,
	void* info)
{
	this->debug = 0;
	this->merger = 0;
	this->ui = ui;
	this->hint = hint;
	this->actionmerger = m;
	this->info = info;

}

std::string
P4MergeData::GetYourName()
{
	if (merger && yours.Length())
	    return std::string(yours.Text());
	else
	    return nullptr;
}

std::string
P4MergeData::GetTheirName()
{
	if (merger && theirs.Length())
	    return std::string(theirs.Text());
	else
	    return nullptr;
}

std::string
P4MergeData::GetBaseName()
{
	if (merger && base.Length())
	    return std::string(base.Text());
	else
	    return nullptr;
}

std::string
P4MergeData::GetYourPath()
{
	if (merger && merger->GetYourFile())
	    return std::string(merger->GetYourFile()->Name());
	else
	    return nullptr;
}

std::string
P4MergeData::GetTheirPath()
{
	if (merger && merger->GetTheirFile())
	    return std::string(merger->GetTheirFile()->Name());
	else
	    return nullptr;
}

std::string
P4MergeData::GetBasePath()
{
	if (merger && merger->GetBaseFile())
	    return std::string(merger->GetBaseFile()->Name());
	else
	    return nullptr;
}

std::string
P4MergeData::GetResultPath()
{
	if (merger && merger->GetResultFile())
	    return std::string(merger->GetResultFile()->Name());
	else
	    return nullptr;
}

std::string
P4MergeData::GetMergeHint()
{
	if (hint.Length())
	    return std::string(hint.Text());
	else
	    return nullptr;
}

bool
P4MergeData::RunMergeTool()
{
	Error e;
	if( merger ) {
	    ui->Merge( merger->GetBaseFile(), merger->GetTheirFile(),
	               merger->GetYourFile(), merger->GetResultFile(), &e );

	    if (e.Test())
		return false;
	    return true;
	}
	return false;
}

bool
P4MergeData::GetActionResolveStatus()
{
	return actionmerger ? true : false;
}

bool
P4MergeData::GetContentResolveStatus()
{
	return merger ? true : false;
}

void*
P4MergeData::GetMergeInfo()
{
	return this->info;
}

std::string
P4MergeData::GetMergeAction()
{
	//	If we don't have an actionMerger then return nil
	if (actionmerger) {
	    StrBuf buf;
	    actionmerger->GetMergeAction().Fmt(&buf, EF_PLAIN);
	    if (buf.Length())
		return std::string(buf.Text());
	    else
		return nullptr;
	}
	return nullptr;
}

std::string
P4MergeData::GetYoursAction()
{
	if (actionmerger) {
	    StrBuf buf;
	    actionmerger->GetYoursAction().Fmt(&buf, EF_PLAIN);
	    if (buf.Length())
		return std::string(buf.Text());
	    else
		return nullptr;
	}
	return nullptr;
}

std::string
P4MergeData::GetTheirAction()
{
	if (actionmerger) {
	    StrBuf buf;
	    actionmerger->GetTheirAction().Fmt(&buf, EF_PLAIN);
	    if (buf.Length())
		return std::string(buf.Text());
	    else
		return nullptr;
	}
	return nullptr;
}

std::string
P4MergeData::GetType()
{
	if (actionmerger) {
	    StrBuf buf;
	    actionmerger->GetType().Fmt(&buf, EF_PLAIN);
	    if (buf.Length())
		return std::string(buf.Text());
	    else
		return nullptr;
	}
	return nullptr;
}

void
P4MergeData::Invalidate()
{
	actionmerger = NULL;
	merger = NULL;
}

std::string
P4MergeData::GetString()
{
	StrBuf result;
	StrBuf buffer;

	if (actionmerger) {
		result << "P4MergeData - Action\n";
		actionmerger->GetMergeAction().Fmt(&buffer, EF_INDENT);
		result << "\tmergeAction: " << buffer << "\n";
		buffer.Clear();

		actionmerger->GetTheirAction().Fmt(&buffer, EF_INDENT);
		result << "\ttheirAction: " << buffer << "\n";
		buffer.Clear();

		actionmerger->GetYoursAction().Fmt(&buffer, EF_INDENT);
		result << "\tyoursAction: " << buffer << "\n";
		buffer.Clear();

		actionmerger->GetType().Fmt(&buffer, EF_INDENT);
		result << "\ttype: " << buffer << "\n";
		buffer.Clear();

		result << "\thint: " << hint << "\n";
		return result.Text();
	} else {
		result << "P4MergeData - Content\n";
		if (yours.Length()) result << "yourName: " << yours << "\n";
		if (theirs.Length()) result << "thierName: " << theirs << "\n";
		if (base.Length()) result << "baseName: " << base << "\n";

		if ( merger && merger->GetYourFile())
			result << "\tyourFile: " << merger->GetYourFile()->Name() << "\n";
		if ( merger && merger->GetTheirFile())
			result << "\ttheirFile: " << merger->GetTheirFile()->Name() << "\n";
		if ( merger && merger->GetBaseFile())
			result << "\tbaseFile: " << merger->GetBaseFile()->Name() << "\n";

		return result.Text();
	}
	return nullptr;
}

}

# endif
