/*
	xmm_arcsincos.c
	xmmLibm

	Created by Ian Ollmann, Ph.D. on 8/2/05.
	Copyright Â© 2005 Apple Computer, Inc. All rights reserved.

	asin, acos, and atan were removed 12/7/2007 and replaced by new
	implementations in asin.c, acos.c, and atan.c.
 */


#include "xmmLibm_prefix.h"
#include <math.h>

double atan2( double x, double y )
{
	return atan2l( (long double) x, (long double) y );
}
