#ifndef AUDIOUNITBASE_H
#define AUDIOUNITBASE_H

#include "AudioUnit.h"
#include "AudioUnitProperties.h"
#include <vector>
#include <set>
#include <mutex>
#include <memory>
#include <CoreFoundation/CFString.h>

#ifndef kOutputBus
#	define kOutputBus		0 // playback
#	define kInputBus		1 // recording
#endif

class AudioUnitRenderer;

class AudioUnitComponent
{
protected:
	AudioUnitComponent(std::initializer_list<CFStringRef> elements);
public:
    virtual ~AudioUnitComponent();

	virtual OSStatus init() = 0;
	virtual OSStatus deinit() = 0;
	
	virtual OSStatus reset(AudioUnitScope inScope, AudioUnitElement inElement) = 0;
	
	virtual OSStatus render(AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList *ioData) = 0;

	virtual OSStatus setProperty(AudioUnitPropertyID prop, AudioUnitScope scope, AudioUnitElement elem, const void* data, UInt32 dataSize);
	virtual OSStatus getProperty(AudioUnitPropertyID prop, AudioUnitScope scope, AudioUnitElement elem, void* data, UInt32* dataSize);
	virtual OSStatus getPropertyInfo(AudioUnitPropertyID prop, AudioUnitScope scope, AudioUnitElement elem, UInt32* dataSize, Boolean* writable);
	
	OSStatus addRenderNotify(AURenderCallback inProc, void* opaque);
	OSStatus removeRenderNotify(AURenderCallback inProc, void* opaque);
	OSStatus notifyListeners(AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList *ioData);
protected:
	std::vector<CFStringRef> m_elementNames;
	std::vector<std::pair<AudioStreamBasicDescription, AudioStreamBasicDescription>> m_config;
	//AudioStreamBasicDescription m_configOutputPlayback, m_configInputPlayback, m_configInputCapture, m_configOutputCapture;

	// We're simplifying things here - instead of having generic sets of input/output busses
	// and associated connections/callbacks, we have just this.
	// For kOutputBus only
	AudioUnitConnection m_inputUnit;
	std::unique_ptr<AudioUnitRenderer> m_inputRenderer;

	// For kInputBus only
	AURenderCallbackStruct m_outputCallback;

	bool m_shouldAllocateBuffer = true;
	OSStatus m_lastRenderError = 0;
	CFStringRef m_contextName = nullptr;
	std::mutex m_listenersMutex;
	std::set<std::pair<AURenderCallback, void*>> m_listeners;
};

#endif

