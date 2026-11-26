// Copyright (C) 2026 Lubos Dolezel
//
// The UID of the default output device is a real UID (not the "unknown" placeholder) and
// kAudioHardwarePropertyTranslateUIDToDevice leads back from the UID to the same device.
// Needs a PulseAudio sink: run it inside Darling.
#include <stdio.h>
#include <string.h>
#include <CoreAudio/CoreAudio.h>
#include <CoreFoundation/CoreFoundation.h>

int main(void)
{
	AudioObjectPropertyAddress defaultAddress = {
		kAudioHardwarePropertyDefaultOutputDevice, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster
	};
	AudioObjectID dev = kAudioObjectUnknown;
	UInt32 devSize = sizeof(dev);
	OSStatus status = AudioObjectGetPropertyData(kAudioObjectSystemObject, &defaultAddress, 0, NULL, &devSize, &dev);
	if (status != noErr || dev == kAudioObjectUnknown)
	{
		fprintf(stderr, "FAIL: no default output device (status=%d)\n", (int)status);
		return 1;
	}

	AudioObjectPropertyAddress addr = {
		kAudioDevicePropertyDeviceUID, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster
	};

	CFStringRef uid = NULL;
	UInt32 size = sizeof(uid);
	status = AudioObjectGetPropertyData(dev, &addr, 0, NULL, &size, &uid);

	if (status != noErr || !uid)
	{
		fprintf(stderr, "FAIL: could not read device UID (status=%d)\n", (int)status);
		return 1;
	}

	char buf[256];
	CFStringGetCString(uid, buf, sizeof(buf), kCFStringEncodingUTF8);

	// Must no longer be the old hardcoded "unknown" placeholder.
	if (strcmp(buf, "unknown") == 0 || buf[0] == 0)
	{
		fprintf(stderr, "FAIL: device UID is still the placeholder \"unknown\"\n");
		return 1;
	}

	AudioObjectPropertyAddress translate = {
		kAudioHardwarePropertyTranslateUIDToDevice, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster
	};
	AudioObjectID back = kAudioObjectUnknown;
	UInt32 backSize = sizeof(back);
	status = AudioObjectGetPropertyData(kAudioObjectSystemObject, &translate, sizeof(uid), &uid, &backSize, &back);
	CFRelease(uid);
	if (status != noErr || back != dev)
	{
		fprintf(stderr, "FAIL: the UID %s leads to device %u, not %u (status=%d)\n", buf, (unsigned)back, (unsigned)dev, (int)status);
		return 1;
	}

	printf("test_device_uid: OK (uid=%s)\n", buf);
	return 0;
}
