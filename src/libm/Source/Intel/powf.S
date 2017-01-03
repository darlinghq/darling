
/*
 *    powf.s
 *
 *        by Ian Ollmann
 *
 *    Copyright (c) 2007, Apple Inc. All Rights Reserved.
 *
 *    Implementation of C99 powf function for MacOS X __i386__ and __x86_64__ architectures.
 *
 */


#define LOCAL_STACK_SIZE 3*FRAME_SIZE
 
#include "machine/asm.h"
#include "abi.h"

.const
gMaskShift:    .byte    0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    \
                        0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    \
                        0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    \
                        0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    \
                        0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    \
                        0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    \
                        0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    \
                        0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7,    \
                        8, 9,10,11, 12,13,14,15,16,17,18,19,20,21,22,23,    \
                       24,25,26,27, 28,29,30,31,31,31,31,31,31,31,31,31,    \
                       31,31,31,31, 31,31,31,31,31,31,31,31,31,31,31,31,    \
                       31,31,31,31, 31,31,31,31,31,31,31,31,31,31,31,31,    \
                       31,31,31,31, 31,31,31,31,31,31,31,31,31,31,31,31,    \
                       31,31,31,31, 31,31,31,31,31,31,31,31,31,31,31,31,    \
                       31,31,31,31, 31,31,31,31,31,31,31,31,31,31,31,31,    \
                       31,31,31,31, 31,31,31,31,31,31,31,31,31,31,31,31

.align 4
// 8th order minimax fit of exp2 on [-1.0,1.0].  |error| < 0.402865722354948566583852e-9:
powf_exp_c:     .quad 0x40bc03f30399c376,   0x3ff000000001ea2a          // c4/c8 = 0.961813690023115610862381719985771e-2 / 0.134107709538786543922336536865157e-5, c0 = 1.0 + 0.278626872016317130037181614004e-10
                .quad 0x408f10e7f73e6d8f,   0x3fe62e42fd0933ee            // c5/c8 = 0.133318252930790403741964203236548e-2 / 0.134107709538786543922336536865157e-5, c1 = .693147176943623740308984004029708   
                .quad 0x405cb616a9384e69,   0x3fcebfbdfd0f0afa            // c6/c8 = 0.154016177542147239746127455226575e-3 / 0.134107709538786543922336536865157e-5, c2 = .240226505817268621584559118975830   
                .quad 0x4027173ebd288ba1,   0x3fac6b0a74f15403            // c7/c8 = 0.154832722143258821052933667742417e-4 / 0.134107709538786543922336536865157e-5, c3 = 0.555041568519883074165425891257052e-1 
                .quad 0x3eb67fe1dc3105ba                                // c8 = 0.134107709538786543922336536865157e-5 

