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

#include "PCMConverter.h"
#include <CarbonCore/MacErrors.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstring>

// The conversion rules below follow the public AudioConverter documentation. Where it
// is silent, the behaviour that was observed on macOS is matched and marked QUIRK.
// The tests are in tests/src/audioconverter_pcm.c.

namespace {

const double kPi = 3.14159265358979323846;

double bessel0(double x)
{
	double sum = 1, term = 1;
	const double q = x * x / 4;

	for (int k = 1; k < 60; k++)
	{
		term *= q / (double(k) * k);
		sum += term;
		if (term < sum * 1e-17)
			break;
	}
	return sum;
}

inline uint32_t swap32(uint32_t v) { return __builtin_bswap32(v); }
inline uint64_t swap64(uint64_t v) { return __builtin_bswap64(v); }

}

// A windowed-sinc resampler for any rate ratio. The kernel is a Kaiser-windowed sinc whose
// cutoff is lowered when reducing the rate, so that nothing above the new Nyquist rate folds back.
class __attribute__((visibility("hidden"))) PCMResampler
{
public:
	PCMResampler(double inRate, double outRate, UInt32 channels, UInt32 quality)
	: m_step(inRate / outRate), m_ratio(outRate / inRate), m_channels(channels), m_buffers(channels)
	{
		m_cutoff = std::min(1.0, m_ratio);

		// Half width of the kernel in units of (scaled) input samples
		if (quality >= kAudioConverterQuality_Max)
			m_lobes = 32;
		else if (quality >= kAudioConverterQuality_High)
			m_lobes = 24;
		else if (quality >= kAudioConverterQuality_Medium)
			m_lobes = 16;
		else if (quality >= kAudioConverterQuality_Low)
			m_lobes = 12;
		else
			m_lobes = 8;

		const double beta = (quality >= kAudioConverterQuality_High) ? 10.0 : 8.0;
		const double norm = bessel0(beta);

		m_table.resize(size_t(m_lobes) * RESOLUTION + 2, 0.0);
		for (size_t i = 0; i <= size_t(m_lobes) * RESOLUTION; i++)
		{
			const double u = double(i) / RESOLUTION;
			const double sinc = (i == 0) ? 1.0 : std::sin(kPi * u) / (kPi * u);
			const double r = u / m_lobes;
			m_table[i] = sinc * bessel0(beta * std::sqrt(std::max(0.0, 1.0 - r * r))) / norm;
		}

		m_span = m_lobes / m_cutoff;
	}

	void append(UInt32 channel, const double* data, UInt32 frames)
	{
		m_buffers[channel].insert(m_buffers[channel].end(), data, data + frames);
	}

	void inputAppended(UInt32 frames) { m_inputFrames += frames; }

	// Produces up to maxFrames, in planar form. Without flushing, output that would need
	// input that has not arrived yet waits. When flushing, the end of the stream is silence and
	// the output stops at the length implied by the input length.
	UInt32 produce(std::vector<std::vector<double>>& out, UInt32 maxFrames, bool flushing)
	{
		UInt32 produced = 0;

		for (auto& channel : out)
			channel.clear();

		const int64_t target = int64_t(std::llround(double(m_inputFrames) * m_ratio));

		while (produced < maxFrames)
		{
			if (flushing && m_outputFrames >= target)
				break;

			const double position = double(m_outputFrames) * m_step;
			const int64_t first = int64_t(std::ceil(position - m_span));
			const int64_t last = int64_t(std::floor(position + m_span));

			if (!flushing && last >= m_base + int64_t(m_buffers[0].size()))
				break;

			for (UInt32 c = 0; c < m_channels; c++)
				out[c].push_back(evaluate(c, position, first, last));

			m_outputFrames++;
			produced++;
		}

		trim();
		return produced;
	}

	void reset()
	{
		for (auto& buffer : m_buffers)
			buffer.clear();
		m_base = 0;
		m_inputFrames = 0;
		m_outputFrames = 0;
	}

