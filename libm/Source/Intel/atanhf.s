/*
 *  atanhf.s
 *
 *      by Stephen Canon
 *
 *  Copyright (c) 2007, Apple Inc.  All rights reserved
 *
 *  Single precision implementation of C99 math library function atanhf for the OS X i386 and x86_64 abis.
 *
 */
 
#include <machine/asm.h>
#include "abi.h"

.const
.align 4
// Polynomial coefficients used for the 7/8 < |x| < 1 case.  These are addressed by offset from big_table,
// so take care if you move things around.

				.quad 0x3fc00400e807c4f5, 0xbf82304e7c64d56d // c4
				.quad 0xbff5533310026652, 0x4002a85c54ffbc1c // c3/c4
				.quad 0x3fcfffffaa90af21, 0xbfb0005bce0fa090 // c2
				.quad 0xc0000000244fd260, 0x400fff47b17904a4 // c1/c2
				
/*********************************************************
void gen_big_table() {
	union { double d; uint64_t x; } constant;
	
	int i,p,q;
	for (i=0; i<256; ++i) {
		double a = (double)i / 256.0;
		constant.d = -0.5*log1p(a);		printf("\t\t\t\t.quad 0x%016llx,", constant.x);
		constant.d = 1.0 / (1.0 + a);	printf(" 0x%016llx", constant.x);
		if (!i) {
			printf("\t// a = 0\n");
		} else {
			p = i; q = 256;
			while (!(p % 2)) {
				p = p/2; q = q/2;
			}
			printf("\t// a = %d/%d\n", p, q);
		}
	}
}
*********************************************************/

				//		 -0.5*log1p(a)		    1/(1+a)
