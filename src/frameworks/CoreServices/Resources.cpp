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

#include <CoreServices/Resources.h>
#include <unordered_map>
#include <mutex>
#include <limits>
#include <set>
#include <algorithm>
#include "ResourcesImpl.h"

// Documentation:
// http://web.archive.org/web/20040603192835/http://developer.apple.com/documentation/Carbon/Reference/Resource_Manager/index.html
// https://developer.apple.com/library/archive/documentation/mac/pdf/MoreMacintoshToolbox.pdf

// Usage example:
// https://github.com/nathanday/ndalias/blob/master/Classes/NDResourceFork.m

static std::unordered_map<ResFileRefNum, Resources*> g_resources;
static std::mutex g_resourcesLock;

static Resources* g_currentResources = nullptr;
static ResFileRefNum g_currentResFile = kResFileNotOpened;
static ResFileRefNum g_nextRefNum = 1;

static OSErr g_lastError = noErr;

static void clearCaches();

ResFileRefNum FSOpenResFile(const FSRef* ref, SInt8 permissions)
{
	HFSUniStr255 forkName;
	FSGetDataForkName(&forkName);

	ResFileRefNum refNum;
	OSErr err = FSOpenResourceFile(ref, forkName.length, forkName.unicode, permissions, &refNum);

	if (err != noErr)
		return kResFileNotOpened;
	return refNum;
}

// Should return eofErr if given fork doesn't exist / is empty
OSErr FSOpenResourceFile(const FSRef* ref, UniCharCount forkNameLength, const UniChar* forkName, SInt8 permissions, ResFileRefNum* refNum)
{
	bool resourceFork = false;

	if (forkNameLength != 0)
	{
		HFSUniStr255 rsrcForkName;
		FSGetResourceForkName(&rsrcForkName);

		if (rsrcForkName.length == forkNameLength && memcmp(forkName, rsrcForkName.unicode, 2 * forkNameLength) == 0)
			resourceFork = true;
	}

	try
	{
		char path[4096];

		OSErr rv = FSRefMakePath(ref, (UInt8*) path, sizeof(path) - 1);
		if (rv != noErr)
			throw rv;

		Resources* r = new Resources(path, permissions, resourceFork);

		std::unique_lock<std::mutex> l(g_resourcesLock);
		while (g_resources.find(g_nextRefNum) != g_resources.end())
		{
			if (g_nextRefNum == std::numeric_limits<short>::max())
				g_nextRefNum = 1;
			else
				g_nextRefNum++;
		}

		g_resources.insert(std::make_pair(g_nextRefNum, r));
		*refNum = g_nextRefNum;
		g_nextRefNum++;
		clearCaches();

		return noErr;
	}
	catch (OSErr e)
	{
		*refNum = kResFileNotOpened;
		return e;
	}
}

OSErr ResError(void)
{
	return g_lastError;
}

ResFileRefNum CurResFile(void)
{
	return g_currentResFile;
}

void UseResFile(ResFileRefNum ref)
{
	std::unique_lock<std::mutex> l(g_resourcesLock);

	if (ref == kResFileNotOpened)
	{
		g_currentResFile = ref;
		g_currentResources = nullptr;
		g_lastError = noErr;
		return;
	}

	if (auto it = g_resources.find(ref); it != g_resources.end())
	{
		g_currentResFile = ref;
		g_currentResources = it->second;
		g_lastError = noErr;
	}
	else
		g_lastError = resFNotFound;
}

void CloseResFile(ResFileRefNum refNum)
{
	std::unique_lock<std::mutex> l(g_resourcesLock);
	if (g_currentResFile == refNum)
	{
		g_currentResFile = kResFileNotOpened;
		g_currentResources = nullptr;
	}

	auto it = g_resources.find(refNum);

	if (it != g_resources.end())
	{
		delete it->second;
		g_resources.erase(it);
		g_lastError = noErr;
		clearCaches();
	}
	else
		g_lastError = resFNotFound;
}

