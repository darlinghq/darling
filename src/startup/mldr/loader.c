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

static int native_prot(int prot);
static void load(const char* path, cpu_type_t cpu, bool expect_dylinker, char** argv, struct load_results* lr);
static void setup_space(struct load_results* lr, bool is_64_bit);
static void* compatible_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);

#ifndef PAGE_SIZE
#	define PAGE_SIZE	4096
#endif
#define PAGE_ALIGN(x) (x & ~(PAGE_SIZE-1))
#define PAGE_ROUNDUP(x) (((((x)-1) / PAGE_SIZE)+1) * PAGE_SIZE)

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
#	define MAP_EXTRA MAP_32BIT
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
	uint32_t fat_offset;
	void* tmp_map_base = NULL;

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
			if (seg->cmd == SEGMENT_COMMAND && strcmp(seg->segname, "__PAGEZERO") != 0)
			{
				if (base == -1)
				{
					base = seg->vmaddr;
					//if (base != 0 && header.filetype == MH_DYLINKER)
					//	goto no_slide;
				}
				mmapSize = seg->vmaddr + seg->vmsize - base;
			}

			p += seg->cmdsize;
		}

		slide = (uintptr_t) mmap((void*) base, mmapSize, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_EXTRA, -1, 0);
		if (slide == (uintptr_t)MAP_FAILED)
		{
			fprintf(stderr, "Cannot mmap anonymous memory range: %s\n", strerror(errno));
			exit(1);
		}

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

				if (seg->filesize < seg->vmsize)
				{
					unsigned long map_addr;
					if (slide != 0)
					{
						unsigned long addr = seg->vmaddr;

						if (addr != 0)
							addr += slide;

						// Some segments' filesize != vmsize, thus this mprotect().
						rv = compatible_mmap((void*)addr, seg->vmsize, useprot, MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED_NOREPLACE, -1, 0);
						if (rv == (void*)MAP_FAILED)
						{
							if (seg->vmaddr == 0 && useprot == 0) {
								// this is the PAGEZERO segment;
								// if we can't map it, assume everything is fine and the system has already made that area inaccessible
								rv = 0;
							} else {
								fprintf(stderr, "Cannot mmap segment %s at %p: %s\n", seg->segname, (void*)(uintptr_t)seg->vmaddr, strerror(errno));
								exit(1);
							}
						}
					}
					else
					{
						size_t size = seg->vmsize - seg->filesize;
						rv = compatible_mmap((void*) PAGE_ALIGN(seg->vmaddr + seg->vmsize - size), PAGE_ROUNDUP(size), useprot,
								MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED_NOREPLACE, -1, 0);
						if (rv == (void*)MAP_FAILED)
						{
							if (seg->vmaddr == 0 && useprot == 0) {
								// this is the PAGEZERO segment;
								// if we can't map it, assume everything is fine and the system has already made that area inaccessible
								rv = 0;
							} else {
								fprintf(stderr, "Cannot mmap segment %s at %p: %s\n", seg->segname, (void*)(uintptr_t)seg->vmaddr, strerror(errno));
								exit(1);
							}
						}
					}
				}

				if (seg->filesize > 0)
				{
					unsigned long addr = seg->vmaddr + slide;
					int flag = MAP_FIXED_NOREPLACE;
					if (seg->filesize < seg->vmsize) {
						flag = MAP_FIXED;
					}
					rv = compatible_mmap((void*)addr, seg->filesize, useprot,
							flag | MAP_PRIVATE, fd, seg->fileoff + fat_offset);
					if (rv == (void*)MAP_FAILED)
					{
						if (seg->vmaddr == 0 && useprot == 0) {
							// this is the PAGEZERO segment;
							// if we can't map it, assume everything is fine and the system has already made that area inaccessible
							rv = 0;
						} else {
							fprintf(stderr, "Cannot mmap segment %s at %p: %s\n", seg->segname, (void*)(uintptr_t)seg->vmaddr, strerror(errno));
							exit(1);
						}
					}

					if (seg->fileoff == 0)
						mappedHeader = (struct MACH_HEADER_STRUCT*) (seg->vmaddr + slide);
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
#if defined(__x86_64__)
				// Grab rip from x86_thread_state64
				entryPoint = ((uint64_t*) lc)[18];
#elif defined(__i386__)
				// Grab eip from i386_thread_state
				entryPoint = ((uint32_t*) lc)[14];
#elif defined(__aarch64__)
				// Grab pc from arm_thread_state64
				entryPoint = ((uint64_t*) lc)[34];
#else
#error "Missing entryPoint assignment from thread state"
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
				char* path;
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
}


#undef FUNCTION_NAME
#undef SEGMENT_STRUCT
#undef SEGMENT_COMMAND
#undef MACH_HEADER_STRUCT
#undef SECTION_STRUCT
#undef MAP_EXTRA

