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

// The file layouts implemented here are the public ones: Microsoft's RIFF/WAVE format
// (including WAVE_FORMAT_EXTENSIBLE), the AIFF and AIFF-C specifications, and Apple's
// published Core Audio Format specification.

#include "AudioFileNativeFormat.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <cstdio>
#include <string>

namespace {

const SInt64 DATA_ALIGNMENT = 4096;

constexpr UInt32 fourcc(char a, char b, char c, char d)
{
	return (UInt32(uint8_t(a)) << 24) | (UInt32(uint8_t(b)) << 16) | (UInt32(uint8_t(c)) << 8) | uint8_t(d);
}

uint16_t getBE16(const uint8_t* p) { return uint16_t((p[0] << 8) | p[1]); }
uint32_t getBE32(const uint8_t* p) { return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) | p[3]; }
uint64_t getBE64(const uint8_t* p) { return (uint64_t(getBE32(p)) << 32) | getBE32(p + 4); }
uint16_t getLE16(const uint8_t* p) { return uint16_t(p[0] | (p[1] << 8)); }
uint32_t getLE32(const uint8_t* p) { return uint32_t(p[0]) | (uint32_t(p[1]) << 8) | (uint32_t(p[2]) << 16) | (uint32_t(p[3]) << 24); }

void putBE16(std::vector<uint8_t>& v, uint16_t x) { v.push_back(x >> 8); v.push_back(uint8_t(x)); }
void putBE32(std::vector<uint8_t>& v, uint32_t x) { for (int s = 24; s >= 0; s -= 8) v.push_back(uint8_t(x >> s)); }
void putBE64(std::vector<uint8_t>& v, uint64_t x) { putBE32(v, uint32_t(x >> 32)); putBE32(v, uint32_t(x)); }
void putLE16(std::vector<uint8_t>& v, uint16_t x) { v.push_back(uint8_t(x)); v.push_back(x >> 8); }
void putLE32(std::vector<uint8_t>& v, uint32_t x) { for (int s = 0; s < 32; s += 8) v.push_back(uint8_t(x >> s)); }
void putTag(std::vector<uint8_t>& v, const char* tag) { v.insert(v.end(), tag, tag + 4); }

void setBE32(std::vector<uint8_t>& v, size_t at, uint32_t x) { for (int i = 0; i < 4; i++) v[at + i] = uint8_t(x >> (24 - 8 * i)); }

// The 80-bit extended floating point format used for the AIFF sample rate
double readExtended(const uint8_t* p)
{
	const int exponent = ((p[0] & 0x7f) << 8) | p[1];
	const uint64_t mantissa = getBE64(p + 2);
	double value;

	if (exponent == 0 && mantissa == 0)
		value = 0;
	else if (exponent == 0x7fff)
		value = HUGE_VAL;
	else
		value = std::ldexp(double(mantissa), exponent - 16383 - 63);

	return (p[0] & 0x80) ? -value : value;
}

void writeExtended(std::vector<uint8_t>& v, double rate)
{
	uint8_t out[10] = {};

	if (rate > 0)
	{
		int exponent;
		const double fraction = std::frexp(rate, &exponent);	// rate = fraction * 2^exponent, fraction in [0.5, 1)
		const uint64_t mantissa = uint64_t(std::ldexp(fraction, 64));
		const int biased = exponent - 1 + 16383;

		out[0] = uint8_t(biased >> 8);
		out[1] = uint8_t(biased);
		for (int i = 0; i < 8; i++)
			out[2 + i] = uint8_t(mantissa >> (56 - 8 * i));
	}

	v.insert(v.end(), out, out + 10);
}

const UInt32 WAVE_FORMAT_PCM = 1;
const UInt32 WAVE_FORMAT_IEEE_FLOAT = 3;
const UInt32 WAVE_FORMAT_EXTENSIBLE = 0xFFFE;

const uint8_t WAVE_GUID_TAIL[14] = { 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 };

struct MaskLayout
{
	UInt32 mask;
	AudioChannelLayoutTag tag;
};

// WAVE speaker masks that have a layout tag of their own
const MaskLayout MASK_LAYOUTS[] = {
	{ 0x4, kAudioChannelLayoutTag_Mono },
	{ 0x3, kAudioChannelLayoutTag_Stereo },
	{ 0x33, kAudioChannelLayoutTag_Quadraphonic },
	{ 0x60f, kAudioChannelLayoutTag_MPEG_5_1_A },
};

const size_t LAYOUT_HEADER = offsetof(AudioChannelLayout, mChannelDescriptions);

std::vector<uint8_t> layoutFromMask(UInt32 mask)
{
	std::vector<uint8_t> bytes(sizeof(AudioChannelLayout), 0);
	AudioChannelLayout* layout = reinterpret_cast<AudioChannelLayout*>(bytes.data());

	layout->mChannelLayoutTag = kAudioChannelLayoutTag_UseChannelBitmap;
	layout->mChannelBitmap = mask;

	for (const MaskLayout& known : MASK_LAYOUTS)
	{
		if (known.mask == mask)
		{
			layout->mChannelLayoutTag = known.tag;
			layout->mChannelBitmap = 0;
		}
	}

	return bytes;
}

bool maskFromLayout(const AudioChannelLayout* layout, UInt32& mask)
{
	if (layout->mChannelLayoutTag == kAudioChannelLayoutTag_UseChannelBitmap)
	{
		mask = layout->mChannelBitmap;
		return mask != 0;
	}

	for (const MaskLayout& known : MASK_LAYOUTS)
	{
		if (known.tag == layout->mChannelLayoutTag)
		{
			mask = known.mask;
			return true;
		}
	}
	return false;
}

