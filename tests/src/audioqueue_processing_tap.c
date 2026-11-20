// Copyright (C) 2026 Lubos Dolezel
//
// AudioQueue processing taps (AudioQueueProcessingTapNew / Dispose /
// GetSourceAudio / GetQueueTime) on an output queue, driven through offline
// rendering so no audio device is needed.
// The program passes unchanged on macOS 26.7.3 (x86_64).
#include <AudioToolbox/AudioToolbox.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)
#define CHECK_EQ(a, b) do { long _a = (long)(a), _b = (long)(b); if (_a != _b) { printf("FAIL %d: %s == %ld, expected %ld\n", __LINE__, #a, _a, _b); g_failures++; } } while (0)

#define CHANNELS 2
#define RATE 44100
#define FRAMES_PER_BUFFER 1000
#define BUFFERS 4
#define TOTAL_FRAMES (FRAMES_PER_BUFFER * BUFFERS)
#define RENDER_FRAMES 500

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	fflush(stdout);
	_exit(2);
}

static AudioStreamBasicDescription queue_format(void)
{
	AudioStreamBasicDescription f = { RATE, kAudioFormatLinearPCM,
		kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked, 4 * CHANNELS, 1, 4 * CHANNELS, CHANNELS, 32, 0 };
	return f;
}

// The source signal: a ramp, different for each channel.
static float source_sample(int frame, int channel)
{
	return (channel ? -0.5f : 0.5f) * (float) frame / (float) TOTAL_FRAMES;
}

static void returned(void* user, AudioQueueRef q, AudioQueueBufferRef b)
{
	(void) q;
	(void) b;
	(*(int*) user)++;
}

static AudioQueueRef new_queue(int* returnedCount, AudioQueueBufferRef bufs[BUFFERS], int offline)
{
	AudioStreamBasicDescription f = queue_format();
	AudioQueueRef q = NULL;
	CHECK_EQ(AudioQueueNewOutput(&f, returned, returnedCount, NULL, NULL, 0, &q), noErr);
	CHECK(q != NULL);
	if (!q)
		return NULL;
	// A queue that is to be rendered offline is put into that mode first: without it, a
	// tap needs the output device (which a remote session may not have).
	if (offline)
		CHECK_EQ(AudioQueueSetOfflineRenderFormat(q, &f, NULL), noErr);
	for (int i = 0; i < BUFFERS; i++)
	{
		CHECK_EQ(AudioQueueAllocateBuffer(q, FRAMES_PER_BUFFER * 4 * CHANNELS, &bufs[i]), noErr);
		float* p = (float*) bufs[i]->mAudioData;
		for (int fr = 0; fr < FRAMES_PER_BUFFER; fr++)
			for (int c = 0; c < CHANNELS; c++)
				p[fr * CHANNELS + c] = source_sample(i * FRAMES_PER_BUFFER + fr, c);
		bufs[i]->mAudioDataByteSize = FRAMES_PER_BUFFER * 4 * CHANNELS;
	}
	return q;
}

static void enqueue_all(AudioQueueRef q, AudioQueueBufferRef bufs[BUFFERS])
{
	for (int i = 0; i < BUFFERS; i++)
		CHECK_EQ(AudioQueueEnqueueBuffer(q, bufs[i], 0, NULL), noErr);
}

// Renders everything that was enqueued, RENDER_FRAMES at a time; out has TOTAL_FRAMES frames.
static void render_all(AudioQueueRef q, float* out)
{
	AudioQueueBufferRef ob = NULL;
	CHECK_EQ(AudioQueueAllocateBuffer(q, RENDER_FRAMES * 4 * CHANNELS, &ob), noErr);
	if (!ob)
		return;
	for (int done = 0; done < TOTAL_FRAMES; done += RENDER_FRAMES)
	{
		AudioTimeStamp ts;
		memset(&ts, 0, sizeof(ts));
		ts.mFlags = kAudioTimeStampSampleTimeValid;
		ts.mSampleTime = done;
		CHECK_EQ(AudioQueueOfflineRender(q, &ts, ob, RENDER_FRAMES), noErr);
		CHECK_EQ(ob->mAudioDataByteSize, RENDER_FRAMES * 4 * CHANNELS);
		memcpy(out + (size_t) done * CHANNELS, ob->mAudioData, RENDER_FRAMES * 4 * CHANNELS);
	}
	CHECK_EQ(AudioQueueFreeBuffer(q, ob), noErr);
}