	double inputPerOutput() const { return m_step; }

private:
	double evaluate(UInt32 channel, double position, int64_t first, int64_t last) const
	{
		const std::vector<double>& buffer = m_buffers[channel];
		const int64_t end = m_base + int64_t(buffer.size());
		double sum = 0, weights = 0;

		for (int64_t n = first; n <= last; n++)
		{
			const double u = std::fabs((position - double(n)) * m_cutoff);
			if (u >= m_lobes)
				continue;

			const double index = u * RESOLUTION;
			const size_t i = size_t(index);
			const double fraction = index - double(i);
			const double w = m_table[i] + (m_table[i + 1] - m_table[i]) * fraction;

			weights += w;
			if (n >= m_base && n < end)
				sum += w * buffer[size_t(n - m_base)];
		}

		// Unity gain at DC however the kernel falls between the samples
		return (weights != 0) ? sum / weights : 0.0;
	}

	void trim()
	{
		const double position = double(m_outputFrames) * m_step;
		const int64_t keepFrom = int64_t(std::ceil(position - m_span)) - 1;

		if (keepFrom <= m_base)
			return;

		const int64_t drop = std::min<int64_t>(keepFrom - m_base, int64_t(m_buffers[0].size()));
		for (auto& buffer : m_buffers)
			buffer.erase(buffer.begin(), buffer.begin() + drop);
		m_base += drop;
	}

private:
	static const int RESOLUTION = 512;

	double m_step, m_ratio, m_cutoff = 1, m_span = 0;
	UInt32 m_channels;
	UInt32 m_lobes = 16;
	std::vector<double> m_table;
	std::vector<std::vector<double>> m_buffers;
	int64_t m_base = 0;			// index of the first buffered input frame
	int64_t m_inputFrames = 0;
	int64_t m_outputFrames = 0;
};

OSStatus PCMConverter::describe(const AudioStreamBasicDescription* asbd, Layout& l)
{
	if (asbd->mFormatID != kAudioFormatLinearPCM)
		return kAudioConverterErr_FormatNotSupported;

	if (asbd->mChannelsPerFrame == 0)
		return paramErr;

	const bool isFloat = (asbd->mFormatFlags & kAudioFormatFlagIsFloat) != 0;
	const UInt32 bits = asbd->mBitsPerChannel;

	// Checked before the sizes: a format of an unsupported depth is a format error, not a bad parameter
	if (isFloat ? (bits != 32 && bits != 64) : (bits != 8 && bits != 16 && bits != 24 && bits != 32))
		return kAudioConverterErr_FormatNotSupported;

	l.asbd = *asbd;
	l.kind = isFloat ? Kind::Float : Kind::Int;
	l.channels = asbd->mChannelsPerFrame;
	l.bits = bits;
	l.bigEndian = (asbd->mFormatFlags & kAudioFormatFlagIsBigEndian) != 0;
	l.alignedHigh = (asbd->mFormatFlags & kAudioFormatFlagIsAlignedHigh) != 0;
	l.isSigned = isFloat || (asbd->mFormatFlags & kAudioFormatFlagIsSignedInteger) != 0;

	const bool interleaved = !(asbd->mFormatFlags & kAudioFormatFlagIsNonInterleaved);

	if (asbd->mFramesPerPacket != 1 || asbd->mBytesPerPacket != asbd->mBytesPerFrame || asbd->mBytesPerFrame == 0)
		return paramErr;

	if (interleaved)
	{
		if (asbd->mBytesPerFrame % l.channels != 0)
			return paramErr;
		l.container = asbd->mBytesPerFrame / l.channels;
	}
	else
		l.container = asbd->mBytesPerFrame;

	l.frameBytes = asbd->mBytesPerFrame;
	l.buffers = interleaved ? 1 : l.channels;

	if (l.container * 8 < bits || l.container > 8 || (!isFloat && l.container > 4))
		return paramErr;

	// A packed format has no room around its samples
	if ((asbd->mFormatFlags & kAudioFormatFlagIsPacked) && l.container * 8 != bits)
		return paramErr;

	if (isFloat && l.container * 8 != bits)
		return paramErr;

	return noErr;
}

