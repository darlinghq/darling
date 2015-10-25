
/*
 *	lgammaf.c
 *
 *		by Ian Ollmann
 *
 *	Copyright (c) 2007, Apple Inc.  All Rights Reserved.
 */
 
extern double lgamma(double);
extern double lgamma_r(double, int *);

float lgammaf( float );
float lgammaf( float x )
{
	return (float) lgamma( (double) x );
}

float lgammaf_r( float, int *);
float lgammaf_r( float x, int *psigngam )
{
	return (float) lgamma_r((double) x, psigngam);
}
