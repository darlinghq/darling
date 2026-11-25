// Copyright (C) 2026 Lubos Dolezel
//
// The time/pitch effect of an AudioQueue (kAudioQueueProperty_EnableTimePitch, _TimePitchAlgorithm and
// _TimePitchBypass, kAudioQueueParam_PlayRate and _Pitch): which property settings are accepted and which
// status the rest return, and what comes out of an offline render of a tone (so that no audio device is
// needed). The left channel carries 440 Hz and the right one 660 Hz, a sine of amplitude 0.5, for 2 s.
//
// The expected values were observed on macOS 26.7.3 (x86_64) and 10.14.4 (i386), on queues in offline render
// mode; a check marked QUIRK covers behaviour that the headers do not state. Neither Mac has an audio device.
// On both, AudioQueueOfflineRender stops responding for some play rates with the Spectral and Varispeed
// algorithms (Spectral 1.2, 1.5, 3; Varispeed 1.5, 2, 3, 4), and on 10.14.4 AudioQueueGetCurrentTime of an
// offline queue returns meaningless sample times: the rates used here are ones that work on both, and
// the check of the sample time is for macOS 26 and Darling only (run with the argument "mac" on 10.14.4).
//
//   guest-cc.sh -a x86_64 -o audioqueue_timepitch tests/src/audioqueue_timepitch.c -framework AudioToolbox -framework CoreFoundation
#include <AudioToolbox/AudioToolbox.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)
#define CHECK_EQ(a, b) do { long _a = (long)(a), _b = (long)(b); if (_a != _b) { printf("FAIL %d: %s == %ld, expected %ld\n", __LINE__, #a, _a, _b); g_failures++; } } while (0)
#define CHECK_NEAR(a, b, tol) do { double _a = (a), _b = (b); if (fabs(_a - _b) > (tol)) { printf("FAIL %d: %s == %.4f, expected %.4f\n", __LINE__, #a, _a, _b); g_failures++; } } while (0)

#define TP kAudioQueueProperty_EnableTimePitch
#define ALG kAudioQueueProperty_TimePitchAlgorithm
#define BYP kAudioQueueProperty_TimePitchBypass
#define SPEC kAudioQueueTimePitchAlgorithm_Spectral
#define TIDO kAudioQueueTimePitchAlgorithm_TimeDomain
#define VSPD kAudioQueueTimePitchAlgorithm_Varispeed
#define LQZL 0x6c717a6c /* 'lqzl' */

// Statuses the headers do not name (QUIRK)
#define ERR_NOT_ALLOWED (-66629)
#define ERR_INVALID_VALUE (-66630)

enum { RATE = 44100, NIN = 2 * RATE };

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	fflush(stdout);
	_exit(2);
}

static void returned(void* user, AudioQueueRef q, AudioQueueBufferRef b)
{
	(void) user; (void) q; (void) b;
}

// A stereo float queue that renders offline.
static AudioQueueRef new_queue(void)
{
	AudioStreamBasicDescription f;
	memset(&f, 0, sizeof(f));
	f.mSampleRate = RATE;
	f.mFormatID = kAudioFormatLinearPCM;
	f.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;
	f.mChannelsPerFrame = 2;
	f.mBitsPerChannel = 32;
	f.mFramesPerPacket = 1;
	f.mBytesPerFrame = f.mBytesPerPacket = 8;
	AudioQueueRef q = NULL;
	CHECK_EQ(AudioQueueNewOutput(&f, returned, NULL, NULL, NULL, 0, &q), noErr);
	CHECK_EQ(AudioQueueSetOfflineRenderFormat(q, &f, NULL), noErr);
	return q;
}

static OSStatus setp(AudioQueueRef q, AudioQueuePropertyID id, UInt32 v) { return AudioQueueSetProperty(q, id, &v, sizeof(v)); }
static UInt32 getp(AudioQueueRef q, AudioQueuePropertyID id)
{
	UInt32 v = 0xdeadbeef, size = sizeof(v);
	AudioQueueGetProperty(q, id, &v, &size);
	return v;
}
static OSStatus setpar(AudioQueueRef q, AudioQueueParameterID id, float v) { return AudioQueueSetParameter(q, id, v); }

