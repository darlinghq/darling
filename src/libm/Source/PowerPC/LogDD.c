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
//	LogDD.c
//	
//	Double-double Function Library
//	Copyright: © 1995-96 by Apple Computer, Inc., all rights reserved
//	
//	long double logl( long double x );
//	long double log10l( long double x );
//	long double log2l( long double x );
//	long double log1pl( long double x );
//
//			_LogInnerLD() is exported for use by other functions.
//

#include "math.h"
#include "fp_private.h"
#if (__WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L)
#include "DD.h"

// Floating-point constants

static const double kLn2  = 6.9314718055994529e-1;			// {0x3FE62E42,0xFEFA39EF}
static const double kLn2b = 2.3190468138462996e-17;			// {0x3C7ABC9E,0x3B39803F}
static const double kLn2c = 5.7077084384162121e-34;			// {0x3907B57A,0x079A1934}
//static const double kLn2d = -3.58243221060181156e-50;		// {0xB5AACE93,0xA4EBE5D1}

static const double kBig = 6.755399441055744e+15;			// {0x43380000,0x00000000}
static const double recip15fac = 0.138750138750138750138750e-5; // 1/15!
static const double scaleup = 1.3407807929942597e+154;		// {0x5ff00000,0x00000000}
static const double kLog10eHi = 4.3429448190325182e-1;		// log10(e) {0x3FDBCB7B,0x1526E50E}
static const double kLog10eMid = 1.0983196502167651e-17;	// {0x3C695355,0xBAAAFAD3}
static const double kLog10eLow = 3.7171812331109602e-34;	// {0x38FEE191,0xF71A3015}
static const double kLog2eHi = 1.4426950408889634e+0;		// log2(e) {0x3ff71547,0x652b82fe}
static const double kLog2eMid = 2.0355273740931033e-17;		// {0x3c7777d0,0xffda0d24}
static const double kLog2eLow = -1.0614659956117258e-33;	// {0xb9160bb8,0xa5442ab9}

static const Double kInfinityu = {{0x7ff00000, 0x0}};

static const double coeff[] = {720720.0, -360360.0, 240240.0, -180180.0,
	144144.0, -120120.0, 102960.0, -90090.0, 80080.0, -72072.0, 65520.0,
	-60060.0, 55440.0, -51480.0, 48048.0, -45045.0};

extern uint32_t LogTableLD[];

