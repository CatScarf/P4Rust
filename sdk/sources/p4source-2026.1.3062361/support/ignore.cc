/*
 * Copyright 1995, 2003 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 *
 * Git style ignore file for add/reconcile
 */

# include <stdhdrs.h>
# include <strbuf.h>
# include <strops.h>
# include <error.h>
# include <strarray.h>
# include <vararray.h>
# include <vartree.h>
# include <debug.h>

# include <pathsys.h>
# include <filesys.h>

# include <maptable.h>

# include "ignore.h"

#include "maphalf.h"

# ifdef OS_NT
# define SLASH "\\"
# else
# define SLASH "/"
# endif

# define ELLIPSE "..."

/*
 * IgnoreArray -- VarArray of MapHalves
 */

class IgnoreMap
{
    public:
	IgnoreMap() : exclude( false ) {}
	MapHalf h;
	bool exclude;
};

class IgnorePtrArray : public VVarArray {
    public:
			IgnorePtrArray() : VVarArray() {}
	virtual		~IgnorePtrArray()
			{
			    // don't delete
			}

	virtual int	Compare( const void *a, const void *b ) const
			{
			    return ((IgnoreMap *)a)->h.Compare( ((IgnoreMap *)b)->h );
			}

	virtual void	Destroy( void *a ) const
			{
			    // don't delete
			}
	
	virtual void	PutItem( IgnoreMap *h ) { Put( h ); }
	IgnoreMap *	GetItem( int i ) { return (IgnoreMap *)Get( i ); }
} ;

class IgnoreArray : public IgnorePtrArray {
    public:
			IgnoreArray() : IgnorePtrArray() {}
	virtual		~IgnoreArray()
			{
			    for( int i = 0; i < Count(); i++ )
			        delete (IgnoreMap *)Get( i );
			}

	virtual void	Destroy( void *a ) const
			{
			    delete (IgnoreMap *)a;
			}

	void		Clear()
			{
			    for( int i = 0; i < Count(); i++ )
			        delete (IgnoreMap *)Get( i );
			    IgnorePtrArray::Clear();
			}
			
	
	virtual void	PutItem( IgnoreMap *h ) { *NewItem() = *h; }
	IgnoreMap *	NewItem() { return (IgnoreMap *)Put( new IgnoreMap ); }
} ;

/*
 * IgnoreTable -- cached ignore rules
 */

class IgnoreItem {
    public:
			IgnoreItem()  { ignoreList = new IgnoreArray(); }
			~IgnoreItem() { delete ignoreList; }

	void		AppendToList( IgnorePtrArray *list )
			{
			    for( int i = 0; i < ignoreList->Count(); i++ )
			        list->PutItem( ignoreList->GetItem( i ) );
			}

	void		Copy( IgnoreItem *src )
			{
			    ignoreFile = src->ignoreFile;
			    ignoreList->Clear();
			    for( int i = 0; i < src->ignoreList->Count(); i++ )
			        ignoreList->PutItem( src->ignoreList->GetItem( i ) );
			}

	StrBuf		ignoreFile;
	IgnoreArray*	ignoreList;
} ;

class IgnoreTable : public VVarTree {

    public:
			IgnoreTable() : VVarTree() {}
			~IgnoreTable() { Clear(); }

	int		Compare( const void *a, const void *b ) const;
	void *		Copy( const void *src ) const;
	void		Delete( void *a ) const;
	void		Dump( void *a, StrBuf &buf ) const;

	IgnoreItem	*GetItem( const StrRef &file );
	IgnoreItem	*PutItem( const StrRef &file, Error *e );
} ;

int
IgnoreTable::Compare( const void *a, const void *b ) const
{
	return ((IgnoreItem*)a)->ignoreFile.Compare(((IgnoreItem*)b)->ignoreFile);
}

void
IgnoreTable::Delete( void *a ) const
{
	delete (IgnoreItem *)a;
}
void
IgnoreTable::Dump( void *a, StrBuf &buf ) const
{
	buf = ((IgnoreItem *)a)->ignoreFile;
}

void *
IgnoreTable::Copy( const void *src ) const
{
	IgnoreItem *ret = new IgnoreItem();
	ret->Copy( (IgnoreItem *)src );

	return ret;
}


