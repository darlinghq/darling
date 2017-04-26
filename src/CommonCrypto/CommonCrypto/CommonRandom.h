/*
 * Copyright (c) 2014 Apple Inc. All Rights Reserved.
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

//
//  CommonRandom.h
//  CommonCrypto

#ifndef CommonCrypto_CommonRandom_h
#define CommonCrypto_CommonRandom_h

#if defined(__cplusplus)
extern "C" {
#endif	

typedef CCCryptorStatus CCRNGStatus;
    
/*!
 @function      CCRandomGenerateBytes
 
 @abstract      Return random bytes in a buffer allocated by the caller.
 
 @discussion    The PRNG returns cryptographically strong random
                bits suitable for use as cryptographic keys, IVs, nonces etc.
 
 @param         bytes   Pointer to the return buffer.
 @param         count   Number of random bytes to return.

 @result        Return kCCSuccess on success.
 */

CCRNGStatus CCRandomGenerateBytes(void *bytes, size_t count)
__OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_8_0);
    
#if defined(__cplusplus)
}
#endif

#endif
