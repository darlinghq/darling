/*
     File: CACFData.h
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
#if !defined(__CACFData_h__)
#define __CACFData_h__

//=============================================================================
//	Includes
//=============================================================================

#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
	#include <CoreAudio/CoreAudioTypes.h>
	#include <CoreFoundation/CFData.h>
#else
	#include <CoreAudioTypes.h>
	#include <CFData.h>
#endif

#include "CADebugMacros.h"

//=============================================================================
//	CACFData
//=============================================================================

class CACFData
{

//	Construction/Destruction
public:
	explicit	CACFData(CFDataRef inCFData) : mCFData(inCFData), mWillRelease(true) {}
				CACFData(CFDataRef inCFData, bool inWillRelease) : mCFData(inCFData), mWillRelease(inWillRelease) {}
				CACFData(const void* inData, UInt32 inDataSize) : mCFData(NULL), mWillRelease(true) { mCFData = CFDataCreate(NULL, static_cast<const UInt8*>(inData), static_cast<CFIndex>(inDataSize)); }
				~CACFData() { Release(); }
				CACFData(const CACFData& inNumber) : mCFData(inNumber.mCFData), mWillRelease(inNumber.mWillRelease) { Retain(); }
	CACFData&	operator=(const CACFData& inNumber) { Release(); mCFData = inNumber.mCFData; mWillRelease = inNumber.mWillRelease; Retain(); return *this; }
	CACFData&	operator=(CFDataRef inCFData) { Release(); mCFData = inCFData; mWillRelease = true; return *this; }

private:
	void		Retain() { if(mWillRelease && (mCFData != NULL)) { CFRetain(mCFData); } }
	void		Release() { if(mWillRelease && (mCFData != NULL)) { CFRelease(mCFData); } }
	
	CFDataRef	mCFData;
	bool		mWillRelease;

//	Operations
public:
	void		AllowRelease() { mWillRelease = true; }
	void		DontAllowRelease() { mWillRelease = false; }
	bool		IsValid() { return mCFData != NULL; }

//	Value Access
public:
	CFDataRef	GetCFData() const { return mCFData; }
	CFDataRef	CopyCFData() const { if(mCFData != NULL) { CFRetain(mCFData); } return mCFData; }
	
	UInt32		GetSize() const { return ToUInt32(CFDataGetLength(mCFData)); }
	const void*	GetDataPtr() const { return CFDataGetBytePtr(mCFData); }
	void		CopyData(UInt32 inStartOffset, void* outData, UInt32 inDataSize) const { CFRange theRange = { static_cast<CFIndex>(inStartOffset), static_cast<CFIndex>(inDataSize) }; CFDataGetBytes(mCFData, theRange, static_cast<UInt8*>(outData)); }
	
	SInt32		GetSInt32() const { SInt32 theAnswer = 0; CopyData(0, &theAnswer, SizeOf32(SInt32)); return theAnswer; }
	Float32		GetFloat32() const { Float32 theAnswer = 0; CopyData(0, &theAnswer, SizeOf32(Float32)); return theAnswer; }

};

#endif
