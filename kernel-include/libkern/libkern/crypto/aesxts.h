/*
 * Copyright (c) 2012 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

#ifndef _CRYPTO_AESXTS_H
#define _CRYPTO_AESXTS_H

#if defined(__cplusplus)
extern "C"
{
#endif

#include <corecrypto/ccmode.h>
#include <corecrypto/ccaes.h>
#include <corecrypto/ccn.h>

//Unholy HACK: this works because we know the size of the context for every
//possible corecrypto implementation is less than this.
#define AES_XTS_CTX_MAX_SIZE (ccn_sizeof_size(3*sizeof(void *)) + 2*ccn_sizeof_size(128*4) + ccn_sizeof_size(16))

typedef struct {
	ccxts_ctx_decl(AES_XTS_CTX_MAX_SIZE, enc);
	ccxts_ctx_decl(AES_XTS_CTX_MAX_SIZE, dec);
} symmetric_xts;


/*
 * These are the interfaces required for XTS-AES support
 */

uint32_t
xts_start(uint32_t cipher, // ignored - we're doing this for xts-aes only
    const uint8_t *IV,               // ignored
    const uint8_t *key1, int keylen,
    const uint8_t *key2, int tweaklen,               // both keys are the same size for xts
    uint32_t num_rounds,               // ignored
    uint32_t options,                  // ignored
    symmetric_xts *xts);

int xts_encrypt(const uint8_t *pt, unsigned long ptlen,
    uint8_t *ct,
    const uint8_t *tweak,                             // this can be considered the sector IV for this use
    symmetric_xts *xts);

int xts_decrypt(const uint8_t *ct, unsigned long ptlen,
    uint8_t *pt,
    const uint8_t *tweak,                             // this can be considered the sector IV for this use
    symmetric_xts *xts);

void xts_done(symmetric_xts *xts);

#if defined(__cplusplus)
}
#endif

#endif
