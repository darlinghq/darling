/****************************************************************************

	tanh.s
	
	by Stephen Canon
	
	Copyright © 2007, Apple Inc.  All Rights Reserved.
	
	Implementation of the C99 tanh function for the MacOS X i386 and x86_64 ABIs.	
	
****************************************************************************/
#include <machine/asm.h>
#include "abi.h"

/****************************************************************************

	Tables and Constants

****************************************************************************/
.const
.align 4
abs_mask:		.quad 0x7fffffffffffffff, 0x0000000000000000
four_exp:		.quad 0x0020000000000000, 0x0000000000000000
one:			.quad 0x3ff0000000000000, 0x0000000000000000

			// Coefficients for minimax approximation in 2 exp(-2x) on [2,22)
			.quad 0xbf8dc165dad1eab3
			.quad 0x3f9fef26c0bbebc1
			.quad 0xbfafffbdaa5e7368
			.quad 0x3fbfffff6ee83673
			.quad 0xbfcfffffff536d53
			.quad 0x3fdfffffffff9b22
			.quad 0x3ce6000000000000

			// Coefficients for the 4th order minimax polynomial
			// approximation of exp2( ) on the interval [-0x1p-27, 0x1p-7]
			.quad 0x3f55e52272e0eaec	// c4
			.quad 0x3f83b2a8abda3d8f	// c3
			.quad 0x3fac6b08d78310b8	// c2
			.quad 0x3fcebfbdff82bda7	// c1
			.quad 0x3fe62e42fefa39ef	// c0
			
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

