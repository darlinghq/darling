/*
 * cbrtf.s
 *
 *      by Stephen Canon
 *
 * Copyright (c) 2007, Apple Inc.  All Rights Reserved.
 *
 * This file implements the cbrtf function for the MacOS X __i386__ and __x86_64__ ABIs.
 */


/*
 
 Overview of algorithm used for cbrt(x):
 
 1. Compute a reduced argument u in the range [1,8) such that 2^(3n) u = |x| for appropriate n.
 2. Lookup an exact cube z such that |z - u| < 2^-6
 
 We then have: cbrt(x) = sign(x) * 2^n * cbrt(z) * cbrt(1 + (x - z) * 1/z)
 
 3. the mantissa of cbrt(z) is looked up in a table, and or-ed into the exponent 2^n
 4. cbrt(1 + (x - z) * 1/z) is computed via a minimax polynomial of the form:
 
	1 + cw(w + a)(w^2 + b1lo w + b0lo)(w^2 + b1hi w + b0hi)
	
	where w = (x-z) * 1/z
	
 5. Finally, we multiply the two parts, or in the signbit and return.
 
 This algorithm produces correctly rounded results for single precision, and avoids setting the inexact flag
 for exact cube arguments.
 
 
 
 
 
 The table also contains the value 1/cbrt(z).  This gets us power-of-two alignment for the table entries, and also
 can be used to implement a correctly rounded reciprocal cube root, which could be useful for computing small rational
 powers if we ever need to provide an ipow function.
 
 The algorithm for recip_cbrtf(x) would proceed exactly as above, except we instead compute:
 
	recip_cbrt(x) = sign(x) * 2^-n * recip_cbrt(z) * p(1 + (x - z) * 1/z)
	
 where p(w) is a polynomial approximation to 1 / cbrt(1 + w) on |w| < 2^-6.  (6th order minimax will suffice)
 
 - scanon, July 2007
 
 */


#include <machine/asm.h>
#include "abi.h"

.const
.align 4

// Minimax polynomial coefficents (addressed by offset from cbrt_table)
			.quad 0x4000204182c17486, 0xbff8b5b876f0d973 // b1lo b1hi
            .quad 0x4005826bbd26eb39, 0x40080ac10b5eee54 // b0lo b0hi
            .quad 0xbf980b1460621e80, 0xbffc1c8e77867969 // c a
			
