#include <stdio.h>
#include <string.h>
#include <CoreAudio/CoreAudio.h>

int main(void)
{
	AudioObjectID dev = kAudioObjectSystemObject + 1; // default output, fixed role from earlier work
	AudioObjectPropertyAddress addr = {
		kAudioDevicePropertyDeviceUID, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster
	};

	CFStringRef uid = NULL;
	UInt32 size = sizeof(uid);
	OSStatus status = AudioObjectGetPropertyData(dev, &addr, 0, NULL, &size, &uid);

	if (status != noErr || !uid)
	{
		fprintf(stderr, "FAIL: could not read device UID (status=%d)\n", (int)status);
		return 1;
	}

	char buf[256];
	CFStringGetCString(uid, buf, sizeof(buf), kCFStringEncodingUTF8);
	CFRelease(uid);

	// Must no longer be the old hardcoded "unknown" placeholder.
	if (strcmp(buf, "unknown") == 0)
	{
		fprintf(stderr, "FAIL: device UID is still the placeholder \"unknown\"\n");
		return 1;
	}

	printf("test_device_uid: OK (uid=%s)\n", buf);
	return 0;
}
