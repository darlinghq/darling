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

#ifndef _AUDIO_FILE_AV_FORMAT_H
#define _AUDIO_FILE_AV_FORMAT_H
#include "AudioFileObject.h"
#include <cstdint>
#include <vector>

extern "C" {
#include <libavformat/version.h>
}

struct AVFormatContext;
struct AVIOContext;

// Reads and writes audio containers through libavformat.
//
// When reading, libavformat finds the audio data and its packets, and the base class
// then reads these bytes directly. When creating a file, packets go through the muxer,
// which writes the headers and finalizes them on close.
class AudioFileAVFormat : public AudioFileObject
{
public:
	AudioFileAVFormat(UInt32 fileType, const char* demuxer, const char* muxer);
	~AudioFileAVFormat();

	Boolean IsDataFormatSupported(const AudioStreamBasicDescription* inFormat) override;
	OSStatus OpenFromDataSource() override;
	OSStatus Close() override;

	OSStatus WriteBytes(Boolean inUseCache, SInt64 inStartingByte, UInt32* ioNumBytes, const void* inBuffer) override;
	OSStatus WritePackets(Boolean inUseCache, UInt32 inNumBytes, const AudioStreamPacketDescription* inPacketDescriptions,
		SInt64 inStartingPacket, UInt32* ioNumPackets, const void* inBuffer) override;

	OSStatus FrameToPacket(SInt64 inFrame, SInt64& outPacket, UInt32& outFrameOffsetInPacket) override;

	OSStatus GetMagicCookieDataSize(UInt32* outDataSize, UInt32* isWritable) override;
	OSStatus GetMagicCookieData(UInt32* ioDataSize, void* ioPropertyData) override;
	OSStatus SetMagicCookieData(UInt32 inDataSize, const void* inPropertyData) override;
private:
	OSStatus startMuxer();
	OSStatus finishMuxer();
	OSStatus appendPackets(UInt32 numBytes, const AudioStreamPacketDescription* descriptions, UInt32 numPackets, const void* buffer);
	OSStatus writePacket(const void* data, UInt32 size, UInt32 frames);
	AVIOContext* createIO(bool writable);
	void freeIO();

	static int ioRead(void* opaque, uint8_t* buf, int size);
#if LIBAVFORMAT_VERSION_MAJOR >= 61
	static int ioWrite(void* opaque, const uint8_t* buf, int size);
#else
	static int ioWrite(void* opaque, uint8_t* buf, int size);
#endif
	static int64_t ioSeek(void* opaque, int64_t offset, int whence);
private:
	const char* m_demuxer;
	const char* m_muxer;
	AVFormatContext* m_output = nullptr;
	AVIOContext* m_io = nullptr;
	int64_t m_ioPosition = 0;
	int64_t m_framesWritten = 0;
	bool m_opened = false;
	bool m_finished = false;
	std::vector<uint8_t> m_cookie;
};

#endif
