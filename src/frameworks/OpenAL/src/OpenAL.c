/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

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


#include <OpenAL/OpenAL.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* alBuffer3f(void)
{
    if (verbose) puts("STUB: alBuffer3f called");
    return NULL;
}

void* alBuffer3i(void)
{
    if (verbose) puts("STUB: alBuffer3i called");
    return NULL;
}

void* alBufferData(void)
{
    if (verbose) puts("STUB: alBufferData called");
    return NULL;
}

void* alBufferDataStatic(void)
{
    if (verbose) puts("STUB: alBufferDataStatic called");
    return NULL;
}

void* alBufferf(void)
{
    if (verbose) puts("STUB: alBufferf called");
    return NULL;
}

void* alBufferfv(void)
{
    if (verbose) puts("STUB: alBufferfv called");
    return NULL;
}

void* alBufferi(void)
{
    if (verbose) puts("STUB: alBufferi called");
    return NULL;
}

void* alBufferiv(void)
{
    if (verbose) puts("STUB: alBufferiv called");
    return NULL;
}

void* alDeleteBuffers(void)
{
    if (verbose) puts("STUB: alDeleteBuffers called");
    return NULL;
}

void* alDeleteSources(void)
{
    if (verbose) puts("STUB: alDeleteSources called");
    return NULL;
}

void* alDisable(void)
{
    if (verbose) puts("STUB: alDisable called");
    return NULL;
}

void* alDistanceModel(void)
{
    if (verbose) puts("STUB: alDistanceModel called");
    return NULL;
}

void* alDopplerFactor(void)
{
    if (verbose) puts("STUB: alDopplerFactor called");
    return NULL;
}

void* alDopplerVelocity(void)
{
    if (verbose) puts("STUB: alDopplerVelocity called");
    return NULL;
}

void* alEnable(void)
{
    if (verbose) puts("STUB: alEnable called");
    return NULL;
}

void* alGenBuffers(void)
{
    if (verbose) puts("STUB: alGenBuffers called");
    return NULL;
}

void* alGenSources(void)
{
    if (verbose) puts("STUB: alGenSources called");
    return NULL;
}

void* alGetBoolean(void)
{
    if (verbose) puts("STUB: alGetBoolean called");
    return NULL;
}

void* alGetBooleanv(void)
{
    if (verbose) puts("STUB: alGetBooleanv called");
    return NULL;
}

void* alGetBuffer3f(void)
{
    if (verbose) puts("STUB: alGetBuffer3f called");
    return NULL;
}

void* alGetBuffer3i(void)
{
    if (verbose) puts("STUB: alGetBuffer3i called");
    return NULL;
}

void* alGetBufferf(void)
{
    if (verbose) puts("STUB: alGetBufferf called");
    return NULL;
}

void* alGetBufferfv(void)
{
    if (verbose) puts("STUB: alGetBufferfv called");
    return NULL;
}

void* alGetBufferi(void)
{
    if (verbose) puts("STUB: alGetBufferi called");
    return NULL;
}

void* alGetBufferiv(void)
{
    if (verbose) puts("STUB: alGetBufferiv called");
    return NULL;
}

void* alGetDouble(void)
{
    if (verbose) puts("STUB: alGetDouble called");
    return NULL;
}

void* alGetDoublev(void)
{
    if (verbose) puts("STUB: alGetDoublev called");
    return NULL;
}

void* alGetEnumValue(void)
{
    if (verbose) puts("STUB: alGetEnumValue called");
    return NULL;
}

void* alGetError(void)
{
    if (verbose) puts("STUB: alGetError called");
    return NULL;
}

void* alGetFloat(void)
{
    if (verbose) puts("STUB: alGetFloat called");
    return NULL;
}

void* alGetFloatv(void)
{
    if (verbose) puts("STUB: alGetFloatv called");
    return NULL;
}

void* alGetInteger(void)
{
    if (verbose) puts("STUB: alGetInteger called");
    return NULL;
}

void* alGetIntegerv(void)
{
    if (verbose) puts("STUB: alGetIntegerv called");
    return NULL;
}

void* alGetListener3f(void)
{
    if (verbose) puts("STUB: alGetListener3f called");
    return NULL;
}

void* alGetListener3i(void)
{
    if (verbose) puts("STUB: alGetListener3i called");
    return NULL;
}

void* alGetListenerf(void)
{
    if (verbose) puts("STUB: alGetListenerf called");
    return NULL;
}

void* alGetListenerfv(void)
{
    if (verbose) puts("STUB: alGetListenerfv called");
    return NULL;
}

void* alGetListeneri(void)
{
    if (verbose) puts("STUB: alGetListeneri called");
    return NULL;
}

void* alGetListeneriv(void)
{
    if (verbose) puts("STUB: alGetListeneriv called");
    return NULL;
}