/*
static uint32_t LogTableLD[] __attribute__ ((aligned(8))) = {
	
	0x3FF02000, 0x00026746,  0x3FEFC07F, 0x01F74C65,  0x3F7FE02A, 0x6D72E887,  0x3B3F98BF, 0x5125A0D9,
	0x3FF06000, 0x000C15E4,  0x3FEF4465, 0x9E332ED4,  0x3F97B91B, 0x0AC9738C,  0x3B56E8A1, 0xB39F02BA,  
	0x3FF0A000, 0x0014A543,  0x3FEECC07, 0xB2DBAE11,  0x3FA39E87, 0xBC7A8F8A,  0x3B626302, 0xA58A48BC,  
	0x3FF0E000, 0x0011E06B,  0x3FEE573A, 0xC8E1C131,  0x3FAB42DD, 0x7337D5A8,  0x3B6B338C, 0x5DDA3A53,  
	0x3FF12000, 0x0013DA38,  0x3FEDE5D6, 0xE3D5DDB6,  0x3FB16536, 0xEFCC407C,  0x3B715E8E, 0x0264EABA,  
	0x3FF16000, 0x000A2948,  0x3FED77B6, 0x54A6F07C,  0x3FB51B07, 0x3F9BCF0A,  0x3B74DB56, 0xDE56C8EF,  
	0x3FF1A000, 0x0012B0CD,  0x3FED0CB5, 0x8F4FF21D,  0x3FB8C345, 0xD7411583,  0x3B786152, 0x587782F8,  
	0x3FF1E000, 0x000EE3A6,  0x3FECA4B3, 0x054705AF,  0x3FBC5E54, 0x90310477,  0x3B7C237F, 0x7657770C,
	0x3FF22000, 0x00153135,  0x3FEC3F8F, 0x01A2F19D,  0x3FBFEC91, 0x33073D1A,  0x3B7FACCC, 0x73F05939,
	0x3FF26000, 0x00241909,  0x3FEBDD2B, 0x895D49D0,  0x3FC1B72A, 0xD62ADC8D,  0x3B817554, 0xB20DE7D5,   
	0x3FF2A000, 0x002BCBD4,  0x3FEB7D6C, 0x3D998F3A,  0x3FC371FC, 0x214B8C8E,  0x3B82E934, 0xDA5043A5,   
	0x3FF2E000, 0x000DA3AC,  0x3FEB2036, 0x4058E6DD,  0x3FC526E5, 0xE3FE32D7,  0x3B851132, 0xC8E084BC,   
	0x3FF32000, 0x0021AE4E,  0x3FEAC570, 0x1A964A93,  0x3FC6D60F, 0xE7FB3D90,  0x3B86BC1B, 0x66F15A74,   
	0x3FF36000, 0x00196038,  0x3FEA6D01, 0xA6AD7E27,  0x3FC87FA0, 0x65C86E05,  0x3B887653, 0xDF907BC0,   
	0x3FF3A000, 0x0021FEB5,  0x3FEA16D3, 0xF94D19C3,  0x3FCA23BC, 0x20C06EFE,  0x3B8A0FA9, 0x3CB516A7,   
	0x3FF3E000, 0x000D0738,  0x3FE9C2D1, 0x4ED3BE0F,  0x3FCBC286, 0x7481747C,  0x3B8BA61B, 0x516322DC,   
	0x3FF42000, 0x0026BB6E,  0x3FE970E4, 0xF7DBC1DE,  0x3FCD5C21, 0x6C46142A,  0x3B8CF4E4, 0x5F70ECE3,   
	0x3FF46000, 0x001A700A,  0x3FE920FB, 0x49B04706,  0x3FCEF0AD, 0xCC826F72,  0x3B8ECCCF, 0xC2085FC9,   
	0x3FF4A000, 0x0013F66B,  0x3FE8D301, 0x8D1811ED,  0x3FD04025, 0x94F2C209,  0x3B903E90, 0x5D16156B,   
	0x3FF4E000, 0x0011F43B,  0x3FE886E5, 0xF09697FA,  0x3FD1058B, 0xF9E55645,  0x3B90EE57, 0x086B5AEC,   
	0x3FF52000, 0x00219250,  0x3FE83C97, 0x7A8C3A9F,  0x3FD1C898, 0xC1CF4F30,  0x3B919E40, 0xEDF99C22,   
	0x3FF56000, 0x0025844D,  0x3FE7F405, 0xFCD7747F,  0x3FD2895A, 0x144EDB60,  0x3B926EE9, 0x301CD24B,   
	0x3FF5A000, 0x00426EFC,  0x3FE7AD22, 0x08983088,  0x3FD347DD, 0x9B5D1A24,  0x3B934057, 0xA2FCAF6F,   
	0x3FF5E000, 0x0023BDD6,  0x3FE767DC, 0xE40E6B97,  0x3FD40430, 0x86EF39B2,  0x3B93FF8C, 0x8952354B,   
	0x3FF62000, 0x003B535A,  0x3FE72428, 0x7F08D1CD,  0x3FD4BE5F, 0x96231467,  0x3B94825C, 0xC3371E24,   
	0x3FF66000, 0x001DA1F2,  0x3FE6E1F7, 0x6B24E9B5,  0x3FD57677, 0x179A1CC5,  0x3B956330, 0xFA60E69E,   
	0x3FF6A000, 0x0037A0C1,  0x3FE6A13C, 0xD11BCEC4,  0x3FD62C82, 0xF35722D2,  0x3B95FA0B, 0x10B586EE,   
	0x3FF6E000, 0x001B76C0,  0x3FE661EC, 0x6A364397,  0x3FD6E08E, 0xAA78789F,  0x3B96D198, 0x6621D929,   
	0x3FF72000, 0x003ED5D5,  0x3FE623FA, 0x76C53936,  0x3FD792A5, 0x608B2E43,  0x3B976089, 0x3B698591,   
	0x3FF76000, 0x003610DA,  0x3FE5E75B, 0xB89D6C37,  0x3FD842D1, 0xDAB292E6,  0x3B982832, 0xEB89C0AF,   
	0x3FF7A000, 0x0016DE9E,  0x3FE5AC05, 0x6AEC6020,  0x3FD8F11E, 0x877456E8,  0x3B98DD6D, 0x9D2A6F65,   
	0x3FF7E000, 0x004C046C,  0x3FE571ED, 0x3C0C2377,  0x3FD99D95, 0x81E3A6B3,  0x3B99848C, 0x4BFDF0E3,   
	0x3FE82000, 0x00393E5C,  0x3FF53909, 0x48C1B475,  0xBFD21445, 0x6C76DD04,  0x3B91F8C3, 0x3DC493A4,   
	0x3FE86000, 0x003EFB2C,  0x3FF50150, 0x14CB09F5,  0xBFD16B5C, 0xCB079DCA,  0x3B910FAA, 0x072FC35E,   
	0x3FE8A000, 0x003E6A46,  0x3FF4CAB8, 0x86F0FC55,  0xBFD0C42D, 0x66BF2B99,  0x3B905CD8, 0x563746EF,   
	0x3FE8E000, 0x00477E7A,  0x3FF49539, 0xE377A7F3,  0xBFD01EAE, 0x556ED4C7,  0x3B8F705E, 0xB9C5B7AA,   
	0x3FE92000, 0x0037DAA4,  0x3FF460CB, 0xC7C8826B,  0xBFCEF5AD, 0xE3C07315,  0x3B8DE963, 0x957971E5,   
	0x3FE96000, 0x0031BEA8,  0x3FF42D66, 0x25AD915C,  0xBFCDB13D, 0xAFD99B61,  0x3B8D27CB, 0xE3668C21,   
	0x3FE9A000, 0x0031523A,  0x3FF3FB01, 0x3F899EFB,  0xBFCC6FFB, 0xC5F9B1E6,  0x3B8BE9E5, 0x5F85EE97,   
	0x3FE9E000, 0x00343918,  0x3FF3C995, 0xA453BC21,  0xBFCB31D8, 0x56597734,  0x3B8AF392, 0xD52D7571,   
	0x3FEA2000, 0x00199638,  0x3FF3991C, 0x2C054DA3,  0xBFC9F6C4, 0x068B3974,  0x3B89EE78, 0x9ACC9199,   
	0x3FEA6000, 0x0033C6EC,  0x3FF3698D, 0xF3B7EB7F,  0xBFC8BEAF, 0xEA3DB758,  0x3B889436, 0x63D97AE8,   
	0x3FEAA000, 0x00276244,  0x3FF33AE4, 0x5B3B4ABA,  0xBFC7898D, 0x8486F5D7,  0x3B86F513, 0xCA9E0A1C,   
	0x3FEAE000, 0x001DE89C,  0x3FF30D19, 0x011B9DF1,  0xBFC6574E, 0xBDFDA066,  0x3B86222A, 0xB12C0834,   
	0x3FEB2000, 0x0037A536,  0x3FF2E025, 0xC024C7B9,  0xBFC527E5, 0xE39B1FE9,  0x3B8458D5, 0xE445BFCF,   
	0x3FEB6000, 0x000CCE1A,  0x3FF2B404, 0xACF86B95,  0xBFC3FB45, 0xA55D490D,  0x3B83C23D, 0xACE1BFEC,   
	0x3FEBA000, 0x002CA378,  0x3FF288B0, 0x126ABD40,  0xBFC2D161, 0x0BB7AC3B,  0x3B82B40D, 0x1D2C66D4,   
	0x3FEBE000, 0x00343A2E,  0x3FF25E22, 0x705E28E9,  0xBFC1AA2B, 0x7D342442,  0x3B812B63, 0x558826AF,   
	0x3FEC2000, 0x0038FF08,  0x3FF23456, 0x7875D88C,  0xBFC08598, 0xB49AD49F,  0x3B7F3F8E, 0x64C28802,   
	0x3FEC6000, 0x001B33E6,  0x3FF20B47, 0x0C567468,  0xBFBEC739, 0x8214A4A6,  0x3B7E277B, 0xA41F0AAF,   
	0x3FECA000, 0x00119564,  0x3FF1E2EF, 0x3B34BBBD,  0xBFBC8858, 0x011F0A29,  0x3B7C3C3D, 0xB4C961E9,   
	0x3FECE000, 0x001996BC,  0x3FF1BB4A, 0x4037367A,  0xBFBA4E76, 0x3FCEDEB6,  0x3B7989B5, 0x3403569D,   
	0x3FED2000, 0x0027E986,  0x3FF19453, 0x80748B78,  0xBFB8197E, 0x2DE212FA,  0x3B779AB6, 0x2FAC97BD,   
	0x3FED6000, 0x0019EB78,  0x3FF16E06, 0x8933124A,  0xBFB5E95A, 0x4CB5AE64,  0x3B754C62, 0x7502E4A2,   
	0x3FEDA000, 0x0012ADBC,  0x3FF1485F, 0x0DFFE7A7,  0xBFB3BDF5, 0xA73085C0,  0x3B7355A8, 0xD9528443,   
	0x3FEDE000, 0x0020DF02,  0x3FF12358, 0xE74A54DA,  0xBFB1973B, 0xD02CA8E2,  0x3B711C99, 0x788FFA22,   
	0x3FEE2000, 0x001D96D8,  0x3FF0FEF0, 0x10EE3E82,  0xBFAEEA31, 0xBE0FD392,  0x3B6E25BF, 0x730D5F04,   
	0x3FEE6000, 0x000AA830,  0x3FF0DB20, 0xA8895CA2,  0xBFAAAEF2, 0xD0476EBF,  0x3B6A4412, 0x0CC9DB9D,   
	0x3FEEA000, 0x001B75A4,  0x3FF0B7E6, 0xEC16A042,  0xBFA67C94, 0xF109A73B,  0x3B653651, 0xD572AAB2,   
	0x3FEEE000, 0x0017A242,  0x3FF0953F, 0x38F457BA,  0xBFA252F3, 0x2E052CD8,  0x3B60EC4F, 0x64C2DE39,   
	0x3FEF2000, 0x000CF922,  0x3FF07326, 0x0A411C88,  0xBF9C63D2, 0xEA69DB02,  0x3B5C53BD, 0xE86F9708,   
	0x3FEF6000, 0x000BAB68,  0x3FF05197, 0xF7D12236,  0xBF9432A9, 0x241B2F6E,  0x3B53F5BE, 0x52B566FB,   
	0x3FEFA000, 0x000E227C,  0x3FF03091, 0xB51821B3,  0xBF882448, 0x9FF5499F,  0x3B472044, 0xA44CCF4D,   
	0x3FF00000, 0x00000000,  0x3FF00000, 0x00000000,  0x00000000, 0x00000000,  0x00000000, 0x00000000   
};
*/