//    The reduction for the log2 stage is done as:
//
//        for log2(x):
//            x = 2**i * 1.m                                    1.0 <= 1.m < 2.0
//            index = top 7 bits of m
//            reduced = 1.m * powf_log2_c[2*index]            1-2**-8 < reduced <= 1+2**-7
//            log2x = exp2(i) + powf_log2_c[2*index+1] + log1p(reduced - 1) / ln(2);
//
//        exp2(i) is exact
//        powf_log2_c[2*index] has 53 bits of precision, and is exact for the first and last entries
//        powf_log2_c[2*index+1] has 53 bits of precision, and is exact for the first and last entries
//        log1p( -2**-8 < x <= 2**-7 )  is done as a 5 term Taylor series. Error should be proportional
//            to the missing 6th order term or < 2**(6*-7)/6 ~ 2**-44, in the worst case. For smaller x, 
//            it will obviously be better. 
//        
//        Error in powf is rougly porportional to error_in_exp2_stage + y * error_in_log2_stage.
//        y is bounded by the point that powf overflows (underflow loses precision a bit sooner) 
//        For our 2**-7 worst case, y is about 10000 at overflow.  10000 * 2**-44 = ~6e-10, ~2**-31
//        which should give us a good margin of safety.  For cases that are in 1.0-2**-8 < x < 1.0+2**-7, 
//        we expect the precision of the Taylor series to improve faster than y's ability to disrupt 
//        the precision in ylog2(x) up to the point that we run out of precision in the double.  The 
//        worst case in this regard is probably 0x1.ffffep-1f, the number closest to 1. This overflows 
//        somewhere in the range y = [ -2**30, -2**31 ]. We predict we'll need 53-55 bits of precision
//        here, which may slightly exceed the precision of the double. While this number may come out 
//        wrong by a few ulps, all the other ones should be within tolerance since the next closest 
//        number is twice as far from 1.0. For these cases, the values in powf_log2_c are exact, so 
//        the only source of error is the Taylor series for log1p, and post-scaling by 1/ln(2).
//        For cases not in 1.0-2**-8 < x < 1.0+2**-7 the point at which y overflows is much smaller, so 
//        we don't need so much precision.
//
// Reduction table for log2 stage of power prepared as:
//        #include <stdio.h>
//        #include <stdint.h>
//        #include <math.h>
//
//        int main( void )
//        {
//                int i;
//
//                for( i = 0; i < 128; i++ )
//                {
//                        long double a = 1.0L /( 1.0L + (long double) i / (long double) 127 );
//                        union{ double d; uint64_t u;}u, v;
//                        u.d = a;
//                        v.d = -log2l(a);
//
//                        printf( ".quad 0x%llx,\t0x%llx\t// %Lg, -log2l(%Lg)\n", u.u, v.u, a, a );
//                }
//
//                return 0;
//        }
//
//    We use /127 here rather than /128 to allow the cases where the most precision is needed to be reduced by
//    exact powers of two. (These are 1.0 + 1 ulp and 1.0 - 1ulp.)  The other values will land somewhere in the
//  range [ 1.0 - 2**-8,  1.0 + 2**-7 ]. (Experimentally verified for all floats 1.0 <= x < 2.0.)
//
.align 3
powf_log2_c:    .quad 0x3ff0000000000000,    0x8000000000000000    // 1, -log2l(1)
                .quad 0x3fefc00000000000,    0x3f872c7ba20f7327    // 0.992188, -log2l(0.992188)
                .quad 0x3fef80fe03f80fe0,    0x3f9715662c7f3dbc    // 0.984496, -log2l(0.984496)
                .quad 0x3fef42f42f42f42f,    0x3fa13eea2b6545df    // 0.976923, -log2l(0.976923)
                .quad 0x3fef05dcd30dadec,    0x3fa6e7f0bd9710dd    // 0.969466, -log2l(0.969466)
                .quad 0x3feec9b26c9b26ca,    0x3fac85f25e12da51    // 0.962121, -log2l(0.962121)
                .quad 0x3fee8e6fa39be8e7,    0x3fb10c8cd0c74414    // 0.954887, -log2l(0.954887)
                .quad 0x3fee540f4898d5f8,    0x3fb3d0c813e48e00    // 0.947761, -log2l(0.947761)
                .quad 0x3fee1a8c536fe1a9,    0x3fb68fbf5169e028    // 0.940741, -log2l(0.940741)
                .quad 0x3fede1e1e1e1e1e2,    0x3fb949866f0b017b    // 0.933824, -log2l(0.933824)
                .quad 0x3fedaa0b3630957d,    0x3fbbfe30e28821c0    // 0.927007, -log2l(0.927007)
                .quad 0x3fed7303b5cc0ed7,    0x3fbeadd1b4ef9a1f    // 0.92029, -log2l(0.92029)
                .quad 0x3fed3cc6e80ebbdb,    0x3fc0ac3dc2e0ca0c    // 0.913669, -log2l(0.913669)
                .quad 0x3fed075075075075,    0x3fc1ff2046fb7116    // 0.907143, -log2l(0.907143)
                .quad 0x3fecd29c244fe2f3,    0x3fc34f99517622ae    // 0.900709, -log2l(0.900709)
                .quad 0x3fec9ea5dbf193d5,    0x3fc49db19c99a54d    // 0.894366, -log2l(0.894366)
                .quad 0x3fec6b699f5423ce,    0x3fc5e971b3a4ee80    // 0.888112, -log2l(0.888112)
                .quad 0x3fec38e38e38e38e,    0x3fc732e1f41ccdba    // 0.881944, -log2l(0.881944)
                .quad 0x3fec070fe3c070fe,    0x3fc87a0a8f0ff9b2    // 0.875862, -log2l(0.875862)
                .quad 0x3febd5eaf57abd5f,    0x3fc9bef38a4ffae5    // 0.869863, -log2l(0.869863)
                .quad 0x3feba5713280dee9,    0x3fcb01a4c19f6811    // 0.863946, -log2l(0.863946)
                .quad 0x3feb759f2298375a,    0x3fcc4225e7d5e3c6    // 0.858108, -log2l(0.858108)
                .quad 0x3feb4671655e7f24,    0x3fcd807e87fa4521    // 0.852349, -log2l(0.852349)
                .quad 0x3feb17e4b17e4b18,    0x3fcebcb6065350a2    // 0.846667, -log2l(0.846667)
                .quad 0x3feae9f5d3eba7d7,    0x3fcff6d3a16f617f    // 0.84106, -log2l(0.84106)
                .quad 0x3feabca1af286bca,    0x3fd0976f3991af9e    // 0.835526, -log2l(0.835526)
                .quad 0x3fea8fe53a8fe53b,    0x3fd1326eb8c0aba3    // 0.830065, -log2l(0.830065)
                .quad 0x3fea63bd81a98ef6,    0x3fd1cc6bb7e3870f    // 0.824675, -log2l(0.824675)
                .quad 0x3fea3827a3827a38,    0x3fd265698fa26c0a    // 0.819355, -log2l(0.819355)
                .quad 0x3fea0d20d20d20d2,    0x3fd2fd6b881e82d3    // 0.814103, -log2l(0.814103)
                .quad 0x3fe9e2a65187566c,    0x3fd39474d95e1649    // 0.808917, -log2l(0.808917)
                .quad 0x3fe9b8b577e61371,    0x3fd42a88abb54986    // 0.803797, -log2l(0.803797)
                .quad 0x3fe98f4bac46d7c0,    0x3fd4bfaa182b7fe3    // 0.798742, -log2l(0.798742)
                .quad 0x3fe9666666666666,    0x3fd553dc28dd9724    // 0.79375, -log2l(0.79375)
                .quad 0x3fe93e032e1c9f02,    0x3fd5e721d95d124d    // 0.78882, -log2l(0.78882)
                .quad 0x3fe9161f9add3c0d,    0x3fd6797e170c5221    // 0.783951, -log2l(0.783951)
                .quad 0x3fe8eeb9533d4065,    0x3fd70af3c177f740    // 0.779141, -log2l(0.779141)
                .quad 0x3fe8c7ce0c7ce0c8,    0x3fd79b85aaad8878    // 0.77439, -log2l(0.77439)
                .quad 0x3fe8a15b8a15b8a1,    0x3fd82b36978f76d5    // 0.769697, -log2l(0.769697)
                .quad 0x3fe87b5f9d4d1bc2,    0x3fd8ba09402697ed    // 0.76506, -log2l(0.76506)
                .quad 0x3fe855d824ca58e9,    0x3fd948004ff12dbf    // 0.760479, -log2l(0.760479)
                .quad 0x3fe830c30c30c30c,    0x3fd9d51e662f92a2    // 0.755952, -log2l(0.755952)
                .quad 0x3fe80c1e4bbd595f,    0x3fda6166162e9ec8    // 0.751479, -log2l(0.751479)
                .quad 0x3fe7e7e7e7e7e7e8,    0x3fdaecd9e78fdbea    // 0.747059, -log2l(0.747059)
                .quad 0x3fe7c41df1077c42,    0x3fdb777c568f9ae2    // 0.74269, -log2l(0.74269)
                .quad 0x3fe7a0be82fa0be8,    0x3fdc014fd448fe3a    // 0.738372, -log2l(0.738372)
                .quad 0x3fe77dc7c4cf2aea,    0x3fdc8a56c6f80bca    // 0.734104, -log2l(0.734104)
                .quad 0x3fe75b37e875b37f,    0x3fdd12938a39d6f0    // 0.729885, -log2l(0.729885)
                .quad 0x3fe7390d2a6c405e,    0x3fdd9a086f4ad416    // 0.725714, -log2l(0.725714)
                .quad 0x3fe71745d1745d17,    0x3fde20b7bd4365a8    // 0.721591, -log2l(0.721591)
                .quad 0x3fe6f5e02e4850ff,    0x3fdea6a3b152b1e6    // 0.717514, -log2l(0.717514)
                .quad 0x3fe6d4da9b536a6d,    0x3fdf2bce7ef7d06b    // 0.713483, -log2l(0.713483)
                .quad 0x3fe6b4337c6cb157,    0x3fdfb03a50395dba    // 0.709497, -log2l(0.709497)
                .quad 0x3fe693e93e93e93f,    0x3fe019f4a2edc134    // 0.705556, -log2l(0.705556)
                .quad 0x3fe673fa57b0cbab,    0x3fe05b6ebbca3d9a    // 0.701657, -log2l(0.701657)
                .quad 0x3fe6546546546546,    0x3fe09c8c7a1fd74c    // 0.697802, -log2l(0.697802)
                .quad 0x3fe63528917c80b3,    0x3fe0dd4ee107ae0a    // 0.693989, -log2l(0.693989)
                .quad 0x3fe61642c8590b21,    0x3fe11db6ef5e7873    // 0.690217, -log2l(0.690217)
                .quad 0x3fe5f7b282135f7b,    0x3fe15dc59fdc06b7    // 0.686486, -log2l(0.686486)
                .quad 0x3fe5d9765d9765d9,    0x3fe19d7be92a2310    // 0.682796, -log2l(0.682796)
                .quad 0x3fe5bb8d015e75bc,    0x3fe1dcdabdfad537    // 0.679144, -log2l(0.679144)
                .quad 0x3fe59df51b3bea36,    0x3fe21be30d1e0ddb    // 0.675532, -log2l(0.675532)
                .quad 0x3fe580ad602b580b,    0x3fe25a95c196bef3    // 0.671958, -log2l(0.671958)
                .quad 0x3fe563b48c20563b,    0x3fe298f3c2af6595    // 0.668421, -log2l(0.668421)
                .quad 0x3fe5470961d7ca63,    0x3fe2d6fdf40e09c5    // 0.664921, -log2l(0.664921)
                .quad 0x3fe52aaaaaaaaaab,    0x3fe314b535c7b89e    // 0.661458, -log2l(0.661458)
                .quad 0x3fe50e97366227cb,    0x3fe3521a64737cf3    // 0.658031, -log2l(0.658031)
                .quad 0x3fe4f2cddb0d3225,    0x3fe38f2e593cda73    // 0.654639, -log2l(0.654639)
                .quad 0x3fe4d74d74d74d75,    0x3fe3cbf1e9f5cf2f    // 0.651282, -log2l(0.651282)
                .quad 0x3fe4bc14e5e0a72f,    0x3fe40865e9285f33    // 0.647959, -log2l(0.647959)
                .quad 0x3fe4a12316176410,    0x3fe4448b2627ade3    // 0.64467, -log2l(0.64467)
                .quad 0x3fe48676f31219dc,    0x3fe480626d20a876    // 0.641414, -log2l(0.641414)
                .quad 0x3fe46c0f6feb6ac6,    0x3fe4bbec872a4505    // 0.638191, -log2l(0.638191)
                .quad 0x3fe451eb851eb852,    0x3fe4f72a3a555958    // 0.635, -log2l(0.635)
                .quad 0x3fe4380a3065e3fb,    0x3fe5321c49bc0c91    // 0.631841, -log2l(0.631841)
                .quad 0x3fe41e6a74981447,    0x3fe56cc37590e6c5    // 0.628713, -log2l(0.628713)
                .quad 0x3fe4050b59897548,    0x3fe5a7207b2d815a    // 0.625616, -log2l(0.625616)
                .quad 0x3fe3ebebebebebec,    0x3fe5e1341520db00    // 0.622549, -log2l(0.622549)
                .quad 0x3fe3d30b3d30b3d3,    0x3fe61afefb3d5201    // 0.619512, -log2l(0.619512)
                .quad 0x3fe3ba68636adfb0,    0x3fe65481e2a6477b    // 0.616505, -log2l(0.616505)
                .quad 0x3fe3a2027932b48f,    0x3fe68dbd7ddd6e15    // 0.613527, -log2l(0.613527)
                .quad 0x3fe389d89d89d89e,    0x3fe6c6b27ccfc698    // 0.610577, -log2l(0.610577)
                .quad 0x3fe371e9f3c04e64,    0x3fe6ff618ce24cd7    // 0.607656, -log2l(0.607656)
                .quad 0x3fe35a35a35a35a3,    0x3fe737cb58fe5716    // 0.604762, -log2l(0.604762)
                .quad 0x3fe342bad7f64b39,    0x3fe76ff0899daa49    // 0.601896, -log2l(0.601896)
                .quad 0x3fe32b78c13521d0,    0x3fe7a7d1c4d64520    // 0.599057, -log2l(0.599057)
                .quad 0x3fe3146e92a10d38,    0x3fe7df6fae65e424    // 0.596244, -log2l(0.596244)
                .quad 0x3fe2fd9b8396ba9e,    0x3fe816cae7bd40b1    // 0.593458, -log2l(0.593458)
                .quad 0x3fe2e6fecf2e6fed,    0x3fe84de4100b0ce2    // 0.590698, -log2l(0.590698)
                .quad 0x3fe2d097b425ed09,    0x3fe884bbc446ae3f    // 0.587963, -log2l(0.587963)
                .quad 0x3fe2ba6574cae996,    0x3fe8bb529f3ab8f3    // 0.585253, -log2l(0.585253)
                .quad 0x3fe2a46756e62a46,    0x3fe8f1a9398f2d58    // 0.582569, -log2l(0.582569)
                .quad 0x3fe28e9ca3a728ea,    0x3fe927c029d3798a    // 0.579909, -log2l(0.579909)
                .quad 0x3fe27904a7904a79,    0x3fe95d980488409a    // 0.577273, -log2l(0.577273)
                .quad 0x3fe2639eb2639eb2,    0x3fe993315c28e8fb    // 0.574661, -log2l(0.574661)
                .quad 0x3fe24e6a171024e7,    0x3fe9c88cc134f3c3    // 0.572072, -log2l(0.572072)
                .quad 0x3fe239662b9f91cb,    0x3fe9fdaac2391e1c    // 0.569507, -log2l(0.569507)
                .quad 0x3fe2249249249249,    0x3fea328bebd84e80    // 0.566964, -log2l(0.566964)
                .quad 0x3fe20fedcba98765,    0x3fea6730c8d44efa    // 0.564444, -log2l(0.564444)
                .quad 0x3fe1fb78121fb781,    0x3fea9b99e21655eb    // 0.561947, -log2l(0.561947)
                .quad 0x3fe1e7307e4ef157,    0x3feacfc7beb75e94    // 0.559471, -log2l(0.559471)
                .quad 0x3fe1d31674c59d31,    0x3feb03bae40852a0    // 0.557018, -log2l(0.557018)
                .quad 0x3fe1bf295cc93903,    0x3feb3773d59a05ff    // 0.554585, -log2l(0.554585)
                .quad 0x3fe1ab68a0473c1b,    0x3feb6af315450638    // 0.552174, -log2l(0.552174)
                .quad 0x3fe197d3abc65f4f,    0x3feb9e3923313e58    // 0.549784, -log2l(0.549784)
                .quad 0x3fe18469ee58469f,    0x3febd1467ddd70a7    // 0.547414, -log2l(0.547414)
                .quad 0x3fe1712ad98b8957,    0x3fec041ba2268731    // 0.545064, -log2l(0.545064)
                .quad 0x3fe15e15e15e15e1,    0x3fec36b90b4ebc3a    // 0.542735, -log2l(0.542735)
                .quad 0x3fe14b2a7c2fee92,    0x3fec691f33049ba0    // 0.540426, -log2l(0.540426)
                .quad 0x3fe1386822b63cbf,    0x3fec9b4e9169de22    // 0.538136, -log2l(0.538136)
                .quad 0x3fe125ce4feeb7a1,    0x3feccd479d1a1f94    // 0.535865, -log2l(0.535865)
                .quad 0x3fe1135c81135c81,    0x3fecff0acb3170e3    // 0.533613, -log2l(0.533613)
                .quad 0x3fe10112358e75d3,    0x3fed30988f52c6d3    // 0.531381, -log2l(0.531381)
                .quad 0x3fe0eeeeeeeeeeef,    0x3fed61f15bae4663    // 0.529167, -log2l(0.529167)
                .quad 0x3fe0dcf230dcf231,    0x3fed9315a1076fa2    // 0.526971, -log2l(0.526971)
                .quad 0x3fe0cb1b810ecf57,    0x3fedc405cebb27dc    // 0.524793, -log2l(0.524793)
                .quad 0x3fe0b96a673e2808,    0x3fedf4c252c5a3e1    // 0.522634, -log2l(0.522634)
                .quad 0x3fe0a7de6d1d6086,    0x3fee254b99c83339    // 0.520492, -log2l(0.520492)
                .quad 0x3fe096771e4d528c,    0x3fee55a20f0eecf9    // 0.518367, -log2l(0.518367)
                .quad 0x3fe0853408534085,    0x3fee85c61c963f0d    // 0.51626, -log2l(0.51626)
                .quad 0x3fe07414ba8f0741,    0x3feeb5b82b10609b    // 0.51417, -log2l(0.51417)
                .quad 0x3fe06318c6318c63,    0x3feee578a1eaa83f    // 0.512097, -log2l(0.512097)
                .quad 0x3fe0523fbe3367d7,    0x3fef1507e752c6c8    // 0.51004, -log2l(0.51004)
                .quad 0x3fe04189374bc6a8,    0x3fef4466603be71d    // 0.508, -log2l(0.508)
                .quad 0x3fe030f4c7e7859c,    0x3fef73947063b3fd    // 0.505976, -log2l(0.505976)
                .quad 0x3fe0208208208208,    0x3fefa2927a574422    // 0.503968, -log2l(0.503968)
                .quad 0x3fe0103091b51f5e,    0x3fefd160df77ed7a    // 0.501976, -log2l(0.501976)
                .quad 0x3fe0000000000000,    0x3ff0000000000000    // 0.5, -log2l(0.5)

