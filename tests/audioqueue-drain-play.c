/*
 * Plays a 440 Hz tone of one second (stereo, 44.1 kHz, amplitude 0.5) through an AudioQueue, asks the queue to stop once
 * what is enqueued has been played (AudioQueueStop with inImmediate false), and prints how long after
 * the start the queue reported that it stopped running. tests/avaudioplayer-sink.py records the output and checks
 * that all of the second was heard: the queue must not stop the device while it still holds the last of the sound.
 *
 *   guest-cc.sh -a x86_64 -o audioqueue-drain-play tests/audioqueue-drain-play.c -framework AudioToolbox -framework CoreFoundation
 *
 * Printed: "START <time>" and "STOPPED <time>" (CLOCK_REALTIME seconds).
 *
 * Copyright (C) 2026 Lubos Dolezel
 */
#include <AudioToolbox/AudioToolbox.h>
#include <CoreFoundation/CoreFoundation.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static double wall_seconds(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return ts.tv_sec + ts.tv_nsec / 1e9;
}

static volatile int running;

static void on_running(void *user, AudioQueueRef queue, AudioQueuePropertyID property)
{
	UInt32 value = 1, size = sizeof(value);
	AudioQueueGetProperty(queue, kAudioQueueProperty_IsRunning, &value, &size);
	if (!value) {
		printf("STOPPED %.3f\n", wall_seconds());
		running = 0;
	}
}

static void buffer_done(void *user, AudioQueueRef queue, AudioQueueBufferRef buffer)
{
}

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);

	AudioStreamBasicDescription format = {};
	format.mSampleRate = 44100;
	format.mFormatID = kAudioFormatLinearPCM;
	format.mFormatFlags = kAudioFormatFlagsNativeFloatPacked;
	format.mChannelsPerFrame = 2;
	format.mBitsPerChannel = 32;
	format.mFramesPerPacket = 1;
	format.mBytesPerFrame = format.mBytesPerPacket = 8;

	AudioQueueRef queue;
	if (AudioQueueNewOutput(&format, buffer_done, NULL, NULL, NULL, 0, &queue) != noErr) {
		printf("no queue\n");
		return 1;
	}
	AudioQueueAddPropertyListener(queue, kAudioQueueProperty_IsRunning, on_running, NULL);

	enum { buffers = 10, frames = 4410 };
	for (int i = 0; i < buffers; i++) {
		AudioQueueBufferRef buffer;
		AudioQueueAllocateBuffer(queue, frames * 8, &buffer);
		float *samples = buffer->mAudioData;
		for (int k = 0; k < frames; k++) {
			float v = 0.5f * sinf(2 * (float)M_PI * 440 * (i * frames + k) / 44100);
			samples[k * 2] = samples[k * 2 + 1] = v;
		}
		buffer->mAudioDataByteSize = frames * 8;
		AudioQueueEnqueueBuffer(queue, buffer, 0, NULL);
	}

	running = 1;
	printf("START %.3f\n", wall_seconds());
	if (AudioQueueStart(queue, NULL) != noErr) {
		printf("no start\n");
		return 1;
	}
	AudioQueueStop(queue, false);
	for (int i = 0; i < 500 && running; i++)
		usleep(10000);
	AudioQueueDispose(queue, true);
	return running ? 1 : 0;
}