size_t layoutSize(const AudioChannelLayout* layout)
{
	return LAYOUT_HEADER + size_t(layout->mNumberChannelDescriptions) * sizeof(AudioChannelDescription);
}

// AIFF and CAF store the layout big-endian, field by field
bool layoutFromBigEndian(const uint8_t* p, size_t size, std::vector<uint8_t>& out)
{
	if (size < LAYOUT_HEADER)
		return false;

	const UInt32 count = getBE32(p + 8);
	if (size_t(count) * sizeof(AudioChannelDescription) > size - LAYOUT_HEADER)
		return false;

	out.assign(LAYOUT_HEADER + count * sizeof(AudioChannelDescription), 0);
	AudioChannelLayout* layout = reinterpret_cast<AudioChannelLayout*>(out.data());

	layout->mChannelLayoutTag = getBE32(p);
	layout->mChannelBitmap = getBE32(p + 4);
	layout->mNumberChannelDescriptions = count;

	for (UInt32 i = 0; i < count; i++)
	{
		const uint8_t* d = p + LAYOUT_HEADER + i * sizeof(AudioChannelDescription);
		AudioChannelDescription* desc = reinterpret_cast<AudioChannelDescription*>(out.data() + LAYOUT_HEADER) + i;

		desc->mChannelLabel = getBE32(d);
		desc->mChannelFlags = getBE32(d + 4);
		for (int c = 0; c < 3; c++)
		{
			const uint32_t raw = getBE32(d + 8 + 4 * c);
			memcpy(&desc->mCoordinates[c], &raw, 4);
		}
	}
	return true;
}

void layoutToBigEndian(const AudioChannelLayout* layout, std::vector<uint8_t>& v)
{
	putBE32(v, layout->mChannelLayoutTag);
	putBE32(v, layout->mChannelBitmap);
	putBE32(v, layout->mNumberChannelDescriptions);

	for (UInt32 i = 0; i < layout->mNumberChannelDescriptions; i++)
	{
		const AudioChannelDescription& desc = layout->mChannelDescriptions[i];

		putBE32(v, desc.mChannelLabel);
		putBE32(v, desc.mChannelFlags);
		for (int c = 0; c < 3; c++)
		{
			uint32_t raw;
			memcpy(&raw, &desc.mCoordinates[c], 4);
			putBE32(v, raw);
		}
	}
}

const char* intName(UInt32 bits)
{
	switch (bits)
	{
		case 8: return "Linear PCM, 8 bit signed integer";
		case 16: return "Linear PCM, 16 bit big-endian signed integer";
		case 24: return "Linear PCM, 24 bit big-endian signed integer";
		default: return "Linear PCM, 32 bit big-endian signed integer";
	}
}

}

struct AudioFileNativeFormat::Parsed
{
	AudioStreamBasicDescription format = {};
	SInt64 dataOffset = -1;
	SInt64 dataBytes = 0;
	SInt64 frames = -1;			// as stored, if the header says
	SInt64 fileSize = 0;
	bool haveFormat = false;
	bool needsFFmpeg = false;	// a valid file, but not of linear PCM
	SInt64 containerSizeField = -1;
	SInt64 dataSizeField = -1;
	SInt64 framesField = -1;
	SInt64 chunkOffsetField = 0;
	std::vector<uint8_t> layout;
};

AudioFileNativeFormat::AudioFileNativeFormat(UInt32 fileType, const char* demuxer, const char* muxer)
: AudioFileAVFormat(fileType, demuxer, muxer), m_fileType(fileType)
{
}

bool AudioFileNativeFormat::handlesFileType(UInt32 fileType)
{
	return fileType == kAudioFileWAVEType || fileType == kAudioFileAIFFType || fileType == kAudioFileAIFCType || fileType == kAudioFileCAFType;
}

bool AudioFileNativeFormat::signatureMatches(UInt32 fileType, const void* data, UInt32 size)
{
	const uint8_t* p = static_cast<const uint8_t*>(data);

	if (size < 12)
		return false;

	switch (fileType)
	{
		case kAudioFileWAVEType:
			return getBE32(p) == fourcc('R', 'I', 'F', 'F') && getBE32(p + 8) == fourcc('W', 'A', 'V', 'E');
		case kAudioFileAIFFType:
			return getBE32(p) == fourcc('F', 'O', 'R', 'M') && getBE32(p + 8) == fourcc('A', 'I', 'F', 'F');
		case kAudioFileAIFCType:
			return getBE32(p) == fourcc('F', 'O', 'R', 'M') && getBE32(p + 8) == fourcc('A', 'I', 'F', 'C');
		case kAudioFileCAFType:
			return getBE32(p) == fourcc('c', 'a', 'f', 'f') && getBE16(p + 4) == 1;
	}
	return false;
}

bool AudioFileNativeFormat::anySignatureMatches(const void* data, UInt32 size)
{
	return signatureMatches(kAudioFileWAVEType, data, size) || signatureMatches(kAudioFileAIFFType, data, size)
		|| signatureMatches(kAudioFileAIFCType, data, size) || signatureMatches(kAudioFileCAFType, data, size);
}

