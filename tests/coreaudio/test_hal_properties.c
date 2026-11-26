// Copyright (C) 2026 Lubos Dolezel
//
// The properties of devices that applications commonly read: identity (name, manufacturer, UIDs, serial),
// liveness and running state, latency and buffers, stream configuration and channel names, clock domain and
// related devices, which scopes may be the default, hog mode, the system object's own settings, and the errors
// for what a device does not have. Needs the devices of fixture-hal.sh and runs inside Darling.
#include "hal_test.h"
#include <dispatch/dispatch.h>
#include <unistd.h>

static AudioObjectID Default(AudioObjectPropertySelector s)
{
	AudioObjectID d = 0;
	GetU32(kAudioObjectSystemObject, s, kGlobal, &d);
	return d;
}

static OSStatus Silence(AudioObjectID d, const AudioTimeStamp* now, const AudioBufferList* in, const AudioTimeStamp* inTime,
	AudioBufferList* out, const AudioTimeStamp* outTime, void* ctx)
{
	if (out)
		for (UInt32 i = 0; i < out->mNumberBuffers; i++)
			memset(out->mBuffers[i].mData, 0, out->mBuffers[i].mDataByteSize);
	return 0;
}

static void CheckIdentity(void)
{
	AudioDeviceID d = DeviceWithUID("darling-hal-48k");
	CHECK(d != kAudioObjectUnknown, "no device");
	CHECK(!strcmp(GetText(d, kAudioObjectPropertyName, kGlobal, 0), "HalSink48k"), "name: %s", GetText(d, kAudioObjectPropertyName, kGlobal, 0));
	CHECK(!strcmp(GetText(d, kAudioObjectPropertyManufacturer, kGlobal, 0), "DarlingVendor"), "manufacturer: %s", GetText(d, kAudioObjectPropertyManufacturer, kGlobal, 0));
	CHECK(!strcmp(GetText(d, kAudioDevicePropertyDeviceUID, kGlobal, 0), "darling-hal-48k"), "UID: %s", GetText(d, kAudioDevicePropertyDeviceUID, kGlobal, 0));
	CHECK(!strcmp(GetText(d, kAudioDevicePropertyModelUID, kGlobal, 0), "DarlingUSBHeadset"), "model UID: %s", GetText(d, kAudioDevicePropertyModelUID, kGlobal, 0));
	CHECK(!strcmp(GetText(d, kAudioObjectPropertyModelName, kGlobal, 0), "DarlingUSBHeadset"), "model name: %s", GetText(d, kAudioObjectPropertyModelName, kGlobal, 0));
	CHECK(!strcmp(GetText(d, kAudioObjectPropertySerialNumber, kGlobal, 0), "SN-48K"), "serial: %s", GetText(d, kAudioObjectPropertySerialNumber, kGlobal, 0));

	// The older names of the strings give C strings.
	AudioObjectPropertyAddress a = Addr(kAudioDevicePropertyDeviceName, kGlobal, 0);
	char name[128] = "";
	UInt32 size = sizeof(name);
	CHECK(AudioObjectGetPropertyData(d, &a, 0, NULL, &size, name) == 0 && !strcmp(name, "HalSink48k"), "old name: %s", name);
	a = Addr(kAudioDevicePropertyDeviceManufacturer, kGlobal, 0);
	size = sizeof(name);
	CHECK(AudioObjectGetPropertyData(d, &a, 0, NULL, &size, name) == 0 && !strcmp(name, "DarlingVendor"), "old manufacturer: %s", name);

	// A device without a serial number does not have the property.
	AudioDeviceID soft = DeviceWithUID("darling-hal-soft");
	a = Addr(kAudioObjectPropertySerialNumber, kGlobal, 0);
	CHECK(!AudioObjectHasProperty(soft, &a), "a serial number of a software device");

	UInt32 klass = 0;
	CHECK(GetU32(d, kAudioObjectPropertyClass, kGlobal, &klass) == 0 && klass == kAudioDeviceClassID, "class %08x", (unsigned)klass);
	CHECK(GetU32(kAudioObjectSystemObject, kAudioObjectPropertyClass, kGlobal, &klass) == 0 && klass == kAudioSystemObjectClassID, "system class %08x", (unsigned)klass);
	CHECK(GetU32(StreamOf(d, kOutput), kAudioObjectPropertyClass, kGlobal, &klass) == 0 && klass == kAudioStreamClassID, "stream class %08x", (unsigned)klass);
	CHECK(GetU32(d, kAudioObjectPropertyBaseClass, kGlobal, &klass) == 0 && klass == kAudioObjectClassID, "base class %08x", (unsigned)klass);

	// UIDs are given back by the translation, and by the older property.
	CFStringRef uid = CFStringCreateWithCString(NULL, "darling-hal-48k", kCFStringEncodingUTF8);
	AudioObjectPropertyAddress translate = Addr(kAudioHardwarePropertyTranslateUIDToDevice, kGlobal, 0);
	AudioObjectID found = 99;
	size = sizeof(found);
	CHECK(AudioObjectGetPropertyData(kAudioObjectSystemObject, &translate, sizeof(uid), &uid, &size, &found) == 0 && found == d, "translated to %u", (unsigned)found);
	CHECK(AudioObjectGetPropertyData(kAudioObjectSystemObject, &translate, 0, NULL, &size, &found) == kAudioHardwareBadPropertySizeError, "no qualifier accepted");
	UInt16 tiny;
	size = sizeof(tiny);
	CHECK(AudioObjectGetPropertyData(kAudioObjectSystemObject, &translate, sizeof(uid), &uid, &size, &tiny) == kAudioHardwareBadPropertySizeError, "small buffer accepted");
	CFRelease(uid);
	uid = CFSTR("no-such-device");
	found = 99;
	size = sizeof(found);
	CHECK(AudioObjectGetPropertyData(kAudioObjectSystemObject, &translate, sizeof(uid), &uid, &size, &found) == 0 && found == kAudioObjectUnknown, "unknown UID gave %u", (unsigned)found);
	AudioObjectPropertyAddress old = Addr(kAudioHardwarePropertyDeviceForUID, kGlobal, 0);
	uid = CFSTR("darling-hal-48k");
	AudioValueTranslation translation = { &uid, sizeof(uid), &found, sizeof(found) };
	size = sizeof(translation);
	CHECK(AudioObjectGetPropertyData(kAudioObjectSystemObject, &old, 0, NULL, &size, &translation) == 0 && found == d, "the older translation gave %u", (unsigned)found);
}

