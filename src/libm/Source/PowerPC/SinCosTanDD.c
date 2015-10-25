/*
 * Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
//
//	SinCosTanDD.c
//
//	Double-double Function Library
//	Copyright: © 1995-96 by Apple Computer, Inc., all rights reserved
//	
//	long double sinl( long double x );
//	long double cosl( long double x );
//	long double tanl( long double x );
//
//	Change History:			[7d]
//
//		06/20/96 PAF - Remove /* */ comments (for color editor)

#include "math.h"
#include "fp_private.h"
#if (__WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L)
#include "DD.h"

static const float	k1_8To52	=  6755399441055744.0f;			// 0x1.8p52 
static const double	kPiBy2		=  1.570796326794896619231322;	// 0x1.921FB54442D18p0
static const double	kPiBy2Tail1	=  6.1232339957367660e-17;		// 0x1.1A62633145C07p-54
static const double	kPiBy2Tail2	= -1.4973849048591698e-33;		// 0x1.F1976B7ED8FBCp-110
static const double	kPiBy2Tail3	=  5.5622711043168264e-50;		// 0x1.4CF98E804177Dp-164
static const double	k2ByPi		=  0.636619772367581382;		// 0x1.45f306dc9c883p-1
static const float	k2To52		=  4503599627370496.0f;			// 0x1.0p+52
static const float	k2To54		=  18014398509481984.0f;			// 0x1.0p+54
static const double	k2To45m1	=  35184372088831.0;			// 0x1.fffffffffffffp44

static const uint32_t CosCoeff[] __attribute__ ((aligned(8))) = {
	0xbFE00000,		0x00000000,		0x39685556,		0x00000000,
	0x3FA55555,		0x55555555,		0x3C455555,		0x51E8C72E,
	0xbF56C16C,		0x16C16C17,		0x3BF02698,		0xA05BBBBD,
	0x3EFA01A0,		0x1A0169EB,		0x3B540FCD,		0xE3AE1BE0,
	0xbE927E4D,		0x648A174F,		0xbB361705,		0xD6F2C25B
};
	 
static const uint32_t SinCoeff[] __attribute__ ((aligned(8))) = {
     0xbFC55555,	0x55555555, 	0xbC655555,		0x55555552,
     0x3F811111,	0x11111111, 	0x3C011111,		0x0DBB7C30,
     0xbF2A01A0,	0x1A01A01A, 	0xbB510055,		0x358E4100,
     0x3EC71DE3,	0xA556A5C2, 	0x3B61A976,		0xF8644324,
     0xbE5AE642,	0x52F2B067, 	0xbAD356A3,		0xF6278F6C
};

static const uint32_t CosCoeff16[] __attribute__ ((aligned(8))) = {
     0xBFE00000,	0x00000000, 	0x395DA200,		0x00000000,
     0x3FA55555,	0x55555555, 	0x3C455555,		0x554FC86B,
     0xBF56C16C,	0x16C16C17, 	0x3BEF49FE,		0x788C8E18,
     0x3EFA01A0,	0x1A01A01A, 	0xBB6A022F,		0xCA316020,
     0xBE927E4F,	0xB7789A12, 	0xBB245398,		0xA829E8F5,
     0x3E21EED8,	0xEE008FB4, 	0x3AC49E38,		0xCA6157DE,
     0xBDA938BA,	0xA889516E, 	0x3A46CBC1,		0x4AF6E840
};

static const uint32_t SinCoeff16[] __attribute__ ((aligned(8))) = {
     0xBFC55555,	0x55555555, 	0xBC655555,		0x55555554,
     0x3F811111,	0x11111111, 	0x3C011111,		0x10F8EB9C,
     0xBF2A01A0,	0x1A01A01A, 	0xBB69FF8C,		0xFBC69E00,
     0x3EC71DE3,	0xA556C733, 	0x3B6EFC3D,		0xB602439A,
     0xBE5AE645,	0x67F53972, 	0xBAF653B7,		0xA917A91D,
     0x3DE61246,	0x1048C848, 	0x3A7CFCE5,		0x9E3CEA38,
     0xBD6AE6FF,	0xCFCF788E, 	0x39DA11EB,		0x64C11650
};

