/*
 *  expm1f.s
 *
 *      by Ian Ollmann
 *
 *  Copyright (c) 2007, Apple Inc. All Rights Reserved.
 *
 *  Implementation for C99 expm1f function for i386 and x86_64  ABIs.
 */
 
#include <machine/asm.h>
#include "abi.h"

//.const

//minimax polynomial for exp2(x)-1
.align (1<<4)
// 8th order minimax fit of exp2 on [-1.0,1.0].  |error| < 0.402865722354948566583852e-9:
expm1f_c:       .quad 0x40bc03f30399c376,   0x3dbea2a63403aaa8          // c4/c8 = 0.961813690023115610862381719985771e-2 / 0.134107709538786543922336536865157e-5, c0 = 0.278626872016317130037181614004e-10
                .quad 0x408f10e7f73e6d8f,   0x3fe62e42fd0933ee	        // c5/c8 = 0.133318252930790403741964203236548e-2 / 0.134107709538786543922336536865157e-5, c1 = .693147176943623740308984004029708   
                .quad 0x405cb616a9384e69,   0x3fcebfbdfd0f0afa	        // c6/c8 = 0.154016177542147239746127455226575e-3 / 0.134107709538786543922336536865157e-5, c2 = .240226505817268621584559118975830   
                .quad 0x4027173ebd288ba1,   0x3fac6b0a74f15403	        // c7/c8 = 0.154832722143258821052933667742417e-4 / 0.134107709538786543922336536865157e-5, c3 = 0.555041568519883074165425891257052e-1 
                .quad 0x3eb67fe1dc3105ba,   0x3ff0000000000000          // c8 = 0.134107709538786543922336536865157e-5, 1.0


