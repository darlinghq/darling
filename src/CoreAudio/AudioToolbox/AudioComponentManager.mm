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
#include "AudioComponentManager.h"
#include <cstdlib>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/MacErrors.h>

AudioComponentManager::AudioComponentManager()
{
	discoverComponents();
}

void AudioComponentManager::discoverComponents()
{
	discoverComponents("/System/Library/Components");

	const char* home = ::getenv("HOME");
	if (home != NULL)
	{
		std::string userPath = home;
		userPath += "/Library/Audio/Plug-Ins/Components";

		discoverComponents(userPath.c_str());
	}
}

void AudioComponentManager::discoverComponents(const char* dir)
{
	CFArrayRef componentBundles;
	CFURLRef urlDir;
	CFStringRef strDir;

	strDir = CFStringCreateWithCStringNoCopy(nullptr, dir, kCFStringEncodingUTF8, kCFAllocatorNull);
	urlDir = CFURLCreateWithFileSystemPath(nullptr, strDir, kCFURLPOSIXPathStyle, true);
	CFRelease(strDir);

	componentBundles = CFBundleCreateBundlesFromDirectory(nullptr, urlDir, CFSTR("component"));

	CFRelease(urlDir);

	for (CFIndex i = 0; i < CFArrayGetCount(componentBundles); i++)
		analyzeComponent((CFBundleRef) CFArrayGetValueAtIndex(componentBundles, i));

	CFRelease(componentBundles);
}

static bool isString(CFStringRef str)
{
	if (!str)
		return false;
	if (CFGetTypeID(str) != CFStringGetTypeID())
		return false;
	return true;
}

static UInt32 stringToFourCC(CFStringRef str)
{
	union {
		char chars[5]; // CFStringGetCString requires space for the NUL character
		uint32_t code;
	} cc;

	if (CFStringGetLength(str) != 4)
		return 0;
	if (!CFStringGetCString(str, cc.chars, 5, kCFStringEncodingUTF8))
		return 0;

#if __LITTLE_ENDIAN__
	cc.code = __builtin_bswap32(cc.code);
#endif

	return cc.code;
}

void AudioComponentManager::analyzeComponent(CFBundleRef bundle)
{
	CFDictionaryRef dict = CFBundleGetInfoDictionary(bundle);
	CFArrayRef components = (CFArrayRef) CFDictionaryGetValue(dict, CFSTR("AudioComponents"));

	if (!components || CFGetTypeID(components) != CFArrayGetTypeID())
		return;

	for (CFIndex i = 0; i < CFArrayGetCount(components); i++)
	{
		CFDictionaryRef props = (CFDictionaryRef) CFArrayGetValueAtIndex(components, i);
		if (!props || CFGetTypeID(props) != CFDictionaryGetTypeID())
			continue;

		CFStringRef name, manufacturer, type, subtype, factoryFunction;
		CFNumberRef version;
		CFBooleanRef sandboxSafe;

		name = (CFStringRef) CFDictionaryGetValue(props, CFSTR("name"));
		manufacturer = (CFStringRef) CFDictionaryGetValue(props, CFSTR("manufacturer"));
		type = (CFStringRef) CFDictionaryGetValue(props, CFSTR("type"));
		subtype = (CFStringRef) CFDictionaryGetValue(props, CFSTR("subtype"));
		factoryFunction = (CFStringRef) CFDictionaryGetValue(props, CFSTR("factoryFunction"));

		if (!isString(name) || !isString(manufacturer) || !isString(type) || !isString(subtype) || !isString(factoryFunction))
			continue;

		version = (CFNumberRef) CFDictionaryGetValue(props, CFSTR("version"));
		sandboxSafe = (CFBooleanRef) CFDictionaryGetValue(props, CFSTR("sandboxSafe"));

		if (version && CFGetTypeID(version) != CFNumberGetTypeID())
			version = nullptr;

		if (!sandboxSafe)
			sandboxSafe = kCFBooleanFalse;

		CFURLRef bundleUrl = CFBundleCopyBundleURL(bundle);
		CFStringRef bundlePath = CFURLCopyFileSystemPath(bundleUrl, kCFURLPOSIXPathStyle);
		CFRelease(bundleUrl);

		AudioComponentDescription desc;
		desc.componentType = stringToFourCC(type);
		desc.componentSubType = stringToFourCC(subtype);
		desc.componentManufacturer = stringToFourCC(manufacturer);
		desc.componentFlags = 0;
		desc.componentFlagsMask = 0;

		AudioComponentFlags flags = 0;
		if (sandboxSafe == kCFBooleanTrue)
			flags |= kAudioComponentFlag_SandboxSafe;

		UInt32 versionValue = 0;
		if (version)
			CFNumberGetValue(version, kCFNumberSInt32Type, &versionValue);

		registerComponent(&desc, CFStringGetCStringPtr(name, kCFStringEncodingUTF8), versionValue,
			CFStringGetCStringPtr(bundlePath, kCFStringEncodingUTF8), CFStringGetCStringPtr(factoryFunction, kCFStringEncodingUTF8),
			flags);

		CFRelease(bundlePath);
	}
}