static const uint32_t TrigTable[] __attribute__ ((aligned(8))) = {
	0x3FB80000,	0x00054E6C, 								// angle 	
	0x3FB7F701,	0x032A9959, 	0xBBAD11F0,	0x5495F5BE,		// Sin Head, Sin Tail
	0x3FEFDC06,	0xBF7E5BB6, 	0xBBDCB529,	0x1917F935,		// Cos Head, Cos Tail
	0x3FBC0000,	0x000A5F0A, 
	0x3FBBF1B7,	0x8572884A, 	0x3BBAADFC,	0xF7E52ACA,
	0x3FEFCF0C,	0x800E7577, 	0xBBE4670B,	0xB43517A0,
	0x3FC00000,	0x000F1278, 
	0x3FBFEAAE,	0xEEA4D6F0, 	0xBB9D8153,	0x7343A887,
	0x3FEFC015,	0x527CE390, 	0x3BE16998,	0xA0C1E22D,
	0x3FC20000,	0x00052790, 	
	0x3FC1F0D3,	0xD7B4E938, 	0xBBB94CF6,	0x02B38414,
	0x3FEFAF22,	0x263C1D96, 	0xBBDE4F5F,	0x508C4D7E,
	0x3FC40000,	0x000143F6, 
	0x3FC3EB31,	0x2C5EA6CF, 	0x3BBF0AD1,	0xD6EF035D,
	0x3FEF9C34,	0x0A7CB78E, 	0xBBE87189,	0x89E0691A,
	0x3FC60000,	0x0007E6FF, 
	0x3FC5E44F,	0xCFA8F023, 	0xBBAF148F,	0x61A85B8E,
	0x3FEF874C,	0x2E1E9676, 	0x3BC38F0F,	0xAB6185B5,
	0x3FC80000,	0x000E77F8, 	
	0x3FC7DC10,	0x2FC929C2, 	0xBBC3C05A,	0x114DA314,
	0x3FEF706B,	0xDF9E2180, 	0xBBD8BEA1,	0x93F3F8C7,
	0x3FCA0000,	0x00102F36, 
	0x3FC9D252,	0xD0DE9D1A, 	0x3B8BF04E,	0xADF4B6FE,
	0x3FEF5794,	0x8CFE96A3, 	0xBBE58FFE,	0x7BB60718,
	0x3FCC0000,	0x0009CE78, 	
	0x3FCBC6F8,	0x4EE5F43E, 	0xBBA7E35C,	0xC27E82D6,
	0x3FEF3CC7,	0xC3B3493B, 	0xBBCC7CA2,	0xE7033121,
	0x3FCE0000,	0x00150434, 
	0x3FCDB9E1,	0x5FCA16EB, 	0x3BC107C0,	0xD41B55C5,
	0x3FEF2007,	0x30852C41, 	0x3BE70655,	0x75B2D6FB,
	0x3FD00000,	0x0061B811, 
	0x3FCFAAEE,	0xD5B07238, 	0xBBCB82BA,	0x01ECEAD1,
	0x3FEF0154,	0x9F71D818, 	0x3BE95043,	0xCC0B6426,
	0x3FD10000,	0x0020111B, 	
	0x3FD0CD00,	0xCF125569, 	0xBBC2D015,	0xDA2CFEAE,
	0x3FEEE0B1,	0xFBBCBB9D, 	0xBBE9AA1F,	0xF00E2374,
	0x3FD20000,	0x000C1AD3, 	
	0x3FD1C37D,	0x64D25988, 	0x3BC4B236,	0xE14BCAA2,
	0x3FEEBE21,	0x4F75419B, 	0x3BE52928,	0x2D338F16,
	0x3FD30000,	0x000E7F89, 		
	0x3FD2B8DD,	0xC44C91CC, 	0xBBC5707F,	0x432D5073,
	0x3FEE99A4,	0xC3A5AEA3, 	0x3BD23919,	0x2EA440E3,
	0x3FD40000,	0x0017A801, 	
	0x3FD3AD12,	0x9780568B, 	0xBBC59FD1,	0x16DF09C5,
	0x3FEE733E,	0xA0159A53, 	0xBBB12BF5,	0xDB2DCDCC,
	0x3FD50000,	0x0039F7ED, 	
	0x3FD4A00C,	0x9B461D51, 	0xBBCDFE0E,	0x1445AAA9,
	0x3FEE4AF1,	0x4B20ED63, 	0x3BD374F6,	0x9F86063A,
	0x3FD60000,	0x0000A1AA, 	
	0x3FD591BC,	0x9FA38DCC, 	0xBBCD07DB,	0x418C1954,
	0x3FEE20BF,	0x49ACBB83, 	0x3B864FA4,	0xEB634A4E,
	0x3FD70000,	0x00011BC1, 
	0x3FD68213,	0x8A39E197, 	0xBBBA6968,	0x3C12FB6D,
	0x3FEDF4AB,	0x3EBD5578, 	0xBBD6766E,	0x746C48E7,
	0x3FD80000,	0x0008ED50, 	
	0x3FD77102,	0x557E9094, 	0x3BB488F1,	0x4A8DF658,
	0x3FEDC6B7,	0xEB97B68C, 	0x3BDCA325,	0xE2462D16,
	0x3FD90000,	0x00861E6C, 
	0x3FD85E7A,	0x12FE6D57, 	0x3BA65076,	0x5FCB7841,
	0x3FED96E8,	0x2F58212A, 	0xBBEC65EC,	0x2CEB0E13,
	0x3FDA0000,	0x00021040, 
	0x3FD94A6B,	0xE9F72C06, 	0x3BA04A4B,	0xE0FC13D6,
	0x3FED653F,	0x073DD7E0, 	0x3BEC1A4A,	0xF1A7037E,
	0x3FDB0000,	0x005EF712, 	
	0x3FDA34C9,	0x1D1BB055, 	0x3BC42083,	0xD43A5519,
	0x3FED31BF,	0x8D7A0AAD, 	0xBB9D5F11,	0x5DAECA9B,
	0x3FDC0000,	0x000C075C, 
	0x3FDB1D83,	0x053CFB6A, 	0xBBDA72E9,	0x9A1C2FBE,
	0x3FECFC6C,	0xFA50214C, 	0xBBCB5164,	0xBC69A105,
	0x3FDD0000,	0x0056E33E, 
	0x3FDC048B,	0x17FF5F2B, 	0x3BD8AF58,	0xF8525AD2,
	0x3FECC54A,	0xA29F9263, 	0x3BD22EA3,	0xDBB55D0D,
	0x3FDE0000,	0x005CCC54,
	0x3FDCE9D2,	0xE4276EF1, 	0xBBAAEDF9,	0xDD5DC899,
	0x3FEC8C5B,	0xF8B9244D, 	0xBBC2D9C6,	0xCE4671F4,
	0x3FDF0000,	0x00161118, 
	0x3FDDCD4C,	0x154623DC, 	0xBBD97F5B,	0x8DF7FEBF,
	0x3FEC51A4,	0x8B85F41A, 	0x3BC354E8,	0x86ABC9CE,
	0x3FE00000,	0x0013AF20, 
	0x3FDEAEE8,	0x746D926F, 	0x3BD2B7DF,	0xA4EEC7DD,
	0x3FEC1528,	0x06520D6D, 	0x3BE9EE87,	0x2601172E,
	0x3FE08000,	0x0046DF51, 
	0x3FDF8E99,	0xE7E60D68, 	0x3BAF1859,	0x1DD2BAB4,
	0x3FEBD6EA,	0x30DFA2E0, 	0x3BE95328,	0x8BAB7ACB,
	0x3FE10000,	0x0002BED1, 
	0x3FE03629,	0x39C8F748, 	0x3BCFAF90,	0x9F47E8F6,
	0x3FEB96EE,	0xEF572000, 	0x3BD89C91,	0x79738697,
	0x3FE18000,	0x00404634, 
	0x3FE0A402,	0x1ED4F66C, 	0x3BD4E9FD,	0x2527C4F7,
	0x3FEB553A,	0x40EAA3C8, 	0xBBE9F185,	0x9F0B73D8,
	0x3FE20000,	0x0005ABAA, 
	0x3FE110D0,	0xC4BB683B, 	0xBBB78FD4,	0x4D302311,
	0x3FEB11D0,	0x415F9E99, 	0x3BDAE453,	0xDBCB3704,
	0x3FE28000,	0x002D02D5, 	
	0x3FE17C8E,	0x5F549FEE, 	0x3B6368B3,	0xE72D341D,
	0x3FEACCB5,	0x26DE0531, 	0xBB8CE182,	0xC27CD7B8,
	0x3FE30000,	0x0002B1A8, 
	0x3FE1E734,	0x323892EB, 	0x3BC9A07C,	0xF3B89184,
	0x3FEA85ED,	0x43725E55, 	0x3BE2C483,	0xBE536F66,
	0x3FE38000,	0x000730F9, 
	0x3FE250BB,	0x937E7157, 	0xBBC3FA7C,	0xADF21ED4,
	0x3FEA3D7D,	0x034EA01E, 	0xBBD5786E,	0x86ABB722,
	0x3FE40000,	0x00076EEF, 
	0x3FE2B91D,	0xEA8E4953, 	0x3BE1CB0F,	0x13421234,
	0x3FE9F368,	0xED8CD61E, 	0xBBC0A358,	0x32653793,
	0x3FE48000,	0x00119ECA, 
	0x3FE32054,	0xB156DCB6, 	0xBBBF1C5F,	0x95EB1A26,
	0x3FE9A7B5,	0xA35FDCFF, 	0x3BE6C143,	0x88B5E209,
	0x3FE50000,	0x000045B6, 
	0x3FE38659,	0x74565F66, 	0x3BD5144E,	0x65AA2BBD,
	0x3FE95A67,	0xE00C8774, 	0x3BC1A72A,	0xE0D050C3,
	0x3FE58000,	0x00A4E7B8, 	
	0x3FE3EB25,	0xD3EDE59C, 	0x3BE1F420,	0x5560DD7C,
	0x3FE90B84,	0x77E73599, 	0x3B9250E3,	0x5B17FE76,
	0x3FE60000,	0x000049F4, 	
	0x3FE44EB3,	0x81CF7192, 	0xBBD5B364,	0x7D4D895F,
	0x3FE8BB10,	0x5A5D9A12, 	0xBBD2BB98,	0xF4F2C91C,
	0x3FE68000,	0x0008C197, 	
	0x3FE4B0FC,	0x46B16552, 	0x3BC863B5,	0xB735A540,
	0x3FE86910,	0x8D71FD5A, 	0x3BE5C4E3,	0x402BA3EE,
	0x3FE70000,	0x00340EE5,
	0x3FE511F9,	0xFDA26352,		0x3BCDC369, 0x16E2A8DB,
	0x3FE8158A,	0x316F2658, 	0x3BD69C38, 0x4E2E4085,
	0x3FE78000,	0x005756FD, 
	0x3FE571A6,	0x96AE2DA7, 	0x3BE24CA0,	0xA2E35E02,
	0x3FE7C082,	0x7ECF5E07, 	0xBBCBD891,	0x2A83004E,
	0x3FE80000,	0x004601DC, 
	0x3FE5CFFC,	0x16F2C847, 	0x3BCA6614,	0x963A9B5C,
	0x3FE769FE,	0xC62568E3, 	0x3BE746CD,	0xBABDB543,
	0x3FE88000,	0x002EF948, 
	0x3FE62CF4,	0x99438A17, 	0x3BA3ED99,	0x63704218,
	0x3FE71204,	0x6F86E919, 	0xBBE0671D,	0x4098C629,
	0x3FE90000,	0x0022F620, 
	0x3FE6888A,	0x4E2C1E13, 	0xBBC0D332,	0x374F8A22,
	0x3FE6B898,	0xFA865CFA, 	0xBBDE0573,	0x1D87548D,
	0x3FE98000,	0x00883EB0, 
	0x3FE6E2B7,	0x7C9FF82A, 	0x3BE241BC,	0xA153A6CC,
	0x3FE65DC1,	0xFD8A1C59, 	0xBBDBAD2A,	0x038DFB75
};

static const uint32_t TanCoeff16[] __attribute__ ((aligned(8))) = {
     0x3FD55555,	0x55555555,   0x3C755555,	0x5555554C,
     0x3FC11111,	0x11111111,   0x3C411111,	0x11327FD4,
     0x3FABA1BA,	0x1BA1BA1C,   0xBC47917B,	0x8730ECE8,
     0x3F9664F4,	0x882C10FA,   0xBC08946C,	0x04337BC0,
     0x3F8226E3,	0x55E6C239,   0xBC16F3F8,	0x5EB82A9C,
     0x3F6D6D3D,	0x0E159BD4,   0xBC0BF02E,	0x91A39520,
     0x3F57DA36,	0x44E2BF4A,   0x3BF4CC81,	0xFDF36B54,
     0x3F435582,	0xB4E9E8D9,   0xBBEAAB03,	0xE323ED60,
     0x3F2F5712,	0xDCD1E82C,   0x3BA67F16,	0xB4DC7710,
     0x3F19C9D2,	0x46D3C681,   0x3BA2B1F2,	0xBC709028
};

static const uint32_t CotCoeff16[] __attribute__ ((aligned(8))) = {
     0x3FD55555,	0x55555555,   0x3C755555,	0x55555550,
     0x3F96C16C,	0x16C16C17,   0xBC2F49F4,	0x9F46AA0E,
     0x3F61566A,	0xBC011567,   0xBC050FFB,	0x0703E386,
     0x3F2BBD77,	0x9334EF0B,   0xBBC541A3,	0x5FEBE630,
     0x3EF66A8F,	0x2BF70EDF,   0xBB960EEE,	0x58853E10,
     0x3EC22805,	0xD642B849,   0x3B49A324,	0x92C33F58,
     0x3E8D6DB2,	0xD58EAF82,   0x3B103D9B,	0xCE4CCBB8,
     0x3E57DA1C,	0x0FF6D02F,   0x3AFBDB54,	0xE9F15B6C,
     0x3E2396A6,	0x6CC1B8E2,   0x3AC04FB7,	0x0AEC86D2
};

static const uint32_t TanCoeff[] __attribute__ ((aligned(8))) = {
     0x3ff00000,	0x00000000,   0x00000000,	0x00000000,
     0x3FD55555,	0x55555555,   0x3C755555,	0x55555549,
     0x3FC11111,	0x11111111,   0x3C411111,	0x17771B40,
     0x3FABA1BA,	0x1BA1BA1C,   0xBC47A471,	0x53F3999C,
     0x3F9664F4,	0x882C119E,   0x3C2A931E,	0x61CFDED8,
     0x3F8226E3,	0x5546FD80,   0xBC22700A,	0xDC5E76C2,
     0x3F6D6DCB,	0xC6BCC1FD,   0x3C0EB3DD,	0x1EBC7C6C
};

