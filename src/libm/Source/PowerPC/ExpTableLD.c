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
// $Revision: 1.2 $
/* - -- --- ---- ----- ------------------------------------------------- */
/*                                                                       */
/*    Copyright (C) 1994 Taligent, Inc. All rights reserved.             */
/*                                                                       */
/*    This code is copyrighted. Under the copyright laws, this code may  */
/*         not be copied, in whole or part, without prior written        */
/*         consent of Taligent.                                          */
/*                                                                       */
/*    RESTRICTED RIGHTS LEGEND: Use, duplication, or disclosure by the   */
/*         government is subject to restrictions as set forth in         */
/*         subparagraph (c)(l)(ii) of the Rights in Technical Data and   */
/*         Computer Software clause at DFARS 252.227-7013 and            */
/*         FAR 52.227-19.                                                */
/*                                                                       */
/*    This product may be protected by one or more U.S. and              */
/*        International Patents.                                         */
/*                                                                       */
/*    TRADEMARKS: Taligent and the Taligent Design Mark are registered   */
/*        trademarks of Taligent, Inc.                                   */
/*                                                                       */
/* - -- --- ---- ----- ------------------------------------------------- */

/****************************************************************************
*
*   FILE:  ExpTableLD.C
*
*   Description:
*      This file contains the uint32_t array ExpTableLD[] used in the
*      approximation for Exp, Exp2, ExpMinus, Hyperbolic and Power functions
*      (long double precision) in <Numerics.h> for PowerPC and RS/6000
*      platforms:
*
*      Copyright (C) 1994 Taligent, Inc. All rights reserved.
*
*      Modified for Taligent by Yinsun Feng, April, 1994.
*
*   MODIFICATION HISTORY:
*
****************************************************************************/
#include "stdint.h"

