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

#ifndef AUDIOTOOLBOX_PCMCONVERTER_H
#define AUDIOTOOLBOX_PCMCONVERTER_H
#include "AudioConverter.h"
#include <cstdint>
#include <memory>
#include <vector>

class PCMResampler;

// Converts between linear PCM formats without any codec library: sample format,
// byte order, packing and alignment, interleaving, channel count (optionally with
// a channel map) and sample rate.
class __attribute__((visibility("hidden"))) PCMConverter
{
public:
	// Returns noErr if the converter can be created for these formats, an
	// AudioConverter error otherwise. Both formats must be linear PCM.
	static OSStatus validate(const AudioStreamBasicDescription* src, const AudioStreamBasicDescription* dst);

	PCMConverter(const AudioStreamBasicDescription* src, const AudioStreamBasicDescription* dst);
	~PCMConverter();

	void reset();

	OSStatus fillComplex(AudioConverterRef owner, AudioConverterComplexInputDataProc dataProc, void* opaque,
		UInt32* ioOutputDataPacketSize, AudioBufferList* outOutputData, AudioStreamPacketDescription* outPacketDescription);
	OSStatus convertBuffer(UInt32 inInputDataSize, const void* inInputData, UInt32* ioOutputDataSize, void* outOutputData);
	OSStatus convertComplexBuffer(UInt32 inNumberPCMFrames, const AudioBufferList* inInputData, AudioBufferList* outOutputData);

	OSStatus getPropertyInfo(AudioConverterPropertyID id, UInt32* outSize, Boolean* outWritable);
	OSStatus getProperty(AudioConverterPropertyID id, UInt32* ioSize, void* out);
	OSStatus setProperty(AudioConverterPropertyID id, UInt32 size, const void* in);

	UInt32 sourceFrameSize() const { return m_src.asbd.mBytesPerFrame; }

private:
	enum class Kind { Int, Float };

	struct Layout
	{
		AudioStreamBasicDescription asbd;
		Kind kind;
		UInt32 channels;
		UInt32 bits;			// significant bits of one sample
		UInt32 container;		// bytes per sample in memory
		UInt32 frameBytes;		// bytes of one frame in one buffer
		UInt32 buffers;			// 1 if interleaved, else one per channel
		bool isSigned;
		bool bigEndian;
		bool alignedHigh;
	};

	static OSStatus describe(const AudioStreamBasicDescription* asbd, Layout& out);

	static uint8_t* sample(const Layout& l, const std::vector<uint8_t*>& base, UInt32 frame, UInt32 channel);

	static double readNormalized(const Layout& l, const uint8_t* p);
	static int64_t readInt(const Layout& l, const uint8_t* p);
	static void writeInt(const Layout& l, uint8_t* p, int64_t value);
	static void writeNormalized(const Layout& l, uint8_t* p, double value);
	static void writeSilence(const Layout& l, uint8_t* p);
	void convertOne(const uint8_t* in, uint8_t* out) const;

	void rebuildMap();
	void convertFrames(const std::vector<uint8_t*>& in, UInt32 inFrame, std::vector<uint8_t*>& out, UInt32 outFrame, UInt32 frames) const;
	OSStatus prepareOutput(AudioBufferList* list, std::vector<uint8_t*>& base, UInt32& capacityFrames) const;
	static void finishOutput(const Layout& l, AudioBufferList* list, UInt32 frames);
	OSStatus fillWithResampling(AudioConverterRef owner, AudioConverterComplexInputDataProc proc, void* opaque,
		std::vector<uint8_t*>& out, UInt32 capacity, UInt32& produced);

private:
	Layout m_src, m_dst;
	std::vector<SInt32> m_channelMap;	// per destination channel, the source channel or -1 for silence
	bool m_customMap = false;
	bool m_resample = false;
	bool m_flushing = false;
	UInt32 m_quality = kAudioConverterQuality_Medium;
	UInt32 m_complexity = kAudioConverterSampleRateConverterComplexity_Normal;
	UInt32 m_primeMethod = kConverterPrimeMethod_Normal;
	std::unique_ptr<PCMResampler> m_resampler;
};

#endif
