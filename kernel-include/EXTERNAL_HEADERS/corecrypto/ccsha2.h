/*
 *  ccsha2.h
 *  corecrypto
 *
 *  Created on 12/03/2010
 *
 *  Copyright (c) 2010,2011,2012,2014,2015 Apple Inc. All rights reserved.
 *
 */

#ifndef _CORECRYPTO_CCSHA2_H_
#define _CORECRYPTO_CCSHA2_H_

#include <corecrypto/ccdigest.h>

/* sha2 selectors */
const struct ccdigest_info *ccsha224_di(void);
const struct ccdigest_info *ccsha256_di(void);
const struct ccdigest_info *ccsha384_di(void);
const struct ccdigest_info *ccsha512_di(void);

/* TODO: Placeholders */
#define ccoid_sha224 ((unsigned char *)"\x06\x09\x60\x86\x48\x01\x65\x03\x04\x02\x04")
#define ccoid_sha224_len 11

#define ccoid_sha256 ((unsigned char *)"\x06\x09\x60\x86\x48\x01\x65\x03\x04\x02\x01")
#define ccoid_sha256_len 11

#define ccoid_sha384 ((unsigned char *)"\x06\x09\x60\x86\x48\x01\x65\x03\x04\x02\x02")
#define ccoid_sha384_len 11

#define ccoid_sha512 ((unsigned char *)"\x06\x09\x60\x86\x48\x01\x65\x03\x04\x02\x03")
#define ccoid_sha512_len 11


/* SHA256 */
#define CCSHA256_BLOCK_SIZE  64
#define	CCSHA256_OUTPUT_SIZE 32
#define	CCSHA256_STATE_SIZE  32
extern const struct ccdigest_info ccsha256_ltc_di;
#if  CCSHA2_VNG_INTEL
extern const struct ccdigest_info ccsha224_vng_intel_SupplementalSSE3_di;
extern const struct ccdigest_info ccsha256_vng_intel_SupplementalSSE3_di;
#endif
#if  CCSHA2_VNG_ARM
extern const struct ccdigest_info ccsha224_vng_arm_di;
extern const struct ccdigest_info ccsha256_vng_arm_di;
#if CC_ACCELERATECRYPTO && defined(__arm64__) && CCSHA2_VNG_ARM
extern const struct ccdigest_info ccsha256_vng_arm64neon_di;
#endif  // CC_ACCELERATECRYPTO
extern const struct ccdigest_info ccsha384_vng_arm_di;
extern const struct ccdigest_info ccsha512_vng_arm_di;
#endif

/* SHA224 */
#define	CCSHA224_OUTPUT_SIZE 28
extern const struct ccdigest_info ccsha224_ltc_di;

/* SHA512 */
#define CCSHA512_BLOCK_SIZE  128
#define	CCSHA512_OUTPUT_SIZE  64
#define	CCSHA512_STATE_SIZE   64
extern const struct ccdigest_info ccsha512_ltc_di;

/* SHA384 */
#define	CCSHA384_OUTPUT_SIZE  48
extern const struct ccdigest_info ccsha384_ltc_di;

#endif /* _CORECRYPTO_CCSHA2_H_ */
