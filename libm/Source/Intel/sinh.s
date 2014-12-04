/*
 *
 *  sinh.s
 *
 *    by Stephen Canon
 *
 *  Copyright (c) 2007, Apple Inc.  All Rights Reserved.
 *
 *  Implementation of the c99 sinh function for the MacOS X __i386__ and __x86_64__ ABIs.
 */
 
#include <machine/asm.h>
#include "abi.h"

.const
.align 4
			// Polynomial coefficients, offset from exp2table
			.quad 0x3f55e52272e0eaec, 0x3f55e52272e0eaec	// c4
			.quad 0x401cc9eea1e24220, 0x401cc9eea1e24220	// c3/c4
			.quad 0x3fac6b08d78310b8, 0x3fac6b08d78310b8	// c2
			.quad 0x3fcebfbdff82bda7, 0x3fcebfbdff82bda7	// c1
			.quad 0x3fe62e42fefa39ef, 0x3fe62e42fefa39ef	// c0

			// Adapted from the table by jkidder in exp2.s
			//          -x					  2^x
exp2table:	.quad 0x8000000000000000, 0x3ff0000000000000
			.quad 0xbf8000003ac95980, 0x3ff0163daa4d2352
			.quad 0xbf9000000f064dc0, 0x3ff02c9a3ea19cb9
			.quad 0xbf98000018938b20, 0x3ff04315e8b3c115
			.quad 0xbfa000000306d960, 0x3ff059b0d326ac37
			.quad 0xbfa4000003824c90, 0x3ff0706b29f1f4cf
			.quad 0xbfa8000004f61bf0, 0x3ff087451892075b
			.quad 0xbfac00000e283450, 0x3ff09e3ecb187ccb
			.quad 0xbfb00000078c7308, 0x3ff0b5586d50f577
			.quad 0xbfb2000001591738, 0x3ff0cc922b81fa4a
			.quad 0xbfb4000006507370, 0x3ff0e3ec331dbe33
			.quad 0xbfb6000002e52340, 0x3ff0fb66b020e713
			.quad 0xbfb8000005a20d30, 0x3ff11301d05505f2
			.quad 0xbfba000000c21f28, 0x3ff12abdc07537b2
			.quad 0xbfbc0000006f2418, 0x3ff1429aaeae5f8c
			.quad 0xbfbe000004e59960, 0x3ff15a98c8e0759d
			.quad 0xbfc0000002aea480, 0x3ff172b83cbe3227
			.quad 0xbfc10000002d00d0, 0x3ff18af93890d45f
			.quad 0xbfc20000017a069c, 0x3ff1a35beb93e6ce
			.quad 0xbfc30000032ca860, 0x3ff1bbe084526787
			.quad 0xbfc40000034f70a4, 0x3ff1d48731ba8c98
			.quad 0xbfc50000026c3308, 0x3ff1ed5023390e5f
			.quad 0xbfc6000002d6a13c, 0x3ff2063b88a9792c
			.quad 0xbfc7000001164930, 0x3ff21f4991992be3
			.quad 0xbfc80000027763f8, 0x3ff2387a6eb3ae9a
			.quad 0xbfc9000003509a78, 0x3ff251ce5006d5a1
			.quad 0xbfca000001a60348, 0x3ff26b45660c949f
			.quad 0xbfcb000003b0e9d0, 0x3ff284dfe254261d
			.quad 0xbfcc0000004cde7c, 0x3ff29e9df5279f0b
			.quad 0xbfcd000000cfdaf4, 0x3ff2b87fd0efebe8
			.quad 0xbfce00000396f458, 0x3ff2d285a741ad9e
			.quad 0xbfcf0000001fc510, 0x3ff2ecafa94170d1
			.quad 0xbfd00000011547ec, 0x3ff306fe0a6adb04
			.quad 0xbfd0800000eec6c6, 0x3ff32170fc7e5139
			.quad 0xbfd1000001d61dae, 0x3ff33c08b2c605fd
			.quad 0xbfd1800001a52e62, 0x3ff356c55fead73c
			.quad 0xbfd200000051ad30, 0x3ff371a7374bdcfb
			.quad 0xbfd28000002c2c50, 0x3ff38cae6d0f32b4
			.quad 0xbfd3000001d21df8, 0x3ff3a7db3548da03
			.quad 0xbfd380000146a8dc, 0x3ff3c32dc3599392
			.quad 0xbfd40000002a7356, 0x3ff3dea64c1b56c2
			.quad 0xbfd480000054f350, 0x3ff3fa4504bee17d
			.quad 0xbfd50000005ea89a, 0x3ff4160a220bc5bf
			.quad 0xbfd5800001dc84b4, 0x3ff431f5d9b8e238
			.quad 0xbfd600000003a7aa, 0x3ff44e08606256f0
			.quad 0xbfd68000007c88ac, 0x3ff46a41ed388948
			.quad 0xbfd7000000e3699a, 0x3ff486a2b5f3cad8
			.quad 0xbfd7800001d7fb72, 0x3ff4a32af1415232
			.quad 0xbfd800000036178e, 0x3ff4bfdad542520c
			.quad 0xbfd880000103da68, 0x3ff4dcb29a38937c
			.quad 0xbfd9000001d0e1f2, 0x3ff4f9b27706c869
			.quad 0xbfd980000045a3a2, 0x3ff516daa2df4e31
			.quad 0xbfda0000015743c6, 0x3ff5342b56ec23d4
			.quad 0xbfda80000182b74e, 0x3ff551a4cab6dc4d
			.quad 0xbfdb00000082fdae, 0x3ff56f4736d39096
			.quad 0xbfdb8000014aa914, 0x3ff58d12d4e4f5b2
			.quad 0xbfdc0000008a02aa, 0x3ff5ab07dd68b75f
			.quad 0xbfdc800001be8102, 0x3ff5c9268ac2a0da
			.quad 0xbfdd0000018186c2, 0x3ff5e76f160896a5
			.quad 0xbfdd800001cbe9ce, 0x3ff605e1b9e48ea4
			.quad 0xbfde0000013fb67c, 0x3ff6247eb0870165
			.quad 0xbfde800000ef7448, 0x3ff6434635067f92
			.quad 0xbfdf0000005a7522, 0x3ff66238826b1001
			.quad 0xbfdf800001b5116a, 0x3ff68155d4b7317e
			.quad 0xbfe0000000888e45, 0x3ff6a09e66c229dd
			.quad 0xbfe0400000205a67, 0x3ff6c012751bcc3c
			.quad 0xbfe0800000b6586f, 0x3ff6dfb23cbf72c3
			.quad 0xbfe0c00000f74e0b, 0x3ff6ff7df9ccc6d4
			.quad 0xbfe1000000a557b8, 0x3ff71f75e93f2fc7
			.quad 0xbfe14000004dbf55, 0x3ff73f9a48cca865
			.quad 0xbfe1800000641c09, 0x3ff75feb567517a8
			.quad 0xbfe1c00000f97615, 0x3ff78069505d5b2f
			.quad 0xbfe20000002ed0b2, 0x3ff7a1147402f7a4
			.quad 0xbfe2400000a7c436, 0x3ff7c1ed018716b2
			.quad 0xbfe28000007d3dbd, 0x3ff7e2f337101b34
			.quad 0xbfe2c000000368bf, 0x3ff80427543fe015
			.quad 0xbfe3000000adbadd, 0x3ff8258999a7ac0d
			.quad 0xbfe3400000d62b8b, 0x3ff8471a46946832
			.quad 0xbfe3800000ec0d48, 0x3ff868d99bc161d2
			.quad 0xbfe3c0000054b58f, 0x3ff88ac7d9b76eb5
			.quad 0xbfe40000006e90f6, 0x3ff8ace54265b990
			.quad 0xbfe44000002abac4, 0x3ff8cf3216cc3af3
			.quad 0xbfe4800000c48632, 0x3ff8f1ae997fa64d
			.quad 0xbfe4c00000cad5c9, 0x3ff9145b0c00301f
			.quad 0xbfe5000000350fe0, 0x3ff93737b0eac151
			.quad 0xbfe5400000a31c3c, 0x3ff95a44cc21e4e0
			.quad 0xbfe5800000ae6cc6, 0x3ff97d82a03e9cf0
			.quad 0xbfe5c00000c26ee9, 0x3ff9a0f17135f7b9
			.quad 0xbfe600000091b67d, 0x3ff9c49182f54513
			.quad 0xbfe640000015c9de, 0x3ff9e86319ef5d59
			.quad 0xbfe68000006ad404, 0x3ffa0c667b9a2c00
			.quad 0xbfe6c000000cd09a, 0x3ffa309bec517245
			.quad 0xbfe7000000fe5d74, 0x3ffa5503b2cf3ac1
			.quad 0xbfe740000011ec75, 0x3ffa799e133afab3
			.quad 0xbfe7800000249e28, 0x3ffa9e6b558f1ac2
			.quad 0xbfe7c000002e4a1c, 0x3ffac36bbfeec930
			.quad 0xbfe80000008c2c1b, 0x3ffae89f99ac8744
			.quad 0xbfe8400000b96bed, 0x3ffb0e0729fa6005
			.quad 0xbfe880000017a522, 0x3ffb33a2b85d048f
			.quad 0xbfe8c000008666ed, 0x3ffb59728e34f847
			.quad 0xbfe90000009231ee, 0x3ffb7f76f3527236
			.quad 0xbfe9400000998280, 0x3ffba5b030fcf4ad
			.quad 0xbfe9800000789634, 0x3ffbcc1e90945d34
			.quad 0xbfe9c00000464c5a, 0x3ffbf2c25c01acba
			.quad 0xbfea000000ac9edb, 0x3ffc199bddee6449
			.quad 0xbfea4000009db0ed, 0x3ffc40ab60605147
			.quad 0xbfea800000b25f74, 0x3ffc67f12ec591f4
			.quad 0xbfeac00000dd93d8, 0x3ffc8f6d948ffb53
			.quad 0xbfeb0000009a8e0d, 0x3ffcb720dd4fb272
			.quad 0xbfeb4000006c9216, 0x3ffcdf0b55a1a9bc
			.quad 0xbfeb80000029211c, 0x3ffd072d4a2165e4
			.quad 0xbfebc00000acf6b0, 0x3ffd2f87087ae250
			.quad 0xbfec000000c25639, 0x3ffd5818dd772ab4
			.quad 0xbfec400000c79407, 0x3ffd80e317490efb
			.quad 0xbfec8000001b543f, 0x3ffda9e603ecc1e4
			.quad 0xbfecc00000bac791, 0x3ffdd321f37a5ea0
			.quad 0xbfed000000264fb7, 0x3ffdfc9733947dd8
			.quad 0xbfed4000007cc76d, 0x3ffe264615471e42
			.quad 0xbfed8000006c7c5f, 0x3ffe502ee7d27b94
			.quad 0xbfedc00000504b69, 0x3ffe7a51fbfc4eb0
			.quad 0xbfee0000003582a0, 0x3ffea4afa2c81573
			.quad 0xbfee400000ef29b3, 0x3ffecf482e2e03c7
			.quad 0xbfee80000056b48b, 0x3ffefa1bee9b87c4
			.quad 0xbfeec00000144453, 0x3fff252b377966cc
			.quad 0xbfef0000002820d1, 0x3fff50765b897d3f
			.quad 0xbfef400000dcd2f6, 0x3fff7bfdae3356ec
			.quad 0xbfef800000132d42, 0x3fffa7c181abb707
			.quad 0xbfefc00000cf5c89, 0x3fffd3c22c1e669f