// Taylor series coefficients for log2 stage
powf_logTaylor:    .double        -0.5, 0.33333333333333333333333333333333, -0.25, 0.2


.literal8
oneD:        .double        1.0
d128:        .double        128.0
dm150:       .double        -150.0
recip_ln2:   .quad          0x3ff71547652b82fe      // 1.0 / ln(2)

.literal4
infF:        .long        0x7f800000        // inf
minfF:       .long        0xff800000        // -inf
oneF:        .long        0x3f800000        // 1.0f
moneF:       .long        0xbf800000        // -1.0f
maxy:        .long        0x4effffff        // 0x1.0p31f - 1 ulp
miny:        .long        0xCeffffff        // -0x1.0p31f + 1 ulp
mantissaMask:        .long        0x007fffff

.text

#if defined( __x86_64__ )
    #define SI_P        %rsi
    #define DI_P        %rdi
    #define RELATIVE_ADDR( _a)                                (_a)( %rip )
#else
    #define SI_P        %esi
    #define DI_P        %edi
    #define RELATIVE_ADDR( _a)                                (_a)-0b( BX_P )
#endif

ENTRY( powf )
#if defined( __i386__ )
    movl    FRAME_SIZE( STACKP ),         %eax
    movl    4+FRAME_SIZE( STACKP ),       %edx
    movss   FRAME_SIZE( STACKP ),         %xmm0
    movss   4+FRAME_SIZE( STACKP ),       %xmm1
