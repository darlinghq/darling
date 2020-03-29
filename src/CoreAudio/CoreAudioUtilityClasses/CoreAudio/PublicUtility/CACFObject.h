/*
     File: CACFObject.h
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
#if !defined(__CACFObject_h__)
#define __CACFObject_h__

//=============================================================================
//	Includes
//=============================================================================

#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
	#include <CoreAudio/CoreAudioTypes.h>
	#include <CoreFoundation/CoreFoundation.h>
#else
	#include <CoreAudioTypes.h>
	#include <CoreFoundation.h>
#endif

//=============================================================================
//	Wrappers for CFRetain and CFRelease that check for NULL
//=============================================================================

template <typename CFObjectType>
CFObjectType CACFRetain(CFObjectType inObject)
{
	CFObjectType theAnswer = inObject;
	if(inObject != NULL)
	{
		theAnswer = reinterpret_cast<CFObjectType>(CFRetain(inObject));
	}
	return theAnswer;
}

inline void	CACFRelease(CFTypeRef inObject)
{
	if(inObject != NULL)
	{
		CFRelease(inObject);
	}
}

//=============================================================================
//	CACFObject
//
//	Notes
//	-	Using the AssignWithoutRetain() method will fool the static analyzer into thinking that the
//		CFObject being assigned will be leaked. This is because the static analyzer is not smart
//		enough to understand that the destructor will release the object.
//=============================================================================

template <class CFObjectType>
class CACFObject
{

//	Construction/Destruction
public:
						CACFObject() : mCFObject(NULL), mWillRelease(true) {}
	explicit			CACFObject(CFObjectType inCFObject) : mCFObject(inCFObject), mWillRelease(true) {}
						CACFObject(CFObjectType inCFObject, bool inWillRelease) : mCFObject(inCFObject), mWillRelease(inWillRelease) {}
						~CACFObject() { Release(); }
						CACFObject(const CACFObject& inObject) : mCFObject(inObject.mCFObject), mWillRelease(inObject.mWillRelease) { Retain(); }
	CACFObject&			operator=(const CACFObject& inObject) { Release(); mCFObject = inObject.mCFObject; mWillRelease = inObject.mWillRelease; Retain(); return *this; }
	CACFObject&			operator=(CFObjectType inCFObject) { Release(); mCFObject = inCFObject; mWillRelease = true; Retain(); return *this; }
	void				AssignWithoutRetain(CFObjectType inObject) { if (inObject != mCFObject) { Release(); mCFObject = inObject; } mWillRelease = true; }

private:
	void				Retain() { if(mWillRelease && (mCFObject != NULL)) { CFRetain(mCFObject); } }
	void				Release() { if(mWillRelease && (mCFObject != NULL)) { CFRelease(mCFObject); mCFObject = NULL; } }
	
	CFObjectType		mCFObject;
	bool				mWillRelease;

//	Operations
public:
	void				AllowRelease() { mWillRelease = true; }
	void				DontAllowRelease() { mWillRelease = false; }
	bool				IsValid() const { return mCFObject != NULL; }
	CFTypeID			GetTypeID() const { return CFGetTypeID(mCFObject); }
	bool				IsEqual(CFObjectType inCFObject) const { return CFEqual(inCFObject, mCFObject) != 0; }

//	Value Access
public:
	CFObjectType		GetCFObject() const { return mCFObject; }
	CFObjectType		CopyCFObject() const { if(mCFObject != NULL) { CFRetain(mCFObject); } return mCFObject; }
	const CFObjectType*	GetPointerToStorage() const	{ return &mCFObject; }

};

typedef	CACFObject<CFBundleRef>			CACFBundle;
typedef	CACFObject<CFPropertyListRef>	CACFPropertyList;
typedef	CACFObject<CFTypeRef>			CACFType;
typedef	CACFObject<CFUUIDRef>			CACFUUID;
typedef	CACFObject<CFURLRef>			CACFURL;

#endif
