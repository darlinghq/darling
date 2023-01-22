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

#ifndef _CS_RESOURCES_IMPL_H
#define _CS_RESOURCES_IMPL_H
#include <CarbonCore/Resources.h>
#include <CarbonCore/MacMemory.h>
#include <string>
#include <stdint.h>
#include <vector>
#include <list>
#include <unordered_map>

class Resources
{
public:
	Resources(const char* file, SInt8 openMode, bool resourceFork);

	struct ResourceData
	{
		ResID id;
		ResType type;
		std::string name;
		ResAttributes attributes;
		std::vector<uint8_t> dataBacking;
		Resources* parent;

		ResourceData() {}
		ResourceData(Handle inData);
		ResourceData(const ResourceData& that) = delete;
		ResourceData(ResourceData&& that);
		~ResourceData();

		Handle getHandle();
		void releaseHandle(bool freeMemory);
		void dataChanged();
private:
		Handle data = nullptr;
	};

	struct ResourceTypeData
	{
		ResType type;
		std::vector<std::unique_ptr<ResourceData>> resources;
		std::unordered_map<ResID, ResourceData*> idMap;
		std::unordered_map<std::string, ResourceData*> nameMap;
	};

	ResFileAttributes attributes() const { return m_attributes; }
	void setAttributes(ResFileAttributes a) { m_attributes = a; }

	std::unordered_map<ResType, ResourceTypeData>& data() { return m_resourceTypes; }
	ResourceData* dataByHandle(Handle h);
	std::unordered_map<Handle, ResourceData*>& handleMap() { return m_handleMap; }

	void save();
	void add(ResType type, ResID id, const char* name, Handle data);
	void remove(Handle data);

	static void setCreateHandles(bool create);
private:
	void loadResources();
	void loadResources(const uint8_t* mem, size_t length);
protected:
	std::string m_file;
	const bool m_resourceFork;
	const SInt8 m_openMode;

	ResFileAttributes m_attributes;
	
	std::unordered_map<ResType, ResourceTypeData> m_resourceTypes;
	std::unordered_map<Handle, ResourceData*> m_handleMap;
	static bool m_createHandles;

	friend struct ResourceData;
};

#endif

