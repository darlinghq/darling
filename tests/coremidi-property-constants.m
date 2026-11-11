// CoreMIDI property-name constants regression test. The expected strings are the values macOS reports. Build as an Objective-C executable linked against
// CoreFoundation and CoreMIDI and run it with `darling shell`. Exits non-zero on failure.
#include <CoreFoundation/CoreFoundation.h>
#include <CoreMIDI/MIDIServices.h>
#include <stdio.h>
#include <string.h>

static const struct { const char *name; const CFStringRef *value; const char *expected; } keys[] = {
	{ "kMIDIPropertyName", &kMIDIPropertyName, "name" },
	{ "kMIDIPropertyModel", &kMIDIPropertyModel, "model" },
	{ "kMIDIPropertyManufacturer", &kMIDIPropertyManufacturer, "manufacturer" },
	{ "kMIDIPropertyUniqueID", &kMIDIPropertyUniqueID, "uniqueID" },
	{ "kMIDIPropertyDeviceID", &kMIDIPropertyDeviceID, "deviceID" },
	{ "kMIDIPropertyNameConfigurationDictionary", &kMIDIPropertyNameConfigurationDictionary, "nameConfigurationDictionary" },
	{ "kMIDIPropertyMaxSysExSpeed", &kMIDIPropertyMaxSysExSpeed, "maxSysExSpeed" },
	{ "kMIDIPropertyNameConfiguration", &kMIDIPropertyNameConfiguration, "nameConfiguration" },
	{ "kMIDIPropertyImage", &kMIDIPropertyImage, "image" },
	{ "kMIDIPropertyDisplayName", &kMIDIPropertyDisplayName, "displayName" },
	{ "kMIDIPropertyOffline", &kMIDIPropertyOffline, "offline" },
	{ "kMIDIPropertyPrivate", &kMIDIPropertyPrivate, "private" },
	{ "kMIDIPropertyDriverVersion", &kMIDIPropertyDriverVersion, "driverVersion" },
	{ "kMIDIPropertyReceiveChannels", &kMIDIPropertyReceiveChannels, "receiveChannels" },
	{ "kMIDIPropertyTransmitChannels", &kMIDIPropertyTransmitChannels, "transmitChannels" },
	{ "kMIDIPropertySupportsMMC", &kMIDIPropertySupportsMMC, "supports MMC" },
	{ "kMIDIPropertySupportsGeneralMIDI", &kMIDIPropertySupportsGeneralMIDI, "supports General MIDI" },
	{ "kMIDIPropertySupportsShowControl", &kMIDIPropertySupportsShowControl, "supports MSC" },
	{ "kMIDIPropertyDriverDeviceEditorApp", &kMIDIPropertyDriverDeviceEditorApp, "editorApp" },
	{ "kMIDIPropertyPanDisruptsStereo", &kMIDIPropertyPanDisruptsStereo, "pan disrupts stereo" },
	{ "kMIDIPropertyProtocolID", &kMIDIPropertyProtocolID, "protocol" },
	{ "kMIDIPropertyUMPActiveGroupBitmap", &kMIDIPropertyUMPActiveGroupBitmap, "active group bitmap" },
	{ "kMIDIPropertyUMPCanTransmitGroupless", &kMIDIPropertyUMPCanTransmitGroupless, "ump endpoint" },
	{ "kMIDIPropertyTransmitsMTC", &kMIDIPropertyTransmitsMTC, "transmits MTC" },
	{ "kMIDIPropertyReceivesMTC", &kMIDIPropertyReceivesMTC, "receives MTC" },
	{ "kMIDIPropertyTransmitsClock", &kMIDIPropertyTransmitsClock, "transmits clock" },
	{ "kMIDIPropertyReceivesClock", &kMIDIPropertyReceivesClock, "receives clock" },
	{ "kMIDIPropertyAdvanceScheduleTimeMuSec", &kMIDIPropertyAdvanceScheduleTimeMuSec, "scheduleAheadMuSec" },
	{ "kMIDIPropertyIsMixer", &kMIDIPropertyIsMixer, "is mixer" },
	{ "kMIDIPropertyIsSampler", &kMIDIPropertyIsSampler, "is sampler" },
	{ "kMIDIPropertyIsEffectUnit", &kMIDIPropertyIsEffectUnit, "is effect unit" },
	{ "kMIDIPropertyIsDrumMachine", &kMIDIPropertyIsDrumMachine, "is drum machine" },
	{ "kMIDIPropertyDriverOwner", &kMIDIPropertyDriverOwner, "driver" },
	{ "kMIDIPropertyCanRoute", &kMIDIPropertyCanRoute, "can route" },
	{ "kMIDIPropertyIsBroadcast", &kMIDIPropertyIsBroadcast, "broadcast" },
	{ "kMIDIPropertyConnectionUniqueID", &kMIDIPropertyConnectionUniqueID, "connUniqueID" },
	{ "kMIDIPropertyIsEmbeddedEntity", &kMIDIPropertyIsEmbeddedEntity, "embedded" },
	{ "kMIDIPropertySingleRealtimeEntity", &kMIDIPropertySingleRealtimeEntity, "single realtime entity" },
	{ "kMIDIPropertyMaxReceiveChannels", &kMIDIPropertyMaxReceiveChannels, "max receive channels" },
	{ "kMIDIPropertyMaxTransmitChannels", &kMIDIPropertyMaxTransmitChannels, "max transmit channels" },
	{ "kMIDIPropertyReceivesBankSelectLSB", &kMIDIPropertyReceivesBankSelectLSB, "receives bank select LSB" },
	{ "kMIDIPropertyReceivesBankSelectMSB", &kMIDIPropertyReceivesBankSelectMSB, "receives bank select MSB" },
	{ "kMIDIPropertyTransmitsBankSelectLSB", &kMIDIPropertyTransmitsBankSelectLSB, "transmits bank select LSB" },
	{ "kMIDIPropertyTransmitsBankSelectMSB", &kMIDIPropertyTransmitsBankSelectMSB, "transmits bank select MSB" },
	{ "kMIDIPropertyReceivesNotes", &kMIDIPropertyReceivesNotes, "receives notes" },
	{ "kMIDIPropertyTransmitsNotes", &kMIDIPropertyTransmitsNotes, "transmits notes" },
	{ "kMIDIPropertyReceivesProgramChanges", &kMIDIPropertyReceivesProgramChanges, "receives program changes" },
	{ "kMIDIPropertyTransmitsProgramChanges", &kMIDIPropertyTransmitsProgramChanges, "transmits program changes" },
};

