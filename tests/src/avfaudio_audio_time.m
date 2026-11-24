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
// Expectations that go beyond the pages are observations of macOS 26.7.3 and 10.14.4 (see AVAudioTime.m; marked QUIRK where
// they are undocumented behaviour).
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
	// The host-time-only initializer gives no sample rate, so it reads 0, and its fields that were not given read 0.
	CHECK(host.sampleRate == 0 && host.sampleTime == 0);

	AVAudioTime* samples = [AVAudioTime timeWithSampleTime:-480 atRate:48000];
	CHECK(samples != nil);
	CHECK(samples.sampleTimeValid && samples.sampleTime == -480 && samples.sampleRate == 48000);
	CHECK(!samples.hostTimeValid);
	CHECK(samples.hostTime == 0); // not given
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
	// The fields of the time stamp beyond the two times are kept as given.
	CHECK(back.mRateScalar == 1.5);

	// The flags say which fields are valid; a field whose flag is clear is kept as it was given and reads back.
	ts.mFlags = kAudioTimeStampSampleTimeValid;
	t = [AVAudioTime timeWithAudioTimeStamp:&ts sampleRate:22050];
	CHECK(t.sampleTimeValid && !t.hostTimeValid);
	CHECK(t.sampleTime == 1000 && t.hostTime == 5000);
	ts.mFlags = kAudioTimeStampHostTimeValid;
	t = [AVAudioTime timeWithAudioTimeStamp:&ts sampleRate:22050];
	CHECK(t.hostTimeValid && !t.sampleTimeValid && t.hostTime == 5000 && t.sampleRate == 22050 && t.sampleTime == 1000);
	ts.mFlags = kAudioTimeStampNothingValid;
	t = [AVAudioTime timeWithAudioTimeStamp:&ts sampleRate:22050];
	CHECK(t != nil && !t.hostTimeValid && !t.sampleTimeValid);

	// QUIRK: the sample time of a time stamp becomes an integer by truncation, and a NaN or a number out of range reads
	// as the smallest integer.
	ts.mFlags = kAudioTimeStampSampleTimeValid;
	static const struct { double sampleTime; int64_t expected; } positions[] = {
		{ 3.7, 3 }, { -3.7, -3 }, { 0.5, 0 }, { -0.5, 0 }, { 4503599627370497.0, 4503599627370497 },
		{ 1e30, INT64_MIN }, { -1e30, INT64_MIN }, { 9.3e18, INT64_MIN }, { NAN, INT64_MIN }, { INFINITY, INT64_MIN }, { -INFINITY, INT64_MIN },
	};
	for (size_t i = 0; i < sizeof positions / sizeof positions[0]; i++)
	{
		ts.mSampleTime = positions[i].sampleTime;
		t = [AVAudioTime timeWithAudioTimeStamp:&ts sampleRate:44100];
		CHECK(t.sampleTime == positions[i].expected);
	}
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

	// QUIRK: the number of nanoseconds is truncated (1.6 ns is 1), a negative one wraps, and NaN and minus infinity give 2^63
	// (macOS 10.14.4 and 26.7.3; see nanosFromDouble in AVAudioTime.m).
	CHECK([AVAudioTime hostTimeForSeconds:1.6e-9] == AudioConvertNanosToHostTime(1));
	CHECK([AVAudioTime hostTimeForSeconds:2.5e-9] == AudioConvertNanosToHostTime(2));
	CHECK([AVAudioTime hostTimeForSeconds:5e-10] == 0);
	CHECK([AVAudioTime hostTimeForSeconds:-1.0] == AudioConvertNanosToHostTime(UINT64_MAX - 999999999ull));
	CHECK([AVAudioTime hostTimeForSeconds:-1e-9] == AudioConvertNanosToHostTime(UINT64_MAX));
	CHECK([AVAudioTime hostTimeForSeconds:1.8e10] == AudioConvertNanosToHostTime(18000000000000000000ull));
	// 2^64 nanoseconds and more, and infinity, give 0 on macOS 10.14.4 and 2^63 on 26.7.3: not pinned here.
	CHECK([AVAudioTime hostTimeForSeconds:-INFINITY] == AudioConvertNanosToHostTime(0x8000000000000000ull));
	CHECK([AVAudioTime hostTimeForSeconds:NAN] == AudioConvertNanosToHostTime(0x8000000000000000ull));
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
	CHECK([noRate extrapolateTimeFromAnchor:anchor] == nil); // a rate of 0 is no rate
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

	// The anchor's own sample rate is not used: the distance in frames is counted in the receiver's rate. From an anchor at
	// sample 48000, the sample 44100 at 44100 Hz is 3900 frames, 0.0884 s, before it (observed on macOS 26.7.3 and 10.14.4).
	AVAudioTime* anchorSecond = [AVAudioTime timeWithHostTime:base sampleTime:48000 atRate:48000];
	full = [[AVAudioTime timeWithSampleTime:44100 atRate:44100] extrapolateTimeFromAnchor:anchorSecond];
	CHECK(full != nil && near(full.hostTime, base - (uint64_t) (second * 3900.0 / 44100.0), 2) && full.sampleRate == 44100 && full.sampleTime == 44100);
	full = [[AVAudioTime timeWithSampleTime:66150 atRate:44100] extrapolateTimeFromAnchor:anchorSecond];
	CHECK(full != nil && near(full.hostTime, base + (uint64_t) (second * 18150.0 / 44100.0), 2));
	full = [[AVAudioTime timeWithSampleTime:144000 atRate:96000] extrapolateTimeFromAnchor:anchorSecond];
	CHECK(full != nil && near(full.hostTime, base + second, 2));
	// An anchor without a sample rate works the same, and so does a negative rate (the division it is).
	AVAudioTime* anchorNoRate = [AVAudioTime timeWithHostTime:base sampleTime:1000 atRate:0];
	full = [[AVAudioTime timeWithSampleTime:49000 atRate:48000] extrapolateTimeFromAnchor:anchorNoRate];
	CHECK(full != nil && near(full.hostTime, base + second, 2));
	full = [[AVAudioTime timeWithSampleTime:48000 atRate:-48000] extrapolateTimeFromAnchor:anchor];
	CHECK(full != nil && near(full.hostTime, base - (uint64_t) (second * 47000.0 / 48000.0), 2));

	// QUIRK: a time before the start of the host clock wraps (as an unsigned number) and is not refused (macOS 26.7.3, 10.14.4).
	full = [[AVAudioTime timeWithSampleTime:-48000 * 1000 atRate:48000] extrapolateTimeFromAnchor:anchor];
	CHECK(full != nil && near(full.hostTime, base - [AVAudioTime hostTimeForSeconds:1000.0208333333], 4) && full.hostTime > base);
}