static AudioQueueBufferRef enqueue_tones(AudioQueueRef q)
{
	AudioQueueBufferRef b;
	CHECK_EQ(AudioQueueAllocateBuffer(q, NIN * 8, &b), noErr);
	float* p = (float*) b->mAudioData;
	for (int i = 0; i < NIN; i++)
	{
		p[2 * i] = 0.5f * sinf(2 * (float) M_PI * 440 * i / RATE);
		p[2 * i + 1] = 0.5f * sinf(2 * (float) M_PI * 660 * i / RATE);
	}
	b->mAudioDataByteSize = NIN * 8;
	CHECK_EQ(AudioQueueEnqueueBuffer(q, b, 0, NULL), noErr);
	return b;
}

typedef struct
{
	float* left;
	float* right;
	int frames;
	double sample_time_last;	// AudioQueueGetCurrentTime after the last block with sound
	double sample_time_ratio;	// sample time per frame rendered, over the second half
} Out;

// Renders everything that is enqueued, in blocks of 4096 frames.
static void render_all(AudioQueueRef q, Out* out, int with_time)
{
	enum { CAP = 500000, BLOCK = 4096 };
	memset(out, 0, sizeof(*out));
	out->left = calloc(CAP, sizeof(float));
	out->right = calloc(CAP, sizeof(float));
	AudioQueueBufferRef ob;
	CHECK_EQ(AudioQueueAllocateBuffer(q, BLOCK * 8, &ob), noErr);
	AudioTimeStamp ts;
	memset(&ts, 0, sizeof(ts));
	ts.mFlags = kAudioTimeStampSampleTimeValid;
	double half_frames = -1, half_time = 0;
	for (int i = 0; i < 400 && out->frames + BLOCK < CAP; i++)
	{
		ts.mSampleTime = out->frames;
		if (AudioQueueOfflineRender(q, &ts, ob, BLOCK) != noErr)
		{
			printf("FAIL %d: AudioQueueOfflineRender\n", __LINE__);
			g_failures++;
			break;
		}
		int n = ob->mAudioDataByteSize / 8;
		if (!n)
			break;
		float* o = (float*) ob->mAudioData;
		for (int k = 0; k < n; k++)
		{
			out->left[out->frames + k] = o[2 * k];
			out->right[out->frames + k] = o[2 * k + 1];
		}
		out->frames += n;
		if (with_time)
		{
			AudioTimeStamp now;
			Boolean discontinuity;
			if (AudioQueueGetCurrentTime(q, NULL, &now, &discontinuity) == noErr)
			{
				out->sample_time_last = now.mSampleTime;
				if (half_frames < 0 && out->frames >= 20000)
				{
					half_frames = out->frames;
					half_time = now.mSampleTime;
				}
			}
		}
	}
	if (half_frames > 0 && out->frames > half_frames + 8192)
		out->sample_time_ratio = (out->sample_time_last - half_time) / (out->frames - half_frames);
}

static void free_out(Out* o) { free(o->left); free(o->right); }

// The frequency of a tone from its rising zero crossings, over the middle of the sound.
static double frequency(const float* x, int n)
{
	int end = n;
	while (end > 0 && fabsf(x[end - 1]) < 1e-4f)
		end--;
	int a = end / 5, b = end - end / 5, count = 0, first = 0, last = 0;
	for (int i = a + 1; i < b; i++)
		if (x[i - 1] < 0 && x[i] >= 0)
		{
			if (!count)
				first = i;
			last = i;
			count++;
		}
	return count < 2 ? 0 : (count - 1) * (double) RATE / (last - first);
}

static int sound_frames(const float* x, int n)
{
	while (n > 0 && fabsf(x[n - 1]) < 1e-4f)
		n--;
	return n;
}

// One rendering with an algorithm (0: the effect is not enabled), a play rate and a pitch.
static void run_case(UInt32 algorithm, float rate, float pitch, Out* out, int with_time)
{
	AudioQueueRef q = new_queue();
	if (algorithm)
	{
		CHECK_EQ(setp(q, TP, 1), noErr);
		CHECK_EQ(setp(q, ALG, algorithm), noErr);
	}
	CHECK_EQ(setpar(q, kAudioQueueParam_PlayRate, rate), noErr);
	CHECK_EQ(setpar(q, kAudioQueueParam_Pitch, pitch), noErr);
	enqueue_tones(q);
	CHECK_EQ(AudioQueueStart(q, NULL), noErr);
	render_all(q, out, with_time);
	AudioQueueStop(q, true);
	AudioQueueDispose(q, true);
}

