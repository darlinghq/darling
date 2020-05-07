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

#ifndef _AES_H
#define _AES_H

#if defined(__cplusplus)
extern "C"
{
#endif

#include <corecrypto/ccmode.h>
#include <corecrypto/ccn.h>

#define AES_BLOCK_SIZE  16  /* the AES block size in bytes          */

//Unholy HACK: this works because we know the size of the context for every
//possible corecrypto implementation is less than this.
#if defined(__ARM_NEON__) && !defined(__arm64__)        // for expanded keys in bit slice format
#define AES_CBC_CTX_MAX_SIZE (ccn_sizeof_size(sizeof(void *)) + ccn_sizeof_size(AES_BLOCK_SIZE) + ccn_sizeof_size(64*4) + (14-1)*128+32 )
#else
#define AES_CBC_CTX_MAX_SIZE (ccn_sizeof_size(sizeof(void *)) + ccn_sizeof_size(AES_BLOCK_SIZE) + ccn_sizeof_size(64*4))
#endif

typedef struct{
	cccbc_ctx_decl(AES_CBC_CTX_MAX_SIZE, ctx);
} aes_decrypt_ctx;

typedef struct{
	cccbc_ctx_decl(AES_CBC_CTX_MAX_SIZE, ctx);
} aes_encrypt_ctx;

typedef struct{
	aes_decrypt_ctx decrypt;
	aes_encrypt_ctx encrypt;
} aes_ctx;


/* for compatibility with old apis*/
#define aes_ret     int
#define aes_good    0
#define aes_error  -1
#define aes_rval    aes_ret



/* Key lengths in the range 16 <= key_len <= 32 are given in bytes, */
/* those in the range 128 <= key_len <= 256 are given in bits       */

aes_rval aes_encrypt_key(const unsigned char *key, int key_len, aes_encrypt_ctx cx[1]);
aes_rval aes_encrypt_key128(const unsigned char *key, aes_encrypt_ctx cx[1]);
aes_rval aes_encrypt_key256(const unsigned char *key, aes_encrypt_ctx cx[1]);

#if defined (__i386__) || defined (__x86_64__) || defined (__arm64__)
aes_rval aes_encrypt(const unsigned char *in, unsigned char *out, aes_encrypt_ctx cx[1]);
#endif

aes_rval aes_encrypt_cbc(const unsigned char *in_blk, const unsigned char *in_iv, unsigned int num_blk,
    unsigned char *out_blk, aes_encrypt_ctx cx[1]);


aes_rval aes_decrypt_key(const unsigned char *key, int key_len, aes_decrypt_ctx cx[1]);
aes_rval aes_decrypt_key128(const unsigned char *key, aes_decrypt_ctx cx[1]);
aes_rval aes_decrypt_key256(const unsigned char *key, aes_decrypt_ctx cx[1]);

#if defined (__i386__) || defined (__x86_64__) || defined (__arm64__)
aes_rval aes_decrypt(const unsigned char *in, unsigned char *out, aes_decrypt_ctx cx[1]);
#endif

aes_rval aes_decrypt_cbc(const unsigned char *in_blk, const unsigned char *in_iv, unsigned int num_blk,
    unsigned char *out_blk, aes_decrypt_ctx cx[1]);

aes_rval aes_encrypt_key_gcm(const unsigned char *key, int key_len, ccgcm_ctx *ctx);
aes_rval aes_encrypt_key_with_iv_gcm(const unsigned char *key, int key_len, const unsigned char *in_iv, ccgcm_ctx *ctx);
aes_rval aes_encrypt_set_iv_gcm(const unsigned char *in_iv, unsigned int len, ccgcm_ctx *ctx);
aes_rval aes_encrypt_reset_gcm(ccgcm_ctx *ctx);
aes_rval aes_encrypt_inc_iv_gcm(unsigned char *out_iv, ccgcm_ctx *ctx);
aes_rval aes_encrypt_aad_gcm(const unsigned char *aad, unsigned int aad_bytes, ccgcm_ctx *ctx);
aes_rval aes_encrypt_gcm(const unsigned char *in_blk, unsigned int num_bytes, unsigned char *out_blk, ccgcm_ctx *ctx);
aes_rval aes_encrypt_finalize_gcm(unsigned char *tag, unsigned int tag_bytes, ccgcm_ctx *ctx);
unsigned aes_encrypt_get_ctx_size_gcm(void);

aes_rval aes_decrypt_key_gcm(const unsigned char *key, int key_len, ccgcm_ctx *ctx);
aes_rval aes_decrypt_key_with_iv_gcm(const unsigned char *key, int key_len, const unsigned char *in_iv, ccgcm_ctx *ctx);
aes_rval aes_decrypt_set_iv_gcm(const unsigned char *in_iv, unsigned int len, ccgcm_ctx *ctx);
aes_rval aes_decrypt_reset_gcm(ccgcm_ctx *ctx);
aes_rval aes_decrypt_inc_iv_gcm(unsigned char *out_iv, ccgcm_ctx *ctx);
aes_rval aes_decrypt_aad_gcm(const unsigned char *aad, unsigned int aad_bytes, ccgcm_ctx *ctx);
aes_rval aes_decrypt_gcm(const unsigned char *in_blk, unsigned int num_bytes, unsigned char *out_blk, ccgcm_ctx *ctx);
aes_rval aes_decrypt_finalize_gcm(unsigned char *tag, unsigned int tag_bytes, ccgcm_ctx *ctx);
unsigned aes_decrypt_get_ctx_size_gcm(void);

#if defined(__cplusplus)
}
#endif

#endif
