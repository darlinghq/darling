
/*
 *	required_arithmetic.h
 *
 *		by Ian Ollmann
 *
 *	Copyright (c) Apple Inc, 2007. All Rights Reserved.
 *
 *	The function of this header is to provide various cLibm routines with
 *	interfaces that cause arithmetic to *always* happen. This header is needed
 *	in various places in Libm, because we use arithmetic to set floating point
 *	state flags. Frequently the product of that arithmetic is not used for the
 *	numerical result of the function, so the compiler erroneously optimizes 
 *	it away. The intent of this header is to provide basic operators that 
 *	utilize compiler and/or platform specific devices (such as volatile asms) 
 *	to prevent the compiler from optimizing away the arithmetic.  If no such
 *	device is available, you can encapsulate these operations as non-inlined
 *	functions with the implementation in a separate compilation unit or static 
 *	library as required to prevent the compiler from removing them.
 *
 *	Sample implementations are provided for __i386__, both as sample code and
 *	so we can test the library.  Other architectures are required to provide
 *	their own implementation.
 *
 */
 
#if defined( __GNUC__ )
	#define ALWAYS_INLINE_NO_DEBUG	__attribute__ (( __always_inline__, __nodebug__ ))
#else
	#define ALWAYS_INLINE_NO_DEBUG
#endif

#ifndef __arm__
	#error  This file is for ARM with VFP only
#endif


#pragma mark -
#pragma mark Declarations

static inline float required_add_float( float a, float b )	ALWAYS_INLINE_NO_DEBUG;
static inline float required_multiply_float( float a, float b ) ALWAYS_INLINE_NO_DEBUG;
static inline float required_divide_float( float a, float b ) ALWAYS_INLINE_NO_DEBUG;
static inline int required_convert_float_to_int( float a ) ALWAYS_INLINE_NO_DEBUG;

static inline double required_add_double( double a, double b ) ALWAYS_INLINE_NO_DEBUG;
static inline double required_multiply_double( double a, double b ) ALWAYS_INLINE_NO_DEBUG;
static inline double required_divide_double( double a, double b ) ALWAYS_INLINE_NO_DEBUG;
static inline int required_convert_double_to_int( double a ) ALWAYS_INLINE_NO_DEBUG;

#pragma mark -
#pragma mark Implementation

// --------------------- single precision -------------------------

static inline float required_add_float( float a, float b )
{
	register float r;
	__asm__ __volatile__ ( "fadds	%0, %1, %2" : "=w" (r) : "w" (a), "w" (b) );
	return r;
}

static inline float required_multiply_float( float a, float b )
{
	register float r;
	__asm__ __volatile__ ( "fmuls	%0, %1, %2" : "=w" (r) : "w" (a), "w" (b) );
	return r;
}

static inline float required_divide_float( float a, float b )
{
	register float r;
	__asm__ __volatile__ ( "fdivs	%0, %1, %2" : "=w" (r) : "w" (a), "w" (b) );
	return r;
}

//rounds toward zero
static inline int required_convert_float_to_int( float a )
{
	register float temp;
	register int result;
	
	__asm__ __volatile__ ( "ftosizs %0, %1" : "=w" (temp) : "w" (a) );
	__asm__ __volatile__ ( "fmrs %0, %1" : "=r" (result) : "w" (temp) );

	return result;
}

// ---------------- double precision --------------------------

static inline double required_add_double( double a, double b )
{
	register double r;
	__asm__ __volatile__ ( "faddd	%P0, %P1, %P2" : "=w" (r) : "w" (a), "w" (b) );
	return r;
}

static inline double required_multiply_double( double a, double b )
{
	register double r;
	__asm__ __volatile__ ( "fmuld	%P0, %P1, %P2" : "=w" (r) : "w" (a), "w" (b) );
	return r;
}

static inline double required_divide_double( double a, double b )
{
	register double r;
	__asm__ __volatile__ ( "fdivd	%P0, %P1, %P2" : "=w" (r) : "w" (a), "w" (b) );
	return r;
}

//rounds toward zero
static inline int required_convert_double_to_int( double a )
{
	register float temp;
	register int result;
	
	__asm__ __volatile__ ( "ftosizd %0, %P1" : "=w" (temp) : "w" (a) );
	__asm__ __volatile__ ( "fmrs %0, %1" : "=r" (result) : "w" (temp) );

	return result;
}

