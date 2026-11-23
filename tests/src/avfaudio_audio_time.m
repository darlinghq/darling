// CFLAGS: -framework Foundation -framework AVFAudio -framework CoreAudio
// Copyright (C) 2026 Darling developers
//
// AVAudioTime, as described on the AVFAudio pages for the class and its members
// (https://developer.apple.com/documentation/avfaudio/avaudiotime):
//  - an instance holds a host time (the mach_absolute_time clock), a sample time at a sample rate, or both;
//    hostTimeValid and sampleTimeValid say which, and audioTimeStamp gives the time as an AudioTimeStamp;
//  - the initializers (initWithAudioTimeStamp:sampleRate:, initWithHostTime:, initWithSampleTime:atRate:,
//    initWithHostTime:sampleTime:atRate:) and their timeWith... class methods set the fields they name;
//  - hostTimeForSeconds: and secondsForHostTime: convert between seconds and host time;
//  - extrapolateTimeFromAnchor: fills in the missing one of the receiver's host time and sample time from an
//    anchor with both valid, and gives nil unless the anchor has both and the receiver has a sample rate and
//    at least one valid time.
// Cases marked GUESS pin choices the pages leave open (see AVAudioTime.m).
// Prints PASS, or the failures with a non-zero exit status.
#import <AVFAudio/AVFAudio.h>
#import <CoreAudio/HostTime.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

// Conversions are checked to within a few ticks because the pages do not say how a fraction of a tick is
// rounded.
static int64_t ticksDiff(uint64_t a, uint64_t b)
{
	return a >= b ? (int64_t)(a - b) : -(int64_t)(b - a);
}

static BOOL near(uint64_t a, uint64_t b, int64_t tolerance)
{
	int64_t d = ticksDiff(a, b);
	return d >= -tolerance && d <= tolerance;
}

static void testInitializers(void)
{
	AVAudioTime* host = [AVAudioTime timeWithHostTime:123456];
	CHECK(host != nil);
	CHECK(host.hostTimeValid && host.hostTime == 123456);
	CHECK(!host.sampleTimeValid);
	CHECK((host.audioTimeStamp.mFlags & kAudioTimeStampHostTimeValid) && !(host.audioTimeStamp.mFlags & kAudioTimeStampSampleTimeValid));
	CHECK(host.audioTimeStamp.mHostTime == 123456);
	// GUESS: the host-time-only initializer gives no sample rate, so it reads 0, and an invalid field reads 0.
	CHECK(host.sampleRate == 0 && host.sampleTime == 0);

	AVAudioTime* samples = [AVAudioTime timeWithSampleTime:-480 atRate:48000];
	CHECK(samples != nil);
	CHECK(samples.sampleTimeValid && samples.sampleTime == -480 && samples.sampleRate == 48000);
	CHECK(!samples.hostTimeValid);
	CHECK(samples.hostTime == 0); // GUESS: an invalid field reads 0.
	CHECK(samples.audioTimeStamp.mSampleTime == -480.0);
	CHECK((samples.audioTimeStamp.mFlags & kAudioTimeStampSampleTimeValid) && !(samples.audioTimeStamp.mFlags & kAudioTimeStampHostTimeValid));

	AVAudioTime* both = [[[AVAudioTime alloc] initWithHostTime:777 sampleTime:44100 atRate:44100] autorelease];
	CHECK(both != nil);
	CHECK(both.hostTimeValid && both.hostTime == 777);
	CHECK(both.sampleTimeValid && both.sampleTime == 44100 && both.sampleRate == 44100);
	CHECK(both.audioTimeStamp.mHostTime == 777 && both.audioTimeStamp.mSampleTime == 44100.0);
	CHECK([[both class] isEqual:[AVAudioTime class]]);

	AVAudioTime* viaInit = [[[AVAudioTime alloc] initWithHostTime:9] autorelease];
	CHECK(viaInit.hostTimeValid && viaInit.hostTime == 9 && !viaInit.sampleTimeValid);
	viaInit = [[[AVAudioTime alloc] initWithSampleTime:5 atRate:8000] autorelease];
	CHECK(viaInit.sampleTimeValid && viaInit.sampleTime == 5 && viaInit.sampleRate == 8000 && !viaInit.hostTimeValid);
}