IgnoreItem *
IgnoreTable::GetItem( const StrRef &file )
{
	IgnoreItem key;
	key.ignoreFile = file;

	return (IgnoreItem *)Get( &key );
}

IgnoreItem *
IgnoreTable::PutItem( const StrRef &file, Error *e )
{
	IgnoreItem *a = GetItem( file );

	if( !a )
	{
	    IgnoreItem key;
	    key.ignoreFile = file;
	    a = (IgnoreItem *)Put( &key, e );
	}

	return a;
}


/*
 * Ignore
 *
 * Loads ignore rules from ignore files and checks to see if files match
 * those fules.
 */

Ignore::Ignore()
{
	ignoreTable = new IgnoreTable;
	ignoreFiles = new StrArray;
	ignoreList = 0;
	relatives = 0;
	defaultList = 0;
}

Ignore::~Ignore()
{
	delete ignoreList;
	delete ignoreTable;
	delete ignoreFiles;
	delete defaultList;
}

int
Ignore::Reject( 
	const StrPtr &path, 
	const StrPtr &ignoreName,
	const char *configName,
	StrBuf *line )
{
	return Build( path, ignoreName, configName ) &&
	    RejectCheck( path, 0, line ) ? 1 : 0;
}

int
Ignore::RejectDir( const StrPtr &path,
	const StrPtr &ignoreName,
	const char *configName,
	StrBuf *line )
{
	return Build( path, ignoreName, configName ) &&
	    RejectCheck( path, 1, line ) ? 1 : 0;
}

int
Ignore::List( const StrPtr &path,
	const StrPtr &ignoreName,
	const char *configName,
	StrArray *outList )
{
	Build( path, ignoreName, configName );

	for( int j = 0; j < ignoreList->Count(); ++j )
	{
	    StrBuf *out = outList->Put();
	    IgnoreMap *i = ignoreList->GetItem(j);
	    if( i->exclude )
	        *out << "!";
	    *out << i->h;
	}

	return outList->Count();
}


int
Ignore::GetIgnoreFiles( const StrPtr &ignoreName,
	int absolute,
	int relative,
	StrArray &ignoreFiles )
{
	StrRef slash( SLASH );
	const StrBuf *ign = 0;
	int i = 0;
	int res = 0;

	BuildIgnoreFiles( ignoreName );
	while( ( ign = this->ignoreFiles->Get( i++ ) ) )
	{
	    if( ign->Contains( slash ) && absolute )
	    {
	        ignoreFiles.Put()->Set( ign );
	        res++;
	    }
	    else if( !ign->Contains( slash ) && relative )
	    {
	        ignoreFiles.Put()->Set( ign );
	        res++;
	    }
	}

	return res;
}

