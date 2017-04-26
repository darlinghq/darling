#ifndef  COMMONRANDOM_H
#define  COMMONRANDOM_H 1

/*
 *  CommonRandom.h
 *
 * Copyright © 2010-2011 by Apple, Inc. All rights reserved.
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
 *
 */

#include <dispatch/dispatch.h>
#include <dispatch/queue.h>
#include <Availability.h>
#include <stdint.h>
#include <sys/types.h>
#include <CommonCrypto/CommonCryptor.h>

/*!

	@header     CommonRNG.h
	@abstract   An interface to a system random number generator. This module
	            provides a managed way either to get random numbers from a
	            NIST-approved random number generator or /dev/random. The NIST
	            random number generator gets its entropy from /dev/random, but
	            operates 9x-10x faster than it.

    @discussion It is inconvenient to call system random number generators
				directly. In the simple case of calling /dev/random, the caller
				has to open the device and close it in addition to managing it
				while it's open. This module has as its immediate raison d'être
				the inconvenience of doing this. It manages a file descriptor to
				/dev/random including the exception processing of what happens
				in a fork() and exec(). Call CCRandomCopyBytes() and all the
				fiddly bits are managed for you. Just get on with whatever you
				were really trying to do.
				
				More importantly, though, it also manages a FIPS 140-compliant
				way to get random numbers. NIST created in their document SP
				800-90 a new type of AES-based "Deterministic Random Bit
				Generator" (DRBG) (what is often called a PRNG) and guidelines
				on how to use it. There are two reasons to prefer it over
				directly calling /dev/random. It's a standard and immediately
				compliant with FIPS 140, and it is dramatically faster per-byte.
				For complete disclosure, this implements an AES-CTR DRBG with
				derivation function using AES-128 as the cipher and prediction
				resistance.

				Thus, we provide two RNGs to call, kCCRandomDefault (the NIST
				one) and kCCRandomDevRandom (a managed wrapper around
				/dev/random). If you are doing anything involving security, call
				the default one. You'll be glad you did, because it does much
				security-related housekeeping for you and you don't have to
				think about it. Really.

				In implementation details, the first time you call
				CCRandomCopyBytes(), it will open up /dev/random and seed the RNG
				with 64 bytes. After each call, there is a reseed operation that
				happens on an async GCD queue that reseeds with 32 bytes and a
				nonce from mach_absolute_time(). All access to the internal DRBG
				is serialized through a GCD queue and is therefore thread safe. 

				Should you need to create your own RNG context or have a secondary
				RNG context, CCRNGCreate() and CCRNGRelease() will let you create
				an RNG yourself and then call CCRandomCopyBytes() with that 
				context.
 */

#include <CommonCrypto/CommonRandom.h>

#if defined(__cplusplus)
extern "C" {
#endif

/*!
    @typedef    CCRandomRef
    @abstract   Abstract Reference to a random number generator.

*/
#ifndef  COMMONRANDOMPRIV_H // Check for the private header
typedef struct __CCRandom *CCRandomRef;
#endif

/*!
 @function      CCRandomCopyBytes
 
 @abstract      Return random bytes in a buffer allocated by the caller.
 
 @discussion    The default PRNG returns cryptographically strong random 
                bits suitable for use as cryptographic keys, IVs, nonces etc.
                
 @param         rnd     The random number generator to use. Pre-defined values:
                        kCCRandomDefault, the NIST AES-based one and
                        kCCRandomDevRandom, /dev/random itself.
 
						Alternately, you can create one with CCRNGCreate().
 
 @param         bytes   Pointer to the return buffer.
 @param         count   Number of random bytes to return.

 @result        Return kCCSuccess on success.  Other values are ...
 */

int CCRandomCopyBytes(CCRandomRef rnd, void *bytes, size_t count)
__OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_5_0);
	
extern const CCRandomRef kCCRandomDefault
__OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_5_0);
	
extern const CCRandomRef kCCRandomDevRandom
__OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_5_0);

/*!
 @function      CCRNGCreate
 
 @abstract      Create an RNG context.
 
 @discussion    This creates a CCRandomRef that you can then pass into
				CCRandomCopyBytes(). Only call this if you need to create
				your own context. You can call CCRandomCopyBytes() with this
				context. Remember to release it.
 
 @param			options	Option flags. See below. Unless you have a very
						good reason, just use kCCRNGOptionCryptoRNG.
 
 @param			rngRef	A pointer to a CCRandomRef.
 
 @result		Returns kCCSuccess on success.
 
 
 */
	
CCRNGStatus
CCRNGCreate(uint32_t options, CCRandomRef *rngRef)
__OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_5_0);

/*!
 @function      CCRNGRelease
 
 @abstract      Release an RNG context.
 
 @discussion    This releases and deallocates a context.
 
 @param			rng		A CCRandomRef.
 
 @result		Returns kCCSuccess on success.
 
 
 */

CCRNGStatus
CCRNGRelease(CCRandomRef rng)
__OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_5_0);

	
/*
 Options flags
 
 The option flags are not exposed through the default use of CCRandomGetBytes(). 
 They are only exposed through direct use of a CCRandomRef.

 The polarity is reversed here for two reasons. One is that I want people to
 think before they make a non-FIPS, predictable RNG. If you're doing any sort of
 crypto, you want FIPS and you want prediction resistance. Prediction resistance
 reseeds after every query which is slightly slower, but more secure. Non-FIPS
 is about 20% faster for very large reads, where very large means well over a MB
 per get, which you will probably never do. If you pull under 500 bytes from the
 RNG, there is *NO* change in performance for non-FIPS.

 Non-FIPS makes two changes. First, it increments the counter in machine-natural
 order, which on little-endian machines makes a very small performance
 improvement. It saves you two byte-swaps for every 32-bit increment of the
 counter, for every int that has to be incremented, which is admittedly not
 much. It is so much not much that this is a compile-time option in the DRBG,
 and likely to be turned off.

 But something that makes a difference is that it  reads from the DRBG in one
 lump sum, instead of in 500 byte chunks, as FIPS demands. On a 50MB test, runs
 about 20% faster, but obviously for 500 bytes would run the same.

 Arguably, we should remove the non-FIPS thing because in most circumstances it
 matters naught. Also, as we've said before, if you're interested in security,
 you shouldn't be worrying about a small performance tweaks.

 Prediction resistance re-seeds the DRBG after every request with 32 bytes from
 /dev/random and a timestamp from mach_absolute_time(). This is a legitimate
 thing you might want and a difference between a "random" and a "urandom"
 variant.
 
*/

enum {
	kCCRNGOptionIgnoreFIPS				= 0x00000001,
	kCCRNGOptionNoPredictionResistance	= 0x00000002,
	
	kCCRNGOptionCryptoRNG				= 0x00000000,
};
    
// Accessor functions to get the rng "states" for internal Security Framework
// use.
#include <corecrypto/ccdrbg.h>
#include <corecrypto/ccrng_system.h>

struct ccrng_state *ccDevRandomGetRngState(void)
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);

struct ccrng_state *ccDRBGGetRngState(void)
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);


#if defined(__cplusplus)
}
#endif

#endif /* COMMONRANDOM_H */

