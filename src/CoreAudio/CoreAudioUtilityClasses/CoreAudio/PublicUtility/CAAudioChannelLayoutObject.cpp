/*
     File: CAAudioChannelLayoutObject.cpp
 Abstract: CAAudioChannelLayoutObject.h
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
#include "CAAudioChannelLayout.h"
#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
	#include <AudioToolbox/AudioFormat.h>
#else
	#include <AudioFormat.h>
#endif

CAAudioChannelLayout::CAAudioChannelLayout ()
{
	mLayout = RefCountedLayout::CreateWithNumberChannelDescriptions(0);
}

//=============================================================================
//	CAAudioChannelLayout::CAAudioChannelLayout
//=============================================================================
CAAudioChannelLayout::CAAudioChannelLayout (UInt32 inNumberChannels, bool inChooseSurround)
{		
		// this chooses default layouts based on the number of channels...
	AudioChannelLayoutTag tag;
	
	switch (inNumberChannels) 
	{
		default:
			// here we have a "broken" layout, in the sense that we haven't any idea how to lay this out
			mLayout = RefCountedLayout::CreateWithNumberChannelDescriptions(inNumberChannels);
			SetAllToUnknown(*mLayout->GetLayout(), inNumberChannels);
			return; // don't fall into the tag case
		case 1:
			tag = kAudioChannelLayoutTag_Mono;
			break;
		case 2:
			tag = inChooseSurround ? kAudioChannelLayoutTag_Binaural : kAudioChannelLayoutTag_Stereo;
			break;
		case 4:
			tag = inChooseSurround ? kAudioChannelLayoutTag_Ambisonic_B_Format : kAudioChannelLayoutTag_AudioUnit_4;
			break;
		case 5:
			tag = inChooseSurround ? kAudioChannelLayoutTag_AudioUnit_5_0 : kAudioChannelLayoutTag_AudioUnit_5;
			break;
		case 6:
			tag = inChooseSurround ? kAudioChannelLayoutTag_AudioUnit_6_0 : kAudioChannelLayoutTag_AudioUnit_6;
			break;
		case 7:
			tag = kAudioChannelLayoutTag_AudioUnit_7_0;
			break;
		case 8:
			tag = kAudioChannelLayoutTag_AudioUnit_8;
			break;
	}
	
	mLayout = RefCountedLayout::CreateWithLayoutTag(tag);
}

//=============================================================================
//	CAAudioChannelLayout::CAAudioChannelLayout
//=============================================================================
CAAudioChannelLayout::CAAudioChannelLayout (AudioChannelLayoutTag inLayoutTag)
	: mLayout(NULL)
{
	SetWithTag(inLayoutTag);
}

//=============================================================================
//	CAAudioChannelLayout::CAAudioChannelLayout
//=============================================================================
CAAudioChannelLayout::CAAudioChannelLayout (const CAAudioChannelLayout &c)
	: mLayout(NULL)
{
	*this = c;
}


//=============================================================================
//	CAAudioChannelLayout::AudioChannelLayout
//=============================================================================
CAAudioChannelLayout::CAAudioChannelLayout (const AudioChannelLayout* inChannelLayout)
	: mLayout(NULL)
{
	*this = inChannelLayout;
}

//=============================================================================
//	CAAudioChannelLayout::~CAAudioChannelLayout
//=============================================================================
CAAudioChannelLayout::~CAAudioChannelLayout ()
{
	if (mLayout) {
		mLayout->release();
		mLayout = NULL;
	}
}

//=============================================================================
//	CAAudioChannelLayout::CAAudioChannelLayout
//=============================================================================
CAAudioChannelLayout& CAAudioChannelLayout::operator= (const CAAudioChannelLayout &c)
{
	if (mLayout != c.mLayout) {
		if (mLayout)
			mLayout->release();
	
		if ((mLayout = c.mLayout) != NULL)
			mLayout->retain();
	}
	
	return *this;
}

CAAudioChannelLayout&	CAAudioChannelLayout::operator= (const AudioChannelLayout* inChannelLayout)
{
	if (mLayout && &mLayout->Layout() == inChannelLayout)
		return *this;
		
	if (mLayout)
		mLayout->release();

	if (inChannelLayout == NULL) 
	{
		mLayout = RefCountedLayout::CreateWithNumberChannelDescriptions(0);
	}
	else
	{
		mLayout = RefCountedLayout::CreateWithLayout(inChannelLayout);
	}
	return *this;
}

void	CAAudioChannelLayout::SetWithTag(AudioChannelLayoutTag inTag)
{
	if (mLayout)
		mLayout->release();
	
	mLayout = RefCountedLayout::CreateWithLayoutTag(inTag);
}

//=============================================================================
//	CAAudioChannelLayout::operator==
//=============================================================================
bool		CAAudioChannelLayout::operator== (const CAAudioChannelLayout &c) const
{
	if (mLayout == c.mLayout)
		return true;
	return Layout() == c.Layout();
}

//=============================================================================
//	CAAudioChannelLayout::operator!=
//=============================================================================
bool		CAAudioChannelLayout::operator!= (const CAAudioChannelLayout &c) const
{
	if (mLayout == c.mLayout)
		return false;

	return !(Layout() == c.Layout());
}

//=============================================================================
//	CAAudioChannelLayout::Print
//=============================================================================
void		CAAudioChannelLayout::Print (FILE* file) const
{
	CAShowAudioChannelLayout (file, &Layout());
}