bool AudioFileNativeFormat::canStoreFormat(UInt32 fileType, const AudioStreamBasicDescription* f)
{
	if (f->mFormatID != kAudioFormatLinearPCM || f->mChannelsPerFrame == 0 || f->mFramesPerPacket != 1)
		return false;

	const UInt32 bits = f->mBitsPerChannel;
	const UInt32 flags = f->mFormatFlags;
	const bool isFloat = (flags & kAudioFormatFlagIsFloat) != 0;
	const bool isSigned = (flags & kAudioFormatFlagIsSignedInteger) != 0;
	const bool big = (flags & kAudioFormatFlagIsBigEndian) != 0;

	// Samples fill their bytes, in a single interleaved stream
	if ((flags & kAudioFormatFlagIsNonInterleaved) || (bits % 8) != 0 || f->mBytesPerFrame != f->mChannelsPerFrame * bits / 8 || f->mBytesPerPacket != f->mBytesPerFrame)
		return false;
	if (!(flags & kAudioFormatFlagIsPacked))
		return false;

	if (isFloat)
	{
		if (bits != 32 && bits != 64)
			return false;
		// Only AIFF-C and CAF have float, and WAVE is little-endian, AIFF-C big-endian
		switch (fileType)
		{
			case kAudioFileWAVEType: return !big;
			case kAudioFileAIFCType: return big;
			case kAudioFileCAFType: return true;
		}
		return false;
	}

	if (bits != 8 && bits != 16 && bits != 24 && bits != 32)
		return false;

	switch (fileType)
	{
		case kAudioFileWAVEType:
			// 8-bit samples are unsigned in WAVE, the others signed
			return !big && (bits == 8 ? !isSigned : isSigned);
		case kAudioFileAIFFType:
			return isSigned && (big || bits == 8);
		case kAudioFileAIFCType:
			// Little-endian is the 'sowt' form, which exists for 16-bit samples only
			return isSigned && (big || bits == 8 || bits == 16);
		case kAudioFileCAFType:
			return isSigned;
	}
	return false;
}

Boolean AudioFileNativeFormat::IsDataFormatSupported(const AudioStreamBasicDescription* inFormat)
{
	if (m_nativeOpen)
		return true;

	if (inFormat->mFormatID == kAudioFormatLinearPCM)
	{
		m_native = true;
		markNative();
		return canStoreFormat(m_fileType, inFormat);
	}

	m_native = false;
	return AudioFileAVFormat::IsDataFormatSupported(inFormat);
}

bool AudioFileNativeFormat::readAt(SInt64 position, void* buffer, UInt32 size)
{
	UInt32 actual = 0;
	return GetDataSource()->ReadBytes(SEEK_SET, position, size, buffer, &actual) == noErr && actual == size;
}

bool AudioFileNativeFormat::writeAt(SInt64 position, const void* buffer, UInt32 size)
{
	UInt32 actual = 0;
	return GetDataSource()->WriteBytes(SEEK_SET, position, size, buffer, &actual) == noErr && actual == size;
}

OSStatus AudioFileNativeFormat::parseWave(Parsed& p)
{
	uint8_t header[12];

	if (!readAt(0, header, sizeof(header)))
		return kAudioFileInvalidFileError;
	if (getBE32(header) != fourcc('R', 'I', 'F', 'F') || getBE32(header + 8) != fourcc('W', 'A', 'V', 'E'))
		return kAudioFileInvalidFileError;

	p.containerSizeField = 4;

	SInt64 position = 12;
	bool haveData = false;
	UInt32 layoutMask = 0;

	while (position + 8 <= p.fileSize)
	{
		uint8_t chunk[8];
		if (!readAt(position, chunk, 8))
			break;

		const UInt32 id = getBE32(chunk);
		const SInt64 size = getLE32(chunk + 4);

		if (id == fourcc('f', 'm', 't', ' '))
		{
			uint8_t fmt[40] = {};
			const UInt32 want = std::min<SInt64>(size, sizeof(fmt));

			if (want < 16 || position + 8 + want > p.fileSize || !readAt(position + 8, fmt, want))
				return kAudioFileInvalidFileError;

			UInt32 tag = getLE16(fmt);
			const UInt32 channels = getLE16(fmt + 2);
			const UInt32 rate = getLE32(fmt + 4);
			const UInt32 blockAlign = getLE16(fmt + 12);
			const UInt32 containerBits = getLE16(fmt + 14);
			UInt32 validBits = containerBits;

			if (tag == WAVE_FORMAT_EXTENSIBLE)
			{
				if (want < 40)
					return kAudioFileInvalidFileError;

				validBits = getLE16(fmt + 18);
				layoutMask = getLE32(fmt + 20);
				tag = getLE16(fmt + 24);

				if (validBits == 0 || validBits > containerBits)
					validBits = containerBits;
			}

			AudioStreamBasicDescription& f = p.format;
			f = {};

			// Other tags are for companded and compressed audio
			if ((tag != WAVE_FORMAT_PCM && tag != WAVE_FORMAT_IEEE_FLOAT) || channels == 0 || containerBits == 0 || (containerBits % 8) != 0)
			{
				p.needsFFmpeg = true;
				return noErr;
			}

			const UInt32 container = containerBits / 8;

			if (blockAlign != channels * container)
				return kAudioFileInvalidFileError;

			f.mFormatID = kAudioFormatLinearPCM;
			f.mSampleRate = rate;
			f.mChannelsPerFrame = channels;
			f.mFramesPerPacket = 1;
			f.mBytesPerFrame = f.mBytesPerPacket = blockAlign;
			f.mBitsPerChannel = validBits;

			if (tag == WAVE_FORMAT_IEEE_FLOAT)
			{
				if (containerBits != 32 && containerBits != 64)
				{
					p.needsFFmpeg = true;
					return noErr;
				}
				f.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;
				f.mBitsPerChannel = containerBits;
			}
			else
			{
				// 8-bit samples are unsigned, and larger ones signed
				f.mFormatFlags = (containerBits == 8) ? 0 : kAudioFormatFlagIsSignedInteger;

				if (containerBits > 32)
				{
					p.needsFFmpeg = true;
					return noErr;
				}

				// Samples narrower than their container sit at its top
				f.mFormatFlags |= (validBits == containerBits) ? kAudioFormatFlagIsPacked : kAudioFormatFlagIsAlignedHigh;
			}

			p.haveFormat = true;
		}
		else if (id == fourcc('d', 'a', 't', 'a'))
		{
			if (!haveData)
			{
				haveData = true;
				p.dataOffset = position + 8;
				p.dataSizeField = position + 4;

				// A size that does not fit in the file (such as one left unfinished) means all there is
				p.dataBytes = std::min<SInt64>(size, std::max<SInt64>(0, p.fileSize - p.dataOffset));
			}
		}

		position += 8 + size + (size & 1);
	}

	if (!p.haveFormat && !p.needsFFmpeg)
		return kAudioFileInvalidFileError;

	if (!haveData)
	{
		p.dataOffset = std::min<SInt64>(position, p.fileSize);
		p.dataBytes = 0;
	}

	if (layoutMask != 0 && __builtin_popcount(layoutMask) == int(p.format.mChannelsPerFrame))
		p.layout = layoutFromMask(layoutMask);

	return noErr;
}

