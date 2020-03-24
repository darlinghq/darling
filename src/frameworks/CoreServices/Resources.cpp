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
#include "ResourcesImpl.h"

// Documentation:
// http://web.archive.org/web/20040603192835/http://developer.apple.com/documentation/Carbon/Reference/Resource_Manager/index.html

// Usage example:
// https://github.com/nathanday/ndalias/blob/master/Classes/NDResourceFork.m

static std::unordered_map<ResFileRefNum, Resources*> g_resources;
static std::mutex g_resourcesLock;
static ResFileRefNum g_currentResFile = kResFileNotOpened;
static ResFileRefNum g_nextRefNum = 1;

ResFileRefNum FSOpenResFile(const FSRef* ref, SInt8 permission)
{
	HFSUniStr255 forkName;
	FSGetDataForkName(&forkName);

	ResFileRefNum refNum;
	OSErr err = FSOpenResourceFile(ref, forkName.length, forkName.unicode, permission, &refNum);

	if (err != noErr)
		return kResFileNotOpened;
	return refNum;
}

// Should return eofErr if given fork doesn't exist / is empty
OSErr FSOpenResourceFile(const FSRef* ref, UniCharCount forkNameLength, const UniChar* forkName, SInt8 permissions, ResFileRefNum* refNum)
{
	bool resourceFork = false;
	const bool readOnly = permissions == fsRdPerm;

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

		Resources* r = new Resources(path, readOnly, resourceFork);

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

		return noErr;
	}
	catch (OSErr e)
	{
		*refNum = kResFileNotOpened;
		return e;
	}
}

ResFileRefNum CurResFile(void)
{
	return g_currentResFile;
}

void UseResFile(ResFileRefNum ref)
{
	std::unique_lock<std::mutex> l(g_resourcesLock);

	if (g_resources.find(ref) != g_resources.end())
		g_currentResFile = ref;
}

void CloseResFile(ResFileRefNum refNum)
{
	if (g_currentResFile == refNum)
		g_currentResFile = kResFileNotOpened;

	std::unique_lock<std::mutex> l(g_resourcesLock);
	auto it = g_resources.find(refNum);

	if (it != g_resources.end())
	{
		delete it->second;
		g_resources.erase(it);
	}
}