struct LogTableEntry {
	double x;
	double u;    // u = 1/x
	double fhead;
	double ftail;
};


long double logl( long double x )
{
	double fpenv;
	DblDbl u;
	double extra;
	
    FEGETENVD(fpenv);
    FESETENVD(0.0);
	
	u.f = x;
	
	if (((u.i[0] & 0x7fffffffu) | u.i[1]) == 0) {		// x = 0.0
		u.d[0] = -1.0/__FABS(u.d[0]);
		u.d[1] = 0.0;
                FESETENVD(fpenv);
		return u.f;
	}
	if (u.i[0] < 0x7ff00000u) {						// x is finite and positive
		if ((u.i[0] == 0x3ff00000) && (u.i[1] | u.i[2] | u.i[3]) == 0) {
                        FESETENVD(fpenv);
			return 0.0L;								// x = 1.0
		}
		u.f = _LogInnerLD(u.d[0], u.d[1], 0.0, &extra, 0);
                FESETENVD(fpenv);
		return u.f;
	}
	if (u.d[0] != u.d[0]) {
                FESETENVD(fpenv);
		return x;										// NaN case
	}
	if ((u.i[0] & 0xfff00000u) == 0x7ff00000u) {
                FESETENVD(fpenv);
		return x;										// +infinity
	}
	else {    											// x < 0.0
		u.d[0] = 0.0 * kInfinityu.f + u.d[0];
		u.d[1] = 0.0;
                FESETENVD(fpenv);
		return u.f;
	}
}

