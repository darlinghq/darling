// CoreMIDI property-name constants regression test. Build as an Objective-C executable linked against
// CoreFoundation and CoreMIDI and run it with `darling shell`. Exits non-zero on failure.
#include <CoreFoundation/CoreFoundation.h>
#include <CoreMIDI/MIDIServices.h>
#include <stdio.h>

#define K(name) { #name, &name }
static const struct { const char *name; const CFStringRef *value; } keys[] = {
	K(kMIDIPropertyName), K(kMIDIPropertyModel), K(kMIDIPropertyManufacturer), K(kMIDIPropertyUniqueID),
	K(kMIDIPropertyDeviceID), K(kMIDIPropertySupportsMMC), K(kMIDIPropertySupportsGeneralMIDI),
	K(kMIDIPropertySupportsShowControl), K(kMIDIPropertyNameConfigurationDictionary),
	K(kMIDIPropertyMaxSysExSpeed), K(kMIDIPropertyDriverDeviceEditorApp), K(kMIDIPropertyNameConfiguration),
	K(kMIDIPropertyImage), K(kMIDIPropertyDisplayName), K(kMIDIPropertyPanDisruptsStereo),
	K(kMIDIPropertyProtocolID), K(kMIDIPropertyUMPActiveGroupBitmap), K(kMIDIPropertyUMPCanTransmitGroupless),
	K(kMIDIPropertyTransmitsMTC), K(kMIDIPropertyReceivesMTC), K(kMIDIPropertyTransmitsClock),
	K(kMIDIPropertyReceivesClock), K(kMIDIPropertyAdvanceScheduleTimeMuSec), K(kMIDIPropertyIsMixer),
	K(kMIDIPropertyIsSampler), K(kMIDIPropertyIsEffectUnit), K(kMIDIPropertyIsDrumMachine),
	K(kMIDIPropertyOffline), K(kMIDIPropertyPrivate), K(kMIDIPropertyDriverOwner),
	K(kMIDIPropertyDriverVersion), K(kMIDIPropertyCanRoute), K(kMIDIPropertyIsBroadcast),
	K(kMIDIPropertyConnectionUniqueID), K(kMIDIPropertyIsEmbeddedEntity), K(kMIDIPropertySingleRealtimeEntity),
	K(kMIDIPropertyReceiveChannels), K(kMIDIPropertyTransmitChannels), K(kMIDIPropertyMaxReceiveChannels),
	K(kMIDIPropertyMaxTransmitChannels), K(kMIDIPropertyReceivesBankSelectLSB),
	K(kMIDIPropertyReceivesBankSelectMSB), K(kMIDIPropertyTransmitsBankSelectLSB),
	K(kMIDIPropertyTransmitsBankSelectMSB), K(kMIDIPropertyReceivesNotes), K(kMIDIPropertyTransmitsNotes),
	K(kMIDIPropertyReceivesProgramChanges), K(kMIDIPropertyTransmitsProgramChanges),
};

int main(void)
{
	int n = sizeof(keys) / sizeof(keys[0]), failures = 0;
	CFMutableSetRef seen = CFSetCreateMutable(NULL, 0, &kCFTypeSetCallBacks);
	for (int i = 0; i < n; i++) {
		CFStringRef v = *keys[i].value;
		if (!v || CFGetTypeID(v) != CFStringGetTypeID() || CFStringGetLength(v) == 0 || CFSetContainsValue(seen, v)) {
			printf("FAIL: %s is missing, not a string, empty or duplicated\n", keys[i].name);
			failures++;
			continue;
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
