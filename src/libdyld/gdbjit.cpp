/*
This file is part of Darling.

Copyright (C) 2016 Lubos Dolezel

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

#include <stdint.h>
#include "MachOMgr.h"
#include "MachOObject.h"
#include "gdbjit.h"
#include <map>
#include <cstring>
#include <memory>
#include <iostream>
#include "eh/BufWriter.h"

enum jit_actions_t
{
  JIT_NOACTION = 0,
  JIT_REGISTER_FN,
  JIT_UNREGISTER_FN
};

struct jit_code_entry
{
  struct jit_code_entry *next_entry;
  struct jit_code_entry *prev_entry;
  uint8_t *symfile_addr;
  uint64_t symfile_size;
};

struct jit_descriptor
{
  uint32_t version;
  /* This type should be jit_actions_t, but we use uint32_t
     to be explicit about the bitwidth.  */
  uint32_t action_flag;
  struct jit_code_entry *relevant_entry;
  struct jit_code_entry *first_entry;
};

/* GDB puts a breakpoint in this function.  */
extern "C"
void __attribute__((noinline)) __jit_debug_register_code() { };

/* Make sure to specify the version statically, because the
   debugger may check the version before we can set it.  */
extern "C"
struct jit_descriptor __jit_debug_descriptor = { 1, 0, 0, 0 };

// MachO loading operates under a mutex, so the following code needs no further locking
// when registering stuff with GDB and so on.
static std::map<Darling::MachOObject*, jit_code_entry*> g_machoToElf;
static void OnMachOLoaded(const struct ::mach_header* mh, intptr_t vmaddr_slide);
static void OnMachOUnloaded(const struct ::mach_header* mh, intptr_t vmaddr_slide);

void Darling::SetupGDBJIT()
{
	Darling::MachOMgr* mgr = Darling::MachOMgr::instance();
	mgr->registerLoadHook(OnMachOLoaded);
	mgr->registerUnloadHook(OnMachOUnloaded);
}

static int MachO2ElfFlags(vm_prot_t prot)
{
	int flags = 0;
	if (prot & VM_PROT_READ)
		flags |= PF_R;
	if (prot & VM_PROT_WRITE)
		flags |= PF_W;
	if (prot & VM_PROT_EXECUTE)
		flags |= PF_X;
	return flags;
}

void OnMachOLoaded(const struct ::mach_header* mh, intptr_t vmaddr_slide)
{
	Darling::MachOObject* object = Darling::MachOMgr::instance()->objectByHeader(const_cast<struct ::mach_header*>(mh));
	Darling::ELFProducer producer;
	MachO* mo;
	
	mo = object->getMachOFile();
	const std::vector<MachO::Symbol> symbols = mo->symbols();
	
	for (const MachO::Symbol& sym : symbols)
	{
		producer.addSymbol(sym.name.c_str(), (void*)(sym.addr + vmaddr_slide), true, false);
	}
	
#ifdef __x86_64__
	const std::vector<segment_command_64*>& segs = mo->segments64();
	for (const segment_command_64* seg : segs)
	{
		producer.addSegment(Darling::ELFProducer::Segment { seg->segname, seg->fileoff, (void*)(seg->vmaddr + vmaddr_slide),
			seg->vmsize, seg->filesize, MachO2ElfFlags(seg->initprot) });
	}
#else
	const std::vector<segment_command*>& segs = mo->segments();
	for (const segment_command* seg : segs)
	{
		producer.addSegment(Darling::ELFProducer::Segment { seg->segname, seg->fileoff, (void*)(seg->vmaddr + vmaddr_slide),
			seg->vmsize, seg->filesize, MachO2ElfFlags(seg->initprot) });
	}
#endif
	
	producer.setEHSection(mo->get_eh_frame().first, mo->get_eh_frame().second);
	
	std::pair<uint8_t*, size_t> elfData = producer.produceElf();
	jit_code_entry* entry = new jit_code_entry;
	
	entry->symfile_addr = elfData.first;
	entry->symfile_size = elfData.second;
	
	std::ofstream dump("/tmp/test.elf");
	dump.write((char*)entry->symfile_addr, entry->symfile_size);
	dump.close();
	
	entry->next_entry = __jit_debug_descriptor.first_entry;
	if (entry->next_entry)
		entry->next_entry->prev_entry = entry;
	entry->prev_entry = nullptr;
	
	g_machoToElf[object] = entry;
	
	std::cout << "Registering " << object->name() << std::endl;
	
	if (!__jit_debug_descriptor.first_entry)
		__jit_debug_descriptor.first_entry = entry;
	
	__jit_debug_descriptor.relevant_entry = entry;
	__jit_debug_descriptor.action_flag = JIT_REGISTER_FN;
	__jit_debug_register_code();
}