long double log10l( long double x )
{
	double fpenv;
	DblDbl u;
	double result, resmid, resbot;
	double uu, vv, ww, xx, yy, c, top, bottom;
	
        FEGETENVD(fpenv);
        FESETENVD(0.0);
	
	u.f = x;
	
	if (((u.i[0] & 0x7fffffffu) | u.i[1]) == 0) {		// x = 0.0
		u.d[0] = -1.0/__FABS(u.d[0]);
		u.d[1] = 0.0;
                FESETENVD(fpenv);
		return u.f;
	}
	if (u.i[0] < 0x7ff00000u) {						// x is finite and positive
		if ((u.i[0] == 0x3ff00000) && (u.i[1] | u.i[2] | u.i[3]) == 0) {
                        FESETENVD(fpenv);
			return 0.0L;								// x = 1.0
		}
		u.f = _LogInnerLD(u.d[0], u.d[1], 0.0, &resbot, 1);
		result = u.d[0];
		resmid = u.d[1];

		uu = __FADD( result * kLog10eLow, __FADD( resmid * kLog10eMid, resbot * kLog10eHi ) );
		vv = result * kLog10eMid;
		ww = __FMSUB( result, kLog10eMid, vv);
		xx = resmid * kLog10eHi;
		yy = __FMSUB( resmid, kLog10eHi, xx );
		c = ww + yy + uu;
		top = result * kLog10eHi;
		ww = __FMSUB( result, kLog10eHi, top );
		uu = __FADD( vv, xx );
		if (__FABS(xx) > __FABS(vv))
			c =  xx - uu + vv + c;
		else
			c = vv - uu + xx + c;
		bottom = __FADD( uu, ww );
		if (__FABS(ww) > __FABS(uu))
			c = ww - bottom + uu + c;
		else
			c = uu - bottom + ww + c;
		u.d[0] = __FADD( top, bottom );
		u.d[1] = top - u.d[0] + bottom + c;
                FESETENVD(fpenv);
		return u.f;
	}
	if (u.d[0] != u.d[0]) {
                FESETENVD(fpenv);
		return x;										// NaN case
	}
	if ((u.i[0] & 0xfff00000u) == 0x7ff00000u) {
                FESETENVD(fpenv);
		return x;										// +infinity
	}
	else {												// x < 0.0
		u.d[0] = 0.0 * kInfinityu.f + u.d[0];
		u.d[1] = 0.0;
                FESETENVD(fpenv);
		return u.f;
	}
}

