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
//	d3ops.c
//	
//	Double-double Function Library
//	Copyright: © 1995-96 by Apple Computer, Inc., all rights reserved
//	
//	Triple-double precision basic operations.
//	
//			(high, mid, low) <-- (a, b, c) <op> (d, e, f)
//				where <op> is add, multiply, or divide
//	
//	void _d3div(double a, double b, double c, double d, double e, double f,
//				double *high, double *mid, double *low);
//	void _d3mul(double a, double b, double c, double d, double e, double f,
//				double *high, double *mid, double *low);
//	void _d3add(double a, double b, double c, double d, double e, double f,
//				double *high, double *mid, double *low);
//

#include "math.h"
#include "fp_private.h"
#if (__WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L)
#include "DD.h"

static const hexdouble infinity  = {{0x7ff00000, 0x00000000}};
static const hexdouble tiny      = {{0x00100000, 0x00000000}};


void _d3div(double a, double b, double c, double d, double e, double f,
			double *high, double *mid, double *low)
{
	//  (high, mid, low) <-- (a, b, c) / (d, e, f)
	
	double p, q, r, s, t, u, v, w, x, y, z;
	double g, h, o, rmid, rlow;
	// int lx, l1, l2;
	
	// lx = (fabs(d) < tiny.d);			// look out for overflow on 1/d
	p = 1.0/d;								// reciprocal of h.o. denominator term
	if (fabs(d) < tiny.d) {
		r=a/d;								// if quotient is infinity,
		if (fabs(r)==infinity.d) {		// stop process, and return
			*high = r;
			*mid = 0.0e0;
			*low = 0.0e0;					// properly signed infinity
			return;
		}
	}
	else {
		q = a*p;
		r = q + (a - q*d)*p;				// h.o. quotient term
	}
	s = a - r*d;							// exact remainder
	t = __FMUL( r, e);						// second order remainder term
	// l1 = (fabs(b) > fabs(t))
	u = (r*e - t) + r*f;					// third order remainder term
	w = __FSUB( b, t );						// combine the
	y = __FADD( s, w );						// second order terms
	// l2 = (fabs(s) > fabs(w));		// and catch bits that may fall
	
	if (fabs(b) > fabs(t))				// off the end to include in the
		x = b - w - t;						// third order term
	else
		x = b - (w + t);
	if (fabs(s) > fabs(w))
		z = s - y + w;
	else
		z = w - y + s;
	g = c - u + x + z;						// third order term
	if (fabs(d) < tiny.d) {				// caution against tiny quotients, again
		o = y/d;
		v = (y - __FMUL( o, d ) + g - (e*o + (f*o))) / d;
	}
	else {
		h = y*p;
		o = h + (y - h*d)*p;				// second order term
		v = (y - o*d + g - (e*o + (f*o)))*p;// third order quotient term
	}
	rmid = o + v;							// start distillation of quotient
	rlow = o - rmid + v;
	
	*high = __FADD( r, rmid );
	*mid  = (r - *high + rmid) + rlow;
	*low  = (r - *high + rmid) - *mid + rlow;
	
	return;
}


void _d3mul(double a, double b, double c, double d, double e, double f,
			double *high, double *mid, double *low)
{
	//  (high, mid, low) <-- (a, b, c) * (d, e, f)
	
	double s, t, u, v, w, x, y, z, tlow, resbump;
	// int l1, l2;
	
	u = d*c + e*b + f*a;					// sextuple precision multiplication
	v = __FMUL( d, b );								// derived from _q_mp10.c
	w = d*b - v;
	x = __FMUL( e, a );
	y = e*a - x;
	// l1=(fabs(x) > fabs(v));
	s = w + y + u;
	z = __FMUL( d, a);
	w = d*a - z;
	u = __FADD( v, x );
	// l2=(fabs(w) > fabs(u));
	if (fabs(x) > fabs(v))
		s = x - u + v + s;
	else
		s = v - u + x + s;
	t = __FADD( u, w );
	if (fabs(w) > fabs(u))
		tlow = w - t + u + s;
	else
		tlow = u - t + w + s;
	resbump = __FADD( t, tlow );
	tlow = t - resbump + tlow;
	
	*high = __FADD( z, resbump );
	*mid  = (z - *high + resbump) + tlow;
	*low  = (z - *high + resbump) - *mid + tlow;
	
	return;
}


