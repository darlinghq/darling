/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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

#include <vDSP/vDSP.h>
#include <math.h>
#include <stdlib.h>

struct OpaqueFFTSetup
{
	vDSP_Length log2n;
	// exp(-2 * pi * i * k / 2^log2n) for k < 2^(log2n - 1), computed in double precision.
	float* twiddleReal;
	float* twiddleImag;
};

FFTSetup vDSP_create_fftsetup(vDSP_Length __Log2n, FFTRadix __Radix)
{
	// Only radix 2 lengths are implemented; failing is the documented way to refuse.
	if (__Radix != kFFTRadix2 || __Log2n >= 31)
		return NULL;

	FFTSetup setup = malloc(sizeof(*setup));
	if (!setup)
		return NULL;

	const size_t size = (size_t) 1 << __Log2n;
	const size_t count = size / 2 + 1;

	setup->log2n = __Log2n;
	setup->twiddleReal = malloc(count * sizeof(float));
	setup->twiddleImag = malloc(count * sizeof(float));

	if (!setup->twiddleReal || !setup->twiddleImag)
	{
		vDSP_destroy_fftsetup(setup);
		return NULL;
	}

	for (size_t k = 0; k < count; k++)
	{
		const double angle = -2.0 * M_PI * (double) k / (double) size;
		setup->twiddleReal[k] = (float) cos(angle);
		setup->twiddleImag[k] = (float) sin(angle);
	}
	return setup;
}

void vDSP_destroy_fftsetup(FFTSetup __setup)
{
	if (!__setup)
		return;
	free(__setup->twiddleReal);
	free(__setup->twiddleImag);
	free(__setup);
}

// In-place unscaled radix 2 transform of 2^log2m complex values. Inputs are in
// normal order, and so are outputs. A positive direction has the forward (e^-i) sign.
static void complexFFT(FFTSetup setup, float* re, float* im, long stride, vDSP_Length log2m, int direction)
{
	const size_t m = (size_t) 1 << log2m;

	for (size_t i = 1, j = 0; i < m; i++)
	{
		size_t bit = m >> 1;
		for (; j & bit; bit >>= 1)
			j ^= bit;
		j ^= bit;

		if (i < j)
		{
			const long a = (long) i * stride, b = (long) j * stride;
			const float tr = re[a], ti = im[a];
			re[a] = re[b];
			im[a] = im[b];
			re[b] = tr;
			im[b] = ti;
		}
	}

	const float sign = direction > 0 ? 1.0f : -1.0f;

	for (vDSP_Length stage = 1; stage <= log2m; stage++)
	{
		const size_t span = (size_t) 1 << stage;
		const size_t half = span / 2;
		const size_t step = ((size_t) 1 << setup->log2n) / span;

		for (size_t start = 0; start < m; start += span)
		{
			for (size_t k = 0; k < half; k++)
			{
				const float wr = setup->twiddleReal[k * step], wi = sign * setup->twiddleImag[k * step];
				const long a = (long) (start + k) * stride, b = (long) (start + k + half) * stride;
				const float tr = re[b] * wr - im[b] * wi;
				const float ti = re[b] * wi + im[b] * wr;

				re[b] = re[a] - tr;
				im[b] = im[a] - ti;
				re[a] += tr;
				im[a] += ti;
			}
		}
	}
}

// Real transform of 2^__Log2N values packed as even samples in realp and odd samples in imagp.
//
// The forward result is twice the mathematical DFT, with bins 0 and N/2 (both real) in
// realp[0] and imagp[0]. The inverse is the plain sum over all N bins, so that a round trip
// multiplies the signal by 2N, which callers are documented to scale away.
void vDSP_fft_zrip(FFTSetup __Setup, const DSPSplitComplex* __C, vDSP_Stride __IC, vDSP_Length __Log2N, FFTDirection __Direction)
{
	if (!__Setup || __Log2N == 0 || __Log2N > __Setup->log2n)
		return;

	float* const re = __C->realp;
	float* const im = __C->imagp;
	const long s = __IC;
	const size_t m = (size_t) 1 << (__Log2N - 1);
	const size_t step = ((size_t) 1 << __Setup->log2n) >> __Log2N;

	if (__Direction == kFFTDirection_Forward)
	{
		complexFFT(__Setup, re, im, s, __Log2N - 1, kFFTDirection_Forward);

		const float z0r = re[0], z0i = im[0];
		re[0] = 2.0f * (z0r + z0i);
		im[0] = 2.0f * (z0r - z0i);

		// Y[k] = Z[k] + conj(Z[m-k]) - i * W^k * (Z[k] - conj(Z[m-k])), and the same for m-k.
		for (size_t k = 1; k <= m / 2; k++)
		{
			const long a = (long) k * s, b = (long) (m - k) * s;
			const float sumR = re[a] + re[b], sumI = im[a] - im[b];
			const float difR = re[a] - re[b], difI = im[a] + im[b];
			const float wr = __Setup->twiddleReal[k * step], wi = __Setup->twiddleImag[k * step];

			// -i * W^k * (difR + i * difI)
			const float tr = wr * difI + wi * difR;
			const float ti = wi * difI - wr * difR;

			re[a] = sumR + tr;
			im[a] = sumI + ti;
			if (a != b)
			{
				re[b] = sumR - tr;
				im[b] = ti - sumI;
			}
		}
	}
	else
	{
		const float dc = re[0], nyquist = im[0];
		re[0] = dc + nyquist;
		im[0] = dc - nyquist;

		// Z[k] = Y[k] + conj(Y[m-k]) + i * conj(W^k) * (Y[k] - conj(Y[m-k])), and the same for m-k.
		for (size_t k = 1; k <= m / 2; k++)
		{
			const long a = (long) k * s, b = (long) (m - k) * s;
			const float sumR = re[a] + re[b], sumI = im[a] - im[b];
			const float difR = re[a] - re[b], difI = im[a] + im[b];
			const float wr = __Setup->twiddleReal[k * step], wi = -__Setup->twiddleImag[k * step];

			// i * conj(W^k) * (difR + i * difI)
			const float tr = -(wr * difI + wi * difR);
			const float ti = wr * difR - wi * difI;

			re[a] = sumR + tr;
			im[a] = sumI + ti;
			if (a != b)
			{
				re[b] = sumR - tr;
				im[b] = ti - sumI;
			}
		}

		complexFFT(__Setup, re, im, s, __Log2N - 1, kFFTDirection_Inverse);
	}
}
