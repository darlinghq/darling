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
 *  crypt_mode_descriptor.c
 *  MacTomCrypt
 */
#include "tomcrypt.h"

/*
 
 This is implemented slightly differently from Tom's cipher descriptor table.  All entries are static and the order and ID match.  Not only that, but the 
 modes advertised by CommonCryptor.h match the IDs and hence the array index.  That's currently defined like this:

enum {
	kCCModeECB		= 1,
	kCCModeCBC		= 2,
	kCCModeCFB		= 3,
	kCCModeCTR		= 4,
	kCCModeF8		= 5, // Not included
	kCCModeLRW		= 6, // Not included
	kCCModeOFB		= 7,
	kCCModeXTS		= 8,
	kCCModeRC4		= 9, // RC4 as a streaming cipher is handled internally as a mode.
};
typedef uint32_t CCMode;

*/

mode_descriptor_ptr mode_descriptor[TAB_SIZE] = {
	&modeUNIMP_desc,
	&modeECB_desc,
	&modeCBC_desc,
	&modeCFB_desc,
	&modeCTR_desc,
	&modeUNIMP_desc,
	&modeUNIMP_desc,
	&modeOFB_desc,
	&modeXTS_desc,
	&modeRC4_desc,
	&modeCFB8_desc,
 };