// Minimax polynomial coefficients constructed by Ali Sazegari
small_poly: .quad 0x3fc5555555555555
			.quad 0x3f81111111111111
			.quad 0x3f2a01a01a0196ae
			.quad 0x3ec71de3a5aa7055
			.quad 0x3e5ae642c86bfbc7
			.quad 0x3de61bfa2e91919a

.literal8
.align 3
one_n7:		.quad 0x3f80000000000000
lge_p7:		.quad 0x40671547652b82fe
lge_hi:		.quad 0x3ff7154760000000
lge_lo:		.quad 0x3e54ae0bf85ddf44
one_p55:	.quad 0x4360000000000000
one_n55:	.quad 0x3c80000000000000
one:		.quad 0x3ff0000000000000
min_normal:	.quad 0x0010000000000000
huge_val:	.quad 0x7fefffffffffffff

.text
#if defined(__x86_64__)
	#define RELATIVE_ADDR(_a)	(_a)(%rip)
#else
	#define RELATIVE_ADDR(_a)	(_a)-sinh_body(%ecx)
	
.align 4
sinh_pic:
	movl		(%esp),			%ecx
	ret
#endif

ENTRY(sinh)
#if defined(__x86_64__)
	movapd		%xmm0,			%xmm1
	psrlq		$32,			%xmm0
	movd		%xmm0,			%eax
