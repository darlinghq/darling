
/*
 *  logf.s
 *
 *      by Ian Ollmann
 *
 *  Copyright (c) 2007, Apple Inc.  All rights reserved
 *
 *  Single precision implementation of C99 math library function logf and log1pf.
 *  Accurate to within 0.51 ulps over entire number range.
 *
 */
 
 
#include <machine/asm.h>
#include "abi.h"

.const
.align 4

//  511 entry of Lookup table of values used for log and log1p calculation, generated as:
//  
/*
    #include <stdint.h>
    #include <stdio.h>
    #include <math.h>

    int main (void) 
    { 
        int i;

        for( i = -255; i < 256; i++ ) 
        {  
            double d = (double) i / 256.0;  
            union  
            { 
                double d;
                uint64_t	u;
            }u, u2;        

            u.d = (double) log1pl( (long double) d );
            u2.d = 1.0 / (d+1.0);

            printf( ".quad\t0x%llx,\t0x%llx\t// log1p(%7.5f), 1/%7.5f\n", u.u, u2.u, d, d+1.0 );
        }

        return 0;
    }
*/
//  logf only uses the entries starting at log_m_table. log1pf uses the full range.

                    .quad	0xbfe62e42fefa39ef,	0x4000000000000000	// log1p(-0.50000), 1/0.50000
                    .quad	0xbfe5ee82aa241920,	0x3fffc07f01fc07f0	// log1p(-0.49609), 1/0.50391
                    .quad	0xbfe5af405c3649e0,	0x3fff81f81f81f820	// log1p(-0.49219), 1/0.50781
                    .quad	0xbfe5707a26bb8c66,	0x3fff44659e4a4271	// log1p(-0.48828), 1/0.51172
                    .quad	0xbfe5322e26867857,	0x3fff07c1f07c1f08	// log1p(-0.48438), 1/0.51562
                    .quad	0xbfe4f45a835a4e19,	0x3ffecc07b301ecc0	// log1p(-0.48047), 1/0.51953
                    .quad	0xbfe4b6fd6f970c1f,	0x3ffe9131abf0b767	// log1p(-0.47656), 1/0.52344
                    .quad	0xbfe47a1527e8a2d3,	0x3ffe573ac901e574	// log1p(-0.47266), 1/0.52734
                    .quad	0xbfe43d9ff2f923c5,	0x3ffe1e1e1e1e1e1e	// log1p(-0.46875), 1/0.53125
                    .quad	0xbfe4019c2125ca93,	0x3ffde5d6e3f8868a	// log1p(-0.46484), 1/0.53516
                    .quad	0xbfe3c6080c36bfb5,	0x3ffdae6076b981db	// log1p(-0.46094), 1/0.53906
                    .quad	0xbfe38ae2171976e7,	0x3ffd77b654b82c34	// log1p(-0.45703), 1/0.54297
                    .quad	0xbfe35028ad9d8c86,	0x3ffd41d41d41d41d	// log1p(-0.45312), 1/0.54688
                    .quad	0xbfe315da4434068b,	0x3ffd0cb58f6ec074	// log1p(-0.44922), 1/0.55078
                    .quad	0xbfe2dbf557b0df43,	0x3ffcd85689039b0b	// log1p(-0.44531), 1/0.55469
                    .quad	0xbfe2a2786d0ec107,	0x3ffca4b3055ee191	// log1p(-0.44141), 1/0.55859
                    .quad	0xbfe269621134db92,	0x3ffc71c71c71c71c	// log1p(-0.43750), 1/0.56250
                    .quad	0xbfe230b0d8bebc98,	0x3ffc3f8f01c3f8f0	// log1p(-0.43359), 1/0.56641
                    .quad	0xbfe1f8635fc61659,	0x3ffc0e070381c0e0	// log1p(-0.42969), 1/0.57031
                    .quad	0xbfe1c07849ae6007,	0x3ffbdd2b899406f7	// log1p(-0.42578), 1/0.57422
                    .quad	0xbfe188ee40f23ca6,	0x3ffbacf914c1bad0	// log1p(-0.42188), 1/0.57812
                    .quad	0xbfe151c3f6f29612,	0x3ffb7d6c3dda338b	// log1p(-0.41797), 1/0.58203
                    .quad	0xbfe11af823c75aa8,	0x3ffb4e81b4e81b4f	// log1p(-0.41406), 1/0.58594
                    .quad	0xbfe0e4898611cce1,	0x3ffb2036406c80d9	// log1p(-0.41016), 1/0.58984
                    .quad	0xbfe0ae76e2d054fa,	0x3ffaf286bca1af28	// log1p(-0.40625), 1/0.59375
                    .quad	0xbfe078bf0533c568,	0x3ffac5701ac5701b	// log1p(-0.40234), 1/0.59766
                    .quad	0xbfe04360be7603ad,	0x3ffa98ef606a63be	// log1p(-0.39844), 1/0.60156
                    .quad	0xbfe00e5ae5b207ab,	0x3ffa6d01a6d01a6d	// log1p(-0.39453), 1/0.60547
                    .quad	0xbfdfb358af7a4884,	0x3ffa41a41a41a41a	// log1p(-0.39062), 1/0.60938
                    .quad	0xbfdf4aa7ee03192d,	0x3ffa16d3f97a4b02	// log1p(-0.38672), 1/0.61328
                    .quad	0xbfdee2a156b413e5,	0x3ff9ec8e951033d9	// log1p(-0.38281), 1/0.61719
                    .quad	0xbfde7b42c3ddad73,	0x3ff9c2d14ee4a102	// log1p(-0.37891), 1/0.62109
                    .quad	0xbfde148a1a2726ce,	0x3ff999999999999a	// log1p(-0.37500), 1/0.62500
                    .quad	0xbfddae75484c9616,	0x3ff970e4f80cb872	// log1p(-0.37109), 1/0.62891
                    .quad	0xbfdd490246defa6b,	0x3ff948b0fcd6e9e0	// log1p(-0.36719), 1/0.63281
                    .quad	0xbfdce42f18064743,	0x3ff920fb49d0e229	// log1p(-0.36328), 1/0.63672
                    .quad	0xbfdc7ff9c74554c9,	0x3ff8f9c18f9c18fa	// log1p(-0.35938), 1/0.64062
                    .quad	0xbfdc1c60693fa39e,	0x3ff8d3018d3018d3	// log1p(-0.35547), 1/0.64453
                    .quad	0xbfdbb9611b80e2fb,	0x3ff8acb90f6bf3aa	// log1p(-0.35156), 1/0.64844
                    .quad	0xbfdb56fa04462909,	0x3ff886e5f0abb04a	// log1p(-0.34766), 1/0.65234
                    .quad	0xbfdaf5295248cdd0,	0x3ff8618618618618	// log1p(-0.34375), 1/0.65625
                    .quad	0xbfda93ed3c8ad9e3,	0x3ff83c977ab2bedd	// log1p(-0.33984), 1/0.66016
                    .quad	0xbfda33440224fa79,	0x3ff8181818181818	// log1p(-0.33594), 1/0.66406
                    .quad	0xbfd9d32bea15ed3b,	0x3ff7f405fd017f40	// log1p(-0.33203), 1/0.66797
                    .quad	0xbfd973a3431356ae,	0x3ff7d05f417d05f4	// log1p(-0.32812), 1/0.67188
                    .quad	0xbfd914a8635bf68a,	0x3ff7ad2208e0ecc3	// log1p(-0.32422), 1/0.67578
                    .quad	0xbfd8b639a88b2df5,	0x3ff78a4c8178a4c8	// log1p(-0.32031), 1/0.67969
                    .quad	0xbfd85855776dcbfb,	0x3ff767dce434a9b1	// log1p(-0.31641), 1/0.68359
                    .quad	0xbfd7fafa3bd8151c,	0x3ff745d1745d1746	// log1p(-0.31250), 1/0.68750
                    .quad	0xbfd79e26687cfb3e,	0x3ff724287f46debc	// log1p(-0.30859), 1/0.69141
                    .quad	0xbfd741d876c67bb1,	0x3ff702e05c0b8170	// log1p(-0.30469), 1/0.69531
                    .quad	0xbfd6e60ee6af1972,	0x3ff6e1f76b4337c7	// log1p(-0.30078), 1/0.69922
                    .quad	0xbfd68ac83e9c6a14,	0x3ff6c16c16c16c17	// log1p(-0.29688), 1/0.70312
                    .quad	0xbfd630030b3aac49,	0x3ff6a13cd1537290	// log1p(-0.29297), 1/0.70703
                    .quad	0xbfd5d5bddf595f30,	0x3ff6816816816817	// log1p(-0.28906), 1/0.71094
                    .quad	0xbfd57bf753c8d1fb,	0x3ff661ec6a5122f9	// log1p(-0.28516), 1/0.71484
                    .quad	0xbfd522ae0738a3d8,	0x3ff642c8590b2164	// log1p(-0.28125), 1/0.71875
                    .quad	0xbfd4c9e09e172c3c,	0x3ff623fa77016240	// log1p(-0.27734), 1/0.72266
                    .quad	0xbfd4718dc271c41b,	0x3ff6058160581606	// log1p(-0.27344), 1/0.72656
                    .quad	0xbfd419b423d5e8c7,	0x3ff5e75bb8d015e7	// log1p(-0.26953), 1/0.73047
                    .quad	0xbfd3c25277333184,	0x3ff5c9882b931057	// log1p(-0.26562), 1/0.73438
                    .quad	0xbfd36b6776be1117,	0x3ff5ac056b015ac0	// log1p(-0.26172), 1/0.73828
                    .quad	0xbfd314f1e1d35ce4,	0x3ff58ed2308158ed	// log1p(-0.25781), 1/0.74219
                    .quad	0xbfd2bef07cdc9354,	0x3ff571ed3c506b3a	// log1p(-0.25391), 1/0.74609
                    .quad	0xbfd269621134db92,	0x3ff5555555555555	// log1p(-0.25000), 1/0.75000
                    .quad	0xbfd214456d0eb8d4,	0x3ff5390948f40feb	// log1p(-0.24609), 1/0.75391
                    .quad	0xbfd1bf99635a6b95,	0x3ff51d07eae2f815	// log1p(-0.24219), 1/0.75781
                    .quad	0xbfd16b5ccbacfb73,	0x3ff5015015015015	// log1p(-0.23828), 1/0.76172
                    .quad	0xbfd1178e8227e47c,	0x3ff4e5e0a72f0539	// log1p(-0.23438), 1/0.76562
                    .quad	0xbfd0c42d676162e3,	0x3ff4cab88725af6e	// log1p(-0.23047), 1/0.76953
                    .quad	0xbfd07138604d5862,	0x3ff4afd6a052bf5b	// log1p(-0.22656), 1/0.77344
                    .quad	0xbfd01eae5626c691,	0x3ff49539e3b2d067	// log1p(-0.22266), 1/0.77734
                    .quad	0xbfcf991c6cb3b379,	0x3ff47ae147ae147b	// log1p(-0.21875), 1/0.78125
                    .quad	0xbfcef5ade4dcffe6,	0x3ff460cbc7f5cf9a	// log1p(-0.21484), 1/0.78516
                    .quad	0xbfce530effe71012,	0x3ff446f86562d9fb	// log1p(-0.21094), 1/0.78906
                    .quad	0xbfcdb13db0d48940,	0x3ff42d6625d51f87	// log1p(-0.20703), 1/0.79297
                    .quad	0xbfcd1037f2655e7b,	0x3ff4141414141414	// log1p(-0.20312), 1/0.79688
                    .quad	0xbfcc6ffbc6f00f71,	0x3ff3fb013fb013fb	// log1p(-0.19922), 1/0.80078
                    .quad	0xbfcbd087383bd8ad,	0x3ff3e22cbce4a902	// log1p(-0.19531), 1/0.80469
                    .quad	0xbfcb31d8575bce3d,	0x3ff3c995a47babe7	// log1p(-0.19141), 1/0.80859
                    .quad	0xbfca93ed3c8ad9e3,	0x3ff3b13b13b13b14	// log1p(-0.18750), 1/0.81250
                    .quad	0xbfc9f6c407089664,	0x3ff3991c2c187f63	// log1p(-0.18359), 1/0.81641
                    .quad	0xbfc95a5adcf7017f,	0x3ff3813813813814	// log1p(-0.17969), 1/0.82031
                    .quad	0xbfc8beafeb38fe8c,	0x3ff3698df3de0748	// log1p(-0.17578), 1/0.82422
                    .quad	0xbfc823c16551a3c2,	0x3ff3521cfb2b78c1	// log1p(-0.17188), 1/0.82812
                    .quad	0xbfc7898d85444c73,	0x3ff33ae45b57bcb2	// log1p(-0.16797), 1/0.83203
                    .quad	0xbfc6f0128b756abc,	0x3ff323e34a2b10bf	// log1p(-0.16406), 1/0.83594
                    .quad	0xbfc6574ebe8c133a,	0x3ff30d190130d190	// log1p(-0.16016), 1/0.83984
                    .quad	0xbfc5bf406b543db2,	0x3ff2f684bda12f68	// log1p(-0.15625), 1/0.84375
                    .quad	0xbfc527e5e4a1b58d,	0x3ff2e025c04b8097	// log1p(-0.15234), 1/0.84766
                    .quad	0xbfc4913d8333b561,	0x3ff2c9fb4d812ca0	// log1p(-0.14844), 1/0.85156
                    .quad	0xbfc3fb45a59928cc,	0x3ff2b404ad012b40	// log1p(-0.14453), 1/0.85547
                    .quad	0xbfc365fcb0159016,	0x3ff29e4129e4129e	// log1p(-0.14062), 1/0.85938
                    .quad	0xbfc2d1610c86813a,	0x3ff288b01288b013	// log1p(-0.13672), 1/0.86328
                    .quad	0xbfc23d712a49c202,	0x3ff27350b8812735	// log1p(-0.13281), 1/0.86719
                    .quad	0xbfc1aa2b7e23f72a,	0x3ff25e22708092f1	// log1p(-0.12891), 1/0.87109
                    .quad	0xbfc1178e8227e47c,	0x3ff2492492492492	// log1p(-0.12500), 1/0.87500
                    .quad	0xbfc08598b59e3a07,	0x3ff23456789abcdf	// log1p(-0.12109), 1/0.87891
                    .quad	0xbfbfe89139dbd566,	0x3ff21fb78121fb78	// log1p(-0.11719), 1/0.88281
                    .quad	0xbfbec739830a1120,	0x3ff20b470c67c0d9	// log1p(-0.11328), 1/0.88672
                    .quad	0xbfbda727638446a2,	0x3ff1f7047dc11f70	// log1p(-0.10938), 1/0.89062
                    .quad	0xbfbc885801bc4b23,	0x3ff1e2ef3b3fb874	// log1p(-0.10547), 1/0.89453
                    .quad	0xbfbb6ac88dad5b1c,	0x3ff1cf06ada2811d	// log1p(-0.10156), 1/0.89844
                    .quad	0xbfba4e7640b1bc38,	0x3ff1bb4a4046ed29	// log1p(-0.09766), 1/0.90234
                    .quad	0xbfb9335e5d594989,	0x3ff1a7b9611a7b96	// log1p(-0.09375), 1/0.90625
                    .quad	0xbfb8197e2f40e3f0,	0x3ff19453808ca29c	// log1p(-0.08984), 1/0.91016
                    .quad	0xbfb700d30aeac0e1,	0x3ff1811811811812	// log1p(-0.08594), 1/0.91406
                    .quad	0xbfb5e95a4d9791cb,	0x3ff16e0689427379	// log1p(-0.08203), 1/0.91797
                    .quad	0xbfb4d3115d207eac,	0x3ff15b1e5f75270d	// log1p(-0.07812), 1/0.92188
                    .quad	0xbfb3bdf5a7d1ee64,	0x3ff1485f0e0acd3b	// log1p(-0.07422), 1/0.92578
                    .quad	0xbfb2aa04a44717a5,	0x3ff135c81135c811	// log1p(-0.07031), 1/0.92969
                    .quad	0xbfb1973bd1465567,	0x3ff12358e75d3033	// log1p(-0.06641), 1/0.93359
                    .quad	0xbfb08598b59e3a07,	0x3ff1111111111111	// log1p(-0.06250), 1/0.93750
                    .quad	0xbfaeea31c006b87c,	0x3ff0fef010fef011	// log1p(-0.05859), 1/0.94141
                    .quad	0xbfaccb73cdddb2cc,	0x3ff0ecf56be69c90	// log1p(-0.05469), 1/0.94531
                    .quad	0xbfaaaef2d0fb10fc,	0x3ff0db20a88f4696	// log1p(-0.05078), 1/0.94922
                    .quad	0xbfa894aa149fb343,	0x3ff0c9714fbcda3b	// log1p(-0.04688), 1/0.95312
                    .quad	0xbfa67c94f2d4bb58,	0x3ff0b7e6ec259dc8	// log1p(-0.04297), 1/0.95703
                    .quad	0xbfa466aed42de3ea,	0x3ff0a6810a6810a7	// log1p(-0.03906), 1/0.96094
                    .quad	0xbfa252f32f8d183f,	0x3ff0953f39010954	// log1p(-0.03516), 1/0.96484
                    .quad	0xbfa0415d89e74444,	0x3ff0842108421084	// log1p(-0.03125), 1/0.96875
                    .quad	0xbf9c63d2ec14aaf2,	0x3ff073260a47f7c6	// log1p(-0.02734), 1/0.97266
                    .quad	0xbf98492528c8cabf,	0x3ff0624dd2f1a9fc	// log1p(-0.02344), 1/0.97656
                    .quad	0xbf9432a925980cc1,	0x3ff05197f7d73404	// log1p(-0.01953), 1/0.98047
                    .quad	0xbf90205658935847,	0x3ff0410410410410	// log1p(-0.01562), 1/0.98438
                    .quad	0xbf882448a388a2aa,	0x3ff03091b51f5e1a	// log1p(-0.01172), 1/0.98828
                    .quad	0xbf8010157588de71,	0x3ff0204081020408	// log1p(-0.00781), 1/0.99219
                    .quad	0xbf70080559588b35,	0x3ff0101010101010	// log1p(-0.00391), 1/0.99609
