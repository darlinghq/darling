#ifndef AUDIOHARDWAREBASE_H
#define AUDIOHARDWAREBASE_H

#ifdef __cplusplus
extern "C" {
#endif
	
#include <CoreAudio/CoreAudioTypes.h>

typedef UInt32 AudioObjectID;
typedef UInt32 AudioClassID;
typedef UInt32 AudioObjectPropertySelector;
typedef UInt32 AudioObjectPropertyScope;
typedef UInt32 AudioObjectPropertyElement;

struct AudioObjectPropertyAddress
{
	AudioObjectPropertySelector mSelector;
	AudioObjectPropertyScope mScope;
	AudioObjectPropertyElement mElement;
};

enum
{
	kAudioHardwareNoError                   = 0,
	kAudioHardwareNotRunningError           = 'stop',
	kAudioHardwareUnspecifiedError          = 'what',
	kAudioHardwareUnknownPropertyError      = 'who?',
	kAudioHardwareBadPropertySizeError      = '!siz',
	kAudioHardwareIllegalOperationError     = 'nope',
	kAudioHardwareBadObjectError            = '!obj',
	kAudioHardwareBadDeviceError            = '!dev',
	kAudioHardwareBadStreamError            = '!str',
	kAudioHardwareUnsupportedOperationError = 'unop',
	kAudioDeviceUnsupportedFormatError      = '!dat',
	kAudioDevicePermissionsError            = '!hog',
};

enum
{
	kAudioObjectPropertyScopeGlobal         = 'glob',
	kAudioObjectPropertyScopeInput          = 'inpt',
	kAudioObjectPropertyScopeOutput         = 'outp',
	kAudioObjectPropertyScopePlayThrough    = 'ptru',
	kAudioObjectPropertyElementMaster       = 0,
};

enum
{
	kAudioObjectPropertyBaseClass           = 'bcls',
	kAudioObjectPropertyClass               = 'clas',
	kAudioObjectPropertyOwner               = 'stdv',
	kAudioObjectPropertyName                = 'lnam',
	kAudioObjectPropertyModelName           = 'lmod',
	kAudioObjectPropertyManufacturer        = 'lmak',
	kAudioObjectPropertyElementName         = 'lchn',
	kAudioObjectPropertyElementCategoryName = 'lccn',
	kAudioObjectPropertyElementNumberName   = 'lcnn',
	kAudioObjectPropertyOwnedObjects        = 'ownd',
	kAudioObjectPropertyIdentify            = 'iden',
	kAudioObjectPropertySerialNumber        = 'snum',
	kAudioObjectPropertyFirmwareVersion     = 'fwvn',
};

enum
{
    kAudioDevicePropertyConfigurationApplication        = 'capp',
    kAudioDevicePropertyDeviceUID                       = 'uid ',
    kAudioDevicePropertyModelUID                        = 'muid',
    kAudioDevicePropertyTransportType                   = 'tran',
    kAudioDevicePropertyRelatedDevices                  = 'akin',
    kAudioDevicePropertyClockDomain                     = 'clkd',
    kAudioDevicePropertyDeviceIsAlive                   = 'livn',
    kAudioDevicePropertyDeviceIsRunning                 = 'goin',
    kAudioDevicePropertyDeviceCanBeDefaultDevice        = 'dflt',
    kAudioDevicePropertyDeviceCanBeDefaultSystemDevice  = 'sflt',
    kAudioDevicePropertyLatency                         = 'ltnc',
    kAudioDevicePropertyStreams                         = 'stm#',
    kAudioObjectPropertyControlList                     = 'ctrl',
    kAudioDevicePropertySafetyOffset                    = 'saft',
    kAudioDevicePropertyNominalSampleRate               = 'nsrt',
    kAudioDevicePropertyAvailableNominalSampleRates     = 'nsr#',
    kAudioDevicePropertyIcon                            = 'icon',
    kAudioDevicePropertyIsHidden                        = 'hidn',
    kAudioDevicePropertyPreferredChannelsForStereo      = 'dch2',
    kAudioDevicePropertyPreferredChannelLayout          = 'srnd',
};


// TODO: A lot more is missing

#ifdef __cplusplus
}
#endif

#endif /* AUDIOHARDWAREBASE_H */

