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


#include <AudioToolbox/ExtendedAudioFile.h>
#include <AudioToolbox/AudioConverter.h>
#include <CarbonCore/MacErrors.h>
#include <CarbonCore/Files.h>
#include <algorithm>
#include <cstddef>
#include <cstring>
#include <vector>

// A private status for the converter's input callback: no data right now, but it isn't the end
static const OSStatus NO_MORE_DATA_YET = 'nomo';

// The client's side of the conversion. The file side is whatever the file holds.
class ExtAudioFile
{
public:
	ExtAudioFile(AudioFileID file, bool ownsFile, bool forWriting);
	~ExtAudioFile();

	OSStatus init();
	OSStatus read(UInt32* ioFrames, AudioBufferList* data);
	OSStatus write(UInt32 frames, const AudioBufferList* data);
	OSStatus seek(SInt64 frame);
	SInt64 tell() const { return m_position; }

	OSStatus propertyInfo(ExtAudioFilePropertyID property, UInt32* size, Boolean* writable);
	OSStatus getProperty(ExtAudioFilePropertyID property, UInt32* ioSize, void* out);
	OSStatus setProperty(ExtAudioFilePropertyID property, UInt32 size, const void* in);
private:
	static OSStatus readInputProc(AudioConverterRef, UInt32* ioPackets, AudioBufferList* data, AudioStreamPacketDescription** descriptions, void* opaque);
	static OSStatus writeInputProc(AudioConverterRef, UInt32* ioPackets, AudioBufferList* data, AudioStreamPacketDescription** descriptions, void* opaque);

	bool needsConverter() const;
	OSStatus setClientFormat(const AudioStreamBasicDescription* format);
	OSStatus createConverter();
	OSStatus readPackets(UInt32 packets, void* buffer, UInt32 bufferSize, UInt32* outBytes, AudioStreamPacketDescription* descriptions);
	OSStatus writeInterleaved(const void* buffer, UInt32 frames);
	OSStatus drainConverter(bool endOfStream);
	OSStatus fileFrameCount(SInt64* frames);
	void defaultLayout(AudioChannelLayout* layout, UInt32 channels) const;

	// Interleaved version of the client format, which is what the converter handles
	AudioStreamBasicDescription interleavedClient() const;
private:
	AudioFileID m_file;
	bool m_ownsFile, m_forWriting;
	AudioStreamBasicDescription m_fileFormat = {};
	AudioStreamBasicDescription m_clientFormat = {};
	bool m_clientSet = false;
	AudioConverterRef m_converter = nullptr;
	std::vector<uint8_t> m_cookie;

	SInt64 m_position = 0; // in client frames
	SInt64 m_nextPacket = 0;
	SInt64 m_skipFrames = 0; // client frames to drop after a seek into a packet
	UInt32 m_ioBufferSize = 64 * 1024;

	std::vector<uint8_t> m_inputBuffer;
	std::vector<uint8_t> m_interleaveBuffer;
	AudioStreamPacketDescription m_inputDescription = {};

	const uint8_t* m_writeData = nullptr;
	UInt32 m_writeFrames = 0;
	bool m_writeEnded = false;
	std::vector<uint8_t> m_outputBuffer;
	std::vector<AudioChannelLayout> m_clientLayout;
};

ExtAudioFile::ExtAudioFile(AudioFileID file, bool ownsFile, bool forWriting)
: m_file(file), m_ownsFile(ownsFile), m_forWriting(forWriting)
{
}

ExtAudioFile::~ExtAudioFile()
{
	if (m_converter)
	{
		if (m_forWriting)
			drainConverter(true);
		AudioConverterDispose(m_converter);
	}

	if (m_ownsFile)
		AudioFileClose(m_file);
}

OSStatus ExtAudioFile::init()
{
	UInt32 size = sizeof(m_fileFormat);
	OSStatus status = AudioFileGetProperty(m_file, kAudioFilePropertyDataFormat, &size, &m_fileFormat);
	if (status != noErr)
		return status;

	// PCM files need no conversion by default, and compressed ones need a format from the client
	if (m_fileFormat.mFormatID == kAudioFormatLinearPCM)
	{
		m_clientFormat = m_fileFormat;
		m_clientSet = true;
	}

	if (m_forWriting)
	{
		UInt64 packets = 0;
		size = sizeof(packets);
		if (AudioFileGetProperty(m_file, kAudioFilePropertyAudioDataPacketCount, &size, &packets) == noErr)
			m_nextPacket = packets;

		if (m_fileFormat.mFramesPerPacket)
			m_position = m_nextPacket * m_fileFormat.mFramesPerPacket;
	}

	return noErr;
}

