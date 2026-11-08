#ifndef AUDIOBUFFER_H
#define	AUDIOBUFFER_H
#include <CoreAudio/CoreAudioTypes.h>
#include <CoreServices/MacTypes.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFRunLoop.h>

#ifdef	__cplusplus
extern "C" {
#endif

typedef UInt32 AudioQueuePropertyID;
typedef UInt32 AudioQueueParameterID;
typedef Float32 AudioQueueParameterValue;

enum
{
	kAudioQueueErr_InvalidBuffer = -66687,
	kAudioQueueErr_BufferEmpty = -66686,
	kAudioQueueErr_DisposalPending = -66685,
	kAudioQueueErr_InvalidProperty = -66684,
	kAudioQueueErr_InvalidPropertySize = -66683,
	kAudioQueueErr_InvalidParameter = -66682,
	kAudioQueueErr_CannotStart = -66681,
	kAudioQueueErr_InvalidDevice = -66680,
	kAudioQueueErr_BufferInQueue = -66679,
	kAudioQueueErr_InvalidRunState = -66678,
	kAudioQueueErr_InvalidQueueType = -66677,
	kAudioQueueErr_Permissions = -66676,
	kAudioQueueErr_InvalidPropertyValue = -66675,
	kAudioQueueErr_PrimeTimedOut = -66674,
	kAudioQueueErr_CodecNotFound = -66673,
	kAudioQueueErr_InvalidCodecAccess = -66672,
	kAudioQueueErr_QueueInvalidated = -66671,
	kAudioQueueErr_BufferEnqueuedTwice = -66666,
	kAudioQueueErr_CannotStartYet = -66665,
	kAudioQueueErr_EnqueueDuringReset = -66632,
};

enum
{
	kAudioQueueProperty_IsRunning = 'aqrn',
	kAudioQueueProperty_StreamDescription = 'aqft',
};

enum
{
	kAudioQueueParam_Volume = 1,
};

struct AudioQueueParameterEvent
{
	AudioQueueParameterID mID;
	AudioQueueParameterValue mValue;
};

class AudioQueue;
typedef AudioQueue* AudioQueueRef;

struct AudioQueueBuffer
{
	const UInt32 mAudioDataBytesCapacity;
	void* mAudioData;
	UInt32 mAudioDataByteSize;
	void* mUserData;
	
	UInt32 mPacketDescriptionCapacity;
	AudioStreamPacketDescription* mPacketDescriptions;
	UInt32 mPacketDescriptionCount;
};
typedef AudioQueueBuffer* AudioQueueBufferRef;
	
OSStatus AudioQueueStart(AudioQueueRef inAQ, const AudioTimeStamp *inStartTime);
OSStatus AudioQueuePrime(AudioQueueRef inAQ, UInt32 inNumberOfFramesToPrepare, UInt32 *outNumberOfFramesPrepared);
OSStatus AudioQueueFlush(AudioQueueRef inAQ);
OSStatus AudioQueueStop(AudioQueueRef inAQ, Boolean inImmediate);
OSStatus AudioQueuePause(AudioQueueRef inAQ);
OSStatus AudioQueueReset(AudioQueueRef inAQ);

typedef void (*AudioQueueOutputCallback)(void* inUserData, AudioQueueRef inAQ,
		AudioQueueBufferRef inBuffer);
OSStatus AudioQueueNewOutput(const AudioStreamBasicDescription *inFormat,
		AudioQueueOutputCallback inCallbackProc,
		void *inUserData, CFRunLoopRef inCallbackRunLoop,
		CFStringRef inCallbackRunLoopMode, UInt32 inFlags,
		AudioQueueRef *outAQ);

typedef void (*AudioQueueInputCallback)(void* inUserData, AudioQueueRef inAQ,
		AudioQueueBufferRef inBuffer, const AudioTimeStamp* inStartTime,
		UInt32 inNumberPacketDescriptions,
		const AudioStreamPacketDescription* inPacketDescs);
OSStatus AudioQueueNewInput(const AudioStreamBasicDescription *inFormat,
		AudioQueueInputCallback inCallbackProc,
		void *inUserData, CFRunLoopRef inCallbackRunLoop,
		CFStringRef inCallbackRunLoopMode, UInt32 inFlags,
		AudioQueueRef *outAQ);

OSStatus AudioQueueDispose(AudioQueueRef inAQ, Boolean inImmediate);

OSStatus AudioQueueGetParameter(AudioQueueRef inAQ, AudioQueueParameterID inParamID, AudioQueueParameterValue *outValue);
OSStatus AudioQueueSetParameter(AudioQueueRef inAQ, AudioQueueParameterID inParamID, AudioQueueParameterValue inValue);
 
OSStatus AudioQueueGetProperty(AudioQueueRef inAQ, AudioQueuePropertyID inID, void *outData, UInt32 *ioDataSize);
OSStatus AudioQueueSetProperty(AudioQueueRef inAQ, AudioQueuePropertyID inID, const void *inData, UInt32 inDataSize);
OSStatus AudioQueueGetPropertySize(AudioQueueRef inAQ, AudioQueuePropertyID inID, UInt32 *outDataSize);

typedef void (*AudioQueuePropertyListenerProc)(void* inUserData, AudioQueueRef inAQ, AudioQueuePropertyID inID);
OSStatus AudioQueueAddPropertyListener(AudioQueueRef inAQ, AudioQueuePropertyID inID, AudioQueuePropertyListenerProc inProc, void *inUserData);
OSStatus AudioQueueRemovePropertyListener(AudioQueueRef inAQ, AudioQueuePropertyID inID, AudioQueuePropertyListenerProc inProc, void *inUserData);

OSStatus AudioQueueSetOfflineRenderFormat(AudioQueueRef inAQ, const AudioStreamBasicDescription *inFormat, const AudioChannelLayout *inLayout);
OSStatus AudioQueueOfflineRender(AudioQueueRef inAQ, const AudioTimeStamp *inTimestamp, AudioQueueBufferRef ioBuffer, UInt32 inNumberFrames);

OSStatus AudioQueueAllocateBuffer(AudioQueueRef inAQ, UInt32 inBufferByteSize, AudioQueueBufferRef *outBuffer);
OSStatus AudioQueueAllocateBufferWithPacketDescriptions(AudioQueueRef inAQ,
		UInt32 inBufferByteSize, UInt32 inNumberPacketDescriptions,
		AudioQueueBufferRef *outBuffer);
OSStatus AudioQueueFreeBuffer(AudioQueueRef inAQ, AudioQueueBufferRef inBuffer);
OSStatus AudioQueueEnqueueBuffer(AudioQueueRef inAQ, AudioQueueBufferRef inBuffer,
		UInt32 inNumPacketDescs, const AudioStreamPacketDescription *inPacketDescs);
OSStatus AudioQueueEnqueueBufferWithParameters(AudioQueueRef inAQ,
		AudioQueueBufferRef inBuffer, UInt32 inNumPacketDescs,
		const AudioStreamPacketDescription *inPacketDescs,
		UInt32 inTrimFramesAtStart, UInt32 inTrimFramesAtEnd,
		UInt32 inNumParamValues, const AudioQueueParameterEvent *inParamValues,
		const AudioTimeStamp *inStartTime, AudioTimeStamp *outActualStartTime); 

class AudioQueueTimeline;
typedef AudioQueueTimeline* AudioQueueTimelineRef;

OSStatus AudioQueueCreateTimeline(AudioQueueRef inAQ, AudioQueueTimelineRef *outTimeline);
OSStatus AudioQueueDisposeTimeline(AudioQueueRef inAQ, AudioQueueTimelineRef inTimeline);
OSStatus AudioQueueDeviceGetCurrentTime(AudioQueueRef inAQ, AudioTimeStamp *outTimeStamp);
OSStatus AudioQueueDeviceGetNearestStartTime(AudioQueueRef inAQ, AudioTimeStamp *ioRequestedStartTime, UInt32 inFlags);
OSStatus AudioQueueDeviceTranslateTime(AudioQueueRef inAQ, const AudioTimeStamp *inTime, AudioTimeStamp *outTime);
OSStatus AudioQueueGetCurrentTime(AudioQueueRef inAQ, AudioQueueTimelineRef inTimeline, AudioTimeStamp *outTimeStamp, Boolean *outTimelineDiscontinuity);

#ifdef	__cplusplus
}
#endif

#endif	/* AUDIOBUFFER_H */

