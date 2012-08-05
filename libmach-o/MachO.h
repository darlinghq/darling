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

#include "mach-o/loader.h"

class MachO
{
public:
	static MachO* readFile(const char* path, const char* arch, bool need_exports = true);

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
		string std::name;
		uint64_t addr;
		uint32_t flag;
	};

	struct Symbol
	{
		string std::name;
		uint64_t addr;
	};

	const std::vector<segment_command_64*>& segments64() const {
		return m_segments64;
	}

	const std::vector<segment_command*>& segments() const {
		return m_segments;
	}

	const std::vector<const char*>& dylibs() const { return m_dylibs; }

	const std::vector<Rebase*>& rebases() const { return m_rebases; }

	const std::vector<Bind*>& binds() const { return m_binds; }

	const std::vector<Export*>& exports() const { return m_exports; }

	const std::vector<Symbol>& symbols() const { return m_symbols; }

	const char* base() const { return m_base; }

	uint64_t entry() const { return m_entry; }

	const std::vector<uint64_t>& init_funcs() const { return m_init_funcs; }
	const std::vector<uint64_t>& exit_funcs() const { return m_exit_funcs; }

	uint64_t dyld_data() const { return dyld_data_; }

	bool is64() const { return m_is64; }

	int fd() const { return m_fd; }
	size_t offset() const { return m_offset; }

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
	uint64_t m_entry;
	std::vector<uint64_t> m_init_funcs;
	std::vector<uint64_t> m_exit_funcs;
	uint64_t m_dyld_data;
	bool m_is64;
	int m_ptrsize;
	int m_fd;
	size_t m_offset;
};

#endif	// MACH_O_H_