piecewise:
				/*-0.2864515168839901090209747108579654875605616e-3*(x+.4933464573244525103789200064459993631425989)*
				(x-0.1182415726241881770954331861172954748535105e-1)*(x-.1019569932000777092899138256855970885981370)*
				(x-1.707457006002457111723639323617400555673950)*(x-17.55700120861493418461246339928875474977667)*
				(x^2+2.770717309872218703235489009413553490784394*x+2.118123962269468702798445003445138300343937)*
				(x^2+1.838419944636830507881531448964308951713899*x+2.273811442636048173850054792255851453680793)*
				(x^2-1.510861832472913549784965349644347645304161*x+2.694330951091002235734548632049431595168001)*
				(x^2-2.891286467193815429903104107548024238154762*x+2.855309648308176377344446946752634506917244)*/
				
				.quad 0x3feef20000000000, 0x3f6e000000000000 	// 3961/4096 x + 15/4096
				.quad 0xbfbd20c2f0233edb, 0xc03343b38896f0b4	// a_1,			a_2
				.quad 0x40062a6dd642e5f2, 0x3ffd6a2b08287816	// a_3,			a_4
				.quad 0xbff82c7d74f3eec6, 0xc007215acc9fb7f0	// a_5,			a_6
				.quad 0x3f53c077627b845c, 0x403dfa52b8813743	// b_1,			b_2
				.quad 0x4000f1eaf9d69209, 0x400230c40dbb23a6	// b_3,			b_4
				.quad 0x40058dfd62bc4a9c, 0x4006d7ac95bb7ebb	// b_5,			b_6
				.quad 0xbf32c5dbe59ba738, 0xbf2285e830f09106	// a_0,			b_0
				
				/* -0.2375878883133903234332653094979448890904333e-2*(x+1.141393572200027712958325552880364704841667)*
				(x+0.9432391007626355646599517236151910406352282e-1)*(x-0.8037992537205077203224291088009425075443164e-1)*
				(x^2+2.922334131587764592096900045673310598662462*x+2.343651060006581810841903559429949385193724)*
				(x^2+2.164423024969638405314605029797899150324956*x+2.538857443423985682428496892587673324322619)*
				(x^2-.8978880630510944949367543166122444149618204*x+2.647052449400489220824414241183706204517465)*
				(x^2-2.338357501383843298875790789553535972448012*x+2.678582971901324051327760583919669959590076)*
				(x^2-3.175478809229660118488076975757657039109498*x+2.696893643567215337517965055644178016901177)*/
				
				.quad 0x3febfc0000000000, 0x3f9cc00000000000 	// 3582/4096 x + 115/4096
				.quad 0x3f8c8ea9f23e804c, 0x400760f0b7993b69	// a_1,			a_2
				.quad 0x400150bd04d7a1c5, 0xbfecbb7fbf48be5b	// a_3,			a_4
				.quad 0xc002b4f4c7166628, 0xc00967616f164262	// a_5,			a_6
				.quad 0xbf7f0e0a35e743ee, 0x4002bfcc207fb647	// b_1,			b_2
				.quad 0x40044f947dc5b290, 0x40052d29d5a7c63d	// b_3,			b_4
				.quad 0x40056dbce8bf8267, 0x4005933cf97f4ab1	// b_5,			b_6
				.quad 0xbf63769443462e8e, 0xbf66371574fff23b	// a_0,			b_0
				
				/* 0.6821180293172612295640961212495022937011365e-3*(x+0.9386570864065908327923776423721617419905786e-1)*
				(x-0.8589816406847459765176398027517267526480442e-1)*(x-2.166095361145909667282101208582047402400105)*
				(x^2+2.983194540901171801414314552018551787362570*x+2.327924268962062274660991496770128609481354)*
				(x^2+2.500812465242608806630929644245570226918524*x+2.604020578541220636907356420919440871778391)*
				(x^2-.4811225869073539084506952758557871069271204*x+3.113496944535685107372520625657988666598883)*
				(x^2-2.258568925224693781821970135740820585965534*x+3.443166841217303155561660113644021763452671)*
				(x^2-3.663633157183776833100177799273882127652364*x+3.986165335805472349372636980074778113068575)*/
				
				.quad 0x3fe6400000000000, 0x3fbdf00000000000 	// 2848/4096 x + 479/4096
				.quad 0x3f805149baef85ef, 0x4007dd95197635cc	// a_1,			a_2
				.quad 0x400401a9f73d2b4c, 0xbfdecab664089381	// a_3,			a_4
				.quad 0xc002118c95acd0c0, 0xc00d4f1ee6952a16	// a_5,			a_6
				.quad 0xbf8083470d069524, 0x40029f96c2560ddb	// b_1,			b_2
				.quad 0x4004d508bdb79078, 0x4008e8711607d045	// b_3,			b_4
				.quad 0x400b8b9b0e8d997b, 0x400fe3aaa6cddded	// b_5,			b_6
				.quad 0x3f465a05505fff51, 0xbf583538a7c2274b	// a_0,			b_0
				
				/* 0.3675050415084445460952193551199634192284019e-3*(x+2.538253007068358766085970620345190291400062)*
				(x+1.891265936573533097652685331666438995456676)*(x+0.9156287399777636946014542863238438354098965e-1)*
				(x-0.8403859216027488106552551230547659805950613e-1)*(x-1.802954954543863782501756058330497972336359)*
				(x^2+3.119393795298718299316411466993714733886600*x+3.334575315228986457403114264913740785470140)*
				(x^2-0.2660697000023952164642638899375535969090176e-1*x+3.205893970965164102059321144392191513892765)*
				(x^2-1.805070720874556914625952264641807822799429*x+3.222143486665259634799789338307105717457736)*
				(x^2-3.121702245703248826686097843186573720076780*x+3.242552497083970905169346770196317588771562)*/
				
				.quad 0x3fe0400000000000, 0x3fd0700000000000 	// 2080/4096 x + 1052/4096
				.quad 0x3ffcea67384b11e9, 0xbffb61dca98942f1	// a_1,			a_2
				.quad 0x4008f484bbf139fd, 0xbf9b3edb87fc9a2b	// a_3,			a_4
				.quad 0xbffce191d611f94f, 0xc008f93f06e92964	// a_5,			a_6
				.quad 0xbfc4581fb52271d2, 0xbfc52176c275a3ab	// b_1,			b_2
				.quad 0x400aad35d2a7a8bb, 0x4009a5abbcfde960	// b_3,			b_4
				.quad 0x4009c6f32a11f93f, 0x4009f0bf5d144f71	// b_5,			b_6
				.quad 0x3f3815b62267a35a, 0x3f4e91115d517686	// a_0,			b_0
				
				/* -0.1454916941539841281056690225872902433188312e-3*(x+1.895467253199031252686203675164860153418254)*
				(x+0.9130291828799857823935486078744448547578965e-1)*(x-0.8499583911816027391302764050755799469607102e-1)*
				(x^2+3.398145257103028458687443509002980550445610*x+3.636501613989623235844429443005415702389483)*
				(x^2+.5498410413574713586659146118099117563075050*x+3.861165354740107211770934343766878654423520)*
				(x^2-1.359498118602839703247623757217688185600174*x+4.035047519552677285194086315801939227901986)*
				(x^2-3.036050352195771266788909949443243206515610*x+4.189494379230451407340774930662591118810411)*
				(x^2-4.012340397489436305623606583590817514912250*x+4.276055559103643625266538933042186424497733)*/
				
				.quad 0x3fd6400000000000, 0x3fdaa00000000000 	// 1424/4096 x + 1704/4096
				.quad 0x3f79d573ac48a801, 0x400b2f66c7d28529	// a_1,			a_2
				.quad 0x3fe1984c3d542023, 0xbff5c0811965f6af	// a_3,			a_4
				.quad 0xc00849d4c45d855d, 0xc0100ca2f60e8c0d	// a_5,			a_6
				.quad 0xbf7fc955f6cf414f, 0x400d178e287f7e41	// b_1,			b_2
				.quad 0x400ee3aaa95874bc, 0x401023e37f392380	// b_3,			b_4
				.quad 0x4010c20ad086484e, 0x40111aae4ef8eaa5	// b_5,			b_6
				.quad 0xbf2311e422ef2c53, 0xbf3212bb7f586ef1	// a_0,			b_0
				
				/* -0.5412652598572911167212102331230339557075992e-4*(x+2.629392008383213598304883848137312998666697)*
				(x+0.8789494961083907158478561801157544587360792e-1)*(x-0.8208279725873500595208981438214151574275247e-1)*
				(x^2+4.195478155052112358115820942477950941061380*x+4.966757346123734917905738242970175837409293)*
				(x^2+.8823263428901269743834670128774202678301578*x+4.118062244853954226344188393026476912078044)*
				(x^2-1.131877687016688662064358097058320698192151*x+4.076018964523053136439220066275491038611692)*
				(x^2-2.893472150157534957506432929430867039460720*x+4.093336646134720875078946869223410975744725)*
				(x^2-3.919024916687541429883627778453309015591906*x+4.107472917357761636545594014132211005272860)*/
				
				.quad 0x3fcd280000000000, 0x3fe2160000000000 	// 933/4096 x + 2315/4096
				.quad 0x3f77ce7bc458b46c, 0x4010c82b6cec1ffd	// a_1,			a_2
				.quad 0x3fec3c0474639899, 0xbff21c2bc70cf0da	// a_3,			a_4
				.quad 0xc00725d4ba0681df, 0xc00f5a29bc4b0e92	// a_5,			a_6
				.quad 0xbf7d8d1f70cb1f6f, 0x4013ddf5a34313a8	// b_1,			b_2
				.quad 0x401078e54f22290a, 0x40104dd7ea5a025a	// b_3,			b_4
				.quad 0x40105f93a44aaaf5, 0x40106e0d61650779	// b_5,			b_6
				.quad 0xbf0c60bd458a1834, 0xbf22a7763e951b37	// a_0,			b_0
				
				/* 0.4924053668989264983145283706733415056216404e-5*(x+0.8796471725749816598263629969503552203165658e-1)*
				(x-0.8243298434040044427057532032166616955244352e-1)*(x-5.247080327413196729391477686672343881798544)*
				(x^2+4.375087198300077517171213474173238111040758*x+5.113333656883580100278523194277394673560613)*
				(x^2+1.332348590031229077157574373887077497579276*x+5.115849567237085180807802824713706539664123)*
				(x^2-.9350409521747968106518680203179119052124734*x+5.390531195312283061461451786153366587344480)*
				(x^2-3.215852224589302541442409521049125766355344*x+5.835283059223468050039391207582881254538750)*
				(x^2-4.805142513405023792476061013566488171009830*x+6.284366447110418558084552080608121886245705)*/
				
				.quad 0x3fc2900000000000, 0x3fe60a0000000000 	// 594/4096 x + 2821/4096
				.quad 0xc015516bf006503c, 0x40118016dbc7636e	// a_1,			a_2
				.quad 0x3ff5514cc150e4b9, 0xbfedebdb00c05dd1	// a_3,			a_4
				.quad 0xc009ba10bb2b0a0a, 0xc0133877476ec678	// a_5,			a_6
				.quad 0x3fdbae9cc13f99d1, 0x4014740dbcf7677d	// b_1,			b_2
				.quad 0x401476a144da27f9, 0x40158fe768dfbc76	// b_3,			b_4
				.quad 0x401757547139120d, 0x40192330f539ad04	// b_5,			b_6
				.quad 0x3ed4a72990f33fe0, 0x3e9d115702afdd8d	// a_0,			b_0
				
				/* 0.6862565065994843133907783301321216634622772e-5*(x+0.9630374666835468858692155213887501416937462e-1)*
				(x-0.9263910186969991762077298562812547568924601e-1)*(x-2.323730050905880210881006882732927135598709)*
				(x^2+4.915343084278116341741257562832236772519938*x+6.366157435903189155603019667371637988996918)*
				(x^2+1.824272160555820074533801014349118587449441*x+5.502457749802857122655878777027393347399835)*
				(x^2-.3300561409680903303325074516366401983263062*x+5.332584863498335175639855633785690269440549)*
				(x^2-2.470967591354500287186505101999528533350514*x+5.359889068546726215460509020568700628842379)*
				(x^2-4.062368446227079298604593445606996624167424*x+5.389376533522531429130148833360296979673207)*/
				
				.quad 0x3fb7300000000000, 0x3fe9120000000000 	// 371/4096 x + 3209/4096
				.quad 0xc00354b959b1dff0, 0x4013a94fb28e608b	// a_1,			a_2
				.quad 0x3ffd3038014972b9, 0xbfd51fa3cad3528e	// a_3,			a_4
				.quad 0xc003c48aa812bf6e, 0xc0103fdd839363fc	// a_5,			a_6
				.quad 0x3fcb8de916d04f6d, 0x401976f1f99190cf	// b_1,			b_2
				.quad 0x4016028448cc199f, 0x40155491205f7ada	// b_3,			b_4
				.quad 0x40157086c28e6266, 0x40158eb8b8d53c8a	// b_5,			b_6
				.quad 0x3edcc89f858814a5, 0x3ea62d021453335f	// a_0,			b_0
				
