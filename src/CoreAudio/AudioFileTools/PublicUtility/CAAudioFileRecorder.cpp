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
	CAAudioFileRecorder.cpp
	
=============================================================================*/

#include "CAAudioFileRecorder.h"
#include "CAXException.h"

CAAudioFileRecorder::CAAudioFileRecorder(int nBuffers, UInt32 bufferSizeFrames) :
	CAAudioFileWriter(nBuffers, bufferSizeFrames),
	mInputUnit(NULL),
	mAudioInputPtrs(NULL)
{
	// open input unit
	Component comp;
	ComponentDescription desc;
	
	desc.componentType = kAudioUnitType_Output;
	desc.componentSubType = kAudioUnitSubType_HALOutput;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	desc.componentFlags = 0;
	desc.componentFlagsMask = 0;
	comp = FindNextComponent(NULL, &desc);
	XThrowIf(comp == NULL, -1, "find audio input unit");
	XThrowIfError(OpenAComponent(comp, &mInputUnit), "open audio input unit");

	UInt32 enableIO;
	UInt32 propSize;
	
	enableIO = 0;
	XThrowIfError(AudioUnitSetProperty(mInputUnit,
		kAudioOutputUnitProperty_EnableIO,
		kAudioUnitScope_Output,
		0,
		&enableIO,
		sizeof(enableIO)), "failed to disable output");

	enableIO = 1;
	XThrowIfError(AudioUnitSetProperty(mInputUnit,
		kAudioOutputUnitProperty_EnableIO,
		kAudioUnitScope_Input,
		1,
		&enableIO,
		sizeof(enableIO)), "failed to enable input");
	
	// select the default input device
	propSize = sizeof(AudioDeviceID);
	AudioDeviceID inputDevice;
	XThrowIfError(
		AudioHardwareGetProperty(kAudioHardwarePropertyDefaultInputDevice, &propSize, &inputDevice),
		"failed to get default input device");
	
	XThrowIfError(
		AudioUnitSetProperty(mInputUnit, kAudioOutputUnitProperty_CurrentDevice, kAudioUnitScope_Global, 0, &inputDevice, sizeof(inputDevice)),
		"failed to select input device");
	
	// set render callback
	AURenderCallbackStruct input;
	input.inputProc = InputProc;
	input.inputProcRefCon = this;
	XThrowIfError(AudioUnitSetProperty(
								mInputUnit, 
								kAudioOutputUnitProperty_SetInputCallback, 
								kAudioUnitScope_Global,
								0,
								&input, 
								sizeof(input)), "connect input proc to output unit");

	XThrowIfError(AudioUnitInitialize(mInputUnit), "initialize audio input unit");
	
	// get the hardware format
	propSize = sizeof(mInputDataFormat);
	XThrowIfError(AudioUnitGetProperty(mInputUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 1, &mInputDataFormat, &propSize), "couldn't get input unit's stream format");
}

CAAudioFileRecorder::~CAAudioFileRecorder()
{
	if (mInputUnit)
		CloseComponent(mInputUnit);
	delete mAudioInputPtrs;
}

void	CAAudioFileRecorder::SetFile(const FSRef &parentFSRef, CFStringRef filename, AudioFileTypeID filetype, const CAStreamBasicDescription &dataFormat, const CAAudioChannelLayout *layout)
{
	delete mAudioInputPtrs; mAudioInputPtrs = NULL;
	
	CAStreamBasicDescription fileDataFormat = dataFormat;
	if (fileDataFormat.mSampleRate == 0.)
		fileDataFormat.mSampleRate = mInputDataFormat.mSampleRate;
	
	CAAudioFileWriter::SetFile(parentFSRef, filename, filetype, fileDataFormat, layout);

	const CAStreamBasicDescription &fmt = GetFile().GetClientDataFormat();
	XThrowIfError(AudioUnitSetProperty(
							mInputUnit,
							kAudioUnitProperty_StreamFormat,
							kAudioUnitScope_Output,
							1,
							(void *)&fmt,
							sizeof(AudioStreamBasicDescription)), "set audio input format");

	GetFile().SetIOBufferSizeBytes(GetBufferSizeFrames() * fmt.mBytesPerFrame);

	mAudioInputPtrs = CABufferList::New("audio input ptrs", fmt);
}

/*
void	CAAudioFileRecorder::SetFile(AudioFileID fileID)
{
	delete mAudioInputPtrs; mAudioInputPtrs = NULL;
	
	CAAudioFileWriter::SetFile(fileID, mInputDataFormat.mSampleRate);

	const CAStreamBasicDescription &fmt = GetFile().GetClientDataFormat();
	XThrowIfError(AudioUnitSetProperty(
							mInputUnit,
							kAudioUnitProperty_StreamFormat,
							kAudioUnitScope_Output,
							1,
							(void *)&fmt,
							sizeof(AudioStreamBasicDescription)), "set audio input format");

	GetFile().SetIOBufferSize(GetBufferSizeFrames() * fmt.mBytesPerFrame);

	mAudioInputPtrs = CABufferList::New("audio input ptrs", fmt);
}
*/
	
void	CAAudioFileRecorder::Start()
{
	CAAudioFileWriter::Start();
	XThrowIfError(AudioOutputUnitStart(mInputUnit), "start audio input unit");
}

void	CAAudioFileRecorder::Stop()
{
	XThrowIfError(AudioOutputUnitStop(mInputUnit), "stop audio input unit");
	CAAudioFileWriter::Stop();
}

OSStatus	CAAudioFileRecorder::InputProc(
							void *						inRefCon,
							AudioUnitRenderActionFlags *ioActionFlags,
							const AudioTimeStamp *		inTimeStamp,
							UInt32 						inBusNumber,
							UInt32 						inNumberFrames,
							AudioBufferList *			ioData)
{
	CAAudioFileRecorder *This = static_cast<CAAudioFileRecorder *>(inRefCon);
	AudioUnitRenderActionFlags flags = 0;
	AudioBufferList *abl = &This->mAudioInputPtrs->GetModifiableBufferList();
	for (UInt32 i = 0; i < abl->mNumberBuffers; ++i)
		abl->mBuffers[i].mData = NULL;
	OSStatus err = AudioUnitRender(This->mInputUnit, &flags, inTimeStamp, 1, inNumberFrames, abl);
	if (err)
		return err;
	
	This->PushBuffer(inNumberFrames, abl);
	return noErr;
}
