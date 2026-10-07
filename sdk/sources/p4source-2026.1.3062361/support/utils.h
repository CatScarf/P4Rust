/*
 * @file utils.h
 *
 * @brief miscellaneous utilties available to all
 *
 * Threading: none
 *
 * @invariants:
 *
 * Copyright (c) 2012 Perforce Software
 * Confidential.  All Rights Reserved.
 * @author Wendy Heffner
 *
 * Creation Date: December 5, 2012
 */

// global utility funciton

/**
 * ISHex
 * - return true if c is a valid character for a hex value, 
 * - otherwise return false.
 */

bool IsHex( char c );

/**
 * FindRevspec
 * - return a pointer to the beginning of a revspec in a string
 * - return 0 if no revspec is present
 */

const char *FindRevspec( const char *p, int n );
