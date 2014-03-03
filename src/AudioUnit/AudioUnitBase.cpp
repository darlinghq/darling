#include "AudioUnitBase.h"
#include "AudioUnitProperties.h"
#include "AudioUnitRenderer.h"
#include <CoreServices/MacErrors.h>
#include <util/debug.h>
#include <cstring>

AudioUnitComponent::AudioUnitComponent(size_t numElements)
{
	m_config.resize(numElements);
	
	// Default audio params
	const AudioStreamBasicDescription defaultConfig = AudioStreamBasicDescription {
		44100.0, kAudioFormatLinearPCM, kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked,
		4, 1, 4, 2, 16, 0
	};
	
	for (size_t i = 0; i < numElements; i++)
	{
		m_config[i] = std::pair<AudioStreamBasicDescription, AudioStreamBasicDescription>(defaultConfig, defaultConfig);
	}
	
	memset(&m_inputUnit, 0, sizeof(m_inputUnit));
}

AudioUnitComponent::~AudioUnitComponent()
{
	CloseComponent(m_inputUnit.sourceAudioUnit);
}

OSStatus AudioUnitComponent::getPropertyInfo(AudioUnitPropertyID prop, AudioUnitScope scope, AudioUnitElement elem, UInt32* dataSize, Boolean* writable)
{
	switch (prop)
	{
		case kAudioUnitProperty_StreamFormat:
			*dataSize = sizeof(AudioStreamBasicDescription);
			*writable = true;
			break;
		case kAudioOutputUnitProperty_EnableIO:
			*dataSize = sizeof(UInt32);
			*writable = true;
			break;
		case kAudioUnitProperty_SetRenderCallback:
			*dataSize = sizeof(AURenderCallbackStruct);
			*writable = true;
			break;
		case kAudioUnitProperty_ShouldAllocateBuffer:
			*dataSize = sizeof(int);
			*writable = true;
			break;
		default:
			return kAudioUnitErr_InvalidProperty;
	}

	return noErr;
}

OSStatus AudioUnitComponent::setProperty(AudioUnitPropertyID prop, AudioUnitScope scope, AudioUnitElement elem, const void* data, UInt32 dataSize)
{
	TRACE5(prop, scope, elem, data, dataSize);

	switch (prop)
	{
		case kAudioUnitProperty_StreamFormat:
		{
			const AudioStreamBasicDescription* newConfig = static_cast<const AudioStreamBasicDescription*>(data);
			
			if (dataSize != sizeof(*newConfig))
				return kAudioUnitErr_InvalidParameter;
			
			if (elem >= m_config.size())
				return kAudioUnitErr_InvalidElement;

			// TODO: perform validation

			if (scope == kAudioUnitScope_Output)
				m_config[elem].second = *newConfig;
			else if (scope == kAudioUnitScope_Input)
				m_config[elem].first = *newConfig;
			else
				return kAudioUnitErr_InvalidScope;

			return noErr;
		}
		case kAudioUnitProperty_SetRenderCallback:
		{
			if (dataSize != sizeof(AURenderCallbackStruct))
				return kAudioUnitErr_InvalidParameter;
			//if (scope == kAudioUnitScope_Input)
			//{
				if (elem != 0)
					return kAudioUnitErr_InvalidElement;
				
				CloseComponent(m_inputUnit.sourceAudioUnit); // TODO: wrong, we may not own the unit!
				m_inputUnit.sourceOutputNumber = 0;
				m_inputUnit.destInputNumber = 0;
				m_inputUnit.sourceAudioUnit = new AudioUnitRenderer(*(AURenderCallbackStruct*) data);
			//}
			//else
			//	return kAudioUnitErr_InvalidScope;
			
			return noErr;
		}
		case kAudioUnitProperty_MakeConnection:
		{
			if (dataSize != sizeof(AudioUnitConnection))
				return kAudioUnitErr_InvalidParameter;
			if (scope != kAudioUnitScope_Input)
				return kAudioUnitErr_InvalidScope;
			
			CloseComponent(m_inputUnit.sourceAudioUnit); // TODO: wrong, we may not own the unit!
			memcpy(&m_inputUnit, data, sizeof(AudioUnitConnection));
			
			return noErr;
		}
		case kAudioUnitProperty_ShouldAllocateBuffer:
		{
			int* b = (int*) data;
			if (dataSize < sizeof(int))
				return kAudioUnitErr_InvalidParameter;
			
			m_shouldAllocateBuffer = *b != 0;
			return noErr;
		}
		default:
			return kAudioUnitErr_InvalidProperty;
	}
}

OSStatus AudioUnitComponent::getProperty(AudioUnitPropertyID prop, AudioUnitScope scope, AudioUnitElement elem, void* data, UInt32* dataSize)
{
	TRACE5(prop, scope, elem, data, dataSize);

	switch (prop)
	{
		case kAudioUnitProperty_StreamFormat:
		{
			AudioStreamBasicDescription* newConfig = static_cast<AudioStreamBasicDescription*>(data);
			
			if (*dataSize < sizeof(AudioStreamBasicDescription))
				return kAudioUnitErr_InvalidParameter;
			
			if (elem >= m_config.size())
				return kAudioUnitErr_InvalidElement;

			if (scope == kAudioUnitScope_Output)
				*newConfig = m_config[elem].second;
			else if (scope == kAudioUnitScope_Input)
				*newConfig = m_config[elem].first;
			else
				return kAudioUnitErr_InvalidScope;

			*dataSize = sizeof(AudioStreamBasicDescription);

			return noErr;
		}
		case kAudioUnitProperty_ShouldAllocateBuffer:
		{
			int* out = (int*) data;
			if (*dataSize < sizeof(UInt32))
				return kAudioUnitErr_InvalidParameter;
			
			*out = m_shouldAllocateBuffer;
			return noErr;
		}
		default:
			return kAudioUnitErr_InvalidProperty;
	}
}