AudioStreamBasicDescription ExtAudioFile::interleavedClient() const
{
	AudioStreamBasicDescription format = m_clientFormat;

	if (format.mFormatFlags & kAudioFormatFlagIsNonInterleaved)
	{
		format.mFormatFlags &= ~kAudioFormatFlagIsNonInterleaved;
		format.mBytesPerFrame = format.mBytesPerPacket = format.mBytesPerFrame * format.mChannelsPerFrame;
	}

	return format;
}

bool ExtAudioFile::needsConverter() const
{
	const AudioStreamBasicDescription client = interleavedClient();

	if (m_fileFormat.mFormatID != kAudioFormatLinearPCM || client.mFormatID != kAudioFormatLinearPCM)
		return true;

	const UInt32 mask = kAudioFormatFlagIsFloat | kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsBigEndian;

	return m_fileFormat.mSampleRate != client.mSampleRate
		|| m_fileFormat.mChannelsPerFrame != client.mChannelsPerFrame
		|| m_fileFormat.mBitsPerChannel != client.mBitsPerChannel
		|| m_fileFormat.mBytesPerFrame != client.mBytesPerFrame
		|| (m_fileFormat.mFormatFlags & mask) != (client.mFormatFlags & mask);
}

OSStatus ExtAudioFile::createConverter()
{
	if (m_converter)
		return noErr;

	const AudioStreamBasicDescription client = interleavedClient();
	OSStatus status = m_forWriting ? AudioConverterNew(&client, &m_fileFormat, &m_converter)
		: AudioConverterNew(&m_fileFormat, &client, &m_converter);

	if (status != noErr)
		return status;

	if (!m_forWriting)
	{
		UInt32 size = 0;
		UInt32 writable = 0;

		if (AudioFileGetPropertyInfo(m_file, kAudioFilePropertyMagicCookieData, &size, &writable) == noErr && size > 0)
		{
			m_cookie.resize(size);
			status = AudioFileGetProperty(m_file, kAudioFilePropertyMagicCookieData, &size, m_cookie.data());

			if (status == noErr)
				status = AudioConverterSetProperty(m_converter, kAudioConverterDecompressionMagicCookie, size, m_cookie.data());

			if (status != noErr)
			{
				AudioConverterDispose(m_converter);
				m_converter = nullptr;
				return status;
			}
		}
	}

	return noErr;
}

OSStatus ExtAudioFile::setClientFormat(const AudioStreamBasicDescription* format)
{
	// Only PCM can be handed over to the client
	if (format->mFormatID != kAudioFormatLinearPCM)
		return kExtAudioFileError_NonPCMClientFormat;

	if (m_converter)
	{
		if (m_forWriting)
			drainConverter(true);

		AudioConverterDispose(m_converter);
		m_converter = nullptr;
	}

	const SInt64 fileFrame = m_clientSet && m_clientFormat.mSampleRate > 0
		? SInt64(m_position * (m_fileFormat.mSampleRate / m_clientFormat.mSampleRate)) : m_position;

	m_clientFormat = *format;
	m_clientSet = true;

	// A compressed file can be created without a sample rate, to take it from the client's audio
	if (m_forWriting && m_fileFormat.mSampleRate == 0)
	{
		m_fileFormat.mSampleRate = format->mSampleRate;

		OSStatus status = AudioFileSetProperty(m_file, kAudioFilePropertyDataFormat, sizeof(m_fileFormat), &m_fileFormat);
		if (status != noErr)
			return status;
	}

	if (!m_forWriting)
		m_position = SInt64(fileFrame * (m_clientFormat.mSampleRate / m_fileFormat.mSampleRate));

	return needsConverter() ? createConverter() : noErr;
}

OSStatus ExtAudioFile::readPackets(UInt32 packets, void* buffer, UInt32 bufferSize, UInt32* outBytes, AudioStreamPacketDescription* descriptions)
{
	UInt32 bytes = bufferSize;
	OSStatus status = AudioFileReadPacketData(m_file, false, &bytes, descriptions, m_nextPacket, &packets, buffer);

	if (status == kAudioFileEndOfFileError)
		status = noErr;
	if (status != noErr)
		return status;

	m_nextPacket += packets;
	*outBytes = (packets > 0) ? bytes : 0;
	return noErr;
}

