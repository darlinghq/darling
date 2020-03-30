/*
     File: ACComponentResources.r
 Abstract: ACComponentResources.r
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
#ifndef GEN_MISSING
	#define GEN_MISSING 0
#endif

#ifndef thng_RezTemplateVersion
	#define thng_RezTemplateVersion	2
#endif

//=============================================================================
//	Includes
//=============================================================================

#include "ConditionalMacros.r"
#include "MacTypes.r"
#include "Components.r"

//=============================================================================
//	Platform constants for the thng resources
//=============================================================================

#if	TARGET_OS_MAC && TARGET_API_MAC_OSX
	#define Target_PlatformType		1000
	#define Target_CodeResType		'dlle'
	#define kUseDLLEResource		1
#elif TARGET_OS_WIN32
	#define Target_PlatformType      platformWin32
	#define Target_CodeResType		'dlle'
	#define kUseDLLEResource		1
#else
	#define Target_PlatformType		platformPowerPC
	#define Target_CodeResType		'tppc'
	#define kUseDLLEResource		0
#endif

#if kComponentIsThreadSafe
	#ifndef cmpThreadSafeOnMac	// so we don't need Panther headers to build
		#define cmpThreadSafeOnMac	0x10000000
	#endif
	#define COMPONENT_FLAGS		cmpThreadSafeOnMac
#else
	#define COMPONENT_FLAGS		0
#endif

//=============================================================================
//	The thng and related resources
//
//	The definitions below use the following macros, all of which must be
//	defined. Note that kPrimaryResourceID is used to define two 'STR '
//	resources with consecutive IDs so be sure to space them at least two'
//	apart. Here's a sample of how to do the defines:
//	
//	#define kPrimaryResourceID				128
//	#define kComponentType					'aenc'
//	#define kComponentSubtype				'ima4'
//	#define kComponentManufacturer			'appl'
//	#define	kComponentFlags					0
//	#define kComponentVersion				0x00010000
//	#define kComponentName					"Apple IMA4 Encoder"
//	#define kComponentInfo					"An AudioCodec that encodes linear PCM data into IMA4"
//	#define kComponentEntryPoint			"ACAppleIMA4EncoderEntry"
//	#define	kComponentPublicResourceMapType	0
//	#define kComponentIsThreadSafe			1
//=============================================================================

#ifndef AC_LOCALIZED
resource 'strn' (kPrimaryResourceID, purgeable)
{
	kComponentName
};

resource 'stri' (kPrimaryResourceID, purgeable)
{
	kComponentInfo
};
#endif

#if !GEN_MISSING

#if kUseDLLEResource

	resource 'dlle' (kPrimaryResourceID)
	{
		kComponentEntryPoint
	};

#endif

#define kComponentRegistrationFlags	componentHasMultiplePlatforms | componentDoAutoVersion | componentLoadResident
resource 'thng' (kPrimaryResourceID, kComponentName)
{
	kComponentType,											//	Component type
	kComponentSubtype,										//	Component subtype
	kComponentManufacturer,									//	Component manufacturer
	kComponentFlags,										//	Component flags
	0,														//	Component flags mask
	0, 0,													//	Code type, Code ID
	'strn',	kPrimaryResourceID,								//	Name resource type, resource ID
	'stri',	kPrimaryResourceID,								//	Info resource type, resource ID
	0, 0,													//	Icon resource type, resource ID
	kComponentVersion,										//	Component version
	kComponentRegistrationFlags,							//	Registration flags
	0,														//	Icon family resource ID
	{														//	Beginning of platform info
		COMPONENT_FLAGS,									//	Component flags
		Target_CodeResType, kPrimaryResourceID,				//	Code resource type, resource ID
		Target_PlatformType,								//	Platform type
	},
#if thng_RezTemplateVersion >= 2
	kComponentPublicResourceMapType, kPrimaryResourceID		//	Resource map type, resource map ID
#endif	
};

#else	//	GEN_MISSING

resource 'thga' (kPrimaryResourceID) {
	kComponentType,											//	Component type
	kComponentSubtype,										//	Component subtype
	kComponentManufacturer,									//	Component manufacturer
	kComponentFlags,										//	Component flags
	0,														//	Component flags mask
	0, 0,													//	Code type, Code ID
	'strn',	kPrimaryResourceID,								//	Name resource type, resource ID
	'stri',	kPrimaryResourceID,								//	Info resource type, resource ID
	0, 0,													//	Icon resource type, resource ID
	'miss',													//	Alias component type
	'base',													//	Alias component subtype
	0,														//	Alias component manufacturer
	0,														//	Alias component flags
	0,														//	Alias component flags mask
#if thng_RezTemplateVersion >= 2
	kComponentPublicResourceMapType, kPrimaryResourceID,	//	Resource map type, resource map ID
	cmpAliasNoFlags											//	Alias flags	
#endif
};

#endif	//	GEN_MISSING

#undef kPrimaryResourceID
#undef kComponentType
#undef kComponentSubtype
#undef kComponentManufacturer
#undef kComponentVersion
#undef kComponentRegistrationFlags
#undef kComponentName
#undef kComponentInfo
#undef kComponentEntryPoint
#undef kComponentPublicResourceMapType
#undef Target_PlatformType
#undef Target_CodeResType
#undef kUseDLLEResource