static const uint32_t TanTable[] __attribute__ ((aligned(8))) = {
	0x3FB80000,	0x00095B76, 
	0x3FB81210,	0x420B0DDD,   0xBBADBC24,	0xD03EE61B,
	0x40254552,	0xEE62D182,   0xBC23A778,	0xD4D4C82D,
	0x3FBC0000,	0x00F6D8DC,   
	0x3FBC1CB8,	0x85A84FC9,   0xBBA0734F,	0xD6E646D7,
	0x40223676,	0x163ABE75,   0x3C19C932,	0xFC451F09,
	0x3FC00000,	0x002177FC,   
	0x3FC01577,	0xAF3710E9,   0xBBBB1D68,	0x25554EB9,
	0x401FD549,	0xF004A93B,   0x3BF1B140,	0x750F85AD,
	0x3FC20000,	0x0000C2F1,   
	0x3FC21E9E,	0x0175E475,   0x3BBD91B3,	0xA31924A6,
	0x401C41B6,	0xE169DC8D,   0x3C057A49,	0x2ABEE9CF,
	0x3FC40000,	0x004D703E,   
	0x3FC42A13,	0xDFCB15A0,   0xBB94ED3B,	0x3620DA54,
	0x4019642D,	0xFDBA3F8F,   0x3BBB47D8,	0x80CDA3C6,
	0x3FC60000,	0x000CAC24,   
	0x3FC6381F,	0x200F2AED,   0x3BBF3967,	0x0381AC58,
	0x40170B09,	0x205DE76C,   0xBBFEFF3A,	0x14B71AB8,
	0x3FC80000,	0x00473296,   
	0x3FC84906,	0xF15CE818,   0x3BB03E11,	0x36E0F2D3,
	0x4015152E,	0xCDA71D37,   0xBBE77628,	0xDB83D025,
	0x3FCA0000,	0x00B2D7A8,   
	0x3FCA5D14,	0x0081E4D7,   0xBBC0188C,	0x9C6AB2DE,
	0x40136BB4,	0xBA046E82,   0xBC0F80BA,	0xA34DE4A8,
	0x3FCC0000,	0x0066CDE2,   
	0x3FCC7490,	0xA23DC3B4,   0xBBCB362F,	0xA69FE4DA,
	0x4011FE3C,	0xA58F0345,   0xBBFCB31C,	0x2F2152B4,
	0x3FCE0000,	0x00252978,   
	0x3FCE8FC9,	0x01177F3A,   0xBBA5E8BA,	0x10809B66,
	0x4010C0C5,	0xABFB3524,   0xBBFA678B,	0xCA50DEEC,
	0x3FD00000,	0x00638D21,   
	0x3FD05785,	0xA4A65715,   0x3BC29365,	0x7FDA2145,
	0x400F549E,	0x3203FCA2,   0x3BE2971C,	0x11C2A8A3,
	0x3FD10000,	0x002C35A8,   
	0x3FD16953,	0xEACF2DA7,   0x3BC40A91,	0xBA0FE60F,
	0x400D67EC,	0xF327EFBB,   0x3C05D1EC,	0x4B5FCFCB,
	0x3FD20000,	0x001C3AB2,   
	0x3FD27D78,	0xB42A0CE9,   0x3BCCED21, 	0x016F79E8,
	0x400BB0C1,	0xF1319BD2,   0xBC051F68,	0x2CBAE56E,
	0x3FD30000,	0x000F9C12,   
	0x3FD3941E,	0xADA8C533,   0xBBBB6B83,	0x08722CFA,
	0x400A26A8,	0xA4D1FD5D,   0xBBF11692,	0xFACCF74A,
	0x3FD40000,	0x000B6423,   
	0x3FD4AD71,	0xED5E62C2,   0xBBAD5590,	0x3A1538DA,
	0x4008C2DD,	0x5F133FBD,   0xBBDDCA83,	0x1B096361,
	0x3FD50000,	0x0017ECB5,   
	0x3FD5C9A0,	0x105DB3D0,   0x3BCE9A3C,	0xD13B3D17,
	0x40077FE6,	0x3A7531D9,   0x3BE036AE,	0x48AA7EC4,
	0x3FD60000,	0x00785229,   
	0x3FD6E8D8,	0x5AEC50EC,   0xBBC83B79,	0x6A9E4244,
	0x40065948,	0x26E6F5E4,   0xBBFE36C3,	0x52F1803B,
	0x3FD70000,	0x001C8263,   
	0x3FD80B4B,	0xD8D44659,   0xBBB499F2,	0xDC83C2D8,
	0x40054B4F,	0x852854B9,   0x3BFB7522,	0xCB8BEDBE,
	0x3FD80000,	0x0092A4EC,   
	0x3FD9312D,	0x86455645,   0x3B93DC76,	0x9413BABA,
	0x400452E6,	0x95D5E9C0,   0xBBDBF037,	0x35429FA9,
	0x3FD90000,	0x00353578,   
	0x3FDA5AB2,	0x70323F27,   0xBBD5E11A,	0x4F5FE60A,
	0x40036D75,	0xEB84FD4A,   0xBBE3A384,	0xFD52C107,
	0x3FDA0000,	0x002177A0,   
	0x3FDB8811,	0xE4F7B2DE,   0x3BC5B70A,	0x7D15AD3F,
	0x400298CC,	0x1A74BDDF,   0x3BD41900,	0x83E8A76D,
	0x3FDB0000,	0x0071F1F6,   
	0x3FDCB985,	0x9CFB26A0,   0x3BC8BA6E,	0xF094701B,
	0x4001D30A,	0xD7B1694B,   0xBBFECEAB,	0x555C2BC9,
	0x3FDC0000,	0x0071BD74,   
	0x3FDDEF49,	0xEB35D726,   0xBBCCC6A5,	0xDDB5912C,
	0x40011A98,	0x2075779C,   0xBBF95852,	0x2DBF4C5B,
	0x3FDD0000,	0x00A18446,   
	0x3FDF299D,	0xF3CB9E1D,   0xBBDAB47B,	0x6F4E4DBE,
	0x40006E12,	0x72BBF45D,   0x3BE9A288,	0x2C8CD3D9,
	0x3FDE0000,	0x0009F5E0,   
	0x3FE03461,	0xF090AA3D,   0xBBB6411F,	0x2E55E33D,
	0x3FFF988E,	0xC8064412,   0xBBE51DE0,	0x7EC2ADBC,
	0x3FDF0000,	0x00263EAE,   
	0x3FE0D680,	0x92D62104,   0x3BDD6620,	0x8142D974,
	0x3FFE6858,	0x0C88DF44,   0xBBDD8335,	0xD4838950,
	0x3FE00000,	0x002FD5FE,   
	0x3FE17B4F,	0x5C31640E,   0xBBCF7FD9,	0x4387A68E,
	0x3FFD49AD,	0x7DDFB168,   0x3BF71CBF,	0x13808D41,
	0x3FE08000,	0x00760E88,   
	0x3FE222F4,	0xAFFFC590,   0x3BDD3CE8,	0x22236C45,
	0x3FFC3AF4,	0x70DCC003,   0xBBFBC120,	0xA2F987AC,
	0x3FE10000,	0x000F8806,   
	0x3FE2CD98,	0xFEB5905C,   0x3BE169C8,	0x3A579A2C,
	0x3FFB3AC2,	0x7605E88C,   0xBBC11C38,	0xB96450D3,
	0x3FE18000,	0x002B5E5D,   
	0x3FE37B66,	0xF43D000E,   0x3BD2830E,	0xBAC9646C,
	0x3FFA47D6,	0x6F7CCB9D,   0xBBEB41B3,	0x171B37E1,
	0x3FE20000,	0x000A3C10,   
	0x3FE42C8B,	0xA0F7A0E3,   0x3BE38DF3,	0x66A12B85,
	0x3FF96112,	0xDACBFCBC,   0xBBEFEF64,	0xEF1EA7D1,
	0x3FE28000,	0x00595422,   
	0x3FE4E136,	0xB0CFA783,   0xBBBA398C,	0xEE061521,
	0x3FF88578,	0xFAA0A3DE,   0x3BDA1845,	0x4AE391EC,
	0x3FE30000,	0x0026966D,   
	0x3FE5999A,	0x9E477C73,   0xBBCE4AC5,	0x97E84259,
	0x3FF7B424,	0xCEF83049,   0xBBF35B98,	0x9FE75083,
	0x3FE38000,	0x0015B3FD,   
	0x3FE655EC,	0xF397B5BE,   0xBBE51A33,	0x112B4404,
	0x3FF6EC49,	0xA1E2DBE5,   0x3BB86EE4,	0x93E45292,
	0x3FE40000,	0x00146EE3,   
	0x3FE71666,	0x89F330B5,   0xBBE35378,	0x5A4867EE,
	0x3FF62D2F,	0x215A3E9B,   0x3BEA0C9E,	0xF65CC2CF,
	0x3FE48000,	0x000F229A,   
	0x3FE7DB43,	0xD3A2EEDF,   0xBBE0A454,	0xC31E28C0,
	0x3FF5762E,	0xE1176AE6,   0xBBE1C3BD,	0x493469EA,
	0x3FE50000,	0x002E680F,   
	0x3FE8A4C5,	0x2CF1484A,   0xBB9432B2,	0x063946DC,
	0x3FF4C6B2,	0x384B4991,   0x3BD8E9D0,	0xF949DFE6,
	0x3FE58000,	0x00672096,   
	0x3FE9732F,	0x3459A187,   0x3BDDADD7,	0x84F3E87C,
	0x3FF41E30,	0x6C72DBD4,   0xBBEC67AE,	0xAE1EBB1F,
	0x3FE60000,	0x00B6C23F,   
	0x3FEA46CB,	0x2D189DAF,   0x3BB739F4,	0x75A8271C,
	0x3FF37C2D,	0x1B3BE6EB,   0xBBC85203,	0x8CECB4C0,
	0x3FE68000,	0x0028A725,   
	0x3FEB1FE7,	0x6A3CF1FF,   0xBBEA7D29,	0x7EE6B9D1,
	0x3FF2E036,	0xDC04F43A,   0x3BED69F5,	0xAF37688F,
	0x3FE70000,	0x004CA480,   
	0x3FEBFED7,	0xCD2DB8E2,   0xBBCD123B,	0x0537EE2D,
	0x3FF249E6,	0x09D6492E,   0xBBD38032,	0x80A8D415,
	0x3FE78000,	0x000FFDA8,   
	0x3FECE3F6,	0x42FE6158,   0x3BD569F8,	0x793799F7,
	0x3FF1B8DB,	0xBED5846E,   0x3BD9797E,	0x39EAD48F,
	0x3FE80000,	0x001E1B0A,   
	0x3FEDCFA3,	0x61492AAF,   0xBBDA3DEC,	0xB61278D5,
	0x3FF12CC0,	0xE58AA7DC,   0x3BB3088E,	0x1168567F,
	0x3FE88000,	0x000B208A,   
	0x3FEEC247,	0x07D4CEEF,   0xBBE3FF76,	0x1F807DFF,
	0x3FF0A545,	0x6F3C80B0,   0xBBCF3891,	0xE44E8437,
	0x3FE90000,	0x000657F8,   
	0x3FEFBC51,	0x1E0226B6,   0xBBEC6FF5,	0x5F02EDF6,
	0x3FF0221F,	0x9DADF743,   0xBBD42807,	0xFD02BF0B,
	0x3FE98000,	0x00655AE2,   
	0x3FF05F1D,	0x31752EE7,   0xBBD8BE76,	0xB0E002E9,
	0x3FEF4616,	0xC9018897,   0xBBDB4E55,	0xDEAF57E4
};
 
