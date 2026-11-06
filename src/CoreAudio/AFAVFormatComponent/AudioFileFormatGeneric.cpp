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
#include <set>
#include <sstream>
#include <string>
#include <CarbonCore/MacErrors.h>

extern "C" {
#include <libavformat/avformat.h>
}

AudioFileFormatGeneric::AudioFileFormatGeneric(const Description& description)
: AudioFileFormat(description.fileType), m_description(description)
{
}

Boolean AudioFileFormatGeneric::ExtensionIsThisFormat(CFStringRef inExtension)
{
	char ext[32];

	if (!CFStringGetCString(inExtension, ext, sizeof(ext), kCFStringEncodingUTF8))
		return false;

	for (const char* known : m_description.extensions)
	{
		if (::strcasecmp(known, ext) == 0)
			return true;
	}

	return false;
}

void AudioFileFormatGeneric::GetExtensions(CFArrayRef *outArray)
{
	*outArray = toCFArray(m_description.extensions);
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
	*outArray = toCFArray(m_description.utis);
}

void AudioFileFormatGeneric::GetMIMETypes(CFArrayRef *outArray)
{
	*outArray = toCFArray(m_description.mimeTypes);
}

void AudioFileFormatGeneric::GetFileTypeName(CFStringRef *outName)
{
	*outName = CFStringCreateWithCString(nullptr, m_description.name, kCFStringEncodingUTF8);
}

UncertainResult AudioFileFormatGeneric::FileDataIsThisFormat(UInt32 inDataByteSize, const void* inData)
{
	std::vector<uint8_t> buf(inDataByteSize + AVPROBE_PADDING_SIZE);
	AVProbeData probeData = {};

	memcpy(buf.data(), inData, inDataByteSize);

	probeData.filename = "";
	probeData.buf = buf.data();
	probeData.buf_size = inDataByteSize;

	const AVInputFormat* detected = av_probe_input_format(&probeData, true);
	if (!detected)
		return kFalse;

	// Some demuxers have a list of names, e.g. "mov,mp4,m4a"
	std::istringstream names(detected->name);
	std::string name;

	while (std::getline(names, name, ','))
	{
		if (name == m_description.demuxer)
			return kTrue;
	}

	return kFalse;
}

AudioFileObject* AudioFileFormatGeneric::New()
{
	// This method is never called from AFPublic
	return nullptr;
}

OSStatus AudioFileFormatGeneric::GetAvailableFormatIDs(UInt32* ioDataSize, void* outPropertyData)
{
	std::set<UInt32> ids;

	for (const Format& format : m_description.formats)
		ids.insert(format.mFormatID);

	if (outPropertyData)
	{
		const UInt32 capacity = *ioDataSize / sizeof(UInt32);
		UInt32* dest = static_cast<UInt32*>(outPropertyData);
		UInt32 count = 0;

		for (UInt32 id : ids)
		{
			if (count == capacity)
				break;
			dest[count++] = id;
		}

		*ioDataSize = count * sizeof(UInt32);
	}
	else
		*ioDataSize = ids.size() * sizeof(UInt32);

	return noErr;
}

OSStatus AudioFileFormatGeneric::GetAvailableStreamDescriptions(UInt32 inFormatID, UInt32* ioDataSize, void* outPropertyData)
{
	const UInt32 capacity = *ioDataSize / sizeof(AudioStreamBasicDescription);
	UInt32 count = 0;

	for (const Format& format : m_description.formats)
	{
		if (format.mFormatID != inFormatID)
			continue;

		if (outPropertyData)
		{
			if (count == capacity)
				break;
			static_cast<AudioStreamBasicDescription*>(outPropertyData)[count] = format;
		}
		count++;
	}

	*ioDataSize = count * sizeof(AudioStreamBasicDescription);
	return noErr;
}