log_m_table:        .quad	0x0000000000000000,	0x3ff0000000000000	// log1p(0.00000), 1/1.00000
                    .quad	0x3f6ff00aa2b10bc0,	0x3fefe01fe01fe020	// log1p(0.00391), 1/1.00391
                    .quad	0x3f7fe02a6b106789,	0x3fefc07f01fc07f0	// log1p(0.00781), 1/1.00781
                    .quad	0x3f87dc475f810a77,	0x3fefa11caa01fa12	// log1p(0.01172), 1/1.01172
                    .quad	0x3f8fc0a8b0fc03e4,	0x3fef81f81f81f820	// log1p(0.01562), 1/1.01562
                    .quad	0x3f93cea44346a575,	0x3fef6310aca0dbb5	// log1p(0.01953), 1/1.01953
                    .quad	0x3f97b91b07d5b11b,	0x3fef44659e4a4271	// log1p(0.02344), 1/1.02344
                    .quad	0x3f9b9fc027af9198,	0x3fef25f644230ab5	// log1p(0.02734), 1/1.02734
                    .quad	0x3f9f829b0e783300,	0x3fef07c1f07c1f08	// log1p(0.03125), 1/1.03125
                    .quad	0x3fa1b0d98923d980,	0x3feee9c7f8458e02	// log1p(0.03516), 1/1.03516
                    .quad	0x3fa39e87b9febd60,	0x3feecc07b301ecc0	// log1p(0.03906), 1/1.03906
                    .quad	0x3fa58a5bafc8e4d5,	0x3feeae807aba01eb	// log1p(0.04297), 1/1.04297
                    .quad	0x3fa77458f632dcfc,	0x3fee9131abf0b767	// log1p(0.04688), 1/1.04688
                    .quad	0x3fa95c830ec8e3eb,	0x3fee741aa59750e4	// log1p(0.05078), 1/1.05078
                    .quad	0x3fab42dd711971bf,	0x3fee573ac901e574	// log1p(0.05469), 1/1.05469
                    .quad	0x3fad276b8adb0b52,	0x3fee3a9179dc1a73	// log1p(0.05859), 1/1.05859
                    .quad	0x3faf0a30c01162a6,	0x3fee1e1e1e1e1e1e	// log1p(0.06250), 1/1.06250
                    .quad	0x3fb075983598e471,	0x3fee01e01e01e01e	// log1p(0.06641), 1/1.06641
                    .quad	0x3fb16536eea37ae1,	0x3fede5d6e3f8868a	// log1p(0.07031), 1/1.07031
                    .quad	0x3fb253f62f0a1417,	0x3fedca01dca01dca	// log1p(0.07422), 1/1.07422
                    .quad	0x3fb341d7961bd1d1,	0x3fedae6076b981db	// log1p(0.07812), 1/1.07812
                    .quad	0x3fb42edcbea646f0,	0x3fed92f2231e7f8a	// log1p(0.08203), 1/1.08203
                    .quad	0x3fb51b073f06183f,	0x3fed77b654b82c34	// log1p(0.08594), 1/1.08594
                    .quad	0x3fb60658a93750c4,	0x3fed5cac807572b2	// log1p(0.08984), 1/1.08984
                    .quad	0x3fb6f0d28ae56b4c,	0x3fed41d41d41d41d	// log1p(0.09375), 1/1.09375
                    .quad	0x3fb7da766d7b12cd,	0x3fed272ca3fc5b1a	// log1p(0.09766), 1/1.09766
                    .quad	0x3fb8c345d6319b21,	0x3fed0cb58f6ec074	// log1p(0.10156), 1/1.10156
                    .quad	0x3fb9ab42462033ad,	0x3fecf26e5c44bfc6	// log1p(0.10547), 1/1.10547
                    .quad	0x3fba926d3a4ad563,	0x3fecd85689039b0b	// log1p(0.10938), 1/1.10938
                    .quad	0x3fbb78c82bb0eda1,	0x3fecbe6d9601cbe7	// log1p(0.11328), 1/1.11328
                    .quad	0x3fbc5e548f5bc743,	0x3feca4b3055ee191	// log1p(0.11719), 1/1.11719
                    .quad	0x3fbd4313d66cb35d,	0x3fec8b265afb8a42	// log1p(0.12109), 1/1.12109
                    .quad	0x3fbe27076e2af2e6,	0x3fec71c71c71c71c	// log1p(0.12500), 1/1.12500
                    .quad	0x3fbf0a30c01162a6,	0x3fec5894d10d4986	// log1p(0.12891), 1/1.12891
                    .quad	0x3fbfec9131dbeabb,	0x3fec3f8f01c3f8f0	// log1p(0.13281), 1/1.13281
                    .quad	0x3fc0671512ca596e,	0x3fec26b5392ea01c	// log1p(0.13672), 1/1.13672
                    .quad	0x3fc0d77e7cd08e59,	0x3fec0e070381c0e0	// log1p(0.14062), 1/1.14062
                    .quad	0x3fc14785846742ac,	0x3febf583ee868d8b	// log1p(0.14453), 1/1.14453
                    .quad	0x3fc1b72ad52f67a0,	0x3febdd2b899406f7	// log1p(0.14844), 1/1.14844
                    .quad	0x3fc2266f190a5acb,	0x3febc4fd65883e7b	// log1p(0.15234), 1/1.15234
                    .quad	0x3fc29552f81ff523,	0x3febacf914c1bad0	// log1p(0.15625), 1/1.15625
                    .quad	0x3fc303d718e47fd3,	0x3feb951e2b18ff23	// log1p(0.16016), 1/1.16016
                    .quad	0x3fc371fc201e8f74,	0x3feb7d6c3dda338b	// log1p(0.16406), 1/1.16406
                    .quad	0x3fc3dfc2b0ecc62a,	0x3feb65e2e3beee05	// log1p(0.16797), 1/1.16797
                    .quad	0x3fc44d2b6ccb7d1e,	0x3feb4e81b4e81b4f	// log1p(0.17188), 1/1.17188
                    .quad	0x3fc4ba36f39a55e5,	0x3feb37484ad806ce	// log1p(0.17578), 1/1.17578
                    .quad	0x3fc526e5e3a1b438,	0x3feb2036406c80d9	// log1p(0.17969), 1/1.17969
                    .quad	0x3fc59338d9982086,	0x3feb094b31d922a4	// log1p(0.18359), 1/1.18359
                    .quad	0x3fc5ff3070a793d4,	0x3feaf286bca1af28	// log1p(0.18750), 1/1.18750
                    .quad	0x3fc66acd4272ad51,	0x3feadbe87f94905e	// log1p(0.19141), 1/1.19141
                    .quad	0x3fc6d60fe719d21d,	0x3feac5701ac5701b	// log1p(0.19531), 1/1.19531
                    .quad	0x3fc740f8f54037a5,	0x3feaaf1d2f87ebfd	// log1p(0.19922), 1/1.19922
                    .quad	0x3fc7ab890210d909,	0x3fea98ef606a63be	// log1p(0.20312), 1/1.20312
                    .quad	0x3fc815c0a14357eb,	0x3fea82e65130e159	// log1p(0.20703), 1/1.20703
                    .quad	0x3fc87fa06520c911,	0x3fea6d01a6d01a6d	// log1p(0.21094), 1/1.21094
                    .quad	0x3fc8e928de886d41,	0x3fea574107688a4a	// log1p(0.21484), 1/1.21484
                    .quad	0x3fc9525a9cf456b4,	0x3fea41a41a41a41a	// log1p(0.21875), 1/1.21875
                    .quad	0x3fc9bb362e7dfb83,	0x3fea2c2a87c51ca0	// log1p(0.22266), 1/1.22266
                    .quad	0x3fca23bc1fe2b563,	0x3fea16d3f97a4b02	// log1p(0.22656), 1/1.22656
                    .quad	0x3fca8becfc882f19,	0x3fea01a01a01a01a	// log1p(0.23047), 1/1.23047
                    .quad	0x3fcaf3c94e80bff3,	0x3fe9ec8e951033d9	// log1p(0.23438), 1/1.23438
                    .quad	0x3fcb5b519e8fb5a4,	0x3fe9d79f176b682d	// log1p(0.23828), 1/1.23828
                    .quad	0x3fcbc286742d8cd6,	0x3fe9c2d14ee4a102	// log1p(0.24219), 1/1.24219
                    .quad	0x3fcc2968558c18c1,	0x3fe9ae24ea5510da	// log1p(0.24609), 1/1.24609
                    .quad	0x3fcc8ff7c79a9a22,	0x3fe999999999999a	// log1p(0.25000), 1/1.25000
                    .quad	0x3fccf6354e09c5dc,	0x3fe9852f0d8ec0ff	// log1p(0.25391), 1/1.25391
                    .quad	0x3fcd5c216b4fbb91,	0x3fe970e4f80cb872	// log1p(0.25781), 1/1.25781
                    .quad	0x3fcdc1bca0abec7d,	0x3fe95cbb0be377ae	// log1p(0.26172), 1/1.26172
                    .quad	0x3fce27076e2af2e6,	0x3fe948b0fcd6e9e0	// log1p(0.26562), 1/1.26562
                    .quad	0x3fce8c0252aa5a60,	0x3fe934c67f9b2ce6	// log1p(0.26953), 1/1.26953
                    .quad	0x3fcef0adcbdc5936,	0x3fe920fb49d0e229	// log1p(0.27344), 1/1.27344
                    .quad	0x3fcf550a564b7b37,	0x3fe90d4f120190d5	// log1p(0.27734), 1/1.27734
                    .quad	0x3fcfb9186d5e3e2b,	0x3fe8f9c18f9c18fa	// log1p(0.28125), 1/1.28125
                    .quad	0x3fd00e6c45ad501d,	0x3fe8e6527af1373f	// log1p(0.28516), 1/1.28516
                    .quad	0x3fd0402594b4d041,	0x3fe8d3018d3018d3	// log1p(0.28906), 1/1.28906
                    .quad	0x3fd071b85fcd590d,	0x3fe8bfce8062ff3a	// log1p(0.29297), 1/1.29297
                    .quad	0x3fd0a324e27390e3,	0x3fe8acb90f6bf3aa	// log1p(0.29688), 1/1.29688
                    .quad	0x3fd0d46b579ab74b,	0x3fe899c0f601899c	// log1p(0.30078), 1/1.30078
                    .quad	0x3fd1058bf9ae4ad5,	0x3fe886e5f0abb04a	// log1p(0.30469), 1/1.30469
                    .quad	0x3fd136870293a8b0,	0x3fe87427bcc092b9	// log1p(0.30859), 1/1.30859
                    .quad	0x3fd1675cababa60e,	0x3fe8618618618618	// log1p(0.31250), 1/1.31250
                    .quad	0x3fd1980d2dd4236f,	0x3fe84f00c2780614	// log1p(0.31641), 1/1.31641
                    .quad	0x3fd1c898c16999fb,	0x3fe83c977ab2bedd	// log1p(0.32031), 1/1.32031
                    .quad	0x3fd1f8ff9e48a2f3,	0x3fe82a4a0182a4a0	// log1p(0.32422), 1/1.32422
                    .quad	0x3fd22941fbcf7966,	0x3fe8181818181818	// log1p(0.32812), 1/1.32812
                    .quad	0x3fd2596010df763a,	0x3fe8060180601806	// log1p(0.33203), 1/1.33203
                    .quad	0x3fd2895a13de86a3,	0x3fe7f405fd017f40	// log1p(0.33594), 1/1.33594
                    .quad	0x3fd2b9303ab89d25,	0x3fe7e225515a4f1d	// log1p(0.33984), 1/1.33984
                    .quad	0x3fd2e8e2bae11d31,	0x3fe7d05f417d05f4	// log1p(0.34375), 1/1.34375
                    .quad	0x3fd31871c9544185,	0x3fe7beb3922e017c	// log1p(0.34766), 1/1.34766
                    .quad	0x3fd347dd9a987d55,	0x3fe7ad2208e0ecc3	// log1p(0.35156), 1/1.35156
                    .quad	0x3fd3772662bfd85b,	0x3fe79baa6bb6398b	// log1p(0.35547), 1/1.35547
                    .quad	0x3fd3a64c556945ea,	0x3fe78a4c8178a4c8	// log1p(0.35938), 1/1.35938
                    .quad	0x3fd3d54fa5c1f710,	0x3fe77908119ac60d	// log1p(0.36328), 1/1.36328
                    .quad	0x3fd404308686a7e4,	0x3fe767dce434a9b1	// log1p(0.36719), 1/1.36719
                    .quad	0x3fd432ef2a04e814,	0x3fe756cac201756d	// log1p(0.37109), 1/1.37109
                    .quad	0x3fd4618bc21c5ec2,	0x3fe745d1745d1746	// log1p(0.37500), 1/1.37500
                    .quad	0x3fd49006804009d1,	0x3fe734f0c541fe8d	// log1p(0.37891), 1/1.37891
                    .quad	0x3fd4be5f957778a1,	0x3fe724287f46debc	// log1p(0.38281), 1/1.38281
                    .quad	0x3fd4ec973260026a,	0x3fe713786d9c7c09	// log1p(0.38672), 1/1.38672
                    .quad	0x3fd51aad872df82d,	0x3fe702e05c0b8170	// log1p(0.39062), 1/1.39062
                    .quad	0x3fd548a2c3add263,	0x3fe6f26016f26017	// log1p(0.39453), 1/1.39453
                    .quad	0x3fd5767717455a6c,	0x3fe6e1f76b4337c7	// log1p(0.39844), 1/1.39844
                    .quad	0x3fd5a42ab0f4cfe2,	0x3fe6d1a62681c861	// log1p(0.40234), 1/1.40234
                    .quad	0x3fd5d1bdbf5809ca,	0x3fe6c16c16c16c17	// log1p(0.40625), 1/1.40625
                    .quad	0x3fd5ff3070a793d4,	0x3fe6b1490aa31a3d	// log1p(0.41016), 1/1.41016
                    .quad	0x3fd62c82f2b9c795,	0x3fe6a13cd1537290	// log1p(0.41406), 1/1.41406
                    .quad	0x3fd659b57303e1f3,	0x3fe691473a88d0c0	// log1p(0.41797), 1/1.41797
                    .quad	0x3fd686c81e9b14af,	0x3fe6816816816817	// log1p(0.42188), 1/1.42188
                    .quad	0x3fd6b3bb2235943e,	0x3fe6719f3601671a	// log1p(0.42578), 1/1.42578
                    .quad	0x3fd6e08eaa2ba1e4,	0x3fe661ec6a5122f9	// log1p(0.42969), 1/1.42969
                    .quad	0x3fd70d42e2789236,	0x3fe6524f853b4aa3	// log1p(0.43359), 1/1.43359
                    .quad	0x3fd739d7f6bbd007,	0x3fe642c8590b2164	// log1p(0.43750), 1/1.43750
                    .quad	0x3fd7664e1239dbcf,	0x3fe63356b88ac0de	// log1p(0.44141), 1/1.44141
                    .quad	0x3fd792a55fdd47a2,	0x3fe623fa77016240	// log1p(0.44531), 1/1.44531
                    .quad	0x3fd7bede0a37afc0,	0x3fe614b36831ae94	// log1p(0.44922), 1/1.44922
                    .quad	0x3fd7eaf83b82afc3,	0x3fe6058160581606	// log1p(0.45312), 1/1.45312
                    .quad	0x3fd816f41da0d496,	0x3fe5f66434292dfc	// log1p(0.45703), 1/1.45703
                    .quad	0x3fd842d1da1e8b17,	0x3fe5e75bb8d015e7	// log1p(0.46094), 1/1.46094
                    .quad	0x3fd86e919a330ba0,	0x3fe5d867c3ece2a5	// log1p(0.46484), 1/1.46484
                    .quad	0x3fd89a3386c1425b,	0x3fe5c9882b931057	// log1p(0.46875), 1/1.46875
                    .quad	0x3fd8c5b7c858b48b,	0x3fe5babcc647fa91	// log1p(0.47266), 1/1.47266
                    .quad	0x3fd8f11e873662c7,	0x3fe5ac056b015ac0	// log1p(0.47656), 1/1.47656
                    .quad	0x3fd91c67eb45a83e,	0x3fe59d61f123ccaa	// log1p(0.48047), 1/1.48047
                    .quad	0x3fd947941c2116fb,	0x3fe58ed2308158ed	// log1p(0.48438), 1/1.48438
                    .quad	0x3fd972a341135158,	0x3fe5805601580560	// log1p(0.48828), 1/1.48828
                    .quad	0x3fd99d958117e08b,	0x3fe571ed3c506b3a	// log1p(0.49219), 1/1.49219
                    .quad	0x3fd9c86b02dc0863,	0x3fe56397ba7c52e2	// log1p(0.49609), 1/1.49609
                    .quad	0x3fd9f323ecbf984c,	0x3fe5555555555555	// log1p(0.50000), 1/1.50000
                    .quad	0x3fda1dc064d5b995,	0x3fe54725e6bb82fe	// log1p(0.50391), 1/1.50391
                    .quad	0x3fda484090e5bb0a,	0x3fe5390948f40feb	// log1p(0.50781), 1/1.50781
                    .quad	0x3fda72a4966bd9ea,	0x3fe52aff56a8054b	// log1p(0.51172), 1/1.51172
                    .quad	0x3fda9cec9a9a084a,	0x3fe51d07eae2f815	// log1p(0.51562), 1/1.51562
                    .quad	0x3fdac718c258b0e4,	0x3fe50f22e111c4c5	// log1p(0.51953), 1/1.51953
                    .quad	0x3fdaf1293247786b,	0x3fe5015015015015	// log1p(0.52344), 1/1.52344
                    .quad	0x3fdb1b1e0ebdfc5b,	0x3fe4f38f62dd4c9b	// log1p(0.52734), 1/1.52734
                    .quad	0x3fdb44f77bcc8f63,	0x3fe4e5e0a72f0539	// log1p(0.53125), 1/1.53125
                    .quad	0x3fdb6eb59d3cf35e,	0x3fe4d843bedc2c4c	// log1p(0.53516), 1/1.53516
                    .quad	0x3fdb9858969310fb,	0x3fe4cab88725af6e	// log1p(0.53906), 1/1.53906
                    .quad	0x3fdbc1e08b0dad0a,	0x3fe4bd3edda68fe1	// log1p(0.54297), 1/1.54297
                    .quad	0x3fdbeb4d9da71b7c,	0x3fe4afd6a052bf5b	// log1p(0.54688), 1/1.54688
                    .quad	0x3fdc149ff115f027,	0x3fe4a27fad76014a	// log1p(0.55078), 1/1.55078
                    .quad	0x3fdc3dd7a7cdad4d,	0x3fe49539e3b2d067	// log1p(0.55469), 1/1.55469
                    .quad	0x3fdc66f4e3ff6ff8,	0x3fe4880522014880	// log1p(0.55859), 1/1.55859
                    .quad	0x3fdc8ff7c79a9a22,	0x3fe47ae147ae147b	// log1p(0.56250), 1/1.56250
                    .quad	0x3fdcb8e0744d7aca,	0x3fe46dce34596066	// log1p(0.56641), 1/1.56641
                    .quad	0x3fdce1af0b85f3eb,	0x3fe460cbc7f5cf9a	// log1p(0.57031), 1/1.57031
                    .quad	0x3fdd0a63ae721e64,	0x3fe453d9e2c776ca	// log1p(0.57422), 1/1.57422
                    .quad	0x3fdd32fe7e00ebd5,	0x3fe446f86562d9fb	// log1p(0.57812), 1/1.57812
                    .quad	0x3fdd5b7f9ae2c684,	0x3fe43a2730abee4d	// log1p(0.58203), 1/1.58203
                    .quad	0x3fdd83e7258a2f3e,	0x3fe42d6625d51f87	// log1p(0.58594), 1/1.58594
                    .quad	0x3fddac353e2c5954,	0x3fe420b5265e5951	// log1p(0.58984), 1/1.58984
                    .quad	0x3fddd46a04c1c4a1,	0x3fe4141414141414	// log1p(0.59375), 1/1.59375
                    .quad	0x3fddfc859906d5b5,	0x3fe40782d10e6566	// log1p(0.59766), 1/1.59766
                    .quad	0x3fde24881a7c6c26,	0x3fe3fb013fb013fb	// log1p(0.60156), 1/1.60156
                    .quad	0x3fde4c71a8687704,	0x3fe3ee8f42a5af07	// log1p(0.60547), 1/1.60547
                    .quad	0x3fde744261d68788,	0x3fe3e22cbce4a902	// log1p(0.60938), 1/1.60938
                    .quad	0x3fde9bfa659861f5,	0x3fe3d5d991aa75c6	// log1p(0.61328), 1/1.61328
                    .quad	0x3fdec399d2468cc0,	0x3fe3c995a47babe7	// log1p(0.61719), 1/1.61719
                    .quad	0x3fdeeb20c640ddf4,	0x3fe3bd60d9232955	// log1p(0.62109), 1/1.62109
                    .quad	0x3fdf128f5faf06ed,	0x3fe3b13b13b13b14	// log1p(0.62500), 1/1.62500
                    .quad	0x3fdf39e5bc811e5c,	0x3fe3a524387ac822	// log1p(0.62891), 1/1.62891
                    .quad	0x3fdf6123fa7028ac,	0x3fe3991c2c187f63	// log1p(0.63281), 1/1.63281
                    .quad	0x3fdf884a36fe9ec2,	0x3fe38d22d366088e	// log1p(0.63672), 1/1.63672
                    .quad	0x3fdfaf588f78f31f,	0x3fe3813813813814	// log1p(0.64062), 1/1.64062
                    .quad	0x3fdfd64f20f61572,	0x3fe3755bd1c945ee	// log1p(0.64453), 1/1.64453
                    .quad	0x3fdffd2e0857f498,	0x3fe3698df3de0748	// log1p(0.64844), 1/1.64844
                    .quad	0x3fe011fab125ff8a,	0x3fe35dce5f9f2af8	// log1p(0.65234), 1/1.65234
                    .quad	0x3fe02552a5a5d0ff,	0x3fe3521cfb2b78c1	// log1p(0.65625), 1/1.65625
                    .quad	0x3fe0389eefce633b,	0x3fe34679ace01346	// log1p(0.66016), 1/1.66016
                    .quad	0x3fe04bdf9da926d2,	0x3fe33ae45b57bcb2	// log1p(0.66406), 1/1.66406
                    .quad	0x3fe05f14bd26459c,	0x3fe32f5ced6a1dfa	// log1p(0.66797), 1/1.66797
                    .quad	0x3fe0723e5c1cdf40,	0x3fe323e34a2b10bf	// log1p(0.67188), 1/1.67188
                    .quad	0x3fe0855c884b450e,	0x3fe3187758e9ebb6	// log1p(0.67578), 1/1.67578
                    .quad	0x3fe0986f4f573521,	0x3fe30d190130d190	// log1p(0.67969), 1/1.67969
                    .quad	0x3fe0ab76bece14d2,	0x3fe301c82ac40260	// log1p(0.68359), 1/1.68359
                    .quad	0x3fe0be72e4252a83,	0x3fe2f684bda12f68	// log1p(0.68750), 1/1.68750
                    .quad	0x3fe0d163ccb9d6b8,	0x3fe2eb4ea1fed14b	// log1p(0.69141), 1/1.69141
                    .quad	0x3fe0e44985d1cc8c,	0x3fe2e025c04b8097	// log1p(0.69531), 1/1.69531
                    .quad	0x3fe0f7241c9b497d,	0x3fe2d50a012d50a0	// log1p(0.69922), 1/1.69922
                    .quad	0x3fe109f39e2d4c97,	0x3fe2c9fb4d812ca0	// log1p(0.70312), 1/1.70312
                    .quad	0x3fe11cb81787ccf8,	0x3fe2bef98e5a3711	// log1p(0.70703), 1/1.70703
                    .quad	0x3fe12f719593efbc,	0x3fe2b404ad012b40	// log1p(0.71094), 1/1.71094
                    .quad	0x3fe1422025243d45,	0x3fe2a91c92f3c105	// log1p(0.71484), 1/1.71484
                    .quad	0x3fe154c3d2f4d5ea,	0x3fe29e4129e4129e	// log1p(0.71875), 1/1.71875
                    .quad	0x3fe1675cababa60e,	0x3fe293725bb804a5	// log1p(0.72266), 1/1.72266
                    .quad	0x3fe179eabbd899a1,	0x3fe288b01288b013	// log1p(0.72656), 1/1.72656
                    .quad	0x3fe18c6e0ff5cf06,	0x3fe27dfa38a1ce4d	// log1p(0.73047), 1/1.73047
                    .quad	0x3fe19ee6b467c96f,	0x3fe27350b8812735	// log1p(0.73438), 1/1.73438
                    .quad	0x3fe1b154b57da29f,	0x3fe268b37cd60127	// log1p(0.73828), 1/1.73828
                    .quad	0x3fe1c3b81f713c25,	0x3fe25e22708092f1	// log1p(0.74219), 1/1.74219
                    .quad	0x3fe1d610fe677003,	0x3fe2539d7e9177b2	// log1p(0.74609), 1/1.74609
                    .quad	0x3fe1e85f5e7040d0,	0x3fe2492492492492	// log1p(0.75000), 1/1.75000
                    .quad	0x3fe1faa34b87094c,	0x3fe23eb79717605b	// log1p(0.75391), 1/1.75391
                    .quad	0x3fe20cdcd192ab6e,	0x3fe23456789abcdf	// log1p(0.75781), 1/1.75781
                    .quad	0x3fe21f0bfc65beec,	0x3fe22a0122a0122a	// log1p(0.76172), 1/1.76172
                    .quad	0x3fe23130d7bebf43,	0x3fe21fb78121fb78	// log1p(0.76562), 1/1.76562
                    .quad	0x3fe2434b6f483934,	0x3fe21579804855e6	// log1p(0.76953), 1/1.76953
                    .quad	0x3fe2555bce98f7cb,	0x3fe20b470c67c0d9	// log1p(0.77344), 1/1.77344
                    .quad	0x3fe26762013430e0,	0x3fe2012012012012	// log1p(0.77734), 1/1.77734
                    .quad	0x3fe2795e1289b11b,	0x3fe1f7047dc11f70	// log1p(0.78125), 1/1.78125
                    .quad	0x3fe28b500df60783,	0x3fe1ecf43c7fb84c	// log1p(0.78516), 1/1.78516
                    .quad	0x3fe29d37fec2b08b,	0x3fe1e2ef3b3fb874	// log1p(0.78906), 1/1.78906
                    .quad	0x3fe2af15f02640ad,	0x3fe1d8f5672e4abd	// log1p(0.79297), 1/1.79297
                    .quad	0x3fe2c0e9ed448e8c,	0x3fe1cf06ada2811d	// log1p(0.79688), 1/1.79688
                    .quad	0x3fe2d2b4012edc9e,	0x3fe1c522fc1ce059	// log1p(0.80078), 1/1.80078
                    .quad	0x3fe2e47436e40268,	0x3fe1bb4a4046ed29	// log1p(0.80469), 1/1.80469
                    .quad	0x3fe2f62a99509546,	0x3fe1b17c67f2bae3	// log1p(0.80859), 1/1.80859
                    .quad	0x3fe307d7334f10be,	0x3fe1a7b9611a7b96	// log1p(0.81250), 1/1.81250
                    .quad	0x3fe3197a0fa7fe6a,	0x3fe19e0119e0119e	// log1p(0.81641), 1/1.81641
                    .quad	0x3fe32b1339121d71,	0x3fe19453808ca29c	// log1p(0.82031), 1/1.82031
                    .quad	0x3fe33ca2ba328995,	0x3fe18ab083902bdb	// log1p(0.82422), 1/1.82422
                    .quad	0x3fe34e289d9ce1d3,	0x3fe1811811811812	// log1p(0.82812), 1/1.82812
                    .quad	0x3fe35fa4edd36ea0,	0x3fe1778a191bd684	// log1p(0.83203), 1/1.83203
                    .quad	0x3fe37117b54747b6,	0x3fe16e0689427379	// log1p(0.83594), 1/1.83594
                    .quad	0x3fe38280fe58797f,	0x3fe1648d50fc3201	// log1p(0.83984), 1/1.83984
                    .quad	0x3fe393e0d3562a1a,	0x3fe15b1e5f75270d	// log1p(0.84375), 1/1.84375
                    .quad	0x3fe3a5373e7ebdfa,	0x3fe151b9a3fdd5c9	// log1p(0.84766), 1/1.84766
                    .quad	0x3fe3b68449fffc23,	0x3fe1485f0e0acd3b	// log1p(0.85156), 1/1.85156
                    .quad	0x3fe3c7c7fff73206,	0x3fe13f0e8d344724	// log1p(0.85547), 1/1.85547
                    .quad	0x3fe3d9026a7156fb,	0x3fe135c81135c811	// log1p(0.85938), 1/1.85938
                    .quad	0x3fe3ea33936b2f5c,	0x3fe12c8b89edc0ac	// log1p(0.86328), 1/1.86328
                    .quad	0x3fe3fb5b84d16f42,	0x3fe12358e75d3033	// log1p(0.86719), 1/1.86719
                    .quad	0x3fe40c7a4880dce9,	0x3fe11a3019a74826	// log1p(0.87109), 1/1.87109
                    .quad	0x3fe41d8fe84672ae,	0x3fe1111111111111	// log1p(0.87500), 1/1.87500
                    .quad	0x3fe42e9c6ddf80bf,	0x3fe107fbbe011080	// log1p(0.87891), 1/1.87891
                    .quad	0x3fe43f9fe2f9ce67,	0x3fe0fef010fef011	// log1p(0.88281), 1/1.88281
                    .quad	0x3fe4509a5133bb0a,	0x3fe0f5edfab325a2	// log1p(0.88672), 1/1.88672
                    .quad	0x3fe4618bc21c5ec2,	0x3fe0ecf56be69c90	// log1p(0.89062), 1/1.89062
                    .quad	0x3fe472743f33aaad,	0x3fe0e40655826011	// log1p(0.89453), 1/1.89453
                    .quad	0x3fe48353d1ea88df,	0x3fe0db20a88f4696	// log1p(0.89844), 1/1.89844
                    .quad	0x3fe4942a83a2fc07,	0x3fe0d24456359e3a	// log1p(0.90234), 1/1.90234
                    .quad	0x3fe4a4f85db03ebb,	0x3fe0c9714fbcda3b	// log1p(0.90625), 1/1.90625
                    .quad	0x3fe4b5bd6956e274,	0x3fe0c0a7868b4171	// log1p(0.91016), 1/1.91016
                    .quad	0x3fe4c679afccee3a,	0x3fe0b7e6ec259dc8	// log1p(0.91406), 1/1.91406
                    .quad	0x3fe4d72d3a39fd00,	0x3fe0af2f722eecb5	// log1p(0.91797), 1/1.91797
                    .quad	0x3fe4e7d811b75bb1,	0x3fe0a6810a6810a7	// log1p(0.92188), 1/1.92188
                    .quad	0x3fe4f87a3f5026e9,	0x3fe09ddba6af8360	// log1p(0.92578), 1/1.92578
                    .quad	0x3fe50913cc01686b,	0x3fe0953f39010954	// log1p(0.92969), 1/1.92969
                    .quad	0x3fe519a4c0ba3446,	0x3fe08cabb37565e2	// log1p(0.93359), 1/1.93359
                    .quad	0x3fe52a2d265bc5ab,	0x3fe0842108421084	// log1p(0.93750), 1/1.93750
                    .quad	0x3fe53aad05b99b7d,	0x3fe07b9f29b8eae2	// log1p(0.94141), 1/1.94141
                    .quad	0x3fe54b2467999498,	0x3fe073260a47f7c6	// log1p(0.94531), 1/1.94531
                    .quad	0x3fe55b9354b40bcd,	0x3fe06ab59c7912fb	// log1p(0.94922), 1/1.94922
                    .quad	0x3fe56bf9d5b3f399,	0x3fe0624dd2f1a9fc	// log1p(0.95312), 1/1.95312
                    .quad	0x3fe57c57f336f191,	0x3fe059eea0727586	// log1p(0.95703), 1/1.95703
                    .quad	0x3fe58cadb5cd7989,	0x3fe05197f7d73404	// log1p(0.96094), 1/1.96094
                    .quad	0x3fe59cfb25fae87e,	0x3fe04949cc1664c5	// log1p(0.96484), 1/1.96484
                    .quad	0x3fe5ad404c359f2d,	0x3fe0410410410410	// log1p(0.96875), 1/1.96875
                    .quad	0x3fe5bd7d30e71c73,	0x3fe038c6b78247fc	// log1p(0.97266), 1/1.97266
                    .quad	0x3fe5cdb1dc6c1765,	0x3fe03091b51f5e1a	// log1p(0.97656), 1/1.97656
                    .quad	0x3fe5ddde57149923,	0x3fe02864fc7729e9	// log1p(0.98047), 1/1.98047
                    .quad	0x3fe5ee02a9241675,	0x3fe0204081020408	// log1p(0.98438), 1/1.98438
                    .quad	0x3fe5fe1edad18919,	0x3fe0182436517a37	// log1p(0.98828), 1/1.98828
                    .quad	0x3fe60e32f44788d9,	0x3fe0101010101010	// log1p(0.99219), 1/1.99219
                    .quad	0x3fe61e3efda46467,	0x3fe0080402010080	// log1p(0.99609), 1/1.99609


