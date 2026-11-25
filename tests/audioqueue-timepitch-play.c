/*
 * Plays a stereo tone through an AudioQueue with the time/pitch effect (left 440 Hz, right 660 Hz, 2 s, 44.1 kHz,
 * amplitude 0.5), the way tests/audioqueue-timepitch-sink.py records it:
 *
 *   audioqueue-timepitch-play <algorithm> <rate> <pitch> [change <seconds> <rate>] [bypass]
 *
 * algorithm: spec, tido, vspd (the effect is on) or none. "change" sets another play rate that many seconds after
 * the start; "bypass" bypasses the effect after the queue has started.
 *
 *   guest-cc.sh -a x86_64 -o audioqueue-timepitch-play tests/audioqueue-timepitch-play.c -framework AudioToolbox -framework CoreFoundation
 *
 * Printed: "START <time>", "CT <time> <sample time>" about every 100 ms while playing, and "STOPPED <time>"
 * (CLOCK_REALTIME seconds).
 *
 * Copyright (C) 2026 Lubos Dolezel
 */
#include <AudioToolbox/AudioToolbox.h>
#include <CoreFoundation/CoreFoundation.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
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

int main(int argc, char **argv)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	if (argc < 4) {
		printf("usage\n");
		return 2;
	}
	const char *algorithm = argv[1];
	float rate = (float)atof(argv[2]), pitch = (float)atof(argv[3]);
	double change_at = -1;
	float change_rate = 1;
	int bypass = 0;
	for (int i = 4; i < argc; i++) {
		if (!strcmp(argv[i], "change") && i + 2 < argc) {
			change_at = atof(argv[i + 1]);
			change_rate = (float)atof(argv[i + 2]);
			i += 2;
		} else if (!strcmp(argv[i], "bypass"))
			bypass = 1;
	}

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
	if (strcmp(algorithm, "none")) {
		UInt32 on = 1, alg = !strcmp(algorithm, "tido") ? kAudioQueueTimePitchAlgorithm_TimeDomain
			: !strcmp(algorithm, "vspd") ? kAudioQueueTimePitchAlgorithm_Varispeed : kAudioQueueTimePitchAlgorithm_Spectral;
		if (AudioQueueSetProperty(queue, kAudioQueueProperty_EnableTimePitch, &on, sizeof(on)) != noErr ||
		    AudioQueueSetProperty(queue, kAudioQueueProperty_TimePitchAlgorithm, &alg, sizeof(alg)) != noErr) {
			printf("no time pitch\n");
			return 1;
		}
	}
	AudioQueueSetParameter(queue, kAudioQueueParam_PlayRate, rate);
	AudioQueueSetParameter(queue, kAudioQueueParam_Pitch, pitch);

	running = 1;
	printf("START %.3f\n", wall_seconds());
	if (AudioQueueStart(queue, NULL) != noErr) {
		printf("no start\n");
		return 1;
	}
	if (bypass) {
		UInt32 on = 1;
		if (AudioQueueSetProperty(queue, kAudioQueueProperty_TimePitchBypass, &on, sizeof(on)) != noErr)
			printf("no bypass\n");
	}

	// The output stream is up and the recorder has moved it to its sink before any sound is enqueued.
	usleep(400000);

	enum { buffers = 20, frames = 4410 };
	for (int i = 0; i < buffers; i++) {
		AudioQueueBufferRef buffer;
		AudioQueueAllocateBuffer(queue, frames * 8, &buffer);
		float *samples = buffer->mAudioData;
		for (int k = 0; k < frames; k++) {
			double n = i * frames + k;
			samples[k * 2] = 0.5f * sinf(2 * (float)M_PI * 440 * (float)(n / 44100));
			samples[k * 2 + 1] = 0.5f * sinf(2 * (float)M_PI * 660 * (float)(n / 44100));
		}
		buffer->mAudioDataByteSize = frames * 8;
		AudioQueueEnqueueBuffer(queue, buffer, 0, NULL);
	}
	AudioQueueStop(queue, false);

	const double start = wall_seconds();
	int changed = 0;
	for (int i = 0; i < 1500 && running; i++) {
		usleep(10000);
		if (change_at >= 0 && !changed && wall_seconds() - start >= change_at) {
			AudioQueueSetParameter(queue, kAudioQueueParam_PlayRate, change_rate);
			printf("CHANGE %.3f\n", wall_seconds());
			changed = 1;
		}
		if (i % 10 == 0) {
			AudioTimeStamp stamp;
			Boolean discontinuity;
			if (AudioQueueGetCurrentTime(queue, NULL, &stamp, &discontinuity) == noErr)
				printf("CT %.3f %.0f\n", wall_seconds(), stamp.mSampleTime);
		}
	}
	AudioQueueDispose(queue, true);
	return running ? 1 : 0;
}