small_poly:
				/* -0.1912438313598201685992817228652558781857235e-3*
				(x^2+3.756675270173468426449695171076913404673252*x+6.166462828789874819735603927346200075670479)*
				(x^2+.7625866507820391697007654102468723536049672*x+6.364574963873789771039813999418889409285683)*
				(x^2-2.655819614791829218878773973800635875503330*x+6.590413750655284949335220033866164267136093)*
				(x^2-4.892571285931185215076239999657804121172708*x+6.738653176394183551008839845454158365598159)*/
				
				.quad 0x400e0dabc398b233, 0x3fe8671c1eaf3374	// a_1,			a_2
				.quad 0xc0053f1e5aacd98c, 0xc01391fe3509b143	// a_3,			a_4
				.quad 0x4018aa753b569bd0, 0x4019755323ab1cfd	// b_1,			b_2
				.quad 0x401a5c956c18b14e, 0x401af4617f8ecc54	// b_3,			b_4
				.quad 0xbf29111400a41fbf, 0x0000000000000000	// c
					
.literal8
.align 3
m2_lge_p7:		.quad 0xc0771547652b82fe
m2_lge:			.quad 0xc0071547652b82fe
m2_lge_hi:		.quad 0xc007154760000000
m2_lge_lo:		.quad 0xbe64ae0bf85ddf44

