// Copyright (C) 2026 Lubos Dolezel
//
// AudioComponentFindNext() returns the newest version of a component first
// when several versions with the same type, subtype and manufacturer are
// registered.
//
// Registers, in this order:
//   A v1.0 ('Dtst'/'vord'/'Dtst'), B ('Dtst'/'othr'/'Dtst'), A v2.0
// and walks the components of type 'Dtst'.
// The program passes unchanged on macOS 27.0.1.
#include <AudioToolbox/AudioComponent.h>
#include <CoreFoundation/CoreFoundation.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void finish(int status)
{
	fflush(stdout);
	_exit(status);
}

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	finish(2);
}

// Never instantiated
static AudioComponentPlugInInterface* factory(const AudioComponentDescription* desc)
{
	(void) desc;
	return NULL;
}

static const OSType kType = 'Dtst';

int main(void)
{
	signal(SIGALRM, on_alarm);
	alarm(30);

	AudioComponentDescription a = { kType, 'vord', 'Dtst', 0, 0 };
	AudioComponentDescription b = { kType, 'othr', 'Dtst', 0, 0 };

	AudioComponent a1 = AudioComponentRegister(&a, CFSTR("Darling test: A 1.0"), 0x00010000, factory);
	AudioComponent other = AudioComponentRegister(&b, CFSTR("Darling test: B"), 0x00010000, factory);
	AudioComponent a2 = AudioComponentRegister(&a, CFSTR("Darling test: A 2.0"), 0x00020000, factory);
	CHECK(a1 != NULL && other != NULL && a2 != NULL);

	// The exact component: the newest version comes first.
	AudioComponent first = AudioComponentFindNext(NULL, &a);
	UInt32 version = 0;
	CHECK(first == a2);
	CHECK(first != NULL && AudioComponentGetVersion(first, &version) == noErr && version == 0x00020000);

	// Every component of the type, in order
	AudioComponentDescription any = { kType, 0, 0, 0, 0 };
	AudioComponent found[8];
	int count = 0;
	for (AudioComponent c = AudioComponentFindNext(NULL, &any); c != NULL && count < 8; c = AudioComponentFindNext(c, &any))
		found[count++] = c;

	printf("found %d component(s) of the test type\n", count);
	for (int i = 0; i < count; i++)
	{
		UInt32 v = 0;
		AudioComponentDescription d;
		AudioComponentGetVersion(found[i], &v);
		AudioComponentGetDescription(found[i], &d);
		printf("  %d: %s v0x%08x\n", i, d.componentSubType == 'vord' ? "A" : "B", (unsigned) v);
	}

	// All three are found, and A's versions come newest first. The order of
	// A relative to B is not checked: this implementation keeps registration
	// order there, macOS 27 sorts by subtype.
	CHECK(count == 3);
	int posA2 = -1, posA1 = -1, posB = -1;
	for (int i = 0; i < count; i++)
	{
		if (found[i] == a2)
			posA2 = i;
		else if (found[i] == a1)
			posA1 = i;
		else if (found[i] == other)
			posB = i;
	}
	CHECK(posA2 >= 0 && posA1 >= 0 && posB >= 0);
	CHECK(posA2 < posA1);
	CHECK(AudioComponentCount(&any) == 3);

	if (g_failures)
		printf("FAIL: %d check(s) failed\n", g_failures);
	else
		printf("PASS\n");
	finish(g_failures ? 1 : 0);
}
