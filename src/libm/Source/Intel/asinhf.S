/*
 * asinhf.s
 *
 *      by Stephen Canon
 *
 * Copyright (c) 2007, Apple Inc.  All Rights Reserved.
 *
 * This file implements the C99 asinhf function for the MacOS X __i386__ and __x86_64__ ABIs.
 */

/*
 This code divides the floating-point numbers into three ranges, and uses a different approximation
 to asinhf on each range.
 
  For the "small" case, |x| < 1/4, a simpler approximation by a 7th-order polynomial suffices:
 
    asinhf(x) = copysign(x) p(x)
		
	p(x) has no constant term, and is computed via the factorization:
	
		p(x) = cx(x + a)(x + b)(x^2 + cx + d)(x^2 + ex + f)
		
	the two linear terms and the two quadratic terms are computed side-by-side in packed arithmetic.
	Some care must be taken to avoid setting inexact in the product (x+b)(x^2 + ex + f) for the case
	when x = 0, as b and f are full-width double-precision numbers.
	

  The "middle" case is 1/4 < |x| < 4.  Here a straightforward rational polynomial approximation is used:
                               p(x)
		asinhf(x) = copysign(x) ----------
                               q(x)
	p(x) and q(x) are both order 6 polynomials, so they are computed side-by-side in packed arithmetic.
	

	The "large" case is the most complicated, owing to the fact that asinh(|x|) = log(2|x|) + O(1/x^2)
	for large x. Because of this, we take a divide and compute a logarithm (via polynomial approximation).
	The logarithm code is copied from the file logf.s (Ians code), though some registers have been renamed,
	so take care in making edits.  A sketch of the algorithm for the log is provided where it is used in
	this code, but more detailed comments may be found in logf.s.
	
	After the log, a minimax polynomial in 1/x is used as a correction.
	
	The "large" codepath is probably most suceptable to improvement - it is possible that a rational
	approximation would prove faster (in terms of latency) for this case.  The "small" and "middle"
	code paths are believed to be more nearly optimal.

	Globally, the error of this code is < .51 ulps
	
	- stc (14 june, 2007)
 */

#include <machine/asm.h>
#include "abi.h"

////.const

// Coefficients for 7th order polynomial approximation on [0, 1/4]
// The polynomail is computed in packed factored form as follows:
// 
//     hi double:        (x + ahi)  * (x(x + b1hi) + b0hi)
//     lo double:  (cx * (x + alo)) * (x(x + b1lo) + b0lo)
//
// The high and low parts are then unpacked and multiplied.
.align (1<<4)
asinhf_low: .quad 0x4008183efcaf7119, 0x4007eba0a6c21cf1 // b0hi, b0lo
						.quad 0x3ffe9547f4507ace, 0xbffd2c173c2ad586 // ahi, alo
						.quad 0x40021aa6afb79159, 0xc0015e2ed556dde5 // b1hi, b1lo
						.quad 0xbfa050433953b0f9                  // c

// Coefficients for rational approximation on [1/4, 4]
// p(x) and q(x) are computed side-by-side in packed arithmetic, then unpacked and divided.
.align (1<<4)
asinhf_mid:	.quad 0x3e4328ccef61bd30,	0x3f80f6f9cf323b3c	// p[0], q[0]
						.quad 0x3f80f6e561f06785,	0x3f85b29a3e277523	// p[1], q[1]
						.quad 0x3f85b32a2f11b40a, 0x3f8e26416e925090	// p[2], q[2]
						.quad 0x3f8b5062b37338e9,	0x3f8417abb8190857	// p[3], q[3]
						.quad 0x3f807d5f61f237fc, 0x3f74ccecc3a9e525	// p[4], q[4]
						.quad 0x3f6a9bd617bd81f1,	0x3f5135659e34c549	// p[5], q[5]
						.quad 0x3f26c17c7b263d18,	0x3f001e11059bddca	// p[6], q[6]
						

.align (1<<4)
// Polynomial coefficients for the correction to log(2x) for the "large" case.
// The polynomial is computed in factored form as follows:
//
//          cx(x + a)(x^2 + b1hi x + b0hi)(x^2 + b1lo x + b0lo)
//
// The two quadratic terms are computed with packed arithmetic.
                 .quad 0x4005413ddf1d2ea5, 0x4003fbe28981b6b8 // b0hi, b0lo
                 .quad 0x4006dd1e92567458, 0xc0057cccb680cfdf // b1hi, b1lo
                 .quad 0x3e9564f782290c65, 0x3fa3493848c3ecc6 // a, c
