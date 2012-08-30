
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
		delete m.second;
	}
}

void FileMap::add(const MachO& mach, uintptr_t slide, uintptr_t base)
{
	SymbolMap* symbol_map = new SymbolMap();
	symbol_map->filename = mach.filename();
	symbol_map->base = base;
	if (!m_maps.insert(std::make_pair(base, symbol_map)).second)
	{
		std::stringstream ss;
		ss << "dupicated base addr: " << (void*) base << " in " << mach.filename();
		throw std::runtime_error(ss.str());
	}

	for (MachO::Symbol sym : mach.symbols())
	{
		if (sym.name.empty() || sym.name[0] != '_')
			continue;
		sym.addr += slide;
		if (sym.addr < base)
			continue;
		symbol_map->symbols.insert(std::make_pair(sym.addr, sym.name.substr(1)));
	}
}

void FileMap::addWatchDog(uintptr_t addr)
{
	bool r = m_maps.insert(std::make_pair(addr, (SymbolMap*)NULL)).second;
	assert(r);
}

const char* FileMap::gdbInfoForAddr(void* p)
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

bool FileMap::findSymbolInfo(void* p, Dl_info* info)
{
	uintptr_t addr = reinterpret_cast<uintptr_t>(p);
	const SymbolMap* symbol_map;
	
	std::map<uintptr_t, SymbolMap*>::const_iterator found = m_maps.upper_bound(addr);
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