two:			.quad 0x4000000000000000
tiny_m_two:		.quad 0xfe40000000000000
eighth_m_two:	.quad 0xffc0000000000000
large_m_two:	.quad 0x003c000000000000

split_mask:		.quad 0xfffffffff8000000
one_half:		.quad 0x3fe0000000000000
min_normal:		.quad 0x0010000000000000
infinity:		.quad 0x7ff0000000000000
one_p55:		.quad 0x4360000000000000
one_n55:		.quad 0x3c80000000000000

/****************************************************************************

	PIC code

****************************************************************************/
#if defined( __x86_64__ )
	#define REL_ADDR(_a)	(_a)(%rip)
#else
	#define REL_ADDR(_a)	(_a)-0b(%ecx)
#endif

/****************************************************************************

	Entry point for tanh( )

****************************************************************************/
.text
ENTRY( tanh )
#if defined( __x86_64__ )
	movapd			%xmm0,				%xmm1
	andpd		REL_ADDR(abs_mask),		%xmm0	// |x|
	movd			%xmm0,				%rax
	movapd			%xmm1,				%xmm7
	sub			REL_ADDR(two),			%rax
	xorpd			%xmm0,				%xmm7	// signbit(x)
	cmp			REL_ADDR(large_m_two),	%rax
	ja				1f
