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

#ifndef _CRYPTO_DES_H
#define _CRYPTO_DES_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <corecrypto/ccmode.h>
#include <corecrypto/ccdes.h>
#include <corecrypto/ccn.h>

/* must be 32bit quantity */
#define DES_LONG u_int32_t

typedef unsigned char des_cblock[8];

/* Unholy hack: this is currently the size for the only implementation of DES in corecrypto */
#define DES_ECB_CTX_MAX_SIZE (64*4)
#define DES3_ECB_CTX_MAX_SIZE (64*4*3)


typedef struct{
	ccecb_ctx_decl(DES_ECB_CTX_MAX_SIZE, enc);
	ccecb_ctx_decl(DES_ECB_CTX_MAX_SIZE, dec);
} des_ecb_key_schedule;

typedef struct{
	ccecb_ctx_decl(DES3_ECB_CTX_MAX_SIZE, enc);
	ccecb_ctx_decl(DES3_ECB_CTX_MAX_SIZE, dec);
} des3_ecb_key_schedule;

/* Only here for backward compatibility with smb kext */
typedef des_ecb_key_schedule des_key_schedule[1];
#define des_set_key des_ecb_key_sched

#define DES_ENCRYPT     1
#define DES_DECRYPT     0


/* Single DES ECB - 1 block */
int des_ecb_key_sched(des_cblock *key, des_ecb_key_schedule *ks);
int des_ecb_encrypt(des_cblock * in, des_cblock *out, des_ecb_key_schedule *ks, int encrypt);

/* Triple DES ECB - 1 block */
int des3_ecb_key_sched(des_cblock *key, des3_ecb_key_schedule *ks);
int des3_ecb_encrypt(des_cblock *block, des_cblock *, des3_ecb_key_schedule *ks, int encrypt);

int des_is_weak_key(des_cblock *key);

#ifdef  __cplusplus
}
#endif

#endif
