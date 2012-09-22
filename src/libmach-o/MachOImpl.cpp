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


#include "MachOImpl.h"
#include "log.h"
#include "RebaseState.h"
#include "BindState.h"
#include "leb.h"

#include <mach-o/loader.h>
#include <sys/mman.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

#define N_WEAK_DEF			0x0080
#define FLAGS_READ_SYMTAB	1
#define FLAGS_READ_DYSYMTAB	1

template <class section>
void MachOImpl::readClassicBind(const section& sec, uint32_t* dysyms, uint32_t* symtab, const char* symstrtab)
{
	uint32_t indirect_offset = sec.reserved1;
	int count = sec.size / m_ptrsize;
	
	for (int i = 0; i < count; i++)
	{
		uint32_t dysym = dysyms[indirect_offset + i];
		uint32_t index = dysym & 0x3fffffff;
		nlist* sym = (nlist*)(symtab + index * (m_is64 ? 4 : 3));

		MachO::Bind* bind = new MachO::Bind();
		bind->name = symstrtab + sym->n_strx;
		bind->vmaddr = sec.addr + i * m_ptrsize;
		bind->value = sym->n_value;
		bind->type = BIND_TYPE_POINTER;
		bind->ordinal = 1;
		bind->is_weak = ((sym->n_desc & N_WEAK_DEF) != 0);
		bind->is_classic = true;
		LOGF("add classic bind! %s type=%d sect=%d desc=%d value=%lld "
			"vmaddr=%p is_weak=%d\n",
			bind->name.c_str(), sym->n_type, sym->n_sect, sym->n_desc, (ll)sym->n_value,
			(void*)(bind->vmaddr), bind->is_weak);
		m_binds.push_back(bind);
    }
}


template <class segment_command, class section>
void MachOImpl::readSegment(char* cmds_ptr, std::vector<segment_command*>* segments, std::vector<section*>* bind_sections)
{
	segment_command* segment = reinterpret_cast<segment_command*>(cmds_ptr);
	segments->push_back(segment);

	LOGF("segment %s: vmaddr=%p vmsize=%llu "
		"fileoff=%llu filesize=%llu "
		"maxprot=%d initprot=%d nsects=%u flags=%u\n",
		segment->segname,
		(void*)(intptr_t)segment->vmaddr, (ull)segment->vmsize,
		(ull)segment->fileoff, (ull)segment->filesize,
		segment->maxprot, segment->initprot,
		segment->nsects, segment->flags);

	section* sections = reinterpret_cast<section*>(cmds_ptr + sizeof(segment_command));
	
	if (!strcmp(segment->segname, "__TEXT"))
		m_text_offset = (intptr_t) segment->vmaddr; // needed for LC_MAIN
	
	for (uint32_t j = 0; j < segment->nsects; j++)
	{
		const section& sec = sections[j];
		LOGF("section %s in %s: "
				"addr=%p size=%llu offset=%u align=%u "
				"reloff=%u nreloc=%u flags=%u "
				"reserved1=%u reserved2=%u\n",
				sec.sectname, sec.segname,
				(void*)(intptr_t)sec.addr, (ull)sec.size,
				sec.offset, sec.align,
				sec.reloff, sec.nreloc, sec.flags,
				sec.reserved1, sec.reserved2);

		if (!strcmp(sec.sectname, "__dyld") && !strcmp(sec.segname, "__DATA"))
			m_dyld_data = sec.addr;
		if (!strcmp(sec.segname, "__TEXT"))
		{
			if (!strcmp(sec.sectname, "__eh_frame"))
			{
				m_eh_frame.first = sec.addr;
				m_eh_frame.second = sec.size;
			}
			else if (!strcmp(sec.sectname, "__unwind_info"))
			{
				m_unwind_info.first = sec.addr;
				m_unwind_info.second = sec.size;
			}
		}

		int section_type = sec.flags & SECTION_TYPE;
		switch (section_type)
		{
		case S_REGULAR: // 0x0
			/* Regular section: nothing to do */
			break;

		case S_MOD_INIT_FUNC_POINTERS: // 0x9
		{
			for (uint64_t p = sec.addr; p < sec.addr + sec.size; p += m_ptrsize)
				m_init_funcs.push_back(p);
			
			break;
		}
		case S_MOD_TERM_FUNC_POINTERS: // 0xA
			for (uint64_t p = sec.addr; p < sec.addr + sec.size; p += m_ptrsize)
				m_exit_funcs.push_back(p);
			
			break;
		case S_NON_LAZY_SYMBOL_POINTERS: // 0x6
		case S_LAZY_SYMBOL_POINTERS: // 0x7
			bind_sections->push_back(sections + j);
			break;
		
		case S_ZEROFILL:
		case S_CSTRING_LITERALS: // 0x2
		case S_4BYTE_LITERALS: // 0x3
		case S_8BYTE_LITERALS: // 0x4
		case S_LITERAL_POINTERS: // 0x5
		case S_SYMBOL_STUBS: // 0x8, byte size of stub in reserved2
		case S_COALESCED:
		case S_GB_ZEROFILL:
		case S_INTERPOSING: // 0xD
		case S_16BYTE_LITERALS:
		case S_DTRACE_DOF:
		case S_LAZY_DYLIB_SYMBOL_POINTERS:
			LOGF("FIXME: section type %d will not be handled for %s in %s (%p)\n",
				section_type, sec.sectname, sec.segname, sec.addr);
			break;

		default:
			fprintf(stderr, "Unknown section type: %d\n", section_type);
			abort();
			break;
		}
	}
}

