#ifndef AUDIOHARDWARE_H
#define AUDIOHARDWARE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <CoreAudio/CoreAudioTypes.h>
#include <CoreAudio/AudioHardwareBase.h>
#include <CoreFoundation/CFDictionary.h>
#include <dispatch/dispatch.h>
	
enum
{
	kAudioObjectUnknown = 0,
	kAudioObjectSystemObject = 1,
};

typedef OSStatus (*AudioObjectPropertyListenerProc)(AudioObjectID inObjectID,
		UInt32 inNumberAddresses, const AudioObjectPropertyAddress* inAddresses,
		void* inClientData);

#ifdef __BLOCKS__
typedef void (^AudioObjectPropertyListenerBlock)(UInt32 inNumberAddresses,
		const AudioObjectPropertyAddress* inAddresses);
#endif

typedef AudioObjectID AudioDeviceID;

enum
{
	kAudioDeviceUnknown = kAudioObjectUnknown,
};

enum
{
    kAudioObjectPropertyCreator             = 'oplg',
    kAudioObjectPropertyListenerAdded       = 'lisa',
    kAudioObjectPropertyListenerRemoved     = 'lisr',
	kAudioHardwarePropertyDevices                               = 'dev#',
    kAudioHardwarePropertyDefaultInputDevice                    = 'dIn ',
    kAudioHardwarePropertyDefaultOutputDevice                   = 'dOut',
    kAudioHardwarePropertyDefaultSystemOutputDevice             = 'sOut',
    kAudioHardwarePropertyTranslateUIDToDevice                  = 'uidd',
    kAudioHardwarePropertyMixStereoToMono                       = 'stmo',
    kAudioHardwarePropertyPlugInList                            = 'plg#',
    kAudioHardwarePropertyTranslateBundleIDToPlugIn             = 'bidp',
    kAudioHardwarePropertyTransportManagerList                  = 'tmg#',
    kAudioHardwarePropertyTranslateBundleIDToTransportManager   = 'tmbi',
    kAudioHardwarePropertyBoxList                               = 'box#',
    kAudioHardwarePropertyTranslateUIDToBox                     = 'uidb',
    kAudioHardwarePropertyClockDeviceList                       = 'clk#',
    kAudioHardwarePropertyTranslateUIDToClockDevice             = 'uidc',
    kAudioHardwarePropertyProcessIsMaster                       = 'mast',
    kAudioHardwarePropertyIsInitingOrExiting                    = 'inot',
    kAudioHardwarePropertyUserIDChanged                         = 'euid',
    kAudioHardwarePropertyProcessIsAudible                      = 'pmut',
    kAudioHardwarePropertySleepingIsAllowed                     = 'slep',
    kAudioHardwarePropertyUnloadingIsAllowed                    = 'unld',
    kAudioHardwarePropertyHogModeIsAllowed                      = 'hogr',
    kAudioHardwarePropertyUserSessionIsActiveOrHeadless         = 'user',
    kAudioHardwarePropertyServiceRestarted                      = 'srst',
    kAudioHardwarePropertyPowerHint                             = 'powh'

};

enum
{
	kAudioHardwarePropertyRunLoop = 'rnlp',
	kAudioHardwarePropertyDeviceForUID = 'duid',
	kAudioHardwarePropertyPluginForBundleID = 'pibi',
};

enum
{
	    kAudioDevicePropertyDeviceName                          = 'name',
    kAudioDevicePropertyDeviceNameCFString                  = kAudioObjectPropertyName,
    kAudioDevicePropertyDeviceManufacturer                  = 'makr',
    kAudioDevicePropertyDeviceManufacturerCFString          = kAudioObjectPropertyManufacturer,
    kAudioDevicePropertyRegisterBufferList                  = 'rbuf',
    kAudioDevicePropertyBufferSize                          = 'bsiz',
    kAudioDevicePropertyBufferSizeRange                     = 'bsz#',
    kAudioDevicePropertyChannelName                         = 'chnm',
    kAudioDevicePropertyChannelNameCFString                 = kAudioObjectPropertyElementName,
    kAudioDevicePropertyChannelCategoryName                 = 'ccnm',
    kAudioDevicePropertyChannelCategoryNameCFString         = kAudioObjectPropertyElementCategoryName,
    kAudioDevicePropertyChannelNumberName                   = 'cnnm',
    kAudioDevicePropertyChannelNumberNameCFString           = kAudioObjectPropertyElementNumberName,
    kAudioDevicePropertySupportsMixing                      = 'mix?',
    kAudioDevicePropertyStreamFormat                        = 'sfmt',
    kAudioDevicePropertyStreamFormats                       = 'sfm#',
    kAudioDevicePropertyStreamFormatSupported               = 'sfm?',
    kAudioDevicePropertyStreamFormatMatch                   = 'sfmm',
    kAudioDevicePropertyDataSourceNameForID                 = 'sscn',
    kAudioDevicePropertyClockSourceNameForID                = 'cscn',
    kAudioDevicePropertyPlayThruDestinationNameForID        = 'mddn',
    kAudioDevicePropertyChannelNominalLineLevelNameForID    = 'cnlv',
    kAudioDevicePropertyHighPassFilterSettingNameForID      = 'chip'
};

