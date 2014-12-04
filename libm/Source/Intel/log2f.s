/*
 *  log2f.s
 *
 *      by Ian Ollmann
 *
 *  Copyright © 2007 Apple Inc. All Rights Reserved.
 */
 
#include <machine/asm.h>
#include "abi.h"

.const
.align 4

//  256 entry of Lookup table of values used for log2 calculation, generated as:
//
//  [ollmia:/tmp] iano% cat main.c
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
//  	u.d = log2l( (long double) d );
//  	u2.d = 1.0 / d;
//  
//          printf( "0x%llx, 0x%llx,\t//log2(%7.5f), 1/%7.5f\n", u.u, u2.u, d, d );
//      }
//  
//      return 0;
//   }

log2f_table:    .quad	               0x0,	0x3ff0000000000000	//log2(1.00000), 1/1.00000
                .quad	0x3f7709c46d7aac77,	0x3fefe01fe01fe020	//log2(1.00391), 1/1.00391
                .quad	0x3f86fe50b6ef0851,	0x3fefc07f01fc07f0	//log2(1.00781), 1/1.00781
                .quad	0x3f91363117a97b0c,	0x3fefa11caa01fa12	//log2(1.01172), 1/1.01172
                .quad	0x3f96e79685c2d22a,	0x3fef81f81f81f820	//log2(1.01562), 1/1.01562
                .quad	0x3f9c9363ba850f86,	0x3fef6310aca0dbb5	//log2(1.01953), 1/1.01953
                .quad	0x3fa11cd1d5133413,	0x3fef44659e4a4271	//log2(1.02344), 1/1.02344
                .quad	0x3fa3ed3094685a26,	0x3fef25f644230ab5	//log2(1.02734), 1/1.02734
                .quad	0x3fa6bad3758efd87,	0x3fef07c1f07c1f08	//log2(1.03125), 1/1.03125
                .quad	0x3fa985bfc3495194,	0x3feee9c7f8458e02	//log2(1.03516), 1/1.03516
                .quad	0x3fac4dfab90aab5f,	0x3feecc07b301ecc0	//log2(1.03906), 1/1.03906
                .quad	0x3faf1389833253a0,	0x3feeae807aba01eb	//log2(1.04297), 1/1.04297
                .quad	0x3fb0eb389fa29f9b,	0x3fee9131abf0b767	//log2(1.04688), 1/1.04688
                .quad	0x3fb24b5b7e135a3d,	0x3fee741aa59750e4	//log2(1.05078), 1/1.05078
                .quad	0x3fb3aa2fdd27f1c3,	0x3fee573ac901e574	//log2(1.05469), 1/1.05469
                .quad	0x3fb507b836033bb7,	0x3fee3a9179dc1a73	//log2(1.05859), 1/1.05859
                .quad	0x3fb663f6fac91316,	0x3fee1e1e1e1e1e1e	//log2(1.06250), 1/1.06250
                .quad	0x3fb7beee96b8a281,	0x3fee01e01e01e01e	//log2(1.06641), 1/1.06641
                .quad	0x3fb918a16e46335b,	0x3fede5d6e3f8868a	//log2(1.07031), 1/1.07031
                .quad	0x3fba7111df348494,	0x3fedca01dca01dca	//log2(1.07422), 1/1.07422
                .quad	0x3fbbc84240adabba,	0x3fedae6076b981db	//log2(1.07812), 1/1.07812
                .quad	0x3fbd1e34e35b82da,	0x3fed92f2231e7f8a	//log2(1.08203), 1/1.08203
                .quad	0x3fbe72ec117fa5b2,	0x3fed77b654b82c34	//log2(1.08594), 1/1.08594
                .quad	0x3fbfc66a0f0b00a5,	0x3fed5cac807572b2	//log2(1.08984), 1/1.08984
                .quad	0x3fc08c588cda79e4,	0x3fed41d41d41d41d	//log2(1.09375), 1/1.09375
                .quad	0x3fc134e1b489062e,	0x3fed272ca3fc5b1a	//log2(1.09766), 1/1.09766
                .quad	0x3fc1dcd197552b7b,	0x3fed0cb58f6ec074	//log2(1.10156), 1/1.10156
                .quad	0x3fc284294b07a640,	0x3fecf26e5c44bfc6	//log2(1.10547), 1/1.10547
                .quad	0x3fc32ae9e278ae1a,	0x3fecd85689039b0b	//log2(1.10938), 1/1.10938
                .quad	0x3fc3d1146d9a8a64,	0x3fecbe6d9601cbe7	//log2(1.11328), 1/1.11328
                .quad	0x3fc476a9f983f74d,	0x3feca4b3055ee191	//log2(1.11719), 1/1.11719
                .quad	0x3fc51bab907a5c8a,	0x3fec8b265afb8a42	//log2(1.12109), 1/1.12109
                .quad	0x3fc5c01a39fbd688,	0x3fec71c71c71c71c	//log2(1.12500), 1/1.12500
                .quad	0x3fc663f6fac91316,	0x3fec5894d10d4986	//log2(1.12891), 1/1.12891
                .quad	0x3fc70742d4ef027f,	0x3fec3f8f01c3f8f0	//log2(1.13281), 1/1.13281
                .quad	0x3fc7a9fec7d05ddf,	0x3fec26b5392ea01c	//log2(1.13672), 1/1.13672
                .quad	0x3fc84c2bd02f03b3,	0x3fec0e070381c0e0	//log2(1.14062), 1/1.14062
                .quad	0x3fc8edcae8352b6c,	0x3febf583ee868d8b	//log2(1.14453), 1/1.14453
                .quad	0x3fc98edd077e70df,	0x3febdd2b899406f7	//log2(1.14844), 1/1.14844
                .quad	0x3fca2f632320b86b,	0x3febc4fd65883e7b	//log2(1.15234), 1/1.15234
                .quad	0x3fcacf5e2db4ec94,	0x3febacf914c1bad0	//log2(1.15625), 1/1.15625
                .quad	0x3fcb6ecf175f95e9,	0x3feb951e2b18ff23	//log2(1.16016), 1/1.16016
                .quad	0x3fcc0db6cdd94dee,	0x3feb7d6c3dda338b	//log2(1.16406), 1/1.16406
                .quad	0x3fccac163c770dc9,	0x3feb65e2e3beee05	//log2(1.16797), 1/1.16797
                .quad	0x3fcd49ee4c325970,	0x3feb4e81b4e81b4f	//log2(1.17188), 1/1.17188
                .quad	0x3fcde73fe3b1480f,	0x3feb37484ad806ce	//log2(1.17578), 1/1.17578
                .quad	0x3fce840be74e6a4d,	0x3feb2036406c80d9	//log2(1.17969), 1/1.17969
                .quad	0x3fcf205339208f27,	0x3feb094b31d922a4	//log2(1.18359), 1/1.18359
                .quad	0x3fcfbc16b902680a,	0x3feaf286bca1af28	//log2(1.18750), 1/1.18750
                .quad	0x3fd02baba24d0664,	0x3feadbe87f94905e	//log2(1.19141), 1/1.19141
                .quad	0x3fd0790adbb03009,	0x3feac5701ac5701b	//log2(1.19531), 1/1.19531
                .quad	0x3fd0c62975542a8f,	0x3feaaf1d2f87ebfd	//log2(1.19922), 1/1.19922
                .quad	0x3fd11307dad30b76,	0x3fea98ef606a63be	//log2(1.20312), 1/1.20312
                .quad	0x3fd15fa676bb08ff,	0x3fea82e65130e159	//log2(1.20703), 1/1.20703
                .quad	0x3fd1ac05b291f070,	0x3fea6d01a6d01a6d	//log2(1.21094), 1/1.21094
                .quad	0x3fd1f825f6d88e13,	0x3fea574107688a4a	//log2(1.21484), 1/1.21484
                .quad	0x3fd24407ab0e073a,	0x3fea41a41a41a41a	//log2(1.21875), 1/1.21875
                .quad	0x3fd28fab35b32683,	0x3fea2c2a87c51ca0	//log2(1.22266), 1/1.22266
                .quad	0x3fd2db10fc4d9aaf,	0x3fea16d3f97a4b02	//log2(1.22656), 1/1.22656
                .quad	0x3fd32639636b2836,	0x3fea01a01a01a01a	//log2(1.23047), 1/1.23047
                .quad	0x3fd37124cea4cded,	0x3fe9ec8e951033d9	//log2(1.23438), 1/1.23438
                .quad	0x3fd3bbd3a0a1dcfb,	0x3fe9d79f176b682d	//log2(1.23828), 1/1.23828
                .quad	0x3fd406463b1b0449,	0x3fe9c2d14ee4a102	//log2(1.24219), 1/1.24219
                .quad	0x3fd4507cfedd4fc4,	0x3fe9ae24ea5510da	//log2(1.24609), 1/1.24609
                .quad	0x3fd49a784bcd1b8b,	0x3fe999999999999a	//log2(1.25000), 1/1.25000
                .quad	0x3fd4e43880e8fb6a,	0x3fe9852f0d8ec0ff	//log2(1.25391), 1/1.25391
                .quad	0x3fd52dbdfc4c96b3,	0x3fe970e4f80cb872	//log2(1.25781), 1/1.25781
                .quad	0x3fd577091b3378cb,	0x3fe95cbb0be377ae	//log2(1.26172), 1/1.26172
                .quad	0x3fd5c01a39fbd688,	0x3fe948b0fcd6e9e0	//log2(1.26562), 1/1.26562
                .quad	0x3fd608f1b42948ae,	0x3fe934c67f9b2ce6	//log2(1.26953), 1/1.26953
                .quad	0x3fd6518fe4677ba7,	0x3fe920fb49d0e229	//log2(1.27344), 1/1.27344
                .quad	0x3fd699f5248cd4b8,	0x3fe90d4f120190d5	//log2(1.27734), 1/1.27734
                .quad	0x3fd6e221cd9d0cde,	0x3fe8f9c18f9c18fa	//log2(1.28125), 1/1.28125
                .quad	0x3fd72a1637cbc183,	0x3fe8e6527af1373f	//log2(1.28516), 1/1.28516
                .quad	0x3fd771d2ba7efb3c,	0x3fe8d3018d3018d3	//log2(1.28906), 1/1.28906
                .quad	0x3fd7b957ac51aac4,	0x3fe8bfce8062ff3a	//log2(1.29297), 1/1.29297
                .quad	0x3fd800a563161c54,	0x3fe8acb90f6bf3aa	//log2(1.29688), 1/1.29688
                .quad	0x3fd847bc33d8618e,	0x3fe899c0f601899c	//log2(1.30078), 1/1.30078
                .quad	0x3fd88e9c72e0b226,	0x3fe886e5f0abb04a	//log2(1.30469), 1/1.30469
                .quad	0x3fd8d54673b5c372,	0x3fe87427bcc092b9	//log2(1.30859), 1/1.30859
                .quad	0x3fd91bba891f1709,	0x3fe8618618618618	//log2(1.31250), 1/1.31250
                .quad	0x3fd961f90527409c,	0x3fe84f00c2780614	//log2(1.31641), 1/1.31641
                .quad	0x3fd9a802391e232f,	0x3fe83c977ab2bedd	//log2(1.32031), 1/1.32031
                .quad	0x3fd9edd6759b25e0,	0x3fe82a4a0182a4a0	//log2(1.32422), 1/1.32422
                .quad	0x3fda33760a7f6051,	0x3fe8181818181818	//log2(1.32812), 1/1.32812
                .quad	0x3fda78e146f7bef4,	0x3fe8060180601806	//log2(1.33203), 1/1.33203
                .quad	0x3fdabe18797f1f49,	0x3fe7f405fd017f40	//log2(1.33594), 1/1.33594
                .quad	0x3fdb031befe06434,	0x3fe7e225515a4f1d	//log2(1.33984), 1/1.33984
                .quad	0x3fdb47ebf73882a1,	0x3fe7d05f417d05f4	//log2(1.34375), 1/1.34375
                .quad	0x3fdb8c88dbf8867a,	0x3fe7beb3922e017c	//log2(1.34766), 1/1.34766
                .quad	0x3fdbd0f2e9e79031,	0x3fe7ad2208e0ecc3	//log2(1.35156), 1/1.35156
                .quad	0x3fdc152a6c24cae6,	0x3fe79baa6bb6398b	//log2(1.35547), 1/1.35547
                .quad	0x3fdc592fad295b56,	0x3fe78a4c8178a4c8	//log2(1.35938), 1/1.35938
                .quad	0x3fdc9d02f6ca47b4,	0x3fe77908119ac60d	//log2(1.36328), 1/1.36328
                .quad	0x3fdce0a4923a587d,	0x3fe767dce434a9b1	//log2(1.36719), 1/1.36719
                .quad	0x3fdd2414c80bf27d,	0x3fe756cac201756d	//log2(1.37109), 1/1.37109
                .quad	0x3fdd6753e032ea0f,	0x3fe745d1745d1746	//log2(1.37500), 1/1.37500
                .quad	0x3fddaa6222064fb9,	0x3fe734f0c541fe8d	//log2(1.37891), 1/1.37891
                .quad	0x3fdded3fd442364c,	0x3fe724287f46debc	//log2(1.38281), 1/1.38281
                .quad	0x3fde2fed3d097298,	0x3fe713786d9c7c09	//log2(1.38672), 1/1.38672
                .quad	0x3fde726aa1e754d2,	0x3fe702e05c0b8170	//log2(1.39062), 1/1.39062
                .quad	0x3fdeb4b847d15bce,	0x3fe6f26016f26017	//log2(1.39453), 1/1.39453
                .quad	0x3fdef6d67328e220,	0x3fe6e1f76b4337c7	//log2(1.39844), 1/1.39844
                .quad	0x3fdf38c567bcc541,	0x3fe6d1a62681c861	//log2(1.40234), 1/1.40234
                .quad	0x3fdf7a8568cb06cf,	0x3fe6c16c16c16c17	//log2(1.40625), 1/1.40625
                .quad	0x3fdfbc16b902680a,	0x3fe6b1490aa31a3d	//log2(1.41016), 1/1.41016
                .quad	0x3fdffd799a83ff9b,	0x3fe6a13cd1537290	//log2(1.41406), 1/1.41406
                .quad	0x3fe01f57277264e0,	0x3fe691473a88d0c0	//log2(1.41797), 1/1.41797
                .quad	0x3fe03fda8b97997f,	0x3fe6816816816817	//log2(1.42188), 1/1.42188
                .quad	0x3fe0604719f24eb2,	0x3fe6719f3601671a	//log2(1.42578), 1/1.42578
                .quad	0x3fe0809cf27f703d,	0x3fe661ec6a5122f9	//log2(1.42969), 1/1.42969
                .quad	0x3fe0a0dc34f8e1fc,	0x3fe6524f853b4aa3	//log2(1.43359), 1/1.43359
                .quad	0x3fe0c10500d63aa6,	0x3fe642c8590b2164	//log2(1.43750), 1/1.43750
                .quad	0x3fe0e117754d7c11,	0x3fe63356b88ac0de	//log2(1.44141), 1/1.44141
                .quad	0x3fe10113b153c8ea,	0x3fe623fa77016240	//log2(1.44531), 1/1.44531
                .quad	0x3fe120f9d39e1807,	0x3fe614b36831ae94	//log2(1.44922), 1/1.44922
                .quad	0x3fe140c9faa1e544,	0x3fe6058160581606	//log2(1.45312), 1/1.45312
                .quad	0x3fe160844495e006,	0x3fe5f66434292dfc	//log2(1.45703), 1/1.45703
                .quad	0x3fe18028cf72976a,	0x3fe5e75bb8d015e7	//log2(1.46094), 1/1.46094
                .quad	0x3fe19fb7b8f32421,	0x3fe5d867c3ece2a5	//log2(1.46484), 1/1.46484
                .quad	0x3fe1bf311e95d00e,	0x3fe5c9882b931057	//log2(1.46875), 1/1.46875
                .quad	0x3fe1de951d9cbba6,	0x3fe5babcc647fa91	//log2(1.47266), 1/1.47266
                .quad	0x3fe1fde3d30e8126,	0x3fe5ac056b015ac0	//log2(1.47656), 1/1.47656
                .quad	0x3fe21d1d5bb6d59a,	0x3fe59d61f123ccaa	//log2(1.48047), 1/1.48047
                .quad	0x3fe23c41d42727c8,	0x3fe58ed2308158ed	//log2(1.48438), 1/1.48438
                .quad	0x3fe25b5158b73d04,	0x3fe5805601580560	//log2(1.48828), 1/1.48828
                .quad	0x3fe27a4c0585cbf8,	0x3fe571ed3c506b3a	//log2(1.49219), 1/1.49219
                .quad	0x3fe29931f6791560,	0x3fe56397ba7c52e2	//log2(1.49609), 1/1.49609
                .quad	0x3fe2b803473f7ad1,	0x3fe5555555555555	//log2(1.50000), 1/1.50000
                .quad	0x3fe2d6c013501380,	0x3fe54725e6bb82fe	//log2(1.50391), 1/1.50391
                .quad	0x3fe2f56875eb3f26,	0x3fe5390948f40feb	//log2(1.50781), 1/1.50781
                .quad	0x3fe313fc8a1b36f2,	0x3fe52aff56a8054b	//log2(1.51172), 1/1.51172
                .quad	0x3fe3327c6ab49ca7,	0x3fe51d07eae2f815	//log2(1.51562), 1/1.51562
                .quad	0x3fe350e8325707d9,	0x3fe50f22e111c4c5	//log2(1.51953), 1/1.51953
                .quad	0x3fe36f3ffb6d9162,	0x3fe5015015015015	//log2(1.52344), 1/1.52344
                .quad	0x3fe38d83e02f5d09,	0x3fe4f38f62dd4c9b	//log2(1.52734), 1/1.52734
                .quad	0x3fe3abb3faa02167,	0x3fe4e5e0a72f0539	//log2(1.53125), 1/1.53125
                .quad	0x3fe3c9d06490ae12,	0x3fe4d843bedc2c4c	//log2(1.53516), 1/1.53516
                .quad	0x3fe3e7d9379f7016,	0x3fe4cab88725af6e	//log2(1.53906), 1/1.53906
                .quad	0x3fe405ce8d38f4bc,	0x3fe4bd3edda68fe1	//log2(1.54297), 1/1.54297
                .quad	0x3fe423b07e986aa9,	0x3fe4afd6a052bf5b	//log2(1.54688), 1/1.54688
                .quad	0x3fe4417f24c82165,	0x3fe4a27fad76014a	//log2(1.55078), 1/1.55078
                .quad	0x3fe45f3a98a20739,	0x3fe49539e3b2d067	//log2(1.55469), 1/1.55469
                .quad	0x3fe47ce2f2d02587,	0x3fe4880522014880	//log2(1.55859), 1/1.55859
                .quad	0x3fe49a784bcd1b8b,	0x3fe47ae147ae147b	//log2(1.56250), 1/1.56250
                .quad	0x3fe4b7fabbe49795,	0x3fe46dce34596066	//log2(1.56641), 1/1.56641
                .quad	0x3fe4d56a5b33cec4,	0x3fe460cbc7f5cf9a	//log2(1.57031), 1/1.57031
                .quad	0x3fe4f2c741a9f33e,	0x3fe453d9e2c776ca	//log2(1.57422), 1/1.57422
                .quad	0x3fe510118708a8f9,	0x3fe446f86562d9fb	//log2(1.57812), 1/1.57812
                .quad	0x3fe52d4942e47909,	0x3fe43a2730abee4d	//log2(1.58203), 1/1.58203
                .quad	0x3fe54a6e8ca5438e,	0x3fe42d6625d51f87	//log2(1.58594), 1/1.58594
                .quad	0x3fe567817b86b02c,	0x3fe420b5265e5951	//log2(1.58984), 1/1.58984
                .quad	0x3fe5848226989d34,	0x3fe4141414141414	//log2(1.59375), 1/1.59375
                .quad	0x3fe5a170a4bf8d5c,	0x3fe40782d10e6566	//log2(1.59766), 1/1.59766
                .quad	0x3fe5be4d0cb51435,	0x3fe3fb013fb013fb	//log2(1.60156), 1/1.60156
                .quad	0x3fe5db177508413c,	0x3fe3ee8f42a5af07	//log2(1.60547), 1/1.60547
                .quad	0x3fe5f7cff41e09af,	0x3fe3e22cbce4a902	//log2(1.60938), 1/1.60938
                .quad	0x3fe61476a031b109,	0x3fe3d5d991aa75c6	//log2(1.61328), 1/1.61328
                .quad	0x3fe6310b8f553048,	0x3fe3c995a47babe7	//log2(1.61719), 1/1.61719
                .quad	0x3fe64d8ed7719bf0,	0x3fe3bd60d9232955	//log2(1.62109), 1/1.62109
                .quad	0x3fe66a008e4788cc,	0x3fe3b13b13b13b14	//log2(1.62500), 1/1.62500
                .quad	0x3fe68660c96f6f87,	0x3fe3a524387ac822	//log2(1.62891), 1/1.62891
                .quad	0x3fe6a2af9e5a0f0a,	0x3fe3991c2c187f63	//log2(1.63281), 1/1.63281
                .quad	0x3fe6beed2250cdae,	0x3fe38d22d366088e	//log2(1.63672), 1/1.63672
                .quad	0x3fe6db196a76194a,	0x3fe3813813813814	//log2(1.64062), 1/1.64062
                .quad	0x3fe6f7348bc5c618,	0x3fe3755bd1c945ee	//log2(1.64453), 1/1.64453
                .quad	0x3fe7133e9b156c7c,	0x3fe3698df3de0748	//log2(1.64844), 1/1.64844
                .quad	0x3fe72f37ad14c5b0,	0x3fe35dce5f9f2af8	//log2(1.65234), 1/1.65234
                .quad	0x3fe74b1fd64e0754,	0x3fe3521cfb2b78c1	//log2(1.65625), 1/1.65625
                .quad	0x3fe766f72b263dee,	0x3fe34679ace01346	//log2(1.66016), 1/1.66016
                .quad	0x3fe782bdbfdda657,	0x3fe33ae45b57bcb2	//log2(1.66406), 1/1.66406
                .quad	0x3fe79e73a8900620,	0x3fe32f5ced6a1dfa	//log2(1.66797), 1/1.66797
                .quad	0x3fe7ba18f93502e4,	0x3fe323e34a2b10bf	//log2(1.67188), 1/1.67188
                .quad	0x3fe7d5adc5a078a4,	0x3fe3187758e9ebb6	//log2(1.67578), 1/1.67578
                .quad	0x3fe7f1322182cf16,	0x3fe30d190130d190	//log2(1.67969), 1/1.67969
                .quad	0x3fe80ca620694df9,	0x3fe301c82ac40260	//log2(1.68359), 1/1.68359
                .quad	0x3fe82809d5be7073,	0x3fe2f684bda12f68	//log2(1.68750), 1/1.68750
                .quad	0x3fe8435d54ca3774,	0x3fe2eb4ea1fed14b	//log2(1.69141), 1/1.69141
                .quad	0x3fe85ea0b0b27b26,	0x3fe2e025c04b8097	//log2(1.69531), 1/1.69531
                .quad	0x3fe879d3fc7b3b71,	0x3fe2d50a012d50a0	//log2(1.69922), 1/1.69922
                .quad	0x3fe894f74b06ef8b,	0x3fe2c9fb4d812ca0	//log2(1.70312), 1/1.70312
                .quad	0x3fe8b00aaf16d4a9,	0x3fe2bef98e5a3711	//log2(1.70703), 1/1.70703
                .quad	0x3fe8cb0e3b4b3bbe,	0x3fe2b404ad012b40	//log2(1.71094), 1/1.71094
                .quad	0x3fe8e6020223d661,	0x3fe2a91c92f3c105	//log2(1.71484), 1/1.71484
                .quad	0x3fe900e6160002cd,	0x3fe29e4129e4129e	//log2(1.71875), 1/1.71875
                .quad	0x3fe91bba891f1709,	0x3fe293725bb804a5	//log2(1.72266), 1/1.72266
                .quad	0x3fe9367f6da0ab2f,	0x3fe288b01288b013	//log2(1.72656), 1/1.72656
                .quad	0x3fe95134d584e2e3,	0x3fe27dfa38a1ce4d	//log2(1.73047), 1/1.73047
                .quad	0x3fe96bdad2acb5f6,	0x3fe27350b8812735	//log2(1.73438), 1/1.73438
                .quad	0x3fe9867176da382f,	0x3fe268b37cd60127	//log2(1.73828), 1/1.73828
                .quad	0x3fe9a0f8d3b0e050,	0x3fe25e22708092f1	//log2(1.74219), 1/1.74219
                .quad	0x3fe9bb70fab5ce4d,	0x3fe2539d7e9177b2	//log2(1.74609), 1/1.74609
                .quad	0x3fe9d5d9fd5010b3,	0x3fe2492492492492	//log2(1.75000), 1/1.75000
                .quad	0x3fe9f033ecc8e956,	0x3fe23eb79717605b	//log2(1.75391), 1/1.75391
                .quad	0x3fea0a7eda4c112d,	0x3fe23456789abcdf	//log2(1.75781), 1/1.75781
                .quad	0x3fea24bad6e7fb77,	0x3fe22a0122a0122a	//log2(1.76172), 1/1.76172
                .quad	0x3fea3ee7f38e181f,	0x3fe21fb78121fb78	//log2(1.76562), 1/1.76562
                .quad	0x3fea590641131564,	0x3fe21579804855e6	//log2(1.76953), 1/1.76953
                .quad	0x3fea7315d02f20c8,	0x3fe20b470c67c0d9	//log2(1.77344), 1/1.77344
                .quad	0x3fea8d16b17e2745,	0x3fe2012012012012	//log2(1.77734), 1/1.77734
                .quad	0x3feaa708f58014d3,	0x3fe1f7047dc11f70	//log2(1.78125), 1/1.78125
                .quad	0x3feac0ecac99133b,	0x3fe1ecf43c7fb84c	//log2(1.78516), 1/1.78516
                .quad	0x3feadac1e711c833,	0x3fe1e2ef3b3fb874	//log2(1.78906), 1/1.78906
                .quad	0x3feaf488b51792d6,	0x3fe1d8f5672e4abd	//log2(1.79297), 1/1.79297
                .quad	0x3feb0e4126bcc86c,	0x3fe1cf06ada2811d	//log2(1.79688), 1/1.79688
                .quad	0x3feb27eb4bf8f08a,	0x3fe1c522fc1ce059	//log2(1.80078), 1/1.80078
                .quad	0x3feb418734a9008c,	0x3fe1bb4a4046ed29	//log2(1.80469), 1/1.80469
                .quad	0x3feb5b14f08f9666,	0x3fe1b17c67f2bae3	//log2(1.80859), 1/1.80859
                .quad	0x3feb74948f5532da,	0x3fe1a7b9611a7b96	//log2(1.81250), 1/1.81250
                .quad	0x3feb8e0620887309,	0x3fe19e0119e0119e	//log2(1.81641), 1/1.81641
                .quad	0x3feba769b39e4964,	0x3fe19453808ca29c	//log2(1.82031), 1/1.82031
                .quad	0x3febc0bf57f23606,	0x3fe18ab083902bdb	//log2(1.82422), 1/1.82422
                .quad	0x3febda071cc67e6e,	0x3fe1811811811812	//log2(1.82812), 1/1.82812
                .quad	0x3febf341114464a7,	0x3fe1778a191bd684	//log2(1.83203), 1/1.83203
                .quad	0x3fec0c6d447c5dd3,	0x3fe16e0689427379	//log2(1.83594), 1/1.83594
                .quad	0x3fec258bc5664829,	0x3fe1648d50fc3201	//log2(1.83984), 1/1.83984
                .quad	0x3fec3e9ca2e1a055,	0x3fe15b1e5f75270d	//log2(1.84375), 1/1.84375
                .quad	0x3fec579febb5b658,	0x3fe151b9a3fdd5c9	//log2(1.84766), 1/1.84766
                .quad	0x3fec7095ae91e1c7,	0x3fe1485f0e0acd3b	//log2(1.85156), 1/1.85156
                .quad	0x3fec897dfa0db58e,	0x3fe13f0e8d344724	//log2(1.85547), 1/1.85547
                .quad	0x3feca258dca93316,	0x3fe135c81135c811	//log2(1.85938), 1/1.85938
                .quad	0x3fecbb2664ccfcf6,	0x3fe12c8b89edc0ac	//log2(1.86328), 1/1.86328
                .quad	0x3fecd3e6a0ca8907,	0x3fe12358e75d3033	//log2(1.86719), 1/1.86719
                .quad	0x3fecec999edc5203,	0x3fe11a3019a74826	//log2(1.87109), 1/1.87109
                .quad	0x3fed053f6d260896,	0x3fe1111111111111	//log2(1.87500), 1/1.87500
                .quad	0x3fed1dd819b4c3f1,	0x3fe107fbbe011080	//log2(1.87891), 1/1.87891
                .quad	0x3fed3663b27f31d5,	0x3fe0fef010fef011	//log2(1.88281), 1/1.88281
                .quad	0x3fed4ee24565c62b,	0x3fe0f5edfab325a2	//log2(1.88672), 1/1.88672
                .quad	0x3fed6753e032ea0f,	0x3fe0ecf56be69c90	//log2(1.89062), 1/1.89062
                .quad	0x3fed7fb8909b2a6c,	0x3fe0e40655826011	//log2(1.89453), 1/1.89453
                .quad	0x3fed9810643d6615,	0x3fe0db20a88f4696	//log2(1.89844), 1/1.89844
                .quad	0x3fedb05b68a2fb65,	0x3fe0d24456359e3a	//log2(1.90234), 1/1.90234
                .quad	0x3fedc899ab3ff56c,	0x3fe0c9714fbcda3b	//log2(1.90625), 1/1.90625
                .quad	0x3fede0cb397338a4,	0x3fe0c0a7868b4171	//log2(1.91016), 1/1.91016
                .quad	0x3fedf8f02086af2c,	0x3fe0b7e6ec259dc8	//log2(1.91406), 1/1.91406
                .quad	0x3fee11086daf7496,	0x3fe0af2f722eecb5	//log2(1.91797), 1/1.91797
                .quad	0x3fee29142e0e0140,	0x3fe0a6810a6810a7	//log2(1.92188), 1/1.92188
                .quad	0x3fee41136eae553d,	0x3fe09ddba6af8360	//log2(1.92578), 1/1.92578
                .quad	0x3fee59063c8822ce,	0x3fe0953f39010954	//log2(1.92969), 1/1.92969
                .quad	0x3fee70eca47ef86f,	0x3fe08cabb37565e2	//log2(1.93359), 1/1.93359
                .quad	0x3fee88c6b3626a73,	0x3fe0842108421084	//log2(1.93750), 1/1.93750
                .quad	0x3feea09475ee3c3a,	0x3fe07b9f29b8eae2	//log2(1.94141), 1/1.94141
                .quad	0x3feeb855f8ca88fb,	0x3fe073260a47f7c6	//log2(1.94531), 1/1.94531
                .quad	0x3feed00b488bec23,	0x3fe06ab59c7912fb	//log2(1.94922), 1/1.94922
                .quad	0x3feee7b471b3a950,	0x3fe0624dd2f1a9fc	//log2(1.95312), 1/1.95312
                .quad	0x3feeff5180afd3e4,	0x3fe059eea0727586	//log2(1.95703), 1/1.95703
                .quad	0x3fef16e281db7630,	0x3fe05197f7d73404	//log2(1.96094), 1/1.96094
                .quad	0x3fef2e67817eb845,	0x3fe04949cc1664c5	//log2(1.96484), 1/1.96484
                .quad	0x3fef45e08bcf0655,	0x3fe0410410410410	//log2(1.96875), 1/1.96875
                .quad	0x3fef5d4dacef36be,	0x3fe038c6b78247fc	//log2(1.97266), 1/1.97266
                .quad	0x3fef74aef0efafae,	0x3fe03091b51f5e1a	//log2(1.97656), 1/1.97656
                .quad	0x3fef8c0463ce8c69,	0x3fe02864fc7729e9	//log2(1.98047), 1/1.98047
                .quad	0x3fefa34e1177c233,	0x3fe0204081020408	//log2(1.98438), 1/1.98438
                .quad	0x3fefba8c05c544df,	0x3fe0182436517a37	//log2(1.98828), 1/1.98828
                .quad	0x3fefd1be4c7f2af9,	0x3fe0101010101010	//log2(1.99219), 1/1.99219
                .quad	0x3fefe8e4f15bd1a0,	0x3fe0080402010080	//log2(1.99609), 1/1.99609

