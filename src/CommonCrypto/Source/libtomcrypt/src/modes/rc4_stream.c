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
#include "ccdebug.h"
// Wrench RC4 into a common interface.

const struct ltc_mode_descriptor modeRC4_desc =
{
	"RC4",
	10,
	1,
	sizeof(RC4_KEY),
	&rc4_stream_setup,
	&rc4_stream_encrypt, 
	&rc4_stream_decrypt, 
	&unimp_mode_encrypt_tweaked, 
	&unimp_mode_decrypt_tweaked, 
	&rc4_stream_done, 
	&unimp_mode_setiv,
	&unimp_mode_getiv,
};


int rc4_stream_setup(int cipher, const unsigned char *IV, const unsigned char *key, int keylen, 
const unsigned char *tweak, int tweaklen, int num_rounds, int options, RC4_KEY *ctx) {
	CC_RC4_set_key(ctx, keylen, key);
	return CRYPT_OK; 
}

int rc4_stream_encrypt(const unsigned char *pt, unsigned char *ct, unsigned long len, mode_context *ctx) {
	ccdebug(ASL_LEVEL_ERR, " RC4 Processing %d bytes\n"	, len);
	CC_RC4(ctx, len, pt, ct);
	return CRYPT_OK; 
}

int rc4_stream_decrypt(const unsigned char *ct, unsigned char *pt, unsigned long len, mode_context *ctx) {
	ccdebug(ASL_LEVEL_ERR, " RC4 Processing %d bytes\n"	, len);
	CC_RC4(ctx, len, ct, pt);
	return CRYPT_OK; 
}

int rc4_stream_done(mode_context *ctx) { 
	memset(ctx, 0, sizeof(RC4_KEY));
	return CRYPT_OK; 
}
