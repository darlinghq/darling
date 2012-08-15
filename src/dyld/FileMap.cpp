// Copyright 2011 Shinichiro Hamaji. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//   1. Redistributions of source code must retain the above copyright
//      notice, this list of  conditions and the following disclaimer.
//
//   2. Redistributions in binary form must reproduce the above
//      copyright notice, this list of conditions and the following
//      disclaimer in the documentation and/or other materials
//      provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY Shinichiro Hamaji ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Shinichiro Hamaji OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
// USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
// OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
// SUCH DAMAGE.

#include "FileMap.h"
#include <cassert>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <err.h>

void FileMap::add(const MachO& mach, uintptr_t slide, uintptr_t base)
{
	SymbolMap* symbol_map = new SymbolMap();
	symbol_map->filename = mach.filename();
	symbol_map->base = base;
	if (!m_maps.insert(std::make_pair(base, symbol_map)).second)
	{
		err(1, "dupicated base addr: %p in %s",
		(void*)base, mach.filename().c_str());
	}

	for (size_t i = 0; i < mach.symbols().size(); i++)
	{
		MachO::Symbol sym = mach.symbols()[i];
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

const char* FileMap::dumpSymbol(void* p)
{
	uintptr_t addr = reinterpret_cast<uintptr_t>(p);
	std::map<uintptr_t, SymbolMap*>::const_iterator found = m_maps.upper_bound(addr);
	if (found == m_maps.begin() || found == m_maps.end())
	{
		return 0;
	}

	--found;
	return dumpSymbolFromMap(*found->second, addr);
}

const char* FileMap::dumpSymbolFromMap(const SymbolMap& symbol_map, uintptr_t addr)
{
	uintptr_t file_offset = addr - symbol_map.base;

	// Use lower_bound as PC may be in just after call.
	std::map<uintptr_t, std::string>::const_iterator found = symbol_map.symbols.lower_bound(addr);
	if (found == symbol_map.symbols.begin())
	{
#ifdef __i386__
		const char* fmt_string = "0x%08lx in ?? () from %s";
#else
		const char* fmt_string = "0x%016lx in ?? () from %s";
#endif
		snprintf(m_dumped_stack_frame_buf, 4095, fmt_string,
			(void*)addr, symbol_map.filename.c_str());
		return m_dumped_stack_frame_buf;
	}

	--found;
	
#ifdef __i386__
	const char* fmt_string = "0x%08lx in %s () from %s";
#else
	const char* fmt_string = "0x%016lx in %s () from %s";
#endif
	
	const char* name = found->second.c_str();
	uintptr_t func_offset = addr - found->first;
	snprintf(m_dumped_stack_frame_buf, 4095, fmt_string,
		(void*)addr, name, symbol_map.filename.c_str());
	return m_dumped_stack_frame_buf;
}