.literal8
.align 3
one:            .double     1.0
onehalf:        .double     0.5
onethird:       .quad       0x3fd5555555555555      // 1/3
onequarter:     .double     0.25
onefifth:       .double     0.2
onesixth:       .quad       0x3fc5555555555555
recip_log2:     .quad       0x3ff71547652b82fe      // 1.0 / ln(2)

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
log2f_pic:      movl    (%esp),     %ecx                    //copy address of local_addr to %ecx
                ret
#else
	#error arch not supported
#endif

//
//  log2f -- overall approach
//
//      We break up log2f(x) as follows:
//
//          i = ilogb(x)                //integer part of the result
//          m = scalbn( x, -i )         //mantissa has range 1.0 <= m < 2.0
//  
//      log2f(x) = i + log2f(m)
//
//      We further break down m as :
//
//          m = (1+a/256.0)(1+r)              a = high 8 explicit bits of mantissa(m) 
//          log2f(m) = log2(1+a/256.0) + log2(1+r)
//          
//      We use the high 8 bits of the mantissa to look up log2(1+a/256.0) in log2f_table above
//      We calculate 1+r as:
//
//          1+r = m * (1 /(1+a/256.0))
//
//      We can lookup (from the same table) the value of 1/(1+a/256.0) based on a too.
//
//      So the whole calculation is:
//
//          log2f(x) = i + log2(1+a/256.0) + log2(1+r) = i + log2f_table[a][0] + log2f( m * log2f_table[a][1] )
//
//      The third term is calculated using the Taylor series:
//
//          log(x+1) = x - x**2/2 + x**3/3
//
//      Adding one more term produces excellent results, but this is good enough to get within 0.519028 ulps
//      for the entire single precision number range and saves 7 cycles.
//
//      The edge case code is done in integer to avoid setting flags, and because it is faster that way.
//

