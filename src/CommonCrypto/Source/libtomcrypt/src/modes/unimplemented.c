/* 
 * Copyright (c) 2004 Apple Computer, Inc. All Rights Reserved.
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

#include "tomcrypt.h"

// Filler routines for unimplemented mode methods.

const struct ltc_mode_descriptor modeUNIMP_desc =
{
	NULL,
	0,
	0,
	0,
	unimp_mode_setup,
	unimp_mode_encrypt, 
	unimp_mode_decrypt, 
	unimp_mode_encrypt_tweaked, 
	unimp_mode_decrypt_tweaked, 
	unimp_mode_done, 
	unimp_mode_setiv,
	unimp_mode_getiv,
};


int unimp_mode_setup(int cipher, const unsigned char *IV, const unsigned char *key, int keylen, 
const unsigned char *tweak, int tweaklen, int num_rounds, int options, mode_context *ctx) { return CRYPT_UNIMPLEMENTED; }

int unimp_mode_encrypt(const unsigned char *pt, unsigned char *ct, unsigned long len, mode_context *ctx) { return CRYPT_UNIMPLEMENTED; }

int unimp_mode_decrypt(const unsigned char *ct, unsigned char *pt, unsigned long len, mode_context *ctx) { return CRYPT_UNIMPLEMENTED; }

int unimp_mode_encrypt_tweaked(const unsigned char *pt, unsigned long len, unsigned char *ct, const unsigned char *tweak, mode_context *ctx) { return CRYPT_UNIMPLEMENTED; }

int unimp_mode_decrypt_tweaked(const unsigned char *ct, unsigned long len, unsigned char *pt, const unsigned char *tweak, mode_context *ctx) { return CRYPT_UNIMPLEMENTED; }

int unimp_mode_done(mode_context *ctx) { return CRYPT_UNIMPLEMENTED; }

int unimp_mode_setiv(const unsigned char *IV, unsigned long len, mode_context *ctx) { return CRYPT_UNIMPLEMENTED; }

int unimp_mode_getiv(const unsigned char *IV, unsigned long *len, mode_context *ctx) { return CRYPT_UNIMPLEMENTED; }

