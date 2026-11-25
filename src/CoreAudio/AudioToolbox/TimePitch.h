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

#ifndef AUDIOTOOLBOX_TIMEPITCH_H
#define AUDIOTOOLBOX_TIMEPITCH_H
#include <cstddef>
#include <cstdint>
#include <vector>

// The time/pitch effect of an output AudioQueue: plays audio faster or slower with or without
// changing its pitch, and shifts the pitch by itself. Operates on planar 32-bit float audio.
//
//  Spectral, TimeDomain  tempo change by waveform similarity overlap-add (WSOLA: W. Verhelst and
//                        M. Roelands, "An overlap-add technique based on waveform similarity
//                        (WSOLA) for high quality time-scale modification of speech", ICASSP 1993),
//                        the two differing in the length of the analysis window; a pitch shift
//                        (Spectral only) is the same tempo change followed by resampling.
//  Varispeed             resampling only: speed and pitch change together.
//
// The processor pulls the audio it needs from a Source, so what it holds ahead of the output is
// the lookahead of the algorithm only.
class __attribute__((visibility("hidden"))) TimePitchProcessor
{
public:
	enum Algorithm { Spectral, TimeDomain, Varispeed };

	class Source
	{
	public:
		virtual ~Source() {}
		// Writes up to `frames` frames to every plane and returns how many it had.
		virtual size_t pull(float* const* planes, size_t frames) = 0;
		// Asked after a short pull: true if what was pulled is the end of the stream (the lookahead of the
		// algorithm is then played out and the next audio starts afresh), false if the supply merely ran dry
		// (an underrun: the processor waits for more and goes on where it was).
		virtual bool endOfStream() = 0;
	};

	TimePitchProcessor(unsigned channels, double sampleRate);

	// rate: media time per time played (clamped to 1/32...32); cents: pitch shift, ignored unless Spectral
	// (clamped to +-2400).
	void configure(Algorithm algorithm, double rate, double cents);

	// The settings leave the audio as it is.
	bool neutral() const { return m_neutral; }
	// The processor holds no audio.
	bool idle() const;
	// Media frames passing per frame played.
	double speed() const { return m_speed; }

	// Produces up to `frames` frames; fewer means the source ran dry and the stream's tail was played.
	size_t render(float* const* out, size_t frames, Source& source);
	void reset();

	// Whether the settings leave the audio as it is.
	static bool isNeutral(Algorithm algorithm, double rate, double cents);
	static double clampRate(double rate);
	static double clampCents(double cents);

private:
	struct Fifo
	{
		std::vector<std::vector<float>> ch;
		size_t head = 0;
		size_t size() const { return ch.empty() ? 0 : ch[0].size() - head; }
		void init(unsigned channels) { ch.assign(channels, std::vector<float>()); head = 0; }
		void clear();
		void append(const float* const* planes, size_t frames);
		void drop(size_t frames);
	};

	void buildWindow();
	void resetStream();
	float inAt(unsigned c, int64_t abs) const;
	int64_t inEnd() const { return m_inBase + int64_t(m_in.size()); }
	size_t pullInput(Source& source, size_t frames);
	bool fillMid(Source& source, size_t frames);
	void wsolaStep(bool flushing);
	void handOff(Source& source);
	size_t takeMid(float* const* out, size_t offset, size_t frames);
	size_t resample(float* const* out, size_t offset, size_t frames, Source& source);
	float midAt(unsigned c, int64_t abs) const;

	const unsigned m_channels;
	const double m_sampleRate;

	// settings
	Algorithm m_algorithm = Spectral;
	double m_rate = 1.0, m_cents = 0.0;
	bool m_neutral = true;
	double m_speed = 1.0;
	double m_tempo = 1.0;	// of the WSOLA stage; 0: none (Varispeed)
	double m_ratio = 1.0;	// reading speed of the resampling stage

	// WSOLA
	int64_t m_window = 0, m_half = 0, m_tolerance = 0;
	std::vector<float> m_rise, m_fall;
	Fifo m_in;
	int64_t m_inBase = 0;	// stream position of m_in's first frame
	bool m_haveSegment = false, m_flushing = false, m_ended = false;
	int64_t m_prevStart = 0;
	double m_nominal = 0.0;
	std::vector<std::vector<float>> m_tail;

	// what leaves the WSOLA stage, read by the resampling stage at m_pos (an absolute position
	// from m_midBase, the position of its first frame)
	Fifo m_mid;
	int64_t m_midBase = 0;
	double m_pos = 0.0;

	std::vector<std::vector<float>> m_scratch;
	std::vector<float*> m_ptrs;
	std::vector<const float*> m_cptrs;
	std::vector<float> m_reference;
	std::vector<std::vector<float>> m_block;
	std::vector<float*> m_blockPtrs;
	std::vector<const float*> m_blockCptrs;
};

#endif
