// Copyright (C) 2026 Lubos Dolezel
//
// AudioQueue output queues: property listeners (IsRunning), the common queue
// properties and parameters, and AudioQueueEnqueueBufferWithParameters (trimming,
// start times, volume/pan events), checked by rendering offline so that no audio
// device is needed. Expected values were observed on macOS 26.7.3 (x86_64) and
// 10.14.4 (x86_64 and i386); a check marked QUIRK covers behaviour the headers do
// not state.
#include <AudioToolbox/AudioToolbox.h>
#include <CoreAudio/CoreAudio.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)
#define CHECK_EQ(a, b) do { long _a = (long)(a), _b = (long)(b); if (_a != _b) { printf("FAIL %d: %s == %ld, expected %ld\n", __LINE__, #a, _a, _b); g_failures++; } } while (0)
#define CHECK_NEAR(a, b, tol) do { double _a = (a), _b = (b); if (fabs(_a - _b) > (tol)) { printf("FAIL %d: %s == %.4f, expected %.4f\n", __LINE__, #a, _a, _b); g_failures++; } } while (0)

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	fflush(stdout);
	_exit(2);
}

static AudioStreamBasicDescription queue_format(void)
{
	AudioStreamBasicDescription f;
	memset(&f, 0, sizeof(f));
	f.mSampleRate = 44100;
	f.mFormatID = kAudioFormatLinearPCM;
	f.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;
	f.mChannelsPerFrame = 2;
	f.mBitsPerChannel = 32;
	f.mFramesPerPacket = 1;
	f.mBytesPerFrame = 8;
	f.mBytesPerPacket = 8;
	return f;
}

static void returned(void* user, AudioQueueRef q, AudioQueueBufferRef b)
{
	(void) q; (void) b;
	__sync_fetch_and_add((int*) user, 1);
}

#define NF 1000

// A queue rendering offline; the source is L = 1 + i/1000, R = -1 for 1000 frames.
static AudioQueueRef new_queue(int* callbacks, AudioQueueBufferRef* buf)
{
	AudioStreamBasicDescription f = queue_format();
	AudioQueueRef q = NULL;
	CHECK_EQ(AudioQueueNewOutput(&f, returned, callbacks, NULL, NULL, 0, &q), noErr);
	if (!q)
		return NULL;
	CHECK_EQ(AudioQueueSetOfflineRenderFormat(q, &f, NULL), noErr);
	if (buf)
	{
		CHECK_EQ(AudioQueueAllocateBuffer(q, NF * 8, buf), noErr);
		float* p = (float*) (*buf)->mAudioData;
		for (int i = 0; i < NF; i++)
		{
			p[2 * i] = 1.0f + i / 1000.0f;
			p[2 * i + 1] = -1.0f;
		}
		(*buf)->mAudioDataByteSize = NF * 8;
	}
	return q;
}

typedef struct
{
	int frames;		// frames rendered (the offline render's byte size)
	int nonzero, first;
	float left[1500];
	float right[1500];
} Result;

static void render(AudioQueueRef q, int want, Result* r)
{
	AudioQueueBufferRef ob;
	AudioTimeStamp ts;
	memset(&ts, 0, sizeof(ts));
	ts.mFlags = kAudioTimeStampSampleTimeValid;
	memset(r, 0, sizeof(*r));
	r->first = -1;
	CHECK_EQ(AudioQueueStart(q, NULL), noErr);
	CHECK_EQ(AudioQueueAllocateBuffer(q, want * 8, &ob), noErr);
	CHECK_EQ(AudioQueueOfflineRender(q, &ts, ob, want), noErr);
	r->frames = ob->mAudioDataByteSize / 8;
	float* o = (float*) ob->mAudioData;
	for (int i = 0; i < r->frames && i < 1500; i++)
	{
		r->left[i] = o[2 * i];
		r->right[i] = o[2 * i + 1];
		if (o[2 * i] != 0 || o[2 * i + 1] != 0)
		{
			if (r->first < 0)
				r->first = i;
			r->nonzero++;
		}
	}
	AudioQueueStop(q, true);
}