int
Ignore::Build( const StrPtr &path,
	const StrPtr &ignoreName,
	const char *configName )
{
	if( ( !configName && !this->configName.Length() ) ||
	    ( configName && this->configName != configName ) )
	{
	    delete ignoreList;
	    ignoreList = 0;
	    delete defaultList;
	    defaultList = 0;
	    this->configName.Clear();
	    if( configName )
	        this->configName = configName;
	}

	// If we don't have an ignore file, we can just load up the defaults
	// and test against those. If we've already loaded them, lets not do
	// it again and use the existing list instead.
	
	if( !strcmp( ignoreName.Text(), "unset" ) )
	{
	    if( !ignoreList )
	        ignoreList = new IgnorePtrArray;

	    if( !ignoreList->Count() )
	        InsertDefaults( ignoreList );

	    return 1;
	}


	PathSys *p = PathSys::Create();
	p->Set( path );
	p->ToParent();

	// Try real hard not to regenerate the ignorelist, this
	// optimization uses the current directory depth and the
	// last found ignorefile depth to reduce the search for
	// config files.

	if( ignoreList && dirDepth.Length() )
	{
	    if( !dirDepth.SCompare( *p ) )
	    {
	        // matching depth bail early
	        if( DEBUG_BUILD )
	            p4debug.printf(
	                "Ignore optimization - same directory: %s\n",
	                p->Text() );

	        delete p;
	        return 1;
	    }

	    if( !dirDepth.SCompareN( *p ) )
	    {
	        // descending directories can't be shortcut.
	        // ignoreList hasn't been populated this deep.
	    }
	    else if( !p->SCompareN( dirDepth ) &&
	             foundDepth.Length() && !foundDepth.SCompareN( *p ) )
	    {
	        // ascending directories can be shortcut.
	        // ignoreList doesn't have any ignore entries for the
	        // directory we're stepping back out of.

	        if( DEBUG_BUILD )
	            p4debug.printf( "Ignore optimization - higher directory, "
	                "higher than last found ignorefile: %s vs %s\n",
	                p->Text(), foundDepth.Text() );

	        dirDepth.Set( *p );
	        delete p;
	        return 1;
	    }
	}
	
	
	// Split the potential ignoreName list into a real list
	
	BuildIgnoreFiles( ignoreName );


	StrBuf line;
	StrBuf closestFound;
	int found = 0;
	int foundStatic = 0;
	Error e;
	PathSys *q = PathSys::Create();
	FileSys *f = FileSys::Create( FileSysType( FST_TEXT|FST_L_CRLF ) );
	IgnoreItem *ignoreItem = 0;
	const StrBuf *ignoreFile;

	closestFound.Clear();
	dirDepth.Set( *p );

	// Pre-test for any ignore files in any undiscovered directories

	if( relatives && ignoreList )
	{
	    p->Set( path );
	    p->ToParent();
	    bool workToDo = false;
	    bool dirhit;

	    // Starting with the current directory, walk up to root
	    do {
	        // Check to see if we have to discard ignore lines
	        if( foundDepth.Length() && foundDepth.SCompareN( *p ) )
	            workToDo = true;

	        // For each relative ignoreFile, see if we've already checked
	        // checked for an ignoreFile at this level
	        dirhit = true;
	        for( int m = 0; !workToDo && m < ignoreFiles->Count(); m++ )
	        {
	            ignoreFile = ignoreFiles->Get( m );
	            if( ignoreFile->Contains( StrRef( SLASH ) ) )
	                continue;

	            q->SetLocal( *p, *ignoreFile );
	            if( ignoreTable->GetItem( *q ) )
	                continue;

	            if( FileSys::FileExists( q->Text() ) )
	                workToDo = true;
	            else
	            {
	                Error e;
	                ignoreTable->PutItem( *q, &e );
	                dirhit = false; // keep descending
	            }
	        }
	    } while( !workToDo && !dirhit && p->ToParent() );

	    if( !workToDo )
	    {
	        p->Set( path );
	        p->ToParent();
	        dirDepth.Set( *p );

	        if( DEBUG_BUILD )
	            p4debug.printf( "Ignore optimization - lower directory, "
	                            "no additional ignorefiles: %s\n",
	                            p->Text() );

	        delete q;
	        delete p;
	        delete f;
	        return 1;
	    }
	}

	// No descending optimization, remove list we will recreate it


	IgnorePtrArray newList;
	InsertDefaults( &newList );

	for( int m = 0; m < ignoreFiles->Count(); m++ )
	{
	    ignoreFile = ignoreFiles->Get( m );
	    
	    if( ignoreFile->Contains( StrRef( SLASH ) ) )
	    {
	        // load the file; we have a path, so no need to wander the
	        // tree looking for it

	        if( !( ignoreItem = ignoreTable->GetItem( *ignoreFile ) ) )
	        {
	            Error e;
	            ignoreItem = ignoreTable->PutItem( *ignoreFile, &e );
	            if( !ignoreItem ) // Failed to cache. Out of memory?
	                continue;

	            f->Set( *ignoreFile );
	            if( !ParseFile( f, "", ignoreItem->ignoreList ) )
	                continue;
	        }

	        if( !ignoreItem->ignoreList->Count() )
	            continue;

	        foundStatic++;
	        ignoreItem->AppendToList( &newList );
	    }
	    else
	    {
	        // starting from the directory in which the argument supplied
	        // file lives,  walk up the tree collecting ignore files as
	        // we go

	        p->Set( path );
	        p->ToParent();

	        do {
	            q->SetLocal( *p, *ignoreFile );

	            if( !( ignoreItem = ignoreTable->GetItem( *q ) ) )
	            {
	                Error e;
	                ignoreItem = ignoreTable->PutItem( *q, &e );
	                if( !ignoreItem ) // Failed to cache. Out of memory?
	                    continue;

	                f->Set( *q );
	                if( !ParseFile( f, p->Text(), ignoreItem->ignoreList ) )
	                    continue;
	            }

	            if( !ignoreItem->ignoreList->Count() )
	                continue;

	            if( closestFound.Length() < p->Length() )
	                closestFound = *p;

	            found++;
	            ignoreItem->AppendToList( &newList );

	        } while( p->ToParent() );
	    }
	}

	p->Set( path );
	p->ToParent();
	if( found || !ignoreList )
	{
	    if( DEBUG_BUILD )
	        p4debug.printf(
	            "Ignore %sbuilt - %d new files, %d lines: %s\n",
	            ignoreList ? "re" : "", found + foundStatic,
	            newList.Count(), p->Text() );

	    delete ignoreList;
	    ignoreList = new IgnorePtrArray;

	    for( int i = 0; i < newList.Count(); i++ )
	        ignoreList->PutItem( newList.GetItem( i ) );

	    foundDepth = closestFound;
	}
	else if( DEBUG_BUILD )
	    p4debug.printf( "Ignore not rebuilt - no new files found: %s\n",
	                    p->Text() );


	delete q;
	delete p;
	delete f;

	if( DEBUG_LIST )
	{
	    p4debug.printf( "\n\tIgnore list:\n\n" );
	    for( int j = 0; j < ignoreList->Count(); ++j )
	    {
	         IgnoreMap *i = ignoreList->GetItem( j );
	         p4debug.printf( "\t%s%s\n",
	                         i->exclude ? "!" : "",
	                         i->h.Text() );
	    }
	    p4debug.printf( "\n" );
	}

	return 1;
}