#else
    movd    %xmm0,                        %eax
    movd    %xmm1,                        %edx
#endif

    //early out for x == 1.0
    cmpl    $0x3f800000,                  %eax        //if( x == 1.0 )
    je      6f                                        //        goto 6

    //early out for y == 1.0  (costs 1 cycle for x86_64, free for i386) 
    cmpl    $0x3f800000,                  %edx        //if( y == 1.0 )
    je        6f                                      //        goto 6

    andl    $0x7fffffff,                  %edx        // |y|
    
    
    // Find out if y is an integer without raising inexact
    // Note tested over entire range. Fails for Inf/NaN, but we don't care about that here.
    push    BX_P
    push    SI_P
    push    DI_P

#if defined( __i386__ )    
    call    0f
0:  pop        BX_P
#else
    xorq    %rdi,                         %rdi
#endif

    // check to see if we fell into an edge case
    subl    $1,                           %eax
    subl    $1,                           %edx
    cmpl    $0x7f7fffff,                  %eax        // if( x < 0 || x == inf || isnan(x) )
    jae        7f                                     //        goto 7
    cmpl    $0x4affffff,                  %edx        // if( |y| >= 0x1.0p23 || 0 == y || isnan(y)  )
    jae        7f                                     //        goto 7

    cmpl    $0x3effffff,                  %edx        // if( |y| == 0.5f )
    je        8f                                      //        goto 8
    
