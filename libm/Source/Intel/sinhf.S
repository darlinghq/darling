/*
 * sinhf.s
 *
 *      by Stephen Canon
 *
 * Copyright (c) 2007, Apple Inc.  All Rights Reserved.
 *
 * This file implements the C99 sinhf function for the MacOS X __i386__ and __x86_64__ abis.
 */

#include <machine/asm.h>
#include "abi.h"

// Basic approach
//
//  First, we take the absolute value of x, compute on that, and then patch the sign back in at the
//  end of the computation.
//
//  Arguments to sinhf can be separated into two ranges - the "uninteresting range" from 11 to
//  the overflow boundary, on which sinh(x) = e/2 * exp(x - 1), and the "interesting range" on which
//  exp(-x) and exp(x) are close enough in value that something more careful must be done.
//
//  Particular care is required for values between 0 and 1, where massive cancellation can result in
//  a rather drastic loss of accuracy.  Fortunately, a simple taylor series approximation
//
//      sinh x = x + x^3 / 6 + x^5/120
//
//  suffices to handle all values smaller than 2^-4.  To extend this to all values between 0 and 11,
//  we do an argument reduction via the sinh addition formula:
//
//      sinh x + y = sinh x cosh y + cosh x sinh y
//
//  Specifically, we separate x into high + low, 0 <= low < 2^-4.  We use the value of high to perform
//  a table lookup for the values of Sh = sinh(high) and Ch = cosh(high).  Then let
//
//     Sl = low + 1/6 low^3 + 1/120 low^5
//     Cl = 1 +   1/2 low^2 + 0.0416715 low^4
//
//     *Note that Cl is "almost" the taylor series for cosh(low), but the highest order
//      term has been fudged to get a smaller absolute error on the approximation interval.
//      (This allows us to keep our error below .51 ulps globally - actually below .5002 ulps on [-11,11]).
//
//  We now compute
//
//     result = Sl * Ch   +   Cl * Sh
//
//  For the "uninteresting range", the code is taken directly from expf.  Read the
//  comments in that file for more information.

//.const

.align (1<<4)
				.quad	0x3f811111111110fe,	0x3fa555f78359bc34 // c5, c4 = 0.0083333..., 0.0416715
				.quad	0x4034000000000018,	0x4027ff4991a5ebc5 // c3/c5 = 0.1666666.../c5, c2/c4 = 0.5/c4
				.quad	0x0000000000000000,	0x3ff0000000000000 // 0, 1
