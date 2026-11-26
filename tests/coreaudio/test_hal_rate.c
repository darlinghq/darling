// Copyright (C) 2026 Lubos Dolezel
//
// A device reports the rate its server runs it at, the rates it can be asked for and its channels, in the
// nominal rate property and in the formats of its stream; the nominal rate can be set to an offered rate and
// listeners are told. Needs the devices of fixture-hal.sh (run: ./fixture-hal.sh up) and runs inside Darling.
#include "hal_test.h"

static int HasRate(AudioDeviceID d, Float64 rate)
{
	AudioObjectPropertyAddress a = Addr(kAudioDevicePropertyAvailableNominalSampleRates, kGlobal, 0);
	UInt32 size = 0;
	AudioObjectGetPropertyDataSize(d, &a, 0, NULL, &size);
	AudioValueRange* ranges = malloc(size);
	AudioObjectGetPropertyData(d, &a, 0, NULL, &size, ranges);
	int found = 0;
	for (UInt32 i = 0; i < size / sizeof(AudioValueRange); i++)
		if (ranges[i].mMinimum <= rate && rate <= ranges[i].mMaximum)
			found = 1;
	free(ranges);
	return found;
}

static void CheckDevice(const char* uid, Float64 rate, UInt32 channels, AudioObjectPropertyScope scope)
{
	AudioDeviceID d = DeviceWithUID(uid);
	CHECK(d != kAudioObjectUnknown, "%s is not a device", uid);
	if (d == kAudioObjectUnknown)
		return;
	printf("-- %s: %g Hz, %u channels\n", uid, rate, (unsigned)channels);

	Float64 nominal = 0, actual = 0;
	CHECK(GetF64(d, kAudioDevicePropertyNominalSampleRate, kGlobal, &nominal) == 0 && nominal == rate, "nominal rate %g", nominal);
	CHECK(GetF64(d, kAudioDevicePropertyActualSampleRate, kGlobal, &actual) == 0 && actual == rate, "actual rate %g", actual);
	CHECK(HasRate(d, rate), "the rate %g is not offered", rate);
	CHECK(HasRate(d, 48000) && HasRate(d, 44100), "the usual rates are not offered");
	CHECK(!HasRate(d, 12345), "12345 Hz is offered");

	AudioObjectPropertyAddress a = Addr(kAudioDevicePropertyAvailableNominalSampleRates, kGlobal, 0);
	UInt32 size = 0;
	AudioObjectGetPropertyDataSize(d, &a, 0, NULL, &size);
	AudioValueRange* ranges = malloc(size);
	AudioObjectGetPropertyData(d, &a, 0, NULL, &size, ranges);
	int ascending = size >= sizeof(AudioValueRange);
	for (UInt32 i = 1; i < size / sizeof(AudioValueRange); i++)
		if (ranges[i].mMinimum <= ranges[i - 1].mMinimum || ranges[i].mMinimum > ranges[i].mMaximum)
			ascending = 0;
	free(ranges);
	CHECK(ascending, "the available rates are not ascending");

	Boolean settable = 0;
	AudioObjectPropertyAddress rateAddress = Addr(kAudioDevicePropertyNominalSampleRate, kGlobal, 0);
	CHECK(AudioObjectIsPropertySettable(d, &rateAddress, &settable) == 0 && settable, "the nominal rate is not settable");
	AudioObjectPropertyAddress availableAddress = Addr(kAudioDevicePropertyAvailableNominalSampleRates, kGlobal, 0);
	CHECK(AudioObjectIsPropertySettable(d, &availableAddress, &settable) == 0 && !settable, "the available rates are settable");

	CHECK(ChannelsOf(d, scope) == channels, "%u channels", (unsigned)ChannelsOf(d, scope));
	CHECK(ChannelsOf(d, scope == kInput ? kOutput : kInput) == 0, "channels on the other side");

	AudioObjectID stream = StreamOf(d, scope);
	CHECK(stream != kAudioObjectUnknown, "no stream");
	AudioStreamBasicDescription format;
	UInt32 formatSize = sizeof(format);
	AudioObjectPropertyAddress virtualFormat = Addr(kAudioStreamPropertyVirtualFormat, kGlobal, 0);
	CHECK(AudioObjectGetPropertyData(stream, &virtualFormat, 0, NULL, &formatSize, &format) == 0
		&& format.mSampleRate == rate && format.mChannelsPerFrame == channels, "stream format %g Hz %u channels",
		format.mSampleRate, (unsigned)format.mChannelsPerFrame);
	AudioObjectPropertyAddress physicalFormat = Addr(kAudioStreamPropertyPhysicalFormat, kGlobal, 0);
	formatSize = sizeof(format);
	CHECK(AudioObjectGetPropertyData(stream, &physicalFormat, 0, NULL, &formatSize, &format) == 0 && format.mSampleRate == rate,
		"physical format %g Hz", format.mSampleRate);
	formatSize = sizeof(format);
	AudioObjectPropertyAddress deviceFormat = Addr(kAudioDevicePropertyStreamFormat, scope, 0);
	CHECK(AudioObjectGetPropertyData(d, &deviceFormat, 0, NULL, &formatSize, &format) == 0 && format.mSampleRate == rate,
		"device format %g Hz", format.mSampleRate);
}

