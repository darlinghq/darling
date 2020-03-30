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
#include <CoreServices/Components.h>
#include <CoreServices/Resources.h>
#include <CoreServices/MacErrors.h>
#include <CoreServices/MacMemory.h>
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
	if (!matching.empty())
	{
		if (!inComponent)
			return matching[0];
		
		for (size_t i = 0; i < matching.size()-1; i++)
		{
			if (matching[i] == inComponent)
			{
				return matching[i+1];
			}
		}
	}

	return (AudioComponent) ::FindNextComponent(Component(inComponent), (ComponentDescription*) desc);
}

UInt32 AudioComponentCount(const AudioComponentDescription* desc)
{
	const UInt32 carbonComponents = ::CountComponents((ComponentDescription*) desc);
	UInt32 numDuplicates = 0;
	Component carbonComponent = nullptr;

	while ((carbonComponent = ::FindNextComponent(carbonComponent, (ComponentDescription*) desc)) != nullptr)
	{
		ComponentDescription cd;

		::GetComponentInfo(carbonComponent, &cd, nullptr, nullptr, nullptr);

		if (::AudioComponentFindNext(nullptr, (AudioComponentDescription*) &cd) != nullptr)
			numDuplicates++;
	}
	
	return AudioComponentManager::instance()->findMatching(desc).size() + carbonComponents - numDuplicates;
}

OSStatus AudioComponentCopyName(AudioComponent component, CFStringRef* outName)
{
	if (AudioComponentManager::isOurInstance(component))
	{
		const char* name;
		OSStatus status = AudioComponentManager::instance()->getName(component, &name);

		if (status == noErr)
			*outName = CFStringCreateWithCString(nullptr, name, kCFStringEncodingUTF8);
		else
			*outName = nullptr;

		return status;
	}
	else
	{
		Handle handle = ::NewEmptyHandle();
		OSStatus status = ::GetComponentInfo(Component(component), nullptr, handle, nullptr, nullptr);

		if (status != noErr)
		{
			::DisposeHandle(handle);
			*outName = nullptr;
			return status;
		}
		else
		{
			*outName = ::CFStringCreateWithPascalString(nullptr, ConstStr255Param(*handle), kCFStringEncodingMacRoman);
			::DisposeHandle(handle);
			return noErr;
		}
	}
}

OSStatus AudioComponentGetDescription(AudioComponent component, AudioComponentDescription* outDesc)
{
	if (AudioComponentManager::isOurInstance(component))
	{
		return AudioComponentManager::instance()->getDescription(component, outDesc);
	}
	else
	{
		return ::GetComponentInfo(Component(component), (ComponentDescription*) outDesc, nullptr, nullptr, nullptr);
	}
}

#pragma pack(push, 1)
struct PlatformInfo
{
	uint32_t ComponentFlags;
	uint32_t CodeType;
	uint16_t CodeId;
	uint16_t PlatformType;
};

struct CarbonThng
{
    uint32_t Type;
    uint32_t Subtype;
    uint32_t Manufacturer;
    uint32_t ComponentFlags;
    uint32_t ComponentFlagsMask;
    uint32_t CodeType;
    uint16_t CodeId;
    uint32_t NameType;
    uint16_t NameId;
    uint32_t InfoType;
    uint16_t InfoId;
    uint32_t IconType;
    uint16_t IconId;
    uint32_t ComponentVersion;
    uint32_t RegistrationFlags;
    uint16_t IconFamilyId;

    uint32_t PlatformInfoCount;
    PlatformInfo PlatformInfos[];
};
#pragma pack(pop)
#ifdef __LITTLE_ENDIAN__
#	define SWAP(x) x = _bswap(x)

static inline uint16_t _bswap(uint16_t v) { return __builtin_bswap16(v); }
static inline int16_t _bswap(int16_t v) { return __builtin_bswap16(v); }
static inline uint32_t _bswap(uint32_t v) { return __builtin_bswap32(v); }

#else
#	define SWAP(x)
#endif

OSStatus AudioComponentGetVersion(AudioComponent component, UInt32* outVersion)
{
	if (AudioComponentManager::isOurInstance(component))
	{
		return AudioComponentManager::instance()->getVersion(component, (uint32_t*) outVersion);
	}
	else
	{
		ResFileRefNum resFile, oldFile;
		AudioComponentDescription desc;
		OSStatus status;
		
		status = ::AudioComponentGetDescription(component, &desc);
		if (status != noErr)
			return status;

		status = ::OpenAComponentResFile(Component(component), &resFile);
		if (status != noErr)
			return status;

		oldFile = ::CurResFile();
		::UseResFile(resFile);

		const ResourceCount count = ::Count1Resources('thng');
		bool found = false;

		for (int i = 1; i <= count; i++)
		{
			Handle handle = ::Get1IndResource('thng', i);
			if (handle && ::GetHandleSize(handle) >= sizeof(CarbonThng))
			{
				CarbonThng thng = *((CarbonThng*) *handle);

				SWAP(thng.Type);
				SWAP(thng.Subtype);
				SWAP(thng.Manufacturer);

				if (thng.Type == desc.componentType && thng.Subtype == desc.componentSubType && thng.Manufacturer == desc.componentManufacturer)
				{
					SWAP(thng.ComponentVersion);
					*outVersion = thng.ComponentVersion;
					found = true;
					break;
				}
			}
		}

		::UseResFile(oldFile);
		::CloseComponentResFile(resFile);

		return found ? noErr : invalidComponentID;
	}
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
	if (AudioComponentManager::isOurInstance(component))
	{
		return AudioComponentManager::instance()->instantiate(component, outInstance);
	}
	else
	{
		return ::OpenAComponent(Component(component), (ComponentInstance*) outInstance);
	}
}

void AudioComponentInstantiate(AudioComponent component, AudioComponentInstantiationOptions opts, void(^handler)(AudioComponentInstance, OSStatus))
{
	std::cerr << "AudioComponentInstantiate(): NOT IMPLEMENTED\n";
}

OSStatus AudioComponentInstanceDispose(AudioComponentInstance inst)
{
	if (AudioComponentManager::isOurInstance(inst))
	{
		return AudioComponentManager::instance()->dispose(inst);
	}
	else
	{
		return ::CloseComponent(inst);
	}
}

AudioComponent AudioComponentInstanceGetComponent(AudioComponentInstance inst)
{
	if (AudioComponentManager::isOurInstance(inst))
	{
		return AudioComponentManager::instance()->getComponent(inst);
	}
	else
	{
		return (AudioComponent) ::GetComponentIDFromComponentInstance(inst);
	}
}

Boolean AudioComponentInstanceCanDo(AudioComponentInstance inst, SInt16 sel)
{
	if (AudioComponentManager::isOurInstance(inst))
	{
		AudioComponentPlugInInterface* iface = AudioComponentManager::instance()->instanceInterface(inst);
		return iface->Lookup(sel) != nullptr;
	}
	else
	{
		return ::CallComponentCanDo(inst, sel);
	}
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