// The result of a case: duration within 8 % (plus the lookahead of the algorithm) of 2 s / speed, tones at
// the frequencies given.
static void expect(const char* name, const Out* o, double seconds, double hz_left, double hz_right)
{
	const double got = sound_frames(o->left, o->frames) / (double) RATE;
	const double fl = frequency(o->left, o->frames), fr = frequency(o->right, o->frames);
	if (fabs(got - seconds) > 0.08 * seconds + 0.06 || fabs(fl - hz_left) > 0.015 * hz_left || fabs(fr - hz_right) > 0.015 * hz_right)
	{
		printf("FAIL %s: %.3f s (expected %.3f), left %.1f Hz (%.1f), right %.1f Hz (%.1f)\n", name, got, seconds, fl, hz_left, fr, hz_right);
		g_failures++;
	}
}

static void test_defaults_and_ranges(void)
{
	AudioQueueRef q = new_queue();
	float v = -1;
	CHECK_EQ(getp(q, TP), 0);
	CHECK_EQ(AudioQueueGetParameter(q, kAudioQueueParam_PlayRate, &v), noErr);
	CHECK_NEAR(v, 1, 0);
	CHECK_EQ(AudioQueueGetParameter(q, kAudioQueueParam_Pitch, &v), noErr);
	CHECK_NEAR(v, 0, 0);
	// The play rate and pitch are accepted as they are, whatever the effect does with them
	CHECK_EQ(setpar(q, kAudioQueueParam_PlayRate, 2.5f), noErr);
	CHECK_EQ(setpar(q, kAudioQueueParam_Pitch, 3000), noErr);
	AudioQueueGetParameter(q, kAudioQueueParam_Pitch, &v);
	CHECK_NEAR(v, 3000, 0);
	AudioQueueDispose(q, true);
}

