/*
     File: CAComponent.h
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
#ifndef __CAComponent_h__
#define __CAComponent_h__

#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
#else
	#include <ConditionalMacros.h>
#endif

#include "CAComponentDescription.h"

class CAComponent 
{
public:
	CAComponent ()
		: mComp (0), mDesc(), mManuName(0), mAUName(0), mCompName(0) {}
		
		// if next is specifed that is used to find the next component after that one
	CAComponent (const AudioComponentDescription& inDesc, CAComponent* next = 0);
	
	CAComponent (const CAComponent& y) 
		: mComp (0), mDesc(), mManuName(0), mAUName(0), mCompName(0) { *this = y; }

	CAComponent (const AudioComponent& comp);
	
	CAComponent (const AudioComponentInstance& compInst);

	CAComponent (OSType inType, OSType inSubtype = 0, OSType inManu = 0);
	
	~CAComponent ();
	
	CAComponent&	operator= (const CAComponent& y);
	
		// returns true if this object references a valid component
	bool			IsValid () const { return Comp() != 0; }
	
	bool			HasAUStrings() const { 	SetCompNames (); return mManuName != 0; }

		// CFStringRef should be retained by caller if needed beyond lifetime of this object
		
		// Can return NULL if component doesn't follow AU naming conventions
	CFStringRef		GetAUManu () const { SetCompNames (); return mManuName; }
	CFStringRef		GetAUName () const { SetCompNames (); return mAUName ? mAUName : mCompName; }
		
		// Return value of NULL indicates a problem getting that information from the component
	CFStringRef		GetCompName () const { SetCompNames(); return mCompName; }
	
	const CAComponentDescription&	Desc () const { return mDesc; }
			
	OSStatus		Open (AudioComponentInstance& outInst) const 
	{
		return AudioComponentInstanceNew (Comp(), &outInst);
  	}

	OSStatus			GetVersion (UInt32 &outVersion) const;
	
	const AudioComponent&		Comp() const { return mComp; }
	
	void				Print(FILE* file = stdout) const;

	OSStatus			Save (CFPropertyListRef *outData) const;
		
	OSStatus			Restore (CFPropertyListRef &inData);
	
private:
	AudioComponent mComp;
	CAComponentDescription mDesc;
	
	CFStringRef mManuName, mAUName, mCompName;

	void	SetCompNames () const;
	void	SetCompInfo () const;
	void	Clear ();
};

#endif