.literal8
.align 3
one:            .double     1.0
onehalf:        .double     0.5
onethird:       .quad       0x3fd5555555555555      // 1/3
onequarter:     .double     0.25
onefifth:       .double     0.2
log1p_amask:    .quad       0x7ffff00000000000ULL   // top 8 bits of mantissa
exp_maskd:      .quad       0x7ff0000000000000ULL   // exponent bits / +Inf
almost1:        .quad       0x3fefffffffffffffULL   // 1.0 - DBL_EPSILON/2
log2:           .quad       0x3fe62e42fefa39efULL   // ln(2)

.literal4
.align 2
f256:           .long       0x43800000              //256.0f
r256:           .long       0x3b800000              //1.0f/256.0f


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
logf_pic:       movl    (%esp),     %ecx                    //copy address of local_addr to %ecx
                ret
#else
	#error arch not supported
#endif


//
//  logf -- overall approach
//
//      We break up logf(x) as follows:
//
//          x = 2**i * m                1.0 <= m < 2.0
//          log(x) = log(2**i) + log(m)
//
//      log(2**i) is simply read from log_e_table.
//      To obtain log(m), we further break down m as :
//
//          m = (1+a/256.0)(1+r)              a = high 8 explicit bits of mantissa(m) 
//          log(m) = log(1+a/256.0) + log2(1+r)
//          
//      We use the high 8 bits of the mantissa to look up log(1+a/256.0) in log_m_table above
//      We calculate 1+r as:
//
//          1+r = m * (1 /(1+a/256.0))
//
//      We can lookup (from the same table) the value of 1/(1+a/256.0) based on a too.
//
//      So the whole calculation is:
//
//          logf(x) = log(2**i) + log(1+a/256.0) + log(1+r) = log_e_table[i+149] + log_m_table[a][0] + log( m * log_m_table[a][1] )
//
//      The third term is calculated using the Taylor series:
//
//          log(x+1) = x - x**2/2 + x**3/3 - x**4/4 + x**5/5
//
//      The edge case code is done in integer to avoid setting flags, and because it is faster that way.
//      In the particular case of 1.0:  
//          i = 0 -> log_e_table[0+149] = 0.0
//          a = 0 -> log_m_table[a][0]  = 0.0
//          a = 0 -> log_m_table[a][1]  = 1.0
//          1+r = 1.0 * 1.0  
//          r = 0
//          log(1+r) = 0 - 0/2 + 0/3 - 0/4 + 0/5
//          
//      so we get logf(x) = 0.0, with no inexact flag set. Other values should set inexact when we round to single precision, 
//      either because the mantissa is not 1.0 or log_e_table[i+149] is non-zero.
//


