#include "AudioUnitBase.h"
#include "AudioUnitProperties.h"
#include <CoreServices/MacErrors.h>
#include <util/debug.h>
#include <cstring>

AudioUnitComponent::AudioUnitComponent()
{
	// Default audio params
	m_configOutputPlayback = AudioStreamBasicDescription {
		44100.0, kAudioFormatLinearPCM, kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked,
		4, 1, 4, 2, 16, 0
	};
	memcpy(&m_configInputPlayback, &m_configOutputPlayback, sizeof(AudioStreamBasicDescription));
	memcpy(&m_configInputCapture, &m_configOutputPlayback, sizeof(AudioStreamBasicDescription));
	memcpy(&m_configOutputCapture, &m_configOutputPlayback, sizeof(AudioStreamBasicDescription));

	memset(&m_renderCallback, 0, sizeof(m_renderCallback));
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
			if (dataSize != sizeof(AudioStreamBasicDescription))
				return kAudioUnitErr_InvalidParameter;

			// TODO: perform validation

			if (scope == kAudioUnitScope_Output)
			{
				if (elem == 0)
					memcpy(&m_configOutputPlayback, data, dataSize);
				else if (elem == 1)
					memcpy(&m_configOutputCapture, data, dataSize);
				else
					return kAudioUnitErr_InvalidElement;
			}
			else if (scope == kAudioUnitScope_Input)
			{
				if (elem == 0)
					memcpy(&m_configInputPlayback, data, dataSize);
				else if (elem == 1)
					memcpy(&m_configInputCapture, data, dataSize);
				else
					return kAudioUnitErr_InvalidElement;
			}
			else
				return kAudioUnitErr_InvalidScope;

			return noErr;
		}
		case kAudioUnitProperty_SetRenderCallback:
		{
			if (dataSize != sizeof(AURenderCallbackStruct))
				return kAudioUnitErr_InvalidParameter;
			if (scope != kAudioUnitScope_Input)
				return kAudioUnitErr_InvalidScope;
			if (elem != 0)
				return kAudioUnitErr_InvalidElement;
			
			memcpy(&m_renderCallback, data, dataSize);
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
			if (*dataSize < sizeof(AudioStreamBasicDescription))
				return kAudioUnitErr_InvalidParameter;

			if (scope == kAudioUnitScope_Output)
			{
				if (elem == 0)
					memcpy(data, &m_configOutputPlayback, *dataSize);
				else if (elem == 1)
					memcpy(data, &m_configOutputCapture, *dataSize);
				else
					return kAudioUnitErr_InvalidElement;
			}
			else if (scope == kAudioUnitScope_Input)
			{
				if (elem == 0)
					memcpy(data, &m_configInputPlayback, *dataSize);
				else if (elem == 1)
					memcpy(data, &m_configInputCapture, *dataSize);
				else
					return kAudioUnitErr_InvalidElement;
			}
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