void* alGetProcAddress(void)
{
    if (verbose) puts("STUB: alGetProcAddress called");
    return NULL;
}

void* alGetSource3f(void)
{
    if (verbose) puts("STUB: alGetSource3f called");
    return NULL;
}

void* alGetSource3i(void)
{
    if (verbose) puts("STUB: alGetSource3i called");
    return NULL;
}

void* alGetSourcef(void)
{
    if (verbose) puts("STUB: alGetSourcef called");
    return NULL;
}

void* alGetSourcefv(void)
{
    if (verbose) puts("STUB: alGetSourcefv called");
    return NULL;
}

void* alGetSourcei(void)
{
    if (verbose) puts("STUB: alGetSourcei called");
    return NULL;
}

void* alGetSourceiv(void)
{
    if (verbose) puts("STUB: alGetSourceiv called");
    return NULL;
}

void* alGetString(void)
{
    if (verbose) puts("STUB: alGetString called");
    return NULL;
}

void* alHint(void)
{
    if (verbose) puts("STUB: alHint called");
    return NULL;
}

void* alIsBuffer(void)
{
    if (verbose) puts("STUB: alIsBuffer called");
    return NULL;
}

void* alIsEnabled(void)
{
    if (verbose) puts("STUB: alIsEnabled called");
    return NULL;
}

void* alIsExtensionPresent(void)
{
    if (verbose) puts("STUB: alIsExtensionPresent called");
    return NULL;
}

void* alIsSource(void)
{
    if (verbose) puts("STUB: alIsSource called");
    return NULL;
}

void* alListener3f(void)
{
    if (verbose) puts("STUB: alListener3f called");
    return NULL;
}

void* alListener3i(void)
{
    if (verbose) puts("STUB: alListener3i called");
    return NULL;
}

void* alListenerf(void)
{
    if (verbose) puts("STUB: alListenerf called");
    return NULL;
}

void* alListenerfv(void)
{
    if (verbose) puts("STUB: alListenerfv called");
    return NULL;
}

void* alListeneri(void)
{
    if (verbose) puts("STUB: alListeneri called");
    return NULL;
}

void* alListeneriv(void)
{
    if (verbose) puts("STUB: alListeneriv called");
    return NULL;
}

void* alMacOSXGetRenderChannelCount(void)
{
    if (verbose) puts("STUB: alMacOSXGetRenderChannelCount called");
    return NULL;
}

void* alMacOSXRenderChannelCount(void)
{
    if (verbose) puts("STUB: alMacOSXRenderChannelCount called");
    return NULL;
}

void* alSetDouble(void)
{
    if (verbose) puts("STUB: alSetDouble called");
    return NULL;
}

void* alSetInteger(void)
{
    if (verbose) puts("STUB: alSetInteger called");
    return NULL;
}

void* alSource3f(void)
{
    if (verbose) puts("STUB: alSource3f called");
    return NULL;
}

void* alSource3i(void)
{
    if (verbose) puts("STUB: alSource3i called");
    return NULL;
}

void* alSourcePause(void)
{
    if (verbose) puts("STUB: alSourcePause called");
    return NULL;
}

void* alSourcePausev(void)
{
    if (verbose) puts("STUB: alSourcePausev called");
    return NULL;
}

void* alSourcePlay(void)
{
    if (verbose) puts("STUB: alSourcePlay called");
    return NULL;
}

void* alSourcePlayv(void)
{
    if (verbose) puts("STUB: alSourcePlayv called");
    return NULL;
}

void* alSourceQueueBuffers(void)
{
    if (verbose) puts("STUB: alSourceQueueBuffers called");
    return NULL;
}

void* alSourceRewind(void)
{
    if (verbose) puts("STUB: alSourceRewind called");
    return NULL;
}

void* alSourceRewindv(void)
{
    if (verbose) puts("STUB: alSourceRewindv called");
    return NULL;
}

void* alSourceStop(void)
{
    if (verbose) puts("STUB: alSourceStop called");
    return NULL;
}

void* alSourceStopv(void)
{
    if (verbose) puts("STUB: alSourceStopv called");
    return NULL;
}

void* alSourceUnqueueBuffers(void)
{
    if (verbose) puts("STUB: alSourceUnqueueBuffers called");
    return NULL;
}

void* alSourcef(void)
{
    if (verbose) puts("STUB: alSourcef called");
    return NULL;
}

void* alSourcefv(void)
{
    if (verbose) puts("STUB: alSourcefv called");
    return NULL;
}

void* alSourcei(void)
{
    if (verbose) puts("STUB: alSourcei called");
    return NULL;
}

void* alSourceiv(void)
{
    if (verbose) puts("STUB: alSourceiv called");
    return NULL;
}

