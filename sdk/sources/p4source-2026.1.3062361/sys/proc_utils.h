/*
 * Copyright 1995, 2022 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

// Utilities related to configuring OS-level processes.

# ifdef HAS_CPP11

class OSProcessUtils
{
    public:

	OSProcessUtils();
	~OSProcessUtils();

	// Linux-specific.
	bool	HideFromOOMKiller( Error* e, const bool verify );
	bool	TestHideFromOOMKiller();
	void	ResetOOMKillerStatus( Error* e );
#ifdef OS_NT
	static int	GetCurrentExecutablePath( StrBuf *exename, Error *e );
#endif

    private:

	// This variant does not perform the up-front "Can I do this?" check.
	bool	HideFromOOMKillerRaw( Error* e, const bool verify );

	void	WriteOOMFile( const StrPtr* buf, Error* e ) const;
	void	ReadOOMFile( StrBuf& buf, Error* e ) const;

	bool	isHidden;

} ;

# endif // HAS_CPP11
