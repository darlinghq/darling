/*
     File: ACBaseCodec.h
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
#if !defined(__ACBaseCodec_h__)
#define __ACBaseCodec_h__

//=============================================================================
//	Includes
//=============================================================================

#include "ACCodec.h"
#include "CAStreamBasicDescription.h"
#include <vector>
#include "GetCodecBundle.h"

//=============================================================================
//	ACBaseCodec
//
//	An abstract subclass of ACCodec that implements all the nuts and bolts
//	of the ACCodec interface, except for buffer handling. This class does
//	the proper dispatching of property requests and manages the list of
//	input and output formats.
//=============================================================================

class ACBaseCodec
:
	public ACCodec
{

//	Construction/Destruction
public:
									ACBaseCodec( AudioComponentInstance inInstance );
	virtual							~ACBaseCodec();

//	Property Management
public:
	virtual void					GetPropertyInfo(AudioCodecPropertyID inPropertyID, UInt32& outPropertyDataSize, Boolean& outWritable);
	virtual void					GetProperty(AudioCodecPropertyID inPropertyID, UInt32& ioPropertyDataSize, void* outPropertyData);
	virtual void					SetProperty(AudioCodecPropertyID inPropertyID, UInt32 inPropertyDataSize, const void* inPropertyData);

//	Data Handling
public:
	bool							IsInitialized() const { return mIsInitialized; }
	virtual void					Initialize(const AudioStreamBasicDescription* inInputFormat, const AudioStreamBasicDescription* inOutputFormat, const void* inMagicCookie, UInt32 inMagicCookieByteSize);
	virtual void					Uninitialize();
	virtual void					Reset();
	virtual UInt32					GetInputBufferByteSize() const = 0;
	virtual UInt32					GetUsedInputBufferByteSize() const = 0;

protected:
	virtual void					ReallocateInputBuffer(UInt32 inInputBufferByteSize) = 0;
	
	bool							mIsInitialized;

//	Format Management
public:
	UInt32							GetNumberSupportedInputFormats() const;
	void							GetSupportedInputFormats(AudioStreamBasicDescription* outInputFormats, UInt32& ioNumberInputFormats) const;

	void							GetCurrentInputFormat(AudioStreamBasicDescription& outInputFormat);
	virtual void					SetCurrentInputFormat(const AudioStreamBasicDescription& inInputFormat);
	
	UInt32							GetNumberSupportedOutputFormats() const;
	void							GetSupportedOutputFormats(AudioStreamBasicDescription* outOutputFormats, UInt32& ioNumberOutputFormats) const;
	
	void							GetCurrentOutputFormat(AudioStreamBasicDescription& outOutputFormat);
	virtual void					SetCurrentOutputFormat(const AudioStreamBasicDescription& inOutputFormat);
	
	virtual UInt32					GetMagicCookieByteSize() const;
	virtual void					GetMagicCookie(void* outMagicCookieData, UInt32& ioMagicCookieDataByteSize) const;
	virtual void					SetMagicCookie(const void* outMagicCookieData, UInt32 inMagicCookieDataByteSize);

	virtual bool					ImplementsFeature(UInt32 /* feature */) const { return false; }
	virtual void					AppendInputBufferList(const AudioBufferList * /* ioBufferList */, UInt32& /* ioNumberPackets */, const AudioStreamPacketDescription * /* inPacketDescription */, UInt32 * /* outBytesConsumed */) { CODEC_THROW(-4); /*unimpErr*/ }
	virtual UInt32					ProduceOutputBufferList(AudioBufferList * /* ioBufferList */, UInt32& /* ioNumberPackets */,  AudioStreamPacketDescription * /* outPacketDescription */) { CODEC_THROW(-4); /*unimpErr*/ }
protected:
	void							AddInputFormat(const AudioStreamBasicDescription& inInputFormat);
	void							AddOutputFormat(const AudioStreamBasicDescription& inOutputFormat);
	
	OSType							GetSubType();
	
	typedef std::vector<CAStreamBasicDescription>	FormatList;
	
	FormatList						mInputFormatList;
#if TARGET_OS_WIN32
	// VC 2005 screws up if this is not aligned to 8-byte boundaries
	__declspec(align(8)) CAStreamBasicDescription		mInputFormat;
#else
	CAStreamBasicDescription		mInputFormat;
#endif
	
	FormatList						mOutputFormatList;
#if TARGET_OS_WIN32
	// VC 2005 screws up if this is not aligned to 8-byte boundaries
	__declspec(align(8)) CAStreamBasicDescription		mOutputFormat;
#else
	CAStreamBasicDescription		mOutputFormat;
#endif
	OSType							mCodecSubType;
	UInt32							mPadding[3]; // Align this with 16-byte boundaries

};

#endif