OSStatus PCMConverter::validate(const AudioStreamBasicDescription* src, const AudioStreamBasicDescription* dst)
{
	Layout a, b;
	OSStatus status = describe(src, a);
	if (status != noErr)
		return status;
	return describe(dst, b);
}

PCMConverter::PCMConverter(const AudioStreamBasicDescription* src, const AudioStreamBasicDescription* dst)
{
	describe(src, m_src);
	describe(dst, m_dst);

	// A sample rate of zero means it is not specified, so nothing is converted
	m_resample = src->mSampleRate > 0 && dst->mSampleRate > 0 && src->mSampleRate != dst->mSampleRate;

	rebuildMap();
}

PCMConverter::~PCMConverter() = default;

void PCMConverter::reset()
{
	m_flushing = false;
	if (m_resampler)
		m_resampler->reset();
}

void PCMConverter::rebuildMap()
{
	if (m_customMap && m_channelMap.size() == m_dst.channels)
		return;

	m_channelMap.assign(m_dst.channels, -1);

	// QUIRK: without a channel map or layouts, a mono source is copied to both channels of a stereo
	// destination, and every other difference in channel count keeps the leading channels and
	// fills the rest with silence (so stereo to mono keeps the left channel only; it does not mix).
	// Evidence: black-box observation on macOS 26.7.3 (tier 2).
	// Test: test_quirk_channel_count_changes (tests/src/audioconverter_pcm.c)
	for (UInt32 c = 0; c < m_dst.channels; c++)
	{
		if (m_src.channels == 1 && m_dst.channels == 2)
			m_channelMap[c] = 0;
		else if (c < m_src.channels)
			m_channelMap[c] = SInt32(c);
	}
}

uint8_t* PCMConverter::sample(const Layout& l, const std::vector<uint8_t*>& base, UInt32 frame, UInt32 channel)
{
	if (l.buffers == 1)
		return base[0] + size_t(frame) * l.frameBytes + size_t(channel) * l.container;
	return base[channel] + size_t(frame) * l.container;
}

int64_t PCMConverter::readInt(const Layout& l, const uint8_t* p)
{
	uint64_t raw = 0;

	if (l.bigEndian)
	{
		for (UInt32 i = 0; i < l.container; i++)
			raw = (raw << 8) | p[i];
	}
	else
	{
		for (UInt32 i = l.container; i > 0; i--)
			raw = (raw << 8) | p[i - 1];
	}

	if (l.alignedHigh)
		raw >>= (l.container * 8 - l.bits);

	if (l.bits < 64)
		raw &= (uint64_t(1) << l.bits) - 1;

	const int64_t half = int64_t(1) << (l.bits - 1);

	if (l.isSigned)
		return int64_t(raw ^ uint64_t(half)) - half;

	return int64_t(raw) - half;
}

void PCMConverter::writeInt(const Layout& l, uint8_t* p, int64_t value)
{
	const int64_t half = int64_t(1) << (l.bits - 1);
	uint64_t raw = uint64_t(l.isSigned ? value : value + half);

	raw &= (uint64_t(1) << l.bits) - 1;

	if (l.alignedHigh)
		raw <<= (l.container * 8 - l.bits);

	if (l.bigEndian)
	{
		for (UInt32 i = l.container; i > 0; i--)
		{
			p[i - 1] = uint8_t(raw);
			raw >>= 8;
		}
	}
	else
	{
		for (UInt32 i = 0; i < l.container; i++)
		{
			p[i] = uint8_t(raw);
			raw >>= 8;
		}
	}
}

double PCMConverter::readNormalized(const Layout& l, const uint8_t* p)
{
	if (l.kind == Kind::Float)
	{
		if (l.bits == 32)
		{
			uint32_t raw;
			memcpy(&raw, p, 4);
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
			if (l.bigEndian)
#else
			if (!l.bigEndian)
#endif
				raw = swap32(raw);
			float f;
			memcpy(&f, &raw, 4);
			return f;
		}

		uint64_t raw;
		memcpy(&raw, p, 8);
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		if (l.bigEndian)
#else
		if (!l.bigEndian)
#endif
			raw = swap64(raw);
		double d;
		memcpy(&d, &raw, 8);
		return d;
	}

	return double(readInt(l, p)) / std::ldexp(1.0, int(l.bits) - 1);
}