big_table:		.quad 0x8000000000000000, 0x3ff0000000000000	// a = 0
				.quad 0xbf5ff00aa2b10bc0, 0x3fefe01fe01fe020	// a = 1/256
				.quad 0xbf6fe02a6b106789, 0x3fefc07f01fc07f0	// a = 1/128
				.quad 0xbf77dc475f810a77, 0x3fefa11caa01fa12	// a = 3/256
				.quad 0xbf7fc0a8b0fc03e4, 0x3fef81f81f81f820	// a = 1/64
				.quad 0xbf83cea44346a575, 0x3fef6310aca0dbb5	// a = 5/256
				.quad 0xbf87b91b07d5b11b, 0x3fef44659e4a4271	// a = 3/128
				.quad 0xbf8b9fc027af9198, 0x3fef25f644230ab5	// a = 7/256
				.quad 0xbf8f829b0e783300, 0x3fef07c1f07c1f08	// a = 1/32
				.quad 0xbf91b0d98923d980, 0x3feee9c7f8458e02	// a = 9/256
				.quad 0xbf939e87b9febd60, 0x3feecc07b301ecc0	// a = 5/128
				.quad 0xbf958a5bafc8e4d5, 0x3feeae807aba01eb	// a = 11/256
				.quad 0xbf977458f632dcfc, 0x3fee9131abf0b767	// a = 3/64
				.quad 0xbf995c830ec8e3eb, 0x3fee741aa59750e4	// a = 13/256
				.quad 0xbf9b42dd711971bf, 0x3fee573ac901e574	// a = 7/128
				.quad 0xbf9d276b8adb0b52, 0x3fee3a9179dc1a73	// a = 15/256
				.quad 0xbf9f0a30c01162a6, 0x3fee1e1e1e1e1e1e	// a = 1/16
				.quad 0xbfa075983598e471, 0x3fee01e01e01e01e	// a = 17/256
				.quad 0xbfa16536eea37ae1, 0x3fede5d6e3f8868a	// a = 9/128
				.quad 0xbfa253f62f0a1417, 0x3fedca01dca01dca	// a = 19/256
				.quad 0xbfa341d7961bd1d1, 0x3fedae6076b981db	// a = 5/64
				.quad 0xbfa42edcbea646f0, 0x3fed92f2231e7f8a	// a = 21/256
				.quad 0xbfa51b073f06183f, 0x3fed77b654b82c34	// a = 11/128
				.quad 0xbfa60658a93750c4, 0x3fed5cac807572b2	// a = 23/256
				.quad 0xbfa6f0d28ae56b4c, 0x3fed41d41d41d41d	// a = 3/32
				.quad 0xbfa7da766d7b12cc, 0x3fed272ca3fc5b1a	// a = 25/256
				.quad 0xbfa8c345d6319b21, 0x3fed0cb58f6ec074	// a = 13/128
				.quad 0xbfa9ab42462033ad, 0x3fecf26e5c44bfc6	// a = 27/256
				.quad 0xbfaa926d3a4ad563, 0x3fecd85689039b0b	// a = 7/64
				.quad 0xbfab78c82bb0eda1, 0x3fecbe6d9601cbe7	// a = 29/256
				.quad 0xbfac5e548f5bc743, 0x3feca4b3055ee191	// a = 15/128
				.quad 0xbfad4313d66cb35d, 0x3fec8b265afb8a42	// a = 31/256
				.quad 0xbfae27076e2af2e6, 0x3fec71c71c71c71c	// a = 1/8
				.quad 0xbfaf0a30c01162a6, 0x3fec5894d10d4986	// a = 33/256
				.quad 0xbfafec9131dbeabb, 0x3fec3f8f01c3f8f0	// a = 17/128
				.quad 0xbfb0671512ca596e, 0x3fec26b5392ea01c	// a = 35/256
				.quad 0xbfb0d77e7cd08e59, 0x3fec0e070381c0e0	// a = 9/64
				.quad 0xbfb14785846742ac, 0x3febf583ee868d8b	// a = 37/256
				.quad 0xbfb1b72ad52f67a0, 0x3febdd2b899406f7	// a = 19/128
				.quad 0xbfb2266f190a5acc, 0x3febc4fd65883e7b	// a = 39/256
				.quad 0xbfb29552f81ff523, 0x3febacf914c1bad0	// a = 5/32
				.quad 0xbfb303d718e47fd3, 0x3feb951e2b18ff23	// a = 41/256
				.quad 0xbfb371fc201e8f74, 0x3feb7d6c3dda338b	// a = 21/128
				.quad 0xbfb3dfc2b0ecc62a, 0x3feb65e2e3beee05	// a = 43/256
				.quad 0xbfb44d2b6ccb7d1e, 0x3feb4e81b4e81b4f	// a = 11/64
				.quad 0xbfb4ba36f39a55e5, 0x3feb37484ad806ce	// a = 45/256
				.quad 0xbfb526e5e3a1b438, 0x3feb2036406c80d9	// a = 23/128
				.quad 0xbfb59338d9982086, 0x3feb094b31d922a4	// a = 47/256
				.quad 0xbfb5ff3070a793d4, 0x3feaf286bca1af28	// a = 3/16
				.quad 0xbfb66acd4272ad51, 0x3feadbe87f94905e	// a = 49/256
				.quad 0xbfb6d60fe719d21d, 0x3feac5701ac5701b	// a = 25/128
				.quad 0xbfb740f8f54037a5, 0x3feaaf1d2f87ebfd	// a = 51/256
				.quad 0xbfb7ab890210d909, 0x3fea98ef606a63be	// a = 13/64
				.quad 0xbfb815c0a14357eb, 0x3fea82e65130e159	// a = 53/256
				.quad 0xbfb87fa06520c911, 0x3fea6d01a6d01a6d	// a = 27/128
				.quad 0xbfb8e928de886d41, 0x3fea574107688a4a	// a = 55/256
				.quad 0xbfb9525a9cf456b4, 0x3fea41a41a41a41a	// a = 7/32
				.quad 0xbfb9bb362e7dfb83, 0x3fea2c2a87c51ca0	// a = 57/256
				.quad 0xbfba23bc1fe2b563, 0x3fea16d3f97a4b02	// a = 29/128
				.quad 0xbfba8becfc882f19, 0x3fea01a01a01a01a	// a = 59/256
				.quad 0xbfbaf3c94e80bff3, 0x3fe9ec8e951033d9	// a = 15/64
				.quad 0xbfbb5b519e8fb5a4, 0x3fe9d79f176b682d	// a = 61/256
				.quad 0xbfbbc286742d8cd6, 0x3fe9c2d14ee4a102	// a = 31/128
				.quad 0xbfbc2968558c18c1, 0x3fe9ae24ea5510da	// a = 63/256
				.quad 0xbfbc8ff7c79a9a22, 0x3fe999999999999a	// a = 1/4
				.quad 0xbfbcf6354e09c5dc, 0x3fe9852f0d8ec0ff	// a = 65/256
				.quad 0xbfbd5c216b4fbb91, 0x3fe970e4f80cb872	// a = 33/128
				.quad 0xbfbdc1bca0abec7d, 0x3fe95cbb0be377ae	// a = 67/256
				.quad 0xbfbe27076e2af2e6, 0x3fe948b0fcd6e9e0	// a = 17/64
				.quad 0xbfbe8c0252aa5a60, 0x3fe934c67f9b2ce6	// a = 69/256
				.quad 0xbfbef0adcbdc5936, 0x3fe920fb49d0e229	// a = 35/128
				.quad 0xbfbf550a564b7b37, 0x3fe90d4f120190d5	// a = 71/256
				.quad 0xbfbfb9186d5e3e2b, 0x3fe8f9c18f9c18fa	// a = 9/32
				.quad 0xbfc00e6c45ad501d, 0x3fe8e6527af1373f	// a = 73/256
				.quad 0xbfc0402594b4d041, 0x3fe8d3018d3018d3	// a = 37/128
				.quad 0xbfc071b85fcd590d, 0x3fe8bfce8062ff3a	// a = 75/256
				.quad 0xbfc0a324e27390e3, 0x3fe8acb90f6bf3aa	// a = 19/64
				.quad 0xbfc0d46b579ab74b, 0x3fe899c0f601899c	// a = 77/256
				.quad 0xbfc1058bf9ae4ad5, 0x3fe886e5f0abb04a	// a = 39/128
				.quad 0xbfc136870293a8b0, 0x3fe87427bcc092b9	// a = 79/256
				.quad 0xbfc1675cababa60e, 0x3fe8618618618618	// a = 5/16
				.quad 0xbfc1980d2dd4236f, 0x3fe84f00c2780614	// a = 81/256
				.quad 0xbfc1c898c16999fb, 0x3fe83c977ab2bedd	// a = 41/128
				.quad 0xbfc1f8ff9e48a2f3, 0x3fe82a4a0182a4a0	// a = 83/256
				.quad 0xbfc22941fbcf7966, 0x3fe8181818181818	// a = 21/64
				.quad 0xbfc2596010df763a, 0x3fe8060180601806	// a = 85/256
				.quad 0xbfc2895a13de86a3, 0x3fe7f405fd017f40	// a = 43/128
				.quad 0xbfc2b9303ab89d25, 0x3fe7e225515a4f1d	// a = 87/256
				.quad 0xbfc2e8e2bae11d31, 0x3fe7d05f417d05f4	// a = 11/32
				.quad 0xbfc31871c9544185, 0x3fe7beb3922e017c	// a = 89/256
				.quad 0xbfc347dd9a987d55, 0x3fe7ad2208e0ecc3	// a = 45/128
				.quad 0xbfc3772662bfd85b, 0x3fe79baa6bb6398b	// a = 91/256
				.quad 0xbfc3a64c556945ea, 0x3fe78a4c8178a4c8	// a = 23/64
				.quad 0xbfc3d54fa5c1f710, 0x3fe77908119ac60d	// a = 93/256
				.quad 0xbfc404308686a7e4, 0x3fe767dce434a9b1	// a = 47/128
				.quad 0xbfc432ef2a04e814, 0x3fe756cac201756d	// a = 95/256
				.quad 0xbfc4618bc21c5ec2, 0x3fe745d1745d1746	// a = 3/8
				.quad 0xbfc49006804009d1, 0x3fe734f0c541fe8d	// a = 97/256
				.quad 0xbfc4be5f957778a1, 0x3fe724287f46debc	// a = 49/128
				.quad 0xbfc4ec973260026a, 0x3fe713786d9c7c09	// a = 99/256
				.quad 0xbfc51aad872df82d, 0x3fe702e05c0b8170	// a = 25/64
				.quad 0xbfc548a2c3add263, 0x3fe6f26016f26017	// a = 101/256
				.quad 0xbfc5767717455a6c, 0x3fe6e1f76b4337c7	// a = 51/128
				.quad 0xbfc5a42ab0f4cfe2, 0x3fe6d1a62681c861	// a = 103/256
				.quad 0xbfc5d1bdbf5809ca, 0x3fe6c16c16c16c17	// a = 13/32
				.quad 0xbfc5ff3070a793d4, 0x3fe6b1490aa31a3d	// a = 105/256
				.quad 0xbfc62c82f2b9c795, 0x3fe6a13cd1537290	// a = 53/128
				.quad 0xbfc659b57303e1f3, 0x3fe691473a88d0c0	// a = 107/256
				.quad 0xbfc686c81e9b14af, 0x3fe6816816816817	// a = 27/64
				.quad 0xbfc6b3bb2235943e, 0x3fe6719f3601671a	// a = 109/256
				.quad 0xbfc6e08eaa2ba1e4, 0x3fe661ec6a5122f9	// a = 55/128
				.quad 0xbfc70d42e2789236, 0x3fe6524f853b4aa3	// a = 111/256
				.quad 0xbfc739d7f6bbd007, 0x3fe642c8590b2164	// a = 7/16
				.quad 0xbfc7664e1239dbcf, 0x3fe63356b88ac0de	// a = 113/256
				.quad 0xbfc792a55fdd47a2, 0x3fe623fa77016240	// a = 57/128
				.quad 0xbfc7bede0a37afc0, 0x3fe614b36831ae94	// a = 115/256
				.quad 0xbfc7eaf83b82afc3, 0x3fe6058160581606	// a = 29/64
				.quad 0xbfc816f41da0d496, 0x3fe5f66434292dfc	// a = 117/256
				.quad 0xbfc842d1da1e8b17, 0x3fe5e75bb8d015e7	// a = 59/128
				.quad 0xbfc86e919a330ba0, 0x3fe5d867c3ece2a5	// a = 119/256
				.quad 0xbfc89a3386c1425b, 0x3fe5c9882b931057	// a = 15/32
				.quad 0xbfc8c5b7c858b48b, 0x3fe5babcc647fa91	// a = 121/256
				.quad 0xbfc8f11e873662c7, 0x3fe5ac056b015ac0	// a = 61/128
				.quad 0xbfc91c67eb45a83e, 0x3fe59d61f123ccaa	// a = 123/256
				.quad 0xbfc947941c2116fb, 0x3fe58ed2308158ed	// a = 31/64
				.quad 0xbfc972a341135158, 0x3fe5805601580560	// a = 125/256
				.quad 0xbfc99d958117e08b, 0x3fe571ed3c506b3a	// a = 63/128
				.quad 0xbfc9c86b02dc0863, 0x3fe56397ba7c52e2	// a = 127/256
				.quad 0xbfc9f323ecbf984c, 0x3fe5555555555555	// a = 1/2
				.quad 0xbfca1dc064d5b995, 0x3fe54725e6bb82fe	// a = 129/256
				.quad 0xbfca484090e5bb0a, 0x3fe5390948f40feb	// a = 65/128
				.quad 0xbfca72a4966bd9ea, 0x3fe52aff56a8054b	// a = 131/256
				.quad 0xbfca9cec9a9a084a, 0x3fe51d07eae2f815	// a = 33/64
				.quad 0xbfcac718c258b0e4, 0x3fe50f22e111c4c5	// a = 133/256
				.quad 0xbfcaf1293247786b, 0x3fe5015015015015	// a = 67/128
				.quad 0xbfcb1b1e0ebdfc5b, 0x3fe4f38f62dd4c9b	// a = 135/256
				.quad 0xbfcb44f77bcc8f63, 0x3fe4e5e0a72f0539	// a = 17/32
				.quad 0xbfcb6eb59d3cf35e, 0x3fe4d843bedc2c4c	// a = 137/256
				.quad 0xbfcb9858969310fb, 0x3fe4cab88725af6e	// a = 69/128
				.quad 0xbfcbc1e08b0dad0a, 0x3fe4bd3edda68fe1	// a = 139/256
				.quad 0xbfcbeb4d9da71b7c, 0x3fe4afd6a052bf5b	// a = 35/64
				.quad 0xbfcc149ff115f027, 0x3fe4a27fad76014a	// a = 141/256
				.quad 0xbfcc3dd7a7cdad4d, 0x3fe49539e3b2d067	// a = 71/128
				.quad 0xbfcc66f4e3ff6ff8, 0x3fe4880522014880	// a = 143/256
				.quad 0xbfcc8ff7c79a9a22, 0x3fe47ae147ae147b	// a = 9/16
				.quad 0xbfccb8e0744d7aca, 0x3fe46dce34596066	// a = 145/256
				.quad 0xbfcce1af0b85f3eb, 0x3fe460cbc7f5cf9a	// a = 73/128
				.quad 0xbfcd0a63ae721e64, 0x3fe453d9e2c776ca	// a = 147/256
				.quad 0xbfcd32fe7e00ebd5, 0x3fe446f86562d9fb	// a = 37/64
				.quad 0xbfcd5b7f9ae2c684, 0x3fe43a2730abee4d	// a = 149/256
				.quad 0xbfcd83e7258a2f3e, 0x3fe42d6625d51f87	// a = 75/128
				.quad 0xbfcdac353e2c5954, 0x3fe420b5265e5951	// a = 151/256
				.quad 0xbfcdd46a04c1c4a1, 0x3fe4141414141414	// a = 19/32
				.quad 0xbfcdfc859906d5b5, 0x3fe40782d10e6566	// a = 153/256
				.quad 0xbfce24881a7c6c26, 0x3fe3fb013fb013fb	// a = 77/128
				.quad 0xbfce4c71a8687704, 0x3fe3ee8f42a5af07	// a = 155/256
				.quad 0xbfce744261d68788, 0x3fe3e22cbce4a902	// a = 39/64
				.quad 0xbfce9bfa659861f5, 0x3fe3d5d991aa75c6	// a = 157/256
				.quad 0xbfcec399d2468cc0, 0x3fe3c995a47babe7	// a = 79/128
				.quad 0xbfceeb20c640ddf4, 0x3fe3bd60d9232955	// a = 159/256
				.quad 0xbfcf128f5faf06ed, 0x3fe3b13b13b13b14	// a = 5/8
				.quad 0xbfcf39e5bc811e5c, 0x3fe3a524387ac822	// a = 161/256
				.quad 0xbfcf6123fa7028ac, 0x3fe3991c2c187f63	// a = 81/128
				.quad 0xbfcf884a36fe9ec2, 0x3fe38d22d366088e	// a = 163/256
				.quad 0xbfcfaf588f78f31f, 0x3fe3813813813814	// a = 41/64
				.quad 0xbfcfd64f20f61572, 0x3fe3755bd1c945ee	// a = 165/256
				.quad 0xbfcffd2e0857f498, 0x3fe3698df3de0748	// a = 83/128
				.quad 0xbfd011fab125ff8a, 0x3fe35dce5f9f2af8	// a = 167/256
				.quad 0xbfd02552a5a5d0ff, 0x3fe3521cfb2b78c1	// a = 21/32
				.quad 0xbfd0389eefce633b, 0x3fe34679ace01346	// a = 169/256
				.quad 0xbfd04bdf9da926d2, 0x3fe33ae45b57bcb2	// a = 85/128
				.quad 0xbfd05f14bd26459c, 0x3fe32f5ced6a1dfa	// a = 171/256
				.quad 0xbfd0723e5c1cdf40, 0x3fe323e34a2b10bf	// a = 43/64
				.quad 0xbfd0855c884b450e, 0x3fe3187758e9ebb6	// a = 173/256
				.quad 0xbfd0986f4f573521, 0x3fe30d190130d190	// a = 87/128
				.quad 0xbfd0ab76bece14d2, 0x3fe301c82ac40260	// a = 175/256
				.quad 0xbfd0be72e4252a83, 0x3fe2f684bda12f68	// a = 11/16
				.quad 0xbfd0d163ccb9d6b8, 0x3fe2eb4ea1fed14b	// a = 177/256
				.quad 0xbfd0e44985d1cc8c, 0x3fe2e025c04b8097	// a = 89/128
				.quad 0xbfd0f7241c9b497d, 0x3fe2d50a012d50a0	// a = 179/256
				.quad 0xbfd109f39e2d4c97, 0x3fe2c9fb4d812ca0	// a = 45/64
				.quad 0xbfd11cb81787ccf8, 0x3fe2bef98e5a3711	// a = 181/256
				.quad 0xbfd12f719593efbc, 0x3fe2b404ad012b40	// a = 91/128
				.quad 0xbfd1422025243d45, 0x3fe2a91c92f3c105	// a = 183/256
				.quad 0xbfd154c3d2f4d5ea, 0x3fe29e4129e4129e	// a = 23/32
				.quad 0xbfd1675cababa60e, 0x3fe293725bb804a5	// a = 185/256
				.quad 0xbfd179eabbd899a1, 0x3fe288b01288b013	// a = 93/128
				.quad 0xbfd18c6e0ff5cf06, 0x3fe27dfa38a1ce4d	// a = 187/256
				.quad 0xbfd19ee6b467c96f, 0x3fe27350b8812735	// a = 47/64
				.quad 0xbfd1b154b57da29f, 0x3fe268b37cd60127	// a = 189/256
				.quad 0xbfd1c3b81f713c25, 0x3fe25e22708092f1	// a = 95/128
				.quad 0xbfd1d610fe677003, 0x3fe2539d7e9177b2	// a = 191/256
				.quad 0xbfd1e85f5e7040d0, 0x3fe2492492492492	// a = 3/4
				.quad 0xbfd1faa34b87094c, 0x3fe23eb79717605b	// a = 193/256
				.quad 0xbfd20cdcd192ab6e, 0x3fe23456789abcdf	// a = 97/128
				.quad 0xbfd21f0bfc65beec, 0x3fe22a0122a0122a	// a = 195/256
				.quad 0xbfd23130d7bebf43, 0x3fe21fb78121fb78	// a = 49/64
				.quad 0xbfd2434b6f483934, 0x3fe21579804855e6	// a = 197/256
				.quad 0xbfd2555bce98f7cb, 0x3fe20b470c67c0d9	// a = 99/128
				.quad 0xbfd26762013430e0, 0x3fe2012012012012	// a = 199/256
				.quad 0xbfd2795e1289b11b, 0x3fe1f7047dc11f70	// a = 25/32
				.quad 0xbfd28b500df60783, 0x3fe1ecf43c7fb84c	// a = 201/256
				.quad 0xbfd29d37fec2b08b, 0x3fe1e2ef3b3fb874	// a = 101/128
				.quad 0xbfd2af15f02640ad, 0x3fe1d8f5672e4abd	// a = 203/256
				.quad 0xbfd2c0e9ed448e8c, 0x3fe1cf06ada2811d	// a = 51/64
				.quad 0xbfd2d2b4012edc9e, 0x3fe1c522fc1ce059	// a = 205/256
				.quad 0xbfd2e47436e40268, 0x3fe1bb4a4046ed29	// a = 103/128
				.quad 0xbfd2f62a99509546, 0x3fe1b17c67f2bae3	// a = 207/256
				.quad 0xbfd307d7334f10be, 0x3fe1a7b9611a7b96	// a = 13/16
				.quad 0xbfd3197a0fa7fe6a, 0x3fe19e0119e0119e	// a = 209/256
				.quad 0xbfd32b1339121d71, 0x3fe19453808ca29c	// a = 105/128
				.quad 0xbfd33ca2ba328995, 0x3fe18ab083902bdb	// a = 211/256
				.quad 0xbfd34e289d9ce1d3, 0x3fe1811811811812	// a = 53/64
				.quad 0xbfd35fa4edd36ea0, 0x3fe1778a191bd684	// a = 213/256
				.quad 0xbfd37117b54747b6, 0x3fe16e0689427379	// a = 107/128
				.quad 0xbfd38280fe58797f, 0x3fe1648d50fc3201	// a = 215/256
				.quad 0xbfd393e0d3562a1a, 0x3fe15b1e5f75270d	// a = 27/32
				.quad 0xbfd3a5373e7ebdfa, 0x3fe151b9a3fdd5c9	// a = 217/256
				.quad 0xbfd3b68449fffc23, 0x3fe1485f0e0acd3b	// a = 109/128
				.quad 0xbfd3c7c7fff73206, 0x3fe13f0e8d344724	// a = 219/256
				.quad 0xbfd3d9026a7156fb, 0x3fe135c81135c811	// a = 55/64
				.quad 0xbfd3ea33936b2f5c, 0x3fe12c8b89edc0ac	// a = 221/256
				.quad 0xbfd3fb5b84d16f42, 0x3fe12358e75d3033	// a = 111/128
				.quad 0xbfd40c7a4880dce9, 0x3fe11a3019a74826	// a = 223/256
				.quad 0xbfd41d8fe84672ae, 0x3fe1111111111111	// a = 7/8
				.quad 0xbfd42e9c6ddf80bf, 0x3fe107fbbe011080	// a = 225/256
				.quad 0xbfd43f9fe2f9ce67, 0x3fe0fef010fef011	// a = 113/128
				.quad 0xbfd4509a5133bb0a, 0x3fe0f5edfab325a2	// a = 227/256
				.quad 0xbfd4618bc21c5ec2, 0x3fe0ecf56be69c90	// a = 57/64
				.quad 0xbfd472743f33aaad, 0x3fe0e40655826011	// a = 229/256
				.quad 0xbfd48353d1ea88df, 0x3fe0db20a88f4696	// a = 115/128
				.quad 0xbfd4942a83a2fc07, 0x3fe0d24456359e3a	// a = 231/256
				.quad 0xbfd4a4f85db03ebb, 0x3fe0c9714fbcda3b	// a = 29/32
				.quad 0xbfd4b5bd6956e274, 0x3fe0c0a7868b4171	// a = 233/256
				.quad 0xbfd4c679afccee3a, 0x3fe0b7e6ec259dc8	// a = 117/128
				.quad 0xbfd4d72d3a39fd00, 0x3fe0af2f722eecb5	// a = 235/256
				.quad 0xbfd4e7d811b75bb1, 0x3fe0a6810a6810a7	// a = 59/64
				.quad 0xbfd4f87a3f5026e9, 0x3fe09ddba6af8360	// a = 237/256
				.quad 0xbfd50913cc01686b, 0x3fe0953f39010954	// a = 119/128
				.quad 0xbfd519a4c0ba3446, 0x3fe08cabb37565e2	// a = 239/256
				.quad 0xbfd52a2d265bc5ab, 0x3fe0842108421084	// a = 15/16
				.quad 0xbfd53aad05b99b7d, 0x3fe07b9f29b8eae2	// a = 241/256
				.quad 0xbfd54b2467999498, 0x3fe073260a47f7c6	// a = 121/128
				.quad 0xbfd55b9354b40bcd, 0x3fe06ab59c7912fb	// a = 243/256
				.quad 0xbfd56bf9d5b3f399, 0x3fe0624dd2f1a9fc	// a = 61/64
				.quad 0xbfd57c57f336f191, 0x3fe059eea0727586	// a = 245/256
				.quad 0xbfd58cadb5cd7989, 0x3fe05197f7d73404	// a = 123/128
				.quad 0xbfd59cfb25fae87e, 0x3fe04949cc1664c5	// a = 247/256
				.quad 0xbfd5ad404c359f2d, 0x3fe0410410410410	// a = 31/32
				.quad 0xbfd5bd7d30e71c73, 0x3fe038c6b78247fc	// a = 249/256
				.quad 0xbfd5cdb1dc6c1765, 0x3fe03091b51f5e1a	// a = 125/128
				.quad 0xbfd5ddde57149923, 0x3fe02864fc7729e9	// a = 251/256
				.quad 0xbfd5ee02a9241675, 0x3fe0204081020408	// a = 63/64
				.quad 0xbfd5fe1edad18919, 0x3fe0182436517a37	// a = 253/256
				.quad 0xbfd60e32f44788d9, 0x3fe0101010101010	// a = 127/128
				.quad 0xbfd61e3efda46467, 0x3fe0080402010080	// a = 255/256
				
				
				