static void CheckSetting(const char* uid, AudioObjectPropertyScope scope)
{
	AudioDeviceID d = DeviceWithUID(uid);
	if (d == kAudioObjectUnknown)
		return;
	printf("-- setting the rate of %s\n", uid);

	Float64 original = 0;
	GetF64(d, kAudioDevicePropertyNominalSampleRate, kGlobal, &original);
	const Float64 other = original == 96000 ? 88200 : 96000;
	AudioObjectPropertyAddress rateAddress = Addr(kAudioDevicePropertyNominalSampleRate, kGlobal, 0);

	notified = 0;
	CHECK(AudioObjectAddPropertyListener(d, &rateAddress, OnChange, NULL) == 0, "listener");
	CHECK(AudioObjectSetPropertyData(d, &rateAddress, 0, NULL, sizeof(other), &other) == 0, "set to %g", other);
	Float64 now = 0;
	CHECK(GetF64(d, kAudioDevicePropertyNominalSampleRate, kGlobal, &now) == 0 && now == other, "now %g", now);
	CHECK(notified == 1 && lastNotified.mSelector == kAudioDevicePropertyNominalSampleRate, "listener called %d times", notified);

	AudioStreamBasicDescription format;
	UInt32 formatSize = sizeof(format);
	AudioObjectPropertyAddress virtualFormat = Addr(kAudioStreamPropertyVirtualFormat, kGlobal, 0);
	CHECK(AudioObjectGetPropertyData(StreamOf(d, scope), &virtualFormat, 0, NULL, &formatSize, &format) == 0 && format.mSampleRate == other,
		"the stream follows: %g Hz", format.mSampleRate);

	// A rate the device does not offer, and a value of the wrong size.
	Float64 odd = 12345;
	CHECK(AudioObjectSetPropertyData(d, &rateAddress, 0, NULL, sizeof(odd), &odd) == kAudioHardwareIllegalOperationError, "unsupported rate accepted");
	Float32 small = 1;
	CHECK(AudioObjectSetPropertyData(d, &rateAddress, 0, NULL, sizeof(small), &small) == kAudioHardwareBadPropertySizeError, "bad size accepted");
	CHECK(GetF64(d, kAudioDevicePropertyNominalSampleRate, kGlobal, &now) == 0 && now == other, "a refused change took effect");

	// Choosing the format of the stream chooses the rate of the device.
	AudioStreamBasicDescription chosen = format;
	chosen.mSampleRate = original;
	CHECK(AudioObjectSetPropertyData(StreamOf(d, scope), &virtualFormat, 0, NULL, sizeof(chosen), &chosen) == 0, "set the stream format");
	CHECK(GetF64(d, kAudioDevicePropertyNominalSampleRate, kGlobal, &now) == 0 && now == original, "the device follows the stream: %g", now);

	CHECK(AudioObjectRemovePropertyListener(d, &rateAddress, OnChange, NULL) == 0, "remove listener");
}

int main(void)
{
	CheckDevice("darling-hal-48k", 48000, 2, kOutput);
	CheckDevice("darling-hal-44k", 44100, 2, kOutput);
	CheckDevice("darling-hal-51", 32000, 6, kOutput);
	CheckDevice("darling-hal-mic", 16000, 1, kInput);
	CheckDevice("darling-hal-line", 44100, 2, kInput);
	CheckSetting("darling-hal-44k", kOutput);
	CheckSetting("darling-hal-mic", kInput);

	printf(failures ? "test_hal_rate: FAILED (%d)\n" : "test_hal_rate: OK\n", failures);
	return failures != 0;
}
