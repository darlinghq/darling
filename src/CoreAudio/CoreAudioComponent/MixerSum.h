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

#ifndef MIXERSUM_H
#define MIXERSUM_H
#include <cstddef>

// out[i] = sum(inputs[k][i] * gains[k]) for k in [0, numInputs).
// Pure math, no AudioUnit types, so it can be unit tested standalone.
void mixerSum(const float* const* inputs, const float* gains, size_t numInputs, float* out, size_t numFrames);

#endif /* MIXERSUM_H */
