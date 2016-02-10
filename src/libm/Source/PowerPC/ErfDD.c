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
//	ErfDD.c
//
//	Double-double Function Library
//	Copyright: © 1995-96 by Apple Computer, Inc., all rights reserved
//	
//	long double erfl( long double x );
//	long double erfcl( long double x );
//

#include "math.h"
#include "fp_private.h"
#if (__WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L)
#include "DD.h"

// Requires the following functions:

void _d3div(double a, double b, double c, double d, double e, double f,
			double *high, double *mid, double *low);
void _d3mul(double a, double b, double c, double d, double e, double f,
			double *high, double *mid, double *low);
void _d3add(double a, double b, double c, double d, double e, double f,
			double *high, double *mid, double *low);
long double _ExpInnerLD(double alpha, double beta, double gamma,
			double *extra, int exptype);

//  real*8 erfxtr(0:2)
//  FORTRAN: erfxtr(i)    C: erfxtr[i]
static const uint32_t erfxtrData[] __attribute__ ((aligned(8))) = {
	0x00000000, 0x00000000,
	0xB9155555, 0x55555555,
	0x38F99999, 0x9999999A
};

//  real*8 erfpow(1:2, 0:21)
//  FORTRAN: erfpow(i,j)   C: erfpow[j][i-1]
static const uint32_t erfpowData[] __attribute__ ((aligned(8))) = {
	0x3FF00000, 0x00000000,   0x00000000, 0x00000000,
	0xBFD55555, 0x55555555,   0xBC755555, 0x55555555,
	0x3FB99999, 0x9999999A,   0xBC599999, 0x9999999A,
	0xBF986186, 0x18618618,   0xBC386186, 0x18618613,
	0x3F72F684, 0xBDA12F68,   0x3C12F684, 0xBDA15772,
	0xBF48D301, 0x8D3018D3,   0xBB88D301, 0xBA2FF990,
	0x3F1C01C0, 0x1C01C01C,   0x3B5C01FF, 0xEF4E45A0,
	0xBEEBBD77, 0x9334EF0B,   0x3B84E5AE, 0x2FF08B10,
	0x3EB87A00, 0x187A0018,   0x3B5EC7BC, 0xA752F683,
	0xBE83777C, 0x55568CCD,   0xBB29453D, 0xDF9ADD3E,
	0x3E4C2E30, 0x54870B52,   0xBADB98FC, 0x71C71C77,
	0xBE12B673, 0x10AAA27C,   0x3AACE36B, 0x74F03296,
	0x3DD6F448, 0xE13F19E7,   0xBA7D15ED, 0x097B425F,
	0xBD9A289E, 0xE7F6D02A,   0xBA21F9AD, 0xD3C0CA47,
	0x3D5BD577, 0xE7FBCC09,   0xB9E48B0F, 0xCD6E9E08,
	0xBD1BC625, 0x25C514CA,   0xB9B161F9, 0xADD3C0CA,
	0x3CDA173A, 0x4FC330FD,   0xB9648B0F, 0xCD6E9E03,
	0xBC97270A, 0xC344BDA1,   0xB927B425, 0xED097B45,
	0x3C537610, 0xC735BA78,   0x38D948B0, 0xFCD6E9DE,
	0xBC0EF15A, 0xDD3C0CA4,   0xB8A61F9A, 0xDD3C0CA5,
	0x3BC67194, 0x8B0FCD6F,   0xB8687E6B, 0x74F03292,
	0xBB778E38, 0xE38E38E4,   0x381C71C7, 0x1C71C71C
};