static int offline_start(AudioQueueRef q)
{
	OSStatus s = AudioQueueStart(q, NULL);
	CHECK_EQ(s, noErr);
	return s == noErr;
}


// ---- A scaling tap ----

typedef struct
{
	AudioQueueProcessingTapFlags enterFlags;
	AudioQueueProcessingTapFlags firstSourceFlags;
	int calls;
	int sawStart;
	long framesRequested;
	long framesSourced;
	long framesReturned;
	int nullPointersOnEntry;
	int channelsSeen;
	Float64 lastQueueTime;
	int queueTimeOk;
	int timeContinuous;
	Float64 expectedTime;
	OSStatus outsideSource;
	OSStatus outsideTime;
	float gain;
	int invert;
} ScaleState;


static void scale_callback(void* user, AudioQueueProcessingTapRef tap, UInt32 frames,
		AudioTimeStamp* ts, AudioQueueProcessingTapFlags* flags, UInt32* outFrames, AudioBufferList* data)
{
	ScaleState* st = user;
	st->calls++;
	st->enterFlags = *flags;
	st->framesRequested += frames;
	st->channelsSeen = (int) data->mNumberBuffers;
	for (UInt32 i = 0; i < data->mNumberBuffers; i++)
		if (data->mBuffers[i].mData != NULL || data->mBuffers[i].mDataByteSize != 0)
			st->nullPointersOnEntry = 0;
	if (ts->mSampleTime != st->expectedTime)
		st->timeContinuous = 0;
	st->expectedTime += frames;

	AudioQueueProcessingTapFlags srcFlags = 0;
	UInt32 got = 0;
	OSStatus s = AudioQueueProcessingTapGetSourceAudio(tap, frames, ts, &srcFlags, &got, data);
	if (s != noErr)
	{
		*outFrames = 0;
		st->framesSourced = -1000000;
		return;
	}
	if (st->calls == 1)
		st->firstSourceFlags = srcFlags;
	if (srcFlags & kAudioQueueProcessingTap_StartOfStream)
		st->sawStart++;
	st->framesSourced += got;

	Float64 qt = -1;
	UInt32 qframes = 0;
	if (AudioQueueProcessingTapGetQueueTime(tap, &qt, &qframes) != noErr || qframes != frames)
		st->queueTimeOk = 0;
	st->lastQueueTime = qt;

	for (UInt32 c = 0; c < data->mNumberBuffers; c++)
	{
		float* p = data->mBuffers[c].mData;
		for (UInt32 i = 0; i < got; i++)
			p[i] = (st->invert ? -p[i] : p[i]) * st->gain;
	}
	*outFrames = got;
	st->framesReturned += got;
}

static void init_state(ScaleState* st, float gain, int invert)
{
	memset(st, 0, sizeof(*st));
	st->nullPointersOnEntry = 1;
	st->queueTimeOk = 1;
	st->timeContinuous = 1;
	st->gain = gain;
	st->invert = invert;
}