#warning  These probably ought to static or typedefs below. They are being exported.

struct TrigTblEntry { 
   double arg;
   double sinHead;
   double sinTail;
   double cosHead;
   double cosTail;
} TrigTblEntry;

struct CosCoeffEntry { 
   double Head;				
   double Tail;
} CosCoeffEntry;

struct SinCoeffEntry { 
   double Head;				
   double Tail;
} SinCoeffEntry;

struct CosCoeff16Entry { 
   double Head;				
   double Tail;
} CosCoeff16Entry;

struct SinCoeff16Entry { 
   double Head;				
   double Tail;
} SinCoeff16Entry;

struct TanTblEntry { 
   double arg;
   double TanHead;
   double TanTail;
   double CotHead;
   double CotTail;
} TanTblEntry;

struct TanCoeffEntry { 
   double Head;				
   double Tail;
} TanCoeffEntry;

struct CotCoeff16Entry { 
   double Head;				
   double Tail;
} CotCoeff16Entry;

struct TanCoeff16Entry { 
   double Head;				
   double Tail;
} TanCoeff16Entry;	  

static void argReduce( double xHead, double xTail, double *dLeft, int *M, double *redHead, double *redTail )   
{
	double amid, alow, carry, high, q, q1;
	double b, blow, b1, c, clow, c1, c2, c3, c4;
	double d, dlow, d1, d2, d3, d4, d5, e1, e2, e3;
	Double DeN;
	amid = 0.0;
	alow = 0.0;
	
	while (fabs(xHead) > k2To45m1)    		// reduce until argument
	{											// isn't larger than k2To45m1
		q = xHead*k2ByPi; 		     
	   	if (fabs(q) < k2To54) {	     		// maybe q(mod 4) != 0
			if (fabs(q) < k2To52) {     		// maybe q isn't even an integer
				if (q > 0)
					q = __FADD( q, k2To52 ) - k2To52;	// remove fraction if any exists
				else
					q = __FSUB( q, k2To52 ) + k2To52;
	      	}			     					// at this point, q is an integer
			q1 = q*0.25; 		     			// and determine q(mod 4)
			if (q > 0)
				q1 = __FADD( q1, k2To52 ) - k2To52;		// giving the quadrant in which the angle resides.
			else
				q1 = __FSUB( q1, k2To52 ) + k2To52;		// right adjust quadrant
			DeN.f = __FNMSUB( 4.0, q1, q ) + k1_8To52; // q - 4.0*q1 + k1_8To52;		     
	      
			*M = *M + DeN.i[1];
	   	}

		//****************************************************************************
		//   Tackling the subtraction of q*(pi/2) from the octuple precision word:   *
		//		      (xHead, xTail, amid, alow)			   						 *
		//****************************************************************************
	   
		high = __FNMSUB( q, kPiBy2, xHead );	// xHead - q*kPiBy2;		     	// exact (Spectacular use of MAF)
		b = q*kPiBy2Tail1;
		blow = __FNMSUB( q, kPiBy2Tail1, b );	// b - q*kPiBy2Tail1;		     	// Yet another use of MAF
		c = q*kPiBy2Tail2;
		clow = __FNMSUB( q, kPiBy2Tail2, c );	// c - q*kPiBy2Tail2;
		d = q*kPiBy2Tail3;
		dlow = __FNMSUB( q, kPiBy2Tail3, d );	// d - q*kPiBy2Tail3;
		b1 = __FADD( xTail, high );
		if (fabs(xTail) > fabs(high)) 		// sum and propagate carries to the right
			c1 = xTail - b1 + high;
		else
			c1 = high - b1 + xTail;
		xHead = __FSUB( b1, b );	
		if (fabs(b1) > fabs(b))
			c2 = b1 - xHead - b + c1;
		else
			c2 = b1 - (xHead+b) + c1;
		c3 = __FADD( amid,  blow);
		if (fabs(amid) > fabs(blow))
			d1 = amid - c3 + blow;
		else
			d1 = blow - c3 + amid;
		c4 = __FSUB( c2, c );
		if (fabs(c2) > fabs(c))
			d2 = c2 - c4 - c + d1;
		else
			d2 = c2 - __FADD( c4, c ) + d1;
		xTail = __FADD( c3, c4 );
		if (fabs(c3) > fabs(c4))
			d3 = c3 - xTail + c4 + d2;
		else
			d3 = c4 - xTail + c3 + d2;
		d4 = __FADD( alow, clow );
		if (fabs(alow) > fabs(clow))
			e1 = alow - d4 + clow;
		else
			e1 = clow - d4 + alow;
		d5 = __FSUB( d3, d );
		if (fabs(d3) > fabs(d))
			e2 = d3 - d5 - d + e1;
		else
			e2 = d3 - (d5+d) + e1;
		amid = __FADD( d4, d5 );
		if (fabs(d4) > fabs(d5))
			e3 = d4 - amid + d5 + e2;
		else
			e3 = d5 - amid + d4 + e2;
		alow = e3 + dlow;
	}
	
	high  = __FADD( xHead, xTail );		 // At long last,
	carry = xHead - high + xTail;		 // A final distillation
	
	xTail  = __FADD( carry, __FADD( amid, alow ) );
	xHead  = high;
	*dLeft = __FSUB( carry, xTail ) + __FADD( amid, alow );
		
	*redHead = xHead;
	*redTail = xTail;
}
	