void _d3add(double a, double b, double c, double d, double e, double f,
			double *high, double *mid, double *low)
{		 
	//  (high, mid, low) <-- (a, b, c) + (d, e, f)
	
	// int l1, l2, l3, l4, l5, l5, l6, l7, l8;
	double p, q, r, s, t, u, v, w, x, y, z;
	double res, carry, z1, z2, resmid, reslow;
	
	// l1=(fabs(a) > fabs(d));
	// l2=(fabs(b) > fabs(e));
	// l3=(fabs(c) > fabs(f));
	
	p = __FADD( a, d );
	q = __FADD( b, e );
	r = __FADD( c, f );
	
	if (fabs(a) > fabs(d))			// a    b      c
		s = a - p + d;					//  d    e      f
	else								// p    s 	  (replaces a and d
		s = d - p + a;
	
	if (fabs(b) > fabs(e))
		t = b - q + e;					//      q       t   (replaces b and e
	else
		t = e - q + b;
	
	// l4=(fabs(s) > fabs(q));
	// l5=(fabs(t) > fabs(r));
	
	if (fabs(c) > fabs(f))
		u = c - r + f;					// 	      r    u  (replaces c and f
	else
		u = f - r + c;
	
	v = __FADD( s, q );
	x = __FADD( t, r );
	
	// l6=(fabs(p) > fabs(v));
	
	if (fabs(s) > fabs(q))
		w = s - v + q;					//      v       w     (replaces s and q
	else
		w = q - v + s;
	
	// l7=(fabs(x) > fabs(w));
	
	if (fabs(t) > fabs(r))
		y = t - x + r;					// 	      x    y  (replaces t and r
	else
		y = r - x + t;
	
	z = __FADD( x, w );
	res = __FADD( p, v );
	
	if (fabs(p) > fabs(v))
		carry = p - res + v;			// res	carry	     (replaces p and v
	else
		carry = v - res + p;
	
	// l8=(fabs(carry) > fabs(z));
	
	if (fabs(x) > fabs(w))
		z1 = x - z + w;					// 	       z  z1  (replaces x and w
	else
		z1 = w - z + x;
	
	z2 = u + y + z1;					// 		  z2  (replaces u, y, and z1
	resmid = __FADD( carry, z );
	
	if (fabs(carry) > fabs(z))
		reslow = carry - resmid + z;	//     resmid carry2  (replaces carry, z
	else
		reslow = z - resmid + carry;
	
	*high = __FADD( res, resmid );
	*mid = (res - *high + resmid) + reslow;
	*low = (res - *high + resmid) - *mid + reslow + z2;

	return;
}
#endif
#if 0	// set to 1 to include code for self-contained sanity test

#include <stdio.h>

main()
{
	double a1,a2,a3, b1,b2,b3, c1,c2,c3, d1,d2,d3, e1,e2,e3;
	
	_d3div( 2,  0,  0,  3,  0,  0, &a1, &a2, &a3);	// a = 2/3
	_d3div( 5,  0,  0,  7,  0,  0, &b1, &b2, &b3);	// b = 5/7
	_d3mul(a1, a2, a3, b1, b2, b3, &c1, &c2, &c3);	// c = a*b = 10/21
	_d3div(21,  0,  0, 10,  0,  0, &d1, &d2, &d3);	// d = 21/10
	_d3mul(c1, c2, c3, d1, d2, d3, &e1, &e2, &e3);	// e = c*d (should be 1)
	
	printf("e: %24.18e %24.18e %24.18e\n", e1, e2, e3);
	
	_d3add(a1, a2, a3, b1, b2, b3, &c1, &c2, &c3);	// c = a+b = 29/21
	_d3div( 8,  0,  0, 21,  0,  0, &d1, &d2, &d3);	// d = 8/21
	_d3add(c1, c2, c3,-d1,-d2,-d3, &e1, &e2, &e3);	// e = c-d (should be 1)
	
	printf("e: %24.18e %24.18e %24.18e\n", e1, e2, e3);
}

// Correct test results:
// 
// e: 1.000000000000000000e+00 0.000000000000000000e+00 0.000000000000000000e+00
// e: 1.000000000000000000e+00 0.000000000000000000e+00 0.000000000000000000e+00

#endif