static void CheckState(void)
{
	AudioDeviceID d = DeviceWithUID("darling-hal-44k");
	UInt32 v = 9;
	CHECK(GetU32(d, kAudioDevicePropertyDeviceIsAlive, kGlobal, &v) == 0 && v == 1, "alive %u", (unsigned)v);
	CHECK(GetU32(d, kAudioDevicePropertyDeviceIsRunning, kGlobal, &v) == 0 && v == 0, "running %u before IO", (unsigned)v);
	CHECK(GetU32(d, kAudioDevicePropertyDeviceIsRunningSomewhere, kGlobal, &v) == 0, "running somewhere cannot be read");
	CHECK(GetU32(d, kAudioDevicePropertyIsHidden, kGlobal, &v) == 0 && v == 0, "hidden %u", (unsigned)v);

	// IO on the device makes it run; listeners hear of it.
	AudioObjectPropertyAddress runningAddress = Addr(kAudioDevicePropertyDeviceIsRunning, kGlobal, 0);
	notified = 0;
	CHECK(AudioObjectAddPropertyListener(d, &runningAddress, OnChange, NULL) == 0, "listener");
	AudioDeviceIOProcID proc = NULL;
	CHECK(AudioDeviceCreateIOProcID(d, Silence, NULL, &proc) == 0, "create IOProc");
	CHECK(AudioDeviceStart(d, proc) == 0, "start");
	CHECK(GetU32(d, kAudioDevicePropertyDeviceIsRunning, kGlobal, &v) == 0 && v == 1, "running %u during IO", (unsigned)v);
	CHECK(GetU32(d, kAudioDevicePropertyDeviceIsRunningSomewhere, kGlobal, &v) == 0 && v == 1, "running somewhere %u during IO", (unsigned)v);
	for (int i = 0; i < 40 && notified < 1; i++)
		usleep(50000);
	CHECK(notified >= 1, "no notification of the start");
	usleep(300000);
	const int afterStart = notified;
	CHECK(AudioDeviceStop(d, proc) == 0, "stop");
	CHECK(GetU32(d, kAudioDevicePropertyDeviceIsRunning, kGlobal, &v) == 0 && v == 0, "running %u after IO", (unsigned)v);
	for (int i = 0; i < 40 && notified <= afterStart; i++)
		usleep(50000);
	CHECK(notified > afterStart, "no notification of the stop");
	CHECK(AudioDeviceDestroyIOProcID(d, proc) == 0, "destroy IOProc");
	CHECK(AudioObjectRemovePropertyListener(d, &runningAddress, OnChange, NULL) == 0, "remove the listener");
}

