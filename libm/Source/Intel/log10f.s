/*
 *  log10f.s
 *
 *      by Ian Ollmann
 *
 *  Copyright © 2007 Apple Inc. All Rights Reserved.
 */

#include <machine/asm.h>
#include "abi.h"

.const
.align 4

//  256 entry of Lookup table of values used for log10 calculation, generated as:
//
//  #include <stdio.h>
//  #include <stdint.h>
//  #include <math.h>
//  
//   int main (void) 
//    {
//  
//      int i;
//      
//      for( i = 0; i < 256; i++ )
//      {
//          double d = (double) i / 256.0 + 1.0;
//  	union
//  	{
//  		double 	d;
//  		uint64_t	u;
//  	}u, u2;        
//  
//  	u.d = (double) log10l( (long double) d );
//  	u2.d = 1.0 / d;
//  
//          printf( ".quad\t0x%llx,\t0x%llx\t// log10(%7.5f), 1/%7.5f\n", u.u, u2.u, d, d );
//      }
//  
//      return 0;
//   }

log10_m_table:      .quad                  0x0,	0x3ff0000000000000	// log10(1.00000), 1/1.00000
                    .quad	0x3f5bbd9e9482af09,	0x3fefe01fe01fe020	// log10(1.00391), 1/1.00391
                    .quad	0x3f6bafd47221ed26,	0x3fefc07f01fc07f0	// log10(1.00781), 1/1.00781
                    .quad	0x3f74b99563d2a1bd,	0x3fefa11caa01fa12	// log10(1.01172), 1/1.01172
                    .quad	0x3f7b9476a4fcd10f,	0x3fef81f81f81f820	// log10(1.01562), 1/1.01562
                    .quad	0x3f81344daa2d7553,	0x3fef6310aca0dbb5	// log10(1.01953), 1/1.01953
                    .quad	0x3f849b0851443684,	0x3fef44659e4a4271	// log10(1.02344), 1/1.02344
                    .quad	0x3f87fe71ccc4e6b0,	0x3fef25f644230ab5	// log10(1.02734), 1/1.02734
                    .quad	0x3f8b5e908eb13790,	0x3fef07c1f07c1f08	// log10(1.03125), 1/1.03125
                    .quad	0x3f8ebb6af653e2ee,	0x3feee9c7f8458e02	// log10(1.03516), 1/1.03516
                    .quad	0x3f910a83a8446c78,	0x3feecc07b301ecc0	// log10(1.03906), 1/1.03906
                    .quad	0x3f92b5b5ec0209d3,	0x3feeae807aba01eb	// log10(1.04297), 1/1.04297
                    .quad	0x3f945f4f5acb8be0,	0x3fee9131abf0b767	// log10(1.04688), 1/1.04688
                    .quad	0x3f960753003a94ef,	0x3fee741aa59750e4	// log10(1.05078), 1/1.05078
                    .quad	0x3f97adc3df3b1ff8,	0x3fee573ac901e574	// log10(1.05469), 1/1.05469
                    .quad	0x3f9952a4f22c5ae9,	0x3fee3a9179dc1a73	// log10(1.05859), 1/1.05859
                    .quad	0x3f9af5f92b00e610,	0x3fee1e1e1e1e1e1e	// log10(1.06250), 1/1.06250
                    .quad	0x3f9c97c3735e7c0a,	0x3fee01e01e01e01e	// log10(1.06641), 1/1.06641
                    .quad	0x3f9e3806acbd058f,	0x3fede5d6e3f8868a	// log10(1.07031), 1/1.07031
                    .quad	0x3f9fd6c5b0851c4c,	0x3fedca01dca01dca	// log10(1.07422), 1/1.07422
                    .quad	0x3fa0ba01a8170000,	0x3fedae6076b981db	// log10(1.07812), 1/1.07812
                    .quad	0x3fa187e12aad8077,	0x3fed92f2231e7f8a	// log10(1.08203), 1/1.08203
                    .quad	0x3fa25502c0fc314c,	0x3fed77b654b82c34	// log10(1.08594), 1/1.08594
                    .quad	0x3fa32167c82bdcda,	0x3fed5cac807572b2	// log10(1.08984), 1/1.08984
                    .quad	0x3fa3ed1199a5e425,	0x3fed41d41d41d41d	// log10(1.09375), 1/1.09375
                    .quad	0x3fa4b8018b21ed4f,	0x3fed272ca3fc5b1a	// log10(1.09766), 1/1.09766
                    .quad	0x3fa58238eeb353da,	0x3fed0cb58f6ec074	// log10(1.10156), 1/1.10156
                    .quad	0x3fa64bb912d65c07,	0x3fecf26e5c44bfc6	// log10(1.10547), 1/1.10547
                    .quad	0x3fa71483427d2a99,	0x3fecd85689039b0b	// log10(1.10938), 1/1.10938
                    .quad	0x3fa7dc98c51c8242,	0x3fecbe6d9601cbe7	// log10(1.11328), 1/1.11328
                    .quad	0x3fa8a3fadeb847f4,	0x3feca4b3055ee191	// log10(1.11719), 1/1.11719
                    .quad	0x3fa96aaacfefcf3c,	0x3fec8b265afb8a42	// log10(1.12109), 1/1.12109
                    .quad	0x3faa30a9d609efea,	0x3fec71c71c71c71c	// log10(1.12500), 1/1.12500
                    .quad	0x3faaf5f92b00e610,	0x3fec5894d10d4986	// log10(1.12891), 1/1.12891
                    .quad	0x3fabba9a058dfd84,	0x3fec3f8f01c3f8f0	// log10(1.13281), 1/1.13281
                    .quad	0x3fac7e8d993509f9,	0x3fec26b5392ea01c	// log10(1.13672), 1/1.13672
                    .quad	0x3fad41d5164facb4,	0x3fec0e070381c0e0	// log10(1.14062), 1/1.14062
                    .quad	0x3fae0471aa1868f5,	0x3febf583ee868d8b	// log10(1.14453), 1/1.14453
                    .quad	0x3faec6647eb58808,	0x3febdd2b899406f7	// log10(1.14844), 1/1.14844
                    .quad	0x3faf87aebb43ce06,	0x3febc4fd65883e7b	// log10(1.15234), 1/1.15234
                    .quad	0x3fb02428c1f08016,	0x3febacf914c1bad0	// log10(1.15625), 1/1.15625
                    .quad	0x3fb08426fcdb1ee7,	0x3feb951e2b18ff23	// log10(1.16016), 1/1.16016
                    .quad	0x3fb0e3d29d81165e,	0x3feb7d6c3dda338b	// log10(1.16406), 1/1.16406
                    .quad	0x3fb1432c31917d08,	0x3feb65e2e3beee05	// log10(1.16797), 1/1.16797
                    .quad	0x3fb1a23445501816,	0x3feb4e81b4e81b4f	// log10(1.17188), 1/1.17188
                    .quad	0x3fb200eb639a3173,	0x3feb37484ad806ce	// log10(1.17578), 1/1.17578
                    .quad	0x3fb25f5215eb594a,	0x3feb2036406c80d9	// log10(1.17969), 1/1.17969
                    .quad	0x3fb2bd68e4621371,	0x3feb094b31d922a4	// log10(1.18359), 1/1.18359
                    .quad	0x3fb31b3055c47118,	0x3feaf286bca1af28	// log10(1.18750), 1/1.18750
                    .quad	0x3fb378a8ef84971e,	0x3feadbe87f94905e	// log10(1.19141), 1/1.19141
                    .quad	0x3fb3d5d335c53179,	0x3feac5701ac5701b	// log10(1.19531), 1/1.19531
                    .quad	0x3fb432afab5dd3ff,	0x3feaaf1d2f87ebfd	// log10(1.19922), 1/1.19922
                    .quad	0x3fb48f3ed1df48fb,	0x3fea98ef606a63be	// log10(1.20312), 1/1.20312
                    .quad	0x3fb4eb812997cde4,	0x3fea82e65130e159	// log10(1.20703), 1/1.20703
                    .quad	0x3fb5477731973e85,	0x3fea6d01a6d01a6d	// log10(1.21094), 1/1.21094
                    .quad	0x3fb5a32167b32f02,	0x3fea574107688a4a	// log10(1.21484), 1/1.21484
                    .quad	0x3fb5fe80488af4fd,	0x3fea41a41a41a41a	// log10(1.21875), 1/1.21875
                    .quad	0x3fb659944f8ba02d,	0x3fea2c2a87c51ca0	// log10(1.22266), 1/1.22266
                    .quad	0x3fb6b45df6f3e2c9,	0x3fea16d3f97a4b02	// log10(1.22656), 1/1.22656
                    .quad	0x3fb70eddb7d7ea07,	0x3fea01a01a01a01a	// log10(1.23047), 1/1.23047
                    .quad	0x3fb769140a2526fd,	0x3fe9ec8e951033d9	// log10(1.23438), 1/1.23438
                    .quad	0x3fb7c30164a60836,	0x3fe9d79f176b682d	// log10(1.23828), 1/1.23828
                    .quad	0x3fb81ca63d05a44a,	0x3fe9c2d14ee4a102	// log10(1.24219), 1/1.24219
                    .quad	0x3fb8760307d355ab,	0x3fe9ae24ea5510da	// log10(1.24609), 1/1.24609
                    .quad	0x3fb8cf183886480d,	0x3fe999999999999a	// log10(1.25000), 1/1.25000
                    .quad	0x3fb927e64180f790,	0x3fe9852f0d8ec0ff	// log10(1.25391), 1/1.25391
                    .quad	0x3fb9806d9414a209,	0x3fe970e4f80cb872	// log10(1.25781), 1/1.25781
                    .quad	0x3fb9d8aea084aa9c,	0x3fe95cbb0be377ae	// log10(1.26172), 1/1.26172
                    .quad	0x3fba30a9d609efea,	0x3fe948b0fcd6e9e0	// log10(1.26562), 1/1.26562
                    .quad	0x3fba885fa2d6151e,	0x3fe934c67f9b2ce6	// log10(1.26953), 1/1.26953
                    .quad	0x3fbadfd07416be07,	0x3fe920fb49d0e229	// log10(1.27344), 1/1.27344
                    .quad	0x3fbb36fcb5f8be8a,	0x3fe90d4f120190d5	// log10(1.27734), 1/1.27734
                    .quad	0x3fbb8de4d3ab3d98,	0x3fe8f9c18f9c18fa	// log10(1.28125), 1/1.28125
                    .quad	0x3fbbe4893762cbf7,	0x3fe8e6527af1373f	// log10(1.28516), 1/1.28516
                    .quad	0x3fbc3aea4a5c6eff,	0x3fe8d3018d3018d3	// log10(1.28906), 1/1.28906
                    .quad	0x3fbc910874e09f98,	0x3fe8bfce8062ff3a	// log10(1.29297), 1/1.29297
                    .quad	0x3fbce6e41e463da5,	0x3fe8acb90f6bf3aa	// log10(1.29688), 1/1.29688
                    .quad	0x3fbd3c7dacf5780b,	0x3fe899c0f601899c	// log10(1.30078), 1/1.30078
                    .quad	0x3fbd91d5866aa99c,	0x3fe886e5f0abb04a	// log10(1.30469), 1/1.30469
                    .quad	0x3fbde6ec0f392b05,	0x3fe87427bcc092b9	// log10(1.30859), 1/1.30859
                    .quad	0x3fbe3bc1ab0e19fe,	0x3fe8618618618618	// log10(1.31250), 1/1.31250
                    .quad	0x3fbe9056bcb315e8,	0x3fe84f00c2780614	// log10(1.31641), 1/1.31641
                    .quad	0x3fbee4aba610f204,	0x3fe83c977ab2bedd	// log10(1.32031), 1/1.32031
                    .quad	0x3fbf38c0c8325d86,	0x3fe82a4a0182a4a0	// log10(1.32422), 1/1.32422
                    .quad	0x3fbf8c9683468191,	0x3fe8181818181818	// log10(1.32812), 1/1.32812
                    .quad	0x3fbfe02d36a3956d,	0x3fe8060180601806	// log10(1.33203), 1/1.33203
                    .quad	0x3fc019c2a064b486,	0x3fe7f405fd017f40	// log10(1.33594), 1/1.33594
                    .quad	0x3fc0434f7fb1f307,	0x3fe7e225515a4f1d	// log10(1.33984), 1/1.33984
                    .quad	0x3fc06cbd67a6c3b6,	0x3fe7d05f417d05f4	// log10(1.34375), 1/1.34375
                    .quad	0x3fc0960c8648e406,	0x3fe7beb3922e017c	// log10(1.34766), 1/1.34766
                    .quad	0x3fc0bf3d0937c41c,	0x3fe7ad2208e0ecc3	// log10(1.35156), 1/1.35156
                    .quad	0x3fc0e84f1dadb526,	0x3fe79baa6bb6398b	// log10(1.35547), 1/1.35547
                    .quad	0x3fc11142f0811357,	0x3fe78a4c8178a4c8	// log10(1.35938), 1/1.35938
                    .quad	0x3fc13a18ae256b99,	0x3fe77908119ac60d	// log10(1.36328), 1/1.36328
                    .quad	0x3fc162d082ac9d10,	0x3fe767dce434a9b1	// log10(1.36719), 1/1.36719
                    .quad	0x3fc18b6a99c7f679,	0x3fe756cac201756d	// log10(1.37109), 1/1.37109
                    .quad	0x3fc1b3e71ec94f7b,	0x3fe745d1745d1746	// log10(1.37500), 1/1.37500
                    .quad	0x3fc1dc463ca41df8,	0x3fe734f0c541fe8d	// log10(1.37891), 1/1.37891
                    .quad	0x3fc204881dee8777,	0x3fe724287f46debc	// log10(1.38281), 1/1.38281
                    .quad	0x3fc22cacece26ead,	0x3fe713786d9c7c09	// log10(1.38672), 1/1.38672
                    .quad	0x3fc254b4d35e7d3c,	0x3fe702e05c0b8170	// log10(1.39062), 1/1.39062
                    .quad	0x3fc27c9ffae729c1,	0x3fe6f26016f26017	// log10(1.39453), 1/1.39453
                    .quad	0x3fc2a46e8ca7ba2a,	0x3fe6e1f76b4337c7	// log10(1.39844), 1/1.39844
                    .quad	0x3fc2cc20b1734279,	0x3fe6d1a62681c861	// log10(1.40234), 1/1.40234
                    .quad	0x3fc2f3b691c5a001,	0x3fe6c16c16c16c17	// log10(1.40625), 1/1.40625
                    .quad	0x3fc31b3055c47118,	0x3fe6b1490aa31a3d	// log10(1.41016), 1/1.41016
                    .quad	0x3fc3428e2540096d,	0x3fe6a13cd1537290	// log10(1.41406), 1/1.41406
                    .quad	0x3fc369d027b462f4,	0x3fe691473a88d0c0	// log10(1.41797), 1/1.41797
                    .quad	0x3fc390f6844a0b83,	0x3fe6816816816817	// log10(1.42188), 1/1.42188
                    .quad	0x3fc3b80161d70f33,	0x3fe6719f3601671a	// log10(1.42578), 1/1.42578
                    .quad	0x3fc3def0e6dfdf85,	0x3fe661ec6a5122f9	// log10(1.42969), 1/1.42969
                    .quad	0x3fc405c539983762,	0x3fe6524f853b4aa3	// log10(1.43359), 1/1.43359
                    .quad	0x3fc42c7e7fe3fc02,	0x3fe642c8590b2164	// log10(1.43750), 1/1.43750
                    .quad	0x3fc4531cdf581ac6,	0x3fe63356b88ac0de	// log10(1.44141), 1/1.44141
                    .quad	0x3fc479a07d3b6411,	0x3fe623fa77016240	// log10(1.44531), 1/1.44531
                    .quad	0x3fc4a0097e876329,	0x3fe614b36831ae94	// log10(1.44922), 1/1.44922
                    .quad	0x3fc4c65807e93338,	0x3fe6058160581606	// log10(1.45312), 1/1.45312
                    .quad	0x3fc4ec8c3dc25169,	0x3fe5f66434292dfc	// log10(1.45703), 1/1.45703
                    .quad	0x3fc512a644296c3d,	0x3fe5e75bb8d015e7	// log10(1.46094), 1/1.46094
                    .quad	0x3fc538a63eeb3011,	0x3fe5d867c3ece2a5	// log10(1.46484), 1/1.46484
                    .quad	0x3fc55e8c518b10f8,	0x3fe5c9882b931057	// log10(1.46875), 1/1.46875
                    .quad	0x3fc584589f4411db,	0x3fe5babcc647fa91	// log10(1.47266), 1/1.47266
                    .quad	0x3fc5aa0b4b0988fa,	0x3fe5ac056b015ac0	// log10(1.47656), 1/1.47656
                    .quad	0x3fc5cfa47787e1d7,	0x3fe59d61f123ccaa	// log10(1.48047), 1/1.48047
                    .quad	0x3fc5f52447255c92,	0x3fe58ed2308158ed	// log10(1.48438), 1/1.48438
                    .quad	0x3fc61a8adc02cabd,	0x3fe5805601580560	// log10(1.48828), 1/1.48828
                    .quad	0x3fc63fd857fc49bb,	0x3fe571ed3c506b3a	// log10(1.49219), 1/1.49219
                    .quad	0x3fc6650cdca9faad,	0x3fe56397ba7c52e2	// log10(1.49609), 1/1.49609
                    .quad	0x3fc68a288b60b7fc,	0x3fe5555555555555	// log10(1.50000), 1/1.50000
                    .quad	0x3fc6af2b8532c884,	0x3fe54725e6bb82fe	// log10(1.50391), 1/1.50391
                    .quad	0x3fc6d415eaf0906b,	0x3fe5390948f40feb	// log10(1.50781), 1/1.50781
                    .quad	0x3fc6f8e7dd293fb1,	0x3fe52aff56a8054b	// log10(1.51172), 1/1.51172
                    .quad	0x3fc71da17c2b7e80,	0x3fe51d07eae2f815	// log10(1.51562), 1/1.51562
                    .quad	0x3fc74242e8061741,	0x3fe50f22e111c4c5	// log10(1.51953), 1/1.51953
                    .quad	0x3fc766cc40889e85,	0x3fe5015015015015	// log10(1.52344), 1/1.52344
                    .quad	0x3fc78b3da54418c4,	0x3fe4f38f62dd4c9b	// log10(1.52734), 1/1.52734
                    .quad	0x3fc7af97358b9e04,	0x3fe4e5e0a72f0539	// log10(1.53125), 1/1.53125
                    .quad	0x3fc7d3d91074fb64,	0x3fe4d843bedc2c4c	// log10(1.53516), 1/1.53516
                    .quad	0x3fc7f80354d952a0,	0x3fe4cab88725af6e	// log10(1.53906), 1/1.53906
                    .quad	0x3fc81c162155b785,	0x3fe4bd3edda68fe1	// log10(1.54297), 1/1.54297
                    .quad	0x3fc84011944bcb75,	0x3fe4afd6a052bf5b	// log10(1.54688), 1/1.54688
                    .quad	0x3fc863f5cbe256ed,	0x3fe4a27fad76014a	// log10(1.55078), 1/1.55078
                    .quad	0x3fc887c2e605e119,	0x3fe49539e3b2d067	// log10(1.55469), 1/1.55469
                    .quad	0x3fc8ab790069458b,	0x3fe4880522014880	// log10(1.55859), 1/1.55859
                    .quad	0x3fc8cf183886480d,	0x3fe47ae147ae147b	// log10(1.56250), 1/1.56250
                    .quad	0x3fc8f2a0ab9e2696,	0x3fe46dce34596066	// log10(1.56641), 1/1.56641
                    .quad	0x3fc9161276ba2978,	0x3fe460cbc7f5cf9a	// log10(1.57031), 1/1.57031
                    .quad	0x3fc9396db6ac31bc,	0x3fe453d9e2c776ca	// log10(1.57422), 1/1.57422
                    .quad	0x3fc95cb2880f45ba,	0x3fe446f86562d9fb	// log10(1.57812), 1/1.57812
                    .quad	0x3fc97fe107481bfb,	0x3fe43a2730abee4d	// log10(1.58203), 1/1.58203
                    .quad	0x3fc9a2f95085a45c,	0x3fe42d6625d51f87	// log10(1.58594), 1/1.58594
                    .quad	0x3fc9c5fb7fc18f86,	0x3fe420b5265e5951	// log10(1.58984), 1/1.58984
                    .quad	0x3fc9e8e7b0c0d4be,	0x3fe4141414141414	// log10(1.59375), 1/1.59375
                    .quad	0x3fca0bbdff14360e,	0x3fe40782d10e6566	// log10(1.59766), 1/1.59766
                    .quad	0x3fca2e7e8618c2d2,	0x3fe3fb013fb013fb	// log10(1.60156), 1/1.60156
                    .quad	0x3fca512960f858ae,	0x3fe3ee8f42a5af07	// log10(1.60547), 1/1.60547
                    .quad	0x3fca73beaaaa22f4,	0x3fe3e22cbce4a902	// log10(1.60938), 1/1.60938
                    .quad	0x3fca963e7df3187c,	0x3fe3d5d991aa75c6	// log10(1.61328), 1/1.61328
                    .quad	0x3fcab8a8f56677fc,	0x3fe3c995a47babe7	// log10(1.61719), 1/1.61719
                    .quad	0x3fcadafe2b6642d9,	0x3fe3bd60d9232955	// log10(1.62109), 1/1.62109
                    .quad	0x3fcafd3e3a23b680,	0x3fe3b13b13b13b14	// log10(1.62500), 1/1.62500
                    .quad	0x3fcb1f693b9fc44f,	0x3fe3a524387ac822	// log10(1.62891), 1/1.62891
                    .quad	0x3fcb417f49ab8807,	0x3fe3991c2c187f63	// log10(1.63281), 1/1.63281
                    .quad	0x3fcb63807de8bcd7,	0x3fe38d22d366088e	// log10(1.63672), 1/1.63672
                    .quad	0x3fcb856cf1ca3105,	0x3fe3813813813814	// log10(1.64062), 1/1.64062
                    .quad	0x3fcba744be943831,	0x3fe3755bd1c945ee	// log10(1.64453), 1/1.64453
                    .quad	0x3fcbc907fd5d1c40,	0x3fe3698df3de0748	// log10(1.64844), 1/1.64844
                    .quad	0x3fcbeab6c70d8cf3,	0x3fe35dce5f9f2af8	// log10(1.65234), 1/1.65234
                    .quad	0x3fcc0c5134610e26,	0x3fe3521cfb2b78c1	// log10(1.65625), 1/1.65625
                    .quad	0x3fcc2dd75de664cf,	0x3fe34679ace01346	// log10(1.66016), 1/1.66016
                    .quad	0x3fcc4f495c0002a2,	0x3fe33ae45b57bcb2	// log10(1.66406), 1/1.66406
                    .quad	0x3fcc70a746e4708a,	0x3fe32f5ced6a1dfa	// log10(1.66797), 1/1.66797
                    .quad	0x3fcc91f1369eb7ca,	0x3fe323e34a2b10bf	// log10(1.67188), 1/1.67188
                    .quad	0x3fccb327430ec9f9,	0x3fe3187758e9ebb6	// log10(1.67578), 1/1.67578
                    .quad	0x3fccd44983e9e7bd,	0x3fe30d190130d190	// log10(1.67969), 1/1.67969
                    .quad	0x3fccf55810bb0654,	0x3fe301c82ac40260	// log10(1.68359), 1/1.68359
                    .quad	0x3fcd165300e333f7,	0x3fe2f684bda12f68	// log10(1.68750), 1/1.68750
                    .quad	0x3fcd373a6b99fb0d,	0x3fe2eb4ea1fed14b	// log10(1.69141), 1/1.69141
                    .quad	0x3fcd580e67edc43d,	0x3fe2e025c04b8097	// log10(1.69531), 1/1.69531
                    .quad	0x3fcd78cf0cc4375d,	0x3fe2d50a012d50a0	// log10(1.69922), 1/1.69922
                    .quad	0x3fcd997c70da9b47,	0x3fe2c9fb4d812ca0	// log10(1.70312), 1/1.70312
                    .quad	0x3fcdba16aac6348e,	0x3fe2bef98e5a3711	// log10(1.70703), 1/1.70703
                    .quad	0x3fcdda9dd0f4a329,	0x3fe2b404ad012b40	// log10(1.71094), 1/1.71094
                    .quad	0x3fcdfb11f9ac3f03,	0x3fe2a91c92f3c105	// log10(1.71484), 1/1.71484
                    .quad	0x3fce1b733b0c7381,	0x3fe29e4129e4129e	// log10(1.71875), 1/1.71875
                    .quad	0x3fce3bc1ab0e19fe,	0x3fe293725bb804a5	// log10(1.72266), 1/1.72266
                    .quad	0x3fce5bfd5f83d342,	0x3fe288b01288b013	// log10(1.72656), 1/1.72656
                    .quad	0x3fce7c266e1a5ff2,	0x3fe27dfa38a1ce4d	// log10(1.73047), 1/1.73047
                    .quad	0x3fce9c3cec58f807,	0x3fe27350b8812735	// log10(1.73438), 1/1.73438
                    .quad	0x3fcebc40efa1a142,	0x3fe268b37cd60127	// log10(1.73828), 1/1.73828
                    .quad	0x3fcedc328d3184af,	0x3fe25e22708092f1	// log10(1.74219), 1/1.74219
                    .quad	0x3fcefc11da21432b,	0x3fe2539d7e9177b2	// log10(1.74609), 1/1.74609
                    .quad	0x3fcf1bdeeb654901,	0x3fe2492492492492	// log10(1.75000), 1/1.75000
                    .quad	0x3fcf3b99d5ce208f,	0x3fe23eb79717605b	// log10(1.75391), 1/1.75391
                    .quad	0x3fcf5b42ae08c407,	0x3fe23456789abcdf	// log10(1.75781), 1/1.75781
                    .quad	0x3fcf7ad9889eee47,	0x3fe22a0122a0122a	// log10(1.76172), 1/1.76172
                    .quad	0x3fcf9a5e79f76ac5,	0x3fe21fb78121fb78	// log10(1.76562), 1/1.76562
                    .quad	0x3fcfb9d1965664a1,	0x3fe21579804855e6	// log10(1.76953), 1/1.76953
                    .quad	0x3fcfd932f1ddb4d6,	0x3fe20b470c67c0d9	// log10(1.77344), 1/1.77344
                    .quad	0x3fcff882a08d2f89,	0x3fe2012012012012	// log10(1.77734), 1/1.77734
                    .quad	0x3fd00be05b217844,	0x3fe1f7047dc11f70	// log10(1.78125), 1/1.78125
                    .quad	0x3fd01b76a35dd375,	0x3fe1ecf43c7fb84c	// log10(1.78516), 1/1.78516
                    .quad	0x3fd02b0432c96ff0,	0x3fe1e2ef3b3fb874	// log10(1.78906), 1/1.78906
                    .quad	0x3fd03a891321a85c,	0x3fe1d8f5672e4abd	// log10(1.79297), 1/1.79297
                    .quad	0x3fd04a054e139004,	0x3fe1cf06ada2811d	// log10(1.79688), 1/1.79688
                    .quad	0x3fd05978ed3c170b,	0x3fe1c522fc1ce059	// log10(1.80078), 1/1.80078
                    .quad	0x3fd068e3fa282e3d,	0x3fe1bb4a4046ed29	// log10(1.80469), 1/1.80469
                    .quad	0x3fd078467e54ea75,	0x3fe1b17c67f2bae3	// log10(1.80859), 1/1.80859
                    .quad	0x3fd087a0832fa7ac,	0x3fe1a7b9611a7b96	// log10(1.81250), 1/1.81250
                    .quad	0x3fd096f212162b9f,	0x3fe19e0119e0119e	// log10(1.81641), 1/1.81641
                    .quad	0x3fd0a63b3456c819,	0x3fe19453808ca29c	// log10(1.82031), 1/1.82031
                    .quad	0x3fd0b57bf3307cde,	0x3fe18ab083902bdb	// log10(1.82422), 1/1.82422
                    .quad	0x3fd0c4b457d3193d,	0x3fe1811811811812	// log10(1.82812), 1/1.82812
                    .quad	0x3fd0d3e46b5f5d3e,	0x3fe1778a191bd684	// log10(1.83203), 1/1.83203
                    .quad	0x3fd0e30c36e71a7f,	0x3fe16e0689427379	// log10(1.83594), 1/1.83594
                    .quad	0x3fd0f22bc36d54b0,	0x3fe1648d50fc3201	// log10(1.83984), 1/1.83984
                    .quad	0x3fd1014319e661bd,	0x3fe15b1e5f75270d	// log10(1.84375), 1/1.84375
                    .quad	0x3fd1105243380999,	0x3fe151b9a3fdd5c9	// log10(1.84766), 1/1.84766
                    .quad	0x3fd11f594839a5bd,	0x3fe1485f0e0acd3b	// log10(1.85156), 1/1.85156
                    .quad	0x3fd12e5831b4404c,	0x3fe13f0e8d344724	// log10(1.85547), 1/1.85547
                    .quad	0x3fd13d4f0862b2e1,	0x3fe135c81135c811	// log10(1.85938), 1/1.85938
                    .quad	0x3fd14c3dd4f1c510,	0x3fe12c8b89edc0ac	// log10(1.86328), 1/1.86328
                    .quad	0x3fd15b24a0004a92,	0x3fe12358e75d3033	// log10(1.86719), 1/1.86719
                    .quad	0x3fd16a03721f4120,	0x3fe11a3019a74826	// log10(1.87109), 1/1.87109
                    .quad	0x3fd178da53d1ee01,	0x3fe1111111111111	// log10(1.87500), 1/1.87500
                    .quad	0x3fd187a94d8dfb46,	0x3fe107fbbe011080	// log10(1.87891), 1/1.87891
                    .quad	0x3fd1967067bb94b8,	0x3fe0fef010fef011	// log10(1.88281), 1/1.88281
                    .quad	0x3fd1a52faab58480,	0x3fe0f5edfab325a2	// log10(1.88672), 1/1.88672
                    .quad	0x3fd1b3e71ec94f7b,	0x3fe0ecf56be69c90	// log10(1.89062), 1/1.89062
                    .quad	0x3fd1c296cc375143,	0x3fe0e40655826011	// log10(1.89453), 1/1.89453
                    .quad	0x3fd1d13ebb32d7f9,	0x3fe0db20a88f4696	// log10(1.89844), 1/1.89844
                    .quad	0x3fd1dfdef3e23fb6,	0x3fe0d24456359e3a	// log10(1.90234), 1/1.90234
                    .quad	0x3fd1ee777e5f0dc3,	0x3fe0c9714fbcda3b	// log10(1.90625), 1/1.90625
                    .quad	0x3fd1fd0862b60b80,	0x3fe0c0a7868b4171	// log10(1.91016), 1/1.91016
                    .quad	0x3fd20b91a8e76105,	0x3fe0b7e6ec259dc8	// log10(1.91406), 1/1.91406
                    .quad	0x3fd21a1358e6af85,	0x3fe0af2f722eecb5	// log10(1.91797), 1/1.91797
                    .quad	0x3fd2288d7a9b2b64,	0x3fe0a6810a6810a7	// log10(1.92188), 1/1.92188
                    .quad	0x3fd2370015dfb60d,	0x3fe09ddba6af8360	// log10(1.92578), 1/1.92578
                    .quad	0x3fd2456b3282f786,	0x3fe0953f39010954	// log10(1.92969), 1/1.92969
                    .quad	0x3fd253ced84777be,	0x3fe08cabb37565e2	// log10(1.93359), 1/1.93359
                    .quad	0x3fd2622b0ee3b79d,	0x3fe0842108421084	// log10(1.93750), 1/1.93750
                    .quad	0x3fd2707fde0249d4,	0x3fe07b9f29b8eae2	// log10(1.94141), 1/1.94141
                    .quad	0x3fd27ecd4d41eb67,	0x3fe073260a47f7c6	// log10(1.94531), 1/1.94531
                    .quad	0x3fd28d1364359c03,	0x3fe06ab59c7912fb	// log10(1.94922), 1/1.94922
                    .quad	0x3fd29b522a64b609,	0x3fe0624dd2f1a9fc	// log10(1.95312), 1/1.95312
                    .quad	0x3fd2a989a74b0665,	0x3fe059eea0727586	// log10(1.95703), 1/1.95703
                    .quad	0x3fd2b7b9e258e422,	0x3fe05197f7d73404	// log10(1.96094), 1/1.96094
                    .quad	0x3fd2c5e2e2f347ca,	0x3fe04949cc1664c5	// log10(1.96484), 1/1.96484
                    .quad	0x3fd2d404b073e27e,	0x3fe0410410410410	// log10(1.96875), 1/1.96875
                    .quad	0x3fd2e21f522934e0,	0x3fe038c6b78247fc	// log10(1.97266), 1/1.97266
                    .quad	0x3fd2f032cf56a5be,	0x3fe03091b51f5e1a	// log10(1.97656), 1/1.97656
                    .quad	0x3fd2fe3f2f34987f,	0x3fe02864fc7729e9	// log10(1.98047), 1/1.98047
                    .quad	0x3fd30c4478f0835f,	0x3fe0204081020408	// log10(1.98438), 1/1.98438
                    .quad	0x3fd31a42b3ad0570,	0x3fe0182436517a37	// log10(1.98828), 1/1.98828
                    .quad	0x3fd32839e681fc62,	0x3fe0101010101010	// log10(1.99219), 1/1.99219
                    .quad	0x3fd3362a187c9a17,	0x3fe0080402010080	// log10(1.99609), 1/1.99609