ENTRY( logf )
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
    
    mov         $(-127),                    DX_P            // negative single precision bias
    cmpl        $0x00800000,                %ecx            // if( isnormal( x ))
    jae         1f                                          //      skip denormal handling
    
    //denormal whatnot
    SUBP        $126,                       DX_P            // accumulate in -126 as pseudo exponent of denormal
    movl        $0x3f800000,                %eax            // 1.0
    orl         %eax,                       %ecx            // multiply low bits by 2**126 by oring in 1.0
    movd        %eax,                       %xmm1           // 1.0
    movd        %ecx,                       %xmm0           // 1.0 | denorm bits
    subss       %xmm1,                      %xmm0           // (1.0 | denorm bits) - 1.0
    movd        %xmm0,                      %ecx
  
    //find log_e_table[i] and load into %xmm0
1:  shrl        $23,                        %ecx
    addl        %ecx,                       %edx
    
#if defined( __i386__ )
    calll       logf_pic                                    // set %ecx to point to local_addr
rel_addr:   
#endif    
    cvtsi2sd    %edx,                       %xmm7
    mulsd       RELATIVE_ADDR(log2),        %xmm7           // log_e_table[i]

    //find a, and load values from log_m_table
    movd        %xmm0,                      %eax            // x
    movl        %eax,                       %edx            // x
    andl        $0x00007fff,                %eax            // low bits of x
    andl        $0x007f8000,                %edx            // a << 15
    orl         $0x3f800000,                %eax            // 1.0 | low bits of x
    shrl        $11,                        %edx            // a << 4
    movd        %eax,                       %xmm1           // 1.0 | low bits of x
    cvtss2sd    %xmm1,                      %xmm1           // 1.0 | low bits of x
    subsd       RELATIVE_ADDR( one ),       %xmm1           // (1.0 | low bits of x) - 1.0
    lea         RELATIVE_ADDR( log_m_table), AX_P
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
    
    //add up major terms
    addsd       %xmm6,                      %xmm1           // log(1+a/256.0) + (r - 0.5rr + (1./3.)rrr - 0.25rrrr + 0.2rrrrr)
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