AudioComponentManager* AudioComponentManager::instance()
{
	static AudioComponentManager inst;
	return &inst;
}

bool AudioComponentManager::isOurInstance(AudioComponentInstance instance)
{
	uint32_t v = uint32_t(uintptr_t(instance));
	return v & 0x80000000;
}

/*
AudioComponentInstance AudioComponentManager::audioUnitToInstance(AudioUnit unit)
{
	uint32_t v = uint32_t(uintptr_t(unit));
	if (v & 0x80000000)
	{
		v &= 0x7fffffff;
		return AudioComponentInstance(uintptr_t(v));
	}
	return nullptr;
}
*/

std::vector<AudioComponent> AudioComponentManager::findMatching(const AudioComponentDescription* cd)
{
	std::vector<AudioComponent> rv;
	std::unique_lock<std::recursive_mutex> l(m_componentsMutex);

	for (auto const& [componentId, d] : m_components)
	{
		if (!cd->componentManufacturer || cd->componentManufacturer == d.desc.componentManufacturer)
		{
			if (!cd->componentSubType || cd->componentSubType == d.desc.componentSubType)
			{
				if (!cd->componentType || cd->componentType == d.desc.componentType)
				{
					if (d.flags & kAudioComponentFlag_Unsearchable)
					{
						if (!cd->componentManufacturer || !cd->componentSubType || !cd->componentType)
							continue;
					}
					rv.push_back(d.id);
				}
			}
		}
	}

	return rv;
}

AudioComponent AudioComponentManager::registerComponent(const AudioComponentDescription* desc, const char* name,
		uint32_t version, AudioComponentFactoryFunction factory)
{
	std::unique_lock<std::recursive_mutex> l(m_componentsMutex);

	RegisteredComponent rc;
	rc.desc = *desc;
	rc.name = name;
	rc.version = version;
	rc.factory = factory;
	rc.id = (AudioComponent) uintptr_t(m_nextComponentId++);

	m_components.insert({ rc.id, rc });

	return rc.id;
}

AudioComponent AudioComponentManager::registerComponent(const AudioComponentDescription* desc, const char* name,
		uint32_t version, const char* bundlePath, const char* entryPointName, AudioComponentFlags flags)
{
	std::unique_lock<std::recursive_mutex> l(m_componentsMutex);

	RegisteredComponent rc;
	rc.desc = *desc;
	rc.name = name;
	rc.version = version;
	rc.factory = nullptr;
	rc.bundlePath = bundlePath;
	rc.entryPointName = entryPointName;
	rc.id = (AudioComponent) uintptr_t(m_nextComponentId++);
	rc.flags = flags;

	m_components.insert({ rc.id, rc });

	return rc.id;
}

CFBundleRef AudioComponentManager::bundleFromPath(const char* path)
{
	CFStringRef cfpath = CFStringCreateWithCString(nullptr, path, kCFStringEncodingUTF8);

	CFURLRef url = CFURLCreateWithFileSystemPath(nullptr, cfpath, kCFURLPOSIXPathStyle, true);
	CFRelease(cfpath);

	CFBundleRef bundle = CFBundleCreate(nullptr, url);
	CFRelease(url);

	return bundle;
}

