#ifndef _MLDR_LOADER_H_
#define _MLDR_LOADER_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#if __has_include(<mach-o/loader.h>)
#include <mach-o/loader.h>
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
