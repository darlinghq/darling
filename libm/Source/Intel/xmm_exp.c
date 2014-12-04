/*
 *  xmm_exp.c
 *  xmmLibm
 *
 *  Created by Ian Ollmann, Ph.D. on 6/30/05.
 *  Copyright © 2005 Apple Computer, Inc. All rights reserved.
 *
 *  Constants from original typing by Earl Killian of MIPS on March 23rd, 1992.         
 *  Converted from pairs of 32-bit hexadecimal words to C99 notation, by Ian Ollmann
 *
 *  Algorithm from Peter Tang:
 *
 *      ACM Transactions on Mathematical Software, Vol 15, 2, June 1989, pp 144-157
 *      ACM Transactions on Mathematical Software, Vol 18, 2, June 1992, pp 211-222
 *      
 */


#include "xmmLibm_prefix.h"

#include "math.h"
#include "fenv.h"


#pragma mark exp

//Functions implemented here
static inline double _xexp( double x )   ALWAYS_INLINE;

static const hexdouble S_Lead[32] =      
     { { 0x3FF0000000000000ULL },
       { 0x3FF059B0D3158540ULL },
       { 0x3FF0B5586CF98900ULL },
       { 0x3FF11301D0125B40ULL },
       { 0x3FF172B83C7D5140ULL },
       { 0x3FF1D4873168B980ULL },
       { 0x3FF2387A6E756200ULL },
       { 0x3FF29E9DF51FDEC0ULL },
       { 0x3FF306FE0A31B700ULL },
       { 0x3FF371A7373AA9C0ULL },
       { 0x3FF3DEA64C123400ULL },
       { 0x3FF44E0860618900ULL },
       { 0x3FF4BFDAD5362A00ULL },
       { 0x3FF5342B569D4F80ULL },
       { 0x3FF5AB07DD485400ULL },
       { 0x3FF6247EB03A5580ULL },
       { 0x3FF6A09E667F3BC0ULL },
       { 0x3FF71F75E8EC5F40ULL },
       { 0x3FF7A11473EB0180ULL },
       { 0x3FF82589994CCE00ULL },
       { 0x3FF8ACE5422AA0C0ULL },
       { 0x3FF93737B0CDC5C0ULL },
       { 0x3FF9C49182A3F080ULL },
       { 0x3FFA5503B23E2540ULL },
       { 0x3FFAE89F995AD380ULL },
       { 0x3FFB7F76F2FB5E40ULL },
       { 0x3FFC199BDD855280ULL },
       { 0x3FFCB720DCEF9040ULL },
       { 0x3FFD5818DCFBA480ULL },
       { 0x3FFDFC97337B9B40ULL },
       { 0x3FFEA4AFA2A490C0ULL },
       { 0x3FFF50765B6E4540ULL } };

static const hexdouble S_Trail[32] =              
     { { 0x0000000000000000ULL },
       { 0x3D0A1D73E2A475B4ULL },
       { 0x3CEEC5317256E308ULL },
       { 0x3CF0A4EBBF1AED93ULL },
       { 0x3D0D6E6FBE462876ULL },
       { 0x3D053C02DC0144C8ULL },
       { 0x3D0C3360FD6D8E0BULL },
       { 0x3D009612E8AFAD12ULL },
       { 0x3CF52DE8D5A46306ULL },
       { 0x3CE54E28AA05E8A9ULL },
       { 0x3D011ADA0911F09FULL },
       { 0x3D068189B7A04EF8ULL },
       { 0x3D038EA1CBD7F621ULL },
       { 0x3CBDF0A83C49D86AULL },
       { 0x3D04AC64980A8C8FULL },
       { 0x3CD2C7C3E81BF4B7ULL },
       { 0x3CE921165F626CDDULL },
       { 0x3D09EE91B8797785ULL },
       { 0x3CDB5F54408FDB37ULL },
       { 0x3CF28ACF88AFAB35ULL },
       { 0x3CFB5BA7C55A192DULL },
       { 0x3D027A280E1F92A0ULL },
       { 0x3CF01C7C46B071F3ULL },
       { 0x3CFC8B424491CAF8ULL },
       { 0x3D06AF439A68BB99ULL },
       { 0x3CDBAA9EC206AD4FULL },
       { 0x3CFC2220CB12A092ULL },
       { 0x3D048A81E5E8F4A5ULL },
       { 0x3CDC976816BAD9B8ULL },
       { 0x3CFEB968CAC39ED3ULL },
       { 0x3CF9858F73A18F5EULL },
       { 0x3C99D3E12DD8A18BULL } };

