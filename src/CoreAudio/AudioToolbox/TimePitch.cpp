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

#include "TimePitch.h"
#include <algorithm>
#include <cmath>
#include <cstring>

static const double kMinRate = 1.0 / 32.0, kMaxRate = 32.0, kMaxCents = 2400.0;

double TimePitchProcessor::clampRate(double rate)
{
	if (!(rate > kMinRate)) // also NaN
		return kMinRate;
	return std::min(rate, kMaxRate);
}

double TimePitchProcessor::clampCents(double cents)
{
	if (!(cents == cents))
		return 0.0;
	return std::min(std::max(cents, -kMaxCents), kMaxCents);
}

bool TimePitchProcessor::isNeutral(Algorithm algorithm, double rate, double cents)
{
	const double r = clampRate(rate);
	const double pitch = algorithm == Spectral || algorithm == Varispeed ? std::pow(2.0, clampCents(cents) / 1200.0) : 1.0;
	const double speed = algorithm == Varispeed ? r * pitch : r;
	const double ratio = algorithm == Varispeed ? speed : pitch;
	return std::fabs(speed - 1.0) < 1e-6 && std::fabs(ratio - 1.0) < 1e-6;
}

void TimePitchProcessor::Fifo::clear()
{
	for (auto& c : ch)
		c.clear();
	head = 0;
}

void TimePitchProcessor::Fifo::append(const float* const* planes, size_t frames)
{
	for (size_t c = 0; c < ch.size(); c++)
		ch[c].insert(ch[c].end(), planes[c], planes[c] + frames);
}

void TimePitchProcessor::Fifo::drop(size_t frames)
{
	head += std::min(frames, size());
	// Compact once the consumed part dominates, so that dropping stays cheap.
	if (head >= 4096 && head * 2 >= (ch.empty() ? 0 : ch[0].size()))
	{
		for (auto& c : ch)
			c.erase(c.begin(), c.begin() + head);
		head = 0;
	}
}

TimePitchProcessor::TimePitchProcessor(unsigned channels, double sampleRate)
: m_channels(channels), m_sampleRate(sampleRate)
{
	m_in.init(channels);
	m_mid.init(channels);
	m_tail.assign(channels, std::vector<float>());
	m_scratch.assign(channels, std::vector<float>());
	m_ptrs.assign(channels, nullptr);
	m_cptrs.assign(channels, nullptr);
	m_block.assign(channels, std::vector<float>());
	m_blockPtrs.assign(channels, nullptr);
	m_blockCptrs.assign(channels, nullptr);
	buildWindow();
}

void TimePitchProcessor::buildWindow()
{
	// Windows of about 23 ms (TimeDomain) and 46 ms (Spectral); the longer one follows low
	// frequencies better, the shorter one keeps transients tighter.
	const double seconds = m_algorithm == Spectral ? 0.0464 : 0.0232;
	int64_t window = int64_t(std::llround(m_sampleRate * seconds)) & ~int64_t(1);
	window = std::max<int64_t>(window, 128);
	if (window == m_window)
		return;
	m_window = window;
	m_half = window / 2;
	m_tolerance = window / 4;
	// Periodic Hann window: the rising half and the falling half of neighbouring windows
	// sum to one when they overlap by half a window.
	m_rise.resize(m_half);
	m_fall.resize(m_half);
	for (int64_t i = 0; i < m_half; i++)
	{
		m_rise[i] = float(0.5 - 0.5 * std::cos(2.0 * M_PI * double(i) / double(window)));
		m_fall[i] = float(0.5 - 0.5 * std::cos(2.0 * M_PI * double(i + m_half) / double(window)));
	}
	for (auto& t : m_tail)
		t.assign(m_half, 0.0f);
	m_reference.assign(m_half, 0.0f);
}

void TimePitchProcessor::configure(Algorithm algorithm, double rate, double cents)
{
	if (algorithm != m_algorithm)
	{
		m_algorithm = algorithm;
		const int64_t before = m_window;
		buildWindow();
		if (m_window != before)
			reset();
	}
	m_rate = clampRate(rate);
	m_cents = algorithm == Spectral ? clampCents(cents) : 0.0;
	if (algorithm == Varispeed)
	{
		// Speed and pitch move together; the pitch setting adds to the rate.
		m_tempo = 0.0;
		m_ratio = m_rate * std::pow(2.0, clampCents(cents) / 1200.0);
		m_speed = m_ratio;
	}
	else
	{
		const double pitch = std::pow(2.0, m_cents / 1200.0);
		m_tempo = m_rate / pitch;
		m_ratio = pitch;
		m_speed = m_rate;
	}
	m_neutral = std::fabs(m_speed - 1.0) < 1e-6 && std::fabs(m_ratio - 1.0) < 1e-6;
	if (m_neutral)
	{
		m_ratio = 1.0;
		m_speed = 1.0;
		if (m_tempo != 0.0)
			m_tempo = 1.0;
	}
}

