
#include "ELFBlock.h"
#include <elf.h>
#include <string.h>
#include <iostream>

#ifdef __x86_64__
typedef Elf64_Ehdr Elf_Ehdr;
typedef Elf64_Shdr Elf_Shdr;
typedef Elf64_Phdr Elf_Phdr;
typedef Elf64_Sym  Elf_Sym;
#define ELFCLASS ELFCLASS64
#define EMACHINE EM_X86_64
#else
typedef Elf32_Ehdr Elf_Ehdr;
typedef Elf32_Shdr Elf_Shdr;
typedef Elf32_Phdr Elf_Phdr;
typedef Elf32_Sym  Elf_Sym;
#define ELFCLASS ELFCLASS32
#define EMACHINE EM_386
#endif

#define EXTRA_SECTIONS 3
#define EXTRA_SYMBOLS 1
#define STRTAB_EXTRA (1 + 8 /* ".strtab" */ + 8 /* ".symtab" */)

uint64_t ELFBlock::StringTable::add(const std::string &str)
{
	uint64_t offset = m_currentOffset;
	strcpy(&m_data[m_offset + m_currentOffset], str.c_str());
	m_currentOffset += str.length() + 1;
	return offset;
}

uint64_t ELFBlock::calcSize(StringTable &strtab, uint64_t &symtabOffset)
{
	uint64_t size = sizeof(Elf_Ehdr);
	size += sizeof(Elf_Shdr) * (m_sections.size() + EXTRA_SECTIONS);
	size += sizeof(Elf_Phdr) * m_sections.size();

	symtabOffset = size;
	size += sizeof(Elf_Sym) * (m_symbols.size() + EXTRA_SYMBOLS);

	strtab.m_offset = size;

	size += m_filename.length() + 1;

	for (auto &section : m_sections)
	{
		size += section.m_name.length() + 1;
	}

	for (auto &symbol : m_symbols)
	{
		size += symbol.m_name.length() + 1;
	}

	size += STRTAB_EXTRA;

	strtab.m_size = size - strtab.m_offset;

	return size;
}

