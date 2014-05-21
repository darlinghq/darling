#include "AudioOutputUnitComponent.h"
#include <CoreServices/MacErrors.h>
#include <cstring>

AudioOutputUnitComponent::AudioOutputUnitComponent()
: AudioUnitComponent(2)
{
	memset(&m_outputCallback, 0, sizeof(m_outputCallback));
}

OSStatus AudioOutputUnitComponent::setProperty(AudioUnitPropertyID prop, AudioUnitScope scope, AudioUnitElement elem, const void* data, UInt32 dataSize)
{
	switch (prop)
	{
		case kAudioOutputUnitProperty_EnableIO:
		{
			const UInt32* state;

			if (dataSize != sizeof(UInt32))
				return kAudioUnitErr_InvalidParameter;

			state = static_cast<const UInt32*>(data);
			
			if (scope == kAudioUnitScope_Output)
			{
				if (elem != 0)
					return kAudioUnitErr_InvalidElement;
				m_enableOutput = *state != 0;
			}
			else if (scope == kAudioUnitScope_Input)
			{
				if (elem != 1)
					return kAudioUnitErr_InvalidElement;
				m_enableInput = *state != 0;
			}
			else
				return kAudioUnitErr_InvalidScope;

			return noErr;
		}
		case kAudioOutputUnitProperty_SetInputCallback:
		{
			if (dataSize != sizeof(AURenderCallbackStruct))
				return kAudioUnitErr_InvalidParameter;
			//if (scope == kAudioUnitScope_Output)
			//{
				if (elem != 1)
					return kAudioUnitErr_InvalidElement;
				
				memcpy(&m_outputCallback, data, sizeof(AURenderCallbackStruct));
				
				return noErr;
			//}
		}
		default:
			return AudioUnitComponent::setProperty(prop, scope, elem, data, dataSize);
	}
}

OSStatus AudioOutputUnitComponent::getProperty(AudioUnitPropertyID prop, AudioUnitScope scope, AudioUnitElement elem, void* data, UInt32* dataSize)
{
	switch (prop)
	{
		case kAudioOutputUnitProperty_EnableIO:
		{
			UInt32* state;

			if (*dataSize < sizeof(UInt32))
				return kAudioUnitErr_InvalidParameter;

			state = static_cast<UInt32*>(data);

			if (scope == kAudioUnitScope_Output)
			{
				if (elem != 0)
					return kAudioUnitErr_InvalidElement;
				*state = m_enableOutput;
			}
			else if (scope == kAudioUnitScope_Input)
			{
				if (elem != 1)
					return kAudioUnitErr_InvalidElement;
				*state = m_enableInput;
			}
			else
				return kAudioUnitErr_InvalidScope;

			*dataSize = sizeof(UInt32);
			return noErr;
		}
		default:
			return AudioUnitComponent::getProperty(prop, scope, elem, data, dataSize);
	}
}

OSStatus AudioOutputUnitComponent::getPropertyInfo(AudioUnitPropertyID prop, AudioUnitScope scope, AudioUnitElement elem, UInt32* dataSize, Boolean* writable)
{
	switch (prop)
	{
		default:
			return AudioUnitComponent::getPropertyInfo(prop, scope, elem, dataSize, writable);
	}
}