// Table used to compute log(2x) for the "large" case.  Copied from logf.s
asinhf_hi_table: .quad	0x0000000000000000,	0x3ff0000000000000	// log(1.00000), 1/1.00000
                 .quad	0x3f6ff00aa2b10bc0,	0x3fefe01fe01fe020	// log(1.00391), 1/1.00391
                 .quad	0x3f7fe02a6b106789,	0x3fefc07f01fc07f0	// log(1.00781), 1/1.00781
                 .quad	0x3f87dc475f810a77,	0x3fefa11caa01fa12	// log(1.01172), 1/1.01172
                 .quad	0x3f8fc0a8b0fc03e4,	0x3fef81f81f81f820	// log(1.01562), 1/1.01562
                 .quad	0x3f93cea44346a575,	0x3fef6310aca0dbb5	// log(1.01953), 1/1.01953
                 .quad	0x3f97b91b07d5b11b,	0x3fef44659e4a4271	// log(1.02344), 1/1.02344
                 .quad	0x3f9b9fc027af9198,	0x3fef25f644230ab5	// log(1.02734), 1/1.02734
                 .quad	0x3f9f829b0e783300,	0x3fef07c1f07c1f08	// log(1.03125), 1/1.03125
                 .quad	0x3fa1b0d98923d980,	0x3feee9c7f8458e02	// log(1.03516), 1/1.03516
                 .quad	0x3fa39e87b9febd60,	0x3feecc07b301ecc0	// log(1.03906), 1/1.03906
                 .quad	0x3fa58a5bafc8e4d5,	0x3feeae807aba01eb	// log(1.04297), 1/1.04297
                 .quad	0x3fa77458f632dcfc,	0x3fee9131abf0b767	// log(1.04688), 1/1.04688
                 .quad	0x3fa95c830ec8e3eb,	0x3fee741aa59750e4	// log(1.05078), 1/1.05078
                 .quad	0x3fab42dd711971bf,	0x3fee573ac901e574	// log(1.05469), 1/1.05469
                 .quad	0x3fad276b8adb0b52,	0x3fee3a9179dc1a73	// log(1.05859), 1/1.05859
                 .quad	0x3faf0a30c01162a6,	0x3fee1e1e1e1e1e1e	// log(1.06250), 1/1.06250
                 .quad	0x3fb075983598e471,	0x3fee01e01e01e01e	// log(1.06641), 1/1.06641
                 .quad	0x3fb16536eea37ae1,	0x3fede5d6e3f8868a	// log(1.07031), 1/1.07031
                 .quad	0x3fb253f62f0a1417,	0x3fedca01dca01dca	// log(1.07422), 1/1.07422
                 .quad	0x3fb341d7961bd1d1,	0x3fedae6076b981db	// log(1.07812), 1/1.07812
                 .quad	0x3fb42edcbea646f0,	0x3fed92f2231e7f8a	// log(1.08203), 1/1.08203
                 .quad	0x3fb51b073f06183f,	0x3fed77b654b82c34	// log(1.08594), 1/1.08594
                 .quad	0x3fb60658a93750c4,	0x3fed5cac807572b2	// log(1.08984), 1/1.08984
                 .quad	0x3fb6f0d28ae56b4c,	0x3fed41d41d41d41d	// log(1.09375), 1/1.09375
                 .quad	0x3fb7da766d7b12cd,	0x3fed272ca3fc5b1a	// log(1.09766), 1/1.09766
                 .quad	0x3fb8c345d6319b21,	0x3fed0cb58f6ec074	// log(1.10156), 1/1.10156
                 .quad	0x3fb9ab42462033ad,	0x3fecf26e5c44bfc6	// log(1.10547), 1/1.10547
                 .quad	0x3fba926d3a4ad563,	0x3fecd85689039b0b	// log(1.10938), 1/1.10938
                 .quad	0x3fbb78c82bb0eda1,	0x3fecbe6d9601cbe7	// log(1.11328), 1/1.11328
                 .quad	0x3fbc5e548f5bc743,	0x3feca4b3055ee191	// log(1.11719), 1/1.11719
                 .quad	0x3fbd4313d66cb35d,	0x3fec8b265afb8a42	// log(1.12109), 1/1.12109
                 .quad	0x3fbe27076e2af2e6,	0x3fec71c71c71c71c	// log(1.12500), 1/1.12500
                 .quad	0x3fbf0a30c01162a6,	0x3fec5894d10d4986	// log(1.12891), 1/1.12891
                 .quad	0x3fbfec9131dbeabb,	0x3fec3f8f01c3f8f0	// log(1.13281), 1/1.13281
                 .quad	0x3fc0671512ca596e,	0x3fec26b5392ea01c	// log(1.13672), 1/1.13672
                 .quad	0x3fc0d77e7cd08e59,	0x3fec0e070381c0e0	// log(1.14062), 1/1.14062
                 .quad	0x3fc14785846742ac,	0x3febf583ee868d8b	// log(1.14453), 1/1.14453
                 .quad	0x3fc1b72ad52f67a0,	0x3febdd2b899406f7	// log(1.14844), 1/1.14844
                 .quad	0x3fc2266f190a5acb,	0x3febc4fd65883e7b	// log(1.15234), 1/1.15234
                 .quad	0x3fc29552f81ff523,	0x3febacf914c1bad0	// log(1.15625), 1/1.15625
                 .quad	0x3fc303d718e47fd3,	0x3feb951e2b18ff23	// log(1.16016), 1/1.16016
                 .quad	0x3fc371fc201e8f74,	0x3feb7d6c3dda338b	// log(1.16406), 1/1.16406
                 .quad	0x3fc3dfc2b0ecc62a,	0x3feb65e2e3beee05	// log(1.16797), 1/1.16797
                 .quad	0x3fc44d2b6ccb7d1e,	0x3feb4e81b4e81b4f	// log(1.17188), 1/1.17188
                 .quad	0x3fc4ba36f39a55e5,	0x3feb37484ad806ce	// log(1.17578), 1/1.17578
                 .quad	0x3fc526e5e3a1b438,	0x3feb2036406c80d9	// log(1.17969), 1/1.17969
                 .quad	0x3fc59338d9982086,	0x3feb094b31d922a4	// log(1.18359), 1/1.18359
                 .quad	0x3fc5ff3070a793d4,	0x3feaf286bca1af28	// log(1.18750), 1/1.18750
                 .quad	0x3fc66acd4272ad51,	0x3feadbe87f94905e	// log(1.19141), 1/1.19141
                 .quad	0x3fc6d60fe719d21d,	0x3feac5701ac5701b	// log(1.19531), 1/1.19531
                 .quad	0x3fc740f8f54037a5,	0x3feaaf1d2f87ebfd	// log(1.19922), 1/1.19922
                 .quad	0x3fc7ab890210d909,	0x3fea98ef606a63be	// log(1.20312), 1/1.20312
                 .quad	0x3fc815c0a14357eb,	0x3fea82e65130e159	// log(1.20703), 1/1.20703
                 .quad	0x3fc87fa06520c911,	0x3fea6d01a6d01a6d	// log(1.21094), 1/1.21094
                 .quad	0x3fc8e928de886d41,	0x3fea574107688a4a	// log(1.21484), 1/1.21484
                 .quad	0x3fc9525a9cf456b4,	0x3fea41a41a41a41a	// log(1.21875), 1/1.21875
                 .quad	0x3fc9bb362e7dfb83,	0x3fea2c2a87c51ca0	// log(1.22266), 1/1.22266
                 .quad	0x3fca23bc1fe2b563,	0x3fea16d3f97a4b02	// log(1.22656), 1/1.22656
                 .quad	0x3fca8becfc882f19,	0x3fea01a01a01a01a	// log(1.23047), 1/1.23047
                 .quad	0x3fcaf3c94e80bff3,	0x3fe9ec8e951033d9	// log(1.23438), 1/1.23438
                 .quad	0x3fcb5b519e8fb5a4,	0x3fe9d79f176b682d	// log(1.23828), 1/1.23828
                 .quad	0x3fcbc286742d8cd6,	0x3fe9c2d14ee4a102	// log(1.24219), 1/1.24219
                 .quad	0x3fcc2968558c18c1,	0x3fe9ae24ea5510da	// log(1.24609), 1/1.24609
                 .quad	0x3fcc8ff7c79a9a22,	0x3fe999999999999a	// log(1.25000), 1/1.25000
                 .quad	0x3fccf6354e09c5dc,	0x3fe9852f0d8ec0ff	// log(1.25391), 1/1.25391
                 .quad	0x3fcd5c216b4fbb91,	0x3fe970e4f80cb872	// log(1.25781), 1/1.25781
                 .quad	0x3fcdc1bca0abec7d,	0x3fe95cbb0be377ae	// log(1.26172), 1/1.26172
                 .quad	0x3fce27076e2af2e6,	0x3fe948b0fcd6e9e0	// log(1.26562), 1/1.26562
                 .quad	0x3fce8c0252aa5a60,	0x3fe934c67f9b2ce6	// log(1.26953), 1/1.26953
                 .quad	0x3fcef0adcbdc5936,	0x3fe920fb49d0e229	// log(1.27344), 1/1.27344
                 .quad	0x3fcf550a564b7b37,	0x3fe90d4f120190d5	// log(1.27734), 1/1.27734
                 .quad	0x3fcfb9186d5e3e2b,	0x3fe8f9c18f9c18fa	// log(1.28125), 1/1.28125
                 .quad	0x3fd00e6c45ad501d,	0x3fe8e6527af1373f	// log(1.28516), 1/1.28516
                 .quad	0x3fd0402594b4d041,	0x3fe8d3018d3018d3	// log(1.28906), 1/1.28906
                 .quad	0x3fd071b85fcd590d,	0x3fe8bfce8062ff3a	// log(1.29297), 1/1.29297
                 .quad	0x3fd0a324e27390e3,	0x3fe8acb90f6bf3aa	// log(1.29688), 1/1.29688
                 .quad	0x3fd0d46b579ab74b,	0x3fe899c0f601899c	// log(1.30078), 1/1.30078
                 .quad	0x3fd1058bf9ae4ad5,	0x3fe886e5f0abb04a	// log(1.30469), 1/1.30469
                 .quad	0x3fd136870293a8b0,	0x3fe87427bcc092b9	// log(1.30859), 1/1.30859
                 .quad	0x3fd1675cababa60e,	0x3fe8618618618618	// log(1.31250), 1/1.31250
                 .quad	0x3fd1980d2dd4236f,	0x3fe84f00c2780614	// log(1.31641), 1/1.31641
                 .quad	0x3fd1c898c16999fb,	0x3fe83c977ab2bedd	// log(1.32031), 1/1.32031
                 .quad	0x3fd1f8ff9e48a2f3,	0x3fe82a4a0182a4a0	// log(1.32422), 1/1.32422
                 .quad	0x3fd22941fbcf7966,	0x3fe8181818181818	// log(1.32812), 1/1.32812
                 .quad	0x3fd2596010df763a,	0x3fe8060180601806	// log(1.33203), 1/1.33203
                 .quad	0x3fd2895a13de86a3,	0x3fe7f405fd017f40	// log(1.33594), 1/1.33594
                 .quad	0x3fd2b9303ab89d25,	0x3fe7e225515a4f1d	// log(1.33984), 1/1.33984
                 .quad	0x3fd2e8e2bae11d31,	0x3fe7d05f417d05f4	// log(1.34375), 1/1.34375
                 .quad	0x3fd31871c9544185,	0x3fe7beb3922e017c	// log(1.34766), 1/1.34766
                 .quad	0x3fd347dd9a987d55,	0x3fe7ad2208e0ecc3	// log(1.35156), 1/1.35156
                 .quad	0x3fd3772662bfd85b,	0x3fe79baa6bb6398b	// log(1.35547), 1/1.35547
                 .quad	0x3fd3a64c556945ea,	0x3fe78a4c8178a4c8	// log(1.35938), 1/1.35938
                 .quad	0x3fd3d54fa5c1f710,	0x3fe77908119ac60d	// log(1.36328), 1/1.36328
                 .quad	0x3fd404308686a7e4,	0x3fe767dce434a9b1	// log(1.36719), 1/1.36719
                 .quad	0x3fd432ef2a04e814,	0x3fe756cac201756d	// log(1.37109), 1/1.37109
                 .quad	0x3fd4618bc21c5ec2,	0x3fe745d1745d1746	// log(1.37500), 1/1.37500
                 .quad	0x3fd49006804009d1,	0x3fe734f0c541fe8d	// log(1.37891), 1/1.37891
                 .quad	0x3fd4be5f957778a1,	0x3fe724287f46debc	// log(1.38281), 1/1.38281
                 .quad	0x3fd4ec973260026a,	0x3fe713786d9c7c09	// log(1.38672), 1/1.38672
                 .quad	0x3fd51aad872df82d,	0x3fe702e05c0b8170	// log(1.39062), 1/1.39062
                 .quad	0x3fd548a2c3add263,	0x3fe6f26016f26017	// log(1.39453), 1/1.39453
                 .quad	0x3fd5767717455a6c,	0x3fe6e1f76b4337c7	// log(1.39844), 1/1.39844
                 .quad	0x3fd5a42ab0f4cfe2,	0x3fe6d1a62681c861	// log(1.40234), 1/1.40234
                 .quad	0x3fd5d1bdbf5809ca,	0x3fe6c16c16c16c17	// log(1.40625), 1/1.40625
                 .quad	0x3fd5ff3070a793d4,	0x3fe6b1490aa31a3d	// log(1.41016), 1/1.41016
                 .quad	0x3fd62c82f2b9c795,	0x3fe6a13cd1537290	// log(1.41406), 1/1.41406
                 .quad	0x3fd659b57303e1f3,	0x3fe691473a88d0c0	// log(1.41797), 1/1.41797
                 .quad	0x3fd686c81e9b14af,	0x3fe6816816816817	// log(1.42188), 1/1.42188
                 .quad	0x3fd6b3bb2235943e,	0x3fe6719f3601671a	// log(1.42578), 1/1.42578
                 .quad	0x3fd6e08eaa2ba1e4,	0x3fe661ec6a5122f9	// log(1.42969), 1/1.42969
                 .quad	0x3fd70d42e2789236,	0x3fe6524f853b4aa3	// log(1.43359), 1/1.43359
                 .quad	0x3fd739d7f6bbd007,	0x3fe642c8590b2164	// log(1.43750), 1/1.43750
                 .quad	0x3fd7664e1239dbcf,	0x3fe63356b88ac0de	// log(1.44141), 1/1.44141
                 .quad	0x3fd792a55fdd47a2,	0x3fe623fa77016240	// log(1.44531), 1/1.44531
                 .quad	0x3fd7bede0a37afc0,	0x3fe614b36831ae94	// log(1.44922), 1/1.44922
                 .quad	0x3fd7eaf83b82afc3,	0x3fe6058160581606	// log(1.45312), 1/1.45312
                 .quad	0x3fd816f41da0d496,	0x3fe5f66434292dfc	// log(1.45703), 1/1.45703
                 .quad	0x3fd842d1da1e8b17,	0x3fe5e75bb8d015e7	// log(1.46094), 1/1.46094
                 .quad	0x3fd86e919a330ba0,	0x3fe5d867c3ece2a5	// log(1.46484), 1/1.46484
                 .quad	0x3fd89a3386c1425b,	0x3fe5c9882b931057	// log(1.46875), 1/1.46875
                 .quad	0x3fd8c5b7c858b48b,	0x3fe5babcc647fa91	// log(1.47266), 1/1.47266
                 .quad	0x3fd8f11e873662c7,	0x3fe5ac056b015ac0	// log(1.47656), 1/1.47656
                 .quad	0x3fd91c67eb45a83e,	0x3fe59d61f123ccaa	// log(1.48047), 1/1.48047
                 .quad	0x3fd947941c2116fb,	0x3fe58ed2308158ed	// log(1.48438), 1/1.48438
                 .quad	0x3fd972a341135158,	0x3fe5805601580560	// log(1.48828), 1/1.48828
                 .quad	0x3fd99d958117e08b,	0x3fe571ed3c506b3a	// log(1.49219), 1/1.49219
                 .quad	0x3fd9c86b02dc0863,	0x3fe56397ba7c52e2	// log(1.49609), 1/1.49609
                 .quad	0x3fd9f323ecbf984c,	0x3fe5555555555555	// log(1.50000), 1/1.50000
                 .quad	0x3fda1dc064d5b995,	0x3fe54725e6bb82fe	// log(1.50391), 1/1.50391
                 .quad	0x3fda484090e5bb0a,	0x3fe5390948f40feb	// log(1.50781), 1/1.50781
                 .quad	0x3fda72a4966bd9ea,	0x3fe52aff56a8054b	// log(1.51172), 1/1.51172
                 .quad	0x3fda9cec9a9a084a,	0x3fe51d07eae2f815	// log(1.51562), 1/1.51562
                 .quad	0x3fdac718c258b0e4,	0x3fe50f22e111c4c5	// log(1.51953), 1/1.51953
                 .quad	0x3fdaf1293247786b,	0x3fe5015015015015	// log(1.52344), 1/1.52344
                 .quad	0x3fdb1b1e0ebdfc5b,	0x3fe4f38f62dd4c9b	// log(1.52734), 1/1.52734
                 .quad	0x3fdb44f77bcc8f63,	0x3fe4e5e0a72f0539	// log(1.53125), 1/1.53125
                 .quad	0x3fdb6eb59d3cf35e,	0x3fe4d843bedc2c4c	// log(1.53516), 1/1.53516
                 .quad	0x3fdb9858969310fb,	0x3fe4cab88725af6e	// log(1.53906), 1/1.53906
                 .quad	0x3fdbc1e08b0dad0a,	0x3fe4bd3edda68fe1	// log(1.54297), 1/1.54297
                 .quad	0x3fdbeb4d9da71b7c,	0x3fe4afd6a052bf5b	// log(1.54688), 1/1.54688
                 .quad	0x3fdc149ff115f027,	0x3fe4a27fad76014a	// log(1.55078), 1/1.55078
                 .quad	0x3fdc3dd7a7cdad4d,	0x3fe49539e3b2d067	// log(1.55469), 1/1.55469
                 .quad	0x3fdc66f4e3ff6ff8,	0x3fe4880522014880	// log(1.55859), 1/1.55859
                 .quad	0x3fdc8ff7c79a9a22,	0x3fe47ae147ae147b	// log(1.56250), 1/1.56250
                 .quad	0x3fdcb8e0744d7aca,	0x3fe46dce34596066	// log(1.56641), 1/1.56641
                 .quad	0x3fdce1af0b85f3eb,	0x3fe460cbc7f5cf9a	// log(1.57031), 1/1.57031
                 .quad	0x3fdd0a63ae721e64,	0x3fe453d9e2c776ca	// log(1.57422), 1/1.57422
                 .quad	0x3fdd32fe7e00ebd5,	0x3fe446f86562d9fb	// log(1.57812), 1/1.57812
                 .quad	0x3fdd5b7f9ae2c684,	0x3fe43a2730abee4d	// log(1.58203), 1/1.58203
                 .quad	0x3fdd83e7258a2f3e,	0x3fe42d6625d51f87	// log(1.58594), 1/1.58594
                 .quad	0x3fddac353e2c5954,	0x3fe420b5265e5951	// log(1.58984), 1/1.58984
                 .quad	0x3fddd46a04c1c4a1,	0x3fe4141414141414	// log(1.59375), 1/1.59375
                 .quad	0x3fddfc859906d5b5,	0x3fe40782d10e6566	// log(1.59766), 1/1.59766
                 .quad	0x3fde24881a7c6c26,	0x3fe3fb013fb013fb	// log(1.60156), 1/1.60156
                 .quad	0x3fde4c71a8687704,	0x3fe3ee8f42a5af07	// log(1.60547), 1/1.60547
                 .quad	0x3fde744261d68788,	0x3fe3e22cbce4a902	// log(1.60938), 1/1.60938
                 .quad	0x3fde9bfa659861f5,	0x3fe3d5d991aa75c6	// log(1.61328), 1/1.61328
                 .quad	0x3fdec399d2468cc0,	0x3fe3c995a47babe7	// log(1.61719), 1/1.61719
                 .quad	0x3fdeeb20c640ddf4,	0x3fe3bd60d9232955	// log(1.62109), 1/1.62109
                 .quad	0x3fdf128f5faf06ed,	0x3fe3b13b13b13b14	// log(1.62500), 1/1.62500
                 .quad	0x3fdf39e5bc811e5c,	0x3fe3a524387ac822	// log(1.62891), 1/1.62891
                 .quad	0x3fdf6123fa7028ac,	0x3fe3991c2c187f63	// log(1.63281), 1/1.63281
                 .quad	0x3fdf884a36fe9ec2,	0x3fe38d22d366088e	// log(1.63672), 1/1.63672
                 .quad	0x3fdfaf588f78f31f,	0x3fe3813813813814	// log(1.64062), 1/1.64062
                 .quad	0x3fdfd64f20f61572,	0x3fe3755bd1c945ee	// log(1.64453), 1/1.64453
                 .quad	0x3fdffd2e0857f498,	0x3fe3698df3de0748	// log(1.64844), 1/1.64844
                 .quad	0x3fe011fab125ff8a,	0x3fe35dce5f9f2af8	// log(1.65234), 1/1.65234
                 .quad	0x3fe02552a5a5d0ff,	0x3fe3521cfb2b78c1	// log(1.65625), 1/1.65625
                 .quad	0x3fe0389eefce633b,	0x3fe34679ace01346	// log(1.66016), 1/1.66016
                 .quad	0x3fe04bdf9da926d2,	0x3fe33ae45b57bcb2	// log(1.66406), 1/1.66406
                 .quad	0x3fe05f14bd26459c,	0x3fe32f5ced6a1dfa	// log(1.66797), 1/1.66797
                 .quad	0x3fe0723e5c1cdf40,	0x3fe323e34a2b10bf	// log(1.67188), 1/1.67188
                 .quad	0x3fe0855c884b450e,	0x3fe3187758e9ebb6	// log(1.67578), 1/1.67578
                 .quad	0x3fe0986f4f573521,	0x3fe30d190130d190	// log(1.67969), 1/1.67969
                 .quad	0x3fe0ab76bece14d2,	0x3fe301c82ac40260	// log(1.68359), 1/1.68359
                 .quad	0x3fe0be72e4252a83,	0x3fe2f684bda12f68	// log(1.68750), 1/1.68750
                 .quad	0x3fe0d163ccb9d6b8,	0x3fe2eb4ea1fed14b	// log(1.69141), 1/1.69141
                 .quad	0x3fe0e44985d1cc8c,	0x3fe2e025c04b8097	// log(1.69531), 1/1.69531
                 .quad	0x3fe0f7241c9b497d,	0x3fe2d50a012d50a0	// log(1.69922), 1/1.69922
                 .quad	0x3fe109f39e2d4c97,	0x3fe2c9fb4d812ca0	// log(1.70312), 1/1.70312
                 .quad	0x3fe11cb81787ccf8,	0x3fe2bef98e5a3711	// log(1.70703), 1/1.70703
                 .quad	0x3fe12f719593efbc,	0x3fe2b404ad012b40	// log(1.71094), 1/1.71094
                 .quad	0x3fe1422025243d45,	0x3fe2a91c92f3c105	// log(1.71484), 1/1.71484
                 .quad	0x3fe154c3d2f4d5ea,	0x3fe29e4129e4129e	// log(1.71875), 1/1.71875
                 .quad	0x3fe1675cababa60e,	0x3fe293725bb804a5	// log(1.72266), 1/1.72266
                 .quad	0x3fe179eabbd899a1,	0x3fe288b01288b013	// log(1.72656), 1/1.72656
                 .quad	0x3fe18c6e0ff5cf06,	0x3fe27dfa38a1ce4d	// log(1.73047), 1/1.73047
                 .quad	0x3fe19ee6b467c96f,	0x3fe27350b8812735	// log(1.73438), 1/1.73438
                 .quad	0x3fe1b154b57da29f,	0x3fe268b37cd60127	// log(1.73828), 1/1.73828
                 .quad	0x3fe1c3b81f713c25,	0x3fe25e22708092f1	// log(1.74219), 1/1.74219
                 .quad	0x3fe1d610fe677003,	0x3fe2539d7e9177b2	// log(1.74609), 1/1.74609
                 .quad	0x3fe1e85f5e7040d0,	0x3fe2492492492492	// log(1.75000), 1/1.75000
                 .quad	0x3fe1faa34b87094c,	0x3fe23eb79717605b	// log(1.75391), 1/1.75391
                 .quad	0x3fe20cdcd192ab6e,	0x3fe23456789abcdf	// log(1.75781), 1/1.75781
                 .quad	0x3fe21f0bfc65beec,	0x3fe22a0122a0122a	// log(1.76172), 1/1.76172
                 .quad	0x3fe23130d7bebf43,	0x3fe21fb78121fb78	// log(1.76562), 1/1.76562
                 .quad	0x3fe2434b6f483934,	0x3fe21579804855e6	// log(1.76953), 1/1.76953
                 .quad	0x3fe2555bce98f7cb,	0x3fe20b470c67c0d9	// log(1.77344), 1/1.77344
                 .quad	0x3fe26762013430e0,	0x3fe2012012012012	// log(1.77734), 1/1.77734
                 .quad	0x3fe2795e1289b11b,	0x3fe1f7047dc11f70	// log(1.78125), 1/1.78125
                 .quad	0x3fe28b500df60783,	0x3fe1ecf43c7fb84c	// log(1.78516), 1/1.78516
                 .quad	0x3fe29d37fec2b08b,	0x3fe1e2ef3b3fb874	// log(1.78906), 1/1.78906
                 .quad	0x3fe2af15f02640ad,	0x3fe1d8f5672e4abd	// log(1.79297), 1/1.79297
                 .quad	0x3fe2c0e9ed448e8c,	0x3fe1cf06ada2811d	// log(1.79688), 1/1.79688
                 .quad	0x3fe2d2b4012edc9e,	0x3fe1c522fc1ce059	// log(1.80078), 1/1.80078
                 .quad	0x3fe2e47436e40268,	0x3fe1bb4a4046ed29	// log(1.80469), 1/1.80469
                 .quad	0x3fe2f62a99509546,	0x3fe1b17c67f2bae3	// log(1.80859), 1/1.80859
                 .quad	0x3fe307d7334f10be,	0x3fe1a7b9611a7b96	// log(1.81250), 1/1.81250
                 .quad	0x3fe3197a0fa7fe6a,	0x3fe19e0119e0119e	// log(1.81641), 1/1.81641
                 .quad	0x3fe32b1339121d71,	0x3fe19453808ca29c	// log(1.82031), 1/1.82031
                 .quad	0x3fe33ca2ba328995,	0x3fe18ab083902bdb	// log(1.82422), 1/1.82422
                 .quad	0x3fe34e289d9ce1d3,	0x3fe1811811811812	// log(1.82812), 1/1.82812
                 .quad	0x3fe35fa4edd36ea0,	0x3fe1778a191bd684	// log(1.83203), 1/1.83203
                 .quad	0x3fe37117b54747b6,	0x3fe16e0689427379	// log(1.83594), 1/1.83594
                 .quad	0x3fe38280fe58797f,	0x3fe1648d50fc3201	// log(1.83984), 1/1.83984
                 .quad	0x3fe393e0d3562a1a,	0x3fe15b1e5f75270d	// log(1.84375), 1/1.84375
                 .quad	0x3fe3a5373e7ebdfa,	0x3fe151b9a3fdd5c9	// log(1.84766), 1/1.84766
                 .quad	0x3fe3b68449fffc23,	0x3fe1485f0e0acd3b	// log(1.85156), 1/1.85156
                 .quad	0x3fe3c7c7fff73206,	0x3fe13f0e8d344724	// log(1.85547), 1/1.85547
                 .quad	0x3fe3d9026a7156fb,	0x3fe135c81135c811	// log(1.85938), 1/1.85938
                 .quad	0x3fe3ea33936b2f5c,	0x3fe12c8b89edc0ac	// log(1.86328), 1/1.86328
                 .quad	0x3fe3fb5b84d16f42,	0x3fe12358e75d3033	// log(1.86719), 1/1.86719
                 .quad	0x3fe40c7a4880dce9,	0x3fe11a3019a74826	// log(1.87109), 1/1.87109
                 .quad	0x3fe41d8fe84672ae,	0x3fe1111111111111	// log(1.87500), 1/1.87500
                 .quad	0x3fe42e9c6ddf80bf,	0x3fe107fbbe011080	// log(1.87891), 1/1.87891
                 .quad	0x3fe43f9fe2f9ce67,	0x3fe0fef010fef011	// log(1.88281), 1/1.88281
                 .quad	0x3fe4509a5133bb0a,	0x3fe0f5edfab325a2	// log(1.88672), 1/1.88672
                 .quad	0x3fe4618bc21c5ec2,	0x3fe0ecf56be69c90	// log(1.89062), 1/1.89062
                 .quad	0x3fe472743f33aaad,	0x3fe0e40655826011	// log(1.89453), 1/1.89453
                 .quad	0x3fe48353d1ea88df,	0x3fe0db20a88f4696	// log(1.89844), 1/1.89844
                 .quad	0x3fe4942a83a2fc07,	0x3fe0d24456359e3a	// log(1.90234), 1/1.90234
                 .quad	0x3fe4a4f85db03ebb,	0x3fe0c9714fbcda3b	// log(1.90625), 1/1.90625
                 .quad	0x3fe4b5bd6956e274,	0x3fe0c0a7868b4171	// log(1.91016), 1/1.91016
                 .quad	0x3fe4c679afccee3a,	0x3fe0b7e6ec259dc8	// log(1.91406), 1/1.91406
                 .quad	0x3fe4d72d3a39fd00,	0x3fe0af2f722eecb5	// log(1.91797), 1/1.91797
                 .quad	0x3fe4e7d811b75bb1,	0x3fe0a6810a6810a7	// log(1.92188), 1/1.92188
                 .quad	0x3fe4f87a3f5026e9,	0x3fe09ddba6af8360	// log(1.92578), 1/1.92578
                 .quad	0x3fe50913cc01686b,	0x3fe0953f39010954	// log(1.92969), 1/1.92969
                 .quad	0x3fe519a4c0ba3446,	0x3fe08cabb37565e2	// log(1.93359), 1/1.93359
                 .quad	0x3fe52a2d265bc5ab,	0x3fe0842108421084	// log(1.93750), 1/1.93750
                 .quad	0x3fe53aad05b99b7d,	0x3fe07b9f29b8eae2	// log(1.94141), 1/1.94141
                 .quad	0x3fe54b2467999498,	0x3fe073260a47f7c6	// log(1.94531), 1/1.94531
                 .quad	0x3fe55b9354b40bcd,	0x3fe06ab59c7912fb	// log(1.94922), 1/1.94922
                 .quad	0x3fe56bf9d5b3f399,	0x3fe0624dd2f1a9fc	// log(1.95312), 1/1.95312
                 .quad	0x3fe57c57f336f191,	0x3fe059eea0727586	// log(1.95703), 1/1.95703
                 .quad	0x3fe58cadb5cd7989,	0x3fe05197f7d73404	// log(1.96094), 1/1.96094
                 .quad	0x3fe59cfb25fae87e,	0x3fe04949cc1664c5	// log(1.96484), 1/1.96484
                 .quad	0x3fe5ad404c359f2d,	0x3fe0410410410410	// log(1.96875), 1/1.96875
                 .quad	0x3fe5bd7d30e71c73,	0x3fe038c6b78247fc	// log(1.97266), 1/1.97266
                 .quad	0x3fe5cdb1dc6c1765,	0x3fe03091b51f5e1a	// log(1.97656), 1/1.97656
                 .quad	0x3fe5ddde57149923,	0x3fe02864fc7729e9	// log(1.98047), 1/1.98047
                 .quad	0x3fe5ee02a9241675,	0x3fe0204081020408	// log(1.98438), 1/1.98438
                 .quad	0x3fe5fe1edad18919,	0x3fe0182436517a37	// log(1.98828), 1/1.98828
                 .quad	0x3fe60e32f44788d9,	0x3fe0101010101010	// log(1.99219), 1/1.99219
                 .quad	0x3fe61e3efda46467,	0x3fe0080402010080	// log(1.99609), 1/1.99609