OSStatus ExtAudioFile::readInputProc(AudioConverterRef, UInt32* ioPackets, AudioBufferList* data, AudioStreamPacketDescription** descriptions, void* opaque)
{
	ExtAudioFile* self = static_cast<ExtAudioFile*>(opaque);
	const AudioStreamBasicDescription& format = self->m_fileFormat;
	UInt32 packets, capacity;

	if (format.mBytesPerPacket)
	{
		capacity = self->m_ioBufferSize - self->m_ioBufferSize % format.mBytesPerPacket;
		packets = std::min(*ioPackets, capacity / format.mBytesPerPacket);
	}
	else
	{
		// Each compressed packet goes to the decoder on its own
		UInt32 maxPacket = 0;
		UInt32 size = sizeof(maxPacket);
		OSStatus status = AudioFileGetProperty(self->m_file, kAudioFilePropertyMaximumPacketSize, &size, &maxPacket);
		if (status != noErr)
			return status;

		capacity = maxPacket;
		packets = 1;
	}

	self->m_inputBuffer.resize(capacity);

	UInt32 bytes = 0;
	OSStatus status = self->readPackets(packets, self->m_inputBuffer.data(), capacity, &bytes,
		format.mBytesPerPacket ? nullptr : &self->m_inputDescription);
	if (status != noErr)
		return status;

	data->mNumberBuffers = 1;
	data->mBuffers[0].mNumberChannels = format.mChannelsPerFrame;
	data->mBuffers[0].mData = self->m_inputBuffer.data();
	data->mBuffers[0].mDataByteSize = bytes;

	if (format.mBytesPerPacket)
		*ioPackets = bytes / format.mBytesPerPacket;
	else
	{
		*ioPackets = bytes ? 1 : 0;
		self->m_inputDescription.mStartOffset = 0;
	}

	if (descriptions)
		*descriptions = format.mBytesPerPacket ? nullptr : &self->m_inputDescription;

	return noErr;
}

OSStatus ExtAudioFile::read(UInt32* ioFrames, AudioBufferList* data)
{
	if (!ioFrames || !data)
		return kAudio_ParamError;
	if (m_forWriting)
		return kExtAudioFileError_InvalidOperationOrder;
	if (!m_clientSet)
		return kExtAudioFileError_NonPCMClientFormat;

	const AudioStreamBasicDescription client = interleavedClient();
	const bool nonInterleaved = m_clientFormat.mFormatFlags & kAudioFormatFlagIsNonInterleaved;
	UInt32 wanted = *ioFrames;
	UInt32 got = 0;

	// What the caller's buffers can hold limits the request as well
	if (!data->mNumberBuffers)
		return kAudio_ParamError;
	wanted = std::min(wanted, data->mBuffers[0].mDataByteSize / m_clientFormat.mBytesPerFrame);

	if (nonInterleaved)
		m_interleaveBuffer.resize(wanted * client.mBytesPerFrame);

	while (got < wanted)
	{
		uint8_t* target = (nonInterleaved ? m_interleaveBuffer.data() : static_cast<uint8_t*>(data->mBuffers[0].mData))
			+ got * client.mBytesPerFrame;
		UInt32 frames = wanted - got;
		OSStatus status;

		if (needsConverter())
		{
			status = createConverter();
			if (status != noErr)
				return status;

			AudioBufferList output;
			output.mNumberBuffers = 1;
			output.mBuffers[0].mNumberChannels = client.mChannelsPerFrame;
			output.mBuffers[0].mData = target;
			output.mBuffers[0].mDataByteSize = frames * client.mBytesPerFrame;

			status = AudioConverterFillComplexBuffer(m_converter, readInputProc, this, &frames, &output, nullptr);
			if (status != noErr)
				return status;
		}
		else
		{
			UInt32 bytes = 0;
			status = readPackets(frames, target, frames * client.mBytesPerFrame, &bytes, nullptr);
			if (status != noErr)
				return status;

			frames = bytes / client.mBytesPerFrame;
		}

		if (frames == 0)
			break;

		// A seek may have landed in the middle of a packet
		if (m_skipFrames > 0)
		{
			const UInt32 skipped = UInt32(std::min<SInt64>(m_skipFrames, frames));
			m_skipFrames -= skipped;
			frames -= skipped;
			memmove(target, target + skipped * client.mBytesPerFrame, frames * client.mBytesPerFrame);
		}

		got += frames;
	}

	if (nonInterleaved)
	{
		const UInt32 sampleSize = m_clientFormat.mBytesPerFrame;
		const UInt32 channels = std::min<UInt32>(data->mNumberBuffers, client.mChannelsPerFrame);

		for (UInt32 channel = 0; channel < channels; channel++)
		{
			uint8_t* out = static_cast<uint8_t*>(data->mBuffers[channel].mData);

			for (UInt32 frame = 0; frame < got; frame++)
				memcpy(out + frame * sampleSize, m_interleaveBuffer.data() + (frame * client.mChannelsPerFrame + channel) * sampleSize, sampleSize);

			data->mBuffers[channel].mDataByteSize = got * sampleSize;
		}
	}
	else
		data->mBuffers[0].mDataByteSize = got * client.mBytesPerFrame;

	m_position += got;
	*ioFrames = got;
	return noErr;
}

