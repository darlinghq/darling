#include "config.h"
#include "public.h"
#include "MachOLoader.h"
#include "FileMap.h"
#include "trace.h"
#include <cstring>
#include <cstdlib>

extern FileMap g_file_map;
extern "C" char* dyld_getDarwinExecutablePath();

uint32_t _dyld_image_count(void)
{
	return g_file_map.images().size();
}

const struct mach_header* _dyld_get_image_header(uint32_t image_index)
{
	return &g_file_map.images().at(image_index)->header;
}

intptr_t _dyld_get_image_vmaddr_slide(uint32_t image_index)
{
	return g_file_map.images().at(image_index)->slide;
}

const char* _dyld_get_image_name(uint32_t image_index)
{
	return g_file_map.images().at(image_index)->filename.c_str();
}

void _dyld_register_func_for_add_image(void (*func)(const struct mach_header* mh, intptr_t vmaddr_slide))
{
}

void _dyld_register_func_for_remove_image(void (*func)(const struct mach_header* mh, intptr_t vmaddr_slide))
{
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
	strncpy(buf, dyld_getDarwinExecutablePath(), (*size)-1);
	buf[(*size)-1] = 0;

	*size = strlen(buf);

	return 0;
}

void __assert_rtn(const char* func, const char* file, int line, const char* failedexpr)
{
	fprintf(stderr, "Assertion failed: (%s), function %s, file %s, line %d.\n", failedexpr, func, file, line);
	abort();
}

const char* dyld_image_path_containing_address(const void* addr)
{
	return g_file_map.fileNameForAddr(addr);
}

bool _dyld_find_unwind_sections(void* addr, struct dyld_unwind_sections* info)
{
	TRACE1(addr);
	const FileMap::ImageMap* map = g_file_map.imageMapForAddr(addr);
	if (!map)
	{
		Dl_info dinfo;
		if (dladdr(addr, &dinfo))
		{
			LOG << "The address given belongs to " << dinfo.dli_fname << ", sym " << dinfo.dli_sname << std::endl;
			if (strstr(dinfo.dli_fname, "/libcxxdarwin.so"))
			{
				return _dyld_find_unwind_sections((void*) 0x100000b60, info);
				LOG << "Returning fake data\n";
				memset(info, 0, sizeof(*info));
				return true;
			}
		}
		return false;
	}

	info->mh = &map->header;
	info->dwarf_section = reinterpret_cast<const void*>(map->eh_frame.first + map->slide);
	info->dwarf_section_length = map->eh_frame.second;
	info->compact_unwind_section = reinterpret_cast<const void*>(map->unwind_info.first + map->slide);
	info->compact_unwind_section_length = map->unwind_info.second;

	return true;
}

