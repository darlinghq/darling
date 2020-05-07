/*
 *  ccrng.h
 *  corecrypto
 *
 *  Created on 12/13/2010
 *
 *  Copyright (c) 2010,2011,2013,2014,2015 Apple Inc. All rights reserved.
 *
 */

#ifndef _CORECRYPTO_CCRNG_H_
#define _CORECRYPTO_CCRNG_H_

#include <corecrypto/cc.h>

#define CCRNG_STATE_COMMON \
    int (*generate)(struct ccrng_state *rng, size_t outlen, void *out);

/*!
 @type      struct ccrng_state
 @abstract  Default state structure. Do not instantiate. ccrng() returns a reference to this structure
 */
struct ccrng_state {
    CCRNG_STATE_COMMON
};

/*!
 @function   ccrng
 @abstract   Initializes an AES-CTR mode cryptographic random number generator and returns the statically-allocated rng object.
             Getting a pointer to a ccrng has never been simpler!
             Call this function, get an rng object and then pass the object to ccrng_generate() to generate randoms.
             ccrng() may be called more than once. It returns pointer to the same object on all calls.

 @result  a cryptographically secure random number generator or NULL if fails

 @discussion
 - It is significantly faster than using the system /dev/random
 - FIPS Compliant: NIST SP800-80A + FIPS 140-2
 - Seeded from the system entropy.
 - Provides at least 128bit security if the system provide 2bit of entropy / byte.
 - Entropy accumulation
 - Backtracing resistance
 - Prediction break with frequent (asynchronous) reseed
 */

struct ccrng_state *ccrng(int *error);

/*!
 @function   ccrng_generate
 @abstract   Generate `outlen` bytes of output, stored in `out`, using ccrng_state `rng`.

 @param rng  `struct ccrng_state` representing the state of the RNG.
 @param outlen  Amount of random bytes to generate.
 @param out  Pointer to memory where random bytes are stored, of size at least `outlen`.

 @result 0 on success and nonzero on failure.
 */
#define ccrng_generate(rng, outlen, out) \
    ((rng)->generate((struct ccrng_state *)(rng), (outlen), (out)))

/*!
  @function ccrng_uniform
  @abstract Generate a random value in @p [0, bound).

  @param rng   The state of the RNG.
  @param bound The exclusive upper bound on the output.
  @param rand  A pointer to a single @p uint64_t to store the result.

  @result Returns zero iff the operation is successful.
 */
int ccrng_uniform(struct ccrng_state *rng, uint64_t bound, uint64_t *rand);

#endif /* _CORECRYPTO_CCRNG_H_ */
