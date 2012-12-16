/*
This file is part of Darling.

Copyright (C) 2012 Lubos Dolezel
Copyright (C) 2011 Shinichiro Hamaji

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

#include "FileMap.h"
#include <cassert>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <sstream>

FileMap::~FileMap()
{
	for (auto m : m_maps)
	{
		if (m.second)
			delete m.second->header;
		delete m.second;
	}
}

const FileMap::ImageMap* FileMap::add(const MachO& mach, uintptr_t slide, uintptr_t base, bool bindLazy)
{
	ImageMap* symbol_map = new ImageMap;

	symbol_map->filename = mach.filename();
	symbol_map->base = base;
	symbol_map->slide = slide;
	
	// This needs to be allocated dynamically, because its memory location must be permanent.
	// It is used in certain silly APIs as a handle.
	symbol_map->header = new mach_header;
	*symbol_map->header = mach.header();
	
	symbol_map->eh_frame = mach.get_eh_frame();
	symbol_map->unwind_info = mach.get_unwind_info();
	symbol_map->sections = mach.sections();
	
	if (!bindLazy)
	{
		for (auto* b : mach.binds())
		{
			if (!b->is_lazy)
				continue;
			symbol_map->lazy_binds.push_back(*b);
		}
	}

	if (!m_maps.insert(std::make_pair(base, symbol_map)).second)
	{
		std::stringstream ss;
		ss << "dupicated base addr: " << (void*) base << " in " << mach.filename();
		throw std::runtime_error(ss.str());
	}

	m_maps_vec.push_back(symbol_map);
	m_maps_mach[symbol_map->header] = symbol_map;

	for (MachO::Symbol sym : mach.symbols())
	{
		if (sym.name.empty() || sym.name[0] != '_')
			continue;
		sym.addr += slide;
		if (sym.addr < base)
			continue;
		symbol_map->symbols.insert(std::make_pair(sym.addr, sym.name.substr(1)));
	}
	for (const char* rpath : mach.rpaths())
		symbol_map->rpaths.push_back(rpath);

	return symbol_map;
}

void FileMap::addWatchDog(uintptr_t addr)
{
	bool r = m_maps.insert(std::make_pair(addr, (ImageMap*)NULL)).second;
	assert(r);
}

const char* FileMap::gdbInfoForAddr(const void* p) const
{
	Dl_info i;
	
	if (!findSymbolInfo(p, &i))
		return 0;
	
	if (i.dli_fbase)
	{
#ifdef __i386__
		const char* fmt_string = "0x%08lx in %s+%x () from %s";
#else
		const char* fmt_string = "0x%016lx in %s%x () from %s";
#endif
	
		ptrdiff_t func_offset = uintptr_t(p) - uintptr_t(i.dli_saddr);
		snprintf(m_dumped_stack_frame_buf, sizeof(m_dumped_stack_frame_buf)-1, fmt_string, p, i.dli_sname, func_offset, i.dli_fname);
	}
	else
	{
#ifdef __i386__
		const char* fmt_string = "0x%08lx in ?? () from %s";
#else
		const char* fmt_string = "0x%016lx in ?? () from %s";
#endif
		snprintf(m_dumped_stack_frame_buf, sizeof(m_dumped_stack_frame_buf)-1, fmt_string, p, i.dli_fname);
	}
	
	return m_dumped_stack_frame_buf;
}

const FileMap::ImageMap* FileMap::mainExecutable() const
{
	assert(!m_maps_vec.empty());
	return m_maps_vec[0];
}

const char* FileMap::fileNameForAddr(const void* p) const
{
	const ImageMap* map = imageMapForAddr(p);
	if (!map)
		return 0;
	return map->filename.c_str();
}

const FileMap::ImageMap* FileMap::imageMapForAddr(const void* p) const
{
	uintptr_t addr = reinterpret_cast<uintptr_t>(p);
	const ImageMap* symbol_map;
	
	std::map<uintptr_t, ImageMap*>::const_iterator found = m_maps.upper_bound(addr);
	if (found == m_maps.begin() || found == m_maps.end())
		return 0;
	
	--found;
	return found->second;
}

const FileMap::ImageMap* FileMap::imageMapForHeader(const mach_header* p) const
{
	auto it = m_maps_mach.find(const_cast<mach_header*>(p));
	if (it == m_maps_mach.end())
		return nullptr;
	else
		return it->second;
}

/*
const FileMap::ImageMap* FileMap::imageMapForName(const std::string& name) const
{
	auto it = m_maps.find(name);
	if (it == m_maps.end())
		return nullptr;
	else
		return it->second;
}
*/

bool FileMap::findSymbolInfo(const void* p, Dl_info* info) const
{
	uintptr_t addr = reinterpret_cast<uintptr_t>(p);
	const ImageMap* symbol_map;
	
	std::map<uintptr_t, ImageMap*>::const_iterator found = m_maps.upper_bound(addr);
	if (found == m_maps.begin() || found == m_maps.end())
		return false;
	
	--found;
	symbol_map = found->second;
	
	std::map<uintptr_t, std::string>::const_iterator sfound = symbol_map->symbols.lower_bound(addr);
	if (sfound != symbol_map->symbols.begin())
	{
		// found in lib and in symbols
		info->dli_sname = sfound->second.c_str();
		info->dli_saddr = reinterpret_cast<void*>(sfound->first);
	}
	
	info->dli_fname = symbol_map->filename.c_str();
	info->dli_fbase = reinterpret_cast<void*>(symbol_map->base);
	
	return true;
}