#else  // arch = i386
	movl  4+FRAME_SIZE(STACKP),	%eax
	movsd	FRAME_SIZE(STACKP), %xmm1
	calll		sinh_pic
sinh_body:
#endif

	pcmpeqd		%xmm0,			%xmm0
	andl		$0x7fffffff,	%eax
	psrlq		$1,				%xmm0
	subl		$0x3fd62e42,	%eax
	cmpl		$0x005ed1bd,	%eax	// If (|x| < .34657... or |x| > 21 or isnan(x))
	ja			1f						//    goto 1
	
	/* If .3465.... < |x| < 21:
	 *
	 *	1. compute the head-tail product
	 *			
	 *			scaled_hi + scaled_lo = lg_e * |x|		with relative error < 2^-80
	 *
	 *	2. set n = floor(lg_e * |x|)
	 *
	 *	3. set a = top 7 bits of the fractional part of lg_e * |x|
	 *
	 *	4. use a to lookup to find f_hi and g_hi such that
	 *
	 *			|x| * lg_e = n + f_hi + f_lo			with relative error < 2^-60 and -2^-24 < f_lo < 2^-7 + tiny
	 *			|x| * lg_e = -n-1 + g_hi + g_lo			with relative error < 2^-60 and -2^-24 < g_lo < 2^-7 + tiny
	 *
	 *	5. evaluate a minimax polynomial p(x) ~ 2^x at f_lo and g_lo
	 *
	 *	6. lookup b = 2^f_hi and c = 2^g_hi with relative error < 2^-75
	 *
	 *	7. if n < 16, split c into c_head and c_tail with c_head 21 bits wide.
	 *     if n >= 16, then c_head = 0 and c_tail = c
	 *
	 *	8. Assemble the final result:
	 *
	 *			(2^(n-1)b - 2^(-n-2)c_head) + ((2^(n-1)b * f_lo)p(f_lo) - (2^(-n-2)c * g_lo)p(g_lo) + c_tail)
	 *
	 *	9. The first subtraction is exact.  The rounding point of the other subtractions is at least 6 bits to the
	 *		right of the rounding point of the final result.  The two summands are therefore correct to within 2^-5 ulps
	 *      of the final result, so the error is bounded by ~ .53 ulps.
	 *
	 *  The worst case errors occur in the binade from .25 - .5, as this is where b and c are of comparable magnitude.
	 *     
	 */
	
	movsd	RELATIVE_ADDR(lge_p7),	%xmm3
	movapd		%xmm0,			%xmm2	// xmm2	<-- 0x7fffffff....ffff
	andpd		%xmm1,			%xmm0	// xmm0 <-- |x|
	mulsd		%xmm0,			%xmm3	// xmm3 <-- lg(e) * |x| * 128.0
	psllq		$27,			%xmm2	// xmm2 <-- 0xfffffffff8000000 (mask for the high 26 mantissa bits)
	xorpd		%xmm0,			%xmm1	// xmm1 <-- signbit(x)
	cvttsd2si	%xmm3,			%eax	// %eax <-- (int)(lg(e) * |x| * 128.0)
	movlhps		%xmm1,			%xmm1
	movl		%eax,			%edx
	and			$0x7f,			AX_P
	shrl		$7,				%edx	// %edx <-- n = (int)(lg(e) * |x|)
	shll		$4,				%eax
	
	/* Needed values are now:
	     xmm0 - |x|
		 xmm1 - signbit(x)
		 xmm2 - 0xfffffffff8000000
		 xmm3 - lg(e) * |x| * 0x1.0p7
		 %eax - lookup value from top 7 bits of the fractional part of |x| * lg_e
		 %edx - n = (int)(|x| * lg_e)
	   Everything else is fair game. */
	
	mulsd	RELATIVE_ADDR(one_n7),	%xmm3	// xmm3 <-- scaled_hi = lg(e) * |x|
	andpd		%xmm0,			%xmm2	// xmm2 <-- x_hi
	subsd		%xmm2,			%xmm0	// xmm0 <-- x_lo
	movsd	RELATIVE_ADDR(lge_hi),	%xmm4
	movsd	RELATIVE_ADDR(lge_lo),	%xmm5
	movapd		%xmm2,			%xmm6	// xmm6 <-- x_hi
	mulsd		%xmm4,			%xmm2	// xmm2	<-- x_hi * lge_hi
	mulsd		%xmm0,			%xmm4	// xmm4 <-- x_lo * lge_hi
	mulsd		%xmm5,			%xmm6	// xmm6 <-- x_hi * lge_lo
	mulsd		%xmm5,			%xmm0	// xmm0 <-- x_lo * lge_lo
	subsd		%xmm3,			%xmm2	// xmm2 <--    x_hi*lge_hi - x*lge
	addsd		%xmm4,			%xmm2	// xmm2 <--   (x_hi*lge_hi - x*lge) + x_lo*lge_hi
	addsd		%xmm6,			%xmm2	// xmm2 <--  ((x_hi*lge_hi - x*lge) + x_lo*lge_hi) + x_hi*lge_lo
	addsd		%xmm0,			%xmm2	// xmm2 <-- (((x_hi*lge_hi - x*lge) + x_lo*lge_hi) + x_hi*lge_lo) + x_lo*lge_lo = scaled_lo
	
	/* Needed values are now:
	     xmm1 - signbit of x
		 xmm2 - low part of |x| * lg_e
		 xmm3 - high part of |x| * lg_e
		 %eax - lookup value from top 7 bits of the fractional part of |x| * lg_e
		 %edx - n = (int)(|x| * lg_e)
	   Everything else is fair game. */
	
	cvtsi2sd	%edx,			%xmm4	// xmm4 <-- (double)n
	subsd		%xmm4,			%xmm3	// xmm3 <-- frac = scaled_hi - (double)n
	lea		RELATIVE_ADDR(exp2table),	CX_P
	movapd		(CX_P,AX_P,1),	%xmm5	// xmm5 <-- -f_hi, exp2(f_hi)
	negl		%eax
	addl		$0x7f0,			%eax
	movapd		(CX_P,AX_P,1),	%xmm7	// xmm7 <-- -g_hi, exp2(g_hi)
	addsd		%xmm3,			%xmm5	// xmm5 <-- frac - f_hi, exp2(f_hi)
	subsd		8(CX_P),		%xmm3	// xmm3 <-- frac - 1.0
	addsd		%xmm2,			%xmm5	// xmm5 <-- f_lo = (frac - f_hi) + scaled_lo, exp2(f_hi)
	subsd		%xmm3,			%xmm7	// xmm7 <-- (1.0 - frac) - g_hi, exp2(g_hi)
	subsd		%xmm2,			%xmm7	// xmm7 <-- g_lo = ((1.0 - frac) - g_hi) - scaled_lo, exp2(g_hi)
	
	movapd		%xmm7,			%xmm0
	shufpd	$3,	%xmm5,			%xmm7	// xmm7 <-- exp2(g_hi), exp2(f_hi)
	shufpd  $0, %xmm5,			%xmm0	// xmm0 <-- g_lo, f_lo
	orpd		%xmm7,			%xmm1	// xmm1 <-- signbit*exp2(g_hi), signbit*exp2(f_hi)
	
	/* Needed values are now:
		 xmm0 - g_lo, f_lo
		 xmm1 - signed exp2(g_hi), signed exp2(f_hi)
		 %edx - n
	   Everything else is fair game. */
	   
	// put [2^(-n-2), 2^(n-1)] into xmm7
	movl		%edx,			%eax
	addl		$(1023 - 1),	%edx
	movd		%edx,			%xmm6
	movl		$(1023 - 2),	%edx
	subl		%eax,			%edx
	movd		%edx,			%xmm7
	movlhps		%xmm6,			%xmm7
	psllq		$52,			%xmm7
	
	// For the purposes of commenting polynomial evaluation, "x" = [f_lo, g_lo]
	movapd		%xmm0,			%xmm2
	mulpd		%xmm0,			%xmm0	// xmm0 <-- x * x
	movapd		%xmm2,			%xmm3
	mulpd		-64(CX_P),		%xmm2	// xmm2 <-- c3/c4 * x
	movapd		%xmm3,			%xmm4	// xmm4 <-- x
	mulpd		-32(CX_P),		%xmm3	// xmm3 <-- c1 * x
	movapd		%xmm0,			%xmm5
	mulpd		-80(CX_P),		%xmm0	// xmm0 <-- c4 * xx
	addpd		%xmm5,			%xmm2	// xmm2 <-- xx + c3x/c4
	mulpd		-48(CX_P),		%xmm5	// xmm5 <-- c2 * xx
	addpd		-16(CX_P),		%xmm3	// xmm3 <-- c1x + c0
	mulpd		%xmm0,			%xmm2	// xmm2 <-- c4xx * (xx + c3x/c4)		xmm0 freed
	addpd		%xmm5,			%xmm3	// xmm3 <-- c2xx + (c1x + c0)			xmm5 freed
	addpd		%xmm3,			%xmm2	// xmm2 <-- exp2poly(x)					xmm3 freed
	
	// Compute the mask to separate the high and low parts of exp2(-n-2 + g_hi)
	movl		$16,			%edx
	movd		%eax,			%xmm6	// xmm6 <-- n
	movd		%edx,			%xmm5	// xmm5 <-- 16
	pcmpgtd		%xmm6,			%xmm5	// xmm5 = (n < 16) ? [ 0,  0,  0, -1] : [ 0,  0,  0,  0]
	psllq		$32,			%xmm5	// xmm5 = (n < 16) ? [ 0,  0, -1,  0] : [ 0,  0,  0,  0]
	
	// multiply the scale factors in xmm7 into [exp2(g_hi), exp2(f_hi)]
	mulpd		%xmm7,			%xmm1	// xmm1 <-- exp2(-n-2 + g_hi), exp2(n-1 + f_hi)
	// multiply the scaled results into [g_lo, f_lo]
	mulpd		%xmm1,			%xmm4
	// multiply that result into the polynomial
	mulpd		%xmm4,			%xmm2	// xmm2 <-- g_lo*exp2(-n-2 + g_hi) * g_lo_poly, f_lo*exp2(n-1 + f_hi) * f_lo_poly
	
	movhlps		%xmm1,			%xmm0	// xmm0 <-- exp2(n-1 + f_hi)
	movhlps		%xmm2,			%xmm3	// xmm3 <-- f_lo*exp2(n-1 + f_hi) * f_lo_poly
	
	andpd		%xmm1,			%xmm5	// xmm5 <-- high part of exp2(-n-2 + g_hi)
	subsd		%xmm5,			%xmm1	// xmm1 <--  low part of exp2(-n-2 + g_hi)

	addsd		%xmm1,			%xmm2	// result_tail = g_lo*exp2(-n-2 + g_hi) * g_lo_poly + low part of exp2(-n-2 + g_hi)
	subsd		%xmm2,			%xmm3	// result_tail = f_lo*exp2(n-1 + f_hi) * f_lo_poly - result_tail
	subsd		%xmm5,			%xmm0	// result_head = exp2(n-1 + f_hi) - high part of exp2(-n-2 + g_hi)
	addsd		%xmm3,			%xmm0	// result = result_head + result_tail
	
