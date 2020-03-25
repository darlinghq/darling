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
#include "ResourcesImpl.h"
#include <stdexcept>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/xattr.h>
#include <memory>

// File format documentation:
// https://developer.apple.com/library/archive/documentation/mac/MoreToolbox/MoreToolbox-99.html

// All is big endian!
struct ResourcesHeader
{
	uint32_t resourceDataOffset;
	uint32_t resourceMapOffset;
	uint32_t resourceDataLength;
	uint32_t resourceMapLength;
};

struct ResourceMapHeader
{
	ResourcesHeader _copy;
	uint32_t _handleNextMap;
	uint16_t _fileRefNumber;
	ResFileAttributes attributes;
	uint16_t typeListOffset; // from this
	uint16_t nameListOffset; // from this
	uint16_t numOfTypesMinusOne;
};

struct ResourceType
{
	uint32_t resourceType;
	uint16_t numOfThisTypeMinusOne;
	uint16_t referenceListOffset; // from this
};

struct ReferenceListItem
{
	ResID resourceID;
	uint16_t resourceNameListOffset;
	// Since this bitfield values have full byte sized values,
	// order change for BE/LE is not needed.
	uint32_t attributes : 8;
	uint32_t resourceDataOffset : 24;
	uint32_t _handle;
};

// Resource data is prefixed with a 4-byte length
// Resource name is prefixed with a 1-byte length

bool Resources::m_createHandles = true;

Resources::Resources(const char* file, SInt8 openMode, bool resourceFork)
: m_file(file), m_openMode(openMode), m_resourceFork(resourceFork)
{
	if (openMode != fsWrPerm)
	{
		// Load existing resources
		loadResources();
	}
}

void Resources::loadResources()
{
	if (!m_resourceFork)
	{
		int fd = ::open(m_file.c_str(), O_RDONLY);
		if (fd == -1)
			throw OSErr(fnfErr);

		off_t size = ::lseek(fd, 0, SEEK_END);

		if (size == 0)
		{
			::close(fd);
			throw OSErr(eofErr);
		}

		void* map = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
		::close(fd);

		if (map == MAP_FAILED)
			throw OSErr(fnfErr);

		try
		{
			loadResources(static_cast<uint8_t*>(map), size);
			::munmap(map, size);
		}
		catch(...)
		{
			::munmap(map, size);
			throw;
		}
	}
	else
	{
		ssize_t s = ::getxattr(m_file.c_str(), "com.apple.ResourceFork", nullptr, 0, 0, 0);
		if (s < 0)
			throw OSErr(eofErr);

		std::unique_ptr<uint8_t[]> buf(new uint8_t[s]);
		s = ::getxattr(m_file.c_str(), "com.apple.ResourceFork", buf.get(), s, 0, 0);
		if (s < 0)
			throw OSErr(eofErr);

		loadResources(buf.get(), s);
	}
}

Resources::ResourceData::ResourceData(ResourceData&& that)
{
	id = that.id;
	name = std::move(that.name);
	dataBacking = std::move(that.dataBacking);
	attributes = that.attributes;
	data = that.data;
	that.data = nullptr;
}

Resources::ResourceData::ResourceData(Handle inData)
{
	this->data = inData;
	dataChanged();
}

Resources::ResourceData::~ResourceData()
{
	if (data)
		DisposeHandle(data);
}

Handle Resources::ResourceData::getHandle()
{
	if (!data)
	{
		PtrToHand(dataBacking.data(), &data, dataBacking.size());
		parent->m_handleMap.insert({ data, this });
	}
	return data;
}

void Resources::ResourceData::releaseHandle(bool freeMemory)
{
	if (data)
	{
		if (freeMemory)
			DisposeHandle(data);
		parent->m_handleMap.erase(data);
		data = nullptr;
	}
}

void Resources::ResourceData::dataChanged()
{
	if (data)
		dataBacking.assign(*data, *data + GetHandleSize(data));
}

#ifdef __LITTLE_ENDIAN__
#	define SWAP(x) x = _bswap(x)

static inline uint16_t _bswap(uint16_t v) { return __builtin_bswap16(v); }
static inline int16_t _bswap(int16_t v) { return __builtin_bswap16(v); }
static inline uint32_t _bswap(uint32_t v) { return __builtin_bswap32(v); }

#else
#	define SWAP(x)
#endif