//expm1f_table         
                .quad	0xbfef800000000000,	0xbfe40adf8d149383	//-63/64, expm1(-63/64)
                .quad	0xbfef000000000000,	0xbfe3daaae2395759	//-62/64, expm1(-62/64)
                .quad	0xbfee800000000000,	0xbfe3a9b3e10921cd	//-61/64, expm1(-61/64)
                .quad	0xbfee000000000000,	0xbfe377f77a0fcb45	//-60/64, expm1(-60/64)
                .quad	0xbfed800000000000,	0xbfe345729182bf1f	//-59/64, expm1(-59/64)
                .quad	0xbfed000000000000,	0xbfe31221ff0f3ecc	//-58/64, expm1(-58/64)
                .quad	0xbfec800000000000,	0xbfe2de028da7dc59	//-57/64, expm1(-57/64)
                .quad	0xbfec000000000000,	0xbfe2a910fb51295a	//-56/64, expm1(-56/64)
                .quad	0xbfeb800000000000,	0xbfe27349f8ed96eb	//-55/64, expm1(-55/64)
                .quad	0xbfeb000000000000,	0xbfe23caa2a088391	//-54/64, expm1(-54/64)
                .quad	0xbfea800000000000,	0xbfe2052e24a073a5	//-53/64, expm1(-53/64)
                .quad	0xbfea000000000000,	0xbfe1ccd270f070f9	//-52/64, expm1(-52/64)
                .quad	0xbfe9800000000000,	0xbfe1939389388e3d	//-51/64, expm1(-51/64)
                .quad	0xbfe9000000000000,	0xbfe1596dd9858ab1	//-50/64, expm1(-50/64)
                .quad	0xbfe8800000000000,	0xbfe11e5dbf7792a9	//-49/64, expm1(-49/64)
                .quad	0xbfe8000000000000,	0xbfe0e25f8a081941	//-48/64, expm1(-48/64)
                .quad	0xbfe7800000000000,	0xbfe0a56f794ec7a4	//-47/64, expm1(-47/64)
                .quad	0xbfe7000000000000,	0xbfe06789be457e3b	//-46/64, expm1(-46/64)
                .quad	0xbfe6800000000000,	0xbfe028aa7a8b63f2	//-45/64, expm1(-45/64)
                .quad	0xbfe6000000000000,	0xbfdfd19b804dffbf	//-44/64, expm1(-44/64)
                .quad	0xbfe5800000000000,	0xbfdf4fdf228eb2ad	//-43/64, expm1(-43/64)
                .quad	0xbfe5000000000000,	0xbfdecc17c0083500	//-42/64, expm1(-42/64)
                .quad	0xbfe4800000000000,	0xbfde463d1c396301	//-41/64, expm1(-41/64)
                .quad	0xbfe4000000000000,	0xbfddbe46d96cd831	//-40/64, expm1(-40/64)
                .quad	0xbfe3800000000000,	0xbfdd342c7833133a	//-39/64, expm1(-39/64)
                .quad	0xbfe3000000000000,	0xbfdca7e556da7e48	//-38/64, expm1(-38/64)
                .quad	0xbfe2800000000000,	0xbfdc1968b0e55333	//-37/64, expm1(-37/64)
                .quad	0xbfe2000000000000,	0xbfdb88ad9e7d52ea	//-36/64, expm1(-36/64)
                .quad	0xbfe1800000000000,	0xbfdaf5ab13e5474f	//-35/64, expm1(-35/64)
                .quad	0xbfe1000000000000,	0xbfda6057e0e846a4	//-34/64, expm1(-34/64)
                .quad	0xbfe0800000000000,	0xbfd9c8aab046af7a	//-33/64, expm1(-33/64)
                .quad	0xbfe0000000000000,	0xbfd92e9a0720d3ec	//-32/64, expm1(-32/64)
                .quad	0xbfdf000000000000,	0xbfd8921c445f4add	//-31/64, expm1(-31/64)
                .quad	0xbfde000000000000,	0xbfd7f327a018ddb2	//-30/64, expm1(-30/64)
                .quad	0xbfdd000000000000,	0xbfd751b22af608f0	//-29/64, expm1(-29/64)
                .quad	0xbfdc000000000000,	0xbfd6adb1cd9205ee	//-28/64, expm1(-28/64)
                .quad	0xbfdb000000000000,	0xbfd6071c47d953b2	//-27/64, expm1(-27/64)
                .quad	0xbfda000000000000,	0xbfd55de73065b4df	//-26/64, expm1(-26/64)
                .quad	0xbfd9000000000000,	0xbfd4b207f3d79870	//-25/64, expm1(-25/64)
                .quad	0xbfd8000000000000,	0xbfd40373d42ce2e3	//-24/64, expm1(-24/64)
                .quad	0xbfd7000000000000,	0xbfd3521fe8150d2b	//-23/64, expm1(-23/64)
                .quad	0xbfd6000000000000,	0xbfd29e011a428ec6	//-22/64, expm1(-22/64)
                .quad	0xbfd5000000000000,	0xbfd1e70c28b987f3	//-21/64, expm1(-21/64)
                .quad	0xbfd4000000000000,	0xbfd12d35a41ba104	//-20/64, expm1(-20/64)
                .quad	0xbfd3000000000000,	0xbfd07071eef11388	//-19/64, expm1(-19/64)
                .quad	0xbfd2000000000000,	0xbfcf616a79dda3a8	//-18/64, expm1(-18/64)
                .quad	0xbfd1000000000000,	0xbfcddbe7247382af	//-17/64, expm1(-17/64)
                .quad	0xbfd0000000000000,	0xbfcc5041854df7d4	//-16/64, expm1(-16/64)
                .quad	0xbfce000000000000,	0xbfcabe60e1f21836	//-15/64, expm1(-15/64)
                .quad	0xbfcc000000000000,	0xbfc9262c1c3430a1	//-14/64, expm1(-14/64)
                .quad	0xbfca000000000000,	0xbfc78789b0a5e0c0	//-13/64, expm1(-13/64)
                .quad	0xbfc8000000000000,	0xbfc5e25fb4fde211	//-12/64, expm1(-12/64)
                .quad	0xbfc6000000000000,	0xbfc43693d679612d	//-11/64, expm1(-11/64)
                .quad	0xbfc4000000000000,	0xbfc2840b5836cf67	//-10/64, expm1(-10/64)
                .quad	0xbfc2000000000000,	0xbfc0caab118a1278	//-9/64, expm1(-9/64)
                .quad	0xbfc0000000000000,	0xbfbe14aed893eef4	//-8/64, expm1(-8/64)
                .quad	0xbfbc000000000000,	0xbfba85e8c62d9c13	//-7/64, expm1(-7/64)
                .quad	0xbfb8000000000000,	0xbfb6e8caff341fea	//-6/64, expm1(-6/64)
                .quad	0xbfb4000000000000,	0xbfb33d1bb17df2e7	//-5/64, expm1(-5/64)
                .quad	0xbfb0000000000000,	0xbfaf0540438fd5c3	//-4/64, expm1(-4/64)
                .quad	0xbfa8000000000000,	0xbfa7723950130405	//-3/64, expm1(-3/64)
                .quad	0xbfa0000000000000,	0xbf9f8152aee9450e	//-2/64, expm1(-2/64)
                .quad	0xbf90000000000000,	0xbf8fc055004416db	//-1/64, expm1(-1/64)
