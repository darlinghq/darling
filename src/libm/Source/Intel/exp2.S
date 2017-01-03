/****************************************************************************

	exp2.s
	
	by Stephen Canon
	
	Copyright © 2007, Apple Inc.  All Rights Reserved.
	
	Implementation of the C99 exp2 function for the MacOS X i386 and x86_64 ABIs.	
	
****************************************************************************/
#include <machine/asm.h>
#include "abi.h"

/****************************************************************************

	Tables and Constants

****************************************************************************/
.const
.align 4


			// Coefficients for the 4th order minimax polynomial
			// approximation of exp2( ) on the interval [-0x1p-27, 0x1p-7]
			
			// p(x) = ax(x^2 + bx + c)(x^2 + dx + e)
			
			.quad 0x0000000000000000
			.double 0.133636821875765095445302638781823e-2 // a
			.double 20.9994886368921956041911010330331 // c
			.double 24.6996388387715486518175431533940 // e
			.double 7.73568837260218089745241122384476 // b
			.double -.538488999896042287022709624001146 // d
			
			// table of values of x for which exp2(x) is very precise in
			// double precision constructed by J. Kidder.
			//			  x				    exp2(x)
exp2table:	.quad 0xbfe000000005f3f1, 0x3fe6a09e667c50df // x = -0x1.000000005f3f1p-1, exp2(x) = 0x1.6a09e667c50dfp-1 to 76 bits
			.quad 0xbfdf800000504e67, 0x3fe6c01274f81141 // x = -0x1.f800000504e67p-2, exp2(x) = 0x1.6c01274f81141p-1 to 75 bits
			.quad 0xbfdf000000413114, 0x3fe6dfb23c54f3bf // x = -0x1.f000000413114p-2, exp2(x) = 0x1.6dfb23c54f3bfp-1 to 75 bits
			.quad 0xbfde800000026750, 0x3fe6ff7df950fb41 // x = -0x1.e800000026750p-2, exp2(x) = 0x1.6ff7df950fb41p-1 to 75 bits
			.quad 0xbfde00000001f188, 0x3fe71f75e8ebe2db // x = -0x1.e00000001f188p-2, exp2(x) = 0x1.71f75e8ebe2dbp-1 to 75 bits
			.quad 0xbfdd800000060771, 0x3fe73f9a48a3fcd4 // x = -0x1.d800000060771p-2, exp2(x) = 0x1.73f9a48a3fcd4p-1 to 75 bits
			.quad 0xbfdd000000027151, 0x3fe75feb5641c97b // x = -0x1.d000000027151p-2, exp2(x) = 0x1.75feb5641c97bp-1 to 75 bits
			.quad 0xbfdc800000133b3c, 0x3fe780694fd97820 // x = -0x1.c800000133b3cp-2, exp2(x) = 0x1.780694fd97820p-1 to 77 bits
			.quad 0xbfdc000000006742, 0x3fe7a11473eae71a // x = -0x1.c000000006742p-2, exp2(x) = 0x1.7a11473eae71ap-1 to 76 bits
			.quad 0xbfdb8000000026c4, 0x3fe7c1ed0130b739 // x = -0x1.b8000000026c4p-2, exp2(x) = 0x1.7c1ed0130b739p-1 to 75 bits
			.quad 0xbfdb00000010b0f8, 0x3fe7e2f336cafcf6 // x = -0x1.b00000010b0f8p-2, exp2(x) = 0x1.7e2f336cafcf6p-1 to 75 bits
			.quad 0xbfda800000006fca, 0x3fe80427543dfcfe // x = -0x1.a800000006fcap-2, exp2(x) = 0x1.80427543dfcfep-1 to 75 bits
			.quad 0xbfda0000000a60b7, 0x3fe82589994a174b // x = -0x1.a0000000a60b7p-2, exp2(x) = 0x1.82589994a174bp-1 to 75 bits
			.quad 0xbfd98000000f3180, 0x3fe8471a461fc8fa // x = -0x1.98000000f3180p-2, exp2(x) = 0x1.8471a461fc8fap-1 to 82 bits
			.quad 0xbfd9000000065897, 0x3fe868d99b42e570 // x = -0x1.9000000065897p-2, exp2(x) = 0x1.868d99b42e570p-1 to 76 bits
			.quad 0xbfd8800000057289, 0x3fe88ac7d988f3ee // x = -0x1.8800000057289p-2, exp2(x) = 0x1.88ac7d988f3eep-1 to 76 bits
			.quad 0xbfd800000005c28b, 0x3fe8ace5422916cb // x = -0x1.800000005c28bp-2, exp2(x) = 0x1.8ace5422916cbp-1 to 75 bits
			.quad 0xbfd780000060838c, 0x3fe8cf32169b55bb // x = -0x1.780000060838cp-2, exp2(x) = 0x1.8cf32169b55bbp-1 to 75 bits
			.quad 0xbfd7000000160f6a, 0x3fe8f1ae990f8189 // x = -0x1.7000000160f6ap-2, exp2(x) = 0x1.8f1ae990f8189p-1 to 75 bits
			.quad 0xbfd6800000015814, 0x3fe9145b0b91a250 // x = -0x1.6800000015814p-2, exp2(x) = 0x1.9145b0b91a250p-1 to 79 bits
			.quad 0xbfd600000005b1dc, 0x3fe93737b0cc37c2 // x = -0x1.600000005b1dcp-2, exp2(x) = 0x1.93737b0cc37c2p-1 to 75 bits
			.quad 0xbfd58000000ced09, 0x3fe95a44cbc4c577 // x = -0x1.58000000ced09p-2, exp2(x) = 0x1.95a44cbc4c577p-1 to 75 bits
			.quad 0xbfd500000004055e, 0x3fe97d829fdd3222 // x = -0x1.500000004055ep-2, exp2(x) = 0x1.97d829fdd3222p-1 to 76 bits
			.quad 0xbfd4800000044514, 0x3fe9a0f170c8d852 // x = -0x1.4800000044514p-2, exp2(x) = 0x1.9a0f170c8d852p-1 to 75 bits
			.quad 0xbfd400000062eb27, 0x3fe9c49182885576 // x = -0x1.400000062eb27p-2, exp2(x) = 0x1.9c49182885576p-1 to 76 bits
			.quad 0xbfd380000025f5a3, 0x3fe9e86319d87c72 // x = -0x1.380000025f5a3p-2, exp2(x) = 0x1.9e86319d87c72p-1 to 75 bits
			.quad 0xbfd300000026d9fb, 0x3fea0c667b52ef7a // x = -0x1.300000026d9fbp-2, exp2(x) = 0x1.a0c667b52ef7ap-1 to 75 bits
			.quad 0xbfd28000000ef1d6, 0x3fea309bec45f003 // x = -0x1.28000000ef1d6p-2, exp2(x) = 0x1.a309bec45f003p-1 to 75 bits
			.quad 0xbfd200000028a35e, 0x3fea5503b2328e73 // x = -0x1.200000028a35ep-2, exp2(x) = 0x1.a5503b2328e73p-1 to 75 bits
			.quad 0xbfd18000003892f0, 0x3fea799e13207a90 // x = -0x1.18000003892f0p-2, exp2(x) = 0x1.a799e13207a90p-1 to 77 bits
			.quad 0xbfd100000036ebd4, 0x3fea9e6b556a2865 // x = -0x1.100000036ebd4p-2, exp2(x) = 0x1.a9e6b556a2865p-1 to 75 bits
			.quad 0xbfd08000001df4d6, 0x3feac36bbfcb4499 // x = -0x1.08000001df4d6p-2, exp2(x) = 0x1.ac36bbfcb4499p-1 to 75 bits
			.quad 0xbfd00000007109bc, 0x3feae89f9939e248 // x = -0x1.00000007109bcp-2, exp2(x) = 0x1.ae89f9939e248p-1 to 76 bits
			.quad 0xbfcf0000000d0ce0, 0x3feb0e07298bccef // x = -0x1.f0000000d0ce0p-3, exp2(x) = 0x1.b0e07298bccefp-1 to 76 bits
			.quad 0xbfce000000038571, 0x3feb33a2b84e9132 // x = -0x1.e000000038571p-3, exp2(x) = 0x1.b33a2b84e9132p-1 to 75 bits
			.quad 0xbfcd0000003291c8, 0x3feb59728ddddbeb // x = -0x1.d0000003291c8p-3, exp2(x) = 0x1.b59728ddddbebp-1 to 77 bits
			.quad 0xbfcc00000009a474, 0x3feb7f76f2f9eeb5 // x = -0x1.c00000009a474p-3, exp2(x) = 0x1.b7f76f2f9eeb5p-1 to 77 bits
			.quad 0xbfcb000000041435, 0x3feba5b030a069f4 // x = -0x1.b000000041435p-3, exp2(x) = 0x1.ba5b030a069f4p-1 to 77 bits
			.quad 0xbfca0000001c5bde, 0x3febcc1e90477d01 // x = -0x1.a0000001c5bdep-3, exp2(x) = 0x1.bcc1e90477d01p-1 to 76 bits
			.quad 0xbfc90000000fd3f7, 0x3febf2c25bd4b8c9 // x = -0x1.90000000fd3f7p-3, exp2(x) = 0x1.bf2c25bd4b8c9p-1 to 76 bits
			.quad 0xbfc800000042ee2b, 0x3fec199bdd7b2358 // x = -0x1.800000042ee2bp-3, exp2(x) = 0x1.c199bdd7b2358p-1 to 76 bits
			.quad 0xbfc700000012ef60, 0x3fec40ab5ffceadc // x = -0x1.700000012ef60p-3, exp2(x) = 0x1.c40ab5ffceadcp-1 to 78 bits
			.quad 0xbfc600000017f9f5, 0x3fec67f12e542120 // x = -0x1.600000017f9f5p-3, exp2(x) = 0x1.c67f12e542120p-1 to 75 bits
			.quad 0xbfc50000001c6b03, 0x3fec8f6d9402828e // x = -0x1.50000001c6b03p-3, exp2(x) = 0x1.c8f6d9402828ep-1 to 75 bits
			.quad 0xbfc40000004dc028, 0x3fecb720dce37952 // x = -0x1.40000004dc028p-3, exp2(x) = 0x1.cb720dce37952p-1 to 75 bits
			.quad 0xbfc300000002c957, 0x3fecdf0b555d5473 // x = -0x1.300000002c957p-3, exp2(x) = 0x1.cdf0b555d5473p-1 to 80 bits
			.quad 0xbfc20000000307a2, 0x3fed072d4a070f8f // x = -0x1.20000000307a2p-3, exp2(x) = 0x1.d072d4a070f8fp-1 to 77 bits
			.quad 0xbfc100000008225f, 0x3fed2f87080c40d5 // x = -0x1.100000008225fp-3, exp2(x) = 0x1.d2f87080c40d5p-1 to 78 bits
			.quad 0xbfc00000000d35cc, 0x3fed5818dcf98b25 // x = -0x1.00000000d35ccp-3, exp2(x) = 0x1.d5818dcf98b25p-1 to 75 bits
			.quad 0xbfbe00000000a23c, 0x3fed80e316c976a2 // x = -0x1.e00000000a23cp-4, exp2(x) = 0x1.d80e316c976a2p-1 to 75 bits
			.quad 0xbfbc0000001a43df, 0x3feda9e603d9167a // x = -0x1.c0000001a43dfp-4, exp2(x) = 0x1.da9e603d9167ap-1 to 75 bits
			.quad 0xbfba0000000a59b6, 0x3fedd321f300de67 // x = -0x1.a0000000a59b6p-4, exp2(x) = 0x1.dd321f300de67p-1 to 78 bits
			.quad 0xbfb800000004084f, 0x3fedfc97337b478e // x = -0x1.800000004084fp-4, exp2(x) = 0x1.dfc97337b478ep-1 to 75 bits
			.quad 0xbfb60000000cc2e4, 0x3fee264614f49679 // x = -0x1.60000000cc2e4p-4, exp2(x) = 0x1.e264614f49679p-1 to 75 bits
			.quad 0xbfb40000002a7000, 0x3fee502ee787c449 // x = -0x1.40000002a7000p-4, exp2(x) = 0x1.e502ee787c449p-1 to 75 bits
			.quad 0xbfb20000001b557e, 0x3fee7a51fbc50b11 // x = -0x1.20000001b557ep-4, exp2(x) = 0x1.e7a51fbc50b11p-1 to 79 bits
			.quad 0xbfb0000000067db9, 0x3feea4afa2a406fa // x = -0x1.0000000067db9p-4, exp2(x) = 0x1.ea4afa2a406fap-1 to 75 bits
			.quad 0xbfac0000004cac7c, 0x3feecf482d8b353b // x = -0x1.c0000004cac7cp-5, exp2(x) = 0x1.ecf482d8b353bp-1 to 79 bits
			.quad 0xbfa80000002255b2, 0x3feefa1bee5fe98b // x = -0x1.80000002255b2p-5, exp2(x) = 0x1.efa1bee5fe98bp-1 to 75 bits
			.quad 0xbfa40000000fbb0b, 0x3fef252b376b10cb // x = -0x1.40000000fbb0bp-5, exp2(x) = 0x1.f252b376b10cbp-1 to 75 bits
			.quad 0xbfa000000013426b, 0x3fef50765b6d743c // x = -0x1.000000013426bp-5, exp2(x) = 0x1.f50765b6d743cp-1 to 75 bits
			.quad 0xbf980000001b1b4b, 0x3fef7bfdad9c2a30 // x = -0x1.80000001b1b4bp-6, exp2(x) = 0x1.f7bfdad9c2a30p-1 to 75 bits
			.quad 0xbf90000000108597, 0x3fefa7c1819e3637 // x = -0x1.0000000108597p-6, exp2(x) = 0x1.fa7c1819e3637p-1 to 75 bits
			.quad 0xbf800000001446bb, 0x3fefd3c22b8f3a07 // x = -0x1.00000001446bbp-7, exp2(x) = 0x1.fd3c22b8f3a07p-1 to 78 bits
			.quad 0x0000000000000000, 0x3ff0000000000000 // x = 0x0.0000000000000p+0, exp2(x) = 0x1.0000000000000p+0 exactly
			.quad 0x3f80000000046787, 0x3ff0163da9fb3959 // x = 0x1.0000000046787p-7, exp2(x) = 0x1.0163da9fb3959p+0 to 75 bits
			.quad 0x3f90000000413bdf, 0x3ff02c9a3e783737 // x = 0x1.0000000413bdfp-6, exp2(x) = 0x1.02c9a3e783737p+0 to 75 bits
			.quad 0x3f9800000009e3ad, 0x3ff04315e86e9b63 // x = 0x1.800000009e3adp-6, exp2(x) = 0x1.04315e86e9b63p+0 to 75 bits
			.quad 0x3fa00000000c4c0e, 0x3ff059b0d315cb23 // x = 0x1.00000000c4c0ep-5, exp2(x) = 0x1.059b0d315cb23p+0 to 77 bits
			.quad 0x3fa40000001dce0d, 0x3ff0706b29dea0ad // x = 0x1.40000001dce0dp-5, exp2(x) = 0x1.0706b29dea0adp+0 to 76 bits
			.quad 0x3fa80000002def51, 0x3ff087451876a2e9 // x = 0x1.80000002def51p-5, exp2(x) = 0x1.087451876a2e9p+0 to 79 bits
			.quad 0x3fac000000110f6a, 0x3ff09e3ecac755c5 // x = 0x1.c000000110f6ap-5, exp2(x) = 0x1.09e3ecac755c5p+0 to 75 bits
			.quad 0x3fb0000000042de0, 0x3ff0b5586cf9b976 // x = 0x1.0000000042de0p-4, exp2(x) = 0x1.0b5586cf9b976p+0 to 75 bits
			.quad 0x3fb20000000af8a9, 0x3ff0cc922b72c7b8 // x = 0x1.20000000af8a9p-4, exp2(x) = 0x1.0cc922b72c7b8p+0 to 75 bits
			.quad 0x3fb40000000120ca, 0x3ff0e3ec32d3ded7 // x = 0x1.40000000120cap-4, exp2(x) = 0x1.0e3ec32d3ded7p+0 to 77 bits
			.quad 0x3fb600000008f69e, 0x3ff0fb66afff3c9d // x = 0x1.600000008f69ep-4, exp2(x) = 0x1.0fb66afff3c9dp+0 to 75 bits
			.quad 0x3fb8000000305124, 0x3ff11301d0149725 // x = 0x1.8000000305124p-4, exp2(x) = 0x1.11301d0149725p+0 to 76 bits
			.quad 0x3fba0000000021c9, 0x3ff12abdc06c335e // x = 0x1.a0000000021c9p-4, exp2(x) = 0x1.12abdc06c335ep+0 to 75 bits
			.quad 0x3fbc0000000b765e, 0x3ff1429aaea9b702 // x = 0x1.c0000000b765ep-4, exp2(x) = 0x1.1429aaea9b702p+0 to 75 bits
			.quad 0x3fbe0000000561d9, 0x3ff15a98c8a5cf0f // x = 0x1.e0000000561d9p-4, exp2(x) = 0x1.15a98c8a5cf0fp+0 to 75 bits
			.quad 0x3fc00000000d9993, 0x3ff172b83c7e9a70 // x = 0x1.00000000d9993p-3, exp2(x) = 0x1.172b83c7e9a70p+0 to 76 bits
			.quad 0x3fc10000001aa7db, 0x3ff18af9388f162b // x = 0x1.10000001aa7dbp-3, exp2(x) = 0x1.18af9388f162bp+0 to 76 bits
			.quad 0x3fc2000000023143, 0x3ff1a35beb700111 // x = 0x1.2000000023143p-3, exp2(x) = 0x1.1a35beb700111p+0 to 76 bits
			.quad 0x3fc300000035902b, 0x3ff1bbe0840981a5 // x = 0x1.300000035902bp-3, exp2(x) = 0x1.1bbe0840981a5p+0 to 78 bits
			.quad 0x3fc40000000c261e, 0x3ff1d4873169e5f6 // x = 0x1.40000000c261ep-3, exp2(x) = 0x1.1d4873169e5f6p+0 to 76 bits
			.quad 0x3fc50000000f8bff, 0x3ff1ed5022fe5b7c // x = 0x1.50000000f8bffp-3, exp2(x) = 0x1.1ed5022fe5b7cp+0 to 75 bits
			.quad 0x3fc600000019e18b, 0x3ff2063b88651387 // x = 0x1.600000019e18bp-3, exp2(x) = 0x1.2063b88651387p+0 to 78 bits
			.quad 0x3fc700000010f9d2, 0x3ff21f49917f8711 // x = 0x1.700000010f9d2p-3, exp2(x) = 0x1.21f49917f8711p+0 to 78 bits
			.quad 0x3fc800000021f3d9, 0x3ff2387a6e78bbd5 // x = 0x1.800000021f3d9p-3, exp2(x) = 0x1.2387a6e78bbd5p+0 to 76 bits
			.quad 0x3fc900000007aca6, 0x3ff251ce4fb36926 // x = 0x1.900000007aca6p-3, exp2(x) = 0x1.251ce4fb36926p+0 to 77 bits
			.quad 0x3fca0000000834d3, 0x3ff26b4565e34e68 // x = 0x1.a0000000834d3p-3, exp2(x) = 0x1.26b4565e34e68p+0 to 76 bits
			.quad 0x3fcb0000000f383c, 0x3ff284dfe1f6ea3c // x = 0x1.b0000000f383cp-3, exp2(x) = 0x1.284dfe1f6ea3cp+0 to 75 bits
			.quad 0x3fcc000000295470, 0x3ff29e9df52409b2 // x = 0x1.c000000295470p-3, exp2(x) = 0x1.29e9df52409b2p+0 to 75 bits
			.quad 0x3fcd000000aa2014, 0x3ff2b87fd0ec18b9 // x = 0x1.d000000aa2014p-3, exp2(x) = 0x1.2b87fd0ec18b9p+0 to 75 bits
			.quad 0x3fce00000089ee06, 0x3ff2d285a6f21217 // x = 0x1.e00000089ee06p-3, exp2(x) = 0x1.2d285a6f21217p+0 to 77 bits
			.quad 0x3fcf00000011e56f, 0x3ff2ecafa94004d8 // x = 0x1.f00000011e56fp-3, exp2(x) = 0x1.2ecafa94004d8p+0 to 76 bits
			.quad 0x3fd00000002450b9, 0x3ff306fe0a3932e5 // x = 0x1.00000002450b9p-2, exp2(x) = 0x1.306fe0a3932e5p+0 to 78 bits
			.quad 0x3fd0800000018109, 0x3ff32170fc4d27f8 // x = 0x1.0800000018109p-2, exp2(x) = 0x1.32170fc4d27f8p+0 to 75 bits
			.quad 0x3fd10000000f3232, 0x3ff33c08b2674165 // x = 0x1.10000000f3232p-2, exp2(x) = 0x1.33c08b2674165p+0 to 75 bits
			.quad 0x3fd18000000237f0, 0x3ff356c55f9316e5 // x = 0x1.18000000237f0p-2, exp2(x) = 0x1.356c55f9316e5p+0 to 76 bits
			.quad 0x3fd20000000199c8, 0x3ff371a7373b0016 // x = 0x1.20000000199c8p-2, exp2(x) = 0x1.371a7373b0016p+0 to 75 bits
			.quad 0x3fd28000000b48f9, 0x3ff38cae6d083c14 // x = 0x1.28000000b48f9p-2, exp2(x) = 0x1.38cae6d083c14p+0 to 75 bits
			.quad 0x3fd30000004c6957, 0x3ff3a7db34f5e42c // x = 0x1.30000004c6957p-2, exp2(x) = 0x1.3a7db34f5e42cp+0 to 77 bits
			.quad 0x3fd38000004e208e, 0x3ff3c32dc32461b8 // x = 0x1.38000004e208ep-2, exp2(x) = 0x1.3c32dc32461b8p+0 to 75 bits
			.quad 0x3fd4000000013df2, 0x3ff3dea64c12788e // x = 0x1.4000000013df2p-2, exp2(x) = 0x1.3dea64c12788ep+0 to 75 bits
			.quad 0x3fd480000015af05, 0x3ff3fa4504b13129 // x = 0x1.480000015af05p-2, exp2(x) = 0x1.3fa4504b13129p+0 to 78 bits
			.quad 0x3fd500000009a838, 0x3ff4160a21f947f9 // x = 0x1.500000009a838p-2, exp2(x) = 0x1.4160a21f947f9p+0 to 75 bits
			.quad 0x3fd5800000234f8c, 0x3ff431f5d95861bd // x = 0x1.5800000234f8cp-2, exp2(x) = 0x1.431f5d95861bdp+0 to 77 bits
			.quad 0x3fd600000003a7aa, 0x3ff44e08606256f0 // x = 0x1.600000003a7aap-2, exp2(x) = 0x1.44e08606256f0p+0 to 78 bits
			.quad 0x3fd68000000e661d, 0x3ff46a41ed202f5b // x = 0x1.68000000e661dp-2, exp2(x) = 0x1.46a41ed202f5bp+0 to 75 bits
			.quad 0x3fd700000047e5a0, 0x3ff486a2b5d13877 // x = 0x1.700000047e5a0p-2, exp2(x) = 0x1.486a2b5d13877p+0 to 75 bits
			.quad 0x3fd780000013fc02, 0x3ff4a32af0dc4b5b // x = 0x1.780000013fc02p-2, exp2(x) = 0x1.4a32af0dc4b5bp+0 to 75 bits
			.quad 0x3fd800000018514b, 0x3ff4bfdad53ba122 // x = 0x1.800000018514bp-2, exp2(x) = 0x1.4bfdad53ba122p+0 to 75 bits
			.quad 0x3fd88000004b65dc, 0x3ff4dcb29a0ee638 // x = 0x1.88000004b65dcp-2, exp2(x) = 0x1.4dcb29a0ee638p+0 to 76 bits
			.quad 0x3fd90000002a7909, 0x3ff4f9b276a6d2b4 // x = 0x1.90000002a7909p-2, exp2(x) = 0x1.4f9b276a6d2b4p+0 to 76 bits
			.quad 0x3fd98000000e9ddf, 0x3ff516daa2d2bcec // x = 0x1.98000000e9ddfp-2, exp2(x) = 0x1.516daa2d2bcecp+0 to 77 bits
			.quad 0x3fda0000001165b7, 0x3ff5342b56a14e49 // x = 0x1.a0000001165b7p-2, exp2(x) = 0x1.5342b56a14e49p+0 to 76 bits
			.quad 0x3fda800000347522, 0x3ff551a4ca69aec0 // x = 0x1.a800000347522p-2, exp2(x) = 0x1.551a4ca69aec0p+0 to 76 bits
			.quad 0x3fdb000000163445, 0x3ff56f4736ba4f70 // x = 0x1.b000000163445p-2, exp2(x) = 0x1.56f4736ba4f70p+0 to 75 bits
			.quad 0x3fdb800000173fe4, 0x3ff58d12d49d3534 // x = 0x1.b800000173fe4p-2, exp2(x) = 0x1.58d12d49d3534p+0 to 75 bits
			.quad 0x3fdc0000000bbaca, 0x3ff5ab07dd4b14d7 // x = 0x1.c0000000bbacap-2, exp2(x) = 0x1.5ab07dd4b14d7p+0 to 76 bits
			.quad 0x3fdc80000016a2c1, 0x3ff5c9268a5e9dfb // x = 0x1.c80000016a2c1p-2, exp2(x) = 0x1.5c9268a5e9dfbp+0 to 75 bits
			.quad 0x3fdd0000004564c2, 0x3ff5e76f15bd979e // x = 0x1.d0000004564c2p-2, exp2(x) = 0x1.5e76f15bd979ep+0 to 75 bits
			.quad 0x3fdd8000001d2bb6, 0x3ff605e1b97dd138 // x = 0x1.d8000001d2bb6p-2, exp2(x) = 0x1.605e1b97dd138p+0 to 75 bits
			.quad 0x3fde000000122703, 0x3ff6247eb03eafef // x = 0x1.e000000122703p-2, exp2(x) = 0x1.6247eb03eafefp+0 to 76 bits
			.quad 0x3fde8000001fd876, 0x3ff6434634d470cf // x = 0x1.e8000001fd876p-2, exp2(x) = 0x1.6434634d470cfp+0 to 76 bits
			.quad 0x3fdf0000004a6dab, 0x3ff6623882672d39 // x = 0x1.f0000004a6dabp-2, exp2(x) = 0x1.6623882672d39p+0 to 76 bits
			.quad 0x3fdf80000033c91f, 0x3ff68155d45948c1 // x = 0x1.f80000033c91fp-2, exp2(x) = 0x1.68155d45948c1p+0 to 75 bits
			.quad 0x3fe000000006d6f2, 0x3ff6a09e668295fd // x = 0x1.000000006d6f2p-1, exp2(x) = 0x1.6a09e668295fdp+0 to 76 bits
			// Other useful constants