#elif defined( __i386__ )
	movl	  4+FRAME_SIZE(STACKP),		%eax
	movsd		FRAME_SIZE(STACKP),		%xmm0
	call			0f
0:	pop				%ecx
	and				$0x7fffffff,		%eax
	movapd			%xmm0,				%xmm1
	andpd		REL_ADDR(abs_mask),		%xmm0	// |x|
	movapd			%xmm1,				%xmm7
	sub				$0x40000000,		%eax
	xorpd			%xmm0,				%xmm7	// signbit(x)
	cmp				$0x003c0000,		%eax
	ja				1f
#else
	#error UNKNOWN ARCH
#endif
	
	// Code path for 2 < |x| < 22:
	
	// Code copied from exp( ) -----------------------
	movapd			%xmm0,				%xmm1
	mulsd		REL_ADDR(m2_lge_p7),	%xmm0
	cvttsd2si		%xmm0,				AX_P
	add				$63,				AX_P
	mov				AX_P,				DX_P
	sar				$7,					AX_P
	and				$0x7f,				DX_P
	sal				$4,					DX_P
	movapd			%xmm1,				%xmm2
	mulsd		REL_ADDR(m2_lge),		%xmm1
	cvtsi2sd		AX_P,				%xmm0
	movapd			%xmm1,				%xmm6
	subsd			%xmm0,				%xmm1
	movapd			%xmm2,				%xmm4
	movsd		REL_ADDR(split_mask),	%xmm3
	mulsd		REL_ADDR(m2_lge_lo),	%xmm4
	movsd		REL_ADDR(m2_lge_hi),	%xmm5
	lea			REL_ADDR(exp2table),	CX_P
	subsd			(DX_P, CX_P, 1),	%xmm1
	andpd			%xmm2,				%xmm3
	subsd			%xmm3,				%xmm2
	mulsd			%xmm5,				%xmm3
	mulsd			%xmm5,				%xmm2
	subsd			%xmm6,				%xmm3
	addsd			%xmm3,				%xmm2
	addsd			%xmm4,				%xmm2
	movsd			-40(CX_P),			%xmm6
	mulsd			%xmm1,				%xmm6
	addsd			-32(CX_P),			%xmm6
	mulsd			%xmm1,				%xmm6
	addsd			-24(CX_P),			%xmm6
	addsd			%xmm1,				%xmm2
	movapd			%xmm2,				%xmm3
	mulsd			%xmm2,				%xmm6
	mulsd			%xmm3,				%xmm3
	addsd			-16(CX_P),			%xmm6
	mulsd			-8(CX_P),			%xmm2
	mulsd			%xmm6,				%xmm3
	addsd			%xmm3,				%xmm2
	movsd			8(DX_P, CX_P, 1),	%xmm1
	// End exp( ) code block -------------------------
	
	// At this point, the following registers have useful values:
	//
	//		AX_P	n				integer
	//		xmm1	a				double-precision number in the interval [exp(-1/2), exp(1/2))
	//		xmm2	b				double-precision number < 2^-7 ish
	//		xmm7	signbit(x)		bit pattern
	// 
	// where 2^(n+1)*(a + a*b) = 2 exp(-2x).
	
	inc				AX_P
	movd			AX_P,				%xmm4
	psllq			$52,				%xmm4
	
	paddd			%xmm4,				%xmm1	// 2^(n+1) a
	mulsd			%xmm1,				%xmm2	// 2^(n+1) a b
	movapd			%xmm2,				%xmm4
	addsd			%xmm1,				%xmm2	// z = 2 exp(-2x) = 2^(n+1)*(a + ab)
		
	movsd			-96(CX_P),			%xmm3	// c7
	mulsd			%xmm2,				%xmm3	// c7z
	addsd			-88(CX_P),			%xmm3	// c6 + c7z
	mulsd			%xmm2,				%xmm3	// c6z + c7z^2
	addsd			-80(CX_P),			%xmm3	// c5 + c6z + c7z^2
	mulsd			%xmm2,				%xmm3	// c5z + c6z^2 + c7z^3
	addsd			-72(CX_P),			%xmm3	// c4 + c5z + c6z^2 + c7z^3
	mulsd			%xmm2,				%xmm3	// c4z + c5z^2 + c6z^3 + c7z^4
	addsd			-64(CX_P),			%xmm3	// c3z + c4z + c5z^2 + c6z^3 + c7z^4
	mulsd			%xmm2,				%xmm3	// c3z + c4z^2 + c5z^3 + c6z^4 + c7z^5
	addsd			-56(CX_P),			%xmm3	// c2 + c3z + c4z^2 + c5z^3 + c6z^4 + c7z^5
	mulsd			%xmm2,				%xmm3	// c2z + c3z^2 + c4z^3 + c5z^4 + c6z^5 + c7z^6
	addsd			-48(CX_P),			%xmm3	// (c1 + 1.0) + c2z + c3z^2 + c4z^3 + c5z^4 + c6z^5 + c7z^6
	mulsd			%xmm2,				%xmm3	// (c1 + 1.0)z + c2z^2 + c3z^3 + c4z^4 + c5z^5 + c6z^6 + c7z^7
	
	movsd			-112(CX_P),			%xmm0	// 1.0
	subsd			%xmm1,				%xmm0	// result_hi = 1.0 - 2^(n+1) a
	movapd			%xmm0,				%xmm5
	subsd			-112(CX_P),			%xmm5
	addsd			%xmm1,				%xmm5	// result_hi_err = (result_hi - 1.0) + 2^(n+1) a
		
	addsd			%xmm5,				%xmm4	// 2^(n+1) a b + result_hi_err
	subsd			%xmm4,				%xmm3	// result_lo = z p(z) - (2^(n+1) a b + result_hi_err)
	
	xorpd			%xmm7,				%xmm0	// or in signbit
	xorpd			%xmm7,				%xmm3
	
	addsd			%xmm3,				%xmm0	// result_hi + result_lo
												//  = 1.0 - 2^(n+1)a - 2^(n+1)a b + z p(z)
