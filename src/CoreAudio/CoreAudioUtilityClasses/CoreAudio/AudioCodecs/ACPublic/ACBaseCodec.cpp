/*
     File: ACBaseCodec.cpp
 Abstract: ACBaseCodec.h
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
//=============================================================================
//	Includes
//=============================================================================
#include <algorithm>
#include "ACBaseCodec.h"

#include "CABundleLocker.h"

#if TARGET_OS_WIN32
	#include "CAWin32StringResources.h"
#endif

//=============================================================================
//	ACBaseCodec
//=============================================================================

ACBaseCodec::ACBaseCodec( AudioComponentInstance inInstance )
:
	ACCodec(inInstance),
	mIsInitialized(false),
	mInputFormatList(),
	mInputFormat(),
	mOutputFormatList(),
	mOutputFormat()
{
	mCodecSubType = 0;
	if (inInstance) GetSubType();
}

ACBaseCodec::~ACBaseCodec()
{
}

void	ACBaseCodec::GetPropertyInfo(AudioCodecPropertyID inPropertyID, UInt32& outPropertyDataSize, Boolean& outWritable)
{
	switch(inPropertyID)
	{
#if BUILD_ADEC_LIB
		case kAudioCodecPropertyNameCFString:
		case kAudioCodecPropertyFormatCFString:
		case kAudioCodecPropertyManufacturerCFString:
			outPropertyDataSize = 0;
			outWritable = false;
			break;
#else
		case kAudioCodecPropertyNameCFString:
			outPropertyDataSize = SizeOf32(CFStringRef);
			outWritable = false;
			break;
			
		case kAudioCodecPropertyManufacturerCFString:
			outPropertyDataSize = SizeOf32(CFStringRef);
			outWritable = false;
			break;
			
		case kAudioCodecPropertyFormatCFString:
			outPropertyDataSize = SizeOf32(CFStringRef);
			outWritable = false;
			break;			
#endif
		case kAudioCodecPropertyRequiresPacketDescription:
			outPropertyDataSize = SizeOf32(UInt32);
			outWritable = false;
			break;
			
		case kAudioCodecPropertyMinimumNumberInputPackets :
			outPropertyDataSize = SizeOf32(UInt32);
			outWritable = false;
			break;
			
		case kAudioCodecPropertyMinimumNumberOutputPackets :
			outPropertyDataSize = SizeOf32(UInt32);
			outWritable = false;
			break;

		case kAudioCodecPropertyCurrentInputFormat:
			outPropertyDataSize = SizeOf32(AudioStreamBasicDescription);
			outWritable = true;
			break;
			
		case kAudioCodecPropertySupportedInputFormats:
		case kAudioCodecPropertyInputFormatsForOutputFormat:
			outPropertyDataSize = GetNumberSupportedInputFormats() * SizeOf32(AudioStreamBasicDescription);
			outWritable = false;
			break;
			
		case kAudioCodecPropertyCurrentOutputFormat:
			outPropertyDataSize = SizeOf32(AudioStreamBasicDescription);
			outWritable = true;
			break;
			
		case kAudioCodecPropertySupportedOutputFormats:
		case kAudioCodecPropertyOutputFormatsForInputFormat:
			outPropertyDataSize = GetNumberSupportedOutputFormats() * SizeOf32(AudioStreamBasicDescription);
			outWritable = false;
			break;
			
		case kAudioCodecPropertyMagicCookie:
			outPropertyDataSize = GetMagicCookieByteSize();
			outWritable = true;
			break;
			
		case kAudioCodecPropertyInputBufferSize:
			outPropertyDataSize = SizeOf32(UInt32);
			outWritable = false;
			break;
			
		case kAudioCodecPropertyUsedInputBufferSize:
			outPropertyDataSize = SizeOf32(UInt32);
			outWritable = false;
			break;
		
		case kAudioCodecPropertyIsInitialized:
			outPropertyDataSize = SizeOf32(UInt32);
			outWritable = false;
			break;

		case kAudioCodecPropertyAvailableNumberChannels:
			outPropertyDataSize = SizeOf32(UInt32) * 2; // Mono, stereo
			outWritable = false;
			break;
			
 		case kAudioCodecPropertyPrimeMethod:
			outPropertyDataSize = SizeOf32(UInt32);
			outWritable = false;
			break;

 		case kAudioCodecPropertyPrimeInfo:
			outPropertyDataSize = SizeOf32(AudioCodecPrimeInfo);
			outWritable = false;
			break;

 		case kAudioCodecPropertyDoesSampleRateConversion:
			outPropertyDataSize = SizeOf32(UInt32);
			outWritable = false;
			break;

		default:
			CODEC_THROW(kAudioCodecUnknownPropertyError);
			break;
			
	};
}

void	ACBaseCodec::GetProperty(AudioCodecPropertyID inPropertyID, UInt32& ioPropertyDataSize, void* outPropertyData)
{
	UInt32 thePacketsToGet;
	
	switch(inPropertyID)
	{
#if !BUILD_ADEC_LIB
		case kAudioCodecPropertyNameCFString:
		{
			if (ioPropertyDataSize != SizeOf32(CFStringRef)) CODEC_THROW(kAudioCodecBadPropertySizeError);
			
			CABundleLocker lock;
			CFStringRef name = CFCopyLocalizedStringFromTableInBundle(CFSTR("unknown codec"), CFSTR("CodecNames"), GetCodecBundle(), CFSTR(""));
			*(CFStringRef*)outPropertyData = name;
			break; 
		}
		
		case kAudioCodecPropertyManufacturerCFString:
		{
			if (ioPropertyDataSize != SizeOf32(CFStringRef)) CODEC_THROW(kAudioCodecBadPropertySizeError);
			
			CABundleLocker lock;
			CFStringRef name = CFCopyLocalizedStringFromTableInBundle(CFSTR("Apple, Inc."), CFSTR("CodecNames"), GetCodecBundle(), CFSTR(""));
			*(CFStringRef*)outPropertyData = name;
			break; 
		}
#else
			// If called on the device these should return nothing but 0
		case kAudioCodecPropertyNameCFString:
		case kAudioCodecPropertyFormatCFString:
		case kAudioCodecPropertyManufacturerCFString:
			ioPropertyDataSize = 0;
			outPropertyData = 0;
			break;
#endif
        case kAudioCodecPropertyRequiresPacketDescription:
  			if(ioPropertyDataSize == SizeOf32(UInt32))
			{
                *reinterpret_cast<UInt32*>(outPropertyData) = 0; 
            }
			else
			{
				CODEC_THROW(kAudioCodecBadPropertySizeError);
			}
            break;
			
		case kAudioCodecPropertyMinimumNumberInputPackets :
			if(ioPropertyDataSize != SizeOf32(UInt32)) CODEC_THROW(kAudioCodecBadPropertySizeError);
			*(UInt32*)outPropertyData = 1;
			break;
			
		case kAudioCodecPropertyMinimumNumberOutputPackets :
			if(ioPropertyDataSize != SizeOf32(UInt32)) CODEC_THROW(kAudioCodecBadPropertySizeError);
			*(UInt32*)outPropertyData = 1;
			break;
			
		case kAudioCodecPropertyCurrentInputFormat:
			if(ioPropertyDataSize == SizeOf32(AudioStreamBasicDescription))
			{
				GetCurrentInputFormat(*reinterpret_cast<AudioStreamBasicDescription*>(outPropertyData));
			}
			else
			{
				CODEC_THROW(kAudioCodecBadPropertySizeError);
			}
			break;
			
		case kAudioCodecPropertySupportedInputFormats:
		case kAudioCodecPropertyInputFormatsForOutputFormat:
			thePacketsToGet = ioPropertyDataSize / SizeOf32(AudioStreamBasicDescription);
			GetSupportedInputFormats(reinterpret_cast<AudioStreamBasicDescription*>(outPropertyData), thePacketsToGet);
			ioPropertyDataSize = thePacketsToGet * SizeOf32(AudioStreamBasicDescription);
			break;
			
		case kAudioCodecPropertyCurrentOutputFormat:
			if(ioPropertyDataSize == SizeOf32(AudioStreamBasicDescription))
			{
				GetCurrentOutputFormat(*reinterpret_cast<AudioStreamBasicDescription*>(outPropertyData));
			}
			else
			{
				CODEC_THROW(kAudioCodecBadPropertySizeError);
			}
			break;
			
		case kAudioCodecPropertySupportedOutputFormats:
		case kAudioCodecPropertyOutputFormatsForInputFormat:
			thePacketsToGet = ioPropertyDataSize / SizeOf32(AudioStreamBasicDescription);
			GetSupportedOutputFormats(reinterpret_cast<AudioStreamBasicDescription*>(outPropertyData), thePacketsToGet);
			ioPropertyDataSize = thePacketsToGet * SizeOf32(AudioStreamBasicDescription);
			break;
			
		case kAudioCodecPropertyMagicCookie:
			if(ioPropertyDataSize >= GetMagicCookieByteSize())
			{
				GetMagicCookie(outPropertyData, ioPropertyDataSize);
			}
			else
			{
				CODEC_THROW(kAudioCodecBadPropertySizeError);
			}
			break;
			
		case kAudioCodecPropertyInputBufferSize:
			if(ioPropertyDataSize == SizeOf32(UInt32))
			{
				*reinterpret_cast<UInt32*>(outPropertyData) = GetInputBufferByteSize();
			}
			else
			{
				CODEC_THROW(kAudioCodecBadPropertySizeError);
			}
			break;
			
		case kAudioCodecPropertyUsedInputBufferSize:
			if(ioPropertyDataSize == SizeOf32(UInt32))
			{
				*reinterpret_cast<UInt32*>(outPropertyData) = GetUsedInputBufferByteSize();
			}
			else
			{
				CODEC_THROW(kAudioCodecBadPropertySizeError);
			}
			break;
			
		case kAudioCodecPropertyIsInitialized:
			if(ioPropertyDataSize == SizeOf32(UInt32))
			{
				*reinterpret_cast<UInt32*>(outPropertyData) = IsInitialized() ? 1 : 0;
			}
			else
			{
				CODEC_THROW(kAudioCodecBadPropertySizeError);
			}
			break;
			
        case kAudioCodecPropertyAvailableNumberChannels:
  			if(ioPropertyDataSize == SizeOf32(UInt32) * 2)
			{
				(reinterpret_cast<UInt32*>(outPropertyData))[0] = 1;
				(reinterpret_cast<UInt32*>(outPropertyData))[1] = 2;
			}
			else
			{
				CODEC_THROW(kAudioCodecBadPropertySizeError);
			}
			break;

        case kAudioCodecPropertyPrimeMethod:
  			if(ioPropertyDataSize == SizeOf32(UInt32))
			{
				*reinterpret_cast<UInt32*>(outPropertyData) = (UInt32)kAudioCodecPrimeMethod_None;
			}
			else
			{
				CODEC_THROW(kAudioCodecBadPropertySizeError);
			}
			break;

		case kAudioCodecPropertyPrimeInfo:
  			if(ioPropertyDataSize == SizeOf32(AudioCodecPrimeInfo) )
			{
				(reinterpret_cast<AudioCodecPrimeInfo*>(outPropertyData))->leadingFrames = 0;
				(reinterpret_cast<AudioCodecPrimeInfo*>(outPropertyData))->trailingFrames = 0;
			}
			else
			{
				CODEC_THROW(kAudioCodecBadPropertySizeError);
			}
			break;

        case kAudioCodecPropertyDoesSampleRateConversion:
  			if(ioPropertyDataSize == SizeOf32(UInt32))
			{
				*reinterpret_cast<UInt32*>(outPropertyData) = 0;
			}
			else
			{
				CODEC_THROW(kAudioCodecBadPropertySizeError);
			}
			break;

		default:
			CODEC_THROW(kAudioCodecUnknownPropertyError);
			break;
			
	};
}

void	ACBaseCodec::SetProperty(AudioCodecPropertyID inPropertyID, UInt32 inPropertyDataSize, const void* inPropertyData)
{
	// No property can be set when the codec is initialized
	if(mIsInitialized)
	{
		CODEC_THROW(kAudioCodecIllegalOperationError);
	}
	
	switch(inPropertyID)
	{
		case kAudioCodecPropertyCurrentInputFormat:
			if(inPropertyDataSize == SizeOf32(AudioStreamBasicDescription))
			{
				SetCurrentInputFormat(*reinterpret_cast<const AudioStreamBasicDescription*>(inPropertyData));
			}
			else
			{
				CODEC_THROW(kAudioCodecBadPropertySizeError);
			}
			break;
			
		case kAudioCodecPropertyCurrentOutputFormat:
			if(inPropertyDataSize == SizeOf32(AudioStreamBasicDescription))
			{
				SetCurrentOutputFormat(*reinterpret_cast<const AudioStreamBasicDescription*>(inPropertyData));
			}
			else
			{
				CODEC_THROW(kAudioCodecBadPropertySizeError);
			}
			break;
			
		case kAudioCodecPropertyMagicCookie:
			SetMagicCookie(inPropertyData, inPropertyDataSize);
			break;
			
		case kAudioCodecPropertyMinimumNumberOutputPackets :
		case kAudioCodecPropertyMinimumNumberInputPackets :
		case kAudioCodecPropertyInputBufferSize:
		case kAudioCodecPropertyNameCFString:
		case kAudioCodecPropertyManufacturerCFString:
		case kAudioCodecPropertyFormatCFString:
		case kAudioCodecPropertySupportedInputFormats:
		case kAudioCodecPropertySupportedOutputFormats:
		case kAudioCodecPropertyUsedInputBufferSize:
		case kAudioCodecPropertyIsInitialized:
		case kAudioCodecPropertyAvailableNumberChannels:
		case kAudioCodecPropertyPrimeMethod:
		case kAudioCodecPropertyPrimeInfo:
		case kAudioCodecPropertyOutputFormatsForInputFormat:
		case kAudioCodecPropertyInputFormatsForOutputFormat:
		case kAudioCodecPropertyDoesSampleRateConversion:
		case kAudioCodecPropertyRequiresPacketDescription:
			CODEC_THROW(kAudioCodecIllegalOperationError);
			break;
			
		default:
			CODEC_THROW(kAudioCodecUnknownPropertyError);
			break;
	};
}

void	ACBaseCodec::Initialize(const AudioStreamBasicDescription* /* inInputFormat */, const AudioStreamBasicDescription* /* inOutputFormat */, const void* /* inMagicCookie */, UInt32 /* inMagicCookieByteSize */)
{
	mIsInitialized = true;
}

