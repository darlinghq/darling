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

#include "ComponentManager.h"
#include <CoreServices/MacErrors.h>
#include <CoreServices/Resources.h>
#include <CoreServices/MacMemory.h>
#include <dirent.h>
#include <cstdlib>
#include <CoreFoundation/CFBundle.h>
#include <CoreFoundation/CFURL.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFArray.h>

// Some clues about how this works can be found here:
// https://vintageapple.org/develop/pdf/develop-12_9212_December_1992.pdf

#if defined(__i386__)
#	define CURRENT_PLATFORM platformIA32NativeEntryPoint
#elif defined(__x86_64__)
#	define CURRENT_PLATFORM platformX86_64NativeEntryPoint
#elif defined(__ppc__)
#	define CURRENT_PLATFORM platformPowerPCNativeEntryPoint
#elif defined(__ppc64__)
#	define CURRENT_PLATFORM platformPowerPC64NativeEntryPoint
#elif defined(__arm64__)
#	define CURRENT_PLATFORM platformArm64NativeEntryPoint
#endif

ComponentManager* ComponentManager::instance()
{
	static ComponentManager inst;
	return &inst;
}

ComponentManager::ComponentManager()
{
	discoverComponents();
}

void ComponentManager::discoverComponents()
{
	discoverComponents("/System/Library/Components");
	discoverComponents("/Library/Components");

	const char* home = ::getenv("HOME");
	if (home != NULL)
	{
		std::string userPath = home;
		userPath += "/Library/Audio/Plug-Ins/Components";

		discoverComponents(userPath.c_str());
	}
}

void ComponentManager::discoverComponents(const char* dir)
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
	{
		CFBundleRef bundle = (CFBundleRef) CFArrayGetValueAtIndex(componentBundles, i);
		analyzeComponent(bundle);
		CFRelease(bundle);
	}

	CFRelease(componentBundles);
}

void ComponentManager::analyzeComponent(CFBundleRef bundle)
{
	ResFileRefNum resFile = resFileForBundle(bundle);
	analyzeComponent(bundle, resFile);
	CloseResFile(resFile);
}

OSStatus ComponentManager::resFileForComponent(Component c, ResFileRefNum* resFile)
{
	std::unique_lock<std::recursive_mutex> l(m_componentsMutex);

	*resFile = kResFileNotOpened;

	auto itMap = m_componentsMap.find(c);
	if (itMap == m_componentsMap.end())
		return invalidComponentID;

	ComponentData* cd = itMap->second;
	if (cd->bundlePath.empty())
		return resFNotFound; // This is a dynamically registered component

	CFBundleRef bundle = bundleFromPath(cd->bundlePath.c_str());
	if (!bundle)
		return resFNotFound;

	*resFile = resFileForBundle(bundle);
	CFRelease(bundle);

	if (*resFile == kResFileNotOpened)
		return resFNotFound;

	return noErr;
}

ResFileRefNum ComponentManager::resFileForBundle(CFBundleRef bundle)
{
	OSStatus status;
	FSRef fsref;

	CFURLRef url = CFBundleCopyExecutableURL(bundle);
	if (url != nullptr)
	{
		CFStringRef filePath = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);

		// Check for a resource fork to analyze
		status = FSPathMakeRef((const uint8_t*) CFStringGetCStringPtr(filePath, kCFStringEncodingUTF8), &fsref, nullptr);

		if (status == noErr)
		{
			HFSUniStr255 rsrcForkName;
			ResFileRefNum resFile;

			FSGetResourceForkName(&rsrcForkName);

			status = FSOpenResourceFile(&fsref, rsrcForkName.length, rsrcForkName.unicode, fsRdPerm, &resFile);

			if (status == noErr)
			{
				CFRelease(filePath);
				CFRelease(url);
				return resFile;
			}
		}

		CFRelease(filePath);
		CFRelease(url);
	}

	CFURLRef bundleUrl = CFBundleCopyBundleURL(bundle);
	CFURLRef bundleUrlNoExt = CFURLCreateCopyDeletingPathExtension(nullptr, bundleUrl);
	CFRelease(bundleUrl);

	CFStringRef bundleName = CFURLCopyLastPathComponent(bundleUrlNoExt);
	CFRelease(bundleUrlNoExt);

	url = CFBundleCopyResourceURL(bundle, bundleName, CFSTR("rsrc"), nullptr);
	CFRelease(bundleName);

	// Check for a resource file
	if (url != nullptr)
	{
		CFStringRef filePath = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);

		status = FSPathMakeRef((const uint8_t*) CFStringGetCStringPtr(filePath, kCFStringEncodingUTF8), &fsref, nullptr);

		if (status == noErr)
		{
			HFSUniStr255 rsrcForkName;
			ResFileRefNum resFile;

			FSGetDataForkName(&rsrcForkName);

			status = FSOpenResourceFile(&fsref, rsrcForkName.length, rsrcForkName.unicode, fsRdPerm, &resFile);

			if (status == noErr)
			{
				return resFile;
			}
		}

		CFRelease(filePath);
		CFRelease(url);
	}

	return kResFileNotOpened;
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