// Rounds a normalized value (full scale is +-1.0) to an integer sample, clipping at the ends
static int64_t floatToInt(double x, UInt32 bits)
{
	const int64_t lo = -(int64_t(1) << (bits - 1));
	int64_t hi = (int64_t(1) << (bits - 1)) - 1;

	// QUIRK: converting floating-point to 32-bit integers clips at 0x7fffff80, the largest
	// value below 2^31 that a 32-bit float can hold, instead of at 0x7fffffff.
	// Evidence: black-box observation on macOS 26.7.3 (tier 2), for 32-bit and 64-bit float input.
	// Test: test_quirk_float_to_int32_clip (tests/src/audioconverter_pcm.c)
	if (bits == 32)
		hi = 2147483520;

	// QUIRK: NaN becomes the most negative sample.
	// Evidence: black-box observation on macOS 26.7.3 (tier 2).
	// Test: test_quirk_float_nan_inf (tests/src/audioconverter_pcm.c)
	if (std::isnan(x))
		return lo;

	const double scaled = x * std::ldexp(1.0, int(bits) - 1);
	double rounded;

	// Halves round up for 16, 24 and 32-bit output.
	// QUIRK: 8-bit output rounds halves away from zero below -0.5 (so -1.5 gives -2 and
	// -2.5 gives -3), while -0.5 gives 0 and the positive halves round up.
	// Evidence: black-box observation on macOS 26.7.3 (tier 2).
	// Test: test_quirk_float_to_int8_rounding (tests/src/audioconverter_pcm.c)
	if (bits == 8 && scaled < -0.5)
		rounded = std::ceil(scaled - 0.5);
	else
		rounded = std::floor(scaled + 0.5);

	if (rounded >= double(hi))
		return hi;
	if (rounded <= double(lo))
		return lo;
	return int64_t(rounded);
}

void PCMConverter::writeNormalized(const Layout& l, uint8_t* p, double value)
{
	if (l.kind == Kind::Float)
	{
		if (l.bits == 32)
		{
			const float f = float(value);
			uint32_t raw;
			memcpy(&raw, &f, 4);
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
			if (l.bigEndian)
#else
			if (!l.bigEndian)
#endif
				raw = swap32(raw);
			memcpy(p, &raw, 4);
		}
		else
		{
			uint64_t raw;
			memcpy(&raw, &value, 8);
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
			if (l.bigEndian)
#else
			if (!l.bigEndian)
#endif
				raw = swap64(raw);
			memcpy(p, &raw, 8);
		}
		return;
	}

	writeInt(l, p, floatToInt(value, l.bits));
}

void PCMConverter::writeSilence(const Layout& l, uint8_t* p)
{
	if (l.kind == Kind::Float)
		writeNormalized(l, p, 0.0);
	else
		writeInt(l, p, 0);
}

void PCMConverter::convertOne(const uint8_t* in, uint8_t* out) const
{
	if (m_src.kind == Kind::Float || m_dst.kind == Kind::Float)
	{
		writeNormalized(m_dst, out, readNormalized(m_src, in));
		return;
	}

	const int64_t v = readInt(m_src, in);
	const UInt32 sb = m_src.bits, db = m_dst.bits;

	if (db >= sb)
	{
		writeInt(m_dst, out, v * (int64_t(1) << (db - sb)));
		return;
	}

	const UInt32 shift = sb - db;

	// QUIRK: reducing 16-bit integers to 8 bits drops the low byte (rounding towards minus infinity)
	// instead of rounding, while the other reductions round halves up.
	// Evidence: black-box observation on macOS 26.7.3 (tier 2).
	// Test: test_quirk_int_narrowing (tests/src/audioconverter_pcm.c)
	if (db == 8 && sb == 16)
	{
		writeInt(m_dst, out, v >> shift);
		return;
	}

	// QUIRK: 24 and 32-bit integers reduced to 8 bits go through a 32-bit float on the way, which
	// loses the low bits of 32-bit input and rounds like float to 8-bit output.
	// Evidence: black-box observation on macOS 26.7.3 (tier 2).
	// Test: test_quirk_int_narrowing (tests/src/audioconverter_pcm.c)
	if (db == 8)
	{
		const double f = double(float(v)) / std::ldexp(1.0, int(sb) - 1);
		writeInt(m_dst, out, floatToInt(f, 8));
		return;
	}

	const int64_t rounded = (v + (int64_t(1) << (shift - 1))) >> shift;
	const int64_t hi = (int64_t(1) << (db - 1)) - 1;

	writeInt(m_dst, out, std::max(-hi - 1, std::min(hi, rounded)));
}