ENTRY( log2f )
//read argument into %eax
#if defined( __i386__ )
    movl    FRAME_SIZE( STACKP ),   %eax
#else
    movd    %xmm0,                  %eax
#endif

    //Get exceptional cases out of the way
    //push NaNs around to negative
    movl    %eax,                   %ecx                // set aside x
    addl    $0x00800000,            %eax                // push Infs, NaNs negative
    xorps   %xmm1,                  %xmm1               // 0.0f
    cmpl    $0x00800000,            %eax                // if( x <= 0 or Inf, NaN )
    jle     2f                                          //      goto 2
        
    //Check for narrow range near 1 where 3rd order Taylor series is not enough
    subl    $0x3ff80000,            %eax                // subtract low value in our range (+0x00800000)
    cmpl    $0x000d8000,            %eax                // check to see if we are in the range 
    jb      7f
    
    movl    $-127,                  %edx                // bias of float
    cmpl    $0x00800000,            %ecx                // if isnormal( x )
    jae     1f                                          //      goto 1
    
    //deal with denormal
    subl    $126,                   %edx                // set aside -126 as a bias adjustment (was 0 )
    orl     $0x3f800000,            %ecx                // multiply low bits by 2**127 by oring in 1.0 
    movl    $0x3f800000,            %eax                // prepare 1.0f
    movd    %ecx,                   %xmm3               // copy to vr
    movd    %eax,                   %xmm4               // copy to vr
    subss   %xmm4,                  %xmm3               // (x | 2**127) - 1.0f
    movd    %xmm3,                  %ecx                // copy normalized scaled number back to %ecx
        
