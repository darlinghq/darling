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

#include "AudioFileAVFormat.h"
#include <algorithm>
#include <cstring>

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/mem.h>
}

static const int IO_BUFFER_SIZE = 32 * 1024;

static const struct
{
	AVCodecID codec;
	UInt32 flags;
	UInt32 bits;
} PCM_CODECS[] = {
	{ AV_CODEC_ID_PCM_U8, 0, 8 },
	{ AV_CODEC_ID_PCM_S8, kAudioFormatFlagIsSignedInteger, 8 },
	{ AV_CODEC_ID_PCM_S16LE, kAudioFormatFlagIsSignedInteger, 16 },
	{ AV_CODEC_ID_PCM_S16BE, kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsBigEndian, 16 },
	{ AV_CODEC_ID_PCM_S24LE, kAudioFormatFlagIsSignedInteger, 24 },
	{ AV_CODEC_ID_PCM_S24BE, kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsBigEndian, 24 },
	{ AV_CODEC_ID_PCM_S32LE, kAudioFormatFlagIsSignedInteger, 32 },
	{ AV_CODEC_ID_PCM_S32BE, kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsBigEndian, 32 },
	{ AV_CODEC_ID_PCM_F32LE, kAudioFormatFlagIsFloat, 32 },
	{ AV_CODEC_ID_PCM_F32BE, kAudioFormatFlagIsFloat | kAudioFormatFlagIsBigEndian, 32 },
	{ AV_CODEC_ID_PCM_F64LE, kAudioFormatFlagIsFloat, 64 },
	{ AV_CODEC_ID_PCM_F64BE, kAudioFormatFlagIsFloat | kAudioFormatFlagIsBigEndian, 64 },
};

static const struct
{
	AVCodecID codec;
	UInt32 format;
} COMPRESSED_CODECS[] = {
	{ AV_CODEC_ID_PCM_MULAW, kAudioFormatULaw },
	{ AV_CODEC_ID_PCM_ALAW, kAudioFormatALaw },
	{ AV_CODEC_ID_MP1, kAudioFormatMPEGLayer1 },
	{ AV_CODEC_ID_MP2, kAudioFormatMPEGLayer2 },
	{ AV_CODEC_ID_MP3, kAudioFormatMPEGLayer3 },
	{ AV_CODEC_ID_AAC, kAudioFormatMPEG4AAC },
	{ AV_CODEC_ID_AC3, kAudioFormatAC3 },
	{ AV_CODEC_ID_ALAC, kAudioFormatAppleLossless },
	{ AV_CODEC_ID_FLAC, kAudioFormatFLAC },
	{ AV_CODEC_ID_ADPCM_IMA_QT, kAudioFormatAppleIMA4 },
	{ AV_CODEC_ID_AMR_NB, kAudioFormatAMR },
};

static AVCodecID codecForFormat(const AudioStreamBasicDescription* format)
{
	if (format->mFormatID == kAudioFormatLinearPCM)
	{
		const UInt32 mask = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsFloat | kAudioFormatFlagIsBigEndian;

		for (auto const& pcm : PCM_CODECS)
		{
			// Byte order means nothing to 8-bit samples
			const UInt32 ignored = (format->mBitsPerChannel == 8) ? kAudioFormatFlagIsBigEndian : 0;

			if (pcm.bits == format->mBitsPerChannel && pcm.flags == (format->mFormatFlags & mask & ~ignored))
				return pcm.codec;
		}
		return AV_CODEC_ID_NONE;
	}

	for (auto const& compressed : COMPRESSED_CODECS)
	{
		if (compressed.format == format->mFormatID)
			return compressed.codec;
	}
	return AV_CODEC_ID_NONE;
}