// The main part of pow:
//        0 < x < inf,    |y| < 0x1.0p31,        x != 1,        y != 0

    addl    $1,                          %eax
    andl    $0x7fffffff,                 %eax        // |x|

#if 0
    // if y is integer, call ipowf instead
    addl    $1,                          %edx
    movl    %edx,                        %edi        // |y|
    lea     RELATIVE_ADDR(gMaskShift),   CX_P        // gMaskShift ptr
    shrl    $23,                         %edi        // |y| >> 23
    movzbl  (CX_P, DI_P, 1),             %ecx        // gMaskShift[ |y| >> 23 ]
    mov     $0x3fffffff,                 DI_P        // 0x3fffffff
    shrl    %cl,                         %edi        // 0x3fffffff >> gMaskShift[ |y| >> 23 ]
    andl    %edx,                        %edi        // fractional part of y
    cmpl    $0,                          %edi
    je      ___ipowf
#endif

    //separate |x| into 2**i * 1.m
    movss   RELATIVE_ADDR( mantissaMask), %xmm3
    movss   RELATIVE_ADDR( oneF),         %xmm2
    andps   %xmm3,                        %xmm0       // m
    orps    %xmm2,                        %xmm0       // 1.m
    shrl    $23,                          %eax        // exponent + bias
    cmpl    $0,                           %eax
    jne     1f
    
    // normalize denormal x
    subss   %xmm2,                        %xmm0       //    1.m - 1.0
    movd    %xmm0,                        %eax
    shrl    $23,                          %eax        // exponent + bias
    andps   %xmm3,                        %xmm0       // m
    orps    %xmm2,                        %xmm0       // 1.m
    subl    $126,                         %eax

1:  subl    $127,                         %eax        // i = exponent - bias
    cvtsi2sd    %eax,                     %xmm3       // log2x = (double) i
     
    //check for unit mantissa
    ucomiss %xmm2,                        %xmm0       // if( 1.m == 1.0 )
    je      2f                                        //        skip to 2

    //handle non-unit mantissa here
    movd    %xmm0,                        %eax        // set aside 1.m
#if defined( __x86_64__ )
    cdqe
