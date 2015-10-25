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
//	ArcTanDD.c
//	
//	Double-double Function Library
//	Copyright: © 1995-96 by Apple Computer, Inc., all rights reserved
//	
//	long double atanl( long double x );
//	long double atan2l( long double y, long double x );
//
//	Change History:
//		8/31/96	- PAF - Changed if() to else (see Note 1)
//

#include "math.h"
#include "fp_private.h"
#if (__WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L)
#include "DD.h"

static const double kBig = 6.755399441055744e+15;				// 0x1.8p52   

static const double kPiBy2 = 1.570796326794896619231322;		// 0x1.921FB54442D18p0   
static const double kPiBy2Tail1  =  6.123233995736766036e-17;	// 0x1.1A62633145C07p-54  
static const double kPiBy2Tail1m  = 6.123233995736764803e-17;	// 0x1.1A62633145C06p-54  
static const double kPiBy2Tail2 = -1.4973849048591698e-33;		// 0x1.F1976B7ED8FBCp-110  
static const double kRecip = 6.869637070016756334588131e-12;	// 0x1.e368613a3b430p-38
static const double kLimit1 = 0.0859375;						// 0x1.6p-4   
static const double kLimit2 = 1.015625;	
static const double kLimit3 = 13.0;	
static const double kLimit4 =  8.11296384146066817e+31;			// 0x1.0p+106 
static const Float  kInfinityu = {0x7f800000};

//***************************************************************************
//    factor -> gives the number of table values between the consecutive    *
//                  integers 1 to 12                                        *
//***************************************************************************

static const float factor[12] = {
	32.0f,16.0f,8.0f,4.0f,4.0f,2.0f,2.0f,1.0f,1.0f,1.0f,1.0f,1.0f
};


//***************************************************************************
//    Adjust     -> index into accurate atan table, for integer arguments   *
//                  from 1 to 12.                                           *
//***************************************************************************

static const float adjust[12] = {
	32.0f,64.0f,88.0f,104.0f,104.0f,116.0f,116.0f,124.0f,124.0f,124.0f,124.0f,124.0f
};


//***************************************************************************
//         atancoeff: coefficients of atan approximating polynomial,        *
//                    multiplied by 145568097675.0                          *
//***************************************************************************

static const double atancoeff[15] = {
	145568097675.0, -48522699225.0,
	 29113619535.0, -20795442525.0,   16174233075.0,
	-13233463425.0,  11197545975.0,   -9704539845.0,
	  8562829275.0,  -7661478825.0,    6931814175.0,
	 -6329047725.0,   5822723907.0,   -5391411025.0,
	  5019589575.0
};

