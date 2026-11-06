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

#include "MixerSum.h"
#include <cstring>

void mixerSum(const float* const* inputs, const float* gains, size_t numInputs, float* out, size_t numFrames)
{
	std::memset(out, 0, numFrames * sizeof(float));
	for (size_t k = 0; k < numInputs; k++)
	{
		float gain = gains[k];
		const float* in = inputs[k];
		for (size_t i = 0; i < numFrames; i++)
			out[i] += in[i] * gain;
	}
}
