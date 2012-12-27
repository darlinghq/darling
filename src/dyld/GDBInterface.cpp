
#define DUMP_FILES 0

#include "GDBInterface.h"

#if DUMP_FILES
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

extern "C" {

/* GDB puts a breakpoint in this function.  */
void __attribute__((noinline)) __jit_debug_register_code() { };

/* Make sure to specify the version statically, because the
   debugger may check the version before we can set it.  */
struct jit_descriptor __jit_debug_descriptor = { 1, 0, 0, 0 };

}

std::list<GDBInterface::ELFBlockEntry> GDBInterface::m_ELFBlocks;

void GDBInterface::addELF(ELFBlock *elf)
{
	m_ELFBlocks.emplace_back(elf, (m_ELFBlocks.size() > 0) ? &m_ELFBlocks.back().m_jitEntry : nullptr);
	ELFBlockEntry &entry = m_ELFBlocks.back();

#if DUMP_FILES
	static int fileIndex = 0;
	std::ostringstream ss;
	ss << "/tmp/elf." << fileIndex++;
	std::string s = ss.str();
	int fd = ::open(s.c_str(), O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU);
	::write(fd, elf->address(), elf->size());
	::close(fd);
#endif

	if (__jit_debug_descriptor.first_entry == nullptr)
	{
		__jit_debug_descriptor.first_entry = &entry.m_jitEntry;
	}
	__jit_debug_descriptor.relevant_entry = &entry.m_jitEntry;
	__jit_debug_descriptor.action_flag = JIT_REGISTER_FN;
	__jit_debug_register_code();
}

void GDBInterface::removeELF(ELFBlock *elf)
{
	for (auto entry = m_ELFBlocks.begin(); entry != m_ELFBlocks.end(); entry++)
	{
		if (entry->m_ELFBlock == elf)
		{
			if (__jit_debug_descriptor.first_entry == &entry->m_jitEntry)
			{
				__jit_debug_descriptor.first_entry = entry->m_jitEntry.next_entry;
			}

			if (nullptr != entry->m_jitEntry.next_entry)
			{
				entry->m_jitEntry.next_entry->prev_entry = entry->m_jitEntry.prev_entry;
			}

			if (nullptr != entry->m_jitEntry.prev_entry)
			{
				entry->m_jitEntry.prev_entry->next_entry = entry->m_jitEntry.next_entry;
			}

			__jit_debug_descriptor.relevant_entry = &entry->m_jitEntry;
			__jit_debug_descriptor.action_flag = JIT_UNREGISTER_FN;
			__jit_debug_register_code();

			m_ELFBlocks.erase(entry);
			break;
		}
	}
}

GDBInterface::ELFBlockEntry::ELFBlockEntry(ELFBlock *elf, struct jit_code_entry *prev, struct jit_code_entry *next)
{
	m_ELFBlock = elf;
	m_jitEntry.next_entry = next;
	m_jitEntry.prev_entry = prev;
	m_jitEntry.symfile_addr = (const char*)elf->address();
	m_jitEntry.symfile_size = elf->size();

	if (nullptr != next)
	{
		next->prev_entry = &m_jitEntry;
	}

	if (nullptr != prev)
	{
		prev->next_entry = &m_jitEntry;
	}
}