static const double scalars_data[2] = { 0x1.0000000000000p+54, 0x1.0000000000000p-54 };
static const int32_t exponents_data[2] = { 0x7920, 0x86a0 };
static const double * const scalars = &scalars_data[1];

static const xDouble xNaN = { NAN, NAN };
static const xDouble xInfinity = { INFINITY, INFINITY };
static const xDouble minusOneD = { -1.0, -1.0 };
static const DoubleConstant_sd Inv_L = GET_CONSTANT_sd(0x1.71547652B82FEp+5);   //46.1662413084468283841
static const DoubleConstant_sd A0 = GET_CONSTANT_sd(0x1.0000000000000p-1);      // 0.5
static const DoubleConstant_sd L1 = GET_CONSTANT_sd(0x1.62E42FEF00000p-6);      //0.0216608493901730980724
static const DoubleConstant_sd L2 = GET_CONSTANT_sd(-0x1.473DE6AF278EDp-39);    //-2.32519284687887401481e-12
static const DoubleConstant_sd A1 = GET_CONSTANT_sd(0x1.5555555548F7Cp-3);      //0.166666666665260865265
static const DoubleConstant_sd A2 = GET_CONSTANT_sd(0x1.5555555545D4Ep-5);      //0.0416666666662260792853
static const DoubleConstant_sd A3 = GET_CONSTANT_sd(0x1.11115B7AA905Ep-7);      //0.00833336798434219580556
static const DoubleConstant_sd A4 = GET_CONSTANT_sd(0x1.6C1728D739765p-10);     //0.00138889490863777190395
static const DoubleConstant_sd oneD = GET_CONSTANT_sd( 1.0 );
static const double largeD = 0x1.fffffffffffffp1022;
static const double tinyD = 0x1.0p-1022;