void
Ignore::InsertDefaults( IgnorePtrArray *newList )
{
	if( defaultList )
	{
	    for( int i = 0; i < defaultList->Count(); i++ )
	       newList->PutItem( defaultList->GetItem( i ) );
	    return;
	}

	StrArray defaultsList;
	int l = 0;
	defaultList = new IgnoreArray;

	StrBuf configDirLine;
	configDirLine.Clear();


	// Always add in .p4root and P4CONFIG to the top of new lists

	if( configName.Length() )
	{
	    StrBuf line;
	    line << "**/" << configName;
	    Insert( &defaultsList, line.Text(), "", ++l );
	    configDirLine << ".../" << configName << SLASH << "...";
	}

	Insert( &defaultsList, "**/.p4root", "", ++l );

	// Add debug line

	defaultsList.Put()->Set( StrRef( "#FILE - defaults" ) );

	// Add the generated lines to the ignore list (in reverse)

	StrBuf line;
	for( int i = defaultsList.Count(); i > 0; --i )
	{
	    if( configName.Length() &&
	        *defaultsList.Get( i - 1 ) == configDirLine )
	        continue;

	    line.Set( defaultsList.Get( i - 1 ) );
#ifdef OS_NT
	    // On NT slash is \ but the matcher's * wildcard only uses /,
	    // so we need to convert our slashes

	    StrOps::Sub( line, '\\', '/' );
#endif
	    IgnoreMap *im = defaultList->NewItem();
	    im->h = line;
	}

	for( int i = 0; i < defaultList->Count(); i++ )
	    newList->PutItem( defaultList->GetItem( i ) );
}

