#ifndef _MLDR_LOADER_H_
#define _MLDR_LOADER_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#if __has_include(<mach-o/loader.h>)
#include <mach-o/loader.h>
#else
#ifndef _MACH_HEADER_
#define _MACH_HEADER_
struct mach_header {
	uint32_t magic;
	int32_t cputype;
	int32_t cpusubtype;
	uint32_t filetype;
	uint32_t ncmds;
	uint32_t sizeofcmds;
	uint32_t flags;
};
#endif

#ifndef _MACH_HEADER_64_
#define _MACH_HEADER_64_
struct mach_header_64 {
	uint32_t magic;
	int32_t cputype;
	int32_t cpusubtype;
	uint32_t filetype;
	uint32_t ncmds;
	uint32_t sizeofcmds;
	uint32_t flags;
	uint32_t reserved;
};
#endif

#ifndef _LOAD_COMMAND_
#define _LOAD_COMMAND_
struct load_command {
	uint32_t cmd;
	uint32_t cmdsize;
};
#endif

#ifndef _VM_PROT_T_
#define _VM_PROT_T_
typedef int vm_prot_t;
#endif

#ifndef _SEGMENT_COMMAND_
#define _SEGMENT_COMMAND_
struct segment_command {
	uint32_t cmd;
	uint32_t cmdsize;
	char segname[16];
	uint32_t vmaddr;
	uint32_t vmsize;
	uint32_t fileoff;
	uint32_t filesize;
	vm_prot_t maxprot;
	vm_prot_t initprot;
	uint32_t nsects;
	uint32_t flags;
};
#endif

#ifndef _SEGMENT_COMMAND_64_
#define _SEGMENT_COMMAND_64_
struct segment_command_64 {
	uint32_t cmd;
	uint32_t cmdsize;
	char segname[16];
	uint64_t vmaddr;
	uint64_t vmsize;
	uint64_t fileoff;
	uint64_t filesize;
	vm_prot_t maxprot;
	vm_prot_t initprot;
	uint32_t nsects;
	uint32_t flags;
};
#endif

#ifndef _SECTION_
#define _SECTION_
struct section {
	char sectname[16];
	char segname[16];
	uint32_t addr;
	uint32_t size;
	uint32_t offset;
	uint32_t align;
	uint32_t reloff;
	uint32_t nreloc;
	uint32_t flags;
	uint32_t reserved1;
	uint32_t reserved2;
};
#endif

#ifndef _SECTION_64_
#define _SECTION_64_
struct section_64 {
	char sectname[16];
	char segname[16];
	uint64_t addr;
	uint64_t size;
	uint32_t offset;
	uint32_t align;
	uint32_t reloff;
	uint32_t nreloc;
	uint32_t flags;
	uint32_t reserved1;
	uint32_t reserved2;
	uint32_t reserved3;
};
#endif

#ifndef S_REGULAR
#define S_REGULAR 0x0
#endif
#ifndef S_ATTR_SOME_INSTRUCTIONS
#define S_ATTR_SOME_INSTRUCTIONS 0x00000400
#endif
#ifndef S_ATTR_PURE_INSTRUCTIONS
#define S_ATTR_PURE_INSTRUCTIONS 0x80000000
#endif

#ifndef _LC_STR_
#define _LC_STR_
union lc_str {
	uint32_t offset;
#if !defined(__LP64__) && UINTPTR_MAX == 0xffffffffU
	char *ptr;
#endif
};
#endif

#ifndef _DYLINKER_COMMAND_
#define _DYLINKER_COMMAND_
struct dylinker_command {
	uint32_t cmd;
	uint32_t cmdsize;
	union lc_str name;
};
#endif

#ifndef _ENTRY_POINT_COMMAND_
#define _ENTRY_POINT_COMMAND_
struct entry_point_command {
	uint32_t cmd;
	uint32_t cmdsize;
	uint64_t entryoff;
	uint64_t stacksize;
};
#endif

#ifndef _UUID_COMMAND_
#define _UUID_COMMAND_
struct uuid_command {
	uint32_t cmd;
	uint32_t cmdsize;
	uint8_t uuid[16];
};
#endif

