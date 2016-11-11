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
#ifndef GDBJIT_H
#define GDBJIT_H
#include <vector>
#include <stdint.h>
#include <elf.h>

#ifndef ElfW
#ifdef __x86_64__
#	define ElfW(name) Elf64_##name
#else
#	define ElfW(name) Elf32_##name
#endif
#endif

namespace Darling
{
	// Prepare for supplying information to GDB through its JIT interface
	// (although we're surely no JIT compiler)
	void SetupGDBJIT();
	
	class ELFProducer
	{
	public:
		struct Segment
		{
			const char* name;
			uintptr_t offsetInFile;
			void* vaddr;
			uintptr_t memSize;
			uintptr_t fileSize;
			int flags;
		};
		
		ELFProducer();
		void addSymbol(const char* name, void* addr, bool function, bool local);
		void addSegment(Segment seg);
		void setEHSection(uintptr_t addr, uintptr_t size);
		std::pair<uint8_t*,size_t> produceElf();
	private:
		uintptr_t addString(const char* str);
		void addSection(const ElfW(Shdr)& section, const std::vector<uint8_t>& data = std::vector<uint8_t>());
	private:
		std::vector<char> m_stringTable;
		
		struct Symbol
		{
			uintptr_t name;
			void* addr;
			bool function, local;
		};
		std::vector<ElfW(Sym)> m_symbols;
		std::vector<Segment> m_segments;
		
		struct Section
		{
			ElfW(Shdr) hdr;
			std::vector<uint8_t> data;
		};
		std::vector<Section> m_sections;
		
		uintptr_t m_ehAddr = 0, m_ehSize = 0;
		uintptr_t m_dotSymtab, m_dotStrtab, m_dotText, m_dotEhframe;
	};
}

#endif /* GDBJIT_H */