enum
{
	    kAudioDevicePropertyPlugIn                          = 'plug',
    kAudioDevicePropertyDeviceHasChanged                = 'diff',
    kAudioDevicePropertyDeviceIsRunningSomewhere        = 'gone',
    kAudioDeviceProcessorOverload                       = 'over',
    kAudioDevicePropertyIOStoppedAbnormally             = 'stpd',
    kAudioDevicePropertyHogMode                         = 'oink',
    kAudioDevicePropertyBufferFrameSize                 = 'fsiz',
    kAudioDevicePropertyBufferFrameSizeRange            = 'fsz#',
    kAudioDevicePropertyUsesVariableBufferFrameSizes    = 'vfsz',
    kAudioDevicePropertyIOCycleUsage                    = 'ncyc',
    kAudioDevicePropertyStreamConfiguration             = 'slay',
    kAudioDevicePropertyIOProcStreamUsage               = 'suse',
    kAudioDevicePropertyActualSampleRate                = 'asrt',
    kAudioDevicePropertyClockDevice                     = 'apcd'
};

enum {
    kAudioDevicePropertyScopeInput = kAudioObjectPropertyScopeInput,
    kAudioDevicePropertyScopeOutput = kAudioObjectPropertyScopeOutput,
    kAudioDevicePropertyScopePlayThrough = kAudioObjectPropertyScopePlayThrough,
};

enum {
    kAudioDevicePropertyVolumeScalar = 'volm',
    kAudioDevicePropertyVolumeDecibels = 'vold',
    kAudioDevicePropertyVolumeDecibelsToScalar = 'db2v',
    kAudioDevicePropertyVolumeScalarToDecibels = 'v2db',
    kAudioDevicePropertyMute = 'mute',
};

typedef AudioObjectPropertySelector AudioHardwarePropertyID;
typedef AudioObjectPropertySelector AudioDevicePropertyID;

void AudioObjectShow(AudioObjectID inObjectID);

Boolean AudioObjectHasProperty(AudioObjectID inObjectID,
		const AudioObjectPropertyAddress* inAddress);

OSStatus AudioObjectIsPropertySettable(AudioObjectID inObjectID,
		const AudioObjectPropertyAddress* inAddress, Boolean* outIsSettable);

OSStatus AudioObjectGetPropertyDataSize(AudioObjectID inObjectID,
		const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize,
		const void* inQualifierData, UInt32* outDataSize);

OSStatus AudioObjectGetPropertyData(AudioObjectID inObjectID,
		const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize,
		const void* inQualifierData, UInt32* ioDataSize, void* outData);

OSStatus AudioObjectSetPropertyData(AudioObjectID inObjectID,
		const AudioObjectPropertyAddress* inAddress, UInt32 inQualifierDataSize,
		const void* inQualifierData, UInt32 inDataSize, const void* inData);

OSStatus AudioObjectAddPropertyListener(AudioObjectID inObjectID,
		const AudioObjectPropertyAddress* inAddress,
		AudioObjectPropertyListenerProc inListener, void* inClientData);

OSStatus AudioObjectRemovePropertyListener(AudioObjectID inObjectID,
		const AudioObjectPropertyAddress* inAddress,
		AudioObjectPropertyListenerProc inListener, void* inClientData);

#ifdef __BLOCKS__

