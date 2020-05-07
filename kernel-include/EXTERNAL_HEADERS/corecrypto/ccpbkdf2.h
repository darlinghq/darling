/*
 *  ccpbkdf2.h
 *  corecrypto
 *
 *  Created on 12/15/2010
 *
 *  Copyright (c) 2010,2011,2012,2015 Apple Inc. All rights reserved.
 *
 */

#ifndef _CORECRYPTO_CCPBKDF2_H_
#define _CORECRYPTO_CCPBKDF2_H_

#include <corecrypto/ccdigest.h>

/*! @function ccpbkdf2_hmac
    @abstract perform a pbkdf2 using HMAC(di) for the PRF (see PKCS#5 for specification)
    @discussion This performs a standard PBKDF2 transformation of password and salt through 
an HMAC PRF of the callers slection (any Digest, typically SHA-1) returning dkLen bytes
containing the entropy.

Considerations:
The salt used should be at least 8 bytes long. Each session should use it's own salt.
We use the password as the key for the HMAC and the running data as the text for the HMAC to make a PRF.
SHA-1 is a good hash to use for the core of the HMAC PRF.
    @param di           digest info defining the digest type to use in the PRF.
    @param passwordLen  amount of data to be fed in
    @param password     data to be fed into the PBKDF
    @param saltLen      length of the salt
    @param salt         salt to be used in pbkdf
    @param iterations   itrations to go
    @param dkLen        length of the results
    @param dk           buffer for the results of the PBKDF tranformation, must be dkLen big
 
 */
int ccpbkdf2_hmac(const struct ccdigest_info *di,
                   size_t passwordLen, const void *password,
                   size_t saltLen, const void *salt,
                   size_t iterations,
                   size_t dkLen, void *dk);

#endif /* _CORECRYPTO_CCPBKDF2_H_ */