static std::set<ResType> allTypesCache;
static std::set<ResType>& allTypes()
{
	std::set<ResType>& resTypes = allTypesCache;

	if (resTypes.empty())
	{
		for (auto const& [refNum, resources] : g_resources)
		{
			for (auto const& [type, data] : resources->data())
				resTypes.insert(type);
		}
	}

	return resTypes;
}

ResourceCount CountTypes(void)
{
	std::unique_lock<std::mutex> l(g_resourcesLock);
	std::set<ResType>& resTypes = allTypes();

	return resTypes.size();
}

ResourceCount Count1Types(void)
{
	std::unique_lock<std::mutex> l(g_resourcesLock);

	if (!g_currentResources)
	{
		g_lastError = resFNotFound;
		return 0;
	}

	g_lastError = noErr;
	return g_currentResources->data().size();
}

static Resources::ResourceData* findResourceDataByHandle(Handle res, Resources** resOut = nullptr)
{
	for (auto const& [refNum, resources] : g_resources)
	{
		Resources::ResourceData* rv = resources->dataByHandle(res);
		if (rv)
		{
			if (resOut)
				*resOut = resources;
			return rv;
		}
	}

	g_lastError = resNotFound;
	return nullptr;
}

ResAttributes GetResAttrs(Handle res)
{
	std::unique_lock<std::mutex> l(g_resourcesLock);
	auto data = findResourceDataByHandle(res);

	if (!data)
		return 0;

	g_lastError = noErr;
	return data->attributes;
}

void GetResInfo(Handle res, ResID* resId, ResType* type, Str255 name)
{
	std::unique_lock<std::mutex> l(g_resourcesLock);
	auto data = findResourceDataByHandle(res);

	if (!data)
	{
		*resId = 0;
		*type = 0;
		name[0] = 0;
		return;
	}

	g_lastError = noErr;
	*resId = data->id;
	*type = data->type;

	// Pascal string
	name[0] = std::min<uint8_t>(254, data->name.length());
	memcpy(name+1, data->name.c_str(), name[0]);
}

void SetResInfo(Handle res, ResID theID, ConstStr255Param name)
{
	std::unique_lock<std::mutex> l(g_resourcesLock);
	Resources* resources;
	auto data = findResourceDataByHandle(res, &resources);

	if (!data)
	{
		g_lastError = resNotFound;
		return;
	}

	g_lastError = noErr;
	if (data->id != theID)
	{
		auto it = resources->data().find(data->type);
		if (it != resources->data().end())
		{
			Resources::ResourceTypeData& rtd = it->second;
			rtd.idMap.erase(data->id);
			rtd.idMap.insert({ theID, data });
		}

		data->id = theID;
	}

	// Pascal string
	std::string cname((char*) &name[1], name[0]);
	if (data->name != cname)
	{
		auto it = resources->data().find(data->type);
		if (it != resources->data().end())
		{
			Resources::ResourceTypeData& rtd = it->second;
			rtd.nameMap.erase(data->name);
			rtd.nameMap.insert({ cname, data });
		}

		data->name = cname;
	}
}

void SetResAttrs(Handle res, ResAttributes attrs)
{
	std::unique_lock<std::mutex> l(g_resourcesLock);
	auto data = findResourceDataByHandle(res);

	if (!data)
	{
		g_lastError = resNotFound;
		return;
	}

	g_lastError = noErr;
	data->attributes = attrs;
}

void GetIndType(ResType* type, ResourceIndex index)
{
	std::unique_lock<std::mutex> l(g_resourcesLock);
	std::set<ResType>& resTypes = allTypes();

	if (index < 1 || index > resTypes.size())
	{
		g_lastError = inputOutOfBounds;
		*type = 0;
		return;
	}

	auto it = resTypes.begin();
	std::advance(it, index-1);

	*type = *it;
	g_lastError = noErr;
}