static void one(UInt32 trimStart, UInt32 trimEnd, UInt32 np, const AudioQueueParameterEvent* ev,
		const AudioTimeStamp* start, AudioTimeStamp* actual, Result* r)
{
	int callbacks = 0;
	AudioQueueBufferRef b;
	AudioQueueRef q = new_queue(&callbacks, &b);
	if (!q)
	{
		memset(r, 0, sizeof(*r));
		return;
	}
	CHECK_EQ(AudioQueueEnqueueBufferWithParameters(q, b, 0, NULL, trimStart, trimEnd, np, ev, start, actual), noErr);
	render(q, 1500, r);
	AudioQueueDispose(q, true);
}

static AudioTimeStamp sample_time(double t)
{
	AudioTimeStamp ts;
	memset(&ts, 0, sizeof(ts));
	ts.mFlags = kAudioTimeStampSampleTimeValid;
	ts.mSampleTime = t;
	return ts;
}

static void test_trim_and_start_times(void)
{
	Result r;

	one(0, 0, 0, NULL, NULL, NULL, &r);
	CHECK_EQ(r.frames, 1000);
	CHECK_NEAR(r.left[0], 1.0, 1e-4);
	CHECK_NEAR(r.left[999], 1.999, 1e-4);

	one(100, 50, 0, NULL, NULL, NULL, &r);
	CHECK_EQ(r.frames, 850);
	CHECK_NEAR(r.left[0], 1.1, 1e-4);
	CHECK_NEAR(r.left[849], 1.949, 1e-4);

	one(100, 0, 0, NULL, NULL, NULL, &r);
	CHECK_EQ(r.frames, 900);
	CHECK_NEAR(r.left[0], 1.1, 1e-4);

	one(0, 100, 0, NULL, NULL, NULL, &r);
	CHECK_EQ(r.frames, 900);
	CHECK_NEAR(r.left[899], 1.899, 1e-4);

	// Everything trimmed: nothing to play.
	one(500, 500, 0, NULL, NULL, NULL, &r);
	CHECK_EQ(r.frames, 0);
	one(1000, 0, 0, NULL, NULL, NULL, &r);
	CHECK_EQ(r.frames, 0);

	// QUIRK (test_quirk_trim_larger_than_buffer_is_ignored): trims that add up to more
	// than the buffer holds leave it untouched.
	one(600, 500, 0, NULL, NULL, NULL, &r);
	CHECK_EQ(r.frames, 1000);
	CHECK_NEAR(r.left[0], 1.0, 1e-4);
	one(2000, 0, 0, NULL, NULL, NULL, &r);
	CHECK_EQ(r.frames, 1000);
	CHECK_NEAR(r.left[0], 1.0, 1e-4);

	// A start time later than the queue's position: silence first.
	AudioTimeStamp st = sample_time(500);
	one(0, 0, 0, NULL, &st, NULL, &r);
	CHECK_EQ(r.frames, 1500);
	CHECK_EQ(r.first, 500);
	CHECK_EQ(r.nonzero, 1000);
	CHECK_NEAR(r.left[500], 1.0, 1e-4);
	CHECK_NEAR(r.left[1499], 1.999, 1e-4);

	// A start time at the queue's position changes nothing.
	st = sample_time(0);
	one(0, 0, 0, NULL, &st, NULL, &r);
	CHECK_EQ(r.frames, 1000);
	CHECK_EQ(r.first, 0);

	// QUIRK (test_quirk_start_time_in_the_past_drops_frames)
	st = sample_time(-100);
	one(0, 0, 0, NULL, &st, NULL, &r);
	CHECK_EQ(r.frames, 900);
	CHECK_NEAR(r.left[0], 1.1, 1e-4);

	// Trim, start time and a volume event together: the start time is that of the trimmed audio.
	AudioQueueParameterEvent half = { kAudioQueueParam_Volume, 0.5f };
	st = sample_time(200);
	one(100, 100, 1, &half, &st, NULL, &r);
	CHECK_EQ(r.frames, 1000);
	CHECK_EQ(r.first, 200);
	CHECK_EQ(r.nonzero, 800);
	CHECK_NEAR(r.left[499], 0.6995, 1e-3);
	CHECK_NEAR(r.left[999], 0.9495, 1e-3);

	// QUIRK (test_quirk_actual_start_time_echoes_request): the actual start time is either
	// zero (macOS 26.7.3) or the requested sample time with the valid flag (10.14.4).
	AudioTimeStamp actual;
	st = sample_time(500);
	memset(&actual, 0xAA, sizeof(actual));
	one(0, 0, 0, NULL, &st, &actual, &r);
	CHECK((actual.mFlags == 0 && actual.mSampleTime == 0) || (actual.mFlags == kAudioTimeStampSampleTimeValid && actual.mSampleTime == 500));
	memset(&actual, 0xAA, sizeof(actual));
	one(0, 0, 0, NULL, NULL, &actual, &r);
	CHECK_EQ(actual.mFlags, 0);
	CHECK_EQ(actual.mSampleTime, 0);
}