void MachOImpl::readRebase(const uint8_t* p, const uint8_t* end)
{
	RebaseState state(this);
	while (p < end)
	{
		if (!state.readRebaseOp(p))
			break;
	}
}


void MachOImpl::readBind(const uint8_t* p, const uint8_t* end, bool is_weak)
{
	BindState state(this, is_weak);
	while (p < end)
	{
		state.readBindOp(p);
	}
}

void MachOImpl::readExport(const uint8_t* start, const uint8_t* p, const uint8_t* end, std::string* name_buf)
{
	LOGF("readExport: %p-%p\n", p, end);
#if 0
  char buf[17];
  buf[16] = '\0';
  for (int i = 0; i < 16*8; i++) {
    LOGF("%02x ", p[i]);
    buf[i % 16] = p[i] < 32 ? '?' : p[i];
    if (i % 16 == 15) LOGF("%s\n", buf);
  }
#endif

	if (p >= end)
	{
		fprintf(stderr, "broken export trie\n"); // TODO: throw an exception instead
		exit(1);
	}

	if (uint8_t term_size = *p++)
	{
		const uint8_t* expected_term_end = p + term_size;
		Export* exp = new Export;
		exp->name = *name_buf;
		exp->flag = uleb128(p);
		exp->addr = uleb128(p);
		LOGF("export: %s %lu %p\n",
				name_buf->c_str(), (long)exp->flag, (void*)exp->addr);

		m_exports.push_back(exp);

		CHECK(expected_term_end == p);
	}

	const uint8_t num_children = *p++;
	for (uint8_t i = 0; i < num_children; i++)
	{
		size_t orig_name_size = name_buf->size();
		while (*p)
			name_buf->push_back(*p++);
		p++;

		uint64_t off = uleb128(p);
		CHECK(off != 0);
		readExport(start, start + off, end, name_buf);

		name_buf->resize(orig_name_size);
	}
}