long double log2l( long double x )
{
	double fpenv;
	DblDbl u;
	double result, resmid, resbot;
	double uu, vv, ww, xx, yy, c, top, bottom;
	
        FEGETENVD(fpenv);
        FESETENVD(0.0);
	
	u.f = x;
	
	if (((u.i[0] & 0x7fffffffu) | u.i[1]) == 0) {		// x = 0.0
		u.d[0] = -1.0/__FABS(u.d[0]);
		u.d[1] = 0.0;
                FESETENVD(fpenv);
		return u.f;
	}
	if (u.i[0] < 0x7ff00000u) {						// x is finite and positive
		if ((u.i[0] == 0x3ff00000) && (u.i[1] | u.i[2] | u.i[3]) == 0) {
                        FESETENVD(fpenv);
			return 0.0L;								// x = 1.0
		}
		u.f = _LogInnerLD(u.d[0], u.d[1], 0.0, &resbot, 1);
		result = u.d[0];
		resmid = u.d[1];
		uu = __FADD( result * kLog2eLow, __FADD( resmid * kLog2eMid, resbot * kLog2eHi ) );
		vv = result * kLog2eMid;
		ww = __FMSUB( result, kLog2eMid, vv );
		xx = resmid * kLog2eHi;
		yy = __FMSUB( resmid, kLog2eHi, xx );
		c = ww + yy + uu;
		top = result * kLog2eHi;
		ww = __FMSUB( result, kLog2eHi, top );
		uu = __FADD( vv, xx );
		if (__FABS(xx) > __FABS(vv))
			c =  xx - uu + vv + c;
		else
			c = vv - uu + xx + c;
		bottom = __FADD( uu, ww );
		if (__FABS(ww) > __FABS(uu))
			c = ww - bottom + uu + c;
		else
			c = uu - bottom + ww + c;
		u.d[0] = __FADD( top, bottom );
		u.d[1] = top - u.d[0] + bottom + c;
                FESETENVD(fpenv);
		return u.f;
	}
	if (u.d[0] != u.d[0]) {
                FESETENVD(fpenv);
		return x;										// NaN case
	}
	if ((u.i[0] & 0xfff00000u) == 0x7ff00000u) {
                FESETENVD(fpenv);
		return x;										// +infinity
	}
	else {												// x < 0.0
		u.d[0] = 0.0 * kInfinityu.f + u.d[0];
		u.d[1] = 0.0;
                FESETENVD(fpenv);
		return u.f;
	}
}

