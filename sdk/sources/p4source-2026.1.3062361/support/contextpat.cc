/*
 * Copyright 1996, 2022 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */


/*
 * Implement the Context design pattern
 *
 * This allows a function to query the setting of some state setting
 * that is determined by the last context object to exist on the stack.
 * That lets you query some ealier setting wihtout having to pass the
 * value via stack parameters.
 * Once the context goes out of scope the state reverts back
 * to it's previous pre-context state.
 *
 * There are 32bits of state available and the bits of interest
 * are set by specifying a mask and value to the constructor.
 * The mask parameter sets which bits are to be worked on,
 * and the value supplies the values to be placed in the bits
 * to be worked on. The initial state of the bits are all off.
 * So with a mask of 2 and a value of 2 we would get a setting of
 * 2. If that is follwed by a new context with mask of 1 and value of 1
 * while the old context is valid, then the new value would be 0x3.
 *
 * The code assumes that the contexts are ALWAYS popped in reverse
 * construction order, so don't go creating these contexts on the heap
 * unless you know the pop order will be maintained. (We could use
 * a linked-list to allow this)
 *
 * Works by using a thread local variable pointer that links all the
 * current contexts together in a single link list.
 */

# include <stdhdrs.h>
# include <contextpat.h>

MT_STATIC ContextPat *contextpat_ptr = 0;

ContextPat::ContextPat(unsigned int mask, unsigned int value)
{
	if( contextpat_ptr )
	    current = contextpat_ptr->current;
	else
	    current = 0;
	last = contextpat_ptr;
	contextpat_ptr = this;

	current &= ~(mask & ~value);
	current |= (mask & value);
}

ContextPat::~ContextPat()
{
	contextpat_ptr = last;
}

// This is static
unsigned int
ContextPat::is_set(unsigned int mask)
{
	unsigned int curr = 0;
	if( contextpat_ptr )
	    curr = contextpat_ptr->current & mask;
	return curr;
}
