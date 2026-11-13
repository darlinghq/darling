#include <stdint.h>
#include <mach-o/loader.h>
#include <mach-o/fat.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>

#include "loader.h"

// Keyed on GEN_64BIT, not __aarch64__. mldr.c includes this file twice, once per word
// size, and __aarch64__ is true in both passes, so a definition guarded only by the
// architecture is emitted twice and the second one is a redefinition error.
#if defined(GEN_64BIT) && defined(__aarch64__)
#define MLDR_ARM_THREAD_STATE64 6
struct mldr_arm_thread_state64 {
	uint64_t x[29];
	uint64_t fp;
	uint64_t lr;
	uint64_t sp;
	uint64_t pc;
	uint32_t cpsr;
};
#define MLDR_ARM_THREAD_STATE64_COUNT (sizeof(struct mldr_arm_thread_state64) / sizeof(uint32_t))
#endif

static int native_prot(int prot);
static void load(const char* path, cpu_type_t cpu, bool expect_dylinker, char** argv, struct load_results* lr);
static void setup_space(struct load_results* lr, bool is_64_bit);
static void* compatible_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);

#ifndef PAGE_ROUNDUP
#	define PAGE_ROUNDUP(x) (((x) + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1))
#endif

#ifndef MLDR_PAGE_SIZE_HELPERS_DEFINED
#define MLDR_PAGE_SIZE_HELPERS_DEFINED

/*
 * Dynamic page size helper for Mach-O loading.
 *
 * On Apple Silicon (and any aarch64 macOS target) the standard ABI uses 16 KB
 * segment alignment. When running under a Linux kernel with 4 KB pages, using a
 * plain 4 KB host page size would leave gaps between adjacent Mach-O segments
 * unmapped (the file-backed mmap of segment N ends at filesize rounded to 4 KB,
 * but segment N+1 starts at vmaddr rounded to 16 KB — leaving 12 KB unmapped,
 * triggering an immediate SIGSEGV on any access).
 *
 * Conversely, on host kernels configured with 64 KB pages (e.g. some RHEL or
 * Asahi Linux kernels), the host page size is larger than 16 KB.
 *
 * We dynamically query the host kernel page size at runtime via
 * sysconf(_SC_PAGESIZE) and use max(host_page_size, abi_page_size) so that
 * the alignment is never smaller than either the kernel requires or the Mach-O
 * ABI expects.
 */
static inline size_t mldr_get_host_page_size(void) {
	static size_t hps = 0;
	size_t cached = __atomic_load_n(&hps, __ATOMIC_RELAXED);
	if (__builtin_expect(!cached, 0)) {
		long ps = sysconf(_SC_PAGESIZE);
		cached = (ps > 0) ? (size_t)ps : (size_t)PAGE_SIZE;
		__atomic_store_n(&hps, cached, __ATOMIC_RELAXED);
	}
	return cached;
}

static inline size_t mldr_get_macho_page_size(uint32_t cputype) {
	size_t host_ps = mldr_get_host_page_size();
#if defined(__aarch64__) || defined(__arm64__)
	/* On aarch64, standard macOS Mach-O segment alignment is 16 KB.
	 * If the host kernel has larger pages (e.g. 64 KB), host_ps takes precedence. */
	size_t abi_ps = (cputype == CPU_TYPE_ARM64) ? 0x4000UL : host_ps;
	return (host_ps > abi_ps) ? host_ps : abi_ps;
#else
	(void)cputype;
	return host_ps;
#endif
}

static inline size_t mldr_page_roundup(size_t val, size_t ps) {
	return (val + ps - 1) & ~(ps - 1);
}

#endif /* MLDR_PAGE_SIZE_HELPERS_DEFINED */

#ifndef MLDR_MMAP_MIN_ADDR_HINT_DEFINED
#define MLDR_MMAP_MIN_ADDR_HINT_DEFINED

/*
 * A mapping at a fixed address fails with EPERM when the address is below the
 * kernel's vm.mmap_min_addr. Executables that are not position independent,
 * common among old i386 applications, are linked to load at 0x1000. Say what
 * is wrong and how to allow it, instead of only reporting EPERM.
 */
static void mldr_explain_mmap_failure(int err, uintptr_t addr)
{
	unsigned long min_addr;
	FILE* f;

	if (err != EPERM)
		return;

	f = fopen("/proc/sys/vm/mmap_min_addr", "r");
	if (f == NULL)
		return;
	if (fscanf(f, "%lu", &min_addr) != 1)
		min_addr = 0;
	fclose(f);

	if (addr >= min_addr)
		return;

	fprintf(stderr,
		"This executable is not position independent and must be loaded at %#lx,\n"
		"below the lowest address Linux allows to be mapped (vm.mmap_min_addr = %lu).\n"
		"To allow it, run: sudo sysctl -w vm.mmap_min_addr=%lu\n"
		"See https://docs.darlinghq.org/common-issues.html#cannot-mmap-segment-__text-at-0x1000-operation-not-permitted\n",
		(unsigned long) addr, min_addr, (unsigned long) addr);
}