#if defined(__i386__)
	movsd		%xmm0,			FRAME_SIZE(STACKP)
	fldl		FRAME_SIZE(STACKP)
#endif
	ret
	
1:
	jg				6f
#if defined( __x86_64__ )
	cmp			REL_ADDR(eighth_m_two),	%rax
#else
	cmp				$0xffc00000,		%eax
#endif
	jbe				2f
	
	// Code path for 0.125 < |x| < 2.0

	movapd		REL_ADDR(four_exp),		%xmm2
	paddd			%xmm0,				%xmm2	// 4|x| (adding 2 to the exponent bits effects
												//       a multiplication by 4 because x has
												//       exponent between -3 and 1).
	cvttsd2si		%xmm2,				AX_P	// floor(4|x|)
	cvtsi2sd		AX_P,				%xmm2
	addsd		REL_ADDR(one_half),		%xmm2	// floor(4|x|) + 0.5
	psubd		REL_ADDR(four_exp),		%xmm2	// 0.25*floor(4|x|) + 0.125
												//      (again, we can use exponent subtraction
												//       instead of multiplication because we
												//       are guaranteed that floor(4|x|) + 0.5
												//		 has an exponent between -1 and 3)
	subsd			%xmm2,				%xmm0	// u = |x| - 0.25*floor(4|x|) - 0.125
												// u is in the interval [-0.125, 0.125]
												
	shl				$7,					AX_P	// 16 * sizeof(double) * floor(4|x|)
	
	movlhps			%xmm0,				%xmm0	// [ u , u ]
	por			REL_ADDR(one),			%xmm7
	
	movq		REL_ADDR(split_mask),	%xmm3
	andpd			%xmm1,				%xmm3	// xhi -- top 26 bits of x
	subsd			%xmm3,				%xmm1	// xlo -- x - xhi
	
	lea			REL_ADDR(piecewise),	DX_P
	movsd			8(AX_P, DX_P, 1),	%xmm6
	mulsd			(AX_P, DX_P, 1),	%xmm3	// L1 xhi
	mulsd			(AX_P, DX_P, 1),	%xmm1	// L1 xlo
	mulsd			%xmm7,				%xmm6
	addsd			%xmm6,				%xmm3	// L1 xhi + L0
	
	/* Code for factored polynomial evaluation */
	movapd			%xmm0,				%xmm4
	movapd			%xmm4,				%xmm5
	movapd			%xmm5,				%xmm6
	addpd			16(AX_P, DX_P, 1),	%xmm0	// [         u + a1 , u + a2         ]
	addpd			32(AX_P, DX_P, 1),	%xmm4	// [         u + a3 , u + a4         ]
	addpd			48(AX_P, DX_P, 1),	%xmm5	// [         u + a5 , u + a6         ]
	mulpd			%xmm6,				%xmm0	// [      (u + a1)u , (u + a2)u      ]
	mulpd			%xmm6,				%xmm4	// [      (u + a3)u , (u + a4)u      ]
	mulpd			%xmm6,				%xmm5	// [      (u + a5)u , (u + a6)u      ]
	mulsd			112(AX_P, DX_P, 1),	%xmm6	// a0 u
	addpd			64(AX_P, DX_P, 1),	%xmm0	// [ (u + a1)u + b1 , (u + a2)u + b2 ]
	addpd			80(AX_P, DX_P, 1),	%xmm4	// [ (u + a3)u + b3 , (u + a4)u + b4 ]
	addpd			96(AX_P, DX_P, 1),	%xmm5	// [ (u + a5)u + b5 , (u + a6)u + b6 ]
	addsd			120(AX_P, DX_P, 1),	%xmm6	// a0 u + b0
	
	movlhps			%xmm7,				%xmm6	// [ lin , signbit ]
	mulpd			%xmm6,				%xmm0	// [ lin * quad1, signbit * quad2 ]
	mulpd			%xmm5,				%xmm4	// [ quad3 * quad5, quad4 * quad6 ]
	mulpd			%xmm4,				%xmm0	// [ lin * quad1 * quad3 * quad5, signbit * quad2 * quad4 * quad6 ]
	movhlps			%xmm0,				%xmm4	// signbit * quad2 * quad4 * quad6
	mulsd			%xmm4,				%xmm0	// signbit * lin * quad1 * quad2 * quad3 * quad4 * quad5 * quad6
	
	addsd			%xmm1,				%xmm0	// L1 xlo + p(u)
	addsd			%xmm3,				%xmm0	// (L1 xhi + L0) + (L1 xlo + p(u))
	