void OnMachOUnloaded(const struct ::mach_header* mh, intptr_t vmaddr_slide)
{
	Darling::MachOObject* object = Darling::MachOMgr::instance()->objectByHeader(const_cast<struct ::mach_header*>(mh));
	jit_code_entry* jitEntry;
	
	std::map<Darling::MachOObject*, jit_code_entry*>::iterator it = g_machoToElf.find(object);
	
	if (it == g_machoToElf.end())
		return;
	
	jitEntry = it->second;
	
	// Remove entry from linked list
	if (jitEntry->prev_entry)
		jitEntry->prev_entry->next_entry = jitEntry->next_entry;
	if (jitEntry->next_entry)
		jitEntry->next_entry->prev_entry = jitEntry->prev_entry;
	
	if (__jit_debug_descriptor.first_entry == jitEntry)
		__jit_debug_descriptor.first_entry = nullptr;
	
	// Signal removal to GDB
	__jit_debug_descriptor.relevant_entry = jitEntry;
	__jit_debug_descriptor.action_flag = JIT_UNREGISTER_FN;
	__jit_debug_register_code();
	
	g_machoToElf.erase(it);
	delete [] jitEntry->symfile_addr;
	delete jitEntry;
}

Darling::ELFProducer::ELFProducer()
{
	addString("");
	m_dotSymtab = addString(".symtab");
	m_dotStrtab = addString(".strtab");
	m_dotText = addString(".text");
	// m_dotEhframe = addString(".eh_frame");
	
	ElfW(Shdr) shdr;
		
	memset(&shdr, 0, sizeof(shdr));
	shdr.sh_type = SHT_NULL;
	addSection(shdr);
}

void Darling::ELFProducer::addSymbol(const char* name, void* addr, bool function, bool local)
{
	ElfW(Sym) sym;
	
	sym.st_name = addString(name);
	sym.st_value = uintptr_t(addr);
	sym.st_info = ELF32_ST_INFO(local ? STB_LOCAL : STB_GLOBAL, function ? STT_FUNC : STT_OBJECT);
	sym.st_shndx = SHN_ABS;
	sym.st_other = 0;
	sym.st_size = 0x100;
	
	m_symbols.push_back(sym);
}

void Darling::ELFProducer::addSegment(Segment seg)
{
	m_segments.push_back(seg);
}

void Darling::ELFProducer::setEHSection(uintptr_t addr, uintptr_t size)
{
	m_ehAddr = addr;
	m_ehSize = size;
}

uintptr_t Darling::ELFProducer::addString(const char* str)
{
	uintptr_t offset = m_stringTable.size();
	m_stringTable.insert(m_stringTable.end(), str, str + strlen(str) + 1);
	return offset;
}

template <typename T>
inline static size_t pushStruct(std::vector<uint8_t>& buf, const T& _struct)
{
	size_t off = buf.size();
	buf.insert(buf.end(), reinterpret_cast<const uint8_t*>(&_struct), reinterpret_cast<const uint8_t*>((&_struct) + 1));
	return off;
}