small_poly:	.quad 0x3ff0000000000000
			.double .693147180559945309420618787399210
			.double .240226506959089504788316195798243
			.double 0.555041086648186946112103135980407e-1
			.double 0.961813204561913206522420450734562e-2
			.double 0.133335622192926517572920729812647e-2

					
.literal8
.align 3
one_p7:		.quad 0x4060000000000000
tiny_val:	.quad 0x0010000000000001
huge_val:	.quad 0x7fefffffffffffff
neg_inf:	.quad 0xfff0000000000000

#if defined( __x86_64__ )
abs_mask:	.quad 0x7fffffffffffffff
small_cut:	.quad 0x3f70000000000000
large_cut:	.quad 0x011ff00000000000
tiny_cut:	.quad 0xfd10000000000000
huge_cut:	.quad 0x0120cc0000000000
overflow:	.quad 0x0120000000000000
#endif

/****************************************************************************

	PIC code

****************************************************************************/
.text
#if defined(__x86_64__)
	#define REL_ADDR(_a)	(_a)(%rip)
#else
	#define REL_ADDR(_a)	(_a)-0b(%ecx)
#endif




/****************************************************************************

	Entry point for exp2( )

****************************************************************************/
ENTRY( exp2 )
#if defined( __x86_64__ )
	movd		%xmm0,				%rax
	movd		%xmm0,				%rdx
	and		REL_ADDR(abs_mask),		%rax
	sar			$63,				%rdx
	sub		REL_ADDR(small_cut),	%rax
	add			$64,				%rdx
	cmp		REL_ADDR(large_cut),	%rax