long double log1pl( long double x )
{
	double fpenv;
	DblDbl u;
	double high, mid, low, extra, a, b, c, d, e, f;
	
	u.f = x;
	
	if (u.d[0] != u.d[0])								// x = NaN
		return x;
	if (((u.i[0] & 0x7fffffffu) | u.i[1]) == 0)		// x = 0.0
		return x;
        FEGETENVD(fpenv);
        FESETENVD(0.0);
	if ((x > -1.0L) && ((u.i[0] & 0x7ff00000) != 0x7ff00000)) { // -1<x< Inf
		if (1.0 > __FABS(u.d[0])) {						// 1.0 > |head| > |tail|
			high = 1.0;
			mid = u.d[0];
			low = u.d[1];
		}
		else if (__FABS(u.d[1]) > 1.0) {				// |head| > |tail| > 1.0
			high = u.d[0];
			mid = u.d[1];
			low = 1.0;
		}
		else {											// |head| >= 1.0 >= |low|
			high = u.d[0];
			mid = 1.0;
			low = u.d[1];
		}
		a = __FADD( mid, low );
		b = (mid - a) + low;
		c = __FADD( high, a );
		d = (high - c) + a;
		e = __FADD( d, b );
		f = (d - e) + b;
		u.f = _LogInnerLD(c, e, f, &extra, 2);
                FESETENVD(fpenv);
		return u.f;
	}
	if ((u.i[0] == 0xbff00000) && (u.i[1] | (u.i[2] & 0x7fffffffu)  | u.i[3]) == 0) {
														// x = -1.0
		u.d[0] = -INFINITY;
		u.d[1] = 0.0;
                FESETENVD(fpenv);
		return u.f;
	}
	if ((u.i[0] & 0xfff00000u) == 0x7ff00000u) {
                FESETENVD(fpenv);
		return x;										// x = +inf
	}
	else {												// x < -1.0
		u.d[0] = 0.0 * kInfinityu.f + u.d[0];
		u.d[1] = 0.0;
                FESETENVD(fpenv);
		return u.f;
	}
}

//**********************************************************************************
//
//    FUNCTION:  long double _LogInnerLD(double alpha, double beta, double gamma,
//               double *extra, int logtype)
//    
//    DESCRIPTION: This routine is called internally by the following functions:
//
//                 long double Log(long double);
//                 long double Log2(long double);
//                 long double Log10(long double);
//                 long double Log1Plus(long double);
//                 long double Power(long double, long double);
//
//                 1) logtype = 0 (called by Log()):
//                    on entry: (alpha, beta)
//                    on exit:  (head, tail) of  Log(alpha + beta)
//
//                 2) logtype = 1 (called by Power(), Log10(), Log2()):
//                    on entry: (alpha, beta)
//                    on exit:  (head, tail, extra) of Log(alpha + beta)
//
//                 3) logtype = 2 (called by Log1Plus()):
//                    on entry: (alpha, beta, gamma)
//                    on exit:  (head, tail) of Log(alpha + beta + gamma)
//
//                 This routine assumes that the rounding direction on entry
//                 is round-to-nearest,  and NaN, infinity, Signed zeros, 
//                 negative values have been pre-filtered so that the input
//                 is a postive normal/denormal values.
//
//*********************************************************************************

