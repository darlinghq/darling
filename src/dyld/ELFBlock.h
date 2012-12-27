
#ifndef _ELF_BLOCK_H_
#define _ELF_BLOCK_H_

#include <stdint.h>
#include <string>
#include <vector>

class ELFBlock
{
	public:
		ELFBlock(const std::string &filename) : m_filename(filename), m_data(nullptr), m_size(0) {}

		const uint8_t* address(void) { if (nullptr == m_data) { finalize(); } return m_data; }

		uint64_t size(void) const { return m_size; }

		void addSection(const std::string &name, void *start, uint64_t size, uint64_t flags)
		{
			m_sections.emplace_back(name, start, size, flags);
		}

		void addSymbol(const std::string &name, void *addr)
		{
			m_symbols.emplace_back(name, addr);
		}

		void finalize(void);

	protected:
		class StringTable
		{
			public:
				StringTable(void) : m_offset(0), m_size(0), m_currentOffset(1), m_data(nullptr) {}

				uint64_t add(const std::string &str);

			public:
				uint64_t m_offset;
				uint64_t m_size;
				uint64_t m_currentOffset;
				char *m_data;
		};

		class Section
		{
			public:
				Section(const std::string &name, void *start, uint64_t size, uint64_t flags) :
					m_name(name), m_start(start), m_size(size), m_flags(flags) {}

				std::string m_name;
				void       *m_start;
				uint64_t    m_size;
				uint64_t    m_flags;
		};

		class Symbol
		{
			public:
				Symbol(const std::string &name, void* addr) : m_name(name), m_addr(addr) {}

				std::string m_name;
				void       *m_addr;
		};

	protected:
		uint64_t calcSize(StringTable &strtab, uint64_t &symtabOffset);

	protected:
		const std::string   &m_filename;
		std::vector<Section> m_sections;
		std::vector<Symbol>  m_symbols;

		uint8_t *m_data;
		uint64_t m_size;
};

#endif // _ELF_BLOCK_H_