static void SinCosCore(int M, double xHead, double xTail, double dleft, double *yHead, double *yTail )
{
	double intquo;
	double temp, temps, tempc; 
	double b, c, ca, d; 
	double redarg, redarg1, res, reslow, resmid, resbot, restop, resin, resextra;
	double ressup, restiny, resinf; 
	double p1, p2, p3, p4, p5, p6, p6a,  p7, p8, p9, p10;
	double p11, p12, p13, p14, p15, p16, p17, p18, p19, p20;
	double p21, p22, p23, p23a, p23b, p24, p25, p26, p27, p27a, p28, p29, p30, p31;
	double arg, arg2, arg2a, argsq, argsq2, argmid, argnew, argres;  
	double sin, sin2, sint, sinl, sinargtiny;
	double sintab, sintab2, arg2b;
	double cos, cos2, cosl, cost;
	double prods, prodsl, prodc, prodcl;
	double blow, carry, cerr;
	int i, index;
	Double DeN;
	
	struct TrigTblEntry *TrigPtr = (struct TrigTblEntry *)TrigTable - 6;
	struct CosCoeffEntry *CosPtr = (struct CosCoeffEntry *) CosCoeff;
	struct SinCoeffEntry *SinPtr = (struct SinCoeffEntry *) SinCoeff;
	struct CosCoeff16Entry *Cos16Ptr = (struct CosCoeff16Entry *) CosCoeff16;
	struct SinCoeff16Entry *Sin16Ptr = (struct SinCoeff16Entry *) SinCoeff16;

	DeN.f = __FMADD( xHead, k2ByPi, k1_8To52 );		// xHead*k2ByPi + k1_8To52;				// # of multiples of pi/2 to remove
       	
	intquo = DeN.f - k1_8To52;						// Integer
    redarg = __FNMSUB( intquo, kPiBy2, xHead );		// xHead - intquo*kPiBy2;				// Exact result
	   
	M = M + DeN.i[1];								// M=quadrant in which to comp. sin
       
	b = kPiBy2Tail1*intquo;							// continue argument reduction
	blow = __FMSUB( kPiBy2Tail1, intquo, b );		// kPiBy2Tail1*intquo - b;
	redarg1 = __FSUB( xTail, b );															// second word of reduced argument
	DeN.f = __FMADD( fabs(redarg), 64.0, k1_8To52 );// fabs(redarg)*64.0 + k1_8To52;		// index into acctbl
	index = DeN.i[1];

	ca = kPiBy2Tail2*intquo;

	if (fabs(b) > fabs(xTail))
		carry = xTail - (b+redarg1);    			// tail-b may be inexact
	else
		carry = (xTail-redarg1) - b;
	   
	if (! (index < 6)) {
		if (redarg > 0.0) { 
			redarg = redarg - TrigPtr[index].arg ;	// arg-intquo ln2-tbl(index) ->
			sintab = TrigPtr[index].sinHead ;		// fetch sine of table value
			sintab2 = TrigPtr[index].sinTail ;
		} 
		else {										// much trailing zeroes in redarg
			redarg = redarg + TrigPtr[index].arg;
			sintab =- TrigPtr[index].sinHead ;		// fetch sine of table value
			sintab2 =- TrigPtr[index].sinTail ;
		}
	}
	   
	arg = __FADD( redarg, redarg1 );				// get signif. bits from the right
	arg2a = (redarg - arg + redarg1);
	arg2b = __FSUB( arg2a, blow );             		// fill in with bits from 2nd term
	c = __FSUB( carry, ca );                   		// 3rd word of reduced argument

	argmid = __FADD( arg2b , c );
	argnew = __FADD( arg, argmid );
	argres = arg - argnew + argmid;
	arg = argnew;
	cerr = (carry-c) - ca;

	if (fabs(arg2b) > fabs(c))
		d = arg2b - argmid + c + dleft;
	else
		d = c - argmid + arg2b + dleft;

	// d = d + cerr + (ca-kPiBy2Tail2*intquo) - kPiBy2Tail3*intquo; // 4th word
	d = __FNMSUB( kPiBy2Tail3, intquo, d + cerr + __FNMSUB( kPiBy2Tail2, intquo, ca ) );
	arg2 = __FADD( argres, d );								// reduced arg is (arg, arg2)
	temp = 2.0*arg*arg2;
	argsq = __FMADD( arg, arg, temp );				// arg*arg + temp;
	argsq2 = __FMADD( arg2, arg2, __FMSUB( arg, arg, argsq ) + temp ); // arg*arg - argsq + temp + arg2*arg2;
       
	   
	// Now we have the reduced arg = (arg, arg2)  
	   
	if (index < 6) {								// straight power series
          
		if ((M & 1) == 0) { 						// sin series evaluation
			sin = Sin16Ptr[6].Head ;				// need extra terms when
			sinl = Sin16Ptr[6].Tail ;				// used without table lookup
			sinargtiny = argres - arg2 + d;

			for(i = 5; i > -1; i--) {
				temps = __FMADD( sin, argsq2, sinl*argsq ); // sin*argsq2 + sinl*argsq;
				prods = __FMADD( sin, argsq, temps );		// sin*argsq + temps;
				prodsl = __FMSUB( sin, argsq, prods ) + temps; // sin*argsq - prods + temps;
				sin = __FADD( Sin16Ptr[i].Head, prods );
				sin2 = Sin16Ptr[i].Tail + prodsl;
				sinl = Sin16Ptr[i].Head - sin+prods + sin2;
			}
		} 
		else {                     					// cos series evaluation
			cos = Cos16Ptr[6].Head;
			cosl = Cos16Ptr[6].Tail ;
             
			for(i = 5; i > -1; i--) {
				temps = __FMADD( cos, argsq2, cosl*argsq ); // cos*argsq2 + cosl*argsq;
				prods = __FMADD( cos, argsq, temps );		// cos*argsq + temps;
				prodsl = __FMSUB( cos, argsq, prods) + temps; // cos*argsq - prods + temps;
				cos = __FADD( Cos16Ptr[i].Head, prods );
				cos2 = Cos16Ptr[i].Tail + prodsl;
				cosl = Cos16Ptr[i].Head - cos + prods + cos2;
			}
		}
	} 
	else {											// shorter series in case

		cos = CosPtr[4].Head; 						// where exact table is used.
		sin = SinPtr[4].Head;
		cosl = CosPtr[4].Tail;
		sinl = SinPtr[4].Tail;
		for (i = 3; i > -1; i--) {					// add short series
			tempc = __FMADD( cos, argsq2, cosl*argsq ); // cos*argsq2 + cosl*argsq;
			prodc = __FMADD( cos, argsq, tempc );	   // cos*argsq + tempc;
			prodcl = __FMSUB( cos, argsq, prodc) + tempc; // cos*argsq - prodc + tempc;
			cos = __FADD( CosPtr[i].Head, prodc );
			cos2 = CosPtr[i].Tail + prodcl;
			cosl = CosPtr[i].Head - cos + prodc + cos2;
			temps = __FMADD( sin, argsq2, sinl*argsq ); // sin*argsq2 + sinl*argsq;
			prods = __FMADD( sin, argsq, temps );		// sin*argsq + temps;
			prodsl = __FMSUB( sin, argsq, prods ) + temps; // sin*argsq - prods + temps;
			sin = __FADD( SinPtr[i].Head, prods );
			sin2 = SinPtr[i].Tail + prodsl;
			sinl = SinPtr[i].Head - sin + prods + sin2;
		}
	}
	   
	tempc = __FMADD( cos, argsq2, cosl*argsq );		// cos*argsq2 + cosl*argsq;
	cost = __FMADD( cos, argsq, tempc );			// cos*argsq + tempc;
	cosl = __FMSUB( cos, argsq, cost ) + tempc;		// cos*argsq - cost + tempc;
	cos = cost;                          			// cos(arg) - 1.0 complete
	temps = __FMADD( sin, argsq2, sinl*argsq );		// sin*argsq2 + sinl*argsq;
	sint = __FMADD( sin, argsq, temps );			// sin*argsq + temps;
	sinl = __FMSUB( sin, argsq, sint ) + temps;		// sin*argsq - sint + temps;
	temps = __FMADD( sint, arg2, sinl * arg );		// sint*arg2 + sinl*arg;
	sin = __FMADD( sint, arg, temps );				// sint*arg + temps;
	sinl = __FMSUB( sint, arg, sin ) + temps;		// sint*arg - sin + temps;         			// sin(arg) - arg complete
	   
	if (index < 6) {
		if ((M & 1) == 0) {
			res = __FADD( arg, sin );               // sin of a small angle
			reslow = __FADD( arg2, sinl );          // careful distallation
			resmid = __FADD( arg - res, sin );
			resbot = arg2 - reslow + sinl;
			restop = __FADD( res, (reslow+resmid) );
			resin = __FADD( res - restop, (reslow+resmid) );
			resextra = resmid - (reslow+resmid) + reslow;
			ressup = __FADD( restop, (resin+resbot) );
			restiny = __FSUB( resin, (resin+resbot) + resbot );
			resinf = restop - ressup + (resin+resbot) + (resextra+restiny+sinargtiny);
			*yHead = ressup;
			*yTail = resinf;
		} 
		else {
			res = __FADD( 1.0, cos );               			// cos of a small angle
			reslow = cosl;
			resmid = __FADD( 1.0 - res, cos );
			restop = __FADD( res, (reslow+resmid) );
			resin = __FADD( res - restop, (reslow+resmid) );
			resextra = resmid - (reslow+resmid) + reslow;
			ressup = __FADD( restop, resin );
			resinf = restop - ressup + resin + resextra;
			*yHead = ressup;
			*yTail = resinf;
		}
	}    // end of if (index < 6)
	else {
		if ((M & 1) == 0) {        					// even quadrant-- eval. sine(x)
			p1 = cos*sintab;																// o(-8)  exact
			p2 = __FMSUB( cos, sintab, p1 );		// cos*sintab - p1;          			// o(-62) exact
			p3 = __FMADD( cosl, sintab, cos*sintab2 ); //cosl*sintab + (cos*sintab2); 		// o(-67)
			p4 = arg* TrigPtr[index].cosHead ;												// o(-4)  exact
			p5 = __FMSUB(arg, TrigPtr[index].cosHead, p4 ); // arg* TrigPtr[index].cosHead -p4;	// o(-58) exact
			p6 = arg2* TrigPtr[index].cosHead ;												// o(-58)
			p6a = __FMSUB( arg2, TrigPtr[index].cosHead, p6 ); // arg2* TrigPtr[index].cosHead -p6;
			p7 = sin* TrigPtr[index].cosHead ;												// o(-4)  exact
			p8 = __FMSUB( sin, TrigPtr[index].cosHead, p7 ); // sin* TrigPtr[index].cosHead -p7; // o(-59) exact
			p9 = __FMADD( sinl, TrigPtr[index].cosHead, (sin* TrigPtr[index].cosTail ) ); //sinl* TrigPtr[index].cosHead +(sin* TrigPtr[index].cosTail );
			p10 = __FMADD( arg, TrigPtr[index].cosTail, p9 + p8); // arg* TrigPtr[index].cosTail +p9+p8;  // o(-59)
			p11 = __FADD( p2, p5 );															// o(-58) exact
			p12 = p5 - p11 + p2 + p6a;														// o(-112) exact
			p13 = __FADD( p6, p11 );														// o(-57) exact
			p15 = __FADD( sintab, p4 );														// o(0)   exact
			p16 = sintab - p15 + p4;														// o(-54) exact
			p17 = sintab2 + p10 + p12 + p3;													// o(-58)
			p18 = __FADD( p15, p1 );														// o(0) exact
			p20 = __FADD( p18, p7 );														// o(0) exact
			if (fabs(p6) > fabs(p11))
				p14 = p6 - p13 + p11;														// o(-111)
			else
				p14 = p11 - p13 + p6;
			p19 = p15 - p18 + p1 + p16;														// o(-53) exact
			p21 = p18 - p20 + p7;															// o(-54) exact
			p22 = p13 + p17;																// o(-56)
			p23 = __FADD( p19, p21 );														// o(-52) exact
			p25 = __FADD( p20, p23 );														// o(0) exact
			p26 = p20 - p25 + p23;															// o(-54) exact
			if (fabs(p19) > fabs(p21))
				p24 = p19 - p23 + p21;														// o(-108)
			else
				p24 = p21 - p23 + p19;
			p27 = __FADD( (p14+p24), p22 + p26 );											// o(-54)
			p27a = p26 - p27 + ((p14+p24)+p22);
			p28 = __FADD( p25, p27 );
	
			*yHead = p28;
			*yTail = p25 - p28 + p27 + p27a;
		}
		// end of if ((M & 1) == 0) 
		else {         								// evaluation of cos series.
			p1 = TrigPtr[index].cosHead *cos;													// o(-16) exact
			p2 = __FMSUB( TrigPtr[index].cosHead, cos, p1 ); // TrigPtr[index].cosHead *cos-p1;	// o(-71) exact
			p5 = sintab*arg;																	// o(-8) exact
			p6 = __FMSUB( sintab, arg, p5 );		// sintab*arg - p5 ;						// o(-62) exact
			p7 = sintab*arg2;																	// o(-62)
			p8 = sintab*sin;																	// o(-24) exact
			p9 = __FMSUB( sintab, sin, p8 );		// sintab*sin - p8 ;						// o(-78) exact
			p12 = __FMSUB( TrigPtr[index].cosTail, cos, (sintab2*arg) ); // TrigPtr[index].cosTail *cos - (sintab2*arg); // o(-80) 
			p13 = __FNMSUB( sintab2, sin, p12 );	// p12 - (sintab2*sin);						// o(-72)
			p14 = p13 + p2;																		// o(-70)
			p15 = __FMADD( TrigPtr[index].cosHead, cosl, p14 ); // p14 + TrigPtr[index].cosHead *cosl;	// o(-69)
			p16 = __FNMSUB( sintab, sinl, p15 - p9 );// p15 - p9 - sintab*sinl;					// o(-67)
			p17 = __FADD( TrigPtr[index].cosHead, p1 );											// o(-1) exact
			p18 = TrigPtr[index].cosHead - p17 + p1;											// o(-55) exact-16 bits
			p19 = __FSUB( p17, p5 );															// o(-1) exact
			p20 = p17 - p19 - p5;																// o(-55) exact-8 bits
			p21 = p18 + p20 ;																	// o(-54) exact-17 bits
			p22 = __FSUB( p19, p8 );															// o(-1) exact
			p23 = p19 - p22 - p8;																// o(-53) exact-25 bits
			p24 = TrigPtr[index].cosTail + p16;													// o(-65)
			p23a = __FADD( p23, p21 );															// o(-54)
			p25 = __FSUB( p23a, p6 );															// o(-53) exact
			p26 = p23a - p25 - p6;																// o(-106)
			p27 = __FSUB( p25, p7 );															// o(-53) exact
			if (fabs(p23) > fabs(p21))
				p23b = p23 - p23a + p21;														// o(-106)
			else
				p23b = p21 - p23a + p23;
			p28 = p25 - p27 - p7 + p26 + p23b;													// o(-105)
			p29 = __FADD( p27, p24 );															// o(-53) exact
			p30 = p27 - p29 + p24 + p28;														// o(-104)
			p31 = __FADD( p22, p29 );															// o(-1) exact
	
			*yHead = p31;
			*yTail = p22 - p31 + p29 + p30;
		}   //end of else if ((M & 1) == 0) 

	}	//end of else if (index < 6)
       
	   
	if((M & 2) != 0) {								// change sign for below x axis.
		*yHead = - *yHead;
		*yTail = - *yTail;
	}
}