OSStatus ExtAudioFile::writeInputProc(AudioConverterRef, UInt32* ioPackets, AudioBufferList* data, AudioStreamPacketDescription** descriptions, void* opaque)
{
	ExtAudioFile* self = static_cast<ExtAudioFile*>(opaque);

	if (descriptions)
		*descriptions = nullptr;

	data->mNumberBuffers = 1;

	if (self->m_writeFrames == 0)
	{
		*ioPackets = 0;
		data->mBuffers[0].mData = nullptr;
		data->mBuffers[0].mDataByteSize = 0;

		// The converter keeps what it has until we say that this is the end
		return self->m_writeEnded ? noErr : NO_MORE_DATA_YET;
	}

	const AudioStreamBasicDescription client = self->interleavedClient();
	const UInt32 frames = std::min(*ioPackets, self->m_writeFrames);

	data->mBuffers[0].mNumberChannels = client.mChannelsPerFrame;
	data->mBuffers[0].mData = const_cast<uint8_t*>(self->m_writeData);
	data->mBuffers[0].mDataByteSize = frames * client.mBytesPerFrame;

	self->m_writeData += frames * client.mBytesPerFrame;
	self->m_writeFrames -= frames;
	*ioPackets = frames;

	return noErr;
}

// Writes out whatever the converter produces from the current input
OSStatus ExtAudioFile::drainConverter(bool endOfStream)
{
	m_writeEnded = endOfStream;
	m_outputBuffer.resize(m_ioBufferSize);

	while (true)
	{
		const UInt32 capacityPackets = m_fileFormat.mBytesPerPacket ? UInt32(m_outputBuffer.size() / m_fileFormat.mBytesPerPacket) : 1;
		UInt32 packets = capacityPackets;

		AudioBufferList output;
		output.mNumberBuffers = 1;
		output.mBuffers[0].mNumberChannels = m_fileFormat.mChannelsPerFrame;
		output.mBuffers[0].mData = m_outputBuffer.data();
		output.mBuffers[0].mDataByteSize = packets * (m_fileFormat.mBytesPerPacket ? m_fileFormat.mBytesPerPacket : UInt32(m_outputBuffer.size()));

		OSStatus status = AudioConverterFillComplexBuffer(m_converter, writeInputProc, this, &packets, &output, nullptr);
		if (status != noErr)
			return status;
		if (packets == 0 || output.mBuffers[0].mDataByteSize == 0)
			return noErr;

		status = AudioFileWritePackets(m_file, false, output.mBuffers[0].mDataByteSize, nullptr, m_nextPacket, &packets, m_outputBuffer.data());
		if (status != noErr)
			return status;

		m_nextPacket += packets;
	}
}

OSStatus ExtAudioFile::writeInterleaved(const void* buffer, UInt32 frames)
{
	if (needsConverter())
	{
		OSStatus status = createConverter();
		if (status != noErr)
			return status;

		m_writeData = static_cast<const uint8_t*>(buffer);
		m_writeFrames = frames;

		return drainConverter(false);
	}

	UInt32 packets = frames;
	OSStatus status = AudioFileWritePackets(m_file, false, frames * m_fileFormat.mBytesPerPacket, nullptr, m_nextPacket, &packets, buffer);

	if (status == noErr)
		m_nextPacket += packets;

	return status;
}

