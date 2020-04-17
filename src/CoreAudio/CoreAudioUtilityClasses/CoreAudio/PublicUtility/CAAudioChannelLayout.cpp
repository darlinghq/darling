/*
     File: CAAudioChannelLayout.cpp
 Abstract: CAAudioChannelLayout.h
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
//=============================================================================
//	Includes
//=============================================================================

//	Self Include
#include "CAAudioChannelLayout.h"
#include "CAAutoDisposer.h"
#include <stdlib.h>
#include <string.h>

//=============================================================================
//	CAAudioChannelLayout
//=============================================================================

AudioChannelLayout*	CAAudioChannelLayout::Create(UInt32 inNumberChannelDescriptions)
{
	UInt32 theSize = CalculateByteSize(inNumberChannelDescriptions);
	AudioChannelLayout* theAnswer = static_cast<AudioChannelLayout*>(CA_calloc(1, theSize));
	if(theAnswer != NULL)
	{
		SetAllToUnknown(*theAnswer, inNumberChannelDescriptions);
	}
	return theAnswer;
}

void	CAAudioChannelLayout::Destroy(AudioChannelLayout* inChannelLayout)
{
	free(inChannelLayout);
}

void	CAAudioChannelLayout::SetAllToUnknown(AudioChannelLayout& outChannelLayout, UInt32 inNumberChannelDescriptions)
{
	outChannelLayout.mChannelLayoutTag = kAudioChannelLayoutTag_UseChannelDescriptions;
	outChannelLayout.mChannelBitmap = 0;
	outChannelLayout.mNumberChannelDescriptions = inNumberChannelDescriptions;
	for(UInt32 theChannelIndex = 0; theChannelIndex < inNumberChannelDescriptions; ++theChannelIndex)
	{
		outChannelLayout.mChannelDescriptions[theChannelIndex].mChannelLabel = kAudioChannelLabel_Unknown;
		outChannelLayout.mChannelDescriptions[theChannelIndex].mChannelFlags = 0;
		outChannelLayout.mChannelDescriptions[theChannelIndex].mCoordinates[0] = 0;
		outChannelLayout.mChannelDescriptions[theChannelIndex].mCoordinates[1] = 0;
		outChannelLayout.mChannelDescriptions[theChannelIndex].mCoordinates[2] = 0;
	}
}

bool	operator== (const AudioChannelLayout &x, const AudioChannelLayout &y)
{
	// compare based on the number of channel descriptions present
	// (this may be too strict a comparison if all you care about are matching layout tags)
	UInt32 theSize1 = CAAudioChannelLayout::CalculateByteSize(x.mNumberChannelDescriptions);
	UInt32 theSize2 = CAAudioChannelLayout::CalculateByteSize(y.mNumberChannelDescriptions);
	
	if (theSize1 != theSize2)
		return false;
		
	return !memcmp (&x, &y, theSize1);
}

bool	operator!= (const AudioChannelLayout &x, const AudioChannelLayout &y)
{
	return !(x == y);
}

// counting the one bits in a word
inline UInt32 CountOnes(UInt32 x)
{
	// secret magic algorithm for counting bits in a word.
	UInt32 t;
	x = x - ((x >> 1) & 0x55555555);
	t = ((x >> 2) & 0x33333333);
	x = (x & 0x33333333) + t;
	x = (x + (x >> 4)) & 0x0F0F0F0F;
	x = x + (x << 8);
	x = x + (x << 16);
	return x >> 24;
}

UInt32	CAAudioChannelLayout::NumberChannels (const AudioChannelLayout& inLayout)
{
	if (inLayout.mChannelLayoutTag == kAudioChannelLayoutTag_UseChannelDescriptions)
		return inLayout.mNumberChannelDescriptions;
	
	if (inLayout.mChannelLayoutTag == kAudioChannelLayoutTag_UseChannelBitmap)
		return CountOnes (inLayout.mChannelBitmap);

	return AudioChannelLayoutTag_GetNumberOfChannels(inLayout.mChannelLayoutTag);
}

void 	CAShowAudioChannelLayout (FILE* file, const AudioChannelLayout *layout)
{
	if (layout == NULL) 
	{
		fprintf (file, "\tNULL layout\n");
		return;
	}
	fprintf (file, "\tTag=0x%X, ", (int)layout->mChannelLayoutTag);
	if (layout->mChannelLayoutTag == kAudioChannelLayoutTag_UseChannelBitmap)
		fprintf (file, "Using Bitmap:0x%X\n", (int)layout->mChannelBitmap);
	else {
		fprintf (file, "Num Chan Descs=%d\n", (int)layout->mNumberChannelDescriptions);
		const AudioChannelDescription *desc = layout->mChannelDescriptions;
		for (unsigned int i = 0; i < layout->mNumberChannelDescriptions; ++i, ++desc) {
			fprintf (file, "\t\tLabel=%d, Flags=0x%X, ", (int)desc->mChannelLabel, (int)desc->mChannelFlags);
			fprintf (file, "[az=%f,el=%f,dist=%f]\n", desc->mCoordinates[0], desc->mCoordinates[1], desc->mCoordinates[2]);
		}
	}
}