void TimePitchProcessor::resetStream()
{
	m_in.clear();
	m_inBase = 0;
	m_haveSegment = m_flushing = m_ended = false;
	m_prevStart = 0;
	m_nominal = 0.0;
	m_mid.clear();
	m_midBase = 0;
	m_pos = 0.0;
}

void TimePitchProcessor::reset()
{
	resetStream();
}

bool TimePitchProcessor::idle() const
{
	if (m_haveSegment || m_in.size())
		return false;
	const double avail = double(m_midBase + int64_t(m_mid.size())) - m_pos;
	return avail <= (m_ratio == 1.0 ? 0.0 : 2.0);
}

float TimePitchProcessor::inAt(unsigned c, int64_t abs) const
{
	const int64_t i = abs - m_inBase;
	if (i < 0 || i >= int64_t(m_in.size()))
		return 0.0f; // before what is kept, or after the end of the stream: silence
	return m_in.ch[c][m_in.head + size_t(i)];
}

float TimePitchProcessor::midAt(unsigned c, int64_t abs) const
{
	if (!m_mid.size())
		return 0.0f;
	int64_t i = abs - m_midBase;
	i = std::min<int64_t>(std::max<int64_t>(i, 0), int64_t(m_mid.size()) - 1);
	return m_mid.ch[c][m_mid.head + size_t(i)];
}

size_t TimePitchProcessor::pullInput(Source& source, size_t frames)
{
	for (auto& s : m_scratch)
		s.resize(frames);
	for (unsigned c = 0; c < m_channels; c++)
		m_ptrs[c] = m_scratch[c].data();
	const size_t got = std::min(source.pull(m_ptrs.data(), frames), frames);
	for (unsigned c = 0; c < m_channels; c++)
		m_cptrs[c] = m_ptrs[c];
	m_in.append(m_cptrs.data(), got);
	return got;
}

void TimePitchProcessor::wsolaStep(bool flushing)
{
	const int64_t Hs = m_half;
	const double Ha = m_tempo * double(Hs);
	std::vector<float*>& planes = m_blockPtrs;
	std::vector<const float*>& cplanes = m_blockCptrs;
	for (unsigned c = 0; c < m_channels; c++)
	{
		m_block[c].resize(size_t(Hs));
		planes[c] = m_block[c].data();
		cplanes[c] = planes[c];
	}

	if (!m_haveSegment)
	{
		const int64_t pos = int64_t(std::llround(m_nominal));
		if (flushing && pos >= inEnd())
		{
			m_ended = true;
			m_in.clear();
			m_inBase = 0;
			return;
		}
		// The first segment of a stream goes out without a fade-in.
		for (unsigned c = 0; c < m_channels; c++)
			for (int64_t i = 0; i < Hs; i++)
			{
				planes[c][i] = inAt(c, pos + i);
				m_tail[c][i] = m_fall[i] * inAt(c, pos + Hs + i);
			}
		m_mid.append(cplanes.data(), size_t(Hs));
		m_haveSegment = true;
		m_prevStart = pos;
		m_nominal += Ha;
	}
	else
	{
		const int64_t nominal = int64_t(std::llround(m_nominal));
		if (flushing && nominal >= inEnd())
		{
			// The stream is over: what remains of the last segment fades out.
			for (unsigned c = 0; c < m_channels; c++)
				std::copy(m_tail[c].begin(), m_tail[c].end(), planes[c]);
			m_mid.append(cplanes.data(), size_t(Hs));
			m_haveSegment = false;
			m_ended = true;
			m_in.clear();
			m_inBase = 0;
			return;
		}

		// Where the previous segment would have gone on, and the start near the nominal one
		// whose first half resembles it most (normalized cross-correlation).
		const int64_t natural = m_prevStart + Hs;
		const int64_t stride = std::max<int64_t>(1, Hs / 128);
		for (int64_t i = 0; i < Hs; i += stride)
		{
			float v = 0;
			for (unsigned c = 0; c < m_channels; c++)
				v += inAt(c, natural + i);
			m_reference[i] = v;
		}
		const int64_t lo = std::max(nominal - m_tolerance, m_inBase), hi = nominal + m_tolerance;
		int64_t best = std::min(std::max(nominal, lo), hi);
		double bestScore = -1e30;
		for (int64_t k = 0; k <= m_tolerance; k++)
		{
			for (int sign = 0; sign < (k ? 2 : 1); sign++)
			{
				const int64_t s = sign ? nominal - k : nominal + k;
				if (s < lo || s > hi)
					continue;
				double dot = 0, energy = 0;
				for (int64_t i = 0; i < Hs; i += stride)
				{
					float v = 0;
					for (unsigned c = 0; c < m_channels; c++)
						v += inAt(c, s + i);
					dot += double(m_reference[i]) * v;
					energy += double(v) * v;
				}
				const double score = dot / std::sqrt(energy + 1e-9);
				// Candidates are tried from the nominal start outward, and only a clearly better
				// one replaces the current: ties (silence, steady tones) keep the earliest.
				if (score > bestScore + 1e-9 * std::fabs(bestScore) + 1e-12)
				{
					bestScore = score;
					best = s;
				}
			}
		}

		for (unsigned c = 0; c < m_channels; c++)
			for (int64_t i = 0; i < Hs; i++)
			{
				planes[c][i] = m_tail[c][i] + m_rise[i] * inAt(c, best + i);
				m_tail[c][i] = m_fall[i] * inAt(c, best + Hs + i);
			}
		m_mid.append(cplanes.data(), size_t(Hs));
		m_prevStart = best;
		m_nominal += Ha;
	}

	// Input that no later segment will read goes.
	const int64_t keep = std::min(m_prevStart + Hs, int64_t(std::llround(m_nominal)) - m_tolerance);
	if (keep > m_inBase)
	{
		const int64_t drop = std::min<int64_t>(keep - m_inBase, int64_t(m_in.size()));
		m_in.drop(size_t(drop));
		m_inBase += drop;
	}
}