OSStatus ExtAudioFile::write(UInt32 frames, const AudioBufferList* data)
{
	if (!m_forWriting)
		return kExtAudioFileError_InvalidOperationOrder;
	if (!m_clientSet)
		return kExtAudioFileError_NonPCMClientFormat;
	if (frames == 0)
		return noErr;
	if (!data || !data->mNumberBuffers)
		return kAudio_ParamError;

	OSStatus status;

	if (m_clientFormat.mFormatFlags & kAudioFormatFlagIsNonInterleaved)
	{
		const AudioStreamBasicDescription client = interleavedClient();
		const UInt32 sampleSize = m_clientFormat.mBytesPerFrame;

		if (data->mNumberBuffers < client.mChannelsPerFrame)
			return kAudio_ParamError;

		m_interleaveBuffer.resize(frames * client.mBytesPerFrame);

		for (UInt32 channel = 0; channel < client.mChannelsPerFrame; channel++)
		{
			const uint8_t* in = static_cast<const uint8_t*>(data->mBuffers[channel].mData);

			for (UInt32 frame = 0; frame < frames; frame++)
				memcpy(m_interleaveBuffer.data() + (frame * client.mChannelsPerFrame + channel) * sampleSize, in + frame * sampleSize, sampleSize);
		}

		status = writeInterleaved(m_interleaveBuffer.data(), frames);
	}
	else
		status = writeInterleaved(data->mBuffers[0].mData, frames);

	if (status == noErr)
		m_position += frames;

	return status;
}

OSStatus ExtAudioFile::seek(SInt64 frame)
{
	if (frame < 0)
		return kExtAudioFileError_InvalidSeek;
	if (m_forWriting)
		return (frame == m_position) ? noErr : kExtAudioFileError_InvalidSeek;

	const Float64 ratio = m_clientSet ? m_fileFormat.mSampleRate / m_clientFormat.mSampleRate : 1.0;
	const SInt64 fileFrame = SInt64(frame * ratio);

	if (m_fileFormat.mBytesPerPacket && m_fileFormat.mFramesPerPacket)
	{
		m_nextPacket = fileFrame / m_fileFormat.mFramesPerPacket;
		m_skipFrames = SInt64((fileFrame % m_fileFormat.mFramesPerPacket) / ratio);
	}
	else
	{
		AudioFramePacketTranslation translation = {};
		translation.mFrame = fileFrame;
		UInt32 size = sizeof(translation);

		SInt64 length = 0;
		if (fileFrameCount(&length) == noErr && fileFrame >= length)
		{
			UInt64 packets = 0;
			size = sizeof(packets);
			OSStatus status = AudioFileGetProperty(m_file, kAudioFilePropertyAudioDataPacketCount, &size, &packets);
			if (status != noErr)
				return kExtAudioFileError_InvalidSeek;

			m_nextPacket = packets;
			m_skipFrames = 0;
		}
		else
		{
			OSStatus status = AudioFileGetProperty(m_file, kAudioFilePropertyFrameToPacket, &size, &translation);
			if (status != noErr)
				return kExtAudioFileError_InvalidSeek;

			m_nextPacket = translation.mPacket;
			m_skipFrames = SInt64(translation.mFrameOffsetInPacket / ratio);
		}
	}

	if (m_converter)
		AudioConverterReset(m_converter);

	m_position = frame;
	return noErr;
}

OSStatus ExtAudioFile::fileFrameCount(SInt64* frames)
{
	UInt64 packets = 0;
	UInt32 size = sizeof(packets);
	OSStatus status = AudioFileGetProperty(m_file, kAudioFilePropertyAudioDataPacketCount, &size, &packets);
	if (status != noErr)
		return status;

	if (m_fileFormat.mFramesPerPacket)
	{
		*frames = packets * m_fileFormat.mFramesPerPacket;
		return noErr;
	}

	if (packets == 0)
	{
		*frames = 0;
		return noErr;
	}

	// Packets vary in length, so ask where the last one starts and how long it is
	AudioFramePacketTranslation translation = {};
	translation.mPacket = packets - 1;
	size = sizeof(translation);
	status = AudioFileGetProperty(m_file, kAudioFilePropertyPacketToFrame, &size, &translation);
	if (status != noErr)
		return status;

	UInt32 maxPacket = 0;
	size = sizeof(maxPacket);
	status = AudioFileGetProperty(m_file, kAudioFilePropertyMaximumPacketSize, &size, &maxPacket);
	if (status != noErr)
		return status;

	std::vector<uint8_t> lastPacket(maxPacket);
	AudioStreamPacketDescription description = {};
	UInt32 bytes = maxPacket;
	UInt32 count = 1;

	status = AudioFileReadPacketData(m_file, false, &bytes, &description, packets - 1, &count, lastPacket.data());
	if (status != noErr && status != kAudioFileEndOfFileError)
		return status;

	*frames = translation.mFrame + description.mVariableFramesInPacket;
	return noErr;
}

