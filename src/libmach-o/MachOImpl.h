/*
This file is part of Darling.

Copyright (C) 2012-2013 Lubos Dolezel
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

#ifndef MACHOIMPL_H
#define MACHOIMPL_H
#include "MachO.h"
#include <mach-o/loader.h>
#include <mach-o/reloc.h>
#include <stdint.h>
#include <vector>
#include <string>

class MachOImpl : public MachO
{
public:
	// Takes ownership of fd.
	// If len is 0, the size of file will be used as len.
	MachOImpl(const char* filename, int fd, size_t offset, size_t len, bool need_exports);
	
	virtual ~MachOImpl();
	virtual void close();
	
	typedef long long ll;
	typedef unsigned long long ull;

private:
	friend class RebaseState;
	friend class BindState;

	void processLoaderCommands(const mach_header* header);

	template <class segment_command, class section>
		void readSegment(char* cmds_ptr, std::vector<segment_command*>* segments, std::vector<section*>* bind_sections);
		
	void readRebase(const uint8_t* p, const uint8_t* end);
	void readBind(const uint8_t* p, const uint8_t* end, bool is_weak, bool is_lazy = false);
	void readExport(const uint8_t* start, const uint8_t* p, const uint8_t* end, std::string* name_buf);

	template <class section>
		void readClassicBind(const section& sec, uint32_t* dysyms, uint32_t* symtab, const char* symstrtab);
	void readStubBind(const section& sec,  uint32_t* dysyms, uint32_t* symtab, const char* symstrtab);

	// classic
	void readExternalRelocation(const struct relocation_info* reloc, uint32_t* symtab, const char* symstrtab);
	// classic
	void readInternalRelocation(const struct relocation_info* reloc);

	char* m_mapped;
	size_t m_mapped_size;
	bool m_need_exports;
	intptr_t m_text_offset;
	
	struct sym
	{
		uint32_t name;
		uint32_t addr;
		uint32_t flag;
	};

	struct sym64
	{
		uint32_t name;
		uint64_t addr;
		uint32_t flag;
	};

	// See mach-o/nlist.h for this layout
	struct nlist
	{
		uint32_t n_strx;
		uint8_t n_type;
		uint8_t n_sect;
		uint16_t n_desc;
		uint64_t n_value;
	};
};

#define LC_MAIN (0x28|LC_REQ_DYLD)
struct entry_point_command {
	uint32_t  cmd;  /* LC_MAIN only used in MH_EXECUTE filetypes */
	uint32_t  cmdsize;      /* 24 */
	uint64_t  entryoff;     /* file (__TEXT) offset of main() */
	uint64_t  stacksize;/* if not zero, initial stack size */
};

#endif
