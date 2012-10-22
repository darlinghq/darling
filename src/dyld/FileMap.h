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
#include "MachO.h"
#include <dlfcn.h>

class FileMap
{
public:
	~FileMap();
	
	struct ImageMap;

	const ImageMap* add(const MachO& mach, uintptr_t slide, uintptr_t base);

	void addWatchDog(uintptr_t addr);

	struct ImageMap
	{
		std::string filename;
		std::map<uintptr_t, std::string> symbols;
		uintptr_t base, slide;
		mach_header header;
		std::pair<uint64_t,uint64_t> eh_frame;
		std::pair<uint64_t,uint64_t> unwind_info;
		std::vector<MachO::Section> sections;
	};
	
	const ImageMap* imageMapForAddr(const void* p);
	const char* fileNameForAddr(const void* p);
	const char* gdbInfoForAddr(const void* p);
	bool findSymbolInfo(const void* addr, Dl_info* p); // used by __darwin_dladdr

	const std::vector<ImageMap*>& images() const { return m_maps_vec; }

private:

	std::map<uintptr_t, ImageMap*> m_maps;
	std::vector<ImageMap*> m_maps_vec; // for easier access
	char m_dumped_stack_frame_buf[4096];
};


#endif