// ------------------------------------- log1pf --------------------------------------

#if defined( __i386__ )
    #define INDEX       %edi
#else
    #define INDEX       %r8
#endif

ENTRY( log1pf )
// Load input value
#if defined( __i386__ )
    movl        FRAME_SIZE( STACKP ),       %eax
    movss       FRAME_SIZE( STACKP ),       %xmm0
#else
    movd        %xmm0,                      %eax
#endif

    //move special cases ( x <= -1.0, +-Inf, NaN) to special case code
    cmpl    $0xbf800000,                    %eax        // if( x <= -1.0f || x is a negative NaN )
    jae     2f                                          //      goto 2
            
    //Deal with +Inf, +NaN
    movl    %eax,                           %edx
    andl    $0x7f800000,                    %eax
    cmpl    $0x7f800000,                    %eax        // if( x == +inf || x is positive NaN )
    je      4f                                          //      goto 3

#if defined( __i386__ )
    pushl   %edi
    calll   logf_pic                                    // set %ecx to point to local_addr
rel_addr_b:   
#endif
    
    //Normal number in range
    //Check for |x| < 0.5                   cutoff between algorithms determined empirically
    cmpl    $0x3f000000,                    %eax        // if( |x| < 1.0f )
    jb      1f                                          //      goto 1
    
    // x >= 1.0f.  We just add 1.0 (in double precision) and do things much like log above
    cvtss2sd %xmm0,                         %xmm1       // (double) x