// QUIRK: the statuses of the time/pitch properties (see AudioQueueOutput::setProperty).
static void test_quirk_timepitch_property_status(void)
{
	// stopped, not primed
	AudioQueueRef q = new_queue();
	CHECK_EQ(setp(q, ALG, TIDO), ERR_INVALID_VALUE);	// the effect is not enabled
	CHECK_EQ(setp(q, BYP, 1), ERR_INVALID_VALUE);
	CHECK_EQ(setp(q, TP, 1), noErr);
	CHECK_EQ(getp(q, TP), 1);
	CHECK_EQ(setp(q, TP, 2), noErr);
	CHECK_EQ(getp(q, TP), 1);	// nonzero reads back as 1
	CHECK_EQ(setp(q, ALG, SPEC), noErr);
	CHECK_EQ(setp(q, ALG, TIDO), noErr);
	CHECK_EQ(setp(q, ALG, VSPD), noErr);
	CHECK_EQ(setp(q, ALG, LQZL), ERR_NOT_ALLOWED);
	CHECK_EQ(setp(q, ALG, 0x626f6775 /* 'bogu' */), ERR_INVALID_VALUE);
	CHECK_EQ(setp(q, ALG, 0), ERR_INVALID_VALUE);
	CHECK_EQ(setp(q, ALG, 1), ERR_INVALID_VALUE);
	CHECK_EQ(setp(q, BYP, 1), ERR_INVALID_VALUE);	// never started
	CHECK_EQ(AudioQueueSetProperty(q, ALG, (UInt16[]) { 1 }, 2), kAudioQueueErr_InvalidPropertySize);
	CHECK_EQ(AudioQueueSetProperty(q, TP, (UInt32[]) { 1, 1 }, 8), kAudioQueueErr_InvalidPropertySize);
	AudioQueueDispose(q, true);

	// primed: the effect can be switched off, the algorithm changed, but it cannot be switched on
	q = new_queue();
	setp(q, TP, 1);
	setp(q, ALG, SPEC);
	AudioQueueBufferRef b;
	CHECK_EQ(AudioQueueAllocateBuffer(q, 4096, &b), noErr);
	memset(b->mAudioData, 0, 4096);
	b->mAudioDataByteSize = 4096;
	CHECK_EQ(AudioQueueEnqueueBuffer(q, b, 0, NULL), noErr);
	UInt32 prepared = 0;
	CHECK_EQ(AudioQueuePrime(q, 0, &prepared), noErr);
	CHECK_EQ(setp(q, TP, 1), noErr);
	CHECK_EQ(setp(q, ALG, TIDO), noErr);
	CHECK_EQ(setp(q, ALG, LQZL), ERR_NOT_ALLOWED);
	CHECK_EQ(setp(q, BYP, 1), ERR_INVALID_VALUE);
	CHECK_EQ(setp(q, TP, 0), noErr);
	CHECK_EQ(setp(q, TP, 1), ERR_NOT_ALLOWED);
	AudioQueueDispose(q, true);

	// AudioQueueReset allows it again
	q = new_queue();
	CHECK_EQ(AudioQueueAllocateBuffer(q, 4096, &b), noErr);
	memset(b->mAudioData, 0, 4096);
	b->mAudioDataByteSize = 4096;
	CHECK_EQ(AudioQueueEnqueueBuffer(q, b, 0, NULL), noErr);
	CHECK_EQ(AudioQueuePrime(q, 0, &prepared), noErr);
	CHECK_EQ(AudioQueueReset(q), noErr);
	CHECK_EQ(setp(q, TP, 1), noErr);
	AudioQueueDispose(q, true);

	// running: nothing but the bypass can be changed
	q = new_queue();
	setp(q, TP, 1);
	setp(q, ALG, SPEC);
	CHECK_EQ(AudioQueueAllocateBuffer(q, 4096, &b), noErr);
	memset(b->mAudioData, 0, 4096);
	b->mAudioDataByteSize = 4096;
	CHECK_EQ(AudioQueueEnqueueBuffer(q, b, 0, NULL), noErr);
	CHECK_EQ(AudioQueueStart(q, NULL), noErr);
	CHECK_EQ(setp(q, TP, 1), kAudioQueueErr_InvalidRunState);
	CHECK_EQ(setp(q, TP, 0), kAudioQueueErr_InvalidRunState);
	CHECK_EQ(setp(q, ALG, TIDO), kAudioQueueErr_InvalidRunState);
	CHECK_EQ(setp(q, BYP, 1), noErr);
	CHECK_EQ(setp(q, BYP, 0), noErr);
	CHECK_EQ(setp(q, BYP, 7), noErr);
	AudioQueueStop(q, true);
	// stopped again: a queue that has been started is not locked
	CHECK_EQ(setp(q, ALG, TIDO), noErr);
	CHECK_EQ(setp(q, BYP, 1), noErr);
	CHECK_EQ(setp(q, TP, 0), noErr);
	CHECK_EQ(setp(q, TP, 1), noErr);
	AudioQueueDispose(q, true);

	// started without the effect: enabling is allowed after stopping, and a started queue without it refuses the bypass
	q = new_queue();
	CHECK_EQ(AudioQueueAllocateBuffer(q, 4096, &b), noErr);
	memset(b->mAudioData, 0, 4096);
	b->mAudioDataByteSize = 4096;
	CHECK_EQ(AudioQueueEnqueueBuffer(q, b, 0, NULL), noErr);
	CHECK_EQ(AudioQueueStart(q, NULL), noErr);
	CHECK_EQ(setp(q, BYP, 1), ERR_INVALID_VALUE);
	AudioQueueStop(q, true);
	CHECK_EQ(setp(q, TP, 1), noErr);
	AudioQueueDispose(q, true);
}

static void test_without_the_effect(void)
{
	Out o;
	// no effect: the play rate and pitch do nothing
	run_case(0, 2.0f, 1200, &o, 0);
	CHECK_EQ(o.frames, NIN);
	expect("effect off, rate 2, pitch 1200", &o, 2.0, 440, 660);
	free_out(&o);
	// Spectral at the neutral settings leaves the audio as it is (on macOS it delays it by about 80 ms)
	run_case(SPEC, 1.0f, 0, &o, 0);
	expect("Spectral, rate 1", &o, 2.0, 440, 660);
	free_out(&o);
}