/********************************************************
void gen_small_table() {
	union { double d; uint64_t x; } constant;
	
	int i,p,q;
	for (i=0; i<256; ++i) {
		double a = (double)i / 256.0;
		constant.d = atanh(a);			printf("\t\t\t\t.quad 0x%016llx,", constant.x);
		constant.d = 1.0 / (1.0 - a*a);	printf(" 0x%016llx", constant.x);
		if (!i) {
			printf("\t// a = 0\n");
		} else {
			p = i; q = 256;
			while (!(p % 2)) {
				p = p/2; q = q/2;
			}
			printf("\t// a = %d/%d\n", p, q);
		}
	}
}
*********************************************************/
				
				//         atanh(hi)		  1/(1-hi^2)
small_table:	.quad 0x0000000000000000, 0x3ff0000000000000	// a = 0
				.quad 0x3f7000055558888b, 0x3ff0001000100010	// a = 1/256
				.quad 0x3f8000155588891b, 0x3ff0004001000400	// a = 1/128
				.quad 0x3f8800480184d690, 0x3ff0009005102d92	// a = 3/256
				.quad 0x3f9000555888ad1c, 0x3ff0010010010010	// a = 1/64
				.quad 0x3f9400a6b46f591b, 0x3ff001902713d0ef	// a = 5/256
				.quad 0x3f980120184f3ded, 0x3ff00240510b659a	// a = 3/128
				.quad 0x3f9c01c989e21e45, 0x3ff00310962cbe90	// a = 7/256
				.quad 0x3fa001558891aee2, 0x3ff0040100401004	// a = 1/32
				.quad 0x3fa201e65c5878df, 0x3ff005119a91e82a	// a = 9/256
				.quad 0x3fa4029b471650a5, 0x3ff0064271f48383	// a = 5/128
				.quad 0x3fa60378514ed016, 0x3ff0079394c14f5f	// a = 11/256
				.quad 0x3fa8048185694820, 0x3ff0090512da9af7	// a = 3/64
				.quad 0x3faa05baefe1fa74, 0x3ff00a96fdad7784	// a = 13/256
				.quad 0x3fac07289f7b9245, 0x3ff00c496833c7a5	// a = 7/128
				.quad 0x3fae08cea570e1e7, 0x3ff00e1c66f67ea5	// a = 15/256
				.quad 0x3fb005588ad375ad, 0x3ff0101010101010	// a = 1/16
				.quad 0x3fb1066a036f9cec, 0x3ff012247b2f1021	// a = 17/256
				.quad 0x3fb2079dc9754942, 0x3ff01459c19905ab	// a = 9/128
				.quad 0x3fb308f5eb6e013e, 0x3ff016affe2d6e10	// a = 19/256
				.quad 0x3fb40a74799e283e, 0x3ff019274d68f3fd	// a = 5/64
				.quad 0x3fb50c1b861eec5e, 0x3ff01bbfcd68d99f	// a = 21/256
				.quad 0x3fb60ded24f86c90, 0x3ff01e799dee9716	// a = 11/128
				.quad 0x3fb70feb6c3c1a5a, 0x3ff02154e063adfb	// a = 23/256
				.quad 0x3fb81218741f5a6a, 0x3ff02451b7ddb2d2	// a = 3/32
				.quad 0x3fb9147657166782, 0x3ff0277049228d5b	// a = 25/256
				.quad 0x3fba170731ef7b1e, 0x3ff02ab0baacf0ac	// a = 13/128
				.quad 0x3fbb19cd23ee3f68, 0x3ff02e1334b10c2c	// a = 27/256
				.quad 0x3fbc1cca4ee78e02, 0x3ff03197e121767b	// a = 7/64
				.quad 0x3fbd2000d75d7f60, 0x3ff0353eebb45366	// a = 29/256
				.quad 0x3fbe2372e49bce55, 0x3ff0390881e8b620	// a = 15/128
				.quad 0x3fbf2722a0d493b5, 0x3ff03cf4d30c4100	// a = 31/256
				.quad 0x3fc015891c9eaef7, 0x3ff0410410410410	// a = 1/8
				.quad 0x3fc097a1ef16543f, 0x3ff045366c839bda	// a = 33/256
				.quad 0x3fc119dce19bdbaf, 0x3ff0498c1cb191d7	// a = 17/128
				.quad 0x3fc19c3b0fa86d54, 0x3ff04e0557900010	// a = 35/256
				.quad 0x3fc21ebd96730f38, 0x3ff052a255d27987	// a = 9/64
				.quad 0x3fc2a165950035bc, 0x3ff0576352223903	// a = 37/256
				.quad 0x3fc324342c318e81, 0x3ff05c488925980e	// a = 19/128
				.quad 0x3fc3a72a7ed6082c, 0x3ff061523987cfea	// a = 39/256
				.quad 0x3fc42a49b1ba196a, 0x3ff06680a4010668	// a = 5/32
				.quad 0x3fc4ad92ebb84987, 0x3ff06bd40b5ea891	// a = 41/256
				.quad 0x3fc5310755c9fd18, 0x3ff0714cb48c1542	// a = 21/128
				.quad 0x3fc5b4a81b18894f, 0x3ff076eae69b99da	// a = 43/256
				.quad 0x3fc63876690e9070, 0x3ff07caeeacfc334	// a = 11/64
				.quad 0x3fc6bc736f69aa39, 0x3ff082990ca50557	// a = 45/256
				.quad 0x3fc740a0604c5adb, 0x3ff088a999dbbc40	// a = 23/128
				.quad 0x3fc7c4fe70505b75, 0x3ff08ee0e282885b	// a = 47/256
				.quad 0x3fc8498ed69936dc, 0x3ff0953f39010954	// a = 3/16
				.quad 0x3fc8ce52cce73dc7, 0x3ff09bc4f222fa0b	// a = 49/256
				.quad 0x3fc9534b8faad565, 0x3ff0a2726523b088	// a = 25/128
				.quad 0x3fc9d87a5e18238b, 0x3ff0a947ebba04fd	// a = 51/256
				.quad 0x3fca5de07a3b1bc2, 0x3ff0b045e224a2f9	// a = 13/64
				.quad 0x3fcae37f290bf096, 0x3ff0b76ca736c81a	// a = 53/256
				.quad 0x3fcb6957b283ec92, 0x3ff0bebc9c657399	// a = 27/128
				.quad 0x3fcbef6b61b2b693, 0x3ff0c63625d50a60	// a = 55/256
				.quad 0x3fcc75bb84d40518, 0x3ff0cdd9aa677344	// a = 7/32
				.quad 0x3fccfc496d65c453, 0x3ff0d5a793caaf5c	// a = 57/256
				.quad 0x3fcd8316703eb314, 0x3ff0dda04e87f26e	// a = 29/128
				.quad 0x3fce0a23e5a57a70, 0x3ff0e5c44a133fbe	// a = 59/256
				.quad 0x3fce917329684475, 0x3ff0ee13f8db8f93	// a = 15/64
				.quad 0x3fcf19059af4d646, 0x3ff0f68fd05b8216	// a = 61/256
				.quad 0x3fcfa0dc9d7131ff, 0x3ff0ff38492aa44b	// a = 31/128
				.quad 0x3fd0147ccbea629a, 0x3ff1080ddf0f4c2c	// a = 63/256
				.quad 0x3fd058aefa811451, 0x3ff1111111111111	// a = 1/4
				.quad 0x3fd09d0591f0bb21, 0x3ff11a42618be5dd	// a = 65/256
				.quad 0x3fd0e1814bbd9d56, 0x3ff123a25643da93	// a = 33/128
				.quad 0x3fd12622e38a03ab, 0x3ff12d3178798b4c	// a = 67/256
				.quad 0x3fd16aeb1724557c, 0x3ff136f054ff42a4	// a = 17/64
				.quad 0x3fd1afdaa6958afb, 0x3ff140df7c4ed62d	// a = 69/256
				.quad 0x3fd1f4f2542ff85b, 0x3ff14aff82a0438d	// a = 35/128
				.quad 0x3fd23a32e49e74ec, 0x3ff1555100011555	// a = 71/256
				.quad 0x3fd27f9d1ef3e177, 0x3ff15fd4906c96f1	// a = 9/32
				.quad 0x3fd2c531ccbb110c, 0x3ff16a8ad3e4df4c	// a = 73/256
				.quad 0x3fd30af1ba0717b8, 0x3ff175746e8cba40	// a = 37/128
				.quad 0x3fd350ddb58402ab, 0x3ff1809208c27917	// a = 75/256
				.quad 0x3fd396f69087fd7c, 0x3ff18be44f3bb2f6	// a = 19/64
				.quad 0x3fd3dd3d1f24e85f, 0x3ff1976bf321fe4a	// a = 77/256
				.quad 0x3fd423b2383a6343, 0x3ff1a329aa30acca	// a = 39/128
				.quad 0x3fd46a56b58851f7, 0x3ff1af1e2ed3940c	// a = 79/256
				.quad 0x3fd4b12b73c1dd95, 0x3ff1bb4a4046ed29	// a = 5/16
				.quad 0x3fd4f83152a0f7b5, 0x3ff1c7aea2b8565d	// a = 81/256
				.quad 0x3fd53f6934fa63f8, 0x3ff1d44c1f69021b	// a = 41/128
				.quad 0x3fd586d400d24cbe, 0x3ff1e12384d11f8a	// a = 83/256
				.quad 0x3fd5ce729f71680a, 0x3ff1ee35a6c48900	// a = 21/64
				.quad 0x3fd61645fd7ab1ba, 0x3ff1fb835e98c5a2	// a = 85/256
				.quad 0x3fd65e4f0b01c08e, 0x3ff2090d8b4c6bdc	// a = 43/128
				.quad 0x3fd6a68ebba1bb84, 0x3ff216d511aff336	// a = 87/256
				.quad 0x3fd6ef060694f581, 0x3ff224dadc900489	// a = 11/32
				.quad 0x3fd737b5e6cd3547, 0x3ff2331fdce15884	// a = 89/256
				.quad 0x3fd7809f5b0cb028, 0x3ff241a509ee3506	// a = 45/128
				.quad 0x3fd7c9c365ffbdfc, 0x3ff2506b61859acc	// a = 91/256
				.quad 0x3fd813230e574d5a, 0x3ff25f73e82c35af	// a = 23/64
				.quad 0x3fd85cbf5ee41f2a, 0x3ff26ebfa94f2298	// a = 93/256
				.quad 0x3fd8a69966b2d128, 0x3ff27e4fb7789f5c	// a = 47/128
				.quad 0x3fd8f0b23928bf15, 0x3ff28e252c86b994	// a = 95/256
				.quad 0x3fd93b0aee21c2c8, 0x3ff29e4129e4129e	// a = 3/8
				.quad 0x3fd985a4a20edba2, 0x3ff2aea4d8c2d024	// a = 97/256
				.quad 0x3fd9d0807615c643, 0x3ff2bf516a59d19c	// a = 49/128
				.quad 0x3fda1b9f90318dcb, 0x3ff2d04818244483	// a = 99/256
				.quad 0x3fda67031b542059, 0x3ff2e18a2423b269	// a = 25/64
				.quad 0x3fdab2ac4788f0dc, 0x3ff2f318d924a53c	// a = 101/256
				.quad 0x3fdafe9c4a18b0e3, 0x3ff304f58b05ffd0	// a = 51/128
				.quad 0x3fdb4ad45dae2d59, 0x3ff3172197032a26	// a = 103/256
				.quad 0x3fdb9755c27c59e0, 0x3ff3299e6401329a	// a = 13/32
				.quad 0x3fdbe421be6596cb, 0x3ff33c6d62df06fc	// a = 105/256
				.quad 0x3fdc31399d243e72, 0x3ff34f900ec8ea4b	// a = 53/128
				.quad 0x3fdc7e9eb074870b, 0x3ff36307ed8f4df6	// a = 107/256
				.quad 0x3fdccc52503fc6fe, 0x3ff376d69001376d	// a = 27/64
				.quad 0x3fdd1a55dac92a26, 0x3ff38afd924a5d42	// a = 109/256
				.quad 0x3fdd68aab4dbe74b, 0x3ff39f7e9c55292e	// a = 55/128
				.quad 0x3fddb75249fb05b3, 0x3ff3b45b6230cf21	// a = 111/256
				.quad 0x3fde064e0c92c396, 0x3ff3c995a47babe7	// a = 7/16
				.quad 0x3fde559f762baeee, 0x3ff3df2f30d22100	// a = 113/256
				.quad 0x3fdea548079f8314, 0x3ff3f529e2422615	// a = 57/128
				.quad 0x3fdef549494fde6b, 0x3ff40b87a1c3cbdf	// a = 115/256
				.quad 0x3fdf45a4cb5ee468, 0x3ff4224a66b6ef90	// a = 29/64
				.quad 0x3fdf965c25e9e132, 0x3ff4397437666199	// a = 117/256
				.quad 0x3fdfe770f9460541, 0x3ff451072990c667	// a = 59/128
				.quad 0x3fe01c72771fa832, 0x3ff4690562f77bee	// a = 119/256
				.quad 0x3fe0455cdb2ce279, 0x3ff4817119f3d325	// a = 15/32
				.quad 0x3fe06e78860a7e8c, 0x3ff49a4c9612f15e	// a = 121/256
				.quad 0x3fe097c659991ec1, 0x3ff4b39a30b8b264	// a = 61/128
				.quad 0x3fe0c1473c7e911c, 0x3ff4cd5c55c9e98b	// a = 123/256
				.quad 0x3fe0eafc1a4b81ea, 0x3ff4e795845e65bf	// a = 31/64
				.quad 0x3fe114e5e3a29a89, 0x3ff502484f7b2291	// a = 125/256
				.quad 0x3fe13f058e611d13, 0x3ff51d775ed516de	// a = 63/128
				.quad 0x3fe1695c15c90ea9, 0x3ff539256f9d18b1	// a = 127/256
				.quad 0x3fe193ea7aad030b, 0x3ff5555555555555	// a = 1/2
				.quad 0x3fe1beb1c39d9d1a, 0x3ff57209fab0e4c8	// a = 129/256
				.quad 0x3fe1e9b2fd18d91c, 0x3ff58f46627e080b	// a = 65/128
				.quad 0x3fe214ef39bb369d, 0x3ff5ad0da89bab4f	// a = 131/256
				.quad 0x3fe240679272d92c, 0x3ff5cb6302face89	// a = 33/64
				.quad 0x3fe26c1d26b4b850, 0x3ff5ea49c2ac81d8	// a = 133/256
				.quad 0x3fe298111cb3f8ad, 0x3ff609c554fd2e40	// a = 67/128
				.quad 0x3fe2c444a19b89a5, 0x3ff629d9449defb6	// a = 135/256
				.quad 0x3fe2f0b8e9ca2471, 0x3ff64a893adcd25f	// a = 17/32
				.quad 0x3fe31d6f3110cb49, 0x3ff66bd900ecd324	// a = 137/256
				.quad 0x3fe34a68baf3e921, 0x3ff68dcc813e92e9	// a = 69/128
				.quad 0x3fe377a6d2ef3448, 0x3ff6b067c8eabc0a	// a = 139/256
				.quad 0x3fe3a52accbc786f, 0x3ff6d3af092f2b6d	// a = 35/64
				.quad 0x3fe3d2f6049d6eb3, 0x3ff6f7a69900016f	// a = 141/256
				.quad 0x3fe40109dfa8ccb5, 0x3ff71c52f6add38a	// a = 71/128
				.quad 0x3fe42f67cc1ab64c, 0x3ff741b8c9a24d98	// a = 143/256
				.quad 0x3fe45e1141a8c010, 0x3ff767dce434a9b1	// a = 9/16
				.quad 0x3fe48d07c1d9b3ea, 0x3ff78ec445977f4f	// a = 145/256
				.quad 0x3fe4bc4cd8614bf6, 0x3ff7b6741be18685	// a = 73/128
				.quad 0x3fe4ebe21b801b3d, 0x3ff7def1c6330a52	// a = 147/256
				.quad 0x3fe51bc92c67dfa3, 0x3ff80842d6f9e5e7	// a = 37/64
				.quad 0x3fe54c03b7a47bf0, 0x3ff8326d16560c53	// a = 149/256
				.quad 0x3fe57c937589dd3f, 0x3ff85d7684a0c0a3	// a = 75/128
				.quad 0x3fe5ad7a2aa7137d, 0x3ff889655d18ce68	// a = 151/256
				.quad 0x3fe5deb9a83ee95c, 0x3ff8b64018b64019	// a = 19/32
				.quad 0x3fe61053ccc64d7b, 0x3ff8e40d7128425c	// a = 153/256
				.quad 0x3fe6424a8468e3d0, 0x3ff912d464001913	// a = 77/128
				.quad 0x3fe6749fc9941cbb, 0x3ff9429c360c45bd	// a = 155/256
				.quad 0x3fe6a755a5893549, 0x3ff9736c76e73ebc	// a = 39/64
				.quad 0x3fe6da6e30f68b8e, 0x3ff9a54d04bd5ccc	// a = 157/256
				.quad 0x3fe70deb9498b947, 0x3ff9d846104df033	// a = 79/128
				.quad 0x3fe741d009e3ef5a, 0x3ffa0c60212bc26a	// a = 159/256
				.quad 0x3fe7761ddbb61598, 0x3ffa41a41a41a41a	// a = 5/8
				.quad 0x3fe7aad767123bc7, 0x3ffa781b3ea00af6	// a = 161/256
				.quad 0x3fe7dfff1be5f383, 0x3ffaafcf36993030	// a = 81/128
				.quad 0x3fe815977dd935af, 0x3ffae8ca15319829	// a = 163/256
				.quad 0x3fe84ba3252982b5, 0x3ffb23165deb6f69	// a = 41/64
				.quad 0x3fe88224bf90fa20, 0x3ffb5ebf0af3b992	// a = 165/256
				.quad 0x3fe8b91f113a34a3, 0x3ffb9bcf93b8ede9	// a = 83/128
				.quad 0x3fe8f094f5c1bba9, 0x3ffbda53f3f34c2e	// a = 167/256
				.quad 0x3fe9288961460abe, 0x3ffc1a58b327f576	// a = 21/32
				.quad 0x3fe960ff61871a12, 0x3ffc5beaecb0a99a	// a = 169/256
				.quad 0x3fe999fa1f16860c, 0x3ffc9f185852f521	// a = 85/128
				.quad 0x3fe9d37cde997e64, 0x3ffce3ef53729f97	// a = 171/256
				.quad 0x3fea0d8b021dc00c, 0x3ffd2a7eeaec4a48	// a = 43/64
				.quad 0x3fea48280a82f83e, 0x3ffd72d6e5a66e97	// a = 173/256
				.quad 0x3fea835798fa0cba, 0x3ffdbd07cfe84d5e	// a = 87/128
				.quad 0x3feabf1d709be5fb, 0x3ffe09230787ea78	// a = 175/256
				.quad 0x3feafb7d78197bed, 0x3ffe573ac901e574	// a = 11/16
				.quad 0x3feb387bbb870d4d, 0x3ffea7623d8fe82c	// a = 177/256
				.quad 0x3feb761c6e44955e, 0x3ffef9ad8a548440	// a = 89/128
				.quad 0x3febb463ed05c38d, 0x3fff4e31e0b5b7e6	// a = 179/256
				.quad 0x3febf356bffbedc0, 0x3fffa5059001fa50	// a = 45/64
				.quad 0x3fec32f99d24b0c8, 0x3ffffe40187ea913	// a = 181/256
				.quad 0x3fec73516ac03329, 0x40002cfd200102d0	// a = 91/128
				.quad 0x3fecb46341f246c6, 0x40005c27141f5891	// a = 183/256
				.quad 0x3fecf6347191f5b5, 0x40008cabb37565e2	// a = 23/32
				.quad 0x3fed38ca812b5fa0, 0x4000be998ff8ce1a	// a = 185/256
				.quad 0x3fed7c2b3438300b, 0x4000f20010f20011	// a = 93/128
				.quad 0x3fedc05c8d936455, 0x400126ef8270fab7	// a = 187/256
				.quad 0x3fee0564d32d9391, 0x40015d79261f33f6	// a = 47/64
				.quad 0x3fee4b4a92077457, 0x400195af45931bd8	// a = 189/256
				.quad 0x3fee9214a278f736, 0x4001cfa5464e21e2	// a = 95/128
				.quad 0x3feed9ca2ccbf9e9, 0x40020b6fbf932b06	// a = 191/256
				.quad 0x3fef2272ae325a57, 0x4002492492492492	// a = 3/4
				.quad 0x3fef6c15fe200bf7, 0x400288db0323f01c	// a = 193/256
				.quad 0x3fefb6bc5412c9c8, 0x4002caabd7556820	// a = 97/128
				.quad 0x3ff0013726e90b80, 0x40030eb17410dc80	// a = 195/256
				.quad 0x3ff0279a7b19be98, 0x4003550801355080	// a = 49/64
				.quad 0x3ff04e8ce6382f3a, 0x40039dcd8f7e31ca	// a = 197/256
				.quad 0x3ff07613660e2537, 0x4003e92242a773b1	// a = 99/128
				.quad 0x3ff09e333ae64b65, 0x4004372880014373	// a = 199/256
				.quad 0x3ff0c6f1ec420786, 0x4004880522014880	// a = 25/32
				.quad 0x3ff0f0554dfbf7da, 0x4004dbdfb17409a8	// a = 201/256
				.quad 0x3ff11a6385e30cf6, 0x400532e2a5092677	// a = 101/128
				.quad 0x3ff1452311dbc75f, 0x40058d3ba8114219	// a = 203/256
				.quad 0x3ff1709ace96ee39, 0x4005eb1be9658b37	// a = 51/64
				.quad 0x3ff19cd1feef2a07, 0x40064cb87397b01f	// a = 205/256
				.quad 0x3ff1c9d0540158c9, 0x4006b24a8fb6f220	// a = 103/128
				.quad 0x3ff1f79df6163ed8, 0x40071c10342d5c96	// a = 207/256
				.quad 0x3ff226438e777c99, 0x40078a4c8178a4c8	// a = 13/16
				.quad 0x3ff255ca524d82c3, 0x4007fd484ecf128b	// a = 209/256
				.quad 0x3ff2863c0ea8b8cf, 0x40087552c91cb5b8	// a = 105/128
				.quad 0x3ff2b7a335dd4c9f, 0x4008f2c227337192	// a = 211/256
				.quad 0x3ff2ea0aee5f5810, 0x400975f4768d3a48	// a = 53/64
				.quad 0x3ff31d7f23546375, 0x4009ff5084a080c5	// a = 213/256
				.quad 0x3ff3520c9718089f, 0x400a8f46e989d487	// a = 107/128
				.quad 0x3ff387c0f7fbe5f1, 0x400b265339bb9f3a	// a = 215/256
				.quad 0x3ff3beaaf7978c58, 0x400bc4fd65883e7b	// a = 27/32
				.quad 0x3ff3f6da650c1586, 0x400c6bdb4ec15ed3	// a = 217/256
				.quad 0x3ff430604ab1313f, 0x400d1b929e6308d0	// a = 109/128
				.quad 0x3ff46b4f0fb778a3, 0x400dd4dae66843b4	// a = 219/256
				.quad 0x3ff4a7ba9e66a99b, 0x400e98801e98801f	// a = 55/64
				.quad 0x3ff4e5b88fbf4bf0, 0x400f67658e7f8c33	// a = 221/256
				.quad 0x3ff525605d6fc1b9, 0x401021449d84e212	// a = 111/128
				.quad 0x3ff566cb9b3ef248, 0x40109583f9d88406	// a = 223/256
																// a = 7/8

