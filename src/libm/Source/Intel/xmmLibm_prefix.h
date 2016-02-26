

//
//  Basic types used internally in xmmLibm
//

#ifndef __XMMLIBM_PREFIX_H__
#define	__XMMLIBM_PREFIX_H__

#include <stdint.h>
#include <emmintrin.h>


//The types we use
typedef float  xFloat __attribute__ ((vector_size (16)));
typedef double xDouble __attribute__ ((vector_size (16)));
typedef int32_t xSInt32 __attribute__ ((vector_size (16)));
typedef uint32_t xUInt32 __attribute__ ((vector_size (16)));
typedef int64_t xSInt64 __attribute__ ((vector_size (16)));
typedef uint64_t xUInt64 __attribute__ ((vector_size (16)));
typedef double DoubleConstant_sd;
typedef xDouble DoubleConstant_pd;

typedef union 
{
  uint64_t  u;
  double    d;
}hexdouble;

typedef union 
{
  uint32_t  u;
  float     f;
}hexfloat;

#if defined( __i386__ ) || defined( __x86_64__ )
    #include <xmmintrin.h>
    #include <emmintrin.h>

    #define DEFAULT_MXCSR   0x1F80
    #define INVALID_FLAG    0x0001
    #define DENORMAL_FLAG   0x0002
    #define DIVIDE_0_FLAG   0x0004
    #define OVERFLOW_FLAG   0x0008
    #define UNDERFLOW_FLAG  0x0010
    #define INEXACT_FLAG    0x0020
    #define ALL_FLAGS       0x3F
    #define DEFAULT_MASK    ( DEFAULT_MXCSR | ALL_FLAGS )
    #define INVALID_MASK    0x0080
    #define DENORMAL_MASK   0x0100
    #define DIVIDE_0_MASK   0x0200
    #define OVERFLOW_MASK   0x0400
    #define UNDERFLOW_MASK  0x0800
    #define INEXACT_MASK    0x1000
    #define ALL_MASKS       0x1F80
	
	#define ROUND_TO_NEAREST		0x0000
	#define ROUND_TO_ZERO			0x6000
	#define ROUND_TO_INFINITY		0x4000
	#define ROUND_TO_NEG_INFINITY	0x2000
	#define ROUND_MASK				ROUND_TO_ZERO

    static const xDouble minusZeroD = {-0.0, -0.0};
    static const xFloat minusZeroF = {-0.0f, -0.0f, -0.0f, -0.0f };

    //Macros to handle conversions between scalar types to vector types
#if __APPLE_CC__-0 < 5342
    #define DOUBLE_2_XDOUBLE( _d )       _mm_load_sd( &(_d) )
    #define FLOAT_2_XFLOAT( _f )         _mm_load_ss( &(_f) )
    #define XDOUBLE_2_DOUBLE( _xd )      ({ double _d; _mm_store_sd( &_d, _xd ); /*return*/ _d; })
    #define XFLOAT_2_FLOAT( _xf )        ({ float  _f; _mm_store_ss( &_f, _xf ); /*return*/ _f; })
    #define FLOAT_2_XDOUBLE( _f )        ({ xFloat _xf = _mm_load_ss( &(_f) ); xDouble _xd = _mm_cvtss_sd( (xDouble) _xf, _xf ); /*return*/ _xd; })
    #define XDOUBLE_2_FLOAT( _xd )       ({ float _f; xDouble _d = _xd; xFloat _xf = _mm_cvtsd_ss( (xFloat) _d, _d ); _mm_store_ss( &_f, _xf ); /* return*/ _f; })
#else
    #define DOUBLE_2_XDOUBLE( _d )       _mm_set_sd( _d )
    #define FLOAT_2_XFLOAT( _f )         _mm_set_ss( _f )
    #define XDOUBLE_2_DOUBLE( _xd )      _mm_cvtsd_f64( _xd )
    #define XFLOAT_2_FLOAT( _xf )        _mm_cvtss_f32( _xf )
    #define FLOAT_2_XDOUBLE( _f )        ({ xFloat _xf = FLOAT_2_XFLOAT(_f); xDouble _xd = _mm_cvtss_sd( (xDouble) _xf, _xf ); /*return*/ _xd; })
    #define XDOUBLE_2_FLOAT( _xd )       ({ float _f; xDouble _d = _xd; xFloat _xf = _mm_cvtsd_ss( (xFloat) _d, _d ); _f = XFLOAT_2_FLOAT( _xf); /* return*/ _f; })
