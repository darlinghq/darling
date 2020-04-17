/*
     File: AUOutputBL.cpp
 Abstract: AUOutputBL.h
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
#include "AUOutputBL.h"
#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
	#include <AudioUnit/AUComponent.h>
#else
	#include <AUComponent.h>
#endif
/*
struct AudioBufferList
{
	UInt32		mNumberBuffers;
	AudioBuffer	mBuffers[1];
};
struct AudioBuffer
{
	UInt32	mNumberChannels;	//	number of interleaved channels in the buffer
	UInt32	mDataByteSize;		//	the size of the buffer pointed to by mData
	void*	mData;				//	the pointer to the buffer
};
*/

AUOutputBL::AUOutputBL (const CAStreamBasicDescription &inDesc, UInt32 inDefaultNumFrames) 
		: mFormat (inDesc),
		  mBufferMemory(NULL),
		  mBufferList (NULL),
		  mNumberBuffers (0), // keep this here, so can ensure integrity of ABL
		  mBufferSize (0),
		  mFrames(inDefaultNumFrames)
{
	mNumberBuffers = mFormat.IsInterleaved() ? 1 : mFormat.NumberChannels();
	mBufferList = reinterpret_cast<AudioBufferList*>(new Byte[offsetof(AudioBufferList, mBuffers) + (mNumberBuffers * sizeof(AudioBuffer))]);
}

AUOutputBL::~AUOutputBL()
{
	if (mBufferMemory)
		delete[] mBufferMemory;

	if (mBufferList)
		delete [] (Byte *)mBufferList;
}

void 	AUOutputBL::Prepare (UInt32 inNumFrames, bool inWantNullBufferIfAllocated) 
{
	UInt32 channelsPerBuffer = mFormat.IsInterleaved() ? mFormat.NumberChannels() : 1;
	
	if (mBufferMemory == NULL || inWantNullBufferIfAllocated)
	{
		mBufferList->mNumberBuffers = mNumberBuffers;
		AudioBuffer *buf = &mBufferList->mBuffers[0];
		for (UInt32 i = 0; i < mNumberBuffers; ++i, ++buf) {
			buf->mNumberChannels = channelsPerBuffer;
			buf->mDataByteSize = mFormat.FramesToBytes (inNumFrames);
			buf->mData = NULL;
		}
	}
	else
	{
		UInt32 nBytes = mFormat.FramesToBytes (inNumFrames);
		if ((nBytes * mNumberBuffers) > AllocatedBytes())
			throw OSStatus(kAudioUnitErr_TooManyFramesToProcess);
			
		mBufferList->mNumberBuffers = mNumberBuffers;
		AudioBuffer *buf = &mBufferList->mBuffers[0];
		Byte* p = mBufferMemory;
		for (UInt32 i = 0; i < mNumberBuffers; ++i, ++buf) {
			buf->mNumberChannels = channelsPerBuffer;
			buf->mDataByteSize = nBytes;
			buf->mData = p;
			p += mBufferSize;
		}
	}
}


void	AUOutputBL::Allocate (UInt32 inNumFrames)
{
	if (inNumFrames) 
	{
		UInt32 nBytes = mFormat.FramesToBytes (inNumFrames);
		
		if (nBytes <= AllocatedBytes()) 
			return;
		
			// align successive buffers for Altivec and to take alternating
			// cache line hits by spacing them by odd multiples of 16
		if (mNumberBuffers > 1)
			nBytes = (nBytes + (0x10 - (nBytes & 0xF))) | 0x10;
		
		mBufferSize = nBytes;
		
		UInt32 memorySize = mBufferSize * mNumberBuffers;
		Byte *newMemory = new Byte[memorySize];
		memset(newMemory, 0, memorySize);	// make buffer "hot"
		
		Byte *oldMemory = mBufferMemory;
		mBufferMemory = newMemory;
		delete[] oldMemory;
		
		mFrames = inNumFrames;
	} 
	else 
	{
		if (mBufferMemory) {
			delete [] mBufferMemory;
			mBufferMemory = NULL;
		}
		mBufferSize = 0;
		mFrames = 0;
	}
}

#if DEBUG
void			AUOutputBL::Print()
{
	printf ("AUOutputBL::Print\n");
	mFormat.Print();
	printf ("Num Buffers:%d, mFrames:%d, allocatedMemory:%c\n", (int)mBufferList->mNumberBuffers, (int)mFrames, (mBufferMemory != NULL ? 'T' : 'F'));
	AudioBuffer *buf = &mBufferList->mBuffers[0];
	for (UInt32 i = 0; i < mBufferList->mNumberBuffers; ++i, ++buf)
		printf ("\tBuffer:%d, Size:%d, Chans:%d, Buffer:%p\n", (int)i, (int)buf->mDataByteSize, (int)buf->mNumberChannels, buf->mData);
}
#endif