//                   base point z      mantissa(cbrt(z))       1/cbrt(z)             1/z
//                ------------------------------------------------------------------------------
cbrt_table: .quad 0x3ff0000000000000, 0x0000000000000000, 0x3ff0000000000000, 0x3ff0000000000000 // z ~ 0x1.00p0
            .quad 0x3ff060c080000000, 0x0000200000000000, 0x3fefc07f01fc07f0, 0x3fef42f61dacddc6 // z ~ 0x1.04p0
            .quad 0x3ff09fe97c0b2e80, 0x000034a000000000, 0x3fef9815c85b04a3, 0x3feecc3168ac46e4 // z ~ 0x1.08p0
            .quad 0x3ff0c30400000000, 0x0000400000000000, 0x3fef81f81f81f820, 0x3fee8bb1d5b6e585 // z ~ 0x1.0cp0
            .quad 0x3ff126cd80000000, 0x0000600000000000, 0x3fef44659e4a4271, 0x3fedd9fb30af3365 // z ~ 0x1.10p0
            .quad 0x3ff15f9b5b480000, 0x0000720000000000, 0x3fef222c82dba316, 0x3fed786108fd7a9f // z ~ 0x1.14p0
            .quad 0x3ff18c2000000000, 0x0000800000000000, 0x3fef07c1f07c1f08, 0x3fed2d9cbd756afd // z ~ 0x1.18p0
            .quad 0x3ff1f2fe80000000, 0x0000a00000000000, 0x3feecc07b301ecc0, 0x3fec86636f753a66 // z ~ 0x1.1cp0
            .quad 0x3ff21fac7ca59c00, 0x0000adc000000000, 0x3feeb2a412496abd, 0x3fec40112c606d3e // z ~ 0x1.20p0
            .quad 0x3ff25b6c00000000, 0x0000c00000000000, 0x3fee9131abf0b767, 0x3febe41e7ee3f7ed // z ~ 0x1.24p0
            .quad 0x3ff29ff9aaaa2c00, 0x0000d4c000000000, 0x3fee6b8275501adb, 0x3feb7d7596e80007 // z ~ 0x1.28p0
            .quad 0x3ff2c56b80000000, 0x0000e00000000000, 0x3fee573ac901e574, 0x3feb469f4adc7794 // z ~ 0x1.2cp0
            .quad 0x3ff3310000000000, 0x0001000000000000, 0x3fee1e1e1e1e1e1e, 0x3feaadb93d39ae9c // z ~ 0x1.30p0
            .quad 0x3ff35fb6f4579c00, 0x00010dc000000000, 0x3fee05d5a24448c5, 0x3fea6d6548fa984d // z ~ 0x1.34p0
            .quad 0x3ff39e2c80000000, 0x0001200000000000, 0x3fede5d6e3f8868a, 0x3fea1941b013022d // z ~ 0x1.38p0
            .quad 0x3ff3dfc1312b0000, 0x0001330000000000, 0x3fedc4cfaf10eb5c, 0x3fe9c322b87f17e8 // z ~ 0x1.3cp0
            .quad 0x3ff40cf400000000, 0x0001400000000000, 0x3fedae6076b981db, 0x3fe9890fd4bf368f // z ~ 0x1.40p0
            .quad 0x3ff47d5980000000, 0x0001600000000000, 0x3fed77b654b82c34, 0x3fe8fcfc9c44e2f4 // z ~ 0x1.44p0
            .quad 0x3ff49feb2bc0dc00, 0x000169c000000000, 0x3fed67366d6ddfd0, 0x3fe8d31a9f2d47fb // z ~ 0x1.48p0
            .quad 0x3ff4ef6000000000, 0x0001800000000000, 0x3fed41d41d41d41d, 0x3fe874e2a121159f // z ~ 0x1.4cp0
            .quad 0x3ff51ff889bc6000, 0x00018d8000000000, 0x3fed2b539aeee152, 0x3fe83ca00a5a8f32 // z ~ 0x1.50p0
            .quad 0x3ff5630a80000000, 0x0001a00000000000, 0x3fed0cb58f6ec074, 0x3fe7f09e124e78b8 // z ~ 0x1.54p0
            .quad 0x3ff59fc8db9a7e80, 0x0001b0a000000000, 0x3fecf1688b3b4e6a, 0x3fe7ad5e68ed5f8c // z ~ 0x1.58p0
            .quad 0x3ff5d85c00000000, 0x0001c00000000000, 0x3fecd85689039b0b, 0x3fe7700c9f78cc63 // z ~ 0x1.5cp0
            .quad 0x3ff61fbc0c515400, 0x0001d34000000000, 0x3fecb92ff3a86d65, 0x3fe7246f92d40d4c // z ~ 0x1.60p0
            .quad 0x3ff64f5780000000, 0x0001e00000000000, 0x3feca4b3055ee191, 0x3fe6f30d6649f11b // z ~ 0x1.64p0
            .quad 0x3ff69fc04b688980, 0x0001f56000000000, 0x3fec829b51036037, 0x3fe6a17c8a1a662e // z ~ 0x1.68p0
            .quad 0x3ff6c80000000000, 0x0002000000000000, 0x3fec71c71c71c71c, 0x3fe67980e0bf08c7 // z ~ 0x1.6cp0
            .quad 0x3ff71fc3c5870000, 0x0002170000000000, 0x3fec4d9cd40d7cfd, 0x3fe6243421ae7a84 // z ~ 0x1.70p0
            .quad 0x3ff7425880000000, 0x0002200000000000, 0x3fec3f8f01c3f8f0, 0x3fe60348d4756756 // z ~ 0x1.74p0
            .quad 0x3ff7be6400000000, 0x0002400000000000, 0x3fec0e070381c0e0, 0x3fe5904842e0271b // z ~ 0x1.78p0
            .quad 0x3ff7dfa08e162000, 0x0002488000000000, 0x3fec00fc08dc4fbf, 0x3fe57242f8b50298 // z ~ 0x1.7cp0
            .quad 0x3ff83c2580000000, 0x0002600000000000, 0x3febdd2b899406f7, 0x3fe520635a583b96 // z ~ 0x1.80p0
            .quad 0x3ff85fd33ff90000, 0x0002690000000000, 0x3febcf8c69606a07, 0x3fe50176a58004f0 // z ~ 0x1.84p0
            .quad 0x3ff8bba000000000, 0x0002800000000000, 0x3febacf914c1bad0, 0x3fe4b37f67f9d05c // z ~ 0x1.88p0
            .quad 0x3ff8dfca52590000, 0x0002890000000000, 0x3feb9f88e001b9f9, 0x3fe495664ea7f47d // z ~ 0x1.8cp0
            .quad 0x3ff93cd680000000, 0x0002a00000000000, 0x3feb7d6c3dda338b, 0x3fe44982ca42a2eb // z ~ 0x1.90p0
            .quad 0x3ff95ff68a951e80, 0x0002a8a000000000, 0x3feb70b72f76e7dd, 0x3fe42d6dab45c848 // z ~ 0x1.94p0
            .quad 0x3ff9bfcc00000000, 0x0002c00000000000, 0x3feb4e81b4e81b4f, 0x3fe3e254e465d72c // z ~ 0x1.98p0
            .quad 0x3ff9dfc708557c00, 0x0002c7c000000000, 0x3feb433cf4756912, 0x3fe3c9c1357411b6 // z ~ 0x1.9cp0
            .quad 0x3ffa1f8756df7480, 0x0002d72000000000, 0x3feb2cfd6b4a2ec0, 0x3fe39976b1b376fb // z ~ 0x1.a0p0
            .quad 0x3ffa448380000000, 0x0002e00000000000, 0x3feb2036406c80d9, 0x3fe37dde124a87f2 // z ~ 0x1.a4p0
            .quad 0x3ffa9fbaa05b1c00, 0x0002f5c000000000, 0x3feb01182b5ac1ce, 0x3fe33b1676d97a5b // z ~ 0x1.a8p0
            .quad 0x3ffacb0000000000, 0x0003000000000000, 0x3feaf286bca1af28, 0x3fe31c079d2b089f // z ~ 0x1.acp0
            .quad 0x3ffb1ff52f400000, 0x0003140000000000, 0x3fead646ddd321c2, 0x3fe2e02d4701d501 // z ~ 0x1.b0p0
            .quad 0x3ffb534480000000, 0x0003200000000000, 0x3feac5701ac5701b, 0x3fe2bcbbb0cb73f6 // z ~ 0x1.b4p0
            .quad 0x3ffb9fa0378e5c00, 0x000331c000000000, 0x3feaacae5fd5e77d, 0x3fe288f0567537ff // z ~ 0x1.b8p0
            .quad 0x3ffbdd5400000000, 0x0003400000000000, 0x3fea98ef606a63be, 0x3fe25fe5513ebf45 // z ~ 0x1.bcp0
            .quad 0x3ffc1fc1c0569400, 0x00034f4000000000, 0x3fea83eded1251e7, 0x3fe2347ec39d66b0 // z ~ 0x1.c0p0
            .quad 0x3ffc693180000000, 0x0003600000000000, 0x3fea6d01a6d01a6d, 0x3fe2057051321929 // z ~ 0x1.c4p0
            .quad 0x3ffc9fc4ad339d80, 0x00036c6000000000, 0x3fea5c2b87b4e25a, 0x3fe1e3144d16fd97 // z ~ 0x1.c8p0
            .quad 0x3ffcf6e000000000, 0x0003800000000000, 0x3fea41a41a41a41a, 0x3fe1ad4948b6e145 // z ~ 0x1.ccp0
            .quad 0x3ffd1f9c6201cc80, 0x0003892000000000, 0x3fea35607552f1cd, 0x3fe1948fa1f5ff30 // z ~ 0x1.d0p0
            .quad 0x3ffd5f8615bde180, 0x0003976000000000, 0x3fea22504db000b7, 0x3fe16e4ee12da718 // z ~ 0x1.d4p0
            .quad 0x3ffd866280000000, 0x0003a00000000000, 0x3fea16d3f97a4b02, 0x3fe1575d8c8402f4 // z ~ 0x1.d8p0
            .quad 0x3ffddfdfe805bc00, 0x0003b3c000000000, 0x3fe9fcacece0b241, 0x3fe1236b509d4023 // z ~ 0x1.dcp0
            .quad 0x3ffe17bc00000000, 0x0003c00000000000, 0x3fe9ec8e951033d9, 0x3fe1039b25a7f122 // z ~ 0x1.e0p0
            .quad 0x3ffe5ff3ecf6fc00, 0x0003cfc000000000, 0x3fe9d7f292cef9ba, 0x3fe0db275be001a6 // z ~ 0x1.e4p0
            .quad 0x3ffeaaef80000000, 0x0003e00000000000, 0x3fe9c2d14ee4a102, 0x3fe0b1f0c9a4ed7c // z ~ 0x1.e8p0
            .quad 0x3ffedfb5912a5180, 0x0003eb6000000000, 0x3fe9b41b55ca11fc, 0x3fe0956733c0be03 // z ~ 0x1.ecp0
            .quad 0x3fff1fd112ab0c80, 0x0003f92000000000, 0x3fe9a2696dd75ba1, 0x3fe0733ed7907e73 // z ~ 0x1.f0p0
            .quad 0x3fff400000000000, 0x0004000000000000, 0x3fe999999999999a, 0x3fe0624dd2f1a9fc // z ~ 0x1.f4p0
            .quad 0x3fff9fe36d7a7d80, 0x0004146000000000, 0x3fe97f9f956c92fd, 0x3fe030a055aebedd // z ~ 0x1.f8p0
            .quad 0x3fffd6f080000000, 0x0004200000000000, 0x3fe970e4f80cb872, 0x3fe014a239d8b1a9 // z ~ 0x1.fcp0
            .quad 0x400037e200000000, 0x0004400000000000, 0x3fe948b0fcd6e9e0, 0x3fdf91bd1b62b9cf // z ~ 0x1.00p1
            .quad 0x40005ff4c356ff40, 0x000450a000000000, 0x3fe933fff9b30002, 0x3fdf447b132ca3ac // z ~ 0x1.04p1
            .quad 0x4000853ec0000000, 0x0004600000000000, 0x3fe920fb49d0e229, 0x3fdefde7dcdacefd // z ~ 0x1.08p1
            .quad 0x4000d39000000000, 0x0004800000000000, 0x3fe8f9c18f9c18fa, 0x3fde6da80ced1523 // z ~ 0x1.0cp1
            .quad 0x400122d740000000, 0x0004a00000000000, 0x3fe8d3018d3018d3, 0x3fdde0e209af882e // z ~ 0x1.10p1
            .quad 0x4001731600000000, 0x0004c00000000000, 0x3fe8acb90f6bf3aa, 0x3fdd577b2f5c6f87 // z ~ 0x1.14p1
            .quad 0x40019fb2ce620540, 0x0004d1a000000000, 0x3fe897d564f5cf98, 0x3fdd0d34ccd78141 // z ~ 0x1.18p1
            .quad 0x4001c44dc0000000, 0x0004e00000000000, 0x3fe886e5f0abb04a, 0x3fdcd159cdbba714 // z ~ 0x1.1cp1
            .quad 0x4002168000000000, 0x0005000000000000, 0x3fe8618618618618, 0x3fdc4e651e0c37d7 // z ~ 0x1.20p1
            .quad 0x400269ae40000000, 0x0005200000000000, 0x3fe83c977ab2bedd, 0x3fdbce853967753c // z ~ 0x1.24p1
            .quad 0x4002bdda00000000, 0x0005400000000000, 0x3fe8181818181818, 0x3fdb51a30f9739f8 // z ~ 0x1.28p1
            .quad 0x4002dfff74f29dc0, 0x00054ce000000000, 0x3fe80987c755886a, 0x3fdb203708429799 // z ~ 0x1.2cp1
            .quad 0x40031304c0000000, 0x0005600000000000, 0x3fe7f405fd017f40, 0x3fdad7a85e593e54 // z ~ 0x1.30p1
            .quad 0x4003693000000000, 0x0005800000000000, 0x3fe7d05f417d05f4, 0x3fda607fa909db1f // z ~ 0x1.34p1
            .quad 0x40039fe541ac7840, 0x0005942000000000, 0x3fe7ba298eae8947, 0x3fda16f787114257 // z ~ 0x1.38p1
            .quad 0x4003c05d40000000, 0x0005a00000000000, 0x3fe7ad2208e0ecc3, 0x3fd9ec1430b0dfc7 // z ~ 0x1.3cp1
            .quad 0x4004188e00000000, 0x0005c00000000000, 0x3fe78a4c8178a4c8, 0x3fd97a51ec6b707e // z ~ 0x1.40p1
            .quad 0x400471c3c0000000, 0x0005e00000000000, 0x3fe767dce434a9b1, 0x3fd90b25822e2a9f // z ~ 0x1.44p1
            .quad 0x40049fcfb130a6c0, 0x0005f06000000000, 0x3fe75664a1a72c8d, 0x3fd8d33bb2686480 // z ~ 0x1.48p1
            .quad 0x4004cc0000000000, 0x0006000000000000, 0x3fe745d1745d1746, 0x3fd89e7c3fdb1246 // z ~ 0x1.4cp1
            .quad 0x4005274440000000, 0x0006200000000000, 0x3fe724287f46debc, 0x3fd8344414a70cbd // z ~ 0x1.50p1
            .quad 0x40055fc05a5df140, 0x000633a000000000, 0x3fe70fb3e12b41c4, 0x3fd7f44d50c76c8e // z ~ 0x1.54p1
            .quad 0x4005839200000000, 0x0006400000000000, 0x3fe702e05c0b8170, 0x3fd7cc6b8acae7cb // z ~ 0x1.58p1
            .quad 0x4005e0eac0000000, 0x0006600000000000, 0x3fe6e1f76b4337c7, 0x3fd766e1c17c26ec // z ~ 0x1.5cp1
            .quad 0x40063f5000000000, 0x0006800000000000, 0x3fe6c16c16c16c17, 0x3fd70396672a04e5 // z ~ 0x1.60p1
            .quad 0x40065fa1cdfa11c0, 0x00068ae000000000, 0x3fe6b671c62a2d0a, 0x3fd6e257c2026aef // z ~ 0x1.64p1
            .quad 0x40069ec340000000, 0x0006a00000000000, 0x3fe6a13cd1537290, 0x3fd6a279b3fb4a4e // z ~ 0x1.68p1
            .quad 0x4006ff4600000000, 0x0006c00000000000, 0x3fe6816816816817, 0x3fd6437c6489c8e0 // z ~ 0x1.6cp1
            .quad 0x40071fef1bff2600, 0x0006cac000000000, 0x3fe676caae4b2e0f, 0x3fd6240aa2fa0dfd // z ~ 0x1.70p1
            .quad 0x400760d9c0000000, 0x0006e00000000000, 0x3fe661ec6a5122f9, 0x3fd5e68fb4d877a7 // z ~ 0x1.74p1
            .quad 0x40079fec8fa79000, 0x0006f48000000000, 0x3fe64def50b37b22, 0x3fd5ac1740057116 // z ~ 0x1.78p1
            .quad 0x4007c38000000000, 0x0007000000000000, 0x3fe642c8590b2164, 0x3fd58ba55b815609 // z ~ 0x1.7cp1
            .quad 0x4008273a40000000, 0x0007200000000000, 0x3fe623fa77016240, 0x3fd532af851862ac // z ~ 0x1.80p1
            .quad 0x40085fccde240000, 0x0007320000000000, 0x3fe612cc01b977f0, 0x3fd5017c2589970e // z ~ 0x1.84p1
            .quad 0x40088c0a00000000, 0x0007400000000000, 0x3fe6058160581606, 0x3fd4dba0cfc11861 // z ~ 0x1.88p1
            .quad 0x4008f1f0c0000000, 0x0007600000000000, 0x3fe5e75bb8d015e7, 0x3fd4866c46f405db // z ~ 0x1.8cp1
            .quad 0x40091fabaf07d200, 0x00076e4000000000, 0x3fe5da09741396f7, 0x3fd461102bc1cb8f // z ~ 0x1.90p1
            .quad 0x400958f000000000, 0x0007800000000000, 0x3fe5c9882b931057, 0x3fd433055f7235db // z ~ 0x1.94p1
            .quad 0x40099ffaac1ec3c0, 0x000795e000000000, 0x3fe5b55320eae3fd, 0x3fd3fb056724ebb2 // z ~ 0x1.98p1
            .quad 0x4009c10940000000, 0x0007a00000000000, 0x3fe5ac056b015ac0, 0x3fd3e15ff3643c49 // z ~ 0x1.9cp1
            .quad 0x400a2a3e00000000, 0x0007c00000000000, 0x3fe58ed2308158ed, 0x3fd391703ea2d9b9 // z ~ 0x1.a0p1
            .quad 0x400a5fad7a3ee040, 0x0007d02000000000, 0x3fe580391c97b3f3, 0x3fd369cab16c4bb8 // z ~ 0x1.a4p1
            .quad 0x400a948fc0000000, 0x0007e00000000000, 0x3fe571ed3c506b3a, 0x3fd3432adb274266 // z ~ 0x1.a8p1
            .quad 0x400adffcaf535000, 0x0007f68000000000, 0x3fe55dca75792aa1, 0x3fd30d1b5accf7d2 // z ~ 0x1.acp1
            .quad 0x400b000000000000, 0x0008000000000000, 0x3fe5555555555555, 0x3fd2f684bda12f68 // z ~ 0x1.b0p1
            .quad 0x400b6c9040000000, 0x0008200000000000, 0x3fe5390948f40feb, 0x3fd2ab733230f96f // z ~ 0x1.b4p1
            .quad 0x400b9fd76ec78000, 0x00082f0000000000, 0x3fe52bdf6a7a2620, 0x3fd288cb4a41a9b5 // z ~ 0x1.b8p1
            .quad 0x400bda4200000000, 0x0008400000000000, 0x3fe51d07eae2f815, 0x3fd261ebd944131e // z ~ 0x1.bcp1
            .quad 0x400c1fd3bf5cf840, 0x0008542000000000, 0x3fe50b90cb22a299, 0x3fd234731d751ccc // z ~ 0x1.c0p1
            .quad 0x400c4916c0000000, 0x0008600000000000, 0x3fe5015015015015, 0x3fd219e4a4924f1f // z ~ 0x1.c4p1
            .quad 0x400cb91000000000, 0x0008800000000000, 0x3fe4e5e0a72f0539, 0x3fd1d353d43a7247 // z ~ 0x1.c8p1
            .quad 0x400cdfd181598000, 0x00088b0000000000, 0x3fe4dc82df5d0542, 0x3fd1bb66cda74540 // z ~ 0x1.ccp1
            .quad 0x400d2a2f40000000, 0x0008a00000000000, 0x3fe4cab88725af6e, 0x3fd18e2ff3fca5ac // z ~ 0x1.d0p1
            .quad 0x400d5f9b87878000, 0x0008af0000000000, 0x3fe4be15f5393e98, 0x3fd16e4227697dbf // z ~ 0x1.d4p1
            .quad 0x400d9c7600000000, 0x0008c00000000000, 0x3fe4afd6a052bf5b, 0x3fd14a6fd8916ecf // z ~ 0x1.d8p1
            .quad 0x400ddff55aa1e600, 0x0008d2c000000000, 0x3fe4a036770fd266, 0x3fd1235f02ce295a // z ~ 0x1.dcp1
            .quad 0x400e0fe5c0000000, 0x0008e00000000000, 0x3fe49539e3b2d067, 0x3fd1080a9d1be542 // z ~ 0x1.e0p1
            .quad 0x400e5fefa40c0000, 0x0008f60000000000, 0x3fe48315b6c3fc79, 0x3fd0db29bc986108 // z ~ 0x1.e4p1
            .quad 0x400e848000000000, 0x0009000000000000, 0x3fe47ae147ae147b, 0x3fd0c6f7a0b5ed8d // z ~ 0x1.e8p1
            .quad 0x400efa4640000000, 0x0009200000000000, 0x3fe460cbc7f5cf9a, 0x3fd0872e8415508d // z ~ 0x1.ecp1
            .quad 0x400f1fc8b255bc40, 0x00092a2000000000, 0x3fe45898cb57730c, 0x3fd0734344eaebef // z ~ 0x1.f0p1
            .quad 0x400f713a00000000, 0x0009400000000000, 0x3fe446f86562d9fb, 0x3fd048a727489527 // z ~ 0x1.f4p1
            .quad 0x400f9f8b6ce70ec0, 0x00094c6000000000, 0x3fe43d0d2af8e146, 0x3fd030cd637fd65e // z ~ 0x1.f8p1
            .quad 0x400fe95cc0000000, 0x0009600000000000, 0x3fe42d6625d51f87, 0x3fd00b59a78a8ffc // z ~ 0x1.fcp1
            .quad 0x4010315800000000, 0x0009800000000000, 0x3fe4141414141414, 0x3fcf9e7cba5753af // z ~ 0x1.00p2
            .quad 0x40106e9aa0000000, 0x0009a00000000000, 0x3fe3fb013fb013fb, 0x3fcf289bb31fd41c // z ~ 0x1.04p2
            .quad 0x4010ac7700000000, 0x0009c00000000000, 0x3fe3e22cbce4a902, 0x3fceb501ca81bb3e // z ~ 0x1.08p2
            .quad 0x4010eaede0000000, 0x0009e00000000000, 0x3fe3c995a47babe7, 0x3fce43a0fc24fe4b // z ~ 0x1.0cp2
            .quad 0x40112a0000000000, 0x000a000000000000, 0x3fe3b13b13b13b14, 0x3fcdd46baab49c24 // z ~ 0x1.10p2
            .quad 0x401169ae20000000, 0x000a200000000000, 0x3fe3991c2c187f63, 0x3fcd67549c6f9b67 // z ~ 0x1.14p2
            .quad 0x4011a9f900000000, 0x000a400000000000, 0x3fe3813813813814, 0x3fccfc4ef7db5bff // z ~ 0x1.18p2
            .quad 0x4011eae160000000, 0x000a600000000000, 0x3fe3698df3de0748, 0x3fcc934e4095d202 // z ~ 0x1.1cp2
            .quad 0x40122c6800000000, 0x000a800000000000, 0x3fe3521cfb2b78c1, 0x3fcc2c46544650c1 // z ~ 0x1.20p2
            .quad 0x40126e8da0000000, 0x000aa00000000000, 0x3fe33ae45b57bcb2, 0x3fcbc72b67ab9ce7 // z ~ 0x1.24p2
            .quad 0x4012b15300000000, 0x000ac00000000000, 0x3fe323e34a2b10bf, 0x3fcb63f203c60c07 // z ~ 0x1.28p2
            .quad 0x4012f4b8e0000000, 0x000ae00000000000, 0x3fe30d190130d190, 0x3fcb028f031c8644 // z ~ 0x1.2cp2
            .quad 0x401338c000000000, 0x000b000000000000, 0x3fe2f684bda12f68, 0x3fcaa2f78f1b4cc6 // z ~ 0x1.30p2
            .quad 0x40137d6920000000, 0x000b200000000000, 0x3fe2e025c04b8097, 0x3fca45211d8b748a // z ~ 0x1.34p2
            .quad 0x40139ffaac000000, 0x000b300000000000, 0x3fe2d50a012d50a0, 0x3fca16db0ec408b2 // z ~ 0x1.38p2
            .quad 0x4013c2b500000000, 0x000b400000000000, 0x3fe2c9fb4d812ca0, 0x3fc9e9016e2211b6 // z ~ 0x1.3cp2
            .quad 0x401408a460000000, 0x000b600000000000, 0x3fe2b404ad012b40, 0x3fc98e8e88261b62 // z ~ 0x1.40p2
            .quad 0x40144f3800000000, 0x000b800000000000, 0x3fe29e4129e4129e, 0x3fc935beb82c1ae7 // z ~ 0x1.44p2
            .quad 0x40149670a0000000, 0x000ba00000000000, 0x3fe288b01288b013, 0x3fc8de888de6c48f // z ~ 0x1.48p2
            .quad 0x4014de4f00000000, 0x000bc00000000000, 0x3fe27350b8812735, 0x3fc888e2da0ba19d // z ~ 0x1.4cp2
            .quad 0x401526d3e0000000, 0x000be00000000000, 0x3fe25e22708092f1, 0x3fc834c4ac4afd3b // z ~ 0x1.50p2
            .quad 0x4015700000000000, 0x000c000000000000, 0x3fe2492492492492, 0x3fc7e225515a4f1d // z ~ 0x1.54p2
            .quad 0x4015b9d420000000, 0x000c200000000000, 0x3fe23456789abcdf, 0x3fc790fc51106751 // z ~ 0x1.58p2
            .quad 0x4015dfdce5811360, 0x000c306000000000, 0x3fe229c346a04441, 0x3fc7680273c586ed // z ~ 0x1.5cp2
            .quad 0x4016045100000000, 0x000c400000000000, 0x3fe21fb78121fb78, 0x3fc741416c92a70b // z ~ 0x1.60p2
            .quad 0x40164f7760000000, 0x000c600000000000, 0x3fe20b470c67c0d9, 0x3fc6f2ec9c929a29 // z ~ 0x1.64p2
            .quad 0x40169b4800000000, 0x000c800000000000, 0x3fe1f7047dc11f70, 0x3fc6a5f60f9b4c97 // z ~ 0x1.68p2
            .quad 0x4016e7c3a0000000, 0x000ca00000000000, 0x3fe1e2ef3b3fb874, 0x3fc65a56286dbe08 // z ~ 0x1.6cp2
            .quad 0x401734eb00000000, 0x000cc00000000000, 0x3fe1cf06ada2811d, 0x3fc610057c6bdd38 // z ~ 0x1.70p2
            .quad 0x40175fb34f0902a0, 0x000cd1a000000000, 0x3fe1c4227955e4f1, 0x3fc5e7a396f89f71 // z ~ 0x1.74p2
            .quad 0x401782bee0000000, 0x000ce00000000000, 0x3fe1bb4a4046ed29, 0x3fc5c6fcd2117a65 // z ~ 0x1.78p2
            .quad 0x4017d14000000000, 0x000d000000000000, 0x3fe1a7b9611a7b96, 0x3fc57f351f7aa6ea // z ~ 0x1.7cp2
            .quad 0x4018206f20000000, 0x000d200000000000, 0x3fe19453808ca29c, 0x3fc538a788f6fdd6 // z ~ 0x1.80p2
            .quad 0x4018704d00000000, 0x000d400000000000, 0x3fe1811811811812, 0x3fc4f34d5fa956d6 // z ~ 0x1.84p2
            .quad 0x40189fbb1ca4e0e0, 0x000d52e000000000, 0x3fe175d3b160af03, 0x3fc4caf2b205f9dd // z ~ 0x1.88p2
            .quad 0x4018c0da60000000, 0x000d600000000000, 0x3fe16e0689427379, 0x3fc4af2020336a59 // z ~ 0x1.8cp2
            .quad 0x4019121800000000, 0x000d800000000000, 0x3fe15b1e5f75270d, 0x3fc46c19716cf2c0 // z ~ 0x1.90p2
            .quad 0x40196406a0000000, 0x000da00000000000, 0x3fe1485f0e0acd3b, 0x3fc42a332325db6b // z ~ 0x1.94p2
            .quad 0x4019b6a700000000, 0x000dc00000000000, 0x3fe135c81135c811, 0x3fc3e9672cf3131d // z ~ 0x1.98p2
            .quad 0x4019dfe6c1816fe0, 0x000dcfe000000000, 0x3fe12c9df926137b, 0x3fc3c9a8f2a1f8a5 // z ~ 0x1.9cp2
            .quad 0x401a09f9e0000000, 0x000de00000000000, 0x3fe12358e75d3033, 0x3fc3a9afad059b87 // z ~ 0x1.a0p2
            .quad 0x401a5e0000000000, 0x000e000000000000, 0x3fe1111111111111, 0x3fc36b06e70b7421 // z ~ 0x1.a4p2
            .quad 0x401ab2ba20000000, 0x000e200000000000, 0x3fe0fef010fef011, 0x3fc32d67431a0280 // z ~ 0x1.a8p2
            .quad 0x401adfb1053dbae0, 0x000e30e000000000, 0x3fe0f57023f898dc, 0x3fc30d50fe844fd2 // z ~ 0x1.acp2
            .quad 0x401b082900000000, 0x000e400000000000, 0x3fe0ecf56be69c90, 0x3fc2f0cb4ca19e1e // z ~ 0x1.b0p2
            .quad 0x401b5e4d60000000, 0x000e600000000000, 0x3fe0db20a88f4696, 0x3fc2b52db169e95e // z ~ 0x1.b4p2
            .quad 0x401bb52800000000, 0x000e800000000000, 0x3fe0c9714fbcda3b, 0x3fc27a894096a4f5 // z ~ 0x1.b8p2
            .quad 0x401bdfd332712ca0, 0x000e8fa000000000, 0x3fe0c0dc264ce74b, 0x3fc25e3ff656ec87 // z ~ 0x1.bcp2
            .quad 0x401c0cb9a0000000, 0x000ea00000000000, 0x3fe0b7e6ec259dc8, 0x3fc240d8e9b4ae5d // z ~ 0x1.c0p2
            .quad 0x401c650300000000, 0x000ec00000000000, 0x3fe0a6810a6810a7, 0x3fc20817bbcedd1f // z ~ 0x1.c4p2
            .quad 0x401cbe04e0000000, 0x000ee00000000000, 0x3fe0953f39010954, 0x3fc1d040e48a75cd // z ~ 0x1.c8p2
            .quad 0x401cdfeef0724420, 0x000eec2000000000, 0x3fe08ebe9d4e24ae, 0x3fc1bb54ba55bb8e // z ~ 0x1.ccp2
            .quad 0x401d17c000000000, 0x000f000000000000, 0x3fe0842108421084, 0x3fc1994faf4aec92 // z ~ 0x1.d0p2
            .quad 0x401d723520000000, 0x000f200000000000, 0x3fe073260a47f7c6, 0x3fc1633f845cb3de // z ~ 0x1.d4p2
            .quad 0x401d9fb5ac000000, 0x000f300000000000, 0x3fe06ab59c7912fb, 0x3fc1488a6b10c148 // z ~ 0x1.d8p2
            .quad 0x401dcd6500000000, 0x000f400000000000, 0x3fe0624dd2f1a9fc, 0x3fc12e0be826d695 // z ~ 0x1.dcp2
            .quad 0x401e295060000000, 0x000f600000000000, 0x3fe05197f7d73404, 0x3fc0f9b07a631f92 // z ~ 0x1.e0p2
            .quad 0x401e5fe06d9140e0, 0x000f72e000000000, 0x3fe047cca585fbe4, 0x3fc0db322dce8431 // z ~ 0x1.e4p2
            .quad 0x401e85f800000000, 0x000f800000000000, 0x3fe0410410410410, 0x3fc0c628f55c92de // z ~ 0x1.e8p2
            .quad 0x401ee35ca0000000, 0x000fa00000000000, 0x3fe03091b51f5e1a, 0x3fc093712d33ff42 // z ~ 0x1.ecp2
            .quad 0x401f1ff2ff2d4ba0, 0x000fb4a000000000, 0x3fe02609989a73cf, 0x3fc0732ce999c3d1 // z ~ 0x1.f0p2
            .quad 0x401f417f00000000, 0x000fc00000000000, 0x3fe0204081020408, 0x3fc061850f2a7123 // z ~ 0x1.f4p2
            .quad 0x401fa05fe0000000, 0x000fe00000000000, 0x3fe0101010101010, 0x3fc03060a0f151c2 // z ~ 0x1.f8p2
            .quad 0x401fdfcad4496d00, 0x000ff54000000000, 0x3fe00561cedb7dbc, 0x3fc0102adb93eef1 // z ~ 0x1.fcp2