#if defined(__i386__)
	movsd		%xmm0,			FRAME_SIZE(STACKP)
	fldl		FRAME_SIZE(STACKP)
#endif
	ret
	
1:
	jg			4f						// If |x| > 21 or isnan(x), goto 4
	cmpl		$0xfe69d1be,	%eax	// If |x| <= 2^-27
	jbe			2f
	
	// If .3465.... >= |x| > 2^-27, we use a minimax polynomial approximation with error < .53 ulps
	movapd		%xmm1,			%xmm0	// xmm0 <-- x
	mulsd		%xmm1,			%xmm1	// xmm1 <-- u = xx
	lea		RELATIVE_ADDR(small_poly),	AX_P
	
	// Horner's is needed for numerics reasons, and the performance is actually pretty good.
	movapd		%xmm1,			%xmm2
	mulsd		40(AX_P),		%xmm1
	movapd		%xmm2,			%xmm3
	mulsd		%xmm0,			%xmm2
	addsd		32(AX_P),		%xmm1
	mulsd		%xmm3,			%xmm1
	addsd		24(AX_P),		%xmm1
	mulsd		%xmm3,			%xmm1
	addsd		16(AX_P),		%xmm1
	mulsd		%xmm3,			%xmm1
	addsd		8(AX_P),		%xmm1
	mulsd		%xmm3,			%xmm1
	addsd		(AX_P),			%xmm1
	mulsd		%xmm2,			%xmm1
	addsd		%xmm1,			%xmm0