std::pair<uint8_t*,size_t> Darling::ELFProducer::produceElf()
{
	std::unique_ptr<BufWriter> bufWriter;
	size_t memSize;
	uint8_t* mem;
	
	memSize = sizeof(ElfW(Ehdr));
	memSize += m_stringTable.size();
	memSize += 5*sizeof(ElfW(Shdr));
	memSize += sizeof(ElfW(Sym)) * (2+m_symbols.size());
	
	mem = new uint8_t[memSize];
	bufWriter.reset(new BufWriter(mem, memSize));
	
	ElfW(Ehdr) elfHeader, *elfHeaderPtr;
    
	elfHeader.e_ident[EI_MAG0] = ELFMAG0;
	elfHeader.e_ident[EI_MAG1] = ELFMAG1;
	elfHeader.e_ident[EI_MAG2] = ELFMAG2;
	elfHeader.e_ident[EI_MAG3] = ELFMAG3;
#ifdef __x86_64__
	elfHeader.e_ident[EI_CLASS] = ELFCLASS64;
#else
	elfHeader.e_ident[EI_CLASS] = ELFCLASS32;
#endif
	
	elfHeader.e_ident[EI_DATA] = ELFDATA2LSB;
	elfHeader.e_ident[EI_VERSION] = EV_CURRENT;
	elfHeader.e_ident[EI_OSABI] = ELFOSABI_GNU;
	elfHeader.e_ident[EI_ABIVERSION] = 0;
	
#if defined(__x86_64__)
	elfHeader.e_machine = EM_X86_64;
#elif defined(__i386__)
	elfHeader.e_machine = EM_386;
#elif defined(__arm__)
	elfHeader.e_machine = EM_ARM;
#else
#	warning Add support for your arch here
	elfHeader.e_machine = 0;
#endif
	
	elfHeader.e_type = ET_DYN;
	elfHeader.e_entry = 0;
	elfHeader.e_flags = 0;
	elfHeader.e_phoff = 0; //sizeof(elfHeader);
	elfHeader.e_phnum = 0; //m_segments.size() /*+ (m_ehSize ? 1 : 0)*/;
	elfHeader.e_phentsize = sizeof(ElfW(Phdr));
	elfHeader.e_shentsize = sizeof(ElfW(Shdr));
	elfHeader.e_shstrndx = 2; // string section
	elfHeader.e_shnum = 4 /*+ (m_ehSize ? 1 : 0)*/; // string table + symbol table
	elfHeader.e_ehsize = sizeof(elfHeader);
	
	bufWriter->writeType(elfHeader);
	
	const Segment* textSegment = nullptr;
	for (const Segment& seg : m_segments)
	{
		if (strcmp(seg.name, "__TEXT") == 0)
			textSegment = &seg;
	}
	
	elfHeaderPtr = (ElfW(Ehdr)*) mem;
	elfHeaderPtr->e_shoff = bufWriter->relativePos();
	
	ElfW(Shdr)* symtab;
	
	{
		ElfW(Shdr) shdr;
		
		memset(&shdr, 0, sizeof(shdr));
		shdr.sh_type = SHT_NULL;
		bufWriter->writeType(shdr);
		
		shdr.sh_name = m_dotSymtab;
		shdr.sh_type = SHT_SYMTAB;
		shdr.sh_entsize = sizeof(ElfW(Sym));
		shdr.sh_size = shdr.sh_entsize * (m_symbols.size() + 2);
		shdr.sh_addr = 0;
		shdr.sh_offset = bufWriter->relativePos() + 3*sizeof(shdr) + m_stringTable.size(); // after this shdr and the string table
		shdr.sh_flags = SHF_ALLOC;
		shdr.sh_addralign = 0;
		shdr.sh_link = 2; // which string table section
		shdr.sh_info = 0;
		
		symtab = (ElfW(Shdr)*) bufWriter->posPtr();
		bufWriter->writeType(shdr);
	}
	
	{
		ElfW(Shdr) shdr;
		
		shdr.sh_name = m_dotStrtab;
		shdr.sh_type = SHT_STRTAB;
		shdr.sh_size = m_stringTable.size();
		shdr.sh_addr = 0;
		shdr.sh_offset = bufWriter->relativePos() + 2*sizeof(shdr); // after this and next shdr
		shdr.sh_entsize = 0;
		shdr.sh_flags = 0;
		shdr.sh_link = 0;
		shdr.sh_addralign = 0;
		shdr.sh_link = 0;
		shdr.sh_info = 0;
		
		bufWriter->writeType(shdr);
	}
	
	if (textSegment != nullptr)
	{
		ElfW(Shdr) shdr;
		
		shdr.sh_name = m_dotText;
		shdr.sh_type = SHT_PROGBITS;
		shdr.sh_size = textSegment->memSize;
		shdr.sh_addr = uintptr_t(textSegment->vaddr);
		shdr.sh_offset = 0;
		shdr.sh_entsize = 0;
		shdr.sh_flags = SHF_ALLOC | SHF_EXECINSTR;
		shdr.sh_link = 0;
		shdr.sh_addralign = 1;
		shdr.sh_link = 0;
		shdr.sh_info = 0;
		
		bufWriter->writeType(shdr);
	}
	
	/*
	if (m_ehSize)
	{
		ElfW(Shdr) shdr;
		
		shdr.sh_name = m_dotEhframe;
		shdr.sh_type = SHT_PROGBITS;
		shdr.sh_size = m_ehSize;
		shdr.sh_addr = uintptr_t(m_ehAddr);
		shdr.sh_offset = 0;
		shdr.sh_entsize = 0;
		shdr.sh_flags = SHF_ALLOC;
		shdr.sh_link = 0;
		shdr.sh_addralign = 1;
		shdr.sh_link = 0;
		shdr.sh_info = 0;
		
		bufWriter->writeType(shdr);
	}
	*/
	
	bufWriter->writeBlock(&m_stringTable[0], m_stringTable.size());
	symtab->sh_addr = (uintptr_t) bufWriter->posPtr();
	
	ElfW(Sym) sym;
	
	sym.st_name = 0;
	sym.st_value = 0;
	sym.st_info = ELF32_ST_INFO(STB_LOCAL, STT_NOTYPE);
	sym.st_shndx = 0;
	sym.st_other = 0;
	sym.st_size = 0;
	
	bufWriter->writeType(sym);
	
	sym.st_name = 0;
	sym.st_value = 0;
	sym.st_info = ELF32_ST_INFO(STB_LOCAL, STT_SECTION);
	sym.st_shndx = 3;
	sym.st_other = 0;
	
	bufWriter->writeType(sym);
	
	for (ElfW(Sym) sym : m_symbols)
	{
		sym.st_shndx = 3; // .text section
		// sym.st_value -= uintptr_t(textSegment->vaddr);
		bufWriter->writeType(sym);
	}
	
	return std::make_pair(mem, memSize);
}

void Darling::ELFProducer::addSection(const ElfW(Shdr)& section, const std::vector<uint8_t>& data)
{
	Section sect { section, data };
	m_sections.push_back(sect);
}
