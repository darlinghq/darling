/*
     File: ACCodec.h
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
#if !defined(__ACCodec_h__)
#define __ACCodec_h__

//=============================================================================
//	Includes
//=============================================================================



#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
	#include <AudioUnit/AudioCodec.h>
#else
	#include "AudioCodec.h"
#endif

#include "ComponentBase.h"

//=============================================================================
//	ACCodec
//
//	A totally abstract base class for implementing components that conform to
//	the AudioCodec API.
//=============================================================================

class ACCodec : public ComponentBase
{

//	Construction/Destruction
public:
					ACCodec(AudioComponentInstance inInstance);
	virtual			~ACCodec();
	
#if !CA_USE_AUDIO_PLUGIN_ONLY && !TARGET_OS_IPHONE
	static		OSStatus ComponentEntryDispatch(ComponentParameters *p, ACCodec *This);
#endif

//	Property Management
public:
	virtual void	GetPropertyInfo(AudioCodecPropertyID inPropertyID, UInt32& outSize, Boolean& outWritable) = 0;
	virtual void	GetProperty(AudioCodecPropertyID inPropertyID, UInt32& ioPropertyDataSize, void* outPropertyData) = 0;
	virtual void	SetProperty(AudioCodecPropertyID inPropertyID, UInt32 inPropertyDataSize, const void* inPropertyData) = 0;

//	Data Handling
public:
	virtual void	Initialize(const AudioStreamBasicDescription* inInputFormat, const AudioStreamBasicDescription* inOutputFormat, const void* inMagicCookie, UInt32 inMagicCookieByteSize) = 0;
	virtual void	Uninitialize() = 0;
	virtual void	Reset() = 0;
	
	virtual void	AppendInputData(const void* inInputData, UInt32& ioInputDataByteSize, UInt32& ioNumberPackets, const AudioStreamPacketDescription* inPacketDescription) = 0;
	virtual void	AppendInputBufferList(const AudioBufferList *ioBufferList, UInt32& ioNumberPackets, const AudioStreamPacketDescription *inPacketDescription, UInt32 *outBytesConsumed) = 0;
	
	virtual UInt32	ProduceOutputPackets(void* outOutputData, UInt32& ioOutputDataByteSize, UInt32& ioNumberPackets, AudioStreamPacketDescription* outPacketDescription) = 0;
	virtual UInt32	ProduceOutputBufferList(AudioBufferList *ioBufferList, UInt32& ioNumberPackets,  AudioStreamPacketDescription *outPacketDescription) = 0;

//	Component Support
public:
// used internally
	virtual bool	ImplementsFeature(UInt32 feature) const = 0;
};

// when throwing static_cast to OSStatus so the catch will grab the error code correctly
#define CODEC_THROW(err) \
	throw static_cast<OSStatus>(err)

#define CODEC_THROW_IF(cond, err) \
	if(bool(cond)) CODEC_THROW(err);


#endif
