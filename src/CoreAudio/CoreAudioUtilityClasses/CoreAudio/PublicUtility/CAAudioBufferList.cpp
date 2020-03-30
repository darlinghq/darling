/*
     File: CAAudioBufferList.cpp
 Abstract: CAAudioBufferList.h
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

#include "CAAudioBufferList.h"
#include "CADebugMacros.h"
#include "CALogMacros.h"
#include <stdlib.h>
#include <string.h>

//=============================================================================
//	CAAudioBufferList
//=============================================================================

AudioBufferList*	CAAudioBufferList::Create(UInt32 inNumberBuffers)
{
	UInt32 theSize = CalculateByteSize(inNumberBuffers);
	AudioBufferList* theAnswer = static_cast<AudioBufferList*>(calloc(1, theSize));
	if(theAnswer != NULL)
	{
		theAnswer->mNumberBuffers = inNumberBuffers;
	}
	return theAnswer;
}

void	CAAudioBufferList::Destroy(AudioBufferList* inBufferList)
{
	free(inBufferList);
}

UInt32	CAAudioBufferList::CalculateByteSize(UInt32 inNumberBuffers)
{
	UInt32 theSize = SizeOf32(AudioBufferList) - SizeOf32(AudioBuffer);
	theSize += inNumberBuffers * SizeOf32(AudioBuffer);
	return theSize;
}

UInt32	CAAudioBufferList::GetTotalNumberChannels(const AudioBufferList& inBufferList)
{
	UInt32 theAnswer = 0;
	
	for(UInt32 theIndex = 0; theIndex < inBufferList.mNumberBuffers; ++theIndex)
	{
		theAnswer += inBufferList.mBuffers[theIndex].mNumberChannels;
	}
	
	return theAnswer;
}

bool	CAAudioBufferList::GetBufferForChannel(const AudioBufferList& inBufferList, UInt32 inChannel, UInt32& outBufferNumber, UInt32& outBufferChannel)
{
	bool theAnswer = false;
	UInt32 theIndex = 0;
	
	while((theIndex < inBufferList.mNumberBuffers) && (inChannel >= inBufferList.mBuffers[theIndex].mNumberChannels))
	{
		inChannel -= inBufferList.mBuffers[theIndex].mNumberChannels;
		++theIndex;
	}
	
	if(theIndex < inBufferList.mNumberBuffers)
	{
		outBufferNumber = theIndex;
		outBufferChannel = inChannel;
		theAnswer = true;
	}
	
	return theAnswer;
}

void	CAAudioBufferList::Clear(AudioBufferList& outBufferList)
{
	//	assumes that "0" is actually the 0 value for this stream format
	for(UInt32 theBufferIndex = 0; theBufferIndex < outBufferList.mNumberBuffers; ++theBufferIndex)
	{
		if(outBufferList.mBuffers[theBufferIndex].mData != NULL)
		{
			memset(outBufferList.mBuffers[theBufferIndex].mData, 0, outBufferList.mBuffers[theBufferIndex].mDataByteSize);
		}
	}
}

void	CAAudioBufferList::Copy(const AudioBufferList& inSource, UInt32 inStartingSourceChannel, AudioBufferList& outDestination, UInt32 inStartingDestinationChannel)
{
	//  This is a brute force copy method that can handle ABL's that have different buffer layouts
	//  This means that this method is probably not the fastest way to do this for all cases.
	//  This method also assumes that both the source and destination sample formats are Float32

	UInt32 theInputChannel = inStartingSourceChannel;
	UInt32 theNumberInputChannels = GetTotalNumberChannels(inSource);
	UInt32 theOutputChannel = inStartingDestinationChannel;
	UInt32 theNumberOutputChannels = GetTotalNumberChannels(outDestination);
	
	UInt32 theInputBufferIndex = 0;
	UInt32 theInputBufferChannel = 0;
	UInt32 theOutputBufferIndex = 0;
	UInt32 theOutputBufferChannel = 0;
	while((theInputChannel < theNumberInputChannels) && (theOutputChannel < theNumberOutputChannels))
	{
		GetBufferForChannel(inSource, theInputChannel, theInputBufferIndex, theInputBufferChannel);
		
		GetBufferForChannel(inSource, theOutputChannel, theOutputBufferIndex, theOutputBufferChannel);
		
		CopyChannel(inSource.mBuffers[theInputBufferIndex], theInputBufferChannel, outDestination.mBuffers[theOutputBufferIndex], theOutputBufferChannel);
		
		++theInputChannel;
		++theOutputChannel;
	}
}

void	CAAudioBufferList::CopyChannel(const AudioBuffer& inSource, UInt32 inSourceChannel, AudioBuffer& outDestination, UInt32 inDestinationChannel)
{
	//  set up the stuff for the loop
	UInt32 theNumberFramesToCopy = outDestination.mDataByteSize / (outDestination.mNumberChannels * SizeOf32(Float32));
	const Float32* theSource = static_cast<const Float32*>(inSource.mData);
	Float32* theDestination = static_cast<Float32*>(outDestination.mData);
	
	//  loop through the data and copy the samples
	while(theNumberFramesToCopy > 0)
	{
		//  copy the data
		theDestination[inDestinationChannel] = theSource[inSourceChannel];
		
		//  adjust the pointers
		--theNumberFramesToCopy;
		theSource += inSource.mNumberChannels;
		theDestination += outDestination.mNumberChannels;
	}
}

void	CAAudioBufferList::Sum(const AudioBufferList& inSourceBufferList, AudioBufferList& ioSummedBufferList)
{
	//	assumes that the buffers are Float32 samples and the listst have the same layout
	//	this is a lame algorithm, by the way. it could at least be unrolled a couple of times
	for(UInt32 theBufferIndex = 0; theBufferIndex < ioSummedBufferList.mNumberBuffers; ++theBufferIndex)
	{
		Float32* theSourceBuffer = static_cast<Float32*>(inSourceBufferList.mBuffers[theBufferIndex].mData);
		Float32* theSummedBuffer = static_cast<Float32*>(ioSummedBufferList.mBuffers[theBufferIndex].mData);
		UInt32 theNumberSamplesToMix = ioSummedBufferList.mBuffers[theBufferIndex].mDataByteSize / SizeOf32(Float32);
		if((theSourceBuffer != NULL) && (theSummedBuffer != NULL) && (theNumberSamplesToMix > 0))
		{
			while(theNumberSamplesToMix > 0)
			{
				*theSummedBuffer += *theSourceBuffer;
				++theSummedBuffer;
				++theSourceBuffer;
				--theNumberSamplesToMix;
			}
		}
	}
}

bool	CAAudioBufferList::HasData(AudioBufferList& inBufferList)
{
	bool hasData = false;
	for(UInt32 theBufferIndex = 0; !hasData && (theBufferIndex < inBufferList.mNumberBuffers); ++theBufferIndex)
	{
		if(inBufferList.mBuffers[theBufferIndex].mData != NULL)
		{
			UInt32* theBuffer = (UInt32*)inBufferList.mBuffers[theBufferIndex].mData;
			UInt32 theNumberSamples = inBufferList.mBuffers[theBufferIndex].mDataByteSize / SizeOf32(UInt32);
			for(UInt32 theSampleIndex = 0; !hasData && (theSampleIndex < theNumberSamples); ++theSampleIndex)
			{
				hasData = theBuffer[theSampleIndex] != 0;
			}
		}
	}
	return hasData;
}

#if CoreAudio_Debug
void	CAAudioBufferList::PrintToLog(const AudioBufferList& inBufferList)
{
	PrintInt("  Number streams: ", inBufferList.mNumberBuffers);
	
	for(UInt32 theIndex = 0; theIndex < inBufferList.mNumberBuffers; ++theIndex)
	{
		PrintIndexedInt("  Channels in stream", theIndex + 1, inBufferList.mBuffers[theIndex].mNumberChannels);
		PrintIndexedInt("  Buffer size of stream", theIndex + 1, inBufferList.mBuffers[theIndex].mDataByteSize);
	}
}
#endif

const AudioBufferList*	CAAudioBufferList::GetEmptyBufferList()
{
	static bool	sInitializer = false;
	static AudioBufferList	sEmptyABL;
	if(!sInitializer)
	{
		memset(&sEmptyABL, 0, sizeof(AudioBufferList));
	}
	return &sEmptyABL;
}
