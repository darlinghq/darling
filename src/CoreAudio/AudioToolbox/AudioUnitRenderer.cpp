#include "AudioUnitRenderer.h"
#include <CoreServices/MacErrors.h>

AudioUnitRenderer::AudioUnitRenderer(const AURenderCallbackStruct& cb)
: AudioUnitComponent({ CFSTR("Fake") }), m_renderCallback(cb)
{
}

AudioUnitRenderer::AudioUnitRenderer()
: AudioUnitComponent({ CFSTR("Fake") }), m_renderCallback({0})
{
}

OSStatus AudioUnitRenderer::init()
{
	return noErr;
}

OSStatus AudioUnitRenderer::deinit()
{
	return noErr;
}

OSStatus AudioUnitRenderer::reset(AudioUnitScope inScope, AudioUnitElement inElement)
{
	return noErr;
}

OSStatus AudioUnitRenderer::render(AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList *ioData)
{
	return m_renderCallback.inputProc(m_renderCallback.inputProcRefCon, ioActionFlags, inTimeStamp, inBusNumber, inNumberFrames, ioData);
}

