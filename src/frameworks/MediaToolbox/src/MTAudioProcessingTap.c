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

#include <MediaToolbox/MTAudioProcessingTap.h>
#include <CoreFoundation/CFRuntime.h>
#include <dispatch/dispatch.h>

// A tap keeps its callbacks and the storage its init callback gives it. The
// prepare, process and unprepare callbacks are the audio pipeline's to call;
// Darling's MediaToolbox has no audio pipeline that plays through a tap, so it
// never calls them, and there is no source audio to fetch.

struct opaqueMTAudioProcessingTap {
    CFRuntimeBase base;
    MTAudioProcessingTapCallbacks callbacks;
    MTAudioProcessingTapCreationFlags flags;
    void *storage;
};

static void tapFinalize(CFTypeRef object)
{
    struct opaqueMTAudioProcessingTap *tap = (struct opaqueMTAudioProcessingTap *) object;
    if (tap->callbacks.finalize != NULL)
        tap->callbacks.finalize(tap);
}

static CFStringRef tapCopyDescription(CFTypeRef object)
{
    return CFStringCreateWithFormat(CFGetAllocator(object), NULL, CFSTR("<MTAudioProcessingTap %p>"), object);
}

static const CFRuntimeClass tapClass = {
    .version = 0,
    .className = "MTAudioProcessingTap",
    .finalize = tapFinalize,
    .copyDebugDesc = tapCopyDescription,
};

CFTypeID MTAudioProcessingTapGetTypeID(void)
{
    static CFTypeID typeID = _kCFRuntimeNotATypeID;
    static dispatch_once_t once;
    dispatch_once(&once, ^{
        typeID = _CFRuntimeRegisterClass(&tapClass);
    });
    return typeID;
}

OSStatus MTAudioProcessingTapCreate(CFAllocatorRef allocator, const MTAudioProcessingTapCallbacks *callbacks,
    MTAudioProcessingTapCreationFlags flags, MTAudioProcessingTapRef *tapOut)
{
    // The process callback is required, and version 0 is the only one there is.
    if (tapOut == NULL)
        return kAudio_ParamError;
    *tapOut = NULL;
    if (callbacks == NULL || callbacks->version != kMTAudioProcessingTapCallbacksVersion_0 || callbacks->process == NULL)
        return kAudio_ParamError;

    struct opaqueMTAudioProcessingTap *tap = (struct opaqueMTAudioProcessingTap *) _CFRuntimeCreateInstance(allocator,
        MTAudioProcessingTapGetTypeID(), sizeof(*tap) - sizeof(CFRuntimeBase), NULL);
    if (tap == NULL)
        return kAudio_MemFullError;
    tap->callbacks = *callbacks;
    tap->flags = flags;
    tap->storage = NULL;
    if (callbacks->init != NULL)
        callbacks->init(tap, callbacks->clientInfo, &tap->storage);
    *tapOut = tap;
    return noErr;
}

void *MTAudioProcessingTapGetStorage(MTAudioProcessingTapRef tap)
{
    return tap != NULL ? tap->storage : NULL;
}

OSStatus MTAudioProcessingTapGetSourceAudio(MTAudioProcessingTapRef tap, CMItemCount numberFrames,
    AudioBufferList *bufferListInOut, MTAudioProcessingTapFlags *flagsOut, CMTimeRange *timeRangeOut, CMItemCount *numberFramesOut)
{
    (void) numberFrames;
    (void) timeRangeOut;
    if (tap == NULL || bufferListInOut == NULL)
        return kAudio_ParamError;
    // Source audio is only there while the pipeline is running the process
    // callback, which Darling's never does.
    if (numberFramesOut != NULL)
        *numberFramesOut = 0;
    if (flagsOut != NULL)
        *flagsOut = 0;
    return kAudio_ParamError;
}
