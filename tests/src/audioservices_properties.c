// Copyright (C) 2026 Lubos Dolezel
//
// AudioServicesGetPropertyInfo/GetProperty/SetProperty report
// kAudioServicesUnsupportedPropertyError for a property ID that is not defined
// (AudioServicesGetProperty only has to fail: macOS 27 returns an undocumented
// error code for it), and AudioServicesRemoveSystemSoundCompletion accepts an
// unregistered sound ID.
#include <AudioToolbox/AudioServices.h>
#include <stdio.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

int main(void)
{
	const AudioServicesPropertyID bogus = 'zzzz';
	UInt32 size = 0;
	Boolean writable = false;
	UInt32 value = 0, ioSize = sizeof(value);

	CHECK(AudioServicesGetPropertyInfo(bogus, 0, NULL, &size, &writable) == kAudioServicesUnsupportedPropertyError);
	CHECK(AudioServicesGetProperty(bogus, 0, NULL, &ioSize, &value) != noErr);
	CHECK(AudioServicesSetProperty(bogus, 0, NULL, sizeof(value), &value) == kAudioServicesUnsupportedPropertyError);

	AudioServicesRemoveSystemSoundCompletion(0xFFFFFFF0);

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
