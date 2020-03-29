/*
     File: ACSimpleCodec.h
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
#if !defined(__ACSimpleCodec_h__)
#define __ACSimpleCodec_h__

//=============================================================================
//	Includes
//=============================================================================

#include "ACBaseCodec.h"

//=============================================================================
//	ACSimpleCodec
//
//	This extension of ACBaseCodec provides for a simple ring buffer to handle
//	input data.
//=============================================================================

class ACSimpleCodec
:
	public ACBaseCodec
{

//	Construction/Destruction
public:
						ACSimpleCodec(UInt32 inInputBufferByteSize, AudioComponentInstance inInstance);
	virtual				~ACSimpleCodec();

//	Data Handling
public:
	virtual void		Initialize(const AudioStreamBasicDescription* inInputFormat, const AudioStreamBasicDescription* inOutputFormat, const void* inMagicCookie, UInt32 inMagicCookieByteSize) = 0;
	virtual void		Uninitialize();
	virtual void		Reset();

	virtual void		AppendInputData(const void* inInputData, UInt32& ioInputDataByteSize, UInt32& ioNumberPackets, const AudioStreamPacketDescription* inPacketDescription);
	virtual void		ZeroPadInputData(UInt32& ioNumberPackets, const AudioStreamPacketDescription* inPacketDescription);
	virtual UInt32		GetInputBufferByteSize() const;
	virtual UInt32		GetUsedInputBufferByteSize() const;
	virtual void		GetPropertyInfo(AudioCodecPropertyID inPropertyID, UInt32& outPropertyDataSize, Boolean& outWritable);
	virtual void		SetProperty(AudioCodecPropertyID inPropertyID, UInt32 inPropertyDataSize, const void* inPropertyData);

protected:
	void				ConsumeInputData(UInt32 inConsumedByteSize);	
	Byte*				GetInputBufferStart() const { return mInputBuffer + mInputBufferStart; }
	UInt32				GetInputBufferContiguousByteSize() const { return (mInputBufferStart <= mInputBufferEnd) ? (mInputBufferEnd - mInputBufferStart) : (mInputBufferByteSize - mInputBufferStart); }
	virtual void		ReallocateInputBuffer(UInt32 inInputBufferByteSize);
	
	// returns a pointer to contiguous bytes. 
	// will do some copying if the request wraps around the internal buffer.
	// request must be less than available bytes
	Byte*				GetBytes(UInt32& ioNumberBytes) const;

private:	
	Byte*				mInputBuffer;
	UInt32				mInputBufferByteSize;
	UInt32				mInputBufferStart;
	UInt32				mInputBufferEnd;

};

#endif
