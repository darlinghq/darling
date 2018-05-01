/*
 *  expf_logf_powf.c
 *  Libm
 *
 *  Written by Ian Ollmann
 *  Polynomials by Ali Sazegari.
 *
 *  Copyright 2007 Apple Inc. All rights reserved.
 *
 */

#include "math.h"
#include <emmintrin.h>
#include <stdint.h>
#include "xmmLibm_prefix.h"
#include <float.h>

#if defined( __SSE3__ )
    #include <pmmintrin.h>
#endif

                                                         
//11th order minimax polynomial approximation of y = log2(x)
//Valid over the range [1, 2]. Error is 0.160490514e-9.
//Packaged as vector doubles, in order from c0 to c11
static const __m128d log2polynomialCoefficients[6] = {   
                                                        { -3.86011461177189878, 11.2166692423937703 },
                                                        {-19.7108996153552616,  27.5580934192221441 },
                                                        {-28.7390040495347408,  22.2562416071123559 },
                                                        {-12.7549704819157692,   5.34170406305677394 },
                                                        {-1.59009195684350404,  0.318880468456163145 },
                                                        {-0.0386477049377158355, 0.00213962027817084459 }
                                                    };


//Second order minimax polynomial approximation of  y = 2**x
//Valid over the range [0.0,0.4e-2]. (We need [0, 2**-8] )
//  error is +- 0.11116223e-9,  We need less than 2**-28 here.
static const double exp2polynomialCoefficients[3] = {   1.00000000011116223,
                                                        0.693146680445468386,
                                                        0.240559810709772152    };


typedef uint32_t           vUInt32         __attribute__ ((__vector_size__ (16)));

static const vUInt32 v1023 = { 1023, 0, 0, 0 };
static const __m128d exponentMask = { __builtin_inf(), __builtin_inf() };
static const __m128d half_sd = { 0.5, 0.0 };
static const __m128d one_sd = { 1.0, 0.0 };
static const __m128d two_pd = { 2.0, 2.0 };

