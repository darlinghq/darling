// tests/coreaudio/test_device_list_notifications.c
// Copyright (C) 2026 Lubos Dolezel
//
// The hardware property listener for kAudioHardwarePropertyDevices is called when a device appears
// or disappears, not when a device is used: starting and stopping playback on the default output
// leaves the list as it was and must not announce a change. Needs a running PulseAudio server;
// run it inside Darling. If another program adds or removes a device while the test runs, the
// result says so and the test passes without judging the notifications.
#include <AudioUnit/AudioUnit.h>
#include <CoreAudio/CoreAudio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CHECK(expr) do { OSStatus s_ = (expr); if (s_ != noErr) { printf("FAIL %s:%d: %s = %d\n", __FILE__, __LINE__, #expr, (int) s_); exit(1); } } while (0)

static volatile int g_notifications;

static OSStatus devicesChanged(AudioObjectID object, UInt32 count, const AudioObjectPropertyAddress* addresses, void* clientData)
{
	for (UInt32 i = 0; i < count; i++)
		if (addresses[i].mSelector == kAudioHardwarePropertyDevices)
			g_notifications++;
	return noErr;
}

static OSStatus renderSilence(void* refCon, AudioUnitRenderActionFlags* flags, const AudioTimeStamp* time,
	UInt32 bus, UInt32 frames, AudioBufferList* data)
{
	for (UInt32 i = 0; i < data->mNumberBuffers; i++)
		memset(data->mBuffers[i].mData, 0, data->mBuffers[i].mDataByteSize);
	return noErr;
}

static UInt32 deviceList(AudioObjectID* devices, UInt32 capacity)
{
	AudioObjectPropertyAddress address = { kAudioHardwarePropertyDevices, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
	UInt32 size = capacity * sizeof(AudioObjectID);
	CHECK(AudioObjectGetPropertyData(kAudioObjectSystemObject, &address, 0, NULL, &size, devices));
	return size / sizeof(AudioObjectID);
}

int main(void)
{
	AudioObjectPropertyAddress address = { kAudioHardwarePropertyDevices, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
	AudioObjectID before[64], after[64];

	// Asking for the list starts the tracking of changes.
	UInt32 beforeCount = deviceList(before, 64);
	CHECK(AudioObjectAddPropertyListener(kAudioObjectSystemObject, &address, devicesChanged, NULL));
	sleep(1);
	g_notifications = 0;

	AudioComponentDescription desc = { kAudioUnitType_Output, kAudioUnitSubType_DefaultOutput, kAudioUnitManufacturer_Apple, 0, 0 };
	AudioComponent component = AudioComponentFindNext(NULL, &desc);
	AudioUnit unit = NULL;
	CHECK(AudioComponentInstanceNew(component, &unit));
	AudioStreamBasicDescription asbd = { 48000, kAudioFormatLinearPCM, kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked, 8, 1, 8, 2, 32, 0 };
	AURenderCallbackStruct callback = { renderSilence, NULL };
	CHECK(AudioUnitSetProperty(unit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &asbd, sizeof(asbd)));
	CHECK(AudioUnitSetProperty(unit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &callback, sizeof(callback)));
	CHECK(AudioUnitInitialize(unit));
	for (int round = 0; round < 2; round++) {
		CHECK(AudioOutputUnitStart(unit));
		sleep(2);
		CHECK(AudioOutputUnitStop(unit));
		sleep(1);
	}
	CHECK(AudioUnitUninitialize(unit));
	CHECK(AudioComponentInstanceDispose(unit));
	sleep(1);

	UInt32 afterCount = deviceList(after, 64);
	CHECK(AudioObjectRemovePropertyListener(kAudioObjectSystemObject, &address, devicesChanged, NULL));

	if (afterCount != beforeCount || memcmp(before, after, beforeCount * sizeof(AudioObjectID)) != 0) {
		printf("SKIP: the device list changed during the test (%u -> %u devices)\n", (unsigned) beforeCount, (unsigned) afterCount);
		return 0;
	}
	if (g_notifications != 0) {
		printf("FAIL: %d device list notifications while the list was unchanged\n", g_notifications);
		return 1;
	}
	printf("PASS\n");
	return 0;
}