#if defined( __x86_64__ )
    xorq    %rax,                           %rax        // zero top 32-bits of register a (and bottom 32-bits too)
#endif    
    movsd   RELATIVE_ADDR_B( one ),         %xmm3       // 1.0
    addsd   %xmm3,                          %xmm1       // x + 1.0      //Note: because double precision, cant overflow here for round to +Inf
    movsd   RELATIVE_ADDR_B( exp_maskd),    %xmm2       // 0x7ff0000000000000ULL
    andnpd  %xmm1,                          %xmm2       // (x + 1.0) mantissa, exponent all zero   Note: sign bit always 0 here
    psrlq   $(52-8-4),                      %xmm1       // (x + 1.0) >> (52-8-4)   52 bit significand - 8 bits for a - 4 bits for size of log_m_table entries
    movsd   RELATIVE_ADDR_B(onethird),      %xmm6       // 1/3
    orpd    %xmm3,                          %xmm2       // mantissa    1.0 <= m < 2.0
    movd    %xmm1,                          %eax        // (x + 1.0) >> (52-8)
    movsd   RELATIVE_ADDR_B(log1p_amask ),  %xmm1       // 0x7ffff00000000000ULL
    MOVP    AX_P,                           INDEX       // set aside a bits
    lea     RELATIVE_ADDR_B( log_m_table ), DX_P        // get pointer to log_m_table
    andpd   %xmm2,                          %xmm1       // 1 + a/256.0
    shrl    $(8+4),                         %eax        // biased exponent
    subsd   %xmm1,                          %xmm2
    and     $0xff0,                         INDEX       // a << 4       index into log_m_table    clears high 32 bits too for x86_64
    mulsd   8(DX_P, INDEX, 1),              %xmm2       // r = (reduced mantissa) / (1+a/256.0)
    movsd   (DX_P, INDEX, 1),               %xmm7       // log1p(a/256.0)
    movapd  %xmm2,                          %xmm1       // r
    mulsd   %xmm1,                          %xmm6       // r/3
    mulsd   %xmm2,                          %xmm2       // rr
    subl    $(1023),                        %eax        // unbiased exponent, always positive so 32-bit subtract ok. Top 32-bits zeroed above. 149 added because table starts at 2**-149.
    subsd   RELATIVE_ADDR_B(onehalf),       %xmm6       // -0.5 + r/3
    mulsd   %xmm2,                          %xmm6       // -0.5rr + rrr/3
    cvtsi2sd %eax,                          %xmm5       // exponent
    addsd   %xmm1,                          %xmm6       // r + -0.5rr + rrr/3
    mulsd   RELATIVE_ADDR_B( log2 ),        %xmm5       // ln(2) * exponent
    addsd   %xmm7,                          %xmm6       // r + -0.5rr + rrr/3 + log1p(a/256.0)
    addsd   %xmm5,                          %xmm6       // (r + -0.5rr + rrr/3 + log1p(a/256.0)) + log( 2**exponent )
    cvtsd2ss %xmm6,                         %xmm0       // round to single precision
    
