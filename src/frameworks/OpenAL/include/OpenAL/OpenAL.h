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


#ifndef _OpenAL_H_
#define _OpenAL_H_

void* alBuffer3f(void);
void* alBuffer3i(void);
void* alBufferData(void);
void* alBufferDataStatic(void);
void* alBufferf(void);
void* alBufferfv(void);
void* alBufferi(void);
void* alBufferiv(void);
void* alDeleteBuffers(void);
void* alDeleteSources(void);
void* alDisable(void);
void* alDistanceModel(void);
void* alDopplerFactor(void);
void* alDopplerVelocity(void);
void* alEnable(void);
void* alGenBuffers(void);
void* alGenSources(void);
void* alGetBoolean(void);
void* alGetBooleanv(void);
void* alGetBuffer3f(void);
void* alGetBuffer3i(void);
void* alGetBufferf(void);
void* alGetBufferfv(void);
void* alGetBufferi(void);
void* alGetBufferiv(void);
void* alGetDouble(void);
void* alGetDoublev(void);
void* alGetEnumValue(void);
void* alGetError(void);
void* alGetFloat(void);
void* alGetFloatv(void);
void* alGetInteger(void);
void* alGetIntegerv(void);
void* alGetListener3f(void);
void* alGetListener3i(void);
void* alGetListenerf(void);
void* alGetListenerfv(void);
void* alGetListeneri(void);
void* alGetListeneriv(void);
void* alGetProcAddress(void);
void* alGetSource3f(void);
void* alGetSource3i(void);
void* alGetSourcef(void);
void* alGetSourcefv(void);
void* alGetSourcei(void);
void* alGetSourceiv(void);
void* alGetString(void);
void* alHint(void);
void* alIsBuffer(void);
void* alIsEnabled(void);
void* alIsExtensionPresent(void);
void* alIsSource(void);
void* alListener3f(void);
void* alListener3i(void);
void* alListenerf(void);
void* alListenerfv(void);
void* alListeneri(void);
void* alListeneriv(void);
void* alMacOSXGetRenderChannelCount(void);
void* alMacOSXRenderChannelCount(void);
void* alSetDouble(void);
void* alSetInteger(void);
void* alSource3f(void);
void* alSource3i(void);
void* alSourcePause(void);
void* alSourcePausev(void);
void* alSourcePlay(void);
void* alSourcePlayv(void);
void* alSourceQueueBuffers(void);
void* alSourceRewind(void);
void* alSourceRewindv(void);
void* alSourceStop(void);
void* alSourceStopv(void);
void* alSourceUnqueueBuffers(void);
void* alSourcef(void);
void* alSourcefv(void);
void* alSourcei(void);
void* alSourceiv(void);
void* alSpeedOfSound(void);
void* alcASAGetListener(void);
void* alcASAGetSource(void);
void* alcASASetListener(void);
void* alcASASetSource(void);
void* alcCaptureCloseDevice(void);
void* alcCaptureOpenDevice(void);
void* alcCaptureSamples(void);
void* alcCaptureStart(void);
void* alcCaptureStop(void);
void* alcCloseDevice(void);
void* alcCreateContext(void);
void* alcDestroyContext(void);
void* alcGetContextsDevice(void);
void* alcGetCurrentContext(void);
void* alcGetEnumValue(void);
void* alcGetError(void);
void* alcGetIntegerv(void);
void* alcGetProcAddress(void);
void* alcGetString(void);
void* alcIsExtensionPresent(void);
void* alcMacOSXGetMixerMaxiumumBusses(void);
void* alcMacOSXGetMixerOutputRate(void);
void* alcMacOSXGetRenderingQuality(void);
void* alcMacOSXMixerMaxiumumBusses(void);
void* alcMacOSXMixerOutputRate(void);
void* alcMacOSXRenderingQuality(void);
void* alcMakeContextCurrent(void);
void* alcOpenDevice(void);
void* alcProcessContext(void);
void* alcSuspendContext(void);
void* alutExit(void);
void* alutInit(void);
void* alutLoadWAVFile(void);
void* alutLoadWAVMemory(void);
void* alutUnloadWAV(void);

#endif
