#ifndef AUDIOBUFFER_H
#define	AUDIOBUFFER_H
#include <CoreAudio/CoreAudioTypes.h>
#include <CoreServices/MacTypes.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFRunLoop.h>
#include <dispatch/dispatch.h>

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
	kAudioQueueErr_TooManyTaps = -66670,
	kAudioQueueErr_InvalidTapContext = -66669,
	kAudioQueueErr_RecordUnderrun = -66668,
	kAudioQueueErr_InvalidTapType = -66667,
	kAudioQueueErr_BufferEnqueuedTwice = -66666,
	kAudioQueueErr_CannotStartYet = -66665,
	kAudioQueueErr_EnqueueDuringReset = -66632,
	kAudioQueueErr_InvalidOfflineMode = -66626,
};

enum
{
	kAudioQueueProperty_IsRunning = 'aqrn',
	kAudioQueueDeviceProperty_SampleRate = 'aqsr',
	kAudioQueueDeviceProperty_NumberChannels = 'aqdc',
	kAudioQueueProperty_CurrentDevice = 'aqcd',
	kAudioQueueProperty_MagicCookie = 'aqmc',
	kAudioQueueProperty_MaximumOutputPacketSize = 'xops',
	kAudioQueueProperty_StreamDescription = 'aqft',
	kAudioQueueProperty_ChannelLayout = 'aqcl',
	kAudioQueueProperty_EnableLevelMetering = 'aqme',
	kAudioQueueProperty_CurrentLevelMeter = 'aqmv',
	kAudioQueueProperty_CurrentLevelMeterDB = 'aqmd',
	kAudioQueueProperty_DecodeBufferSizeFrames = 'dcbf',
	kAudioQueueProperty_ConverterError = 'qcve',
	kAudioQueueProperty_EnableTimePitch = 'q_tp',
	kAudioQueueProperty_TimePitchAlgorithm = 'qtpa',
	kAudioQueueProperty_TimePitchBypass = 'qtpb',
};

enum
{
	kAudioQueueTimePitchAlgorithm_Spectral = 'spec',
	kAudioQueueTimePitchAlgorithm_TimeDomain = 'tido',
	kAudioQueueTimePitchAlgorithm_Varispeed = 'vspd',
};

typedef struct AudioQueueLevelMeterState
{
	Float32 mAveragePower;
	Float32 mPeakPower;
} AudioQueueLevelMeterState;

enum
{
	kAudioQueueParam_Volume = 1,
	kAudioQueueParam_PlayRate = 2,
	kAudioQueueParam_Pitch = 3,
	kAudioQueueParam_VolumeRampTime = 4,
	kAudioQueueParam_Pan = 13,
};

typedef struct AudioQueueParameterEvent
{
	AudioQueueParameterID mID;
	AudioQueueParameterValue mValue;
} AudioQueueParameterEvent;

typedef struct OpaqueAudioQueue* AudioQueueRef;

typedef UInt32 AudioQueueProcessingTapFlags;
enum
{
	kAudioQueueProcessingTap_PreEffects = (1 << 0),
	kAudioQueueProcessingTap_PostEffects = (1 << 1),
	kAudioQueueProcessingTap_Siphon = (1 << 2),
	kAudioQueueProcessingTap_StartOfStream = (1 << 8),
	kAudioQueueProcessingTap_EndOfStream = (1 << 9),
};

typedef struct OpaqueAudioQueueProcessingTap* AudioQueueProcessingTapRef;

typedef void (*AudioQueueProcessingTapCallback)(void* inClientData,
		AudioQueueProcessingTapRef inAQTap, UInt32 inNumberFrames,
		AudioTimeStamp* ioTimeStamp, AudioQueueProcessingTapFlags* ioFlags,
		UInt32* outNumberFrames, AudioBufferList* ioData);

typedef struct AudioQueueBuffer
{
	const UInt32 mAudioDataBytesCapacity;
	void* mAudioData;
	UInt32 mAudioDataByteSize;
	void* mUserData;
	
	UInt32 mPacketDescriptionCapacity;
	AudioStreamPacketDescription* mPacketDescriptions;
	UInt32 mPacketDescriptionCount;
} AudioQueueBuffer;
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

#ifdef __BLOCKS__
typedef void (^AudioQueueInputCallbackBlock)(AudioQueueRef inAQ,
		AudioQueueBufferRef inBuffer, const AudioTimeStamp* inStartTime,
		UInt32 inNumberPacketDescriptions,
		const AudioStreamPacketDescription* inPacketDescs);
OSStatus AudioQueueNewInputWithDispatchQueue(AudioQueueRef *outAQ,
		const AudioStreamBasicDescription *inFormat, UInt32 inFlags,
		dispatch_queue_t inCallbackDispatchQueue,
		AudioQueueInputCallbackBlock inCallbackBlock);
#endif

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

typedef struct OpaqueAudioQueueTimeline* AudioQueueTimelineRef;

OSStatus AudioQueueCreateTimeline(AudioQueueRef inAQ, AudioQueueTimelineRef *outTimeline);
OSStatus AudioQueueDisposeTimeline(AudioQueueRef inAQ, AudioQueueTimelineRef inTimeline);
OSStatus AudioQueueDeviceGetCurrentTime(AudioQueueRef inAQ, AudioTimeStamp *outTimeStamp);
OSStatus AudioQueueDeviceGetNearestStartTime(AudioQueueRef inAQ, AudioTimeStamp *ioRequestedStartTime, UInt32 inFlags);
OSStatus AudioQueueDeviceTranslateTime(AudioQueueRef inAQ, const AudioTimeStamp *inTime, AudioTimeStamp *outTime);
OSStatus AudioQueueGetCurrentTime(AudioQueueRef inAQ, AudioQueueTimelineRef inTimeline, AudioTimeStamp *outTimeStamp, Boolean *outTimelineDiscontinuity);

OSStatus AudioQueueProcessingTapNew(AudioQueueRef inAQ, AudioQueueProcessingTapCallback inCallback,
		void* inClientData, AudioQueueProcessingTapFlags inFlags, UInt32* outMaxFrames,
		AudioStreamBasicDescription* outProcessingFormat, AudioQueueProcessingTapRef* outAQTap);
OSStatus AudioQueueProcessingTapDispose(AudioQueueProcessingTapRef inAQTap);
OSStatus AudioQueueProcessingTapGetSourceAudio(AudioQueueProcessingTapRef inAQTap, UInt32 inNumberFrames,
		AudioTimeStamp* ioTimeStamp, AudioQueueProcessingTapFlags* outFlags, UInt32* outNumberFrames,
		AudioBufferList* ioData);
OSStatus AudioQueueProcessingTapGetQueueTime(AudioQueueProcessingTapRef inAQTap,
		Float64* outQueueSampleTime, UInt32* outQueueFrameCount);

#ifdef	__cplusplus
}
#endif

#endif	/* AUDIOBUFFER_H */

