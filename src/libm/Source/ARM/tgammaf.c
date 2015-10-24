/*
 *	tgammaf.c
 *
 *		by Ian Ollmann
 *
 *	Copyright (c) 2007, Apple Inc.  All Rights Reserved.
 *
 */
 
#include <math.h>

float tgammaf( float x )
{
	return (float) tgamma( (double) x );
}