__private_extern__
const uint32_t ExpTableLD[] __attribute__ ((aligned(8))) = {

   0xBFD7FFFF, 0xFFF78346,    0x3FE5FE46, 0x15EC7934,    0xBBA5D094, 0x3F817397,
   0xBFD6FFFF, 0xFFFFAA8D,    0x3FE656F0, 0x0BF5973E,    0x3B9DDE0B, 0xAFD470C5,
   0xBFD5FFFF, 0xFFFFC4B4,    0x3FE6B0FF, 0x72DECDA3,    0x3BA482D0, 0xD1C3FD44,
   0xBFD4FFFF, 0xFFFFD89A,    0x3FE70C79, 0xEBA34A37,    0x3B713FA4, 0x162E00F9,
   0xBFD3FFFF, 0xFFFFEB0A,    0x3FE76965, 0x2DF23729,    0xBB870FEE, 0xEEBD3A08,
   0xBFD2FFFF, 0xFFFFC587,    0x3FE7C7C7, 0x08878BF6,    0x3B96439C, 0x82816F81,
   0xBFD1FFFF, 0xFFFE834A,    0x3FE827A5, 0x618926C6,    0xBBA80AD5, 0x14F789EA,
   0xBFD0FFFF, 0xFFFFDACD,    0x3FE88906, 0x36E32D97,    0x3BA02C0F, 0x1AAB3177,
   0xBFCFFFFF, 0xFFFE8A16,    0x3FE8EBEF, 0x9EACCAD8,    0xBBA6337C, 0x56F8CB1D,
   0xBFCDFFFF, 0xFFFE54CC,    0x3FE95067, 0xC783CE6F,    0xBBA5CD29, 0x758448F6,
   0xBFCBFFFF, 0xFFFE8C9A,    0x3FE9B674, 0xF8F33E73,    0xBBA2FEFB, 0x001F06AC,
   0xBFC9FFFF, 0xFFFFF380,    0x3FEA1E1D, 0x93D68A5D,    0x3B94BC1B, 0x34F72879,
   0xBFC7FFFF, 0xFFFF88CE,    0x3FEA8768, 0x12C0A030,    0xBB95BE29, 0x64926F2A,
   0xBFC5FFFF, 0xFFFFCB72,    0x3FEAF25B, 0x0A61B2C5,    0xBBA5D75A, 0xCA0F1016,
   0xBFC3FFFF, 0xFFFF951A,    0x3FEB5EFD, 0x29F26302,    0x3BA9F0AD, 0xD4A5C367,
   0xBFC1FFFF, 0xFFFFF6D4,    0x3FEBCD55, 0x3B9D7D60,    0xBBAADABC, 0xB264015F,
   0xBFBFFFFF, 0xFFFFE014,    0x3FEC3D6A, 0x24ED85A7,    0x3B747961, 0x10476F00,
   0xBFBBFFFF, 0xFFFFD9AC,    0x3FECAF42, 0xE73A50C9,    0x3B915C7B, 0x4A361F1E,
   0xBFB7FFFF, 0xFFFFD862,    0x3FED22E6, 0xA0198085,    0x3BA1BBB3, 0xBFFB06E1,
   0xBFB3FFFF, 0xFFFFF168,    0x3FED985C, 0x89D04353,    0x3B5234BB, 0x7C215B4D,
   0xBFAFFFFF, 0xFFFFB37A,    0x3FEE0FAB, 0xFBC70722,    0xBB8AE48A, 0x8045BF54,
   0xBFA7FFFF, 0xFFFC4C34,    0x3FEE88DC, 0x6AFF0846,    0xBBACF2F5, 0x70E3E267,
   0xBF9FFFFF, 0xFFFC6F5E,    0x3FEF03F5, 0x6A88D17C,    0xBBACA2B2, 0x05D8EB45,
   0xBF8FFFFF, 0xFFFF580A,    0x3FEF80FE, 0xABFEF23A,    0xBB61AF65, 0xBD7BA074,
   0x00000000, 0x00000000,    0x3FF00000, 0x00000000,    0x00000000, 0x00000000,
   0x3F900000, 0x000039D1,    0x3FF04080, 0xAB55DF24,    0xBBA5A69A, 0xB0AA4DB6,
   0x3FA00000, 0x00040E38,    0x3FF08205, 0x60114966,    0xBBAD3180, 0x7B207EEE,
   0x3FA80000, 0x000000A0,    0x3FF0C492, 0x36829E91,    0x3BA01DE1, 0xDE261F9E,
   0x3FB00000, 0x00008504,    0x3FF1082B, 0x577D3DC7,    0x3B8DB72F, 0x752AE77F,
   0x3FB40000, 0x0000727E,    0x3FF14CD4, 0xFC98A493,    0xBBA8B371, 0x3285C6A1,
   0x3FB80000, 0x00014EB4,    0x3FF19293, 0x7074F7C7,    0x3B8A95AD, 0x4E3A0016,
   0x3FBC0000, 0x00007514,    0x3FF1D96B, 0x0EFF16A3,    0xBB8C7B78, 0xAC0BEE62,
   0x3FC00000, 0x0000387B,    0x3FF22160, 0x45B6FDCD,    0xBB833654, 0xFC2C852C,
   0x3FC20000, 0x00009DEE,    0x3FF26A77, 0x93F6181D,    0xBBA7415B, 0x71161E35,
   0x3FC40000, 0x000018F5,    0x3FF2B4B5, 0x8B37301F,    0x3BB0D4C5, 0x93F1E693,
   0x3FC60000, 0x00002679,    0x3FF3001E, 0xCF6020AD,    0x3B6A3633, 0xB13F9963,
   0x3FC80000, 0x0000D54A,    0x3FF34CB8, 0x170B785E,    0xBBB2D691, 0x1C45DA93,
   0x3FCA0000, 0x0001BC66,    0x3FF39A86, 0x2BD40516,    0xBBA4860B, 0xD2E696BD,
   0x3FCC0000, 0x0002F156,    0x3FF3E98D, 0xEAA192FD,    0x3BB1EAA6, 0x9AC1EC1A,
   0x3FCE0000, 0x00006E22,    0x3FF439D4, 0x43F602C0,    0x3BB1687C, 0x5C76A673,
   0x3FD00000, 0x0000344F,    0x3FF48B5E, 0x3C3E9251,    0x3BB0168A, 0xB7BA044E,
   0x3FD10000, 0x00005518,    0x3FF4DE30, 0xEC213A1F,    0x3BB16D9F, 0xF11FD1E1,
   0x3FD20000, 0x000F1F6D,    0x3FF53251, 0x80D4AF2B,    0x3BB5182E, 0x8AA41550,
   0x3FD30000, 0x00004CC9,    0x3FF587C5, 0x3C5A94C5,    0x3BADF0B5, 0x25890C47,
   0x3FD40000, 0x00003C35,    0x3FF5DE91, 0x76047488,    0x3B930965, 0x71BF40CF,
   0x3FD50000, 0x00001F9F,    0x3FF636BB, 0x9A983D52,    0x3B834A89, 0x43379035,
   0x3FD60000, 0x00001C8E,    0x3FF69049, 0x2CBF9E44,    0x3BAD13A5, 0x639824C3,
   0x3FD70000, 0x00002B17,    0x3FF6EB3F, 0xC55B2DE4,    0x3BAD1F08, 0xB4D7D659,
   0x3FD80000, 0x000018C8,    0x3FF747A5, 0x13DBF86E,    0xBB8B5F69, 0xBD7540CF
};