bool TimePitchProcessor::fillMid(Source& source, size_t frames)
{
	if (m_ended)
		return false;
	const size_t before = m_mid.size();
	if (m_tempo == 0.0)
	{
		// No tempo stage: the resampler reads the source as it comes.
		for (auto& s : m_scratch)
			s.resize(frames);
		for (unsigned c = 0; c < m_channels; c++)
			m_ptrs[c] = m_scratch[c].data();
		const size_t got = std::min(source.pull(m_ptrs.data(), frames), frames);
		for (unsigned c = 0; c < m_channels; c++)
			m_cptrs[c] = m_ptrs[c];
		m_mid.append(m_cptrs.data(), got);
		if (got < frames && source.endOfStream())
			m_ended = true;
		return m_mid.size() > before;
	}
	while (m_mid.size() - before < frames && !m_ended)
	{
		const int64_t needEnd = int64_t(std::llround(m_nominal)) + (m_haveSegment ? m_tolerance : 0) + m_window;
		if (!m_flushing && inEnd() < needEnd)
		{
			const size_t want = size_t(needEnd - inEnd());
			if (pullInput(source, want) < want)
			{
				if (!source.endOfStream())
					break; // an underrun: whatever has been produced is all there is for now
				m_flushing = true;
			}
			continue;
		}
		wsolaStep(m_flushing);
	}
	return m_mid.size() > before;
}

// The settings became neutral while the tempo stage holds a segment: finish the overlap with the
// audio that follows it and give the rest of what it read ahead back as it is.
void TimePitchProcessor::handOff(Source& source)
{
	if (m_haveSegment)
	{
		const int64_t Hs = m_half;
		const int64_t natural = m_prevStart + Hs;
		const int64_t needEnd = natural + Hs;
		if (inEnd() < needEnd)
			pullInput(source, size_t(needEnd - inEnd()));
		std::vector<std::vector<float>> block(m_channels, std::vector<float>(size_t(Hs)));
		std::vector<const float*> planes(m_channels);
		for (unsigned c = 0; c < m_channels; c++)
		{
			for (int64_t i = 0; i < Hs; i++)
				block[c][i] = m_tail[c][i] + m_rise[i] * inAt(c, natural + i);
			planes[c] = block[c].data();
		}
		m_mid.append(planes.data(), size_t(Hs));
		// What was read beyond that goes through untouched.
		const int64_t restFrom = needEnd - m_inBase;
		if (restFrom < int64_t(m_in.size()))
		{
			for (unsigned c = 0; c < m_channels; c++)
				planes[c] = m_in.ch[c].data() + m_in.head + restFrom;
			m_mid.append(planes.data(), m_in.size() - size_t(restFrom));
		}
	}
	else if (m_in.size())
	{
		std::vector<const float*> planes(m_channels);
		for (unsigned c = 0; c < m_channels; c++)
			planes[c] = m_in.ch[c].data() + m_in.head;
		m_mid.append(planes.data(), m_in.size());
	}
	m_in.clear();
	m_inBase = 0;
	m_haveSegment = false;
	m_nominal = 0.0;
}

