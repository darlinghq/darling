/*
 *  xmm_dimMinMax.c
 *  xmmLibm
 *
 *  Created by Ian Ollmann, Ph.D. on 8/15/05.
 *  Copyright © 2005 Apple Computer, Inc. All rights reserved.
 *
 */


#include "xmmLibm_prefix.h"
#include "math.h"

#if defined( BUILDING_FOR_CARBONCORE_LEGACY )
double fdim( double x, double y )
{
    xDouble xx = DOUBLE_2_XDOUBLE(x);
    xDouble yy = DOUBLE_2_XDOUBLE(y);

	xDouble isNaN = _mm_cmpunord_pd( xx, yy );
	xDouble testX = _mm_andnot_pd( isNaN, xx );
	xDouble testY = _mm_andnot_pd( isNaN, yy );
	xDouble xGTy = _mm_cmplt_sd( testY, testX );
	xGTy = _mm_or_pd( xGTy, isNaN );
	
    xx = _mm_and_pd( xx, xGTy );
    yy = _mm_and_pd( yy, xGTy );
    xx = _mm_sub_sd( xx, yy );
	
    return XDOUBLE_2_DOUBLE( xx );
}

double fmax( double x, double y )
{
    xDouble xx = DOUBLE_2_XDOUBLE(x);
    xDouble yy = DOUBLE_2_XDOUBLE(y);
	
	xDouble isNaN = _mm_cmpunord_pd( xx, yy );
	xDouble yIsNaN = _mm_cmpunord_pd( yy, yy );
	xDouble xEQy = _mm_cmpeq_pd( xx, yy );
	xDouble safeX = _mm_andnot_pd( isNaN, xx );
	xDouble safeY = _mm_andnot_pd( isNaN, yy );
	xDouble xGTy = _mm_cmplt_sd( safeY, safeX );
	xDouble xGEy = _mm_or_pd( xGTy, xEQy );
	xGEy = _mm_or_pd( xGEy, yIsNaN );
    
	xx = _mm_sel_pd( yy, xx, xGEy );
		    
    return XDOUBLE_2_DOUBLE( xx );
}

double fmin( double x, double y )
{
    xDouble xx = DOUBLE_2_XDOUBLE(x);
    xDouble yy = DOUBLE_2_XDOUBLE(y);
	
	xDouble isNaN = _mm_cmpunord_pd( xx, yy );
	xDouble yIsNaN = _mm_cmpunord_pd( yy, yy );
	xDouble xEQy = _mm_cmpeq_pd( xx, yy );
	xDouble safeX = _mm_andnot_pd( isNaN, xx );
	xDouble safeY = _mm_andnot_pd( isNaN, yy );
	xDouble xLTy = _mm_cmplt_sd( safeX, safeY );
	xDouble xLEy = _mm_or_pd( xLTy, xEQy );
	xLEy = _mm_or_pd( xLEy, yIsNaN );
    
	xx = _mm_sel_pd( yy, xx, xLEy );
		    
    return XDOUBLE_2_DOUBLE( xx );
}

#else
float fdimf( float x, float y )
{
    xFloat xx = FLOAT_2_XFLOAT(x);
    xFloat yy = FLOAT_2_XFLOAT(y);

	xFloat isNaN = _mm_cmpunord_ps( xx, yy );
	xFloat testX = _mm_andnot_ps( isNaN, xx );
	xFloat testY = _mm_andnot_ps( isNaN, yy );
	xFloat xGTy = _mm_cmplt_ss( testY, testX );
	xGTy = _mm_or_ps( xGTy, isNaN );
	
    xx = _mm_and_ps( xx, xGTy );
    yy = _mm_and_ps( yy, xGTy );
    xx = _mm_sub_ss( xx, yy );
	
    return XFLOAT_2_FLOAT( xx );
}

float fmaxf( float x, float y )
{
    xFloat xx = FLOAT_2_XFLOAT(x);
    xFloat yy = FLOAT_2_XFLOAT(y);
	
	xFloat isNaN = _mm_cmpunord_ps( xx, yy );
	xFloat yIsNaN = _mm_cmpunord_ps( yy, yy );
	xFloat xEQy = _mm_cmpeq_ss( xx, yy );
	xFloat safeX = _mm_andnot_ps( isNaN, xx );
	xFloat safeY = _mm_andnot_ps( isNaN, yy );
	xFloat xGTy = _mm_cmplt_ss( safeY, safeX );
	xFloat xGEy = _mm_or_ps( xGTy, xEQy );
	xGEy = _mm_or_ps( xGEy, yIsNaN );
    
	xx = _mm_sel_ps( yy, xx, xGEy );
		    
    return XFLOAT_2_FLOAT( xx );
}

float fminf( float x, float y )
{
    xFloat xx = FLOAT_2_XFLOAT(x);
    xFloat yy = FLOAT_2_XFLOAT(y);
	
	xFloat isNaN = _mm_cmpunord_ps( xx, yy );
	xFloat yIsNaN = _mm_cmpunord_ps( yy, yy );
	xFloat xEQy = _mm_cmpeq_ps( xx, yy );
	xFloat safeX = _mm_andnot_ps( isNaN, xx );
	xFloat safeY = _mm_andnot_ps( isNaN, yy );
	xFloat xLTy = _mm_cmplt_ss( safeX, safeY );
	xFloat xLEy = _mm_or_ps( xLTy, xEQy );
	xLEy = _mm_or_ps( xLEy, yIsNaN );
    
	xx = _mm_sel_ps( yy, xx, xLEy );
		    
    return XFLOAT_2_FLOAT( xx );
}

#endif /* CARBONCORE_LEGACY */


