/*	Copyright: 	© Copyright 2005 Apple Computer, Inc. All rights reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
			("Apple") in consideration of your agreement to the following terms, and your
			use, installation, modification or redistribution of this Apple software
			constitutes acceptance of these terms.  If you do not agree with these terms,
			please do not use, install, modify or redistribute this Apple software.

			In consideration of your agreement to abide by the following terms, and subject
			to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
			copyrights in this original Apple software (the "Apple Software"), to use,
			reproduce, modify and redistribute the Apple Software, with or without
			modifications, in source and/or binary forms; provided that if you redistribute
			the Apple Software in its entirety and without modifications, you must retain
			this notice and the following text and disclaimers in all such redistributions of
			the Apple Software.  Neither the name, trademarks, service marks or logos of
			Apple Computer, Inc. may be used to endorse or promote products derived from the
			Apple Software without specific prior written permission from Apple.  Except as
			expressly stated in this notice, no other rights or licenses, express or implied,
			are granted by Apple herein, including but not limited to any patent rights that
			may be infringed by your derivative works or by other works in which the Apple
			Software may be incorporated.

			The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
			WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
			WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
			PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
			COMBINATION WITH YOUR PRODUCTS.

			IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
			CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
			GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
			ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
			OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
			(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
			ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/*=============================================================================
	CAChannelMapper.cpp
	
=============================================================================*/

#include "CAChannelMapper.h"
#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
	#include <AudioToolbox/AudioToolbox.h>
#else
	#include <AudioToolbox.h>
#endif

static void DefaultChannelLayout(CAAudioChannelLayout &layout, UInt32 nchannels)
{
	// we really can't do a sensible downmix without valid source/destination channel layouts
	if (nchannels == 1)
		layout = CAAudioChannelLayout(kAudioChannelLayoutTag_Mono);
	else if (nchannels == 2)
		layout = CAAudioChannelLayout(kAudioChannelLayoutTag_Stereo);
}

CAChannelMapper::CAChannelMapper(	const CAStreamBasicDescription &srcFormat, 
									const CAStreamBasicDescription &destFormat,
									const CAAudioChannelLayout *	inSrcLayout,
									const CAAudioChannelLayout *	inDestLayout) :
	mSrcNChannels(srcFormat.mChannelsPerFrame),
	mDestNChannels(destFormat.mChannelsPerFrame)
{
	if (inSrcLayout && inSrcLayout->IsValid())
		mSrcLayout = *inSrcLayout;
	else
		DefaultChannelLayout(mSrcLayout, srcFormat.mChannelsPerFrame);

	if (inDestLayout && inDestLayout->IsValid())
		mDestLayout = *inDestLayout;
	else
		DefaultChannelLayout(mDestLayout, destFormat.mChannelsPerFrame);
}

CAChannelMapper::~CAChannelMapper()
{
	if (mMatrixMixer)
		CloseComponent(mMatrixMixer);
}

OSStatus	CAChannelMapper::OpenMixer(double sampleRate)
{
	CAComponent comp(kAudioUnitType_Mixer, kAudioUnitSubType_MatrixMixer, kAudioUnitManufacturer_Apple);
	OSStatus err;
	
	err = CAAudioUnit::Open(comp, mMatrixMixer);
	if (err) return err;

	CAStreamBasicDescription fmt;
	fmt.mSampleRate = sampleRate;
	UInt32 nbuses = 1;

	err = mMatrixMixer.SetProperty(kAudioUnitProperty_BusCount, kAudioUnitScope_Input, 0, &nbuses, sizeof(UInt32));
	if (err) return err;
	err = mMatrixMixer.SetProperty(kAudioUnitProperty_BusCount, kAudioUnitScope_Output, 0, &nbuses, sizeof(UInt32));
	if (err) return err;

	fmt.SetCanonical(mSrcNChannels, false);
	err = mMatrixMixer.SetFormat(kAudioUnitScope_Input, 0, fmt);
	if (err) return err;

	fmt.SetCanonical(mDestNChannels, false);
	err = mMatrixMixer.SetFormat(kAudioUnitScope_Output, 0, fmt);
	if (err) return err;
	
	// set render callback
	AURenderCallbackStruct input;
	input.inputProc = MixerInputProc;
	input.inputProcRefCon = this;
	err = mMatrixMixer.SetProperty(
								kAudioUnitProperty_SetRenderCallback, 
								kAudioUnitScope_Global,
								0,
								&input, 
								sizeof(input));
	
	err = mMatrixMixer.Initialize();
	if (err) return err;
	
	return ResetMixer();
}