sinhf_table:	.quad	0x3ff0000000000001,	0x0000000000000000 // cosh(0/16) + eps, sinh(0/16)
				.quad	0x3ff00800aab05b20,	0x3fb002aacccd9cdd // cosh(1/16), sinh(1/16)
				.quad	0x3ff0200aac16db6f,	0x3fc00aaccd00d2f1 // cosh(2/16), sinh(2/16)
				.quad	0x3ff048361035cdfa,	0x3fc8241036ac51dd // cosh(3/16), sinh(3/16)
				.quad	0x3ff080ab05ca6146,	0x3fd02accd9d08102 // cosh(4/16), sinh(4/16)
				.quad	0x3ff0c9a2067ebbda,	0x3fd453bdbe16906c // cosh(5/16), sinh(5/16)
				.quad	0x3ff123640f685b59,	0x3fd8910411ce5046 // cosh(6/16), sinh(6/16)
				.quad	0x3ff18e4aea0b3f4a,	0x3fdce6dd75bf0317 // cosh(7/16), sinh(7/16)
				.quad	0x3ff20ac1862ae8d0,	0x3fe0acd00fe63b97 // cosh(8/16), sinh(8/16)
				.quad	0x3ff2994464c307c6,	0x3fe2f6df98c4b901 // cosh(9/16), sinh(9/16)
				.quad	0x3ff33a621492d6da,	0x3fe553e795dc19cd // cosh(10/16), sinh(10/16)
				.quad	0x3ff3eebbc0b7bc6c,	0x3fe7c645419678b8 // cosh(11/16), sinh(11/16)
				.quad	0x3ff4b705d1e5d6a8,	0x3fea506b2dd3c690 // cosh(12/16), sinh(12/16)
				.quad	0x3ff59408a2dfb8da,	0x3fecf4e3b6afe2ad // cosh(13/16), sinh(13/16)
				.quad	0x3ff686a148e1e0d1,	0x3fefb6538d14eafc // cosh(14/16), sinh(14/16)
				.quad	0x3ff78fc270ca6067,	0x3ff14bbe2dd24609 // cosh(15/16), sinh(15/16)
				.quad	0x3ff8b07551d9f550,	0x3ff2cd9fc44eb982 // cosh(16/16), sinh(16/16)
				.quad	0x3ff9e9dab7016488,	0x3ff462508bbf80a9 // cosh(17/16), sinh(17/16)
				.quad	0x3ffb3d2c1fc47ccc,	0x3ff60b6556a69204 // cosh(18/16), sinh(18/16)
				.quad	0x3ffcabbcf9d3bb3f,	0x3ff7ca875d3c6932 // cosh(19/16), sinh(19/16)
				.quad	0x3ffe36fbf49645fa,	0x3ff9a175e6cbafe6 // cosh(20/16), sinh(20/16)
				.quad	0x3fffe0746ff7e2d8,	0x3ffb9208091dcfc6 // cosh(21/16), sinh(21/16)
				.quad	0x4000d4e803f4eb7f,	0x3ffd9e2e7fb7fef3 // cosh(22/16), sinh(22/16)
				.quad	0x4001ca6c1f11287b,	0x3fffc7f59cc02b95 // cosh(23/16), sinh(23/16)
				.quad	0x4002d1bc21e22022,	0x400108c3aabd6a60 // cosh(24/16), sinh(24/16)
				.quad	0x4003ebdf725cb441,	0x40023e96b6373d25 // cosh(25/16), sinh(25/16)
				.quad	0x400519f04b551971,	0x400386a9ddab7a8a // cosh(26/16), sinh(26/16)
				.quad	0x40065d1cd6d130fb,	0x4004e2454f996e13 // cosh(27/16), sinh(27/16)
				.quad	0x4007b6a85c4bbdc2,	0x400652c4c46b9bbb // cosh(28/16), sinh(28/16)
				.quad	0x400927ec8416d084,	0x4007d998da4d257b // cosh(29/16), sinh(29/16)
				.quad	0x400ab25ab120e8ea,	0x4009784885e6af4c // cosh(30/16), sinh(30/16)
				.quad	0x400c577d7276ad4e,	0x400b307299739d43 // cosh(31/16), sinh(31/16)
				.quad	0x400e18fa0df2d9bc,	0x400d03cf63b6e19f // cosh(32/16), sinh(32/16)
				.quad	0x400ff89225a736fd,	0x400ef432686e7235 // cosh(33/16), sinh(33/16)
				.quad	0x4010fc12bcd212e7,	0x401081c619fefea9 // cosh(34/16), sinh(34/16)
				.quad	0x40120cd9e3f055a1,	0x401199f6261257ea // cosh(35/16), sinh(35/16)
				.quad	0x40132faf66118731,	0x4012c3c19fd775d1 // cosh(36/16), sinh(36/16)
				.quad	0x401465b630f50d1d,	0x401400526b99e613 // cosh(37/16), sinh(37/16)
				.quad	0x4015b024653c8da5,	0x401550e53487b291 // cosh(38/16), sinh(38/16)
				.quad	0x401710448ca66a4d,	0x4016b6caa976f3de // cosh(39/16), sinh(39/16)
				.quad	0x40188776e4b30aa3,	0x40183368cdb0b6d3 // cosh(40/16), sinh(40/16)
				.quad	0x401a1732beffb81f,	0x4019c83c5f121c3c // cosh(41/16), sinh(41/16)
				.quad	0x401bc107f8b78338,	0x401b76da52e9f182 // cosh(42/16), sinh(42/16)
				.quad	0x401d86a08a91c20b,	0x401d40f16b0fbfb3 // cosh(43/16), sinh(43/16)
				.quad	0x401f69c232ee483d,	0x401f284be4c989bd // cosh(44/16), sinh(44/16)
				.quad	0x4020b6281ddccbf9,	0x40209768a197a1bd // cosh(45/16), sinh(45/16)
				.quad	0x4021c826aeef8ae6,	0x4021ab441b6b45a1 // cosh(46/16), sinh(46/16)
				.quad	0x4022ebeee2166d42,	0x4022d0cc52573d2d // cosh(47/16), sinh(47/16)
				.quad	0x402422a497d6185e,	0x40240926e70949ae // cosh(48/16), sinh(48/16)
				.quad	0x40256d7e9fc9a2ac,	0x4025558c4e1e87b4 // cosh(49/16), sinh(49/16)
				.quad	0x4026cdc7ef8c1654,	0x4026b74908b216cf // cosh(50/16), sinh(50/16)
				.quad	0x402844e0edc9a1a2,	0x40282fbef0f9eaff // cosh(51/16), sinh(51/16)
				.quad	0x4029d440d2c3a213,	0x4029c0669c3e8083 // cosh(52/16), sinh(52/16)
				.quad	0x402b7d771fa82b6c,	0x402b6ad0d38f8723 // cosh(53/16), sinh(53/16)
				.quad	0x402d422d2e3481ad,	0x402d30a824ae5918 // cosh(54/16), sinh(54/16)
				.quad	0x402f2427da3249ad,	0x402f13b28cbf4962 // cosh(55/16), sinh(55/16)
				.quad	0x403092a4a33c887b,	0x40308ae99f364f3b // cosh(56/16), sinh(56/16)
				.quad	0x4031a3c95f9caa21,	0x40319c8642a0c10e // cosh(57/16), sinh(57/16)
				.quad	0x4032c6935db9bbdc,	0x4032bfc0e41034cd // cosh(58/16), sinh(58/16)
				.quad	0x4033fc257fce2961,	0x4033f5bcd66bcbd3 // cosh(59/16), sinh(59/16)
				.quad	0x403545b571c910c9,	0x40353fb02f7bbd05 // cosh(60/16), sinh(60/16)
				.quad	0x4036a48cdf1400d7,	0x40369ee4fe18f513 // cosh(61/16), sinh(61/16)
				.quad	0x40381a0abc59dc70,	0x403814ba94577184 // cosh(62/16), sinh(62/16)
				.quad	0x4039a7a4a698c57c,	0x4039a2a6e6f59c8a // cosh(63/16), sinh(63/16)
				.quad	0x403b4ee858de3e80,	0x403b4a3803703631 // cosh(64/16), sinh(64/16)
				.quad	0x403d117d3a235e29,	0x403d0d159e30fdff // cosh(65/16), sinh(65/16)
				.quad	0x403ef12604d71220,	0x403eed02ba666cf1 // cosh(66/16), sinh(66/16)
				.quad	0x404077e144df0f63,	0x404075efb6963d64 // cosh(67/16), sinh(67/16)
				.quad	0x404187a8c7f5f0ae,	0x404185d55ee4de8c // cosh(68/16), sinh(68/16)
				.quad	0x4042a8f969d9fab0,	0x4042a7425270a2a2 // cosh(69/16), sinh(69/16)
				.quad	0x4043dcf49349ecb2,	0x4043db58164c4cde // cosh(70/16), sinh(70/16)
				.quad	0x404524ce591a551f,	0x4045234ad9e90f00 // cosh(71/16), sinh(71/16)
				.quad	0x404681ceb0641358,	0x40468062ab5fa9fc // cosh(72/16), sinh(72/16)
				.quad	0x4047f552b694c5ea,	0x4047f3fcbf99de24 // cosh(73/16), sinh(73/16)
				.quad	0x404980ce0ea950eb,	0x40497f8ccfa46fa0 // cosh(74/16), sinh(74/16)
				.quad	0x404b25cc54efd428,	0x404b249e8c872e52 // cosh(75/16), sinh(75/16)
				.quad	0x404ce5f2aac4f20f,	0x404ce4d72b16f828 // cosh(76/16), sinh(76/16)
				.quad	0x404ec3015bd8459a,	0x404ec1f7094da8e6 // cosh(77/16), sinh(77/16)
				.quad	0x40505f6acf4eb766,	0x40505eedb766b932 // cosh(78/16), sinh(78/16)
				.quad	0x40516db5b8d52617,	0x40516d403528230b // cosh(79/16), sinh(79/16)
				.quad	0x40528d6fcbeff3aa,	0x40528d0166f07374 // cosh(80/16), sinh(80/16)
				.quad	0x4053bfb8daad33cb,	0x4053bf5125ed0389 // cosh(81/16), sinh(81/16)
				.quad	0x405505c347a2941d,	0x40550561db644eef // cosh(82/16), sinh(82/16)
				.quad	0x405660d538697af3,	0x40566079b338c3f3 // cosh(83/16), sinh(83/16)
				.quad	0x4057d249dbdfcf6b,	0x4057d1f3e22fd533 // cosh(84/16), sinh(84/16)
				.quad	0x40595b92c573c6e2,	0x40595b420143af17 // cosh(85/16), sinh(85/16)
				.quad	0x405afe395ed62077,	0x405afded7f5affc4 // cosh(86/16), sinh(86/16)
				.quad	0x405cbbe071849fb1,	0x405cbb992ad8a81e // cosh(87/16), sinh(87/16)
				.quad	0x405e9645c9b6718b,	0x405e9602d48d0661 // cosh(88/16), sinh(88/16)
				.quad	0x406047a1fa26c59e,	0x4060478286d5f738 // cosh(89/16), sinh(89/16)
				.quad	0x4061546a0cc58c9a,	0x4061544c8142b58e // cosh(90/16), sinh(90/16)
				.quad	0x40627287fb30ed34,	0x4062726c39ee144b // cosh(91/16), sinh(91/16)
				.quad	0x4063a319fb2ff225,	0x4063a2ffe8698457 // cosh(92/16), sinh(92/16)
				.quad	0x4064e750b824f30a,	0x4064e73839c5fd9c // cosh(93/16), sinh(93/16)
				.quad	0x4066407083d25b35,	0x40664059815a7498 // cosh(94/16), sinh(94/16)
				.quad	0x4067afd29ac773cd,	0x4067afbcfd32392a // cosh(95/16), sinh(95/16)
				.quad	0x406936e67db9b919,	0x406936d22f67c805 // cosh(96/16), sinh(96/16)
				.quad	0x406ad73361243111,	0x406ad7204dc5865b // cosh(97/16), sinh(97/16)
				.quad	0x406c9259b49c812f,	0x406c9247c91c272c // cosh(98/16), sinh(98/16)
				.quad	0x406e6a14c3653934,	0x406e6a03edd63132 // cosh(99/16), sinh(99/16)
				.quad	0x4070301e37ef03f1,	0x407030164fb4add6 // cosh(100/16), sinh(100/16)
				.quad	0x40713b6385c6b76d,	0x40713b5c1830ac2d // cosh(101/16), sinh(101/16)
				.quad	0x407257e5a6ce1350,	0x407257deac6e1e63 // cosh(102/16), sinh(102/16)
				.quad	0x407386c134dc6c0c,	0x407386baa6b7989b // cosh(103/16), sinh(103/16)
				.quad	0x4074c92524bd9ddc,	0x4074c91efc453b37 // cosh(104/16), sinh(104/16)
				.quad	0x40762053f540188a,	0x4076204e2c4b2aea // cosh(105/16), sinh(105/16)
				.quad	0x40778da4f1ce8eab,	0x40778d9f8293a5b4 // cosh(106/16), sinh(106/16)
				.quad	0x4079128589d7fce2,	0x407912806ee769b9 // cosh(107/16), sinh(107/16)
				.quad	0x407ab07abe5d8dd7,	0x407ab075f29bf2fb // cosh(108/16), sinh(108/16)
				.quad	0x407c6922a7140735,	0x407c691e25b53cb7 // cosh(109/16), sinh(109/16)
				.quad	0x407e3e36109e018f,	0x407e3e31d5204885 // cosh(110/16), sinh(110/16)
				.quad	0x408018c51abea3f7,	0x408018c31dd26428 // cosh(111/16), sinh(111/16)
				.quad	0x4081228949ba3a8c,	0x408122876ba380c9 // cosh(112/16), sinh(112/16)
				.quad	0x40823d716f972bb3,	0x40823d6fae77b96b // cosh(113/16), sinh(113/16)
				.quad	0x40836a988c0f760c,	0x40836a96e626065d // cosh(114/16), sinh(114/16)
				.quad	0x4084ab2bdf58ffc1,	0x4084ab2a52ff8611 // cosh(115/16), sinh(115/16)
				.quad	0x4086006c177ee7f2,	0x4086006aa328e9c0 // cosh(116/16), sinh(116/16)
				.quad	0x40876bae912a4be4,	0x40876bad33635429 // cosh(117/16), sinh(117/16)
				.quad	0x4088ee5ead1b6375,	0x4088ee5d64858e12 // cosh(118/16), sinh(118/16)
				.quad	0x408a89ff3ba88a65,	0x408a89fe06fb2623 // cosh(119/16), sinh(119/16)
				.quad	0x408c402bffaed3ce,	0x408c402addb5198d // cosh(120/16), sinh(120/16)
				.quad	0x408e129b4a773895,	0x408e129a3a0f1671 // cosh(121/16), sinh(121/16)
				.quad	0x4090018fd9163433,	0x4090018f5922afc4 // cosh(122/16), sinh(122/16)
				.quad	0x409109d4f24bebc3,	0x409109d47a18f5c9 // cosh(123/16), sinh(123/16)
				.quad	0x409223254bfc76bb,	0x40922324db11d23c // cosh(124/16), sinh(124/16)
				.quad	0x40934e9a4df3aa86,	0x40934e99e3e06372 // cosh(125/16), sinh(125/16)
				.quad	0x40948d5f8628be20,	0x40948d5f2282b85b // cosh(126/16), sinh(126/16)
				.quad	0x4095e0b3d46538ab,	0x4095e0b376c8c5e0 // cosh(127/16), sinh(127/16)
				.quad	0x409749eaa93f4e76,	0x409749ea514eca66 // cosh(128/16), sinh(128/16)
				.quad	0x4098ca6d59a6c18c,	0x4098ca6d070a345e // cosh(129/16), sinh(129/16)
				.quad	0x409a63bc8857eedb,	0x409a63bc3abcb51f // cosh(130/16), sinh(130/16)
				.quad	0x409c1771a69e9936,	0x409c17715db7113a // cosh(131/16), sinh(131/16)
				.quad	0x409de7408de954fc,	0x409de740496c9126 // cosh(132/16), sinh(132/16)
				.quad	0x409fd4f933c74a08,	0x409fd4f8f370c7de // cosh(133/16), sinh(133/16)
				.quad	0x40a0f144bd0236f5,	0x40a0f1449ec9e8c6 // cosh(134/16), sinh(134/16)
				.quad	0x40a208ff8e5a3cb1,	0x40a208ff71f6a696 // cosh(135/16), sinh(135/16)
				.quad	0x40a332c4e00d669f,	0x40a332c4c56222a3 // cosh(136/16), sinh(136/16)
				.quad	0x40a46fbe903ead25,	0x40a46fbe77310dcc // cosh(137/16), sinh(137/16)
				.quad	0x40a5c129b30946fa,	0x40a5c1299b803c8e // cosh(138/16), sinh(138/16)
				.quad	0x40a72857cfaf3193,	0x40a72857b9933114 // cosh(139/16), sinh(139/16)
				.quad	0x40a8a6b0323c94ae,	0x40a8a6b01d778043 // cosh(140/16), sinh(140/16)
				.quad	0x40aa3db152f1c077,	0x40aa3db13f6ed151 // cosh(141/16), sinh(141/16)
				.quad	0x40abeef254db4e46,	0x40abeef24286ffd5 // cosh(142/16), sinh(142/16)
				.quad	0x40adbc249d171bee,	0x40adbc248bdf185d // cosh(143/16), sinh(143/16)
				.quad	0x40afa715845d8894,	0x40afa7157430966f // cosh(144/16), sinh(144/16)
				.quad	0x40b0d8d812405031,	0x40b0d8d80aa748b0 // cosh(145/16), sinh(145/16)
				.quad	0x40b1eeffa1d76e64,	0x40b1eeff9ab43ec9 // cosh(146/16), sinh(146/16)
				.quad	0x40b31717afb27271,	0x40b31717a8fdf6f5 // cosh(147/16), sinh(147/16)
				.quad	0x40b452486c8cb5c2,	0x40b452486640395b // cosh(148/16), sinh(148/16)
				.quad	0x40b5a1cd2368027d,	0x40b5a1cd1d7d3828 // cosh(149/16), sinh(149/16)
				.quad	0x40b706f574f1dc72,	0x40b706f56f62d8e6 // cosh(150/16), sinh(150/16)
				.quad	0x40b88326a740279b,	0x40b88326a2075b5e // cosh(151/16), sinh(151/16)
				.quad	0x40ba17dd0b3504d5,	0x40ba17dd064d36ac // cosh(152/16), sinh(152/16)
				.quad	0x40bbc6ad78ef6678,	0x40bbc6ad7453ae2b // cosh(153/16), sinh(153/16)
				.quad	0x40bd9146e4c4ed16,	0x40bd9146e070ae8b // cosh(154/16), sinh(154/16)
				.quad	0x40bf79740e5a27ff,	0x40bf79740a490e9c // cosh(155/16), sinh(155/16)
				.quad	0x40c0c08ea5c43ade,	0x40c0c08ea3db3808 // cosh(156/16), sinh(156/16)
				.quad	0x40c1d525386e484e,	0x40c1d52536a2e627 // cosh(157/16), sinh(157/16)
				.quad	0x40c2fb926ccb3747,	0x40c2fb926b1baa48 // cosh(158/16), sinh(158/16)
				.quad	0x40c434fcc8994fcd,	0x40c434fcc703e844 // cosh(159/16), sinh(159/16)
				.quad	0x40c5829dd053712d,	0x40c5829dced69992 // cosh(160/16), sinh(160/16)
				.quad	0x40c6e5c340cfae1d,	0x40c6e5c33f69e977 // cosh(161/16), sinh(161/16)
				.quad	0x40c85fd05d17f374,	0x40c85fd05bc7dbe2 // cosh(162/16), sinh(162/16)
				.quad	0x40c9f23f51caae03,	0x40c9f23f508ef350 // cosh(163/16), sinh(163/16)
				.quad	0x40cb9ea2af66edc5,	0x40cb9ea2ae3e541e // cosh(164/16), sinh(164/16)
				.quad	0x40cd66a6fcfe721a,	0x40cd66a6fbe7d0cb // cosh(165/16), sinh(165/16)
				.quad	0x40cf4c1464e0729e,	0x40cf4c1463dab2ef // cosh(166/16), sinh(166/16)
				.quad	0x40d0a8683e74f99a,	0x40d0a8683dfa07a7 // cosh(167/16), sinh(167/16)
				.quad	0x40d1bb7016220cc0,	0x40d1bb7015ae8db6 // cosh(168/16), sinh(168/16)
				.quad	0x40d2e034d83b3566,	0x40d2e034d7ceb5bc // cosh(169/16), sinh(169/16)
				.quad	0x40d417db61e917bf,	0x40d417db61832aee // cosh(170/16), sinh(170/16)
				.quad	0x40d5639b73aecacb,	0x40d5639b734f0adb // cosh(171/16), sinh(171/16)
				.quad	0x40d6c4c0e94456f9,	0x40d6c4c0e8ea6424 // cosh(172/16), sinh(172/16)
				.quad	0x40d83cad058e16c7,	0x40d83cad05399712 // cosh(173/16), sinh(173/16)
				.quad	0x40d9ccd7d3fd0c53,	0x40d9ccd7d3adab38 // cosh(174/16), sinh(174/16)
				.quad	0x40db76d1a0b9a9b1,	0x40db76d1a06f17c7 // cosh(175/16), sinh(175/16)