OSStatus AudioFileNativeFormat::parseAiff(Parsed& p)
{
	uint8_t header[12];

	if (!readAt(0, header, sizeof(header)))
		return kAudioFileInvalidFileError;

	const bool compressed = getBE32(header + 8) == fourcc('A', 'I', 'F', 'C');

	if (getBE32(header) != fourcc('F', 'O', 'R', 'M') || (!compressed && getBE32(header + 8) != fourcc('A', 'I', 'F', 'F')))
		return kAudioFileInvalidFileError;

	p.containerSizeField = 4;

	SInt64 position = 12;
	bool haveData = false;
	SInt64 commFrames = 0;
	SInt64 ssndSizeField = -1;
	SInt64 ssndOffset = 0;

	while (position + 8 <= p.fileSize)
	{
		uint8_t chunk[8];
		if (!readAt(position, chunk, 8))
			break;

		const UInt32 id = getBE32(chunk);
		const SInt64 size = getBE32(chunk + 4);

		if (id == fourcc('C', 'O', 'M', 'M'))
		{
			uint8_t comm[18 + 4] = {};
			const UInt32 want = std::min<SInt64>(size, sizeof(comm));

			if (want < 18 || position + 8 + want > p.fileSize || !readAt(position + 8, comm, want))
				return kAudioFileInvalidFileError;

			const UInt32 channels = getBE16(comm);
			commFrames = getBE32(comm + 2);
			const UInt32 bits = getBE16(comm + 6);
			const double rate = readExtended(comm + 8);
			UInt32 type = fourcc('N', 'O', 'N', 'E');

			if (compressed)
			{
				if (want < 22)
					return kAudioFileInvalidFileError;
				type = getBE32(comm + 18);
			}

			AudioStreamBasicDescription& f = p.format;
			f = {};

			if (channels == 0 || bits == 0 || bits > 64)
				return kAudioFileInvalidFileError;

			bool isFloat = false, littleEndian = false, isSigned = true;

			switch (type)
			{
				case fourcc('N', 'O', 'N', 'E'):
				case fourcc('t', 'w', 'o', 's'):
					break;
				case fourcc('s', 'o', 'w', 't'):
					littleEndian = true;
					break;
				case fourcc('i', 'n', '2', '4'):
				case fourcc('i', 'n', '3', '2'):
					break;
				case fourcc('r', 'a', 'w', ' '):
					isSigned = false;
					break;
				case fourcc('f', 'l', '3', '2'):
				case fourcc('F', 'L', '3', '2'):
				case fourcc('f', 'l', '6', '4'):
				case fourcc('F', 'L', '6', '4'):
					isFloat = true;
					break;
				default:
					p.needsFFmpeg = true;
					return noErr;
			}

			if (isFloat ? (bits != 32 && bits != 64) : bits > 32)
			{
				p.needsFFmpeg = true;
				return noErr;
			}

			const UInt32 container = (bits + 7) / 8;

			f.mFormatID = kAudioFormatLinearPCM;
			f.mSampleRate = rate;
			f.mChannelsPerFrame = channels;
			f.mFramesPerPacket = 1;
			f.mBytesPerFrame = f.mBytesPerPacket = channels * container;
			f.mBitsPerChannel = bits;
			f.mFormatFlags = isFloat ? kAudioFormatFlagIsFloat : (isSigned ? kAudioFormatFlagIsSignedInteger : 0);

			if (!littleEndian && isSigned)
				f.mFormatFlags |= kAudioFormatFlagIsBigEndian;

			// The samples of other depths are left-aligned in their bytes
			f.mFormatFlags |= (bits == container * 8) ? kAudioFormatFlagIsPacked : kAudioFormatFlagIsAlignedHigh;

			p.haveFormat = true;
			p.framesField = position + 8 + 2;
		}
		else if (id == fourcc('S', 'S', 'N', 'D'))
		{
			uint8_t ssnd[8];

			if (!haveData && size >= 8 && readAt(position + 8, ssnd, 8))
			{
				haveData = true;
				ssndOffset = getBE32(ssnd);
				p.dataOffset = position + 16 + ssndOffset;
				ssndSizeField = position + 4;
				p.dataBytes = std::min<SInt64>(size - 8 - ssndOffset, std::max<SInt64>(0, p.fileSize - p.dataOffset));
			}
		}
		else if (id == fourcc('C', 'H', 'A', 'N'))
		{
			std::vector<uint8_t> raw(std::min<SInt64>(size, 4096));

			if (!raw.empty() && readAt(position + 8, raw.data(), UInt32(raw.size())))
				layoutFromBigEndian(raw.data(), raw.size(), p.layout);
		}

		position += 8 + size + (size & 1);
	}

	if (!p.haveFormat && !p.needsFFmpeg)
		return kAudioFileInvalidFileError;

	if (!haveData)
	{
		p.dataOffset = std::min<SInt64>(position, p.fileSize);
		p.dataBytes = 0;
	}

	p.dataSizeField = ssndSizeField;
	p.chunkOffsetField = 8 + ssndOffset;

	if (p.haveFormat && commFrames > 0)
		p.frames = commFrames;

	return noErr;
}