long double sinl( long double x )
{
	uint32_t hibits;
	double xHead, xTail, dleft, fenv, yHead, yTail;
	DblDbl ddx;
	int M;

        FEGETENVD(fenv);
        FESETENVD(0.0);
	
	ddx.f = x;
	
	xHead = ddx.d[0];
	xTail = ddx.d[1];

	hibits = ddx.i[0] & 0x7FFFFFFFu;				// highest 32 bits as uint32_t

	// NaNs and infinities
	
	if (hibits >= 0x7ff00000u) {					// special cases: NaN and infinity
		if (xHead != xHead) {						// x is a NaN
			FESETENVD(fenv);
			return x;
		}
		else {
			ddx.d[0] = xHead - xHead;				// inf - inf gives NaN
			ddx.d[1] = ddx.d[0];
			FESETENVD(fenv);
			return (ddx.f);
		}
	}

	// x = zero hence xHead is zero
	if ((hibits | ddx.i[1]) == 0) {
                FESETENVD(fenv);
		return x;
	}
		
	// x is not infinity, NaN, or zero
    
	//for tiny x, sin x = x
	if (hibits < 0x29900000u) { 					// 2^(-358);  358 = (1022 + 52 )/3
                FESETENVD(fenv);
		return x;
	}
		
	M = 0;	// M = 0 for Sine, M = 1 for Cosine
	dleft = 0.0;
	if (hibits >= 0x42d00000u) {					// |x| > = 2 ^ 46
		argReduce(xHead, xTail, &dleft, &M, &xHead, &xTail);
	}   
	
	//  Call core algorithm
	SinCosCore( M, xHead, xTail, dleft, &yHead, &yTail );
	
	ddx.d[0] = yHead + yTail; 						// Cannonize result
	ddx.d[1] = (yHead - ddx.d[0]) + yTail; 
	
        FESETENVD(fenv);
	return (ddx.f);
}

long double cosl( long double x )
{
	uint32_t hibits;
	double xHead, xTail, dleft, fenv, yHead, yTail;
	DblDbl ddx;
	int M;

        FEGETENVD(fenv);
        FESETENVD(0.0);
	
	ddx.f = x;
	
	xHead = ddx.d[0];
	xTail = ddx.d[1];

	hibits = ddx.i[0] & 0x7FFFFFFFu;				// highest 32 bits as uint32_t

	// NaNs and infinities
	
	if (hibits >= 0x7ff00000u) {					// special cases: NaN and infinity
		if (xHead != xHead) {						// x is a NaN
			FESETENVD(fenv);
			return x;
		}
		else {
			ddx.d[0] = xHead - xHead;				// inf - inf gives NaN
			ddx.d[1] = ddx.d[0];
			FESETENVD(fenv);
			return (ddx.f);
		}
	}

	// x = zero hence xHead is zero
	if ((hibits | ddx.i[1]) == 0) {
                FESETENVD(fenv);
		return (long double) 1.0;
	}
		
	// x is not infinity, NaN, or zero
    
	//for tiny x, cos x = 1.0
	if (hibits < 0x1e600000u) { 					// 2^(-537);  537 = (1022 + 52 )/2
                FESETENVD(fenv);
		return 1.0;
	}
		
	M = 1;	// M = 0 for Sine, M = 1 for Cosine
	dleft = 0.0;
	if (hibits >= 0x42d00000u) {					// |x| > = 2 ^ 46
		argReduce(xHead, xTail, &dleft, &M, &xHead, &xTail);
	}   
	
	//  Call core algorithm
	SinCosCore( M, xHead, xTail, dleft, &yHead, &yTail );
	
	ddx.d[0] = __FADD( yHead, yTail ); 						// Cannonize result
	ddx.d[1] = (yHead - ddx.d[0]) + yTail; 
	
        FESETENVD(fenv);
	return (ddx.f);
}