1:  //normal operand
    movl    %ecx,                   %eax                // x
    andl    $0x7f800000,            %ecx                // exponent of x
    cmpl    %ecx,                   %eax                // is this a power of 2?
    je      6f                                          //  early out for power of 2, saves inexact flag for exact cases
    
    //non-power of two
    xorl    %ecx,                   %eax                // mask off exponent
    shrl    $23,                    %ecx                // push exponent to unit precision
    addl    %edx,                   %ecx                // remove bias from exponent part
    movl    %eax,                   %edx                // set aside mantissa bits
    cvtsi2sd  %ecx,                 %xmm0               // move integer part of the result to xmm0. 
    andl    $0x00007fff,            %eax                // isolate low bits of mantissa
    andl    $0x007f8000,            %edx                // isolate high bits of mantissa
    orl     $0x3f800000,            %eax                // set exponent of the mantissa to 0 + bias
    shrl    $(15-4),                %edx
    movd    %eax,                   %xmm1               // move reduced mantissa to xmm0 (1+r)
    cvtss2sd    %xmm1,              %xmm1

//do PIC   
#if defined( __i386__ )
    calll   log2f_pic                                   // set %ecx to point to local_addr
rel_addr:
#else
    andq    $0xff0,                 %rdx
#endif    

    subsd   RELATIVE_ADDR( one ),   %xmm1               // r = (1+r) - 1.0
    
    //load the address of the log2f_table
    lea     RELATIVE_ADDR(log2f_table), AX_P

    mulsd   8( AX_P, DX_P, 1),      %xmm1               // r *= 1 / (1+a)
    
    //Do a relatively small/cheap Taylor series:    log2(r+1) = (r - 0.5rr + (1/3)rrr)/ln(2)
    movsd   RELATIVE_ADDR( onethird), %xmm3             // 1/3
    movapd  %xmm1,                  %xmm2               // r
    mulsd   %xmm1,                  %xmm3               // (1/3)r
    mulsd   %xmm2,                  %xmm2               // rr
    subsd   RELATIVE_ADDR( onehalf),%xmm3               // -0.5 + (1/3)r
    mulsd   %xmm2,                  %xmm3               // -0.5rr + (1/3)rrr
    addsd   %xmm1,                  %xmm3               // r - 0.5rr + (1/3)rrr
    mulsd   RELATIVE_ADDR( recip_log2 ), %xmm3          // (r - 0.5rr + (1/3)rrr)/ln(2)
    addsd   ( AX_P, DX_P, 1),       %xmm3               // log2(1+a) + (r - 0.5rr + (1/3)rrr)/ln(2)
    addsd   %xmm3,                  %xmm0               // add in integer part
    cvtsd2ss %xmm0,                 %xmm0               // round to float
    