void	ACBaseCodec::Uninitialize()
{
	mIsInitialized = false;
}

void	ACBaseCodec::Reset()
{
}

UInt32	ACBaseCodec::GetNumberSupportedInputFormats() const
{
	return (UInt32)mInputFormatList.size();
}

void	ACBaseCodec::GetSupportedInputFormats(AudioStreamBasicDescription* outInputFormats, UInt32& ioNumberInputFormats) const
{
	UInt32 theNumberFormats = (UInt32)mInputFormatList.size();
	ioNumberInputFormats = (theNumberFormats < ioNumberInputFormats) ? theNumberFormats : ioNumberInputFormats;
	
	FormatList::const_iterator theIterator = mInputFormatList.begin();
	theNumberFormats = ioNumberInputFormats;
	while((theNumberFormats > 0) && (theIterator != mInputFormatList.end()))
	{
		*outInputFormats = *theIterator;
		
		++outInputFormats;
		--theNumberFormats;
		std::advance(theIterator, 1);
	}
}

void	ACBaseCodec::GetCurrentInputFormat(AudioStreamBasicDescription& outInputFormat)
{
	outInputFormat = mInputFormat;
}

void	ACBaseCodec::SetCurrentInputFormat(const AudioStreamBasicDescription& inInputFormat)
{
	if(!mIsInitialized)
	{
		mInputFormat = inInputFormat;
	}
	else
	{
		CODEC_THROW(kAudioCodecStateError);
	}
}