static void CheckBuffers(void)
{
	AudioDeviceID d = DeviceWithUID("darling-hal-48k");
	UInt32 frames = 0, safety = 99, latency = 99;
	CHECK(GetU32(d, kAudioDevicePropertyBufferFrameSize, kGlobal, &frames) == 0 && frames > 0, "buffer frames %u", (unsigned)frames);
	AudioObjectPropertyAddress rangeAddress = Addr(kAudioDevicePropertyBufferFrameSizeRange, kGlobal, 0);
	AudioValueRange range = {0, 0};
	UInt32 size = sizeof(range);
	CHECK(AudioObjectGetPropertyData(d, &rangeAddress, 0, NULL, &size, &range) == 0 && range.mMinimum > 0 && range.mMinimum <= frames && frames <= range.mMaximum,
		"range %g..%g for %u", range.mMinimum, range.mMaximum, (unsigned)frames);
	Boolean settable = 0;
	AudioObjectPropertyAddress framesAddress = Addr(kAudioDevicePropertyBufferFrameSize, kGlobal, 0);
	CHECK(AudioObjectIsPropertySettable(d, &framesAddress, &settable) == 0 && settable, "buffer frame size is not settable");
	CHECK(AudioObjectIsPropertySettable(d, &rangeAddress, &settable) == 0 && !settable, "the range is settable or an error");

	// The same in bytes: a frame of the device is 2 channels of 4 bytes.
	UInt32 bytes = 0;
	CHECK(GetU32(d, kAudioDevicePropertyBufferSize, kGlobal, &bytes) == 0 && bytes == frames * 8, "buffer bytes %u for %u frames", (unsigned)bytes, (unsigned)frames);
	AudioObjectPropertyAddress byteRangeAddress = Addr(kAudioDevicePropertyBufferSizeRange, kGlobal, 0);
	AudioValueRange byteRange = {0, 0};
	size = sizeof(byteRange);
	CHECK(AudioObjectGetPropertyData(d, &byteRangeAddress, 0, NULL, &size, &byteRange) == 0 && byteRange.mMinimum == range.mMinimum * 8 && byteRange.mMaximum == range.mMaximum * 8,
		"byte range %g..%g", byteRange.mMinimum, byteRange.mMaximum);

	UInt32 wanted = (UInt32) range.mMinimum * 2;
	CHECK(AudioObjectSetPropertyData(d, &framesAddress, 0, NULL, sizeof(wanted), &wanted) == 0, "set the buffer size");
	CHECK(GetU32(d, kAudioDevicePropertyBufferFrameSize, kGlobal, &frames) == 0 && frames == wanted, "buffer frames %u", (unsigned)frames);
	UInt32 outside = (UInt32) range.mMaximum * 2;
	CHECK(AudioObjectSetPropertyData(d, &framesAddress, 0, NULL, sizeof(outside), &outside) != 0, "a size outside the range was accepted");

	CHECK(GetU32(d, kAudioDevicePropertySafetyOffset, kOutput, &safety) == 0 && safety < 100000, "safety offset %u", (unsigned)safety);
	CHECK(GetU32(d, kAudioDevicePropertyLatency, kOutput, &latency) == 0 && latency < 1000000, "output latency %u", (unsigned)latency);
	AudioDeviceID mic = DeviceWithUID("darling-hal-mic");
	CHECK(GetU32(mic, kAudioDevicePropertyLatency, kInput, &latency) == 0 && latency < 1000000, "input latency %u", (unsigned)latency);
}