static bool formatForCodec(const AVCodecParameters* par, AudioStreamBasicDescription* format)
{
	*format = AudioStreamBasicDescription{};
	format->mSampleRate = par->sample_rate;
	format->mChannelsPerFrame = par->ch_layout.nb_channels;

	for (auto const& pcm : PCM_CODECS)
	{
		if (pcm.codec == par->codec_id)
		{
			format->mFormatID = kAudioFormatLinearPCM;
			format->mFormatFlags = pcm.flags | kAudioFormatFlagIsPacked;
			format->mBitsPerChannel = pcm.bits;
			format->mFramesPerPacket = 1;
			format->mBytesPerFrame = format->mBytesPerPacket = pcm.bits / 8 * format->mChannelsPerFrame;
			return true;
		}
	}

	for (auto const& compressed : COMPRESSED_CODECS)
	{
		if (compressed.codec == par->codec_id)
		{
			format->mFormatID = compressed.format;

			if (par->codec_id == AV_CODEC_ID_PCM_MULAW || par->codec_id == AV_CODEC_ID_PCM_ALAW)
			{
				format->mBitsPerChannel = 8;
				format->mFramesPerPacket = 1;
				format->mBytesPerFrame = format->mBytesPerPacket = format->mChannelsPerFrame;
			}
			return true;
		}
	}

	return false;
}

AudioFileAVFormat::AudioFileAVFormat(UInt32 fileType, const char* demuxer, const char* muxer)
: AudioFileObject(fileType), m_demuxer(demuxer), m_muxer(muxer)
{
}

AudioFileAVFormat::~AudioFileAVFormat()
{
	finishMuxer();
	freeIO();
}

Boolean AudioFileAVFormat::IsDataFormatSupported(const AudioStreamBasicDescription* inFormat)
{
	// Whatever the demuxer found is supported
	if (m_opened)
		return true;
	if (!m_muxer)
		return false;

	const AVOutputFormat* muxer = av_guess_format(m_muxer, nullptr, nullptr);
	const AVCodecID codec = codecForFormat(inFormat);

	if (!muxer || codec == AV_CODEC_ID_NONE)
		return false;

	return avformat_query_codec(muxer, codec, FF_COMPLIANCE_NORMAL) == 1;
}

int AudioFileAVFormat::ioRead(void* opaque, uint8_t* buf, int size)
{
	AudioFileAVFormat* self = static_cast<AudioFileAVFormat*>(opaque);
	UInt32 actual = 0;

	if (self->GetDataSource()->ReadBytes(SEEK_SET, self->m_ioPosition, size, buf, &actual) != noErr && actual == 0)
		return AVERROR(EIO);
	if (actual == 0)
		return AVERROR_EOF;

	self->m_ioPosition += actual;
	return actual;
}

#if LIBAVFORMAT_VERSION_MAJOR >= 61
int AudioFileAVFormat::ioWrite(void* opaque, const uint8_t* buf, int size)
#else
int AudioFileAVFormat::ioWrite(void* opaque, uint8_t* buf, int size)
#endif
{
	AudioFileAVFormat* self = static_cast<AudioFileAVFormat*>(opaque);
	UInt32 actual = 0;

	if (self->GetDataSource()->WriteBytes(SEEK_SET, self->m_ioPosition, size, buf, &actual) != noErr)
		return AVERROR(EIO);

	self->m_ioPosition += actual;
	return actual;
}

int64_t AudioFileAVFormat::ioSeek(void* opaque, int64_t offset, int whence)
{
	AudioFileAVFormat* self = static_cast<AudioFileAVFormat*>(opaque);
	SInt64 size;

	if (self->GetDataSource()->GetSize(size) != noErr)
		return AVERROR(EIO);

	switch (whence & ~AVSEEK_FORCE)
	{
		case AVSEEK_SIZE:
			return size;
		case SEEK_SET:
			self->m_ioPosition = offset;
			break;
		case SEEK_CUR:
			self->m_ioPosition += offset;
			break;
		case SEEK_END:
			self->m_ioPosition = size + offset;
			break;
		default:
			return AVERROR(EINVAL);
	}

	return self->m_ioPosition;
}