UInt32	ACBaseCodec::GetNumberSupportedOutputFormats() const
{
	return (UInt32)mOutputFormatList.size();
}

void	ACBaseCodec::GetSupportedOutputFormats(AudioStreamBasicDescription* outOutputFormats, UInt32& ioNumberOutputFormats) const
{
	UInt32 theNumberFormats = (UInt32)mOutputFormatList.size();
	ioNumberOutputFormats = (theNumberFormats < ioNumberOutputFormats) ? theNumberFormats : ioNumberOutputFormats;
	
	FormatList::const_iterator theIterator = mOutputFormatList.begin();
	theNumberFormats = ioNumberOutputFormats;
	while((theNumberFormats > 0) && (theIterator != mOutputFormatList.end()))
	{
		*outOutputFormats = *theIterator;
		
		++outOutputFormats;
		--theNumberFormats;
		std::advance(theIterator, 1);
	}
}

void	ACBaseCodec::GetCurrentOutputFormat(AudioStreamBasicDescription& outOutputFormat)
{
	outOutputFormat = mOutputFormat;
}

void	ACBaseCodec::SetCurrentOutputFormat(const AudioStreamBasicDescription& inOutputFormat)
{
	if(!mIsInitialized)
	{
		mOutputFormat = inOutputFormat;
	}
	else
	{
		CODEC_THROW(kAudioCodecStateError);
	}
}