// Shamelessly stolen from Ians expf code.  (sinhf is expf/2 for |x| > 11)
expf_c:			.quad		0x40bc03f30399c376, 0x3ff000000001ea2a // c4/c8, c0
				.quad		0x408f10e7f73e6d8f, 0x3fe62e42fd0933ee // c5/c8, c1  
				.quad		0x405cb616a9384e69, 0x3fcebfbdfd0f0afa // c6/c8, c2 
				.quad		0x4027173ebd288ba1, 0x3fac6b0a74f15403 // c7/c8, c3
				.quad		0x3eb67fe1dc3105ba										 // c8

.literal8
e_over_2:		.quad		0x3ff5bf0a8b145769	// 0x1.5bf0a8b145769p+0
one_over_ln2:	.quad		0x3ff71547652b82fe	// 0x1.71547652b82fep+0
lookup_mask:	.quad		0x7ffff00000000000	// exponent and 8 bits of mantissa
sixteen:		.quad		0x4030000000000000	// 16

.text

#if defined( __x86_64__ )
	#define RELATIVE_ADDR(_a)	(_a)( %rip )
	#define RELATIVE_ADDR_B(_a)	(_a)( %rip )
#else
	#define RELATIVE_ADDR(_a)	(_a)-sinhf_body( %ecx )
	#define RELATIVE_ADDR_B(_a)	(_a)-sinhf_body_eleven( %ecx )
 