static void testAudioTimeStamp(void)
{
	AudioTimeStamp ts;
	memset(&ts, 0, sizeof(ts));
	ts.mSampleTime = 1000;
	ts.mHostTime = 5000;
	ts.mRateScalar = 1.5;
	ts.mFlags = kAudioTimeStampSampleHostTimeValid | kAudioTimeStampRateScalarValid;

	AVAudioTime* t = [AVAudioTime timeWithAudioTimeStamp:&ts sampleRate:22050];
	CHECK(t != nil);
	CHECK(t.hostTimeValid && t.hostTime == 5000);
	CHECK(t.sampleTimeValid && t.sampleTime == 1000 && t.sampleRate == 22050);
	AudioTimeStamp back = t.audioTimeStamp;
	CHECK(back.mFlags == ts.mFlags && back.mHostTime == 5000 && back.mSampleTime == 1000.0);
	// GUESS: the fields of the time stamp beyond the two times are kept as given.
	CHECK(back.mRateScalar == 1.5);

	// Only the fields the flags mark are valid.
	ts.mFlags = kAudioTimeStampSampleTimeValid;
	t = [AVAudioTime timeWithAudioTimeStamp:&ts sampleRate:22050];
	CHECK(t.sampleTimeValid && !t.hostTimeValid);
	CHECK(t.sampleTime == 1000 && t.hostTime == 0); // GUESS: the field of a clear flag reads 0.
	ts.mFlags = kAudioTimeStampHostTimeValid;
	t = [AVAudioTime timeWithAudioTimeStamp:&ts sampleRate:22050];
	CHECK(t.hostTimeValid && !t.sampleTimeValid && t.hostTime == 5000 && t.sampleRate == 22050);
	ts.mFlags = kAudioTimeStampNothingValid;
	t = [AVAudioTime timeWithAudioTimeStamp:&ts sampleRate:22050];
	CHECK(t != nil && !t.hostTimeValid && !t.sampleTimeValid);

	// GUESS: a NULL time stamp gives nil (the page declares a non-optional pointer).
	const AudioTimeStamp* noStamp = NULL;
	CHECK([AVAudioTime timeWithAudioTimeStamp:noStamp sampleRate:44100] == nil);
}

static void testHostTimeConversion(void)
{
	// hostTimeForSeconds: and secondsForHostTime: go through the host clock of CoreAudio's HostTime.h.
	uint64_t second = [AVAudioTime hostTimeForSeconds:1.0];
	CHECK(second == AudioConvertNanosToHostTime(1000000000ull));
	CHECK(near(second, (uint64_t) AudioGetHostClockFrequency(), 1));
	CHECK(fabs([AVAudioTime secondsForHostTime:second] - 1.0) < 1e-6);
	CHECK([AVAudioTime hostTimeForSeconds:0.0] == 0);
	CHECK([AVAudioTime secondsForHostTime:0] == 0.0);

	uint64_t ten = [AVAudioTime hostTimeForSeconds:10.0];
	CHECK(near(ten, 10 * second, 10));
	CHECK([AVAudioTime hostTimeForSeconds:2.5] > [AVAudioTime hostTimeForSeconds:2.0]);
	CHECK(fabs([AVAudioTime secondsForHostTime:[AVAudioTime hostTimeForSeconds:0.3]] - 0.3) < 1e-6);
	CHECK(fabs([AVAudioTime secondsForHostTime:3 * second] - 3.0) < 1e-6);

	// The host time is the mach_absolute_time clock: a time from now is a positive number of seconds.
	CHECK([AVAudioTime secondsForHostTime:AudioGetCurrentHostTime()] > 0.0);

	// GUESS: a negative or NaN number of seconds gives 0, and one beyond 64 bits of nanoseconds is limited to
	// UINT64_MAX nanoseconds before the conversion (this only checks that the limit is passed to CoreAudio's
	// conversion, which wraps for some timebases).
	CHECK([AVAudioTime hostTimeForSeconds:-1.0] == 0);
	CHECK([AVAudioTime hostTimeForSeconds:NAN] == 0);
	CHECK([AVAudioTime hostTimeForSeconds:1e300] == AudioConvertNanosToHostTime(UINT64_MAX));
}

