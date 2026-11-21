// Copyright (C) 2026 Lubos Dolezel
//
// The legacy kAudioDevicePropertyDeviceName and ...DeviceManufacturer
// properties return a NUL-terminated C string and report its size including
// the terminator. The values must come out whatever the internal storage of
// the underlying CFString is (CFStringGetCStringPtr() may return NULL).
#include <CoreAudio/CoreAudio.h>
#include <stdio.h>
#include <string.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void check_string_property(AudioDeviceID dev, AudioDevicePropertyID prop, const char *what)
{
	char buf[256];
	memset(buf, 'x', sizeof(buf));
	UInt32 size = sizeof(buf);
	OSStatus st = AudioDeviceGetProperty(dev, 0, false, prop, &size, buf);
	if (st != noErr) {
		printf("FAIL: %s status %d\n", what, (int) st);
		g_failures++;
		return;
	}
	CHECK(strlen(buf) > 0);
	CHECK(size == strlen(buf) + 1);

	// a buffer that is too small for the string is not overrun
	char small[2] = { 'x', 'x' };
	char guard = 'g';
	UInt32 ssize = sizeof(small);
	(void) AudioDeviceGetProperty(dev, 0, false, prop, &ssize, small);
	CHECK(guard == 'g');
}

int main(void)
{
	AudioDeviceID devs[32];
	UInt32 dsize = sizeof(devs);
	OSStatus dst = AudioHardwareGetProperty(kAudioHardwarePropertyDevices, &dsize, devs);
	printf("devices: status %d, count %u\n", (int) dst, (unsigned) (dsize / sizeof(AudioDeviceID)));
	if (dst == noErr) {
		for (UInt32 i = 0; i < dsize / sizeof(AudioDeviceID); i++) {
			printf("device %u:\n", (unsigned) devs[i]);
			check_string_property(devs[i], kAudioDevicePropertyDeviceName, "DeviceName");
		}
	}
	AudioDeviceID dev = kAudioObjectUnknown;
	UInt32 size = sizeof(dev);
	OSStatus st = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice, &size, &dev);
	if (st != noErr || dev == kAudioObjectUnknown) {
		printf("SKIP: no default output device (status %d)\n", (int) st);
		return 0;
	}
	check_string_property(dev, kAudioDevicePropertyDeviceName, "DeviceName");
	check_string_property(dev, kAudioDevicePropertyDeviceManufacturer, "DeviceManufacturer");
	printf(g_failures ? "FAILED\n" : "ALL PASSED\n");
	return g_failures ? 1 : 0;
}
