#include <stdio.h>
#include <CoreAudio/CoreAudio.h>

int main(void)
{
	AudioObjectID defaultOut = kAudioObjectSystemObject + 1;
	AudioObjectID defaultIn  = kAudioObjectSystemObject + 2;
	AudioObjectID eventOut   = kAudioObjectSystemObject + 3;

	AudioObjectPropertyAddress addr = {
		kAudioDevicePropertyDeviceUID, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster
	};

	// All three must independently answer kAudioHardwareNoError, not
	// kAudioHardwareBadObjectError, and defaultOut/eventOut must report
	// *different* UIDs (they are different PulseAudio roles).
	CFStringRef uidOut = NULL, uidEvent = NULL;
	UInt32 size = sizeof(CFStringRef);

	OSStatus s1 = AudioObjectGetPropertyData(defaultOut, &addr, 0, NULL, &size, &uidOut);
	size = sizeof(CFStringRef);
	OSStatus s2 = AudioObjectGetPropertyData(eventOut, &addr, 0, NULL, &size, &uidEvent);
	size = sizeof(CFStringRef);
	OSStatus s3 = AudioObjectGetPropertyData(defaultIn, &addr, 0, NULL, &size, &uidOut);

	if (s1 != noErr || s2 != noErr || s3 != noErr)
	{
		fprintf(stderr, "FAIL: one of the three fixed objects is not addressable (s1=%d s2=%d s3=%d)\n", (int)s1, (int)s2, (int)s3);
		return 1;
	}

	if (uidOut && uidEvent && CFEqual(uidOut, uidEvent))
	{
		fprintf(stderr, "FAIL: defaultOut and eventOut report the same UID -- map collision\n");
		return 1;
	}

	if (uidOut)
		CFRelease(uidOut);
	if (uidEvent)
		CFRelease(uidEvent);

	printf("test_object_ids: OK\n");
	return 0;
}