static void test_scaling_tap(void)
{
	int returnedCount = 0;
	AudioQueueBufferRef bufs[BUFFERS];
	AudioQueueRef q = new_queue(&returnedCount, bufs, 1);
	if (!q)
		return;

	ScaleState st;
	init_state(&st, 0.5f, 1);
	UInt32 maxFrames = 0;
	AudioStreamBasicDescription pf;
	memset(&pf, 0xAA, sizeof(pf));
	AudioQueueProcessingTapRef tap = NULL;

	// Argument errors: neither placement flag, both, unknown bits.
	CHECK_EQ(AudioQueueProcessingTapNew(q, scale_callback, &st, 0, &maxFrames, &pf, &tap), kAudio_ParamError);
	CHECK_EQ(AudioQueueProcessingTapNew(q, scale_callback, &st,
		kAudioQueueProcessingTap_PreEffects | kAudioQueueProcessingTap_PostEffects, &maxFrames, &pf, &tap),
		kAudio_ParamError);

	CHECK_EQ(AudioQueueProcessingTapNew(q, scale_callback, &st, kAudioQueueProcessingTap_PreEffects,
		&maxFrames, &pf, &tap), noErr);
	CHECK(tap != NULL);

	// The processing format: float, at the queue's rate and channel count.
	CHECK(maxFrames >= 256 && maxFrames <= 65536);
	CHECK_EQ(pf.mFormatID, kAudioFormatLinearPCM);
	CHECK(pf.mFormatFlags & kAudioFormatFlagIsFloat);
	CHECK_EQ(pf.mBitsPerChannel, 32);
	CHECK_EQ(pf.mChannelsPerFrame, CHANNELS);
	CHECK_EQ(pf.mSampleRate, RATE);
	CHECK_EQ(pf.mFramesPerPacket, 1);
	int nonInterleaved = (pf.mFormatFlags & kAudioFormatFlagIsNonInterleaved) != 0;
	CHECK(nonInterleaved);

	// Only one tap per queue.
	AudioQueueProcessingTapRef second = NULL;
	CHECK_EQ(AudioQueueProcessingTapNew(q, scale_callback, &st, kAudioQueueProcessingTap_PostEffects,
		&maxFrames, &pf, &second), kAudioQueueErr_TooManyTaps);

	// The tap functions that belong to the callback fail outside of it.
	Float64 qt;
	UInt32 qf;
	AudioBufferList empty;
	memset(&empty, 0, sizeof(empty));
	CHECK(AudioQueueProcessingTapGetQueueTime(tap, &qt, &qf) != noErr);
	UInt32 got;
	AudioTimeStamp stamp0;
	AudioQueueProcessingTapFlags flags0;
	memset(&stamp0, 0, sizeof(stamp0));
	CHECK(AudioQueueProcessingTapGetSourceAudio(tap, 100, &stamp0, &flags0, &got, &empty) != noErr);

	enqueue_all(q, bufs);
	if (offline_start(q))
	{
		// A tap can only be created while the queue is stopped.
		AudioQueueProcessingTapRef late = NULL;
		CHECK_EQ(AudioQueueProcessingTapNew(q, scale_callback, &st, kAudioQueueProcessingTap_PostEffects,
			&maxFrames, &pf, &late), kAudioQueueErr_InvalidRunState);

		static float out[TOTAL_FRAMES * CHANNELS];
		render_all(q, out);

		int ok = 1;
		for (int i = 0; i < TOTAL_FRAMES; i++)
			for (int c = 0; c < CHANNELS; c++)
				if (fabsf(out[i * CHANNELS + c] - (-0.5f * source_sample(i, c))) > 1e-6f)
					ok = 0;
		CHECK(ok);

		CHECK(st.calls > 0);
		CHECK_EQ(st.framesRequested, TOTAL_FRAMES);
		CHECK_EQ(st.framesSourced, TOTAL_FRAMES);
		CHECK_EQ(st.framesReturned, TOTAL_FRAMES);
		CHECK(st.enterFlags & kAudioQueueProcessingTap_PreEffects);
		CHECK(!(st.enterFlags & kAudioQueueProcessingTap_Siphon));
		CHECK_EQ(st.channelsSeen, CHANNELS);
		CHECK(st.nullPointersOnEntry);
		CHECK(st.firstSourceFlags & kAudioQueueProcessingTap_StartOfStream);
		CHECK_EQ(st.sawStart, 1);
		CHECK(st.queueTimeOk);
		CHECK(st.timeContinuous);
		CHECK_EQ((long) st.lastQueueTime, TOTAL_FRAMES - RENDER_FRAMES);
		CHECK_EQ(AudioQueueStop(q, true), noErr);
	}

	CHECK_EQ(AudioQueueProcessingTapDispose(tap), noErr);

	// The queue now has no tap, and a new one can be installed.
	CHECK_EQ(AudioQueueProcessingTapNew(q, scale_callback, &st, kAudioQueueProcessingTap_PostEffects,
		&maxFrames, &pf, &tap), noErr);
	CHECK_EQ(AudioQueueProcessingTapDispose(tap), noErr);
	CHECK_EQ(AudioQueueDispose(q, true), noErr);
}

