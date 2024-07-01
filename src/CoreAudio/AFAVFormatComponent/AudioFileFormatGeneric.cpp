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
#include "AudioFileFormatGeneric.h"
#include <cstring>
#include <CarbonCore/MacErrors.h>

extern "C" {
#include <libavformat/avformat.h>
}

AudioFileFormatGeneric::AudioFileFormatGeneric(UInt32 inFileType, const char* avformatShortName)
: AudioFileFormat(inFileType), m_avformatShortName(avformatShortName)
{
}

Boolean AudioFileFormatGeneric::ExtensionIsThisFormat(CFStringRef inExtension)
{
	const char* inStr = CFStringGetCStringPtr(inExtension, kCFStringEncodingUTF8);
	const Description& desc = description();

	for (const char* ext : desc.extensions)
	{
		if (::strcasecmp(ext, inStr) == 0)
			return true;
	}

	return false;
}

void AudioFileFormatGeneric::GetExtensions(CFArrayRef *outArray)
{
	*outArray = toCFArray(description().extensions);
}

CFArrayRef AudioFileFormatGeneric::toCFArray(const std::vector<const char*>& array)
{
	std::vector<CFStringRef> strings;
	strings.reserve(array.size());

	for (const char* str : array)
		strings.push_back(CFStringCreateWithCString(nullptr, str, kCFStringEncodingUTF8));

	CFArrayRef rv = CFArrayCreate(nullptr, (const void**) strings.data(), array.size(), &kCFTypeArrayCallBacks);

	for (CFStringRef str : strings)
		CFRelease(str);

	return rv;
}

void AudioFileFormatGeneric::GetUTIs(CFArrayRef *outArray)
{
	*outArray = toCFArray(description().utis);
}

void AudioFileFormatGeneric::GetMIMETypes(CFArrayRef *outArray)
{
	*outArray = toCFArray(description().mimeTypes);
}

void AudioFileFormatGeneric::GetFileTypeName(CFStringRef *outName)
{
	*outName = CFStringCreateWithCString(nullptr, description().name, kCFStringEncodingUTF8);
}

UncertainResult AudioFileFormatGeneric::FileDataIsThisFormat(UInt32 inDataByteSize, const void* inData)
{
	std::vector<uint8_t> buf;
	AVProbeData probeData;

	buf.reserve(inDataByteSize + AVPROBE_PADDING_SIZE);
	buf.insert(buf.end(), static_cast<const uint8_t*>(inData), static_cast<const uint8_t*>(inData) + inDataByteSize);
	buf.resize(inDataByteSize + AVPROBE_PADDING_SIZE);

	probeData.filename = "";
	probeData.buf = buf.data();
	probeData.buf_size = inDataByteSize;

#warning "TODO: Remove old `read_probe` call once we no longer support older distros" 
#if LIBAVCODEC_VERSION_MAJOR >= 61
	return av_probe_input_format(&probeData, false) != nullptr ? kTrue : kFalse;
#else
	const AVInputFormat* fmt = av_find_input_format(m_avformatShortName);
	if (!fmt)
		return false;
	return fmt->read_probe(&probeData) ? kTrue : kFalse;
#endif
}

AudioFileObject* AudioFileFormatGeneric::New()
{
	// This method is never called from AFPublic
	return nullptr;
}

OSStatus AudioFileFormatGeneric::GetAvailableFormatIDs(UInt32* ioDataSize, void* outPropertyData)
{
	const std::vector<Format>& formats = description().formats;

	*ioDataSize = formats.size() * sizeof(UInt32);
	if (outPropertyData)
	{
		UInt32* dest = static_cast<UInt32*>(outPropertyData);
		for (int i = 0; i < formats.size(); i++)
			dest[i] = formats[i].mFormatID;
	}

	return noErr;
}

OSStatus AudioFileFormatGeneric::GetAvailableStreamDescriptions(UInt32 inFormatID, UInt32* ioDataSize, void* outPropertyData)
{
	int count = 0;

	for (const Format& format : description().formats)
	{
		if (format.mFormatID == inFormatID)
		{
			if (outPropertyData)
			{
				AudioStreamBasicDescription* ptr = static_cast<AudioStreamBasicDescription*>(outPropertyData);
				ptr[count] = format;
			}
			count++;
		}
	}

	*ioDataSize = count * sizeof(AudioStreamBasicDescription);
	return noErr;
}