MachOImpl::MachOImpl(const char* filename, int fd, size_t offset, size_t len, bool need_exports)
	: m_mapped(0), m_mapped_size(len)
{
	m_filename = filename;
	m_need_exports = need_exports;
	m_dyld_data = 0;
	CHECK(fd);
	m_fd = fd;
	m_offset = offset;
	m_text_offset = 0;
	m_main = m_entry = 0;

	if (!m_mapped_size)
		m_mapped_size = ::lseek(m_fd, 0, SEEK_END);

	::lseek(fd, 0, SEEK_SET);

	char* bin = m_mapped = reinterpret_cast<char*>(
		::mmap(NULL, m_mapped_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE, m_fd, offset)
	);
	
	if (bin == MAP_FAILED)
		throw std::runtime_error("Cannot mmap Mach-O file");
	
	m_base = bin;

	mach_header* header = reinterpret_cast<mach_header*>(bin);
	memcpy(&m_header, header, sizeof(*header));
	
	m_is64 = false;
	if (header->magic == MH_MAGIC_64)
		m_is64 = true;
	else if (header->magic == MH_CIGAM)
	{
		m_reverse_endian = true;
		throw std::runtime_error("Unsupported endianness"); // TODO: start swapping the byte order of all structures
	}
	else if (header->magic != MH_MAGIC)
		throw std::runtime_error("Not a Mach-O file");
	
	LOGF("magic=%x cpu=%d cpusub=%d file=%d ncmds=%d sizecmd=%d flags=%x\n",
		header->magic, header->cputype, header->cpusubtype,
		header->filetype, header->ncmds, header->sizeofcmds,
		header->flags);

	m_ptrsize = m_is64 ? 8 : 4;

	if ((header->cputype & 0x00ffffff) != CPU_TYPE_X86)
	{
		throw std::runtime_error("Unsupported CPU type in Mach-O");
	}
	
	// TODO: split into a method

	struct load_command* cmds_ptr = reinterpret_cast<struct load_command*>(
		bin + (m_is64 ? sizeof(mach_header_64) : sizeof(mach_header))
	);

	uint32_t* symtab = 0;
	uint32_t* dysyms = 0;
	const char* symstrtab = 0;
	dyld_info_command* dyinfo = 0;
	std::vector<section_64*> bind_sections_64;
	std::vector<section*> bind_sections_32;

	for (uint32_t ii = 0; ii < header->ncmds; ii++)
	{
		LOGF("cmd type:%x\n", cmds_ptr->cmd);

		switch (cmds_ptr->cmd)
		{
		case LC_SEGMENT_64:
			readSegment<segment_command_64, section_64>((char*)cmds_ptr, &m_segments64, &bind_sections_64);
			break;
		

		case LC_SEGMENT:
			readSegment<segment_command, section>((char*)cmds_ptr, &m_segments, &bind_sections_32);
			break;

		case LC_DYLD_INFO:
		case LC_DYLD_INFO_ONLY:
		{
			dyinfo = reinterpret_cast<dyld_info_command*>(cmds_ptr);
			LOGF("dyld info: rebase_off=%u rebase_size=%u "
				"bind_off=%u bind_size=%u "
				"weak_bind_off=%u weak_bind_size=%u "
				"lazy_bind_off=%u lazy_bind_size=%u "
				"export_off=%u export_size=%u\n",
				dyinfo->rebase_off, dyinfo->rebase_size,
				dyinfo->bind_off, dyinfo->bind_size,
				dyinfo->weak_bind_off, dyinfo->weak_bind_size,
				dyinfo->lazy_bind_off, dyinfo->lazy_bind_size,
				dyinfo->export_off, dyinfo->export_size);

			{
				const uint8_t* p = reinterpret_cast<uint8_t*>(
					bin + dyinfo->rebase_off);
				const uint8_t* end = p + dyinfo->rebase_size;
				if (dyinfo->rebase_off && dyinfo->rebase_size)
					readRebase(p, end);
			}

			{
				const uint8_t* p = reinterpret_cast<uint8_t*>(
					bin + dyinfo->bind_off);
				const uint8_t* end = p + dyinfo->bind_size;
				readBind(p, end, false);
			}

			{
				const uint8_t* p = reinterpret_cast<uint8_t*>(
					bin + dyinfo->lazy_bind_off);
				const uint8_t* end = p + dyinfo->lazy_bind_size;
				readBind(p, end, false);
			}

			{
				const uint8_t* p = reinterpret_cast<uint8_t*>(
					bin + dyinfo->weak_bind_off);
				const uint8_t* end = p + dyinfo->weak_bind_size;
				readBind(p, end, true);
			}

			if (m_need_exports)
			{
				const uint8_t* p = reinterpret_cast<uint8_t*>(
					bin + dyinfo->export_off);
				const uint8_t* end = p + dyinfo->export_size;
				if (dyinfo->export_off && dyinfo->export_size)
				{
					std::string buf;
					readExport(p, p, end, &buf);
				}
			}

			break;
		}

		case LC_SYMTAB:
		{
			symtab_command* symtab_cmd = reinterpret_cast<symtab_command*>(cmds_ptr);

			LOGF("symoff=%u nsysm=%u stroff=%u strsize=%u\n",
				symtab_cmd->symoff, symtab_cmd->nsyms,
				symtab_cmd->stroff, symtab_cmd->strsize);

			uint32_t* symtab_top = symtab = reinterpret_cast<uint32_t*>(bin + symtab_cmd->symoff);
			symstrtab = bin + symtab_cmd->stroff;

			if (FLAGS_READ_SYMTAB)
			{
				for (uint32_t i = 0; i < symtab_cmd->nsyms; i++)
				{
					Symbol sym;
					nlist* nl = (nlist*)symtab;
					sym.name = symstrtab + nl->n_strx;
					if (m_is64)
					{
						sym.addr = nl->n_value;
						symtab += 4;
					}
					else
					{
						sym.addr = (uint32_t)nl->n_value;
						symtab += 3;
					}

					LOGF("%d %s(%d) %p\n",
						i, sym.name.c_str(), nl->n_strx, (void*)sym.addr);
					m_symbols.push_back(sym);
				}
			}

			// Will be used by other load commands.
			symtab = symtab_top;

			break;
		}

		case LC_DYSYMTAB:
		{
			dysymtab_command* dysymtab_cmd = reinterpret_cast<dysymtab_command*>(cmds_ptr);

			LOGF("dysym:\n"
				" ilocalsym=%u nlocalsym=%u\n"
				" iextdefsym=%u nextdefsym=%u\n"
				" iundefsym=%u nundefsym=%u\n"
				" tocoff=%u ntoc=%u\n"
				" modtaboff=%u nmodtab=%u\n"
				" extrefsymoff=%u nextrefsyms=%u\n"
				" indirectsymoff=%u nindirectsyms=%u\n"
				" extreloff=%u nextrel=%u\n"
				" locreloff=%u nlocrel=%u\n"
				,
				dysymtab_cmd->ilocalsym, dysymtab_cmd->nlocalsym,
				dysymtab_cmd->iextdefsym, dysymtab_cmd->nextdefsym,
				dysymtab_cmd->iundefsym, dysymtab_cmd->nundefsym,
				dysymtab_cmd->tocoff, dysymtab_cmd->ntoc,
				dysymtab_cmd->modtaboff, dysymtab_cmd->nmodtab,
				dysymtab_cmd->extrefsymoff, dysymtab_cmd->nextrefsyms,
				dysymtab_cmd->indirectsymoff, dysymtab_cmd->nindirectsyms,
				dysymtab_cmd->extreloff, dysymtab_cmd->nextrel,
				dysymtab_cmd->locreloff, dysymtab_cmd->nlocrel);

			if (dysymtab_cmd->nindirectsyms)
			{
				dysyms = reinterpret_cast<uint32_t*>(
					bin + dysymtab_cmd->indirectsymoff);
			}
			if (FLAGS_READ_DYSYMTAB)
			{
				for (uint32_t j = 0; j < dysymtab_cmd->nindirectsyms; j++)
				{
					uint32_t dysym = dysyms[j];
					uint32_t index = dysym & 0x3fffffff;
					const char* local =
					(dysym & INDIRECT_SYMBOL_LOCAL) ? " local" : "";
					const char* abs =
					(dysym & INDIRECT_SYMBOL_ABS) ? " abs" : "";

					uint32_t* sym = symtab;
					sym += index * (m_is64 ? 4 : 3);

					LOGF("dysym %d %s(%u)%s%s\n", j, symstrtab + sym[0], index, local, abs);
				}

				uint32_t* dymods = reinterpret_cast<uint32_t*>( bin + dysymtab_cmd->modtaboff);
				for (uint32_t j = 0; j < dysymtab_cmd->nmodtab; j++)
					LOGF("dymods: %u\n", dymods[j]);
			}

			break;
		}

		case LC_LOAD_DYLINKER:
		{
			lc_str name = reinterpret_cast<struct dylinker_command*>(cmds_ptr)->name;
			LOGF("dylinker: %s\n", (char*)cmds_ptr + name.offset);
			break;
		}

		case LC_UUID:
			break;

		case LC_UNIXTHREAD:
		{
			uint32_t* p = reinterpret_cast<uint32_t*>(cmds_ptr);
			LOGF("UNIXTHREAD");
			
			for (uint32_t i = 2; i < p[1]; i++)
				LOGF(" %d:%x", i, p[i]);
			
			LOGF("\n");
			
			if (m_is64)
				m_entry = reinterpret_cast<uint64_t*>(cmds_ptr)[18];
			else
				m_entry = reinterpret_cast<uint32_t*>(cmds_ptr)[14];
			
			LOGF("entry=%llx\n", (ull)m_entry);
			break;
		}
		
		case LC_MAIN:
		{
			entry_point_command* cmd = reinterpret_cast<entry_point_command*>(cmds_ptr);
			LOGF("MAIN: entry offset: %x\n", cmd->entryoff);
			m_main = reinterpret_cast<uint64_t>(m_text_offset + cmd->entryoff);
			break;
		}

		case LC_LOAD_DYLIB:
		{
			dylib* lib = &reinterpret_cast<dylib_command*>(cmds_ptr)->dylib;
			LOGF("dylib: '%s'\n", (char*)cmds_ptr + lib->name.offset);
			m_dylibs.push_back((char*)cmds_ptr + lib->name.offset);
			break;
		}

	}

	cmds_ptr = reinterpret_cast<load_command*>(
		reinterpret_cast<char*>(cmds_ptr) + cmds_ptr->cmdsize);
	}

	LOGF("%p vs %p\n", cmds_ptr, bin + m_mapped_size);

	LOGF("dyinfo: %p, dysyms: %p, symtab: %p, symstrtab: %p, symbol count: %d\n", dyinfo, dysyms, symtab, symstrtab, m_symbols.size());
	// No LC_DYLD_INFO_ONLY, we will read classic binding info.
	if (!dyinfo && dysyms && symtab && symstrtab)
	{
		LOGF("Reading classic binding info\n");
		for (size_t i = 0; i < bind_sections_64.size(); i++)
		{
			readClassicBind<section_64>(*bind_sections_64[i], dysyms, symtab, symstrtab);
		}
		for (size_t i = 0; i < bind_sections_32.size(); i++)
		{
			readClassicBind<section>(*bind_sections_32[i], dysyms, symtab, symstrtab);
		}
	}
}

MachOImpl::~MachOImpl()
{
	close();
}

void MachOImpl::close()
{
	for (size_t i = 0; i < m_binds.size(); i++)
		delete m_binds[i];
	
	m_binds.clear();
	for (size_t i = 0; i < m_rebases.size(); i++)
		delete m_rebases[i];
	
	m_rebases.clear();
	for (size_t i = 0; i < m_exports.size(); i++)
		delete m_exports[i];
	
	m_exports.clear();

	if (m_mapped)
	{
		::munmap(m_mapped, m_mapped_size);
		::close(m_fd);
		m_mapped = 0;
		m_fd = -1;
	}
}
