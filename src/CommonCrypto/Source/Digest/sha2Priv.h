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
 
#ifndef _CC_SHA2_PRIV_H_
#define _CC_SHA2_PRIV_H_


/* 
 * This is a replacement for sha2.h; all types, consts, and prototypes are defined
 * in CommonDigest.h. We redefine them here so the original Gladman source is 
 * unmodified except for the include of sha2.h. 
 */
#include "CommonDigestPriv.h"
#include <CommonCrypto/CommonDigest.h>

#define SHA224_DIGEST_SIZE  CC_SHA224_DIGEST_LENGTH
#define SHA256_DIGEST_SIZE  CC_SHA256_DIGEST_LENGTH
#define SHA384_DIGEST_SIZE  CC_SHA384_DIGEST_LENGTH
#define SHA512_DIGEST_SIZE  CC_SHA512_DIGEST_LENGTH

#define SHA224_BLOCK_SIZE   CC_SHA224_BLOCK_BYTES
#define SHA256_BLOCK_SIZE   CC_SHA256_BLOCK_BYTES
#define SHA384_BLOCK_SIZE   CC_SHA384_BLOCK_BYTES
#define SHA512_BLOCK_SIZE   CC_SHA512_BLOCK_BYTES

#define SHA2_GOOD   0
#define SHA2_BAD    1

typedef void sha2_void;

typedef CC_LONG sha2_32t;
typedef CC_LONG64 sha2_64t;

typedef CC_SHA256_CTX sha224_ctx;	
typedef CC_SHA256_CTX sha256_ctx;
typedef CC_SHA512_CTX sha384_ctx;
typedef CC_SHA512_CTX sha512_ctx;

#define sha224_begin(c)			CC_SHA224_Init(c)
#define sha224_end(md, c)		CC_SHA224_Final(md, c)

#define sha256_begin(c)			CC_SHA256_Init(c)
#define sha256_end(md, c)		CC_SHA256_Final(md, c)

#define sha384_begin(c)			CC_SHA384_Init(c)
#define sha384_end(md, c)		CC_SHA384_Final(md, c)

#define sha512_begin(c)			CC_SHA512_Init(c)
#define sha512_end(md, c)		CC_SHA512_Final(md, c)

#endif  /* _CC_SHA2_PRIV_H_ */

