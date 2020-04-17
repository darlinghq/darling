/*
     File: CAAudioUnitOutputCapturer.h
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
#ifndef __CAAudioUnitOutputCapturer_h__
#define __CAAudioUnitOutputCapturer_h__

#include <AudioToolbox/ExtendedAudioFile.h>

/*
	Class to capture output from an AudioUnit for analysis.

	example:

	CFURL fileurl = CFURLCreateWithFileSystemPath(NULL, CFSTR("/tmp/recording.caf"), kCFURLPOSIXPathStyle, false);

	CAAudioUnitOutputCapturer captor(someAU, fileurl, 'caff', anASBD);

	{
	captor.Start();
	...
	captor.Stop();
	} // can repeat

	captor.Close(); // can be omitted; happens automatically from destructor
*/

class CAAudioUnitOutputCapturer {
public:
	enum { noErr = 0 };

	CAAudioUnitOutputCapturer(AudioUnit au, CFURLRef outputFileURL, AudioFileTypeID fileType, const AudioStreamBasicDescription &format, UInt32 busNumber = 0) :
		mFileOpen(false),
		mClientFormatSet(false),
		mAudioUnit(au),
		mExtAudioFile(NULL),
		mBusNumber (busNumber)
	{	
		CFShow(outputFileURL);
		OSStatus err = ExtAudioFileCreateWithURL(outputFileURL, fileType, &format, NULL, kAudioFileFlags_EraseFile, &mExtAudioFile);
		if (!err)
			mFileOpen = true;
	}
	
	void	Start() {
		if (mFileOpen) {
			if (!mClientFormatSet) {
				AudioStreamBasicDescription clientFormat;
				UInt32 size = sizeof(clientFormat);
				AudioUnitGetProperty(mAudioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, mBusNumber, &clientFormat, &size);
				ExtAudioFileSetProperty(mExtAudioFile, kExtAudioFileProperty_ClientDataFormat, size, &clientFormat);
				mClientFormatSet = true;
			}
			ExtAudioFileWriteAsync(mExtAudioFile, 0, NULL);	// initialize async writes
			AudioUnitAddRenderNotify(mAudioUnit, RenderCallback, this);
		}
	}
	
	void	Stop() {
		if (mFileOpen)
			AudioUnitRemoveRenderNotify(mAudioUnit, RenderCallback, this);
	}
	
	void	Close() {
		if (mExtAudioFile) {
			ExtAudioFileDispose(mExtAudioFile);
			mExtAudioFile = NULL;
		}
	}
	
	~CAAudioUnitOutputCapturer() {
		Close();
	}

private:
	static OSStatus RenderCallback(	void *							inRefCon,
									AudioUnitRenderActionFlags *	ioActionFlags,
									const AudioTimeStamp *			inTimeStamp,
									UInt32							inBusNumber,
									UInt32							inNumberFrames,
									AudioBufferList *				ioData)
	{
		if (*ioActionFlags & kAudioUnitRenderAction_PostRender) {
			CAAudioUnitOutputCapturer *This = (CAAudioUnitOutputCapturer *)inRefCon;
			static int TEMP_kAudioUnitRenderAction_PostRenderError	= (1 << 8);
			if (This->mBusNumber == inBusNumber && !(*ioActionFlags & TEMP_kAudioUnitRenderAction_PostRenderError)) {
				OSStatus result = ExtAudioFileWriteAsync(This->mExtAudioFile, inNumberFrames, ioData);
				if (result) DebugMessageN1("ERROR WRITING FRAMES: %d\n", (int)result);
			}
		}
		return noErr;
	}
	
	bool				mFileOpen;
	bool				mClientFormatSet;
	AudioUnit			mAudioUnit;
	ExtAudioFileRef		mExtAudioFile;
	UInt32				mBusNumber;
};

#endif // __CAAudioUnitOutputCapturer_h__
