/*
This file is part of Darling.

Copyright (C) 2020 Lubos Dolezel

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <AudioToolbox/AudioFormat.h>
#include <CarbonCore/MacErrors.h>
#include <CoreFoundation/CoreFoundation.h>
#include <algorithm>
#include <cstddef>
#include <cstring>

// These go through the FFmpeg codecs used by AudioConverter
static const UInt32 DECODE_FORMATS[] = {
	kAudioFormatLinearPCM, kAudioFormatULaw, kAudioFormatALaw, kAudioFormatMPEGLayer1, kAudioFormatMPEGLayer2,
	kAudioFormatMPEGLayer3, kAudioFormatMPEG4AAC, kAudioFormatAC3, kAudioFormatAppleLossless, kAudioFormatFLAC,
};
static const UInt32 ENCODE_FORMATS[] = {
	kAudioFormatLinearPCM, kAudioFormatULaw, kAudioFormatALaw,
};

static bool layoutChannelCount(const AudioChannelLayout* layout, UInt32* count)
{
	if (layout->mChannelLayoutTag == kAudioChannelLayoutTag_UseChannelDescriptions)
		*count = layout->mNumberChannelDescriptions;
	else if (layout->mChannelLayoutTag == kAudioChannelLayoutTag_UseChannelBitmap)
		*count = __builtin_popcount(layout->mChannelBitmap);
	else
		*count = AudioChannelLayoutTag_GetNumberOfChannels(layout->mChannelLayoutTag);

	return *count != 0;
}

static CFStringRef layoutName(const AudioChannelLayout* layout)
{
	switch (layout->mChannelLayoutTag)
	{
		case kAudioChannelLayoutTag_Mono:
			return CFSTR("Mono");
		case kAudioChannelLayoutTag_Stereo:
			return CFSTR("Stereo");
		default:
			break;
	}

	UInt32 channels = 0;
	layoutChannelCount(layout, &channels);
	return CFStringCreateWithFormat(nullptr, nullptr, CFSTR("%u Channels"), (unsigned) channels);
}

static OSStatus fillFormatInfo(AudioStreamBasicDescription* format)
{
	switch (format->mFormatID)
	{
		case kAudioFormatLinearPCM:
			if (!format->mBitsPerChannel)
				return kAudioFormatBadPropertySizeError;
			format->mFramesPerPacket = 1;
			format->mBytesPerFrame = format->mBytesPerPacket = (format->mBitsPerChannel + 7) / 8 * format->mChannelsPerFrame;
			break;
		case kAudioFormatULaw:
		case kAudioFormatALaw:
			format->mBitsPerChannel = 8;
			format->mFramesPerPacket = 1;
			format->mBytesPerFrame = format->mBytesPerPacket = format->mChannelsPerFrame;
			break;
		case kAudioFormatMPEGLayer1:
			format->mFramesPerPacket = 384;
			break;
		case kAudioFormatMPEGLayer2:
		case kAudioFormatMPEGLayer3:
			format->mFramesPerPacket = 1152;
			break;
		case kAudioFormatMPEG4AAC:
			format->mFramesPerPacket = 1024;
			break;
		case kAudioFormatAC3:
			format->mFramesPerPacket = 1536;
			format->mFormatFlags = 0;
			break;
		case kAudioFormatAppleLossless:
			format->mFramesPerPacket = 4096;
			break;
		case kAudioFormatFLAC:
			break;
		default:
			return kAudioFormatUnsupportedDataFormatError;
	}
	return noErr;
}

template <size_t N>
static OSStatus copyFormatList(const UInt32 (&list)[N], UInt32* ioSize, void* out)
{
	if (out)
	{
		*ioSize = std::min<UInt32>(*ioSize, sizeof(list));
		memcpy(out, list, *ioSize);
	}
	else
		*ioSize = sizeof(list);
	return noErr;
}

OSStatus AudioFormatGetPropertyInfo( AudioFormatPropertyID inPropertyID,
       UInt32 inSpecifierSize,
       const void * _Nullable inSpecifier,
       UInt32 * outPropertyDataSize)
{
	if (!outPropertyDataSize)
		return kAudio_ParamError;

	switch (inPropertyID)
	{
		case kAudioFormatProperty_DecodeFormatIDs:
			*outPropertyDataSize = sizeof(DECODE_FORMATS);
			return noErr;
		case kAudioFormatProperty_EncodeFormatIDs:
			*outPropertyDataSize = sizeof(ENCODE_FORMATS);
			return noErr;
		case kAudioFormatProperty_FormatInfo:
			*outPropertyDataSize = sizeof(AudioStreamBasicDescription);
			return noErr;
		case kAudioFormatProperty_FormatIsVBR:
		case kAudioFormatProperty_NumberOfChannelsForLayout:
			*outPropertyDataSize = sizeof(UInt32);
			return noErr;
		case kAudioFormatProperty_ChannelLayoutName:
			*outPropertyDataSize = sizeof(CFStringRef);
			return noErr;
		default:
			return kAudioFormatUnsupportedPropertyError;
	}
}

OSStatus AudioFormatGetProperty( AudioFormatPropertyID inPropertyID,
      UInt32 inSpecifierSize,
      const void * _Nullable inSpecifier,
      UInt32 * _Nullable ioPropertyDataSize,
      void * _Nullable outPropertyData)
{
	if (!ioPropertyDataSize)
		return kAudio_ParamError;

	switch (inPropertyID)
	{
		case kAudioFormatProperty_DecodeFormatIDs:
			return copyFormatList(DECODE_FORMATS, ioPropertyDataSize, outPropertyData);
		case kAudioFormatProperty_EncodeFormatIDs:
			return copyFormatList(ENCODE_FORMATS, ioPropertyDataSize, outPropertyData);
		case kAudioFormatProperty_FormatInfo:
		{
			// The format to fill in comes in the property data
			if (*ioPropertyDataSize < sizeof(AudioStreamBasicDescription) || !outPropertyData)
				return kAudioFormatBadPropertySizeError;
			return fillFormatInfo(static_cast<AudioStreamBasicDescription*>(outPropertyData));
		}
		case kAudioFormatProperty_FormatIsVBR:
		{
			if (inSpecifierSize < sizeof(AudioStreamBasicDescription) || *ioPropertyDataSize < sizeof(UInt32) || !outPropertyData)
				return kAudioFormatBadPropertySizeError;

			AudioStreamBasicDescription format = *static_cast<const AudioStreamBasicDescription*>(inSpecifier);
			OSStatus status = fillFormatInfo(&format);
			if (status != noErr)
				return status;

			*static_cast<UInt32*>(outPropertyData) = (format.mBytesPerPacket == 0);
			return noErr;
		}
		case kAudioFormatProperty_NumberOfChannelsForLayout:
		{
			if (inSpecifierSize < offsetof(AudioChannelLayout, mChannelDescriptions) || *ioPropertyDataSize < sizeof(UInt32) || !outPropertyData)
				return kAudioFormatBadPropertySizeError;
			if (!layoutChannelCount(static_cast<const AudioChannelLayout*>(inSpecifier), static_cast<UInt32*>(outPropertyData)))
				return kAudioFormatUnsupportedDataFormatError;
			return noErr;
		}
		case kAudioFormatProperty_ChannelLayoutName:
		{
			if (inSpecifierSize < offsetof(AudioChannelLayout, mChannelDescriptions) || *ioPropertyDataSize < sizeof(CFStringRef) || !outPropertyData)
				return kAudioFormatBadPropertySizeError;

			CFStringRef name = layoutName(static_cast<const AudioChannelLayout*>(inSpecifier));
			*static_cast<CFStringRef*>(outPropertyData) = (CFStringRef) CFRetain(name);
			CFRelease(name);
			return noErr;
		}
		default:
			return kAudioFormatUnsupportedPropertyError;
	}
}