#if defined(__i386__)
	movsd		%xmm0,			FRAME_SIZE(STACKP)
	fldl		FRAME_SIZE(STACKP)
#endif
	ret

2:
	movsd	RELATIVE_ADDR(one),	%xmm2	// xmm2 <-- 1.0
	andpd		%xmm1,			%xmm0	// xmm0 <-- |x|
	xorpd		%xmm0,			%xmm1	// xmm1 <-- signbit(x)
	
	cmpl		$0xc039d1be,	%eax	// If x is normal
	jae			3f						// goto 3
	
	// Denormal handling.  Zero passes through here just fine.
	orpd		%xmm2,			%xmm0	// xmm0 <-- 1.0 | x
	subsd		%xmm2,			%xmm0	// xmm1 <-- 1.0 | x - 1.0 = 0x1p1022 * x
	movsd	RELATIVE_ADDR(min_normal), %xmm2	// xmm0 <-- 0x1p-1022 denormal "bias"
	
3:
	orpd		%xmm1,			%xmm0	// restore the signbit
	movapd		%xmm0,			%xmm1
	mulsd	RELATIVE_ADDR(one_p55),	%xmm0
	addsd		%xmm1,			%xmm0
	mulsd	RELATIVE_ADDR(one_n55),	%xmm0		// x += 0x1p-55*x,  for inexact and rounding (no underflow)
	mulsd		%xmm2,			%xmm0	// multiply in pseudo bias if x is denormal, 1.0 otherwise.
	