.literal8
.align (1<<3)
one:            .quad				0x3ff0000000000000
onehalf:        .quad				0x3fe0000000000000
onethird:       .quad       0x3fd5555555555555
log2:           .quad       0x3fe62e42fefa39ef   // ln(2)

.text
#if defined( __x86_64__ )
	#define RELATIVE_ADDR( _a )			(_a)( %rip )
#else
	#define RELATIVE_ADDR( _a )			(_a)-asinhf_body( %ecx )

.align (1<<4)
asinhf_pic:
	movl			(%esp),								%ecx				// Copy address of this instruction to %ecx
	ret
#endif

ENTRY(asinhf)
#if defined(__i386__)
	movl			FRAME_SIZE(STACKP),		%eax
	movss			FRAME_SIZE(STACKP),		%xmm0
	calll			asinhf_pic
asinhf_body:
#else
	movd			%xmm0,								%eax
#endif
	andnpd		%xmm5,								%xmm5				// zero out xmm5
	
	andl			$0x7fffffff,					%eax				// eax <-- |x|
	
	movd			%eax,									%xmm1				// xmm1 <-- |x|
	cvtss2sd	%xmm1,								%xmm2				// xmm2 <-- (double)|x|
	andnps		%xmm0,								%xmm1				// xmm1 <-- signbit(x)
	
	subl			$0x3e800000,					%eax				// eax <-- |x| - 1/4 as integers
	cmpl			$0x02000000,					%eax				// if ( |x| > 4 or |x| < 1/4 or isnan(x) )
	ja				2f																//      goto 2
	
	// 1/4 < |x| < 4
	lea		RELATIVE_ADDR(asinhf_mid), DX_P