void
Ignore::Insert( StrArray *subList, const char *ignore, const char *cwd,
		int lineno )
{
	StrBuf buf;
	StrBuf buf2;
	StrBuf raw = ignore;
	const char *lastCwdChar = cwd + strlen( cwd ) - 1;
	const char *lastIgnChar = ignore + strlen( ignore ) - 1;
	char *terminating = (char *)SLASH;

	int reverse = ( *ignore == '!' );
	int isWild = strchr( ignore, '*' ) != 0;
	// One slash at the end is a sign that we're just matching directories
	int isDir = ( *lastIgnChar == *terminating );

	if( strstr( ignore, "*****" ) || strstr( ignore, "..." ) )
	    buf << "### SENSELESS JUXTAPOSITION ";

	if( reverse )
	{
	    buf << "!";
	    ignore++;
	}

	// If the ignore line starts with a slash, it's relative to this ingore file

	int isRel = ( *ignore == *terminating );
	if( isRel )
	    ignore++;

	// Add the base path - in needs a trailing / unless it's not set (global)

	buf << cwd;
	if( strlen( cwd ) && *lastCwdChar != *terminating )
	    buf << SLASH;
	
	// buf contains the relative path
	// buf2 contains the non-relative path
	// If the path is relative (starts with /) then we don't want to
	// add .../ to the start.
	// Otherwise we will need both (to include files here and in
	// directories below us)

	buf2 << buf << ELLIPSE;
	buf << ignore;

	if( !isRel && *ignore == '*' )
	{
	    // The path isn't relative, so we'll use buf2.
	    // If it starts with a * (or **) lets not add the slash.
	    // E.g. *.foo/bar -> ....foo/bar (not .../*.foo/bar)

	    while( *ignore == '*' )
	        ignore++;

	    buf2 << ignore;
	}
	else
	    buf2 << SLASH << ignore;

	if( isDir )
	{
	    // The path ends with a shash, so add the ...

	    buf << ELLIPSE;
	    buf2 << ELLIPSE;
	}

	// It's possible that we've expanded out buf2 so that we no longer
	// need buf (E.g. *.foo/bar). If the path was wild, but isn't
	// anymore, then lets not add it.

	if( isRel || !isWild || ( isWild && strchr( ignore, '*' ) != 0 ) )
	    StrOps::Replace( *subList->Put(), buf,
	                     StrRef( "**" ), StrRef( ELLIPSE ));

	if( !isRel )
	    StrOps::Replace( *subList->Put(), buf2,
	                     StrRef( "**" ), StrRef( ELLIPSE ));

	// If the path was not explicitly a directory, it might be one.
	// Unless of course it edded with a or **

	if( !isDir && !buf.EndsWith( "**", 2 ) )
	{
	    buf << SLASH << ELLIPSE;
	    buf2 << SLASH << ELLIPSE;

	    if( isRel || !isWild || ( isWild && strchr( ignore, '*' ) != 0 ) )
	        StrOps::Replace( *subList->Put(), buf,
	                          StrRef( "**" ), StrRef( ELLIPSE ));

	    if( !isRel )
	        StrOps::Replace( *subList->Put(), buf2,
	                          StrRef( "**" ), StrRef( ELLIPSE ));
	}

	// Add the debug line
	
	buf.Clear();
	buf << "#LINE " << lineno << ":" << raw;
	subList->Put()->Set( buf );
}

int
Ignore::ParseFile( FileSys *f, const char *cwd, IgnoreArray *list )
{
	Error e;
	StrBuf line;
	StrBuf dline;
	StrArray tmpList;

	f->Open( FOM_READ, &e );
	
	if( e.Test() )
	    return 0;

	for( int l = 1; f->ReadLine( &line, &e ); l++ )
	{
	    line.TrimBlanks();
	    if( !line.Length() || line.Text()[0] == '#' )
	        continue;

	    if( line.Text()[0] == '\\' && line.Text()[1] == '#' )
	    {
	        StrBuf tmp( line.Text() + 1 );
	        line = tmp;
	    }

#ifdef OS_NT
	    // On NT slash is \ so we need to normalise our slashes to
	    // avoid cross platform issues.

	    StrOps::Sub( line, '/', '\\' );
#endif

	    Insert( &tmpList, line.Text(), cwd, l );
	}

	f->Close( &e );

	line.Clear();
	line << "#FILE " << f->Name();
	IgnoreMap *im = list->NewItem();
	im->h = line;

	
	// Add the read lines to the target list (in reverse)

	for( int i = tmpList.Count(); i > 0; --i )
	{
	    bool exclude = false;
	    char *tmp = tmpList.Get( i - 1 )->Text();
	    if( tmp[0] == '!' )
	    {
	        exclude = true;
	        tmp++;
	    }

	    line.Set( tmp );

#ifdef OS_NT
	    // On NT slash is \ but the matcher's * wildcard only uses /,
	    // so we need to convert our slashes

	    StrOps::Sub( line, '\\', '/' );
#endif

	    IgnoreMap *im = list->NewItem();
	    im->h = line;
	    im->exclude = exclude;
	}

	return 1;
}