.literal8
.align 3
one:            .double     1.0
onehalf:        .double     0.5
onethird:       .quad       0x3fd5555555555555      // 1/3
onequarter:     .double     0.25
onefifth:       .double     0.2
recip_log10:    .quad       0x3fdbcb7b1526e50d      // 1.0 / ln(10)
log10_of_2:     .quad       0x3fd34413509f79ff      // log10(2)

.text

#if defined( __x86_64__ )
	#define RELATIVE_ADDR( _a)								(_a)( %rip )
	#define RELATIVE_ADDR_B( _a)							(_a)( %rip )
	#define RELATIVE_ADDR2( _a, _i, _step)                  ( %r8, _i, _step )
#elif defined( __i386__ )
	#define RELATIVE_ADDR( _a)								(_a)-rel_addr( CX_P )
	#define RELATIVE_ADDR_B( _a)							(_a)-rel_addr_b( CX_P )
	#define RELATIVE_ADDR2( _a, _i, _step)                  (_a)-rel_addr( CX_P, _i, _step )

//a short routine to get the local address
.align 4
log10f_pic:     movl    (%esp),     %ecx                    //copy address of local_addr to %ecx
                ret
#else
	#error arch not supported
#endif

//
//  log10f -- overall approach
//
//      We break up log10f(x) as follows:
//
//          x = 2**i * m                1.0 <= m < 2.0
//          log10(x) = log10(2**i) + log10(m)
//
//      log10(2**i) is simply read from log10_e_table.
//      To obtain log10(m), we further break down m as :
//
//          m = (1+a/256.0)(1+r)              a = high 8 explicit bits of mantissa(m) 
//          log10(m) = log10(1+a/256.0) + log10(1+r)
//          
//      We use the high 8 bits of the mantissa to look up log10(1+a/256.0) in log_m_table above
//      We calculate 1+r as:
//
//          1+r = m * (1 /(1+a/256.0))
//
//      We can lookup (from the same table) the value of 1/(1+a/256.0) based on a too.
//
//      So the whole calculation is:
//
//          log10f(x) = log10(2**i) + log10(1+a/256.0) + log10(1+r) = log10_e_table[i+149] + log10_m_table[a][0] + log10( m * log_m_table[a][1] )
//
//      The third term is calculated using the Taylor series:
//
//          log10(x+1) = (x - x**2/2 + x**3/3 - x**4/4 + x**5/5) / ln(10)
//
//

