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

class AudioFileFormatGeneric : public AudioFileFormat
{
public:
	AudioFileFormatGeneric(UInt32 inFileType, const char* avformatShortName);
	Boolean ExtensionIsThisFormat(CFStringRef inExtension) override;
	void GetExtensions(CFArrayRef *outArray) override;
	void GetUTIs(CFArrayRef *outArray) override;
	void GetMIMETypes(CFArrayRef *outArray) override;
	void GetFileTypeName(CFStringRef *outName) override;
	UncertainResult FileDataIsThisFormat(UInt32 inDataByteSize, const void* inData) override;

	AudioFileObject* New() override;
	AudioFileStreamObject* NewStream() override { return NULL; }

	// File format -> data formats
	// https://developer.apple.com/library/archive/documentation/MusicAudio/Conceptual/CoreAudioOverview/SupportedAudioFormatsMacOSX/SupportedAudioFormatsMacOSX.html
	OSStatus GetAvailableFormatIDs(UInt32* ioDataSize, void* outPropertyData) override;

	OSStatus GetAvailableStreamDescriptions(UInt32 inFormatID, UInt32* ioDataSize, void* outPropertyData) override;
private:
	static CFArrayRef toCFArray(const std::vector<const char*>& array);
protected:
	struct Format : public AudioStreamBasicDescription
	{
		Format(UInt32 formatID) { mFormatID = formatID; }
		Format(UInt32 formatID, UInt32 formatFlags, UInt32 bitsPerChannel) { mFormatID = formatID; mFormatFlags = formatFlags; mBitsPerChannel = bitsPerChannel; }
		Format() {}
	};
	struct Description
	{
		const char* name;
		std::vector<const char*> extensions;
		std::vector<const char*> utis;
		std::vector<const char*> mimeTypes;
		std::vector<Format> formats;
	};
	virtual const Description& description() const = 0;
private:
	const char* m_avformatShortName;
};

#endif

