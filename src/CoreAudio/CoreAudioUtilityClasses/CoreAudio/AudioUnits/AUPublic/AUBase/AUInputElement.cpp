/*
     File: AUInputElement.cpp
 Abstract: AUInputElement.h
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
#include "AUBase.h"

inline bool	HasGoodBufferPointers(const AudioBufferList &abl, UInt32 nBytes)
{
	const AudioBuffer *buf = abl.mBuffers;
	for (UInt32 i = abl.mNumberBuffers; i--;++buf) {
		if (buf->mData == NULL || buf->mDataByteSize < nBytes)
			return false;
	}
	return true;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUInputElement::AUInputElement
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AUInputElement::AUInputElement(AUBase *audioUnit) :
	AUIOElement(audioUnit),
	mInputType(kNoInput)
{
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUInputElement::SetConnection
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void	AUInputElement::SetConnection(const AudioUnitConnection &conn)
{
	if (conn.sourceAudioUnit == 0) {
		Disconnect();
		return;
	}	
	
	mInputType = kFromConnection;
	mConnection = conn;
	AllocateBuffer();

	mConnInstanceStorage = NULL;

#if !CA_USE_AUDIO_PLUGIN_ONLY
	mConnRenderProc = NULL;
	UInt32 size = sizeof(AudioUnitRenderProc);
	OSStatus result = AudioUnitGetProperty(	conn.sourceAudioUnit,
							kAudioUnitProperty_FastDispatch,
							kAudioUnitScope_Global,
							kAudioUnitRenderSelect,
							&mConnRenderProc,
							&size);
	if (result == noErr)
		mConnInstanceStorage = CMgr_GetComponentInstanceStorage (conn.sourceAudioUnit);
	else
		mConnRenderProc = NULL;
#endif
}

void	AUInputElement::Disconnect()
{
	mInputType = kNoInput;
	mIOBuffer.Deallocate();
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUInputElement::SetInputCallback
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void	AUInputElement::SetInputCallback(AURenderCallback proc, void *refCon)
{
	if (proc == NULL)
		Disconnect();
	else {
		mInputType = kFromCallback;
		mInputProc = proc;
		mInputProcRefCon = refCon;
		AllocateBuffer();
	}
}

OSStatus	AUInputElement::SetStreamFormat(const CAStreamBasicDescription &fmt)
{
	OSStatus err = AUIOElement::SetStreamFormat(fmt);
	if (err == AUBase::noErr)
		AllocateBuffer();
	return err;
}

OSStatus		AUInputElement::PullInput(	AudioUnitRenderActionFlags &  	ioActionFlags,
											const AudioTimeStamp &			inTimeStamp,
											AudioUnitElement				inElement,
											UInt32							nFrames)
{	
	if (!IsActive())
		return kAudioUnitErr_NoConnection;
		
	AudioBufferList *pullBuffer;
	
	if (HasConnection() || !WillAllocateBuffer())
		pullBuffer = &mIOBuffer.PrepareNullBuffer(mStreamFormat, nFrames);
	else
		pullBuffer = &mIOBuffer.PrepareBuffer(mStreamFormat, nFrames);
	
	return PullInputWithBufferList (ioActionFlags, inTimeStamp, inElement, nFrames, pullBuffer);
}