expm1f_table:   .quad	0x0000000000000000,	0x0000000000000000	//0/64, expm1(0/64)
                .quad	0x3f90000000000000,	0x3f90202ad5778e46	//1/64, expm1(1/64)
                .quad	0x3fa0000000000000,	0x3fa040ac0224fd93	//2/64, expm1(2/64)
                .quad	0x3fa8000000000000,	0x3fa89246d053d178	//3/64, expm1(3/64)
                .quad	0x3fb0000000000000,	0x3fb082b577d34ed8	//4/64, expm1(4/64)
                .quad	0x3fb4000000000000,	0x3fb4cd4fc989cd64	//5/64, expm1(5/64)
                .quad	0x3fb8000000000000,	0x3fb92937074e0cd7	//6/64, expm1(6/64)
                .quad	0x3fbc000000000000,	0x3fbd96b0eff0e794	//7/64, expm1(7/64)
                .quad	0x3fc0000000000000,	0x3fc10b022db7ae68	//8/64, expm1(8/64)
                .quad	0x3fc2000000000000,	0x3fc353bc9fb00b21	//9/64, expm1(9/64)
                .quad	0x3fc4000000000000,	0x3fc5a5ac59b963cb	//10/64, expm1(10/64)
                .quad	0x3fc6000000000000,	0x3fc800f67b00d7b8	//11/64, expm1(11/64)
                .quad	0x3fc8000000000000,	0x3fca65c0b85ac1a9	//12/64, expm1(12/64)
                .quad	0x3fca000000000000,	0x3fccd4315e9e0833	//13/64, expm1(13/64)
                .quad	0x3fcc000000000000,	0x3fcf4c6f5508ee5d	//14/64, expm1(14/64)
                .quad	0x3fce000000000000,	0x3fd0e7510fd7c564	//15/64, expm1(15/64)
                .quad	0x3fd0000000000000,	0x3fd22d78f0fa061a	//16/64, expm1(16/64)
                .quad	0x3fd1000000000000,	0x3fd378c3b0847980	//17/64, expm1(17/64)
                .quad	0x3fd2000000000000,	0x3fd4c946033eb3de	//18/64, expm1(18/64)
                .quad	0x3fd3000000000000,	0x3fd61f14f169ebc1	//19/64, expm1(19/64)
                .quad	0x3fd4000000000000,	0x3fd77a45d8117fd5	//20/64, expm1(20/64)
                .quad	0x3fd5000000000000,	0x3fd8daee6a60c961	//21/64, expm1(21/64)
                .quad	0x3fd6000000000000,	0x3fda4124b2fe50cb	//22/64, expm1(22/64)
                .quad	0x3fd7000000000000,	0x3fdbacff156c79d7	//23/64, expm1(23/64)
                .quad	0x3fd8000000000000,	0x3fdd1e944f6fbdaa	//24/64, expm1(24/64)
                .quad	0x3fd9000000000000,	0x3fde95fb7a7a88f8	//25/64, expm1(25/64)
                .quad	0x3fda000000000000,	0x3fe009a6068f6a8c	//26/64, expm1(26/64)
                .quad	0x3fdb000000000000,	0x3fe0cb4eee42c98b	//27/64, expm1(27/64)
                .quad	0x3fdc000000000000,	0x3fe190048ef60020	//28/64, expm1(28/64)
                .quad	0x3fdd000000000000,	0x3fe257d334137dff	//29/64, expm1(29/64)
                .quad	0x3fde000000000000,	0x3fe322c75a963b98	//30/64, expm1(30/64)
                .quad	0x3fdf000000000000,	0x3fe3f0edb1d18acd	//31/64, expm1(31/64)
                .quad	0x3fe0000000000000,	0x3fe4c2531c3c0d38	//32/64, expm1(32/64)
                .quad	0x3fe0800000000000,	0x3fe59704b03ddca9	//33/64, expm1(33/64)
                .quad	0x3fe1000000000000,	0x3fe66f0fb901f2bd	//34/64, expm1(34/64)
                .quad	0x3fe1800000000000,	0x3fe74a81b74adcac	//35/64, expm1(35/64)
                .quad	0x3fe2000000000000,	0x3fe82968624ac88d	//36/64, expm1(36/64)
                .quad	0x3fe2800000000000,	0x3fe90bd1a87ef9a1	//37/64, expm1(37/64)
                .quad	0x3fe3000000000000,	0x3fe9f1cbb08eb151	//38/64, expm1(38/64)
                .quad	0x3fe3800000000000,	0x3feadb64da2d9acf	//39/64, expm1(39/64)
                .quad	0x3fe4000000000000,	0x3febc8abbf01c781	//40/64, expm1(40/64)
                .quad	0x3fe4800000000000,	0x3fecb9af338d4a9c	//41/64, expm1(41/64)
                .quad	0x3fe5000000000000,	0x3fedae7e481b8284	//42/64, expm1(42/64)
                .quad	0x3fe5800000000000,	0x3feea72849b21ebd	//43/64, expm1(43/64)
                .quad	0x3fe6000000000000,	0x3fefa3bcc305f191	//44/64, expm1(44/64)
                .quad	0x3fe6800000000000,	0x3ff05225beb9ce55	//45/64, expm1(45/64)
                .quad	0x3fe7000000000000,	0x3ff0d47240fe1412	//46/64, expm1(46/64)
                .quad	0x3fe7800000000000,	0x3ff158cc0d22ca02	//47/64, expm1(47/64)
                .quad	0x3fe8000000000000,	0x3ff1df3b68cfb9ef	//48/64, expm1(48/64)
                .quad	0x3fe8800000000000,	0x3ff267c8bb05d2a3	//49/64, expm1(49/64)
                .quad	0x3fe9000000000000,	0x3ff2f27c8ca598a0	//50/64, expm1(50/64)
                .quad	0x3fe9800000000000,	0x3ff37f5f88f7b4e5	//51/64, expm1(51/64)
                .quad	0x3fea000000000000,	0x3ff40e7a7e37aa30	//52/64, expm1(52/64)
                .quad	0x3fea800000000000,	0x3ff49fd65e20b96f	//53/64, expm1(53/64)
                .quad	0x3feb000000000000,	0x3ff5337c3e7cfe38	//54/64, expm1(54/64)
                .quad	0x3feb800000000000,	0x3ff5c97559b6cc28	//55/64, expm1(55/64)
                .quad	0x3fec000000000000,	0x3ff661cb0f6c564f	//56/64, expm1(56/64)
                .quad	0x3fec800000000000,	0x3ff6fc86e505a9dd	//57/64, expm1(57/64)
                .quad	0x3fed000000000000,	0x3ff799b2864d0569	//58/64, expm1(58/64)
                .quad	0x3fed800000000000,	0x3ff83957c6099668	//59/64, expm1(59/64)
                .quad	0x3fee000000000000,	0x3ff8db809e9ca670	//60/64, expm1(60/64)
                .quad	0x3fee800000000000,	0x3ff9803732a14221	//61/64, expm1(61/64)
                .quad	0x3fef000000000000,	0x3ffa2785cd8e63ad	//62/64, expm1(62/64)
                .quad	0x3fef800000000000,	0x3ffad176e45bab25	//63/64, expm1(63/64)