long double _LogInnerLD(double alpha, double beta, double gamma, double *extra,
						int logtype)
{
	int i, iexp, ndx;
	double head, tail, redarg, diff, diff2, quot, arg, rem;
	double reciparg, e, sum, arglow, u, x, y, remlost, arglost, temp, suml;
	double argl, argl2, hi, prod, c, enlog2hi, w, enlog2mid, prodl, sum2;
	double sum3, enlog2low, small, petit, smallres, summid, resmid, summid1;
	double resmid1, summid2, resmid2, top, resmid3, resmid4, bottom, resmid5;
	double residual, result, residual2, resmidtemp;
	Double test, rscale, scale;
	DblDbl out;
	struct LogTableEntry *TblPtr = (struct LogTableEntry *)LogTableLD;
	
	test.f = alpha;
	tail = beta;
	rscale.f = 0.0;
	scale.f = kBig;
	if (test.i[0] >= 0x07000000u) {   					// alpha > 2^(-911)
		iexp = (test.i[0] & 0x7ff00000);
		rscale.i[0] = 2046 * 1048576 - iexp;
		scale.i[1] = (iexp >> 20);
	}
	else {  											// alpha < 2^(-911)
		test.f *= scaleup;
		tail *= scaleup;
		if (logtype == 2) gamma *= scaleup;
		head = __FADD( test.f, tail );
		tail = test.f - head + tail;
		iexp = (test.i[0] & 0x7ff00000);
		rscale.i[0] = 2046 * 1048576 - iexp;
		scale.i[1] = (iexp >> 20);
		scale.f -= 512.0;
	}
	ndx = ((test.i[0] & 0x000fc000) >> 14);
	if (rscale.i[0] == 0)
		rscale.i[0] = 524288;
	head = test.f * rscale.f;
	tail = tail * rscale.f;
	if (logtype == 2)
		gamma = gamma * rscale.f;
	scale.f -= kBig + 1023.0;
	if (test.i[0] & 0x00080000) { 
		scale.f  += 1.0;
		head *= 0.5;
		tail *= 0.5;
		if (logtype == 2)
			gamma *= 0.5;
	}
	if (ndx == 0)
		if ((test.i[0] & 0x000fffffu) < 0x00002000u)
			ndx = 63;
	redarg = head - TblPtr[ndx].x;              
	diff = __FADD( redarg, tail );
	diff2 = redarg - diff + tail + gamma;
	quot = __FMUL( diff, TblPtr[ndx].u );                
	arg = __FMADD( __FNMSUB( quot, TblPtr[ndx].x, diff ), TblPtr[ndx].u, quot ); // quot + (diff - quot * TblPtr[ndx].x) * TblPtr[ndx].u;
	remlost = __FNMSUB( arg, TblPtr[ndx].x, diff );
	rem = __FADD( remlost,  diff2 );
	reciparg = 1.0 - arg;
	e = arg * arg;
	sum = __FMADD( arg, coeff[15], coeff[14] ); // coeff[14] + arg * coeff[15];
	arglow = rem * TblPtr[ndx].u;
	u = scale.f * kLn2c;
	x = scale.f * kLn2b;
	y = __FMSUB( scale.f, kLn2b, x );
	if (__FABS(remlost) > __FABS(diff2))
		remlost = remlost - rem + diff2;
	else
		remlost = diff2 - rem + remlost;
	arglost = __FNMSUB( arglow, TblPtr[ndx].x, rem );
	arglost = __FADD( remlost, arglost );
	arglost = arglost * TblPtr[ndx].u;
	sum = __FMADD( arg, sum, coeff[13] ); // coeff[13] + arg * sum;
	reciparg = __FMADD( reciparg, e, reciparg ); // reciparg + reciparg * e;
	sum = __FMADD( arg, sum, coeff[12] ); // coeff[12] + arg * sum;
	e = e * e;
	sum = __FMADD( arg, sum, coeff[11] ); // coeff[11] + arg * sum;
	reciparg = __FMADD( reciparg, e, reciparg ); // reciparg + reciparg * e;
	sum = __FMADD( arg, sum, coeff[10] ); // coeff[10] + arg * sum;
	e = e * e;
	sum = __FMADD( arg, sum, coeff[ 9] ); // coeff[9] + arg * sum;
	reciparg = __FMADD( reciparg, e, reciparg ); // reciparg + reciparg * e;
	temp = __FMADD( arg, sum, coeff[8] ); // coeff[8] + arg * sum;
	suml = __FSUB( coeff[8], temp );
	suml = __FMADD( arg, sum, suml ); // suml + arg * sum;
	sum = temp;
	argl = arglow * reciparg;
	argl2 = (__FNMSUB( argl, arg, arglow - argl) + arglost) *reciparg; // ((arglow - argl - argl * arg) + arglost) * reciparg;
	for (i = 7; i >= 2; i--) {
		hi = __FMADD( sum, arg, coeff[i] );
		suml = __FMADD( suml, arg, __FMADD( sum, arg, coeff[i] - hi ) ); // coeff[i] - hi + sum*arg + suml*arg;
		sum = hi;
	}
	prod = sum * recip15fac;
	c = u + y;
	enlog2hi = __FMUL( scale.f, kLn2 );
	w = __FMSUB( scale.f , kLn2 , enlog2hi );
	enlog2mid = __FADD( x , w );
	prodl = (__FNMSUB( prod, coeff[0], sum ) + suml) * recip15fac; // (sum - prod * coeff[0] + suml) * recip15fac;
	sum2 = __FMSUB( prod, arg, 0.5);
	suml = __FMADD( prodl, arg, __FMADD( prod, arg, __FMSUB( -1.0, 0.5, sum2 ) ) ); // -1.0 * 0.5 - sum2 + prod * arg + prodl * arg;
	sum3 = __FMADD( sum2, arg, (suml * arg) );
	suml = __FMADD( suml, arg, __FMSUB( sum2, arg, sum3 ) ); // sum2 * arg - sum3 + (suml * arg);
	sum = __FMADD( sum3, arg, (suml * arg) );
	suml = __FMADD( suml, arg, __FMSUB( sum3, arg, sum ) ); // sum3 * arg - sum + (suml * arg);
	if (scale.f == 0.0) {
		top = __FADD( arg,  sum );
		resmid = arg - top + sum;
		small = __FADD( suml, argl );
		smallres = argl - small + suml;
		if (TblPtr[ndx].x == 1.0) {
			bottom = __FADD( resmid, small );
			residual2 = smallres + argl2;
			residual = resmid - bottom + small + residual2;
		}
		else {
			hi = __FADD( TblPtr[ndx].fhead, top );
			resmid2 = TblPtr[ndx].fhead - hi + top + resmid;
			petit = small;
			bottom = __FADD( resmid2,  petit );
			if (__FABS(resmid2) > __FABS(petit))
				residual = resmid2-bottom+petit+smallres+argl2+TblPtr[ndx].ftail;
			else
				residual = petit-bottom+resmid2+smallres+argl2+TblPtr[ndx].ftail;
			top = hi;
		}
	}
	else {
		if (__FABS(x) > __FABS(w))
			enlog2low = x - enlog2mid + w + c;
		else
			enlog2low = w - enlog2mid + x + c;
		small = __FADD( argl, suml );
		petit = TblPtr[ndx].ftail + enlog2low + argl2 + small;
		smallres = argl - small + suml;
		summid = __FADD( sum, enlog2mid );
		if (__FABS(sum) > __FABS(enlog2mid)) 
			resmid = sum - summid + enlog2mid;
		else                                   
			resmid = enlog2mid - summid + sum;
		summid1 = __FADD( arg, summid );
		resmid1 = arg - summid1 + summid;
		summid2 = __FADD( summid1, TblPtr[ndx].fhead );
		resmid2 = TblPtr[ndx].fhead - summid2 + summid1 + resmid1;
		petit = petit + resmid + smallres;
		top = __FADD( enlog2hi, summid2 );
		resmid3 = enlog2hi - top + summid2;
		resmid4 = __FADD( resmid3, resmid2 );
		bottom = __FADD( petit, resmid4 );
		resmid5 = resmid3 - resmid4 + resmid2;
		residual = resmid4 - bottom + petit + resmid5;
	}

	result = __FADD( top, bottom );

	if (logtype != 1) 
		resmid = top - result + bottom + residual;
	else {  											// logtype = 1
		resmidtemp = top - result + bottom;
		resmid = __FADD( resmidtemp,  residual );
		*extra = resmidtemp - resmid + residual;
	}

	out.d[0] = result;
	out.d[1] = resmid;
	return out.f;
}
#endif
