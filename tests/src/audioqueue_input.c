/*
 * AudioQueue input (recording) queues.
 *
 * Part 1 needs no audio device: queue creation and argument errors, buffer
 * handling, property errors, state calls on a stopped queue, and
 * AudioQueueNewInputWithDispatchQueue argument checks. The error codes that the
 * headers leave open are pinned as quirks, each marked QUIRK below with the
 * macOS 26.7.3 observation it follows.
 *
 * Part 2 records from the default input device and runs when one exists
 * (--require-device makes a missing one a failure). --device UID records from
 * that device instead of the default one, so that no microphone is opened by accident. What it checks does not
 * depend on what the source carries: callback context, buffer sizes, the
 * sample time of each buffer, the packet description arguments, start/stop/
 * pause/reset behaviour with partly filled buffers, IsRunning notifications,
 * the properties, and the stream formats (direct and converted).
 * With --ramp the source is expected to carry a 440 Hz tone whose level rises
 * linearly (darling-skills audio-ramp.py): then the recorded samples of every
 * format are checked for continuity, frequency, rising level and metering.
 *
 * Exit status is 0 when every check passed.
 *
 * Copyright (C) 2026 Lubos Dolezel
 */
#include <AudioToolbox/AudioToolbox.h>
#include <CoreAudio/CoreAudio.h>
#include <CoreFoundation/CoreFoundation.h>
#include <dispatch/dispatch.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mach/mach_time.h>

static int failures;
static int require_device, ramp;
static const char *device_uid;	/* --device: record from this device (UID) instead of the default one */

