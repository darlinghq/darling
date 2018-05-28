#ifndef AUDIOHARDWARE_H
#define AUDIOHARDWARE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <CoreAudio/CoreAudiotypes.h>
#include <CoreAudio/AudioHardwareBase.h>
	
enum
{
	kAudioObjectSystemObject = 1
};

typedef OSStatus (*AudioObjectPropertyListenerProc)(AudioObjectID inObjectID,
		UInt32 inNumberAddresses, const AudioObjectPropertyAddress* inAddresses,
		void* inClientData);

#ifdef __BLOCKS__
typedef void (^AudioObjectPropertyListenerBlock)(UInt32 inNumberAddresses,
		const AudioObjectPropertyAddress* inAddresses);
#endif

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

OSStatus AudioDeviceGetCurrentTime(AudioObjectID inDevice, AudioTimeStamp* outTime);

OSStatus AudioDeviceTranslateTime(AudioObjectID inDevice, const AudioTimeStamp* inTime,
		AudioTimeStamp* outTime);

OSStatus AudioDeviceGetNearestStartTime(AudioObjectID inDevice,
		AudioTimeStamp* ioRequestedStartTime, UInt32 inFlags);

#ifdef __cplusplus
}
#endif

#endif /* AUDIOHARDWARE_H */