void Get1IndType(ResType* type, ResourceIndex index)
{
	std::unique_lock<std::mutex> l(g_resourcesLock);

	if (!g_currentResources)
	{
		g_lastError = resFNotFound;
		*type = 0;
		return;
	}

	auto it = g_currentResources->data().begin();
	if (index < 1 || index > g_currentResources->data().size())
	{
		g_lastError = inputOutOfBounds;
		*type = 0;
		return;
	}

	std::advance(it, index-1);

	*type = it->first;
	g_lastError = noErr;
}

static std::vector<Resources::ResourceData*> resourcesOfTypeCache;
static ResType resourcesOfTypeCacheType = -1;
static std::vector<Resources::ResourceData*>& resourcesOfType(ResType type)
{
	std::vector<Resources::ResourceData*>& rv = resourcesOfTypeCache;

	if (resourcesOfTypeCacheType != type)
	{
		for (auto const& [refNum, resources] : g_resources)
		{
			auto it = resources->data().find(type);
			if (it == resources->data().end())
				continue;

			for (auto& d : it->second.resources)
				rv.push_back(d.get());
		}
		resourcesOfTypeCacheType = type;
	}

	return rv;
}

static std::unordered_map<ResID,Resources::ResourceData*> resourcesOfTypeMapCache;
static ResType resourcesOfTypeMapCacheType = -1;
static std::unordered_map<ResID,Resources::ResourceData*>& resourcesOfTypeMap(ResType type)
{
	std::unordered_map<ResID,Resources::ResourceData*>& rv = resourcesOfTypeMapCache;

	if (resourcesOfTypeMapCacheType != type)
	{
		for (auto const& [refNum, resources] : g_resources)
		{
			auto it = resources->data().find(type);
			if (it == resources->data().end())
				continue;

			rv.insert(it->second.idMap.begin(), it->second.idMap.end());
		}

		resourcesOfTypeMapCacheType = type;
	}

	return rv;
}

static std::unordered_map<std::string,Resources::ResourceData*> resourcesOfTypeNameMapCache;
static ResType resourcesOfTypeNameMapCacheType = -1;
static std::unordered_map<std::string,Resources::ResourceData*>& resourcesOfTypeNameMap(ResType type)
{
	std::unordered_map<std::string,Resources::ResourceData*>& rv = resourcesOfTypeNameMapCache;

	if (resourcesOfTypeMapCacheType != type)
	{
		for (auto const& [refNum, resources] : g_resources)
		{
			auto it = resources->data().find(type);
			if (it == resources->data().end())
				continue;

			rv.insert(it->second.nameMap.begin(), it->second.nameMap.end());
		}

		resourcesOfTypeNameMapCacheType = type;
	}

	return rv;
}

ResourceCount CountResources(ResType type)
{
	std::unique_lock<std::mutex> l(g_resourcesLock);
	auto& allRes = resourcesOfType(type);

	return allRes.size();
}

ResourceCount Count1Resources(ResType type)
{
	std::unique_lock<std::mutex> l(g_resourcesLock);
	
	if (!g_currentResources)
	{
		g_lastError = resFNotFound;
		return 0;
	}

	g_lastError = noErr;

	auto it = g_currentResources->data().find(type);
	if (it == g_currentResources->data().end())
		return 0;

	return it->second.resources.size();
}

Handle GetIndResource(ResType type, ResourceIndex index)
{
	std::unique_lock<std::mutex> l(g_resourcesLock);
	auto& allRes = resourcesOfType(type);

	if (index < 1 || index > allRes.size())
	{
		g_lastError = inputOutOfBounds;
		return nullptr;
	}

	return allRes[index-1]->getHandle();
}

Handle Get1IndResource(ResType type, ResourceIndex index)
{
	std::unique_lock<std::mutex> l(g_resourcesLock);

	if (!g_currentResources)
	{
		g_lastError = resFNotFound;
		return nullptr;
	}

	auto it = g_currentResources->data().find(type);
	if (it == g_currentResources->data().end())
	{
		g_lastError = resNotFound;
		return nullptr;
	}

	if (index < 1 || index > it->second.resources.size())
	{
		g_lastError = inputOutOfBounds;
		return nullptr;
	}

	g_lastError = noErr;

	return it->second.resources[index-1]->getHandle();
}

