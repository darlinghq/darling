/*
 *  ccmd5.h
 *  corecrypto
 *
 *  Created on 12/06/2010
 *
 *  Copyright (c) 2010,2011,2012,2015 Apple Inc. All rights reserved.
 *
 */

#ifndef _CORECRYPTO_CCMD5_H_
#define _CORECRYPTO_CCMD5_H_

#include <corecrypto/ccdigest.h>

#define CCMD5_BLOCK_SIZE   64
#define CCMD5_OUTPUT_SIZE  16
#define CCMD5_STATE_SIZE   16

/* Selector */
const struct ccdigest_info *ccmd5_di(void);

/* Implementations */
extern const struct ccdigest_info ccmd5_ltc_di;

#endif /* _CORECRYPTO_CCMD5_H_ */