// A rate scalar scales the extrapolation: the receiver's if it has one, else the anchor's. The result keeps it, with its flag.
static void testExtrapolateRateScalar(void)
{
	uint64_t second = [AVAudioTime hostTimeForSeconds:1.0];
	uint64_t base = 50 * second;
	AudioTimeStamp as;
	memset(&as, 0, sizeof(as));
	as.mHostTime = base;
	as.mSampleTime = 1000;
	as.mRateScalar = 2.0;
	as.mWordClockTime = 99;
	as.mFlags = kAudioTimeStampHostTimeValid | kAudioTimeStampSampleTimeValid | kAudioTimeStampRateScalarValid | kAudioTimeStampWordClockTimeValid;
	AVAudioTime* scaled = [AVAudioTime timeWithAudioTimeStamp:&as sampleRate:48000];

	// Sample to host: one second of frames is two seconds of host time at a scalar of 2.
	AVAudioTime* full = [[AVAudioTime timeWithSampleTime:49000 atRate:48000] extrapolateTimeFromAnchor:scaled];
	CHECK(full != nil && near(full.hostTime, base + 2 * second, 4) && full.sampleTime == 49000);
	CHECK(full.audioTimeStamp.mRateScalar == 2.0 && (full.audioTimeStamp.mFlags & kAudioTimeStampRateScalarValid));
	CHECK(!(full.audioTimeStamp.mFlags & kAudioTimeStampWordClockTimeValid) && full.audioTimeStamp.mWordClockTime == 0);

	// Host to sample: one second of host time is half a second of frames.
	AudioTimeStamp ts;
	memset(&ts, 0, sizeof(ts));
	ts.mHostTime = base + second;
	ts.mFlags = kAudioTimeStampHostTimeValid;
	full = [[AVAudioTime timeWithAudioTimeStamp:&ts sampleRate:48000] extrapolateTimeFromAnchor:scaled];
	CHECK(full != nil && llabs(full.sampleTime - (1000 + 24000)) <= 1 && full.audioTimeStamp.mRateScalar == 2.0);

	// The receiver's own scalar wins.
	memset(&ts, 0, sizeof(ts));
	ts.mSampleTime = 49000;
	ts.mRateScalar = 3.0;
	ts.mFlags = kAudioTimeStampSampleTimeValid | kAudioTimeStampRateScalarValid;
	full = [[AVAudioTime timeWithAudioTimeStamp:&ts sampleRate:48000] extrapolateTimeFromAnchor:scaled];
	CHECK(full != nil && near(full.hostTime, base + 3 * second, 4) && full.audioTimeStamp.mRateScalar == 3.0);

	// A scalar whose flag is clear is not used.
	as.mFlags = kAudioTimeStampHostTimeValid | kAudioTimeStampSampleTimeValid;
	AVAudioTime* unflagged = [AVAudioTime timeWithAudioTimeStamp:&as sampleRate:48000];
	full = [[AVAudioTime timeWithSampleTime:49000 atRate:48000] extrapolateTimeFromAnchor:unflagged];
	CHECK(full != nil && near(full.hostTime, base + second, 4) && !(full.audioTimeStamp.mFlags & kAudioTimeStampRateScalarValid));
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

	// The sample time rounds to the nearest frame, halves to even: 0.75 frame at 48000 after the anchor is frame 1001, which
	// truncation would not give; 1.5 frames after is 1002 and 1.5 before is 998 (observed on macOS 26.7.3 and 10.14.4).
	// 15625 ns is exactly 0.75 frame.
	ts.mHostTime = base + AudioConvertNanosToHostTime(15625);
	full = [[AVAudioTime timeWithAudioTimeStamp:&ts sampleRate:48000] extrapolateTimeFromAnchor:anchor];
	CHECK(full != nil && full.sampleTime == 1001);
	ts.mHostTime = base + AudioConvertNanosToHostTime(31250);
	full = [[AVAudioTime timeWithAudioTimeStamp:&ts sampleRate:48000] extrapolateTimeFromAnchor:anchor];
	CHECK(full != nil && full.sampleTime == 1002);
	ts.mHostTime = base - AudioConvertNanosToHostTime(31250);
	full = [[AVAudioTime timeWithAudioTimeStamp:&ts sampleRate:48000] extrapolateTimeFromAnchor:anchor];
	CHECK(full != nil && full.sampleTime == 998);

	// The result carries the receiver's rate; the anchor's sample time (48000 here) is counted in the receiver's rate, not
	// converted to it (observed on macOS 26.7.3 and 10.14.4).
	AVAudioTime* anchorSecond = [AVAudioTime timeWithHostTime:base sampleTime:48000 atRate:48000];
	ts.mHostTime = base + second;
	full = [[AVAudioTime timeWithAudioTimeStamp:&ts sampleRate:44100] extrapolateTimeFromAnchor:anchorSecond];
	CHECK(full != nil && llabs(full.sampleTime - (48000 + 44100)) <= 1 && full.sampleRate == 44100);
	ts.mHostTime = base;
	full = [[AVAudioTime timeWithAudioTimeStamp:&ts sampleRate:44100] extrapolateTimeFromAnchor:anchorSecond];
	CHECK(full != nil && llabs(full.sampleTime - 48000) <= 1);

	// A host time more than 2^63 ticks above the anchor's counts as before it (a signed difference).
	ts.mHostTime = UINT64_MAX;
	full = [[AVAudioTime timeWithAudioTimeStamp:&ts sampleRate:48000] extrapolateTimeFromAnchor:anchor];
	CHECK(full != nil && full.sampleTime < 1000);

	// A receiver with both times valid is returned as an equal time (a new object).
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
		testExtrapolateRateScalar();
		testExtrapolateFromHostTime();
	}

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