#if defined( __i386__ )
    popl    %edi
    movss   %xmm0,                          FRAME_SIZE( STACKP )
    flds    FRAME_SIZE( STACKP )
#endif
    ret
    
    // |x| < 0.5
    //
1:  // check for |x| < 0x1.0p-24
    cmpl    $0x33800000,                    %eax        // if( exponent is < -24 )
    jb      6f                                          //      goto 6f

    //  Here we approximate as follows
    //
    // log(1+x) = log( (1+a) * (1+r ) )
    //  r = (1+x) / (1+a) - 1 = (x-a)/(1+a)  
    //
    //  This works well, as long as x > -0.5. For values below -0.5, the 1/(1+a) term blows up
    //  and the reduction fails to produce a r sufficiently near zero such that it will quickly 
    //  converge with only a few terms.
    //

    // find the closest a/256.0f which is not greater in magnitide than f. 
    // and reduce f as r = (f - a/256.0f) / (1+a/256.0f)
    movaps  %xmm0,                          %xmm1       // f
    mulss   RELATIVE_ADDR_B(f256),          %xmm0       // f * 256.0f
    cvttss2si %xmm0,                        AX_P        // (int)(f * 256.0f )
    cvtsi2ss  %eax,                         %xmm0       // trunc( f * 256.0f )