#if defined( __SSE3__ )
	movddup		%xmm2,								%xmm0				// xmm0 <-- [x, x]
#else
	movapd    %xmm2,                %xmm0
	unpcklpd  %xmm0,                %xmm0
#endif

	mulsd			%xmm2,								%xmm2				// xmm2 <-- x*x
	movapd		%xmm0,								%xmm3				// xmm3 <-- [x, x]
	movapd		%xmm0,								%xmm4				// xmm4 <-- [x, x]
	mulpd			48(DX_P),							%xmm3				// xmm3 <-- [p3x, q3x]
	mulpd			80(DX_P),							%xmm4				// xmm4 <-- [p5x, q5x]
	mulpd			16(DX_P),							%xmm0				// xmm0 <-- [p1x, q1x]
	movapd		96(DX_P),							%xmm6				// xmm6 <-- [p6, q6]
	
#if defined( __SSE3__ )
	movddup		%xmm2,								%xmm5				// xmm5 <-- [x2, x2]
#else
	movapd    %xmm2,                %xmm5
	unpcklpd  %xmm5,                %xmm5
#endif
	
	mulsd			%xmm2,								%xmm2				// xmm2 <-- x4
	mulpd			%xmm5,								%xmm6				// xmm6 <-- [p6x2, q6x2]
	addpd			32(DX_P),							%xmm3				// xmm3 <-- [p3x + p2, q3x + q2]
	addpd			64(DX_P),							%xmm4				// xmm4 <-- [p5x + p4, q5x + q4]
	addpd			(DX_P),								%xmm0				// xmm0 <-- [p1x + p0, q1x + q0]
	
	mulpd			%xmm3,								%xmm5				// xmm5 <-- [p3x3 + p2x2, q3x3 + q2x2]
	addpd			%xmm4,								%xmm6				// xmm6 <-- [p6x2 + p5x + p4, q6x2 + q5x + q4]
	unpcklpd	%xmm2,								%xmm2				// xmm2 <-- [x4, x4]
	mulpd			%xmm2,								%xmm6				// xmm6 <-- [p6x6 + p5x5 + p4x4, q6x6 + q5x5 + q4x4]
	addpd			%xmm0,								%xmm5				// xmm5 <-- [p3x3 + p2x2 + p1x + p0, q3x3 + q2x2 + q1x + q0]
	addpd			%xmm6,								%xmm5				// xmm5 <-- [p(x), q(x)]
	
	movhlps		%xmm5,								%xmm6				// xmm6 <-- q(x)
	divsd			%xmm6,								%xmm5				// xmm5 <-- p(x)/q(x)
	
	cvtsd2ss	%xmm5,								%xmm0
	orps			%xmm1,								%xmm0