#if defined(__i386__)
	movsd		%xmm0,			FRAME_SIZE(STACKP)
	fldl		FRAME_SIZE(STACKP)
#endif
	ret
	
2:
#if defined( __x86_64__ )
	cmp			REL_ADDR(tiny_m_two),	%rax
#else
	cmp				$0xfe400000,		%eax
#endif
	jbe				3f
	
	// Polynomial for 2^-54 < |x| < 0.125
	
	lea			REL_ADDR(small_poly),	DX_P
	mulsd			%xmm0,				%xmm0	// u = x^2
	movapd			%xmm1,				%xmm2	// x
	mulsd			64(DX_P),			%xmm1	// cx
	movlhps			%xmm0,				%xmm0	// [ u , u ]
	movapd			%xmm0,				%xmm3	// [ u , u ]
	movapd			%xmm3,				%xmm4
	addpd			(DX_P),				%xmm0	// [         u + a1 , u + a2 ]
	addpd			16(DX_P),			%xmm3	// [         u + a3 , u + a4 ]
	mulpd			%xmm4,				%xmm0	// [      (u + a1)u , (u + a2)u      ]
	mulpd			%xmm4,				%xmm3	// [      (u + a3)u , (u + a4)u      ]
	mulsd			%xmm4,				%xmm1	// cx^3
	addpd			32(DX_P),			%xmm0	// [ (u + a1)u + b1 , (u + a2)u + b2 ]
	addpd			48(DX_P),			%xmm3	// [ (u + a3)u + b3 , (u + a4)u + b4 ]
	mulpd			%xmm3,				%xmm0	// [ quad1 * quad3 , quad2 * quad4 ]
	mulsd			%xmm0,				%xmm1	// cx^3 * quad1 * quad3
	movhlps			%xmm0,				%xmm0
	mulsd			%xmm1,				%xmm0	// cx^3 * quad1 * quad2 * quad3 * quad4
	addsd			%xmm2,				%xmm0	// x + c x^3 p(x^2)

