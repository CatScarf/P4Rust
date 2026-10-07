// -*- mode: C++; tab-width: 4; -*-
// vi:ts=4 sw=4 noexpandtab autoindent

/**
 * uuidstrong.h
 *
 * Description:
 *	Perforce wrapper for boost::uuid (Boost implementation of UUID support).
 *
 * Invariants:
 *
 * Copyright (c) 2011 Perforce, Inc.
 * Confidential.  All Rights Reserved.
 * Original Author:	Mark Wittenberg
 *
 * IDENTIFICATION
 *    $Header: $
 */

#include <string>
#include <iosfwd>

// opaque storage replacement for boost::uuids:uuid
class uuid_storage {
public:
	enum { kUuidSize = 16 };

	// buffer big enough to hold the raw uuid data (implementation-dependent)
	typedef BaseUUID::ValueType	DataType[kUuidSize];

	uuid_storage()
	{
		memset( m_uuid, 0, kUuidSize );
	}

	uuid_storage(
		const uuid_storage	&rhs)
	{
		Set( rhs.m_uuid );
	}

	const uuid_storage	&
	operator=(
		const uuid_storage	&rhs)
	{
		if( this != &rhs ) {
			Set( rhs.m_uuid );
		}

		return *this;
	} // op=

	bool
	operator==(
		const uuid_storage	&rhs) const
	{
		if( this == &rhs ) {
			return true;
		}

		return memcmp(m_uuid, rhs.m_uuid, kUuidSize) == 0;
	} // op==

	void
	swap(
		uuid_storage	&rhs)
	{
		uuid_storage	tmp = *this;
		*this = rhs;
		rhs = tmp;
	} // Swap

	void
	Set( const DataType data )
	{
		memcpy(m_uuid, data, kUuidSize);
	}

	void
	Get( DataType data ) const
	{
		memcpy(data, m_uuid, kUuidSize);
	}

	BaseUUID::ValueType *
	Get()
	{
		return m_uuid;
	}

	// iterators

	virtual BaseUUID::iterator
	begin()
	{
	    return &m_uuid[0];
	}

	virtual BaseUUID::iterator
	end()
	{
	    return &m_uuid[kUuidSize];
	}

	virtual BaseUUID::const_iterator
	begin() const
	{
	    return &m_uuid[0];
	}

	virtual BaseUUID::const_iterator
	end() const
	{
	    return &m_uuid[kUuidSize];
	}

private:
	DataType	m_uuid;
};

class StrongUUID : public BaseUUID {
public:
	// Orthodox Canonical Form (OCF) methods
	StrongUUID();

	StrongUUID(
		int		val);

	StrongUUID(
		const StrongUUID	&rhs);

	~StrongUUID();

	const StrongUUID &
	operator=(
		const StrongUUID	&rhs);


	virtual bool
	operator==(
		const StrongUUID	&rhs) const;

	virtual bool
	operator!=(
		const StrongUUID	&rhs) const;

	// accessors

	virtual bool
	IsNil() const;

	virtual unsigned int
	Hash() const;

	virtual TypeVariant
	VariantType() const;

	virtual TypeVersion
	VersionType() const;

	static unsigned int
	Size();

	virtual unsigned int
	SizeofData() const;

	// mutators

	virtual void
	Swap(
		StrongUUID	&rhs);

	// Other methods

	virtual std::ostream &
	Stream(
		std::ostream	&os) const;

	virtual std::string
	ToString() const;

	// iterators

	virtual iterator
	begin()
	{
	    return m_uuid.begin();
	}

	virtual iterator
	end()
	{
	    return m_uuid.end();
	}

	virtual const_iterator
	begin() const
	{
	    return m_uuid.begin();
	}

	virtual const_iterator
	end() const
	{
	    return m_uuid.end();
	}

	// Other methods

	// Set "buf" to the formatted hexadecimal character representation
	// of this UUID.
	virtual StrPtr
	ToStrBuf(StrBuf	&buf) const;

	friend std::ostream &
	operator<<(
		std::ostream	&os,
		const StrongUUID
						&uuid);

protected:

private:
	//boost::uuids::uuid	m_uuid;
	/*
	 * I don't want to pollute the p4 namespace with boost,
	 * so I'm declaring the storage here.
	 * If boost changes the storage type then this will break.  Sigh.
	 */
	uuid_storage	m_uuid;
};

/**
 * op<<
 */
inline std::ostream &
operator<<(
	std::ostream	&os,
	const StrongUUID
					&uuid)
{
	return uuid.Stream(os);
} // op<<
