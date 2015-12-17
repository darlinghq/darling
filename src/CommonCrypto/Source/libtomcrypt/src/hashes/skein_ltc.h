/*
 *  skein_ltc.h
 *  Skein-Test
 *
 *  Created by Jon Callas on 5/25/10.
 *  Copyright 2010 Apple, Inc. All rights reserved.
 *
 */

#include "tomcrypt.h"

#ifndef _SKEIN_LTC_H_
#define _SKEIN_LTC_H_	1
#ifdef LTC_SKEIN

enum {
	SKEIN512_128_LTC_TAG = 30,
	SKEIN512_160_LTC_TAG = 31,
	SKEIN512_224_LTC_TAG = 32,
	SKEIN512_256_LTC_TAG = 33,
	SKEIN512_384_LTC_TAG = 34,
	SKEIN512_512_LTC_TAG = 35
};

const struct ltc_hash_descriptor skein512_128_desc;
const struct ltc_hash_descriptor skein512_160_desc;
const struct ltc_hash_descriptor skein512_224_desc;
const struct ltc_hash_descriptor skein512_256_desc;
const struct ltc_hash_descriptor skein512_384_desc;
const struct ltc_hash_descriptor skein512_512_desc;
#endif
#endif