void* alSpeedOfSound(void)
{
    if (verbose) puts("STUB: alSpeedOfSound called");
    return NULL;
}

void* alcASAGetListener(void)
{
    if (verbose) puts("STUB: alcASAGetListener called");
    return NULL;
}

void* alcASAGetSource(void)
{
    if (verbose) puts("STUB: alcASAGetSource called");
    return NULL;
}

void* alcASASetListener(void)
{
    if (verbose) puts("STUB: alcASASetListener called");
    return NULL;
}

void* alcASASetSource(void)
{
    if (verbose) puts("STUB: alcASASetSource called");
    return NULL;
}

void* alcCaptureCloseDevice(void)
{
    if (verbose) puts("STUB: alcCaptureCloseDevice called");
    return NULL;
}

void* alcCaptureOpenDevice(void)
{
    if (verbose) puts("STUB: alcCaptureOpenDevice called");
    return NULL;
}

void* alcCaptureSamples(void)
{
    if (verbose) puts("STUB: alcCaptureSamples called");
    return NULL;
}

void* alcCaptureStart(void)
{
    if (verbose) puts("STUB: alcCaptureStart called");
    return NULL;
}

void* alcCaptureStop(void)
{
    if (verbose) puts("STUB: alcCaptureStop called");
    return NULL;
}

void* alcCloseDevice(void)
{
    if (verbose) puts("STUB: alcCloseDevice called");
    return NULL;
}

void* alcCreateContext(void)
{
    if (verbose) puts("STUB: alcCreateContext called");
    return NULL;
}

void* alcDestroyContext(void)
{
    if (verbose) puts("STUB: alcDestroyContext called");
    return NULL;
}

void* alcGetContextsDevice(void)
{
    if (verbose) puts("STUB: alcGetContextsDevice called");
    return NULL;
}

void* alcGetCurrentContext(void)
{
    if (verbose) puts("STUB: alcGetCurrentContext called");
    return NULL;
}

void* alcGetEnumValue(void)
{
    if (verbose) puts("STUB: alcGetEnumValue called");
    return NULL;
}

void* alcGetError(void)
{
    if (verbose) puts("STUB: alcGetError called");
    return NULL;
}

void* alcGetIntegerv(void)
{
    if (verbose) puts("STUB: alcGetIntegerv called");
    return NULL;
}

void* alcGetProcAddress(void)
{
    if (verbose) puts("STUB: alcGetProcAddress called");
    return NULL;
}

void* alcGetString(void)
{
    if (verbose) puts("STUB: alcGetString called");
    return NULL;
}

void* alcIsExtensionPresent(void)
{
    if (verbose) puts("STUB: alcIsExtensionPresent called");
    return NULL;
}

void* alcMacOSXGetMixerMaxiumumBusses(void)
{
    if (verbose) puts("STUB: alcMacOSXGetMixerMaxiumumBusses called");
    return NULL;
}

void* alcMacOSXGetMixerOutputRate(void)
{
    if (verbose) puts("STUB: alcMacOSXGetMixerOutputRate called");
    return NULL;
}

void* alcMacOSXGetRenderingQuality(void)
{
    if (verbose) puts("STUB: alcMacOSXGetRenderingQuality called");
    return NULL;
}

void* alcMacOSXMixerMaxiumumBusses(void)
{
    if (verbose) puts("STUB: alcMacOSXMixerMaxiumumBusses called");
    return NULL;
}

void* alcMacOSXMixerOutputRate(void)
{
    if (verbose) puts("STUB: alcMacOSXMixerOutputRate called");
    return NULL;
}

void* alcMacOSXRenderingQuality(void)
{
    if (verbose) puts("STUB: alcMacOSXRenderingQuality called");
    return NULL;
}

void* alcMakeContextCurrent(void)
{
    if (verbose) puts("STUB: alcMakeContextCurrent called");
    return NULL;
}

void* alcOpenDevice(void)
{
    if (verbose) puts("STUB: alcOpenDevice called");
    return NULL;
}

void* alcProcessContext(void)
{
    if (verbose) puts("STUB: alcProcessContext called");
    return NULL;
}

void* alcSuspendContext(void)
{
    if (verbose) puts("STUB: alcSuspendContext called");
    return NULL;
}

void* alutExit(void)
{
    if (verbose) puts("STUB: alutExit called");
    return NULL;
}

void* alutInit(void)
{
    if (verbose) puts("STUB: alutInit called");
    return NULL;
}

void* alutLoadWAVFile(void)
{
    if (verbose) puts("STUB: alutLoadWAVFile called");
    return NULL;
}

void* alutLoadWAVMemory(void)
{
    if (verbose) puts("STUB: alutLoadWAVMemory called");
    return NULL;
}

void* alutUnloadWAV(void)
{
    if (verbose) puts("STUB: alutUnloadWAV called");
    return NULL;
}