/*
float log2f( float x )
{
//Edge cases for C99 TC2:

    //Handle NaN
    if( x != x )
        return x;

// log2(+-0)returns -Inf and raises the divide-by-zero floating-point exception. 
// log2(x)returns a NaN and raises the  invalid floating-point exception for x < 0. 
    if( x <= 0.0f )
    {
        if( x == 0.0f )
            return XFLOAT_2_FLOAT( REQUIRED_DIVIDE_ss( FLOAT_2_XFLOAT(-1.0f), _mm_setzero_ps() ));
        else
            return XFLOAT_2_FLOAT( REQUIRED_MULTIPLY_ss( FLOAT_2_XFLOAT(__builtin_inff()), _mm_setzero_ps() ));
    }
    
// log2(+Inf)returns +Inf. 
    if( x == __builtin_inff() )
        return x;

// We are going to try to get the last edge case to just happen in arithmetic
// log2(1)returns +0. 


//Main body

    //Convert to double  
    __m128d X = FLOAT_2_XDOUBLE(x);      //{ 0, x }

    //extract exponent -- denormals can't happen here
    __m128i iexponent = _mm_sub_epi32( _mm_srli_epi64( (__m128i) _mm_and_pd( X, exponentMask), 52 ), v1023 );
    __m128d exponent = _mm_cvtepi32_pd( iexponent );

    //get mantissa of X as value in the range [ 1.0, 2.0 )
    __m128d mantissa = _mm_or_pd( _mm_andnot_pd( exponentMask, X ), one_sd );

    //avoid setting inexact flag for exact powers of two
    if( 1.0 == XDOUBLE_2_DOUBLE( mantissa ) )
        return XDOUBLE_2_FLOAT( exponent );

    __m128d m1, m2, m4, v1, v;

#if 1
    if( 0.98f < x && x < 1.02f )
    {
        float temp;

        //use fyl2x to get y*log2(x)
        asm volatile( "fld1 \n flds (%1)\n fyl2x \n fstps %0" : "=m" (*(&temp)) : "r" (&x) );
            
        return temp;
    }
#else
    //inputs between 0.98 and 1.02 need special care
    if( 0.98f < x && x < 1.02f )
    {
        __m128d XX = _mm_unpacklo_pd( X, X );
        m1 = _mm_unpacklo_pd( one_sd, X );                          // { m, 1 }
        m2 = _mm_mul_pd( XX, XX );                                   //{ m**2, m**2 }
        m4 = _mm_mul_pd( m2, m2 );                                  //{ m**4, m**4 }

        v1 = _mm_mul_pd( m4, log2near1polynomialCoefficients[5] );               // { c11 m**4, c10 m**4 } 
        v  = _mm_mul_pd( m4, log2near1polynomialCoefficients[4] );               // { c9 m**4, c8 m**4 } 
        m2 = _mm_mul_pd( m2, m1 );
        v1 = _mm_add_pd( v1, log2near1polynomialCoefficients[3] );                       // { c7 + c11 m**4, c6 + c10 m**4 }
        v  = _mm_add_pd( v , log2near1polynomialCoefficients[2] );                       // { c5 + c9 m**4, c4 + c8 m**4 }
        v1 = _mm_mul_pd( v1, m4 );                                                  // { c7 m**4 + c11 m**8, c6 m**4 + c10 m**8 }
        v  = _mm_mul_pd( v , m4 );                                                  // { c5 m**4 + c9 m**8, c4 m**4 + c8 m**8 }
        v1 = _mm_add_pd( v1, log2near1polynomialCoefficients[1] );                       // { c3 + c7 m**4 + c11 m**8, c2 + c6 m**4 + c10 m**8 }
        v  = _mm_add_pd( v , log2near1polynomialCoefficients[0] );                       // { c1 + c5 m**4 + c9 m**8, c0 + c4 m**4 + c8 m**8 }
        v1 = _mm_mul_pd( v1, m2 );                                                  // { c3 m**3 + c7 m**7 + c11 m**11, c2 m**2 + c6 m**4 + c10 m**8 }
        v  = _mm_mul_pd( v , m1 );                                                  // { c1 m + c5 m**5 + c9 m**9, c0 + c4 m**4 + c8 m**8 }
        v  = _mm_add_pd( v , v1 );

        //Add the two sub terms horizontally
    #if defined( __SSE3__ )
        v = _mm_hadd_pd(  v, _mm_setzero_pd() );
    #else
        v = _mm_add_sd( v, _mm_unpackhi_pd( v, v ) );
    #endif
    
        return XDOUBLE_2_FLOAT( v );
    }
#endif

    //First make a vector out of the mantissa
    __m128d vMantissa = _mm_unpacklo_pd( mantissa, mantissa );                  //{ mantissa, mantissa }
    m1 = _mm_unpacklo_pd( one_sd, mantissa );                           // { m, 1 }
    m2 = _mm_mul_pd( vMantissa, vMantissa );                            //{ m**2, m**2 }
    m4 = _mm_mul_pd( m2, m2 );                                          //{ m**4, m**4 }
    v1 = _mm_mul_pd( m4, log2polynomialCoefficients[5] );               // { c11 m**4, c10 m**4 } 
    v  = _mm_mul_pd( m4, log2polynomialCoefficients[4] );               // { c9 m**4, c8 m**4 } 
    m2 = _mm_mul_pd( m2, m1 );
    v1 = _mm_add_pd( v1, log2polynomialCoefficients[3] );                       // { c7 + c11 m**4, c6 + c10 m**4 }
    v  = _mm_add_pd( v , log2polynomialCoefficients[2] );                       // { c5 + c9 m**4, c4 + c8 m**4 }
    v1 = _mm_mul_pd( v1, m4 );                                                  // { c7 m**4 + c11 m**8, c6 m**4 + c10 m**8 }
    v  = _mm_mul_pd( v , m4 );                                                  // { c5 m**4 + c9 m**8, c4 m**4 + c8 m**8 }
    v1 = _mm_add_pd( v1, log2polynomialCoefficients[1] );                       // { c3 + c7 m**4 + c11 m**8, c2 + c6 m**4 + c10 m**8 }
    v  = _mm_add_pd( v , log2polynomialCoefficients[0] );                       // { c1 + c5 m**4 + c9 m**8, c0 + c4 m**4 + c8 m**8 }
    v1 = _mm_mul_pd( v1, m2 );                                                  // { c3 m**3 + c7 m**7 + c11 m**11, c2 m**2 + c6 m**4 + c10 m**8 }
    v  = _mm_mul_pd( v , m1 );                                                  // { c1 m + c5 m**5 + c9 m**9, c0 + c4 m**4 + c8 m**8 }
    v  = _mm_add_pd( v , v1 );

    //Add the two sub terms horizontally
#if defined( __SSE3__ )
    v = _mm_hadd_pd(  v, _mm_setzero_pd() );
#else
    v = _mm_add_sd( v, _mm_unpackhi_pd( v, v ) );
#endif


    //add in the exponent
    v = _mm_add_sd( v, exponent );
    
    return XDOUBLE_2_FLOAT( v );
}
*/

static const __m128i mantissaHiBits = { 0x000FF00000000000ULL, 0ULL };

/* 
    exp2_table produced as follows:
    
    #include <emmintrin.h>
    #include <stdio.h>
    #include <stdint.h>
    #include <math.h>


    int main( void )
    {
            int i;

            for( i = 0; i < 256; i++ )
            {
                    union
                    {
                            uint64_t        u;
                            double          d;
                    }u = { 0x3ff0000000000000ULL | ((uint64_t) i << 44 ) };

                    if( 0 == i % 4 )
                            printf( "\n" );

                    printf( " {  %17.21f, 0x0.%2.2xp0 },", exp2( u.d - 1.0 ), i  );

            }
            printf( "\n" );

            return 0;
    }
*/