static void test_events(void)
{
	Result r;
	AudioQueueParameterEvent ev[2];

	ev[0].mID = kAudioQueueParam_Volume; ev[0].mValue = 0.5f;
	one(0, 0, 1, ev, NULL, NULL, &r);
	CHECK_EQ(r.frames, 1000);
	CHECK_NEAR(r.left[0], 0.5, 1e-3);
	CHECK_NEAR(r.left[500], 0.75, 1e-3);
	CHECK_NEAR(r.right[0], -0.5, 1e-3);

	// The last of several volume events wins.
	ev[1].mID = kAudioQueueParam_Volume; ev[1].mValue = 0.25f;
	one(0, 0, 2, ev, NULL, NULL, &r);
	CHECK_NEAR(r.left[0], 0.25, 1e-3);
	CHECK_NEAR(r.left[500], 0.375, 1e-3);

	// Values outside 0..1 are applied as they are.
	ev[0].mValue = 2.0f;
	one(0, 0, 1, ev, NULL, NULL, &r);
	CHECK_NEAR(r.left[0], 2.0, 1e-3);
	ev[0].mValue = -1.0f;
	one(0, 0, 1, ev, NULL, NULL, &r);
	CHECK_NEAR(r.left[0], -1.0, 1e-3);

	// Pan: a positive value lowers the left channel, a negative one the right.
	ev[0].mID = kAudioQueueParam_Pan; ev[0].mValue = 1.0f;
	one(0, 0, 1, ev, NULL, NULL, &r);
	CHECK_NEAR(r.left[0], 0.0, 1e-3);
	CHECK_NEAR(r.right[0], -1.0, 1e-3);
	ev[0].mValue = -1.0f;
	one(0, 0, 1, ev, NULL, NULL, &r);
	CHECK_NEAR(r.left[0], 1.0, 1e-3);
	CHECK_NEAR(r.right[0], 0.0, 1e-3);

	// Parameters that mean nothing here are accepted and leave the audio alone.
	ev[0].mID = 999; ev[0].mValue = 0.5f;
	one(0, 0, 1, ev, NULL, NULL, &r);
	CHECK_NEAR(r.left[0], 1.0, 1e-4);
	ev[0].mID = kAudioQueueParam_PlayRate; ev[0].mValue = 2.0f;
	one(0, 0, 1, ev, NULL, NULL, &r);
	CHECK_EQ(r.frames, 1000);
	CHECK_NEAR(r.left[0], 1.0, 1e-4);
}

// Volume events after the first buffer ramp from the current volume. Four buffers of 1.0:
// the second carries the events; its audio starts at frame 1000.
static void ramp_case(float rampTime, int useRamp, float* at, int* firstZero)
{
	int callbacks = 0;
	AudioStreamBasicDescription f = queue_format();
	AudioQueueRef q = NULL;
	CHECK_EQ(AudioQueueNewOutput(&f, returned, &callbacks, NULL, NULL, 0, &q), noErr);
	CHECK_EQ(AudioQueueSetOfflineRenderFormat(q, &f, NULL), noErr);
	AudioQueueBufferRef b[4];
	for (int k = 0; k < 4; k++)
	{
		CHECK_EQ(AudioQueueAllocateBuffer(q, NF * 8, &b[k]), noErr);
		float* p = (float*) b[k]->mAudioData;
		for (int i = 0; i < 2 * NF; i++)
			p[i] = 1.0f;
		b[k]->mAudioDataByteSize = NF * 8;
	}
	AudioQueueParameterEvent ev[2] = { { kAudioQueueParam_VolumeRampTime, rampTime }, { kAudioQueueParam_Volume, 0.0f } };
	CHECK_EQ(AudioQueueEnqueueBuffer(q, b[0], 0, NULL), noErr);
	CHECK_EQ(AudioQueueEnqueueBufferWithParameters(q, b[1], 0, NULL, 0, 0, useRamp ? 2 : 1, useRamp ? ev : ev + 1, NULL, NULL), noErr);
	CHECK_EQ(AudioQueueEnqueueBuffer(q, b[2], 0, NULL), noErr);
	CHECK_EQ(AudioQueueEnqueueBuffer(q, b[3], 0, NULL), noErr);
	CHECK_EQ(AudioQueueStart(q, NULL), noErr);
	AudioQueueBufferRef ob;
	AudioTimeStamp ts = sample_time(0);
	CHECK_EQ(AudioQueueAllocateBuffer(q, 4000 * 8, &ob), noErr);
	CHECK_EQ(AudioQueueOfflineRender(q, &ts, ob, 4000), noErr);
	float* o = (float*) ob->mAudioData;
	for (int i = 0; i < 16; i++)
		at[i] = o[2 * 250 * i];
	*firstZero = -1;
	for (int i = 1000; i < 4000; i++)
	{
		if (o[2 * i] == 0)
		{
			*firstZero = i;
			break;
		}
	}
	AudioQueueStop(q, true);
	AudioQueueDispose(q, true);
}