#endif
    cvtss2sd    %xmm0,                    %xmm0        // r = (double) 1.m
    lea      RELATIVE_ADDR( powf_log2_c ), CX_P
    
    // use the top 7 bits of the mantissa to index the powf_log2_c table
    shr      $(23-7-4),                   AX_P
    and      $0x7f0,                      AX_P
    
    // reduce r to 1-2**7 < r < 1+2**-7
    mulsd    (CX_P, AX_P, 1),            %xmm0        // r *= powf_log2_c[ 2 * index ]

    // compensate in log2x by adding powf_log2_c[ 2 * index + 1]
    //     do this early so that we force -1.0 + 1.0 to avoid (-1.0 + tiny) + 1.0 later. 
    //    Precision loss from this is at most 7 bits, which is acceptable    
    addsd    8(CX_P, AX_P, 1),          %xmm3        // log2x + powf_log2_c[ 2 * index + 1]
    
    // we calculate log2(r) as log1p( r-1 ) / ln(2)
    subsd    RELATIVE_ADDR(oneD),       %xmm0        // r -= 1.0

    
    // log(1+r) = r - rr/2 + rrr/3 - rrrr/4 + rrrrr/5        
    //        with -2**-7 < r < 2**-7, should be good to (5+1)*7 +2 = 44 bits of accuracy or so
    //            (5+1) because the error term is roughly equal to the missing r**6/6 term 
    lea          RELATIVE_ADDR( powf_logTaylor ), CX_P
    movsd        8(CX_P),                  %xmm4
    movsd        24(CX_P),                 %xmm5
    movapd       %xmm0,                    %xmm2        // r
    mulsd        %xmm0,                    %xmm0        // rr
    mulsd        %xmm2,                    %xmm4        // 0.333333333333333333333r
    mulsd        %xmm2,                    %xmm5        // 0.2r
    addsd        (CX_P),                   %xmm4        // -0.5 + 0.333333333333333333333r
    addsd        16(CX_P),                 %xmm5        // -0.25 + 0.2r
    mulsd        %xmm0,                    %xmm4        // -0.5rr + 0.333333333333333333333rrr
    mulsd        %xmm0,                    %xmm0        // rrrr
    addsd        %xmm2,                    %xmm4        // r - 0.5rr + 0.333333333333333333333rrr
    mulsd        %xmm0,                    %xmm5        // -0.25rrrr + 0.2rrrrr
    addsd        %xmm5,                    %xmm4        // r - 0.5rr + 0.333333333333333333333rrr - 0.25rrrr + 0.2rrrrr
    mulsd        RELATIVE_ADDR( recip_ln2), %xmm4       // ( r - 0.5rr + 0.333333333333333333333rrr - 0.25rrrr + 0.2rrrrr ) * (1/ln(2))
    addsd        %xmm4,                    %xmm3        // log2x + powf_log2_c[ 2 * index + 1] + ( r - 0.5rr + 0.333333333333333333333rrr - 0.25rrrr + 0.2rrrrr ) * (1/ln(2))

    // multiply by y
2:  cvtss2sd     %xmm1,                    %xmm0
    mulsd        %xmm3,                    %xmm0        // y * log2(x) 

    ucomisd      RELATIVE_ADDR( d128),     %xmm0        // if( ylog2(x) >= 128 )
    jae            4f                                   //        goto 4

    ucomisd      RELATIVE_ADDR( dm150),    %xmm0        // if( ylog2(x) <= -150
    jbe            4f                                   //        goto 4

    // separate ylog2(x) into i + f
    cvttsd2si    %xmm0,                    %eax         // i = (int) ylog2(x)
    cvtsi2sd     %eax,                     %xmm1        // trunc( ylog2(x) )
    subsd        %xmm1,                    %xmm0        // f

    // calculate 2**i
    addl        $1023,                     %eax         // exponent + bias
    movd        %eax,                      %xmm7        // move to vector register
    psllq       $52,                       %xmm7        // shift exponent + bias into place

    // early out for power of 2
    xorpd        %xmm6,                    %xmm6
    ucomisd      %xmm0,                    %xmm6
    movsd        RELATIVE_ADDR( oneD),     %xmm1
    je            3f
    
    //f = exp2(f)
#if defined( __SSE3__ )
    movddup      %xmm0,                    %xmm1        // { f, f }
#else
    movapd       %xmm0,                    %xmm1
    unpcklpd     %xmm1,                    %xmm1        //    { f, f }
#endif
    mulsd        %xmm0,                    %xmm0        // ff = f*f
    movapd       %xmm1,                    %xmm3        // { f, f }
    lea          RELATIVE_ADDR( powf_exp_c ), CX_P
    mulpd        48(CX_P),                 %xmm1        // { c3f, (c7/c8)f }
    mulpd        16(CX_P),                 %xmm3        // { c1f, (c5/c8)f }
#if defined( __SSE3__ )
    movddup      %xmm0,                    %xmm4        // { ff, ff }
#else
    movapd       %xmm0,                    %xmm4
    unpcklpd     %xmm4,                    %xmm4        //    { ff, ff }
#endif
    mulsd        %xmm0,                    %xmm0        // ffff = ff * ff
    addpd        32(CX_P),                 %xmm1        // { c2 + c3f, (c6/c8) + (c7/c8)f }
    addpd        (CX_P),                   %xmm3        // { c0 + c1f, (c4/c8) + (c5/c8)f }
    mulpd        %xmm4,                    %xmm1        // { c2ff + c3fff, (c6/c8)ff + (c7/c8)fff }
    addsd        %xmm0,                    %xmm3        // { c0 + c1x, (c4/c8) + (c5/c8)f + ffff }
    mulsd        64(CX_P),                 %xmm0        // c8ffff
    addpd        %xmm1,                    %xmm3        // { c0 + c1f + c2ff + c3fff, (c4/c8) + (c5/c8)f + (c6/c8)ff + (c7/c8)fff + ffff }
    movhlps      %xmm3,                    %xmm1        // { ?, c0 + c1f + c2ff + c3fff }
    mulsd        %xmm0,                    %xmm3        // { ..., c8ffff* ((c4/c8) + (c5/c8)f + (c6/c8)ff + (c7/c8)fff + ffff) } 
    addsd        %xmm3,                    %xmm1        // c0 + c1f + c2ff + c3fff + c4ffff + c5fffff + c6ffffff + c7fffffff + c8fffffffff

    // scale by 2**i, and convert to float
3:    mulsd      %xmm1,                    %xmm7
    xorps        %xmm0,                    %xmm0
    cvtsd2ss     %xmm7,                    %xmm0

    pop        DI_P
    pop        SI_P
    pop        BX_P
