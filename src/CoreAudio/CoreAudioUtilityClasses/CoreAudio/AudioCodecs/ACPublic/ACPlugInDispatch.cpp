/*
     File: ACPlugInDispatch.cpp
 Abstract: ACPlugInDispatch.h
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
#if AUDIOCODECS_SUPPORT_PLUGINS
#include "ACPlugInDispatch.h"
#include "CAXException.h"
#include "ACCodec.h"
#include <new>

#define ACPI ((AudioComponentPlugInInstance *)self)
#define ACC	((ACCodec *)&ACPI->mInstanceStorage)

static OSStatus	GetPropertyInfo(void *self, AudioCodecPropertyID inPropertyID, UInt32 *outSize, Boolean *outWritable)
{
	OSStatus result = noErr;
	try {
		UInt32 size;
		Boolean writable;
		ACC->GetPropertyInfo(inPropertyID, size, writable);
		if (outSize) *outSize = size;
		if (outWritable) *outWritable = writable;
	}
	COMPONENT_CATCH
	return result;
}

static OSStatus	GetProperty(void *self, AudioCodecPropertyID inPropertyID, UInt32 *ioPropertyDataSize, void *outPropertyData)
{
	OSStatus result = noErr;
	try {
		ACC->GetProperty(inPropertyID, *ioPropertyDataSize, outPropertyData);
	}
	COMPONENT_CATCH
	return result;
}

static OSStatus	SetProperty(void *self, AudioCodecPropertyID inPropertyID, UInt32 inPropertyDataSize, const void *inPropertyData)
{
	OSStatus result = noErr;
	try {
		ACC->SetProperty(inPropertyID, inPropertyDataSize, inPropertyData);
	}
	COMPONENT_CATCH
	return result;
}


static OSStatus Initialize(void *self, const AudioStreamBasicDescription *inInputFormat, const AudioStreamBasicDescription *inOutputFormat, const void *inMagicCookie, UInt32 inMagicCookieByteSize)
{
	OSStatus result = noErr;
	try {
		ACC->Initialize(inInputFormat, inOutputFormat, inMagicCookie, inMagicCookieByteSize);
	}
	COMPONENT_CATCH
	return result;
}

static OSStatus Uninitialize(void *self)
{
	OSStatus result = noErr;
	try {
		ACC->Uninitialize();
	}
	COMPONENT_CATCH
	return result;
}

static OSStatus	AppendInputData(void *self, const void *inInputData, UInt32 *ioInputDataByteSize, UInt32 *ioNumberPackets, const AudioStreamPacketDescription *inPacketDescription)
{
	OSStatus result = noErr;
	try {
		UInt32 npackets = (ioNumberPackets != NULL) ? *ioNumberPackets : 0;
		ACC->AppendInputData(inInputData, *ioInputDataByteSize, npackets, inPacketDescription);
        if(ioNumberPackets != NULL)
            *ioNumberPackets = npackets;
	}
	COMPONENT_CATCH
	return result;
}

static OSStatus	ProduceOutputPackets(void *self, void *outOutputData, UInt32 *ioOutputDataByteSize, UInt32 *ioNumberPackets, AudioStreamPacketDescription *outPacketDescription, UInt32 *outStatus)
{
	OSStatus result = noErr;
	try {
		*outStatus = ACC->ProduceOutputPackets(outOutputData, *ioOutputDataByteSize, *ioNumberPackets, outPacketDescription);
		if (*outStatus == kAudioCodecProduceOutputPacketFailure)
			result = kAudio_ParamError;
	}
	COMPONENT_CATCH
	return result;
}

static OSStatus	Reset(void *self)
{
	OSStatus result = noErr;
	try {
		ACC->Reset();
	}
	COMPONENT_CATCH
	return result;
}

#if AC_NON_INTERLEAVED_SUPPORT
static OSStatus AppendInputBufferList(void *self, const AudioBufferList *ioBufferList, UInt32 *inNumberPackets, const AudioStreamPacketDescription *inPacketDescription, UInt32 *outBytesConsumed)
{
	OSStatus result = noErr;
	try {
		if((ioBufferList != NULL) && (outBytesConsumed != NULL))
		{
			if(inNumberPackets != NULL)
			{
				ACC->AppendInputBufferList(ioBufferList, *inNumberPackets, inPacketDescription, outBytesConsumed);
			}
			else
			{
				UInt32 theNumberPackets = 0;
				ACC->AppendInputBufferList(ioBufferList, theNumberPackets, inPacketDescription, outBytesConsumed);
			}
		}
		else
		{
			result = kAudio_ParamError;
		}
	}
	COMPONENT_CATCH
	return result;
}

static OSStatus ProduceOutputBufferList(void *self, AudioBufferList *ioBufferList, UInt32 *ioNumberPackets, AudioStreamPacketDescription *outPacketDescription, UInt32 *outStatus)
{
	OSStatus result = noErr;
	try {
		if((ioBufferList != NULL) && (ioNumberPackets != NULL) && (outStatus != NULL))
		{
			*outStatus = ACC->ProduceOutputBufferList(ioBufferList, *ioNumberPackets, outPacketDescription);
			if(kAudioCodecProduceOutputPacketFailure == *outStatus)
			{
				result = kAudio_ParamError;
			}
		}
		else
		{
			result = kAudio_ParamError;
		}
	}
	COMPONENT_CATCH
	return result;
}
#endif

#if TARGET_OS_IPHONE && AUDIOCONV_HAVE_AMC
// The ACTransformerCodecBase class is the base for ALL hardware codecs.
// No need to check for ImplementFeature...
#include "ACTransformerManager.h"
#define ACTM	((ACTransformerCodecBase*)&ACPI->mInstanceStorage)

static OSStatus	AcquireHardware(void *self)
{
	OSStatus result = noErr;
	try {
        ACTM->AcquireHardware();
	}
	COMPONENT_CATCH
	return result;
}

static OSStatus	ReleaseHardware(void *self)
{
	OSStatus result = noErr;
	try {
        ACTM->ReleaseHardware();
	}
	COMPONENT_CATCH
	return result;
}
#endif // TARGET_OS_IPHONE && AUDIOCONV_HAVE_AMC


AudioComponentMethod AudioCodecLookup::Lookup (SInt16 selector)
{
	switch (selector) {
		case kAudioCodecGetPropertyInfoSelect:			return (AudioComponentMethod)GetPropertyInfo;
		case kAudioCodecGetPropertySelect:				return (AudioComponentMethod)GetProperty;
		case kAudioCodecSetPropertySelect:				return (AudioComponentMethod)SetProperty;
		case kAudioCodecInitializeSelect:				return (AudioComponentMethod)Initialize;
		case kAudioCodecUninitializeSelect:				return (AudioComponentMethod)Uninitialize;
		case kAudioCodecAppendInputDataSelect:			return (AudioComponentMethod)AppendInputData;
		case kAudioCodecProduceOutputDataSelect:		return (AudioComponentMethod)ProduceOutputPackets;
		case kAudioCodecResetSelect:					return (AudioComponentMethod)Reset;
		default:
			break;
	}
	return NULL;
}

#if AC_NON_INTERLEAVED_SUPPORT
AudioComponentMethod AudioCodecNonInterleavedEncoderLookup::Lookup (SInt16 selector)
{
	AudioComponentMethod method = AudioCodecLookup::Lookup(selector);
	if (method)
		return method;
	
	if (selector == kAudioCodecAppendInputBufferListSelect)
		return (AudioComponentMethod)AppendInputBufferList;
	
	return NULL;
}

AudioComponentMethod AudioCodecNonInterleavedDecoderLookup::Lookup (SInt16 selector)
{
	AudioComponentMethod method = AudioCodecLookup::Lookup(selector);
	if (method)
		return method;
	
	if (selector == kAudioCodecProduceOutputBufferListSelect)
		return (AudioComponentMethod)ProduceOutputBufferList;
	
	return NULL;
}
#endif

#if TARGET_OS_IPHONE && AUDIOCONV_HAVE_AMC
#include "AudioCodecPriv.h"

AudioComponentMethod AudioCodecHWCodecLookup::Lookup (SInt16 selector)
{
	AudioComponentMethod method = AudioCodecLookup::Lookup(selector);
	if (method) return method;
	
	switch (selector) {
		case kAudioCodecAcquireHardwareSelect:			return (AudioComponentMethod)AcquireHardware;
		case kAudioCodecReleaseHardwareSelect:			return (AudioComponentMethod)ReleaseHardware;
		default:
			break;
	}
	return NULL;	
}
#endif // TARGET_OS_IPHONE && AUDIOCONV_HAVE_AMC

#endif // AUDIOCODECS_SUPPORT_PLUGINS
