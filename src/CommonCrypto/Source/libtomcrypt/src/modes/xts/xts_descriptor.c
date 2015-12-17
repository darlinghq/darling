/* 
 * Copyright (c) 2006 Apple Computer, Inc. All Rights Reserved.
 * 
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
 
/*
 *  xts_descriptor.c
 *  MacTomCrypt
 */

#include "tomcrypt.h"
#if defined(LTC_XTS_MODE) && defined(MACTOMCRYPT)

const struct ltc_mode_descriptor modeXTS_desc =
{
	"XTS",
	8,			/* Must match 	kCCModeXTS = 8 */
	1,
	sizeof(symmetric_xts),
	&xts_start,
	&unimp_mode_encrypt, 
	&unimp_mode_decrypt, 
	&xts_encrypt, 
	&xts_decrypt, 
	&xts_done, 
	&unimp_mode_setiv,
	&unimp_mode_getiv,
};

#endif /* LTC_XTS_MODE */