static void testExtrapolateRefusals(void)
{
	AVAudioTime* anchor = [AVAudioTime timeWithHostTime:1000000 sampleTime:0 atRate:48000];
	AVAudioTime* sampleOnly = [AVAudioTime timeWithSampleTime:48000 atRate:48000];
	AVAudioTime* hostOnly = [AVAudioTime timeWithHostTime:1000000];

	// The anchor must have a valid host time and sample time.
	CHECK([sampleOnly extrapolateTimeFromAnchor:hostOnly] == nil);
	CHECK([sampleOnly extrapolateTimeFromAnchor:sampleOnly] == nil);
	AVAudioTime* noAnchor = nil;
	CHECK([sampleOnly extrapolateTimeFromAnchor:noAnchor] == nil);

	// The receiver must have a sample rate and a valid time.
	CHECK([hostOnly extrapolateTimeFromAnchor:anchor] == nil); // no sample rate
	AudioTimeStamp none;
	memset(&none, 0, sizeof(none));
	AVAudioTime* empty = [AVAudioTime timeWithAudioTimeStamp:&none sampleRate:48000];
	CHECK([empty extrapolateTimeFromAnchor:anchor] == nil); // no valid time
	AVAudioTime* noRate = [AVAudioTime timeWithSampleTime:48000 atRate:0];
	CHECK([noRate extrapolateTimeFromAnchor:anchor] == nil); // GUESS: a rate of 0 is no rate
}

static void testExtrapolateFromSampleTime(void)
{
	uint64_t second = [AVAudioTime hostTimeForSeconds:1.0];
	uint64_t base = 50 * second;
	AVAudioTime* anchor = [AVAudioTime timeWithHostTime:base sampleTime:1000 atRate:48000];

	// The receiver has only a sample time one second after the anchor's: its host time is one second later.
	AVAudioTime* receiver = [AVAudioTime timeWithSampleTime:1000 + 48000 atRate:48000];
	AVAudioTime* full = [receiver extrapolateTimeFromAnchor:anchor];
	CHECK(full != nil);
	CHECK(full.hostTimeValid && near(full.hostTime, base + second, 2));
	CHECK(full.sampleTimeValid && full.sampleTime == 1000 + 48000 && full.sampleRate == 48000);
	CHECK(full != receiver && !receiver.hostTimeValid); // instances are immutable

	// Before the anchor, and at the anchor.
	full = [[AVAudioTime timeWithSampleTime:1000 - 24000 atRate:48000] extrapolateTimeFromAnchor:anchor];
	CHECK(full != nil && full.hostTimeValid && near(full.hostTime, base - second / 2, 2));
	full = [[AVAudioTime timeWithSampleTime:1000 atRate:48000] extrapolateTimeFromAnchor:anchor];
	CHECK(full != nil && full.hostTime == base);

	// GUESS: the pages do not cover an anchor at another rate than the receiver's. The anchor's sample time is
	// converted to the receiver's rate, so a distance of one second at 44100 from an anchor at 48000 is one second.
	AVAudioTime* anchorSecond = [AVAudioTime timeWithHostTime:base sampleTime:48000 atRate:48000];
	full = [[AVAudioTime timeWithSampleTime:44100 atRate:44100] extrapolateTimeFromAnchor:anchorSecond];
	CHECK(full != nil && near(full.hostTime, base, 2) && full.sampleRate == 44100 && full.sampleTime == 44100);
	full = [[AVAudioTime timeWithSampleTime:66150 atRate:44100] extrapolateTimeFromAnchor:anchorSecond];
	CHECK(full != nil && near(full.hostTime, base + second / 2, 2));
	full = [[AVAudioTime timeWithSampleTime:144000 atRate:96000] extrapolateTimeFromAnchor:anchorSecond];
	CHECK(full != nil && near(full.hostTime, base + second / 2, 2));

	// GUESS: a time before the start of the host clock cannot be represented and gives nil.
	CHECK([[AVAudioTime timeWithSampleTime:-48000 * 1000 atRate:48000] extrapolateTimeFromAnchor:anchor] == nil);
}

