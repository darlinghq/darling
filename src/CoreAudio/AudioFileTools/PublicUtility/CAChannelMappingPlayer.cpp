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
	CAChannelMappingPlayer.cpp
	
=============================================================================*/

#include "CAChannelMappingPlayer.h"
#include "CAXException.h"

#if DEBUG
	#define VERBOSE 1
#endif

CAChannelMappingPlayer::CAChannelMappingPlayer(int nBuffers, UInt32 bufferSizeFrames) :
	CAAudioFilePlayer(nBuffers, bufferSizeFrames),
	mMapper(NULL)
{
}

CAChannelMappingPlayer::~CAChannelMappingPlayer()
{
	delete mMapper;
}

void	CAChannelMappingPlayer::SetupChannelMapping()
{
	delete mMapper;
	mMapper = NULL;
	
	const CAStreamBasicDescription &fileFormat = GetFile().GetClientDataFormat();
	CAStreamBasicDescription deviceFormat;
	UInt32 propertySize = sizeof(AudioStreamBasicDescription);
	
	XThrowIfError(AudioUnitGetProperty(
							GetOutputUnit(),
							kAudioUnitProperty_StreamFormat,
							kAudioUnitScope_Output,
							0,
							(void *)&deviceFormat,
							&propertySize), "get output device's format");

#if VERBOSE
	printf("CAChannelMappingPlayer::SetupChannelMapping: %ld-ch file, %ld-ch device\n",
		fileFormat.mChannelsPerFrame, deviceFormat.mChannelsPerFrame);
#endif

	if (fileFormat.mChannelsPerFrame <= deviceFormat.mChannelsPerFrame) {
		// no mapping needed, use output unit's default behavior 
		// (default stereo pair and speaker config from AMS)
#if VERBOSE
		printf("  using output unit's channel mapping\n");
#endif
		CAAudioFilePlayer::SetupChannelMapping();
	} else {
		// fewer device than file channels, mapping needed
		CAAudioChannelLayout fileLayout, deviceLayout;
		
#if VERBOSE
		printf("  using our own channel mapping\n");
#endif
		deviceFormat.mSampleRate = fileFormat.mSampleRate;
		deviceFormat.SetCanonical(deviceFormat.mChannelsPerFrame, false);	// force deinterleaved
		
		fileLayout = GetFile().GetFileChannelLayout();

		UInt32 layoutSize;
		Boolean writable;
		OSStatus err = AudioUnitGetPropertyInfo(
								GetOutputUnit(),
								kAudioUnitProperty_AudioChannelLayout,
								kAudioUnitScope_Input,
								0,
								&layoutSize,
								&writable);
		if (!err) {
			char *buf = (char *)malloc(layoutSize);
			err = AudioUnitGetProperty(
								GetOutputUnit(),
								kAudioUnitProperty_AudioChannelLayout,
								kAudioUnitScope_Input,
								0,
								buf,
								&layoutSize);
			deviceLayout = CAAudioChannelLayout(reinterpret_cast<AudioChannelLayout *>(buf));
			free(buf);
		}
		mMapper = new CAChannelMapper(fileFormat, deviceFormat, &fileLayout, &deviceLayout);

		// give the output unit the same number of channels as in the device, 
		// since we'll be doing the mapping ourselves
		XThrowIfError(AudioUnitSetProperty(
								GetOutputUnit(),
								kAudioUnitProperty_StreamFormat,
								kAudioUnitScope_Input,
								0,
								(void *)&deviceFormat,
								sizeof(AudioStreamBasicDescription)), "set audio output format");
		
		XThrowIfError(mMapper->OpenMixer(fileFormat.mSampleRate), "open mixer");
		XThrowIfError(mMapper->ConfigureDownmix(), "configure downmix");
		
		AudioUnitConnection conn;
		conn.sourceAudioUnit = mMapper->GetMixer();
		conn.sourceOutputNumber = 0;
		conn.destInputNumber = 0;

		XThrowIfError(AudioUnitSetProperty(
								GetOutputUnit(),
								kAudioUnitProperty_MakeConnection,
								kAudioUnitScope_Global,
								0,
								(void *)&conn,
								sizeof(AudioUnitConnection)), "connect mixer to output unit");
		
		AURenderCallbackStruct input;
		input.inputProc = InputProc;
		input.inputProcRefCon = this;
		XThrowIfError(AudioUnitSetProperty(
									conn.sourceAudioUnit, 
									kAudioUnitProperty_SetRenderCallback, 
									kAudioUnitScope_Global,
									0,
									&input, 
									sizeof(input)), "connect input proc to mixer");
		// provide NO channel layout
//		mReadBuf = CABufferList::New("", fileFormat);
//		mReadBuf->AllocateBuffers(
	}
}