#ifndef MH_MAGIC
#define MH_MAGIC 0xfeedface
#define MH_CIGAM 0xcefaedfe
#define MH_MAGIC_64 0xfeedfacf
#define MH_CIGAM_64 0xcffaedfe
#define MH_EXECUTE 0x2
#define MH_DYLINKER 0x7
#define MH_PIE 0x200000
#endif

#ifndef LC_SEGMENT
#define LC_SEGMENT 0x1
#define LC_UNIXTHREAD 0x5
#define LC_LOAD_DYLINKER 0xe
#define LC_SEGMENT_64 0x19
#define LC_UUID 0x1b
#define LC_MAIN (0x28 | 0x80000000)
#endif

#ifndef SEG_PAGEZERO
#define SEG_PAGEZERO "__PAGEZERO"
#define SEG_TEXT "__TEXT"
#define SEG_DATA "__DATA"
#define SEG_LINKEDIT "__LINKEDIT"
#endif

#ifndef VM_PROT_NONE
#define VM_PROT_NONE 0x00
#define VM_PROT_READ 0x01
#define VM_PROT_WRITE 0x02
#define VM_PROT_EXECUTE 0x04
#endif
#endif
#if __has_include(<mach-o/fat.h>)
#include <mach-o/fat.h>
#else
struct fat_arch {
	int32_t cputype;
	int32_t cpusubtype;
	uint32_t offset;
	uint32_t size;
	uint32_t align;
};
typedef int32_t cpu_type_t;
#ifndef CPU_TYPE_ANY
#define CPU_TYPE_ANY ((cpu_type_t) -1)
#define CPU_TYPE_X86 ((cpu_type_t) 7)
#define CPU_TYPE_ARM ((cpu_type_t) 12)
#define CPU_TYPE_X86_64 ((cpu_type_t) (CPU_TYPE_X86 | 0x01000000))
#define CPU_TYPE_ARM64 ((cpu_type_t) (CPU_TYPE_ARM | 0x01000000))
#endif
#endif

struct load_results {
	unsigned long mh;
	unsigned long entry_point;
	unsigned long stack_size;
	unsigned long dyld_all_image_location;
	unsigned long dyld_all_image_size;
	uint8_t uuid[16];

	unsigned long vm_addr_max;
	bool _32on64;
	bool emulated_x86_64;
	int32_t cputype;
	unsigned long base;
	uint32_t bprefs[4];
	char* root_path;
	size_t root_path_length;
	unsigned long stack_top;
	// size of the initial mapping below stack_top
	unsigned long stack_map_size;
	char* socket_path;
	int kernfd;
	int lifetime_pipe;

	// seal the read-only segments of the images mldr maps (code_signing_gate())
	bool seal_images;

	size_t argc;
	size_t envc;
	char** argv;
	char** envp;
};

static inline void mldr_consider_fat_arch(
	const struct fat_arch* arch,
	cpu_type_t forced_arch,
	const uint32_t bprefs[4],
	struct fat_arch* best_arch,
	int* bpref_index)
{
	if (!forced_arch)
	{
		for (int j = 0; j < 4; j++)
		{
			if (bprefs[j] && (uint32_t)arch->cputype == bprefs[j])
			{
				if (*bpref_index == -1 || *bpref_index > j)
				{
					*best_arch = *arch;
					*bpref_index = j;
					break;
				}
			}
		}

		if (*bpref_index == -1)
		{
#if defined(__x86_64__)
			if (arch->cputype == CPU_TYPE_X86_64)
				*best_arch = *arch;
			else if (best_arch->cputype == CPU_TYPE_ANY && arch->cputype == CPU_TYPE_X86)
				*best_arch = *arch;
#elif defined(__i386__)
			if (arch->cputype == CPU_TYPE_X86)
				*best_arch = *arch;
#elif defined(__aarch64__)
			if (arch->cputype == CPU_TYPE_ARM64)
				*best_arch = *arch;
			else if ((best_arch->cputype == CPU_TYPE_ANY || best_arch->cputype == CPU_TYPE_ARM) &&
			         arch->cputype == CPU_TYPE_X86_64)
				*best_arch = *arch;
			else if (best_arch->cputype == CPU_TYPE_ANY && arch->cputype == CPU_TYPE_ARM)
				*best_arch = *arch;
#else
#error Unsupported CPU architecture
#endif
		}
	}
	else
	{
		if (arch->cputype == forced_arch)
			*best_arch = *arch;
	}
}

#endif // _MLDR_LOADER_H_