OSStatus AudioFileNativeFormat::parseCaf(Parsed& p)
{
	uint8_t header[8];

	if (!readAt(0, header, sizeof(header)) || getBE32(header) != fourcc('c', 'a', 'f', 'f') || getBE16(header + 4) != 1)
		return kAudioFileInvalidFileError;

	SInt64 position = 8;
	bool haveData = false;

	while (position + 12 <= p.fileSize)
	{
		uint8_t chunk[12];
		if (!readAt(position, chunk, 12))
			break;

		const UInt32 id = getBE32(chunk);
		SInt64 size = SInt64(getBE64(chunk + 4));

		if (id == fourcc('d', 'e', 's', 'c'))
		{
			uint8_t desc[32];

			if (size < 32 || position + 12 + 32 > p.fileSize || !readAt(position + 12, desc, 32))
				return kAudioFileInvalidFileError;

			if (getBE32(desc + 8) != kAudioFormatLinearPCM)
			{
				p.needsFFmpeg = true;
				return noErr;
			}

			uint64_t rateBits = getBE64(desc);
			double rate;
			memcpy(&rate, &rateBits, 8);

			const UInt32 flags = getBE32(desc + 12);
			const UInt32 bytesPerPacket = getBE32(desc + 16);
			const UInt32 framesPerPacket = getBE32(desc + 20);
			const UInt32 channels = getBE32(desc + 24);
			const UInt32 bits = getBE32(desc + 28);
			const bool isFloat = (flags & 1) != 0;
			const bool little = (flags & 2) != 0;

			if (channels == 0 || bits == 0 || framesPerPacket != 1 || bytesPerPacket != channels * bits / 8 || (bits % 8) != 0
				|| (isFloat ? (bits != 32 && bits != 64) : bits > 32))
			{
				p.needsFFmpeg = true;
				return noErr;
			}

			AudioStreamBasicDescription& f = p.format;
			f = {};
			f.mFormatID = kAudioFormatLinearPCM;
			f.mSampleRate = rate;
			f.mChannelsPerFrame = channels;
			f.mFramesPerPacket = 1;
			f.mBytesPerFrame = f.mBytesPerPacket = bytesPerPacket;
			f.mBitsPerChannel = bits;
			f.mFormatFlags = (isFloat ? kAudioFormatFlagIsFloat : kAudioFormatFlagIsSignedInteger) | kAudioFormatFlagIsPacked;

			// Byte order means nothing to 8-bit samples
			if (!little && bits > 8)
				f.mFormatFlags |= kAudioFormatFlagIsBigEndian;

			p.haveFormat = true;
		}
		else if (id == fourcc('d', 'a', 't', 'a'))
		{
			if (!haveData && (size >= 4 || size < 0))
			{
				haveData = true;
				p.dataOffset = position + 12 + 4;
				p.dataSizeField = position + 4;

				// An unknown size (-1) extends to the end of the file
				const SInt64 available = std::max<SInt64>(0, p.fileSize - p.dataOffset);
				p.dataBytes = (size < 0) ? available : std::min<SInt64>(size - 4, available);
			}
		}
		else if (id == fourcc('c', 'h', 'a', 'n'))
		{
			std::vector<uint8_t> raw(std::min<SInt64>(size, 4096));

			if (!raw.empty() && readAt(position + 12, raw.data(), UInt32(raw.size())))
				layoutFromBigEndian(raw.data(), raw.size(), p.layout);
		}

		if (size < 0)
			break;

		position += 12 + size;
	}

	if (!p.haveFormat && !p.needsFFmpeg)
		return kAudioFileInvalidFileError;

	if (!haveData)
		return kAudioFileInvalidFileError;

	return noErr;
}

OSStatus AudioFileNativeFormat::OpenFromDataSource()
{
	Parsed parsed;
	OSStatus status;

	if (GetDataSource()->GetSize(parsed.fileSize) != noErr)
		return kAudioFileUnspecifiedError;

	switch (m_fileType)
	{
		case kAudioFileWAVEType:
			status = parseWave(parsed);
			break;
		case kAudioFileAIFFType:
		case kAudioFileAIFCType:
			status = parseAiff(parsed);
			break;
		default:
			status = parseCaf(parsed);
	}

	if (status != noErr || parsed.needsFFmpeg)
	{
		// Whatever is not linear PCM is for libavformat, if there is any
		m_native = false;
		OSStatus fallback = AudioFileAVFormat::OpenFromDataSource();

		return (fallback == kAudioFileUnsupportedFileTypeError && status != noErr) ? status : fallback;
	}

	m_native = true;
	markNative();
	m_nativeOpen = true;
	status = SetDataFormat(&parsed.format);
	m_nativeOpen = false;

	if (status != noErr)
		return status;

	const AudioStreamBasicDescription& f = parsed.format;
	SInt64 packets = parsed.dataBytes / f.mBytesPerPacket;

	if (parsed.frames >= 0)
		packets = std::min(packets, parsed.frames);

	SetDataOffset(parsed.dataOffset);
	SetNumBytes(packets * f.mBytesPerPacket);
	SetNumPackets(packets);
	SetMaximumPacketSize(f.mBytesPerPacket);

	// Anything after the audio data would be overwritten by appending to it
	SetIsOptimized(parsed.dataOffset + parsed.dataBytes + 1 >= parsed.fileSize);

	m_layout = parsed.layout;
	m_containerSizeField = parsed.containerSizeField;
	m_dataSizeField = parsed.dataSizeField;
	m_framesField = parsed.framesField;
	m_chunkOffsetField = parsed.chunkOffsetField;
	m_writable = CanWrite();

	return noErr;
}

