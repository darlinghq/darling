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

#ifndef AVFAUDIO_AVAUDIOTYPES_H
#define AVFAUDIO_AVAUDIOTYPES_H

#include <Foundation/Foundation.h>

// Index of an input or output bus of an audio node.
typedef NSUInteger AVAudioNodeBus;

// A position in three-dimensional space, in meters.
typedef struct AVAudio3DPoint {
	float x;
	float y;
	float z;
} AVAudio3DPoint;

NS_INLINE AVAudio3DPoint AVAudioMake3DPoint(float x, float y, float z)
{
	AVAudio3DPoint point = { x, y, z };
	return point;
}

#endif
