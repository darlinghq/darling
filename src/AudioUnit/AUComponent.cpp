#include "AUComponent.h"
#include "AUComponentInternal.h"
#include "log.h"
#include <alsa/asoundlib.h>
#include <CoreServices/MacErrors.h>

AudioComponent AudioComponentFindNext(AudioComponent inAComponent, AudioComponentDescription *inDesc)
{
	int index = -1;

	if (inDesc->componentType != kAudioUnitType_Output && inDesc->componentType != kAudioUnitType_Mixer)
		return nullptr;

	if (inAComponent != nullptr)
		index = GetComponentIndex(inAComponent);

	if (snd_card_next(&index) || index == -1)
		return nullptr;

	return CreateComponent(kComponentTypeAudioUnit, index);
}

OSStatus AudioComponentGetDescription(AudioComponent inComponent, AudioComponentDescription *outDesc)
{
	if (!outDesc)
		return paramErr;
	
	memset(outDesc, 0, sizeof(*outDesc));
	outDesc->componentType = kAudioUnitType_Output;
	
	return noErr;
}

OSStatus AudioComponentGetVersion(AudioComponent inComponent, UInt32 *outVersion)
{
	*outVersion = 0;
	return noErr;
}

Boolean AudioComponentInstanceCanDo(AudioComponentInstance inInstance, SInt16 inSelectorID)
{
	return true;
}

OSStatus AudioComponentInstanceDispose(AudioComponentInstance inInstance)
{
	delete inInstance;
	return noErr;
}

AudioComponent AudioComponentInstanceGetComponent(AudioComponentInstance inInstance)
{
	return CreateComponent(kComponentTypeAudioUnit, inInstance->cardIndex());
}

OSStatus AudioComponentInstanceNew(AudioComponent inComponent, AudioComponentInstance *outInstance)
{
	*outInstance = AudioUnitComponent::create(GetComponentIndex(inComponent));
	return *outInstance ? noErr : paramErr;
}

OSStatus AudioComponentCopyName(AudioComponent inComponent, CFStringRef *outName)
{
	int index = GetComponentIndex(inComponent);
	char* name;
	
	if (!outName)
		return paramErr;
	
	if (snd_card_get_longname(index, &name))
		return paramErr;
	
	*outName = CFStringCreateWithCString(nullptr, name, kCFStringEncodingUTF8);
	free(name);
	
	return noErr;
}

UInt32 AudioComponentCount(AudioComponentDescription *inDesc)
{
	UInt32 count = 0;
	int index = -1;
	
	// Is there a better way?
	do
	{
		snd_card_next(&index);
		if (index != -1)
			count++;
	}
	while (index != -1);
	
	return count;
}

//////////////////

AudioUnitComponent::AudioUnitComponent(int cardIndex, char* cardName)
: m_cardIndex(cardIndex), m_cardName(cardName)
{
}

AudioUnitComponent* AudioUnitComponent::create(int cardIndex)
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
	
	return new AudioUnitComponent(cardIndex, name);
}

AudioUnitComponent::~AudioUnitComponent()
{
	free(m_cardName);
}