static void CheckChannels(void)
{
	AudioDeviceID d = DeviceWithUID("darling-hal-51");
	// Stream configuration: one buffer of 6 channels on the output side, none on the input side.
	AudioObjectPropertyAddress a = Addr(kAudioDevicePropertyStreamConfiguration, kOutput, 0);
	UInt32 size = 0;
	CHECK(AudioObjectGetPropertyDataSize(d, &a, 0, NULL, &size) == 0 && size == offsetof(AudioBufferList, mBuffers) + sizeof(AudioBuffer), "output configuration size %u", (unsigned)size);
	a.mScope = kInput;
	CHECK(AudioObjectGetPropertyDataSize(d, &a, 0, NULL, &size) == 0 && size == offsetof(AudioBufferList, mBuffers), "input configuration size %u", (unsigned)size);
	AudioBufferList list;
	size = 1;
	CHECK(AudioObjectGetPropertyData(d, &a, 0, NULL, &size, &list) == kAudioHardwareBadPropertySizeError, "a buffer too small");

	// Names of the channels.
	CHECK(!strcmp(GetText(d, kAudioObjectPropertyElementName, kOutput, 1), "Front Left"), "channel 1: %s", GetText(d, kAudioObjectPropertyElementName, kOutput, 1));
	// PulseAudio's 5.1 order is front left and right, rear left and right, centre, low frequency.
	CHECK(!strcmp(GetText(d, kAudioObjectPropertyElementName, kOutput, 4), "Rear Right"), "channel 4: %s", GetText(d, kAudioObjectPropertyElementName, kOutput, 4));
	CHECK(!strcmp(GetText(d, kAudioObjectPropertyElementName, kOutput, 6), "Subwoofer"), "channel 6: %s", GetText(d, kAudioObjectPropertyElementName, kOutput, 6));
	CHECK(!*GetText(d, kAudioObjectPropertyElementName, kOutput, 7), "a name for channel 7");
	CHECK(!*GetText(d, kAudioObjectPropertyElementName, kInput, 1), "a name for an input channel of an output device");

	// The layout lists the channels.
	a = Addr(kAudioDevicePropertyPreferredChannelLayout, kOutput, 0);
	size = 0;
	CHECK(AudioObjectGetPropertyDataSize(d, &a, 0, NULL, &size) == 0, "no layout");
	AudioChannelLayout* layout = malloc(size);
	CHECK(AudioObjectGetPropertyData(d, &a, 0, NULL, &size, layout) == 0 && layout->mNumberChannelDescriptions == 6
		&& layout->mChannelDescriptions[0].mChannelLabel == kAudioChannelLabel_Left && layout->mChannelDescriptions[1].mChannelLabel == kAudioChannelLabel_Right
		&& layout->mChannelDescriptions[2].mChannelLabel == kAudioChannelLabel_LeftSurround && layout->mChannelDescriptions[3].mChannelLabel == kAudioChannelLabel_RightSurround
		&& layout->mChannelDescriptions[4].mChannelLabel == kAudioChannelLabel_Center && layout->mChannelDescriptions[5].mChannelLabel == kAudioChannelLabel_LFEScreen,
		"layout of %u channels", (unsigned)layout->mNumberChannelDescriptions);
	free(layout);
	a = Addr(kAudioDevicePropertyPreferredChannelsForStereo, kOutput, 0);
	UInt32 stereo[2] = {0, 0};
	size = sizeof(stereo);
	CHECK(AudioObjectGetPropertyData(d, &a, 0, NULL, &size, stereo) == 0 && stereo[0] == 1 && stereo[1] == 2, "stereo channels %u %u", (unsigned)stereo[0], (unsigned)stereo[1]);
}

