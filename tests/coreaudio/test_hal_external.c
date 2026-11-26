// Copyright (C) 2026 Lubos Dolezel
//
// Another client of the sound server changes the volume and the mute of a sink and of a source: the listeners of
// the devices are told, with the address that changed, and what is read afterwards is the new value.
// test_hal_external.sh makes the changes with pactl once this prints READY.
#include "hal_test.h"
#include <unistd.h>

static volatile int volumeHeard, muteHeard, inputVolumeHeard;

static OSStatus Heard(AudioObjectID o, UInt32 count, const AudioObjectPropertyAddress* a, void* ctx)
{
	for (UInt32 i = 0; i < count; i++)
	{
		if (a[i].mSelector == kAudioDevicePropertyVolumeScalar && a[i].mElement == 0)
		{
			if (a[i].mScope == kOutput)
				volumeHeard++;
			else
				inputVolumeHeard++;
		}
		if (a[i].mSelector == kAudioDevicePropertyMute)
			muteHeard++;
	}
	return 0;
}

static void WaitFor(volatile int* what)
{
	for (int i = 0; i < 100 && !*what; i++)
		usleep(100000);
}

int main(void)
{
	AudioDeviceID sink = DeviceWithUID("darling-hal-48k"), source = DeviceWithUID("darling-hal-mic");
	CHECK(sink != kAudioObjectUnknown && source != kAudioObjectUnknown, "devices missing");
	AudioObjectPropertyAddress volume = Addr(kAudioDevicePropertyVolumeScalar, kOutput, 0);
	AudioObjectPropertyAddress mute = Addr(kAudioDevicePropertyMute, kOutput, 0);
	AudioObjectPropertyAddress inputVolume = Addr(kAudioDevicePropertyVolumeScalar, kInput, 0);
	CHECK(AudioObjectAddPropertyListener(sink, &volume, Heard, NULL) == 0, "listener");
	CHECK(AudioObjectAddPropertyListener(sink, &mute, Heard, NULL) == 0, "listener");
	CHECK(AudioObjectAddPropertyListener(source, &inputVolume, Heard, NULL) == 0, "listener");

	printf("READY\n");
	fflush(stdout);

	WaitFor(&volumeHeard);
	CHECK(volumeHeard >= 1, "the output volume change was not heard");
	Float32 v = 0;
	CHECK(GetF32(sink, kAudioDevicePropertyVolumeScalar, kOutput, 0, &v) == 0 && fabsf(v - 0.3f) < 0.01f, "output volume %g", v);
	WaitFor(&muteHeard);
	CHECK(muteHeard >= 1, "the mute change was not heard");
	UInt32 m = 0;
	CHECK(GetU32(sink, kAudioDevicePropertyMute, kOutput, &m) == 0 && m == 1, "mute %u", (unsigned)m);
	WaitFor(&inputVolumeHeard);
	CHECK(inputVolumeHeard >= 1, "the input volume change was not heard");
	CHECK(GetF32(source, kAudioDevicePropertyVolumeScalar, kInput, 0, &v) == 0 && fabsf(v - 0.6f) < 0.01f, "input volume %g", v);

	// What is heard once is not heard again.
	const int before = volumeHeard;
	usleep(1500000);
	CHECK(volumeHeard == before, "the volume was announced again");

	printf(failures ? "test_hal_external: FAILED (%d)\n" : "test_hal_external: OK\n", failures);
	return failures != 0;
}