expm1f_taylor_polynomial:       .double         0.5
                                .double         0.16666666666666666666666666666
                                .double         0.04166666666666666666666666666

.literal8
reciprocalLn2:  .quad   0x3ff71547652b82fe                          // 1.0 / ln(2)
two6:           .quad   0x4050000000000000                          // 0x1.0p6


.text

#if defined( __x86_64__ )
	#define RELATIVE_ADDR( _a)								(_a)( %rip )
	#define RELATIVE_ADDR_B( _a)							(_a)( %rip )
#elif defined( __i386__ )
	#define RELATIVE_ADDR( _a)								(_a)-expm1f_body( CX_P )
	#define RELATIVE_ADDR_B( _a)							(_a)-expm1f_no_fenv_body( CX_P )

//a short routine to get the local address
.align (1<<4)
expm1f_pic:      movl    (%esp),     %ecx                    //copy address of local_addr to %ecx
                ret
#else
	#error arch not supported
#endif

//0x1.0p-24df
#define LOW_CUTOFF  0x33800000

ENTRY( expm1f )
#if defined( __i386__ )
    movl        FRAME_SIZE(STACKP),     %eax
    movss       FRAME_SIZE(STACKP),     %xmm0
#else
    movd        %xmm0,                  %eax
#endif
    
    movl        %eax,                   %ecx
    andl        $0x7fffffff,            %eax
    cvtss2sd    %xmm0,                  %xmm2
    subl        $LOW_CUTOFF,            %eax        
    cmpl        $(0x42b17218-LOW_CUTOFF), %eax            // if( |x| >= 128.0f * ln(2) || |x| <= 0x1.0p-24f || isnan(f) )
    jae         3f
    
