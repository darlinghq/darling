/* @(#)s_ldexp.c 5.1 93/09/24 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

#include <sys/cdefs.h>
#if defined(LIBM_SCCS) && !defined(lint)
__RCSID("$NetBSD: s_ldexp.c,v 1.8 1999/07/02 15:37:43 simonb Exp $");
#endif

#include "math.h"
#include "math_private.h"
#include <errno.h>

long double ldexpl(long double value, int exp)
{
	if(!isfinite(value)||value==0.0) return value;
	value = scalbnl(value,exp);
//	if(!finitel(value)||value==0.0) errno = ERANGE;
	return value;
}