static void test_play_rate_keeps_the_pitch(void)
{
	static const struct { UInt32 algorithm; const char* name; float rate; } cases[] = {
		{ SPEC, "Spectral", 0.5f }, { SPEC, "Spectral", 2.0f },
		{ TIDO, "TimeDomain", 0.5f }, { TIDO, "TimeDomain", 1.5f }, { TIDO, "TimeDomain", 2.0f },
		// beyond 0.5...2: accepted by the queue
		{ TIDO, "TimeDomain", 4.0f }, { SPEC, "Spectral", 0.25f },
	};
	for (unsigned i = 0; i < sizeof(cases) / sizeof(cases[0]); i++)
	{
		Out o;
		char name[64];
		snprintf(name, sizeof(name), "%s rate %.2f", cases[i].name, cases[i].rate);
		run_case(cases[i].algorithm, cases[i].rate, 0, &o, 0);
		expect(name, &o, 2.0 / cases[i].rate, 440, 660);
		free_out(&o);
	}
}

static void test_pitch(void)
{
	Out o;
	run_case(SPEC, 1.0f, 1200, &o, 0);
	expect("Spectral pitch +1200", &o, 2.0, 880, 1320);
	free_out(&o);
	run_case(SPEC, 1.0f, -1200, &o, 0);
	expect("Spectral pitch -1200", &o, 2.0, 220, 330);
	free_out(&o);
	// +-2400 is the limit: more is cut to it
	run_case(SPEC, 1.0f, 3000, &o, 0);
	expect("Spectral pitch +3000", &o, 2.0, 1760, 2640);
	free_out(&o);
	run_case(SPEC, 1.0f, -3000, &o, 0);
	expect("Spectral pitch -3000", &o, 2.0, 110, 165);
	free_out(&o);
	// the pitch with the play rate
	run_case(SPEC, 2.0f, 1200, &o, 0);
	expect("Spectral rate 2, pitch +1200", &o, 1.0, 880, 1320);
	free_out(&o);
	// only Spectral shifts the pitch
	run_case(TIDO, 1.0f, 1200, &o, 0);
	expect("TimeDomain pitch +1200", &o, 2.0, 440, 660);
	free_out(&o);
	// Varispeed changes the speed with the pitch
	run_case(VSPD, 1.0f, 1200, &o, 0);
	expect("Varispeed pitch +1200", &o, 1.0, 880, 1320);
	free_out(&o);
	run_case(VSPD, 1.0f, -1200, &o, 0);
	expect("Varispeed pitch -1200", &o, 4.0, 220, 330);
	free_out(&o);
}

// The queue's timeline is in media time: the sample time goes at the play rate, and ends where the audio ends.
static void test_sample_time(void)
{
	Out o;
	static const float rates[] = { 0.5f, 2.0f };
	for (int i = 0; i < 2; i++)
	{
		run_case(TIDO, rates[i], 0, &o, 1);
		CHECK_NEAR(o.sample_time_ratio, rates[i], 0.12 * rates[i]);
		CHECK_NEAR(o.sample_time_last, NIN, 0.1 * NIN);
		free_out(&o);
	}
	run_case(0, 1.0f, 0, &o, 1);
	CHECK_NEAR(o.sample_time_ratio, 1.0, 0.01);
	free_out(&o);
}

// The bypass of a started queue plays the audio unchanged.
static void test_bypass(void)
{
	AudioQueueRef q = new_queue();
	setp(q, TP, 1);
	setp(q, ALG, TIDO);
	setpar(q, kAudioQueueParam_PlayRate, 2.0f);
	enqueue_tones(q);
	CHECK_EQ(AudioQueueStart(q, NULL), noErr);
	CHECK_EQ(setp(q, BYP, 1), noErr);
	Out o;
	render_all(q, &o, 0);
	expect("bypassed", &o, 2.0, 440, 660);
	free_out(&o);
	AudioQueueStop(q, true);
	AudioQueueDispose(q, true);
}

int main(int argc, char** argv)
{
	signal(SIGALRM, on_alarm);
	alarm(120);

	test_defaults_and_ranges();
	test_quirk_timepitch_property_status();
	test_without_the_effect();
	test_play_rate_keeps_the_pitch();
	test_pitch();
	// Meaningless on 10.14.4 (see above)
	const int mac = argc > 1 && !strcmp(argv[1], "mac");
	test_bypass();
	if (!mac)
		test_sample_time();

	if (g_failures)
	{
		printf("%d check(s) failed\n", g_failures);
		return 1;
	}
	printf("ALL PASSED\n");
	return 0;
}
