// Copyright (C) 2026 Lubos Dolezel
//
// Sounds for the AVAudioPlayer tests, made from sines: WAV files (16, 24 bit integer or 32 bit float samples,
// any number of channels, each with its own amplitude) and big-endian AIFF files.
#ifndef AVAUDIOPLAYER_SOUNDS_H
#define AVAUDIOPLAYER_SOUNDS_H
#import <Foundation/Foundation.h>
#include <math.h>

static inline void put16(NSMutableData *d, unsigned v, int bigEndian)
{
	unsigned char b[2] = { bigEndian ? (v >> 8) & 255 : v & 255, bigEndian ? v & 255 : (v >> 8) & 255 };
	[d appendBytes:b length:2];
}

static inline void put32(NSMutableData *d, unsigned v, int bigEndian)
{
	unsigned char b[4];
	for (int i = 0; i < 4; i++)
		b[i] = bigEndian ? (v >> (24 - 8 * i)) & 255 : (v >> (8 * i)) & 255;
	[d appendBytes:b length:4];
}

// A sine of the given amplitude per channel (0 silences a channel), as a WAV file (16 or 24 bit integer, or 32 bit float).
static inline NSData *makeWavTones(unsigned rate, unsigned channels, unsigned bits, int isFloat, double seconds, const double *amplitudes, int truncateTo, double hz, double hz2)
{
	const unsigned frames = (unsigned) (seconds * rate);
	const unsigned bytes = bits / 8;
	NSMutableData *data = [NSMutableData data];
	[data appendBytes:"RIFF" length:4];
	put32(data, 36 + frames * channels * bytes, 0);
	[data appendBytes:"WAVEfmt " length:8];
	put32(data, 16, 0);
	put16(data, isFloat ? 3 : 1, 0);
	put16(data, channels, 0);
	put32(data, rate, 0);
	put32(data, rate * channels * bytes, 0);
	put16(data, channels * bytes, 0);
	put16(data, bits, 0);
	[data appendBytes:"data" length:4];
	put32(data, frames * channels * bytes, 0);
	for (unsigned i = 0; i < frames; i++) {
		for (unsigned c = 0; c < channels; c++) {
			// the second half of the sound can have another frequency
			double f = (hz2 > 0 && i >= frames / 2) ? hz2 : hz;
			double v = amplitudes[c] * sin(2 * M_PI * f * i / rate);
			if (isFloat) {
				float f = (float) v;
				[data appendBytes:&f length:4];
			} else if (bits == 16) {
				put16(data, (unsigned) (int) lrint(v * 32767) & 0xffff, 0);
			} else {
				int s = (int) lrint(v * 8388607);
				unsigned char b[3] = { s & 255, (s >> 8) & 255, (s >> 16) & 255 };
				[data appendBytes:b length:3];
			}
		}
	}
	if (truncateTo >= 0)
		[data setLength:truncateTo];
	return data;
}

// 16-bit big-endian AIFF
static inline NSData *makeAiff(unsigned rate, unsigned channels, double seconds, double amplitude)
{
	const unsigned frames = (unsigned) (seconds * rate);
	NSMutableData *d = [NSMutableData data];
	[d appendBytes:"FORM" length:4];
	put32(d, 4 + 8 + 18 + 8 + 8 + frames * channels * 2, 1);
	[d appendBytes:"AIFFCOMM" length:8];
	put32(d, 18, 1);
	put16(d, channels, 1);
	put32(d, frames, 1);
	put16(d, 16, 1);
	// the sample rate as an 80-bit extended float
	int exponent = 0;
	double m = rate;
	while (m >= 2) { m /= 2; exponent++; }
	unsigned long long mantissa = (unsigned long long) (m * 9223372036854775808.0);
	put16(d, 16383 + exponent, 1);
	put32(d, (unsigned) (mantissa >> 32), 1);
	put32(d, (unsigned) mantissa, 1);
	[d appendBytes:"SSND" length:4];
	put32(d, 8 + frames * channels * 2, 1);
	put32(d, 0, 1);
	put32(d, 0, 1);
	for (unsigned i = 0; i < frames; i++)
		for (unsigned c = 0; c < channels; c++)
			put16(d, (unsigned) (int) lrint(amplitude * sin(2 * M_PI * 440.0 * i / rate) * 32767) & 0xffff, 1);
	return d;
}


static inline NSData *makeWav(unsigned rate, unsigned channels, unsigned bits, int isFloat, double seconds, const double *amplitudes, int truncateTo)
{
	return makeWavTones(rate, channels, bits, isFloat, seconds, amplitudes, truncateTo, 440.0, 0);
}

#endif
