// Copyright (C) 2026 Lubos Dolezel
//
// Plays a 1000 Hz tone through an AudioQueue made at the given sample rate, to the default output device, whose
// own rate may be another one. Prints the nominal rate of the device and "PLAYED <seconds>" when the queue has
// played everything. test_hal_rate_follow.sh records what the device is sent and checks that the pitch and the
// duration are those of the tone, whatever the rates of the queue and the device are.
//
//   test_hal_rate_follow <queue rate> [seconds]
#include <AudioToolbox/AudioToolbox.h>
#include <CoreAudio/CoreAudio.h>
#include <CoreFoundation/CoreFoundation.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static double rate, seconds, phase;
static UInt64 framesLeft;
static volatile int stopped;

static void fill(void* user, AudioQueueRef queue, AudioQueueBufferRef buffer)
{
	const UInt32 capacity = buffer->mAudioDataBytesCapacity / 8;
	Float32* out = buffer->mAudioData;
	UInt32 frames = framesLeft < capacity ? (UInt32) framesLeft : capacity;
	for (UInt32 i = 0; i < frames; i++)
	{
		Float32 sample = 0.5f * sinf((float) phase);
		phase += 2 * M_PI * 1000.0 / rate;
		if (phase > 2 * M_PI)
			phase -= 2 * M_PI;
		out[2 * i] = out[2 * i + 1] = sample;
	}
	framesLeft -= frames;
	buffer->mAudioDataByteSize = frames * 8;
	if (frames)
		AudioQueueEnqueueBuffer(queue, buffer, 0, NULL);
	else if (!stopped)
	{
		stopped = 1;
		AudioQueueStop(queue, false);
	}
}

static void running(void* user, AudioQueueRef queue, AudioQueuePropertyID property)
{
	UInt32 value = 1, size = sizeof(value);
	AudioQueueGetProperty(queue, kAudioQueueProperty_IsRunning, &value, &size);
	if (!value)
	{
		printf("PLAYED\n");
		fflush(stdout);
		CFRunLoopStop(CFRunLoopGetMain());
	}
}

int main(int argc, char** argv)
{
	rate = argc > 1 ? atof(argv[1]) : 44100;
	seconds = argc > 2 ? atof(argv[2]) : 2;
	framesLeft = (UInt64)(rate * seconds);

	AudioObjectPropertyAddress defaultOutput = { kAudioHardwarePropertyDefaultOutputDevice, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
	AudioObjectID device = 0;
	UInt32 size = sizeof(device);
	AudioObjectGetPropertyData(kAudioObjectSystemObject, &defaultOutput, 0, NULL, &size, &device);
	AudioObjectPropertyAddress rateAddress = { kAudioDevicePropertyNominalSampleRate, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
	Float64 deviceRate = 0;
	size = sizeof(deviceRate);
	AudioObjectGetPropertyData(device, &rateAddress, 0, NULL, &size, &deviceRate);
	printf("DEVICE_RATE %g\n", deviceRate);

	AudioStreamBasicDescription format = {};
	format.mSampleRate = rate;
	format.mFormatID = kAudioFormatLinearPCM;
	format.mFormatFlags = kAudioFormatFlagsNativeFloatPacked;
	format.mChannelsPerFrame = 2;
	format.mBitsPerChannel = 32;
	format.mFramesPerPacket = 1;
	format.mBytesPerFrame = format.mBytesPerPacket = 8;

	AudioQueueRef queue;
	if (AudioQueueNewOutput(&format, fill, NULL, NULL, NULL, 0, &queue) != noErr)
	{
		printf("no queue\n");
		return 1;
	}
	AudioQueueAddPropertyListener(queue, kAudioQueueProperty_IsRunning, running, NULL);
	for (int i = 0; i < 3; i++)
	{
		AudioQueueBufferRef buffer;
		AudioQueueAllocateBuffer(queue, (UInt32)(rate / 10) * 8, &buffer);
		fill(NULL, queue, buffer);
	}
	if (AudioQueueStart(queue, NULL) != noErr)
	{
		printf("no start\n");
		return 1;
	}
	printf("STARTED\n");
	fflush(stdout);
	CFRunLoopRunInMode(kCFRunLoopDefaultMode, seconds + 6, false);
	AudioQueueDispose(queue, true);
	return 0;
}