ENTRY( log10f )
// Load input value
#if defined( __i386__ )
    movl        FRAME_SIZE( STACKP ),       %eax
    movss       FRAME_SIZE( STACKP ),       %xmm0
#else
    movd        %xmm0,                      %eax
#endif
    
    movl        %eax,                       %ecx            //set aside x
    addl        $0x00800000,                %eax            // push Infs, NaNs negative
    xorps       %xmm1,                      %xmm1           // 0.0f
    cmpl        $0x00800000,                %eax            // if( x <= 0 or Inf or NaN )
    jle         2f
    
    movl        $(-127),                    %edx            // negative single precision bias
    cmpl        $0x00800000,                %ecx            // if( isnormal( x ))
    jae         1f                                          //      skip denormal handling
    
    //denormal whatnot
    subl        $126,                       %edx            // accumulate in -126 as pseudo exponent of denormal
    movl        $0x3f800000,                %eax            // 1.0
    orl         %eax,                       %ecx            // multiply low bits by 2**126 by oring in 1.0
    movd        %eax,                       %xmm1           // 1.0
    movd        %ecx,                       %xmm0           // 1.0 | denorm bits
    subss       %xmm1,                      %xmm0           // (1.0 | denorm bits) - 1.0
    movd        %xmm0,                      %ecx
  
    //find log10_e_table[i] and load into %xmm0
