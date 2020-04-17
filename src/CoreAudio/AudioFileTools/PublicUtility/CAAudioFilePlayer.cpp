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
	CAAudioFilePlayer.cpp
	
=============================================================================*/

#include "CAAudioFilePlayer.h"
#include "CAXException.h"

CAAudioFilePlayer::CAAudioFilePlayer(int nBuffers, UInt32 bufferSizeFrames) :
	CAAudioFileReader(nBuffers, bufferSizeFrames),
	mOutputUnit(NULL)
{
	// open output unit
	Component comp;
	ComponentDescription desc;
	
	desc.componentType = kAudioUnitType_Output;
	desc.componentSubType = kAudioUnitSubType_DefaultOutput;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	desc.componentFlags = 0;
	desc.componentFlagsMask = 0;
	comp = FindNextComponent(NULL, &desc);
	
	//	if we can't find the default one, look explicitly for AUHAL
	if(comp == NULL)
	{
		desc.componentType = kAudioUnitType_Output;
		desc.componentSubType = kAudioUnitSubType_HALOutput;
		desc.componentManufacturer = kAudioUnitManufacturer_Apple;
		desc.componentFlags = 0;
		desc.componentFlagsMask = 0;
		comp = FindNextComponent(NULL, &desc);
	}
	
	XThrowIf(comp == NULL, -1, "find audio output unit");
	XThrowIfError(OpenAComponent(comp, &mOutputUnit), "open audio output unit");
	XThrowIfError(AudioUnitInitialize(mOutputUnit), "initialize audio output unit");

#if 0
	AudioStreamBasicDescription theFormat;
	UInt32 theSize = sizeof(AudioStreamBasicDescription);
	AudioUnitGetProperty(mOutputUnit, 'sfmt', kAudioUnitScope_Output, 0, &theFormat, &theSize);
	
	theFormat.mChannelsPerFrame = 6;
	theFormat.mBytesPerFrame = theFormat.mChannelsPerFrame * (theFormat.mBitsPerChannel / 8);
	theFormat.mBytesPerPacket = theFormat.mBytesPerFrame;	
	AudioUnitSetProperty(mOutputUnit, 'sfmt', kAudioUnitScope_Output, 0, &theFormat, theSize);
#endif
}

CAAudioFilePlayer::~CAAudioFilePlayer()
{
	if (mOutputUnit)
		CloseComponent(mOutputUnit);
}

void	CAAudioFilePlayer::SetFile(const FSRef &inFile)
{
	Stop();
	CAAudioFileReader::SetFile(inFile);
	SetupChannelMapping();
}

void	CAAudioFilePlayer::SetupChannelMapping()
{
	// set render callback
	AURenderCallbackStruct input;
	input.inputProc = InputProc;
	input.inputProcRefCon = this;
	XThrowIfError(AudioUnitSetProperty(
								mOutputUnit, 
								kAudioUnitProperty_SetRenderCallback, 
								kAudioUnitScope_Global,
								0,
								&input, 
								sizeof(input)), "connect input proc to output unit");

	const CAStreamBasicDescription &fmt = GetFile().GetClientDataFormat();
	XThrowIfError(AudioUnitSetProperty(
							mOutputUnit,
							kAudioUnitProperty_StreamFormat,
							kAudioUnitScope_Input,
							0,
							(void *)&fmt,
							sizeof(AudioStreamBasicDescription)), "set audio output format");
	const CAAudioChannelLayout &layoutObj = GetFile().GetFileChannelLayout();
	if (layoutObj.IsValid()) {
		const AudioChannelLayout *layout = layoutObj;
		/*OSStatus err =*/ AudioUnitSetProperty(
								mOutputUnit,
								kAudioUnitProperty_AudioChannelLayout,
								kAudioUnitScope_Input,
								0,
								layout,
								layoutObj.Size());
		// $$$ FIXME: why does this fail sometimes?
	}
}

void	CAAudioFilePlayer::Start()
{
	if (IsRunning())
		Stop();
	if (ReachedEndOfStream())
		SetCurrentPosition(0.);
	CAAudioFileReader::Start();
	XThrowIfError(AudioOutputUnitStart(mOutputUnit), "start audio output unit");
}

void	CAAudioFilePlayer::Stop()
{
	if (IsRunning()) {
		XThrowIfError(AudioOutputUnitStop(mOutputUnit), "stop audio output unit");
		CAAudioFileReader::Stop();
	}
}

void	CAAudioFilePlayer::SetVolume(double volume)
{
	AudioUnitSetParameter(mOutputUnit, kHALOutputParam_Volume,
		kAudioUnitScope_Global, 0, volume, 0);
}

OSStatus	CAAudioFilePlayer::GetPlayBuffer(
						UInt32						inNumberFrames,
						AudioBufferList *			ioData)
{
	UInt32 nFrames = inNumberFrames;
	PullBuffer(nFrames, ioData);
//	printf("read %ld frames\n", nFrames);
	if (nFrames < inNumberFrames) {
		AudioBuffer *buf = ioData->mBuffers;
		UInt32 bytesPerFrame = buf->mNumberChannels * sizeof(Float32);
		for (UInt32 i = 0; i < ioData->mNumberBuffers; ++i, ++buf) {
			memset((Byte *)buf->mData + nFrames * bytesPerFrame, 0,
				(inNumberFrames - nFrames) * bytesPerFrame);
		}
	}
	return noErr;
}

OSStatus	CAAudioFilePlayer::InputProc(
						void *						inRefCon,
						AudioUnitRenderActionFlags *ioActionFlags,
						const AudioTimeStamp *		inTimeStamp,
						UInt32 						inBusNumber,
						UInt32 						inNumberFrames,
						AudioBufferList *			ioData)
{
	CAAudioFilePlayer *This = static_cast<CAAudioFilePlayer *>(inRefCon);
	return This->GetPlayBuffer(inNumberFrames, ioData);
}


