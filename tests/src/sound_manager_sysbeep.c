// Copyright (C) 2026 Lubos Dolezel
//
// SysBeep of the Sound Manager (32-bit only, where macOS has it): plays the alert sound and returns at once, whatever
// the duration it is given (the duration is not used on Mac OS X). With no audio output device there is no sound, and
// the call is as quiet as it is without the device on macOS.
#include <stdio.h>
#include <sys/time.h>

#ifdef __LP64__
int main(void)
{
	printf("PASS sound_manager_sysbeep (no 64-bit API)\n");
	return 0;
}
#else

#include <Carbon/Carbon.h>
#include <unistd.h>

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

static int g_failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static double now_ms(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
}

int main(void)
{
	// the call returns at once, for any duration, however many times it is made
	double start = now_ms();
	SysBeep(0);
	SysBeep(30);
	SysBeep(-1);
	for (int i = 0; i < 5; i++)
		SysBeep(10);
	CHECK(now_ms() - start < 500);

	// what the beeps used is given back: channels can still be made and disposed of
	usleep(1500000);
	SndChannelPtr channel = NULL;
	OSErr err = SndNewChannel(&channel, sampledSynth, initMono, NULL);
	CHECK(err == noErr || err == -2004 /* no output device */);
	if (err == noErr)
		CHECK(SndDisposeChannel(channel, true) == noErr);

	printf(g_failures ? "FAIL sound_manager_sysbeep\n" : "PASS sound_manager_sysbeep\n");
	return g_failures != 0;
}
#endif