static inline double _xexp( double _x )    
{
	if( _x != _x || _x == __builtin_inf() )
		return _x + _x;
	
 
    static const double X1 = 0x1.62e42fefa39f0p+9;    // 7.09782712893384087e+02
    static const double X2 = -745.5;    
    static const double c0 = 1.5e-154;
	static const double half = 0.5;
    static const double infD = INFINITY;
	xDouble x = DOUBLE_2_XDOUBLE( _x );
    xDouble fabsx = _mm_andnot_pd( minusZeroD, x );
	
    xDouble xLTx1 = _mm_cmplt_sdm( x, &X1 );
    xDouble xGTx2 = _mm_cmpgt_sdm( x, &X2 );
    xDouble xIsSmall = _mm_cmplt_sdm( fabsx, &c0 );
    xDouble branchtest = _mm_andnot_pd( xIsSmall, _mm_and_pd( xLTx1, xGTx2 ) );
    xDouble result;
	
	// special case handling
    if( _mm_isfalse_sd( branchtest))
	{
		xDouble tiny = _mm_load_sd( &tinyD );
		xDouble one = _mm_load_sd( &oneD );
		xDouble minusInf = _mm_or_pd( minusZeroD, _mm_load_sd( &infD) );

	//if |x| < 1.5e-154, set inexact, return 1.0, unless -Inf, then go straight to zero
		result = _mm_andnot_pd( _mm_cmpeq_sd( x, minusInf ), _mm_add_sd( x, one ) );

	//if x >= X1.d, set inexact and overflow, return +Inf
		xDouble xGEx1 = _mm_cmpge_sdm( x, &X1 );
		xDouble multiplier = _mm_sel_pd( one, _mm_load_sd( &largeD ), xGEx1 );
	 
	//if x <= -745.5, set inexact and underflow, return 0.0
		xDouble xLEx2 = _mm_cmple_sdm( x, &X2 );
		multiplier = _mm_andnot_pd( xLEx2, multiplier );
		multiplier = _mm_or_pd( multiplier, tiny );

	//scale the result appropriately
		result = _mm_andnot_pd( minusZeroD, result );		//results of exp are always positive
		result = _mm_mul_sd( result, multiplier );
		result = _mm_mul_sd( result, multiplier );
		result = _mm_mul_sd( result, multiplier );

		return XDOUBLE_2_DOUBLE( result );
	}

    xDouble mask = _mm_cmpgt_sdm( x, (const double*) &minusZeroD );                //if (x > 0 )... was //if( (x > 1.5e-154) && (x < X1)) 
	xDouble sign = _mm_and_pd( x, minusZeroD );
    xDouble dN =_mm_mul_sdm( x, &Inv_L );
	dN = _mm_add_sd( dN, _mm_or_pd( sign, _mm_load_sd( &half ) ) );
    int N = _mm_cvttsd_si32( dN );
    int N2 = N & 0x1F;																// N2 = N & 0x1f;         //  N2 = N mod 32 with N2 in [0..31] even for N < 0
    dN = _mm_cvtsi32_sd( minusZeroD, N ); //double(N)
    xDouble R1 = _mm_sub_sd( x, _mm_mul_sdm( dN, &L1 ) );                           // R1 = x - N * L1.d;     /*  Leading part of the reduced arg   */
    xDouble R2 = _mm_mul_sdm( dN, &L2 );                                            //  Trailing part of the reduced arg  
    xDouble R =  _mm_add_sd( R1, R2 );                                              // Reduced arg
    xDouble P, Q;
    int index = _mm_cvtsi128_si32((xSInt32)mask);
    xDouble scalar = _mm_load_sd( &scalars[ index ] );
//    	Q = R*R*(A[0].d + R*(A[1].d + R*( A[2].d + R*( A[3].d + R*A[4].d))));
    Q = _mm_add_sdm( _mm_mul_sdm( R, &A4 ), &A3 );
    Q = _mm_add_sdm( _mm_mul_sd( Q, R ), &A2 );
    Q = _mm_add_sdm( _mm_mul_sd( Q, R ), &A1 );
    Q = _mm_add_sdm( _mm_mul_sd( Q, R ), &A0 );
    Q = _mm_mul_sd( _mm_mul_sd( Q, R ), R );
    
//    	P = R1 + ( R2 + Q );
    P = _mm_add_sd( _mm_add_sd( Q, R2 ), R1 );

    R1 = _mm_load_sd( &S_Lead[N2].d );            //R1 = S_Lead[N2].d;
    R2 = _mm_load_sd( &S_Trail[N2].d );           //R2 = S_Trail[N2].d;

//	x  = SCALB((R1 + (R2 + P*(R1 + R2)))*m54.d, (N - N2)/32 + 54);
    //    	t.d = 0.0;
    //    	t.i[0] = ((N-N2) + 0x86a0) << 15;
    N = N - N2;
    N += exponents_data[ index + 1];   //N = N - N2 + 0x86a0
    dN = (__m128d) _mm_cvtsi32_si128( N );
    dN = (__m128d) _mm_slli_epi64( (__m128i) dN, 15 + 32 );                                           //N <<= 15
    //    	result = ((R1 + (R2 + P*(R1 + R2)))*m54.d)*t.d;
    // note, must preserve order of operations here to keep precision
    R =  _mm_add_sd( R1, R2 );
    result = _mm_mul_sd( P, R );
    result = _mm_add_sd( result, R2 );
    result = _mm_add_sd( result, R1 );
    result = _mm_mul_sd( result, scalar );
    result = _mm_mul_sd( result, dN );

    return XDOUBLE_2_DOUBLE( result );
}


static inline double _xexp2( double x ) ALWAYS_INLINE;
static inline double _xexp2( double x )
{
    static const double conversion = 6.9314718055994530942E-1;

	//Early out for NaNs to avoid problems with invalid exceptions in next compare
	if( x != x )
		return x + x;
	
	//avoid underflow errors
	if( __builtin_fabs(x) < 0x1.0p-1020 )
		x = 0x1.0p-1020;
    
    return _xexp( x * conversion );
}


#if ! defined( BUILDING_FOR_CARBONCORE_LEGACY )
/*
double exp ( double x )
{
    return  _xexp( x );
}
 */

/*
float    expf( float x )
{//cheesy fallback on double, that probably fails to get the edge cases right.
    return  (float) _xexp( x );
}
*/

#pragma mark -
#pragma mark expm1