Handle GetResource(ResType type, ResID resId)
{
	std::unique_lock<std::mutex> l(g_resourcesLock);
	auto& allRes = resourcesOfTypeMap(type);

	auto it = allRes.find(resId);
	if (it == allRes.end())
	{
		g_lastError = resNotFound;
		return nullptr;
	}

	return it->second->getHandle();
}

Handle Get1Resource(ResType type, ResID resId)
{
	std::unique_lock<std::mutex> l(g_resourcesLock);

	if (!g_currentResources)
	{
		g_lastError = resFNotFound;
		return nullptr;
	}

	auto itType = g_currentResources->data().find(type);
	if (itType == g_currentResources->data().end())
	{
		g_lastError = resNotFound;
		return nullptr;
	}

	auto it = itType->second.idMap.find(resId);
	if (it == itType->second.idMap.end())
	{
		g_lastError = resNotFound;
		return nullptr;
	}

	return it->second->getHandle();
}

Handle GetNamedResource(ResType type, ConstStr255Param name)
{
	std::unique_lock<std::mutex> l(g_resourcesLock);
	auto& allRes = resourcesOfTypeNameMap(type);

	std::string cname((char*) &name[1], name[0]);
	auto it = allRes.find(cname);
	if (it == allRes.end())
	{
		g_lastError = resNotFound;
		return nullptr;
	}

	g_lastError = noErr;
	return it->second->getHandle();
}

Handle Get1NamedResource(ResType type, ConstStr255Param name)
{
	std::unique_lock<std::mutex> l(g_resourcesLock);

	if (!g_currentResources)
	{
		g_lastError = resFNotFound;
		return nullptr;
	}

	auto itType = g_currentResources->data().find(type);
	if (itType == g_currentResources->data().end())
	{
		g_lastError = resNotFound;
		return nullptr;
	}

	std::string cname((char*) &name[1], name[0]);
	auto it = itType->second.nameMap.find(cname);
	if (it == itType->second.nameMap.end())
	{
		g_lastError = resNotFound;
		return nullptr;
	}

	g_lastError = noErr;
	return it->second->getHandle();
}

ResFileAttributes GetResFileAttrs(ResFileRefNum refNum)
{
	std::unique_lock<std::mutex> l(g_resourcesLock);
	auto it = g_resources.find(refNum);

	if (it == g_resources.end())
	{
		g_lastError = resFNotFound;
		return 0;
	}

	g_lastError = noErr;
	return it->second->attributes();
}

void SetResFileAttrs(ResFileRefNum refNum, ResFileAttributes attrs)
{
	std::unique_lock<std::mutex> l(g_resourcesLock);
	auto it = g_resources.find(refNum);

	if (it == g_resources.end())
	{
		g_lastError = resFNotFound;
		return;
	}

	g_lastError = noErr;
	it->second->setAttributes(attrs);
}

void ChangedResource(Handle res)
{
	std::unique_lock<std::mutex> l(g_resourcesLock);
	auto data = findResourceDataByHandle(res);
	data->dataChanged();
}

void SetResourceSize(Handle res, long size)
{
	SetHandleSize(res, size);
}

static void clearCaches()
{
	resourcesOfTypeMapCache.clear();
	resourcesOfTypeMapCacheType = -1;
	resourcesOfTypeCache.clear();
	resourcesOfTypeCacheType = -1;
	resourcesOfTypeNameMapCache.clear();
	resourcesOfTypeNameMapCacheType = -1;
	allTypesCache.clear();
}

void WriteResource(Handle res)
{
	std::unique_lock<std::mutex> l(g_resourcesLock);
	Resources* resources;
	auto data = findResourceDataByHandle(res, &resources);

	if (!data)
	{
		g_lastError = resNotFound;
		return;
	}

	g_lastError = noErr;
	resources->save();
}

