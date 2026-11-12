// Copyright (C) 2026 Lubos Dolezel
//
// CoreAudio/HostTime.h: AudioGetHostClockFrequency reports the host clock ticks
// per second and AudioGetHostClockMinimumTimeDelta its resolution in ticks. They
// must agree with mach_timebase_info and with the host time conversions.
#include <CoreAudio/HostTime.h>
#include <mach/mach_time.h>
#include <math.h>
#include <stdio.h>

static int g_failures = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

int main(void)
{
	mach_timebase_info_data_t tb;
	mach_timebase_info(&tb);

	Float64 freq = AudioGetHostClockFrequency();
	UInt32 delta = AudioGetHostClockMinimumTimeDelta();
	printf("frequency %.1f, minimum delta %u\n", freq, (unsigned) delta);

	CHECK(freq > 0.0);
	CHECK(fabs(freq - 1.0e9 * tb.denom / tb.numer) < 1.0);
	CHECK(delta >= 1);

	// One second worth of nanoseconds converts to about one frequency worth of ticks.
	UInt64 ticks = AudioConvertNanosToHostTime(1000000000ULL);
	CHECK(fabs((Float64) ticks - freq) <= 1.0);
	// A second of host time converts back to a second of nanoseconds.
	UInt64 ns = AudioConvertHostTimeToNanos((UInt64) freq);
	CHECK(ns > 999999000ULL && ns < 1000001000ULL);

	// The host clock is monotonic.
	UInt64 a = AudioGetCurrentHostTime(), b = AudioGetCurrentHostTime();
	CHECK(b >= a);

	printf(g_failures ? "FAILED (%d)\n" : "PASS\n", g_failures);
	return g_failures != 0;
}
