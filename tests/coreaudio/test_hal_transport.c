// Copyright (C) 2026 Lubos Dolezel
//
// kAudioDevicePropertyTransportType follows what the sound server knows about how the device is attached
// (the bus), and is the virtual transport for a device that no hardware stands behind.
// Needs the devices of fixture-hal.sh and runs inside Darling.
#include "hal_test.h"

static void Expect(const char* uid, UInt32 transport)
{
	AudioDeviceID d = DeviceWithUID(uid);
	UInt32 value = 0;
	CHECK(d != kAudioObjectUnknown, "%s is not a device", uid);
	CHECK(GetU32(d, kAudioDevicePropertyTransportType, kGlobal, &value) == 0 && value == transport,
		"%s: transport %08x, expected %08x", uid, (unsigned)value, (unsigned)transport);
	Boolean settable = 1;
	AudioObjectPropertyAddress a = Addr(kAudioDevicePropertyTransportType, kGlobal, 0);
	CHECK(AudioObjectIsPropertySettable(d, &a, &settable) == 0 && !settable, "transport type is settable");
}

// The devices of the machine, read only: the server names the bus of a sound card in the name of its devices.
static void CheckRealDevices(void)
{
	AudioObjectPropertyAddress a = Addr(kAudioHardwarePropertyDevices, kGlobal, 0);
	UInt32 size = 0;
	AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &a, 0, NULL, &size);
	AudioObjectID* devices = malloc(size);
	AudioObjectGetPropertyData(kAudioObjectSystemObject, &a, 0, NULL, &size, devices);
	for (UInt32 i = 0; i < size / sizeof(AudioObjectID); i++)
	{
		const char* uid = GetText(devices[i], kAudioDevicePropertyDeviceUID, kGlobal, 0);
		UInt32 transport = 0;
		if (GetU32(devices[i], kAudioDevicePropertyTransportType, kGlobal, &transport) != 0)
			continue;
		printf("-- %s: transport %c%c%c%c\n", uid, (char)(transport >> 24), (char)(transport >> 16), (char)(transport >> 8), (char)transport);
		if (strstr(uid, ".usb-"))
			CHECK(transport == kAudioDeviceTransportTypeUSB, "%s is %08x", uid, (unsigned)transport);
		else if (strstr(uid, "bluez"))
			CHECK(transport == kAudioDeviceTransportTypeBluetooth, "%s is %08x", uid, (unsigned)transport);
		else if (strstr(uid, ".pci-"))
			CHECK(transport == kAudioDeviceTransportTypePCI || transport == kAudioDeviceTransportTypeBuiltIn
				|| transport == kAudioDeviceTransportTypeHDMI || transport == kAudioDeviceTransportTypeDisplayPort, "%s is %08x", uid, (unsigned)transport);
	}
	free(devices);
}

int main(void)
{
	Expect("darling-hal-48k", kAudioDeviceTransportTypeUSB);
	Expect("darling-hal-44k", kAudioDeviceTransportTypePCI);
	Expect("darling-hal-int", kAudioDeviceTransportTypeBuiltIn);
	Expect("darling-hal-51", kAudioDeviceTransportTypeBluetooth);
	Expect("darling-hal-soft", kAudioDeviceTransportTypeVirtual);
	// PipeWire gives a pipe source no bus, whatever is asked for: no hardware stands behind it.
	Expect("darling-hal-mic", kAudioDeviceTransportTypeVirtual);
	Expect("darling-hal-line", kAudioDeviceTransportTypeVirtual);

	CheckRealDevices();

	// A bad size is refused.
	UInt32 value;
	UInt32 size = 2;
	AudioObjectPropertyAddress a = Addr(kAudioDevicePropertyTransportType, kGlobal, 0);
	CHECK(AudioObjectGetPropertyData(DeviceWithUID("darling-hal-48k"), &a, 0, NULL, &size, &value) == kAudioHardwareBadPropertySizeError,
		"a buffer of 2 bytes was accepted");

	printf(failures ? "test_hal_transport: FAILED (%d)\n" : "test_hal_transport: OK\n", failures);
	return failures != 0;
}