#else
	movl	4+FRAME_SIZE(STACKP),	%eax
	movl	4+FRAME_SIZE(STACKP),	%edx
	movsd	  FRAME_SIZE(STACKP),	%xmm0
	call		0f
0:	pop			%ecx
	and			$0x7fffffff,		%eax
	sar			$31,				%edx
	sub			$0x3f700000,		%eax
	add			$64,				%edx
	cmp			$0x011ff000,		%eax
#endif
	jae			3f

1:											// Numbers between 1022 and 1024 are
											// reflected back to here
	movapd		%xmm0,				%xmm1	
	mulsd	REL_ADDR(one_p7),		%xmm0
	cvttsd2si	%xmm0,				AX_P
	add			DX_P,				AX_P
	mov			AX_P,				DX_P
	sar			$7,					AX_P	// n = round(x)
		
	cvtsi2sd	AX_P,				%xmm0	// (double)n
	ucomisd		%xmm1,				%xmm0	// if x == (double)n
	je			2f							//   take early out because 2^x is exact.
	
	subsd		%xmm0,				%xmm1	// f = x - n
	and			$0x7f,				DX_P	// 7 bits of f for lookup
	sal			$4,					DX_P
	lea		REL_ADDR(exp2table),	CX_P
	
	subsd		(DX_P, CX_P, 1),	%xmm1	// flo = (x - n) - fhi
	movsd		8(DX_P, CX_P, 1),	%xmm2	// 2^fhi
	
	// Factored Minimax polynomial in flo:
	//		 a flo (flo^2 + b flo + c)(flo^2 + d flo + e) = (2^flo - 1) ± 2 ulp
	
	movlhps		%xmm1,				%xmm1	// flo                , flo
	movapd		%xmm1,				%xmm3
	addpd		-16(CX_P),			%xmm1	// (flo + b)          , (flo + d)
	mulpd		%xmm3,				%xmm1	// (flo(flo + b))     , (flo(flo + d))
	mulsd		-40(CX_P),			%xmm3	// a flo
	addpd		-32(CX_P),			%xmm1	// (flo(flo + b) + c) , (flo(flo + d) + e)
	movhlps		%xmm1,				%xmm4	// (flo(flo + d) + e)
	mulsd		%xmm2,				%xmm1	// 2^fhi (a flo)
	mulsd		%xmm4,				%xmm3	// (flo(flo + b) + c)(flo(flo + d) + e)
	mulsd		%xmm3,				%xmm1	// 2^fhi (2^flo - 1)
	addsd		%xmm1,				%xmm2	// 2^fhi + 2^fhi(2^flo - 1)
	
	movd		AX_P,				%xmm0
	psllq		$52,				%xmm0	// n shifted into exponent bits
	paddd		%xmm2,				%xmm0	// 2^n(2^fhi + 2^fhi(2^flo - 1))
	