#if defined( __i386__ )
    movss   %xmm0,                  FRAME_SIZE(STACKP)
    flds    FRAME_SIZE(STACKP)
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

6:  //early out for power of 2
    shrl    $23,                    %eax
    addl    %edx,                   %eax
#if defined( __i386__ )
    movl    %eax,                   FRAME_SIZE( STACKP )
    fildl    FRAME_SIZE(STACKP)
#else
    cvtsi2ss %eax,                  %xmm0
#endif
    ret

7:
#if defined( __i386__ )
    cvtss2sd FRAME_SIZE(STACKP),    %xmm2               // convert to double
    xorps   %xmm0,                  %xmm0               // zero xmm0

//do PIC   
    calll   log2f_pic                                   // set %ecx to point to local_addr
rel_addr_b:
#else
    cvtss2sd    %xmm0,              %xmm2               // convert to double
#endif

    subsd   RELATIVE_ADDR_B( one ), %xmm2               // r = x - 1.0
    movapd  %xmm2,                  %xmm1               // r again
    mulsd   %xmm2,                  %xmm2

    //Do a more expensive Taylor series:    log2(r+1) = (r - 0.5rr + (1/3)rrr - 0.25rrrr + 0.2rrrrr - 1/6rrrrrr)/ln(2)
    movsd   RELATIVE_ADDR_B( onesixth ), %xmm4         // 1/6
    movsd   RELATIVE_ADDR_B( onefifth ), %xmm3         // 0.2
    mulsd   %xmm2,                  %xmm4               // 1/6rr
    mulsd   %xmm2,                  %xmm3               // 0.2rr
    addsd   RELATIVE_ADDR_B( onequarter ), %xmm4        // 0.25 + 1/6rr
    addsd   RELATIVE_ADDR_B( onethird ), %xmm3          // 1/3 _ 0.2rr
    mulsd   %xmm2,                  %xmm4               // 0.25rr + 1/6rrrr
    mulsd   %xmm2,                  %xmm3           
    addsd   RELATIVE_ADDR_B( onehalf ), %xmm4
    addsd   RELATIVE_ADDR_B( one ), %xmm3
    mulsd   %xmm2,                  %xmm4
    mulsd   %xmm1,                  %xmm3
    subsd   %xmm4,                  %xmm3
    mulsd   RELATIVE_ADDR_B( recip_log2 ), %xmm3        // ( r - 0.5rr + 1/3rrr - 0.25rrrr + 0.2rrrrr - 1/6rrrrrr ) / ln(2)

    cvtsd2ss %xmm3,                 %xmm0               // round to float
    
#if defined( __i386__ )
    movss   %xmm0,                  FRAME_SIZE(STACKP)
    flds    FRAME_SIZE(STACKP)
#endif
    ret
