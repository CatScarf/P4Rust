/*
 * Copyright 1995, 2020 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

class StrRef;

enum MonItemFlags {
	MI_HIDDEN =		0x1, // Visibility flag
	MI_CUMULATIVE =		0x2  // Interpretation flag (Cumulative)
} ;

enum MonItemTypes {
	MIT_NONE =		0x1, // MonItem
	MIT_INT =		0x2, // MonInteger
	MIT_INTMAX =		0x3  // MonIntMax
} ;

class MonItem {

    public:

			MonItem( const char *, int = 0 );
			MonItem( const MonItem & );
			~MonItem();
	
	MonItem &operator =( const MonItem & );
	
	StrRef		Name() const;
	virtual int	DataSize() const;
	virtual int	Alignment() const;
	virtual int	ItemId() const;
	virtual void	Display( StrBuf * ) const;
	
	int		Flags()  const { return flags; }
	bool		Active() const { return data != NULL; }
	static MonItem	*Find(const char *);
	
    protected:
	const char	*Data() const { return data; }
	
    private:
	friend class MemItems;

	static MonItem	*TypeFactory( void * );
	virtual void	Initialize();
	int		ItemSize() const;

	const char	*name;
	const char	*data;
	int		flags;
	MonItem		*next;
	static MonItem	*first;
};

class MonInteger : public MonItem {

    public:
			MonInteger( const char *, int = 0, P4INT64 init = 0 );
			~MonInteger();
	
	virtual int	DataSize() const;
	virtual int	Alignment() const;
	virtual int	ItemId() const;
	
	void		Set( P4INT64 );
	void		Increment();
	void		Increment( P4INT64 );
	void		Decrement();
	void		Decrement( P4INT64 );
	
	P4INT64		Value() const;
	void		Display( StrBuf * ) const;

    protected:
	P4INT64		initialValue;

    private:
	virtual void	Initialize();
	virtual void	NewValue();
};

class MonIntMax : public MonInteger {

    public:
			MonIntMax( const char *a, int f = 0, P4INT64 init = 0 )
			    : MonInteger( a, f, init ) {}
	
	virtual int	DataSize() const;
	virtual int	ItemId() const;
	
	P4INT64		MaxValue() const;
	void		Display( StrBuf * ) const;

    private:
	virtual void	Initialize();
	virtual void	NewValue();
};

class MemItems {
    public:
			// basic constructor
			MemItems();
			~MemItems();
	
			// old init
	void		Init();
			// use and reconcile with a memory item segment file
	bool		Init(const char *);
			// initialize a new memory item segment - clearing old
	bool		HardInit(const char *);
	// load an existing memory item segment
	bool		Load(const char *, Error *e);
	
	// This writes a segment out - should be replaced by shared memory
	void		Dump(const char *, Error *e = 0 );
	// This displays items from a segment
	void		Display();

	MonItem		*First()          const { return MonItem::first; }
	MonItem		*Next(MonItem *i) const { return i->next; }
	

    private:
	char		*mem;
	long		mlen;
	MemFile		*mf;

	void		LoadMem(void *);
	long		ComputeSize();
};
