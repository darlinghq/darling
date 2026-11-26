// Copyright (C) 2026 Lubos Dolezel
//
// The default output and input devices can be chosen and the choice is the PulseAudio default, and a device's
// mute can be read and written. Needs the null sink "darling-test-sink" and the null source
// "darling-test-source" (fixture.sh up) and leaves the defaults as it found them.
#include <CoreAudio/CoreAudio.h>
#include <CoreFoundation/CoreFoundation.h>
#include <dispatch/dispatch.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;
#define CHECK(cond, ...) do { \
	if (cond) printf("PASS: %s\n", #cond); \
	else { failures++; printf("FAIL: %s: ", #cond); printf(__VA_ARGS__); printf("\n"); } \
} while (0)

static AudioObjectPropertyAddress Addr(AudioObjectPropertySelector s, AudioObjectPropertyScope scope)
{
	AudioObjectPropertyAddress a = { s, scope, kAudioObjectPropertyElementMaster };
	return a;
}

static AudioDeviceID DeviceWithUID(const char* uid)
{
	AudioObjectPropertyAddress a = Addr(kAudioHardwarePropertyDevices, kAudioObjectPropertyScopeGlobal);
	UInt32 size = 0;
	AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &a, 0, NULL, &size);
	AudioDeviceID* ids = malloc(size);
	AudioObjectGetPropertyData(kAudioObjectSystemObject, &a, 0, NULL, &size, ids);
	AudioDeviceID found = kAudioObjectUnknown;
	for (UInt32 i = 0; i < size / sizeof(AudioDeviceID) && !found; i++) {
		CFStringRef value = NULL;
		UInt32 s = sizeof(value);
		AudioObjectPropertyAddress ua = Addr(kAudioDevicePropertyDeviceUID, kAudioObjectPropertyScopeGlobal);
		if (AudioObjectGetPropertyData(ids[i], &ua, 0, NULL, &s, &value) == 0 && value) {
			char buffer[256];
			if (CFStringGetCString(value, buffer, sizeof(buffer), kCFStringEncodingUTF8) && !strcmp(buffer, uid))
				found = ids[i];
			CFRelease(value);
		}
	}
	free(ids);
	return found;
}

static AudioDeviceID Default(AudioObjectPropertySelector selector)
{
	AudioObjectPropertyAddress a = Addr(selector, kAudioObjectPropertyScopeGlobal);
	AudioDeviceID d = 0;
	UInt32 size = sizeof(d);
	AudioObjectGetPropertyData(kAudioObjectSystemObject, &a, 0, NULL, &size, &d);
	return d;
}

static OSStatus SetDefault(AudioObjectPropertySelector selector, AudioDeviceID device)
{
	AudioObjectPropertyAddress a = Addr(selector, kAudioObjectPropertyScopeGlobal);
	return AudioObjectSetPropertyData(kAudioObjectSystemObject, &a, 0, NULL, sizeof(device), &device);
}

static dispatch_semaphore_t gChanged;
static OSStatus Listener(AudioObjectID object, UInt32 count, const AudioObjectPropertyAddress* addresses, void* context)
{
	dispatch_semaphore_signal(gChanged);
	return 0;
}

static UInt32 Mute(AudioDeviceID d, AudioObjectPropertyScope scope, OSStatus* status)
{
	AudioObjectPropertyAddress a = Addr(kAudioDevicePropertyMute, scope);
	UInt32 value = 99, size = sizeof(value);
	*status = AudioObjectGetPropertyData(d, &a, 0, NULL, &size, &value);
	return value;
}

int main(void)
{
	AudioDeviceID sink = DeviceWithUID("darling-test-sink"), source = DeviceWithUID("darling-test-source");
	CHECK(sink && source, "the fixture's devices are listed (sink %u, source %u)", (unsigned)sink, (unsigned)source);
	if (!sink || !source)
		return 1;
	AudioDeviceID oldOut = Default(kAudioHardwarePropertyDefaultOutputDevice), oldIn = Default(kAudioHardwarePropertyDefaultInputDevice);

	Boolean settable = 0;
	AudioObjectPropertyAddress outAddr = Addr(kAudioHardwarePropertyDefaultOutputDevice, kAudioObjectPropertyScopeGlobal);
	AudioObjectIsPropertySettable(kAudioObjectSystemObject, &outAddr, &settable);
	CHECK(settable, "the default output device is settable");

	gChanged = dispatch_semaphore_create(0);
	AudioObjectAddPropertyListener(kAudioObjectSystemObject, &outAddr, Listener, NULL);
	CHECK(SetDefault(kAudioHardwarePropertyDefaultOutputDevice, sink) == 0, "setting the default output to the sink works");
	CHECK(Default(kAudioHardwarePropertyDefaultOutputDevice) == sink, "and it is the default output device then (%u)", (unsigned)Default(kAudioHardwarePropertyDefaultOutputDevice));
	CHECK(dispatch_semaphore_wait(gChanged, dispatch_time(DISPATCH_TIME_NOW, 3 * NSEC_PER_SEC)) == 0, "a listener hears of the change");
	CHECK(SetDefault(kAudioHardwarePropertyDefaultInputDevice, source) == 0, "setting the default input to the source works");
	CHECK(Default(kAudioHardwarePropertyDefaultInputDevice) == source, "and it is the default input device then");
	CHECK(SetDefault(kAudioHardwarePropertyDefaultOutputDevice, source) != 0, "an input device is refused as the default output");
	CHECK(SetDefault(kAudioHardwarePropertyDefaultInputDevice, sink) != 0, "an output device is refused as the default input");
	CHECK(SetDefault(kAudioHardwarePropertyDefaultOutputDevice, 0x7fffff) != 0, "so is a device that does not exist");
	CHECK(Default(kAudioHardwarePropertyDefaultOutputDevice) == sink, "a refused change leaves the default as it was");

	OSStatus status;
	CHECK(Mute(sink, kAudioObjectPropertyScopeOutput, &status) == 0 && status == 0, "an output device is not muted at first (status %d)", (int)status);
	AudioObjectPropertyAddress muteAddr = Addr(kAudioDevicePropertyMute, kAudioObjectPropertyScopeOutput);
	CHECK(AudioObjectHasProperty(sink, &muteAddr), "an output device has a mute");
	AudioObjectIsPropertySettable(sink, &muteAddr, &settable);
	CHECK(settable, "which is settable");
	UInt32 on = 1, off = 0;
	CHECK(AudioObjectSetPropertyData(sink, &muteAddr, 0, NULL, sizeof(on), &on) == 0, "muting works");
	CHECK(Mute(sink, kAudioObjectPropertyScopeOutput, &status) == 1, "and the device is muted (status %d)", (int)status);
	CHECK(AudioObjectSetPropertyData(sink, &muteAddr, 0, NULL, sizeof(off), &off) == 0 && Mute(sink, kAudioObjectPropertyScopeOutput, &status) == 0, "unmuting works");
	Mute(sink, kAudioObjectPropertyScopeInput, &status);
	CHECK(status != 0, "an output device has no input mute");
	muteAddr = Addr(kAudioDevicePropertyMute, kAudioObjectPropertyScopeInput);
	CHECK(AudioObjectSetPropertyData(source, &muteAddr, 0, NULL, sizeof(on), &on) == 0 && Mute(source, kAudioObjectPropertyScopeInput, &status) == 1, "an input device mutes too");
	AudioObjectSetPropertyData(source, &muteAddr, 0, NULL, sizeof(off), &off);

	AudioObjectRemovePropertyListener(kAudioObjectSystemObject, &outAddr, Listener, NULL);
	SetDefault(kAudioHardwarePropertyDefaultOutputDevice, oldOut);
	SetDefault(kAudioHardwarePropertyDefaultInputDevice, oldIn);
	printf(failures ? "FAILED: %d\n" : "ALL PASSED\n", failures);
	return failures != 0;
}