void ExtAudioFile::defaultLayout(AudioChannelLayout* layout, UInt32 channels) const
{
	memset(layout, 0, sizeof(*layout));

	if (channels == 1)
		layout->mChannelLayoutTag = kAudioChannelLayoutTag_Mono;
	else if (channels == 2)
		layout->mChannelLayoutTag = kAudioChannelLayoutTag_Stereo;
	else
		layout->mChannelLayoutTag = kAudioChannelLayoutTag_DiscreteInOrder | channels;
}

OSStatus ExtAudioFile::propertyInfo(ExtAudioFilePropertyID property, UInt32* size, Boolean* writable)
{
	UInt32 propertySize;
	Boolean canWrite = false;

	switch (property)
	{
		case kExtAudioFileProperty_FileDataFormat:
			propertySize = sizeof(AudioStreamBasicDescription);
			break;
		case kExtAudioFileProperty_ClientDataFormat:
			propertySize = sizeof(AudioStreamBasicDescription);
			canWrite = true;
			break;
		case kExtAudioFileProperty_FileChannelLayout:
			propertySize = offsetof(AudioChannelLayout, mChannelDescriptions);
			canWrite = m_forWriting;
			break;
		case kExtAudioFileProperty_ClientChannelLayout:
			propertySize = offsetof(AudioChannelLayout, mChannelDescriptions);
			canWrite = true;
			break;
		case kExtAudioFileProperty_AudioConverter:
			propertySize = sizeof(AudioConverterRef);
			break;
		case kExtAudioFileProperty_AudioFile:
			propertySize = sizeof(AudioFileID);
			break;
		case kExtAudioFileProperty_FileMaxPacketSize:
		case kExtAudioFileProperty_ClientMaxPacketSize:
			propertySize = sizeof(UInt32);
			break;
		case kExtAudioFileProperty_FileLengthFrames:
			propertySize = sizeof(SInt64);
			break;
		case kExtAudioFileProperty_IOBufferSizeBytes:
			propertySize = sizeof(UInt32);
			canWrite = true;
			break;
		case kExtAudioFileProperty_ConverterConfig:
			propertySize = sizeof(CFArrayRef);
			canWrite = true;
			break;
		case kExtAudioFileProperty_CodecManufacturer:
			propertySize = sizeof(UInt32);
			canWrite = true;
			break;
		default:
			return kExtAudioFileError_InvalidProperty;
	}

	if (size)
		*size = propertySize;
	if (writable)
		*writable = canWrite;

	return noErr;
}

OSStatus ExtAudioFile::getProperty(ExtAudioFilePropertyID property, UInt32* ioSize, void* out)
{
	if (!ioSize || !out)
		return kAudio_ParamError;

	UInt32 required = 0;
	OSStatus status = propertyInfo(property, &required, nullptr);
	if (status != noErr)
		return status;

	if (*ioSize < required)
		return kExtAudioFileError_InvalidPropertySize;
	*ioSize = required;

	switch (property)
	{
		case kExtAudioFileProperty_FileDataFormat:
			*static_cast<AudioStreamBasicDescription*>(out) = m_fileFormat;
			return noErr;
		case kExtAudioFileProperty_ClientDataFormat:
			*static_cast<AudioStreamBasicDescription*>(out) = m_clientFormat;
			return noErr;
		case kExtAudioFileProperty_FileChannelLayout:
			// Our files don't record a layout, and not knowing one isn't the same as having one
			memset(out, 0, required);
			static_cast<AudioChannelLayout*>(out)->mChannelLayoutTag = kAudioChannelLayoutTag_Unknown;
			return noErr;
		case kExtAudioFileProperty_ClientChannelLayout:
			if (!m_clientLayout.empty())
				*static_cast<AudioChannelLayout*>(out) = m_clientLayout[0];
			else
				defaultLayout(static_cast<AudioChannelLayout*>(out), m_clientSet ? m_clientFormat.mChannelsPerFrame : m_fileFormat.mChannelsPerFrame);
			return noErr;
		case kExtAudioFileProperty_AudioConverter:
			if (m_clientSet && needsConverter())
			{
				status = createConverter();
				if (status != noErr)
					return status;
			}
			*static_cast<AudioConverterRef*>(out) = m_converter;
			return noErr;
		case kExtAudioFileProperty_AudioFile:
			*static_cast<AudioFileID*>(out) = m_file;
			return noErr;
		case kExtAudioFileProperty_FileMaxPacketSize:
		{
			UInt32 size = sizeof(UInt32);
			return AudioFileGetProperty(m_file, kAudioFilePropertyMaximumPacketSize, &size, out);
		}
		case kExtAudioFileProperty_ClientMaxPacketSize:
			*static_cast<UInt32*>(out) = m_clientFormat.mBytesPerPacket;
			return noErr;
		case kExtAudioFileProperty_FileLengthFrames:
			return fileFrameCount(static_cast<SInt64*>(out));
		case kExtAudioFileProperty_IOBufferSizeBytes:
			*static_cast<UInt32*>(out) = m_ioBufferSize;
			return noErr;
		case kExtAudioFileProperty_ConverterConfig:
			*static_cast<CFArrayRef*>(out) = nullptr;
			return noErr;
		case kExtAudioFileProperty_CodecManufacturer:
			*static_cast<UInt32*>(out) = 'appl';
			return noErr;
		default:
			return kExtAudioFileError_InvalidProperty;
	}
}