OSStatus	CAChannelMapper::ResetMixer()
{
	int nin = mSrcNChannels, nout = mDestNChannels;
	int i, j;

	// set global, input and output volumes
	mMatrixMixer.SetParameter(kMatrixMixerParam_Volume, kAudioUnitScope_Global, 0xFFFFFFFF, 1.);
	for (i = 0; i < nout; ++i) {
		mMatrixMixer.SetParameter(kMatrixMixerParam_Volume, kAudioUnitScope_Global, 0xFFFF0000 | i, 1.);
	}
	for (i = 0; i < nin; ++i) {
		mMatrixMixer.SetParameter(kMatrixMixerParam_Volume, kAudioUnitScope_Global, (i<<16) | 0xFFFF, 1.);
	}
	// set crosspoint volumes
	for (i = 0; i < nin; ++i) {
		for (j = 0; j < nout; ++j) {
			mMatrixMixer.SetParameter(kMatrixMixerParam_Volume, kAudioUnitScope_Global, (i<<16) | j, 0.);
		}
	}
	return noErr;
}

OSStatus	CAChannelMapper::ConfigureDownmix()
{
	OSStatus err = ResetMixer();
	if (err)
		return err;
	
	const AudioChannelLayout *layouts[] = { &mSrcLayout.Layout(), &mDestLayout.Layout() };
	UInt32 propSize;
	err = AudioFormatGetPropertyInfo(kAudioFormatProperty_MatrixMixMap, sizeof(layouts), layouts, &propSize);
	if (err)
		return err;
	
	void *mixmap = malloc(propSize);
	err = AudioFormatGetProperty(kAudioFormatProperty_MatrixMixMap, sizeof(layouts), layouts, &propSize, mixmap);
	if (!err) {
		mMatrixMixer.SetParameter(kMatrixMixerParam_Volume, kAudioUnitScope_Global, 0xFFFFFFFF, 1.);
		int nin = mSrcNChannels, nout = mDestNChannels;
		int i, j;
		// set the crosspoint volumes
		Float32 *val = (Float32 *)mixmap;
		for (i = 0; i < nin; ++i) {
			for (j = 0; j < nout; ++j) {
				mMatrixMixer.SetParameter(kMatrixMixerParam_Volume, kAudioUnitScope_Global, (i<<16) | j, *val++);
			}
		}
	}
	free(mixmap);
	return noErr;
}

OSStatus	CAChannelMapper::ConnectChannelToChannel(UInt32 inChannel, UInt32 outChannel)
{
	return mMatrixMixer.SetParameter(kMatrixMixerParam_Volume, kAudioUnitScope_Global,
								(inChannel << 16) | outChannel, 1.);
}

OSStatus	CAChannelMapper::Mix(const AudioBufferList *src, AudioBufferList *dest, UInt32 nFrames)
{
	mMixInputBufferList = src;
	AudioUnitRenderActionFlags flags = 0;
	AudioTimeStamp ts;
	ts.mFlags = 0;
	return AudioUnitRender(mMatrixMixer.AU(), &flags, &ts, 0, nFrames, dest);
}

OSStatus	CAChannelMapper::MixerInputProc(
						void *						inRefCon,
						AudioUnitRenderActionFlags *ioActionFlags,
						const AudioTimeStamp *		inTimeStamp,
						UInt32 						inBusNumber,
						UInt32 						inNumberFrames,
						AudioBufferList *			ioData)
{
	CAChannelMapper *This = static_cast<CAChannelMapper *>(inRefCon);
	const AudioBufferList *mixInputBufferList = This->mMixInputBufferList;
	UInt32 copySize = sizeof(UInt32) + (mixInputBufferList->mNumberBuffers * sizeof(AudioBuffer));
	memcpy(ioData, mixInputBufferList, copySize);
	
	return noErr;
}
