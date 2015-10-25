/*
This file is part of Darling.

Copyright (C) 2012-2013 Lubos Dolezel

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

#include "darling-config.h"
#include "dyld_public.h"
#include "MachOMgr.h"
#include "MachOObject.h"
#include <util/debug.h>
#include <cstring>
#include <cstdlib>
#include <map>
#include <set>
#include <link.h>
#include <stddef.h>
#include "../util/log.h"
#include "../util/leb.h"

using namespace Darling;

uint32_t _dyld_image_count(void)
{
	return MachOMgr::instance()->objectCount();
}

const struct ::mach_header* _dyld_get_image_header(uint32_t image_index)
{
	return MachOMgr::instance()->objectByIndex(image_index)->getMachHeader();
}

intptr_t _dyld_get_image_vmaddr_slide(uint32_t image_index)
{
	return MachOMgr::instance()->objectByIndex(image_index)->slide();
}

intptr_t _dyld_get_image_slide(const struct mach_header* mh)
{
	MachOObject* obj = MachOMgr::instance()->objectByHeader((mach_header*) mh);
	if (obj != nullptr)
		return obj->slide();
	return 0;
}

const char* _dyld_get_image_name(uint32_t image_index)
{
	return MachOMgr::instance()->objectByIndex(image_index)->path().c_str();
}

char* getsectdata(const struct mach_header* header, const char* segname, const char* sectname, unsigned long* size)
{
	MachOObject* obj = MachOMgr::instance()->objectByHeader((mach_header*) header);

	if (!obj || !sectname)
	{
		if (size)
			*size = 0;
		return nullptr;
	}
	if (!segname)
		segname = "";

	return (char*) obj->getSection(segname, sectname, (uintptr_t*) size);
}

void _dyld_register_func_for_add_image(MachOMgr::LoaderHookFunc* func)
{
	MachOMgr::instance()->registerLoadHook(func);
}

void _dyld_register_func_for_remove_image(MachOMgr::LoaderHookFunc* func)
{
	MachOMgr::instance()->registerUnloadHook(func);
}

void _dyld_deregister_func_for_add_image(MachOMgr::LoaderHookFunc* func)
{
	if (!MachOMgr::isTerminated()) // MachOMgr singleton may be destroyed before modules are unloaded
		MachOMgr::instance()->deregisterLoadHook(func);
}

void _dyld_deregister_func_for_remove_image(MachOMgr::LoaderHookFunc* func)
{
	if (!MachOMgr::isTerminated()) // MachOMgr singleton may be destroyed before modules are unloaded
		MachOMgr::instance()->deregisterUnloadHook(func);
}


const char* dyld_image_path_containing_address(const void* addr)
{
	MachOObject* module = MachOMgr::instance()->objectForAddress((void*) addr);
	if (!module)
		return nullptr;
	else
		return module->path().c_str();
}


bool _dyld_find_unwind_sections(void* addr, struct dyld_unwind_sections* info)
{
#if 0
	TRACE1(addr);
	const FileMap::ImageMap* map = g_file_map.imageMapForAddr(addr);

	if (!map) // in ELF
	{
		memset(info, 0, sizeof(*info));

		CBData data = { addr, info };

		dl_iterate_phdr(dlCallback, &data);
		std::cout << "Dwarf section at " << info->dwarf_section << std::endl;
		return info->dwarf_section != 0;
	}
	else // in Mach-O
	{
		info->mh = map->header;
		info->dwarf_section = reinterpret_cast<const void*>(map->eh_frame.first + map->slide);
		info->dwarf_section_length = map->eh_frame.second;

		// FIXME: we would get "malformed __unwind_info" warnings otherwise
		// info->compact_unwind_section = reinterpret_cast<const void*>(map->unwind_info.first + map->slide);
		// info->compact_unwind_section_length = map->unwind_info.second;
		info->compact_unwind_section = 0;
		info->compact_unwind_section_length = 0;

		return true;
	}
#endif
	return false;
}

int32_t NSVersionOfRunTimeLibrary(const char* libraryName)
{
	return -1;
}

int32_t NSVersionOfLinkTimeLibrary(const char* libraryName)
{
	return -1;
}

int _NSGetExecutablePath(char* buf, unsigned int* size)
{
	std::string path;

	MachOObject* mainModule = MachOMgr::instance()->mainModule();

	if (mainModule)
		path = mainModule->path();

	if (*size > path.length()+1)
		*size = path.length()+1;

	strncpy(buf, path.c_str(), *size);
	buf[(*size)-1] = 0;

	return 0;
}

void __dyld_make_delayed_module_initializer_calls()
{
}

void __dyld_mod_term_funcs()
{
}

bool _dyld_bind_fully_image_containing_address(const void* address)
{
	return __dyld_bind_fully_image_containing_address(address);
}

bool __dyld_bind_fully_image_containing_address(const void* address)
{
	STUB();
	return true;
}