std::vector<uint8_t> AudioFileNativeFormat::buildHeader(SInt64& dataOffset)
{
	const AudioStreamBasicDescription& f = GetDataFormat();
	const bool align = AlignDataWithFillerChunks();
	std::vector<uint8_t> h;

	// Chunks that sit before the audio data, so the filler knows how much is left to page-align it
	auto appendFiller = [&](bool bigEndian, bool caf, SInt64 trailer)
	{
		if (!align)
			return;

		const SInt64 chunkHeader = caf ? 12 : 8;
		const SInt64 free = DATA_ALIGNMENT - SInt64(h.size()) - trailer - chunkHeader;

		if (free < 0)
			return;

		if (caf)
		{
			putTag(h, "free");
			putBE64(h, uint64_t(free));
		}
		else
		{
			putTag(h, "FLLR");
			if (bigEndian)
				putBE32(h, uint32_t(free));
			else
				putLE32(h, uint32_t(free));
		}
		h.resize(h.size() + size_t(free), 0);
	};

	switch (m_fileType)
	{
		case kAudioFileWAVEType:
		{
			const bool isFloat = (f.mFormatFlags & kAudioFormatFlagIsFloat) != 0;
			UInt32 mask = 0;
			const AudioChannelLayout* layout = m_layout.empty() ? nullptr : reinterpret_cast<const AudioChannelLayout*>(m_layout.data());
			const bool extensible = layout && maskFromLayout(layout, mask);

			putTag(h, "RIFF");
			putLE32(h, 0);
			putTag(h, "WAVE");
			putTag(h, "fmt ");
			putLE32(h, extensible ? 40 : 16);
			putLE16(h, extensible ? WAVE_FORMAT_EXTENSIBLE : (isFloat ? WAVE_FORMAT_IEEE_FLOAT : WAVE_FORMAT_PCM));
			putLE16(h, uint16_t(f.mChannelsPerFrame));
			putLE32(h, uint32_t(f.mSampleRate));
			putLE32(h, uint32_t(f.mSampleRate * f.mBytesPerFrame));
			putLE16(h, uint16_t(f.mBytesPerFrame));
			putLE16(h, uint16_t(f.mBitsPerChannel));

			if (extensible)
			{
				putLE16(h, 22);
				putLE16(h, uint16_t(f.mBitsPerChannel));
				putLE32(h, mask);
				putLE16(h, isFloat ? WAVE_FORMAT_IEEE_FLOAT : WAVE_FORMAT_PCM);
				h.insert(h.end(), WAVE_GUID_TAIL, WAVE_GUID_TAIL + sizeof(WAVE_GUID_TAIL));
			}

			appendFiller(false, false, 8);
			putTag(h, "data");
			m_containerSizeField = 4;
			m_dataSizeField = SInt64(h.size());
			m_framesField = -1;
			putLE32(h, 0);
			break;
		}
		case kAudioFileAIFFType:
		case kAudioFileAIFCType:
		{
			const bool compressed = m_fileType == kAudioFileAIFCType;
			const bool isFloat = (f.mFormatFlags & kAudioFormatFlagIsFloat) != 0;
			const bool little = !(f.mFormatFlags & kAudioFormatFlagIsBigEndian) && f.mBitsPerChannel > 8;

			putTag(h, "FORM");
			putBE32(h, 0);
			putTag(h, compressed ? "AIFC" : "AIFF");

			if (compressed)
			{
				putTag(h, "FVER");
				putBE32(h, 4);
				putBE32(h, 0xA2805140);	// version 1 of the AIFF-C specification
			}

			putTag(h, "COMM");
			m_framesField = SInt64(h.size()) + 4 + 2;

			const char* type;
			std::string name;

			if (isFloat)
			{
				type = (f.mBitsPerChannel == 32) ? "fl32" : "fl64";
				name = (f.mBitsPerChannel == 32) ? "Linear PCM, 32 bit big-endian floating point" : "Linear PCM, 64 bit big-endian floating point";
			}
			else if (little)
			{
				type = "sowt";
				name = "Linear PCM, 16 bit little-endian signed integer";
			}
			else
			{
				type = (f.mBitsPerChannel == 24) ? "in24" : (f.mBitsPerChannel == 32) ? "in32" : (f.mBitsPerChannel == 8) ? "NONE" : "twos";
				name = intName(f.mBitsPerChannel);
			}

			// The name is a length-prefixed string, padded to an even size
			const size_t nameField = (1 + name.size() + 1) & ~size_t(1);

			putBE32(h, compressed ? uint32_t(18 + 4 + nameField) : 18);
			putBE16(h, uint16_t(f.mChannelsPerFrame));
			putBE32(h, 0);
			putBE16(h, uint16_t(f.mBitsPerChannel));
			writeExtended(h, f.mSampleRate);

			if (compressed)
			{
				putTag(h, type);
				h.push_back(uint8_t(name.size()));
				h.insert(h.end(), name.begin(), name.end());
				h.resize(h.size() + (nameField - 1 - name.size()), 0);
			}

			if (!m_layout.empty())
			{
				std::vector<uint8_t> payload;
				layoutToBigEndian(reinterpret_cast<const AudioChannelLayout*>(m_layout.data()), payload);

				putTag(h, "CHAN");
				putBE32(h, uint32_t(payload.size()));
				h.insert(h.end(), payload.begin(), payload.end());
				if (payload.size() & 1)
					h.push_back(0);
			}

			appendFiller(true, false, 16);
			putTag(h, "SSND");
			m_containerSizeField = 4;
			m_dataSizeField = SInt64(h.size());
			m_chunkOffsetField = 8;
			putBE32(h, 8);
			putBE32(h, 0);	// the offset of the first sample in the chunk
			putBE32(h, 0);	// the block size, which is unused
			break;
		}
		default:
		{
			putTag(h, "caff");
			putBE16(h, 1);
			putBE16(h, 0);

			putTag(h, "desc");
			putBE64(h, 32);

			uint64_t rateBits;
			const double rate = f.mSampleRate;
			memcpy(&rateBits, &rate, 8);
			putBE64(h, rateBits);

			putBE32(h, kAudioFormatLinearPCM);
			putBE32(h, ((f.mFormatFlags & kAudioFormatFlagIsFloat) ? 1 : 0) | ((f.mFormatFlags & kAudioFormatFlagIsBigEndian) ? 0 : 2));
			putBE32(h, f.mBytesPerPacket);
			putBE32(h, f.mFramesPerPacket);
			putBE32(h, f.mChannelsPerFrame);
			putBE32(h, f.mBitsPerChannel);

			if (!m_layout.empty())
			{
				std::vector<uint8_t> payload;
				layoutToBigEndian(reinterpret_cast<const AudioChannelLayout*>(m_layout.data()), payload);

				putTag(h, "chan");
				putBE64(h, payload.size());
				h.insert(h.end(), payload.begin(), payload.end());
			}

			appendFiller(true, true, 16);
			putTag(h, "data");
			m_containerSizeField = -1;
			m_dataSizeField = SInt64(h.size());
			m_framesField = -1;
			putBE64(h, 4);
			putBE32(h, 1);	// the edit count
			break;
		}
	}

	dataOffset = SInt64(h.size());
	return h;
}