size_t TimePitchProcessor::takeMid(float* const* out, size_t offset, size_t frames)
{
	const int64_t first = std::max<int64_t>(int64_t(std::ceil(m_pos)) - m_midBase, 0);
	const int64_t avail = int64_t(m_mid.size()) - first;
	const size_t n = size_t(std::min<int64_t>(std::max<int64_t>(avail, 0), int64_t(frames)));
	for (unsigned c = 0; c < m_channels; c++)
		if (n)
			std::memcpy(out[c] + offset, m_mid.ch[c].data() + m_mid.head + first, n * sizeof(float));
	m_pos = double(m_midBase + first + int64_t(n));
	const int64_t keep = std::min<int64_t>(int64_t(m_pos), m_midBase + int64_t(m_mid.size()));
	if (keep > m_midBase)
	{
		m_mid.drop(size_t(keep - m_midBase));
		m_midBase = keep;
	}
	return n;
}

size_t TimePitchProcessor::resample(float* const* out, size_t offset, size_t frames, Source& source)
{
	size_t n = 0;
	while (n < frames)
	{
		const int64_t i = int64_t(std::floor(m_pos));
		const bool unit = m_ratio == 1.0 && m_pos == double(i);
		const int64_t needEnd = unit ? i + 1 : i + 3;
		while (m_midBase + int64_t(m_mid.size()) < needEnd && !m_ended)
			if (!fillMid(source, 1024))
				break;
		const int64_t end = m_midBase + int64_t(m_mid.size());
		if (unit ? i >= end : (m_ended ? m_pos >= double(end - 1) : end < needEnd))
			break;
		if (unit)
		{
			for (unsigned c = 0; c < m_channels; c++)
				out[c][offset + n] = midAt(c, i);
		}
		else
		{
			// Catmull-Rom cubic interpolation between the two middle points of four.
			const float f = float(m_pos - double(i));
			for (unsigned c = 0; c < m_channels; c++)
			{
				const float y0 = midAt(c, i - 1), y1 = midAt(c, i), y2 = midAt(c, i + 1), y3 = midAt(c, i + 2);
				out[c][offset + n] = 0.5f * ((2 * y1) + (y2 - y0) * f + (2 * y0 - 5 * y1 + 4 * y2 - y3) * f * f
						+ (3 * y1 - y0 - 3 * y2 + y3) * f * f * f);
			}
		}
		m_pos += m_ratio;
		n++;
	}
	const int64_t keep = std::min<int64_t>(int64_t(std::floor(m_pos)) - 1, m_midBase + int64_t(m_mid.size()));
	if (keep > m_midBase)
	{
		m_mid.drop(size_t(keep - m_midBase));
		m_midBase = keep;
	}
	return n;
}

size_t TimePitchProcessor::render(float* const* out, size_t frames, Source& source)
{
	size_t done = 0;
	bool restarted = false;
	std::vector<float*> shifted(m_channels);
	while (done < frames)
	{
		size_t got;
		if (m_neutral && !m_flushing && !m_ended)
		{
			if (m_haveSegment || m_in.size())
				handOff(source);
			if (int64_t(m_mid.size()) + m_midBase > int64_t(std::ceil(m_pos)))
				got = takeMid(out, done, frames - done);
			else
			{
				// Nothing is held back: straight from the source.
				for (unsigned c = 0; c < m_channels; c++)
					shifted[c] = out[c] + done;
				got = std::min(source.pull(shifted.data(), frames - done), frames - done);
				done += got;
				if (done < frames)
					break; // the source has nothing more
				continue;
			}
		}
		else
			got = resample(out, done, frames - done, source);
		done += got;
		const bool drained = m_ended && !m_haveSegment && !m_in.size()
				&& double(m_midBase + int64_t(m_mid.size())) - m_pos <= (m_ratio == 1.0 && m_pos == std::floor(m_pos) ? 0.0 : 1.0);
		if (drained)
		{
			// The stream's tail has been played; what comes next starts afresh.
			resetStream();
			if (got == 0 && restarted)
				break;
			restarted = true;
			continue;
		}
		if (got == 0)
			break;
	}
	return done;
}
