// Copyright (C) 2026 Lubos Dolezel
//
// AudioQueueCreateTimeline / AudioQueueDisposeTimeline, and
// AudioQueueGetCurrentTime with a timeline: a queue that keeps playing on
// one device reports no discontinuity.
// The program passes unchanged on macOS 26.7.3 (x86_64); without an output
// device there (remote session) it skips the part that plays.
#include <AudioToolbox/AudioToolbox.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)
#define CHECK_EQ(a, b) do { long _a = (long)(a), _b = (long)(b); if (_a != _b) { printf("FAIL %d: %s == %ld, expected %ld\n", __LINE__, #a, _a, _b); g_failures++; } } while (0)

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	fflush(stdout);
	_exit(2);
}

static void output(void* user, AudioQueueRef q, AudioQueueBufferRef b)
{
	(void) user;
	memset(b->mAudioData, 0, b->mAudioDataBytesCapacity);
	b->mAudioDataByteSize = b->mAudioDataBytesCapacity;
	AudioQueueEnqueueBuffer(q, b, 0, NULL);
}

int main(void)
{
	signal(SIGALRM, on_alarm);
	alarm(30);

	AudioStreamBasicDescription f = { 44100, kAudioFormatLinearPCM,
		kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked, 4, 1, 4, 2, 16, 0 };
	AudioQueueRef q = NULL;
	CHECK_EQ(AudioQueueNewOutput(&f, output, NULL, NULL, NULL, 0, &q), noErr);
	if (!q)
		return 1;

	AudioQueueTimelineRef t = NULL;
	CHECK_EQ(AudioQueueCreateTimeline(q, NULL), kAudio_ParamError);
	CHECK_EQ(AudioQueueCreateTimeline(q, &t), noErr);
	CHECK(t != NULL);

	for (int i = 0; i < 3; i++)
	{
		AudioQueueBufferRef b;
		CHECK_EQ(AudioQueueAllocateBuffer(q, 4096, &b), noErr);
		output(NULL, q, b);
	}
	OSStatus started = AudioQueueStart(q, NULL);
	if (started == kAudioQueueErr_InvalidDevice)
	{
		// No output device (e.g. a remote session on a Mac)
		printf("SKIP: no output device, current time not checked\n");
	}
	else
	{
		CHECK_EQ(started, noErr);
		usleep(300000);

		AudioTimeStamp ts;
		Boolean discontinuity = true;
		memset(&ts, 0, sizeof(ts));
		CHECK_EQ(AudioQueueGetCurrentTime(q, t, &ts, &discontinuity), noErr);
		CHECK(!discontinuity);
		CHECK(ts.mFlags & kAudioTimeStampSampleTimeValid);
		CHECK(ts.mSampleTime > 0);
		CHECK_EQ(AudioQueueStop(q, true), noErr);
	}
	CHECK_EQ(AudioQueueDisposeTimeline(q, t), noErr);
	CHECK_EQ(AudioQueueDispose(q, true), noErr);

	if (g_failures)
		printf("FAIL: %d check(s) failed\n", g_failures);
	else
		printf("PASS\n");
	fflush(stdout);
	return g_failures ? 1 : 0;
}
