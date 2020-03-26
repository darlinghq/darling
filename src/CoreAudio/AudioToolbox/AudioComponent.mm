/*
This file is part of Darling.

Copyright (C) 2020 Lubos Dolezel

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <AudioToolbox/AudioComponent.h>
#include "AudioComponentManager.h"
#include <iostream>

AudioComponent AudioComponentRegister(const AudioComponentDescription *inDesc, CFStringRef inName,
	UInt32 inVersion, AudioComponentFactoryFunction inFactory)
{
	if (!inDesc || !inName || !inFactory)
		return nullptr;

	const char* name = CFStringGetCStringPtr(inName, kCFStringEncodingUTF8);
	return AudioComponentManager::instance()->registerComponent(inDesc, name, inVersion, inFactory);
}

AudioComponent AudioComponentFindNext(AudioComponent inComponent, const AudioComponentDescription* desc)
{
	auto matching = AudioComponentManager::instance()->findMatching(desc);
	if (matching.empty())
		return nullptr;

	if (!inComponent)
		return matching[0];
	
	for (size_t i = 0; i < matching.size()-1; i++)
	{
		if (matching[i] == inComponent)
		{
			return matching[i+1];
		}
	}

	return nullptr;
}

UInt32 AudioComponentCount(const AudioComponentDescription* desc)
{
	return AudioComponentManager::instance()->findMatching(desc).size();
}

OSStatus AudioComponentCopyName(AudioComponent component, CFStringRef* outName)
{
	const char* name;
	OSStatus status = AudioComponentManager::instance()->getName(component, &name);

	if (status == noErr)
		*outName = CFStringCreateWithCString(nullptr, name, kCFStringEncodingUTF8);
	else
		*outName = nullptr;

	return status;
}

OSStatus AudioComponentGetDescription(AudioComponent component, AudioComponentDescription* outDesc)
{
	return AudioComponentManager::instance()->getDescription(component, outDesc);
}

OSStatus AudioComponentGetVersion(AudioComponent component, UInt32* outVersion)
{
	return AudioComponentManager::instance()->getVersion(component, (uint32_t*) outVersion);
}

#if !TARGET_OS_IPHONE

NSImage* AudioComponentGetIcon(AudioComponent component)
{
	return nullptr;
}

#else

UIImage* AudioComponentGetIcon(AudioComponent component, float desiredPointSize)
{
	return nullptr;
}

#endif

OSStatus AudioComponentInstanceNew(AudioComponent component, AudioComponentInstance* outInstance)
{
	return AudioComponentManager::instance()->instantiate(component, outInstance);
}

void AudioComponentInstantiate(AudioComponent component, AudioComponentInstantiationOptions opts, void(^handler)(AudioComponentInstance, OSStatus))
{
	std::cerr << "AudioComponentInstantiate(): NOT IMPLEMENTED\n";
}

OSStatus AudioComponentInstanceDispose(AudioComponentInstance inst)
{
	return AudioComponentManager::instance()->dispose(inst);
}

AudioComponent AudioComponentInstanceGetComponent(AudioComponentInstance inst)
{
	return AudioComponentManager::instance()->getComponent(inst);
}

Boolean AudioComponentInstanceCanDo(AudioComponentInstance inst, SInt16 sel)
{
	AudioComponentPlugInInterface* iface = AudioComponentManager::instance()->instanceInterface(inst);
	return iface->Lookup(sel) != nullptr;
}

OSStatus AudioComponentCopyConfigurationInfo(AudioComponent component, CFDictionaryRef* outInfo)
{
	std::cerr << "AudioComponentCopyConfigurationInfo(): NOT IMPLEMENTED\n";
	*outInfo = CFDictionaryCreate(nullptr, nullptr, nullptr, 0, nullptr, nullptr);
	return noErr;
}

OSStatus AudioComponentValidate(AudioComponent component, CFDictionaryRef validationParams, AudioComponentValidationResult* result)
{
	*result = kAudioComponentValidationResult_Passed;
	return noErr;
}