static void test_volume_ramp(void)
{
	float at[16];
	int zero;

	// QUIRK (test_quirk_volume_event_ramp_minimum): the change ramps over 25 ms (1102 frames)
	// when no ramp time is given, or one shorter than that.
	ramp_case(0, 0, at, &zero);
	CHECK_NEAR(at[3], 1.0, 1e-4);	// frame 750
	CHECK_NEAR(at[4], 1.0, 1e-4);	// frame 1000: the ramp starts here
	CHECK_NEAR(at[6], 0.546, 0.03);	// frame 1500
	CHECK_NEAR(at[9], 0.0, 1e-4);	// frame 2250
	CHECK(zero >= 2090 && zero <= 2115);

	ramp_case(0.02f, 1, at, &zero);
	CHECK(zero >= 2090 && zero <= 2115);

	// A ramp time above the minimum is followed: 50 ms is 2205 frames.
	ramp_case(0.05f, 1, at, &zero);
	CHECK_NEAR(at[8], 0.546, 0.03);	// frame 2000
	CHECK_NEAR(at[12], 0.093, 0.03);	// frame 3000
	CHECK(zero >= 3190 && zero <= 3225);
}

typedef struct
{
	pthread_mutex_t lock;
	int events;
	UInt32 values[16];
} Events;

static void on_running(void* user, AudioQueueRef q, AudioQueuePropertyID id)
{
	Events* e = (Events*) user;
	UInt32 v = 99, size = sizeof(v);
	AudioQueueGetProperty(q, id, &v, &size);
	pthread_mutex_lock(&e->lock);
	if (e->events < 16)
		e->values[e->events] = v;
	e->events++;
	pthread_mutex_unlock(&e->lock);
}

static int wait_events(Events* e, int n)
{
	for (int i = 0; i < 200; i++)
	{
		pthread_mutex_lock(&e->lock);
		int got = e->events;
		pthread_mutex_unlock(&e->lock);
		if (got >= n)
			return got;
		usleep(10000);
	}
	return e->events;
}

// One offline render of up to `frames` frames from a started queue.
static void render_once(AudioQueueRef q, int frames)
{
	AudioQueueBufferRef ob;
	AudioTimeStamp ts;
	memset(&ts, 0, sizeof(ts));
	ts.mFlags = kAudioTimeStampSampleTimeValid;
	CHECK_EQ(AudioQueueAllocateBuffer(q, frames * 8, &ob), noErr);
	CHECK_EQ(AudioQueueOfflineRender(q, &ts, ob, frames), noErr);
}