#if defined(__i386__)
	movsd		%xmm0,			FRAME_SIZE(STACKP)
	fldl		FRAME_SIZE(STACKP)
#endif
	ret
	
4:
	cmpl		$0x00b009be,	%eax	// If |x| > 711 or isnan(x)
	jae			5f
	
	// Here 21.0 < |x| < 711, and sinh(x) = exp(x) / 2.0
	
	movsd	RELATIVE_ADDR(lge_p7),	%xmm3
	movapd		%xmm0,			%xmm2	// xmm2	<-- 0x7fffffff....ffff
	andpd		%xmm1,			%xmm0	// xmm0 <-- |x|
	mulsd		%xmm0,			%xmm3	// xmm3 <-- lg(e) * |x| * 128.0
	psllq		$27,			%xmm2	// xmm2 <-- 0xfffffffff8000000 (mask for the high 26 mantissa bits)
	xorpd		%xmm0,			%xmm1	// xmm1 <-- signbit(x)
	cvttsd2si	%xmm3,			%eax	// %eax <-- (int)(lg(e) * |x| * 128.0)
	movl		%eax,			%edx
	and			$0x7f,			AX_P
	shrl		$7,				%edx	// %edx <-- n = (int)(lg(e) * |x|)
	shll		$4,				%eax
	
	/* Needed values are now:
	     xmm0 - |x|
		 xmm1 - signbit(x)
		 xmm2 - 0xfffffffff8000000
		 xmm3 - lg(e) * |x| * 0x1.0p7
		 %eax - lookup value from top 7 bits of the fractional part of |x| * lg_e
		 %edx - n = (int)(|x| * lg_e)
	   Everything else is fair game. */
	
	mulsd	RELATIVE_ADDR(one_n7),	%xmm3	// xmm3 <-- scaled_hi = lg(e) * |x|
	andpd		%xmm0,			%xmm2	// xmm2 <-- x_hi
	subsd		%xmm2,			%xmm0	// xmm0 <-- x_lo
	movsd	RELATIVE_ADDR(lge_hi),	%xmm4
	movsd	RELATIVE_ADDR(lge_lo),	%xmm5
	movapd		%xmm2,			%xmm6	// xmm6 <-- x_hi
	mulsd		%xmm4,			%xmm2	// xmm2	<-- x_hi * lge_hi
	mulsd		%xmm0,			%xmm4	// xmm4 <-- x_lo * lge_hi
	mulsd		%xmm5,			%xmm6	// xmm6 <-- x_hi * lge_lo
	mulsd		%xmm5,			%xmm0	// xmm0 <-- x_lo * lge_lo
	subsd		%xmm3,			%xmm2	// xmm2 <--    x_hi*lge_hi - x*lge
	addsd		%xmm4,			%xmm2	// xmm2 <--   (x_hi*lge_hi - x*lge) + x_lo*lge_hi
	addsd		%xmm6,			%xmm2	// xmm2 <--  ((x_hi*lge_hi - x*lge) + x_lo*lge_hi) + x_hi*lge_lo
	addsd		%xmm0,			%xmm2	// xmm2 <-- (((x_hi*lge_hi - x*lge) + x_lo*lge_hi) + x_hi*lge_lo) + x_lo*lge_lo = scaled_lo
	
	/* Needed values are now:
	     xmm1 - signbit of x
		 xmm2 - low part of |x| * lg_e
		 xmm3 - high part of |x| * lg_e
		 %eax - lookup value from top 7 bits of the fractional part of |x| * lg_e
		 %edx - n = (int)(|x| * lg_e)
	   Everything else is fair game. */
	
	cvtsi2sd	%edx,			%xmm4	// xmm4 <-- (double)n
	subsd		%xmm4,			%xmm3	// xmm3 <-- frac = scaled_hi - (double)n
	lea		RELATIVE_ADDR(exp2table),	CX_P
	movsd		(CX_P,AX_P,1),	%xmm0	// xmm5 <-- -f_hi
	movsd		8(CX_P,AX_P,1),	%xmm6	// xmm6 <-- exp2(f_hi)
	addsd		%xmm3,			%xmm0	// xmm5 <-- frac - f_hi
	addsd		%xmm2,			%xmm0	// xmm5 <-- f_lo = (frac - f_hi) + scaled_lo
	orpd		%xmm6,			%xmm1	// xmm1 <-- signbit * exp2(f_hi)
	
	/* Needed values are now:
		 xmm0 - f_lo
		 xmm1 - signed exp2(f_hi)
		 %edx - n
	   Everything else is fair game. */
	
	// put 2^(n-2) into xmm7
	addl		$(1023 - 2),	%edx
	movd		%edx,			%xmm7
	psllq		$52,			%xmm7   // exp2(n-2)
	
	// polynomial in f_lo
	movapd		%xmm0,			%xmm2
	mulsd		%xmm0,			%xmm0	// xmm0 <-- x * x
	movapd		%xmm2,			%xmm3
	mulsd		-64(CX_P),		%xmm2	// xmm2 <-- c3/c4 * x
	movapd		%xmm3,			%xmm4	// xmm4 <-- x
	mulsd		-32(CX_P),		%xmm3	// xmm3 <-- c1 * x

    movl        $0x400,         %eax
    movd        %eax,           %xmm6
    psllq       $52,            %xmm6   // 2.0
    mulsd       %xmm6,          %xmm1   // high = signbit * exp2(1 + f_hi)
    
	movapd		%xmm0,			%xmm5
	mulsd		-80(CX_P),		%xmm0	// xmm0 <-- c4 * xx
	addsd		%xmm5,			%xmm2	// xmm2 <-- xx + c3x/c4
	mulsd		-48(CX_P),		%xmm5	// xmm5 <-- c2 * xx
	addsd		-16(CX_P),		%xmm3	// xmm3 <-- c1x + c0
	mulsd		%xmm2,			%xmm0	// xmm0 <-- c4xx * (xx + c3x/c4)		xmm0 freed
	addsd		%xmm5,			%xmm3	// xmm3 <-- c2xx + (c1x + c0)			xmm5 freed
	addsd		%xmm3,			%xmm0	// xmm0 <-- exp2poly(x)					xmm3 freed
    
	mulsd		%xmm1,			%xmm4	// xmm4 <-- high * f_lo
	mulsd		%xmm4,			%xmm0	// xmm2 <-- high * f_lo * poly(f_lo)
	addsd		%xmm1,			%xmm0	// xmm0 <-- high + high * f_lo * poly(f_lo)
    mulsd       %xmm7,          %xmm0   // scale result by 2**(n - 2)
	
#if defined(__i386__)
	movsd		%xmm0,			FRAME_SIZE(STACKP)
	fldl		FRAME_SIZE(STACKP)
#endif
	ret
	
5:
	movsd	RELATIVE_ADDR(huge_val),	%xmm0
	mulsd		%xmm1,			%xmm0	// quiet NaNs, generate overflow for finite values
#if defined(__i386__)
	movsd		%xmm0,			FRAME_SIZE(STACKP)
	fldl		FRAME_SIZE(STACKP)
#endif
	ret