UInt32	ACBaseCodec::GetMagicCookieByteSize() const
{
	return 0;
}

void	ACBaseCodec::GetMagicCookie(void* /* outMagicCookieData */, UInt32& ioMagicCookieDataByteSize) const
{
	ioMagicCookieDataByteSize = 0;
}

void	ACBaseCodec::SetMagicCookie(const void* /* outMagicCookieData */, UInt32 /* inMagicCookieDataByteSize */)
{
	if(mIsInitialized)
	{
		CODEC_THROW(kAudioCodecStateError);
	}
}

void	ACBaseCodec::AddInputFormat(const AudioStreamBasicDescription& inInputFormat)
{
	FormatList::iterator theIterator = std::find(mInputFormatList.begin(), mInputFormatList.end(), inInputFormat);
	if(theIterator == mInputFormatList.end())
	{
		theIterator = std::lower_bound(mInputFormatList.begin(), mInputFormatList.end(), inInputFormat);
		mInputFormatList.insert(theIterator, inInputFormat);
	}
}

void	ACBaseCodec::AddOutputFormat(const AudioStreamBasicDescription& inOutputFormat)
{
	FormatList::iterator theIterator = std::find(mOutputFormatList.begin(), mOutputFormatList.end(), inOutputFormat);
	if(theIterator == mOutputFormatList.end())
	{
		theIterator = std::lower_bound(mOutputFormatList.begin(), mOutputFormatList.end(), inOutputFormat);
		mOutputFormatList.insert(theIterator, inOutputFormat);
	}
}

OSType ACBaseCodec::GetSubType()
{
	if (!mCodecSubType)
	{
		AudioComponentDescription desc = GetComponentDescription();
		mCodecSubType = desc.componentSubType;
	}
	return mCodecSubType;
}
