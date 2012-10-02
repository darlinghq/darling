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


#ifndef MACH_O_H_
#define MACH_O_H_

#include <stdint.h>

#include <string>
#include <vector>
#include <stdexcept>

#include <mach/vm_types.h>
#include <mach-o/loader.h>

class MachO
{
public:
	__attribute__ ((visibility ("default")))
	static MachO* readFile(std::string path, const char* arch, bool need_exports = true);
	__attribute__ ((visibility ("default")))
	static bool isMachO(const char* path);

	virtual ~MachO() {}
	virtual void close() = 0;

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
		uint8_t ordinal;
		bool is_weak;
		bool is_classic;
	};

	struct Export
	{
		std::string name;
		uint64_t addr;
		uint32_t flag;
	};

	struct Symbol
	{
		std::string name;
		uint64_t addr;
	};
	
	struct Section
	{
		std::string segment, section;
		uintptr_t addr, size;
	};

	const std::vector<segment_command_64*>& segments64() const { return m_segments64; }

	const std::vector<segment_command*>& segments() const { return m_segments; }

	const std::vector<const char*>& dylibs() const { return m_dylibs; }

	const std::vector<Rebase*>& rebases() const { return m_rebases; }

	const std::vector<Bind*>& binds() const { return m_binds; }

	const std::vector<Export*>& exports() const { return m_exports; }

	const std::vector<Symbol>& symbols() const { return m_symbols; }

	const char* base() const { return m_base; }

	uint64_t entry() const { return m_entry; }
	uint64_t main() const { return m_main; }

	const std::vector<uint64_t>& init_funcs() const { return m_init_funcs; }
	const std::vector<uint64_t>& exit_funcs() const { return m_exit_funcs; }
	
	std::pair<uint64_t,uint64_t> get_eh_frame() const { return m_eh_frame; }
	std::pair<uint64_t,uint64_t> get_unwind_info() const { return m_unwind_info; }

	uint64_t dyld_data() const { return m_dyld_data; }

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
	std::vector<Rebase*> m_rebases;
	std::vector<Bind*> m_binds;
	std::vector<Export*> m_exports;
	std::vector<Symbol> m_symbols;
	const char* m_base;
	uint64_t m_entry, m_main;
	std::vector<uint64_t> m_init_funcs;
	std::vector<uint64_t> m_exit_funcs;
	uint64_t m_dyld_data;
	std::pair<uint64_t,uint64_t> m_eh_frame;
	std::pair<uint64_t,uint64_t> m_unwind_info;
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
private:
	std::vector<std::string> m_archs;
};

#endif	// MACH_O_H_
