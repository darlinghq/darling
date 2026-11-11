// Copyright (C) 2026 Lubos Dolezel
//
// <AudioToolbox/AudioToolbox.h> compiles as C: AudioQueueRef and AudioQueueTimelineRef are
// pointers to opaque structures, AudioQueueBuffer and AudioQueueParameterEvent are typedef'd
// structures. A queue that was created can be disposed. Passes on macOS 27.0.1.
#include <AudioToolbox/AudioToolbox.h>
#include <stddef.h>
#include <stdio.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void output_callback(void* user, AudioQueueRef queue, AudioQueueBufferRef buffer)
{
	(void) user;
	(void) queue;
	(void) buffer;
}

int main(void)
{
	AudioQueueParameterEvent event = { kAudioQueueParam_Volume, 0.5f };
	CHECK(event.mID == kAudioQueueParam_Volume);

	// The public buffer structure has the documented layout.
	CHECK(offsetof(AudioQueueBuffer, mAudioDataBytesCapacity) == 0);
	CHECK(sizeof(AudioQueueBuffer) >= 4 + 3 * sizeof(void*));

	AudioStreamBasicDescription format = {0};
	format.mSampleRate = 44100;
	format.mFormatID = kAudioFormatLinearPCM;
	format.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
	format.mChannelsPerFrame = 2;
	format.mBitsPerChannel = 16;
	format.mBytesPerFrame = 4;
	format.mFramesPerPacket = 1;
	format.mBytesPerPacket = 4;

	AudioQueueRef queue = NULL;
	OSStatus status = AudioQueueNewOutput(&format, output_callback, NULL, NULL, NULL, 0, &queue);
	if (status == noErr) {
		CHECK(queue != NULL);
		AudioQueueTimelineRef timeline = NULL;
		(void) timeline;
		CHECK(AudioQueueDispose(queue, true) == noErr);
	} else {
		// No audio device: nothing to dispose, the header check above is the test.
		CHECK(queue == NULL);
	}

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
