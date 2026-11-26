// Copyright (C) 2026 Lubos Dolezel
//
// Volume and mute of output and input devices: the main volume and each channel, the volume in decibels,
// the main volume and balance of the AudioHardwareService names, the mute of the device; which of these exist
// on which scope and element, whether they are settable, and that listeners are told. Reading back what was set
// goes through the server. Needs the devices of fixture-hal.sh and runs inside Darling.
#include "hal_test.h"

#define kVirtualMainVolume 'vmvc'
#define kVirtualMainBalance 'vmbc'

static int Near(Float32 a, Float32 b, Float32 eps)
{
	return fabsf(a - b) <= eps;
}

static int Has(AudioDeviceID d, AudioObjectPropertySelector s, AudioObjectPropertyScope scope, AudioObjectPropertyElement e)
{
	AudioObjectPropertyAddress a = Addr(s, scope, e);
	return AudioObjectHasProperty(d, &a);
}

static int Settable(AudioDeviceID d, AudioObjectPropertySelector s, AudioObjectPropertyScope scope, AudioObjectPropertyElement e)
{
	AudioObjectPropertyAddress a = Addr(s, scope, e);
	Boolean settable = 0;
	return AudioObjectIsPropertySettable(d, &a, &settable) == 0 && settable;
}

static Float32 Volume(AudioDeviceID d, AudioObjectPropertyScope scope, AudioObjectPropertyElement e)
{
	Float32 v = -1;
	GetF32(d, kAudioDevicePropertyVolumeScalar, scope, e, &v);
	return v;
}