1:  shrl        $23,                        %ecx
    addl        %ecx,                       %edx
    
#if defined( __i386__ )
    calll       log10f_pic                                    // set %ecx to point to local_addr
rel_addr:   
#endif    

    //find a, and load values from log10_m_table
    movd        %xmm0,                      %eax            // x
    cvtsi2sd    %edx,                       %xmm7           // exponent of x
    movl        %eax,                       %edx            // x
    andl        $0x00007fff,                %eax            // low bits of x
    andl        $0x007f8000,                %edx            // a << 15
    mulsd       RELATIVE_ADDR( log10_of_2 ), %xmm7          // log10( 2**exponent of x )
    orl         $0x3f800000,                %eax            // 1.0 | low bits of x
    shrl        $11,                        %edx            // a << 4
    movd        %eax,                       %xmm1           // 1.0 | low bits of x
    cvtss2sd    %xmm1,                      %xmm1           // 1.0 | low bits of x
    subsd       RELATIVE_ADDR( one ),       %xmm1           // (1.0 | low bits of x) - 1.0
    lea         RELATIVE_ADDR( log10_m_table), AX_P
    mulsd       8( AX_P, DX_P, 1 ),         %xmm1           // r = ((1.0 | low bits of x) - 1.0) / (1+a/256)
    movapd      %xmm1,                      %xmm2           // r
    mulsd       %xmm2,                      %xmm2           // rr
    movsd       ( AX_P, DX_P, 1 ),          %xmm6           // log(1+a/256.0)

    //Taylor series approximation: log(1+r) = r - rr/2 + rrr/3 - rrrr/4 + rrrrr/5
    movsd       RELATIVE_ADDR( onefifth ),  %xmm3           // 0.2
    movsd       RELATIVE_ADDR( onequarter ), %xmm4           // 0.25
    mulsd       %xmm2,                      %xmm3           // 0.2rr
    mulsd       %xmm2,                      %xmm4           // 0.25rr
    addsd       RELATIVE_ADDR( onethird),   %xmm3           // (1./3.) + 0.2rr
    addsd       RELATIVE_ADDR( onehalf),    %xmm4           // 0.5 + 0.25rr
    mulsd       %xmm2,                      %xmm3           // (1./3.)rr + 0.2rrrr
    mulsd       %xmm2,                      %xmm4           // 0.5rr + 0.25rrrr
    mulsd       %xmm1,                      %xmm3           // (1./3.)rrr + 0.2rrrrr
    subsd       %xmm4,                      %xmm1           // r - 0.5rr - 0.25rrrr
    addsd       %xmm3,                      %xmm1           // r - 0.5rr + (1./3.)rrr - 0.25rrrr + 0.2rrrrr
    mulsd       RELATIVE_ADDR(recip_log10), %xmm1           // (r - 0.5rr + (1./3.)rrr - 0.25rrrr + 0.2rrrrr)/ln(10)
    
    //add up major terms
    addsd       %xmm6,                      %xmm1           // log(1+a/256.0) + (r - 0.5rr + (1./3.)rrr - 0.25rrrr + 0.2rrrrr)/ln(10)
    addsd       %xmm7,                      %xmm1           // log_e_table[i] + log(1+a/256.0) + (r - 0.5rr + (1./3.)rrr - 0.25rrrr + 0.2rrrrr)
    
    //round to float
    xorps       %xmm0,                      %xmm0
    cvtsd2ss    %xmm1,                      %xmm0           // result
    