AVIOContext* AudioFileAVFormat::createIO(bool writable)
{
	uint8_t* buffer = static_cast<uint8_t*>(av_malloc(IO_BUFFER_SIZE));
	if (!buffer)
		return nullptr;

	m_ioPosition = 0;
	m_io = avio_alloc_context(buffer, IO_BUFFER_SIZE, writable, this, ioRead, ioWrite, ioSeek);

	if (!m_io)
		av_free(buffer);

	return m_io;
}

void AudioFileAVFormat::freeIO()
{
	if (m_io)
	{
		av_freep(&m_io->buffer);
		avio_context_free(&m_io);
	}
}

OSStatus AudioFileAVFormat::OpenFromDataSource()
{
	const AVInputFormat* demuxer = av_find_input_format(m_demuxer);
	AVFormatContext* input = avformat_alloc_context();
	OSStatus status = kAudioFileInvalidFileError;

	m_opened = true;

	if (!demuxer || !input || !createIO(false))
	{
		avformat_free_context(input);
		return kAudioFileUnspecifiedError;
	}

	input->pb = m_io;

	// This frees the context on failure
	if (avformat_open_input(&input, nullptr, demuxer, nullptr) < 0)
	{
		freeIO();
		return kAudioFileInvalidFileError;
	}

	const SInt64 headerEnd = avio_tell(m_io);
	std::vector<AudioStreamPacketDescription> packets;
	AudioStreamBasicDescription format;
	SInt64 dataOffset = -1;
	SInt64 dataEnd = -1;
	UInt32 commonFrames = 0;
	bool sameFrames = true;

	const int streamIndex = (avformat_find_stream_info(input, nullptr) >= 0)
		? av_find_best_stream(input, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0) : -1;

	if (streamIndex >= 0)
	{
		const AVStream* stream = input->streams[streamIndex];

		if (formatForCodec(stream->codecpar, &format))
		{
			const AVRational frameBase = { 1, stream->codecpar->sample_rate };
			AVPacket* packet = av_packet_alloc();

			// The demuxer knows where the audio is and how it is framed, but going through
			// the packets is the only way of asking it that works for all formats.
			while (packet && av_read_frame(input, packet) >= 0)
			{
				if (packet->stream_index == streamIndex && packet->pos >= 0)
				{
					if (dataOffset < 0)
						dataOffset = packet->pos;

					UInt32 frames = 0;
					if (packet->duration > 0)
						frames = UInt32(av_rescale_q(packet->duration, stream->time_base, frameBase));
					if (!frames)
						frames = stream->codecpar->frame_size;

					if (packets.empty())
						commonFrames = frames;
					else if (frames != commonFrames)
						sameFrames = false;

					dataEnd = packet->pos + packet->size;

					if (!format.mBytesPerPacket)
					{
						AudioStreamPacketDescription description = {};
						description.mStartOffset = packet->pos - dataOffset;
						description.mDataByteSize = packet->size;
						description.mVariableFramesInPacket = frames;
						packets.push_back(description);
					}
					else if (packets.empty())
						packets.resize(1);
				}
				av_packet_unref(packet);
			}
			av_packet_free(&packet);

			if (dataOffset < 0)
				dataOffset = dataEnd = headerEnd;

			if (!format.mBytesPerPacket)
			{
				format.mFramesPerPacket = sameFrames ? commonFrames : 0;

				if (stream->codecpar->extradata_size > 0)
					m_cookie.assign(stream->codecpar->extradata, stream->codecpar->extradata + stream->codecpar->extradata_size);
			}

			status = SetDataFormat(&format);
		}
		else
			status = kAudioFileUnsupportedDataFormatError;
	}

	avformat_close_input(&input);
	freeIO();

	if (status != noErr)
		return status;

	SetDataOffset(dataOffset);
	SetNumBytes(dataEnd - dataOffset);

	if (format.mBytesPerPacket)
		SetNumPackets((dataEnd - dataOffset) / format.mBytesPerPacket);
	else
	{
		ClearPacketTable();

		// AppendPacket() would have each frame offset point at the end of its packet
		CompressedPacketTable* table = GetPacketTable(true);
		SInt64 frameOffset = 0;

		for (const AudioStreamPacketDescription& packet : packets)
		{
			AudioStreamPacketDescriptionExtended extended = {};
			extended.mStartOffset = packet.mStartOffset;
			extended.mDataByteSize = packet.mDataByteSize;
			extended.mVariableFramesInPacket = packet.mVariableFramesInPacket;
			extended.mFrameOffset = frameOffset;
			table->push_back(extended);

			frameOffset += format.mFramesPerPacket ? format.mFramesPerPacket : packet.mVariableFramesInPacket;
			SetMaximumPacketSize(std::max(GetMaximumPacketSize(), packet.mDataByteSize));
		}

		SetNumPackets(packets.size());
	}

	// There may be other things after the audio data, and the muxer isn't there to move them
	SetIsOptimized(false);

	return noErr;
}

