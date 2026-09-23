#import <CoreMIDI/MIDIServices.h>

// Apple documents these keys but not their string values. Darling has no MIDI server, so the keys only
// travel between callers and this framework: each value is its constant name without the prefix.

/* Identification */
const CFStringRef kMIDIPropertyName = CFSTR("name");
const CFStringRef kMIDIPropertyModel = CFSTR("model");
const CFStringRef kMIDIPropertyManufacturer = CFSTR("manufacturer");
const CFStringRef kMIDIPropertyUniqueID = CFSTR("uniqueID");
const CFStringRef kMIDIPropertyDeviceID = CFSTR("deviceID");

/* Capabilities */
const CFStringRef kMIDIPropertySupportsMMC = CFSTR("supportsMMC");
const CFStringRef kMIDIPropertySupportsGeneralMIDI = CFSTR("supportsGeneralMIDI");
const CFStringRef kMIDIPropertySupportsShowControl = CFSTR("supportsShowControl");

/* Configuration */
const CFStringRef kMIDIPropertyNameConfigurationDictionary = CFSTR("nameConfigurationDictionary");
const CFStringRef kMIDIPropertyMaxSysExSpeed = CFSTR("maxSysExSpeed");
const CFStringRef kMIDIPropertyDriverDeviceEditorApp = CFSTR("driverDeviceEditorApp");
const CFStringRef kMIDIPropertyNameConfiguration = CFSTR("nameConfiguration");

/* Presentation */
const CFStringRef kMIDIPropertyImage = CFSTR("image");
const CFStringRef kMIDIPropertyDisplayName = CFSTR("displayName");

/* Audio */
const CFStringRef kMIDIPropertyPanDisruptsStereo = CFSTR("panDisruptsStereo");

/* Protocols */
const CFStringRef kMIDIPropertyProtocolID = CFSTR("protocolID");

/* Universal MIDI Packet */
const CFStringRef kMIDIPropertyUMPActiveGroupBitmap = CFSTR("umpActiveGroupBitmap");
const CFStringRef kMIDIPropertyUMPCanTransmitGroupless = CFSTR("umpCanTransmitGroupless");

/* Timing */
const CFStringRef kMIDIPropertyTransmitsMTC = CFSTR("transmitsMTC");
const CFStringRef kMIDIPropertyReceivesMTC = CFSTR("receivesMTC");
const CFStringRef kMIDIPropertyTransmitsClock = CFSTR("transmitsClock");
const CFStringRef kMIDIPropertyReceivesClock = CFSTR("receivesClock");
const CFStringRef kMIDIPropertyAdvanceScheduleTimeMuSec = CFSTR("advanceScheduleTimeMuSec");

/* Roles */
const CFStringRef kMIDIPropertyIsMixer = CFSTR("isMixer");
const CFStringRef kMIDIPropertyIsSampler = CFSTR("isSampler");
const CFStringRef kMIDIPropertyIsEffectUnit = CFSTR("isEffectUnit");
const CFStringRef kMIDIPropertyIsDrumMachine = CFSTR("isDrumMachine");

/* Status */
const CFStringRef kMIDIPropertyOffline = CFSTR("offline");
const CFStringRef kMIDIPropertyPrivate = CFSTR("private");

/* Drivers */
const CFStringRef kMIDIPropertyDriverOwner = CFSTR("driverOwner");
const CFStringRef kMIDIPropertyDriverVersion = CFSTR("driverVersion");

/* Connections */
const CFStringRef kMIDIPropertyCanRoute = CFSTR("canRoute");
const CFStringRef kMIDIPropertyIsBroadcast = CFSTR("isBroadcast");
const CFStringRef kMIDIPropertyConnectionUniqueID = CFSTR("connectionUniqueID");
const CFStringRef kMIDIPropertyIsEmbeddedEntity = CFSTR("isEmbeddedEntity");
const CFStringRef kMIDIPropertySingleRealtimeEntity = CFSTR("singleRealtimeEntity");

/* Channels */
const CFStringRef kMIDIPropertyReceiveChannels = CFSTR("receiveChannels");
const CFStringRef kMIDIPropertyTransmitChannels = CFSTR("transmitChannels");
const CFStringRef kMIDIPropertyMaxReceiveChannels = CFSTR("maxReceiveChannels");
const CFStringRef kMIDIPropertyMaxTransmitChannels = CFSTR("maxTransmitChannels");

/* Banks */
const CFStringRef kMIDIPropertyReceivesBankSelectLSB = CFSTR("receivesBankSelectLSB");
const CFStringRef kMIDIPropertyReceivesBankSelectMSB = CFSTR("receivesBankSelectMSB");
const CFStringRef kMIDIPropertyTransmitsBankSelectLSB = CFSTR("transmitsBankSelectLSB");
const CFStringRef kMIDIPropertyTransmitsBankSelectMSB = CFSTR("transmitsBankSelectMSB");

/* Notes */
const CFStringRef kMIDIPropertyReceivesNotes = CFSTR("receivesNotes");
const CFStringRef kMIDIPropertyTransmitsNotes = CFSTR("transmitsNotes");

/* Program Changes */
const CFStringRef kMIDIPropertyReceivesProgramChanges = CFSTR("receivesProgramChanges");
const CFStringRef kMIDIPropertyTransmitsProgramChanges = CFSTR("transmitsProgramChanges");