OSStatus AudioComponentManager::instantiate(AudioComponent c, AudioComponentInstance* out)
{
	std::unique_lock<std::recursive_mutex> l(m_componentsMutex);

	*out = nullptr;

	auto it = m_components.find(c);
	if (it == m_components.end())
		return invalidComponentID;

	RegisteredComponent& regd = it->second;
	if (!regd.factory)
	{
		// Load the bundle
		CFBundleRef bundle = bundleFromPath(regd.bundlePath.c_str());

		if (!bundle)
			return invalidComponentID;

		CFStringRef factoryName = CFStringCreateWithCString(nullptr, regd.entryPointName.c_str(), kCFStringEncodingUTF8);
		regd.factory = (AudioComponentFactoryFunction) CFBundleGetFunctionPointerForName(bundle, factoryName);
		CFRelease(factoryName);

		if (!regd.factory)
		{
			CFRelease(bundle);
			return invalidComponentID;
		}

		regd.loadedBundle = bundle;
	}

	std::unique_lock<std::recursive_mutex> l2(m_componentInstancesMutex);

	ComponentInstanceData cid;
	*out = (AudioComponentInstance) uintptr_t(m_nextComponentInstanceId++);

	cid.component = c;
	cid.object = regd.factory(&regd.desc);

	if (!cid.object)
		return invalidComponentID;

	m_componentInstances.insert({ *out, cid });

	OSStatus status = cid.object->Open(cid.object, *out);
	if (status != noErr)
	{
		dispose(*out);
		*out = nullptr;
		return status;
	}
	regd.instances++;

	return status;
}

OSStatus AudioComponentManager::dispose(AudioComponentInstance instance)
{
	std::unique_lock<std::recursive_mutex> l2(m_componentInstancesMutex);
	auto it = m_componentInstances.find(instance);
	if (it == m_componentInstances.end())
		return badComponentInstance;

	ComponentInstanceData& cid = it->second;
	OSStatus status = cid.object->Close(cid.object);

	std::unique_lock<std::recursive_mutex> l(m_componentsMutex);
	auto itComp = m_components.find(cid.component);
	if (itComp != m_components.end())
	{
		RegisteredComponent& regd = itComp->second;
		regd.instances--;

		if (regd.instances == 0 && regd.loadedBundle != nullptr)
		{
			CFRelease(regd.loadedBundle);
			regd.loadedBundle = nullptr;
			regd.factory = nullptr;
		}
	}

	m_componentInstances.erase(it);
	return status;
}

AudioComponentPlugInInterface* AudioComponentManager::instanceInterface(AudioComponentInstance instance)
{
	std::unique_lock<std::recursive_mutex> l2(m_componentInstancesMutex);
	auto it = m_componentInstances.find(instance);
	if (it == m_componentInstances.end())
		return nullptr;

	return it->second.object;
}

AudioComponentManager::RegisteredComponent* AudioComponentManager::getById(AudioComponent component)
{
	auto itComp = m_components.find(component);
	if (itComp != m_components.end())
		return &itComp->second;
	return nullptr;
}

OSStatus AudioComponentManager::getDescription(AudioComponent c, AudioComponentDescription* out)
{
	std::unique_lock<std::recursive_mutex> l(m_componentsMutex);
	RegisteredComponent* regd = getById(c);

	if (!regd)
		return invalidComponentID;

	*out = regd->desc;
	return noErr;
}

OSStatus AudioComponentManager::getName(AudioComponent c, const char** name)
{
	std::unique_lock<std::recursive_mutex> l(m_componentsMutex);
	RegisteredComponent* regd = getById(c);

	if (!regd)
		return invalidComponentID;

	*name = regd->name.c_str();
	return noErr;
}

OSStatus AudioComponentManager::getVersion(AudioComponent c, uint32_t* version)
{
	std::unique_lock<std::recursive_mutex> l(m_componentsMutex);
	RegisteredComponent* regd = getById(c);

	if (!regd)
		return invalidComponentID;

	*version = regd->version;
	return noErr;
}

AudioComponent AudioComponentManager::getComponent(AudioComponentInstance instance)
{
	std::unique_lock<std::recursive_mutex> l2(m_componentInstancesMutex);
	auto it = m_componentInstances.find(instance);
	if (it == m_componentInstances.end())
		return nullptr;

	return it->second.component;
}
