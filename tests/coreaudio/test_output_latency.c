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

// While a device plays, the output latency it reports must match how far ahead of playback its
// IOProc runs: inOutputTime is when the first frame an IOProc renders will be played, inNow is the present.
// Needs an installed build and a PulseAudio sink: run it inside Darling. Exits non-zero on failure.
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <CoreAudio/CoreAudio.h>

static volatile double g_leadFrames = -1;
static volatile int g_cycles = 0;

static OSStatus ioProc(AudioObjectID device, const AudioTimeStamp* now, const AudioBufferList* inputData,
	const AudioTimeStamp* inputTime, AudioBufferList* outputData, const AudioTimeStamp* outputTime, void* context)
{
	for (UInt32 i = 0; i < outputData->mNumberBuffers; i++)
		memset(outputData->mBuffers[i].mData, 0, outputData->mBuffers[i].mDataByteSize);

	if ((now->mFlags & kAudioTimeStampSampleTimeValid) && (outputTime->mFlags & kAudioTimeStampSampleTimeValid))
		g_leadFrames = outputTime->mSampleTime - now->mSampleTime;
	g_cycles++;
	return noErr;
}

static UInt32 getFrames(AudioObjectID device, AudioObjectPropertySelector selector, AudioObjectPropertyScope scope, OSStatus* status)
{
	AudioObjectPropertyAddress addr = { selector, scope, kAudioObjectPropertyElementMaster };
	UInt32 value = 0, size = sizeof(value);
	*status = AudioObjectGetPropertyData(device, &addr, 0, NULL, &size, &value);
	return value;
}

int main(void)
{
	AudioObjectID device = kAudioObjectUnknown;
	AudioObjectPropertyAddress defaultAddr = { kAudioHardwarePropertyDefaultOutputDevice, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
	UInt32 size = sizeof(device);
	if (AudioObjectGetPropertyData(kAudioObjectSystemObject, &defaultAddr, 0, NULL, &size, &device) != noErr || device == kAudioObjectUnknown)
	{
		fprintf(stderr, "FAIL: no default output device\n");
		return 1;
	}

	OSStatus s1, s2, s3, s4;
	getFrames(device, kAudioDevicePropertyLatency, kAudioObjectPropertyScopeOutput, &s1);
	getFrames(device, kAudioDevicePropertySafetyOffset, kAudioObjectPropertyScopeOutput, &s2);
	const UInt32 bufferFrames = getFrames(device, kAudioDevicePropertyBufferFrameSize, kAudioObjectPropertyScopeOutput, &s3);
	const UInt32 inputLatency = getFrames(device, kAudioDevicePropertyLatency, kAudioObjectPropertyScopeInput, &s4);
	if (s1 != noErr || s2 != noErr || s3 != noErr || s4 != noErr)
	{
		fprintf(stderr, "FAIL: latency properties not readable (%d %d %d %d)\n", (int)s1, (int)s2, (int)s3, (int)s4);
		return 1;
	}
	if (inputLatency != 0)
	{
		fprintf(stderr, "FAIL: an output device reports %u frames of input latency\n", (unsigned)inputLatency);
		return 1;
	}

	AudioDeviceIOProcID procID = NULL;
	if (AudioDeviceCreateIOProcID(device, ioProc, NULL, &procID) != noErr || AudioDeviceStart(device, procID) != noErr)
	{
		fprintf(stderr, "FAIL: could not start the device\n");
		return 1;
	}
	// Let the stream settle past its startup.
	for (int i = 0; i < 200 && g_cycles < 30; i++)
		usleep(10000);
	// The server decides the latency once the stream is connected.
	const UInt32 latency = getFrames(device, kAudioDevicePropertyLatency, kAudioObjectPropertyScopeOutput, &s1);
	const UInt32 safety = getFrames(device, kAudioDevicePropertySafetyOffset, kAudioObjectPropertyScopeOutput, &s2);
	AudioDeviceStop(device, procID);
	AudioDeviceDestroyIOProcID(device, procID);

	if (s1 != noErr || s2 != noErr)
	{
		fprintf(stderr, "FAIL: latency properties not readable while running (%d %d)\n", (int)s1, (int)s2);
		return 1;
	}
	if (g_cycles < 30 || g_leadFrames < 0)
	{
		fprintf(stderr, "FAIL: the IOProc ran %d times without valid timestamps\n", g_cycles);
		return 1;
	}

	// The server only approximates the latency asked of it, so allow two IO buffers either way.
	const double reported = latency + safety;
	if (g_leadFrames < reported - 2.0 * bufferFrames || g_leadFrames > reported + 2.0 * bufferFrames)
	{
		fprintf(stderr, "FAIL: the device reports %.0f frames of latency, but its IOProc runs %.0f frames ahead\n",
			reported, g_leadFrames);
		return 1;
	}

	printf("test_output_latency: OK (reported %.0f frames, IOProc %.0f frames ahead, buffer %u)\n",
		reported, g_leadFrames, (unsigned)bufferFrames);
	return 0;
}