magic_numbers:	.quad 0x0000000000aaaaab, 0x00000000002aaaab
exponent_mask:	.quad 0x0000000001800000, 0x000000007f800000
exponent_bias:	.quad 0x3ff0000000000000, 0x3d50000000000000
denormal_bias:	.quad 0x3ff0000000000000, 0x3ab0000000000000

.text
.align 4

#if defined( __x86_64__ )
	#define RELATIVE_ADDR( _a )		(_a)( %rip )
#else
	#define RELATIVE_ADDR( _a )		(_a)-cbrtf_body( %ecx )
cbrtf_pic:
	movl		(%esp),			%ecx				// Copy address of this instruction to %ecx
	ret
#endif

ENTRY(cbrtf)
#if defined(__i386__)
	movl	FRAME_SIZE(STACKP),	%eax
	movss	FRAME_SIZE(STACKP),	%xmm0
// pic code, boring.
	calll		cbrtf_pic
cbrtf_body:
#else
	movd		%xmm0,			%eax
#endif

	andl		$0x7fffffff,	%eax
	movd		%eax,			%xmm1
	
	subl		$0x00800000,	%eax	// push 0 and denormals negative
	cmpl		$0x7f000000,	%eax	// if ( |x| < 2^-126 or |x| = inf or isnan(x) )
	jae			3f						//    goto 2