static std::string loadResString(ResType type, ResID id)
{
	Handle h = Get1Resource(type, id);
	if (!h)
		return std::string();

	return std::string(*h, GetHandleSize(h));
}

void ComponentManager::analyzeComponent(CFBundleRef bundle, ResFileRefNum resFile)
{
	ResFileRefNum origRes = CurResFile();
	UseResFile(resFile);

	ResourceCount thingCount = Count1Resources('thng');
	for (int i = 1; i <= thingCount; i++)
	{
		Handle handle = Get1IndResource('thng', i);
		if (handle && GetHandleSize(handle) >= sizeof(CarbonThng))
		{
			CarbonThng thng = *((CarbonThng*) *handle);

			SWAP(thng.Type);
			SWAP(thng.Subtype);
			SWAP(thng.Manufacturer);

			SWAP(thng.CodeType);
			SWAP(thng.CodeId);

			SWAP(thng.NameType);
			SWAP(thng.NameId);

			SWAP(thng.InfoType);
			SWAP(thng.InfoId);

			SWAP(thng.ComponentFlags);
			SWAP(thng.ComponentFlagsMask);

			SWAP(thng.PlatformInfoCount);
			SWAP(thng.RegistrationFlags);

			ComponentDescription cd;
			cd.componentType = thng.Type;
			cd.componentSubType = thng.Subtype;
			cd.componentManufacturer = thng.Manufacturer;

			std::string name, info, entryPoint;
			name = loadResString(thng.NameType, thng.NameId);
			info = loadResString(thng.InfoType, thng.InfoId);

			for (int p = 0; p < thng.PlatformInfoCount; p++)
			{
				PlatformInfo pi = ((CarbonThng*) *handle)->PlatformInfos[p];

				SWAP(pi.PlatformType);

				if (pi.PlatformType == CURRENT_PLATFORM)
				{
					SWAP(pi.CodeType);
					SWAP(pi.CodeId);
					entryPoint = loadResString(pi.CodeType, pi.CodeId);
				}
			}

			if (!entryPoint.empty())
			{
				CFURLRef url = CFBundleCopyBundleURL(bundle);
				CFStringRef cfpath = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);

				CFRelease(url);

				const char* path = CFStringGetCStringPtr(cfpath, kCFStringEncodingUTF8);
				registerComponent(&cd, thng.RegistrationFlags, path, entryPoint.c_str(), name.c_str(), info.c_str(), nullptr);
				CFRelease(cfpath);
			}
		}
	}

	UseResFile(origRes);
}

#undef SWAP

Component ComponentManager::registerComponent(const ComponentDescription* cd, SInt16 flags, const char* bundlePath,
	const char* entryPoint, const char* name, const char* info, void* icon)
{
	std::unique_lock<std::recursive_mutex> l(m_componentsMutex);

	ComponentData data;
	data.cd = *cd;
	data.instances = 0;

	if (name)
		data.name = name;
	if (info)
		data.info = info;

	data.entryPoint = nullptr;
	data.entryPointName = entryPoint;
	data.bundlePath = bundlePath;

	ComponentData* dataPtr;
	if (flags & registerComponentAfterExisting)
	{
		m_components.push_back(data);
		dataPtr = &m_components.back();
	}
	else
	{
		m_components.push_front(data);
		dataPtr = &m_components.front();
	}
	
	Component c = Component(uintptr_t(nextComponentId++));
	dataPtr->component = c;

	m_componentsMap.insert(std::make_pair(c, dataPtr));

	return c;
}