//  real*8 coeff(2,0:28,8)
//  FORTRAN: coeff(i,j,k)   C: coeff[k][j][i-1]
static const uint32_t coeffData[] __attribute__ ((aligned(8))) = {
	0x3FEFFFFF, 0xF87B4F81,   0xBC74B6DC, 0x82E2B33B,
	0xBFDFFFF9, 0x651B9CCD,   0xBC77993A, 0x7E74C502,
	0x3FE7FF4C, 0x63E2B209,   0x3C607D84, 0x888EE934,
	0xBFFDF3AD, 0x307BCFA0,   0x3C80189F, 0x21239DB8,
	0x4019F11D, 0x8747B2EA,   0x3CB282E6, 0x775914EA,
	0xC03BFD72, 0x2186F691,   0x3CD1B8A8, 0xA64D28F3,
	0x406123E2, 0xF4070A2E,   0xBD0AA69F, 0x832442C8,
	0xC085E672, 0xCAE6452C,   0xBD264E3F, 0x5B4D5E70,
	0x40AB48F2, 0xAF9DF505,   0x3D3CFDE8, 0x35619194,
	0xC0CF99E0, 0xAC6AFA01,   0xBD525B2E, 0x7AEBA4B8,
	0x40F07ABD, 0x450474EE,   0x3D9F4814, 0x21020E9F,
	0xC10E4BB4, 0xEAA37C6D,   0x3DADDD80, 0x6CA8C8F2,
	0x41282796, 0x01A20544,   0x3DC2C51E, 0x7D2AD0DC,
	0xC1407A5E, 0xA1893145,   0xBDEB8D59, 0x4611C6D5,
	0x4152F9A4, 0x9A0ED425,   0xBDF53C28, 0x002DF666,
	0xC16227D6, 0x896B5A9A,   0x3DD50BAE, 0xCA782EDC,
	0x416C4D5B, 0x1A0268E7,   0xBDE19C5A, 0x4D09E56C,
	0xC1717E27, 0xC2AB2A36,   0x3E122CE5, 0xA75631BC,
	0x41707C32, 0x89EC6733,   0xBE1F4B89, 0xEF2106D9,
	0xC1663DCE, 0x61AF76F4,   0x3E028503, 0xD6CDBB9E,
	0x41531F97, 0x0C38EEF3,   0xBDE20467, 0x5FB2BCF0,
	0xC12F7BE6, 0x257D1D65,   0x3D8DFD02, 0xE8C06000,
	0x00000000, 0x00000000,   0x00000000, 0x00000000,
	0x00000000, 0x00000000,   0x00000000, 0x00000000,
	0x00000000, 0x00000000,   0x00000000, 0x00000000,
	0x00000000, 0x00000000,   0x00000000, 0x00000000,
	0x00000000, 0x00000000,   0x00000000, 0x00000000,
	0x00000000, 0x00000000,   0x00000000, 0x00000000,
	0x00000000, 0x00000000,   0x00000000, 0x00000000,
	
	0x3FEFFFFF, 0xFFEBC039,   0x3C529C69, 0x6CF540A8,
	0xBFDFFFFF, 0xE38F77CF,   0x3BFCDCBB, 0x0CD77D00,
	0x3FE7FFFB, 0x2DCCB103,   0x3C8175D4, 0xB96461CA,
	0xBFFDFF79, 0x49317263,   0xBC906876, 0xAC321F5A,
	0x401A3AAB, 0x9BF9D0BD,   0x3CABAD03, 0xD57E6058,
	0xC03D5E32, 0x130E26B3,   0xBC9E17BB, 0x1F622DC0,
	0x4063C8DE, 0x39F025C5,   0x3CEE695F, 0x35A91AF0,
	0xC08E3E6E, 0x953DCEB7,   0x3D24E053, 0xE29EAEBE,
	0x40B8AA52, 0x3AD56173,   0x3D50EDEF, 0x7FD5BCF1,
	0xC0E440E6, 0xCEADBFBB,   0x3D71A20A, 0x51AF181A,
	0x410FEE4B, 0xDCA4E1E0,   0x3D8CCED6, 0xFF021BF0,
	0xC13752BC, 0xAAA09717,   0xBDDB430A, 0x583D9DFE,
	0x415EC821, 0x1EEE2921,   0x3DDA8E50, 0xAC97C8D8,
	0xC182031E, 0xEDA7A8C5,   0x3E23FDA5, 0x816A17A9,
	0x41A26A35, 0x43351D14,   0x3E4A1C99, 0x4FE13BE4,
	0xC1C0397D, 0x3FF8253E,   0xBE55B81D, 0xBAD43C68,
	0x41D84B8E, 0x7B8414AB,   0x3E797BE5, 0x69A73302,
	0xC1EE6B6B, 0x349E71AE,   0xBE7EB248, 0x45D29614,
	0x41FF34DA, 0xF64919BF,   0xBE87460E, 0x4926C028,
	0xC20984E7, 0x864FAEBA,   0x3EA1DD11, 0x07234030,
	0x420FF82D, 0xCF64D788,   0x3E9B32FC, 0x90159470,
	0xC20CCAA6, 0x72BB40E4,   0x3E619A97, 0x18E25640,
	0x420096A8, 0x68135449,   0xBEAFFD28, 0xED6BBBFA,
	0xC1E25D70, 0xCAB7C34C,   0xBE5B42DB, 0xEC71D440,
	0x00000000, 0x00000000,   0x00000000, 0x00000000,
	0x00000000, 0x00000000,   0x00000000, 0x00000000,
	0x00000000, 0x00000000,   0x00000000, 0x00000000,
	0x00000000, 0x00000000,   0x00000000, 0x00000000,
	0x00000000, 0x00000000,   0x00000000, 0x00000000,
	
	0x3FEFFFFF, 0xFFFFE639,   0xBC76B4F8, 0xF22A1D6A,
	0xBFDFFFFF, 0xFFC4926E,   0x3C74AA2B, 0x05602AA4,
	0x3FE7FFFF, 0xEF8F13A8,   0xBC802572, 0x96640478,
	0xBFFDFFFD, 0x175A90A3,   0x3C8448AC, 0x5447AC1C,
	0x401A3FD0, 0xA3F30955,   0x3CBDC702, 0x0AF0E74C,
	0xC03D85B0, 0xDE15683B,   0x3CD775BC, 0x8D3FFDA1,
	0x406441FD, 0xA9731CF2,   0xBCEC6B02, 0xE82A19B0,
	0xC0904FCE, 0xB4270577,   0x3CF73CA2, 0xBC7037A0,
	0x40BDA90A, 0xA9B95F59,   0xBD5B5C9C, 0x5ABA6422,
	0xC0ED1C94, 0x9666E63C,   0xBD5C789C, 0x5A7928DC,
	0x411D56F4, 0x20DB5218,   0x3DB320CC, 0xDE074D1E,
	0xC14CEC6A, 0x9FD76B11,   0x3DE9DE29, 0x03D42F2C,
	0x417AC1F5, 0xF5446DF4,   0xBE031819, 0x3468BE58,
	0xC1A67958, 0xAC7171B2,   0xBE3581A4, 0xB51D7A74,
	0x41D0B077, 0x6E96B0B2,   0x3E574081, 0x998F5AA4,
	0xC1F56C00, 0xFD9F8D53,   0xBE94D297, 0xFD7D3E08,
	0x42173F0C, 0x0EB8D9D7,   0x3EBE2714, 0x71045A4A,
	0xC234D11D, 0x4DA55FFA,   0x3EC4EA18, 0x03DADE62,
	0x424DDAFE, 0x5B2DFE46,   0x3EE6D6D1, 0x58677012,
	0xC2607209, 0x5B35C7C9,   0x3F04E6A4, 0x0B2429BA,
	0x426A1766, 0x45D71E26,   0xBF0B66BC, 0xFE7F12C0,
	0xC26A80F6, 0x5FAAB021,   0x3F042804, 0xC0D4DFB5,
	0x4259DF2B, 0x9341D4CC,   0xBEE2816A, 0x58CBD636,
	0x00000000, 0x00000000,   0x00000000, 0x00000000,
	0x00000000, 0x00000000,   0x00000000, 0x00000000,
	0x00000000, 0x00000000,   0x00000000, 0x00000000,
	0x00000000, 0x00000000,   0x00000000, 0x00000000,
	0x00000000, 0x00000000,   0x00000000, 0x00000000,
	0x00000000, 0x00000000,   0x00000000, 0x00000000,
	
	0x3FF00000, 0x00000000,   0xBB75CA29, 0xC48EA000,
	0xBFE00000, 0x00000000,   0x3C1A999F, 0xD44FCE00,
	0x3FE7FFFF, 0xFFFFFFFE,   0x3C642A4A, 0xB64D8068,
	0xBFFDFFFF, 0xFFFFFE99,   0xBC9ABDD0, 0x98308E2C,
	0x401A3FFF, 0xFFFFA343,   0x3CB85B18, 0x87B93172,
	0xC03D87FF, 0xFFEDDBFB,   0x3CD309B5, 0xEEE35E41,
	0x40644D7F, 0xFE9A5199,   0x3CFC5B15, 0x3519E834,
	0xC0907EF7, 0xE9B322F4,   0xBD35FCB1, 0x58754A9E,
	0x40BEEE0E, 0xB41A926D,   0xBD57FB38, 0x347B51CF,
	0xC0F06E6C, 0x572C5E36,   0x3D720A1F, 0x8B0D1254,
	0x412382B8, 0xB55E07E1,   0xBDCABBAE, 0x93440AE8,
	0xC1599872, 0x5E40B785,   0x3DF51615, 0xACBF5F19,
	0x41925B4B, 0x43808517,   0xBE3C17EF, 0x5CEF40F8,
	0xC1CC74A4, 0x3D911FB7,   0x3E172705, 0xD877BCC0,
	0x42077587, 0x54FEECDC,   0xBEA98313, 0x07804B47,
	0xC2441B83, 0xD7BACF31,   0xBEEA3073, 0x0D0DBA05,
	0x428165FB, 0x78789F6C,   0xBED45B8E, 0xD3B8907C,
	0xC2BD68C3, 0xB0580084,   0x3F1BFE68, 0xE7B64DE0,
	0x42F78115, 0xFD9904B1,   0xBF8FED1E, 0x29D74809,
	0xC3313FA3, 0xDBF81756,   0x3FDC570B, 0xC88AFF19,
	0x4366A5A6, 0xB262F5E4,   0x3FFA9257, 0x0A3AD3F8,
	0xC399F8EF, 0xB5A4E015,   0x401B7ECE, 0x1B301370,
	0x43C9687F, 0xB88D7E83,   0xC06CE8BA, 0x85792A6C,
	0xC3F4A8E4, 0x451D5574,   0x409B5C8E, 0x70046666,
	0x441B0DCA, 0xB2FAB836,   0x40996671, 0xF8BB2152,
	0xC43B52DB, 0xC1E7B2EE,   0x40DB5AF6, 0x615EB7EE,
	0x4453ECD3, 0xF252D951,   0x40FC199B, 0x4FAD14EA,
	0xC462A264, 0xDCCB32FF,   0xC0E17D1D, 0x52615E34,
	0x4460C115, 0xAC203924,   0x40E780D0, 0xA1F70E00,
	
	0x3FF00000, 0x00000000,   0x3A9A41C2, 0x18000000,
	0xBFE00000, 0x00000000,   0xBB5DCD07, 0x8C200000,
	0x3FE80000, 0x00000000,   0x3C0EBA46, 0xBCD71000,
	0xBFFE0000, 0x00000001,   0xBC88EE2A, 0x6486B1E4,
	0x401A4000, 0x00000101,   0x3CB25362, 0xC332A010,
	0xC03D8800, 0x00009B65,   0x3CD119CB, 0x15915886,
	0x40644D80, 0x0022EC9B,   0xBCE9E9C4, 0x0897A1AC,
	0xC0907EF8, 0x05F94723,   0x3D2AB506, 0xB989F798,
	0x40BEEE12, 0x939C8382,   0x3D5AD230, 0x743AEFD8,
	0xC0F06E8D, 0xB86AB92B,   0x3D98E5AE, 0xBE0C9174,
	0x412384D6, 0x161AD32F,   0x3DBB5BF4, 0xEF6A7DBF,
	0xC159B642, 0x7281154F,   0x3DFECD4C, 0xAA7F47EC,
	0x419305F9, 0xC7BF95B4,   0xBE349B0E, 0x9D9581DE,
	0xC1D12C24, 0x5D3084BD,   0xBE5E4995, 0xB96B303C,
	0x4215451C, 0xDF186F15,   0x3EA8548E, 0x669919A8,
	0xC25F3153, 0x01C60944,   0x3ED326BD, 0x842F8568,
	0x42A25C17, 0xD2287EF6,   0x3F493E23, 0x133CA4B2,
	0xC2D613F5, 0x225EEB4A,   0x3F4A86AC, 0x4B33D440,
	0x00000000, 0x00000000,   0x00000000, 0x00000000,
	0x00000000, 0x00000000,   0x00000000, 0x00000000,
	0x00000000, 0x00000000,   0x00000000, 0x00000000,
	0x00000000, 0x00000000,   0x00000000, 0x00000000,
	0x00000000, 0x00000000,   0x00000000, 0x00000000,
	0x00000000, 0x00000000,   0x00000000, 0x00000000,
	0x00000000, 0x00000000,   0x00000000, 0x00000000,
	0x00000000, 0x00000000,   0x00000000, 0x00000000,
	0x00000000, 0x00000000,   0x00000000, 0x00000000,
	0x00000000, 0x00000000,   0x00000000, 0x00000000,
	0x00000000, 0x00000000,   0x00000000, 0x00000000,
	
	0xBFA4AFAB, 0x9B6E1105,   0x3C463A99, 0xD0AF98EE,
	0xBFD4F526, 0x85059209,   0xBC73ECA6, 0xE2948B70,
	0xBFEA28C7, 0xA68495F3,   0xBC81EAF6, 0x808C4753,
	0xBFA980E7, 0x79855BE9,   0x3C30F43B, 0xBDDFAE98,
	0x3F8895D7, 0xF2B826D4,   0x3C250FDF, 0xAF15221E,
	0xBF6222D1, 0xDE8EFFF2,   0x3BFC3D53, 0x8B04AE74,
	0x3F26522D, 0x3A826B8E,   0xBBBFB440, 0x1723702C,
	0x3F15289F, 0x67803BEA,   0x3BBA9594, 0x12B030A4,
	0xBF08627F, 0xFCF37770,   0x3BAD7A54, 0xCCB615F2,
	0x3EEBEB7F, 0xBC45B14A,   0xBB715091, 0x8DB6D7B4,
	0xBEBE0FDE, 0x30CC0705,   0xBB3ED352, 0x5BE0D640,
	0xBE9B7052, 0x9A2F9B59,   0xBB29B1CC, 0xF66FE0B8,
	0x3E97B4BD, 0x41755DF3,   0xBB10EE4A, 0xCBA6B5AC,
	0xBE807536, 0x0AA20FDE,   0xBAE9E0B1, 0xF22797C0,
	0x3E57D20D, 0xE4C07B40,   0x3AFADF83, 0x4F8A74E3,
	0x3E1E7ADC, 0x1A5BC732,   0xBAACBD1B, 0xDB944806,
	0xBE29CB52, 0xFF49FDEF,   0xBACE7432, 0x1EF280D9,
	0x3E14D367, 0x091F84DD,   0x3ABD9BF9, 0x8C93A4B0,
	0xBDF32CB0, 0xA2D996E6,   0x3A947D3D, 0xF3852A64,
	0x3DC1332B, 0x63A393F5,   0xBA666775, 0x70308F4A,
	0x00000000, 0x00000000,   0x00000000, 0x00000000,
	0x00000000, 0x00000000,   0x00000000, 0x00000000,
	0x00000000, 0x00000000,   0x00000000, 0x00000000,
	0x00000000, 0x00000000,   0x00000000, 0x00000000,
	0x00000000, 0x00000000,   0x00000000, 0x00000000,
	0x00000000, 0x00000000,   0x00000000, 0x00000000,
	0x00000000, 0x00000000,   0x00000000, 0x00000000,
	0x00000000, 0x00000000,   0x00000000, 0x00000000,
	0x00000000, 0x00000000,   0x00000000, 0x00000000,
	
	0xBFD09B84, 0xB58C74B1,   0x3C649EAC, 0xBE7125A0,
	0xBFE04818, 0x9E7A4AF5,   0xBC7247BB, 0x9C14B6B2,
	0xBFEC9082, 0x7462A177,   0xBC65A4D5, 0x2B14D05A,
	0xBF9A71A4, 0xBDE00D7D,   0xBC3D297D, 0x6BADB471,
	0x3F789A87, 0xB48FE138,   0x3BF9AE10, 0x1027D54D,
	0xBF549926, 0x2376DBFA,   0x3BDC0F4D, 0x2A750CAB,
	0x3F2C4E86, 0xAA32FCDB,   0xBBCBD7A9, 0xE43684BF,
	0xBEF790BC, 0x075477C9,   0x3B89898A, 0xADCD6E9C,
	0xBEC63409, 0x8491E3A5,   0xBB572A20, 0x12F684BF,
	0x3EC3E354, 0x0DCFCA75,   0x3B485BE3, 0x87E6B74C,
	0xBEAB790E, 0xE236BFDB,   0x3B40DEF2, 0x9161F9AE,
	0x3E89DA01, 0x98D7CE6C,   0xBB2327A2, 0xC3F35BA6,
	0xBE5DEF8D, 0x200AB94C,   0x3AF95BA7, 0x81948B10,
	0xBE129E35, 0x62080668,   0x3AB6F684, 0xBDA12F68,
	0x3E24ACF9, 0x5B57BC54,   0x3AC55555, 0x55555556,
	0xBE10CB5E, 0x9DE40846,   0x3AAC71C7, 0x1C71C71F,
	0x3DF1C294, 0xD79A2DA8,   0xBA9F9ADD, 0x3C0CA45A,
	0xBDC806EC, 0x95F17FF3,   0xBA66E9E0, 0x6522C3F3,
	0x3D63D790, 0x6BE97B42,   0x3A07B425, 0xED097B40,
	0x3D8A0F87, 0x44E09E06,   0x3A248B0F, 0xCD6E9E04,
	0xBD780C79, 0x96291620,   0x3A1948B0, 0xFCD6E9E0,
	0x3D5B9C07, 0xD9F03291,   0x39F87E6B, 0x74F03291,
	0xBD34D565, 0xE06522C4,   0x39A948B0, 0xFCD6E9DF,
	0x3CF1D800, 0xD6E9E065,   0x398161F9, 0xADD3C0C9,
	0x3CF13B9A, 0xDD3C0CA4,   0x39961F9A, 0xDD3C0CA4,
	0xBCE41FE1, 0xF9ADD3C1,   0x397ADD3C, 0x0CA45880,
	0x3CC7D948, 0xB0FCD6EA,   0xB94F9ADD, 0x3C0CA458,
	0x00000000, 0x00000000,   0x00000000, 0x00000000,
	0x00000000, 0x00000000,   0x00000000, 0x00000000,
	
	0xBF7E4EAA, 0x825F8588,   0x3C0E9883, 0x7D311FB0,
	0xBF9B20DB, 0x38454E52,   0x3C282460, 0xA5A22510,
	0xBFE92D49, 0x5DE062C7,   0x3C842242, 0x550B45F8,
	0xBFAE856A, 0x106722F3,   0xBC3E5242, 0x6EC1567E,
	0x3F8CF900, 0xD71B5137,   0xBC29B9D7, 0x4D921F9C,
	0xBF630F14, 0x2DC98724,   0x3BEFD901, 0xC6097B43,
	0x3EE6A533, 0x044E582C,   0x3B653307, 0x1C71C6A0,
	0x3F261616, 0xBB9808F2,   0x3BC00838, 0xA4587E6A,
	0xBF12A42E, 0x193904D9,   0x3B7FAA5E, 0xD097B432,
	0x3EEFFC81, 0x1FC57602,   0xBB657CD6, 0xE9E0651E,
	0x3E8108C1, 0x35437559,   0xBAFF9ADD, 0x3C0CA484,
	0xBEBA9B0B, 0xE9B78F13,   0x3B555555, 0x55555554,
	0x3EA7C3AD, 0xEB179410,   0xBB3948B0, 0xFCD6E9DF,
	0xBE853AAC, 0x329694F0,   0xBB1948B0, 0xFCD6E9E2,
	0xBE27591E, 0x10C71C72,   0x3ABC71C7, 0x1C71C700,
	0x3E53D507, 0xBCC71C72,   0xBAEC71C7, 0x1C71C720,
	0xBE4220D2, 0xDA12F685,   0x3AE097B4, 0x25ED097C,
	0x3E2224EC, 0xBDA12F68,   0x3AC2F684, 0xBDA12F68,
	0xBDF19BC0, 0xCA4587E7,   0x3A922C3F, 0x35BA781A
	// Last block shorter to save space
};

