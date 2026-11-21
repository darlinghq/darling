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

#ifndef AVPlaybackEngine_h
#define AVPlaybackEngine_h

// Private to AVFoundation: container demuxing and the paced playback loop behind AVPlayer.
//
// Containers are read with the host libavformat (AVFoundation demuxes on its own on macOS too),
// through custom I/O over the guest file descriptor so that every path the guest can open works.
// Video is decoded by VideoToolbox (VTDecompressionSession) from CoreMedia sample buffers.
// Audio is not played: AVFoundation has no audio renderer yet, so only the video track is shown
// and the playback clock runs without sound.

#include <CoreFoundation/CoreFoundation.h>
#include <CoreVideo/CoreVideo.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
	AVPETrackOther = 0,
	AVPETrackVideo = 1,
	AVPETrackAudio = 2,
};

typedef struct {
	int streamIndex;
	int64_t trackID;
	int type;
	int width, height;
	float frameRate;
	int64_t durationUs;
	bool decodable; // video: VideoToolbox has a decoder for the codec
} AVPETrackInfo;

typedef struct AVPEMedia AVPEMedia;

// NULL if the file cannot be opened or its container is not recognized, or if the host FFmpeg
// libraries are not available.
AVPEMedia *AVPEMediaOpen(const char *path);
void AVPEMediaClose(AVPEMedia *media);
int64_t AVPEMediaDurationUs(const AVPEMedia *media);
int AVPEMediaTrackCount(const AVPEMedia *media);
const AVPETrackInfo *AVPEMediaTrack(const AVPEMedia *media, int index);

typedef struct AVPEEngine AVPEEngine;

// The callbacks run on the engine's thread (seekDone for a seek that a newer AVPEEngineSeek replaced
// before it started runs on the thread of that call). They may run before AVPEEngineCreate returns,
// must not wait for the thread that calls AVPEEngineDestroy and must not call back into the engine.
// None runs after AVPEEngineDestroy returned.
typedef struct {
	void *context;
	// A frame to show now. The buffer is only valid during the call: retain it to keep it.
	void (*frame)(void *context, CVPixelBufferRef pixelBuffer);
	// Playback reached the end of the media; the rate is now 0 and the clock stays at the duration.
	void (*ended)(void *context);
	// The seek identified by token ended: the target frame is shown (finished), or a later seek
	// replaced it (not finished).
	void (*seekDone)(void *context, uint64_t token, bool finished);
} AVPECallbacks;

// Takes ownership of the media. The first frame is decoded and shown without playing.
AVPEEngine *AVPEEngineCreate(AVPEMedia *media, AVPECallbacks callbacks);
void AVPEEngineDestroy(AVPEEngine *engine);

void AVPEEngineSetRate(AVPEEngine *engine, float rate);
float AVPEEngineRate(AVPEEngine *engine);
int64_t AVPEEngineTimeUs(AVPEEngine *engine);
void AVPEEngineSeek(AVPEEngine *engine, int64_t timeUs, uint64_t token);
int64_t AVPEEngineDurationUs(const AVPEEngine *engine);

#ifdef __cplusplus
}
#endif

#endif