// Normal numbers code path
	xorps		%xmm1,			%xmm0	// xmm0 <-- signbit(x)
	movdqa	RELATIVE_ADDR(exponent_bias),	%xmm3
	movd		%eax,			%xmm1
	
// Denormals re-enter code path here, using denormal exponent bias to rescale
2:
	psrld		$23,			%xmm1	// xmm1 <-- biased exponent(x) - 1 as integer
	movlhps		%xmm1,			%xmm1
	pmuludq	RELATIVE_ADDR(magic_numbers),	%xmm1	// magic number multiply and mask to put (e-1)/3 and (e-1)%3 
	pand	RELATIVE_ADDR(exponent_mask),	%xmm1	//       in the high and low parts of xmm1, respectively.
	andl		$0x007fffff,	%eax	// mask sign and exponent bits to 0.
	movd		%eax,			%xmm2	// xmm2 <-- mantissa(x)
	por			%xmm2,			%xmm1	// xmm1 <-- [ (e-1)/3, (e-1)%3 | mantissa ]
	
	lea		RELATIVE_ADDR(cbrt_table),	DX_P
	movd		%xmm1,			%eax	// eax <-- (e-1)%3 | mantissa
#if defined(__x86_64__)
	cdqe								// sign extend rax
#endif
	andl		$0x01fe0000,	%eax	// mask off top six mantissa bits and bottom two exponent bits
	shrl		$(17-5),		%eax	// shift into position for lookup (table entry size is 32 = 2^5)
	
	psllq		$29,			%xmm1	// convert xmm1 to double via shift and bias add.
	paddq		%xmm3,			%xmm1
	
	movhlps		%xmm1,			%xmm2	// unpack high(xmm1) = 2^floor(exponent(x)/3)
	// x is now reduced to the range 1-8 via multiplication by 2^(-3n)
	// i.e., x has the biased exponent (e-1)%3 = (e-127)%3
	subsd		(DX_P,AX_P,1),	%xmm1	// xmm1 <-- (x - z) where z ~ x is an exact cube
	mulsd	  24(DX_P,AX_P,1),	%xmm1	// xmm1 <-- (x - z)/z = r
	movsd	   8(DX_P,AX_P,1),	%xmm3	// xmm3 <-- mantissa(cbrt(z))
	orpd		%xmm3,			%xmm2	// xmm2 <-- 2^floor(exponent(x)/3) * cbrt(z) = w
										//			computed by or-ing in the mantissa of cbrt(z)
	
	// polynomial approximation p(r) ~ cbrt(1 + r) - 1:
	movsd	 -16(DX_P),			%xmm4	// load polynomial coefficients
	movapd	 -48(DX_P),			%xmm5