static const __m128d exp2_table[256] = { 
                                            { 1.000000000000000000000, 0x0.00p0 }, { 1.002711275050202521797, 0x0.01p0 }, { 1.005429901112802726360, 0x0.02p0 }, { 1.008155898118417548304, 0x0.03p0 },
                                            { 1.010889286051700475255, 0x0.04p0 }, { 1.013630084951489429557, 0x0.05p0 }, { 1.016378314910953095662, 0x0.06p0 }, { 1.019133996077737913666, 0x0.07p0 },
                                            { 1.021897148654116627142, 0x0.08p0 }, { 1.024667792897135720764, 0x0.09p0 }, { 1.027445949118763746100, 0x0.0ap0 }, { 1.030231637686040979673, 0x0.0bp0 },
                                            { 1.033024879021228414899, 0x0.0cp0 }, { 1.035825693601957198098, 0x0.0dp0 }, { 1.038634101961378730650, 0x0.0ep0 }, { 1.041450124688316103416, 0x0.0fp0 },
                                            { 1.044273782427413754803, 0x0.10p0 }, { 1.047105095879289793359, 0x0.11p0 }, { 1.049944085800687210153, 0x0.12p0 }, { 1.052790773004626423415, 0x0.13p0 },
                                            { 1.055645178360557157049, 0x0.14p0 }, { 1.058507322794512761632, 0x0.15p0 }, { 1.061377227289262092924, 0x0.16p0 }, { 1.064254912884464499001, 0x0.17p0 },
                                            { 1.067140400676823697168, 0x0.18p0 }, { 1.070033711820241872914, 0x0.19p0 }, { 1.072934867525975555225, 0x0.1ap0 }, { 1.075843889062791047806, 0x0.1bp0 },
                                            { 1.078760797757119860307, 0x0.1cp0 }, { 1.081685614993215249768, 0x0.1dp0 }, { 1.084618362213309206155, 0x0.1ep0 }, { 1.087559060917769659937, 0x0.1fp0 },
                                            { 1.090507732665257689675, 0x0.20p0 }, { 1.093464399072885839814, 0x0.21p0 }, { 1.096429081816376882585, 0x0.22p0 }, { 1.099401802630221913759, 0x0.23p0 },
                                            { 1.102382583307840890896, 0x0.24p0 }, { 1.105371445701741173195, 0x0.25p0 }, { 1.108368411723678725878, 0x0.26p0 }, { 1.111373503344817548211, 0x0.27p0 },
                                            { 1.114386742595892432206, 0x0.28p0 }, { 1.117408151567369278823, 0x0.29p0 }, { 1.120437752409606746440, 0x0.2ap0 }, { 1.123475567333019897731, 0x0.2bp0 },
                                            { 1.126521618608241848136, 0x0.2cp0 }, { 1.129575928566288078869, 0x0.2dp0 }, { 1.132638519598719195614, 0x0.2ep0 }, { 1.135709414157805463574, 0x0.2fp0 },
                                            { 1.138788634756691564576, 0x0.30p0 }, { 1.141876203969561576201, 0x0.31p0 }, { 1.144972144431804172982, 0x0.32p0 }, { 1.148076478840178937801, 0x0.33p0 },
                                            { 1.151189229952982673311, 0x0.34p0 }, { 1.154310420590215935377, 0x0.35p0 }, { 1.157440073633751120852, 0x0.36p0 }, { 1.160578212027498778980, 0x0.37p0 },
                                            { 1.163724858777577475522, 0x0.38p0 }, { 1.166880036952481658474, 0x0.39p0 }, { 1.170043769683250189928, 0x0.3ap0 }, { 1.173216080163637320410, 0x0.3bp0 },
                                            { 1.176396991650281220743, 0x0.3cp0 }, { 1.179586527462875844563, 0x0.3dp0 }, { 1.182784710984341014495, 0x0.3ep0 }, { 1.185991565660993840581, 0x0.3fp0 },
                                            { 1.189207115002721026897, 0x0.40p0 }, { 1.192431382583151178167, 0x0.41p0 }, { 1.195664392039827328418, 0x0.42p0 }, { 1.198906167074380357818, 0x0.43p0 },
                                            { 1.202156731452703075647, 0x0.44p0 }, { 1.205416109005123859177, 0x0.45p0 }, { 1.208684323626581624822, 0x0.46p0 }, { 1.211961399276801243374, 0x0.47p0 },
                                            { 1.215247359980468955243, 0x0.48p0 }, { 1.218542229827408451825, 0x0.49p0 }, { 1.221846032972757400969, 0x0.4ap0 }, { 1.225158793637145526745, 0x0.4bp0 },
                                            { 1.228480536106870024682, 0x0.4cp0 }, { 1.231811284734075861991, 0x0.4dp0 }, { 1.235151063936933191201, 0x0.4ep0 }, { 1.238499898199816540156, 0x0.4fp0 },
                                            { 1.241857812073484002013, 0x0.50p0 }, { 1.245224830175257979548, 0x0.51p0 }, { 1.248600977189204819240, 0x0.52p0 }, { 1.251986277866316221719, 0x0.53p0 },
                                            { 1.255380757024691096291, 0x0.54p0 }, { 1.258784439549716527296, 0x0.55p0 }, { 1.262197350394250738859, 0x0.56p0 }, { 1.265619514578806281691, 0x0.57p0 },
                                            { 1.269050957191733219886, 0x0.58p0 }, { 1.272491703389402761815, 0x0.59p0 }, { 1.275941778396392001227, 0x0.5ap0 }, { 1.279401207505669324505, 0x0.5bp0 },
                                            { 1.282870016078778263591, 0x0.5cp0 }, { 1.286348229546025567771, 0x0.5dp0 }, { 1.289835873406665944785, 0x0.5ep0 }, { 1.293332973229089466471, 0x0.5fp0 },
                                            { 1.296839554651009640551, 0x0.60p0 }, { 1.300355643379650594227, 0x0.61p0 }, { 1.303881265191935812098, 0x0.62p0 }, { 1.307416445934677318164, 0x0.63p0 },
                                            { 1.310961211524764413738, 0x0.64p0 }, { 1.314515587949354635811, 0x0.65p0 }, { 1.318079601266064049270, 0x0.66p0 }, { 1.321653277603157539133, 0x0.67p0 },
                                            { 1.325236643159741323217, 0x0.68p0 }, { 1.328829724205954354588, 0x0.69p0 }, { 1.332432547083161500368, 0x0.6ap0 }, { 1.336045138204145832361, 0x0.6bp0 },
                                            { 1.339667524053302916087, 0x0.6cp0 }, { 1.343299731186835321850, 0x0.6dp0 }, { 1.346941786232945803548, 0x0.6ep0 }, { 1.350593715892034252235, 0x0.6fp0 },
                                            { 1.354255546936892651289, 0x0.70p0 }, { 1.357927306212901141791, 0x0.71p0 }, { 1.361609020638224754052, 0x0.72p0 }, { 1.365300717204011915484, 0x0.73p0 },
                                            { 1.369002422974590738036, 0x0.74p0 }, { 1.372714165087668414245, 0x0.75p0 }, { 1.376435970754530169202, 0x0.76p0 }, { 1.380167867260237990479, 0x0.77p0 },
                                            { 1.383909881963832022578, 0x0.78p0 }, { 1.387662042298529074813, 0x0.79p0 }, { 1.391424375771926236212, 0x0.7ap0 }, { 1.395196909966200271569, 0x0.7bp0 },
                                            { 1.398979672538311236352, 0x0.7cp0 }, { 1.402772691220204759333, 0x0.7dp0 }, { 1.406575993819015435449, 0x0.7ep0 }, { 1.410389608217270662749, 0x0.7fp0 },
                                            { 1.414213562373094923430, 0x0.80p0 }, { 1.418047884320415175097, 0x0.81p0 }, { 1.421892602169165575887, 0x0.82p0 }, { 1.425747744105494430045, 0x0.83p0 },
                                            { 1.429613338391970023267, 0x0.84p0 }, { 1.433489413367788900544, 0x0.85p0 }, { 1.437375997448982367644, 0x0.86p0 }, { 1.441273119128625657126, 0x0.87p0 },
                                            { 1.445180806977046650275, 0x0.88p0 }, { 1.449099089642035043113, 0x0.89p0 }, { 1.453027995849052622646, 0x0.8ap0 }, { 1.456967554401443765144, 0x0.8bp0 },
                                            { 1.460917794180647044655, 0x0.8cp0 }, { 1.464878744146405731286, 0x0.8dp0 }, { 1.468850433336981842203, 0x0.8ep0 }, { 1.472832890869367528097, 0x0.8fp0 },
                                            { 1.476826145939499346227, 0x0.90p0 }, { 1.480830227822471867327, 0x0.91p0 }, { 1.484845165872752614789, 0x0.92p0 }, { 1.488870989524397003834, 0x0.93p0 },
                                            { 1.492907728291264835008, 0x0.94p0 }, { 1.496955411767235455400, 0x0.95p0 }, { 1.501014069626425584403, 0x0.96p0 }, { 1.505083731623406473332, 0x0.97p0 },
                                            { 1.509164427593422841412, 0x0.98p0 }, { 1.513256187452609813349, 0x0.99p0 }, { 1.517359041198214741897, 0x0.9ap0 }, { 1.521473018908814589523, 0x0.9bp0 },
                                            { 1.525598150744538195056, 0x0.9cp0 }, { 1.529734466947286986027, 0x0.9dp0 }, { 1.533881997840955913048, 0x0.9ep0 }, { 1.538040773831656826687, 0x0.9fp0 },
                                            { 1.542210825407940744114, 0x0.a0p0 }, { 1.546392183141021670068, 0x0.a1p0 }, { 1.550584877684999973724, 0x0.a2p0 }, { 1.554788939777088430105, 0x0.a3p0 },
                                            { 1.559004400237836929222, 0x0.a4p0 }, { 1.563231289971357629298, 0x0.a5p0 }, { 1.567469639965552774541, 0x0.a6p0 }, { 1.571719481292341402678, 0x0.a7p0 },
                                            { 1.575980845107886496592, 0x0.a8p0 }, { 1.580253762652824578439, 0x0.a9p0 }, { 1.584538265252493749458, 0x0.aap0 }, { 1.588834384317163950229, 0x0.abp0 },
                                            { 1.593142151342266776837, 0x0.acp0 }, { 1.597461597908627073394, 0x0.adp0 }, { 1.601792755682693414343, 0x0.aep0 }, { 1.606135656416771029242, 0x0.afp0 },
                                            { 1.610490331949254283472, 0x0.b0p0 }, { 1.614856814204860491202, 0x0.b1p0 }, { 1.619235135194863728358, 0x0.b2p0 }, { 1.623625327017328867640, 0x0.b3p0 },
                                            { 1.628027421857347833978, 0x0.b4p0 }, { 1.632441451987274971813, 0x0.b5p0 }, { 1.636867449766964410784, 0x0.b6p0 }, { 1.641305447644006321184, 0x0.b7p0 },
                                            { 1.645755478153964945776, 0x0.b8p0 }, { 1.650217573920617741834, 0x0.b9p0 }, { 1.654691767656194523184, 0x0.bap0 }, { 1.659178092161616158151, 0x0.bbp0 },
                                            { 1.663676580326736598181, 0x0.bcp0 }, { 1.668187265130582463968, 0x0.bdp0 }, { 1.672710179641596406341, 0x0.bep0 }, { 1.677245357017878468753, 0x0.bfp0 },
                                            { 1.681792830507429004072, 0x0.c0p0 }, { 1.686352633448393145699, 0x0.c1p0 }, { 1.690924799269305056626, 0x0.c2p0 }, { 1.695509361489332622597, 0x0.c3p0 },
                                            { 1.700106353718523477525, 0x0.c4p0 }, { 1.704715809658051250963, 0x0.c5p0 }, { 1.709337763100462925792, 0x0.c6p0 }, { 1.713972247929925973864, 0x0.c7p0 },
                                            { 1.718619298122477934143, 0x0.c8p0 }, { 1.723278947746273992436, 0x0.c9p0 }, { 1.727951230961837669753, 0x0.cap0 }, { 1.732636182022311066575, 0x0.cbp0 },
                                            { 1.737333835273706217350, 0x0.ccp0 }, { 1.742044225155156444984, 0x0.cdp0 }, { 1.746767386199168825556, 0x0.cep0 }, { 1.751503353031877985302, 0x0.cfp0 },
                                            { 1.756252160373299453511, 0x0.d0p0 }, { 1.761013843037583681550, 0x0.d1p0 }, { 1.765788435933272726430, 0x0.d2p0 }, { 1.770575974063554713922, 0x0.d3p0 },
                                            { 1.775376492526521188253, 0x0.d4p0 }, { 1.780190026515424461806, 0x0.d5p0 }, { 1.785016611318934964814, 0x0.d6p0 }, { 1.789856282321401037549, 0x0.d7p0 },
                                            { 1.794709075003107168200, 0x0.d8p0 }, { 1.799575024940535117324, 0x0.d9p0 }, { 1.804454167806623932080, 0x0.dap0 }, { 1.809346539371031736820, 0x0.dbp0 },
                                            { 1.814252175500398633901, 0x0.dcp0 }, { 1.819171112158608494269, 0x0.ddp0 }, { 1.824103385407053190548, 0x0.dep0 }, { 1.829049031404897274200, 0x0.dfp0 },
                                            { 1.834008086409342430656, 0x0.e0p0 }, { 1.838980586775893710794, 0x0.e1p0 }, { 1.843966568958625984465, 0x0.e2p0 }, { 1.848966069510450838109, 0x0.e3p0 },
                                            { 1.853979125083385470774, 0x0.e4p0 }, { 1.859005772428820479902, 0x0.e5p0 }, { 1.864046048397788979401, 0x0.e6p0 }, { 1.869099989941238604274, 0x0.e7p0 },
                                            { 1.874167634110299962558, 0x0.e8p0 }, { 1.879249018056560194267, 0x0.e9p0 }, { 1.884344179032334309909, 0x0.eap0 }, { 1.889453154390938971474, 0x0.ebp0 },
                                            { 1.894575981586965607306, 0x0.ecp0 }, { 1.899712698176555081275, 0x0.edp0 }, { 1.904863341817674138312, 0x0.eep0 }, { 1.910027950270389629495, 0x0.efp0 },
                                            { 1.915206561397147178027, 0x0.f0p0 }, { 1.920399213163047402730, 0x0.f1p0 }, { 1.925605943636124806062, 0x0.f2p0 }, { 1.930826790987627106233, 0x0.f3p0 },
                                            { 1.936061793492294347274, 0x0.f4p0 }, { 1.941310989528640451596, 0x0.f5p0 }, { 1.946574417579233218234, 0x0.f6p0 }, { 1.951852116230978317901, 0x0.f7p0 },
                                            { 1.957144124175400401455, 0x0.f8p0 }, { 1.962450480208927316994, 0x0.f9p0 }, { 1.967771223233175659217, 0x0.fap0 }, { 1.973106392255234098343, 0x0.fbp0 },
                                            { 1.978456026387950927869, 0x0.fcp0 }, { 1.983820164850219391894, 0x0.fdp0 }, { 1.989198846967266343100, 0x0.fep0 }, { 1.994592112170940234606, 0x0.ffp0 }
                                             };     
                                             