int main(void)
{
	int n = sizeof(keys) / sizeof(keys[0]), failures = 0;
	CFMutableSetRef seen = CFSetCreateMutable(NULL, 0, &kCFTypeSetCallBacks);
	for (int i = 0; i < n; i++) {
		CFStringRef v = *keys[i].value;
		char buf[128];
		if (!v || CFGetTypeID(v) != CFStringGetTypeID() || CFStringGetLength(v) == 0 || CFSetContainsValue(seen, v)) {
			printf("FAIL: %s is missing, not a string, empty or duplicated\n", keys[i].name);
			failures++;
			continue;
		}
		if (!CFStringGetCString(v, buf, sizeof(buf), kCFStringEncodingUTF8) || strcmp(buf, keys[i].expected) != 0) {
			printf("FAIL: %s is '%s', expected '%s'\n", keys[i].name, buf, keys[i].expected);
			failures++;
		}
		CFSetAddValue(seen, v);
	}
	// Property dictionaries are keyed by these constants, so they must work as dictionary keys.
	CFMutableDictionaryRef props = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CFDictionarySetValue(props, kMIDIPropertyOffline, kCFBooleanTrue);
	if (CFDictionaryGetValue(props, kMIDIPropertyOffline) != kCFBooleanTrue || CFDictionaryGetValue(props, kMIDIPropertyPrivate)) {
		printf("FAIL: constants do not behave as distinct dictionary keys\n");
		failures++;
	}
	printf("%d constants checked, %d failures\n", n, failures);
	return failures != 0;
}