void ELFBlock::finalize(void)
{
	if (nullptr != m_data)
	{
		return;
	}

	//std::cout << "Finalizing \"" << m_filename << "\"" << std::endl;

	// Calculate size and allocate it
	StringTable strtab;
	uint64_t symtabOffset = 0;
	m_size = calcSize(strtab, symtabOffset);
	m_data = new uint8_t[m_size];
	memset(m_data, 0, m_size);
	strtab.m_data = (char*)m_data;

	//std::cout << "Size " << m_size << std::endl;

	// Fill in the ELF header
	uint64_t offset = 0;
	Elf_Ehdr *hdr = (Elf_Ehdr*)&m_data[offset];
	offset += sizeof(Elf_Ehdr);

	hdr->e_ident[EI_MAG0]    = ELFMAG0;
	hdr->e_ident[EI_MAG1]    = ELFMAG1;
	hdr->e_ident[EI_MAG2]    = ELFMAG2;
	hdr->e_ident[EI_MAG3]    = ELFMAG3;
	hdr->e_ident[EI_CLASS]   = ELFCLASS;
	hdr->e_ident[EI_DATA]    = ELFDATA2LSB;
	hdr->e_ident[EI_VERSION] = EV_CURRENT;
	hdr->e_ident[EI_OSABI]   = ELFOSABI_GNU;
	hdr->e_ident[EI_ABIVERSION] = 0;
	hdr->e_type      = ET_DYN;
	hdr->e_machine   = EMACHINE;
	hdr->e_version   = EV_CURRENT;
	hdr->e_entry     = 0;
	hdr->e_phoff     = offset;
	hdr->e_shoff     = offset + (sizeof(Elf_Phdr) * m_sections.size());
	hdr->e_flags     = 0;
	hdr->e_ehsize    = sizeof(Elf_Ehdr);
	hdr->e_phentsize = sizeof(Elf_Phdr);
	hdr->e_phnum     = m_sections.size();
	hdr->e_shentsize = sizeof(Elf_Shdr);
	hdr->e_shnum     = m_sections.size() + EXTRA_SECTIONS;
	hdr->e_shstrndx  = 1;

	// Fill in program headers
	for (auto &section : m_sections)
	{
		Elf_Phdr *phdr = (Elf_Phdr*)&m_data[offset];
		offset += sizeof(Elf_Phdr);

		phdr->p_type   = PT_LOAD;
		phdr->p_offset = 0;
		phdr->p_vaddr  = (uintptr_t)section.m_start;
		phdr->p_paddr  = 0;
		phdr->p_filesz = 0;
		phdr->p_memsz  = section.m_size;
		phdr->p_flags  = PF_R | PF_W | PF_X;
		phdr->p_align  = 0;

	}

	// Fill in section headers (first one is reserved, second is strtab, third is symtab)
	offset += sizeof(Elf_Shdr);

	Elf_Shdr *shdr = (Elf_Shdr*)&m_data[offset];
	offset += sizeof(Elf_Shdr);

	shdr->sh_name      = strtab.add(".strtab");
	shdr->sh_type      = SHT_STRTAB;
	shdr->sh_flags     = 0;
	shdr->sh_addr      = (uintptr_t)&m_data[strtab.m_offset];
	shdr->sh_offset    = strtab.m_offset;
	shdr->sh_size      = strtab.m_size;
	shdr->sh_link      = 0;
	shdr->sh_info      = 0;
	shdr->sh_addralign = 0;
	shdr->sh_entsize   = 0;

	shdr = (Elf_Shdr*)&m_data[offset];
	offset += sizeof(Elf_Shdr);

	shdr->sh_name      = strtab.add(".symtab");
	shdr->sh_type      = SHT_SYMTAB;
	shdr->sh_flags     = 0;
	shdr->sh_addr      = (uintptr_t)&m_data[symtabOffset];
	shdr->sh_offset    = symtabOffset;
	shdr->sh_size      = sizeof(Elf_Sym) * (m_symbols.size() + EXTRA_SYMBOLS);
	shdr->sh_link      = 1;
	shdr->sh_info      = 0;
	shdr->sh_addralign = 0;
	shdr->sh_entsize   = sizeof(Elf_Sym);

	for (auto &section : m_sections)
	{
		shdr = (Elf_Shdr*)&m_data[offset];
		offset += sizeof(Elf_Shdr);

		shdr->sh_name      = strtab.add(section.m_name);
		shdr->sh_type      = SHT_NOBITS;
		shdr->sh_flags     = SHF_ALLOC;
		shdr->sh_addr      = (uintptr_t)section.m_start;
		shdr->sh_offset    = 0;
		shdr->sh_size      = section.m_size;
		shdr->sh_link      = 0;
		shdr->sh_info      = 0;
		shdr->sh_addralign = 0;
		shdr->sh_entsize   = 0;
	}

	// Fill in symbols (first is the filename)
	Elf_Sym *sym = (Elf_Sym*)&m_data[offset];
	offset += sizeof(Elf_Sym);

	sym->st_name  = strtab.add(m_filename);
	sym->st_info  = (STB_LOCAL << 4) | STT_FILE;
	sym->st_other = 0;
	sym->st_shndx = SHN_ABS;
	sym->st_value = 0;
	sym->st_size  = 0;

	for (auto &symbol : m_symbols)
	{
		sym = (Elf_Sym*)&m_data[offset];
		offset += sizeof(Elf_Sym);

		sym->st_name  = strtab.add(symbol.m_name);
		sym->st_info  = (STB_GLOBAL << 4) | STT_FUNC; // TODO: differentiate between functions/vars/etc.
		sym->st_other = 0;
		sym->st_shndx = SHN_ABS;
		sym->st_value = (uintptr_t)symbol.m_addr;
		sym->st_size  = 0;
	}
}