OSStatus ExtAudioFile::setProperty(ExtAudioFilePropertyID property, UInt32 size, const void* in)
{
	if (!in && size)
		return kAudio_ParamError;

	switch (property)
	{
		case kExtAudioFileProperty_ClientDataFormat:
			if (size != sizeof(AudioStreamBasicDescription))
				return kExtAudioFileError_InvalidPropertySize;
			return setClientFormat(static_cast<const AudioStreamBasicDescription*>(in));
		case kExtAudioFileProperty_ClientChannelLayout:
			if (size < offsetof(AudioChannelLayout, mChannelDescriptions))
				return kExtAudioFileError_InvalidPropertySize;
			m_clientLayout.assign(1, *static_cast<const AudioChannelLayout*>(in));
			return noErr;
		case kExtAudioFileProperty_FileChannelLayout:
			// The containers we write have no place for this beyond the channel count
			return m_forWriting ? noErr : kExtAudioFileError_InvalidOperationOrder;
		case kExtAudioFileProperty_IOBufferSizeBytes:
			if (size != sizeof(UInt32))
				return kExtAudioFileError_InvalidPropertySize;
			if (*static_cast<const UInt32*>(in) == 0)
				return kAudio_ParamError;
			m_ioBufferSize = *static_cast<const UInt32*>(in);
			return noErr;
		case kExtAudioFileProperty_ConverterConfig:
		case kExtAudioFileProperty_CodecManufacturer:
			// There's a single codec for each format, and it has no settings to change
			return noErr;
		default:
			return kExtAudioFileError_InvalidProperty;
	}
}

static OSStatus wrapFile(AudioFileID file, bool ownsFile, bool forWriting, ExtAudioFileRef* out)
{
	ExtAudioFile* ext = new ExtAudioFile(file, ownsFile, forWriting);
	OSStatus status = ext->init();

	if (status != noErr)
	{
		delete ext;
		return status;
	}

	*out = reinterpret_cast<ExtAudioFileRef>(ext);
	return noErr;
}

static ExtAudioFile* fromRef(ExtAudioFileRef ref)
{
	return reinterpret_cast<ExtAudioFile*>(ref);
}

OSStatus ExtAudioFileOpenURL( CFURLRef inURL,
       ExtAudioFileRef _Nullable * _Nonnull outExtAudioFile)
{
	if (!outExtAudioFile)
		return kAudio_ParamError;

	*outExtAudioFile = nullptr;

	AudioFileID file;
	OSStatus status = AudioFileOpenURL(inURL, kAudioFileReadPermission, 0, &file);
	if (status != noErr)
		return status;

	status = wrapFile(file, true, false, outExtAudioFile);
	if (status != noErr)
		AudioFileClose(file);

	return status;
}

OSStatus ExtAudioFileWrapAudioFileID(AudioFileID inFileID,
       Boolean inForWriting,
       ExtAudioFileRef _Nullable * _Nonnull outExtAudioFile)
{
	if (!inFileID || !outExtAudioFile)
		return kAudio_ParamError;

	*outExtAudioFile = nullptr;
	return wrapFile(inFileID, false, inForWriting, outExtAudioFile);
}

OSStatus ExtAudioFileCreateWithURL( CFURLRef inURL,
       AudioFileTypeID inFileType,
       const AudioStreamBasicDescription * inStreamDesc,
       const AudioChannelLayout * _Nullable inChannelLayout,
                      UInt32 inFlags,
       ExtAudioFileRef _Nullable * _Nonnull outExtAudioFile)
{
	if (!outExtAudioFile)
		return kAudio_ParamError;

	*outExtAudioFile = nullptr;

	AudioFileID file;
	OSStatus status = AudioFileCreateWithURL(inURL, inFileType, inStreamDesc, inFlags, &file);
	if (status != noErr)
		return status;

	status = wrapFile(file, true, true, outExtAudioFile);
	if (status != noErr)
		AudioFileClose(file);

	return status;
}

