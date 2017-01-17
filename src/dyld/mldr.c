#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <mach-o/loader.h>
#include <mach-o/fat.h>
#include <dlfcn.h>
#include "elfcalls.h"
#include "threads.h"

#ifndef PAGE_SIZE
#	define PAGE_SIZE	4096
#endif
#define PAGE_ALIGN(x) (x & ~(PAGE_SIZE-1))
#define PAGE_ROUNDUP(x) (((((x)-1) / PAGE_SIZE)+1) * PAGE_SIZE)

static const char* dyld_path = INSTALL_PREFIX "/libexec/usr/lib/dyld";

// The idea of mldr is to load dyld_path into memory and set up the stack
// as described in dyldStartup.S.
// After that, we pass control over to dyld.
//
// Additionally, mldr providers access to native platforms libdl.so APIs (ELF loader).

static void load64(int fd, uint64_t* entryPoint_out, uint64_t* mh_out);
static void load(const char* path, uint64_t* entryPoint_out, uint64_t* mh_out);
static int native_prot(int prot);
static void apply_root_path(char* path);
static char* elfcalls_make(void);

int main(int argc, const char** argv)
{
	uint64_t entryPoint, mh;
	void** sp;
	int pushCount = 0;
	const char* apple[3];

	if (argc <= 1)
	{
		fprintf(stderr, "mldr is part of Darling. It is not to be executed directly.\n");
		return 1;
	}

	load(argv[1], &entryPoint, &mh);

#ifdef __x86_64__
#       define GETSP(ptr) __asm__ volatile("movq %%rsp, %0" : "=r"(ptr) ::)
#       define JUMPX(pushCount, addr) __asm__ volatile("sub %1, %%rsp; jmpq *%0" :: "m"(addr), "r"(pushCount * sizeof(void*)) :)
#elif defined(__i386__)
#       define GETSP(ptr) __asm__ volatile("movl %%esp, %0" : "=m"(ptr) ::)
#       define JUMPX(pushCount, addr) __asm__ volatile("sub %1, %%esp; jmp *%0" :: "m"(addr), "r"(pushCount * sizeof(void*)) :)
#elif defined(__arm__)
#       define GETSP(ptr) __asm__ volatile("mov %0, sp" : "=r"(ptr) ::)
#       define JUMPX(pushCount, addr) __asm__ volatile("sub sp, %1; bx %0" :: "r"(addr), "r"(pushCount * sizeof(void*)) :)
#else 
#       error Unsupported platform!
#endif

	apple[0] = argv[1];
	apple[1] = elfcalls_make();
	apple[2] = NULL;

	GETSP(sp);
	sp--;

	for (int i = sizeof(apple)/sizeof(apple[0])-1; i >= 0; i--)
		*(sp-(pushCount++)) = (void*) apple[i];

	*(sp-(pushCount++)) = NULL;
	for (int i = 0; environ[i] != NULL; i++)
		*(sp-(pushCount++)) = environ[i];

	for (int i = argc; i >= 1; i--)
		*(sp-(pushCount++)) = (void*) argv[i];

	*(sp-(pushCount++)) = (void*) (uintptr_t)(argc-1);
	*(sp-(pushCount++)) = (void*) mh;
	JUMPX(pushCount, entryPoint);

	__builtin_unreachable();
}

void load(const char* path, uint64_t* entryPoint_out, uint64_t* mh_out)
{
	int fd;
	uint32_t magic;

	fd = open(path, O_RDONLY);
	if (fd == -1)
	{
		fprintf(stderr, "Cannot open %s: %s\n", path, strerror(errno));
		exit(1);
	}

	// TODO: We need to read argv[1] and detect whether it's a 32 or 64-bit application.
	// Then load the appropriate version of dyld from the fat file.
	// In case the to-be-executed executable contains both, we prefer the 64-bit version,
	// unless a special property has been passed to sys_posix_spawn() to force the 32-bit
	// version. See posix_spawnattr_setbinpref_np().
	
	if (read(fd, &magic, sizeof(magic)) != sizeof(magic))
	{
		fprintf(stderr, "Cannot read the file header of %s.\n", path);
		exit(1);
	}

	if (magic == MH_MAGIC_64 || magic == MH_CIGAM_64)
	{
		lseek(fd, 0, SEEK_SET);
		load64(fd, entryPoint_out, mh_out);
	}
	else if (magic == FAT_MAGIC || magic == FAT_CIGAM)
	{
	}
	else
	{
		fprintf(stderr, "Unknown file format: %s.\n", path);
		exit(1);
	}

	close(fd);
}

