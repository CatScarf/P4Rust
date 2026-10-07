/*
 * Copyright 1995, 2020 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

class MemFile {

    public:

	MemFile() : base(0), sz(0) {}
	MemFile( const char *f, Error *e ) : base(0), sz(0) { Open(f, e); }
	~MemFile() { Close(); }
	void Close();
	void Open( const char *, Error * );
	void Open( const char *, unsigned long );
	void *Addr() const { return base; }
	unsigned long Size() const { return sz; }

    private:
	void *base;
	unsigned long sz;

# ifdef OS_NT
	HANDLE f, fm;
# else
	int f;
# endif
};