.align (1<<4)
sinhf_pic:
	movl		(%esp),			%ecx		// Copy address of this instruction to %ecx
	ret
#endif

ENTRY(sinhf)
#if defined(__i386__)
	movl	FRAME_SIZE(STACKP),	%eax
	movss	FRAME_SIZE(STACKP),	%xmm0
#else
	movd		%xmm0,			%eax
#endif
	andl		$0x7fffffff,	%eax		// |x|
	movd		%eax,			%xmm1
	cmpl		$0x41300000,	%eax		// Ours goes up to 11
	jge			2f							// Jump if |x| >= 11f
	
#if defined( __i386__ ) 
	calll		sinhf_pic
sinhf_body:
#endif
	
// |x| < 11, table-based reduction
	xorpd		%xmm6,			%xmm6		// zero out xmm6
	cvtss2sd	%xmm1,			%xmm6		// xmm6 <-- (double)|x|
	andnps		%xmm0,			%xmm1		// xmm1 <-- signbit(x)
	movapd		%xmm6,			%xmm0		// xmm0 <-- |x|

	movsd	RELATIVE_ADDR(sixteen),	%xmm3	// xmm3 <-- 16
	addsd		%xmm3,			%xmm6		// xmm6 <-- 16 + |x| (this puts our two 4-bit lookup values in the first two
	movsd	RELATIVE_ADDR(lookup_mask), %xmm4	//                mantissa bits of xmm6)
	andpd		%xmm4,			%xmm6		// xmm6 <-- 16 + h
	subsd		%xmm6,			%xmm3		// xmm3 <-- -h
	addsd		%xmm3,			%xmm0		// xmm0 <-- l        (|x| = h + l)
	
	lea			RELATIVE_ADDR(sinhf_table), DX_P
	
	movapd		-48(DX_P),		%xmm3		// xmm3 <-- { c5, c4 }