void load64(int fd, uint64_t* entryPoint_out, uint64_t* mh_out)
{
	struct mach_header_64 header;
	uint8_t* cmds;
	uint64_t entryPoint = 0, entryPointDylinker = 0;
	struct mach_header_64* mappedHeader = NULL;
	uint64_t slide = 0;
	uint64_t mmapSize = 0;
	bool pie = false;

	if (read(fd, &header, sizeof(header)) != sizeof(header))
	{
		fprintf(stderr, "Cannot read the dyld header.\n");
		exit(1);
	}

	cmds = (uint8_t*) malloc(header.sizeofcmds);
	if (!cmds)
	{
		fprintf(stderr, "Cannot allocate %u bytes for loader commands.\n", header.sizeofcmds);
		exit(1);
	}

	if (read(fd, cmds, header.sizeofcmds) != header.sizeofcmds)
	{
		fprintf(stderr, "Cannot read loader commands.\n");
		exit(1);
	}

	if ((header.filetype == MH_EXECUTE && header.flags & MH_PIE) || header.filetype == MH_DYLINKER)
	{
		// Go through all LC_SEGMENT_64 commands to get the total continuous range required.
		for (uint32_t i = 0, p = 0; i < header.ncmds; i++)
		{
			struct segment_command_64* seg = (struct segment_command_64*) &cmds[p];

			// Load commands are always sorted, so this will get us the maximum address.
			if (seg->cmd == LC_SEGMENT_64)
				mmapSize = seg->vmaddr + seg->vmsize;

			p += seg->cmdsize;
		}

		slide = (uint64_t) mmap(NULL, mmapSize, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
		if (slide == (uint64_t)MAP_FAILED)
		{
			fprintf(stderr, "Cannot mmap anonymous memory range: %s", strerror(errno));
			exit(1);
		}

		pie = true;
	}

	for (uint32_t i = 0, p = 0; i < header.ncmds; i++)
	{
		struct load_command* lc;

		lc = (struct load_command*) &cmds[p];

		switch (lc->cmd)
		{
			case LC_SEGMENT_64:
			{
				struct segment_command_64* seg = (struct segment_command_64*) lc;
				void* rv;

				if (memcmp(SEG_PAGEZERO, seg->segname, sizeof(SEG_PAGEZERO)) == 0)
				{
					// Let's skip pagezero and map a single page at address 0 elsewhere.
					p += lc->cmdsize;
					continue;
				}

				if (seg->filesize < seg->vmsize)
				{
					if (slide != 0)
					{
						// Some segments' filesize != vmsize, thus this mprotect().
						if (mprotect((void*) (seg->vmaddr + slide), seg->vmsize, native_prot(seg->maxprot)) != 0)
						{
							fprintf(stderr, "Cannot mprotect for segment %s: %s\n", seg->segname, strerror(errno));
							exit(1);
						}
					}
					else
					{
						size_t size = seg->vmsize - seg->filesize;
						rv = mmap((void*) PAGE_ALIGN(seg->vmaddr + seg->vmsize - size), PAGE_ROUNDUP(size), native_prot(seg->maxprot),
								MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, -1, 0);
					}
				}

				if (seg->filesize > 0)
				{
					rv = mmap((void*) (seg->vmaddr + slide), seg->filesize, native_prot(seg->maxprot),
							MAP_FIXED | MAP_PRIVATE, fd, seg->fileoff);
					if (rv == (void*)MAP_FAILED)
					{
						fprintf(stderr, "Cannot mmap segment %s at %p: %s\n", seg->segname, (void*)seg->vmaddr, strerror(errno));
						exit(1);
					}
				
					if (seg->fileoff == 0)
						mappedHeader = (struct mach_header_64*) (seg->vmaddr + slide);
				}
				break;
			}
			case LC_UNIXTHREAD:
			{
				entryPoint = ((uint64_t*) lc)[18];
				entryPoint += slide;
				break;
			}
			case LC_LOAD_DYLINKER:
			{
				struct dylinker_command* dy = (struct dylinker_command*) lc;
				char path[4096];
				size_t length = dy->cmdsize - dy->name.offset;

				if (length > sizeof(path)-1)
				{
					fprintf(stderr, "Dynamic linker name too long: %lu\n", length);
					exit(1);
				}

				memcpy(path, ((char*) dy) + dy->name.offset, length);
				path[length] = '\0';

				apply_root_path(path);

				load(path, &entryPointDylinker, NULL);

				break;
			}
		}

		p += lc->cmdsize;
	}

	free(cmds);

	if (entryPoint_out != NULL)
		*entryPoint_out = entryPointDylinker ? entryPointDylinker : entryPoint;
	if (mh_out != NULL)
		*mh_out = (uint64_t) mappedHeader;
}

int native_prot(int prot)
{
	int protOut = 0;

	if (prot & VM_PROT_READ)
		protOut |= PROT_READ;
	if (prot & VM_PROT_WRITE)
		protOut |= PROT_WRITE;
	if (prot & VM_PROT_EXECUTE)
		protOut |= PROT_EXEC;

	return protOut;
}

// Enable the use of DYLD_ROOT_PATH also for dyld itself.
// This is mainly used for testing.
void apply_root_path(char* path)
{
	const char* roots = getenv("DYLD_ROOT_PATH");
	char my_path[4096];
	const char *p, *next;

	if (!roots)
		return;

	p = roots;

	do
	{
		next = strchr(p, ':');

		if (next != NULL)
		{
			strncpy(my_path, p, next-p);
			my_path[next-p] = '\0';
			next++;
		}
		else
		{
			strcpy(my_path, p);
		}

		strcat(my_path, path);

		if (access(my_path, R_OK) == 0)
		{
			// We've found the desired path in one of the roots, switch to it
			strcpy(path, my_path);
			break;
		}
	}
	while (next != NULL);
}

static void* dlopen_simple(const char* name)
{
	return dlopen(name, RTLD_LAZY);
}

static char* elfcalls_make(void)
{
	static char param[32];
	static struct elf_calls calls;

	calls.dlopen = dlopen_simple;
	calls.dlclose = dlclose;
	calls.dlsym = dlsym;
	calls.dlerror = dlerror;
	calls.darling_thread_create = __darling_thread_create;
	calls.darling_thread_terminate = __darling_thread_terminate;
	calls.darling_thread_get_stack = __darling_thread_get_stack;

	sprintf(param, "elf_calls=%p", &calls);
	return param;
}