#define CHECK(cond, ...) do { \
	if (cond) printf("PASS %s\n", #cond); \
	else { printf("FAIL %s: ", #cond); printf(__VA_ARGS__); printf("\n"); failures++; } \
} while (0)

static AudioStreamBasicDescription pcm(double rate, UInt32 ch, UInt32 bits, UInt32 flags, UInt32 bytesPerFrame)
{
	AudioStreamBasicDescription f;
	memset(&f, 0, sizeof f);
	f.mSampleRate = rate;
	f.mFormatID = kAudioFormatLinearPCM;
	f.mFormatFlags = flags;
	f.mChannelsPerFrame = ch;
	f.mBitsPerChannel = bits;
	f.mFramesPerPacket = 1;
	f.mBytesPerFrame = f.mBytesPerPacket = bytesPerFrame ? bytesPerFrame : ch * bits / 8;
	return f;
}

#define S16 (kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked)
#define F32 (kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked)

static void noop(void *ud, AudioQueueRef q, AudioQueueBufferRef b, const AudioTimeStamp *ts,
	UInt32 np, const AudioStreamPacketDescription *pd) {}

/* ---- recorder ---- */

#define MAXCB 400
struct rec {
	pthread_mutex_t lock;
	int n;
	UInt32 bytes[MAXCB];
	Float64 sampleTime[MAXCB];
	UInt32 tsFlags[MAXCB];
	UInt32 packets[MAXCB];
	int pdsNull[MAXCB];
	double at[MAXCB];
	int onMainThread, onOtherThread;
	int reenqueue;
	unsigned char *data;	/* all recorded bytes, concatenated */
	size_t dataSize, dataCap;
	UInt32 frameBytes;
	int running_notifications, last_running;
	int running_values[16];
};

static double now_ms(void)
{
	static mach_timebase_info_data_t tb;
	if (!tb.denom) mach_timebase_info(&tb);
	return mach_absolute_time() * (double)tb.numer / tb.denom / 1e6;
}

static double t0;

static void rec_init(struct rec *r, UInt32 frameBytes, size_t cap)
{
	memset(r, 0, sizeof *r);
	pthread_mutex_init(&r->lock, NULL);
	r->frameBytes = frameBytes;
	r->dataCap = cap;
	r->data = malloc(cap);
	r->reenqueue = 1;
}

static void rec_cb(void *ud, AudioQueueRef q, AudioQueueBufferRef b, const AudioTimeStamp *ts,
	UInt32 np, const AudioStreamPacketDescription *pd)
{
	struct rec *r = ud;
	pthread_mutex_lock(&r->lock);
	int i = r->n++;
	if (i < MAXCB) {
		r->bytes[i] = b->mAudioDataByteSize;
		r->sampleTime[i] = ts->mSampleTime;
		r->tsFlags[i] = ts->mFlags;
		r->packets[i] = np;
		r->pdsNull[i] = pd == NULL;
		r->at[i] = now_ms() - t0;
	}
	if (pthread_main_np()) r->onMainThread++; else r->onOtherThread++;
	if (r->dataSize + b->mAudioDataByteSize <= r->dataCap) {
		memcpy(r->data + r->dataSize, b->mAudioData, b->mAudioDataByteSize);
		r->dataSize += b->mAudioDataByteSize;
	}
	int again = r->reenqueue;
	pthread_mutex_unlock(&r->lock);
	if (again)
		AudioQueueEnqueueBuffer(q, b, 0, NULL);
}

static void running_listener(void *ud, AudioQueueRef q, AudioQueuePropertyID id)
{
	struct rec *r = ud;
	UInt32 v = 99, sz = sizeof v;
	AudioQueueGetProperty(q, id, &v, &sz);
	pthread_mutex_lock(&r->lock);
	if (r->running_notifications < 16)
		r->running_values[r->running_notifications] = (int)v;
	r->running_notifications++;
	pthread_mutex_unlock(&r->lock);
}

static UInt32 is_running(AudioQueueRef q)
{
	UInt32 v = 99, sz = sizeof v;
	AudioQueueGetProperty(q, kAudioQueueProperty_IsRunning, &v, &sz);
	return v;
}

static void select_device(AudioQueueRef q)
{
	if (!device_uid)
		return;
	CFStringRef uid = CFStringCreateWithCString(NULL, device_uid, kCFStringEncodingUTF8);
	OSStatus s = AudioQueueSetProperty(q, kAudioQueueProperty_CurrentDevice, &uid, sizeof uid);
	if (s != noErr) {
		printf("FAIL selecting device %s: %d\n", device_uid, (int)s);
		failures++;
	}
	CFRelease(uid);
}

static int have_input_device(void)
{
	AudioObjectPropertyAddress a = { kAudioHardwarePropertyDefaultInputDevice, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
	AudioDeviceID d = kAudioObjectUnknown;
	UInt32 sz = sizeof d;
	if (AudioObjectGetPropertyData(kAudioObjectSystemObject, &a, 0, NULL, &sz, &d) != noErr)
		return 0;
	return d != kAudioObjectUnknown;
}

/* ---- part 1: no device needed ---- */

static void part1(void)
{
	AudioStreamBasicDescription f = pcm(44100, 1, 16, S16, 0);
	AudioQueueRef q = NULL;
	OSStatus s;

	/* QUIRK (macOS 26.7.3, tier 2): the construction errors below. Test: part1(). */
	s = AudioQueueNewInput(&f, noop, NULL, NULL, NULL, 0, &q);
	CHECK(s == noErr && q, "AudioQueueNewInput = %d", (int)s);
	if (q) AudioQueueDispose(q, true);

	q = NULL;
	s = AudioQueueNewInput(&f, noop, NULL, NULL, NULL, 1, &q);
	CHECK(s == noErr, "non-zero reserved flags are accepted: %d", (int)s);
	if (!s) AudioQueueDispose(q, true);

	AudioStreamBasicDescription g;
	g = pcm(48000, 2, 32, F32, 0);
	q = NULL; s = AudioQueueNewInput(&g, noop, NULL, NULL, NULL, 0, &q);
	CHECK(s == noErr, "f32 stereo 48000: %d", (int)s);
	if (!s) AudioQueueDispose(q, true);
	g = pcm(44100, 2, 16, S16 | kAudioFormatFlagIsBigEndian, 0);
	q = NULL; s = AudioQueueNewInput(&g, noop, NULL, NULL, NULL, 0, &q);
	CHECK(s == noErr, "big-endian s16 stereo: %d", (int)s);
	if (!s) AudioQueueDispose(q, true);
	g = pcm(44100, 1, 24, S16, 0);
	q = NULL; s = AudioQueueNewInput(&g, noop, NULL, NULL, NULL, 0, &q);
	CHECK(s == noErr, "packed s24 mono: %d", (int)s);
	if (!s) AudioQueueDispose(q, true);
	g = pcm(44100, 1, 8, kAudioFormatFlagIsPacked, 0);
	q = NULL; s = AudioQueueNewInput(&g, noop, NULL, NULL, NULL, 0, &q);
	CHECK(s == noErr, "unsigned 8-bit mono: %d", (int)s);
	if (!s) AudioQueueDispose(q, true);
	g = pcm(44100, 1, 16, S16 | kAudioFormatFlagIsNonInterleaved, 2);
	q = NULL; s = AudioQueueNewInput(&g, noop, NULL, NULL, NULL, 0, &q);
	CHECK(s == noErr, "non-interleaved single channel is the same as interleaved: %d", (int)s);
	if (!s) AudioQueueDispose(q, true);

	g = pcm(0, 1, 16, S16, 0);
	/* differs on macOS 26.7.3 (paramErr): a rate of 0 means the device's rate */
	q = NULL;
	s = AudioQueueNewInput(&g, noop, NULL, NULL, NULL, 0, &q);
	CHECK(s == noErr && q, "rate 0 is accepted: %d", (int)s);
	if (!s) AudioQueueDispose(q, true);
	g = pcm(-5, 1, 16, S16, 0);
	CHECK((s = AudioQueueNewInput(&g, noop, NULL, NULL, NULL, 0, &q)) == kAudio_ParamError, "negative rate: %d", (int)s);
	g = pcm(44100, 0, 16, S16, 2);
	CHECK((s = AudioQueueNewInput(&g, noop, NULL, NULL, NULL, 0, &q)) == kAudio_ParamError, "0 channels: %d", (int)s);
	g = pcm(44100, 1, 16, S16, 3);
	/* differs on macOS 26.7.3 (paramErr) */
	CHECK((s = AudioQueueNewInput(&g, noop, NULL, NULL, NULL, 0, &q)) == kAudioFormatUnsupportedDataFormatError, "frame size not matching the sample size: %d", (int)s);
	memset(&g, 0, sizeof g);
	g.mSampleRate = 44100; g.mFormatID = 0x3f3f3f3f /* unknown id */; g.mChannelsPerFrame = 1;
	CHECK((s = AudioQueueNewInput(&g, noop, NULL, NULL, NULL, 0, &q)) == kAudioFormatUnsupportedDataFormatError, "unknown format id: %d", (int)s);
	CHECK((s = AudioQueueNewInput(&f, NULL, NULL, NULL, NULL, 0, &q)) == kAudio_ParamError, "NULL callback: %d", (int)s);
	CHECK((s = AudioQueueNewInput(NULL, noop, NULL, NULL, NULL, 0, &q)) == kAudio_ParamError, "NULL format: %d", (int)s);

	dispatch_queue_t dq = dispatch_queue_create("audioqueue_input.test", NULL);
	AudioQueueInputCallbackBlock block = ^(AudioQueueRef a, AudioQueueBufferRef b, const AudioTimeStamp *t, UInt32 n, const AudioStreamPacketDescription *p) {};
	q = NULL;
	s = AudioQueueNewInputWithDispatchQueue(&q, &f, 0, dq, block);
	CHECK(s == noErr && q, "AudioQueueNewInputWithDispatchQueue = %d", (int)s);
	if (!s) AudioQueueDispose(q, true);
	CHECK((s = AudioQueueNewInputWithDispatchQueue(&q, &f, 0, dq, NULL)) == kAudio_ParamError, "NULL block: %d", (int)s);
	CHECK((s = AudioQueueNewInputWithDispatchQueue(&q, &f, 0, NULL, block)) == kAudio_ParamError, "NULL dispatch queue: %d", (int)s);
	CHECK((s = AudioQueueNewInputWithDispatchQueue(&q, &g, 0, dq, block)) != noErr, "bad format with dispatch queue: %d", (int)s);

	/* An existing queue: properties, buffers, state calls while stopped */
	s = AudioQueueNewInput(&f, noop, NULL, NULL, NULL, 0, &q);
	if (s) return;
	UInt32 u = 99, sz = sizeof u;
	s = AudioQueueGetProperty(q, kAudioQueueProperty_IsRunning, &u, &sz);
	CHECK(s == noErr && u == 0 && sz == 4, "IsRunning stopped: %d %u", (int)s, u);
	/* QUIRK (macOS 26.7.3, tier 2): IsRunning wants exactly four bytes. Test: part1(). */
	sz = 2; s = AudioQueueGetProperty(q, kAudioQueueProperty_IsRunning, &u, &sz);
	CHECK(s == kAudioQueueErr_InvalidPropertySize, "IsRunning, 2 byte buffer: %d", (int)s);
	sz = 8; s = AudioQueueGetProperty(q, kAudioQueueProperty_IsRunning, &u, &sz);
	CHECK(s == kAudioQueueErr_InvalidPropertySize, "IsRunning, 8 byte buffer: %d", (int)s);
	AudioStreamBasicDescription d;
	sz = sizeof d; s = AudioQueueGetProperty(q, kAudioQueueProperty_StreamDescription, &d, &sz);
	CHECK(s == noErr && sz == sizeof d && d.mSampleRate == 44100 && d.mChannelsPerFrame == 1 && d.mFormatID == kAudioFormatLinearPCM,
		"StreamDescription: %d", (int)s);
	s = AudioQueueGetPropertySize(q, kAudioQueueProperty_StreamDescription, &sz);
	CHECK(s == noErr && sz == sizeof d, "StreamDescription size");
	s = AudioQueueGetPropertySize(q, kAudioQueueProperty_CurrentDevice, &sz);
	CHECK(s == noErr && sz == sizeof(CFStringRef), "CurrentDevice size");
	s = AudioQueueGetPropertySize(q, kAudioQueueProperty_CurrentLevelMeter, &sz);
	CHECK(s == noErr && sz == sizeof(AudioQueueLevelMeterState), "level meter size is one state per channel: %u", sz);
	s = AudioQueueGetPropertySize(q, kAudioQueueProperty_ConverterError, &sz);
	CHECK(s == noErr && sz == 4, "ConverterError size");
	sz = 4; s = AudioQueueGetProperty(q, kAudioQueueProperty_ConverterError, &u, &sz);
	CHECK(s == noErr && u == 0, "ConverterError: %d %u", (int)s, u);
	sz = 4; s = AudioQueueGetProperty(q, kAudioQueueProperty_EnableLevelMetering, &u, &sz);
	CHECK(s == noErr && u == 0, "metering is off at first: %d %u", (int)s, u);
	AudioQueueLevelMeterState m[2];
	sz = sizeof m; s = AudioQueueGetProperty(q, kAudioQueueProperty_CurrentLevelMeter, m, &sz);
	CHECK(s == kAudioQueueErr_InvalidPropertyValue, "level meter while metering is off: %d", (int)s);
	char bogus[8];
	sz = 4; s = AudioQueueGetProperty(q, 'zzzz', bogus, &sz);
	CHECK(s != noErr, "unknown property: %d", (int)s);
	u = 0; s = AudioQueueSetProperty(q, kAudioQueueProperty_IsRunning, &u, 4);
	CHECK(s != noErr, "IsRunning is read-only: %d", (int)s);
	CFStringRef uid = CFSTR("no-such-input-device");
	s = AudioQueueSetProperty(q, kAudioQueueProperty_CurrentDevice, &uid, 2);
	CHECK(s == kAudioQueueErr_InvalidPropertySize, "CurrentDevice, wrong size: %d", (int)s);

	/* buffers */
	AudioQueueBufferRef b[3];
	for (int i = 0; i < 3; i++)
		AudioQueueAllocateBuffer(q, 8820, &b[i]);
	CHECK(b[0]->mAudioDataBytesCapacity == 8820 && b[0]->mAudioDataByteSize == 0, "fresh buffer");
	b[0]->mAudioDataByteSize = 100;
	s = AudioQueueEnqueueBuffer(q, b[0], 0, NULL);
	CHECK(s == noErr, "enqueue before start, size preset: %d", (int)s);
	s = AudioQueueEnqueueBuffer(q, b[0], 0, NULL);
	CHECK(s == kAudioQueueErr_BufferEnqueuedTwice, "enqueue twice: %d", (int)s);
	s = AudioQueueFreeBuffer(q, b[0]);
	CHECK(s == kAudioQueueErr_BufferInQueue, "free an enqueued buffer: %d", (int)s);
	b[1]->mAudioDataByteSize = 0;
	s = AudioQueueEnqueueBuffer(q, b[1], 0, NULL);
	CHECK(s == noErr, "enqueue with byte size 0: %d", (int)s);
	/* the size left in a buffer means nothing (26.7.3 gives paramErr when it is above the capacity) */
	b[2]->mAudioDataByteSize = 9000;
	s = AudioQueueEnqueueBufferWithParameters(q, b[2], 0, NULL, 0, 0, 0, NULL, NULL, NULL);
	CHECK(s == noErr, "enqueue with plain parameters, size above the capacity: %d", (int)s);
	AudioQueueBufferRef pd;
	s = AudioQueueAllocateBufferWithPacketDescriptions(q, 1024, 4, &pd);
	CHECK(s == noErr && pd->mPacketDescriptionCapacity == 4 && pd->mPacketDescriptions, "buffer with packet descriptions");
	s = AudioQueueFreeBuffer(q, pd);
	CHECK(s == noErr, "free a buffer that is not enqueued: %d", (int)s);

	/* state calls on a stopped queue */
	CHECK((s = AudioQueueStop(q, true)) == noErr, "Stop on a stopped queue: %d", (int)s);
	CHECK((s = AudioQueueStop(q, false)) == noErr, "deferred Stop on a stopped queue: %d", (int)s);
	CHECK((s = AudioQueuePause(q)) == noErr, "Pause on a stopped queue: %d", (int)s);
	CHECK((s = AudioQueueFlush(q)) == noErr, "Flush on a stopped queue: %d", (int)s);
	UInt32 prepared;
	CHECK((s = AudioQueuePrime(q, 0, &prepared)) == kAudioQueueErr_InvalidQueueType, "Prime is for playback: %d", (int)s);
	AudioTimeStamp ts;
	CHECK((s = AudioQueueGetCurrentTime(q, NULL, &ts, NULL)) != noErr, "no current time while stopped: %d", (int)s);

	/* a queue has no offline mode or taps */
	CHECK((s = AudioQueueSetOfflineRenderFormat(q, &f, NULL)) != noErr, "offline format on an input queue: %d", (int)s);

	/* property listeners */
	struct rec r;
	rec_init(&r, 2, 16);
	s = AudioQueueAddPropertyListener(q, kAudioQueueProperty_IsRunning, running_listener, &r);
	CHECK(s == noErr, "AudioQueueAddPropertyListener: %d", (int)s);
	s = AudioQueueRemovePropertyListener(q, kAudioQueueProperty_IsRunning, running_listener, &r);
	CHECK(s == noErr, "AudioQueueRemovePropertyListener: %d", (int)s);

	s = AudioQueueDispose(q, true);
	CHECK(s == noErr, "dispose: %d", (int)s);
	free(r.data);
}

/* ---- part 2: recording ---- */

static void drain_runloop(double seconds)
{
	CFRunLoopRunInMode(kCFRunLoopDefaultMode, seconds, false);
}

static int make_queue(AudioQueueRef *q, AudioStreamBasicDescription f, struct rec *r, UInt32 bufferFrames, int nbuf,
	CFRunLoopRef rl)
{
	OSStatus s = AudioQueueNewInput(&f, rec_cb, r, rl, rl ? kCFRunLoopDefaultMode : NULL, 0, q);
	if (s != noErr) {
		printf("FAIL AudioQueueNewInput = %d\n", (int)s);
		failures++;
		return 0;
	}
	select_device(*q);
	for (int i = 0; i < nbuf; i++) {
		AudioQueueBufferRef b;
		AudioQueueAllocateBuffer(*q, bufferFrames * f.mBytesPerFrame, &b);
		AudioQueueEnqueueBuffer(*q, b, 0, NULL);
	}
	return 1;
}

static void test_basic_recording(void)
{
	printf("-- basic recording, callback thread\n");
	AudioStreamBasicDescription f = pcm(44100, 1, 16, S16, 0);
	struct rec r;
	rec_init(&r, 2, 44100 * 2 * 5);
	AudioQueueRef q;
	if (!make_queue(&q, f, &r, 4410, 4, NULL))
		return;
	AudioQueueAddPropertyListener(q, kAudioQueueProperty_IsRunning, running_listener, &r);

	t0 = now_ms();
	OSStatus s = AudioQueueStart(q, NULL);
	CHECK(s == noErr, "AudioQueueStart = %d", (int)s);
	if (s != noErr) { AudioQueueDispose(q, true); free(r.data); return; }
	CHECK(is_running(q) == 1, "IsRunning after start");
	CHECK((s = AudioQueueStart(q, NULL)) == noErr, "Start on a running queue: %d", (int)s);

	AudioTimeStamp ts;
	s = AudioQueueGetCurrentTime(q, NULL, &ts, NULL);
	CHECK(s == noErr && (ts.mFlags & kAudioTimeStampSampleTimeValid) && (ts.mFlags & kAudioTimeStampHostTimeValid),
		"current time while recording: %d flags %x", (int)s, (unsigned)ts.mFlags);

	/* a buffer in use by the queue */
	AudioQueueBufferRef extra;
	AudioQueueAllocateBuffer(q, 1000, &extra);
	usleep(1200000);
	AudioQueueGetCurrentTime(q, NULL, &ts, NULL);
	printf("queue time after 1.2 s: %.0f frames\n", ts.mSampleTime);
	CHECK(ts.mSampleTime > 0.5 * 44100 && ts.mSampleTime < 2.5 * 44100, "queue time follows the recording: %.0f", ts.mSampleTime);

	s = AudioQueueStop(q, true);
	CHECK(s == noErr, "Stop(true) = %d", (int)s);
	CHECK(is_running(q) == 0, "IsRunning after stop");

	pthread_mutex_lock(&r.lock);
	int n = r.n;
	printf("%d callbacks, first at %.0f ms\n", n, r.at[0]);
	CHECK(n >= 6 && n < MAXCB, "about a second of 0.1 s buffers: %d callbacks", n);
	int sizes_ok = 1, contiguous = 1;
	/* the last callback(s) after Stop are returned buffers; judge the full ones */
	int full = 0;
	for (int i = 0; i < n && i < MAXCB; i++) {
		if (r.bytes[i] == 8820) {
			full++;
			if (i && r.bytes[i - 1] == 8820 && r.sampleTime[i] - r.sampleTime[i - 1] != 4410) contiguous = 0;
		} else if (r.bytes[i] != 0 && i < n - 4)
			sizes_ok = 0;
	}
	CHECK(full >= 6, "buffers are filled to capacity: %d", full);
	CHECK(sizes_ok, "no short buffer before the stop");
	CHECK(contiguous, "sample times advance by the buffer's frames");
	CHECK(r.sampleTime[0] == 0, "the first buffer starts at sample time %.0f", r.sampleTime[0]);
	CHECK((r.tsFlags[0] & kAudioTimeStampSampleTimeValid), "sample time valid flag");
	/* QUIRK: PCM callbacks get no packet descriptions. Test: test_basic_recording() */
	CHECK(r.packets[0] == 0 && r.pdsNull[0], "packet descriptions of PCM: count %u null %d", r.packets[0], r.pdsNull[0]);
	CHECK(r.onMainThread == 0 && r.onOtherThread == n, "callbacks run on the queue's own thread");
	CHECK(r.at[0] < 800, "first buffer arrives promptly: %.0f ms", r.at[0]);
	pthread_mutex_unlock(&r.lock);

	usleep(200000);
	pthread_mutex_lock(&r.lock);
	int after = r.n - n;
	/* returned empty buffers after the stop come first and are complete when Stop returns */
	CHECK(after == 0, "no callback after Stop returned: %d", after);
	CHECK(r.running_notifications >= 2 && r.running_values[0] == 1 && r.running_values[r.running_notifications - 1] == 0,
		"IsRunning notifications: %d", r.running_notifications);
	pthread_mutex_unlock(&r.lock);

	CHECK((s = AudioQueueFreeBuffer(q, extra)) == noErr, "free");
	AudioQueueRemovePropertyListener(q, kAudioQueueProperty_IsRunning, running_listener, &r);
	AudioQueueDispose(q, true);
	free(r.data);
}

static void test_partial_buffers(void)
{
	static const char *name[3] = { "Stop(false)", "Stop(true)", "Reset" };
	for (int mode = 0; mode < 3; mode++) {
		printf("-- partly filled buffer, %s\n", name[mode]);
		AudioStreamBasicDescription f = pcm(44100, 1, 16, S16, 0);
		struct rec r;
		rec_init(&r, 2, 44100 * 2 * 10);
		r.reenqueue = 0;
		AudioQueueRef q;
		if (!make_queue(&q, f, &r, 44100 * 3, 2, NULL))
			return;
		AudioQueueAddPropertyListener(q, kAudioQueueProperty_IsRunning, running_listener, &r);
		t0 = now_ms();
		if (AudioQueueStart(q, NULL) != noErr) { printf("FAIL start\n"); failures++; AudioQueueDispose(q, true); free(r.data); return; }
		usleep(600000);
		double a = now_ms();
		OSStatus s = mode == 0 ? AudioQueueStop(q, false) : mode == 1 ? AudioQueueStop(q, true) : AudioQueueReset(q);
		double took = now_ms() - a;
		CHECK(s == noErr, "%s = %d", name[mode], (int)s);
		if (mode == 0)
			CHECK(took < 200, "deferred stop returns at once: %.0f ms", took);
		pthread_mutex_lock(&r.lock);
		int atReturn = r.n;
		pthread_mutex_unlock(&r.lock);
		if (mode != 0)
			CHECK(atReturn == 2, "the buffers are back when %s returns: %d", name[mode], atReturn);
		usleep(500000);
		pthread_mutex_lock(&r.lock);
		CHECK(r.n == 2, "both enqueued buffers come back: %d", r.n);
		double secs = r.bytes[0] / 2.0 / 44100.0;
		printf("partial buffer holds %.2f s, second %u bytes, sample time %.0f\n", secs, r.bytes[1], r.sampleTime[0]);
		CHECK(secs > 0.3 && secs < 1.2, "the partly filled buffer returns what it holds: %.2f s", secs);
		CHECK(r.bytes[1] == 0, "the untouched buffer returns empty: %u", r.bytes[1]);
		CHECK(r.sampleTime[0] == 0, "partial buffer start time");
		pthread_mutex_unlock(&r.lock);
		if (mode == 0) {
			CHECK(is_running(q) == 0, "deferred stop has finished");
			pthread_mutex_lock(&r.lock);
			CHECK(r.running_notifications >= 2 && r.running_values[r.running_notifications - 1] == 0, "IsRunning went to 0");
			pthread_mutex_unlock(&r.lock);
		} else if (mode == 1) {
			CHECK(is_running(q) == 0, "stopped");
		} else {
			CHECK(is_running(q) == 1, "Reset keeps recording");
			AudioQueueStop(q, true);
		}
		AudioQueueDispose(q, true);
		free(r.data);
	}
}

static void test_pause_restart(void)
{
	printf("-- pause, resume, restart\n");
	AudioStreamBasicDescription f = pcm(44100, 1, 16, S16, 0);
	struct rec r;
	rec_init(&r, 2, 44100 * 2 * 8);
	AudioQueueRef q;
	if (!make_queue(&q, f, &r, 4410, 6, NULL))
		return;
	t0 = now_ms();
	if (AudioQueueStart(q, NULL) != noErr) { printf("FAIL start\n"); failures++; AudioQueueDispose(q, true); free(r.data); return; }
	usleep(500000);
	OSStatus s = AudioQueuePause(q);
	CHECK(s == noErr, "Pause = %d", (int)s);
	usleep(100000);
	pthread_mutex_lock(&r.lock);
	int paused_at = r.n;
	Float64 lastTime = r.sampleTime[r.n - 1];
	pthread_mutex_unlock(&r.lock);
	usleep(700000);
	pthread_mutex_lock(&r.lock);
	CHECK(r.n == paused_at, "nothing is recorded while paused: %d more", r.n - paused_at);
	pthread_mutex_unlock(&r.lock);
	CHECK(is_running(q) == 1, "a paused queue is still running");
	s = AudioQueueStart(q, NULL);
	CHECK(s == noErr, "resume: %d", (int)s);
	usleep(500000);
	pthread_mutex_lock(&r.lock);
	CHECK(r.n > paused_at, "recording again after resume");
	if (r.n > paused_at) {
		Float64 gap = r.sampleTime[paused_at] - lastTime;
		printf("sample time step across the pause: %.0f\n", gap);
		CHECK(gap <= 2 * 4410, "paused time does not count in the queue's time: %.0f", gap);
	}
	pthread_mutex_unlock(&r.lock);
	AudioQueueStop(q, true);

	/* start again: the timeline begins at 0 once more */
	pthread_mutex_lock(&r.lock);
	r.n = 0;
	pthread_mutex_unlock(&r.lock);
	r.reenqueue = 1;
	for (int i = 0; i < 4; i++) {
		AudioQueueBufferRef b;
		AudioQueueAllocateBuffer(q, 8820, &b);
		AudioQueueEnqueueBuffer(q, b, 0, NULL);
	}
	s = AudioQueueStart(q, NULL);
	CHECK(s == noErr, "restart after stop: %d", (int)s);
	usleep(500000);
	AudioQueueStop(q, true);
	pthread_mutex_lock(&r.lock);
	CHECK(r.n > 0 && r.sampleTime[0] == 0, "restarted timeline starts at 0: %.0f", r.n ? r.sampleTime[0] : -1.0);
	pthread_mutex_unlock(&r.lock);
	AudioQueueDispose(q, true);
	free(r.data);
}

static void test_no_buffers(void)
{
	printf("-- recording with no buffer, enqueued late\n");
	AudioStreamBasicDescription f = pcm(44100, 1, 16, S16, 0);
	struct rec r;
	rec_init(&r, 2, 44100 * 2 * 4);
	AudioQueueRef q;
	if (!make_queue(&q, f, &r, 4410, 0, NULL))
		return;
	AudioQueueBufferRef b;
	AudioQueueAllocateBuffer(q, 8820, &b);
	t0 = now_ms();
	OSStatus s = AudioQueueStart(q, NULL);
	CHECK(s == noErr, "Start with nothing enqueued: %d", (int)s);
	if (s) { AudioQueueDispose(q, true); free(r.data); return; }
	usleep(600000);
	AudioTimeStamp ts;
	AudioQueueGetCurrentTime(q, NULL, &ts, NULL);
	CHECK(ts.mSampleTime > 0.3 * 44100, "time runs while there is nothing to record into: %.0f", ts.mSampleTime);
	AudioQueueEnqueueBuffer(q, b, 0, NULL);
	usleep(500000);
	pthread_mutex_lock(&r.lock);
	CHECK(r.n >= 1, "buffer enqueued late is filled");
	if (r.n)
		CHECK(r.sampleTime[0] >= 0.3 * 44100, "frames recorded before it was enqueued are not in it (start %.0f)", r.sampleTime[0]);
	pthread_mutex_unlock(&r.lock);
	AudioQueueStop(q, true);
	AudioQueueDispose(q, true);
	free(r.data);
}

/* ---- run loop and dispatch queue delivery ---- */

static int rl_cb_main;
static void rl_cb(void *ud, AudioQueueRef q, AudioQueueBufferRef b, const AudioTimeStamp *ts, UInt32 np, const AudioStreamPacketDescription *pd)
{
	if (pthread_main_np()) rl_cb_main++;
	AudioQueueEnqueueBuffer(q, b, 0, NULL);
}

static void test_delivery_contexts(void)
{
	printf("-- callbacks on a run loop\n");
	AudioStreamBasicDescription f = pcm(44100, 1, 16, S16, 0);
	AudioQueueRef q;
	OSStatus s = AudioQueueNewInput(&f, rl_cb, NULL, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode, 0, &q);
	if (s) { printf("FAIL AudioQueueNewInput = %d\n", (int)s); failures++; return; }
	select_device(q);
	for (int i = 0; i < 3; i++) {
		AudioQueueBufferRef b;
		AudioQueueAllocateBuffer(q, 4410, &b);
		AudioQueueEnqueueBuffer(q, b, 0, NULL);
	}
	if (AudioQueueStart(q, NULL) == noErr) {
		drain_runloop(0.8);
		CHECK(rl_cb_main >= 4, "callbacks ran on the given run loop (main thread): %d", rl_cb_main);
		AudioQueueStop(q, true);
		int before = rl_cb_main;
		drain_runloop(0.2);
		printf("run loop callbacks after Stop: %d\n", rl_cb_main - before);
	} else { printf("FAIL start\n"); failures++; }
	AudioQueueDispose(q, true);

	printf("-- callbacks on a dispatch queue\n");
	dispatch_queue_t dq = dispatch_queue_create("audioqueue_input.client", NULL);
	static int key;
	dispatch_queue_set_specific(dq, &key, &key, NULL);
	__block int on_queue = 0, elsewhere = 0, with_desc = 0;
	dispatch_semaphore_t got = dispatch_semaphore_create(0);
	s = AudioQueueNewInputWithDispatchQueue(&q, &f, 0, dq,
		^(AudioQueueRef a, AudioQueueBufferRef b, const AudioTimeStamp *t, UInt32 n, const AudioStreamPacketDescription *p) {
			if (dispatch_get_specific(&key)) on_queue++; else elsewhere++;
			if (n || p) with_desc++;
			if (on_queue == 3) dispatch_semaphore_signal(got);
			AudioQueueEnqueueBuffer(a, b, 0, NULL);
		});
	CHECK(s == noErr, "AudioQueueNewInputWithDispatchQueue = %d", (int)s);
	if (s) return;
	select_device(q);
	for (int i = 0; i < 3; i++) {
		AudioQueueBufferRef b;
		AudioQueueAllocateBuffer(q, 4410, &b);
		AudioQueueEnqueueBuffer(q, b, 0, NULL);
	}
	if (AudioQueueStart(q, NULL) == noErr) {
		long timeout = dispatch_semaphore_wait(got, dispatch_time(DISPATCH_TIME_NOW, 3 * NSEC_PER_SEC));
		CHECK(timeout == 0, "dispatch queue callbacks arrive");
		AudioQueueStop(q, true);
		dispatch_sync(dq, ^{});
		CHECK(on_queue >= 3 && elsewhere == 0, "all on the given dispatch queue: %d on, %d elsewhere", on_queue, elsewhere);
		CHECK(with_desc == 0, "no packet descriptions");
	} else { printf("FAIL start\n"); failures++; }
	AudioQueueDispose(q, true);
	int snapshot = on_queue;
	usleep(100000);
	dispatch_sync(dq, ^{});
	CHECK(on_queue == snapshot, "no callback after dispose");
}

/* dispose from inside the callback */
static void dispose_cb(void *ud, AudioQueueRef q, AudioQueueBufferRef b, const AudioTimeStamp *ts, UInt32 np, const AudioStreamPacketDescription *pd)
{
	int *count = ud;
	if (++*count == 2)
		AudioQueueDispose(q, true);
	else if (*count < 2)
		AudioQueueEnqueueBuffer(q, b, 0, NULL);
}

static void test_dispose_in_callback(void)
{
	printf("-- dispose from the callback\n");
	AudioStreamBasicDescription f = pcm(44100, 1, 16, S16, 0);
	int count = 0;
	AudioQueueRef q;
	if (AudioQueueNewInput(&f, dispose_cb, &count, NULL, NULL, 0, &q)) return;
	select_device(q);
	for (int i = 0; i < 3; i++) {
		AudioQueueBufferRef b;
		AudioQueueAllocateBuffer(q, 2205, &b);
		AudioQueueEnqueueBuffer(q, b, 0, NULL);
	}
	if (AudioQueueStart(q, NULL) != noErr) { failures++; printf("FAIL start\n"); AudioQueueDispose(q, true); return; }
	usleep(800000);
	CHECK(count == 2, "the queue was disposed by its callback after two buffers: %d", count);
}

/* ---- the recorded samples ---- */

struct fmt_case {
	const char *name;
	AudioStreamBasicDescription f;
	int kind;	/* 0 s16 LE, 1 f32, 2 s16 BE, 3 s24 packed, 4 u8, 5 s32 */
};

static double sample_at(const unsigned char *p, int kind)
{
	switch (kind) {
	case 0: { short s; memcpy(&s, p, 2); return s / 32768.0; }
	case 1: { float v; memcpy(&v, p, 4); return v; }
	case 2: { short s = (short)((p[0] << 8) | p[1]); return s / 32768.0; }
	case 3: { int v = (int)((unsigned)p[0] << 8 | (unsigned)p[1] << 16 | (unsigned)p[2] << 24) >> 8; return v / 8388608.0; }
	case 4: return ((int)p[0] - 128) / 128.0;
	default: { int v; memcpy(&v, p, 4); return v / 2147483648.0; }
	}
}

static void analyze(const struct fmt_case *c, struct rec *r, UInt32 channels, double rate, double seconds)
{
	size_t frames = r->dataSize / r->frameBytes;
	UInt32 sampleBytes = c->f.mBytesPerFrame / c->f.mChannelsPerFrame;
	printf("%s: %zu frames recorded (%.2f s)\n", c->name, frames, frames / rate);
	CHECK(frames > rate * seconds * 0.7, "%s: recorded at least 70%% of the time: %zu frames", c->name, frames);
	if (!ramp || frames < rate / 2)
		return;

	/* 10 ms windows over the first channel */
	size_t win = (size_t)(rate / 100);
	size_t nwin = frames / win;
	double *peak = calloc(nwin, sizeof(double));
	for (size_t w = 0; w < nwin; w++)
		for (size_t i = 0; i < win; i++) {
			double v = fabs(sample_at(r->data + (w * win + i) * r->frameBytes, c->kind));
			if (v > peak[w]) peak[w] = v;
		}
	size_t first = nwin, last = 0, tone = 0;
	for (size_t w = 0; w < nwin; w++)
		if (peak[w] > 0.03) { if (first == nwin) first = w; last = w; tone++; }
	CHECK(tone * 10 >= nwin * 5, "%s: tone present in most of the recording (%zu of %zu windows)", c->name, tone, nwin);
	if (tone < 5) { free(peak); return; }
	size_t holes = 0, longest = 0, run = 0;
	for (size_t w = first; w <= last; w++) {
		if (peak[w] <= 0.03) { run++; holes++; if (run > longest) longest = run; } else run = 0;
	}
	CHECK(longest <= 2, "%s: no gap inside the tone (%zu quiet windows, longest %zu)", c->name, holes, longest);
	/* The source repeats a ramp (level rising linearly, then dropping back). Judge the longest
	 * stretch between two drops: its level must rise steadily. */
	size_t segStart = first, bestStart = first, bestLen = 0;
	for (size_t w = first + 1; w <= last + 1; w++) {
		if (w > last || peak[w] < peak[w - 1] - 0.3) {
			if (w - segStart > bestLen) { bestStart = segStart; bestLen = w - segStart; }
			segStart = w;
		}
	}
	double sx = 0, sy = 0, sxx = 0, sxy = 0, syy = 0, n = 0;
	for (size_t w = bestStart; w < bestStart + bestLen; w++) {
		sx += w; sy += peak[w]; sxx += (double)w * w; sxy += w * peak[w]; syy += peak[w] * peak[w]; n++;
	}
	double corr = (n * sxy - sx * sy) / sqrt((n * sxx - sx * sx) * (n * syy - sy * sy));
	printf("%s: longest rising stretch %zu windows, correlation %.3f, from %.2f to %.2f\n", c->name, bestLen, corr, peak[bestStart], peak[bestStart + bestLen - 1]);
	CHECK(bestLen >= 100, "%s: an unbroken rise of at least a second: %zu windows", c->name, bestLen);
	CHECK(corr > 0.9, "%s: the level rises through the stretch: %.3f", c->name, corr);
	/* frequency by zero crossings of the first channel inside the tone */
	size_t crossings = 0;
	double prev = 0;
	for (size_t i = first * win; i < (last + 1) * win && i < frames; i++) {
		double v = sample_at(r->data + i * r->frameBytes, c->kind);
		if (i > first * win && ((prev < 0 && v >= 0) || (prev >= 0 && v < 0)))
			crossings++;
		prev = v;
	}
	double span = (double)((last + 1 - first) * win) / rate;
	double freq = crossings / 2.0 / span;
	printf("%s: frequency %.1f Hz\n", c->name, freq);
	CHECK(freq > 420 && freq < 460, "%s: 440 Hz tone: %.1f", c->name, freq);
	(void)sampleBytes; (void)channels;
	free(peak);
}

static void test_formats(double seconds)
{
	struct fmt_case cases[] = {
		{ "s16 mono 44100", pcm(44100, 1, 16, S16, 0), 0 },
		{ "f32 stereo 48000", pcm(48000, 2, 32, F32, 0), 1 },
		{ "s16 big-endian mono 44100", pcm(44100, 1, 16, S16 | kAudioFormatFlagIsBigEndian, 0), 2 },
		{ "s24 packed mono 44100", pcm(44100, 1, 24, S16, 0), 3 },
		{ "u8 mono 22050", pcm(22050, 1, 8, kAudioFormatFlagIsPacked, 0), 4 },
		{ "s32 mono 16000", pcm(16000, 1, 32, S16, 0), 5 },
	};
	for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++) {
		struct fmt_case *c = &cases[i];
		printf("-- format %s\n", c->name);
		struct rec r;
		rec_init(&r, c->f.mBytesPerFrame, (size_t)(c->f.mSampleRate * c->f.mBytesPerFrame * (seconds + 3)));
		AudioQueueRef q;
		UInt32 bufferFrames = (UInt32)(c->f.mSampleRate / 10);
		if (!make_queue(&q, c->f, &r, bufferFrames, 5, NULL)) { free(r.data); continue; }
		AudioStreamBasicDescription got;
		UInt32 sz = sizeof got;
		AudioQueueGetProperty(q, kAudioQueueProperty_StreamDescription, &got, &sz);
		CHECK(memcmp(&got, &c->f, sizeof got) == 0, "%s: the stream description is the requested format", c->name);
		UInt32 en = 1;
		OSStatus s = AudioQueueSetProperty(q, kAudioQueueProperty_EnableLevelMetering, &en, sizeof en);
		CHECK(s == noErr, "%s: enable metering: %d", c->name, (int)s);
		t0 = now_ms();
		s = AudioQueueStart(q, NULL);
		CHECK(s == noErr, "%s: start: %d", c->name, (int)s);
		if (s == noErr) {
			usleep((useconds_t)(seconds * 1e6));
			AudioQueueLevelMeterState m[2] = { { -1, -1 }, { -1, -1 } }, db[2];
			sz = c->f.mChannelsPerFrame * sizeof m[0];
			s = AudioQueueGetProperty(q, kAudioQueueProperty_CurrentLevelMeter, m, &sz);
			CHECK(s == noErr && sz == c->f.mChannelsPerFrame * sizeof m[0], "%s: level meter: %d size %u", c->name, (int)s, sz);
			CHECK(m[0].mAveragePower >= 0 && m[0].mAveragePower <= 1 && m[0].mPeakPower >= m[0].mAveragePower && m[0].mPeakPower <= 1,
				"%s: levels are in range: avg %.4f peak %.4f", c->name, m[0].mAveragePower, m[0].mPeakPower);
			sz = c->f.mChannelsPerFrame * sizeof db[0];
			s = AudioQueueGetProperty(q, kAudioQueueProperty_CurrentLevelMeterDB, db, &sz);
			CHECK(s == noErr && db[0].mPeakPower <= 0.01, "%s: level in dB: %d peak %.1f", c->name, (int)s, db[0].mPeakPower);
			if (ramp)
				CHECK(m[0].mPeakPower > 0.02 && db[0].mPeakPower > -40, "%s: the meter sees the tone: peak %.4f", c->name, m[0].mPeakPower);
			AudioQueueStop(q, true);
		}
		UInt32 convErr = 99;
		sz = sizeof convErr;
		AudioQueueGetProperty(q, kAudioQueueProperty_ConverterError, &convErr, &sz);
		CHECK(convErr == 0, "%s: no converter error: %u", c->name, convErr);
		analyze(c, &r, c->f.mChannelsPerFrame, c->f.mSampleRate, seconds);
		AudioQueueDispose(q, true);
		free(r.data);
	}
}

static void test_device_properties(void)
{
	printf("-- device properties\n");
	AudioStreamBasicDescription f = pcm(44100, 1, 16, S16, 0);
	AudioQueueRef q;
	if (AudioQueueNewInput(&f, noop, NULL, NULL, NULL, 0, &q)) return;
	select_device(q);
	CFStringRef uid = NULL;
	UInt32 sz = sizeof uid;
	OSStatus s = AudioQueueGetProperty(q, kAudioQueueProperty_CurrentDevice, &uid, &sz);
	CHECK(s == noErr && uid && CFGetTypeID(uid) == CFStringGetTypeID(), "CurrentDevice: %d", (int)s);
	Float64 rate = 0;
	sz = sizeof rate;
	s = AudioQueueGetProperty(q, kAudioQueueDeviceProperty_SampleRate, &rate, &sz);
	CHECK(s == noErr && rate > 0, "device sample rate: %d %.0f", (int)s, rate);
	UInt32 ch = 0;
	sz = sizeof ch;
	s = AudioQueueGetProperty(q, kAudioQueueDeviceProperty_NumberChannels, &ch, &sz);
	CHECK(s == noErr && ch > 0, "device channels: %d %u", (int)s, ch);
	if (uid) {
		s = AudioQueueSetProperty(q, kAudioQueueProperty_CurrentDevice, &uid, sizeof uid);
		CHECK(s == noErr, "set CurrentDevice to the current device: %d", (int)s);
		CFRelease(uid);
	}
	/* a queue made with rate 0 reports the device's rate */
	{
		AudioStreamBasicDescription z = pcm(0, 1, 16, S16, 0), zd;
		AudioQueueRef zq;
		if (AudioQueueNewInput(&z, noop, NULL, NULL, NULL, 0, &zq) == noErr) {
			select_device(zq);
			sz = sizeof zd;
			AudioQueueGetProperty(zq, kAudioQueueProperty_StreamDescription, &zd, &sz);
			Float64 dr = 0;
			sz = sizeof dr;
			AudioQueueGetProperty(zq, kAudioQueueDeviceProperty_SampleRate, &dr, &sz);
			printf("rate-0 queue: stream rate %.0f, device rate %.0f\n", zd.mSampleRate, dr);
			CHECK(zd.mSampleRate > 0 && zd.mSampleRate == dr, "rate 0 resolves to the device rate");
			AudioQueueDispose(zq, true);
		}
	}
	CFStringRef bogus = CFSTR("no-such-input-device");
	s = AudioQueueSetProperty(q, kAudioQueueProperty_CurrentDevice, &bogus, sizeof bogus);
	CHECK(s == noErr, "setting an unknown device is accepted: %d", (int)s);
	s = AudioQueueStart(q, NULL);
	CHECK(s == kAudioQueueErr_InvalidDevice, "starting on an unknown device: %d", (int)s);
	CHECK(is_running(q) == 0, "not running after the failed start");
	UInt32 en = 1;
	CHECK((s = AudioQueueSetProperty(q, kAudioQueueProperty_EnableLevelMetering, &en, 4)) == noErr, "enable metering on a stopped queue: %d", (int)s);
	AudioQueueLevelMeterState m = { -1, -1 };
	sz = sizeof m;
	s = AudioQueueGetProperty(q, kAudioQueueProperty_CurrentLevelMeter, &m, &sz);
	CHECK(s == noErr && m.mAveragePower == 0 && m.mPeakPower == 0, "meter reads zero before recording: %d %f %f", (int)s, m.mAveragePower, m.mPeakPower);
	AudioQueueDispose(q, true);
}

int main(int argc, char **argv)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	double seconds = 2.0;
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--require-device")) require_device = 1;
		else if (!strcmp(argv[i], "--ramp")) ramp = 1;
		else if (!strcmp(argv[i], "--device") && i + 1 < argc) device_uid = argv[++i];
		else if (!strcmp(argv[i], "--seconds") && i + 1 < argc) seconds = atof(argv[++i]);
	}

	part1();

	if (!device_uid && !have_input_device()) {
		printf("no input device\n");
		if (require_device) {
			printf("FAIL no default input device\n");
			failures++;
		} else
			printf("SKIP recording checks\n");
	} else {
		test_device_properties();
		test_basic_recording();
		test_partial_buffers();
		test_pause_restart();
		test_no_buffers();
		test_delivery_contexts();
		test_dispose_in_callback();
		test_formats(seconds);
	}

	printf("%d failure(s)\n", failures);
	printf("RESULT %s\n", failures ? "FAIL" : "PASS");
	return failures ? 1 : 0;
}