// ---- Pass-through, with and without a tap ----

static void test_without_tap(void)
{
	int returnedCount = 0;
	AudioQueueBufferRef bufs[BUFFERS];
	AudioQueueRef q = new_queue(&returnedCount, bufs, 1);
	if (!q)
		return;
	enqueue_all(q, bufs);
	if (offline_start(q))
	{
		static float out[TOTAL_FRAMES * CHANNELS];
		render_all(q, out);
		int ok = 1;
		for (int i = 0; i < TOTAL_FRAMES; i++)
			for (int c = 0; c < CHANNELS; c++)
				if (fabsf(out[i * CHANNELS + c] - source_sample(i, c)) > 1e-6f)
					ok = 0;
		CHECK(ok);

		// Past the enqueued audio nothing is rendered.
		AudioQueueBufferRef ob;
		AudioTimeStamp ts;
		memset(&ts, 0, sizeof(ts));
		ts.mFlags = kAudioTimeStampSampleTimeValid;
		CHECK_EQ(AudioQueueAllocateBuffer(q, RENDER_FRAMES * 4 * CHANNELS, &ob), noErr);
		ob->mAudioDataByteSize = 1234;
		CHECK_EQ(AudioQueueOfflineRender(q, &ts, ob, RENDER_FRAMES), noErr);
		CHECK_EQ(ob->mAudioDataByteSize, 0);
		CHECK_EQ(AudioQueueStop(q, true), noErr);
	}
	CHECK_EQ(AudioQueueDispose(q, true), noErr);
}

// ---- Siphon ----

typedef struct
{
	int calls;
	long frames;
	double sum;
	int buffersValid;
	int queueTimeFails;
} SiphonState;

static void siphon_callback(void* user, AudioQueueProcessingTapRef tap, UInt32 frames,
		AudioTimeStamp* ts, AudioQueueProcessingTapFlags* flags, UInt32* outFrames, AudioBufferList* data)
{
	(void) ts;
	SiphonState* st = user;
	(void) flags;
	Float64 qt;
	UInt32 qf;
	st->queueTimeFails = AudioQueueProcessingTapGetQueueTime(tap, &qt, &qf) != noErr;
	st->calls++;
	st->frames += frames;
	if (data->mNumberBuffers != CHANNELS)
		st->buffersValid = 0;
	for (UInt32 c = 0; c < data->mNumberBuffers; c++)
	{
		float* p = data->mBuffers[c].mData;
		if (!p || data->mBuffers[c].mDataByteSize < frames * sizeof(float))
		{
			st->buffersValid = 0;
			continue;
		}
		for (UInt32 i = 0; i < frames; i++)
			st->sum += p[i];
	}
	*outFrames = 0; // ignored: a siphon does not produce audio
}

static void test_siphon(void)
{
	int returnedCount = 0;
	AudioQueueBufferRef bufs[BUFFERS];
	AudioQueueRef q = new_queue(&returnedCount, bufs, 1);
	if (!q)
		return;
	SiphonState st;
	memset(&st, 0, sizeof(st));
	st.buffersValid = 1;
	UInt32 maxFrames = 0;
	AudioStreamBasicDescription pf;
	AudioQueueProcessingTapRef tap = NULL;
	CHECK_EQ(AudioQueueProcessingTapNew(q, siphon_callback, &st,
		kAudioQueueProcessingTap_PostEffects | kAudioQueueProcessingTap_Siphon, &maxFrames, &pf, &tap), noErr);
	enqueue_all(q, bufs);
	if (offline_start(q))
	{
		static float out[TOTAL_FRAMES * CHANNELS];
		render_all(q, out);
		// What is played is the source, untouched.
		int ok = 1;
		for (int i = 0; i < TOTAL_FRAMES; i++)
			for (int c = 0; c < CHANNELS; c++)
				if (fabsf(out[i * CHANNELS + c] - source_sample(i, c)) > 1e-6f)
					ok = 0;
		CHECK(ok);

		double expected = 0;
		for (int i = 0; i < TOTAL_FRAMES; i++)
			for (int c = 0; c < CHANNELS; c++)
				expected += source_sample(i, c);
		CHECK(st.calls > 0);
		CHECK_EQ(st.frames, TOTAL_FRAMES);
		CHECK(fabs(st.sum - expected) < 1e-2);
		CHECK(st.buffersValid);
		// A siphon never asks for source audio, so it has no queue time either.
		CHECK(st.queueTimeFails);
		// test_quirk_dispose_tap_while_running: the header asks for a stopped queue, but a running
		// one accepts the call, and later renders have no tap.
		CHECK_EQ(AudioQueueProcessingTapDispose(tap), noErr);
		tap = NULL;
		CHECK_EQ(AudioQueueStop(q, true), noErr);
	}
	if (tap)
		CHECK_EQ(AudioQueueProcessingTapDispose(tap), noErr);
	CHECK_EQ(AudioQueueDispose(q, true), noErr);
}