long double tanl( long double x )
{
    double temp, temps; 
	double b, c, c1, c2,  ca, cerr, d; 
	double frac, frac2, frac3, t1, t2;
	double rec, res, reslow, resmid, resbot, restop, resin, resextra;
	double ressup, restiny, resinf; 
	double rarg, rarg0, rarg2, rarg3, rarg2a, redarg, redarg1, redargold;
	double p1, p2, p3, p4, p5, p6,  p7, p8, p9, p10;
	double p11, p12, p13, p14, p15, p16, p17, p18, p19, p20;
	double p21, p22, p23, p24, p25, p26, p27, p28, p29, p30;
	double p31, p32, p33, p34, p35, p36, p37, p38, p39, p40;
	double p41, p42, p43, p44, p45, p46;
	double arg, arg2, arg2a, arg2b, argsq, argsq2, argres, argmid, argnew;  
	double cot, cot2, cotl, cott,  tan, tan2, tant, tanl;
	double cotargtiny, tanargtiny; 
	double prods, prodsl, q;
	double blow, carry, dleft;
	int i, index, M;
	
	uint32_t hibits;
    Double DeN;
	double fenv;

	double xHead, xTail;
	DblDbl ddx;

	struct TanTblEntry *TanTablePtr = (struct TanTblEntry *)TanTable - 6;  
	struct TanCoeffEntry *TanPtr = (struct TanCoeffEntry *) TanCoeff - 1;  
	struct TanCoeff16Entry *Tan16Ptr = (struct TanCoeff16Entry *) TanCoeff16 - 1;  
	struct CotCoeff16Entry *Cot16Ptr = (struct CotCoeff16Entry *) CotCoeff16 - 1;  

        FEGETENVD(fenv);
        FESETENVD(0.0);

	ddx.f = x;
	
	xHead = ddx.d[0];
	xTail = ddx.d[1];
	
	hibits = ddx.i[0] & 0x7FFFFFFFu; 				// highest 32 bits as uint32_t
	
	if (hibits >= 0x7ff00000u){					// special cases: NaN and infinity
		if (xHead != xHead) {						// x is a NaN
			FESETENVD(fenv);
			return x;
		}
		else {
			ddx.d[0] = xHead - xHead;				// inf - inf gives NaN & raises invalid
			ddx.d[1] = ddx.d[0];
			FESETENVD(fenv);
			return (ddx.f);
		}
	}
		
	// x is not infinity nor a NaN
    
	// x = zero
	if ((hibits | ddx.i[1]) == 0) {
                FESETENVD(fenv);
		return x;
	}


	// finite x 

	//for tiny x, tan x = x
	if (hibits < 0x29900000u) {					// 2^(-358);  358 = (1022 + 52 )/3
                FESETENVD(fenv);
		return x;
	}

	// finite result but not tiny

	M=0;											// determine quadrant
	dleft=0.0;

    if (hibits >= 0x42b00000u){					// |x| > = 2 ^ 44
		argReduce (xHead, xTail, &dleft, &M, &xHead, &xTail);
	}   
	
	// now finite result < 2 ^ 44	

	DeN.f = __FMADD( xHead, k2ByPi, k1_8To52 );		// xHead*k2ByPi + k1_8To52;				// # of multiples of pi/2 to remove
	q = DeN.f - k1_8To52;							// to remove integer
    redarg = __FNMSUB( q, kPiBy2, xHead );			// xHead - intquo*kPiBy2;				// Exact result
	
	M = M + DeN.i[1];								// M=quadrant in which to compute sin
	b = kPiBy2Tail1*q;								// more argument reduction
	blow = __FMSUB( kPiBy2Tail1, q, b );			// kPiBy2Tail1*q - b;
	redarg1 = __FSUB( xTail, b );															// second word of reduced argument
	DeN.f = __FMADD( fabs(redarg), 64.0, k1_8To52 );// fabs(redarg)*64.0 + k1_8To52;		// index into acctbl
	ca = kPiBy2Tail2*q;
	
	if (fabs(b) > fabs(xTail))
		carry = xTail - (b+redarg1);				// tail-b may be inexact
	else
		carry = (xTail-redarg1) - b;
	
	redargold = redarg;
	index = DeN.i[1];
	  
	if (!(index < 6)) {
		if (redargold > 0.0)
			redarg = redarg - TanTablePtr[index].arg;	// arg-q ln2-tbl(index) ->
														// mucho trailing zeroes in redarg
		else
			redarg = redarg + TanTablePtr[index].arg;
	}
	
	arg = __FADD( redarg, redarg1 );				// get signif. bits from the right
	arg2a = (redarg - arg + redarg1);
	arg2b = __FSUB( arg2a, blow );             		// fill in with bits from 2nd term
	c = __FSUB( carry, ca );                   		// 3rd word of reduced argument

	argmid = __FADD( arg2b , c );
	argnew = __FADD( arg, argmid );
	argres = arg - argnew + argmid;
	arg = argnew;
	cerr = (carry-c) - ca;
	
	if (fabs(arg2b) > fabs(c))
		d = arg2b - argmid + c + dleft;
	else
		d = c - argmid + arg2b + dleft;
	
	// d = d + cerr + (ca-kPiBy2Tail2*q) - kPiBy2Tail3*q; // 4th word
	d = __FNMSUB( kPiBy2Tail3, q, d + cerr + __FNMSUB( kPiBy2Tail2, q, ca ) );
	arg2 = __FADD( argres, d );								// reduced arg is (arg, arg2)
	temp = 2.0*arg*arg2;
	argsq = __FMADD( arg, arg, temp );				// arg*arg + temp;
	argsq2 = __FMADD( arg2, arg2, __FMSUB( arg, arg, argsq ) + temp ); // arg*arg - argsq + temp + arg2*arg2;

	if (index < 6) {								// straight power series
		if ((M & 1) == 0) { 						// 2n multiple of pi/2 -> use TAN
			tan = Tan16Ptr[10].Head;				// need extra terms when
			tanl = Tan16Ptr[10].Tail;				// used without table lookup
			tanargtiny = argres - arg2 + d;
			
			for(i = 9; i > 0; i--) {
				temps = __FMADD( tan, argsq2, tanl*argsq );		// tan*argsq2 + tanl*argsq;
				prods = __FMADD( tan, argsq, temps );			// tan*argsq + temps;
				prodsl = __FMSUB( tan, argsq, prods ) + temps;	// tan*argsq - prods + temps;
				tan = __FADD( Tan16Ptr[i].Head, prods );
				tan2 = Tan16Ptr[i].Tail + prodsl;
				tanl = Tan16Ptr[i].Head - tan + prods + tan2;
			}
			temps = __FMADD( tan, argsq2, tanl*argsq );			// tan*argsq2 + tanl*argsq;
			tant = __FMADD( tan, argsq, temps );				// tan*argsq + temps;
			tanl = __FMSUB( tan, argsq, tant ) + temps;			// tan*argsq - tant + temps;
			temps = __FMADD( tant, arg2, tanl*arg );			// tant*arg2 + tanl*arg;
			tan = __FMADD( tant, arg, temps );					// tant*arg + temps;
			tanl = __FMSUB( tant, arg, tan ) + temps;			// tant*arg - tan + temps;			// tan(arg)-arg complete
			res = __FADD( arg, tan );																// tan of a small angle
			reslow = __FADD( arg2, tanl );															// distill VERY CAREFULLY
			resmid = arg - res + tan;
			resbot = arg2 - reslow + tanl;
			restop = __FADD( res, (reslow+resmid) );
			resin = res - restop + (reslow+resmid);
			resextra = resmid - (reslow+resmid) + reslow;
			ressup = __FADD( restop, (resin+resbot) );
			restiny = resin - (resin+resbot) + resbot;
			resinf = restop - ressup + (resin+resbot) + ( resextra+restiny+tanargtiny);
			
			//getting a cannonical result
			
			ddx.d[0] = __FADD( ressup, resinf ); 			
			ddx.d[1] = (ressup - ddx.d[0]) + resinf; 
			
			FESETENVD(fenv);
			return (ddx.f);
		}
		else {										// near an 2k+1 multiple of pi/2
													// tan (x)=-cot(x+(2k+1) pi/2)
			cot = Cot16Ptr[9].Head;					// need extra terms when
			cotl = Cot16Ptr[9].Tail;				// used without table lookup
			cotargtiny = argres - arg2 + d;
			rarg0 = 1.0/arg;						// We use the cotangent
			
			for(i = 8; i > 0; i--){ 				// series, which is one term shorter
				temps = __FMADD( cot, argsq2, cotl*argsq );		// cot*argsq2 + cotl*argsq;
				prods = __FMADD( cot, argsq, temps );			// cot*argsq + temps;
				prodsl = __FMSUB( cot, argsq, prods ) + temps;	// cot*argsq - prods + temps;
				cot = __FADD( Cot16Ptr[i].Head, prods );
				cot2 = Cot16Ptr[i].Tail + prodsl;
				cotl = Cot16Ptr[i].Head - cot + prods + cot2;
			}
			cott = cot;
			rarg = rarg0;
			temps = __FMADD( cott, arg2, cotl*arg );		// cott*arg2 + cotl*arg;
			cot = __FMADD( cott, arg, temps );				// cott*arg + temps;
			cotl = __FMSUB( cott, arg, cot ) + temps;		// cott*arg - cot + temps;			// 1/arg-cot(arg) completed
			p1 = __FNMSUB( arg, rarg, 1.0 );				// 1.0 - arg*rarg;					// next complete hi precision
			p2 = arg2*rarg;																		// reciprocal of argument, to subtract
			p3 = __FSUB( p1, p2 );																		// from series.  The reciprocal will
			rarg2a = p3*rarg;																	// be the dominant term.
			rarg2 = rarg2a + __FNMSUB( arg, rarg2a, p3 )*rarg;	// rarg2a + (p3-arg*rarg2a)*rarg
			
			if (fabs(p1) > fabs(p2))
				p4 = p1 - p3 - p2;
			else
				p4 = p1 - (p3+p2);
			
			p5 = p4 + __FNMSUB( arg2, rarg, p2 );			// p4 + (p2-arg2*rarg)
			p6 = __FNMSUB( cotargtiny, rarg, p5 );			// p5 - cotargtiny*rarg;
			p7 = __FNMSUB( rarg2, arg, p3 );				// p3 - rarg2*arg;
			rarg3 = __FNMSUB( arg2, rarg2, p6+p7 )*rarg;	// ((p6+p7) - arg2*rarg2)*rarg;   	// low order recip. bits
			
			//**********************************************************************
			//     BEGIN the subtraction of 1/arg from the series                  *
			//              |cot| much greater than  |rarg|                        *
			//**********************************************************************
			
			p8 = __FSUB( cot, rarg );
			p9 = cot - (rarg+p8);
			p10 = __FSUB( cotl, rarg2 );
			p11 = cotl - (rarg2+p10);
			p12 = p11 - rarg3;
			p13 = __FADD( p9, p10 );
			
			if (fabs(p9) > fabs(p10))
				p14 = p9 - p13 + p10;
			else
				p14 = p10 - p13 + p9;
			
			p15 = p12 + p14;
			p16 = __FADD( p8, p13 );         		// HO term of result
			p17 = p8 - p16 + p13;					// almost LO term
			p18 = p17 + p15;						// assimilate any other LO bits
			
			// getting a cannonical result
			
			ddx.d[0] = __FADD( p16, p18 ); 			
			ddx.d[1] = (p16 - ddx.d[0]) + p18; 
			
			FESETENVD(fenv);
			return (ddx.f);
		}
	}
	else {											// Serious cases
													// Accurate tables will be used.
		tant = TanPtr[7].Head;						// Need fewer terms when
		tant = __FMADD( tant, argsq, TanPtr[6].Head );		// TanPtr[6].Head + tant*argsq;
		tan = __FMADD( tant, argsq, TanPtr[5].Head );		// TanPtr[5].Head + tant*argsq;
		tanl = __FMADD( tant, argsq, TanPtr[5].Head - tan );// TanPtr[5].Head - tan + tant*argsq;
		tanargtiny = argres - arg2 + d;
		
		for(i = 4; i > 0; i--) {
			temps = __FMADD( tan, argsq2, tanl*argsq );		// tan*argsq2 + tanl*argsq;
			prods = __FMADD( tan, argsq, temps );			// tan*argsq + temps;
			prodsl = __FMSUB( tan, argsq, prods ) + temps;	// tan*argsq - prods + temps;
			tan = __FADD( TanPtr[i].Head, prods );
			tan2 = TanPtr[i].Tail + prodsl;
			tanl = TanPtr[i].Head -tan + prods + tan2;
		}
		tant = tan;
		temps = __FMADD( tant, arg2, tanl*arg );			// tant*arg2 + tanl*arg;
		tan = __FMADD( tant, arg, temps );					// tant*arg + temps;
		tanl = __FMSUB( tant, arg, tan ) + temps;			// tant*arg - tan + temps;				// tan(arg) complete
		if (redargold > 0) {																		// read out table values
			t1 = TanTablePtr[index].TanHead;														// (t1,t2) are tan(x)
			t2 = TanTablePtr[index].TanTail;
			c1 = TanTablePtr[index].CotHead;														// (c1,c2) are cot(x)
			c2 = TanTablePtr[index].CotTail;
		}
		else {
			t1 = -TanTablePtr[index].TanHead;
			t2 = -TanTablePtr[index].TanTail;
			c1 = -TanTablePtr[index].CotHead;
			c2 = -TanTablePtr[index].CotTail;
		}
		
		p1 = __FADD( t1, c1 );						// Tan(x)+Cot(x) needed
		p2 = __FADD( t2, c2 );						// for all quadrants
		p3 = c1 - p1 + t1;
		p4 = c2 - p2 + t2;
		p5 = __FADD( p3, p2 );
		p6 = p3 - p5 + p2 + p4;						// p1+p5+p6=Tan(x)+Cot(x) exactly
													// where x = table lookup argument
													// Next, this sum needs to be multiplied
													// by the tangent of the reduced argument
		p7 = p1*tan;
		p8 = __FMSUB( p1, tan, p7 );				// p1*tan - p7;
		p9 = p5*tan;
		p10 = __FMSUB( p5, tan, p9 );				// p5*tan - p9;
		p11 = p1*tanl;
		p12 = __FMSUB( p1, tanl, p11 );				// p1*tanl - p11;
		p13 = __FMADD( p5, tanl, p10 + p12 );		// p10 + p12 + p5*tanl;
		p14 = __FMADD( p6, tan, p13 );				// p13 + p6*tan;
		p15 = __FADD( p8, p9 );
		p17 = __FADD( p15, p11 );
		
		if (fabs(p8) > fabs(p9))
			p16 = p8 - p15 + p9;
		else
			p16 = p9 - p15 + p8;
		
		if (fabs(p11) > fabs(p15))
			p18 = p11 - p17 + p15 + (p14+p16);
		else
			p18 = p15 - p17 + p11 + (p14+p16);
		
		p19 = __FADD( p7, p17 );
		p20 = p7 - p19 + p17;
		p21 = __FADD( p20, p18 );
		p22 = p20 - p21 + p18;
		
		//***********************************************************************
		//         At this point, different paths are taken for arguments       *
		//         "neighborhood" of even and odd multiples of pi/2.            *
		//***********************************************************************
		
		if ((M & 1) == 0) {
			
			//***********************************************************************
			//         Let the argument A be written as A=x+y, where                *
			//         x=TanTablePtr[index].arg.  Then if T=tan(x) and C=cot(x)     *
			//         as read from tantbl, and t=tan(y),  (computed above as       *
			//         (tan+tanl), then compute tan(A) as:                          *
			//                                                                      *
			//                                t ( C+T)                              *
			//                tan(A)= T   +  -----------                            *
			//                                  C-t                                 *
			//                                                                      *
			//         numerator represented as (p19,p21,p22) in the code below.    *
			//                                                                      *
			//***********************************************************************
			p23 = __FSUB( c1, tan );							// High order part of C-t
			p24 = __FSUB( c2, tanl );
			rec = 1.0/p23;
			p25 = c1 - p23 - tan;
			frac = p19*rec;							// start computation of fraction
			
			if (fabs(c2) > fabs(tanl))
				p26 = c2 - p24 - tanl;
			else
				p26 = c2 - (p24+tanl);
			
			p27 = __FADD( p25, p24 );
			p28 = p25 - p27 + p24 + p26;													// C-t=(p23,p27,p28) done
			frac = frac + __FNMSUB( p23, frac, p19 )*rec; // frac + (p19-p23*frac)*rec;		// refine lead fraction wd.
			p29 = __FNMSUB( p23, frac, p19 );			//p19 - p23*frac;
			p30 = frac*p27;																	// p30 a negative term
			p31 = __FNMSUB( frac, p27, p30 ) + p22;		// p30 - frac*p27 + p22;
			p32 = __FNMSUB( frac, p28, p31 );			// p31 - frac*p28;
			p33 = __FSUB( p29, p30 );
			p35 = p33 + p21;
			frac2 = p35*rec;
			frac2 = frac2 + __FNMSUB( frac2, p23, p35)*rec; // frac2 + (p35-frac2*p23)*rec;	// second fraction wd.
			
			if (fabs(p29) > fabs(p30))
				p34 = p29 - p33 - p30 + p32;
			else
				p34 = p29 - (p33+p30) + p32;
			
			p36 = __FNMSUB( frac2, p23, p35 ) + __FNMSUB( frac2, p27, p34 );	// p35 - frac2*p23 + p34 - frac2*p27;
			
			if (fabs(p33) > fabs(p21))
				p37 = p33 - p35 + p21;
			else
				p37 = p21 - p35 + p33;
			
			p38 = p37 + p36;
			frac3 = p38*rec;						// Fraction of above formula done
			
			//***********************************************************************
			//        Finally, add (tan,tanl) and (frac,frac2,frac3) for result     *
			//***********************************************************************
			
			p39 = __FADD( t1, frac );
			p40 = t1 - p39 + frac;
			p41 = __FADD( t2, frac2 );
			
			if (fabs(t2) > fabs(frac2))
				p42 = t2 - p41 + frac2 + frac3;
			else
				p42 = frac2 - p41 + t2 + frac3;
			
			p43 = __FADD( p40, p41 );
			p44 = p40 - p43 + p41 + p42;
			p45 = __FADD( p39, p43 );
			p46 = p39 - p45 + p43 + p44;
			
			// getting a cannonical result
			
			ddx.d[0] = __FADD( p45, p46 ); 			
			ddx.d[1] = (p45 - ddx.d[0]) + p46; 
			
			FESETENVD(fenv);
			return (ddx.f);
		}
		else {
			
			//***********************************************************************
			//      The remaining case is for argments in the neighborhood of       *
			//      even multiples of pi/2.                                         *
			//      Use the identity                                                *
			//                          tan(A+(2k+1)pi/2)=-cot(A)                   *
			//                                                                      *
			//      Let the reduced argument A be written as                        *
			//                     A=x+y, where  x= TanTablePtr[index].arg          *
			//      Then if T=tan(x) and C=cot(x) as read from tantbl, and t=tan(y),*
			//      (computed above as (tan+tanl), compute cot(A) as:               *
			//                                                                      *
			//                                  t(C+T)                              *
			//                      cot(A)=C - ----------                           *
			//                                  T+t                                 *
			//***********************************************************************
			p23 = __FADD( t1, tan );							// High order part of T+t
			p24 = __FADD( t2, tanl );
			rec = 1.0/p23;
			p25 = t1 - p23 + tan;
			frac = p19*rec;							// start computation of fraction
			
			if (fabs(t2) > fabs(tanl))
				p26 = t2 - p24 + tanl;
			else
				p26 = tanl - p24 + t2;
			
			p27 = __FADD( p25, p24 );
			p28 = p25 - p27 + p24 + p26;													// T+t= (p23,p27,p28) done
			frac = frac + __FNMSUB( p23, frac, p19 )*rec; // frac + (p19-p23*frac)*rec;		// refine lead fraction wd.
			p29 = __FNMSUB( p23, frac, p19 );		// p19 - p23*frac;
			p30 = frac*p27;																	// p30 a negative term
			p31 = __FNMSUB( frac, p27, p30 ) + p22; // p30 - frac*p27 + p22;
			p32 = __FNMSUB( frac, p28, p31 );		// p31 - frac*p28;
			p33 = __FSUB( p29, p30 );
			p35 = __FADD( p33, p21 );
			frac2 = p35*rec;
			frac2 = frac2, __FNMSUB( frac2, p23, p35 )*rec; // frac2 + (p35-frac2*p23)*rec;	// second fraction wd.
			
			if (fabs(p29) > fabs(p30))
				p34 = p29 - p33 - p30 + p32;
			else
				p34 = p29 - (p33+p30) + p32;
			
			p36 = __FNMSUB( frac2, p23, p35 ) + __FNMSUB( frac2, p27, p34 ); // p35 - frac2*p23 + p34 - frac2*p27;
			
			if (fabs(p33) > fabs(p21))
				p37 = p33 - p35 + p21;
			else
				p37 = p21 - p35 + p33;
			
			p38 = p37 + p36;
			frac3 = p38*rec;						// Fraction done
			
			//*************************************************************************
			//    At last, subtract (cot,cotl) from (frac,frac2,frac3) for the result *
			//*************************************************************************
			p39 = __FSUB( frac, c1 );
			p40 = frac - (c1+p39);
			p41 = __FSUB( frac2, c2 );
			if (fabs(c2) > fabs(frac2))
				p42 = frac2 - (c2+p41) + frac3;
			else
				p42 = frac2 - p41 - c2 + frac3;
			
			p43 = __FADD( p40, p41 );
			p44 = p40 - p43 + p41 + p42;
			p45 = __FADD( p39, p43 );
			p46 = p39 - p45 + p43 + p44;
			
			//getting a cannonical result
			
			ddx.d[0] = __FADD( p45, p46 ); 			
			ddx.d[1] = (p45 - ddx.d[0]) + p46; 
			
			FESETENVD(fenv);
			return (ddx.f);
		}
	}
}
#endif