#if defined( __SSE3__ )
	movddup		%xmm0,			%xmm2       // xmm2 <-- { l , l }
#else
	movapd		%xmm0,			%xmm2
	unpcklpd	%xmm2,			%xmm2       // xmm2 <-- { l , l }
#endif 
	addpd		-16(DX_P),		%xmm0		// xmm0 <-- { l , 1 }
	mulpd		%xmm0,			%xmm3		// xmm3 <-- { c5l, c4 }
	mulpd		%xmm2,			%xmm2		// xmm2 <-- { l2 , l2 }
	
		psrlq	$40,			%xmm6		// shift the bits of h into position for a lookup
		movd	%xmm6,			%eax
		andl	$0x00000ff0,	%eax		// mask the bits of h
		movapd	(DX_P,AX_P,1),	%xmm5		// xmm5 <-- { sinh h, cosh h }
	
	mulpd		%xmm2,			%xmm3		// xmm3 <-- { c5l3, c4l2 }
	addpd		-32(DX_P),		%xmm2		// xmm2 <-- { c3/c5 + l2, c2/c4 + l2 }
	mulpd		%xmm3,			%xmm2		// xmm2 <-- { c3l3 + c5l5, c2l2 + c4l2 }
	addpd		%xmm2,			%xmm0		// xmm0 <-- { l + c3l3 + c5l5, 1 + c2l2 + c4l2 } = { sinh l, cosh l }
	
	mulpd		%xmm5,			%xmm0		// xmm0 <-- { cosh h sinh l, sinh h cosh l }
