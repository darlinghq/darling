#ifndef AUDIOQUEUEOUTPUT_H
#define	AUDIOQUEUEOUTPUT_H
#include "AudioQueueBase.h"

class AudioQueueOutput : public AudioQueue
{
public:
	AudioQueueOutput(const AudioStreamBasicDescription *inFormat,
			AudioQueueOutputCallback inCallbackProc,
		void *inUserData, CFRunLoopRef inCallbackRunLoop,
		CFStringRef inCallbackRunLoopMode, UInt32 inFlags);
	
	virtual ~AudioQueueOutput();
	
	virtual OSStatus start(const AudioTimeStamp *inStartTime) override;
	virtual OSStatus prime(UInt32 inNumberOfFramesToPrepare, UInt32 *outNumberOfFramesPrepared) override;
	virtual OSStatus flush() override;
	virtual OSStatus stop(Boolean inImmediate) override;
	virtual OSStatus pause() override;
	virtual OSStatus reset() override;
	
	virtual OSStatus setOfflineRenderFormat(const AudioStreamBasicDescription *inFormat, const AudioChannelLayout *inLayout) override;
	virtual OSStatus offlineRender(const AudioTimeStamp *inTimestamp, AudioQueueBufferRef ioBuffer, UInt32 inNumberFrames) override;
	
	virtual OSStatus dispose(Boolean inImmediate) override;
	
	static OSStatus create(const AudioStreamBasicDescription *inFormat,
			AudioQueueOutputCallback inCallbackProc,
		void *inUserData, CFRunLoopRef inCallbackRunLoop,
		CFStringRef inCallbackRunLoopMode, UInt32 inFlags,
			AudioQueueOutput** newQueue);
private:
	AudioQueueOutputCallback m_callback;
};

#endif	/* AUDIOQUEUEOUTPUT_H */