void PCMConverter::convertFrames(const std::vector<uint8_t*>& in, UInt32 inFrame, std::vector<uint8_t*>& out, UInt32 outFrame, UInt32 frames) const
{
	for (UInt32 f = 0; f < frames; f++)
	{
		for (UInt32 c = 0; c < m_dst.channels; c++)
		{
			uint8_t* dest = sample(m_dst, out, outFrame + f, c);
			const SInt32 source = m_channelMap[c];

			if (source < 0 || UInt32(source) >= m_src.channels)
				writeSilence(m_dst, dest);
			else
				convertOne(sample(m_src, in, inFrame + f, UInt32(source)), dest);
		}
	}
}

OSStatus PCMConverter::prepareOutput(AudioBufferList* list, std::vector<uint8_t*>& base, UInt32& capacity) const
{
	if (!list || list->mNumberBuffers < m_dst.buffers)
		return paramErr;

	base.clear();
	capacity = UINT32_MAX;

	const UInt32 unit = (m_dst.buffers == 1) ? m_dst.frameBytes : m_dst.container;

	for (UInt32 i = 0; i < m_dst.buffers; i++)
	{
		if (!list->mBuffers[i].mData)
			return paramErr;
		base.push_back(static_cast<uint8_t*>(list->mBuffers[i].mData));
		capacity = std::min<UInt32>(capacity, list->mBuffers[i].mDataByteSize / unit);
	}

	return noErr;
}

void PCMConverter::finishOutput(const Layout& l, AudioBufferList* list, UInt32 frames)
{
	const UInt32 unit = (l.buffers == 1) ? l.frameBytes : l.container;

	for (UInt32 i = 0; i < l.buffers; i++)
		list->mBuffers[i].mDataByteSize = frames * unit;
}

namespace {

struct InputList
{
	std::vector<uint8_t> storage;

	AudioBufferList* list() { return reinterpret_cast<AudioBufferList*>(storage.data()); }
};

}

OSStatus PCMConverter::fillComplex(AudioConverterRef owner, AudioConverterComplexInputDataProc dataProc, void* opaque,
	UInt32* ioOutputDataPacketSize, AudioBufferList* outOutputData, AudioStreamPacketDescription*)
{
	if (!dataProc || !ioOutputDataPacketSize || !outOutputData)
		return paramErr;

	std::vector<uint8_t*> out;
	UInt32 capacity;
	OSStatus status = prepareOutput(outOutputData, out, capacity);
	if (status != noErr)
		return status;

	capacity = std::min(capacity, *ioOutputDataPacketSize);

	UInt32 produced = 0;

	if (m_resample)
		status = fillWithResampling(owner, dataProc, opaque, out, capacity, produced);
	else
	{
		InputList input;
		input.storage.resize(offsetof(AudioBufferList, mBuffers) + m_src.buffers * sizeof(AudioBuffer));

		while (produced < capacity)
		{
			AudioBufferList* list = input.list();
			UInt32 wanted = capacity - produced;

			list->mNumberBuffers = m_src.buffers;
			for (UInt32 i = 0; i < m_src.buffers; i++)
			{
				list->mBuffers[i].mNumberChannels = (m_src.buffers == 1) ? m_src.channels : 1;
				list->mBuffers[i].mDataByteSize = 0;
				list->mBuffers[i].mData = nullptr;
			}

			status = dataProc(owner, &wanted, list, nullptr, opaque);
			if (status != noErr || wanted == 0)
				break;

			const UInt32 unit = (m_src.buffers == 1) ? m_src.frameBytes : m_src.container;
			std::vector<uint8_t*> in;

			if (list->mNumberBuffers < m_src.buffers)
			{
				status = paramErr;
				break;
			}

			UInt32 got = std::min(wanted, capacity - produced);
			for (UInt32 i = 0; i < m_src.buffers; i++)
			{
				if (!list->mBuffers[i].mData)
				{
					status = paramErr;
					break;
				}
				in.push_back(static_cast<uint8_t*>(list->mBuffers[i].mData));
				got = std::min<UInt32>(got, list->mBuffers[i].mDataByteSize / unit);
			}
			if (status != noErr)
				break;

			convertFrames(in, 0, out, produced, got);
			produced += got;

			// An input that cannot give any frames would never let this finish
			if (got == 0)
				break;
		}
	}

	*ioOutputDataPacketSize = produced;
	finishOutput(m_dst, outOutputData, produced);
	return status;
}

