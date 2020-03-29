/*
     File: AUInputFormatConverter.h
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
#ifndef __AUInputFormatConverter_h__
#define __AUInputFormatConverter_h__

#include "FormatConverterClient.h"
#include "AUTimestampGenerator.h"

// ____________________________________________________________________________
// AUInputFormatConverter
//
// Subclass of FormatConverterClient that applies a format conversion
// to an input of an AudioUnit.
	/*! @class AUInputFormatConverter */
class AUInputFormatConverter : public FormatConverterClient {
public:
	/*! @ctor AUInputFormatConverter */
	AUInputFormatConverter(AUBase *hostAU, int inputBus) :
		mHost(hostAU),
		mHostBus(inputBus),
		mPreviousSilentFrames(0x1000)
	{
#if DEBUG
		mTimestampGenerator.mVerbosity = 0;
		strcpy(mTimestampGenerator.mDebugName, "AUConverter");
#endif
	}
	
	// need to subsequently call Initialize, with the desired formats
	
	/*! @dtor ~AUInputFormatConverter */
	~AUInputFormatConverter()
	{
	}
	
	virtual OSStatus Initialize(const AudioStreamBasicDescription &src, const AudioStreamBasicDescription &dest)
	{
		OSStatus err = FormatConverterClient::Initialize(src, dest);
		if (err) return err;
		mIsPCMToPCM = (src.mFormatID == kAudioFormatLinearPCM) && (dest.mFormatID == kAudioFormatLinearPCM);
		mHasSRC = (fnonzero(src.mSampleRate) && fnonzero(dest.mSampleRate) && fnotequal(src.mSampleRate, dest.mSampleRate));
		return ca_noErr;
	}

	virtual OSStatus	Reset()
	{
		mPreviousSilentFrames = 0x1000;
		mTimestampGenerator.Reset();
		return FormatConverterClient::Reset();
	}
	
	void	SetStartInputTimeAtZero(bool b)
	{
		mTimestampGenerator.SetStartInputAtZero(b);
	}

	/*! @method FillComplexBuffer */
	OSStatus	AUFillComplexBuffer(const AudioTimeStamp &				inTimeStamp,
									UInt32 &							ioOutputDataPacketSize,
									AudioBufferList &					outOutputData,
									AudioStreamPacketDescription*		outPacketDescription,
									bool&								outSilence)
	{
		mTimestampGenerator.AddOutputTime(inTimeStamp, ioOutputDataPacketSize, mOutputFormat.mSampleRate);
		mSilentOutput = true;
		OSStatus err = FillComplexBuffer(ioOutputDataPacketSize, outOutputData, outPacketDescription);
		if (mSilentOutput) {
			if (!mIsPCMToPCM || (mHasSRC && mPreviousSilentFrames < 32))
				mSilentOutput = false;
			mPreviousSilentFrames += ioOutputDataPacketSize;
 		} else
			mPreviousSilentFrames = 0;
		outSilence = mSilentOutput;
		return err;
	}

	/*! @method FormatConverterInputProc */
	virtual OSStatus	FormatConverterInputProc(	
								UInt32 &						ioNumberDataPackets,
								AudioBufferList &				ioData,
								AudioStreamPacketDescription**	outDataPacketDescription)
	{
		OSStatus err = ca_noErr;
		
		AudioUnitRenderActionFlags actionFlags = 0;
		AUInputElement *input = mHost->GetInput(mHostBus);
		*ioNumberDataPackets = std::min(*ioNumberDataPackets, This->mHost->GetMaxFramesPerSlice());
		const AudioTimeStamp &inputTime = mTimestampGenerator.GenerateInputTime(ioNumberDataPackets, mInputFormat.mSampleRate);
		err = input->PullInput(actionFlags, inputTime, mHostBus, ioNumberDataPackets);
		if (!err) {
			input->CopyBufferListTo(ioData);
			if (!(actionFlags & kAudioUnitRenderAction_OutputIsSilence))
				mSilentOutput = false;
		}
		return err;
	}

protected:
	/*! @var mHost */
	AUBase *				mHost;
	/*! @var mHostBus */
	int						mHostBus;
	
	AUTimestampGenerator	mTimestampGenerator;
	bool					mIsPCMToPCM;
	bool					mHasSRC;
	bool					mSilentOutput;
	UInt32					mPreviousSilentFrames;
};

#endif // __AUInputFormatConverter_h__