OSStatus AudioObjectAddPropertyListenerBlock(AudioObjectID inObjectID,
		const AudioObjectPropertyAddress* inAddress,
		dispatch_queue_t inDispatchQueue, AudioObjectPropertyListenerBlock inListener);

OSStatus AudioObjectRemovePropertyListenerBlock(AudioObjectID inObjectID,
		const AudioObjectPropertyAddress* inAddress,
		dispatch_queue_t inDispatchQueue, AudioObjectPropertyListenerBlock inListener);

#endif

OSStatus AudioHardwareUnload(void);

OSStatus AudioHardwareCreateAggregateDevice(CFDictionaryRef, AudioObjectID* outDeviceID);

OSStatus AudioHardwareDestroyAggregateDevice(AudioObjectID inDeviceID);

typedef OSStatus (*AudioDeviceIOProc)(AudioObjectID inObjectID,
		const AudioTimeStamp* inNow, const AudioBufferList* inInputData,
		const AudioTimeStamp* inInputTime,
		AudioBufferList* outOutputData, const AudioTimeStamp* inOutputTime,
		void* inClientData);

typedef AudioDeviceIOProc AudioDeviceIOProcID;

OSStatus AudioDeviceCreateIOProcID(AudioObjectID inDevice,
		AudioDeviceIOProc inProc, void* inClientData,
		AudioDeviceIOProcID* outIOProcID);
OSStatus AudioDeviceAddIOProc(AudioDeviceID inDevice, AudioDeviceIOProc inProc, void *inClientData);

#ifdef __BLOCKS__

typedef void (^AudioDeviceIOBlock)(const AudioTimeStamp* inNow, const AudioBufferList* inInputData,
		const AudioTimeStamp* inInputTime,
		AudioBufferList* outOutputData, const AudioTimeStamp* inOutputTime);

OSStatus AudioDeviceCreateIOProcIDWithBlock(AudioDeviceIOProcID* outIOProcID,
		AudioObjectID inDevice, dispatch_queue_t inDispatchQueue,
		AudioDeviceIOBlock inIOBlock);

#endif

OSStatus AudioDeviceDestroyIOProcID(AudioObjectID inDevice,
		AudioDeviceIOProcID inIOProcID);
OSStatus AudioDeviceRemoveIOProc(AudioDeviceID inDevice, AudioDeviceIOProc inProc);

OSStatus AudioDeviceStart(AudioObjectID inDevice, AudioDeviceIOProcID inProcID);

OSStatus AudioDeviceStartAtTime(AudioObjectID inDevice, AudioDeviceIOProcID inProcID,
		AudioTimeStamp* ioRequestedStartTime, UInt32 inFlags);

OSStatus AudioDeviceStop(AudioObjectID inDevice, AudioDeviceIOProcID inProcID);

struct AudioHardwareIOProcStreamUsage
{
	void* mIOProc;
	UInt32 mNumberStreams;
	UInt32 mStreamIsOn[1];
};

OSStatus AudioDeviceGetProperty(AudioDeviceID inDevice, UInt32 inChannel, Boolean isInput, AudioDevicePropertyID inPropertyID, UInt32* ioPropertyDataSize, void* outPropertyData);
OSStatus AudioDeviceGetPropertyInfo(AudioDeviceID inDevice, UInt32 inChannel, Boolean isInput, AudioDevicePropertyID inPropertyID, UInt32* outSize, Boolean* outWritable);
OSStatus AudioDeviceSetProperty(AudioDeviceID inDevice, const AudioTimeStamp *inWhen, UInt32 inChannel, Boolean isInput, AudioDevicePropertyID inPropertyID, UInt32 inPropertyDataSize, const void *inPropertyData);

OSStatus AudioDeviceGetCurrentTime(AudioObjectID inDevice, AudioTimeStamp* outTime);

OSStatus AudioDeviceTranslateTime(AudioObjectID inDevice, const AudioTimeStamp* inTime,
		AudioTimeStamp* outTime);

OSStatus AudioDeviceGetNearestStartTime(AudioObjectID inDevice,
		AudioTimeStamp* ioRequestedStartTime, UInt32 inFlags);

OSStatus AudioHardwareGetProperty(AudioHardwarePropertyID inPropId, UInt32* ioPropertyDataSize, void* outPropertyData);

#ifdef __cplusplus
}
#endif

#endif /* AUDIOHARDWARE_H */