OSStatus PCMConverter::fillWithResampling(AudioConverterRef owner, AudioConverterComplexInputDataProc proc, void* opaque,
	std::vector<uint8_t*>& out, UInt32 capacity, UInt32& produced)
{
	if (!m_resampler)
		m_resampler.reset(new PCMResampler(m_src.asbd.mSampleRate, m_dst.asbd.mSampleRate, m_dst.channels, m_quality));

	std::vector<std::vector<double>> planar(m_dst.channels);
	InputList input;
	input.storage.resize(offsetof(AudioBufferList, mBuffers) + m_src.buffers * sizeof(AudioBuffer));
	std::vector<double> mapped;

	while (produced < capacity)
	{
		const UInt32 n = m_resampler->produce(planar, capacity - produced, m_flushing);

		for (UInt32 f = 0; f < n; f++)
		{
			for (UInt32 c = 0; c < m_dst.channels; c++)
				writeNormalized(m_dst, sample(m_dst, out, produced + f, c), planar[c][f]);
		}
		produced += n;

		if (produced == capacity)
			break;

		if (m_flushing)
		{
			// The whole stream has come out, and what follows is a new one
			m_resampler->reset();
			m_flushing = false;
			break;
		}

		AudioBufferList* list = input.list();
		UInt32 wanted = std::min<UInt32>(4096, UInt32((capacity - produced) * m_resampler->inputPerOutput()) + 16);

		list->mNumberBuffers = m_src.buffers;
		for (UInt32 i = 0; i < m_src.buffers; i++)
		{
			list->mBuffers[i].mNumberChannels = (m_src.buffers == 1) ? m_src.channels : 1;
			list->mBuffers[i].mDataByteSize = 0;
			list->mBuffers[i].mData = nullptr;
		}

		OSStatus status = proc(owner, &wanted, list, nullptr, opaque);
		if (status != noErr)
			return status;

		if (wanted == 0)
		{
			m_flushing = true;
			continue;
		}

		if (list->mNumberBuffers < m_src.buffers)
			return paramErr;

		const UInt32 unit = (m_src.buffers == 1) ? m_src.frameBytes : m_src.container;
		std::vector<uint8_t*> in;
		UInt32 got = wanted;

		for (UInt32 i = 0; i < m_src.buffers; i++)
		{
			if (!list->mBuffers[i].mData)
				return paramErr;
			in.push_back(static_cast<uint8_t*>(list->mBuffers[i].mData));
			got = std::min<UInt32>(got, list->mBuffers[i].mDataByteSize / unit);
		}

		if (got == 0)
			return noErr;

		for (UInt32 c = 0; c < m_dst.channels; c++)
		{
			const SInt32 source = m_channelMap[c];

			mapped.assign(got, 0.0);
			if (source >= 0 && UInt32(source) < m_src.channels)
			{
				for (UInt32 f = 0; f < got; f++)
					mapped[f] = readNormalized(m_src, sample(m_src, in, f, UInt32(source)));
			}
			m_resampler->append(c, mapped.data(), got);
		}
		m_resampler->inputAppended(got);
	}

	return noErr;
}

