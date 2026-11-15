/*
This file is part of Darling.

Copyright (C) 2026 Lubos Dolezel

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

#ifndef _AUDIO_FILE_NATIVE_FORMAT_H
#define _AUDIO_FILE_NATIVE_FORMAT_H
#include "AudioFileAVFormat.h"
#include <vector>

// Reads and writes WAVE, AIFF, AIFC and CAF files that hold linear PCM, without FFmpeg.
//
// Files with other contents (companded or compressed audio) are handed to the
// libavformat-based object this one derives from, so the choice is made per file.
class AudioFileNativeFormat : public AudioFileAVFormat
{
public:
	AudioFileNativeFormat(UInt32 fileType, const char* demuxer, const char* muxer);

	// The container types with a native implementation
	static bool handlesFileType(UInt32 fileType);
	// Whether the data starts like a file of this type
	static bool signatureMatches(UInt32 fileType, const void* data, UInt32 size);
	// Whether the data starts like a file of any of these types
	static bool anySignatureMatches(const void* data, UInt32 size);
	// Whether the file type can hold this linear PCM format
	static bool canStoreFormat(UInt32 fileType, const AudioStreamBasicDescription* format);

	Boolean IsDataFormatSupported(const AudioStreamBasicDescription* inFormat) override;
	OSStatus Create(CFURLRef inFileRef, const AudioStreamBasicDescription* inFormat) override;
	OSStatus InitializeDataSource(const AudioStreamBasicDescription* inFormat, UInt32 inFlags) override;
	OSStatus OpenFromDataSource() override;
	OSStatus Close() override;
	OSStatus UpdateSize() override;
	OSStatus IsValidFilePosition(SInt64 position) override;

	OSStatus WriteBytes(Boolean inUseCache, SInt64 inStartingByte, UInt32* ioNumBytes, const void* inBuffer) override;
	OSStatus WritePackets(Boolean inUseCache, UInt32 inNumBytes, const AudioStreamPacketDescription* inPacketDescriptions,
		SInt64 inStartingPacket, UInt32* ioNumPackets, const void* inBuffer) override;
	OSStatus FrameToPacket(SInt64 inFrame, SInt64& outPacket, UInt32& outFrameOffsetInPacket) override;

	OSStatus GetMagicCookieDataSize(UInt32* outDataSize, UInt32* isWritable) override;
	OSStatus GetMagicCookieData(UInt32* ioDataSize, void* ioPropertyData) override;
	OSStatus SetMagicCookieData(UInt32 inDataSize, const void* inPropertyData) override;

	OSStatus GetChannelLayoutSize(UInt32* outDataSize, UInt32* isWritable) override;
	OSStatus GetChannelLayout(UInt32* ioDataSize, AudioChannelLayout* ioChannelLayout) override;
	OSStatus SetChannelLayout(UInt32 inDataSize, const AudioChannelLayout* inChannelLayout) override;

private:
	struct Parsed;

	OSStatus parseWave(Parsed& p);
	OSStatus parseAiff(Parsed& p);
	OSStatus parseCaf(Parsed& p);

	OSStatus writeHeader();
	std::vector<uint8_t> buildHeader(SInt64& dataOffset);
	OSStatus patchSizes(bool final);
	bool readAt(SInt64 position, void* buffer, UInt32 size);
	bool writeAt(SInt64 position, const void* buffer, UInt32 size);

private:
	const UInt32 m_fileType;
	bool m_native = false;
	bool m_nativeOpen = false;
	bool m_headerWritten = false;
	bool m_writable = false;
	std::vector<uint8_t> m_layout;			// an AudioChannelLayout
	// Where the sizes are stored in the header (-1 if the file does not have the field)
	SInt64 m_containerSizeField = -1;
	SInt64 m_dataSizeField = -1;
	SInt64 m_framesField = -1;
	SInt64 m_chunkOffsetField = 0;
};

#endif