Component ComponentManager::registerComponent(const ComponentDescription* cd, ComponentRoutineUPP entryPointIn, SInt16 flags,
	const char* name, const char* info, void* icon)
{
	std::unique_lock<std::recursive_mutex> l(m_componentsMutex);
	if (flags & registerComponentNoDuplicates)
	{
		if (!findMatching(cd).empty())
			return nullptr;
	}

	ComponentData data;
	data.cd = *cd;
	data.entryPoint = entryPointIn;
	data.instances = 0;

	if (name)
		data.name = name;
	if (info)
		data.info = info;

	ComponentData* dataPtr;
	if (flags & registerComponentAfterExisting)
	{
		m_components.push_back(data);
		dataPtr = &m_components.back();
	}
	else
	{
		m_components.push_front(data);
		dataPtr = &m_components.front();
	}
	
	Component c = Component(uintptr_t(nextComponentId++));
	dataPtr->component = c;

	m_componentsMap.insert(std::make_pair(c, dataPtr));

	return c;
}

void ComponentManager::setDefault(Component c, SInt16 flags)
{
	// FIXME: This is wrong, because we don't honor flags
	std::unique_lock<std::recursive_mutex> l(m_componentsMutex);
	// Just move the element to the front

	auto itMap = m_componentsMap.find(c);
	if (itMap == m_componentsMap.end())
		return;

	for (auto it = m_components.begin(); it != m_components.end(); it++)
	{
		ComponentData* d = &(*it);
		if (d == itMap->second)
		{
			// TODO: Check whether element address is unchanged after this!
			m_components.splice(m_components.begin(), m_components, it);
			break;
		}
	}
}

OSStatus ComponentManager::unregisterComponent(Component c)
{
	std::unique_lock<std::recursive_mutex> l(m_componentsMutex);

	auto itMap = m_componentsMap.find(c);
	if (itMap == m_componentsMap.end())
		return invalidComponentID;

	if (itMap->second->instances != 0)
		return validInstancesExist;

	for (auto it = m_components.begin(); it != m_components.end(); it++)
	{
		ComponentData* d = &(*it);
		if (d == itMap->second)
		{
			m_components.erase(it);
			break;
		}
	}

	m_componentsMap.erase(itMap);
	return noErr;
}

std::vector<Component> ComponentManager::findMatching(const ComponentDescription* cd)
{
	std::vector<Component> rv;

	std::unique_lock<std::recursive_mutex> l(m_componentsMutex);

	for (const ComponentData& d : m_components)
	{
		if (!cd->componentManufacturer || cd->componentManufacturer == d.cd.componentManufacturer)
		{
			if (!cd->componentSubType || cd->componentSubType == d.cd.componentSubType)
			{
				if (!cd->componentType || cd->componentType == d.cd.componentType)
				{
					rv.push_back(d.component);
				}
			}
		}
	}

	return rv;
}

ComponentInstance ComponentManager::getComponentInstance(ComponentParameters* desc)
{
#if __LP64__
	return (ComponentInstance) desc->params[0];
#else
	return (ComponentInstance) desc->params[(desc->paramSize / sizeof(long)) - 1];
#endif
}

CFBundleRef ComponentManager::bundleFromPath(const char* path)
{
	CFStringRef cfpath = CFStringCreateWithCString(nullptr, path, kCFStringEncodingUTF8);

	CFURLRef url = CFURLCreateWithFileSystemPath(nullptr, cfpath, kCFURLPOSIXPathStyle, true);
	CFRelease(cfpath);

	CFBundleRef bundle = CFBundleCreate(nullptr, url);
	CFRelease(url);

	return bundle;
}

