// -*- mode: C++; tab-width: 8; -*-
// vi:ts=8 sw=4 noexpandtab autoindent

/**
 * uuidimpl.h
 *
 * Description:
 *	Select our UUID implementation class
 */

// looks EC doesn't sync Boost, so disable StrongUUID for now
#if 1
// alias UUID to WeakUUID
class WeakUUID;
typedef WeakUUID	UUID;
#include "uuidweak.h"
#else
// alias UUID to StrongUUID
class StrongUUID;
typedef StrongUUID	UUID;
#include "uuidstrong.h"
#endif
