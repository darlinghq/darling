#ifndef AUCOMPONENTINTERNAL_H
#define AUCOMPONENTINTERNAL_H
#include "AudioUnit.h"
#include <CoreServices/ComponentsInternal.h>
#include <asoundlib.h>
#include <dispatch/dispatch.h>
#include <vector>
#include "AudioOutputUnitComponent.h"

#ifndef kOutputBus
#	define kOutputBus		0 // playback
#	define kInputBus		1 // recording
#endif

class AudioUnitALSA : public AudioOutputUnitComponent
{
private:
	AudioUnitALSA(int cardIndex, char* cardName);
public:
	static AudioUnitComponent* create(int cardIndex);
	virtual ~AudioUnitALSA();
	
	int cardIndex() const override { return m_cardIndex; }

	OSStatus init() override;
	OSStatus deinit() override;

	OSStatus start() override;
	OSStatus stop() override;
	
	OSStatus reset(AudioUnitScope inScope, AudioUnitElement inElement) override;
	
	OSStatus render(AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList *ioData) override;
private:
	void initOutput();
	void initInput();
	
	void startOutput();
	void startInput();
	void startDescriptors(const struct pollfd* fds, int count);
	
	void processAudioEvent(struct pollfd origPoll, int event);
	void requestDataForPlayback();
	void pushDataFromInput();
	
	OSStatus renderOutput(AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inNumberFrames, AudioBufferList *ioData);
	OSStatus renderInterleavedOutput(AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inNumberFrames, AudioBufferList *ioData);
	OSStatus renderPlanarOutput(AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inNumberFrames, AudioBufferList *ioData);
	
	OSStatus renderInput(AudioUnitRenderActionFlags *ioActionFlags,const AudioTimeStamp *inTimeStamp, UInt32 inNumberFrames, AudioBufferList *ioData);
	OSStatus renderInterleavedInput(AudioUnitRenderActionFlags *ioActionFlags,const AudioTimeStamp *inTimeStamp, UInt32 inNumberFrames, AudioBufferList *ioData);
	OSStatus renderPlanarInput(AudioUnitRenderActionFlags *ioActionFlags,const AudioTimeStamp *inTimeStamp, UInt32 inNumberFrames, AudioBufferList *ioData);
	
	inline bool isOutputPlanar() const { return m_config[kOutputBus].second.mFormatFlags & kAudioFormatFlagIsNonInterleaved; }
	inline bool isInputPlanar() const { return m_config[kInputBus].first.mFormatFlags & kAudioFormatFlagIsNonInterleaved; }
private:
	int m_cardIndex;
	char* m_cardName;
	snd_pcm_t *m_pcmOutput, *m_pcmInput;
	std::vector<dispatch_source_t> m_sources;
};

#endif

