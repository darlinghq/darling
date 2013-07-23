#include "AudioUnitALSA.h"
#include "AudioUnitProperties.h"
#include <CoreServices/MacErrors.h>
#include <util/debug.h>


AudioUnitALSA::AudioUnitALSA(int cardIndex, char* cardName)
: m_cardIndex(cardIndex), m_cardName(cardName), m_pcmOutput(nullptr), m_pcmInput(nullptr)
{
}

AudioUnitComponent* AudioUnitALSA::create(int cardIndex)
{
	char* name;
	
	if (cardIndex > 0)
	{
		if (snd_card_get_name(cardIndex, &name))
			return nullptr;
	}
	else
	{
		name = strdup("default");
	}
	
	return new AudioUnitALSA(cardIndex, name);
}

AudioUnitALSA::~AudioUnitALSA()
{
	deinit();
	free(m_cardName);
}

OSStatus AudioUnitALSA::init()
{
	if (m_pcmOutput || m_pcmInput)
		return kAudioUnitErr_Initialized;

	if (m_enableOutput && snd_pcm_open(&m_pcmOutput, m_cardName, SND_PCM_STREAM_PLAYBACK, 0))
	{
		LOG << "Failed to initialize playback PCM " << m_cardName;
		return kAudioUnitErr_FailedInitialization;
	}
	if (m_enableInput && snd_pcm_open(&m_pcmInput, m_cardName, SND_PCM_STREAM_CAPTURE, 0))
	{
		deinit();

		LOG << "Failed to initialize capture PCM " << m_cardName;
		return kAudioUnitErr_FailedInitialization;
	}
	return noErr;
}

OSStatus AudioUnitALSA::deinit()
{
	if (m_pcmOutput)
	{
		snd_pcm_close(m_pcmOutput);
		m_pcmOutput = nullptr;
	}
	if (m_pcmInput)
	{
		snd_pcm_close(m_pcmInput);
		m_pcmInput = nullptr;
	}
	return noErr;
}


OSStatus AudioUnitALSA::start()
{
	if (m_pcmInput)
	{
		if (snd_pcm_start(m_pcmInput))
		{
			return kAudioUnitErr_FailedInitialization;
		}
	}

	if (m_pcmOutput)
	{
		if (snd_pcm_start(m_pcmOutput))
		{
			if (m_pcmInput)
				snd_pcm_drop(m_pcmInput);

			return kAudioUnitErr_FailedInitialization;
		}
	}

	return noErr;
}

OSStatus AudioUnitALSA::stop()
{
	if (m_pcmInput)
		snd_pcm_drop(m_pcmInput);
	if (m_pcmOutput)
		snd_pcm_drop(m_pcmOutput);
	return noErr;
}

