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
along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef FILEMAP_H
#define FILEMAP_H
#include <stdint.h>
#include <string>
#include <map>
#include <list>
#include "MachO.h"
#include <dlfcn.h>
#include "../util/mutex.h"

class FileMap
{
public:
	~FileMap();
	
	struct ImageMap;

	const ImageMap* add(const MachO& mach, uintptr_t slide, uintptr_t base, bool bindLazy);

	void addWatchDog(uintptr_t addr);

	struct ImageMap
	{
		std::string filename;
		std::map<uintptr_t, std::string> symbols;
		uintptr_t base, slide;
		mach_header* header;
		std::pair<uint64_t,uint64_t> eh_frame;
		std::pair<uint64_t,uint64_t> unwind_info;
		std::vector<MachO::Section> sections;
		std::vector<std::string> rpaths;

		std::list<MachO::Bind> lazy_binds;
		mutable Darling::Mutex mutex_lazy_binds;
	};
	
	const ImageMap* imageMapForAddr(const void* p) const;
	const ImageMap* imageMapForHeader(const mach_header* p) const;
	// const ImageMap* imageMapForName(const std::string& name) const;
	const char* fileNameForAddr(const void* p) const;
	const char* gdbInfoForAddr(const void* p) const;
	bool findSymbolInfo(const void* addr, Dl_info* p) const; // used by __darwin_dladdr

	const std::vector<ImageMap*>& images() const { return m_maps_vec; }
	const ImageMap* mainExecutable() const;

private:

	std::map<uintptr_t, ImageMap*> m_maps;
	std::map<mach_header*, ImageMap*> m_maps_mach;
	std::vector<ImageMap*> m_maps_vec; // for easier access
	mutable char m_dumped_stack_frame_buf[4096];
};


#endif
