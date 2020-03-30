/*
     File: AUResources.r
 Abstract: AUResources.r
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
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUResources.r
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/* sample macro definitions -- all of these symbols must be defined
#define RES_ID			kHALOutputResID
#define COMP_TYPE		kAudioUnitComponentType
#define COMP_SUBTYPE	kAudioUnitOutputSubType
#define COMP_MANUF		kAudioUnitAudioHardwareOutputSubSubType
#define VERSION			0x00010000
#define NAME			"AudioHALOutput"
#define DESCRIPTION		"Audio hardware output AudioUnit"
#define ENTRY_POINT		"AUHALEntry"
*/
#define UseExtendedThingResource 1

#include <CoreServices/CoreServices.r>

// this is a define used to indicate that a component has no static data that would mean 
// that no more than one instance could be open at a time - never been true for AUs
#ifndef cmpThreadSafeOnMac
#define cmpThreadSafeOnMac	0x10000000
#endif

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

resource 'STR ' (RES_ID, purgeable) {
	NAME
};

resource 'STR ' (RES_ID + 1, purgeable) {
	DESCRIPTION
};

resource 'dlle' (RES_ID) {
	ENTRY_POINT
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

resource 'thng' (RES_ID, NAME) {
	COMP_TYPE,
	COMP_SUBTYPE,
	COMP_MANUF,
	0, 0, 0, 0,								//	no 68K
	'STR ',	RES_ID,
	'STR ',	RES_ID + 1,
	0,	0,			/* icon */
	VERSION,
	componentHasMultiplePlatforms | componentDoAutoVersion,
	0,
	{
	#if defined(ppc_YES)
			cmpThreadSafeOnMac,
			'dlle', RES_ID, platformPowerPCNativeEntryPoint
			#define NeedLeadingComma 1
	#endif
	#if defined(ppc64_YES)
		#if defined(NeedLeadingComma)
			,
		#endif
			cmpThreadSafeOnMac,
			'dlle', RES_ID, platformPowerPC64NativeEntryPoint
			#define NeedLeadingComma 1
	#endif
	#if defined(i386_YES)
		#if defined(NeedLeadingComma)
			,
		#endif
			cmpThreadSafeOnMac,
			'dlle', RES_ID, platformIA32NativeEntryPoint
			#define NeedLeadingComma 1
	#endif
	#if defined(x86_64_YES)
		#if defined(NeedLeadingComma)
			,
		#endif
			cmpThreadSafeOnMac,
			'dlle', RES_ID, 8
			#define NeedLeadingComma 1
	#endif
	}
};

#undef RES_ID
#undef COMP_TYPE
#undef COMP_SUBTYPE
#undef COMP_MANUF
#undef VERSION
#undef NAME
#undef DESCRIPTION
#undef ENTRY_POINT
#undef NeedLeadingComma