#if defined(__SSE3__)
	haddpd		%xmm0,			%xmm0
#else
	movhlps		%xmm0,			%xmm2
	addsd		%xmm2,			%xmm0
#endif
	
	cvtsd2ss	%xmm0,			%xmm0
	orps		%xmm1,			%xmm0		// use the stored signbit.
#if defined(__i386__)
	movss		%xmm0,			FRAME_SIZE( STACKP )
	flds		FRAME_SIZE( STACKP )
#endif
	ret

2: // |x| >= 11,
	cmpl		$0x42b2d4fc,	%eax		// Compare to overflow threshold
	ja			4f							// Jump if |x| > overflow threshold || isnan(x)

#if defined( __i386__ ) 
	calll		sinhf_pic
sinhf_body_eleven:
#endif
	cvtss2sd	%xmm1,			%xmm2		// xmm2 <-- (double)|x|
	andnps		%xmm0,			%xmm1		// xmm1 <-- signbit(x)
	
	// xmm1 holds sign(x), xmm2 holds (double)|x|
	mulsd	RELATIVE_ADDR_B(one_over_ln2), %xmm2
	subsd	RELATIVE_ADDR_B(one_over_ln2), %xmm2	// xmm2 <-- (|x| - 1) / ln(2)
	movl		$1023,			%edx		// double precision bias
	
	// extract fractional part
	cvttsd2si	%xmm2,			%eax
	addl		%eax,			%edx		// edx <-- biased exponent of result
	cvtsi2sd	%eax,			%xmm0
	movd		%edx,			%xmm7
	psllq		$52,			%xmm7		// xmm7 <-- exponent of result
	subsd		%xmm0,			%xmm2		// fractional part of scaled |x| - 1
	
	lea			RELATIVE_ADDR_B(expf_c),	DX_P
	mulsd		RELATIVE_ADDR_B(e_over_2),	%xmm7	
	