#endif

    //Macros to do constant expansion
    #define GET_CONSTANT_sd( _p )       _p
    #define GET_CONSTANT_pd( _p )       { _p, _p }

    //Macros to extend the set of intrinsics to allow for memory operands
    #define _mm_cmplt_sdm( _xd, _m64 )   ({ register xDouble _r = _xd; asm( "cmpltsd %1, %0" : "+x" (_r) : "m" (*(_m64)) ); /*return*/ _r; })
    #define _mm_cmple_sdm( _xd, _m64 )   ({ register xDouble _r = _xd; asm( "cmplesd %1, %0" : "+x" (_r) : "m" (*(_m64)) ); /*return*/ _r; })
    #define _mm_cmpgt_sdm( _xd, _m64 )   ({ register xDouble _r; asm( "movsd %1, %0" : "=x" (_r) : "m" (*(_m64)) ); _r = (xDouble) _mm_cmplt_sd( _r, _xd ); /*return*/ _r; })
    #define _mm_cmpge_sdm( _xd, _m64 )   ({ register xDouble _r; asm( "movsd %1, %0" : "=x" (_r) : "m" (*(_m64)) ); _r = (xDouble) _mm_cmple_sd( _r, _xd ); /*return*/ _r; })
    #define _mm_cmpeq_sdm( _xd, _m64 )   ({ register xDouble _r = _xd; asm( "cmpeqsd %1, %0" : "+x" (_r) : "m" (*(_m64)) ); /*return*/ _r; })
    #define _mm_cmpne_sdm( _xd, _m64 )   ({ register xDouble _r = _xd; asm( "cmpneqsd %1, %0" : "+x" (_r) : "m" (*(_m64)) ); /*return*/ _r; })
    #define _mm_add_sdm( _xd, _m64 )     ({ register xDouble _r = _xd; asm( "addsd %1, %0" : "+x" (_r) : "m" (*(_m64)) ); /*return*/ _r; })
    #define _mm_sub_sdm( _xd, _m64 )     ({ register xDouble _r = _xd; asm( "subsd %1, %0" : "+x" (_r) : "m" (*(_m64)) ); /*return*/ _r; })
    #define _mm_mul_sdm( _xd, _m64 )     ({ register xDouble _r = _xd; asm( "mulsd %1, %0" : "+x" (_r) : "m" (*(_m64)) ); /*return*/ _r; })
    #define _mm_max_sdm( _xd, _m64 )     ({ register xDouble _r = _xd; asm( "maxsd %1, %0" : "+x" (_r) : "m" (*(_m64)) ); /*return*/ _r; })
    #define _mm_min_sdm( _xd, _m64 )     ({ register xDouble _r = _xd; asm( "minsd %1, %0" : "+x" (_r) : "m" (*(_m64)) ); /*return*/ _r; })
    #define _mm_div_sdm( _xd, _m64 )     ({ register xDouble _r = _xd; asm( "divsd %1, %0" : "+x" (_r) : "m" (*(_m64)) ); /*return*/ _r; })
    #define _mm_sel_pd( _xd1, _xd2, _mask)  ({ xDouble _m = _mask; _m = _mm_or_pd( _mm_and_pd(_xd2, _m ), _mm_andnot_pd( _m, _xd1 ) ); /* return */ _m; })

    #define _mm_ucomieq_sdm( _xd, _m64 ) ({ bool _r; asm("ucomisd %1, %2\n\tsete %b0" : "=q" (_r) : "m" (*(_m64)), "X" (_xd)); /*return */ _r; })
    #define _mm_ucomieq_ssm( _xf, _m32 ) ({ bool _r; asm("ucomiss %1, %2\n\tsete %b0" : "=q" (_r) : "m" (*(_m32)), "X" (_xf)); /*return */ _r; })

    #define _mm_cmplt_ssm( _xf, _m32 )   ({ register xFloat _r = _xf; asm( "cmpltss %1, %0" : "+x" (_r) : "m" (*(_m32)) ); /*return*/ _r; })
    #define _mm_cmple_ssm( _xf, _m32 )   ({ register xFloat _r = _xf; asm( "cmpless %1, %0" : "+x" (_r) : "m" (*(_m32)) ); /*return*/ _r; })
    #define _mm_cmpgt_ssm( _xf, _m32 )   ({ register xFloat _r; asm( "movss %1, %0" : "=x" (_r) : "m" (*(_m32)) ); _r = (xFloat) __builtin_ia32_cmpltss( _r, _xf ); /*return*/ _r; })
    #define _mm_cmpge_ssm( _xf, _m32 )   ({ register xFloat _r; asm( "movss %1, %0" : "=x" (_r) : "m" (*(_m32)) ); _r = (xFloat) __builtin_ia32_cmpless( _r, _xf ); /*return*/ _r; })
    #define _mm_cmpeq_ssm( _xf, _m32 )   ({ register xFloat _r = _xf; asm( "cmpeqss %1, %0" : "+x" (_r) : "m" (*(_m32)) ); /*return*/ _r; })
    #define _mm_cmpne_ssm( _xf, _m32 )   ({ register xFloat _r = _xf; asm( "cmpneqss %1, %0" : "+x" (_r) : "m" (*(_m32)) ); /*return*/ _r; })
    #define _mm_add_ssm( _xf, _m32 )     ({ register xFloat _r = _xf; asm( "addss %1, %0" : "+x" (_r) : "m" (*(_m32)) ); /*return*/ _r; })
    #define _mm_sub_ssm( _xf, _m32 )     ({ register xFloat _r = _xf; asm( "subss %1, %0" : "+x" (_r) : "m" (*(_m32)) ); /*return*/ _r; })
    #define _mm_mul_ssm( _xf, _m32 )     ({ register xFloat _r = _xf; asm( "mulss %1, %0" : "+x" (_r) : "m" (*(_m32)) ); /*return*/ _r; })
    #define _mm_max_ssm( _xf, _m32 )     ({ register xFloat _r = _xf; asm( "maxss %1, %0" : "+x" (_r) : "m" (*(_m32)) ); /*return*/ _r; })
    #define _mm_min_ssm( _xf, _m32 )     ({ register xFloat _r = _xf; asm( "minss %1, %0" : "+x" (_r) : "m" (*(_m32)) ); /*return*/ _r; })
    #define _mm_div_ssm( _xf, _m32 )     ({ register xFloat _r = _xf; asm( "divss %1, %0" : "+x" (_r) : "m" (*(_m32)) ); /*return*/ _r; })
    #define _mm_sel_ps( _xf1, _xf2, _mask)  ({ xFloat _m = _mask; _m = _mm_or_ps( _mm_and_ps(_xf2, _m ), _mm_andnot_ps( _m, _xf1 ) ); /* return */ _m; })


    #define _mm_istrue_sd( _test )       (1 == ( _mm_movemask_pd( _test ) & 1 ))
    #define _mm_isfalse_sd( _test )       (0 == ( _mm_movemask_pd( _test ) & 1 ))

    #define _mm_istrue_ss( _test )       (1 == ( _mm_movemask_ps( _test ) & 1 ))
    #define _mm_isfalse_ss( _test )       (0 == ( _mm_movemask_ps( _test ) & 1 ))

    //fast scalb. Doesn't check for overflow
    #define twoToTheM( _i )         ({ xDouble _xi = (xDouble) _mm_slli_epi64( _mm_cvtsi32_si128( (_i) + 1023 ), 52 );  /*return*/ _xi; })

    //work around for broken _mm_sqrt_sd intrinsic in gcc-4.0
    #define _MM_SQRT_SD( x )              __builtin_ia32_sqrtsd( x )

    #define _mm_cvtsi64_si128( x )      ({ int64_t _x = x; xSInt64 _r; asm volatile( "movq %1, %0" : "=x" (_r) : "m" (*&_x)); _r })

	#define REQUIRED_ADD_sd( _x, _y )	({ xDouble __x = _x; xDouble __y = _y; asm volatile( "addsd %1, %0" : "+x" (__x ) : "x" (__y)); /*return*/ __x; }) 
	#define REQUIRED_ADD_sdm( _x, _m64 )	({ xDouble __x = _x; asm volatile( "addsd %1, %0" : "+x" (__x ) : "m" (*(_m64))); /*return*/ __x; }) 
	#define REQUIRED_MULTIPLY_sd( _x, _y )	({ xDouble __x = _x; xDouble __y = _y; asm volatile( "mulsd %1, %0" : "+x" (__x ) : "x" (__y)); /*return*/ __x; }) 
	#define REQUIRED_MULTIPLY_sdm( _x, _m64 )	({ xDouble __x = _x; xDouble __y = _y; asm volatile( "mulsd %1, %0" : "+x" (__x ) : "m" (*(_m64))); /*return*/ __x; }) 
	#define REQUIRED_DIVIDE_sd( _x, _y )	({ xDouble __x = _x; xDouble __y = _y; asm volatile( "divsd %1, %0" : "+x" (__x ) : "x" (__y)); /*return*/ __x; }) 
	#define REQUIRED_DIVIDE_sdm( _x, _m64 )	({ xDouble __x = _x; asm volatile( "divsd %1, %0" : "+x" (__x ) : "m" (*(_m64))); /*return*/ __x; }) 

	#define REQUIRED_ADD_ss( _x, _y )	({ xFloat __x = _x; xFloat __y = _y; asm volatile( "addss %1, %0" : "+x" (__x ) : "x" (__y)); /*return*/ __x; }) 
	#define REQUIRED_ADD_ssm( _x, _m32 )	({ xFloat __x = _x; asm volatile( "addss %1, %0" : "+x" (__x ) : "m" (*(_m32))); /*return*/ __x; }) 
	#define REQUIRED_MULTIPLY_ss( _x, _y )	({ xFloat __x = _x; xFloat __y = _y; asm volatile( "mulss %1, %0" : "+x" (__x ) : "x" (__y)); /*return*/ __x; }) 
	#define REQUIRED_MULTIPLY_ssm( _x, _m32 )	({ xFloat __x = _x; xFloat __y = _y; asm volatile( "mulss %1, %0" : "+x" (__x ) : "m" (*(_m32))); /*return*/ __x; }) 
	#define REQUIRED_DIVIDE_ss( _x, _y )	({ xFloat __x = _x; xFloat __y = _y; asm volatile( "divss %1, %0" : "+x" (__x ) : "x" (__y)); /*return*/ __x; }) 
	#define REQUIRED_DIVIDE_ssm( _x, _m32 )	({ xFloat __x = _x; asm volatile( "divss %1, %0" : "+x" (__x ) : "m" (*(_m32))); /*return*/ __x; }) 


	#define SET_INVALID_FLAG()          { __m128d __x = _mm_setzero_pd(); __asm__ __volatile__( "pcmpeqd %0, %0 \n\t cmpltsd %0, %0" : "+x" (__x) ); }

#if defined( __SSE3__ )
    #define CVTTLD_SI64( _ld )          ({ long double _x = _ld; int64_t _r; asm( "fldt %1 \n\t fisttpll %0" : "=m" (*&_r): "m" (*&_x) ); /*return*/ _r; })
#else
    #warning SSE3 disabled
    static inline int64_t CVTTLD_SI64( long double ld ){ return (int64_t) ld; }
#endif
    #define CVTLD_SI64( _ld )          ({ long double _x = _ld; int64_t _r; asm( "fldt %1 \n\t fistpll %0" : "=m" (*&_r): "m" (*&_x) ); /*return*/ _r; })

    #define EXPECT_TRUE( _a )           __builtin_expect( (_a), 1 ) 
    #define EXPECT_FALSE( _a )          __builtin_expect( (_a), 0 ) 


#endif

#if defined( XMMLIBM_DEBUG )
	#warning always inline disabled
    #define ALWAYS_INLINE   
#else
    #define ALWAYS_INLINE   __attribute__ ((always_inline))
#endif

#endif /* __XMMLIBM_PREFIX_H__ */
