#ifndef __MIDIServices_h__
#define __MIDIServices_h__

#include <CoreFoundation/CoreFoundation.h>

/* Identification */
extern const CFStringRef kMIDIPropertyName                        API_AVAILABLE(macos(10.0));
// extern const CFStringRef kMIDIPropertyModel                       API_AVAILABLE(macos(10.0));
extern const CFStringRef kMIDIPropertyManufacturer                API_AVAILABLE(macos(10.0));
extern const CFStringRef kMIDIPropertyUniqueID                    API_AVAILABLE(macos(10.0));
// extern const CFStringRef kMIDIPropertyDeviceID                    API_AVAILABLE(macos(10.0));

/* Capabilities */
// extern const CFStringRef kMIDIPropertySupportsMMC                 API_AVAILABLE(macos(10.2));
// extern const CFStringRef kMIDIPropertySupportsGeneralMIDI         API_AVAILABLE(macos(10.2));
// extern const CFStringRef kMIDIPropertySupportsShowControl         API_AVAILABLE(macos(10.4));

/* Configuration */
// extern const CFStringRef kMIDIPropertyNameConfigurationDictionary API_AVAILABLE(macos(10.15));
// extern const CFStringRef kMIDIPropertyMaxSysExSpeed               API_AVAILABLE(macos(10.0));
// extern const CFStringRef kMIDIPropertyDriverDeviceEditorApp       API_AVAILABLE(macos(10.3));
// extern const CFStringRef kMIDIPropertyNameConfiguration           API_AVAILABLE(macos(10.2)); // @deprecated at macos >= 10.15 && ios >= 13.0

/* Presentation */
// extern const CFStringRef kMIDIPropertyImage                       API_AVAILABLE(macos(10.2));
extern const CFStringRef kMIDIPropertyDisplayName                 API_AVAILABLE(macos(10.4));

/* Audio */
// extern const CFStringRef kMIDIPropertyPanDisruptsStereo           API_AVAILABLE(macos(10.2));

/* Protocols */
// extern const CFStringRef kMIDIPropertyProtocolID                  API_AVAILABLE(macos(11.0));

/* Timing */
// extern const CFStringRef kMIDIPropertyTransmitsMTC                API_AVAILABLE(macos(10.2));
// extern const CFStringRef kMIDIPropertyReceivesMTC                 API_AVAILABLE(macos(10.2));
// extern const CFStringRef kMIDIPropertyTransmitsClock              API_AVAILABLE(macos(10.2));
// extern const CFStringRef kMIDIPropertyReceivesClock               API_AVAILABLE(macos(10.2));
extern const CFStringRef kMIDIPropertyAdvanceScheduleTimeMuSec    API_AVAILABLE(macos(10.0));

/* Roles */
// extern const CFStringRef kMIDIPropertyIsMixer                     API_AVAILABLE(macos(10.2));
// extern const CFStringRef kMIDIPropertyIsSampler                   API_AVAILABLE(macos(10.2));
// extern const CFStringRef kMIDIPropertyIsEffectUnit                API_AVAILABLE(macos(10.2));
// extern const CFStringRef kMIDIPropertyIsDrumMachine               API_AVAILABLE(macos(10.2));

/* Status */
// extern const CFStringRef kMIDIPropertyOffline                     API_AVAILABLE(macos(10.1));
// extern const CFStringRef kMIDIPropertyPrivate                     API_AVAILABLE(macos(10.2));

/* Drivers */
// extern const CFStringRef kMIDIPropertyDriverOwner                 API_AVAILABLE(macos(10.1));
extern const CFStringRef kMIDIPropertyDriverVersion               API_AVAILABLE(macos(10.2));

/* Connections */
// extern const CFStringRef kMIDIPropertyCanRoute                    API_AVAILABLE(macos(10.0));
// extern const CFStringRef kMIDIPropertyIsBroadcast                 API_AVAILABLE(macos(10.2));
extern const CFStringRef kMIDIPropertyConnectionUniqueID          API_AVAILABLE(macos(10.1));
// extern const CFStringRef kMIDIPropertyIsEmbeddedEntity            API_AVAILABLE(macos(10.1));
// extern const CFStringRef kMIDIPropertySingleRealtimeEntity        API_AVAILABLE(macos(10.2));

/* Channels */
// extern const CFStringRef kMIDIPropertyReceiveChannels             API_AVAILABLE(macos(10.0));
// extern const CFStringRef kMIDIPropertyTransmitChannels            API_AVAILABLE(macos(10.2));
// extern const CFStringRef kMIDIPropertyMaxReceiveChannels          API_AVAILABLE(macos(10.2));
// extern const CFStringRef kMIDIPropertyMaxTransmitChannels         API_AVAILABLE(macos(10.2));

/* Banks */
// extern const CFStringRef kMIDIPropertyReceivesBankSelectLSB       API_AVAILABLE(macos(10.2));
// extern const CFStringRef kMIDIPropertyReceivesBankSelectMSB       API_AVAILABLE(macos(10.2));
// extern const CFStringRef kMIDIPropertyTransmitsBankSelectLSB      API_AVAILABLE(macos(10.2));
// extern const CFStringRef kMIDIPropertyTransmitsBankSelectMSB      API_AVAILABLE(macos(10.2));

/* Notes */
// extern const CFStringRef kMIDIPropertyReceivesNotes               API_AVAILABLE(macos(10.2));
// extern const CFStringRef kMIDIPropertyTransmitsNotes              API_AVAILABLE(macos(10.2));

/* Program Changes */
// extern const CFStringRef kMIDIPropertyReceivesProgramChanges      API_AVAILABLE(macos(10.2));
// extern const CFStringRef kMIDIPropertyTransmitsProgramChanges     API_AVAILABLE(macos(10.2));

#endif // __MIDIServices_h__