1:
//PIC
#if defined( __i386__ )
    calll       expm1f_pic                                    // set %ecx to point to local_addr
expm1f_body:   
#endif
  
    cmpl        $(0x3f800000-LOW_CUTOFF),    %eax
    jl          2f
    
    // |x| >= 1.0f
    movsd       RELATIVE_ADDR( reciprocalLn2 ), %xmm1       // 1 / ln(2)
    mulsd       %xmm2,                      %xmm1           // x / ln(2)
    cvttsd2si   %xmm1,                      %eax            // trunc( x / ln(2) ) 
    lea         RELATIVE_ADDR( expm1f_c),   CX_P
    cvtsi2sd    %eax,                       %xmm3           // trunc( x / ln(2) )
    addl        $1023,                      %eax            // add bias for exponent
    subsd       %xmm3,                      %xmm1           // f = x / ln(2)  - trunc( x / ln(2) ) 
    movd        %eax,                       %xmm7           // 2**i >> 32
    psllq       $52,                        %xmm7           // 2**i

    // c0 + c1x1 + c2x2 + c3x3 + c4x4 + c5x5 + c6x6 + c7x7 + c8x8 
#if defined( __SSE3__ )
    movddup     %xmm1,                  %xmm2       // { x, x }
#else
    movapd      %xmm1,                  %xmm2       // x
    unpcklpd    %xmm2,                  %xmm2       // { x, x }
#endif    
    mulsd       %xmm1,                  %xmm1       // x*x
    movapd      %xmm2,                  %xmm3
    mulpd       48(CX_P),               %xmm2       // { c3x, (c7/c8)x }
    mulpd       16(CX_P),               %xmm3       // { c1x, (c5/c8)x }
#if defined( __SSE3__ )
    movddup     %xmm1,                  %xmm4       // { xx, xx }
#else
    movapd      %xmm1,                  %xmm4       // xx
    unpcklpd    %xmm4,                  %xmm4       // { xx, xx }
