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

#ifndef _AUDIO_FILE_FORMAT_GENERIC_H
#define _AUDIO_FILE_FORMAT_GENERIC_H
#include "AudioFileFormat.h"
#include <vector>

class AudioFileFormatGeneric : public AudioFileFormat
{
public:
	struct Format : public AudioStreamBasicDescription
	{
		Format(UInt32 formatID, UInt32 formatFlags = 0, UInt32 bitsPerChannel = 0)
		: AudioStreamBasicDescription{}
		{
			mFormatID = formatID;
			mFormatFlags = formatFlags;
			mBitsPerChannel = bitsPerChannel;
		}
	};

	struct Description
	{
		UInt32 fileType;
		const char* name;
		const char* demuxer; // libavformat's name for reading this container
		const char* muxer; // and for writing it, if we can
		std::vector<const char*> extensions;
		std::vector<const char*> utis;
		std::vector<const char*> mimeTypes;
		std::vector<Format> formats;
	};

	AudioFileFormatGeneric(const Description& description);

	Boolean ExtensionIsThisFormat(CFStringRef inExtension) override;
	void GetExtensions(CFArrayRef *outArray) override;
	void GetUTIs(CFArrayRef *outArray) override;
	void GetMIMETypes(CFArrayRef *outArray) override;
	void GetFileTypeName(CFStringRef *outName) override;
	UInt32 CanWrite() const override { return m_description.muxer != nullptr; }
	UncertainResult FileDataIsThisFormat(UInt32 inDataByteSize, const void* inData) override;
	AudioFileObject* New() override;
	AudioFileStreamObject* NewStream() override { return NULL; }

	// File format -> data formats
	OSStatus GetAvailableFormatIDs(UInt32* ioDataSize, void* outPropertyData) override;
	OSStatus GetAvailableStreamDescriptions(UInt32 inFormatID, UInt32* ioDataSize, void* outPropertyData) override;
private:
	static CFArrayRef toCFArray(const std::vector<const char*>& array);
	const Description& m_description;
};

#endif