#endif /* MLDR_MMAP_MIN_ADDR_HINT_DEFINED */

// Definitions:
// FUNCTION_NAME (load32/load64)
// SEGMENT_STRUCT (segment_command/SEGMENT_STRUCT)
// SEGMENT_COMMAND (LC_SEGMENT/SEGMENT_COMMAND)
// MACH_HEADER_STRUCT (mach_header/MACH_HEADER_STRUCT)
// SECTION_STRUCT (section/SECTION_STRUCT)

#if defined(GEN_64BIT)
#   define FUNCTION_NAME load64
#   define SEGMENT_STRUCT segment_command_64
#   define SEGMENT_COMMAND LC_SEGMENT_64
#   define MACH_HEADER_STRUCT mach_header_64
#   define SECTION_STRUCT section_64
#	define MAP_EXTRA 0
#elif defined(GEN_32BIT)
#   define FUNCTION_NAME load32
#   define SEGMENT_STRUCT segment_command
#   define SEGMENT_COMMAND LC_SEGMENT
#   define MACH_HEADER_STRUCT mach_header
#   define SECTION_STRUCT section
#	ifdef MAP_32BIT
#		define MAP_EXTRA MAP_32BIT
#	else
#		define MAP_EXTRA 0
#	endif
#else
#   error See above
#endif

