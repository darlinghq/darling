#include "AudioQueueOutput.h"
#include <util/debug.h>
#include <CoreServices/MacErrors.h>

AudioQueueOutput::AudioQueueOutput(const AudioStreamBasicDescription *inFormat,
		AudioQueueOutputCallback inCallbackProc,
	void *inUserData, CFRunLoopRef inCallbackRunLoop,
	CFStringRef inCallbackRunLoopMode, UInt32 inFlags)
: AudioQueue(inFormat, inUserData, inCallbackRunLoop, inCallbackRunLoopMode, inFlags),
		m_callback(inCallbackProc)
{
	
}

AudioQueueOutput::~AudioQueueOutput()
{
	
}

OSStatus AudioQueueOutput::start(const AudioTimeStamp *inStartTime)
{
	STUB();
	return unimpErr;
}

OSStatus AudioQueueOutput::prime(UInt32 inNumberOfFramesToPrepare, UInt32 *outNumberOfFramesPrepared)
{
	STUB();
	return unimpErr;
}

OSStatus AudioQueueOutput::flush()
{
	STUB();
	return unimpErr;
}

OSStatus AudioQueueOutput::stop(Boolean inImmediate)
{
	STUB();
	return unimpErr;
}

OSStatus AudioQueueOutput::pause()
{
	STUB();
	return unimpErr;
}

OSStatus AudioQueueOutput::reset()
{
	STUB();
	return unimpErr;
}

OSStatus AudioQueueOutput::dispose(Boolean inImmediate)
{
	if (inImmediate)
	{
		delete this;
		return noErr;
	}
	else
	{
		STUB();
		return unimpErr;
	}
}

OSStatus AudioQueueOutput::setOfflineRenderFormat(const AudioStreamBasicDescription *inFormat, const AudioChannelLayout *inLayout)
{
	STUB();
	return unimpErr;
}

OSStatus AudioQueueOutput::offlineRender(const AudioTimeStamp *inTimestamp, AudioQueueBufferRef ioBuffer, UInt32 inNumberFrames)
{
	STUB();
	return unimpErr;
}

OSStatus AudioQueueOutput::create(const AudioStreamBasicDescription *inFormat,
		AudioQueueOutputCallback inCallbackProc,
		void *inUserData, CFRunLoopRef inCallbackRunLoop,
		CFStringRef inCallbackRunLoopMode, UInt32 inFlags,
			AudioQueueOutput** newQueue)
{
	// TODO: check arguments
	*newQueue = new AudioQueueOutput(inFormat, inCallbackProc, inUserData,
			inCallbackRunLoop, inCallbackRunLoopMode,
			inFlags);
	return noErr;
}