OSStatus PCMConverter::convertBuffer(UInt32 inInputDataSize, const void* inInputData, UInt32* ioOutputDataSize, void* outOutputData)
{
	if (!inInputData || !ioOutputDataSize || !outOutputData || m_resample || m_src.buffers != 1 || m_dst.buffers != 1)
		return paramErr;

	const UInt32 frames = inInputDataSize / m_src.frameBytes;

	if (*ioOutputDataSize < frames * m_dst.frameBytes)
		return kAudioConverterErr_InvalidInputSize;

	std::vector<uint8_t*> in{ const_cast<uint8_t*>(static_cast<const uint8_t*>(inInputData)) };
	std::vector<uint8_t*> out{ static_cast<uint8_t*>(outOutputData) };

	convertFrames(in, 0, out, 0, frames);
	*ioOutputDataSize = frames * m_dst.frameBytes;
	return noErr;
}

OSStatus PCMConverter::convertComplexBuffer(UInt32 inNumberPCMFrames, const AudioBufferList* inInputData, AudioBufferList* outOutputData)
{
	if (!inInputData || !outOutputData || m_resample || inInputData->mNumberBuffers < m_src.buffers)
		return paramErr;

	std::vector<uint8_t*> in, out;
	UInt32 capacity;
	const UInt32 unit = (m_src.buffers == 1) ? m_src.frameBytes : m_src.container;

	for (UInt32 i = 0; i < m_src.buffers; i++)
	{
		if (!inInputData->mBuffers[i].mData || inInputData->mBuffers[i].mDataByteSize / unit < inNumberPCMFrames)
			return kAudioConverterErr_InvalidInputSize;
		in.push_back(static_cast<uint8_t*>(inInputData->mBuffers[i].mData));
	}

	OSStatus status = prepareOutput(outOutputData, out, capacity);
	if (status != noErr)
		return status;
	if (capacity < inNumberPCMFrames)
		return kAudioConverterErr_InvalidOutputSize;

	convertFrames(in, 0, out, 0, inNumberPCMFrames);
	finishOutput(m_dst, outOutputData, inNumberPCMFrames);
	return noErr;
}

OSStatus PCMConverter::getPropertyInfo(AudioConverterPropertyID id, UInt32* outSize, Boolean* outWritable)
{
	UInt32 size = sizeof(UInt32);
	Boolean writable = false;

	switch (id)
	{
		case kAudioConverterPropertyMinimumInputBufferSize:
		case kAudioConverterPropertyMaximumInputBufferSize:
		case kAudioConverterPropertyMaximumInputPacketSize:
		case kAudioConverterPropertyMaximumOutputPacketSize:
		case kAudioConverterPropertyCalculateInputBufferSize:
		case kAudioConverterPropertyCalculateOutputBufferSize:
			break;
		case kAudioConverterSampleRateConverterQuality:
		case kAudioConverterSampleRateConverterComplexity:
		case kAudioConverterPrimeMethod:
			writable = true;
			break;
		case kAudioConverterPrimeInfo:
			size = sizeof(AudioConverterPrimeInfo);
			break;
		case kAudioConverterChannelMap:
			size = m_dst.channels * sizeof(SInt32);
			writable = true;
			break;
		case kAudioConverterCurrentInputStreamDescription:
		case kAudioConverterCurrentOutputStreamDescription:
			size = sizeof(AudioStreamBasicDescription);
			break;
		default:
			return kAudioConverterErr_PropertyNotSupported;
	}

	if (outSize)
		*outSize = size;
	if (outWritable)
		*outWritable = writable;
	return noErr;
}