OSStatus AudioFileAVFormat::startMuxer()
{
	if (m_output)
		return noErr;

	const AudioStreamBasicDescription& format = GetDataFormat();
	AVFormatContext* output = nullptr;

	if (avformat_alloc_output_context2(&output, nullptr, m_muxer, nullptr) < 0)
		return kAudioFileUnspecifiedError;

	AVStream* stream = avformat_new_stream(output, nullptr);

	if (!stream || !createIO(true))
	{
		avformat_free_context(output);
		return kAudioFileUnspecifiedError;
	}

	output->pb = m_io;
	output->flags |= AVFMT_FLAG_BITEXACT;

	AVCodecParameters* par = stream->codecpar;
	par->codec_type = AVMEDIA_TYPE_AUDIO;
	par->codec_id = codecForFormat(&format);
	par->sample_rate = int(format.mSampleRate);
	par->bits_per_coded_sample = format.mBitsPerChannel;
	par->block_align = format.mBytesPerPacket;
	par->frame_size = format.mFramesPerPacket;
	par->bit_rate = int64_t(format.mSampleRate * format.mBytesPerPacket * 8);
	av_channel_layout_default(&par->ch_layout, format.mChannelsPerFrame);

	stream->time_base = { 1, par->sample_rate };

	if (!m_cookie.empty())
	{
		par->extradata = static_cast<uint8_t*>(av_mallocz(m_cookie.size() + AV_INPUT_BUFFER_PADDING_SIZE));
		par->extradata_size = int(m_cookie.size());
		memcpy(par->extradata, m_cookie.data(), m_cookie.size());
	}

	if (avformat_write_header(output, nullptr) < 0)
	{
		avformat_free_context(output);
		freeIO();
		return kAudioFileUnsupportedDataFormatError;
	}

	avio_flush(m_io);
	SetDataOffset(avio_tell(m_io));
	m_output = output;

	return noErr;
}

OSStatus AudioFileAVFormat::finishMuxer()
{
	if (m_finished || m_opened || !GetDataSource())
		return noErr;

	m_finished = true;

	// Nothing may have been written, and that makes for a valid empty file
	OSStatus status = startMuxer();
	if (status != noErr)
		return status;

	// Finalizing relies on being positioned at the end of data
	avio_seek(m_io, GetDataOffset() + GetNumBytes(), SEEK_SET);

	if (av_write_trailer(m_output) < 0)
		status = kAudioFileUnspecifiedError;

	avio_flush(m_io);
	avformat_free_context(m_output);
	m_output = nullptr;
	freeIO();

	return status;
}

