/*
 *  ccdrbg_impl.h
 *  corecrypto
 *
 *  Created on 01/03/2012
 *
 *  Copyright (c) 2012,2015 Apple Inc. All rights reserved.
 *
 */

#ifndef _CORECRYPTO_CCDRBG_IMPL_H_
#define _CORECRYPTO_CCDRBG_IMPL_H_

/* opaque drbg structure */
struct ccdrbg_state;

struct ccdrbg_info {
    /*! Size of the DRBG state in bytes **/
    size_t size;

    /*! Instantiate the PRNG
     @param prng       The PRNG state
     @param entropylen Length of entropy
     @param entropy    Entropy bytes
     @param inlen      Length of additional input
     @param in         Additional input bytes
     @return 0 if successful
     */
    int (*init)(const struct ccdrbg_info *info, struct ccdrbg_state *drbg,
                size_t entropyLength, const void* entropy,
                size_t nonceLength, const void* nonce,
                size_t psLength, const void* ps);

    /*! Add entropy to the PRNG
     @param prng       The PRNG state
     @param entropylen Length of entropy
     @param entropy    Entropy bytes
     @param inlen      Length of additional input
     @param in         Additional input bytes
     @return 0 if successful
     */
    int (*reseed)(struct ccdrbg_state *prng,
                  size_t entropylen, const void *entropy,
                  size_t inlen, const void *in);

    /*! Read from the PRNG in a FIPS Testing compliant manor
     @param prng    The PRNG state to read from
     @param out     [out] Where to store the data
     @param outlen  Length of data desired (octets)
     @param inlen   Length of additional input
     @param in      Additional input bytes
     @return 0 if successfull
     */
    int (*generate)(struct ccdrbg_state *prng,
                    size_t outlen, void *out,
                    size_t inlen, const void *in);

    /*! Terminate a PRNG state
     @param prng   The PRNG state to terminate
     */
    void (*done)(struct ccdrbg_state *prng);

    /** private parameters */
    const void *custom;
};



#endif // _CORECRYPTO_CCDRBG_IMPL_H_