void UpdateResFile(ResFileRefNum refNum)
{
	std::unique_lock<std::mutex> l(g_resourcesLock);
	auto it = g_resources.find(refNum);

	if (it == g_resources.end())
	{
		g_lastError = resFNotFound;
		return;
	}

	g_lastError = noErr;
	it->second->save();
}

void DetachResource(Handle res)
{
	std::unique_lock<std::mutex> l(g_resourcesLock);
	Resources* resources;
	auto data = findResourceDataByHandle(res, &resources);

	data->releaseHandle(false);
}

void RemoveResource(Handle res)
{
	std::unique_lock<std::mutex> l(g_resourcesLock);

	if (!g_currentResources)
	{
		g_lastError = resFNotFound;
		return;
	}

	if (!res)
	{
		g_lastError = rmvResFailed;
		return;
	}

	g_currentResources->remove(res);
}

void ReleaseResource(Handle res)
{
	std::unique_lock<std::mutex> l(g_resourcesLock);
	Resources* resources;
	auto data = findResourceDataByHandle(res, &resources);

	data->releaseHandle(true);
}

void SetResLoad(Boolean load)
{
	Resources::setCreateHandles(load);
}

void AddResource(Handle data, ResType type, ResID resId, ConstStr255Param name)
{
	std::unique_lock<std::mutex> l(g_resourcesLock);

	if (!g_currentResources)
	{
		g_lastError = resFNotFound;
		return;
	}

	if (!data)
	{
		g_lastError = addResFailed;
		return;
	}

	auto existingData = findResourceDataByHandle(data);
	if (existingData != nullptr)
	{
		g_lastError = addResFailed;
		return;
	}

	std::string cname((char*) &name[1], name[0]);
	g_currentResources->add(type, resId, cname.c_str(), data);
}

void ReadPartialResource(Handle res, long offset, void* buffer, long count)
{
	std::unique_lock<std::mutex> l(g_resourcesLock);
	Size resLen = 0;
	if (res)
		resLen = GetHandleSize(res);
	
	if (offset + count > resLen)
	{
		g_lastError = inputOutOfBounds;
		return;
	}

	memcpy(buffer, *res + offset, count);
	g_lastError = noErr;
}

void WritePartialResource(Handle res, long offset, const void* buffer, long count)
{
	std::unique_lock<std::mutex> l(g_resourcesLock);
	if (!res)
	{
		g_lastError = resNotFound;
		return;
	}

	Resources* rr;
	auto existingData = findResourceDataByHandle(res, &rr);

	if (!existingData)
	{
		g_lastError = resNotFound;
		return;
	}

	g_lastError = noErr;
	if (offset + count > GetHandleSize(res))
	{
		SetHandleSize(res, offset+count);
		g_lastError = writingPastEnd;
	}

	memcpy(*res + offset, buffer, count);
	rr->save();
}

ResID UniqueID(ResType type)
{
	std::unique_lock<std::mutex> l(g_resourcesLock);
	std::set<ResID> allIds;

	for (auto const& [refNum, resources] : g_resources)
	{
		auto it = resources->data().find(type);
		if (it == resources->data().end())
		{
			g_lastError = noErr;
			return 128;
		}

		for (auto const& [id, rd] : it->second.idMap)
			allIds.insert(id);
	}

	for (ResID i = 128; i < std::numeric_limits<ResID>::max(); i++)
	{
		if (allIds.find(i) == allIds.end())
		{
			g_lastError = noErr;
			return i;
		}
	}
	return 0;
}

ResID Unique1ID(ResType type)
{
	std::unique_lock<std::mutex> l(g_resourcesLock);

	if (!g_currentResources)
	{
		g_lastError = resFNotFound;
		return 0;
	}

	auto it = g_currentResources->data().find(type);
	if (it == g_currentResources->data().end())
	{
		g_lastError = noErr;
		return 128;
	}

	for (ResID i = 128; i < std::numeric_limits<ResID>::max(); i++)
	{
		if (it->second.idMap.find(i) == it->second.idMap.end())
		{
			g_lastError = noErr;
			return i;
		}
	}
	return 0;
}