// c0 + c1x1 + c2x2 + c3x3 + c4x4 + c5x5 + c6x6 + c7x7 + c8x8 
#if defined( __SSE3__ )
	movddup		%xmm2,			%xmm0       // { x, x }
#else
	movapd		%xmm2,			%xmm0       // x
	unpcklpd	%xmm0,			%xmm0       // { x, x }
#endif    
	mulsd		%xmm2,			%xmm2       // x*x
	movapd		%xmm0,			%xmm3
	mulpd		48(DX_P),		%xmm0       // { c3x, (c7/c8)x }
	mulpd		16(DX_P),		%xmm3       // { c1x, (c5/c8)x }
#if defined( __SSE3__ )
	movddup		%xmm2,			%xmm4       // { xx, xx }
#else
	movapd		%xmm2,			%xmm4       // xx
	unpcklpd	%xmm4,			%xmm4       // { xx, xx }
#endif
	mulsd		%xmm2,			%xmm2       // xx*xx
	addpd		32(DX_P),		%xmm0       // { c2 + c3x, (c6/c8) + (c7/c8)x }
	addpd		(DX_P),			%xmm3       // { c0 + c1x, (c4/c8) + (c5/c8)x }
	mulpd		%xmm4,			%xmm0       // { c2xx + c3xxx, (c6/c8)xx + (c7/c8)xxx }
	addsd		%xmm2,			%xmm3       // { c0 + c1x, (c4/c8) + (c5/c8)x + xxxx }
	mulsd		64(DX_P),		%xmm2       // c8 * xxxx
	addpd		%xmm0,			%xmm3       // { c0 + c1x + c2xx + c3xxx, (c4/c8) + (c5/c8)x + (c6/c8)xx + (c7/c8)xxx + xxxx }
	movhlps		%xmm3,			%xmm6       // { ?, c0 + c1x + c2xx + c3xxx }
	mulsd		%xmm2,			%xmm3       // { ..., c8xxxx* ((c4/c8) + (c5/c8)x + (c6/c8)xx + (c7/c8)xxx + xxxx) }
	addsd		%xmm6,			%xmm3       // c0 + c1x + c2xx + c3xxx + c4xxxx + c5xxxxx + c6xxxxxx + c7xxxxxxx + c8xxxxxxxxx
	mulsd		%xmm7,			%xmm3       // 2**i * {c0 + c1x + c2xx + c3xxx + c4xxxx + c5xxxxx + c6xxxxxx + c7xxxxxxx + c8xxxxxxxxx}
	
	cvtsd2ss	%xmm3,			%xmm0
	orps		%xmm1,			%xmm0		// restore sign bit
#if defined(__i386__)
	movss		%xmm0,			FRAME_SIZE( STACKP )
	flds		FRAME_SIZE( STACKP )
#endif
	ret

4: // |x| > overflow threshold || isnan(x)
	cmpl		$0x7f800000,	%eax
	jge			5f
	
	// overflow case
	movl		$0x7f7fffff,	%ecx
	movd		%ecx,			%xmm1
	mulss		%xmm1,			%xmm0		// xmm0 <-- x * hugevalf
#if defined(__i386__)
	movss		%xmm0,			FRAME_SIZE( STACKP )
	flds		FRAME_SIZE( STACKP )
#endif
	ret
 
5: // x = inf, nan
	addss		%xmm0,			%xmm0
#if defined(__i386__)
	movss		%xmm0,			FRAME_SIZE( STACKP )
	flds		FRAME_SIZE( STACKP )
#endif
	ret
