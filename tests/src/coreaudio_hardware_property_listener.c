// Copyright (C) 2026 Lubos Dolezel
//
// The deprecated AudioHardwareAddPropertyListener / AudioHardwareRemovePropertyListener
// are a view on AudioObjectAddPropertyListener on the system object: they succeed
// for hardware properties, can be removed, and (QUIRK, observed on macOS 26.7)
// refuse a NULL proc, a second registration of the same proc for the same
// property, and a removal of a NULL proc with kAudioHardwareIllegalOperationError,
// while removing a proc that is not registered succeeds.
// A listener registered through the AudioObject API on the same property
// coexists with them. Delivery for a default-device change needs an audio device
// change and is not exercised here.
#include <CoreAudio/CoreAudio.h>
#include <stdio.h>

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

static int g_failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static OSStatus hardwareListener(AudioHardwarePropertyID propertyID, void* clientData)
{
	(void) propertyID;
	(void) clientData;
	return noErr;
}

static OSStatus otherHardwareListener(AudioHardwarePropertyID propertyID, void* clientData)
{
	(void) propertyID;
	(void) clientData;
	return noErr;
}

static OSStatus objectListener(AudioObjectID object, UInt32 count, const AudioObjectPropertyAddress addresses[], void* clientData)
{
	(void) object;
	(void) count;
	(void) addresses;
	(void) clientData;
	return noErr;
}

int main(void)
{
	int token = 0;
	AudioObjectPropertyAddress address = {
		kAudioHardwarePropertyDefaultOutputDevice, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster
	};

	CHECK(AudioHardwareAddPropertyListener(kAudioHardwarePropertyDefaultOutputDevice, hardwareListener, &token) == noErr);
	CHECK(AudioHardwareAddPropertyListener(kAudioHardwarePropertyDevices, hardwareListener, &token) == noErr);
	CHECK(AudioHardwareAddPropertyListener(kAudioHardwarePropertyDefaultOutputDevice, otherHardwareListener, NULL) == noErr);

	// QUIRK pinned here: duplicate and NULL registrations are refused.
	CHECK(AudioHardwareAddPropertyListener(kAudioHardwarePropertyDefaultOutputDevice, hardwareListener, &token) == kAudioHardwareIllegalOperationError);
	CHECK(AudioHardwareAddPropertyListener(kAudioHardwarePropertyDefaultOutputDevice, NULL, NULL) == kAudioHardwareIllegalOperationError);
	CHECK(AudioHardwareRemovePropertyListener(kAudioHardwarePropertyDefaultOutputDevice, NULL) == kAudioHardwareIllegalOperationError);

	// An AudioObject listener for the same property is independent.
	CHECK(AudioObjectAddPropertyListener(kAudioObjectSystemObject, &address, objectListener, NULL) == noErr);

	CHECK(AudioHardwareRemovePropertyListener(kAudioHardwarePropertyDefaultOutputDevice, hardwareListener) == noErr);
	// Removing it again, or one never registered for the property, is not an error.
	CHECK(AudioHardwareRemovePropertyListener(kAudioHardwarePropertyDefaultOutputDevice, hardwareListener) == noErr);
	CHECK(AudioHardwareRemovePropertyListener(kAudioHardwarePropertyDefaultInputDevice, hardwareListener) == noErr);

	// Once removed, the same proc can be registered again.
	CHECK(AudioHardwareAddPropertyListener(kAudioHardwarePropertyDefaultOutputDevice, hardwareListener, &token) == noErr);

	CHECK(AudioHardwareRemovePropertyListener(kAudioHardwarePropertyDefaultOutputDevice, hardwareListener) == noErr);
	CHECK(AudioHardwareRemovePropertyListener(kAudioHardwarePropertyDefaultOutputDevice, otherHardwareListener) == noErr);
	CHECK(AudioHardwareRemovePropertyListener(kAudioHardwarePropertyDevices, hardwareListener) == noErr);
	CHECK(AudioObjectRemovePropertyListener(kAudioObjectSystemObject, &address, objectListener, NULL) == noErr);

	printf(g_failures ? "FAILED\n" : "PASS\n");
	return g_failures ? 1 : 0;
}