static const uint32_t atanTable[] __attribute__ ((aligned(8))) = {

	//*******************************************************************
	//	  Accurate table over range (	.093750,   .984375).            *
	//*******************************************************************
	
	0x3FB80000,	0x96AC4A5A,	0x3FB7EE18,	0xBB5F2BDC,	0x3B1F7151,	0xA60EEDFD,
	0x3FBC0000,	0xAB2E5990, 0x3FBBE39F,	0x679755D8, 0x3B5AF7C1,	0xBBB2A139,
	0x3FC00000,	0x7F748304, 0x3FBFD5BB,	0x95A94110, 0x3B5B0A0C,	0x811C880E,
	0x3FC20000,	0x3DF2157F, 0x3FC1E1FB,	0x37C2C7E5, 0x3B617207,	0xB776AF4F,
	0x3FC40000,	0x30754446, 0x3FC3D6EF,	0x1814018D, 0xBB63A092,	0x88503B16,
	0x3FC60000,	0x7996ECD4, 0x3FC5C981,	0x94588C25, 0xBB5B6979,	0x558D8B64,
	0x3FC80000,	0x76D31B22, 0x3FC7B97B,	0xBE985C07, 0x3B6708AD,	0xCC1C4519,
	0x3FCA0000,	0xEADCE47A, 0x3FC9A6A9,	0xCAFAF9A8, 0x3B62971C,	0xA562CA82,
	0x3FCC0000,	0x7E8C3F27, 0x3FCB90D7,	0xCB57348B, 0xBB493C17,	0xA875159C,
	0x3FCE0000,	0x5B063A6A, 0x3FCD77D6,	0x3569311A, 0xBB5A33AB,	0xA192F16A,
	0x3FD00000,	0x64EF22FF, 0x3FCF5B76,	0xB72AE095, 0x3B52E368,	0xB50909D6,
	0x3FD10000,	0x87DE9AAA, 0x3FD09DC6,	0x16C29778, 0xBB55B0DC,	0xF3418D68,
	0x3FD20000,	0x0AAD7C84, 0x3FD18BF5,	0xACF10E71, 0x3B41EAC1,	0x346154B4,
	0x3FD30000,	0x1D92088F, 0x3FD27837,	0x3B84D2FF, 0x3B48BBE9,	0xDC9998E5,
	0x3FD40000,	0xCC4E6285, 0x3FD36277,	0xF12910F6, 0xBB6585F3,	0xFA8B319B,
	0x3FD50000,	0x766A7592, 0x3FD44AA4,	0xA1AA8D78, 0x3B6C6289,	0x13F656CF,
	0x3FD60000,	0xCFA33544, 0x3FD530AE,	0x5303BB5D, 0x3B63F533,	0xC1117E20,
	0x3FD70000,	0xC925EED6, 0x3FD61484,	0xB52DF321, 0xBB70059A,	0x66E93FA9,
	0x3FD80000,	0x54AA10D9, 0x3FD6F619,	0x8C1ECA84, 0x3B60C60C,	0x08BED883,
	0x3FD90000,	0x113AC5B7, 0x3FD7D560,	0x5A568BA4, 0xBB541817,	0xB5654BFC,
	0x3FDA0000,	0x6D101F2E, 0x3FD8B24D,	0x96E71249, 0xBB55A721,	0x1379C827,
	0x3FDB0000,	0xE246C017, 0x3FD98CD6,	0x05641F92, 0x3B6918FF,	0xCA4E1DF9,
	0x3FDC0000,	0x39B58346, 0x3FDA64EE,	0xF43C341A, 0x3B6633D0,	0xEECC747A,
	0x3FDD0000,	0x4C36F71C, 0x3FDB3A91,	0x5CE1B431, 0xBB61E754,	0x0B1A4FB4,
	0x3FDE0000,	0x66CA921B, 0x3FDC0DB5,	0x1D94F195, 0x3B571DE1,	0xE6981964,
	0x3FDF0000,	0x3AD3639E, 0x3FDCDE53,	0x72D1AC94, 0xBB736718,	0x370B3962,
	0x3FE00000,	0x7815CCCD, 0x3FDDAC67,	0xC5849B77, 0x3B52FF7C,	0xC7D84489,
	0x3FE08000,	0xF40DDDFC, 0x3FDE77ED,	0x00AEBFCD, 0x3B62E958,	0xE33FF707,
	0x3FE10000,	0xA6C93DAD, 0x3FDF40DE,	0x0F7AA915, 0x3B77641D,	0x15CA0475,
	0x3FE18000,	0x85EBF4D7, 0x3FE0039C,	0xDAD8BEF9, 0x3B722996,	0x8391BF09,
	0x3FE20000,	0xC129DA76, 0x3FE0657F,	0x27977717, 0xBB761869,	0xC045B9BD,
	0x3FE28000,	0x93F751BC, 0x3FE0C614,	0xCA41B0AE, 0xBB7A8A92,	0x3C961154,
	0x3FE30000,	0x7C1EF6CE, 0x3FE1255D,	0xF7C0A55C, 0x3B591CBF,	0x478B1C38,
	0x3FE38000,	0x169FA349, 0x3FE1835A,	0x993DD546, 0x3B6FBFAB,	0x698175A0,
	0x3FE40000,	0x27D6B511, 0x3FE1E00B,	0xC884E585, 0xBB248C8D,	0x9BB39A1C,
	0x3FE48000,	0x6BE89876, 0x3FE23B72,	0x2F4EF788, 0xBB815690,	0x7DF099CF,
	0x3FE50000,	0x280F4ABE, 0x3FE2958E,	0x7530C25F, 0xBB72B7E0,	0xEE07EAE7,
	0x3FE58000,	0x2FEDFD47, 0x3FE2EE62,	0xA50C9A07, 0xBB2B522D,	0xBFFA3A8C,
	0x3FE60000,	0x875C63BA, 0x3FE345F0,	0x78B8C001, 0xBB408ED6,	0xD056E4C4,
	0x3FE68000,	0x8A368DBA, 0x3FE39C39,	0x79511750, 0x3B77CB64,	0x0C777681,
	0x3FE70000,	0xEE7CA285, 0x3FE3F140,	0x55DECB32, 0xBB5AA9DC,	0xBC68CF79,
	0x3FE78000,	0xA4904A32, 0x3FE44506,	0xC661B510, 0x3B7EBCE3,	0x3EB6C0CD,  
	0x3FE80000,	0x4CA9F6E2, 0x3FE4978F,	0xD4373CAA, 0x3AFF9675,	0xAB73F345,
	0x3FE88000,	0xD7B95553, 0x3FE4E8DE,	0xE3B770C7, 0xBB715BD3,	0xBFA6C0AD,
	0x3FE90000,	0x849C7E04, 0x3FE538F5,	0xCDE27023, 0xBB6B45C2,	0x3E5A5CC8,
	0x3FE98000,	0xC171C662, 0x3FE587D8,	0x95C38C2E, 0x3B7887A1,	0x345B0752,
	0x3FEA0000,	0x4321104C, 0x3FE5D589,	0xAF86142F, 0xBB8438EB,	0x1AE94AC1,
	0x3FEA8000,	0x838D8D26, 0x3FE6220D,	0x5F66C783, 0x3B7CA422,	0x8F96E72F,
	0x3FEB0000,	0xE4DA897E, 0x3FE66D66,	0xBED2B224, 0xBB35FB4D,	0xFD76AD75,
	0x3FEB8000,	0xD775765C, 0x3FE6B799,	0x0DF9D017, 0x3B7B5383,	0xE0D80102,
	0x3FEC0000,	0xAA053CA7, 0x3FE700A8,	0x25C3BB63, 0xBB7D5234,	0x29157DFC,
	0x3FEC8000,	0x773AAC9B, 0x3FE74897,	0xD237C658, 0x3B6ACFB2,	0xF4E46A15,
	0x3FED0000,	0x3FA5ABCF, 0x3FE78F6B,	0xE04F6C27, 0xBB8680F4,	0x55D1C7DF,
	0x3FED8000,	0xD31E46AA, 0x3FE7D528,	0x9AC0235F, 0xBB61A1B0,	0x1EABC199,
	0x3FEE0000,	0xBB676A41, 0x3FE819D1,	0x1AD33A87, 0xBB7726EA,	0x735248A8,
	0x3FEE8000,	0x76CB5B54, 0x3FE85D69,	0x95ABE3CC, 0x3B772D9C,	0x044A3CEB,
	0x3FEF0000,	0x26506206, 0x3FE89FF6,	0x131BC92E, 0x3B860889,	0x51E5F051,
	0x3FEF8000,	0xED69BF50, 0x3FE8E17B,	0x2230274E, 0xBB7FB7C5,	0x5CA2A11A, 
	
	//*******************************************************************
	//	  Accurate table over range (  1.000000,  1.968750).            *
	//*******************************************************************
	
	0x3FF00000,	0x38A5805B, 0x3FE921FB,	0x8CE9AD0F, 0xBB728518,	0x56659FDA,
	0x3FF08000,	0xE4CDB94C, 0x3FE9A001,	0x86F9991F, 0xBB7F4F21,	0xC242FD58,
	0x3FF10000,	0x29DA60EA, 0x3FEA1A26,	0x1A19C30A, 0xBB87F5DE,	0xE73B7B3A,
	0x3FF18000,	0x9BBB03C5, 0x3FEA908B,	0x882B14EE, 0x3B8A364F,	0xC319894C,
	0x3FF20000,	0x42A338B5, 0x3FEB034F,	0x7337C8D1, 0xBB768B38,	0xD9BDAC43,
	0x3FF28000,	0xED0428FC, 0x3FEB7292,	0x7FBAC8B6, 0x3B7B400A,	0x53FD9AAF,
	0x3FF30000,	0x36946784, 0x3FEBDE71,	0x1A8E3A61, 0x3B6C2B37,	0x13D123E1,
	0x3FF38000,	0x21F16647, 0x3FEC470A,	0xDA7DBAA7, 0xBB523353,	0xD7A5886A,
	0x3FF40000,	0xF06227B4, 0x3FECAC7D,	0x132231D2, 0xBB8AEFE3,	0x0558B34B,
	0x3FF48000,	0xF9AB1CF7, 0x3FED0EE2,	0xE23FB012, 0xBB4465DC,	0xD4A6C05D,
	0x3FF50000,	0x1DAE2F90, 0x3FED6E57,	0xE51C7E16, 0xBB85A9B9,	0x83503D52,
	0x3FF58000,	0xD679A51F, 0x3FEDCAF8,	0xC6A4C801, 0x3B8576BE,	0xCEAEFC33,
	0x3FF60000,	0xCE367C5E, 0x3FEE24DD,	0xD375A188, 0xBB78F5B1,	0x1E4802A0,
	0x3FF68000,	0xCD05D052, 0x3FEE7C20,	0xCBEB8A43, 0xBB8AD216,	0x43D5F156,
	0x3FF70000,	0x98AA0697, 0x3FEED0D9,	0xE022B144, 0x3B267908,	0xB6C976B5,
	0x3FF78000,	0xA3FB1493, 0x3FEF2320,	0xDB8F1029, 0xBB85B081,	0x83128CFA,
	0x3FF80000,	0x681E0063, 0x3FEF730C,	0x12946C3F, 0xBB8AA69F,	0xBF359098,
	0x3FF88000,	0x75E901B6, 0x3FEFC0B1,	0xB86DE15F, 0x3B8EAA0C,	0x4663839A,
	0x3FF90000,	0x736A4590, 0x3FF00613,	0x4FBE5B7E, 0xBB4E3551,	0x42306F18,
	0x3FF98000,	0xC5B189FC, 0x3FF02ABF,	0xA107BE50, 0x3B63FD1C,	0xCE1D6AEB,
	0x3FFA0000,	0x41411D33, 0x3FF04E67,	0x3966892F, 0xBB86ED08,	0x00B1EF7B,
	0x3FFA8000,	0x4DDFE3DA, 0x3FF07113,	0xDB772C28, 0x3B746CEB,	0x807EA687,
	0x3FFB0000,	0xA7AB4BE2, 0x3FF092CE,	0x72AC060E, 0xBB72F0AF,	0x054AD94E,
	0x3FFB8000,	0x35EE2E87, 0x3FF0B39F,	0x5C6DBFD7, 0xBB8A699D,	0x14CF8D89,
	0x3FFC0000,	0x79493CCE, 0x3FF0D38F,	0x4A3683E2, 0xBB7C178E,	0x4E0FB167,
	0x3FFC8000,	0x7C2C16CD, 0x3FF0F2A5,	0xF7C1C52E, 0xBB77BBEC,	0x0F5FF2D9,
	0x3FFD0000,	0xF7925B8C, 0x3FF110EB,	0x3A456E0E, 0x3B86B850,	0xB94665FD,
	0x3FFD8000,	0xD4EDAFAA, 0x3FF12E66,	0x2A98E0EA, 0x3B8FC04D,	0xF88FF47B,
	0x3FFE0000,	0x1822F5F9, 0x3FF14B1D,	0xDB5166C5, 0x3B400FEE,	0xA21D9DDA,
	0x3FFE8000,	0x5F375D4F, 0x3FF16719,	0x6EAC8C79, 0xBB45D7BA,	0x99C8508E,
	0x3FFF0000,	0x983AF36D, 0x3FF1825F,	0x2745DBAE, 0xBB8849C2,	0x328CE199,
	0x3FFF8000,	0xDF7044EE, 0x3FF19CF5,	0x48D90CBC, 0xBB876AC4,	0x23C76E06,
	
	//******************************************************************
	//	  Accurate table over range (  2.000000,  2.937500).           *
	//******************************************************************
	
	0x40000000,	0xC89A22AD, 0x3FF1B6E1,	0xE3296298, 0xBB83E57A,	0x4E33C68C,
	0x40008000,	0xAE85BCDD, 0x3FF1E8D4,	0xB635433E, 0x3B8FDE86,	0xD38237A1,
	0x40010000,	0x1E66B4AD, 0x3FF21862,	0xFF00ECE6, 0x3B74E47A,	0x940D38B0,
	0x40018000,	0x257005B2, 0x3FF245B5,	0x07E38198, 0xBB90341D,	0xF7AEB5A2,
	0x40020000,	0x55656B58, 0x3FF270EF,	0x71D13D73, 0x3B88C680,	0x13906BD2,
	0x40028000,	0x466225D9, 0x3FF29A34,	0x0FA8F5E6, 0x3B91F854,	0xF983041D,
	0x40030000,	0x717EC3D3, 0x3FF2C1A2,	0x640509A5, 0x3B8E1C75,	0xAF6F1BFF,
	0x40038000,	0x75A7B542, 0x3FF2E757,	0x4A697F72, 0x3B92AB3B,	0x412F53FB,
	0x40040000,	0x00512EF1, 0x3FF30B6D,	0x7980B2E2, 0x3B8AD702,	0x40329E0C,
	0x40048000,	0x09ED9B8A, 0x3FF32DFE,	0x0460EC6D, 0x3B6B92BF,	0x54B284E9,
	0x40050000,	0x1B9F04CA, 0x3FF34F1F,	0xC21A2D1B, 0xBB82C1A6,	0xD9CE49BA,
	0x40058000,	0x697C1A2B, 0x3FF36EE8,	0x0C4A7DD1, 0x3B73DD4E,	0x5C021394,
	0x40060000,	0xABB8A398, 0x3FF38D6A,	0x94FD5FEE, 0x3B373349,	0x3B4FC3A6,
	0x40068000,	0x18374FED, 0x3FF3AAB9,	0x8BB1767A, 0x3B72834E,	0xCBC4CA13,
	0x40070000,	0xBCD94FDA, 0x3FF3C6E6,	0x7976E8D9, 0xBB931322,	0x7228DCD0,
	0x40078000,	0x7BA476B6, 0x3FF3E200,	0xC84E84B4, 0xBB87D21A,	0xC44FB391,
	
	//******************************************************************
	//	  Accurate table over range (  3.000000,  3.875000).           *
	//******************************************************************
	
	0x40080000,	0xD7180754, 0x3FF3FC17,	0x967F5249, 0xBB64B771,	0x5FA0957B,
	0x40090000,	0x6DFDCD42, 0x3FF42D70,	0x558EAD29, 0x3B84B923,	0xDD410F01,
	0x400A0000,	0xE7C427AE, 0x3FF45B54,	0xAB8A2C51, 0x3B91D694,	0xE38B7BFF,
	0x400B0000,	0x10E3223E, 0x3FF4861B,	0x4FB5B5B4, 0xBB7CB027,	0x50BA194A,
	0x400C0000,	0x8EA13F5E, 0x3FF4AE11,	0x11ECF83F, 0x3B8B6616,	0xD89A7B16,
	0x400D0000,	0x06D105F7, 0x3FF4D378,	0xC2906964, 0x3B93659C,	0x0737481A,
	0x400E0000,	0x727DD5BC, 0x3FF4F68D,	0xF99AE908, 0x3B8514AE,	0x4636F80B,
	0x400F0000,	0x3FDFD1C5, 0x3FF51785,	0x020F4064, 0xBB65DB78,	0x8577ADC5,
	
	//******************************************************************
	//    Accurate table over range (  4.000000,  5.750000).           *
	//******************************************************************
	
	0x40100000,	0xCF49E240, 0x3FF5368C,	0xC5E4B1C8, 0xBB5E5499,	0xA9C4BF51,
	0x40110000,	0x94E90733, 0x3FF56F6F,	0x52E31047, 0x3B8F3B21,	0x724AC4D7,
	0x40120000,	0x46578102, 0x3FF5A250,	0x5F4EF405, 0x3B8657A1,	0x1EBE66F0,
	0x40130000,	0xCA3D6D0F, 0x3FF5D013,	0xE66FF8CA, 0x3B60B423,	0x26CAA3CA,
	0x40140000,	0x32964FAE, 0x3FF5F973,	0x1CEDA34B, 0x3B635D66,	0xC3ED1C18,
	0x40150000,	0xA6AA4915, 0x3FF61F06,	0xDE005085, 0x3B6E8C7F,	0x000B0CDD,
	0x40160000,	0x8906EAE3, 0x3FF6414D,	0x5593660A, 0x3B95FADD,	0xB8BF7A98,
	0x40170000,	0x831FA60C, 0x3FF660B0,	0x3BD94D47, 0x3B96491D,	0x5B9C83F6,
	
	//******************************************************************
	//	  Accurate table over range (  6.000000,  7.500000).           *
	//******************************************************************
	
	0x40180000,	0x8DCF9463, 0x3FF67D88,	0x73114F7D, 0x3B928119,	0xC1230051,
	0x401A0000,	0xCBB9EEAD, 0x3FF6B0BA,	0xFB083C0D, 0x3B93280F,	0xAE0D69E3,
	0x401C0000,	0xDEF559E1, 0x3FF6DCC5,	0x8D8B9598, 0x3B86722D,	0x1022C04A,
	0x401E0000,	0x744BF1EC, 0x3FF7030D,	0x01605442, 0xBB736E8B,	0xF92A87CA,
	
	//******************************************************************
	//    Accurate table over range (  8.000000, 12.000000).           *
	//******************************************************************
	
	0x40200000,	0x456D4528, 0x3FF7249F,	0xB324E4B7, 0x3B675690,	0xDD79F342,
	0x40220000,	0x43E2466D, 0x3FF75CBA,	0xD9437CC1, 0x3B46FCCD,	0xEB45ACCD,
	0x40240000,	0x18A681EA, 0x3FF789BD,	0x2E09D7EA, 0x3B6C324D,	0xB28DD9D9,
	0x40260000,	0xF3FE273A, 0x3FF7AEA3,	0x9C1AAC21, 0x3B8D6F66,	0xFC2D1461,
	0x40280000,	0x455A6241, 0x3FF7CD6F,	0x71992B07, 0x3B8B2FE8,	0xBAEBACBC,
	0x40280000,	0x455A6241, 0x3FF7CD6F,	0x71992B07, 0x3B8B2FE8,	0xBAEBACBC
};