static void CheckDefaults(void)
{
	AudioDeviceID d = DeviceWithUID("darling-hal-48k"), mic = DeviceWithUID("darling-hal-mic");
	UInt32 v = 9;
	CHECK(GetU32(d, kAudioDevicePropertyDeviceCanBeDefaultDevice, kOutput, &v) == 0 && v == 1, "output device as default output: %u", (unsigned)v);
	CHECK(GetU32(d, kAudioDevicePropertyDeviceCanBeDefaultDevice, kInput, &v) == 0 && v == 0, "output device as default input: %u", (unsigned)v);
	CHECK(GetU32(d, kAudioDevicePropertyDeviceCanBeDefaultSystemDevice, kOutput, &v) == 0 && v == 1, "output device as system output: %u", (unsigned)v);
	CHECK(GetU32(mic, kAudioDevicePropertyDeviceCanBeDefaultDevice, kInput, &v) == 0 && v == 1, "input device as default input: %u", (unsigned)v);
	CHECK(GetU32(mic, kAudioDevicePropertyDeviceCanBeDefaultSystemDevice, kInput, &v) == 0 && v == 0, "input device as system output: %u", (unsigned)v);
	CHECK(GetU32(d, kAudioDevicePropertyDeviceCanBeDefaultDevice, kGlobal, &v) == kAudioHardwareUnknownPropertyError, "the global scope");
}

static void CheckRelations(void)
{
	AudioDeviceID d = DeviceWithUID("darling-hal-48k");
	AudioObjectPropertyAddress a = Addr(kAudioDevicePropertyRelatedDevices, kGlobal, 0);
	AudioObjectID related[16];
	UInt32 size = sizeof(related);
	CHECK(AudioObjectGetPropertyData(d, &a, 0, NULL, &size, related) == 0 && size >= sizeof(AudioObjectID), "no related devices");
	int self = 0;
	for (UInt32 i = 0; i < size / sizeof(AudioObjectID); i++)
		if (related[i] == d)
			self = 1;
	CHECK(self, "a device is not related to itself");
	// A device of the software has no card, so it is only related to itself and its clock is not known.
	CHECK(size == sizeof(AudioObjectID), "%u related devices", (unsigned)(size / sizeof(AudioObjectID)));
	UInt32 domain = 9;
	CHECK(GetU32(d, kAudioDevicePropertyClockDomain, kGlobal, &domain) == 0 && domain == 0, "clock domain %u", (unsigned)domain);
}