typedef double DD[2];
typedef DD DDarray[29];

static const Double ca = {{0x3FF20DD7, 0x50429B6D}};
static const Double cb = {{0x3C71AE3A, 0x914FED80}};
static const Double cc = {{0xB903CBBE, 0xBF65F145}};

static long double ErfCommon( double dhead, double dtail, int ierf )
{
	double temp, temp2, temp3, argsq, argsq2, sum, suml, suminf, reslow, resmid, restiny;
	double q, p, prodlow, addend, res, prod1,prod2,px, center, adjust, times;
	double oarg, oarg2, a1, a2, a3, ab, ac, asq1,asq2,asq, arg2,argx;
	double rx, arg, as, asx, resx, qx, exp;
	DblDbl resdd;
	int i, ir, is, ie;
	
	double *erfxtr = (double *)erfxtrData;
	DD *erfpow = (DD *)erfpowData;
	DDarray *coeff = (DDarray *)(coeffData - 116);

	if (fabs(dhead) <= 0.75) {							// power series expansion
	
		temp = (2.0*dhead)*dtail;																			// series is odd series-
		argsq = __FMADD( dhead, dhead, temp );				// dhead*dhead + temp;							// most of expansion is
		argsq2 = __FMSUB( dhead, dhead, argsq + temp ) +	// dhead*dhead - argsq+temp +
				__FMSUB( (2.0*dhead), dtail, temp );		// ((2.0*dhead)*dtail-temp);					// is in terms of arg^2
		sum = erfpow[21][0];																				// sum of series init
		suml = erfpow[21][1];
		for (i = 20; i >= 3; --i) {																			// all but the 3 h.o.
			temp = __FMADD( sum, argsq, erfpow[i][0] );		// erfpow[i][0] + sum*argsq;					// terms executed in
																											// q&d quad precision
			/* suml = erfpow[i][0] - temp + sum*argsq +
					(erfpow[i][1] + sum*argsq2 + suml*argsq); */
			suml = __FMADD( sum, argsq, erfpow[i][0] - temp ) +
					__FMADD( suml, argsq, __FMADD( sum, argsq2, erfpow[i][1] ) );
			sum = temp;
		}
		suminf = 0.0e0;
		for (i = 2; i >= 0; --i) {
			prodlow = __FMADD( suml, argsq, (sum*argsq2) );	// (suml*argsq) + (sum*argsq2);					// mult. by arg^2
			/* addend = (suml*argsq - (suml*argsq)) + (sum*argsq2 - (sum*argsq2)) +
				((suml*argsq) - prodlow + (sum*argsq2)) + suminf*argsq; */
			addend = __FMSUB( suml, argsq, (suml*argsq) ) + __FMSUB( sum, argsq2, (sum*argsq2) ) +
				__FMADD( suminf, argsq, __FMADD( sum, argsq2, __FMSUB( suml, argsq, prodlow ) ) );
			temp = __FMADD( sum, argsq, prodlow );			// sum*argsq + prodlow;							// last 3 terms computed
			temp2 = __FADD( __FMSUB( sum, argsq, temp), prodlow );// sum*argsq - temp + prodlow;				// with greater care
			temp3 = __FMSUB( sum, argsq, temp) - temp2 + prodlow + addend;//(sum*argsq - temp) - temp2 + prodlow + addend;
			res = __FADD( erfpow[i][0], temp );
			reslow = (erfpow[i][0] - res + temp);
			resmid = __FADD( erfpow[i][1], temp2 );
			restiny = erfpow[i][1] - resmid + temp2 + erfxtr[i] + temp3;
			p = __FADD( reslow, resmid );																	// sum of middle terms
			q = reslow - p + resmid;																		// possible residual
			sum = __FADD( res, p );
			suml = __FADD( (res - sum + p), (q + restiny) );
			suminf = (res - sum + p) - suml + (q + restiny);
		}
	
		//********************************************************************
		//   To complete the job, the series (so far summed in even powers), *
		//   must be multiplied by the argument and by 2/PI^.5.              *
		//   These products are executed in sextuple precision to avoid      *
		//   excessive rounding errors.                                      *
		//********************************************************************
		
		_d3mul( sum, suml, suminf, dhead, dtail, 0.0, &prod1, &prod2, &px );
		_d3mul( prod1, prod2, px, ca.f, cb.f, cc.f, &(resdd.d[0]), &(resdd.d[1]), &qx );
		
		if (ierf == 0) {
			// erfc(x) = 1 - erf(x)
			_d3add( 1.0, 0.0, 0.0, -resdd.d[0], -resdd.d[1], -qx,
					&(resdd.d[0]), &(resdd.d[1]), &qx );
		}
		return resdd.f;
	}
	else if (fabs(dhead) <= 2.0) {
	
		//********************************************************************
		//   For arguments 1 < |x| <= 2.0, erfc is computed as:              *
		//                                                                   *
		//              erfc(x)= e^(-x^2+series)                             *
		//                                                                   *
		//   WHERE: "series" is a polynomial approximation devised to        *
		//         make the above formula correct to within 105 bits of      *
		//         precision.  The actual series is in powers of             *
		//         |x|-center.                                               *
		//********************************************************************
		
		if (fabs(dhead) < 1.00) {							// 3 subintervals used:
			center = 0.625;									//    0.75 < x < 1.0
			adjust = 0.28125;								//    1.0 <= x < 1.25
			times = 2.0;									//   1.25 <= x <= 2.0
			ir = 8;
			is = 18;
			ie = 15;
		}
		else if (fabs(dhead) < 1.25) {
			center = 0.8125;
			adjust = 0.28125;
			times = 2.0;
			ir = 6;
			is = 19;
			ie = 16;
		}
		else {
			center = 1.5;
			adjust = 1.375;
			times = 3.0;
			ir = 7;
			is = 26;
			ie = 14;
		}
		
		if (dhead < 0.0) {
			oarg = dhead;
			oarg2 = dtail;
			temp = -center - dhead;							// Compute |x| - center
			arg2 = -dtail;
		}
		else {
			temp = dhead - center;
			arg2 = dtail;
			oarg = -dhead;
			oarg2 = -dtail;
		}
		arg = __FADD( temp, arg2 );
		arg2 = temp - arg + arg2;
		sum = coeff[ir][is][0];
		suml = coeff[ir][is][1];
		for (i = is-1; i >= ie; --i) {						// computed using q&d
			temp = __FMADD( sum, arg, coeff[ir][i][0] );	// coeff[ir][i][0] + sum*arg;				// quad precision
			/* suml = coeff[ir][i][0] - temp + sum*arg +
					(coeff[ir][i][1] + sum*arg2 + suml*arg); */
			suml = __FMADD( sum, arg, coeff[ir][i][0] - temp ) + 
					__FMADD( suml, arg, __FMADD( sum, arg2, coeff[ir][i][1] ) );
			sum = temp;
		}
		suminf = 0.0e0;										// remaining terms require full
		for (i = ie-1; i >= 0; --i) {						// sextuple precision arithmetic
			_d3mul( sum, suml, suminf, arg, arg2, 0.0, &prod1, &prod2, &px );
			_d3add( prod1, prod2, px, coeff[ir][i][0], coeff[ir][i][1], 0.0,
					&sum, &suml, &suminf );
			// NOTE: The original FORTRAN (apparently in error) was:
			// _sum=_q_a6(%val(_prod),%val(px),%val(coeff(1,i,ir)),
			// +			%val(coeff(2,i,ir)),suminf)  <-- wrong number of parameters!
		}
		a2 = times*oarg2;
		a3 = __FMSUB( times, oarg2, a2 );					// times*oarg2 - a2;								// series is adjusted by
		a1 = oarg*times;																						// adding times*x-adjust
		ab = __FADD( __FMSUB( times, oarg2, a1 ), a2 );		// oarg*times - a1 + a2;
		ac = __FMSUB( times, oarg2, a1 ) - ab + a2 + a3;	// (oarg*times - a1) - ab + a2 + a3;
		_d3add( sum, suml, suminf, adjust, 0.0, 0.0, &sum, &suml, &as );
		_d3add( sum, suml, as, a1, ab, ac, &sum, &suml, &asx );
		resdd.f = _ExpInnerLD(sum, suml, asx, &resx, 4);
		
		// Erfc(|x|) done! 
		if (ierf == 0) {
			if (dhead <= 0.0)								// For pos args, done!
				// Erfc(-x) = 2 - Erfc(x)
				_d3add( 2.0, 0.0, 0.0, -resdd.d[0], -resdd.d[1], -resx,
						&(resdd.d[0]), &(resdd.d[1]), &qx );
		}
		else {												// Caller wanted Erf.
			// Use Erf(x) = 1 - Erfc(x)
			if (dhead > 0.0)
				_d3add( 1.0, 0.0, 0.0, -resdd.d[0], -resdd.d[1], -resx,
						&(resdd.d[0]), &(resdd.d[1]), &qx );
			else
				_d3add(-1.0, 0.0, 0.0,  resdd.d[0],  resdd.d[1],  resx,
						&(resdd.d[0]), &(resdd.d[1]), &qx );
		}
		return resdd.f;
	}	
	else {
	
		//********************************************************************
		//  For all remaining arguments, ERFC(x) is calculated as follows:   *
		//                                                                   *
		//             erfc(x)=e^(-x^2) / x*cf,                              *
		//                                                                   *
		//     WHERE: cf is a continued fraction that converges in the left  *
		//        half plane.                                                *
		//                                                                   *
		//       VARIABLE NAME        MEANING                                *
		//           cf        over various intervals, is                    *
		//                     approximated by polynomials                   *
		//                                                                   *
		//           ir        identifies the polynomial                     *
		//                                                                   *
		//           is        the degree of the polynomial                  *
		//                                                                   *
		//           ie        the lower bound after which                   *
		//                     sextuple precision must be used               *
		//                                                                   *
		//   The approximating polynomial is in powers of (1/x)^2            *
		//********************************************************************
		
		_d3mul( dhead, dtail, 0.0, dhead, dtail, 0.0,
				&asq1, &asq2, &asq );						// arg^2
		_d3div( 1.0, 0.0, 0.0, asq1, asq2, asq,
				&arg, &arg2, &argx );						// 1/arg^2
		if (arg >= 0.18e0) {								//  .18 <= _arg < .25
			ir = 1;											// which polynomial
			is = 21;										// How many terms
			ie = 10;										// Last term done q&d
		}
		else if (arg >= 0.11e0) {
			ir = 2;											//  .11 <= _arg < .18
			is = 23;
			ie = 12;
		}
		else if (arg >= 0.06e0) {
			ir = 3;											//  .06 <= _arg < .11
			is = 22;
			ie = 6;
			if (ierf != 1) ie = 11;
		}
		else if (arg >= 0.01e0) {
			ir = 4;											//  .01 <= _arg < .06
			is = 28;
			ie = 10;
		}
		else if (arg >= 0.00138e0) {
			ir = 5;											//  .00138 <= _arg < .01
			is = 17;
			ie = 4;
			if (ierf != 1) ie = 8;
		}
		else {
		
			//***********************************************************
			//   For larger arguments, erfc is so small that it cannot  *
			//   be represented, so the value of zero is used for erfc  *
			//***********************************************************
			
			resdd.d[0] = 0.0;
			resdd.d[1] = 0.0;
			rx = 0.0;
			if (dhead == dhead) goto commontail;
	
			// For NaN inputs ->NaNQ
			resdd.d[0] = dhead;
			return resdd.f;
		}
		if ((ierf == 1) && (arg < .01)) {					// For erf(x), x > 10,
			resdd.d[0] = 0.0;
			resdd.d[1] = 0.0;								// the result is 1.
			rx = 0.0;										// (to save time)
			goto commontail;
		}
		sum = coeff[ir][is][0];
		suml = coeff[ir][is][1];							// q&d quad prec.
		for (i = is-1; i >= ie; --i) {						// part of polynomial
			temp = __FMADD( sum, arg, coeff[ir][i][0] );	// coeff[ir][i][0] + sum*arg;				// evaluation
			/* suml = coeff[ir][i][0] - temp + sum*arg +
					(coeff[ir][i][1] + sum*arg2 + suml*arg); */
			suml = __FMADD( sum, arg, coeff[ir][i][0] - temp ) +
					__FMADD( suml, arg, __FMADD( sum, arg2, coeff[ir][i][1] ) );
			sum = temp;
		}
		suminf = 0.0e0;										// sextuple prec eval of
															// remaining terms of the
		for (i = ie-1; i >= 0; --i) {						// approximating polynomial
			_d3mul( sum, suml, suminf, arg, arg2, argx, &prod1, &prod2, &px);
			_d3add( prod1, prod2, px, coeff[ir][i][0], coeff[ir][i][1], 0.0,
					&sum, &suml, &suminf );
			// NOTE: The original FORTRAN (apparently in error) was:
			// _sum=_q_a6(%val(_prod),%val(px),%val(coeff(1,i,ir)),
			// +		   %val(coeff(2,i,ir)),suminf)  <-- wrong number of parameters!
		}
		_d3mul( sum, suml, suminf, ca.f, cb.f, cc.f,
				&prod1, &prod2, &px );						// multiply by 2/sqrt(pi)
		resdd.f = _ExpInnerLD(-asq1, -asq2, -asq, &exp, 4);
		_d3mul( prod1, prod2, px, resdd.d[0], resdd.d[1], exp,
				&prod1, &prod2, &px );
		_d3div( prod1, prod2, px, dhead, dtail, 0.0,
				&(resdd.d[0]), &(resdd.d[1]), &rx );
		resdd.f = 0.5 * resdd.f;
		rx = 0.5*rx;
		
	commontail:
		
		if (dhead > 0.0) {									// to return ERF or ERFC
			if (ierf == 1)									// for pos and neg args
				_d3add( 1.0, 0.0, 0.0, -resdd.d[0], -resdd.d[1], -rx,
						&(resdd.d[0]), &(resdd.d[1]), &qx );
		}
		else {
			if (ierf == 1)
				_d3add(-1.0, 0.0, 0.0, -resdd.d[0], -resdd.d[1], -rx,
						&(resdd.d[0]), &(resdd.d[1]), &qx );
			else
				_d3add( 2.0, 0.0, 0.0,  resdd.d[0],  resdd.d[1],  rx,
						&(resdd.d[0]), &(resdd.d[1]), &qx );
		}
		return resdd.f;
	}
}


long double erfl( long double x )
{
	double head, fenv;
	DblDbl t;
	
        FEGETENVD(fenv);
        FESETENVD(0.0);
	
	t.f = x;
	
	t.f = ErfCommon(t.d[0], t.d[1], 1);
	
	head   = __FADD( t.d[0], t.d[1] );
	t.d[1] = (t.d[0] - head) + t.d[1];	//  cannonize tail
	t.d[0] = head;						//  cannonize head
	
        FESETENVD(fenv);
	return t.f;
}

long double erfcl( long double x )
{
	double head, fenv;
	DblDbl t;
	
        FEGETENVD(fenv);
        FESETENVD(0.0);
	
	t.f = x;
	
	t.f = ErfCommon(t.d[0], t.d[1], 0);
	
	head   = __FADD( t.d[0], t.d[1] );
	t.d[1] = (t.d[0] - head) + t.d[1];	//  cannonize tail
	t.d[0] = head;						//  cannonize head
	
        FESETENVD(fenv);
	return t.f;
}
#endif