static void test_running_listeners(void)
{
	Events e;
	memset(&e, 0, sizeof(e));
	pthread_mutex_init(&e.lock, NULL);
	int callbacks = 0;
	AudioQueueBufferRef b;
	AudioQueueRef q = new_queue(&callbacks, &b);
	if (!q)
		return;

	CHECK_EQ(AudioQueueAddPropertyListener(q, kAudioQueueProperty_IsRunning, NULL, NULL), kAudio_ParamError);
	CHECK_EQ(AudioQueueAddPropertyListener(NULL, kAudioQueueProperty_IsRunning, on_running, &e), kAudio_ParamError);
	CHECK_EQ(AudioQueueAddPropertyListener(q, kAudioQueueProperty_IsRunning, on_running, &e), noErr);
	// Unknown property IDs are accepted, and removing what is not registered is not an error.
	CHECK_EQ(AudioQueueAddPropertyListener(q, 'zzzz', on_running, &e), noErr);
	CHECK_EQ(AudioQueueRemovePropertyListener(q, 'zzzz', on_running, &e), noErr);
	CHECK_EQ(AudioQueueRemovePropertyListener(q, kAudioQueueProperty_IsRunning, on_running, (void*) 1), noErr);
	CHECK_EQ(AudioQueueEnqueueBuffer(q, b, 0, NULL), noErr);

	// Started (an offline queue starts running with its first render on macOS): one
	// notification, and the property reads 1 by then.
	CHECK_EQ(AudioQueueStart(q, NULL), noErr);
	render_once(q, 1000);
	CHECK_EQ(wait_events(&e, 1), 1);
	CHECK_EQ(e.values[0], 1);
	// Stopped: one notification reading 0.
	CHECK_EQ(AudioQueueStop(q, true), noErr);
	CHECK_EQ(wait_events(&e, 2), 2);
	CHECK_EQ(e.values[1], 0);
	usleep(100000);
	CHECK_EQ(e.events, 2);

	// A removed listener hears nothing more.
	CHECK_EQ(AudioQueueRemovePropertyListener(q, kAudioQueueProperty_IsRunning, on_running, &e), noErr);
	CHECK_EQ(AudioQueueStart(q, NULL), noErr);
	render_once(q, 100);
	CHECK_EQ(AudioQueueStop(q, true), noErr);
	usleep(100000);
	CHECK_EQ(e.events, 2);
	CHECK_EQ(AudioQueueDispose(q, true), noErr);

	// With a run loop given at creation the notification is delivered on it.
	memset(&e, 0, sizeof(e));
	pthread_mutex_init(&e.lock, NULL);
	AudioStreamBasicDescription f = queue_format();
	q = NULL;
	CHECK_EQ(AudioQueueNewOutput(&f, returned, &callbacks, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode, 0, &q), noErr);
	CHECK_EQ(AudioQueueSetOfflineRenderFormat(q, &f, NULL), noErr);
	CHECK_EQ(AudioQueueAddPropertyListener(q, kAudioQueueProperty_IsRunning, on_running, &e), noErr);
	AudioQueueBufferRef rb;
	CHECK_EQ(AudioQueueAllocateBuffer(q, NF * 8, &rb), noErr);
	memset(rb->mAudioData, 0, NF * 8);
	rb->mAudioDataByteSize = NF * 8;
	CHECK_EQ(AudioQueueEnqueueBuffer(q, rb, 0, NULL), noErr);
	CHECK_EQ(AudioQueueStart(q, NULL), noErr);
	render_once(q, NF);
	usleep(100000);
	CHECK_EQ(e.events, 0);	// nothing until the run loop runs
	CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.2, false);
	CHECK_EQ(e.events, 1);
	CHECK_EQ(e.values[0], 1);
	CHECK_EQ(AudioQueueStop(q, true), noErr);
	CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.2, false);
	CHECK_EQ(e.events, 2);
	CHECK_EQ(e.values[1], 0);
	CHECK_EQ(AudioQueueDispose(q, true), noErr);
}