static void CheckHogMode(void)
{
	AudioDeviceID d = DeviceWithUID("darling-hal-48k");
	AudioObjectPropertyAddress a = Addr(kAudioDevicePropertyHogMode, kGlobal, 0);
	pid_t pid = 1;
	UInt32 size = sizeof(pid);
	CHECK(AudioObjectGetPropertyData(d, &a, 0, NULL, &size, &pid) == 0 && pid == -1, "hog mode %d", (int)pid);
	Boolean settable = 0;
	CHECK(AudioObjectIsPropertySettable(d, &a, &settable) == 0 && settable, "hog mode is not settable");

	pid_t me = getpid(), other = me + 1;
	CHECK(AudioObjectSetPropertyData(d, &a, 0, NULL, sizeof(other), &other) == kAudioHardwareIllegalOperationError, "another process took the device");
	notified = 0;
	CHECK(AudioObjectAddPropertyListener(d, &a, OnChange, NULL) == 0, "listener");
	CHECK(AudioObjectSetPropertyData(d, &a, 0, NULL, sizeof(me), &me) == 0, "hog");
	size = sizeof(pid);
	CHECK(AudioObjectGetPropertyData(d, &a, 0, NULL, &size, &pid) == 0 && pid == me, "hog mode %d", (int)pid);
	CHECK(notified == 1, "hog listener called %d times", notified);
	CHECK(AudioObjectSetPropertyData(d, &a, 0, NULL, sizeof(other), &other) == kAudioHardwareIllegalOperationError, "another process took a hogged device");
	pid_t none = -1;
	CHECK(AudioObjectSetPropertyData(d, &a, 0, NULL, sizeof(none), &none) == 0, "release");
	size = sizeof(pid);
	CHECK(AudioObjectGetPropertyData(d, &a, 0, NULL, &size, &pid) == 0 && pid == -1, "hog mode %d", (int)pid);
	CHECK(AudioObjectRemovePropertyListener(d, &a, OnChange, NULL) == 0, "remove the listener");

	// When the process does not allow hogging, a device cannot be taken.
	AudioObjectPropertyAddress allowed = Addr(kAudioHardwarePropertyHogModeIsAllowed, kGlobal, 0);
	UInt32 no = 0, yes = 1, value = 9;
	CHECK(AudioObjectSetPropertyData(kAudioObjectSystemObject, &allowed, 0, NULL, sizeof(no), &no) == 0, "forbid hogging");
	CHECK(AudioObjectSetPropertyData(d, &a, 0, NULL, sizeof(me), &me) == kAudioHardwareIllegalOperationError, "hogged though forbidden");
	CHECK(AudioObjectSetPropertyData(kAudioObjectSystemObject, &allowed, 0, NULL, sizeof(yes), &yes) == 0, "allow hogging");
	CHECK(GetU32(kAudioObjectSystemObject, kAudioHardwarePropertyHogModeIsAllowed, kGlobal, &value) == 0 && value == 1, "allowed %u", (unsigned)value);
}

static void CheckSystemObject(void)
{
	UInt32 v = 9;
	static const AudioObjectPropertySelector settings[] = {
		kAudioHardwarePropertyProcessIsAudible, kAudioHardwarePropertySleepingIsAllowed, kAudioHardwarePropertyUnloadingIsAllowed,
		kAudioHardwarePropertyHogModeIsAllowed, kAudioHardwarePropertyMixStereoToMono, kAudioHardwarePropertyPowerHint
	};
	for (unsigned i = 0; i < sizeof(settings) / sizeof(settings[0]); i++)
	{
		AudioObjectPropertyAddress a = Addr(settings[i], kGlobal, 0);
		Boolean settable = 0;
		CHECK(AudioObjectHasProperty(kAudioObjectSystemObject, &a) && AudioObjectIsPropertySettable(kAudioObjectSystemObject, &a, &settable) == 0 && settable,
			"system setting %08x", (unsigned)settings[i]);
		CHECK(GetU32(kAudioObjectSystemObject, settings[i], kGlobal, &v) == 0, "cannot read %08x", (unsigned)settings[i]);
	}
	CHECK(GetU32(kAudioObjectSystemObject, kAudioHardwarePropertyUserSessionIsActiveOrHeadless, kGlobal, &v) == 0 && v == 1, "session %u", (unsigned)v);

	AudioObjectPropertyAddress mono = Addr(kAudioHardwarePropertyMixStereoToMono, kGlobal, 0);
	UInt32 on = 1;
	notified = 0;
	CHECK(AudioObjectAddPropertyListener(kAudioObjectSystemObject, &mono, OnChange, NULL) == 0, "listener");
	CHECK(AudioObjectSetPropertyData(kAudioObjectSystemObject, &mono, 0, NULL, sizeof(on), &on) == 0, "set");
	CHECK(GetU32(kAudioObjectSystemObject, kAudioHardwarePropertyMixStereoToMono, kGlobal, &v) == 0 && v == 1 && notified == 1, "mono %u, notified %d", (unsigned)v, notified);
	on = 0;
	AudioObjectSetPropertyData(kAudioObjectSystemObject, &mono, 0, NULL, sizeof(on), &on);
	CHECK(AudioObjectRemovePropertyListener(kAudioObjectSystemObject, &mono, OnChange, NULL) == 0, "remove the listener");
	CHECK(AudioObjectSetPropertyData(kAudioObjectSystemObject, &mono, 0, NULL, 2, &on) == kAudioHardwareBadPropertySizeError, "bad size");

	// Lists that stay empty.
	static const AudioObjectPropertySelector empty[] = {
		kAudioHardwarePropertyPlugInList, kAudioHardwarePropertyBoxList, kAudioHardwarePropertyClockDeviceList
	};
	for (unsigned i = 0; i < sizeof(empty) / sizeof(empty[0]); i++)
	{
		AudioObjectPropertyAddress a = Addr(empty[i], kGlobal, 0);
		UInt32 size = 99;
		CHECK(AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &a, 0, NULL, &size) == 0 && size == 0, "list %08x has %u bytes", (unsigned)empty[i], (unsigned)size);
	}
}