OSStatus PCMConverter::getProperty(AudioConverterPropertyID id, UInt32* ioSize, void* out)
{
	UInt32 needed;
	Boolean writable;
	OSStatus status = getPropertyInfo(id, &needed, &writable);

	if (status != noErr)
		return status;
	if (!ioSize || !out || *ioSize < needed)
		return kAudioConverterErr_BadPropertySizeError;

	*ioSize = needed;
	UInt32* word = static_cast<UInt32*>(out);

	switch (id)
	{
		case kAudioConverterPropertyMinimumInputBufferSize:
		case kAudioConverterPropertyMaximumInputPacketSize:
			*word = m_src.container * (m_src.buffers == 1 ? m_src.channels : 1);
			break;
		case kAudioConverterPropertyMaximumInputBufferSize:
			*word = 0xFFFFFFFF;
			break;
		case kAudioConverterPropertyMaximumOutputPacketSize:
			*word = m_dst.container * (m_dst.buffers == 1 ? m_dst.channels : 1);
			break;
		case kAudioConverterSampleRateConverterQuality:
			*word = m_quality;
			break;
		case kAudioConverterSampleRateConverterComplexity:
			*word = m_complexity;
			break;
		case kAudioConverterPrimeMethod:
			*word = m_primeMethod;
			break;
		case kAudioConverterPrimeInfo:
			memset(out, 0, sizeof(AudioConverterPrimeInfo));
			break;
		case kAudioConverterChannelMap:
			memcpy(out, m_channelMap.data(), needed);
			break;
		case kAudioConverterCurrentInputStreamDescription:
			memcpy(out, &m_src.asbd, sizeof(AudioStreamBasicDescription));
			break;
		case kAudioConverterCurrentOutputStreamDescription:
			memcpy(out, &m_dst.asbd, sizeof(AudioStreamBasicDescription));
			break;
		case kAudioConverterPropertyCalculateOutputBufferSize:
		{
			const UInt32 srcFrame = m_src.frameBytes * (m_src.buffers == 1 ? 1 : m_src.channels);
			const UInt32 dstFrame = m_dst.frameBytes * (m_dst.buffers == 1 ? 1 : m_dst.channels);
			double frames = double(*word / srcFrame);

			if (m_resample)
				frames = std::ceil(frames * m_dst.asbd.mSampleRate / m_src.asbd.mSampleRate);
			*word = UInt32(frames) * dstFrame;
			break;
		}
		case kAudioConverterPropertyCalculateInputBufferSize:
		{
			const UInt32 srcFrame = m_src.frameBytes * (m_src.buffers == 1 ? 1 : m_src.channels);
			const UInt32 dstFrame = m_dst.frameBytes * (m_dst.buffers == 1 ? 1 : m_dst.channels);
			double frames = double(*word / dstFrame);

			if (m_resample)
				frames = std::ceil(frames * m_src.asbd.mSampleRate / m_dst.asbd.mSampleRate);
			*word = UInt32(frames) * srcFrame;
			break;
		}
		default:
			return kAudioConverterErr_PropertyNotSupported;
	}

	return noErr;
}

OSStatus PCMConverter::setProperty(AudioConverterPropertyID id, UInt32 size, const void* in)
{
	UInt32 needed;
	Boolean writable;
	OSStatus status = getPropertyInfo(id, &needed, &writable);

	if (status != noErr)
		return status;
	if (!writable)
		return kAudioConverterErr_PropertyNotSupported;
	if (!in)
		return paramErr;

	// QUIRK: a channel map longer than the number of destination channels is accepted
	// and its leading entries are used; a shorter one is a bad property size.
	// Evidence: black-box observation on macOS 26.7.3 (tier 2).
	// Test: test_channel_map (tests/src/audioconverter_pcm.c)
	if (id == kAudioConverterChannelMap ? size < needed : size != needed)
		return kAudioConverterErr_BadPropertySizeError;

	switch (id)
	{
		case kAudioConverterSampleRateConverterQuality:
			m_quality = *static_cast<const UInt32*>(in);
			m_resampler.reset();
			break;
		case kAudioConverterSampleRateConverterComplexity:
			m_complexity = *static_cast<const UInt32*>(in);
			break;
		case kAudioConverterPrimeMethod:
			m_primeMethod = *static_cast<const UInt32*>(in);
			break;
		case kAudioConverterChannelMap:
			m_channelMap.assign(static_cast<const SInt32*>(in), static_cast<const SInt32*>(in) + m_dst.channels);
			m_customMap = true;
			break;
		default:
			return kAudioConverterErr_PropertyNotSupported;
	}

	return noErr;
}