#if defined(__i386__)
	movss			%xmm0,								FRAME_SIZE( STACKP )
	flds			FRAME_SIZE( STACKP )
#endif
	ret

2:
	jge				3f																// if ( |x| > 4 or isnan(x) ) goto 3
	
// Polynomial approximation to asinhf(x) for |x| < 1/4.  Details of the computation are at top with the constants.

	lea		RELATIVE_ADDR(asinhf_low), DX_P
	
#if defined( __SSE3__ )
	movddup		%xmm2,								%xmm0				// xmm0 <-- [x, x]
#else
	movapd    %xmm2,                %xmm0
	unpcklpd  %xmm0,                %xmm0
#endif

	cmppd			$0,				%xmm0,			%xmm5				// detect |x| = 0
	
	mulsd			48(DX_P),							%xmm2				// xmm2 <-- [..., cx]
	movapd		%xmm0,								%xmm3				// xmm3 <-- [x, x]
	addpd			32(DX_P),							%xmm0				// xmm0 <-- [x, x] + [b1hi, b1lo]
	movapd		%xmm3,								%xmm4				// xmm4 <-- [x, x]
	addpd			16(DX_P),							%xmm3				// xmm3 <-- [x, x] + [ahi, alo]
	mulpd			%xmm4,								%xmm0				// xmm0 <-- [x, x] * [x + b1hi, x + b1lo]
	mulsd			%xmm2,								%xmm3				// xmm3 <-- [x + ahi, cx * (x + alo)]
	addpd			(DX_P),								%xmm0				// xmm0 <-- [x^2 + b1hi x, x^2 + b1lo x] + [b0hi, b0lo]
	
	andnpd		%xmm3,								%xmm5				// if |x| = 0, set xmm3 = 0 to suppress inexact from the next multiply.
	
	mulpd			%xmm5,								%xmm0				// xmm0 <-- [(x + ahi)(x^2 + b1hi x + b0hi), cx(x + alo)(x^2 + b1lo x + b0lo)]
	movhlps		%xmm0,								%xmm2				// xmm2 <-- [..., (x + ahi)(x^2 + b1hi x + b0hi)]
	mulsd			%xmm2,								%xmm0				// xmm0 <-- cx(x + alo)(x + ahi)(x^2 + b1lo x + b0lo)(x^2 + b1hi x + b0hi)
	
	cvtsd2ss	%xmm0,								%xmm0
	orps			%xmm1,								%xmm0