// Minimax polynomial approximation for y = 2**x, valid over the range [ 0, 2**-8 ]. Accurate to within 0.11116223e-9. We need to within 2**(-28) here.                                          
static const double exp2_polynomial[3] = { 1.00000000011116223, 0.693146680445468386, 0.240559810709772152 };
static const xFloat huge = { 0x1.0p127f, 0, 0, 0 };
static const xFloat tiny = { 0x1.00001p-127f, 0, 0, 0 };
static const vUInt32 bias = { 1023, 0, 0, 0 };
static const __m128d almost1 = { 1.0 - DBL_EPSILON, 0.0 };

/*
float exp2f( float x )
{
    //take care of NaN
    if( x != x )
        return x;

    //deal with overflow
    if( x >= 128.0f )
    {
        if( __builtin_inff() == x )
            return x;

        return XFLOAT_2_FLOAT( REQUIRED_ADD_ss( huge, huge ) );
    }

    //deal with underflow
    if( x <= -150.0f )
    {
        if( -__builtin_inff() == x )
            return 0;

        return XFLOAT_2_FLOAT( REQUIRED_MULTIPLY_ss( tiny, tiny ) );
    }



    // exp2(+-0)returns 1. 
    // Which will be taken care of through normal arithmetic

    //Split x into fractional and integer parts
    double xd = x;
    // i = floorf( x )
    double magic = x < 0 ? -0x1.0p52f : 0x1.0p52f;
    double i = (xd + magic) - magic;
    if( i == xd )
        return  XDOUBLE_2_FLOAT( (xDouble) _mm_slli_epi64( _mm_add_epi32( _mm_cvtpd_epi32( DOUBLE_2_XDOUBLE( i ) ), bias ), 52 ) );
    else if( i > xd )
        i -= 1.0f;
    double f = xd - i;

    //We do exp2(x) as:
    //
    //      x = i + f               i = floor( x )     f = x - i
    //
    //      exp2(x) = exp2( i ) * exp2( f )
    //
    //  We further break down f into:
    //
    //      f = n * (2**-8) + ff
    //
    //      exp2(x) = exp2 (i ) * exp2( n * (2**-8) ) * exp2(ff)
    //
    //          exp2( i )           calculated by simple slamming of i + 1023 into exponent of a double (with suitable saturation)
    //          exp2( n*(2**-8) )   calculated by table lookup into exp2_table
    //          exp2( ff )          calculated by minimax polynomial

    //reduce the fraction to the range [ 0, 2**-8 )
    xDouble d = DOUBLE_2_XDOUBLE(f);   
    
    //For very small negative x, we can now have d = 1.0, which causes trouble. 
    //In order to keep things sane we clamp vf to be 1.0 - DBL_EPSILON
    d = _mm_min_sd( d, almost1 );

    //extract the high 8 bits of the mantissa by adding one to d, and using integer operations to read out those bits
    int n = _mm_cvtsi128_si32( _mm_srli_epi64( _mm_and_si128( (__m128i) _mm_add_sd( d, one_sd), mantissaHiBits ), 44 ) );

    //subtract out a representation of the high 8 bits we removed from d
    d = _mm_sub_sdm( d, (double*)&exp2_table[ n ] + 1 );     //reduces d to [ 0, 2**-8 )

    
    //d = exp2(d), accurate to within 0.11116223e-9. We need to within 2**(-28) here 
    xDouble e = _mm_mul_sdm( d, exp2_polynomial + 2 );
    e = _mm_add_sdm( e, exp2_polynomial + 1 );
    e = _mm_mul_sd( e, d );
    e = _mm_add_sdm( e, exp2_polynomial );

    //Calculate exp2(i). Denormals are not possible because we use double precision here.
    __m128i ei = _mm_slli_epi64( _mm_cvtpd_epi32( DOUBLE_2_XDOUBLE( i ) ), 52 );     
    
    ei = _mm_add_epi64( ei, (__m128i) exp2_table[ n ] );    

    //scale by ei
    e = _mm_mul_sd( e, (__m128d) ei );
    
    return XDOUBLE_2_FLOAT( e );
}
*/