// ---- The queue volume is applied after the tap ----

typedef struct
{
	double peak;
} PeakState;

static void peak_callback(void* user, AudioQueueProcessingTapRef tap, UInt32 frames,
		AudioTimeStamp* ts, AudioQueueProcessingTapFlags* flags, UInt32* outFrames, AudioBufferList* data)
{
	PeakState* st = user;
	AudioQueueProcessingTapFlags f;
	UInt32 got = 0;
	AudioQueueProcessingTapGetSourceAudio(tap, frames, ts, &f, &got, data);
	for (UInt32 c = 0; c < data->mNumberBuffers; c++)
	{
		float* p = data->mBuffers[c].mData;
		for (UInt32 i = 0; i < got; i++)
			if (fabsf(p[i]) > st->peak)
				st->peak = fabsf(p[i]);
	}
	*outFrames = got;
	(void) flags;
}

static double peak_with_volume(AudioQueueProcessingTapFlags placement, int* outputAttenuated)
{
	int returnedCount = 0;
	AudioQueueBufferRef bufs[BUFFERS];
	AudioQueueRef q = new_queue(&returnedCount, bufs, 1);
	PeakState st = { 0 };
	if (!q)
		return -1;
	UInt32 maxFrames = 0;
	AudioStreamBasicDescription pf;
	AudioQueueProcessingTapRef tap = NULL;
	CHECK_EQ(AudioQueueProcessingTapNew(q, peak_callback, &st, placement, &maxFrames, &pf, &tap), noErr);
	CHECK_EQ(AudioQueueSetParameter(q, kAudioQueueParam_Volume, 0.5f), noErr);
	enqueue_all(q, bufs);
	if (offline_start(q))
	{
		static float out[TOTAL_FRAMES * CHANNELS];
		render_all(q, out);
		int ok = 1;
		for (int i = 0; i < TOTAL_FRAMES; i++)
			for (int c = 0; c < CHANNELS; c++)
				if (fabsf(out[i * CHANNELS + c] - 0.5f * source_sample(i, c)) > 1e-6f)
					ok = 0;
		*outputAttenuated = ok;
		AudioQueueStop(q, true);
	}
	AudioQueueProcessingTapDispose(tap);
	AudioQueueDispose(q, true);
	return st.peak;
}

static void test_volume_after_tap(void)
{
	int ok1 = 0, ok2 = 0;
	double pre = peak_with_volume(kAudioQueueProcessingTap_PreEffects, &ok1);
	double post = peak_with_volume(kAudioQueueProcessingTap_PostEffects, &ok2);
	CHECK(ok1);
	CHECK(ok2);
	// The output carries the volume, but both tap placements see the unattenuated ramp, which
	// peaks at its last frame.
	CHECK(fabs(pre - fabs(source_sample(TOTAL_FRAMES - 1, 0))) < 1e-4);
	CHECK(fabs(post - fabs(source_sample(TOTAL_FRAMES - 1, 0))) < 1e-4);
}

// ---- A tap that returns less than asked ----

