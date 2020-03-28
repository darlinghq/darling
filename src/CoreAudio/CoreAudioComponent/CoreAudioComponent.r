#include <AudioUnit/AudioUnit.r>

#define kAudioUnitResID_AUHAL				1000
#define kAudioUnitResID_DefaultOutputAU		1003
#define kAudioUnitResID_SystemOutputAU		1006
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ AUHAL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define RES_ID			kAudioUnitResID_AUHAL
#define COMP_TYPE		kAudioUnitType_Output
#define COMP_SUBTYPE	kAudioUnitSubType_HALOutput
#define COMP_MANUF		kAudioUnitManufacturer_Apple
#define VERSION			1
#define NAME			"Darling: AudioDeviceOutput"
#define DESCRIPTION		"AudioDevice interface output unit"
#define ENTRY_POINT		"AUHALEntry"
#include "AUResources.r"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DefaultOutputAU ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define RES_ID			kAudioUnitResID_DefaultOutputAU
#define COMP_TYPE		kAudioUnitType_Output
#define COMP_SUBTYPE	kAudioUnitSubType_DefaultOutput
#define COMP_MANUF		kAudioUnitManufacturer_Apple
#define VERSION			1
#define NAME			"Darling: DefaultOutputUnit"
#define DESCRIPTION		"Default output unit that redirects its output to the user selected default device"
#define ENTRY_POINT		"DefaultOutputAUEntry"
#include "AUResources.r"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SystemOutputAU ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define RES_ID			kAudioUnitResID_SystemOutputAU
#define COMP_TYPE		kAudioUnitType_Output
#define COMP_SUBTYPE	kAudioUnitSubType_SystemOutput
#define COMP_MANUF		kAudioUnitManufacturer_Apple
#define VERSION			1
#define NAME			"Darling: SystemOutputUnit"
#define DESCRIPTION		"System Sounds output unit that redirects its output to the user selected System Sounds device"
#define ENTRY_POINT		"SystemOutputAUEntry"
#include "AUResources.r"