struct atantblEntry { 
   double One;				
   double Two;
   double Three;
} atantblEntry;
	
long double atanl( long double x )
{
	double p1, p2, p3, p2a, p4, p5, p6, p7, p8, p29, p30, p33, p35;
	double quot, quot2;
	double rarg, result, restemp;
	double top, top2, topextra, top3;
	double bot, bot2;
	double r, rarg2, rarg2a;
	double temps;
	double remainder, residual;
	double partial;  
	double arg, argl, argsq, argsq2;
	double temp,temp2; 
	double sum, sumt, suml;
	double absx;
	double xHead, xTail;
	double res, reslow, resmid, restiny, reshi, resbot;
	double p, q; 
	double prod, prodlow;	
	Double DeN ;
	double fenv;
	DblDbl ddx;
	int i, rflag;
	uint32_t index;
	
	struct atantblEntry *atanPtr = (struct atantblEntry *)atanTable - 6;
	
        FEGETENVD(fenv);
        FESETENVD(0.0);
	
	ddx.f = x;
	xHead = ddx.d[0];
	xTail = ddx.d[1];
	
	// x is a NaN or zero
	
	if ((xHead != xHead)||(xHead == 0.0)) {			// x is 0 or a NaN
                FESETENVD(fenv);
		return x;
	}
	
	// x is not a NaN nor zero
	
	absx = fabs(xHead);
	
	//finite result
	
	//***************************************************************************
	//      This routine breaks the arguments into five ranges:                 *
	//      Range 1:          0 <= |x| <= .0859375                              *
	//      Range 2:   .0859375 <= |x| <= 1.015625                              *
	//      Range 3:   1.015625 <= |x| < 13                                     *
	//      Range 4:         13 <= |x| < 2^107                                  *
	//      Range 5:      2^107 <= |x| <= Infinity                              *
	//                                                                          *
	//         >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>         *
	//                                                                          *
	// In Range 1, a 15th degree polynomial in x^2 is used to evaluate QATAN.   *
	//                                                                          *
	// In Range 2  and,                                                         *
	// In Range 3  the accurate table method is used to range-reduce the        *
	//             argument to a small value to be used with an interpolating   *
	//             polynomial for QATAN.  In Ranges 2 and 3, the resultant      *
	//             argument is less than 2^-7, so an 8th degree polynomial can  *
	//             be used.  Range 3 requires additional work to find the       *
	//             correct index into the accurate table.                       *
	//                                                                          *
	// In Range 4, QATAN(x) = Pi/2 - QATAN(1/x).  After taking the reciprocal   *
	//             of the argument, the quotient is less than 1/13, so that the *
	//             15th degree polynomial used for Range 1 is used to compute   *
	//             QATAN(1/x)                                                   *
	//                                                                          *
	// In Range 5, the correctly rounded value differs from Pi/2 by less than   *
	//             1/2 ulp.  It is, however, incorrect to let the value become  *
	//             correctly rounded, as that correctly rounded value exceeds   *
	//             PI/2 and is in the wrong quadrant.  Applying the inverse     *
	//             function, QTAN, to the correctly rounded value of PI/2 would *
	//             produce a result of the opposite sign to the argument        *
	//             originally given to ATAN.                                    *
	//***************************************************************************
	
	// Range 5
	
	if (absx >= kLimit4) {							// Filter out unusual arguments
		ddx.d[0] = kPiBy2;  
		ddx.d[1] = kPiBy2Tail1m;					// Intentional misround to
		if (xHead < 0.0) {      
			ddx.d[0] = -ddx.d[0];  
			ddx.d[1] = -ddx.d[1];		
		}
                FESETENVD(fenv);
		return (ddx.f);
	}
	
	// Ranges 1 and 4 (DO NOT USE TABLE)
	
	rflag = 0;	
	
	// Range 4
	
	if (absx >= kLimit3) {							// Range 4
		rarg = 1.0/xHead;                         
		rflag = 1;									// indicate large argument
		p1 = __FNMSUB( xHead, rarg, 1.0 );			// 1.0 - xHead*rarg;						// time to complete hi precision
		p2 = xTail*rarg;							// reciprocal of argument, to
		p3 = p1 - p2;								// use as reduced argument.
		rarg2a = p3*rarg;
		rarg2 = rarg2a + __FNMSUB( xHead, rarg2a, p3 )*rarg; // rarg2a + (p3 - xHead*rarg2a)*rarg;
		xHead = rarg;
		xTail = rarg2;
	}
	
	// Ranges 1 and 4 only
	
	if( (absx < kLimit1) || (absx >= kLimit3) ) {	// Code for Range 1 and Range 4
		temp = 2.0*xHead*xTail;						// direct evaluation of series
		argsq = __FMADD( xHead, xHead, temp );		// xHead*xHead + temp;					// compute square of argument
		argsq2 = __FMADD( xTail, xTail, __FMSUB( xHead, xHead, argsq ) + temp ); // xHead*xHead - argsq + temp + xTail*xTail;
		sum = atancoeff[14];
		sum = __FMADD( argsq, sum, atancoeff[13] );			// atancoeff[13] + argsq*sum;
		sum = __FMADD( argsq, sum, atancoeff[12] );			// atancoeff[12] + argsq*sum;
		sum = __FMADD( argsq, sum, atancoeff[11] );			// atancoeff[11] + argsq*sum;
		sumt = __FMADD( argsq, sum, atancoeff[10] );		// atancoeff[10] + argsq*sum;
		sum = __FMADD( argsq, sum, atancoeff[9] );			// atancoeff[9] + argsq*sumt;
		suml = __FMADD( argsq, sumt, atancoeff[9] - sum );	// atancoeff[9] - sum + argsq*sumt;
		
		for (i = 8; i >= 1; i--) {	 		   
			temps = __FMADD( argsq, sum, atancoeff[i] );	// atancoeff[i] + sum*argsq;
			/* suml = atancoeff[i] - temps+sum*argsq + (sum*argsq2 + suml*argsq); */
			suml = __FMADD( sum, argsq, atancoeff[i] -  temps ) + __FMADD( sum, argsq2, suml*argsq );
			sum=temps;
		}
		
		prodlow = __FMADD( suml, argsq, sum*argsq2 );		// suml*argsq + sum*argsq2;			// multiply by arg^2
		prod = __FMADD( sum, argsq, prodlow );				// sum*argsq + prodlow;
		prodlow = __FMSUB( sum, argsq, prod ) + prodlow;	// sum*argsq - prod + prodlow;
		temp2 = __FMADD( prodlow, xHead, prod*xTail );		// prodlow*xHead + prod*xTail;		// multiply by arg
		temp = __FMADD( prod, xHead, temp2 );				// prod*xHead + temp2;
		temp2 = __FMSUB( prod, xHead, temp ) + temp2;		// prod*xHead - temp + temp2;
		sum = temp*kRecip;																		// sum of series ---
		remainder = __FNMSUB( sum, atancoeff[0], temp );	// (temp - sum*atancoeff[0]);
		partial = __FADD( remainder, temp2 );
		residual = remainder - partial + temp2;
		suml = __FMADD( partial, kRecip, (residual*kRecip) );// partial*kRecip + (residual*kRecip);
		res = __FADD( xHead, sum );																// except for argument
		reslow = (xHead - res + sum);															// exact
		resmid = __FADD( xTail, suml );
		restiny = xTail - resmid + suml;
		p = __FADD( reslow, resmid );															// sum of middle terms
		q = reslow - p + resmid;																// possible residual
		reshi = res + p;
		resbot = (res - reshi + p) + (q + restiny);
		if (rflag == 1) {							// large magnitude argument
			if (xHead > 0) {						// fetch pi/2 with proper sign
				p1 = kPiBy2;
				p2 = kPiBy2Tail1;
				p2a = kPiBy2Tail2;
			}
			else {
				p1 = -kPiBy2;
				p2 = -kPiBy2Tail1;
				p2a = -kPiBy2Tail2;
			}
			p3 = __FSUB( p1, reshi );				// subtract result from pi/2
			p4 = p1 - p3 - reshi;
			p5 = __FSUB( p2, resbot );
			p6 = p2 - p5 - resbot + p2a;
			p7 = __FADD( p4, p5 );
			reshi = __FADD( p3, p7 );
			if (fabs(p4) > fabs(p5)) 
				p8 = p4 - p7 + p5;
			else
				p8 = p5 - p7 + p4;
			
			resbot = p3 - reshi + p7 + (p6 + p8 - restiny);
		}
		ddx.d[0] = __FADD( reshi, resbot );  
		ddx.d[1] = (reshi - ddx.d[0]) + resbot;		
		
                FESETENVD(fenv);
		return (ddx.f);
	}
	
	// Ranges 2 and 3  (USE TABLE)
	
	//if( (absx >= kLimit1) || (absx < kLimit3) ) {	// Code for Range 2 and Range 3
	
	// Note 1: The above statement in the Taligent code always tests true.  Logically,
	// it was probably intended that || be &&.  This would make the test logically
	// equivalent to an "else" clause for the previous if.  Replacing with an else
	// also eliminates the problem of a code path that avoids any return statements
	// (as detected by MrC as a compiler warning).  PAF
	else {
		
		DeN.f = __FMADD( 64.0, absx, kBig ); // 64.0*absx + kBig;
		
		if (absx > kLimit2) {						// for large args, use recip.  Range 3
			FESETENVD(kBig+1.0);						// use trunc mode momentarily
			DeN.f = absx + kBig;					// determine range of argument
			FESETENVD(kBig+1.0);						// use trunc mode momentarily
			DeN.f = absx + kBig;					// determine range of argument
			FESETENVD(kBig);							// return to round-to-nearest
			index = DeN.i[1];
			i = index - 1;
			DeN.f = __FMADD( factor[i], absx, kBig + adjust[i] ); // factor[i]*absx + kBig + adjust[i];
		}
		
		index = DeN.i[1];
		
		if (xHead < 0) {							// use abs. value
			arg  = -xHead;
			argl = -xTail;
		}
		else {
			arg  = xHead;
			argl = xTail;
		}
		
		top = __FSUB( arg, atanPtr[(int32_t)index].One );
		top2 = __FADD( top, argl );
		topextra = arg - top - atanPtr[(int32_t)index].One;
		top3 = top - top2 + argl + topextra;												// reduced numerator
		bot =__FMADD( arg, atanPtr[(int32_t)index].One, 1.0 ); // 1.0 + arg*atanPtr[(int32_t)index].One;
		/* bot2 = 1.0 - bot + arg*atanPtr[(int32_t)index].One +
			argl*atanPtr[(int32_t)index].One;	*/											// denominator
		bot2 = __FMADD( argl, atanPtr[(int32_t)index].One, __FMADD( arg, atanPtr[(int32_t)index].One, 1.0 - bot ) );
		r = 1.0/bot;
		quot = r*top2;
		quot = __FMADD( __FNMSUB( bot, quot, top2), r, quot );	// quot + (top2 - bot*quot)*r;
		
		p29 = __FNMSUB( bot, quot, top2);				// top2 - quot*bot;
		p30 = quot*bot2;																	// negative term
		p33 = p29 - p30;
		p35 = p33 + top3;
		quot2 = p35*r;
		quot2 = __FMADD( __FNMSUB( quot2, bot, p35 ), r, quot2 );	// quot2 + (p35 - quot2*bot)*r;		// second fraction wd.
		temp = 2.0*quot*quot2;																// reduced arg is (quot,quot2)
		argsq = __FMADD( quot, quot, temp );			// quot*quot + temp;				// compute square of argument
		argsq2 = __FMADD( quot2, quot2, __FMSUB( quot, quot, argsq) + temp ); // quot*quot - argsq + temp + quot2*quot2;
		sumt = __FMADD( argsq, atancoeff[7], atancoeff[6] );// atancoeff[6] + argsq*atancoeff[7];
		sum = __FMADD( argsq, sumt, atancoeff[5] );		// atancoeff[5] + argsq*sumt;
		suml = __FMADD( argsq, sumt, atancoeff[5] - sum ); // atancoeff[5] - sum + argsq*sumt;
		
		for (i = 4; i >= 1; i--) {	 	
			temps = __FMADD( sum, argsq, atancoeff[i] );	// atancoeff[i] + sum*argsq;
			/* suml = atancoeff[i] - temps + sum*argsq + (sum*argsq2 + suml*argsq); */
			suml = __FMADD( sum, argsq, atancoeff[i] - temps ) + __FMADD( sum, argsq2, suml*argsq );
			sum = temps;
		}
		
		prodlow = __FMADD( suml, argsq, sum*argsq2 );		// suml*argsq + sum*argsq2;			// mult. by arg^2
		prod = __FMADD( sum, argsq, prodlow );				// sum*argsq + prodlow;
		prodlow = __FMSUB( sum, argsq, prod ) + prodlow;		// sum*argsq - prod + prodlow;
		temp2 = __FMADD( prodlow, quot, prod*quot2 );		// prodlow*quot + prod*quot2;		// mult. by arg
		temp = __FMADD( prod, quot, temp2 );				// prod*quot + temp2;
		temp2 = __FMSUB( prod, quot, temp ) + temp2;		// prod*quot - temp + temp2;
		sum = temp*kRecip;																		// sum of series ---
		remainder = __FNMSUB( sum, atancoeff[0], temp );		// (temp - sum*atancoeff[0]);
		partial = __FADD( remainder, temp2 );
		residual = remainder - partial + temp2;
		suml = __FMADD( partial, kRecip, (residual*kRecip) ); // partial*kRecip + (residual*kRecip);
		res = __FADD( quot, sum );																// except for argument
		reslow = (quot - res + sum);															// exact
		resmid = __FADD( quot2, suml );
		restiny = quot2 - resmid + suml;
		p = __FADD( reslow, resmid );															// sum of middle terms
		q = reslow - p + resmid;																// possible residual
		reshi = __FADD( res, p );
		resbot = (res - reshi + p) + (q + restiny);
		result = __FADD( atanPtr[(int32_t)index].Two, reshi );
		reslow = __FADD( atanPtr[(int32_t)index].Three, resbot );
		resmid = atanPtr[(int32_t)index].Two - result + reshi;
		restiny = resbot - reslow + atanPtr[(int32_t)index].Three;
		restemp = __FADD( resmid, reslow );
		reshi = __FADD( result, restemp );
		
		if (fabs(resmid) > fabs(reslow)) 
			restiny = resmid - restemp + reslow + restiny;
		else
			restiny = reslow - restemp + resmid + restiny;
		
		resbot = result - reshi + restemp + restiny;
		
		if (xHead < 0.0) {
			reshi = -reshi;
			resbot = -resbot;
			result = -result;
			restemp = -restemp;
			restiny = -restiny;
		}
		
		ddx.d[0] = __FADD( reshi, resbot );  
		ddx.d[1] = (reshi - ddx.d[0]) + resbot;		
		
                FESETENVD(fenv);
		return (ddx.f);								// Normal arguments done
	}
}

