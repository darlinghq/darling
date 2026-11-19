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

// The graphic equalizer effect (kAudioUnitSubType_GraphicEQ): 10 octave bands
// or 31 third-octave bands, each a peaking filter with a gain of -20..+20 dB.
// Parameters 0..(bands - 1) are the band gains, kGraphicEQParam_NumberOfBands
// selects 10 (0) or 31 (1) bands.
//
// Each band is a peaking biquad from Robert Bristow-Johnson's "Cookbook
// formulae for audio EQ biquad filter coefficients", with the bandwidth given
// in octaves. The band frequencies, the bandwidths and the narrowing of a band
// as its gain grows match what Apple's unit does, measured from outside
// (tone sweeps through AudioUnitRender on macOS 26.7.3): at 10 bands each
// band is about 1.9 octaves wide, at 31 bands about 0.68 octaves, and the
// filter Q grows with the gain by (A + 1/A) / 2, where A = 10^(gain / 40).

#include "AUEffectBase.h"
#include <AudioToolbox/AudioUnitParameters.h>
#include <algorithm>
#include <cmath>
#include <cstdio>

namespace
{

constexpr int kMaxBands = 31;
constexpr float kMinGain = -20, kMaxGain = 20;

// Centre frequencies as the unit names its bands
const double kOctaveBands[10] = { 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384 };
const double kThirdOctaveBands[31] = {
	20, 25, 31.5, 40, 50, 63, 80, 100, 125, 160, 200, 250, 315, 400, 500, 630,
	800, 1000, 1250, 1600, 2000, 2500, 3150, 4000, 5000, 6300, 8000, 10000,
	12000, 16000, 20000
};

// Q of a band with no gain, at low frequencies
constexpr double kOctaveQ = 0.704;
constexpr double kThirdOctaveQ = 2.115;

int bandCount(AudioUnitParameterValue numberOfBands)
{
	return numberOfBands >= 0.5f ? 31 : 10;
}

const double* bandFrequencies(int bands)
{
	return bands == 31 ? kThirdOctaveBands : kOctaveBands;
}

struct Biquad
{
	double b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
	double z1 = 0, z2 = 0; // transposed direct form II state
	bool active = false;

	void setPeaking(double f0, double sampleRate, double gainDB, double q0)
	{
		const double nyquist = sampleRate / 2;
		active = gainDB != 0 && f0 > 0 && f0 < nyquist;
		if (!active)
			return;

		const double A = std::pow(10.0, gainDB / 40);
		const double w0 = 2 * M_PI * f0 / sampleRate;
		const double sinW0 = std::sin(w0), cosW0 = std::cos(w0);

		// Bandwidth in octaves for this Q, kept constant across the band's
		// frequency by the cookbook's bilinear-transform correction
		const double bandwidth = 2 * std::asinh(1 / (2 * q0)) / M_LN2;
		const double q = (A + 1 / A) / 2 / (2 * std::sinh(M_LN2 / 2 * bandwidth * w0 / sinW0));
		const double alpha = sinW0 / (2 * q);

		const double a0 = 1 + alpha / A;
		b0 = (1 + alpha * A) / a0;
		b1 = -2 * cosW0 / a0;
		b2 = (1 - alpha * A) / a0;
		a1 = -2 * cosW0 / a0;
		a2 = (1 - alpha / A) / a0;
	}

	inline double process(double x)
	{
		const double y = b0 * x + z1;
		z1 = b1 * x - a1 * y + z2;
		z2 = b2 * x - a2 * y;
		return y;
	}

	void reset() { z1 = z2 = 0; }
};

}

class GraphicEQKernel : public AUKernelBase
{
public:
	GraphicEQKernel(AUEffectBase* unit) : AUKernelBase(unit) {}

	void Reset() override
	{
		for (Biquad& b : m_bands)
			b.reset();
	}

	void Process(const Float32* inSourceP, Float32* inDestP, UInt32 inFramesToProcess,
		UInt32 inNumChannels, bool& ioSilence) override
	{
		updateFilters();

		bool anyActive = false;
		for (int i = 0; i < m_count; i++)
			anyActive = anyActive || m_bands[i].active;

		// A silent input leaves silence once the filters have rung out; the
		// caller keeps track of the tail time
		if (!anyActive)
		{
			for (UInt32 i = 0; i < inFramesToProcess; i++)
				inDestP[i * inNumChannels] = inSourceP[i * inNumChannels];
			return;
		}

		ioSilence = false;
		for (UInt32 i = 0; i < inFramesToProcess; i++)
		{
			double v = inSourceP[i * inNumChannels];
			for (int b = 0; b < m_count; b++)
			{
				if (m_bands[b].active)
					v = m_bands[b].process(v);
			}
			inDestP[i * inNumChannels] = Float32(v);
		}
	}

private:
	void updateFilters()
	{
		const int count = bandCount(GetParameter(kGraphicEQParam_NumberOfBands));
		const double rate = GetSampleRate();
		bool layoutChanged = count != m_count || rate != m_rate;

		if (layoutChanged)
		{
			m_count = count;
			m_rate = rate;
			for (Biquad& b : m_bands)
				b = Biquad();
		}

		const double* freqs = bandFrequencies(count);
		const double q0 = count == 31 ? kThirdOctaveQ : kOctaveQ;
		for (int i = 0; i < count; i++)
		{
			float gain = std::clamp(GetParameter(i), kMinGain, kMaxGain);
			if (layoutChanged || gain != m_gains[i])
			{
				m_gains[i] = gain;
				m_bands[i].setPeaking(freqs[i], rate, gain, q0);
			}
		}
	}

