/*
     File: CAComponent.cpp
 Abstract: CAComponent.h
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
#include "CAComponent.h"
#include "CAComponentDescription.h"
#include "CACFDictionary.h"
#include <stdlib.h>
#include "CAAutoDisposer.h"

CAComponent::CAComponent (const AudioComponentDescription& inDesc, CAComponent* next)
	: mManuName(0), mAUName(0), mCompName(0)
{
	mComp = AudioComponentFindNext ((next ? next->Comp() : NULL), &inDesc);
	if (mComp)
		AudioComponentGetDescription(Comp(), &mDesc);
	else
		memcpy (&mDesc, &inDesc, sizeof(AudioComponentDescription));
}

CAComponent::CAComponent (const AudioComponent& comp) 
	: mComp (comp),
	  mManuName(0), 
	  mAUName(0), 
	  mCompName(0)
{
	AudioComponentGetDescription (Comp(), &mDesc);
}

CAComponent::CAComponent (const AudioComponentInstance& compInst) 
	: mComp (NULL), 
	  mManuName(0), 
	  mAUName(0), 
	  mCompName(0)
{ 
	mComp = AudioComponentInstanceGetComponent (compInst);
	AudioComponentGetDescription (Comp(), &mDesc);
}

CAComponent::CAComponent (OSType inType, OSType inSubtype, OSType inManu)
	: mDesc (inType, inSubtype, inManu),
	  mManuName(0), mAUName(0), mCompName(0)
{
	mComp = AudioComponentFindNext (NULL, &mDesc);
	AudioComponentGetDescription (Comp(), &mDesc);
}

CAComponent::~CAComponent ()
{
	Clear();
}

OSStatus		CAComponent::GetVersion (UInt32 &outVersion) const
{
	return AudioComponentGetVersion (mComp, &outVersion);
}

void			CAComponent::Clear ()
{
	if (mManuName) { CFRelease (mManuName); mManuName = 0; }
	if (mAUName) { CFRelease (mAUName);  mAUName = 0; }
	if (mCompName) { CFRelease (mCompName); mCompName = 0; }
}

CAComponent&	CAComponent::operator= (const CAComponent& y)
{
	Clear();

	mComp = y.mComp;
	mDesc = y.mDesc;

	if (y.mManuName) { mManuName = y.mManuName; CFRetain (mManuName); }
	if (y.mAUName) { mAUName = y.mAUName; CFRetain (mAUName); }
	if (y.mCompName) { mCompName = y.mCompName; CFRetain (mCompName); } 

	return *this;
}

void 		CAComponent::SetCompNames () const
{
	if (!mCompName) {
	
		CFStringRef compName;
		OSStatus result = AudioComponentCopyName (Comp(), &compName);
		if (result) return;
		
		const_cast<CAComponent*>(this)->mCompName = compName;
		if (compName)
		{
			CFArrayRef splitStrArray = CFStringCreateArrayBySeparatingStrings(NULL, compName, CFSTR(":"));
			
			// we need to retain these values so the strings are not lost when the array is released
			const_cast<CAComponent*>(this)->mManuName = (CFStringRef)CFArrayGetValueAtIndex(splitStrArray, 0);
            CFRetain(this->mManuName);
			if (CFArrayGetCount(splitStrArray) > 1)
			{
				CFStringRef str = (CFStringRef)CFArrayGetValueAtIndex(splitStrArray, 1);
				
				CFMutableStringRef mstr = CFStringCreateMutableCopy (NULL, CFStringGetLength(str), str);

				// this needs to trim out white space:
				
				CFStringTrimWhitespace (mstr);
			
				const_cast<CAComponent*>(this)->mAUName = mstr;
			} else
				const_cast<CAComponent*>(this)->mAUName = NULL;
			
			CFRelease(splitStrArray);
		}
	}
}

static void	_ShowCF (FILE* file, CFStringRef str)
{
	if (CFGetTypeID(str) != CFStringGetTypeID()) {
		CFShow(str);
		return;
	}

	CFIndex len = CFStringGetLength(str);
	char* chars = (char*)CA_malloc (len * 2); // give us plenty of room for unichar chars
	if (CFStringGetCString (str, chars, len * 2, kCFStringEncodingUTF8))
		fprintf (file, "%s", chars);
	else
		CFShow (str);

	free (chars);
}

void	CAComponent::Print(FILE* file) const
{
	fprintf (file, "CAComponent: %p", Comp());
	if (mManuName) {
		fprintf (file, ", Manu:"); _ShowCF (file, mManuName);
		if (mAUName) fprintf (file, ", Name:"); _ShowCF (file, mAUName);
	}
	fprintf (file, ", "); 
	Desc ().Print(file);
}
