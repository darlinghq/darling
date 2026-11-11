// Copyright (C) 2026 Lubos Dolezel
//
// tests/coreaudio/test_stream_formats.c
// The available virtual and physical formats of the default output device's stream are
// non-empty arrays of linear PCM ranged descriptions that include the current format, and
// kAudioOutputUnitProperty_IsRunning of an output unit is a read-only 0 before the unit
// starts, 1 while it runs and 0 again after it stops.
// Needs a PulseAudio sink: run it inside Darling. The program also passes on macOS 27.
#include <AudioUnit/AudioUnit.h>
#include <CoreAudio/CoreAudio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CHECK(expr) do { OSStatus s_ = (expr); if (s_ != noErr) { printf("FAIL %s:%d: %s = %d\n", __FILE__, __LINE__, #expr, (int) s_); exit(1); } } while (0)
#define REQUIRE(cond) do { if (!(cond)) { printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); exit(1); } } while (0)

static void checkAvailableFormats(AudioObjectID stream, AudioObjectPropertySelector available, AudioObjectPropertySelector current)
{
	AudioObjectPropertyAddress address = { available, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
	Boolean settable = true;
	UInt32 size = 0;

	REQUIRE(AudioObjectHasProperty(stream, &address));
	CHECK(AudioObjectIsPropertySettable(stream, &address, &settable));
	REQUIRE(!settable);
	CHECK(AudioObjectGetPropertyDataSize(stream, &address, 0, NULL, &size));
	REQUIRE(size >= sizeof(AudioStreamRangedDescription) && size % sizeof(AudioStreamRangedDescription) == 0);

	AudioStreamRangedDescription* formats = malloc(size);
	CHECK(AudioObjectGetPropertyData(stream, &address, 0, NULL, &size, formats));
	const UInt32 count = size / sizeof(AudioStreamRangedDescription);

	AudioObjectPropertyAddress currentAddress = { current, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
	AudioStreamBasicDescription now;
	UInt32 nowSize = sizeof(now);
	CHECK(AudioObjectGetPropertyData(stream, &currentAddress, 0, NULL, &nowSize, &now));

	int found = 0;
	for (UInt32 i = 0; i < count; i++)
	{
		REQUIRE(formats[i].mFormat.mFormatID == kAudioFormatLinearPCM);
		REQUIRE(formats[i].mSampleRateRange.mMinimum <= formats[i].mSampleRateRange.mMaximum);
		if (formats[i].mFormat.mSampleRate == now.mSampleRate && formats[i].mFormat.mChannelsPerFrame == now.mChannelsPerFrame
				&& formats[i].mFormat.mBitsPerChannel == now.mBitsPerChannel && formats[i].mFormat.mFormatFlags == now.mFormatFlags
				&& formats[i].mSampleRateRange.mMinimum <= now.mSampleRate && now.mSampleRate <= formats[i].mSampleRateRange.mMaximum)
			found = 1;
	}
	REQUIRE(found);
	free(formats);
}

static OSStatus renderSilence(void* refCon, AudioUnitRenderActionFlags* flags, const AudioTimeStamp* time,
	UInt32 bus, UInt32 frames, AudioBufferList* data)
{
	for (UInt32 i = 0; i < data->mNumberBuffers; i++)
		memset(data->mBuffers[i].mData, 0, data->mBuffers[i].mDataByteSize);
	return noErr;
}

static UInt32 isRunning(AudioUnit unit)
{
	UInt32 value = 99, size = sizeof(value);
	CHECK(AudioUnitGetProperty(unit, kAudioOutputUnitProperty_IsRunning, kAudioUnitScope_Global, 0, &value, &size));
	return value;
}

int main(void)
{
	AudioObjectPropertyAddress deviceAddress = { kAudioHardwarePropertyDefaultOutputDevice, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
	AudioObjectID device = 0;
	UInt32 size = sizeof(device);
	CHECK(AudioObjectGetPropertyData(kAudioObjectSystemObject, &deviceAddress, 0, NULL, &size, &device));

	AudioObjectPropertyAddress streamsAddress = { kAudioDevicePropertyStreams, kAudioObjectPropertyScopeOutput, kAudioObjectPropertyElementMaster };
	AudioObjectID streams[8];
	size = sizeof(streams);
	CHECK(AudioObjectGetPropertyData(device, &streamsAddress, 0, NULL, &size, streams));
	REQUIRE(size >= sizeof(AudioObjectID));

	checkAvailableFormats(streams[0], kAudioStreamPropertyAvailableVirtualFormats, kAudioStreamPropertyVirtualFormat);
	checkAvailableFormats(streams[0], kAudioStreamPropertyAvailablePhysicalFormats, kAudioStreamPropertyPhysicalFormat);

	AudioComponentDescription desc = { kAudioUnitType_Output, kAudioUnitSubType_DefaultOutput, kAudioUnitManufacturer_Apple, 0, 0 };
	AudioComponent component = AudioComponentFindNext(NULL, &desc);
	REQUIRE(component != NULL);
	AudioUnit unit = NULL;
	CHECK(AudioComponentInstanceNew(component, &unit));

	AURenderCallbackStruct callback = { renderSilence, NULL };
	CHECK(AudioUnitSetProperty(unit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &callback, sizeof(callback)));

	UInt32 infoSize = 0;
	Boolean writable = true;
	CHECK(AudioUnitGetPropertyInfo(unit, kAudioOutputUnitProperty_IsRunning, kAudioUnitScope_Global, 0, &infoSize, &writable));
	REQUIRE(infoSize == sizeof(UInt32));
	REQUIRE(!writable);

	CHECK(AudioUnitInitialize(unit));
	REQUIRE(isRunning(unit) == 0);
	CHECK(AudioOutputUnitStart(unit));
	REQUIRE(isRunning(unit) == 1);
	usleep(100000);
	CHECK(AudioOutputUnitStop(unit));
	REQUIRE(isRunning(unit) == 0);

	AudioUnitUninitialize(unit);
	AudioComponentInstanceDispose(unit);
	printf("test_stream_formats: OK\n");
	return 0;
}