#if defined( __i386__ )
    movss    %xmm0,    FRAME_SIZE( STACKP )
    flds    FRAME_SIZE( STACKP )
#endif
    ret

    // overflow / underflow
4:  xorpd    %xmm1,                        %xmm1       // 0
    cmpltsd  %xmm0,                        %xmm1       // 0 < ylog2(x) ? -1LL : 0
    movd     %xmm1,                        %eax        // 0 < ylog2(x) ? -1U : 0
    andl     $0xfff,                       %eax        // 0 < ylog2(x) ?  0xfff : 0
    xorl     $0x801,                       %eax        // 0 < ylog2(x) ?  0x7fe : 0x801 
    movd     %eax,                         %xmm2       // 0 < ylog2(x) ?  0x7fe : 0x801
    psllq    $52,                          %xmm2       // 0 < ylog2(x) ? 0x1.0p+1023 : -0x1.0p-1022
    mulsd    %xmm0,                        %xmm2       // result = ylog2(x) * (0 < ylog2(x) ? 0x1.0p+1023 : -0x1.0p-1022)
    xorps    %xmm0,                        %xmm0       // 0
    cvtsd2ss %xmm2,                        %xmm0       // convert result to float
    jmp      9f


    // ( x < 0 && isfinite(x) && |y| is not in { 0, inf, NaN } )  or x is unknown, but |y| >= 0x1.0p23
5:  cmpl    $0,                            %edi        // if( y is not an integer )
    jne     8f
    
    // since we know y is an integer, we can just call ipowf
    jmp    ___ipowf
    
6:    // x == 1.0f return x
#if    defined( __i386__ )
    flds    FRAME_SIZE( STACKP )
#endif
    ret
    
    //    A whole basket of special cases lands here
    // (x <= 0 || x == Inf || isnan(x)) or ( |y| >= 0x1.0p23f || y == 0 || isnan(y) )
    //        all we have to do is figure out which one!
7:
    addl    $1,                            %eax        // |y|
    addl    $1,                            %edx        // |y|
    andl    $0x7fffffff,                   %eax        // |x|
    cmpl    $0,                            %edx        // if( |y| == 0 )
    je      4f                                         //        goto 4

// (x <= 0 || x == Inf || isnan(x)) or ( |y| >= 0x1.0p23 || isnan(y) )

    //check for NaNs
    ucomiss  %xmm0,                        %xmm1
    jp       7f

// (x <= 0 || x == Inf ) or |y| >= 0x1.0p23f
    
    // calculate fractional part of y and ones bit of y
    movl    %edx,                         %edi        // |y|
    lea     RELATIVE_ADDR(gMaskShift),    CX_P        // gMaskShift ptr
    shrl    $23,                          %edi        // |y| >> 23
    movzbl  (CX_P, DI_P, 1),              %ecx        // gMaskShift[ |y| >> 23 ]
    mov     $0x3fffffff,                  DI_P        // 0x3fffffff
    mov     $0x40000000,                  SI_P        // 0x40000000
    shrl    %cl,                          %edi        // 0x3fffffff >> gMaskShift[ |y| >> 23 ]
    shrl    %cl,                          %esi        // 0x40000000 >> gMaskShift[ |y| >> 23 ]
    andl    %edx,                         %edi        // fractional part of y
    andl    %edx,                         %esi        // ones bit of y

    // if( x == 0 )        goto 2
    xorps   %xmm2,                        %xmm2
    ucomiss %xmm0,                        %xmm2
    je      2f
    
// (x < 0 || x == Inf ) or |y| >= 0x1.0p23f

    // if( |y| == inf )    goto 3
    cmpl    $0x7f800000,                  %edx
    je      3f

// (x < 0 || x == Inf)  or ( 0x1.0p23f <= |y| < inf )

    // if( x == inf )    goto 5
    ucomiss   RELATIVE_ADDR( infF ),      %xmm0
    je        5f

// x < 0 or ( 0x1.0p23f <= |y| < inf )

    // negative finite x or large y go off to be considered for ipowf
    ucomiss   RELATIVE_ADDR( minfF ),     %xmm0        // if( x != -inf )      
    ja        5b                                       //        goto the other 5

// x == -inf

    // At this point, we know that x is -Inf and |y| is not in { 0, Inf, NaN }.
    // Deal with y is odd integer cases
    // if( 0 == fractionalBits && 0 != onesBit )
    movl    %edi,                         %ecx        // fractional Bits
    subl    $1,                           %ecx        // fractionalBits == 0 ? -1 : some non-negative number
    sarl    $31,                          %ecx        // fractionalBits == 0 ? -1 : 0
    andl    %esi,                         %ecx        // fractionalBits == 0 ? onesBit : 0
    cmpl    $0,                           %ecx        // if( 0 == fractionalBits && 0 != onesBit )
    jne      6f                                       //        goto 6
    
//    x = -inf, |y| is not in { 0, Inf, NaN }, and y is not an odd integer

    // if( 0.0f < y )    return -x;        else return 0
    cmpltss  %xmm1,                       %xmm2      //  0.0f < y ? -1 : 0 
    andps    %xmm2,                       %xmm0      //  0.0f < y ?  x : 0
    pslld    $31,                         %xmm2      //  0.0f < y ? 0x80000000 : 0
    xorps    %xmm2,                       %xmm0      //  0.0f < y ?  -x : 0
    jmp      9f        //return 0

    // x < 0 && y is not an integer, or |y| == 0.5f 
8:  xorps    %xmm2,                       %xmm2
    cmpless  %xmm0,                       %xmm2        //  0 <= x ? -1 : 0
    andps    %xmm1,                       %xmm2        //    0 <= x ?  y : 0
    xorps    %xmm3,                       %xmm3
    ucomiss  %xmm2,                       %xmm3        //  if( x >= 0 && 0 > y )
    ja       1f                                        //        goto 1

    sqrtss   %xmm0,                      %xmm0
    jmp      9f    //return
    