static void test_properties(void)
{
	int callbacks = 0;
	AudioQueueRef q = new_queue(&callbacks, NULL);
	if (!q)
		return;
	UInt32 size = 0;
	UInt32 v = 99;

	CHECK_EQ(AudioQueueGetPropertySize(q, kAudioQueueProperty_IsRunning, &size), noErr);
	CHECK_EQ(size, 4);
	size = 2;
	CHECK_EQ(AudioQueueGetProperty(q, kAudioQueueProperty_IsRunning, &v, &size), kAudioQueueErr_InvalidPropertySize);
	size = 8;
	CHECK_EQ(AudioQueueGetProperty(q, kAudioQueueProperty_IsRunning, &v, &size), kAudioQueueErr_InvalidPropertySize);
	size = 4;
	CHECK_EQ(AudioQueueGetProperty(q, kAudioQueueProperty_IsRunning, &v, &size), noErr);
	CHECK_EQ(v, 0);

	AudioStreamBasicDescription f = queue_format(), got;
	size = sizeof(got);
	CHECK_EQ(AudioQueueGetProperty(q, kAudioQueueProperty_StreamDescription, &got, &size), noErr);
	CHECK_EQ(size, sizeof(got));
	CHECK(memcmp(&got, &f, sizeof(f)) == 0);

	size = 4; v = 99;
	CHECK_EQ(AudioQueueGetProperty(q, kAudioQueueProperty_ConverterError, &v, &size), noErr);
	CHECK_EQ(v, 0);
	// QUIRK-free default: 1.5 seconds of frames to decode ahead (66150 at 44100 Hz), settable.
	size = 4; v = 0;
	CHECK_EQ(AudioQueueGetProperty(q, kAudioQueueProperty_DecodeBufferSizeFrames, &v, &size), noErr);
	CHECK_EQ(v, 66150);
	v = 4096;
	CHECK_EQ(AudioQueueSetProperty(q, kAudioQueueProperty_DecodeBufferSizeFrames, &v, 4), noErr);
	size = 4; v = 0;
	CHECK_EQ(AudioQueueGetProperty(q, kAudioQueueProperty_DecodeBufferSizeFrames, &v, &size), noErr);
	CHECK_EQ(v, 4096);

	// Level metering: off by default, and then the meter has nothing to read.
	size = 4; v = 99;
	CHECK_EQ(AudioQueueGetProperty(q, kAudioQueueProperty_EnableLevelMetering, &v, &size), noErr);
	CHECK_EQ(v, 0);
	AudioQueueLevelMeterState meter[2];
	CHECK_EQ(AudioQueueGetPropertySize(q, kAudioQueueProperty_CurrentLevelMeter, &size), noErr);
	CHECK_EQ(size, 16);
	size = sizeof(meter);
	CHECK_EQ(AudioQueueGetProperty(q, kAudioQueueProperty_CurrentLevelMeter, meter, &size), kAudioQueueErr_InvalidPropertyValue);
	CHECK_EQ(AudioQueueGetProperty(q, kAudioQueueProperty_CurrentLevelMeterDB, meter, &size), kAudioQueueErr_InvalidPropertyValue);
	v = 1;
	CHECK_EQ(AudioQueueSetProperty(q, kAudioQueueProperty_EnableLevelMetering, &v, 2), kAudioQueueErr_InvalidPropertySize);
	CHECK_EQ(AudioQueueSetProperty(q, kAudioQueueProperty_EnableLevelMetering, &v, 4), noErr);

	// Read-only and unknown properties.
	CHECK_EQ(AudioQueueSetProperty(q, 'zzzz', &v, 4), kAudioQueueErr_InvalidProperty);
	CHECK_EQ(AudioQueueSetProperty(q, kAudioQueueProperty_CurrentDevice, NULL, 0), kAudio_ParamError);

	// The device: a UID string, settable while stopped; an unknown UID is accepted and read back.
	CFStringRef uid = NULL;
	size = sizeof(uid);
	CHECK_EQ(AudioQueueGetProperty(q, kAudioQueueProperty_CurrentDevice, &uid, &size), noErr);
	CHECK(uid != NULL && CFGetTypeID(uid) == CFStringGetTypeID() && CFStringGetLength(uid) > 0);
	if (uid)
		CFRelease(uid);
	CFStringRef bogus = CFSTR("no-such-device-uid");
	CHECK_EQ(AudioQueueSetProperty(q, kAudioQueueProperty_CurrentDevice, &bogus, 3), kAudioQueueErr_InvalidPropertySize);
	CHECK_EQ(AudioQueueSetProperty(q, kAudioQueueProperty_CurrentDevice, &bogus, sizeof(bogus)), kAudioQueueErr_InvalidOfflineMode);
	CHECK_EQ(AudioQueueDispose(q, true), noErr);

	// On a queue that plays (not offline) the device can be chosen.
	q = NULL;
	CHECK_EQ(AudioQueueNewOutput(&f, returned, &callbacks, NULL, NULL, 0, &q), noErr);
	if (!q)
		return;
	CHECK_EQ(AudioQueueSetProperty(q, kAudioQueueProperty_CurrentDevice, &bogus, sizeof(bogus)), noErr);
	uid = NULL;
	size = sizeof(uid);
	CHECK_EQ(AudioQueueGetProperty(q, kAudioQueueProperty_CurrentDevice, &uid, &size), noErr);
	CHECK(uid != NULL && CFEqual(uid, bogus));
	if (uid)
		CFRelease(uid);
	CFStringRef none = NULL;
	CHECK_EQ(AudioQueueSetProperty(q, kAudioQueueProperty_CurrentDevice, &none, sizeof(none)), noErr);
	// QUIRK (test_quirk_default_device_uid_placeholder): with no device chosen and none to
	// default to, the UID is a placeholder: "AQDefaultOutput" (10.14.4) or "AQDefaultDevice" (26.7.3).
	AudioDeviceID dev = 0;
	UInt32 dsize = sizeof(dev);
	AudioObjectPropertyAddress addr = { kAudioHardwarePropertyDefaultOutputDevice, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
	if (AudioObjectGetPropertyData(kAudioObjectSystemObject, &addr, 0, NULL, &dsize, &dev) != noErr || dev == 0)
	{
		uid = NULL;
		size = sizeof(uid);
		CHECK_EQ(AudioQueueGetProperty(q, kAudioQueueProperty_CurrentDevice, &uid, &size), noErr);
		CHECK(uid != NULL && (CFEqual(uid, CFSTR("AQDefaultOutput")) || CFEqual(uid, CFSTR("AQDefaultDevice"))));
		if (uid)
			CFRelease(uid);
	}
	// An unknown UID is reported when the queue is started.
	CHECK_EQ(AudioQueueSetProperty(q, kAudioQueueProperty_CurrentDevice, &bogus, sizeof(bogus)), noErr);
	AudioQueueBufferRef b;
	CHECK_EQ(AudioQueueAllocateBuffer(q, 4096, &b), noErr);
	memset(b->mAudioData, 0, 4096);
	b->mAudioDataByteSize = 4096;
	CHECK_EQ(AudioQueueEnqueueBuffer(q, b, 0, NULL), noErr);
	CHECK_EQ(AudioQueueStart(q, NULL), kAudioQueueErr_InvalidDevice);
	CHECK_EQ(AudioQueueDispose(q, true), noErr);
}

static void test_parameters(void)
{
	int callbacks = 0;
	AudioQueueRef q = new_queue(&callbacks, NULL);
	if (!q)
		return;
	Float32 v;
	CHECK_EQ(AudioQueueGetParameter(q, kAudioQueueParam_Volume, &v), noErr);
	CHECK_NEAR(v, 1.0, 0);
	CHECK_EQ(AudioQueueGetParameter(q, kAudioQueueParam_PlayRate, &v), noErr);
	CHECK_NEAR(v, 1.0, 0);
	CHECK_EQ(AudioQueueGetParameter(q, kAudioQueueParam_Pitch, &v), noErr);
	CHECK_NEAR(v, 0.0, 0);
	CHECK_EQ(AudioQueueGetParameter(q, kAudioQueueParam_VolumeRampTime, &v), noErr);
	CHECK_NEAR(v, 0.0, 0);
	CHECK_EQ(AudioQueueGetParameter(q, kAudioQueueParam_Pan, &v), noErr);
	CHECK_NEAR(v, 0.0, 0);
	CHECK_EQ(AudioQueueSetParameter(q, kAudioQueueParam_Pan, 0.5f), noErr);
	CHECK_EQ(AudioQueueGetParameter(q, kAudioQueueParam_Pan, &v), noErr);
	CHECK_NEAR(v, 0.5, 0);
	CHECK_EQ(AudioQueueSetParameter(q, kAudioQueueParam_Volume, 2.0f), noErr);
	CHECK_EQ(AudioQueueSetParameter(q, kAudioQueueParam_PlayRate, 2.0f), noErr);
	CHECK_EQ(AudioQueueSetParameter(q, kAudioQueueParam_Pitch, 100.0f), noErr);
	CHECK_EQ(AudioQueueSetParameter(q, 99, 1.0f), kAudioQueueErr_InvalidParameter);
	CHECK_EQ(AudioQueueDispose(q, true), noErr);

	// The queue's own volume and pan scale what is played (set before anything is enqueued).
	AudioQueueBufferRef b;
	q = new_queue(&callbacks, &b);
	if (!q)
		return;
	CHECK_EQ(AudioQueueSetParameter(q, kAudioQueueParam_Volume, 0.5f), noErr);
	CHECK_EQ(AudioQueueSetParameter(q, kAudioQueueParam_Pan, -1.0f), noErr);
	CHECK_EQ(AudioQueueEnqueueBuffer(q, b, 0, NULL), noErr);
	Result r;
	render(q, 1000, &r);
	CHECK_NEAR(r.left[0], 0.5, 1e-3);
	CHECK_NEAR(r.right[0], 0.0, 1e-3);
	AudioQueueDispose(q, true);
	q = new_queue(&callbacks, &b);
	CHECK_EQ(AudioQueueSetParameter(q, kAudioQueueParam_Pan, 0.5f), noErr);
	CHECK_EQ(AudioQueueEnqueueBuffer(q, b, 0, NULL), noErr);
	render(q, 1000, &r);
	CHECK_NEAR(r.left[0], 0.5, 1e-3);
	CHECK_NEAR(r.right[0], -1.0, 1e-3);
	AudioQueueDispose(q, true);
}

static void test_level_meter(void)
{
	int callbacks = 0;
	AudioQueueBufferRef b;
	AudioQueueRef q = new_queue(&callbacks, &b);
	if (!q)
		return;
	UInt32 v = 1;
	CHECK_EQ(AudioQueueSetProperty(q, kAudioQueueProperty_EnableLevelMetering, &v, 4), noErr);
	AudioQueueLevelMeterState m[2];
	UInt32 size = sizeof(m);
	CHECK_EQ(AudioQueueGetProperty(q, kAudioQueueProperty_CurrentLevelMeter, m, &size), noErr);
	CHECK_NEAR(m[0].mAveragePower, 0, 0);
	CHECK_NEAR(m[0].mPeakPower, 0, 0);
	float* p = (float*) b->mAudioData;
	for (int i = 0; i < NF; i++)
	{
		p[2 * i] = (i % 2) ? 0.5f : -0.5f;
		p[2 * i + 1] = 0.25f;
	}
	CHECK_EQ(AudioQueueEnqueueBuffer(q, b, 0, NULL), noErr);
	Result r;
	// Rendered, but the meter is read while the queue is still running: render() stops it, so redo by hand.
	AudioQueueBufferRef ob;
	AudioTimeStamp ts = sample_time(0);
	CHECK_EQ(AudioQueueStart(q, NULL), noErr);
	CHECK_EQ(AudioQueueAllocateBuffer(q, NF * 8, &ob), noErr);
	CHECK_EQ(AudioQueueOfflineRender(q, &ts, ob, NF), noErr);
	size = sizeof(m);
	CHECK_EQ(AudioQueueGetProperty(q, kAudioQueueProperty_CurrentLevelMeter, m, &size), noErr);
	CHECK_NEAR(m[0].mAveragePower, 0.5, 1e-3);
	CHECK_NEAR(m[1].mAveragePower, 0.25, 1e-3);
	CHECK_NEAR(m[0].mPeakPower, 0.5, 1e-3);
	CHECK_NEAR(m[1].mPeakPower, 0.25, 1e-3);
	size = sizeof(m);
	CHECK_EQ(AudioQueueGetProperty(q, kAudioQueueProperty_CurrentLevelMeterDB, m, &size), noErr);
	CHECK_NEAR(m[0].mAveragePower, -6.0206, 1e-2);
	CHECK_NEAR(m[1].mPeakPower, -12.0412, 1e-2);
	AudioQueueStop(q, true);
	v = 0;
	CHECK_EQ(AudioQueueSetProperty(q, kAudioQueueProperty_EnableLevelMetering, &v, 4), noErr);
	size = sizeof(m);
	CHECK_EQ(AudioQueueGetProperty(q, kAudioQueueProperty_CurrentLevelMeter, m, &size), kAudioQueueErr_InvalidPropertyValue);
	(void) r;
	AudioQueueDispose(q, true);
}

int main(void)
{
	signal(SIGALRM, on_alarm);
	alarm(60);

	test_trim_and_start_times();
	test_events();
	test_volume_ramp();
	test_running_listeners();
	test_properties();
	test_parameters();
	test_level_meter();

	if (g_failures)
	{
		printf("%d check(s) failed\n", g_failures);
		return 1;
	}
	printf("ALL PASSED\n");
	return 0;
}
