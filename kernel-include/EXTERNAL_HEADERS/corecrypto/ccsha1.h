/*
 *  ccsha1.h
 *  corecrypto
 *
 *  Created on 12/01/2010
 *
 *  Copyright (c) 2010,2011,2012,2014,2015 Apple Inc. All rights reserved.
 *
 */

#ifndef _CORECRYPTO_CCSHA1_H_
#define _CORECRYPTO_CCSHA1_H_

#include <corecrypto/ccdigest.h>
#include <corecrypto/cc_config.h>

#define CCSHA1_BLOCK_SIZE   64
#define CCSHA1_OUTPUT_SIZE  20
#define CCSHA1_STATE_SIZE   20

/* sha1 selector */
const struct ccdigest_info *ccsha1_di(void);

/* Implementations */
extern const struct ccdigest_info ccsha1_ltc_di;
extern const struct ccdigest_info ccsha1_eay_di;

#if  CCSHA1_VNG_INTEL
extern const struct ccdigest_info ccsha1_vng_intel_SupplementalSSE3_di;
#endif

#if  CCSHA1_VNG_ARM
extern const struct ccdigest_info ccsha1_vng_arm_di;
#endif

/* TODO: Placeholders */
#define ccoid_sha1 ((unsigned char *)"\x06\x05\x2b\x0e\x03\x02\x1a")
#define ccoid_sha1_len 7

#endif /* _CORECRYPTO_CCSHA1_H_ */