void FUNCTION_NAME(int fd, bool expect_dylinker, struct load_results* lr)
{
	struct MACH_HEADER_STRUCT header;
	uint8_t* cmds;
	uintptr_t entryPoint = 0, entryPointDylinker = 0;
	struct MACH_HEADER_STRUCT* mappedHeader = NULL;
	uintptr_t slide = 0;
	uintptr_t mmapSize = 0;
	bool pie = false;
	off_t fat_offset;
	void* tmp_map_base = NULL;
	const int source_fd = fd;

	if (!expect_dylinker)
	{
#if defined(GEN_64BIT)
		setup_space(lr, true);
#elif defined(GEN_32BIT)
		lr->_32on64 = true;
		setup_space(lr, false);
#else
		#error Unsupported architecture
#endif
	}

	fat_offset = lseek(fd, 0, SEEK_CUR);

	if (!expect_dylinker)
		fd = code_signing_gate(fd, fat_offset, lr);

	if (read(fd, &header, sizeof(header)) != sizeof(header))
	{
		fprintf(stderr, "Cannot read the mach header.\n");
		exit(1);
	}

	if (header.filetype != (expect_dylinker ? MH_DYLINKER : MH_EXECUTE))
	{
		fprintf(stderr, "Found unexpected Mach-O file type: %u\n", header.filetype);
		exit(1);
	}

	tmp_map_base = mmap(NULL, PAGE_ROUNDUP(sizeof(header) + header.sizeofcmds), PROT_READ, MAP_PRIVATE, fd, fat_offset);
	if (tmp_map_base == MAP_FAILED) {
		fprintf(stderr, "Failed to mmap header + commands\n");
		exit(1);
	}

	cmds = (void*)((char*)tmp_map_base + sizeof(header));

	if ((header.filetype == MH_EXECUTE && header.flags & MH_PIE) || header.filetype == MH_DYLINKER)
	{
		uintptr_t base = -1;

		// Go through all SEGMENT_COMMAND commands to get the total continuous range required.
		for (uint32_t i = 0, p = 0; i < header.ncmds; i++)
		{
			struct SEGMENT_STRUCT* seg = (struct SEGMENT_STRUCT*) &cmds[p];

			// Load commands are always sorted, so this will get us the maximum address.
			if (seg->cmd == SEGMENT_COMMAND && strcmp(seg->segname, "__PAGEZERO") != 0 && seg->vmsize != 0)
			{
				if (base == -1)
				{
					base = seg->vmaddr;
					//if (base != 0 && header.filetype == MH_DYLINKER)
					//	goto no_slide;
				}
				// the anonymous part of the last segment extends to its Mach-O page
				mmapSize = mldr_page_roundup(seg->vmaddr + seg->vmsize - base, mldr_get_macho_page_size(header.cputype));
			}

			p += seg->cmdsize;
		}

		void* mmap_hint = (void*) base;
#if (defined(__aarch64__) || defined(__arm64__)) && defined(GEN_64BIT)
		/* Set when we allocate the slot ourselves rather than honouring a
		 * preferred address the binary asked for. */
		int managed_slot = 0;
		/* macOS ObjC FAST_DATA_MASK is 0x00007ffffffffff8 — only 47 bits of
		 * data pointer. On Linux ARM64 the kernel happily returns 48-bit VAs
		 * (e.g. 0xfe..) which then get truncated by the mask to a bogus
		 * address, crashing readClass() with SIGSEGV. Force the mapping into
		 * a low-VA window we control. We hand out distinct slots so dyld and
		 * subsequently-loaded dylibs don't collide with the main executable
		 * (which typically wants 0x100000000). */
		/* Leave the modern ARM64 shared cache's preferred range (6 GiB and
         * above) available. dyld reserves its actual cache span before mapping
         * files, so an incompatible future layout fails without overwriting us. */
		static uintptr_t next_low_addr = 0x1000000000ULL; /* 64 GiB; below the ObjC 47-bit ceiling */
		if (base == 0) {
			mmap_hint = (void*)next_low_addr;
			managed_slot = 1;
		}
#endif
		/* When the slot is ours, reserve exactly where we asked. A bare hint lets
		 * the kernel return a different address, which leaves the slot allocator
		 * bookkeeping describing a range it no longer holds. */
#if (defined(__aarch64__) || defined(__arm64__)) && defined(GEN_64BIT)
		if (managed_slot) {
			while (1) {
				slide = (uintptr_t) mmap(mmap_hint, mmapSize, PROT_NONE,
					MAP_ANONYMOUS | MAP_PRIVATE | MAP_EXTRA | MAP_FIXED_NOREPLACE, -1, 0);
				if (slide != (uintptr_t)MAP_FAILED || errno != EEXIST)
					break;
				/* Slot occupied; advance to next 16 MiB boundary and retry */
				uintptr_t bump = ((uintptr_t)mmap_hint + mmapSize + 0xffffff) & ~0xffffffULL;
				if (bump >= 0x800000000000ULL)
					break;
				next_low_addr = bump;
				mmap_hint = (void*)bump;
			}
		} else
#endif
		{
			slide = (uintptr_t) mmap(mmap_hint, mmapSize, PROT_NONE,
				MAP_ANONYMOUS | MAP_PRIVATE | MAP_EXTRA, -1, 0);
		}
		if (slide == (uintptr_t)MAP_FAILED)
		{
			fprintf(stderr, "Cannot mmap anonymous memory range: %s\n", strerror(errno));
			exit(1);
		}
#if (defined(__aarch64__) || defined(__arm64__)) && defined(GEN_64BIT)
		/* If we ended up above 2^47 anyway, retry with MAP_FIXED_NOREPLACE in the low
		 * range so the slid address stays reachable through FAST_DATA_MASK. */
		if (slide >= 0x800000000000ULL) {
			munmap((void*)slide, mmapSize);
			slide = (uintptr_t)mmap((void*)next_low_addr, mmapSize, PROT_NONE,
			                         MAP_ANONYMOUS | MAP_PRIVATE | MAP_EXTRA | MAP_FIXED_NOREPLACE,
			                         -1, 0);
			if (slide == (uintptr_t)MAP_FAILED) {
				fprintf(stderr, "Cannot mmap low-VA range: %s\n", strerror(errno));
				exit(1);
			}
		}
		/* Bump the slot for the next allocation, leaving headroom.
		 * Derive it from the slot we requested, not from wherever the kernel
		 * placed the mapping: had that landed below the current slot, comparing
		 * against it made this whole update a no-op and handed the next load the
		 * same range, with the reservation unmapped immediately below. */
		if (base == 0)
			next_low_addr = ((uintptr_t)mmap_hint + mmapSize + 0xffffff) & ~0xffffffULL;
#endif

		// unmap it so we can map the actual segments later using MAP_FIXED_NOREPLACE;
		// we're the only thread running, so there's no chance this memory range will become occupied from now until then
		munmap((void*)slide, mmapSize);

		if (slide + mmapSize > lr->vm_addr_max)
			lr->vm_addr_max = lr->base = slide + mmapSize;
		slide -= base;

		pie = true;
	}
no_slide:

	for (uint32_t i = 0, p = 0; i < header.ncmds && p < header.sizeofcmds; i++)
	{
		struct load_command* lc;

		lc = (struct load_command*) &cmds[p];

		switch (lc->cmd)
		{
			case SEGMENT_COMMAND:
			{
				struct SEGMENT_STRUCT* seg = (struct SEGMENT_STRUCT*) lc;
				void* rv;

				// This logic is wrong and made up. But it's the only combination where
				// some apps stop crashing (TBD why) and LLDB recognized the memory layout
				// of processes started as suspended.
				int maxprot = native_prot(seg->maxprot);
				int initprot = native_prot(seg->initprot);
				int useprot = (initprot & PROT_EXEC) ? maxprot : initprot;

				// Skip zero-vmsize segments (e.g. __DWARF), __PAGEZERO or any unmapped zero-address segment
				if (seg->vmsize == 0 || strcmp(seg->segname, "__PAGEZERO") == 0 || (seg->vmaddr == 0 && useprot == 0))
				{
					if (seg->vmaddr + slide + seg->vmsize > lr->vm_addr_max)
						lr->vm_addr_max = seg->vmaddr + slide + seg->vmsize;
					break;
				}

				/* Zero-based dylinker __TEXT also needs its slide; PAGEZERO was skipped above. */
				uintptr_t seg_addr = seg->vmaddr + slide;

				const size_t host_ps = mldr_get_host_page_size();
				const size_t page_sz = mldr_get_macho_page_size(header.cputype);

				// 1. Map the file-backed portion of the segment
				if (seg->filesize > 0)
				{
					rv = compatible_mmap((void*)seg_addr, seg->filesize, useprot,
							MAP_FIXED_NOREPLACE | MAP_PRIVATE, fd, seg->fileoff + fat_offset);
					if (rv == (void*)MAP_FAILED)
					{
						int err = errno;
						fprintf(stderr, "Cannot mmap segment %s at %p: %s\n", seg->segname, (void*)seg_addr, strerror(err));
						mldr_explain_mmap_failure(err, seg_addr);
						exit(1);
					}

					if (seg->fileoff == 0)
						mappedHeader = (struct MACH_HEADER_STRUCT*) seg_addr;

					// The file mapping covers its whole last page: past filesize that page
					// holds whatever follows in the file, where the segment must read as
					// zeros (start of __bss/__thread_bss), as the XNU loader guarantees.
					if (seg->filesize < seg->vmsize)
					{
						uintptr_t tail = seg_addr + seg->filesize;
						uintptr_t tail_end = PAGE_ROUNDUP(tail);
						if (tail_end > seg_addr + seg->vmsize)
							tail_end = seg_addr + seg->vmsize;
						if (tail_end > tail)
						{
							if (!(useprot & PROT_WRITE))
								mprotect((void*) PAGE_ALIGN(tail), PAGE_SIZE, useprot | PROT_WRITE);
							memset((void*) tail, 0, tail_end - tail);
							if (!(useprot & PROT_WRITE))
								mprotect((void*) PAGE_ALIGN(tail), PAGE_SIZE, useprot);
						}
					}

					if (lr->seal_images && (useprot & PROT_READ) && !(useprot & PROT_WRITE) &&
						(strncmp(seg->segname, SEG_TEXT, sizeof(seg->segname)) == 0 ||
						 strncmp(seg->segname, SEG_LINKEDIT, sizeof(seg->segname)) == 0))
					{
						seal_segment(seg->segname, seg_addr, seg->filesize, useprot);
					}
				}

				// 2. Map the remaining BSS/anonymous pages (if any) beyond the file-backed pages.
				// Round the total segment size up to the Mach-O target page size (page_sz)
				// so that the gap between adjacent segments is completely covered by BSS mappings.
				size_t file_pages = seg->filesize > 0 ? mldr_page_roundup(seg->filesize, host_ps) : 0;
				size_t total_segment_size = mldr_page_roundup(seg->vmsize, page_sz);
				if (total_segment_size > file_pages)
				{
					uintptr_t bss_addr = seg_addr + file_pages;
					size_t bss_size = total_segment_size - file_pages;

					rv = compatible_mmap((void*)bss_addr, bss_size, useprot,
							MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED_NOREPLACE, -1, 0);
					if (rv == (void*)MAP_FAILED)
					{
						int err = errno;
						fprintf(stderr, "Cannot mmap bss for segment %s at %p: %s\n", seg->segname, (void*)bss_addr, strerror(err));
						mldr_explain_mmap_failure(err, bss_addr);
						exit(1);
					}
				}

				if (seg->vmaddr + slide + seg->vmsize > lr->vm_addr_max)
					lr->vm_addr_max = seg->vmaddr + slide + seg->vmsize;

				if (strcmp(SEG_DATA, seg->segname) == 0)
				{
					// Look for section named __all_image_info for GDB integration
					struct SECTION_STRUCT* sect = (struct SECTION_STRUCT*) (seg+1);
					struct SECTION_STRUCT* end = (struct SECTION_STRUCT*) (&cmds[p + lc->cmdsize]);

					while (sect < end)
					{
						if (strncmp(sect->sectname, "__all_image_info", 16) == 0)
						{
							lr->dyld_all_image_location = slide + sect->addr;
							lr->dyld_all_image_size = sect->size;
							break;
						}
						sect++;
					}
				}
				break;
			}
			case LC_UNIXTHREAD:
			{
#ifdef GEN_64BIT
#if defined(__x86_64__)
				entryPoint = ((uint64_t*) lc)[18];
#elif defined(__aarch64__)
				if (lc->cmdsize < sizeof(*lc) + 2 * sizeof(uint32_t) + sizeof(struct mldr_arm_thread_state64)) {
					fprintf(stderr, "Truncated ARM64 LC_UNIXTHREAD command\n");
					exit(1);
				}
				uint32_t* thread_command_data = (uint32_t*)(lc + 1);
				if (thread_command_data[0] != MLDR_ARM_THREAD_STATE64 ||
						thread_command_data[1] < MLDR_ARM_THREAD_STATE64_COUNT ||
						thread_command_data[1] > (lc->cmdsize - sizeof(*lc) - 2 * sizeof(uint32_t)) / sizeof(uint32_t)) {
					fprintf(stderr, "Invalid ARM64 LC_UNIXTHREAD state\n");
					exit(1);
				}
				struct mldr_arm_thread_state64 state;
				memcpy(&state, &thread_command_data[2], sizeof(state));
				entryPoint = state.pc;
#else
#error Unsupported 64-bit architecture
#endif
#endif
#ifdef GEN_32BIT
				entryPoint = ((uint32_t*) lc)[14];
#endif
				entryPoint += slide;
				break;
			}
			case LC_LOAD_DYLINKER:
			{
				if (header.filetype != MH_EXECUTE)
				{
					// dylinker can't reference another dylinker
					fprintf(stderr, "Dynamic linker can't reference another dynamic linker\n");
					exit(1);
				}

				struct dylinker_command* dy = (struct dylinker_command*) lc;
				char* path = NULL;
				size_t length;
				static char path_buffer[4096];

				if (lr->root_path != NULL)
				{
					const size_t root_len = strlen(lr->root_path);
					const size_t linker_len = dy->cmdsize - dy->name.offset;

					length = linker_len + root_len;
					if (length > sizeof(path_buffer) - 1) {
						fprintf(stderr, "Dynamic loader path too long");
						exit(1);
					}
					path = path_buffer;

					// Concat root path and linker path
					memcpy(path, lr->root_path, root_len);
					memcpy(path + root_len, ((char*) dy) + dy->name.offset, linker_len);
					path[length] = '\0';
				}

				if (path == NULL)
				{
					length = dy->cmdsize - dy->name.offset;
					if (length > sizeof(path_buffer) - 1) {
						fprintf(stderr, "Dynamic loader path too long");
						exit(1);
					}
					path = path_buffer;

					memcpy(path, ((char*) dy) + dy->name.offset, length);
					path[length] = '\0';
				}

				if (path == NULL)
				{
					fprintf(stderr, "Failed to load dynamic linker for executable\n");
					exit(1);
				}

				load(path, header.cputype, true, NULL, lr);

				break;
			}
			case LC_MAIN:
			{
				struct entry_point_command* ee = (struct entry_point_command*) lc;
				if (ee->stacksize > lr->stack_size)
					lr->stack_size = ee->stacksize;
				break;
			}
			case LC_UUID:
			{
				if (header.filetype == MH_EXECUTE)
				{
					struct uuid_command* ue = (struct uuid_command*) lc;
					memcpy(lr->uuid, ue->uuid, sizeof(ue->uuid));
				}
				break;
			}
		}

		p += lc->cmdsize;
	}

	if (header.filetype == MH_EXECUTE)
		lr->mh = (uintptr_t) mappedHeader;
	if (entryPoint && !lr->entry_point)
		lr->entry_point = entryPoint;

	if (tmp_map_base)
		munmap(tmp_map_base, PAGE_ROUNDUP(sizeof(header) + header.sizeofcmds));

	// the image was mapped from a snapshot (code_signing_gate()); the mappings keep it alive
	if (fd != source_fd)
		close(fd);
}


#undef FUNCTION_NAME
#undef SEGMENT_STRUCT
#undef SEGMENT_COMMAND
#undef MACH_HEADER_STRUCT
#undef SECTION_STRUCT
#undef MAP_EXTRA