#if defined( __x86_64__ )
    cdqe                                                // sign extend eax -> rax
#endif    

    shl     $4,                             AX_P        // stride of log_m_table is 16 bytes
    mulss   RELATIVE_ADDR_B(r256),          %xmm0       // fa = trunc( f * 256.0f ) / 256.0f
    lea     RELATIVE_ADDR_B( log_m_table),  DX_P
    subss   %xmm0,                          %xmm1       // f - fa

    // z = (1.0 + (-0.5 + (1./3. + -0.25*r)*r)*r)*r
    movsd   RELATIVE_ADDR_B( onequarter),   %xmm4       // 0.25
    cvtss2sd %xmm1,                         %xmm2       // (double) (f-fa)
    movsd   RELATIVE_ADDR_B( onethird ),    %xmm3       // 1./3.
    mulsd   8(DX_P, AX_P, 1),               %xmm2       // r = log_m_table[a][1] * (f-fa)
    movapd  %xmm2,                          %xmm1       // r
    mulsd   %xmm2,                          %xmm2       // rr
    mulsd   %xmm2,                          %xmm4       // 0.25rr
    mulsd   %xmm2,                          %xmm3       // rr/3.  (almost)
    addsd   RELATIVE_ADDR_B( onehalf ),     %xmm4       // 0.5 + 0.25rr
    addsd   RELATIVE_ADDR_B( one ),         %xmm3       // 1.0 + rr/3
    mulsd   %xmm2,                          %xmm4       // 0.5rr + 0.25rrrr
    mulsd   %xmm1,                          %xmm3       // r + rrr/3
    subsd   %xmm4,                          %xmm3       // r - 0.5rr + rrr/3 - 0.25rrrr

    // Add in log1p( a / 256.0f)
    addsd   (DX_P, AX_P, 1),                %xmm3       // log_m_table[a][0] + r - 0.5rr + rrr/3 - 0.25rrrr

    // round to single precision  -- this sets inexact if r happened to be 0, because log1p(a/256.0) is not exact in single precision
    cvtsd2ss %xmm3,                         %xmm0       
        
#if defined( __i386__ )
    popl    %edi
    movss   %xmm0,                          FRAME_SIZE( STACKP )
    flds    FRAME_SIZE( STACKP )
#endif
    ret
    
    
2:  // x <= -1.0f || x is a negative NaN
    je      3f                                          // x == -1.0f
    ucomiss %xmm0,                          %xmm0       // if( isnan(x) )
    jp      5f                                          //      goto 4

    // x < -1.0f, return NaN and set invalid
    pcmpeqb %xmm0,                          %xmm0               // -1U
    pslld   $23,                            %xmm0               // 0xff800000, -inf
    xorps   %xmm1,                          %xmm1
    mulss   %xmm1,                          %xmm0               // set invalid, create NaN
#if defined( __i386__ )
    movss   %xmm0,                          FRAME_SIZE(STACKP)
    flds    FRAME_SIZE( STACKP )
#endif
    ret
    

3:  // x == -1.0
    //We need to return -Inf and set div/0 flag
    xorps   %xmm1,                          %xmm1               // 0.0f
    divss   %xmm1,                          %xmm0               // -1.0f / 0 = -Inf + div/0 flag
#if defined( __i386__ )
    movss   %xmm0,                          FRAME_SIZE(STACKP)
    flds    FRAME_SIZE( STACKP )
#endif
    ret

4:  //  x == +inf || x is positive NaN
    ucomiss %xmm0,                          %xmm0       // if( isnan(x) )
    jp      5f                                          //      goto 4

    // x == +inf, return +inf
#if defined( __i386__ )
    movss   %xmm0,                          FRAME_SIZE(STACKP)
    flds    FRAME_SIZE( STACKP )
#endif
    ret
    

5:  //  x is NaN
    addss   %xmm0,                          %xmm0       //silence NaN
#if defined( __i386__ )
    movss   %xmm0,                          FRAME_SIZE(STACKP)
    flds    FRAME_SIZE( STACKP )
#endif
    ret

6:  // |x| < 0x1.0p-24f
#if defined( __i386__ )
    popl    %edi
#endif
    andl    $0x7fffffff,                    %edx        // |x|
    jz      7f                                          // if( |x| == 0.0f ) goto 7

    // non-zero
    cvtss2sd    %xmm0,                      %xmm1
    mulsd   RELATIVE_ADDR_B( almost1),      %xmm1       // multiply by 1ulp less than 1.0. Sets inexact.
    cvtsd2ss    %xmm1,                      %xmm0       // convert back to float, underflow as necessary, round appropriately
#if defined( __i386__ )
    movss   %xmm0,                          FRAME_SIZE( STACKP )
#endif    
7:
#if defined( __i386__ )
    flds    FRAME_SIZE( STACKP )                        // { x }
#endif
    ret
