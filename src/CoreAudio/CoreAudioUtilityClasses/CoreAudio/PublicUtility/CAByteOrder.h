/*
     File: CAByteOrder.h
 Abstract: Part of CoreAudio Utility Classes
  Version: 1.1
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
 Inc. ("Apple") in consideration of your agreement to the following
 terms, and your use, installation, modification or redistribution of
 this Apple software constitutes acceptance of these terms.  If you do
 not agree with these terms, please do not use, install, modify or
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following
 text and disclaimers in all such redistributions of the Apple Software.
 Neither the name, trademarks, service marks or logos of Apple Inc. may
 be used to endorse or promote products derived from the Apple Software
 without specific prior written permission from Apple.  Except as
 expressly stated in this notice, no other rights or licenses, express or
 implied, are granted by Apple herein, including but not limited to any
 patent rights that may be infringed by your derivative works or by other
 works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
 MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
 OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
 AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 
 Copyright (C) 2014 Apple Inc. All Rights Reserved.
 
*/
#if !defined(__CAByteOrder_h__)
#define __CAByteOrder_h__

//=============================================================================
//	Includes
//=============================================================================

//	System Includes
#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
	#include <CoreFoundation/CoreFoundation.h>
#else
	#include "CoreFoundation.h"
#endif


#if defined(__cplusplus)
extern "C" {
#endif

CF_INLINE Float32 CASwapFloat32 (Float32 arg) {
	union {
		Float32 f;
		UInt32 i;
	} flip;

	flip.f = arg;
	flip.i = CFSwapInt32 (flip.i);
	
	return flip.f;
}

CF_INLINE Float64 CASwapFloat64 (Float64 arg) {
	union {
		Float64 f;
		UInt64 i;
	} flip;

	flip.f = arg;
	flip.i = CFSwapInt64 (flip.i);
	
	return flip.f;
}

#pragma mark -Flippers

CF_INLINE Float32 CASwapFloat32BigToHost(Float32 arg) {
#if defined(__BIG_ENDIAN__)
    return arg;
#else
	return CASwapFloat32(arg);
#endif
}

CF_INLINE Float64 CASwapFloat64BigToHost(Float64 arg) {
#if defined(__BIG_ENDIAN__)
    return arg;
#else
	return CASwapFloat64(arg);
#endif
}

CF_INLINE Float32 CASwapFloat32HostToBig(Float32 arg) {
#if defined(__BIG_ENDIAN__)
    return arg;
#else
	return CASwapFloat32(arg);
#endif
}

CF_INLINE Float64 CASwapFloat64HostToBig(Float64 arg) {
#if defined(__BIG_ENDIAN__)
    return arg;
#else
	return CASwapFloat64(arg);
#endif
}

CF_INLINE Float32 CASwapFloat32LittleToHost(Float32 arg) {
#if defined(__LITTLE_ENDIAN__)
    return arg;
#else
	return CASwapFloat32(arg);
#endif
}

CF_INLINE Float64 CASwapFloat64LittleToHost(Float64 arg) {
#if defined(__LITTLE_ENDIAN__)
    return arg;
#else
	return CASwapFloat64(arg);
#endif
}

CF_INLINE Float32 CASwapFloat32HostToLittle(Float32 arg) {
#if defined(__LITTLE_ENDIAN__)
    return arg;
#else
	return CASwapFloat32(arg);
#endif
}

CF_INLINE Float64 CASwapFloat64HostToLittle(Float64 arg) {
#if defined(__LITTLE_ENDIAN__)
    return arg;
#else
	return CASwapFloat64(arg);
#endif
}


#if defined(__cplusplus)
}
#endif

#endif