void Resources::loadResources(const uint8_t* mem, size_t length)
{
	m_resourceTypes.clear();
	m_attributes = 0;

	if (length < sizeof(ResourcesHeader))
		throw OSErr(eofErr);

	// TODO: Do more bounds checking

	ResourcesHeader header = *reinterpret_cast<const ResourcesHeader*>(mem);

	SWAP(header.resourceDataOffset);
	SWAP(header.resourceMapOffset);
	SWAP(header.resourceDataLength);
	SWAP(header.resourceMapLength);

	if (header.resourceDataOffset + header.resourceDataLength > length
		|| header.resourceMapOffset + header.resourceMapLength > length)
	{
		throw OSErr(eofErr);
	}

	ResourceMapHeader resourceMapHeader = *reinterpret_cast<const ResourceMapHeader*>(mem + header.resourceMapOffset);
	SWAP(resourceMapHeader.attributes);
	SWAP(resourceMapHeader.typeListOffset);
	SWAP(resourceMapHeader.nameListOffset);
	SWAP(resourceMapHeader.numOfTypesMinusOne);

	m_attributes = resourceMapHeader.attributes;

	const ResourceType* t = reinterpret_cast<const ResourceType*>(mem + header.resourceMapOffset + resourceMapHeader.typeListOffset);
	for (int i = 0; i <= resourceMapHeader.numOfTypesMinusOne; i++, t++)
	{
		ResourceType type = *t;

		SWAP(type.resourceType);
		SWAP(type.numOfThisTypeMinusOne);
		SWAP(type.referenceListOffset);

		const ReferenceListItem* rli = reinterpret_cast<const ReferenceListItem*>(
			reinterpret_cast<const uint8_t*>(t) + type.referenceListOffset
		);

		ResourceTypeData resourceTypeData;
		resourceTypeData.type = type.resourceType;

		for (int j = 0; j <= type.numOfThisTypeMinusOne; j++, rli++)
		{
			ReferenceListItem refListItem = *rli;
			ResourceData res;

			SWAP(refListItem.resourceID);
			SWAP(refListItem.resourceNameListOffset);
			SWAP(refListItem.resourceDataOffset);
			
			const char* pascalStringName = reinterpret_cast<const char*>(mem + header.resourceMapOffset + resourceMapHeader.nameListOffset + refListItem.resourceNameListOffset);

			const uint8_t* dataHeader = mem + header.resourceDataOffset + refListItem.resourceDataOffset;
			uint32_t dataLength = *reinterpret_cast<const uint32_t*>(dataHeader);

			SWAP(dataLength);

			res.name.assign(pascalStringName+1, *pascalStringName);

			res.dataBacking.assign(dataHeader + 4, dataHeader + 4 + dataLength);

			res.id = refListItem.resourceID;
			res.attributes = refListItem.attributes;
			res.type = resourceTypeData.type;
			res.parent = this;

			resourceTypeData.resources.emplace_back(std::make_unique<ResourceData>(std::move(res)));

			ResourceData* d = resourceTypeData.resources.back().get();
			resourceTypeData.idMap.insert({ res.id, d });
			resourceTypeData.nameMap.insert({ res.name, d });

			if (m_createHandles)
				d->getHandle();
		}

		m_resourceTypes.insert({ type.resourceType, std::move(resourceTypeData) });
	}
}

void Resources::setCreateHandles(bool create)
{
	m_createHandles = create;
}

void Resources::add(ResType type, ResID id, const char* name, Handle data)
{
	auto itType = m_resourceTypes.find(type);
	if (itType == m_resourceTypes.end())
	{
		itType = m_resourceTypes.insert({ type, ResourceTypeData{ type } }).first;
	}

	ResourceData rd(data);

	rd.id = id;
	rd.type = type;
	rd.name = name;
	rd.attributes = 0;
	rd.parent = this;

	itType->second.resources.push_back(std::make_unique<ResourceData>(std::move(rd)));

	ResourceData* rdPtr = itType->second.resources.back().get();
	itType->second.idMap.insert({ id, rdPtr });
	itType->second.nameMap.insert({ name, rdPtr });
	m_handleMap.insert({ data, rdPtr });
}

void Resources::remove(Handle data)
{
	auto it = m_handleMap.find(data);
	if (it == m_handleMap.end())
		return;

	auto itType = m_resourceTypes.find(it->second->type);
	if (itType == m_resourceTypes.end())
		return;

	itType->second.idMap.erase(it->second->id);
	itType->second.nameMap.erase(it->second->name);
	m_handleMap.erase(data);

	auto& resources = itType->second.resources;
	for (size_t i = 0; i < resources.size(); i++)
	{
		if (resources[i].get() == it->second)
		{
			resources.erase(resources.begin() + i);
			break;
		}
	}
}

Resources::ResourceData* Resources::dataByHandle(Handle h)
{
	auto it = m_handleMap.find(h);
	if (it == m_handleMap.end())
		return nullptr;
	return it->second;
}

void Resources::save()
{
	// TODO
}