OSStatus AudioFileNativeFormat::writeHeader()
{
	if (!m_native || !GetDataSource() || !CanWrite())
		return noErr;

	SInt64 dataOffset;
	std::vector<uint8_t> header = buildHeader(dataOffset);

	if (m_headerWritten && dataOffset != GetDataOffset())
	{
		// Only a file without audio data has nothing to move
		if (GetNumBytes() != 0)
			return kAudioFileOperationNotSupportedError;

		GetDataSource()->SetSize(0);
	}

	if (!writeAt(0, header.data(), UInt32(header.size())))
		return kAudioFileUnspecifiedError;

	SetDataOffset(dataOffset);

	m_headerWritten = true;
	return noErr;
}

OSStatus AudioFileNativeFormat::patchSizes(bool final)
{
	if (!m_native || !GetDataSource() || !CanWrite())
		return noErr;

	const SInt64 bytes = GetNumBytes();
	const SInt64 frames = GetNumPackets();
	SInt64 end = GetDataOffset() + bytes;
	std::vector<uint8_t> field;

	if (final && m_fileType != kAudioFileWAVEType && m_fileType != kAudioFileCAFType && (bytes & 1))
	{
		// AIFF chunks have an even size
		const uint8_t pad = 0;
		if (!writeAt(end, &pad, 1))
			return kAudioFileUnspecifiedError;
		end++;
	}

	auto putField = [&](SInt64 at, uint64_t value, int width, bool big) -> bool
	{
		if (at < 0)
			return true;

		uint8_t out[8];
		for (int i = 0; i < width; i++)
			out[i] = uint8_t(value >> (big ? 8 * (width - 1 - i) : 8 * i));
		return writeAt(at, out, UInt32(width));
	};

	bool ok = true;

	switch (m_fileType)
	{
		case kAudioFileWAVEType:
			ok = putField(m_containerSizeField, uint64_t(end - 8), 4, false) && putField(m_dataSizeField, uint64_t(bytes), 4, false);
			break;
		case kAudioFileAIFFType:
		case kAudioFileAIFCType:
			ok = putField(m_containerSizeField, uint64_t(end - 8), 4, true)
				&& putField(m_dataSizeField, uint64_t(bytes + m_chunkOffsetField), 4, true)
				&& putField(m_framesField, uint64_t(frames), 4, true);
			break;
		default:
			ok = putField(m_dataSizeField, uint64_t(bytes + 4), 8, true);
	}

	return ok ? noErr : kAudioFileUnspecifiedError;
}

OSStatus AudioFileNativeFormat::Create(CFURLRef inFileRef, const AudioStreamBasicDescription* inFormat)
{
	OSStatus status = AudioFileObject::Create(inFileRef, inFormat);
	if (status != noErr || !m_native)
		return status;

	return writeHeader();
}

OSStatus AudioFileNativeFormat::InitializeDataSource(const AudioStreamBasicDescription* inFormat, UInt32 inFlags)
{
	OSStatus status = AudioFileObject::InitializeDataSource(inFormat, inFlags);
	if (status != noErr || !m_native)
		return status;

	return writeHeader();
}

OSStatus AudioFileNativeFormat::UpdateSize()
{
	if (!m_native)
		return AudioFileObject::UpdateSize();
	return patchSizes(false);
}

OSStatus AudioFileNativeFormat::Close()
{
	if (!m_native)
		return AudioFileAVFormat::Close();

	OSStatus status = noErr;

	if (GetDataSource() && CanWrite())
	{
		status = patchSizes(true);
		SetNeedsSizeUpdate(false);
	}

	OSStatus closeStatus = AudioFileObject::Close();
	return (status != noErr) ? status : closeStatus;
}

