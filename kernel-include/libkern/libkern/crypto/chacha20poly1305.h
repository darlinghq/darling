/*
 * Copyright (c) 2017 Apple Computer, Inc. All rights reserved.
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

#ifndef _CHACHA20POLY1305_H
#define _CHACHA20POLY1305_H

#if defined(__cplusplus)
extern "C"
{
#endif

#include <corecrypto/ccchacha20poly1305.h>

typedef ccchacha20poly1305_ctx chacha20poly1305_ctx;

int     chacha20poly1305_init(chacha20poly1305_ctx *ctx, const uint8_t *key);
int chacha20poly1305_reset(chacha20poly1305_ctx *ctx);
int chacha20poly1305_setnonce(chacha20poly1305_ctx *ctx, const uint8_t *nonce);
int chacha20poly1305_incnonce(chacha20poly1305_ctx *ctx, uint8_t *nonce);
int     chacha20poly1305_aad(chacha20poly1305_ctx *ctx, size_t nbytes, const void *aad);
int     chacha20poly1305_encrypt(chacha20poly1305_ctx *ctx, size_t nbytes, const void *ptext, void *ctext);
int     chacha20poly1305_finalize(chacha20poly1305_ctx *ctx, uint8_t *tag);
int     chacha20poly1305_decrypt(chacha20poly1305_ctx *ctx, size_t nbytes, const void *ctext, void *ptext);
int     chacha20poly1305_verify(chacha20poly1305_ctx *ctx, const uint8_t *tag);

#if defined(__cplusplus)
}
#endif

#endif