//  End of atanl, start of atan2l:

//*******************************************************************************
//     _QATAN2 expects two quad words as arguments.  It returns the arctan      *
//     of the quotient of the arguments, but differs from _QATAN in that        *
//     the result can be in the interval [-Pi,Pi], whereas _QATAN always        *
//     returns a result in the range [-Pi/2, Pi/2].  After argument reduction   *
//     and the testing of numerous special cases, _QATAN is used for the bulk   *
//     of the work.                                                             *
//                                                                              *
//       atan2(NaN, anything)--------------------------------->   NaN           *
//       atan2(anything, NaN)--------------------------------->   NaN           *
//       atan2(+-0, +(anything but NaN))----------------------> +-0             *
//       atan2(+-0, -(anything but NaN))----------------------> +-Pi            *
//       atan2(+-(anything but 0 and NaN), 0 )----------------> +-Pi/2          *
//       atan2(+-(anything but Infinity and NaN), +Infinity)--> +-0             *
//       atan2(+-(anything but Infinity and NaN), -Infinity)--> +-Pi            *
//       atan2(+-Infinity, +Infinity)-------------------------> +-Pi/4          *
//       atan2(+-Infinity, -Infinity)-------------------------> +-3Pi/4         *
//       atan2(+-Infinity, (anything but 0, NaN, Infinity))---> +-Pi/2          *
//                                                                              *
//*******************************************************************************