.literal8
.align 3
one_256th:		.quad 0x3f70000000000000 // 1/256
one:			.quad 0x3ff0000000000000
one_plus_eps:	.quad 0x3ff0000000000001 // 1 + ulp
one_third:		.quad 0x3fd5555555555555
one_fifth:		.quad 0x3fc999999999999a
minimax_c:		.quad 0x3ff0a3d70a3d70a4 // = 1.04
third_over_c:	.quad 0x3fd4834834834834 // = 1/3.12
a_mask:			.quad 0x7ffff00000000000ULL // top 8 bits of mantissa
exp_mask:		.quad 0x7ff0000000000000ULL // exponent bits / +Inf
neglog2_2:		.quad 0xbfd62e42fefa39ef // -ln(2)/2

.literal4
.align 2
f256:           .long 0x43800000         // 256.0f

.text
#if defined( __x86_64__ )
	#define RELATIVE_ADDR( _a)			(_a)( %rip )
	#define	INDEX						%r8
#elif defined( __i386__ )
	#define RELATIVE_ADDR( _a)			(_a)-atanhf_body( CX_P )
	#define INDEX						%edi
.align 4
atanhf_pic:
	movl		(%esp),					%ecx	// copy address of local_addr to %ecx
	ret
#else
	#error arch not supported