#if defined( __SSE3__ )
	movddup		%xmm1,			%xmm6
#else
	movapd		%xmm1,			%xmm6
	unpcklpd	%xmm6,			%xmm6
#endif
	mulsd		%xmm1,			%xmm4	// xmm4 <-- cx
	addpd		%xmm6,			%xmm5	// xmm5 <-- [ x + b1hi, x + b1lo ]
	addsd	  -8(DX_P),			%xmm1	// xmm1 <-- x + a
	mulpd		%xmm5,			%xmm6	// xmm6 <-- [ x^2 + b1hi x, x^2 + b1lo x ]
	mulsd		%xmm4,			%xmm1	// xmm1 <-- cx(x + a)
	addpd	 -32(DX_P),			%xmm6	// xmm6 <-- [ x^2 + b1hi x + b0hi, x^2 + b1lo x + b0lo ]
	movhlps		%xmm6,			%xmm3	// xmm3 <-- x^2 + b1hi x + b0hi
	mulsd		%xmm1,			%xmm6	// xmm6 <-- cx(x + a)(x^2 + b1lo x + b0lo)
	mulsd		%xmm3,			%xmm6	// xmm6 <-- cx(x + a)(x^2 + b1lo x + b0lo)(x^2 + b1hi x + b0hi)
	
	// y = w*((cbrt(1+r) - 1) + 1) ~ w + p(r)*w
	mulsd		%xmm2,			%xmm6
	addsd		%xmm6,			%xmm2
	
	// convert to single and return.
	cvtsd2ss	%xmm2,			%xmm1
	orps		%xmm1,			%xmm0
#if defined(__i386__)
	movss		%xmm0,			FRAME_SIZE( STACKP )
	flds		FRAME_SIZE( STACKP )
#endif
	ret

3:	
	jge			4f
	cmpl		$0xff800000,	%eax
	je			5f
	
	// Denormal code path
	xorps		%xmm1,			%xmm0	// xmm0 <-- signbit(x)
	movdqa	RELATIVE_ADDR(denormal_bias),	%xmm3
	addl		$0x3f800000,	%eax	// 0.5 | denormal bits
	movl		$0x3f000000,	%edx	// 0.5
	movd		%eax,			%xmm1
	movd		%edx,			%xmm2
	subss		%xmm2,			%xmm1
	movd		%xmm1,			%eax
	jmp			2b
	
4:
	je			5f
	addss		%xmm0,			%xmm0
5:
#if defined(__i386__)
	movss		%xmm0,			FRAME_SIZE( STACKP )
	flds		FRAME_SIZE( STACKP )
#endif
	ret