#if defined(__i386__)
	movsd		%xmm0,			FRAME_SIZE(STACKP)
	fldl		FRAME_SIZE(STACKP)
#endif
	ret
	
2:	// Early out for exact integer powers
	// no inexact flag is set.
	add			$1023,				AX_P
	movd		AX_P,				%xmm0
	psllq		$52,				%xmm0
#if defined(__i386__)
	movsd		%xmm0,			FRAME_SIZE(STACKP)
	fldl		FRAME_SIZE(STACKP)
#endif
	ret
	
3:											// if (|x| > 1022 or isnan(x))
	jge			5f							//   goto 5
	
	// |x| < 0x1.0p-8
#if defined( __x86_64__)
	cmp		REL_ADDR(tiny_cut),		%rax	// if (|x| <= 0x1.0p-55)
#else
	cmp			$0xfd100000,		%eax
#endif
	jbe			4f							//   goto 4
	
	// 0x1.0p-55 < |x| < 0x1.0p-8
	// Minimax polynomial approximation
	lea		REL_ADDR(small_poly),	DX_P
	movapd		%xmm0,				%xmm1
	mulsd		40(DX_P),			%xmm1
	addsd		32(DX_P),			%xmm1
	mulsd		%xmm0,				%xmm1
	addsd		24(DX_P),			%xmm1
	mulsd		%xmm0,				%xmm1
	addsd		16(DX_P),			%xmm1
	mulsd		%xmm0,				%xmm1
	addsd		8(DX_P),			%xmm1
	mulsd		%xmm1,				%xmm0
	addsd		(DX_P),				%xmm0