#if defined(__i386__)
	movss			%xmm0,								FRAME_SIZE( STACKP )
	flds			FRAME_SIZE( STACKP )
#endif
	ret

3:
	addl			$0x3e800000,					%eax				// %eax <-- |x|
	cmpl			$0x7f800000,					%eax
	ja				4f																// if isnan(x)  goto 4
	je				5f																// if (|x| = °) goto 5
	
// Go off and compute the reciprocal of |x|.  This will take a little while, but we have other stuff to do
// while this is up in the air.	
	
		movsd		RELATIVE_ADDR( one ),	%xmm0
		divsd		%xmm2,								%xmm0				// xmm0 <-- 1/|x|
	
// We need to take a logarithm for this case.  This code is shamelessly taken from our log2f.s implementation.
// Consult that file for more details of operation.

// begin by factoring x as 2^n*(mantissa)

	movl			%eax,									%edx
	shrl			$23,									%edx				// right-align exponent bits
	subl			$126,									%edx				// subtract exponent bias, but add one to get log(2x) instead of log(x)
																							//    i.e. we subtract 126 instead of 127 (the actual bias).
	cvtsi2sd	%edx,									%xmm2				// xmm2 <-- trunc(lg |x|) = n
	mulsd		RELATIVE_ADDR(log2),		%xmm2				// xmm2 <-- log(2^n)
	
