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

#ifndef MACH_O_H_
#define MACH_O_H_

#include <stdint.h>

#include <string>
#include <vector>
#include <list>
#include <stdexcept>
#include <unistd.h>

	//#include <mach/vm_types.h>
#include "mach-o/loader.h"

class MachO
{
public:
	__attribute__ ((visibility ("default")))
	static MachO* readFile(std::string path, const char* arch, bool need_exports = true, bool loadAny = false);
	__attribute__ ((visibility ("default")))
	static bool isMachO(const char* path);

	__attribute__ ((visibility ("default")))
		const char* platform() const;

	virtual ~MachO() {}
	virtual void close() = 0;
	inline void closeFd() { ::close(m_fd); m_fd = -1; }

	const std::string& filename() const { return m_filename; }
	
	struct Rebase
	{
		uint64_t vmaddr;
		uint8_t type;
	};

	struct Bind
	{
		uint64_t vmaddr;
		std::string name;
		union
		{
			int64_t addend;
			uint64_t value;
		};
		uint8_t type;
		int ordinal;
		bool is_weak, is_lazy, is_classic, is_local;
		uintptr_t offset; // Needed to find the right bind when doing lazy binding

		enum { OrdinalSpecialSelf = 0, OrdinalSpecialExecutable = -1, OrdinalSpecialFlatLookup = -2 };
	};

	struct Export
	{
		std::string name;
		uint64_t addr;
		uint32_t flag;
		uint64_t resolver; // address of the resolver function, addr contains a stub
	};

	struct Symbol
	{
		std::string name;
		uint64_t addr;
		bool thumb; // arm
	};
	
	struct Section
	{
		std::string segment, section;
		uintptr_t addr, size;
	};

	struct Relocation
	{
		uint64_t addr;
		std::string name; // symbol name
		bool pcrel; // i386
		int ordinal;
	};

	struct TLVSection
	{
		uintptr_t firstDescriptor;
		unsigned long count;
	};

	const std::vector<segment_command_64*>& segments64() const { return m_segments64; }

	const std::vector<segment_command*>& segments() const { return m_segments; }

	const std::vector<const char*>& dylibs() const { return m_dylibs; }

	const std::vector<const char*>& rpaths() const { return m_rpaths; }

	const std::vector<Rebase*>& rebases() const { return m_rebases; }

	const std::vector<Bind*>& binds() const { return m_binds; }

	const std::vector<Export*>& exports() const { return m_exports; }

	const std::vector<Symbol>& symbols() const { return m_symbols; }
	const std::vector<Relocation*>& relocations() const { return m_relocations; }

	uintptr_t base() const { return m_base; }

	uint64_t entry() const { return m_entry; }
	uint64_t main() const { return m_main; }

	const std::list<uint64_t>& init_funcs() const { return m_init_funcs; }
	const std::list<uint64_t>& exit_funcs() const { return m_exit_funcs; }
	
	std::pair<uint64_t,uint64_t> get_eh_frame() const { return m_eh_frame; }
	std::pair<uint64_t,uint64_t> get_unwind_info() const { return m_unwind_info; }

	// TLS
	const std::vector<uint64_t>& tlv_init_funcs() const { return m_tlv_init_funcs; }
	std::pair<uint64_t,uint64_t> tlv_initial_values() const { return m_tlv; }
	const std::vector<TLVSection>& tlv_sections() const { return m_tlv_sections; }

	uint64_t dyld_data() const { return m_dyld_data; }
	
	__attribute__ ((visibility ("default")))
	uint64_t relocation_base() const;

	bool is64() const { return m_is64; }

	int fd() const { return m_fd; }
	size_t offset() const { return m_offset; }
	
	mach_header header() const { return m_header; }
	bool reverse_endian() const { return m_reverse_endian; } // true if the file has a different endianness than the current platform
	const std::vector<Section> sections() const { return m_sections; }
	
	inline uint32_t fixEndian(uint32_t i) const { return m_reverse_endian ? __builtin_bswap32(i) : i; }

 protected:
	std::string m_filename;
	std::vector<segment_command_64*> m_segments64;
	std::vector<segment_command*> m_segments;
	std::vector<const char*> m_dylibs;
	std::vector<const char*> m_rpaths;
	std::vector<Rebase*> m_rebases;
	std::vector<Bind*> m_binds;
	std::vector<Export*> m_exports;
	std::vector<Symbol> m_symbols;
	std::vector<Relocation*> m_relocations;
	uintptr_t m_base;
	uint64_t m_entry, m_main;
	std::list<uint64_t> m_init_funcs;
	std::list<uint64_t> m_exit_funcs;
	std::vector<uint64_t> m_tlv_init_funcs;
	std::vector<TLVSection> m_tlv_sections;
	uint64_t m_dyld_data;
	// first: address
	// second: length
	std::pair<uint64_t,uint64_t> m_eh_frame;
	std::pair<uint64_t,uint64_t> m_unwind_info;
	std::pair<uint64_t,uint64_t> m_tlv; // address contains initial values
	
	bool m_is64;
	int m_ptrsize;
	int m_fd;
	size_t m_offset;
	mach_header m_header;
	bool m_reverse_endian;
	std::vector<Section> m_sections;
};

class fat_architecture_not_supported : public std::exception
{
public:
	fat_architecture_not_supported(std::vector<std::string> archs) : m_archs(archs) {}
	inline const std::vector<std::string>& archs() const { return m_archs; }
	const char* what() const throw() { return "Given fat executables does not contain binary for current architecture"; }
private:
	std::vector<std::string> m_archs;
};

// we express stub binds as another type of ordinary bind
#define BIND_TYPE_STUB 100
#define BIND_TYPE_PCREL 101

#endif	// MACH_O_H_
