// Copyright (C) 2026 Lubos Dolezel
//
// CoreAudio/AudioHardware.h: AudioObjectAddPropertyListener and
// AudioObjectRemovePropertyListener accept the stream objects of a device, as they do
// its other objects, and report kAudioHardwareBadObjectError for an object that does not exist.
#include <CoreAudio/CoreAudio.h>
#include <stdio.h>

static int g_failures = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static OSStatus listener(AudioObjectID object, UInt32 count, const AudioObjectPropertyAddress addresses[], void *client)
{
	return noErr;
}

int main(void)
{
	AudioObjectPropertyAddress defaultOutput = {
		kAudioHardwarePropertyDefaultOutputDevice, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster
	};
	AudioObjectID device = kAudioObjectUnknown;
	UInt32 size = sizeof(device);
	OSStatus st = AudioObjectGetPropertyData(kAudioObjectSystemObject, &defaultOutput, 0, NULL, &size, &device);
	printf("default output device %u (%d)\n", (unsigned) device, (int) st);
	CHECK(st == noErr && device != kAudioObjectUnknown);

	AudioObjectPropertyAddress streamsAddress = {
		kAudioDevicePropertyStreams, kAudioObjectPropertyScopeOutput, kAudioObjectPropertyElementMaster
	};
	AudioStreamID streams[8];
	size = sizeof(streams);
	st = AudioObjectGetPropertyData(device, &streamsAddress, 0, NULL, &size, streams);
	printf("streams: %u (%d)\n", (unsigned) (size / sizeof(AudioStreamID)), (int) st);
	CHECK(st == noErr && size >= sizeof(AudioStreamID));

	AudioObjectPropertyAddress formats = {
		kAudioStreamPropertyAvailablePhysicalFormats, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster
	};
	int marker;
	st = AudioObjectAddPropertyListener(streams[0], &formats, listener, &marker);
	printf("add listener on the stream: %d\n", (int) st);
	CHECK(st == noErr);
	st = AudioObjectRemovePropertyListener(streams[0], &formats, listener, &marker);
	printf("remove listener from the stream: %d\n", (int) st);
	CHECK(st == noErr);

	// An object that does not exist.
	st = AudioObjectAddPropertyListener(0x7fff0001, &formats, listener, &marker);
	CHECK(st == kAudioHardwareBadObjectError);
	st = AudioObjectRemovePropertyListener(0x7fff0001, &formats, listener, &marker);
	CHECK(st == kAudioHardwareBadObjectError);
	// A stream of a device that does not exist.
	st = AudioObjectAddPropertyListener(0x4441fffe, &formats, listener, &marker);
	CHECK(st == kAudioHardwareBadObjectError);

	printf(g_failures ? "FAILED (%d)\n" : "PASS\n", g_failures);
	return g_failures != 0;
}