	Biquad m_bands[kMaxBands];
	float m_gains[kMaxBands] = {};
	int m_count = 0;
	double m_rate = 0;
};

class GraphicEQUnit : public AUEffectBase
{
public:
	GraphicEQUnit(AudioComponentInstance instance) : AUEffectBase(instance)
	{
		CreateElements();
		for (int i = 0; i < kMaxBands; i++)
			Globals()->SetParameter(i, 0);
		// 31 bands, as on macOS 26
		Globals()->SetParameter(kGraphicEQParam_NumberOfBands, 1);
	}

	AUKernelBase* NewKernel() override { return new GraphicEQKernel(this); }

	bool SupportsTail() override { return true; }
	Float64 GetTailTime() override { return 0.001; }
	Float64 GetLatency() override { return 0; }

	int bands() { return bandCount(Globals()->GetParameter(kGraphicEQParam_NumberOfBands)); }

	OSStatus GetParameterList(AudioUnitScope inScope, AudioUnitParameterID* outParameterList,
		UInt32& outNumParameters) override
	{
		if (inScope != kAudioUnitScope_Global)
		{
			outNumParameters = 0;
			return noErr;
		}
		const int count = bands();
		if (outParameterList)
		{
			for (int i = 0; i < count; i++)
				outParameterList[i] = i;
			outParameterList[count] = kGraphicEQParam_NumberOfBands;
		}
		outNumParameters = count + 1;
		return noErr;
	}

	OSStatus GetParameterInfo(AudioUnitScope inScope, AudioUnitParameterID inParameterID,
		AudioUnitParameterInfo& outParameterInfo) override
	{
		if (inScope != kAudioUnitScope_Global)
			return kAudioUnitErr_InvalidScope;

		outParameterInfo.flags = kAudioUnitParameterFlag_IsWritable | kAudioUnitParameterFlag_IsReadable;

		if (inParameterID == kGraphicEQParam_NumberOfBands)
		{
			FillInParameterName(outParameterInfo, CFSTR("Number of Bands"), false);
			outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
			outParameterInfo.minValue = 0;
			outParameterInfo.maxValue = 1;
			outParameterInfo.defaultValue = 1;
			// Changing it changes the list of the other parameters
			outParameterInfo.flags |= kAudioUnitParameterFlag_IsGlobalMeta;
			return noErr;
		}

		const int count = bands();
		if (inParameterID >= AudioUnitParameterID(count))
			return kAudioUnitErr_InvalidParameter;

		const double f = bandFrequencies(count)[inParameterID];
		char name[32];
		if (count == 31)
			snprintf(name, sizeof(name), "%.1f Hz", f);
		else
			snprintf(name, sizeof(name), "%d Hz", int(f));
		FillInParameterName(outParameterInfo, CFStringCreateWithCString(nullptr, name, kCFStringEncodingUTF8), true);
		outParameterInfo.unit = kAudioUnitParameterUnit_Decibels;
		outParameterInfo.minValue = kMinGain;
		outParameterInfo.maxValue = kMaxGain;
		outParameterInfo.defaultValue = 0;
		return noErr;
	}

	OSStatus GetParameterValueStrings(AudioUnitScope inScope, AudioUnitParameterID inParameterID,
		CFArrayRef* outStrings) override
	{
		if (inScope != kAudioUnitScope_Global || inParameterID != kGraphicEQParam_NumberOfBands)
			return kAudioUnitErr_InvalidProperty;
		if (outStrings)
		{
			CFStringRef names[] = { CFSTR("10 Bands"), CFSTR("31 Bands") };
			*outStrings = CFArrayCreate(nullptr, (const void**) names, 2, &kCFTypeArrayCallBacks);
		}
		return noErr;
	}

	OSStatus SetParameter(AudioUnitParameterID inID, AudioUnitScope inScope, AudioUnitElement inElement,
		AudioUnitParameterValue inValue, UInt32 inBufferOffsetInFrames) override
	{
		if (inScope != kAudioUnitScope_Global)
			return kAudioUnitErr_InvalidScope;
		if (inElement != 0)
			return kAudioUnitErr_InvalidElement;
		if (inID >= kMaxBands && inID != kGraphicEQParam_NumberOfBands)
			return kAudioUnitErr_InvalidParameter;

		const int before = bands();
		Globals()->SetParameter(inID, inValue);

		if (inID == kGraphicEQParam_NumberOfBands && bands() != before)
		{
			PropertyChanged(kAudioUnitProperty_ParameterList, kAudioUnitScope_Global, 0);
		}
		return noErr;
	}
};

#pragma GCC visibility push(default)
AUDIOCOMPONENT_ENTRY(AUBaseFactory, GraphicEQUnit);
#pragma GCC visibility pop