static const hexdouble Em1_A[9] =      
    { { 0x3FC5555555555549ULL },
       { 0x3FA55555555554B6ULL },
       { 0x3F8111111111A9F3ULL },
       { 0x3F56C16C16CE14C6ULL },
       { 0x3F2A01A01159DD2DULL },
       { 0x3EFA019F635825C4ULL },
       { 0x3EC71E14BFE3DB59ULL },
       { 0x3E928295484734EAULL },
       { 0x3E5A2836AA646B96ULL } };

static inline double _xexpm1( double x )   ALWAYS_INLINE;        

static inline double _xexpm1( double _x )    
{
	//Get the NaN and +inf cases out of the way
	if( EXPECT_FALSE( _x != _x ) || _x == __builtin_inf() )
		return _x + _x;

	xDouble x = DOUBLE_2_XDOUBLE( _x );
	
    //get old fp env, and set the default one
    static const DoubleConstant_sd Em1_Tiny = GET_CONSTANT_sd(0x1.0000000000000p-54); //5.55111512312578270212e-17
    static const DoubleConstant_sd Em1_Pos = GET_CONSTANT_sd(0x1.C4474E1726455p+10); //1809.11414126145723458
    static const DoubleConstant_sd Em1_Neg = GET_CONSTANT_sd(-0x1.2B708872320E1p+5); //-37.4299477502370407933
    static const DoubleConstant_sd Em1_T1 = GET_CONSTANT_sd(-0x1.269621134DB93p-2); //-0.287682072451780956879, log(1-1/4)
    static const DoubleConstant_sd Em1_T2 = GET_CONSTANT_sd(0x1.C8FF7C79A9A22p-3); //0.223143551314209764858, log(1+1/4)
    static const DoubleConstant_sd twoTOn7 = GET_CONSTANT_sd(0x1.0000000000000p-7); //        0.0078125, 2**-7
	static const DoubleConstant_sd half = GET_CONSTANT_sd( 0.5 );
    xDouble fabsx = _mm_andnot_pd( minusZeroD, x );
    xDouble xGEem1Neg = _mm_cmpge_sdm( x, &Em1_Neg );
    xDouble xLEem1Pos = _mm_cmple_sdm( x, &Em1_Pos );

	// special case handling
    if( _mm_isfalse_sd( _mm_and_pd( xLEem1Pos, xGEem1Neg )))
	{
		static const xDouble tiny = { 0x1.0p-1022, 0 }; 
		static const xDouble maxFinite = { 0x1.FFFFFFFFFFFFFp1023, 0 };
		static const xDouble mOneD = { -1.0, 0 };
		xDouble xLTzero = _mm_cmplt_sdm( x, (double*) &minusZeroD );
		xDouble xEQmInf = _mm_cmpeq_sdm( fabsx, (double*) &xInfinity );
		xDouble v = _mm_sel_pd( maxFinite, mOneD, xLTzero );
		xDouble v2 = _mm_sel_pd( maxFinite, _mm_andnot_pd( xEQmInf, tiny), xLTzero );
		x = _mm_add_sd( v, v2 );

		return XDOUBLE_2_DOUBLE( x );
	}

	xDouble xGEem1_t2 = _mm_cmpge_sdm( x, &Em1_T2 );
	xDouble xLEem1_t1 = _mm_cmple_sdm( x, &Em1_T1 );
	if( _mm_istrue_sd( _mm_or_pd( xGEem1_t2, xLEem1_t1 ) ) ) //if ((x >= Em1_T2.d) || (x <= Em1_T1.d))
	{
		//int N  = x*Inv_L.d + (x>0 ? 0.5 : -0.5);                  //  N = rint(x*Inv_L.d)
		//int N2 = N & 0x1f;                                        //  N2 = N mod 32 with N2 in [0..31] even for N < 0
		//int M  = (N - N2) / 32;
		xDouble dN = _mm_mul_sdm( x, &Inv_L );                      // dN = x * Inv_L
		xDouble signedHalf = _mm_sel_pd( _mm_load_sd( &half ), x, minusZeroD );
		dN = _mm_add_sd( dN, signedHalf );
		int N = _mm_cvttsd_si32( dN );                               // N = (int) rint( dN ), round to nearest mode used
		int N2 = N & 0x1f;                                          // N2 = N mod 32 with N2 in [0..31] even for N < 0
		int M  = (N - N2) / 32;
		dN = _mm_cvtsi32_sd( dN, N );
		xDouble R1 = _mm_sub_sd( x, _mm_mul_sdm( dN, &L1 ));        //x - N * L1;    /*  leading part of the reduced arg     */
		xDouble R2 = _mm_mul_sdm( dN, &L2 );                        //N * L2              trailing part of the reduced arg
		xDouble R =  _mm_add_sd( R1, R2 );                          // R1 + R2,  reduced argument

		//Q  = R*R*(A[0].d +  R*(A[1].d + R*(A[2].d + R*( A[3].d + R * A[4].d))));
		xDouble Q = _mm_add_sdm( _mm_mul_sdm( R, &A4 ), &A3 );
		Q = _mm_add_sdm( _mm_mul_sd( Q, R ), &A2 );
		Q = _mm_add_sdm( _mm_mul_sd( Q, R ), &A1 );
		Q = _mm_add_sdm( _mm_mul_sd( Q, R ), &A0 );
		Q = _mm_mul_sd( Q, _mm_mul_sd( R, R ) );
	   
		xDouble P = _mm_add_sd( _mm_add_sd( R2, Q ), R1 );          // P  = R1 + ( R2 + Q );
		xDouble S = _mm_add_sdm( _mm_load_sd( &S_Lead[N2].d), &S_Trail[N2].d );  //S  = S_Lead[N2].d + S_Trail[N2].d;
		xDouble twoM; 
		
		if( M >= 1024 )	//overflow
		{
			x = _mm_set_sd (0x1.0p1023);
			return XDOUBLE_2_DOUBLE( REQUIRED_ADD_sd( x, x ) );  //Note that simpler C syntax here is optimized away resulting in missing overflow flag.
		}
		else if( M >= 53 )
		{
			twoM = twoToTheM( M );
			//  x = SCALB ( 1.0, M ) * ( S_Lead[N2].d + ( S * P + ( S_Trail[N2].d - SCALB ( 1.0, -M ) ) ) );
			R = _mm_sub_sdm( twoToTheM( -M ), &S_Trail[N2].d );       //SCALB ( 1.0, -M ) - S_Trail[N2].d
			R = _mm_sub_sd( _mm_mul_sd( S, P ), R );                // S*P - (SCALB ( 1.0, -M ) - S_Trail[N2].d)
			R = _mm_add_sdm( R, &S_Lead[N2].d );                      // (S*P - (SCALB ( 1.0, -M ) - S_Trail[N2].d)) + S_Lead[N2]
			x = _mm_mul_sd( R, twoM );
		}
		else if( M <= -8 )
		{
			if( M >= -1023 )	
				twoM = twoToTheM( M );
			else
			{
				double zzz = scalbn( 1.0, M );
				twoM = DOUBLE_2_XDOUBLE( zzz ); 
			}
			
			//  x = SCALB ( 1.0, M ) * ( S_Lead[N2].d + ( S * P + S_Trail[N2].d ) ) - 1.0;
			R = _mm_add_sdm( _mm_mul_sd( S, P ), &S_Trail[N2].d );    //S * P + S_Trail[N2].d
			R = _mm_add_sdm( R, &S_Lead[N2].d);                       //S * P + S_Trail[N2].d + S_Lead[N2].d
			R = _mm_mul_sd( R, twoM );
			x = _mm_sub_sdm( R, &oneD );
		}
		else
		{
			twoM = twoToTheM( M );
			// x = SCALB ( 1.0, M ) * ( ( S_Lead[N2].d - SCALB ( 1.0, -M ) ) + ( S_Lead[N2].d * P + S_Trail[N2].d * ( 1.0 + P ) ) );
			R = _mm_add_sdm( P, &oneD );
			R = _mm_mul_sdm( R, &S_Trail[N2].d );
			R = _mm_add_sd( R, _mm_mul_sdm( P, &S_Lead[N2].d ) );             //( S_Lead[N2].d * P + S_Trail[N2].d * ( 1.0 + P ) )
			R = _mm_sub_sd( R, _mm_sub_sdm( twoToTheM( -M ), &S_Lead[N2].d ) );//( S_Lead[N2].d * P + S_Trail[N2].d * ( 1.0 + P ) ) - (SCALB ( 1.0, -M )  - S_Lead[N2].d)
			x = _mm_mul_sd( R, twoM ); 
		}
		
		// Merge new exceptions into old environment
		return XDOUBLE_2_DOUBLE( x );
	}

	xDouble xIsTiny = _mm_cmplt_sdm( fabsx, &Em1_Tiny );
	if( _mm_istrue_sd( xIsTiny ) )
	{
		static const double scale2 = 0x1.0000000000001p-1022;
		static const double half = 0.5;
		
		xDouble xEQzero = _mm_cmpeq_sd( x, minusZeroD ); 
		xDouble oneHalf = _mm_load_sd( &half );
		xDouble isDenorm = _mm_cmplt_sdm( fabsx, &tinyD );

		//scale out of denormal range (if not already)
		xDouble result = (xDouble) _mm_add_epi64( (xSInt64) x, (xSInt64) oneHalf );		
		oneHalf = _mm_and_pd( oneHalf, isDenorm );
		oneHalf = _mm_sel_pd( oneHalf, x, minusZeroD);
		result = _mm_sub_sd( result, oneHalf );		
				
		//scale back  (should set underflow)
		result = _mm_mul_sdm( result, &scale2 );	
		
		result = _mm_sel_pd( result, x, xEQzero );
		
		return XDOUBLE_2_DOUBLE( x );
	}
	else
	{
		xDouble U = _mm_cvtss_sd( x, _mm_cvtsd_ss( (xFloat)x, x ) );                      //U = ( double ) ( ( float ) x );
		xDouble V = _mm_sub_sd( x, U );                                     //V = x - U
		xDouble Y = _mm_mul_sdm( _mm_mul_sd( U, U ), &A0 );                 //Y = U * U * 0.5
		xDouble Z = _mm_mul_sdm( _mm_mul_sd(V, _mm_add_sd( x, U)), &A0 );   //Z = V * ( x + U ) * 0.5;      /* Y + Z = x*x/2 to 24 extra bits   */
		xDouble Q = _mm_add_sdm( _mm_mul_sdm( x, &Em1_A[8] ), &Em1_A[7] );  //Q = Em1_A[7].d + x * Em1_A[8].d;
		//for ( i = 0; i < 7; i++ )
		//  Q = Em1_A[6-i].d + x * Q;
		Q = _mm_add_sdm( _mm_mul_sd( Q, x ), &Em1_A[6] );
		Q = _mm_add_sdm( _mm_mul_sd( Q, x ), &Em1_A[5] );
		Q = _mm_add_sdm( _mm_mul_sd( Q, x ), &Em1_A[4] );
		Q = _mm_add_sdm( _mm_mul_sd( Q, x ), &Em1_A[3] );
		Q = _mm_add_sdm( _mm_mul_sd( Q, x ), &Em1_A[2] );
		Q = _mm_add_sdm( _mm_mul_sd( Q, x ), &Em1_A[1] );
		Q = _mm_add_sdm( _mm_mul_sd( Q, x ), &Em1_A[0] );
		
		Q = _mm_mul_sd( _mm_mul_sd( _mm_mul_sd( Q, x ), x), x );            //Q = ( x * ( x * ( x * Q ) ) );            

		//if ( Y >= twoTOn7.d ) 
		//	x = ( U + Y ) + ( Q + ( V + Z ) );  /* an exact operation   */
		//else 
		//	x += ( Y + ( Q + Z ) );
	
		//zero V and U if Y >= twoTOn7, otherwise zero x
		xDouble test = _mm_cmpge_sdm( Y, &twoTOn7 );
		U = _mm_and_pd( U, test );
		V = _mm_and_pd( V, test );
		x = _mm_andnot_pd( test, x );

		V = _mm_add_sd( V, Z );
		U = _mm_add_sd( U, Y );
		Q = _mm_add_sd( Q, V );
		U = _mm_add_sd( U, Q );
		x = _mm_add_sd( x, U );
	
		// Merge new exceptions into old environment
		return XDOUBLE_2_DOUBLE( x );
	}
	
}

/*
double expm1( double x )
{
    return _xexpm1( x );
}
*/

/*
float    expm1f( float x )
{//cheesy fallback on double, that probably fails to get the edge cases right.
    return (float) _xexpm1( x );
}
*/

#pragma mark -
#pragma mark exp2

/*
float exp2f( float x )
{
    return (float) _xexp2( (double) x );
}
*/

#else /*carbon core legacy */

/*
double exp2( double x )
{
    return _xexp2( x );
}
 */


#endif  /*CARBONCORE LEGACY */

