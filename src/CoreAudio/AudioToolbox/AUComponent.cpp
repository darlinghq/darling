#include "darling-config.h"
#include "AUComponent.h"
#include "AudioUnitCA.h"
#include <alsa/asoundlib.h>
#include <CoreServices/MacErrors.h>
#include "stub.h"

AudioComponent AudioComponentFindNext(AudioComponent inAComponent, AudioComponentDescription *inDesc)
{
	// TODO
	return nullptr;
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
	// return CreateComponent(kComponentTypeAudioUnit, static_cast<AudioOutputUnitComponent*>(inInstance)->cardIndex());
	return nullptr;
}

OSStatus AudioComponentInstanceNew(AudioComponent inComponent, AudioComponentInstance *outInstance)
{
	// TODO: pass inComponent to specify which audio device to use?
	//*outInstance = new AudioUnitCA;
	//return *outInstance ? noErr : paramErr;
	return noErr;
}

OSStatus AudioComponentCopyName(AudioComponent inComponent, CFStringRef *outName)
{
	// TODO
	
	return noErr;
}

UInt32 AudioComponentCount(AudioComponentDescription *inDesc)
{
	UInt32 count = 0;

	count = 1;
	
	return count;
}