static void CheckVolume(const char* uid, AudioObjectPropertyScope scope, UInt32 channels)
{
	AudioDeviceID d = DeviceWithUID(uid);
	CHECK(d != kAudioObjectUnknown, "%s is not a device", uid);
	if (d == kAudioObjectUnknown)
		return;
	const AudioObjectPropertyScope other = scope == kInput ? kOutput : kInput;
	printf("-- %s, %u channels\n", uid, (unsigned)channels);

	// Which properties exist where.
	for (UInt32 e = 0; e <= channels; e++)
	{
		CHECK(Has(d, kAudioDevicePropertyVolumeScalar, scope, e), "no volume on element %u", (unsigned)e);
		CHECK(Has(d, kAudioDevicePropertyVolumeDecibels, scope, e), "no decibels on element %u", (unsigned)e);
		CHECK(Has(d, kAudioDevicePropertyVolumeRangeDecibels, scope, e), "no decibel range on element %u", (unsigned)e);
		CHECK(Settable(d, kAudioDevicePropertyVolumeScalar, scope, e), "volume of element %u not settable", (unsigned)e);
		CHECK(Settable(d, kAudioDevicePropertyVolumeDecibels, scope, e), "decibels of element %u not settable", (unsigned)e);
	}
	CHECK(!Has(d, kAudioDevicePropertyVolumeScalar, scope, channels + 1), "a volume beyond the last channel");
	CHECK(!Has(d, kAudioDevicePropertyVolumeScalar, other, 0), "a volume on the other scope");
	// The global scope is taken for the side the device has.
	CHECK(Has(d, kAudioDevicePropertyVolumeScalar, kGlobal, 0), "no volume on the global scope");
	CHECK(!Has(d, kAudioDevicePropertyMute, other, 0), "a mute on the other scope");
	CHECK(Has(d, kAudioDevicePropertyMute, scope, 0) && Settable(d, kAudioDevicePropertyMute, scope, 0), "no settable mute");
	CHECK(!Has(d, kAudioDevicePropertyMute, scope, 1), "a mute of a single channel");
	CHECK(Has(d, kVirtualMainVolume, scope, 0) && Settable(d, kVirtualMainVolume, scope, 0), "no settable main volume");
	CHECK(!Has(d, kVirtualMainVolume, scope, 1), "a main volume of a channel");
	CHECK(Has(d, kVirtualMainBalance, scope, 0) == (channels >= 2), "balance on %u channels", (unsigned)channels);
	CHECK(GetF32(d, kAudioDevicePropertyVolumeScalar, other, 0, &(Float32){0}) == kAudioHardwareUnknownPropertyError, "wrong error on the other scope");
	CHECK(SetF32(d, kAudioDevicePropertyVolumeScalar, other, 0, 0.5f) == kAudioHardwareUnknownPropertyError, "set on the other scope");
	CHECK(SetF32(d, kAudioDevicePropertyVolumeScalar, scope, channels + 1, 0.5f) == kAudioHardwareUnknownPropertyError, "set beyond the last channel");
	AudioObjectPropertyAddress a = Addr(kAudioDevicePropertyVolumeScalar, scope, 0);
	UInt32 wrongSize = 1;
	CHECK(AudioObjectSetPropertyData(d, &a, 0, NULL, sizeof(wrongSize), &wrongSize) != 0 || sizeof(wrongSize) == sizeof(Float32), "wrong size accepted");
	UInt16 tiny = 1;
	CHECK(AudioObjectSetPropertyData(d, &a, 0, NULL, sizeof(tiny), &tiny) == kAudioHardwareBadPropertySizeError, "a 2 byte value accepted");

	// The main volume moves every channel.
	notified = 0;
	AudioObjectPropertyAddress main = Addr(kAudioDevicePropertyVolumeScalar, scope, 0);
	CHECK(AudioObjectAddPropertyListener(d, &main, OnChange, NULL) == 0, "listener");
	CHECK(SetF32(d, kAudioDevicePropertyVolumeScalar, scope, 0, 0.5f) == 0, "set the main volume");
	CHECK(Near(Volume(d, scope, 0), 0.5f, 0.002f), "main volume %g", Volume(d, scope, 0));
	for (UInt32 e = 1; e <= channels; e++)
		CHECK(Near(Volume(d, scope, e), 0.5f, 0.002f), "channel %u: %g", (unsigned)e, Volume(d, scope, e));
	CHECK(notified >= 1 && lastNotified.mSelector == kAudioDevicePropertyVolumeScalar && lastNotified.mScope == scope,
		"listener called %d times", notified);
	CHECK(AudioObjectRemovePropertyListener(d, &main, OnChange, NULL) == 0, "remove the listener");

	// One channel; the main volume is the loudest channel.
	AudioObjectPropertyAddress first = Addr(kAudioDevicePropertyVolumeScalar, scope, 1);
	notified = 0;
	CHECK(AudioObjectAddPropertyListener(d, &first, OnChange, NULL) == 0, "listener on channel 1");
	CHECK(SetF32(d, kAudioDevicePropertyVolumeScalar, scope, 1, 0.25f) == 0, "set channel 1");
	CHECK(Near(Volume(d, scope, 1), 0.25f, 0.002f), "channel 1: %g", Volume(d, scope, 1));
	CHECK(notified >= 1 && lastNotified.mElement == 1, "listener on channel 1 called %d times", notified);
	if (channels >= 2)
	{
		CHECK(Near(Volume(d, scope, 2), 0.5f, 0.002f), "channel 2 changed: %g", Volume(d, scope, 2));
		CHECK(Near(Volume(d, scope, 0), 0.5f, 0.002f), "main volume is not the loudest: %g", Volume(d, scope, 0));
	}
	else
		CHECK(Near(Volume(d, scope, 0), 0.25f, 0.002f), "main volume %g", Volume(d, scope, 0));
	// A listener on one element is not told about another.
	notified = 0;
	if (channels >= 2)
	{
		CHECK(SetF32(d, kAudioDevicePropertyVolumeScalar, scope, 2, 0.4f) == 0, "set channel 2");
		CHECK(notified == 0, "the listener on channel 1 heard of channel 2");
	}
	CHECK(AudioObjectRemovePropertyListener(d, &first, OnChange, NULL) == 0, "remove the listener");

	// The main volume of the AudioHardwareService names scales the channels together and keeps their balance.
	CHECK(SetF32(d, kAudioDevicePropertyVolumeScalar, scope, 1, 0.2f) == 0, "set channel 1");
	for (UInt32 e = 2; e <= channels; e++)
		CHECK(SetF32(d, kAudioDevicePropertyVolumeScalar, scope, e, 0.4f) == 0, "set channel %u", (unsigned)e);
	Float32 main_volume = 0;
	CHECK(GetF32(d, kVirtualMainVolume, scope, 0, &main_volume) == 0 && Near(main_volume, channels >= 2 ? 0.4f : 0.2f, 0.002f),
		"main volume %g", main_volume);
	notified = 0;
	AudioObjectPropertyAddress mainAddress = Addr(kVirtualMainVolume, scope, 0);
	CHECK(AudioObjectAddPropertyListener(d, &mainAddress, OnChange, NULL) == 0, "listener on the main volume");
	CHECK(SetF32(d, kVirtualMainVolume, scope, 0, channels >= 2 ? 0.8f : 0.4f) == 0, "set the main volume");
	CHECK(Near(Volume(d, scope, 1), 0.4f, 0.003f), "channel 1 after doubling: %g", Volume(d, scope, 1));
	for (UInt32 e = 2; e <= channels; e++)
		CHECK(Near(Volume(d, scope, e), 0.8f, 0.003f), "channel %u after doubling: %g", (unsigned)e, Volume(d, scope, e));
	CHECK(notified >= 1, "main volume listener called %d times", notified);
	CHECK(AudioObjectRemovePropertyListener(d, &mainAddress, OnChange, NULL) == 0, "remove the listener");

	// Decibels follow the volume.
	CHECK(SetF32(d, kAudioDevicePropertyVolumeDecibels, scope, 0, -20.0f) == 0, "set -20 dB");
	Float32 db = 0;
	CHECK(GetF32(d, kAudioDevicePropertyVolumeDecibels, scope, 0, &db) == 0 && Near(db, -20.0f, 0.3f), "decibels %g", db);
	CHECK(Near(Volume(d, scope, 0), powf(10.0f, -20.0f / 60.0f), 0.01f), "volume at -20 dB: %g", Volume(d, scope, 0));
	AudioObjectPropertyAddress rangeAddress = Addr(kAudioDevicePropertyVolumeRangeDecibels, scope, 0);
	AudioValueRange range = {0, 0};
	UInt32 rangeSize = sizeof(range);
	CHECK(AudioObjectGetPropertyData(d, &rangeAddress, 0, NULL, &rangeSize, &range) == 0 && range.mMinimum < -40 && range.mMaximum == 0,
		"range %g..%g", range.mMinimum, range.mMaximum);
	CHECK(SetF32(d, kAudioDevicePropertyVolumeScalar, scope, 0, 1.5f) == 0 && Near(Volume(d, scope, 0), 1.0f, 0.002f), "volume is not limited to 1: %g", Volume(d, scope, 0));
	CHECK(SetF32(d, kAudioDevicePropertyVolumeScalar, scope, 0, -1.0f) == 0 && Near(Volume(d, scope, 0), 0.0f, 0.002f), "volume is not limited to 0: %g", Volume(d, scope, 0));

	// Balance of a stereo device: 0 is left, 1 is right.
	if (channels >= 2)
	{
		CHECK(SetF32(d, kAudioDevicePropertyVolumeScalar, scope, 0, 0.8f) == 0, "set 0.8");
		Float32 balance = -1;
		CHECK(GetF32(d, kVirtualMainBalance, scope, 0, &balance) == 0 && Near(balance, 0.5f, 0.01f), "balance %g", balance);
		CHECK(SetF32(d, kVirtualMainBalance, scope, 0, 0.0f) == 0, "set the balance left");
		CHECK(Near(Volume(d, scope, 1), 0.8f, 0.003f) && Near(Volume(d, scope, 2), 0.0f, 0.003f), "channels %g %g", Volume(d, scope, 1), Volume(d, scope, 2));
		CHECK(GetF32(d, kVirtualMainBalance, scope, 0, &balance) == 0 && Near(balance, 0.0f, 0.01f), "balance %g", balance);
		CHECK(SetF32(d, kVirtualMainBalance, scope, 0, 0.5f) == 0, "set the balance even");
	}
	SetF32(d, kAudioDevicePropertyVolumeScalar, scope, 0, 1.0f);

	// Mute.
	UInt32 mute = 7;
	AudioObjectPropertyAddress muteAddress = Addr(kAudioDevicePropertyMute, scope, 0);
	CHECK(GetU32(d, kAudioDevicePropertyMute, scope, &mute) == 0 && mute == 0, "mute %u", (unsigned)mute);
	notified = 0;
	CHECK(AudioObjectAddPropertyListener(d, &muteAddress, OnChange, NULL) == 0, "listener on mute");
	mute = 1;
	CHECK(AudioObjectSetPropertyData(d, &muteAddress, 0, NULL, sizeof(mute), &mute) == 0, "mute");
	mute = 7;
	CHECK(GetU32(d, kAudioDevicePropertyMute, scope, &mute) == 0 && mute == 1, "mute %u", (unsigned)mute);
	CHECK(notified == 1 && lastNotified.mSelector == kAudioDevicePropertyMute && lastNotified.mScope == scope, "mute listener called %d times", notified);
	mute = 0;
	CHECK(AudioObjectSetPropertyData(d, &muteAddress, 0, NULL, sizeof(mute), &mute) == 0, "unmute");
	CHECK(GetU32(d, kAudioDevicePropertyMute, scope, &mute) == 0 && mute == 0, "mute %u", (unsigned)mute);
	CHECK(notified == 2, "mute listener called %d times", notified);
	CHECK(AudioObjectRemovePropertyListener(d, &muteAddress, OnChange, NULL) == 0, "remove the listener");
}

int main(void)
{
	CheckVolume("darling-hal-48k", kOutput, 2);
	CheckVolume("darling-hal-51", kOutput, 6);
	CheckVolume("darling-hal-mic", kInput, 1);
	CheckVolume("darling-hal-line", kInput, 2);

	printf(failures ? "test_hal_volume: FAILED (%d)\n" : "test_hal_volume: OK\n", failures);
	return failures != 0;
}