#endif

ENTRY( atanhf )
#if defined( __i386__ )
    movl		FRAME_SIZE( STACKP ),	%eax
    movss		FRAME_SIZE( STACKP ),	%xmm0
	calll		atanhf_pic
atanhf_body:
#else
    movd		%xmm0,					%eax
#endif
	andl		$0x7fffffff,			%eax
	movd		%eax,					%xmm1	// xmm1 <-- |x|
	subl		$0x3d000000,			%eax	// |x| - 1/32 as integers
	cmpl		$0x02600000,			%eax	// if (|x| < 1/32 or |x| >= 7/8 or isnan(x))
	jae			2f								//	  goto 2

	/*
	(Set with tab = 4 spaces)
		 
	For 1/32 < x < 7/8:
		 
		Compute a = floor(x*256), set hi = a/256.  a is an integer between 0 and 223.  hi is an approximation to
		x, with
		 
		(1)							0 < (x - a) < 1/256
		 
		We base our approximation on the identity
		 
													1 + x
		(2)						atanh(x) = 1/2 log ------- = 1/2 ( log(1+x) - log(1-x) )
													1 - x
		 
		Expand 1±x as follows:
		 
				(1 + x) = (1 + hi)(1 - even + odd)			(1 - x) = (1 - hi)(1 - even - odd)
		 
		and solve for b,c to get:
			
										  x - a
									c = ---------			b = -ac
										 1 - a^2
		 
		Substituting into (2), we get:
		 
				atanh(x) = 1/2 ( log(1 + hi) + log(1 + even + odd) - log(1 - hi) - log(1 + even - odd) )
		 
		(3)				 = 1/2 atanh(hi) + 1/2 ( log(1 + even + odd) - log(1 + even - odd) )
		 
		We can store atanh(hi), and also the 1/(1-hi^2) factor used to compute odd, in a lookup table indexed on a.
		A fairly straighforward analysis using (1) and the fact that a < 7/8 shows that
		 
										|even|, |odd| < 1/60
		 
		So a 4th-order taylor series will approximate the second term of (3) to very nearly the desired accuracy.
		When we compute the series, some really truly wonderful cancellation takes place, leaving us with a beautiful
		approximation:
		 
		1/2 (log(1+even+odd) - log(1+even-odd)) = odd - odd*even + odd*even^2 + 1/3 odd^3 - odd*even^3 - odd^3*even
												= odd((1 - even)(1 + even^2) + (1/3 - even)odd^2)
		 
		Unfortunately, this is not *quite* accurate enough - it results in errors of approximately .75 ulps.  So we
		tweak the approximation ever so slightly to achieve the necessary accuracy:
		 
			1/2 (log(1+even+odd) - log(1+even-odd)) = odd((1 - even)(1 + even^2) + 1.04(1/3.12 - even)odd^2)
		 
		Putting this all together, we get
		 
		(4)		atanh(x) = atanh(a) + odd((1 - even)(1 + even^2) + (1/3 - 1.04 even)odd^2) + (< .51 ulp)
		 
	- Stephen Canon, July 2007
	
	*/
	
	xorps		%xmm1,					%xmm0	// xmm0 <-- signbit(x)
	movaps		%xmm1,					%xmm2
	mulss	RELATIVE_ADDR(f256),		%xmm1	// xmm1 <-- x * 256.0
	cvtss2sd	%xmm2,					%xmm2	// xmm2 <-- (double)x
	cvttss2si	%xmm1,					AX_P	// trunc(x * 256.0)
	cvtsi2sd	%eax,					%xmm1	// xmm1 <-- (double)trunc(x * 256.0)

	shl			$4,						AX_P	// stride of table is 16 bytes
	mulsd	RELATIVE_ADDR(one_256th),	%xmm1	// xmm1 <-- hi = trunc(x * 256.0) / 256.0
	lea		RELATIVE_ADDR(small_table),DX_P
	subsd		%xmm1,					%xmm2	// xmm2 <-- x - hi
	movsd		(DX_P,AX_P,1),			%xmm7	// xmm7 <-- atanh(a)
	mulsd		8(DX_P,AX_P,1),			%xmm2	// xmm2 <-- odd
	movsd	RELATIVE_ADDR(minimax_c),	%xmm3	// xmm3 <-- 1.04
	movsd	RELATIVE_ADDR(third_over_c),%xmm5	// xmm5 <-- 1/3.12
	movsd	RELATIVE_ADDR(one),			%xmm6	// xmm6 <-- 1.0
	mulsd		%xmm2,					%xmm1	// xmm1 <-- -even
	mulsd		%xmm2,					%xmm3	// xmm3 <-- 1.04*odd
	movapd		%xmm1,					%xmm4	// xmm4 <-- -even
	mulsd		%xmm1,					%xmm1	// xmm1 <-- even*even
	addsd		%xmm4,					%xmm5	// xmm5 <-- 1/3.12 - even
	mulsd		%xmm2,					%xmm3	// xmm3 <-- 1.04*odd*odd
	addsd		%xmm6,					%xmm4	// xmm4 <-- 1.0 - even
	addsd		%xmm6,					%xmm1	// xmm1 <-- 1 + even*even
	mulsd		%xmm3,					%xmm5	// xmm5 <-- (1/3 - 1.04even)*odd^2
	mulsd		%xmm1,					%xmm4	// xmm4 <-- (1.0 - even)(1.0 + even^2)
	addsd		%xmm5,					%xmm4	// xmm4 <-- (1.0 - even)(1.0 + even^2) + (1/3 - 1.04even)*odd^2
	mulsd		%xmm2,					%xmm4	// xmm4 <-- odd*(1.0 - even)(1.0 + even^2) + (1/3 - 1.04even)*odd^2
	addsd		%xmm7,					%xmm4	// xmm4 <-- atanh(x)
	cvtsd2ss	%xmm4,					%xmm1
	orps		%xmm1,					%xmm0	//  signbit