#if defined(__i386__)
	movsd		%xmm0,			FRAME_SIZE(STACKP)
	fldl		FRAME_SIZE(STACKP)
#endif
	ret
	
3:	// Handle |x| < 2^-27
#if defined(__x86_64__)
	movd			%xmm0,				%rax
	cmp			REL_ADDR(min_normal),	%rax
#else
	psrlq			$32,				%xmm0
	movd			%xmm0,				%eax
	cmp				$0x00100000,		%eax
#endif
	jb				4f
	
	movsd		REL_ADDR(one_p55),		%xmm0
	mulsd			%xmm1,				%xmm0
	subsd			%xmm1,				%xmm0
	mulsd		REL_ADDR(one_n55),		%xmm0	// Raises underflow in round-to-zero for x = ±smallest normal
	
#if defined(__i386__)
	movsd		%xmm0,			FRAME_SIZE(STACKP)
	fldl		FRAME_SIZE(STACKP)
#endif
	ret

4:	// Handle denormal inputs
	xorpd			%xmm3,				%xmm3
	comisd			%xmm1,				%xmm3
	jz				5f

	movsd		REL_ADDR(one),			%xmm0
	orpd			%xmm0,				%xmm1
	orpd			%xmm7,				%xmm0	// copysign(1.0, x)
	subsd			%xmm0,				%xmm1	// 2^(1022)*x
	movsd		REL_ADDR(min_normal),	%xmm2
	orpd			%xmm7,				%xmm2	// copysign(tiny, x)
	subsd			%xmm2,				%xmm1	// 2^(1022)*x - copysign(tiny,x)
	movsd		REL_ADDR(min_normal),	%xmm0
	mulsd			%xmm0,				%xmm1	// x - 2^(-1022)*copysign(tiny,x)

5:	movapd			%xmm1,				%xmm0

#if defined(__i386__)
	movsd		%xmm0,			FRAME_SIZE(STACKP)
	fldl		FRAME_SIZE(STACKP)
#endif
	ret

6:	// Handle x = NaN, |x| > 22
	movapd			%xmm0,				%xmm2
	movsd		REL_ADDR(one),			%xmm0
	orpd			%xmm7,				%xmm0	// copysign(1.0, x)
	
	movsd		REL_ADDR(infinity),		%xmm1
	ucomisd			%xmm2,				%xmm1
	jp				8f
	jz				7f

	movsd		REL_ADDR(one_n55),		%xmm1
	orpd			%xmm7,				%xmm1	// copysign(1.0p-55, x)
	subsd			%xmm1,				%xmm0
	
7:	
#if defined(__i386__)
	movsd		%xmm0,			FRAME_SIZE(STACKP)
	fldl		FRAME_SIZE(STACKP)
#endif
	ret
	
8:	addsd			%xmm2,				%xmm2	// quiet NaN
	movapd			%xmm2,				%xmm0
	
#if defined(__i386__)
	movsd		%xmm0,			FRAME_SIZE(STACKP)
	fldl		FRAME_SIZE(STACKP)
#endif
	ret