int
Ignore::RejectCheck( const StrPtr &path, int isDir, StrBuf *line )
{
	char *ignoreFile = 0;
	char *ignoreLine = 0;

	// Fix the path separators

	StrBuf cpath( path );
	StrOps::Sub( cpath, '\\', '/' );

	// Dirs must have trailing / for matching /...
	
	if( isDir && !cpath.EndsWith( "/", 1 ) )
	    cpath << "/";

	// Dirs have /... tails when checking in reverse

	MapTable dmap;
	if( isDir )
	{
	    StrBuf dpath( cpath );
	    dpath << "...";
	    dmap.Insert( dpath );
	}

	for( int i = 0; i < ignoreList->Count(); ++i )
	{
	    IgnoreMap *m = ignoreList->GetItem( i );
	    char *p = m->h.Text();

	    if( !strncmp( p, "#FILE ", 6 ) )
	    {
	        ignoreFile = p+6;
	        continue;
	    }

	    if( !strncmp( p, "#LINE ", 6 ) )
	    {
	        ignoreLine = p+6;
	        continue;
	    }

	    int doAdd = m->exclude;

	    // If we're checking against a directory and this is a reverse
	    // include, it might allow files below this directory, even if
	    // this directory is ignored. To deal with this, we need to look
	    // both ways check match in either direction)

	    MapParams params;
	    if( m->h.Match( cpath, params ) ||
	        ( isDir && doAdd && dmap.JoinCheck( LHS, m->h ) ) )
	    {
	        if( DEBUG_MATCH )
	            p4debug.printf(
	                "\n\t%s[%s]\n\tmatch[%s%s]%s\n\tignore[%s]\n\n",
	                isDir ? "dir" : "file", path.Text(), doAdd ? "+" : "-",
	                p, doAdd ? "KEEP" : "REJECT", ignoreFile );

	        // If an ignoreLine pointer was passed, populate it with the
	         // ignoreFile, line number and rule that we matched.

	         if( line && ignoreFile && ignoreLine )
	         {
	             line->Set( ignoreFile );
	             line->UAppend( ":" );
	             line->UAppend( ignoreLine );
	         }

	        return doAdd ? 0 : 1;
	    }
	}

	if( DEBUG_MATCH )
	    p4debug.printf(
	        "\n\t%s[%s]\n\tmatch[+NONE]KEEP\n\tignore[NONE]\n\n",
	        isDir ? "dir" : "file", path.Text() );

	return 0;
}


void
Ignore::BuildIgnoreFiles( const StrPtr &ignoreNames )
{
	if( ignoreStr == ignoreNames )
	    return;

	relatives = 0;

	if( ignoreFiles )
	    delete ignoreFiles;

	ignoreFiles = new StrArray;


	// We might have more than one ignore file name set: split them up

	if( strchr( ignoreNames.Text(), ';' ) ||
	    ( SLASH[0] == '/' && strchr( ignoreNames.Text(), ':' ) ) )
	{
	    StrBuf iname = ignoreNames;

	    // Do some work to make sure we have the right path seaprators
	    // and normalise the split characters.

#ifdef OS_NT
	    StrOps::Sub( iname, '/', '\\' );
#else
	    StrOps::Sub( iname, '\\', '/' );
	    StrOps::Sub( iname, ':', ';' );
#endif

	    char *c;
	    char *n = iname.Text();
	    while( ( c = strchr( n, ';' ) ) )
	    {
	        if( c > n )
	        {
	            StrBuf *file = ignoreFiles->Put();
	            file->Set( StrRef(n, c - n) );
	            if( !strchr( file->Text(), SLASH[0] ) )
	                relatives++;
	        }
	        n = c + 1;
	    }
	    if( strlen( n ) )
	    {
	        StrBuf *file = ignoreFiles->Put();
	        file->Set( StrRef( n, strlen( n ) ));
	        if( !strchr( file->Text(), SLASH[0] ) )
	            relatives++;
	    }
	}
	else
	{
	    ignoreFiles->Put()->Set( ignoreNames );
	    if( !strchr( ignoreNames.Text(), SLASH[0] ) )
	        relatives++;
	}

	ignoreStr = ignoreNames;
}
