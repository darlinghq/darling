// CFLAGS: -framework CoreMediaIO
// Copyright (C) 2026 Darling Developers
//
// Exercises the public CMIO object property entry points against the system
// object's screen capture device property. The property is a UInt32 boolean
// and round-trips for every caller; unsupported objects and selectors must
// report the documented CMIO errors.
#include <CoreMediaIO/CMIOHardware.h>
#include <stdio.h>

static int failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL: %s (line %d)\n", #cond, __LINE__); failures++; } } while (0)

int main(void)
{
	CMIOObjectPropertyAddress allow = {
		kCMIOHardwarePropertyAllowScreenCaptureDevices,
		kCMIOObjectPropertyScopeGlobal,
		kCMIOObjectPropertyElementMain,
	};
	CMIOObjectPropertyAddress unknown = { 0x74657374, kCMIOObjectPropertyScopeGlobal, kCMIOObjectPropertyElementMain };
	UInt32 value = 0;
	UInt32 used = 0;
	UInt32 size = 0;
	Boolean settable = 0;
	UInt32 enabled = 1;
	UInt32 disabled = 0;

	CHECK(kCMIOHardwarePropertyAllowScreenCaptureDevices == 0x79657320);
	CHECK(CMIOObjectHasProperty(kCMIOObjectSystemObject, &allow));
	CHECK(!CMIOObjectHasProperty(0, &allow));
	CHECK(!CMIOObjectHasProperty(kCMIOObjectSystemObject, NULL));
	CHECK(CMIOObjectIsPropertySettable(kCMIOObjectSystemObject, &allow, &settable) == noErr);
	CHECK(settable);
	CHECK(CMIOObjectIsPropertySettable(kCMIOObjectSystemObject, &allow, NULL)
	      == kCMIOHardwareIllegalOperationError);
	CHECK(CMIOObjectIsPropertySettable(kCMIOObjectSystemObject, NULL, &settable)
	      == kCMIOHardwareUnknownPropertyError);
	CHECK(CMIOObjectGetPropertyDataSize(kCMIOObjectSystemObject, &allow, 0, NULL, NULL)
	      == kCMIOHardwareIllegalOperationError);
	CHECK(CMIOObjectIsPropertySettable(0, &allow, &settable) == kCMIOHardwareBadObjectError);
	CHECK(CMIOObjectIsPropertySettable(kCMIOObjectSystemObject, &unknown, &settable)
	      == kCMIOHardwareUnknownPropertyError);

	CHECK(CMIOObjectGetPropertyDataSize(kCMIOObjectSystemObject, &allow, 0, NULL, &size) == noErr);
	CHECK(size == sizeof(UInt32));
	CHECK(CMIOObjectGetPropertyDataSize(kCMIOObjectSystemObject, &unknown, 0, NULL, &size)
	      == kCMIOHardwareUnknownPropertyError);
	CHECK(CMIOObjectGetPropertyDataSize(kCMIOObjectSystemObject, &allow, 1, NULL, &size)
	      == kCMIOHardwareBadPropertySizeError);

	CHECK(CMIOObjectSetPropertyData(kCMIOObjectSystemObject, &allow, 0, NULL,
	                                sizeof(enabled), &enabled) == noErr);
	CHECK(CMIOObjectGetPropertyData(kCMIOObjectSystemObject, &allow, 0, NULL,
	                                sizeof(value), &used, &value) == noErr);
	CHECK(used == sizeof(value));
	CHECK(value == 1);

	CHECK(CMIOObjectSetPropertyData(kCMIOObjectSystemObject, &allow, 0, NULL,
	                                sizeof(disabled), &disabled) == noErr);
	CHECK(CMIOObjectGetPropertyData(kCMIOObjectSystemObject, &allow, 0, NULL,
	                                sizeof(value), &used, &value) == noErr);
	CHECK(value == 0);

	CHECK(CMIOObjectSetPropertyData(kCMIOObjectSystemObject, &allow, 0, NULL,
	                                1, &enabled) == kCMIOHardwareBadPropertySizeError);
	CHECK(CMIOObjectGetPropertyData(kCMIOObjectSystemObject, &allow, 0, NULL,
	                                1, &used, &value) == kCMIOHardwareBadPropertySizeError);
	CHECK(CMIOObjectSetPropertyData(kCMIOObjectSystemObject, &unknown, 0, NULL,
	                                sizeof(enabled), &enabled) == kCMIOHardwareUnknownPropertyError);

	if (failures) {
		printf("FAILED %d\n", failures);
		return 1;
	}
	puts("OK");
	return 0;
}
