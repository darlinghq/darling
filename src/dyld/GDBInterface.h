
#ifndef _GDB_INTERFACE_H_
#define _GDB_INTERFACE_H_

#include <stdint.h>
#include <list>
#include "ELFBlock.h"

extern "C" {

typedef enum
{
	JIT_NOACTION = 0,
	JIT_REGISTER_FN,
	JIT_UNREGISTER_FN
} jit_actions_t;

struct jit_code_entry
{
	struct jit_code_entry *next_entry;
	struct jit_code_entry *prev_entry;
	const char *symfile_addr;
	uint64_t symfile_size;
};

struct jit_descriptor
{
	uint32_t version;
	uint32_t action_flag;
	struct jit_code_entry *relevant_entry;
	struct jit_code_entry *first_entry;
};

} // extern "C"

class GDBInterface
{
	public:
		static void addELF(ELFBlock *elf);
		static void removeELF(ELFBlock *elf);

	protected:

		class ELFBlockEntry
		{
			public:
				ELFBlockEntry(ELFBlock *elf, struct jit_code_entry *prev = nullptr, struct jit_code_entry *next = nullptr);

				ELFBlock             *m_ELFBlock;
				struct jit_code_entry m_jitEntry;
		};

		static std::list<ELFBlockEntry> m_ELFBlocks;
};

#endif // _GDB_INTERFACE_H_