#endif
    mulsd       %xmm1,                  %xmm1       // xx*xx
    addpd       32(CX_P),               %xmm2       // { c2 + c3x, (c6/c8) + (c7/c8)x }
    addpd       (CX_P),                 %xmm3       // { c0 + c1x, (c4/c8) + (c5/c8)x }
    mulpd       %xmm4,                  %xmm2       // { c2xx + c3xxx, (c6/c8)xx + (c7/c8)xxx }
    addsd       %xmm1,                  %xmm3       // { c0 + c1x, (c4/c8) + (c5/c8)x + xxxx }
    mulsd       64(CX_P),               %xmm1       // c8 * xxxx
    addpd       %xmm2,                  %xmm3       // { c0 + c1x + c2xx + c3xxx, (c4/c8) + (c5/c8)x + (c6/c8)xx + (c7/c8)xxx + xxxx }
    movhlps     %xmm3,                  %xmm6       // { ?, c0 + c1x + c2xx + c3xxx }
    mulsd       %xmm1,                  %xmm3       // { ..., c8xxxx* ((c4/c8) + (c5/c8)x + (c6/c8)xx + (c7/c8)xxx + xxxx) }
    addsd       %xmm6,                  %xmm3       // c0 + c1x + c2xx + c3xxx + c4xxxx + c5xxxxx + c6xxxxxx + c7xxxxxxx + c8xxxxxxxxx
    
    //result = 2**i * ((2**d-1) + 1) - 1
    //       = 2**i * (2**d-1) + 2**i - 1
    mulsd       %xmm7,                  %xmm3       // 2**i * {c0 + c1x + c2xx + c3xxx + c4xxxx + c5xxxxx + c6xxxxxx + c7xxxxxxx + c8xxxxxxxxx}
    subsd       72(CX_P),               %xmm7       // 2**i - 1.0
    addsd       %xmm7,                  %xmm3       

//  convert to single precision and return
    cvtsd2ss    %xmm3,                  %xmm0
#if defined( __i386__ )
    movss       %xmm0,                  FRAME_SIZE(STACKP)
    flds        FRAME_SIZE(STACKP)
#endif
    ret
    
2:  // |x| < 1.0f
    movsd       RELATIVE_ADDR( two6 ),  %xmm1       // 0x1.0p6
    mulsd       %xmm2,                  %xmm1       //  x * 0x1.0p6
    cvttsd2si   %xmm1,                  %eax        // i = trunc( x * 0x1.0p6 )
#if defined( __x86_64__ )
    cdqe                                            // sign extend eax
#endif
    shl         $4,                     AX_P      // sizeof( double[2] ) * trunc( x * 0x1.0p6 )
    lea         RELATIVE_ADDR( expm1f_table ), DX_P
    subsd       (DX_P, AX_P, 1),        %xmm2       // x -= i/64
    movsd       8(DX_P, AX_P, 1),       %xmm7       // e**i - 1
    
    //calculate e**d-1 using minimax polynomial
    lea         RELATIVE_ADDR( expm1f_taylor_polynomial), CX_P
    
    movsd       (2*8)(CX_P),            %xmm3       // 1/24
    mulsd       %xmm2,                  %xmm3       // d/24
    movapd      %xmm2,                  %xmm4       // d
    addsd       (1*8)(CX_P),            %xmm3       // 1/6 + d/24
    mulsd       %xmm4,                  %xmm4       // d * d
    mulsd       %xmm2,                  %xmm3       // d/6 + dd/24
    addsd       (CX_P),                 %xmm3       // 1/2 + d/6 + dd/24
    mulsd       %xmm4,                  %xmm3       // dd/2 + ddd/6 + dddd/24
    addsd       %xmm2,                  %xmm3       // d + dd/2 + ddd/6 + dddd/24
    
    //we have to do a little reduction here, otherwise we end up doing a 6th order Taylor series and it still is not very good:
    // e**x-1   = e**(i+d) -1         c is a table entry chosen so |b| < 1/32
    //          = [(e**(i)-1)+1][(e**(d)-1)+1] -1  = (y+1)(z+1)-1               y = e**(i)-1,  z = e**(d)-1
    //          = yz + y + z + 1 -1 = y + z + yz    
    movapd      %xmm7,                  %xmm4       // y
    mulsd       %xmm3,                  %xmm4       // y*z
    addsd       %xmm3,                  %xmm4       // yz + z
    addsd       %xmm7,                  %xmm4       // (yz + z) + y
    