#if defined(__i386__)
	movsd		%xmm0,			FRAME_SIZE(STACKP)
	fldl		FRAME_SIZE(STACKP)
#endif
	ret

4:	// |x| <= 0x1.0p-55, exp2(x) = 1.0 + x
	addsd	REL_ADDR(small_poly),	%xmm0
#if defined(__i386__)
	movsd		%xmm0,			FRAME_SIZE(STACKP)
	fldl		FRAME_SIZE(STACKP)
#endif
	ret

5:	// |x| > 1022 or isnan(x)
#if defined(__x86_64__)
	cmp		REL_ADDR(huge_cut),		%rax	// if (|x| >= 1075 or isnan(x))
#else
	cmp			$0x0120cc00,		%eax
#endif
	jae			7f							//   goto 7
	
	xorpd		%xmm1,				%xmm1	// 0.0
	comisd		%xmm1,				%xmm0	// if (x > 0)
	jnc			8f							//   goto 8
	
	// Gradual underflow is handled here.
	
	// This code is largely copied from the mainline branch.  To make identifying and explaining
	// the changes easier, all comments have been removed, except where the algorithm differs
	// from the mainline.  For details on the uncommented lines, consult above.
	movapd		%xmm0,				%xmm1	
	mulsd	REL_ADDR(one_p7),		%xmm0
	cvttsd2si	%xmm0,				AX_P
	add			DX_P,				AX_P
	mov			AX_P,				DX_P
	sar			$7,					AX_P
		
	cvtsi2sd	AX_P,				%xmm0
	ucomisd		%xmm1,				%xmm0
	je			6f
	
	movsd	REL_ADDR(tiny_val),		%xmm5	// load 0x1p-1022
	mulsd		%xmm5,				%xmm5	// multiply to force underflow flag.
	
	subsd		%xmm0,				%xmm1
	and			$0x7f,				DX_P
	sal			$4,					DX_P
	
	movsd	REL_ADDR(small_poly),	%xmm0	// load 1.0
	
	lea		REL_ADDR(exp2table),	CX_P
	subsd		(DX_P, CX_P, 1),	%xmm1
	movsd		8(DX_P, CX_P, 1),	%xmm2
	
	movlhps		%xmm1,				%xmm1
	movapd		%xmm1,				%xmm3
	addpd		-16(CX_P),			%xmm1
	mulpd		%xmm3,				%xmm1
	mulsd		-40(CX_P),			%xmm3
	addpd		-32(CX_P),			%xmm1
	movhlps		%xmm1,				%xmm4
	mulsd		%xmm4,				%xmm3
	
	add			$1022,				AX_P
	movd		AX_P,				%xmm6
	psllq		$52,				%xmm6	// n + 1022 shifted into exponent bits.
	paddd		%xmm6,				%xmm2	// hi = 2^(1022 + n + fhi)
	mulsd		%xmm2,				%xmm1
	mulsd		%xmm3,				%xmm1	// lo = hi*(2^flo - 1)
	
	movapd		%xmm2,				%xmm3
	addsd		%xmm0,				%xmm2	// 1.0 + hi
	movapd		%xmm2,				%xmm4
	subsd		%xmm0,				%xmm2	// (1.0 + hi) - 1.0
	subsd		%xmm2,				%xmm3	// hi - ((1.0 + hi) - 1.0) = rounding error of computing 1.0 + hi
	addsd		%xmm3,				%xmm1	// lo + round error
	
	addsd		%xmm1,				%xmm4	// (1.0 + hi) + (lo + round error), rounded in correct place for denormal result.
	xorpd		%xmm4,				%xmm0	// remove exponent bits.
	