#if defined( __i386__ )
    movss		%xmm0,					FRAME_SIZE( STACKP )
    flds		FRAME_SIZE( STACKP )
#endif
    ret

2:
	cmpl		$0x02800000,			%eax	// if (|x| < 1/32 or |x| >= 1 or isnan(x))
	jae			3f								//    goto 3

	/*
	(Set with tab = 4 spaces)
		 
	For 7/8 <= |x| < 1:
		 
		On this range, we use an approximation of the form:
		 
		(1)		atanh(x) = -1/2 log((1-x)/2) - minimax polynomial in (1-x)
		 
		The second term of (1) is just a straightforward 4-term minimax polynomial.  To compute the first term, consider
		 
				(1-x) = mantissa * 2^exponent
		 
		with 1.0 <= mantissa < 2.  Set a = floor(256(mantissa - 1)), and let hi = a/256 and lo = (mantissa-(1+hi))/(1+hi).
		Then:
		 
		(2)        -1/2 log((1-x)/2) = -ln2/2 * (exponent-1) - 1/2 log(1+hi) - 1/2 log(1+lo)
		 
		The first term can be computed directly; the second term is looked up in the table indexed on a, and the final
		term is approximated via another minimax polynomial.
		 
	- Stephen Canon, July 2007
		
	*/
	
	xorps		%xmm1,					%xmm0	// xmm0 <-- signbit(x)
	cvtss2sd	%xmm1,					%xmm1	// xmm1 <-- (double)|x|
	movsd	RELATIVE_ADDR(one),			%xmm2	// xmm2 <-- 1.0
	movsd	RELATIVE_ADDR(exp_mask),	%xmm3	// xmm3 <-- 0x7ff0000000000000ULL
	movapd		%xmm2,					%xmm4	// xmm4 <-- 1.0
	subsd		%xmm1,					%xmm2	// xmm2 <-- 1.0 - x
	movsd	RELATIVE_ADDR(a_mask),		%xmm5	// xmm5 <-- 0x7ffff00000000000ULL
	andnpd		%xmm2,					%xmm3
	movapd		%xmm3,					%xmm1
	orpd		%xmm4,					%xmm3	// xmm3 <-- mantissa(1-x)
	andpd		%xmm3,					%xmm5	// xmm5 <-- 1 + hi
	
	movlhps		%xmm2,					%xmm3
	
	subsd		%xmm5,					%xmm3	// xmm3 <-- mantissa(1-x) - (1+hi)
	lea		RELATIVE_ADDR(big_table),	DX_P
	psrlq		$52-(8+4),				%xmm1	// xmm1 <-- 8 bit integer a | 4 bit junk
	movd		%xmm1,					%eax
	and			$0xff0,					AX_P
	mulsd		8(DX_P,AX_P,1),			%xmm3	// xmm3 <-- u = [ lo , 1-x ]
	movsd		(DX_P,AX_P,1),			%xmm1	// xmm1 <-- -1/2 log(1+hi)
	
	movapd		%xmm3,					%xmm4	// xmm4 <-- u
	mulpd		%xmm3,					%xmm3	// xmm3 <-- uu
	movapd		-48(DX_P),				%xmm6	// xmm6 <-- c3/c4
	movapd		-32(DX_P),				%xmm5	// xmm5 <-- c2
	
	psrlq		$52,					%xmm2	// xmm2 <-- biased exponent of (1-x)
	movd		%xmm2,					%eax
	subl		$0x400,					%eax	// unbiased exponent - 1
	cvtsi2sd	%eax,					%xmm2	// xmm2 <-- unbiased exponent - 1
	mulsd	RELATIVE_ADDR(neglog2_2),	%xmm2	// xmm2 <-- -ln2/2 * (exponent-1)
	addsd		%xmm2,					%xmm1	// xmm1 <-- -ln2/2 * (exponent-1) - 1/2 log(1+hi)
	
	mulpd		%xmm4,					%xmm6	// xmm6 <-- c3/c4 u
	mulpd		%xmm4,					%xmm5	// xmm5 <-- c2 u
	addpd		-16(DX_P),				%xmm4	// xmm4 <-- c1/c2 + u
	addpd		%xmm3,					%xmm6	// xmm6 <-- c3/c4 u + uu
	mulpd		-64(DX_P),				%xmm3	// xmm3 <-- c4uu
	mulpd		%xmm5,					%xmm4	// xmm4 <-- c1 u + c2 u
	mulpd		%xmm3,					%xmm6	// xmm3 <-- c3 uuu + c4 uuuu
	
	// Now add up all the terms
	addsd		%xmm1,					%xmm4
	addpd		%xmm6,					%xmm4
	movhlps		%xmm4,					%xmm1
	addsd		%xmm4,					%xmm1
	
	// Convert to single precision
	cvtsd2ss	%xmm1,					%xmm1
	orps		%xmm1,					%xmm0	// signbit
