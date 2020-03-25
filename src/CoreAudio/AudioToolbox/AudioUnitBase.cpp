#include "AudioUnitBase.h"
#include "AudioUnitProperties.h"
#include "AudioUnitRenderer.h"
#include <CoreServices/MacErrors.h>
#include "stub.h"
#include <cstring>

AudioUnitComponent::AudioUnitComponent(std::initializer_list<CFStringRef> elements)
{
	m_inputRenderer.reset(new AudioUnitRenderer);
	m_elementNames.insert(m_elementNames.end(), elements.begin(), elements.end());
	m_config.resize(m_elementNames.size());
	
	// Default audio params
	const AudioStreamBasicDescription defaultConfig = AudioStreamBasicDescription {
		44100.0, kAudioFormatLinearPCM, kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked,
		4, 1, 4, 2, 16, 0
	};
	
	for (size_t i = 0; i < m_config.size(); i++)
	{
		m_config[i] = std::pair<AudioStreamBasicDescription, AudioStreamBasicDescription>(defaultConfig, defaultConfig);
	}
	
	memset(&m_inputUnit, 0, sizeof(m_inputUnit));
}

AudioUnitComponent::~AudioUnitComponent()
{
	if (m_contextName != nullptr)
		CFRelease(m_contextName);
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
		case kAudioUnitProperty_ElementCount:
			*dataSize = sizeof(UInt32);
			*writable = false;
			break;
		case kAudioUnitProperty_LastRenderError: // TODO: implement
			*dataSize = sizeof(OSStatus);
			*writable = false;
			break;
		case kAudioUnitProperty_SampleRate: // TODO: implement
			*dataSize = sizeof(Float64);
			*writable = true; // TODO: query itself on kAudioUnitProperty_StreamFormat if writable
			break;
		case kAudioUnitProperty_ElementName:
			*dataSize = sizeof(CFStringRef);
			*writable = false;
			break;
		case kAudioUnitProperty_ContextName:
			*dataSize = sizeof(CFStringRef);
			*writable = true;
			break;
		default:
			return kAudioUnitErr_InvalidProperty;
	}

	return noErr;
}

OSStatus AudioUnitComponent::setProperty(AudioUnitPropertyID prop, AudioUnitScope scope, AudioUnitElement elem, const void* data, UInt32 dataSize)
{
	// TRACE5(prop, scope, elem, data, dataSize);

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

			if (scope == kAudioUnitScope_Output) // playback
				m_config[elem].second = *newConfig;
			else if (scope == kAudioUnitScope_Input) // recording
				m_config[elem].first = *newConfig;
			else if (scope == kAudioUnitScope_Global)
				m_config[0].second = *newConfig;
			else
				return kAudioUnitErr_InvalidScope;

			return noErr;
		}
		case 7: // kAudioUnitProperty_SetInputCallback (do not mistake with kAudioOutputUnitProperty_SetInputCallback)
		case kAudioUnitProperty_SetRenderCallback:
		{
			if (dataSize != sizeof(AURenderCallbackStruct))
				return kAudioUnitErr_InvalidParameter;
			//if (scope == kAudioUnitScope_Input)
			//{
				if (elem != kOutputBus)
					return kAudioUnitErr_InvalidElement;
				
				*m_inputRenderer = *(AURenderCallbackStruct*) data;
				m_inputUnit.sourceOutputNumber = 0;
				m_inputUnit.destInputNumber = 0;
				m_inputUnit.sourceAudioUnit = m_inputRenderer.get();
			//}
			//else
			//	return kAudioUnitErr_InvalidScope;
			
			return noErr;
		}
		case kAudioOutputUnitProperty_SetInputCallback:
		{
			// For audio recording

			if (elem != kInputBus)
				return kAudioUnitErr_InvalidElement;

			// Notification callback - the recipient needs to call AudioUnitRender to obtain microphone data
			m_outputCallback = *(AURenderCallbackStruct*) data;
			return noErr;
		}
		case kAudioUnitProperty_MakeConnection:
		{
			if (dataSize != sizeof(AudioUnitConnection))
				return kAudioUnitErr_InvalidParameter;
			if (scope != kAudioUnitScope_Input)
				return kAudioUnitErr_InvalidScope;
			
			// TODO: support multiple units!
			// TODO: reconfigure input format based on output format
			
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
		case kAudioUnitProperty_ContextName:
		{
			if (dataSize < sizeof(CFStringRef))
				return kAudioUnitErr_InvalidParameter;
			
			if (data == nullptr)
			{
				if (m_contextName != nullptr)
				{
					CFRelease(m_contextName);
					m_contextName = nullptr;
				}
			}
			else
			{
				m_contextName = (CFStringRef) data;
				CFRetain(m_contextName);
			}
			return noErr;
		}
		default:
			return kAudioUnitErr_InvalidProperty;
	}
}

OSStatus AudioUnitComponent::getProperty(AudioUnitPropertyID prop, AudioUnitScope scope, AudioUnitElement elem, void* data, UInt32* dataSize)
{
	// TRACE5(prop, scope, elem, data, dataSize);

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
			else if (scope == kAudioUnitScope_Global)
				*newConfig = m_config[0].second;
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
		case kAudioUnitProperty_ElementCount:
		{
			UInt32* out = (UInt32*) data;
			if (*dataSize < sizeof(UInt32))
				return kAudioUnitErr_InvalidParameter;
			
			*out = m_config.size();
			return noErr;
		}
		case kAudioUnitProperty_ElementName:
		{
			CFStringRef* out = (CFStringRef*) data;
			if (*dataSize != sizeof(CFStringRef))
				return kAudioUnitErr_InvalidParameter;
			if (elem >= m_elementNames.size())
				return kAudioUnitErr_InvalidElement;
			
			*out = m_elementNames[elem];
			return noErr;
		}
		case kAudioUnitProperty_ContextName:
		{
			CFStringRef* out = (CFStringRef*) data;
			if (*dataSize != sizeof(CFStringRef))
				return kAudioUnitErr_InvalidParameter;
			
			*out = m_contextName;
			return noErr;
		}
		default:
			return kAudioUnitErr_InvalidProperty;
	}
}

OSStatus AudioUnitComponent::addRenderNotify(AURenderCallback inProc, void* opaque)
{
	std::lock_guard<std::mutex> guard(m_listenersMutex);
	
	if (!inProc)
		return paramErr;
	m_listeners.insert(std::pair<AURenderCallback,void*>(inProc, opaque));
	
	return noErr;
}

OSStatus AudioUnitComponent::removeRenderNotify(AURenderCallback inProc, void* opaque)
{
	std::lock_guard<std::mutex> guard(m_listenersMutex);
	m_listeners.erase(std::pair<AURenderCallback,void*>(inProc, opaque));
	return noErr;
}

OSStatus AudioUnitComponent::notifyListeners(AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp,
		UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList *ioData)
{
	std::lock_guard<std::mutex> guard(m_listenersMutex);
	OSStatus status = noErr;
	
	for (auto p : m_listeners)
	{
		status = p.first(p.second, ioActionFlags, inTimeStamp, inBusNumber, inNumberFrames, ioData);
		if (status != noErr)
			break;
	}
	
	return status;
}