OSStatus ComponentManager::instantiate(Component c, ComponentInstance* out)
{
	std::unique_lock<std::recursive_mutex> l(m_componentsMutex);

	*out = nullptr;

	auto itMap = m_componentsMap.find(c);
	if (itMap == m_componentsMap.end())
		return invalidComponentID;

	ComponentData* cd = itMap->second;

	if (!cd->entryPoint)
	{
		CFBundleRef bundle = bundleFromPath(cd->bundlePath.c_str());

		if (!bundle)
			return invalidComponentID;

		CFStringRef cfEntryPointName = CFStringCreateWithCString(nullptr, cd->entryPointName.c_str(), kCFStringEncodingUTF8);
		cd->entryPoint = (ComponentRoutineUPP) CFBundleGetFunctionPointerForName(bundle, cfEntryPointName);
		CFRelease(cfEntryPointName);

		if (!cd->entryPoint)
		{
			CFRelease(bundle);
			return invalidComponentID;
		}

		cd->loadedBundle = bundle;
	}

	ComponentInstanceData cid;

	cid.component = c;
	cid.componentData = cd;
	cid.componentData->instances++;
	cid.storage = nullptr;

	l.unlock();

	std::unique_lock<std::recursive_mutex> l2(m_componentInstancesMutex);
	ComponentInstance inst = ComponentInstance(uintptr_t(nextInstanceId++));

	m_componentInstances.insert(std::make_pair(inst, cid));
	l2.unlock();

	ComponentParameters cp;
	cp.flags = 0;
	cp.paramSize = sizeof(void*);
	cp.what = kComponentOpenSelect;
	cp.params[0] = uintptr_t(inst);

	OSStatus status = dispatch(&cp);
	if (status != noErr)
	{
		// "Note that whenever an open request fails, the Component Manager always issues the close request."
		dispose(inst);
		return status;
	}

	*out = inst;
	return noErr;
}

OSStatus ComponentManager::dispose(ComponentInstance c)
{
	std::unique_lock<std::recursive_mutex> l(m_componentInstancesMutex);

	auto it = m_componentInstances.find(c);
	if (it == m_componentInstances.end())
		return badComponentInstance;

	ComponentParameters cp;
	cp.flags = 0;
	cp.paramSize = sizeof(void*);
	cp.what = kComponentCloseSelect;
	cp.params[0] = uintptr_t(c);

	OSStatus status = dispatch(&cp);

	Component component = it->second.component;
	auto itMap = m_componentsMap.find(component);
	if (itMap != m_componentsMap.end())
	{
		ComponentData* cd = itMap->second;
		cd->instances--;

		// If this was a dynamically loaded component
		// and there are not instances left, release the bundle.
		if (cd->instances == 0 && cd->loadedBundle)
		{
			cd->entryPoint = nullptr;
			CFRelease(cd->loadedBundle);
			cd->loadedBundle = nullptr;
		}
	}

	m_componentInstances.erase(it);
	return status;
}

OSStatus ComponentManager::dispatch(ComponentParameters* cp)
{
	std::unique_lock<std::recursive_mutex> l(m_componentInstancesMutex);

	auto it = m_componentInstances.find(getComponentInstance(cp));
	if (it == m_componentInstances.end())
		return badComponentInstance;

	return it->second.componentData->entryPoint(cp, it->second.storage);
}

Handle ComponentManager::getStorage(ComponentInstance ci)
{
	std::unique_lock<std::recursive_mutex> l(m_componentInstancesMutex);

	auto it = m_componentInstances.find(ci);
	if (it == m_componentInstances.end())
		return nullptr;

	return it->second.storage;
}

void ComponentManager::setStorage(ComponentInstance ci, Handle storage)
{
	std::unique_lock<std::recursive_mutex> l(m_componentInstancesMutex);

	auto it = m_componentInstances.find(ci);
	if (it == m_componentInstances.end())
		return;

	it->second.storage = storage;
}

OSStatus ComponentManager::componentData(Component c, ComponentData* out)
{
	std::unique_lock<std::recursive_mutex> l(m_componentsMutex);

	auto itMap = m_componentsMap.find(c);
	if (itMap == m_componentsMap.end())
		return invalidComponentID;

	ComponentData* cd = itMap->second;

	*out = *cd;

	return noErr;
}

uint32_t ComponentManager::componentInstances(Component c)
{
	std::unique_lock<std::recursive_mutex> l(m_componentsMutex);

	auto itMap = m_componentsMap.find(c);
	if (itMap == m_componentsMap.end())
		return 0;

	ComponentData* cd = itMap->second;
	return cd->instances;
}

Component ComponentManager::componentID(ComponentInstance ci)
{
	std::unique_lock<std::recursive_mutex> l(m_componentInstancesMutex);

	auto it = m_componentInstances.find(ci);
	if (it == m_componentInstances.end())
		return nullptr;
	return it->second.component;
}
