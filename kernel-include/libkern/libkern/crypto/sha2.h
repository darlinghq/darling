/*
 * copyright (c) 2012 apple computer, inc. all rights reserved.
 *
 * @apple_osreference_license_header_start@
 *
 * this file contains original code and/or modifications of original code
 * as defined in and that are subject to the apple public source license
 * version 2.0 (the 'license'). you may not use this file except in
 * compliance with the license. the rights granted to you under the license
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an apple operating system software license agreement.
 *
 * please obtain a copy of the license at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * the original code and all software distributed under the license are
 * distributed on an 'as is' basis, without warranty of any kind, either
 * express or implied, and apple hereby disclaims all such warranties,
 * including without limitation, any warranties of merchantability,
 * fitness for a particular purpose, quiet enjoyment or non-infringement.
 * please see the license for the specific language governing rights and
 * limitations under the license.
 *
 * @apple_osreference_license_header_end@
 */

#ifndef _CRYPTO_SHA2_H__
#define _CRYPTO_SHA2_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <corecrypto/ccsha2.h>

/*** SHA-256/384/512 Various Length Definitions ***********************/
#define SHA256_BLOCK_LENGTH             CCSHA256_BLOCK_SIZE
#define SHA256_DIGEST_LENGTH    CCSHA256_OUTPUT_SIZE
#define SHA256_DIGEST_STRING_LENGTH     (SHA256_DIGEST_LENGTH * 2 + 1)
#define SHA384_BLOCK_LENGTH             CCSHA512_BLOCK_SIZE
#define SHA384_DIGEST_LENGTH    CCSHA384_OUTPUT_SIZE
#define SHA384_DIGEST_STRING_LENGTH     (SHA384_DIGEST_LENGTH * 2 + 1)
#define SHA512_BLOCK_LENGTH             CCSHA512_BLOCK_SIZE
#define SHA512_DIGEST_LENGTH    CCSHA512_OUTPUT_SIZE
#define SHA512_DIGEST_STRING_LENGTH     (SHA512_DIGEST_LENGTH * 2 + 1)

typedef struct {
	ccdigest_ctx_decl(CCSHA256_STATE_SIZE, CCSHA256_BLOCK_SIZE, ctx);
} SHA256_CTX;

typedef struct SHA512_CTX {
	ccdigest_ctx_decl(CCSHA512_STATE_SIZE, CCSHA512_BLOCK_SIZE, ctx);
} SHA512_CTX;

typedef SHA512_CTX SHA384_CTX;

/*** SHA-256/384/512 Function Prototypes ******************************/

void SHA256_Init(SHA256_CTX *ctx);
void SHA256_Update(SHA256_CTX *ctx, const void *data, size_t len);
void SHA256_Final(void *digest, SHA256_CTX *ctx);

void SHA384_Init(SHA384_CTX *ctx);
void SHA384_Update(SHA384_CTX *ctx, const void *data, size_t len);
void SHA384_Final(void *digest, SHA384_CTX *ctx);

void SHA512_Init(SHA512_CTX *ctx);
void SHA512_Update(SHA512_CTX *ctx, const void *data, size_t len);
void SHA512_Final(void *digest, SHA512_CTX *ctx);

#ifdef  __cplusplus
}
#endif /* __cplusplus */

#endif /* _CRYPTO_SHA2_H__ */
