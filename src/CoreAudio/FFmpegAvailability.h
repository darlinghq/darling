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

#ifndef COREAUDIO_FFMPEG_AVAILABILITY_H
#define COREAUDIO_FFMPEG_AVAILABILITY_H

// The FFmpeg libraries are optional host libraries: when one is missing, only the
// codec and container operations that need it fail, while everything else keeps
// working. Code that calls into FFmpeg must check these first.

extern "C" {
int darling_avformat_available(void);
int darling_avcodec_available(void);
int darling_avutil_available(void);
int darling_swresample_available(void);
}

// Decoding, encoding and sample format conversion
static inline bool ffmpegCodecsAvailable()
{
	return darling_avcodec_available() && darling_avutil_available() && darling_swresample_available();
}

// Demuxing and muxing audio files
static inline bool ffmpegContainersAvailable()
{
	return darling_avformat_available() && darling_avcodec_available() && darling_avutil_available();
}

#endif
