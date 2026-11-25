// Copyright (C) 2026 Lubos Dolezel
//
// Helpers of the HAL property tests. The devices come from fixture-hal.sh.
#ifndef HAL_TEST_H
#define HAL_TEST_H
#include <CoreAudio/CoreAudio.h>
#include <CoreFoundation/CoreFoundation.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;
#define CHECK(cond, ...) do { \
	if (cond) printf("PASS: %s\n", #cond); \
	else { failures++; printf("FAIL: %s:%d: %s: ", __FILE__, __LINE__, #cond); printf(__VA_ARGS__); printf("\n"); } \
} while (0)

#define kInput kAudioObjectPropertyScopeInput
#define kOutput kAudioObjectPropertyScopeOutput
#define kGlobal kAudioObjectPropertyScopeGlobal

static AudioObjectPropertyAddress Addr(AudioObjectPropertySelector s, AudioObjectPropertyScope scope, AudioObjectPropertyElement element)
{
	AudioObjectPropertyAddress a = { s, scope, element };
	return a;
}

static OSStatus GetU32(AudioObjectID o, AudioObjectPropertySelector s, AudioObjectPropertyScope scope, UInt32* value)
{
	AudioObjectPropertyAddress a = Addr(s, scope, 0);
	UInt32 size = sizeof(*value);
	return AudioObjectGetPropertyData(o, &a, 0, NULL, &size, value);
}

static OSStatus GetF32(AudioObjectID o, AudioObjectPropertySelector s, AudioObjectPropertyScope scope, AudioObjectPropertyElement e, Float32* value)
{
	AudioObjectPropertyAddress a = Addr(s, scope, e);
	UInt32 size = sizeof(*value);
	return AudioObjectGetPropertyData(o, &a, 0, NULL, &size, value);
}

static OSStatus SetF32(AudioObjectID o, AudioObjectPropertySelector s, AudioObjectPropertyScope scope, AudioObjectPropertyElement e, Float32 value)
{
	AudioObjectPropertyAddress a = Addr(s, scope, e);
	return AudioObjectSetPropertyData(o, &a, 0, NULL, sizeof(value), &value);
}

static OSStatus GetF64(AudioObjectID o, AudioObjectPropertySelector s, AudioObjectPropertyScope scope, Float64* value)
{
	AudioObjectPropertyAddress a = Addr(s, scope, 0);
	UInt32 size = sizeof(*value);
	return AudioObjectGetPropertyData(o, &a, 0, NULL, &size, value);
}

// The text of a CFString property, "" if it cannot be read.
static const char* GetText(AudioObjectID o, AudioObjectPropertySelector s, AudioObjectPropertyScope scope, AudioObjectPropertyElement e)
{
	static char buffers[8][256];
	static int next;
	char* buffer = buffers[next++ % 8];
	buffer[0] = 0;
	AudioObjectPropertyAddress a = Addr(s, scope, e);
	CFStringRef value = NULL;
	UInt32 size = sizeof(value);
	if (AudioObjectGetPropertyData(o, &a, 0, NULL, &size, &value) == 0 && value)
	{
		CFStringGetCString(value, buffer, 256, kCFStringEncodingUTF8);
		CFRelease(value);
	}
	return buffer;
}

static AudioDeviceID DeviceWithUID(const char* uid)
{
	CFStringRef wanted = CFStringCreateWithCString(NULL, uid, kCFStringEncodingUTF8);
	AudioObjectPropertyAddress a = Addr(kAudioHardwarePropertyTranslateUIDToDevice, kGlobal, 0);
	AudioDeviceID device = kAudioObjectUnknown;
	UInt32 size = sizeof(device);
	AudioObjectGetPropertyData(kAudioObjectSystemObject, &a, sizeof(wanted), &wanted, &size, &device);
	CFRelease(wanted);
	return device;
}

static AudioObjectID StreamOf(AudioDeviceID device, AudioObjectPropertyScope scope)
{
	AudioObjectPropertyAddress a = Addr(kAudioDevicePropertyStreams, scope, 0);
	AudioObjectID streams[4];
	UInt32 size = sizeof(streams);
	if (AudioObjectGetPropertyData(device, &a, 0, NULL, &size, streams) != 0 || size < sizeof(AudioObjectID))
		return kAudioObjectUnknown;
	return streams[0];
}

static UInt32 ChannelsOf(AudioDeviceID device, AudioObjectPropertyScope scope)
{
	AudioObjectPropertyAddress a = Addr(kAudioDevicePropertyStreamConfiguration, scope, 0);
	UInt32 size = 0;
	if (AudioObjectGetPropertyDataSize(device, &a, 0, NULL, &size) != 0)
		return 0;
	AudioBufferList* list = malloc(size);
	UInt32 channels = 0;
	if (AudioObjectGetPropertyData(device, &a, 0, NULL, &size, list) == 0)
		for (UInt32 i = 0; i < list->mNumberBuffers; i++)
			channels += list->mBuffers[i].mNumberChannels;
	free(list);
	return channels;
}

static volatile int notified;
static AudioObjectPropertyAddress lastNotified;
static OSStatus OnChange(AudioObjectID o, UInt32 count, const AudioObjectPropertyAddress* addresses, void* ctx)
{
	lastNotified = addresses[0];
	notified++;
	return 0;
}

#endif