#if defined( __i386__ )
    movss		%xmm0,					FRAME_SIZE( STACKP )
    flds		FRAME_SIZE( STACKP )
#endif
    ret

3:												// if (|x| >= 1 or isnan(x))
	jge			5f								//    goto 5
	
	// Now we know |x| < 1/32
	addl		$0x3d000000,			%eax	// |x|
	cmpl		$0x38000000,			%eax	// if (|x| <= 2^-13)
	jle			4f								//	  goto 4
	
	cvtss2sd	%xmm0,					%xmm1
	movapd		%xmm1,					%xmm2
	mulsd		%xmm2,					%xmm2	// xmm2 <-- x*x
	movsd	RELATIVE_ADDR(one_fifth),	%xmm3
	mulsd		%xmm2,					%xmm3	// xmm3 <-- .2xx
	addsd	RELATIVE_ADDR(one_third),	%xmm3	// xmm3 <-- .333 + .2xx
	mulsd		%xmm3,					%xmm2	// xmm2 <-- .333xx + .2xxxx
	mulsd		%xmm1,					%xmm2	// xmm2 <-- .333xxx + .2xxxxx
	addsd		%xmm2,					%xmm1	// xmm1 <-- x + .333xxx + .2xxxxx
	cvtsd2ss	%xmm1,					%xmm0
	