//    y == -0.5f && x > 0
1:  cvtss2sd   %xmm0,                    %xmm0
    movsd      RELATIVE_ADDR( oneD ),    %xmm1
    divsd      %xmm0,                    %xmm1
    sqrtsd     %xmm1,                    %xmm1
    xorps      %xmm0,                    %xmm0
    cvtsd2ss   %xmm1,                    %xmm0
    jmp        9f    // return


    // x == 0
2:    // if( y is an odd integer )        goto 8
    movl    %edi,                        %ecx        // fractional Bits
    subl    $1,                          %ecx        // fractionalBits == 0 ? -1 : some non-negative number
    sarl    $31,                         %ecx        // fractionalBits == 0 ? -1 : 0
    andl    %esi,                        %ecx        // fractionalBits == 0 ? onesBit : 0
    cmpl    $0,                          %ecx        // if( fractionalBits == 0 && 0 != onesBit )
    jne     8f                                       //        y is an odd integer, goto 8

    xorps    %xmm0,                      %xmm0       // x = fabsf(x)
    ucomiss  %xmm1,                      %xmm0       // if( 0 < y )
    jb       9f                                      //        return x

    //return 1.0 / f
    movss    RELATIVE_ADDR( oneF ),      %xmm1
    divss    %xmm0,                      %xmm1        // return inf and set div/0
    movaps   %xmm1,                      %xmm0
    jmp      9f

    // |y| == inf
3:  ucomiss  RELATIVE_ADDR( moneF ),     %xmm0        // if( -1.0f == x )
    je       4f                                       //        return 1.0f

    cmpl     $0x3f7fffff,                %eax         // if( |x| > 1.0f )
    ja       1f                                       //        goto 1f
    
    xorps    %xmm0,                      %xmm0        // 0.0f
    cmpnless %xmm1,                      %xmm0        // y == inf ? 0 : -1 
    psrld    $1,                         %xmm0        // y == inf ? 0 : 0x7fffffff
    andps    %xmm1,                      %xmm0        // y == inf ? 0 : inf
    jmp      9f                                       // return
    
    // return 1.0f
4:  movl     $1,                        %ecx
    xorps    %xmm0,                     %xmm0
    cvtsi2ss %ecx,                      %xmm0
    jmp      9f

    // x == inf
5:  xorps    %xmm2,                     %xmm2
    cmpltss  %xmm1,                     %xmm2        // 0 < y ? -1 : 0
    andps    %xmm2,                     %xmm0        // 0 < y ?  x : 0
    jmp      9f

    // 0 == fractionalBits && 0 != onesBit 
6:  xorps    %xmm2,                     %xmm2
    ucomiss  %xmm1,                     %xmm2        // if( 0 < y )
    jb       9f                                        //return x                        

    movl    $0x80000000,                %ecx        // -0.0f
    movd    %ecx,                       %xmm0        // copy to xmm, zero high part of register
    jmp     9f        //return -0.0

    
7:    // NaN
    addss   %xmm1,                      %xmm0
    jmp     9f

    // x == 0, y is an odd integer
8:  ucomiss   %xmm1,                    %xmm2    // if( 0 < y )
    jb        9f                                    //        return x
    
    //return 1.0 / f
    movss    RELATIVE_ADDR( oneF ),     %xmm1
    divss    %xmm0,                     %xmm1        // return inf and set div/0
    movaps   %xmm1,                     %xmm0
    jmp      9f

    //|y| == inf, |x| > 1.0f
1:  xorps    %xmm0,                        %xmm0
    cmpltss  %xmm1,                        %xmm0
    andps    %xmm1,                        %xmm0
    jmp      9f
    

.align 4
    // return value in %xmm0
9:
    pop        DI_P
    pop        SI_P
    pop        BX_P
#if defined( __i386__ )
    movss      %xmm0,    FRAME_SIZE( STACKP )
    flds       FRAME_SIZE( STACKP )
#endif
    ret
    
// x and y passed in in xmm0 and xmm1
//  result returned in xmm0 
//    BX_P already points to label 0 above
___ipowf:
    // clamp INT_MIN <= y < INT_MAX.  Values outside this range can't be odd numbers.
    maxss        RELATIVE_ADDR( miny ),    %xmm1
    minss        RELATIVE_ADDR( maxy ),    %xmm1
    cvttss2si    %xmm1,                    %edx        // (int) y
    
    cvtss2sd    %xmm0,                     %xmm0        //  x
    movsd       RELATIVE_ADDR( oneD ),     %xmm2        //    r = 1.0

    cmpl        $0,                        %edx         //    if( y >= 0 )
    jge         1f                                     //        goto 4
    
    // y < 0
    movapd      %xmm0,                     %xmm1        // x
    movapd      %xmm2,                     %xmm0        // 1.0
    divsd       %xmm1,                     %xmm0        // 1.0 / x
    negl        %edx
    
1:  test        $1,                        %edx
    jz          3f                                    // if( |y| is odd )
    movapd      %xmm0,                     %xmm2        //        r = x
    jmp         3f

.align 4
// do{
2:  mulsd       %xmm0,                     %xmm0        //    x *= x
    test        $1,                        %edx
    jz          3f                                    //    if( |y| is odd )    continue
    mulsd       %xmm0,                     %xmm2        //        r *= x

3:  shrl        $1,                        %edx        //    |y| >>= 1
    test        $-1,                       %edx
    jnz         2b                                    //    if( y )        continue
//    }while( |y| )

    // round to float
    xorps       %xmm0,                     %xmm0        // 0
    cvtsd2ss    %xmm2,                     %xmm0        // (float) r

    //exit
    jmp         9b