static const __m128d minusZero_sd = { -0.0, 0.0 };

float powf( float x, float y )
{
// The pow functions compute x raised to the power y. A domain error occurs if x is finite 
// and negative and y is finite and not an integer value. A range error may occur. A domain 
// error may occur if x is zero and y is zero. A domain error or range error may occur if x 
// is zero and y is less than zero. 

    // pow(+1, y) returns 1 for any y, even a NaN. 
    // pow(x, +-0) returns 1 for any x, even a NaN. 
    if( x == 1.0f || y == 0.0f )
        return 1.0f;

    // deal with NaNs before they cause trouble
    if( x != x || y != y )
        return x + y;

    float fabsx = __builtin_fabsf( x );
    float fabsy = __builtin_fabsf( y );
    
    //Find out if Y is an integer and if so if it is even or odd, without setting the inexact flag
    uint32_t yFracBits = 0;
    uint32_t yOneBit = 0;
    int32_t xExp, yExp;
    
    union
    {
        float       f;
        uint32_t    u;
    }uy, ux;
    
    uy.f = fabsy;
    ux.f = fabsx;

    yExp = (uy.u >> 23) - 127;
    xExp = (ux.u >> 23) - 127;
    
    if( fabsy < 1.0f )
        yFracBits = 0x007FFFFF;
    else if( fabsy < 0x1.0p24f )   // y can only be an odd integer if 1.0f <= |y| < 2**24
    {
        yOneBit = (0x00800000 >> yExp) & uy.u;
        yFracBits = (0x007FFFFF >> yExp) & uy.u;
    }

    if( x == 0.0f  )
    {
        if( y < 0.0f )
        {
            // pow(+-0, y) returns +-Inf and raises the ‘‘divide-by-zero’’ floating-point exception for y an odd integer < 0. 
            // pow(+-0, y) returns +Inf and raises the ‘‘divide-by-zero’’ floating-point exception for y < 0 and not an odd integer. 
            x = 1.0f / x;   //set div/0 flag and x to infinity of right sign

            if( 0 != yFracBits || 0 == yOneBit )    //if y is not an odd integer
                x = __builtin_fabsf( x );

            return x;
        }
        else  
        {   //y > 0. y == 0 case taken care of at the top of the function.

            // pow(+-0, y) returns +-0 for y an odd integer > 0. 
            // pow(+-0, y) returns +0 for y > 0 and not an odd integer. 
            if( 0 != yFracBits || 0 == yOneBit )    //if y is not an odd integer
                return 0.0f;
        
            return x;
        }
    }

    if( fabsx == __builtin_inff() )
    {
        // pow(+Inf, y) returns +0 for y < 0. 
        // pow(+Inf, y) returns +Inf for y > 0. 
        if( x == __builtin_inff() )
        {
            if( y < 0.0f )
                return 0.0f;
        
            return x;
        }
        else
        { //x = -Inf
            if( y < 0.0f )
            {
                // pow(-Inf, y) returns −0 for y an odd integer < 0. 
                // pow(-Inf, y) returns +0 for y < 0 and not an odd integer. 
                if( 0 != yFracBits || 0 == yOneBit )    //if y is not an odd integer
                    return 0.0f;
                    
                return -0.0f;
            }
            else
            { //y > 0. y == 0 case taken care of at the top of the function.
                // pow(−Inf, y) returns -Inf for y an odd integer > 0. 
                // pow(−Inf, y) returns +Inf for y > 0 and not an odd integer. 
                if( 0 != yFracBits || 0 == yOneBit )    //if y is not an odd integer
                    return __builtin_inff();
            
                return x;
            }
        }
    }
    
    if( fabsy == __builtin_inff() )
    {
        // pow(−1, +-Inf) returns 1. 
        if( x == -1.0f)
            return 1.0f;

        if( y == __builtin_inff() )
        {
            // pow(x, +Inf) returns +0 for |x| < 1. 
            if( fabsx < 1.0f )
                return 0.0f;
                
            // pow(x, +inf) returns +Inf for |x| > 1. 
            return __builtin_inff();
        }

        // pow(x, -Inf) returns +Inf for |x| < 1. 
        if( fabsx < 1.0f )
            return __builtin_inff();
        // pow(x, -Inf) returns +0 for |x| > 1. 
        return 0.0f;
    }

    // pow(x, y) returns a NaN and raises the ‘‘invalid’’ floating-point exception for finite x < 0 and finite non-integer y. 
    if( x < 0.0f && 0 != yFracBits )
    {
        SET_INVALID_FLAG();
        return __builtin_nanf("37");
    }

    //if y is an integer, less than 2**16, do iPow
    if( 0 == yFracBits && fabsy <= 0x1.0p16f )
    {
        int32_t iy = y; //should be exact
        int32_t yIsNeg = iy >> 31;
        iy = abs( iy );
        
        double dx = x;
        double result = iy & 1 ? dx : 1.0;
        
        while( iy >>= 1 )
        {
            dx *= dx;
            if( iy & 1 )
                result *= dx;
        }
    
        //We are using double precision here, so we don't need to worry about range differences between tiny vs huge numbers for negative Y
        if( yIsNeg )
            return (float) (1.0 / result);
            
        return (float) result;
    }
    
    if( 0.5f == fabsy )
    {
        double d = sqrt( (double) x );
    
        if( y < 0.0f )
            return (float) ( 1.0 / d );
    
        return (float) d;
    }
    
    //deal with 
        
    //Do pow the hard way

    //Convert x and y to double to do the arithmetic
    __m128d ylog2X;

    //We have two fallover cases for the log2 estimate above. 
    //
    //The first happens for values very near 1. This causes results near 0,
    //which means that our minimax error estimate is way off, because the results
    //are in a much smaller binade than normal, so the minimax error seems that much 
    //larger.
    //
    //The second case is for large y. This is a simple matter of minimax error * y
    //eventually exceeds our error threshold.
    //
    //In each case, we just fall back on the 80-bit log2 instruction, which costs us
    //an extra 40 cycles or so.
    //
    if( fabsy > 52.0f || ( 0.98f < fabsx && fabsx < 1.02f ) )
    {
        double temp;

        //use fyl2x to get y*log2(x)
        asm volatile( "flds (%1)\n flds (%2)\n fyl2x \n fstpl %0" : "=m" (*(&temp)) : "r" (&y),  "r" (&fabsx) );
            
        ylog2X = _mm_load_sd( &temp );
    }
    else
    {
        __m128d X = FLOAT_2_XDOUBLE( fabsx );
        __m128d Y = FLOAT_2_XDOUBLE( y );

        //Extract the exponent and mantissa. We are essentially doing frexp here, except that the mantissa is [1, 2)
        __m128d log2X = _mm_cvtepi32_pd( _mm_sub_epi32( _mm_srli_epi64( (__m128i) _mm_and_pd( X, exponentMask), 52 ), v1023 ) );     //always exact
        __m128d mantissa = _mm_or_pd( _mm_andnot_pd( exponentMask, X ), one_sd );

        //avoid setting inexact flag for exact powers of two
        if( 1.0 != XDOUBLE_2_DOUBLE( mantissa ) )
        { //take log2( mantissa )
            __m128d vMantissa = _mm_unpacklo_pd( mantissa, mantissa );                  //{ mantissa, mantissa }
            __m128d m1 = _mm_unpacklo_pd( one_sd, mantissa );                           // { m, 1 }
            __m128d m2 = _mm_mul_pd( vMantissa, vMantissa );                            //{ m**2, m**2 }
            __m128d m4 = _mm_mul_pd( m2, m2 );                                          //{ m**4, m**4 }
            __m128d v1 = _mm_mul_pd( m4, log2polynomialCoefficients[5] );               // { c11 m**4, c10 m**4 } 
            __m128d v  = _mm_mul_pd( m4, log2polynomialCoefficients[4] );               // { c9 m**4, c8 m**4 } 
            m2 = _mm_mul_pd( m2, m1 );
            v1 = _mm_add_pd( v1, log2polynomialCoefficients[3] );                       // { c7 + c11 m**4, c6 + c10 m**4 }
            v  = _mm_add_pd( v , log2polynomialCoefficients[2] );                       // { c5 + c9 m**4, c4 + c8 m**4 }
            v1 = _mm_mul_pd( v1, m4 );                                                  // { c7 m**4 + c11 m**8, c6 m**4 + c10 m**8 }
            v  = _mm_mul_pd( v , m4 );                                                  // { c5 m**4 + c9 m**8, c4 m**4 + c8 m**8 }
            v1 = _mm_add_pd( v1, log2polynomialCoefficients[1] );                       // { c3 + c7 m**4 + c11 m**8, c2 + c6 m**4 + c10 m**8 }
            v  = _mm_add_pd( v , log2polynomialCoefficients[0] );                       // { c1 + c5 m**4 + c9 m**8, c0 + c4 m**4 + c8 m**8 }
            v1 = _mm_mul_pd( v1, m2 );                                                  // { c3 m**3 + c7 m**7 + c11 m**11, c2 m**2 + c6 m**4 + c10 m**8 }
            v  = _mm_mul_pd( v , m1 );                                                  // { c1 m + c5 m**5 + c9 m**9, c0 + c4 m**4 + c8 m**8 }
            v  = _mm_add_pd( v , v1 );

            //Add the two sub terms horizontally
        #if defined( __SSE3__ )
            v = _mm_hadd_pd(  v, _mm_setzero_pd() );
        #else
            v = _mm_add_sd( v, _mm_unpackhi_pd( v, v ) );
        #endif
        
            log2X = _mm_add_sd( log2X, v );
        }
        
        ylog2X = _mm_mul_sd( log2X, Y );
    }

    // ylog2X = y * log2( x )
    double q = XDOUBLE_2_DOUBLE( ylog2X );
    
    //deal with overflow
    if( q >= 128.0 )
        return XFLOAT_2_FLOAT( REQUIRED_ADD_ss( huge, huge ) );

    //deal with underflow to zero
    if( q <= -150.0 )
        return XFLOAT_2_FLOAT( REQUIRED_MULTIPLY_ss( tiny, tiny ) );

    static const __m128d magic = { 0x1.0p52, 0.0 };
    __m128d smagic = _mm_or_pd( magic, _mm_and_pd( ylog2X, minusZero_sd ) );    //smagic = copysign( 0x1.0p52, ylog2X )

    // separate log2X into fractional and integer components
    __m128d vi = _mm_sub_sd( _mm_add_sd( ylog2X, smagic ), smagic );            // vi = rint( ylog2X )
    vi = _mm_sub_sd( vi, _mm_and_pd( _mm_cmpgt_sd( vi, ylog2X ), one_sd ) );     // vi = floor( ylog2X )
    
    //Avoid setting inexact if ylog2X is an integer
    if( _mm_ucomieq_sd( vi, ylog2X ) )
        return XDOUBLE_2_FLOAT( (xDouble) _mm_slli_epi64( _mm_add_epi32( _mm_cvtpd_epi32( vi ), bias ), 52 ) );

    __m128d vf = _mm_sub_sd( ylog2X, vi );                                       // vf = fractional part ( 0 <= vf < 1.0 )
    
    //For very small negative ylog2X, we can now have vf = 1.0, which causes trouble. 
    //In order to keep things sane we clamp vf to be 1.0 - EPSILON
    vf = _mm_min_sd( vf, almost1 );

    //reduce the fraction to the range [ 0, 2**-8 )
    int n = _mm_cvtsi128_si32( _mm_srli_epi64( _mm_and_si128( (__m128i) _mm_add_sd( vf, one_sd), mantissaHiBits ), 44 ) );
    vf = _mm_sub_sdm( vf, (double*)&exp2_table[ n ] + 1 );     //reduces d to [ 0, 2**-8 )
    
    //d = exp2(d), accurate to within 0.11116223e-9. We need to within 2**(-28) here 
    xDouble e = _mm_mul_sdm( vf, exp2_polynomial + 2 );
    e = _mm_add_sdm( e, exp2_polynomial + 1 );
    e = _mm_mul_sd( e, vf );
    e = _mm_add_sdm( e, exp2_polynomial );

    //Calculate exp2(i). Denormals are not possible because we use double precision here.
    __m128i ei = _mm_slli_epi64( _mm_cvtpd_epi32( vi ), 52 );     
    ei = _mm_add_epi64( ei, (__m128i) exp2_table[ n ] );    

    //scale by ei
    e = _mm_mul_sd( e, (__m128d) ei );
    
    float result =  XDOUBLE_2_FLOAT( e );

    if( x < 0.0f && yOneBit )
        return -result;

    return result;
}