//  convert to single precision and return
    cvtsd2ss    %xmm4,                  %xmm0
#if defined( __i386__ )
    movss       %xmm0,                  FRAME_SIZE(STACKP)
    flds        FRAME_SIZE(STACKP)
#endif
    ret
    
3: // |x| >= 128.0f * ln(2) || |x| <= LOW_CUTOFF || isnan(f)
    jl          4f
    
    // |x| >= 128.0f * ln(2) || isnan(f)

    //handle inf and NaN arguments
    cmpl    $(0x7f800000-LOW_CUTOFF),   %eax
    je          5f          // |x| == inf
    jg          6f          // isnan( x )
    
    //bounce denormal results back to the main code path
    // if( x > -150.0f * ln(2)
    cmpl    $0xc2cff1b4,                %ecx        //if( -128.0f*ln(2) >= x > -150.0f*ln(2) )
    jle          1b                                  //     go back to 1
    
    // |x| >= 128.0f * ln(2)
    xorps       %xmm1,                  %xmm1       // 0.0f
    movl        $0x7f7fffff,            %ecx
    movl        $0x21800000,            %eax        // 0x1.0p-60
    cmpltss     %xmm1,                  %xmm0       // x < 0 
    movd        %ecx,                   %xmm3
    movd        %eax,                   %xmm4
    movl        $0x3f800000,            %eax
    andps       %xmm0,                  %xmm4       // x < 0 ? 0x1.0p-60f : 0
    andnps      %xmm3,                  %xmm0       // x < 0 ? 0 : MAX_FLOAT
    orps        %xmm4,                  %xmm0       // x < 0 ? 0x1.0p-60f : MAX_FLT
    mulss       %xmm0,                  %xmm0       // x < 0 ? 0x1.0p-120f : Inf, overflow 
    movd        %eax,                   %xmm1
    subss       %xmm1,                  %xmm0       // x < 0 ? 0x1.0p-120f - 1.0f (inexact, round to correct result) : Inf, overflow
    
#if defined( __i386__ )
    movss       %xmm0,                  FRAME_SIZE( STACKP )
    flds        FRAME_SIZE(STACKP)
#endif
    ret
    
4: // |x| <= LOW_CUTOFF
    movl        $0x3cb00000,            %eax        // 0x1.0p-52 >> 32
    movl        $0x3ff00000,            %ecx        // 1.0 >> 32
    movd        %eax,                   %xmm1
    movd        %ecx,                   %xmm3
    psllq       $32,                    %xmm1       // 0x1.0p-52
    psllq       $32,                    %xmm3       // 1.0
    addsd       %xmm3,                  %xmm1       // 1.0 + DBL_EPSILON
    mulsd       %xmm1,                  %xmm2       // nudge towards correctly rounded
    cvtsd2ss    %xmm2,                  %xmm0       // round correctly, set inexact
#if defined( __i386__ )
    movss       %xmm0,                  FRAME_SIZE( STACKP )
    flds        FRAME_SIZE(STACKP)
#endif
    ret

5:  // |x| == inf, return -1 or Inf
    movl        $0x3f800000,            %eax
    movl        $0x7f800000,            %ecx
    movd        %eax,                   %xmm1
    movd        %ecx,                   %xmm3
    cmpeqss     %xmm0,                  %xmm3       // x == inf
    andps       %xmm3,                  %xmm0       // x == inf ? inf : 0
    subss       %xmm1,                  %xmm0       // x == inf ? inf : -1.0
#if defined( __i386__ )
    movss       %xmm0,                  FRAME_SIZE( STACKP )
    flds        FRAME_SIZE(STACKP)
#endif
    ret

6:  // |x|| == NaN
    addss       %xmm0,                  %xmm0
#if defined( __i386__ )
    movss       %xmm0,                  FRAME_SIZE( STACKP )
    flds        FRAME_SIZE(STACKP)
#endif
    ret


