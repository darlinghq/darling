#import <CoreMIDI/MIDIServices.h>

// The string values are those macOS reports for the public constants (black-box observation, see the
// test); the headers do not document them. The values listed in the test as unchanged equal the
// constant name without the prefix. On macOS the constants are NULL until the first CoreMIDI call;
// here they are set from load time, which satisfies callers that only read them after a CoreMIDI call.

/* Identification */
const CFStringRef kMIDIPropertyName = CFSTR("name");
const CFStringRef kMIDIPropertyModel = CFSTR("model");
const CFStringRef kMIDIPropertyManufacturer = CFSTR("manufacturer");
const CFStringRef kMIDIPropertyUniqueID = CFSTR("uniqueID");
const CFStringRef kMIDIPropertyDeviceID = CFSTR("deviceID");

/* Capabilities */
const CFStringRef kMIDIPropertySupportsMMC = CFSTR("supports MMC");
const CFStringRef kMIDIPropertySupportsGeneralMIDI = CFSTR("supports General MIDI");
const CFStringRef kMIDIPropertySupportsShowControl = CFSTR("supports MSC");

/* Configuration */
const CFStringRef kMIDIPropertyNameConfigurationDictionary = CFSTR("nameConfigurationDictionary");
const CFStringRef kMIDIPropertyMaxSysExSpeed = CFSTR("maxSysExSpeed");
const CFStringRef kMIDIPropertyDriverDeviceEditorApp = CFSTR("editorApp");
const CFStringRef kMIDIPropertyNameConfiguration = CFSTR("nameConfiguration");

/* Presentation */
const CFStringRef kMIDIPropertyImage = CFSTR("image");
const CFStringRef kMIDIPropertyDisplayName = CFSTR("displayName");

/* Audio */
const CFStringRef kMIDIPropertyPanDisruptsStereo = CFSTR("pan disrupts stereo");

/* Protocols */
const CFStringRef kMIDIPropertyProtocolID = CFSTR("protocol");

/* Universal MIDI Packet */
const CFStringRef kMIDIPropertyUMPActiveGroupBitmap = CFSTR("active group bitmap");
const CFStringRef kMIDIPropertyUMPCanTransmitGroupless = CFSTR("ump endpoint");

/* Timing */
const CFStringRef kMIDIPropertyTransmitsMTC = CFSTR("transmits MTC");
const CFStringRef kMIDIPropertyReceivesMTC = CFSTR("receives MTC");
const CFStringRef kMIDIPropertyTransmitsClock = CFSTR("transmits clock");
const CFStringRef kMIDIPropertyReceivesClock = CFSTR("receives clock");
const CFStringRef kMIDIPropertyAdvanceScheduleTimeMuSec = CFSTR("scheduleAheadMuSec");

/* Roles */
const CFStringRef kMIDIPropertyIsMixer = CFSTR("is mixer");
const CFStringRef kMIDIPropertyIsSampler = CFSTR("is sampler");
const CFStringRef kMIDIPropertyIsEffectUnit = CFSTR("is effect unit");
const CFStringRef kMIDIPropertyIsDrumMachine = CFSTR("is drum machine");

/* Status */
const CFStringRef kMIDIPropertyOffline = CFSTR("offline");
const CFStringRef kMIDIPropertyPrivate = CFSTR("private");

/* Drivers */
const CFStringRef kMIDIPropertyDriverOwner = CFSTR("driver");
const CFStringRef kMIDIPropertyDriverVersion = CFSTR("driverVersion");

/* Connections */
const CFStringRef kMIDIPropertyCanRoute = CFSTR("can route");
const CFStringRef kMIDIPropertyIsBroadcast = CFSTR("broadcast");
const CFStringRef kMIDIPropertyConnectionUniqueID = CFSTR("connUniqueID");
const CFStringRef kMIDIPropertyIsEmbeddedEntity = CFSTR("embedded");
const CFStringRef kMIDIPropertySingleRealtimeEntity = CFSTR("single realtime entity");

/* Channels */
const CFStringRef kMIDIPropertyReceiveChannels = CFSTR("receiveChannels");
const CFStringRef kMIDIPropertyTransmitChannels = CFSTR("transmitChannels");
const CFStringRef kMIDIPropertyMaxReceiveChannels = CFSTR("max receive channels");
const CFStringRef kMIDIPropertyMaxTransmitChannels = CFSTR("max transmit channels");

/* Banks */
const CFStringRef kMIDIPropertyReceivesBankSelectLSB = CFSTR("receives bank select LSB");
const CFStringRef kMIDIPropertyReceivesBankSelectMSB = CFSTR("receives bank select MSB");
const CFStringRef kMIDIPropertyTransmitsBankSelectLSB = CFSTR("transmits bank select LSB");
const CFStringRef kMIDIPropertyTransmitsBankSelectMSB = CFSTR("transmits bank select MSB");

/* Notes */
const CFStringRef kMIDIPropertyReceivesNotes = CFSTR("receives notes");
const CFStringRef kMIDIPropertyTransmitsNotes = CFSTR("transmits notes");

/* Program Changes */
const CFStringRef kMIDIPropertyReceivesProgramChanges = CFSTR("receives program changes");
const CFStringRef kMIDIPropertyTransmitsProgramChanges = CFSTR("transmits program changes");