static void CheckErrors(void)
{
	AudioDeviceID d = DeviceWithUID("darling-hal-48k");
	AudioObjectPropertyAddress unknown = Addr('zzzz', kGlobal, 0);
	UInt32 size = 4, value = 0;
	CHECK(!AudioObjectHasProperty(d, &unknown), "an unknown property exists");
	CHECK(AudioObjectGetPropertyDataSize(d, &unknown, 0, NULL, &size) == kAudioHardwareUnknownPropertyError, "size of an unknown property");
	CHECK(AudioObjectGetPropertyData(d, &unknown, 0, NULL, &size, &value) == kAudioHardwareUnknownPropertyError, "an unknown property");
	CHECK(AudioObjectSetPropertyData(d, &unknown, 0, NULL, sizeof(value), &value) == kAudioHardwareUnknownPropertyError, "set an unknown property");
	Boolean settable;
	CHECK(AudioObjectIsPropertySettable(d, &unknown, &settable) == kAudioHardwareUnknownPropertyError, "settable unknown property");
	AudioObjectPropertyAddress uid = Addr(kAudioDevicePropertyDeviceUID, kGlobal, 0);
	CHECK(AudioObjectIsPropertySettable(d, &uid, &settable) == 0 && !settable, "the UID is settable or an error");
	CFStringRef text = CFSTR("x");
	CHECK(AudioObjectSetPropertyData(d, &uid, 0, NULL, sizeof(text), &text) == kAudioHardwareUnsupportedOperationError, "the UID was set");
	size = 1;
	CHECK(AudioObjectGetPropertyData(d, &uid, 0, NULL, &size, &value) == kAudioHardwareBadPropertySizeError, "a one byte buffer for a string");
	CHECK(AudioObjectGetPropertyDataSize(999999, &uid, 0, NULL, &size) == kAudioHardwareBadObjectError, "an object that does not exist");
	CHECK(!AudioObjectHasProperty(999999, &uid), "a property of an object that does not exist");

	// The device has no data sources unless the server has ports for it, which a null sink does not.
	AudioObjectPropertyAddress source = Addr(kAudioDevicePropertyDataSource, kOutput, 0);
	CHECK(!AudioObjectHasProperty(d, &source), "a data source of a device without ports");
	CHECK(AudioObjectGetPropertyData(d, &source, 0, NULL, &size, &value) == kAudioHardwareUnknownPropertyError, "data source of a device without ports");
	AudioObjectPropertyAddress jack = Addr(kAudioDevicePropertyJackIsConnected, kOutput, 0);
	CHECK(!AudioObjectHasProperty(d, &jack), "a jack of a device without ports");
	AudioObjectPropertyAddress iostreams = Addr(kAudioDevicePropertyStreams, kOutput, 0);
	CHECK(AudioObjectHasProperty(d, &iostreams), "no streams");
}