static void short_callback(void* user, AudioQueueProcessingTapRef tap, UInt32 frames,
		AudioTimeStamp* ts, AudioQueueProcessingTapFlags* flags, UInt32* outFrames, AudioBufferList* data)
{
	(void) user;
	(void) flags;
	UInt32 got = 0;
	AudioQueueProcessingTapFlags f;
	AudioQueueProcessingTapGetSourceAudio(tap, frames, ts, &f, &got, data);
	*outFrames = got / 2; // the rest of the chunk is filled with silence
}

static void test_short_return(void)
{
	int returnedCount = 0;
	AudioQueueBufferRef bufs[BUFFERS];
	AudioQueueRef q = new_queue(&returnedCount, bufs, 1);
	if (!q)
		return;
	UInt32 maxFrames = 0;
	AudioStreamBasicDescription pf;
	AudioQueueProcessingTapRef tap = NULL;
	CHECK_EQ(AudioQueueProcessingTapNew(q, short_callback, NULL, kAudioQueueProcessingTap_PreEffects, &maxFrames, &pf, &tap), noErr);
	enqueue_all(q, bufs);
	if (offline_start(q))
	{
		static float out[TOTAL_FRAMES * CHANNELS];
		render_all(q, out);
		// Within each rendered block the first half is the source and the second half is silent;
		// the blocks are at most maxFrames long, so check the tail of the first block.
		UInt32 block = RENDER_FRAMES < maxFrames ? RENDER_FRAMES : maxFrames;
		CHECK(out[(block - 1) * CHANNELS] == 0.0f);
		CHECK(out[1 * CHANNELS + 1] == source_sample(1, 1));
		AudioQueueStop(q, true);
	}
	AudioQueueProcessingTapDispose(tap);
	AudioQueueDispose(q, true);
}

// ---- Offline mode errors ----

static void test_offline_errors(void)
{
	int returnedCount = 0;
	AudioQueueBufferRef bufs[BUFFERS];
	AudioQueueRef q = new_queue(&returnedCount, bufs, 0);
	if (!q)
		return;
	AudioQueueBufferRef ob;
	AudioTimeStamp ts;
	memset(&ts, 0, sizeof(ts));
	CHECK_EQ(AudioQueueAllocateBuffer(q, RENDER_FRAMES * 4 * CHANNELS, &ob), noErr);
	// Not in offline mode.
	CHECK_EQ(AudioQueueOfflineRender(q, &ts, ob, RENDER_FRAMES), kAudioQueueErr_InvalidOfflineMode);
	AudioStreamBasicDescription f = queue_format();
	CHECK_EQ(AudioQueueSetOfflineRenderFormat(q, &f, NULL), noErr);
	CHECK_EQ(AudioQueueSetOfflineRenderFormat(q, NULL, NULL), noErr);
	CHECK_EQ(AudioQueueOfflineRender(q, &ts, ob, RENDER_FRAMES), kAudioQueueErr_InvalidOfflineMode);

	// A timestamp without a valid sample time, or a buffer that is too small.
	CHECK_EQ(AudioQueueSetOfflineRenderFormat(q, &f, NULL), noErr);
	enqueue_all(q, bufs);
	CHECK_EQ(AudioQueueStart(q, NULL), noErr);
	CHECK_EQ(AudioQueueOfflineRender(q, &ts, ob, RENDER_FRAMES), kAudio_ParamError);
	ts.mFlags = kAudioTimeStampSampleTimeValid;
	CHECK_EQ(AudioQueueOfflineRender(q, &ts, ob, RENDER_FRAMES + 1), kAudio_ParamError);
	CHECK_EQ(AudioQueueOfflineRender(q, &ts, ob, RENDER_FRAMES), noErr);
	CHECK_EQ(ob->mAudioDataByteSize, RENDER_FRAMES * 4 * CHANNELS);
	AudioQueueStop(q, true);

	AudioQueueStop(q, true);
	AudioQueueDispose(q, true);
}

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	signal(SIGALRM, on_alarm);
	alarm(60);

	test_without_tap();
	test_scaling_tap();
	test_siphon();
	test_volume_after_tap();
	test_short_return();
	test_offline_errors();

	if (g_failures)
		printf("FAIL: %d check(s) failed\n", g_failures);
	else
		printf("PASS\n");
	fflush(stdout);
	return g_failures ? 1 : 0;
}
