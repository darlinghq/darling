#include "config.h"
#include "public.h"
#include "MachOLoader.h"
#include "FileMap.h"
#include "trace.h"
#include <cstring>
#include <cstdlib>
#include <map>
#include <link.h>
#include <stddef.h>
#include "../libmach-o/leb.h"

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
	// This would be needed for the GCC libunwind
}

void _dyld_register_func_for_remove_image(void (*func)(const struct mach_header* mh, intptr_t vmaddr_slide))
{
	// This would be needed for the GCC libunwind
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

struct CBData
{
	void* addr;
	struct dyld_unwind_sections* info;
};

#pragma pack(1)
struct eh_frame_hdr
{
	uint8_t version, eh_frame_ptr_enc, fde_count_enc, table_enc;
	uint8_t eh_frame_ptr[];
};
#pragma pack()

static uintptr_t readEncodedPointer(const eh_frame_hdr* hdr)
{
	uint8_t format = hdr->eh_frame_ptr_enc & 0xf;
	uint8_t rel = hdr->eh_frame_ptr_enc & 0xf0;
	uintptr_t val;
	bool isSigned = false;

	if (hdr->eh_frame_ptr_enc == 0xff)
		return 0;

	switch (format)
	{
		case 1: // unsigned LEB
		{
			const uint8_t* ptr = reinterpret_cast<const uint8_t*>(hdr->eh_frame_ptr);
			val = uleb128(ptr);
			break;
		}
		case 2: // 2 bytes
			val = *reinterpret_cast<const uint16_t*>(hdr->eh_frame_ptr);
			break;
		case 3:
			val = *reinterpret_cast<const uint32_t*>(hdr->eh_frame_ptr);
			break;
		case 4:
			val = *reinterpret_cast<const uint64_t*>(hdr->eh_frame_ptr);
			break;
		case 9: // signed LEB
		{
			const uint8_t* ptr = reinterpret_cast<const uint8_t*>(hdr->eh_frame_ptr);
			val = sleb128(ptr);
			break;
		}
		// FIXME: add 'dlpi_addr' (base address) to these?
		case 0xa:
			val = *reinterpret_cast<const int16_t*>(hdr->eh_frame_ptr);
			break;
		case 0xb:
			val = *reinterpret_cast<const int32_t*>(hdr->eh_frame_ptr);
			break;
		case 0xc:
			val = *reinterpret_cast<const int64_t*>(hdr->eh_frame_ptr);
			break;
		default:
			return 0;
	}

	switch (rel)
	{
		case 0: // no change
			break;
		case 0x10: // pcrel
			val += reinterpret_cast<uintptr_t>(hdr) + 4;
			break;
		case 0x30: // eh_frame_hdr rel
			val += reinterpret_cast<uintptr_t>(hdr);
			break;
		default:
			return 0;
	}

	return val;
}

static int dlCallback(struct dl_phdr_info *info, size_t size, void *data)
{
	CBData* cbdata = static_cast<CBData*>(data);
	bool addrMatch = false;
	void* maxAddr = 0;
	const eh_frame_hdr* ehdr = 0;

	if (cbdata->info->dwarf_section) // we already have a match
		return 0;

	//std::cout << "Looking into " << info->dlpi_name << std::endl;

	if (size < offsetof(struct dl_phdr_info, dlpi_phnum))
		return 0;

	for (int i = 0; i < info->dlpi_phnum; i++)
	{
		const ElfW(Phdr)* phdr = &info->dlpi_phdr[i];
		/*
		if (strstr(info->dlpi_name, "cxxdarwin"))
		{
			std::cout << "type: " << phdr->p_type << "; vaddr: " << phdr->p_vaddr << std::endl;
		}
		*/
		
		if (phdr->p_type == PT_LOAD)
		{
			void* from = reinterpret_cast<void*>(uintptr_t(info->dlpi_addr) + uintptr_t(phdr->p_vaddr));
			void* to = reinterpret_cast<char*>(from) + phdr->p_memsz;

			if (cbdata->addr >= from && cbdata->addr < to)
				addrMatch = true;
			if (to > maxAddr)
				maxAddr = to; // TODO: could this be improved? libunwind does the same
		}
		else if (phdr->p_type == PT_GNU_EH_FRAME)
		{
			//std::cout << "Found .eh_frame_hdr in " << info->dlpi_name << std::endl;
			ehdr = reinterpret_cast<eh_frame_hdr*>(uintptr_t(info->dlpi_addr) + phdr->p_vaddr);
			// cbdata->info->dwarf_section_length = phdr->p_memsz;
		}
	}

	if (addrMatch && ehdr)
	{
		//std::cout << "*** Match found! " << info->dlpi_name << std::endl;
		
		// Now we find .eh_frame from .eh_frame_hdr
		if (ehdr->version != 1)
			return 0;
		cbdata->info->dwarf_section = reinterpret_cast<void*>(readEncodedPointer(ehdr));
		cbdata->info->dwarf_section_length = uintptr_t(maxAddr) - uintptr_t(cbdata->info->dwarf_section);
	}

	return 0;
}

bool _dyld_find_unwind_sections(void* addr, struct dyld_unwind_sections* info)
{
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
		info->mh = &map->header;
		info->dwarf_section = reinterpret_cast<const void*>(map->eh_frame.first + map->slide);
		info->dwarf_section_length = map->eh_frame.second;

		// FIXME: we would get "malformed __unwind_info" warnings otherwise
		// info->compact_unwind_section = reinterpret_cast<const void*>(map->unwind_info.first + map->slide);
		// info->compact_unwind_section_length = map->unwind_info.second;
		info->compact_unwind_section = 0;
		info->compact_unwind_section_length = 0;

		return true;
	}
}