static void testExtrapolateFromHostTime(void)
{
	uint64_t second = [AVAudioTime hostTimeForSeconds:1.0];
	uint64_t base = 50 * second;
	AVAudioTime* anchor = [AVAudioTime timeWithHostTime:base sampleTime:1000 atRate:48000];

	// A host time (with a sample rate, from a time stamp) half a second after the anchor's.
	AudioTimeStamp ts;
	memset(&ts, 0, sizeof(ts));
	ts.mHostTime = base + second / 2;
	ts.mFlags = kAudioTimeStampHostTimeValid;
	AVAudioTime* receiver = [AVAudioTime timeWithAudioTimeStamp:&ts sampleRate:48000];
	AVAudioTime* full = [receiver extrapolateTimeFromAnchor:anchor];
	CHECK(full != nil);
	CHECK(full.hostTimeValid && full.hostTime == base + second / 2);
	CHECK(full.sampleTimeValid && llabs(full.sampleTime - (1000 + 24000)) <= 1 && full.sampleRate == 48000);
	CHECK(!receiver.sampleTimeValid);

	// Before the anchor: the sample time is lower.
	ts.mHostTime = base - second;
	full = [[AVAudioTime timeWithAudioTimeStamp:&ts sampleRate:48000] extrapolateTimeFromAnchor:anchor];
	CHECK(full != nil && llabs(full.sampleTime - (1000 - 48000)) <= 1);

	// The sample time rounds to the nearest frame (GUESS): 0.75 frame at 48000 after the anchor is frame 1001,
	// which truncation would not give. 15625 ns is exactly 0.75 frame.
	ts.mHostTime = base + AudioConvertNanosToHostTime(15625);
	full = [[AVAudioTime timeWithAudioTimeStamp:&ts sampleRate:48000] extrapolateTimeFromAnchor:anchor];
	CHECK(full != nil && full.sampleTime == 1001);

	// The result carries the receiver's rate. GUESS: the anchor's sample time is converted to it (see above).
	AVAudioTime* anchorSecond = [AVAudioTime timeWithHostTime:base sampleTime:48000 atRate:48000];
	ts.mHostTime = base + second;
	full = [[AVAudioTime timeWithAudioTimeStamp:&ts sampleRate:44100] extrapolateTimeFromAnchor:anchorSecond];
	CHECK(full != nil && llabs(full.sampleTime - (44100 + 44100)) <= 1 && full.sampleRate == 44100);
	ts.mHostTime = base;
	full = [[AVAudioTime timeWithAudioTimeStamp:&ts sampleRate:44100] extrapolateTimeFromAnchor:anchorSecond];
	CHECK(full != nil && llabs(full.sampleTime - 44100) <= 1);

	// GUESS: a receiver with both times valid is returned as an equal time.
	AVAudioTime* complete = [AVAudioTime timeWithHostTime:base + 5 sampleTime:77 atRate:48000];
	full = [complete extrapolateTimeFromAnchor:anchor];
	CHECK(full != nil && full.hostTime == base + 5 && full.sampleTime == 77 && full.sampleRate == 48000);
}

int main(void)
{
	@autoreleasepool
	{
		testInitializers();
		testAudioTimeStamp();
		testHostTimeConversion();
		testExtrapolateRefusals();
		testExtrapolateFromSampleTime();
		testExtrapolateFromHostTime();
	}

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