OSStatus AudioFileNativeFormat::IsValidFilePosition(SInt64 position)
{
	// The sizes in these headers are 32 bits wide
	if (m_native && m_fileType != kAudioFileCAFType && position > 0xFFFFFFF0LL)
		return kAudioFileOperationNotSupportedError;
	return noErr;
}

OSStatus AudioFileNativeFormat::WriteBytes(Boolean inUseCache, SInt64 inStartingByte, UInt32* ioNumBytes, const void* inBuffer)
{
	if (!m_native)
		return AudioFileAVFormat::WriteBytes(inUseCache, inStartingByte, ioNumBytes, inBuffer);
	return AudioFileObject::WriteBytes(inUseCache, inStartingByte, ioNumBytes, inBuffer);
}

OSStatus AudioFileNativeFormat::WritePackets(Boolean inUseCache, UInt32 inNumBytes, const AudioStreamPacketDescription* inPacketDescriptions,
	SInt64 inStartingPacket, UInt32* ioNumPackets, const void* inBuffer)
{
	if (!m_native)
		return AudioFileAVFormat::WritePackets(inUseCache, inNumBytes, inPacketDescriptions, inStartingPacket, ioNumPackets, inBuffer);
	return AudioFileObject::WritePackets(inUseCache, inNumBytes, inPacketDescriptions, inStartingPacket, ioNumPackets, inBuffer);
}

OSStatus AudioFileNativeFormat::FrameToPacket(SInt64 inFrame, SInt64& outPacket, UInt32& outFrameOffsetInPacket)
{
	if (!m_native)
		return AudioFileAVFormat::FrameToPacket(inFrame, outPacket, outFrameOffsetInPacket);
	return AudioFileObject::FrameToPacket(inFrame, outPacket, outFrameOffsetInPacket);
}

OSStatus AudioFileNativeFormat::GetMagicCookieDataSize(UInt32* outDataSize, UInt32* isWritable)
{
	if (!m_native)
		return AudioFileAVFormat::GetMagicCookieDataSize(outDataSize, isWritable);
	return AudioFileObject::GetMagicCookieDataSize(outDataSize, isWritable);
}

OSStatus AudioFileNativeFormat::GetMagicCookieData(UInt32* ioDataSize, void* ioPropertyData)
{
	if (!m_native)
		return AudioFileAVFormat::GetMagicCookieData(ioDataSize, ioPropertyData);
	return AudioFileObject::GetMagicCookieData(ioDataSize, ioPropertyData);
}

OSStatus AudioFileNativeFormat::SetMagicCookieData(UInt32 inDataSize, const void* inPropertyData)
{
	if (!m_native)
		return AudioFileAVFormat::SetMagicCookieData(inDataSize, inPropertyData);
	return AudioFileObject::SetMagicCookieData(inDataSize, inPropertyData);
}

OSStatus AudioFileNativeFormat::GetChannelLayoutSize(UInt32* outDataSize, UInt32* isWritable)
{
	if (!m_native)
		return AudioFileAVFormat::GetChannelLayoutSize(outDataSize, isWritable);

	// QUIRK: a file without a channel layout reports a bad property size for the layout,
	// with a size of 0, rather than an unsupported property.
	// Evidence: black-box observation on macOS 26.7.3 (tier 2), for WAVE, AIFF and CAF files.
	// Test: test_no_channel_layout (tests/src/audiofile_pcm.c)
	if (m_layout.empty())
	{
		if (outDataSize)
			*outDataSize = 0;
		if (isWritable)
			*isWritable = CanWrite() ? 1 : 0;
		return kAudioFileBadPropertySizeError;
	}

	// A layout read from a WAVE file is a whole AudioChannelLayout, the others only as long as stored
	if (outDataSize)
		*outDataSize = UInt32(m_layout.size());
	if (isWritable)
		*isWritable = CanWrite() ? 1 : 0;
	return noErr;
}

OSStatus AudioFileNativeFormat::GetChannelLayout(UInt32* ioDataSize, AudioChannelLayout* ioChannelLayout)
{
	if (!m_native)
		return AudioFileAVFormat::GetChannelLayout(ioDataSize, ioChannelLayout);

	if (m_layout.empty())
	{
		*ioDataSize = 0;
		return kAudioFileBadPropertySizeError;
	}

	*ioDataSize = std::min<UInt32>(*ioDataSize, UInt32(m_layout.size()));
	memcpy(ioChannelLayout, m_layout.data(), *ioDataSize);
	return noErr;
}

OSStatus AudioFileNativeFormat::SetChannelLayout(UInt32 inDataSize, const AudioChannelLayout* inChannelLayout)
{
	if (!m_native)
		return AudioFileAVFormat::SetChannelLayout(inDataSize, inChannelLayout);

	if (!CanWrite())
		return kAudioFilePermissionsError;
	if (inDataSize < LAYOUT_HEADER || inDataSize < layoutSize(inChannelLayout))
		return kAudioFileBadPropertySizeError;

	std::vector<uint8_t> previous = m_layout;

	if (m_fileType == kAudioFileWAVEType)
	{
		UInt32 mask;

		if (!maskFromLayout(inChannelLayout, mask))
			return kAudioFileUnsupportedPropertyError;

		m_layout = layoutFromMask(mask);
	}
	else
		m_layout.assign(reinterpret_cast<const uint8_t*>(inChannelLayout), reinterpret_cast<const uint8_t*>(inChannelLayout) + layoutSize(inChannelLayout));

	// A new file has its header written by now, and the layout is part of it
	if (m_headerWritten)
	{
		OSStatus status = writeHeader();

		if (status != noErr)
		{
			m_layout = previous;
			return status;
		}
	}

	return noErr;
}
