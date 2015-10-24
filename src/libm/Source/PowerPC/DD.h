/*
 * Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
//
//	DD.h
//	
//	Double-double Function Library
//	Copyright: © 1995-96 by Apple Computer, Inc., all rights reserved
//
//  september 03 1996	ali		deleted entries that conflicted with <fp.h>
//
#include "fenv_private.h"
#include "stdint.h"

typedef union {
	uint32_t i;
	float f;
} Float;

typedef union {
	uint32_t i[2];
	double f;
} Double;

typedef union {
	uint32_t i[4];
	double d[2];
	long double	f;
} DblDbl;


// d3ops.c  (private triple-double basic operations required by ErfDD.c and GammaDD.c)
	
void _d3div(double a, double b, double c, double d, double e, double f,
			double *high, double *mid, double *low);
void _d3mul(double a, double b, double c, double d, double e, double f,
			double *high, double *mid, double *low);
void _d3add(double a, double b, double c, double d, double e, double f,
			double *high, double *mid, double *low);


long double _ExpInnerLD(double, double, double, double *, int);

long double _LogInnerLD(double, double, double, double *, int);

long double __sqrtldextra( double x, double y, double *pextra );

