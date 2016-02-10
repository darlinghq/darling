/*
 * Copyright (c) 2010 Apple Inc. All Rights Reserved.
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
 *  aesxts.h
 *
 *
 */

#include "stdint.h"
#include "tomcrypt.h"

#ifndef _AESXTS_H
#define _AESXTS_H

#if defined(__cplusplus)
extern "C"
{
#endif

/*
 * The context for XTS-AES
 */

#ifdef NEVER
#define KS_LENGTH       60

typedef struct {   
	uint32_t ks[KS_LENGTH];
	uint32_t rn;
} aesedp_encrypt_ctx;

typedef struct {   
	uint32_t ks[KS_LENGTH];
	uint32_t rn;
} aesedp_decrypt_ctx;

typedef struct {   
	aesedp_decrypt_ctx decrypt;
	aesedp_encrypt_ctx encrypt;
} aesedp_ctx;

// xts mode context

typedef struct {
   aesedp_ctx				key1, key2;
   uint32_t						cipher; // ignore - this is to fit with the library, but in this case we're only using aes
} symmetric_xts;

#endif
    
/*
 * These are the interfaces required for XTS-AES support
 */
 
uint32_t
aesxts_start(uint32_t cipher, // ignored - we're doing this for xts-aes only
						const uint8_t *IV, // ignored
						const uint8_t *key1, int keylen,
						const uint8_t *key2, int tweaklen, // both keys are the same size for xts
						uint32_t num_rounds, // ignored
						uint32_t options,    // ignored
						symmetric_xts *xts);

int aesxts_encrypt(
   const uint8_t *pt, unsigned long ptlen,
         uint8_t *ct,
   const uint8_t *tweak, // this can be considered the sector IV for this use
         symmetric_xts *xts);
		 
int aesxts_decrypt(
   const uint8_t *ct, unsigned long ptlen,
         uint8_t *pt,
   const uint8_t *tweak, // this can be considered the sector IV for this use
         symmetric_xts *xts);


void aesxts_done(symmetric_xts *xts);

#if defined(__cplusplus)
}
#endif

#endif /* _AESXTS_H */