#if defined( __i386__ )
    movss		%xmm0,					FRAME_SIZE( STACKP )
    flds		FRAME_SIZE( STACKP )
#endif
    ret
	
4:	// very small case, just multiply by (1+ulp) and return
	cvtss2sd	%xmm0,					%xmm1
	mulsd	RELATIVE_ADDR(one_plus_eps),%xmm1	// xmm1 <-- x * (1 + ulp) for rounding and inexact.
	cvtsd2ss	%xmm1,					%xmm0
#if defined( __i386__ )
    movss		%xmm0,					FRAME_SIZE( STACKP )
    flds		FRAME_SIZE( STACKP )
#endif
    ret


5:												// if x = ±1
	je			7f								//	  goto 7
	ucomiss		%xmm0,					%xmm0	// if isnan(x)
	jp			6f								//    goto 6
	
	pcmpeqb		%xmm0,					%xmm0
	pslld		$23,					%xmm0
6:
	movaps		%xmm0,					%xmm1
	subss		%xmm1,					%xmm0
#if defined( __i386__ )
    movss		%xmm0,					FRAME_SIZE( STACKP )
    flds		FRAME_SIZE( STACKP )
#endif
    ret
	
7:
	xorps		%xmm1,					%xmm1	// xmm1 <-- 0
	divss		%xmm1,					%xmm0	// xmm0 <-- ±1/0 = ±inf, div by zero
#if defined( __i386__ )
    movss		%xmm0,					FRAME_SIZE( STACKP )
    flds		FRAME_SIZE( STACKP )
#endif
    ret
	