OSStatus AudioFileAVFormat::Close()
{
	OSStatus status = finishMuxer();
	OSStatus closeStatus = AudioFileObject::Close();

	return (status != noErr) ? status : closeStatus;
}

OSStatus AudioFileAVFormat::writePacket(const void* data, UInt32 size, UInt32 frames)
{
	AVPacket* packet = av_packet_alloc();
	if (!packet)
		return kAudioFileUnspecifiedError;

	const AVRational frameBase = { 1, m_output->streams[0]->codecpar->sample_rate };
	const AVRational timeBase = m_output->streams[0]->time_base;

	packet->data = const_cast<uint8_t*>(static_cast<const uint8_t*>(data));
	packet->size = size;
	packet->pts = packet->dts = av_rescale_q(m_framesWritten, frameBase, timeBase);
	packet->duration = av_rescale_q(frames, frameBase, timeBase);

	int err = av_write_frame(m_output, packet);
	av_packet_free(&packet);

	if (err < 0)
		return kAudioFileUnspecifiedError;

	m_framesWritten += frames;
	return noErr;
}

OSStatus AudioFileAVFormat::appendPackets(UInt32 numBytes, const AudioStreamPacketDescription* descriptions, UInt32 numPackets, const void* buffer)
{
	const AudioStreamBasicDescription& format = GetDataFormat();
	const uint8_t* bytes = static_cast<const uint8_t*>(buffer);
	OSStatus status;

	// An earlier write may have overwritten data, and not at the end
	avio_seek(m_io, GetDataOffset() + GetNumBytes(), SEEK_SET);

	if (format.mBytesPerPacket)
	{
		status = writePacket(bytes, numPackets * format.mBytesPerPacket, numPackets * format.mFramesPerPacket);
		if (status != noErr)
			return status;

		SetNumBytes(GetNumBytes() + numPackets * format.mBytesPerPacket);
		SetNumPackets(GetNumPackets() + numPackets);
		return noErr;
	}

	if (!descriptions)
		return kAudio_ParamError;

	for (UInt32 i = 0; i < numPackets; i++)
	{
		const UInt32 frames = format.mFramesPerPacket ? format.mFramesPerPacket : descriptions[i].mVariableFramesInPacket;

		if (descriptions[i].mStartOffset < 0 || descriptions[i].mStartOffset + descriptions[i].mDataByteSize > numBytes)
			return kAudio_ParamError;

		status = writePacket(bytes + descriptions[i].mStartOffset, descriptions[i].mDataByteSize, frames);
		if (status != noErr)
			return status;

		AudioStreamPacketDescription description = descriptions[i];
		description.mStartOffset = GetNumBytes();
		AppendPacket(description);

		SetNumBytes(GetNumBytes() + description.mDataByteSize);
		SetNumPackets(GetNumPackets() + 1);
	}

	return noErr;
}

OSStatus AudioFileAVFormat::WriteBytes(Boolean inUseCache, SInt64 inStartingByte, UInt32* ioNumBytes, const void* inBuffer)
{
	if (!CanWrite())
		return kAudioFilePermissionsError;

	if (!m_opened)
	{
		OSStatus status = startMuxer();
		if (status != noErr)
			return status;

		avio_flush(m_io);
	}

	return AudioFileObject::WriteBytes(inUseCache, inStartingByte, ioNumBytes, inBuffer);
}