// The data sources of the devices of the server, whatever they are: what is read must agree with itself.
static void CheckDataSources(void)
{
	AudioObjectPropertyAddress a = Addr(kAudioHardwarePropertyDevices, kGlobal, 0);
	UInt32 size = 0;
	AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &a, 0, NULL, &size);
	AudioObjectID* devices = malloc(size);
	AudioObjectGetPropertyData(kAudioObjectSystemObject, &a, 0, NULL, &size, devices);
	int checked = 0;
	for (UInt32 i = 0; i < size / sizeof(AudioObjectID); i++)
	{
		for (int k = 0; k < 2; k++)
		{
			AudioObjectPropertyScope scope = k ? kInput : kOutput;
			AudioObjectPropertyAddress source = Addr(kAudioDevicePropertyDataSource, scope, 0);
			AudioObjectPropertyAddress sources = Addr(kAudioDevicePropertyDataSources, scope, 0);
			if (!AudioObjectHasProperty(devices[i], &source))
				continue;
			UInt32 current = 0, count = 0;
			UInt32 sourceSize = sizeof(current);
			CHECK(AudioObjectGetPropertyData(devices[i], &source, 0, NULL, &sourceSize, &current) == 0, "cannot read the data source");
			CHECK(AudioObjectHasProperty(devices[i], &sources), "no list of data sources");
			UInt32 listSize = 0;
			AudioObjectGetPropertyDataSize(devices[i], &sources, 0, NULL, &listSize);
			UInt32 list[32];
			UInt32 listBytes = sizeof(list);
			CHECK(AudioObjectGetPropertyData(devices[i], &sources, 0, NULL, &listBytes, list) == 0 && listBytes == listSize, "list of sources");
			int found = 0;
			for (count = 0; count < listBytes / sizeof(UInt32); count++)
			{
				if (list[count] == current)
					found = 1;
				// every data source has a name
				CFStringRef name = NULL;
				AudioValueTranslation translation = { &list[count], sizeof(UInt32), &name, sizeof(name) };
				AudioObjectPropertyAddress nameAddress = Addr(kAudioDevicePropertyDataSourceNameForID, scope, 0);
				UInt32 translationSize = sizeof(translation);
				CHECK(AudioObjectGetPropertyData(devices[i], &nameAddress, 0, NULL, &translationSize, &translation) == 0 && name && CFStringGetLength(name) > 0,
					"a data source without a name");
				if (name)
					CFRelease(name);
			}
			CHECK(found, "the current data source is not among the data sources");
			Boolean settable = 0;
			CHECK(AudioObjectIsPropertySettable(devices[i], &source, &settable) == 0 && settable == (listBytes / sizeof(UInt32) > 1), "settable %d for %u sources", settable, (unsigned)(listBytes / sizeof(UInt32)));
			UInt32 bad = 'nope';
			CHECK(AudioObjectSetPropertyData(devices[i], &source, 0, NULL, sizeof(bad), &bad) != 0, "an unknown data source was set");
			checked++;
		}
	}
	free(devices);
	printf("-- data sources of %d device sides checked\n", checked);
}

int main(void)
{
	CheckIdentity();
	CheckState();
	CheckBuffers();
	CheckChannels();
	CheckDefaults();
	CheckRelations();
	CheckHogMode();
	CheckSystemObject();
	CheckErrors();
	CheckDataSources();

	printf(failures ? "test_hal_properties: FAILED (%d)\n" : "test_hal_properties: OK\n", failures);
	return failures != 0;
}