#if defined(__i386__)
	movsd		%xmm0,			FRAME_SIZE(STACKP)
	fldl		FRAME_SIZE(STACKP)
#endif
	ret

6:	// Exact denormal result, no underflow occurs.
	mov			$1,					DX_P
	movd		DX_P,				%xmm0
	add			$1074,				AX_P
	movd		AX_P,				%xmm1
	psllq		%xmm1,				%xmm0	// 1 << (1074 + n) = 2^(-n) for n < -1022
#if defined(__i386__)
	movsd		%xmm0,			FRAME_SIZE(STACKP)
	fldl		FRAME_SIZE(STACKP)
#endif
	ret

7:	// handle underflows, overflows, nans.
	ucomisd		%xmm0,				%xmm0	// if isnan(x)
	jp			9f							//   goto 10
	
	xorpd		%xmm1,				%xmm1
	comisd		%xmm1,				%xmm0	// if result will overflow
	jnc			9f							//   goto 9
	
	// underflows here
	movsd	REL_ADDR(neg_inf),		%xmm1
	cmpneqsd	%xmm0,				%xmm1	// (x != -inf) ? -1ULL : 0
	movsd	REL_ADDR(tiny_val),		%xmm0
	andpd		%xmm0,				%xmm1	// (x != -inf) ? 0x1p-1022 : 0
	mulsd		%xmm1,				%xmm0	// (x != -inf) ? 0 w/ underflow, inexact : exact 0
#if defined(__i386__)
	movsd		%xmm0,			FRAME_SIZE(STACKP)
	fldl		FRAME_SIZE(STACKP)
#endif
	ret
	
8:
#if defined(__x86_64__)
	cmp		REL_ADDR(overflow),		%rax	// reflect large finite numbers < 1024 back into the mainline
#else
	cmp			$0x01200000,		%eax
#endif
	jb			1b
9:
	// handle overflow results and NaN inputs
	mulsd	REL_ADDR(huge_val),		%xmm0	// overflow for large finite nubmers, exact infinity for infinity
											// also quiets NaN inputs
#if defined(__i386__)
	movsd		%xmm0,			FRAME_SIZE(STACKP)
	fldl		FRAME_SIZE(STACKP)
#endif
	ret
	