// now, let mantissa = 1 + a + b = (1 + a)(1 + r), if r = b/(1+a), and
//
//      log(mantissa) = log(1+a) + log(1+r).
//
// we look up log(1+a) and 1/(1+a) in a table, keyed from a, and we compute log(1+r) via taylor series.
	
	movl			%eax,									%edx
	andl			$0x00007fff,					%eax				// eax <-- b
	andl			$0x007f8000,					%edx				// edx <-- a << 15
	orl				$0x3f800000,					%eax				// eax <-- 1 + b
	shrl			$(15-4),							%edx				// edx <-- a << 4, lookup key for log(1+a)
	movd			%eax,									%xmm3				// xmm3 <-- 1 + b
	cvtss2sd	%xmm3,								%xmm3
	subsd		RELATIVE_ADDR( one ),		%xmm3				// xmm3 <-- b
	lea	RELATIVE_ADDR(asinhf_hi_table), AX_P
	mulsd			8(AX_P, DX_P, 1),			%xmm3				// xmm3 <-- r = b * 1/(1+a)
	movapd		%xmm3,								%xmm4
	mulsd	RELATIVE_ADDR(onethird),	%xmm3				// xmm3 <-- 1/3 r
	movapd		%xmm4,								%xmm5				// xmm5 <-- r
	mulsd			%xmm4,								%xmm4				// xmm4 <-- r^2
	subsd	RELATIVE_ADDR(onehalf),		%xmm3				// xmm3 <-- 1/3 r - 1/2
	mulsd			%xmm4,								%xmm3				// xmm3 <-- 1/3 r^3 - 1/2 r^2
	addsd			%xmm5,								%xmm3				// xmm3 <-- 1/3 r^3 - 1/2 r^2 + r ~ log(1+r)
	addsd			(AX_P, DX_P, 1),			%xmm3				// xmm3 <-- log(a) + log(1+r)
	addsd			%xmm2,								%xmm3				// xmm3 <-- log(2x) with error < .5002 ulps
	
	