// Floating-point constants
static const double kTiny = 1.805194375864829576e-276;		// 0x1.0p-916 = 0x06B00000   
// kTiny: number that still allows a 107 bit quad to consist of two normal numbers

static const double kRescale = 2.923003274661805836e+48;	// 0x1.0p+161   
// Needed to rescale if the denominator is "tiny"

static const double kPi = 3.141592653589793116;				// 0x1.921FB54442D18p1   
static const double kPiTail1  = 1.224646799147353207e-16;	// 0x1.1A62633145C07p-53  
static const double kPiTail1m  = 1.224646799147352961e-16;	// 0x1.1A62633145C06p-53  
static const double kPiTail2 = -2.994769809718339666e-33;	// 0x1.F1976B7ED8FBCp-109  

static const double k3PiBy4 = 2.356194490192344837;			// 0x1.2D97C7F3321D2p1   
static const double k3PiBy4Tail1 = 9.184850993605148438e-17;// 0x1.A79394C9E8A0Ap-54  


long double atan2l( long double y, long double x )
{
	double p1, p2, p3, p2a, p4, p5, p6, p7, p8, p29, p30, p33, p35;
	double p31, p32, p34, p36, p37, p38;
	double quot, quot2;
	double rarg, result, restemp;
	double top, top2, topextra, top3;
	double extra;
	double bot, bot2;
	double r, rarg2, rarg2a;
	double temps;
	double remainder, residual;
	double partial;  
	double arg, argl, argsq, argsq2;
	double temp,temp2; 
	double sum, sumt, suml;
	double absx;
	double xHead, xTail;
	double res, reslow, resmid, restiny, reshi, resbot;
	double p, q; 
	double prod, prodlow;	
	double xDen, xDenTail, yNum, yNumTail;
	double frac3, frac2, frac, fract;
	double pin, pin2, pin3;
	double ressmall, restop, resint;
	Double DeN ;
	double fpenv;
	DblDbl ddx, ddy;
	
	int i, rflag;
	
	uint32_t index;
	uint32_t signx, signy, expx, expy, sigx, sigy;
	
	struct atantblEntry *atanPtr = (struct atantblEntry *)atanTable - 6;

        FEGETENVD(fpenv);
        FESETENVD(0.0);
	
	ddx.f = x;				// denominator
	ddy.f = y;				// numerator
	
	xDen     = ddx.d[0];	// dden
	xDenTail = ddx.d[1];	// ddenl    
	yNum     = ddy.d[0];	// dnum
	yNumTail = ddy.d[1];	// dnuml
	
	expy  = (ddy.i[0] >> 20) & 0x7ffu;
	expx  = (ddx.i[0] >> 20) & 0x7ffu;		// get biased exponents
	signy = (ddy.i[0] >> 31) & 1u;
	signx = (ddx.i[0] >> 31) & 1u;			// get signs
	sigy  = (ddy.i[0] & 0xfffffu) | ddy.i[1];
	sigx  = (ddx.i[0] & 0xfffffu) | ddx.i[1];
	
	if (expy == 0x7ffu) {					// y is infinite or NaN
		if (sigy != 0) {
                        FESETENVD(fpenv);
			return (y);						// NaN y is returned
		}
		else if (expx == 0x7ffu) {			// infinite y
			if (sigx != 0) {
                                FESETENVD(fpenv);
				return (x);					// NaN x is returned
			}
			else if (signx == 0) {			// infinite y and x  denominator positive
				if (signy) {				// numerator negative
					ddx.d[0] =  -0.5*kPiBy2 ; 	
					ddx.d[1] =  -0.5*kPiBy2Tail1;
                                        FESETENVD(fpenv);
					return (ddx.f);
				}
				else {						// numerator positive
					ddx.d[0] =  0.5*kPiBy2 ; 	
					ddx.d[1] =  0.5*kPiBy2Tail1;
                                        FESETENVD(fpenv);
					return (ddx.f);
				}
			}
			else {							// denominator negative
				if (signy) { 				// numerator negative
					ddx.d[0] =  -k3PiBy4 ; 	
					ddx.d[1] =  -k3PiBy4Tail1;
                                        FESETENVD(fpenv);
					return (ddx.f);
				}
				else {						// numerator positive
					ddx.d[0] =  k3PiBy4 ; 	
					ddx.d[1] =  k3PiBy4Tail1;
                                        FESETENVD(fpenv);
					return (ddx.f);
				}
			}
		}
		else {								// y is Infinite , x is finite
			if (signy == 0.0) {     
				if (signx == 0.0) {			// first quadrant 
					ddx.d[0] = kPiBy2 ; 	
					ddx.d[1] = kPiBy2Tail1m;
                                        FESETENVD(fpenv);
					return (ddx.f);
				} 
				else {						// second quadrant 
					ddx.d[0] = kPiBy2 ; 	
					ddx.d[1] = kPiBy2Tail1;
                                        FESETENVD(fpenv);
					return (ddx.f);
				}
			} 
			else  {							// yNum < 0.0
				if (signx == 0.0) {			// fourth quadrant
					ddx.d[0] = -kPiBy2 ; 	
					ddx.d[1] = -kPiBy2Tail1m;
                                        FESETENVD(fpenv);
					return (ddx.f);
				} 
				
				else  {						// third quadrant
					ddx.d[0] = -kPiBy2 ; 	
					ddx.d[1] = -kPiBy2Tail1;
                                        FESETENVD(fpenv);
					return (ddx.f);
				}
			}
		}
	}										// [y is infinite or NaN]
	if (expx == 0x7ffu) {					// x is infinite or NaN
		if (sigx != 0)	{					//   NaN x is returned
                        FESETENVD(fpenv);
			return (x);
		}
		else if (signx) {					// x is -INF	
			if (signy == 0) {
				ddx.d[0] = 2.0*kPiBy2 ; 	
				ddx.d[1] = 2.0*kPiBy2Tail1m;
                                FESETENVD(fpenv);
				return (ddx.f);
			}
			else {
				ddx.d[0] = -2.0*kPiBy2 ; 	
				ddx.d[1] = -2.0*kPiBy2Tail1m;
                                FESETENVD(fpenv);
				return (ddx.f);
			}
		}
		else								// x is +INF
		if (signy == 0) {
			ddx.d[0] = 0.0 ; 	
			ddx.d[1] = 0.0;
                        FESETENVD(fpenv);
			return (ddx.f);
		}
		else {
			ddx.d[0] = -0.0 ; 	
			ddx.d[1] = -0.0;
                        FESETENVD(fpenv);
			return (ddx.f);
		}
	}										// x is infinite or NaN
	if ((sigx | expx) == 0u) {				// x is zero
		if ((sigy | expy) == 0u) {			// y is zero
			if (signx) {
				if (signy) {
					ddx.d[0] = -2.0*kPiBy2 ; 	
					ddx.d[1] = -2.0*kPiBy2Tail1m;
                                        FESETENVD(fpenv);
					return (ddx.f);
				}
				else {
					ddx.d[0] = 2.0*kPiBy2 ; 	
					ddx.d[1] = 2.0*kPiBy2Tail1m;
                                        FESETENVD(fpenv);
					return (ddx.f);
				}
			}
			else {
                                FESETENVD(fpenv);
				return (y);
			}
		}
		else if (signy)	{					// y < 0.0
			if (signx) {
				ddx.d[0] = -kPiBy2 ; 	
				ddx.d[1] = -kPiBy2Tail1;
                                FESETENVD(fpenv);
				return (ddx.f);
			}
			else {
				ddx.d[0] = -kPiBy2 ; 	
				ddx.d[1] = -kPiBy2Tail1m;
                                FESETENVD(fpenv);
				return (ddx.f);
			}
		}
		else {								// y > 0.0
			if (signx) {
				ddx.d[0] = kPiBy2 ; 	
				ddx.d[1] = kPiBy2Tail1;	
                                FESETENVD(fpenv);
				return (ddx.f);
			}
			else {
				ddx.d[0] = kPiBy2 ; 	
				ddx.d[1] = kPiBy2Tail1m;
                                FESETENVD(fpenv);
				return (ddx.f);
			}
		}
	}										// [x is zero]
	if ((sigy | expy) == 0u) {				// [zero y, nonzero x]
		if (signx == 0.0) {					// denominator positive		
			ddx.d[0] = yNum;				// correctly signed zero result
			ddx.d[1] = 0.0;
                        FESETENVD(fpenv);
			return (ddx.f);
		} 
		else {								// negative denominator
			if (signy == 0) {				// numerator is 0, 
				ddx.d[0] = kPi ; 	
				ddx.d[1] = kPiTail1m;
                                FESETENVD(fpenv);
				return (ddx.f);
			} 
			else {							// numerator is -0  
				ddx.d[0] = -kPi ; 	
				ddx.d[1] = -kPiTail1m;
                                FESETENVD(fpenv);
				return (ddx.f);
			}
		}
	}										// [zero y, nonzero x]
	// END OF SPECIAL CASES
	
	// The code below is for finite y/x values, we first calculate the ratio y/x
	// for tiny values of either x and y, and for other finite values with extra precision
	// producing xHead, xTail and frac3. These quantities and the sign of the numerator and 
	// the denomitor are used then to produce ArcTan (y,x)
	
	//  Beginning of tiny numerator or denominator
	
	if((((ddy.i[0]) & 0x7fffffff) <  0x06B00000)|| (((ddx.i[0]) & 0x7fffffff) < 0x06B00000)) {
		xDen = xDen*kRescale;				// For a tiny arguments,
		xDenTail = xDenTail*kRescale;		// rescale all the input
		yNum = yNum*kRescale;				// so that the full length
		yNumTail = yNumTail*kRescale;		// division can procede.
		r = 1.0/xDen;
		if (fabs(r) < kTiny) {			// rescaling has made numerator
			if (xDen > 0) {					// too large.  Result is
				xHead = 0.0;				// zero for positive denominator
				xTail = 0.0;
				frac3 = 0.0;
			} 
			else {							// for neg. denominator,
				xHead = yNum;				// send numerator to qatan
				xTail = 0.0;				// result will be subtracted
				frac3 = 0.0;				// from properly signed Pi later
			}
		}
	}
	//  end of tiny numerator or denominator
	
	r = 1.0/xDen;							// start computation of num/den
	// normal non-special values of x and y
	fract = __FMUL( yNum, r );
	
	// A detailed division (yHead,yTail)/(xHead,xTail) follows
	frac = __FMADD( r, __FNMSUB( xDen, fract, yNum ), fract );	// fract + (yNum-xDen*fract)*r;		// refine h.o. quotient
	p29 = __FNMSUB( xDen, frac, yNum ); // yNum - xDen*frac;
	p30 = __FMUL( frac, xDenTail );					// negative term
	p31 = __FNMSUB( frac, xDenTail, p30 ); // p30 - frac*xDenTail;
	p32 = p31;
	p33 = __FSUB( p29, p30 );
	p35 = __FADD( p33, yNumTail );
	
	if (fabs(fract) == kInfinityu.f) {
		xHead = fract;
		xTail = 0.0;
		frac3 = 0.0;
	}
	else if (fabs(fract)< kInfinityu.f) {
		frac2 = __FMUL( p35, r );
		frac2 = __FMADD( r, __FNMSUB( frac2, xDen, p35 ), frac2 ); // frac2 + (p35-frac2*xDen)*r;	// mid. quot. word
		if (fabs(p29) > fabs(p30))
			p34 = p29 - p33 - p30 + p32;
		else 
			p34 = p29 - (p33+p30)+p32;
		p36 = __FNMSUB( frac2, xDen, p35 ) + __FNMSUB( frac2, xDenTail, p34 ); // p35 - frac2*xDen + p34 - frac2*xDenTail;
		if (fabs(p33) > fabs(yNumTail))
			p37 = p33 - p35 + yNumTail;
		else  
			p37 = yNumTail - p35 + p33;
		p38 = p37 + p36;
		frac3 = p38*r/__FMADD( frac, frac, 1.0 ); // p38*r/(1.0+frac*frac);		// Fraction done
		xHead = frac;						// Use ATAN routine to do
		xTail = frac2;						// most of the work
	}
	// Below we use the just evaluated values of xHead and xTail
	// to first evaluate the quantity "ressmall" which add an extra precision
	// contribution to frac3. Then, xHead, xTail, frac3, ressmall and 
	// the sign of the numerator (signy) and denominator (signx)
	// are used to evaluate ArcTan2(y,x)
	
	// finite result
	
	//***************************************************************************
	//      This routine breaks the arguments into five ranges:                 *
	//      Range 1:          0 <= |x| <= .0859375                              *
	//      Range 2:   .0859375 <= |x| <= 1.015625                              *
	//      Range 3:   1.015625 <= |x| < 13                                     *
	//      Range 4:         13 <= |x| < 2^107                                  *
	//      Range 5:      2^107 <= |x| <= Infinity                              *
	//                                                                          *
	//         >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>         *
	//                                                                          *
	// In Range 1, a 15th degree polynomial in x^2 is used to evaluate QATAN.   *
	//                                                                          *
	// In Range 2  and,                                                         *
	// In Range 3  the accurate table method is used to range-reduce the        *
	//             argument to a small value to be used with an interpolating   *
	//             polynomial for QATAN.  In Ranges 2 and 3, the resultant      *
	//             argument is less than 2^-7, so an 8th degree polynomial can  *
	//             be used.  Range 3 requires additional work to find the       *
	//             correct index into the accurate table.                       *
	//                                                                          *
	// In Range 4, QATAN(x) = Pi/2 - QATAN(1/x).  After taking the reciprocal   *
	//             of the argument, the quotient is less than 1/13, so that the *
	//             15th degree polynomial used for Range 1 is used to compute   *
	//             QATAN(1/x)                                                   *
	//                                                                          *
	// In Range 5, the correctly rounded value differs from Pi/2 by less than   *
	//             1/2 ulp.  It is, however, incorrect to let the value become  *
	//             correctly rounded, as that correctly rounded value exceeds   *
	//             PI/2 and is in the wrong quadrant.  Applying the inverse     *
	//             function, QTAN, to the correctly rounded value of PI/2 would *
	//             produce a result of the opposite sign to the argument        *
	//             originally given to ATAN.                                    *
	//***************************************************************************
	
	
	//  THIS CODE ONLY TREATS RANGES 1 TROUGH 5. WE ASSUME THAT xHead and xTail
	//	are such that we are ALWAYS in Range 1-5.
	
	absx = fabs(xHead);
	
	// Range 5
	
	if (absx >= kLimit4) {					// Filter out unusual arguments
		reshi = kPiBy2;						// if the second argument is
		if (signx == 0.0) {					// positive, the result is in
			resbot = kPiBy2Tail1m;			// first or fourth quadrant
			ressmall = 0;					// set up appropriate value of
			if (xHead < 0.0) {				// pi/2, truncated
				reshi = -reshi;
				resbot = -resbot;
			}
		}
		else {								// Otherwise, result is in the
			reshi = kPiBy2;					// second or third quadrant.
			resbot = kPiBy2Tail1;			// set up pi/2, correctly rounded
			ressmall = kPiBy2Tail2;
			if (xHead < 0.0) {
				reshi = -reshi;
				resbot = -resbot;
				ressmall = -ressmall;
			}
		}
	}

	// Ranges 1 and 4 (DO NOT USE TABLE)
	
	rflag = 0;	
	
	// Range 4
	
	if ((absx >= kLimit3) && (absx < kLimit4) ) {	// Range 4
		rarg = 1.0/xHead;                         
		rflag = 1;									// indicate large argument
		p1 = __FNMSUB( xHead, rarg, 1.0 );			// 1.0 - xHead*rarg;						// time to complete hi precision
		p2 = xTail*rarg;							// reciprocal of argument, to
		p3 = p1 - p2;								// use as reduced argument.
		rarg2a = p3*rarg;
		rarg2 = rarg2a + __FNMSUB( xHead, rarg2a, p3 )*rarg; // rarg2a + (p3 - xHead*rarg2a)*rarg;
		xHead = rarg;
		xTail = rarg2;
	}
	
	// Ranges 1 and 4 only
	
	if( (absx < kLimit1) || ((absx >= kLimit3) && (absx < kLimit4) )) {	//Code for Range 1 and Range 4
		temp = 2.0*xHead*xTail;						// direct evaluation of series
		argsq = __FMADD( xHead, xHead, temp );		// xHead*xHead + temp;					// compute square of argument
		argsq2 = __FMADD( xTail, xTail, __FMSUB( xHead, xHead, argsq ) + temp ); // xHead*xHead - argsq + temp + xTail*xTail;
		sum = atancoeff[14];
		sum = __FMADD( argsq, sum, atancoeff[13] );			// atancoeff[13] + argsq*sum;
		sum = __FMADD( argsq, sum, atancoeff[12] );			// atancoeff[12] + argsq*sum;
		sum = __FMADD( argsq, sum, atancoeff[11] );			// atancoeff[11] + argsq*sum;
		sumt = __FMADD( argsq, sum, atancoeff[10] );		// atancoeff[10] + argsq*sum;
		sum = __FMADD( argsq, sum, atancoeff[9] );			// atancoeff[9] + argsq*sumt;
		suml = __FMADD( argsq, sumt, atancoeff[9] - sum );	// atancoeff[9] - sum + argsq*sumt;
		
		for (i = 8; i >= 1; i--) {	 		   
			temps = __FMADD( argsq, sum, atancoeff[i] );	// atancoeff[i] + sum*argsq;
			/* suml = atancoeff[i] - temps+sum*argsq + (sum*argsq2 + suml*argsq); */
			suml = __FMADD( sum, argsq, atancoeff[i] -  temps ) + __FMADD( sum, argsq2, suml*argsq );
			sum=temps;
		}
		
		prodlow = __FMADD( suml, argsq, sum*argsq2 );		// suml*argsq + sum*argsq2;			// multiply by arg^2
		prod = __FMADD( sum, argsq, prodlow );				// sum*argsq + prodlow;
		prodlow = __FMSUB( sum, argsq, prod ) + prodlow;	// sum*argsq - prod + prodlow;
		temp2 = __FMADD( prodlow, xHead, prod*xTail );		// prodlow*xHead + prod*xTail;		// multiply by arg
		temp = __FMADD( prod, xHead, temp2 );				// prod*xHead + temp2;
		temp2 = __FMSUB( prod, xHead, temp ) + temp2;		// prod*xHead - temp + temp2;
		sum = temp*kRecip;																		// sum of series ---
		remainder = __FNMSUB( sum, atancoeff[0], temp );	// (temp - sum*atancoeff[0]);
		partial = __FADD( remainder, temp2 );
		residual = remainder - partial + temp2;
		suml = __FMADD( partial, kRecip, (residual*kRecip) );// partial*kRecip + (residual*kRecip);
		res = __FADD( xHead, sum );																// except for argument
		reslow = (xHead - res + sum);															// exact
		resmid = __FADD( xTail, suml );
		restiny = xTail - resmid + suml;
		p = __FADD( reslow, resmid );															// sum of middle terms
		q = reslow - p + resmid;																// possible residual
		reshi = res + p;
		resbot = (res - reshi + p) + (q + restiny);
		if (rflag == 1) {							// large magnitude argument
			if (xHead > 0) {						//  fetch pi/2 with proper sign
				p1 = kPiBy2;
				p2 = kPiBy2Tail1;
				p2a = kPiBy2Tail2;
			}
			else {
				p1 = -kPiBy2;
				p2 = -kPiBy2Tail1;
				p2a = -kPiBy2Tail2;
			}
			p3 = __FSUB( p1, reshi );				// subtract result from pi/2
			p4 = p1 - p3 - reshi;
			p5 = __FSUB( p2, resbot );
			p6 = p2 - p5 - resbot + p2a;
			p7 = __FADD( p4, p5 );
			reshi = p3 + p7;
			if (fabs(p4) > fabs(p5)) 
				p8 = p4 - p7 + p5;
			else
				p8 = p5 - p7 + p4;
			resbot = __FADD( p3 - reshi + p7, (p6+p8-restiny) );
			ressmall = (p3 - reshi + p7) - resbot + (p6 + p8 - restiny);
		}
		else   //if rflag = 0   i.e., Range 1
			ressmall = (res - reshi + p) - resbot + (q + restiny);
	}
	
	// Ranges 2 and 3  (USE TABLE)
	
	if( (absx >= kLimit1) && (absx < kLimit3) ) {	// Code for Range 2 and Range 3
		DeN.f = __FMADD( 64.0, absx, kBig );		// 64.0 *absx+kBig;
		if (absx > kLimit2) {						// for large args, use recip.  Range 3
			FESETENVD(kBig+1.0);						// use trunc mode momentarily
			DeN.f = absx + kBig;					// determine range of argument
			FESETENVD(kBig+1.0);						// use trunc mode momentarily
			DeN.f = absx + kBig;					// determine range of argument
			FESETENVD(kBig);							// return to round-to-nearest
			index = DeN.i[1];
			i = index - 1;
			DeN.f = __FMADD( factor[i], absx, kBig+adjust[i] ); // factor[i]*absx+kBig+adjust[i];
		}
		index = DeN.i[1];
		if (xHead < 0) {							// use abs. value
			arg = -xHead;
			argl = -xTail;
		}
		else {
			arg = xHead;
			argl = xTail;
		}
		top = __FSUB( arg, atanPtr[(int32_t)index].One );
		top2 = __FADD( top, argl );
		topextra = arg - top - atanPtr[(int32_t)index].One;
		top3 = top - top2 + argl + topextra;		// reduced numerator
		bot = __FMADD( arg, atanPtr[(int32_t)index].One, 1.0 );	 // 1.0 + arg*atanPtr[(int32_t)index].One;
		/* bot2 = 1.0 - bot + arg*atanPtr[(int32_t)index].One + argl*atanPtr[(int32_t)index].One; */ // denominator
		bot2 = __FMADD( argl, atanPtr[(int32_t)index].One, __FMADD( arg, atanPtr[(int32_t)index].One, 1.0 - bot ) );
		r = 1.0/bot;
		quot = r*top2;
		quot = __FMADD( __FNMSUB( bot, quot, top2 ) , r, quot ); // quot + (top2-bot*quot)*r;
		
		p29 = __FNMSUB( bot, quot, top2 );						//top2 - quot*bot;
		p30 = quot*bot2;							// negative term
		p33 = p29-p30;
		p35 = p33 + top3;
		quot2 = p35*r;
		quot2 = __FMADD( __FNMSUB( quot2, bot, p35 ), r, quot2 ); // quot2 + (p35-quot2*bot)*r;			// second fraction wd.
		temp = 2.0*quot*quot2;						// reduced arg is (quot,quot2)
		argsq = __FMADD( quot, quot, temp );			// quot*quot + temp;				// compute square of argument
		argsq2 = __FMADD( quot2, quot2, __FMSUB( quot, quot, argsq) + temp ); // quot*quot - argsq + temp + quot2*quot2;
		sumt = __FMADD( argsq, atancoeff[7], atancoeff[6] );// atancoeff[6] + argsq*atancoeff[7];
		sum = __FMADD( argsq, sumt, atancoeff[5] );		// atancoeff[5] + argsq*sumt;
		suml = __FMADD( argsq, sumt, atancoeff[5] - sum ); // atancoeff[5] - sum + argsq*sumt;
		
		for (i = 4; i >= 1; i--) {	 	
			temps = __FMADD( sum, argsq, atancoeff[i] );	// atancoeff[i] + sum*argsq;
			/* suml = atancoeff[i] - temps + sum*argsq + (sum*argsq2 + suml*argsq); */
			suml = __FMADD( sum, argsq, atancoeff[i] - temps ) + __FMADD( sum, argsq2, suml*argsq );
			sum = temps;
		}
		
		prodlow = __FMADD( suml, argsq, sum*argsq2 );		// suml*argsq + sum*argsq2;			// mult. by arg^2
		prod = __FMADD( sum, argsq, prodlow );				// sum*argsq + prodlow;
		prodlow = __FMSUB( sum, argsq, prod ) + prodlow;		// sum*argsq - prod + prodlow;
		temp2 = __FMADD( prodlow, quot, prod*quot2 );		// prodlow*quot + prod*quot2;		// mult. by arg
		temp = __FMADD( prod, quot, temp2 );				// prod*quot + temp2;
		temp2 = __FMSUB( prod, quot, temp ) + temp2;		// prod*quot - temp + temp2;
		sum = temp*kRecip;																		// sum of series ---
		remainder = __FNMSUB( sum, atancoeff[0], temp );		// (temp - sum*atancoeff[0]);
		partial = __FADD( remainder, temp2 );
		residual = remainder - partial + temp2;
		suml = __FMADD( partial, kRecip, (residual*kRecip) ); // partial*kRecip + (residual*kRecip);
		res = __FADD( quot, sum );																// except for argument
		reslow = (quot - res + sum);															// exact
		resmid = __FADD( quot2, suml );
		restiny = quot2 - resmid + suml;
		p = __FADD( reslow, resmid );															// sum of middle terms
		q = reslow - p + resmid;																// possible residual
		reshi = __FADD( res, p );
		resbot = (res - reshi + p) + (q + restiny);
		result = __FADD( atanPtr[(int32_t)index].Two, reshi );
		reslow = __FADD( atanPtr[(int32_t)index].Three, resbot );
		resmid = atanPtr[(int32_t)index].Two - result + reshi;
		restiny = resbot - reslow + atanPtr[(int32_t)index].Three;
		restemp = __FADD( resmid, reslow );
		reshi = __FADD( result, restemp );
		
		if (fabs(resmid) > fabs(reslow)) 
			restiny = resmid - restemp + reslow + restiny;
		else
			restiny = reslow - restemp + resmid + restiny;
		resbot = result - reshi + restemp + restiny;
		if (xHead < 0.0) {
			reshi = -reshi;
			resbot = -resbot;
			result = -result;
			restemp = -restemp;
			restiny = -restiny;
		}
		ressmall = (result - reshi + restemp) - resbot + restiny;
	}
	extra = ressmall + frac3;
	if (signx == 0) {								// For Positive denominator
		ddx.d[0] = __FADD( reshi, (resbot + extra) );  
		ddx.d[1] = (reshi - ddx.d[0]) + (resbot + extra);					  
	}
	else {
		if (signy == 0) {							// For Negative denominator
			pin = kPi;								// return 2nd|3rd quadrand angle,
			pin2 = kPiTail1;						// depending on sign of numerator
			pin3 = kPiTail2;
		}
		else {
			pin = -kPi;								// -->  Pi+atan, for dnum < 0,  
			pin2 = -kPiTail1;						//      atan-Pi, for dnum > 0
			pin3 = -kPiTail2;
		}
		restop = __FADD( pin, reshi );
		resmid = pin - restop + reshi;
		reslow = __FADD( pin2, resbot );
		restiny = pin2 - reslow + resbot + extra;
		resint = __FADD( resmid, reslow );
		reshi = __FADD( restop, resint );
		resbot = restop - reshi+resint;
		if (fabs(resmid) > fabs(reslow)) 
			resbot = resbot + (resmid-resint+reslow+restiny);
		else 
			resbot = resbot + (reslow-resint+resmid+restiny);
		ddx.d[0] = __FADD( reshi, resbot );  
		ddx.d[1] = (reshi - ddx.d[0]) + resbot;		
	}

        FESETENVD(fpenv);
	return (ddx.f);
}
#endif
	