OSStatus ExtAudioFileOpen( const struct FSRef * inFSRef,
       ExtAudioFileRef _Nullable * _Nonnull outExtAudioFile)
{
	if (!inFSRef)
		return kAudio_ParamError;

	CFURLRef url = CFURLCreateFromFSRef(nullptr, const_cast<FSRef*>(inFSRef));
	if (!url)
		return fnfErr;

	OSStatus status = ExtAudioFileOpenURL(url, outExtAudioFile);
	CFRelease(url);
	return status;
}

OSStatus ExtAudioFileCreateNew( const struct FSRef * inParentDir,
       CFStringRef inFileName,
       AudioFileTypeID inFileType,
       const AudioStreamBasicDescription * inStreamDesc,
       const AudioChannelLayout * _Nullable inChannelLayout,
       ExtAudioFileRef _Nullable * _Nonnull outExtAudioFile)
{
	if (!inParentDir || !inFileName)
		return kAudio_ParamError;

	CFURLRef parent = CFURLCreateFromFSRef(nullptr, const_cast<FSRef*>(inParentDir));
	if (!parent)
		return fnfErr;

	CFURLRef url = CFURLCreateCopyAppendingPathComponent(nullptr, parent, inFileName, false);
	CFRelease(parent);

	OSStatus status = ExtAudioFileCreateWithURL(url, inFileType, inStreamDesc, inChannelLayout, 0, outExtAudioFile);
	CFRelease(url);
	return status;
}

OSStatus ExtAudioFileDispose( ExtAudioFileRef inExtAudioFile)
{
	if (!inExtAudioFile)
		return kAudio_ParamError;

	delete fromRef(inExtAudioFile);
	return noErr;
}

OSStatus ExtAudioFileRead( ExtAudioFileRef inExtAudioFile,
       UInt32 * ioNumberFrames,
       AudioBufferList * ioData)
{
	return fromRef(inExtAudioFile)->read(ioNumberFrames, ioData);
}

OSStatus ExtAudioFileWrite( ExtAudioFileRef inExtAudioFile,
       UInt32 inNumberFrames,
       const AudioBufferList * ioData)
{
	return fromRef(inExtAudioFile)->write(inNumberFrames, ioData);
}

// This completes the write before returning, which satisfies the documented ordering of
// writes, but unlike on macOS the caller's thread is the one that does the work.
OSStatus ExtAudioFileWriteAsync( ExtAudioFileRef inExtAudioFile,
       UInt32 inNumberFrames,
       const AudioBufferList * _Nullable ioData)
{
	return fromRef(inExtAudioFile)->write(inNumberFrames, ioData);
}

OSStatus ExtAudioFileSeek( ExtAudioFileRef inExtAudioFile,
       SInt64 inFrameOffset)
{
	return fromRef(inExtAudioFile)->seek(inFrameOffset);
}

OSStatus ExtAudioFileTell( ExtAudioFileRef inExtAudioFile,
       SInt64 * outFrameOffset)
{
	if (!outFrameOffset)
		return kAudio_ParamError;

	*outFrameOffset = fromRef(inExtAudioFile)->tell();
	return noErr;
}

OSStatus ExtAudioFileGetPropertyInfo(ExtAudioFileRef inExtAudioFile,
       ExtAudioFilePropertyID inPropertyID,
       UInt32 * _Nullable outSize,
       Boolean * _Nullable outWritable)
{
	return fromRef(inExtAudioFile)->propertyInfo(inPropertyID, outSize, outWritable);
}

OSStatus ExtAudioFileGetProperty( ExtAudioFileRef inExtAudioFile,
       ExtAudioFilePropertyID inPropertyID,
       UInt32 * ioPropertyDataSize,
       void * outPropertyData)
{
	return fromRef(inExtAudioFile)->getProperty(inPropertyID, ioPropertyDataSize, outPropertyData);
}

OSStatus ExtAudioFileSetProperty( ExtAudioFileRef inExtAudioFile,
       ExtAudioFilePropertyID inPropertyID,
       UInt32 inPropertyDataSize,
       const void * inPropertyData)
{
	return fromRef(inExtAudioFile)->setProperty(inPropertyID, inPropertyDataSize, inPropertyData);
}