// Now we compute a correction from a polynomial in 1/|x|, which conveniently has landed in %xmm0 about now.

	movsd			-8(AX_P),							%xmm6				// xmm6 <-- [ ... , c ]
	movapd		-32(AX_P),						%xmm7				// xmm7 <-- [ b1hi, b1lo ]
#if defined( __SSE3__ )
	movddup   %xmm0,								%xmm2       // xmm2 <-- [ 1/x , 1/x ]
#else
	movapd    %xmm0,								%xmm2
	unpcklpd  %xmm2,                %xmm2
#endif 
	mulsd			%xmm0,								%xmm6				// xmm6 <-- [ ... , c/x ]
	addpd			%xmm2,								%xmm7				// xmm7 <-- [ 1/x + b1hi, 1/x + b1lo ]
	addsd			-16(AX_P),						%xmm0				// xmm0 <-- [ ... , 1/x + a ]
	mulpd			%xmm7,								%xmm2				// xmm2 <-- [ 1/x^2 + b1hi/x, 1/x^2 + b1lo/x ]
	mulsd			%xmm6,								%xmm0       // xmm0 <-- [ ... , c/x(1/x + a) ]
	addpd			-48(AX_P),						%xmm2				// xmm2 <-- [ 1/x^2 + b1hi/x + b0hi, 1/x^2 + b1lo/x + b0lo ]
	movhlps		%xmm2,								%xmm4				// xmm4 <-- [ ... , 1/x^2 + b1hi/x + b0hi ]
	mulsd			%xmm2,								%xmm0				// xmm0 <-- [ ... , c/x(1/x + a)(1/x^2 + b1lo/x + b0lo) ]
	mulsd			%xmm4,								%xmm0				// xmm0 <-- [ ... , c/x(1/x + a)(1/x^2 + b1lo/x + b0lo)(1/x^2 + b1hi/x + b0hi) ]
	
// Add log(2x) to the correction to get the final result	
	
	addsd			%xmm3,								%xmm0
	cvtsd2ss	%xmm0,								%xmm0
	orps			%xmm1,								%xmm0
#if defined(__i386__)
	movss			%xmm0,								FRAME_SIZE( STACKP )
	flds			FRAME_SIZE( STACKP )
#endif
	ret

4:
	addss			%xmm0,								%xmm0				// quiet the NaN
5:
	#if defined(__i386__)
	movss			%xmm0,								FRAME_SIZE( STACKP )
	flds			FRAME_SIZE( STACKP )
#endif
	ret

