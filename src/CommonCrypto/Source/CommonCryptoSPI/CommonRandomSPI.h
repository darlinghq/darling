/*
 * Copyright (c) 2010 Apple Inc. All Rights Reserved.
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

/*!
    @header CommonRandom
    The functions provided in CommonRandom.h implement accessors
    to cryptographically secure random numbers.
*/

#ifndef _COMMON_SECRANDOM_H_
#define _COMMON_SECRANDOM_H_

#include <Availability.h>
#include <stdint.h>
#include <sys/types.h>

/*
    When this becomes API we should really make a CommonCrypto/CCErrors.h and
    centralize CC error codes
 */

#include <CommonCrypto/CommonCryptor.h>

#if defined(__cplusplus)
extern "C" {
#endif


/*!
    @typedef SecRandomRef
    @abstract Reference to a (psuedo) random number generator.
*/

typedef struct __CCRandom *CCRandomRef;


extern CCRandomRef kCCRandomDefault
    __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);
    
/*!
    @function CCRandomCopyBytes
    @abstract Return count random bytes in *bytes, allocated by the caller.
    @result Return kCCSuccess on success, kCCParamError for parameter error
        (unimplemented PRNG) or -1 if something went wrong, check errno
        to find out the real error.
    @discussion The default PRNG returns cryptographically strong random bits
        suitable for use as cryptographic keys, IVs, nonces etc.
*/
    
int CCRandomCopyBytes(CCRandomRef rnd, void *bytes, size_t count)
    __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);

    
#if defined(__cplusplus)
}
#endif

#endif /* !_COMMON_SECRANDOM_H_ */