#if defined( __i386__ )
    movss       %xmm0,                      FRAME_SIZE(STACKP)
    flds        FRAME_SIZE(STACKP)
#endif
    ret
    
2:  //special case code for negative, NaN or 0
#if defined( __i386__ )
    movss   FRAME_SIZE( STACKP ),   %xmm0
#endif
    ucomiss %xmm1,                  %xmm0               //test for x == 0
    jb      4f                                          //NaN or negative
    
    //We imagine that 0.0, Inf are the most common cases, so those falls through
    ja      3f                                          //Infinity, just return inf
    
    //We need to return -Inf for zero and set div/0 flag
    pcmpeqb %xmm0,                  %xmm0               // -1U
    cvtdq2ps %xmm0,                 %xmm0               // -1.0f
    divss   %xmm1,                  %xmm0               // -1.0f / 0 = -Inf + div/0 flag
#if defined( __i386__ )
    movss   %xmm0,                  FRAME_SIZE(STACKP)
#endif
3:
#if defined( __i386__ )
    flds    FRAME_SIZE(STACKP)
#endif
    ret
    
4:  jp      5f                                          // handle NaN elsewhere
    
    //negative number, return NaN and set invalid
    pcmpeqb %xmm0,                  %xmm0               // -1U
    pslld   $23,                    %xmm0               // 0xff800000, -inf
    mulss   %xmm1,                  %xmm0               // 0 * -inf = NaN, set invalid
#if defined( __i386__ )
    movss   %xmm0,                  FRAME_SIZE(STACKP)
    flds    FRAME_SIZE(STACKP)
#endif
    ret

    //Its a NaN
5:
#if defined( __i386__ )
    flds    FRAME_SIZE(STACKP )                         //load the NaN
    fadd    %st(0),     %st(0)                          //quiet it
#else
    addss   %xmm0,      %xmm0
#endif
    ret

