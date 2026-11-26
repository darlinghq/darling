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

#ifndef _MEDIATOOLBOX_MTAUDIOPROCESSINGTAP_H_
#define _MEDIATOOLBOX_MTAUDIOPROCESSINGTAP_H_

// An audio processing tap: a client's callbacks that see, and may change, the
// audio of a media track as it plays.

#include <CoreFoundation/CoreFoundation.h>
#include <CoreAudio/CoreAudioTypes.h>
#include <CoreMedia/CMBase.h>
#include <CoreMedia/CMTimeRange.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef const struct opaqueMTAudioProcessingTap *MTAudioProcessingTapRef;

CM_EXPORT CFTypeID MTAudioProcessingTapGetTypeID(void);

// where in the audio chain the tap is placed
typedef uint32_t MTAudioProcessingTapCreationFlags;
enum {
    kMTAudioProcessingTapCreationFlag_PreEffects = (1 << 0),
    kMTAudioProcessingTapCreationFlag_PostEffects = (1 << 1),
};

typedef uint32_t MTAudioProcessingTapFlags;
enum {
    kMTAudioProcessingTapFlag_StartOfStream = (1 << 8),
    kMTAudioProcessingTapFlag_EndOfStream = (1 << 9),
};

typedef void (*MTAudioProcessingTapInitCallback)(MTAudioProcessingTapRef tap, void *clientInfo, void **tapStorageOut);
typedef void (*MTAudioProcessingTapFinalizeCallback)(MTAudioProcessingTapRef tap);
typedef void (*MTAudioProcessingTapPrepareCallback)(MTAudioProcessingTapRef tap, CMItemCount maxFrames, const AudioStreamBasicDescription *processingFormat);
typedef void (*MTAudioProcessingTapUnprepareCallback)(MTAudioProcessingTapRef tap);
typedef void (*MTAudioProcessingTapProcessCallback)(MTAudioProcessingTapRef tap, CMItemCount numberFrames, MTAudioProcessingTapFlags flags,
    AudioBufferList *bufferListInOut, CMItemCount *numberFramesOut, MTAudioProcessingTapFlags *flagsOut);

enum {
    kMTAudioProcessingTapCallbacksVersion_0 = 0
};

#pragma pack(push, 4)
typedef struct {
    int version;
    void *clientInfo;
    MTAudioProcessingTapInitCallback init;
    MTAudioProcessingTapFinalizeCallback finalize;
    MTAudioProcessingTapPrepareCallback prepare;
    MTAudioProcessingTapUnprepareCallback unprepare;
    MTAudioProcessingTapProcessCallback process;
} MTAudioProcessingTapCallbacks;
#pragma pack(pop)

CM_EXPORT OSStatus MTAudioProcessingTapCreate(CFAllocatorRef allocator, const MTAudioProcessingTapCallbacks *callbacks,
    MTAudioProcessingTapCreationFlags flags, MTAudioProcessingTapRef *tapOut);

CM_EXPORT void *MTAudioProcessingTapGetStorage(MTAudioProcessingTapRef tap);

CM_EXPORT OSStatus MTAudioProcessingTapGetSourceAudio(MTAudioProcessingTapRef tap, CMItemCount numberFrames,
    AudioBufferList *bufferListInOut, MTAudioProcessingTapFlags *flagsOut, CMTimeRange *timeRangeOut, CMItemCount *numberFramesOut);

#ifdef __cplusplus
}
#endif

#endif