OSStatus AudioFileAVFormat::WritePackets(Boolean inUseCache, UInt32 inNumBytes, const AudioStreamPacketDescription* inPacketDescriptions,
	SInt64 inStartingPacket, UInt32* ioNumPackets, const void* inBuffer)
{
	if (!ioNumPackets || !inBuffer)
		return kAudio_ParamError;
	if (!CanWrite())
		return kAudioFilePermissionsError;
	if (inStartingPacket < 0 || inStartingPacket > GetNumPackets())
		return kAudioFileInvalidPacketOffsetError;

	const AudioStreamBasicDescription& format = GetDataFormat();
	UInt32 remaining = *ioNumPackets;
	const uint8_t* bytes = static_cast<const uint8_t*>(inBuffer);

	if (!m_opened)
	{
		OSStatus status = startMuxer();
		if (status != noErr)
			return status;
	}

	// Existing packets of a fixed size can be replaced in place
	if (inStartingPacket < GetNumPackets())
	{
		if (!format.mBytesPerPacket)
			return kAudioFileOperationNotSupportedError;

		UInt32 overwrite = UInt32(std::min<SInt64>(remaining, GetNumPackets() - inStartingPacket));
		UInt32 byteCount = overwrite * format.mBytesPerPacket;

		OSStatus status = WriteBytes(inUseCache, inStartingPacket * format.mBytesPerPacket, &byteCount, bytes);
		if (status != noErr)
			return status;

		bytes += byteCount;
		remaining -= overwrite;
	}

	if (remaining == 0)
		return noErr;

	// The muxer is what keeps the headers correct as the file grows, and an opened file has none
	if (m_opened)
		return kAudioFileOperationNotSupportedError;

	return appendPackets(inNumBytes, inPacketDescriptions, remaining, bytes);
}

OSStatus AudioFileAVFormat::FrameToPacket(SInt64 inFrame, SInt64& outPacket, UInt32& outFrameOffsetInPacket)
{
	const UInt32 framesPerPacket = GetDataFormat().mFramesPerPacket;

	if (inFrame < 0)
		return kAudioFileInvalidPacketOffsetError;

	if (framesPerPacket)
	{
		outPacket = inFrame / framesPerPacket;
		outFrameOffsetInPacket = UInt32(inFrame % framesPerPacket);
		return noErr;
	}

	CompressedPacketTable* table = GetPacketTable();
	if (!table || table->size() == 0)
		return kAudioFileInvalidPacketOffsetError;

	// Find the last packet starting at or before this frame
	SInt64 low = 0, high = table->size() - 1;

	while (low < high)
	{
		const SInt64 middle = (low + high + 1) / 2;

		if ((*table)[middle].mFrameOffset <= inFrame)
			low = middle;
		else
			high = middle - 1;
	}

	const AudioStreamPacketDescriptionExtended packet = (*table)[low];

	if (inFrame >= packet.mFrameOffset + packet.mVariableFramesInPacket)
		return kAudioFileInvalidPacketOffsetError;

	outPacket = low;
	outFrameOffsetInPacket = UInt32(inFrame - packet.mFrameOffset);
	return noErr;
}

OSStatus AudioFileAVFormat::GetMagicCookieDataSize(UInt32* outDataSize, UInt32* isWritable)
{
	if (m_cookie.empty() && m_opened)
		return kAudioFileUnsupportedPropertyError;
	if (outDataSize)
		*outDataSize = UInt32(m_cookie.size());
	if (isWritable)
		*isWritable = !m_opened && !m_output;
	return noErr;
}

OSStatus AudioFileAVFormat::GetMagicCookieData(UInt32* ioDataSize, void* ioPropertyData)
{
	if (!ioDataSize || !ioPropertyData)
		return kAudio_ParamError;
	if (m_cookie.empty())
		return kAudioFileUnsupportedPropertyError;

	*ioDataSize = std::min<UInt32>(*ioDataSize, UInt32(m_cookie.size()));
	memcpy(ioPropertyData, m_cookie.data(), *ioDataSize);
	return noErr;
}

OSStatus AudioFileAVFormat::SetMagicCookieData(UInt32 inDataSize, const void* inPropertyData)
{
	// It goes in the headers, and these are written before the first packet
	if (m_opened || m_output)
		return kAudioFileOperationNotSupportedError;

	const uint8_t* bytes = static_cast<const uint8_t*>(inPropertyData);
	m_cookie.assign(bytes, bytes + inDataSize);
	return noErr;
}
