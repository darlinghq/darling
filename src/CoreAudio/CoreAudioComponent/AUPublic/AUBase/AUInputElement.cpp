/*
Copyright (C) 2016 Apple Inc. All Rights Reserved.
See LICENSE.txt for this sample’s licensing information

Abstract:
Part of Core Audio AUBase Classes
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
