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


#ifndef MACHOIMPL_H
#define MACHOIMPL_H
#include "MachO.h"
#include <mach-o/loader.h>
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

	template <class segment_command, class section>
		void readSegment(char* cmds_ptr, std::vector<segment_command*>* segments, std::vector<section*>* bind_sections);
		
	void readRebase(const uint8_t* p, const uint8_t* end);
	void readBind(const uint8_t* p, const uint8_t* end, bool is_weak);
	void readExport(const uint8_t* start, const uint8_t* p, const uint8_t* end, std::string* name_buf);

	template <class section>
		void readClassicBind(const section& sec, uint32_t* dysyms, uint32_t* symtab, const char* symstrtab);

	char* m_mapped;
	size_t m_mapped_size;
	bool m_need_exports;
	
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

#endif
