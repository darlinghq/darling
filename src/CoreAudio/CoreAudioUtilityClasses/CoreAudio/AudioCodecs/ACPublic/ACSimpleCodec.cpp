/*
     File: ACSimpleCodec.cpp
 Abstract: ACSimpleCodec.h
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

#include "ACSimpleCodec.h"
#include <string.h>

//=============================================================================
//	ACSimpleCodec
//=============================================================================

static const UInt32 kBufferPad = 64; // this is used to prevent end from passing start.

ACSimpleCodec::ACSimpleCodec(UInt32 inInputBufferByteSize, AudioComponentInstance inInstance)
:
	ACBaseCodec(inInstance),
	mInputBuffer(NULL),
	mInputBufferByteSize(inInputBufferByteSize+kBufferPad),
	mInputBufferStart(0),
	mInputBufferEnd(0)
{
}

ACSimpleCodec::~ACSimpleCodec()
{
	delete[] mInputBuffer;
}

void	ACSimpleCodec::Initialize(const AudioStreamBasicDescription* inInputFormat, const AudioStreamBasicDescription* inOutputFormat, const void* inMagicCookie, UInt32 inMagicCookieByteSize)
{
	ReallocateInputBuffer(mInputBufferByteSize - kBufferPad);

	// By definition CBR has this greater than 0. We must avoid a div by 0 error in AppendInputData()
	// Note this will cause us to fail initialization which is intended
	if (mInputFormat.mBytesPerPacket == 0)
	{
		CODEC_THROW(kAudioCodecUnsupportedFormatError);
	}	
	
	ACBaseCodec::Initialize(inInputFormat, inOutputFormat, inMagicCookie, inMagicCookieByteSize);
}

void	ACSimpleCodec::Uninitialize()
{
	//	get rid of the buffer
	delete[] mInputBuffer;
	mInputBuffer = NULL;
	
	//	reset the ring buffer state
	mInputBufferStart = 0;
	mInputBufferEnd = 0;
	
	ACBaseCodec::Uninitialize();
}

void	ACSimpleCodec::Reset()
{
	//	clear the entire input buffer
	if (mInputBuffer) { // could be called before allocated.
		memset(mInputBuffer, 0, mInputBufferByteSize);
	}
	
	//	reset the ring buffer state
	mInputBufferStart = 0;
	mInputBufferEnd = 0;
	
	ACBaseCodec::Reset();
}

UInt32	ACSimpleCodec::GetInputBufferByteSize() const
{
	return mInputBufferByteSize - kBufferPad; // minus kBufferPad to prevent end moving past start
}

UInt32	ACSimpleCodec::GetUsedInputBufferByteSize() const
{
	UInt32 theAnswer = 0;
	
	//	this object uses a ring buffer
	if(mInputBufferStart <= mInputBufferEnd)
	{
		//	the active region is contiguous
		theAnswer = mInputBufferEnd - mInputBufferStart;
	}
	else
	{
		//	the active region wraps around 
		theAnswer = (mInputBufferByteSize - mInputBufferStart) + mInputBufferEnd;
	}
	
	return theAnswer;
}


void	ACSimpleCodec::AppendInputData(const void* inInputData, UInt32& ioInputDataByteSize, UInt32& ioNumberPackets, const AudioStreamPacketDescription* inPacketDescription)
{
	//	this buffer handling code doesn't care about such things as the packet descriptions
	if(!mIsInitialized) CODEC_THROW(kAudioCodecStateError);
	
	//	this is a ring buffer we're dealing with, so we need to set up a few things
	UInt32 theUsedByteSize = GetUsedInputBufferByteSize();
	UInt32 theAvailableByteSize = GetInputBufferByteSize() - theUsedByteSize;

	UInt32 theMaxAvailableInputBytes = ioInputDataByteSize; // we can't consume more than we get

	const Byte* theInputData = static_cast<const Byte*>(inInputData);
	
	// >>jamesmcc: added this because ioNumberPackets was not being updated if less was taken than given.
	// THIS ASSUMES CBR!
	UInt32 bytesPerPacketOfInput = mInputFormat.mBytesPerPacket;
	UInt32 theAvailablePacketSize = theAvailableByteSize / bytesPerPacketOfInput;
	
	UInt32 minPacketSize = ioNumberPackets < theAvailablePacketSize ? ioNumberPackets : theAvailablePacketSize;
	UInt32 minByteSize = minPacketSize * bytesPerPacketOfInput;
	
	//	we can copy only as much data as there is or up to how much space is availiable
	ioNumberPackets = minPacketSize;
	ioInputDataByteSize = minByteSize;
	
	// ioInputDataByteSize had better be <= to theMaxAvailableInputBytes or we're screwed
	if (ioInputDataByteSize > theMaxAvailableInputBytes)
	{
		CODEC_THROW(kAudioCodecStateError);
	}
	// <<jamesmcc 
	
	//	now we have to copy the data taking into account the wrap around and where the start is
	if(mInputBufferEnd + ioInputDataByteSize < mInputBufferByteSize)
	{
		//	no wrap around here
		memcpy(mInputBuffer + mInputBufferEnd, theInputData, ioInputDataByteSize);
		
		//	adjust the end point
		mInputBufferEnd += ioInputDataByteSize;
	}
	else
	{
		//	the copy will wrap
		
		//	copy the first part
		UInt32 theBeforeWrapByteSize = mInputBufferByteSize - mInputBufferEnd;
		memcpy(mInputBuffer + mInputBufferEnd, theInputData, theBeforeWrapByteSize);
		
		//	and the rest
		UInt32 theAfterWrapByteSize = ioInputDataByteSize - theBeforeWrapByteSize;
		memcpy(mInputBuffer, theInputData + theBeforeWrapByteSize, theAfterWrapByteSize);
		
		//	adjust the end point
		mInputBufferEnd = theAfterWrapByteSize;
	}
	
}


void	ACSimpleCodec::ZeroPadInputData(UInt32& ioNumberPackets, const AudioStreamPacketDescription* inPacketDescription)
{
	//	this buffer handling code doesn't care about such things as the packet descriptions
	if(!mIsInitialized) CODEC_THROW(kAudioCodecStateError);
	
	
	//	this is a ring buffer we're dealing with, so we need to set up a few things
	UInt32 theUsedByteSize = GetUsedInputBufferByteSize();
	UInt32 theAvailableByteSize = GetInputBufferByteSize() - theUsedByteSize;
	
	// >>jamesmcc: added this because ioNumberPackets was not being updated if less was taken than given.
	// THIS ASSUMES CBR!
	UInt32 bytesPerPacketOfInput = mInputFormat.mBytesPerPacket;
	UInt32 theAvailablePacketSize = theAvailableByteSize / bytesPerPacketOfInput;
	
	UInt32 minPacketSize = ioNumberPackets < theAvailablePacketSize ? ioNumberPackets : theAvailablePacketSize;
	UInt32 minByteSize = minPacketSize * bytesPerPacketOfInput;
	
	//	we can copy only as much data as there is or up to how much space is availiable
	ioNumberPackets = minPacketSize;
	
	// <<jamesmcc 
	
	//	now we have to copy the data taking into account the wrap around and where the start is
	if(mInputBufferEnd + minByteSize < mInputBufferByteSize)
	{
		//	no wrap around here
		memset(mInputBuffer + mInputBufferEnd, 0, minByteSize);
		
		//	adjust the end point
		mInputBufferEnd += minByteSize;
	}
	else
	{
		//	the copy will wrap
		
		//	copy the first part
		UInt32 theBeforeWrapByteSize = mInputBufferByteSize - mInputBufferEnd;
		memset(mInputBuffer + mInputBufferEnd, 0, theBeforeWrapByteSize);
		
		//	and the rest
		UInt32 theAfterWrapByteSize = minByteSize - theBeforeWrapByteSize;
		memset(mInputBuffer, 0, theAfterWrapByteSize);
		
		//	adjust the end point
		mInputBufferEnd = theAfterWrapByteSize;
	}
}


void	ACSimpleCodec::ConsumeInputData(UInt32 inConsumedByteSize)
{
	//	this is a convenience routine to make maintaining the ring buffer state easy
	UInt32 theContiguousRange = GetInputBufferContiguousByteSize();
	
	if(inConsumedByteSize > GetUsedInputBufferByteSize()) CODEC_THROW(kAudioCodecUnspecifiedError);
	
	if(inConsumedByteSize <= theContiguousRange)
	{
		//	the region to consume doesn't wrap
		
		//	figure out how much to consume
		inConsumedByteSize = (theContiguousRange < inConsumedByteSize) ? theContiguousRange : inConsumedByteSize;
		
		//	clear the consumed bits
		memset(mInputBuffer + mInputBufferStart, 0, inConsumedByteSize);
		
		//	adjust the start
		mInputBufferStart += inConsumedByteSize;
	}
	else
	{
		//	the region to consume will wrap
		
		//	clear the bits to the end of the buffer
		memset(mInputBuffer + mInputBufferStart, 0, theContiguousRange);
		
		//	now clear the bits left from the start
		memset(mInputBuffer, 0, inConsumedByteSize - theContiguousRange);
		
		//	adjust the start
		mInputBufferStart = inConsumedByteSize - theContiguousRange;
	}
}


Byte* ACSimpleCodec::GetBytes(UInt32& ioNumberBytes) const
{
	// if a client's algorithm has to have contiguous data and mInputBuffer wraps, then someone has to make a copy.
	// I can do it more efficiently than the client. 
	
	if(!mIsInitialized) CODEC_THROW(kAudioCodecStateError);

	UInt32 theUsedByteSize = GetUsedInputBufferByteSize();
	//UInt32 theAvailableByteSize = GetInputBufferByteSize() - theUsedByteSize;
	
	if (ioNumberBytes > theUsedByteSize) ioNumberBytes = theUsedByteSize;
		
	SInt32 leftOver = mInputBufferStart + ioNumberBytes - mInputBufferByteSize;
	
	if(leftOver > 0)
	{
		// need to copy beginning of buffer to the end. 
		// We cleverly over allocated our buffer space to make this possible.
		memmove(mInputBuffer + mInputBufferByteSize, mInputBuffer, leftOver);
	}
	
	return GetInputBufferStart();
}


void	ACSimpleCodec::ReallocateInputBuffer(UInt32 inInputBufferByteSize)
{
	mInputBufferByteSize = inInputBufferByteSize + kBufferPad;
	
	//	toss the old buffer
	delete[] mInputBuffer;
	mInputBuffer = NULL;
	
	//	allocate the new one
	// allocate extra in order to allow making contiguous data.
	UInt32 allocSize = 2*inInputBufferByteSize + kBufferPad;
	mInputBuffer = new Byte[allocSize];
	memset(mInputBuffer, 0, allocSize);
	
	//	reset the ring buffer state
	mInputBufferStart = 0;
	mInputBufferEnd = 0;
}

void	ACSimpleCodec::GetPropertyInfo(AudioCodecPropertyID inPropertyID, UInt32& outPropertyDataSize, Boolean& outWritable)
{
	switch(inPropertyID)
	{
		case kAudioCodecPropertyInputBufferSize:
			outPropertyDataSize = SizeOf32(UInt32);
			outWritable = true;
			break;
		default:
			ACBaseCodec::GetPropertyInfo(inPropertyID, outPropertyDataSize, outWritable);
			break;
	}

}

void	ACSimpleCodec::SetProperty(AudioCodecPropertyID inPropertyID, UInt32 inPropertyDataSize, const void* inPropertyData)
{
	switch(inPropertyID)
	{
		case kAudioCodecPropertyInputBufferSize:
			if(inPropertyDataSize == sizeof(UInt32))
			{
				ReallocateInputBuffer(*reinterpret_cast<const UInt32*>(inPropertyData));
			}
			else
			{
				CODEC_THROW(kAudioCodecBadPropertySizeError);
			}
			break;
		default:
            ACBaseCodec::SetProperty(inPropertyID, inPropertyDataSize, inPropertyData);
            break;            
    }
}
