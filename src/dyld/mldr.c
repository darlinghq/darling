/*
This file is part of Darling.

Copyright (C) 2017 Lubos Dolezel

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
#include <endian.h>
#include "elfcalls.h"
#include "gdb.h"
#include "commpage.h"
#include "32in64.h"

#ifndef PAGE_SIZE
#	define PAGE_SIZE	4096
#endif
#define PAGE_ALIGN(x) (x & ~(PAGE_SIZE-1))
#define PAGE_ROUNDUP(x) (((((x)-1) / PAGE_SIZE)+1) * PAGE_SIZE)

#define CPU_ARCH_ABI64	0x01000000
#define CPU_TYPE_ANY		((cpu_type_t) -1)
#define CPU_TYPE_X86		((cpu_type_t) 7)
#define	CPU_TYPE_X86_64		(CPU_TYPE_X86 | CPU_ARCH_ABI64)

static const char* dyld_path = INSTALL_PREFIX "/libexec/usr/lib/dyld";

// The idea of mldr is to load dyld_path into memory and set up the stack
// as described in dyldStartup.S.
// After that, we pass control over to dyld.
//
// Additionally, mldr providers access to native platforms libdl.so APIs (ELF loader).

static void load64(int fd, uint64_t* entryPoint_out, uint64_t* mh_out);
static void load32(int fd, uint64_t* entryPoint_out, uint64_t* mh_out);
static void load(const char* path, uint64_t* entryPoint_out, uint64_t* mh_out, cpu_type_t cpu);
static int native_prot(int prot);
static void apply_root_path(char* path);
char* elfcalls_make(void);
static char* apple0_make(const char* filepath);
static void* setup_stack32(void* stack, int argc, const char** argv, const char** envp, const char** apple, uint64_t mh);

static bool mode_32in64 = false;
static uint32_t stack_size = 0;

int main(int argc, char** argv, char** envp)
{
	uint64_t entryPoint, mh;
	void** sp;
	int pushCount = 0;
	const char* apple[3];
	char *filename, *p;

	if (argc <= 1)
	{
		fprintf(stderr, "mldr is part of Darling. It is not to be executed directly.\n");
		return 1;
	}

	// sys_execve() passes the original file path appended to the mldr path in argv[0].
	filename = (char*) __builtin_alloca(strlen(argv[0])+1);

	p = strchr(argv[0], '!');
	if (p != NULL)
		strcpy(filename, p + 1);
	else
		strcpy(filename, argv[1]);

	p = argv[0];
	// Update process name in ps output
	for (int i = 1; i < argc; i++)
	{
		size_t len = strlen(argv[i]);
		memmove(p, argv[i], len+1);

		argv[i-1] = p;
		p += len+1;
	}
	memset(p, 0, envp[0]-p);
	argv[--argc] = NULL;

	load(filename, &entryPoint, &mh, CPU_TYPE_ANY);

#if defined(__i386__) || defined(__x86_64__)
	if (getenv("BREAK_AFTER_LOAD") != NULL)
		__asm__("int3");
#endif

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

	apple[0] = apple0_make(filename);
	apple[1] = elfcalls_make();
	apple[2] = NULL;

	if (!mode_32in64)
	{
		GETSP(sp);
		sp--;

		for (int i = sizeof(apple)/sizeof(apple[0])-1; i >= 0; i--)
			*(sp-(pushCount++)) = (void*) apple[i];

		*(sp-(pushCount++)) = NULL;
		for (int i = 0; environ[i] != NULL; i++)
			*(sp-(pushCount++)) = environ[i];

		*(sp-(pushCount++)) = NULL;
		for (int i = argc-1; i >= 0; i--)
			*(sp-(pushCount++)) = (void*) argv[i];

		*(sp-(pushCount++)) = (void*) (uintptr_t)(argc);
		*(sp-(pushCount++)) = (void*) mh;

		JUMPX(pushCount, entryPoint);
	}
	else
	{
		uint32_t size = stack_size ? stack_size : 8*1024*1024;
		size &= ~3u;

		sp = mmap(NULL, size,
				PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_32BIT | MAP_ANONYMOUS,
				-1, 0);
		// stack guard page
		mprotect(sp, PAGE_SIZE, PROT_NONE);

		// move to the top of the stack
		sp += size/sizeof(void*) - 1;

		_64TO32_WITH_STACK(setup_stack32(sp, argc, (const char**) argv, (const char**) envp, apple, mh), entryPoint);
	}


	__builtin_unreachable();
}

static char* copystring(void* stack, const char* str)
{
	size_t len = strlen(str);
	char* pos = (char*) stack;

	pos -= len+1;
	strcpy(pos, str);

	return pos;
}

static size_t arraylen(const char** str)
{
	size_t len = 0;
	while (str[len] != NULL)
		len++;
	return len;
}

void* setup_stack32(void* stack, int argc, const char** argv_in, const char** envp_in, const char** apple_in, uint64_t mh)
{
	char **argv_new, **envp_new, **apple_new;
	size_t size, apple_size;
	uint32_t* stack_pointers;

	argv_new = (char**) __builtin_alloca((arraylen(argv_in) + 1) * sizeof(char*));
	for (int i = argc-1; i >= 0; i--)
		stack = argv_new[i] = copystring(stack, argv_in[i]);
	argv_new[argc] = NULL;

	size = arraylen(envp_in);
	envp_new = (char**) __builtin_alloca((size + 1) * sizeof(char*));
	for (int i = size-1; i >= 0; i--)
		stack = envp_new[i] = copystring(stack, envp_in[i]);
	envp_new[size] = NULL;

	apple_size = arraylen(apple_in);
	apple_new = (char**) __builtin_alloca((apple_size + 1) * sizeof(char*));
	for (int i = apple_size-1; i >= 0; i--)
		stack = apple_new[i] = copystring(stack, apple_in[i]);

	stack_pointers = (uint32_t*) stack;
	stack_pointers--;

	*stack_pointers-- = 0;
	for (int i = apple_size-1; i >= 0; i--)
		*stack_pointers-- = (uint32_t) apple_new[i];

	*stack_pointers-- = 0;
	for (int i = 0; envp_new[i] != NULL; i++)
		*stack_pointers-- = (uint32_t) envp_new[i];

	*stack_pointers-- = 0;
	for (int i = argc-1; i >= 0; i--)
		*stack_pointers-- = (uint32_t) argv_new[i];

	*stack_pointers-- = argc;
	*stack_pointers = (uint32_t) mh;

	return stack_pointers;
}

void load(const char* path, uint64_t* entryPoint_out, uint64_t* mh_out, cpu_type_t cpu_desired)
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
		if (mh_out)
			commpage_setup(true);

		lseek(fd, 0, SEEK_SET);
		load64(fd, entryPoint_out, mh_out);
	}
	else if (magic == MH_MAGIC || magic == MH_CIGAM)
	{
		if (mh_out)
			commpage_setup(false);

		mode_32in64 = true;
		lseek(fd, 0, SEEK_SET);
		load32(fd, entryPoint_out, mh_out);
	}
	else if (magic == FAT_MAGIC || magic == FAT_CIGAM)
	{
		struct fat_header fhdr;
		struct fat_arch best_arch;
		const bool swap = magic == FAT_CIGAM;

		best_arch.cputype = CPU_TYPE_ANY;

		if (read(fd, &fhdr.nfat_arch, sizeof(fhdr.nfat_arch)) != sizeof(fhdr.nfat_arch))
		{
			fprintf(stderr, "Cannot read the file header of %s.\n", path);
			exit(1);
		}

#define SWAP32(x) x = __bswap_32(x)

		if (swap)
			SWAP32(fhdr.nfat_arch);

		for (uint32_t i = 0; i < fhdr.nfat_arch; i++)
		{
			struct fat_arch arch;

			if (read(fd, &arch, sizeof(arch)) != sizeof(arch))
			{
				fprintf(stderr, "Cannot read fat_arch header of %s.\n", path);
				exit(1);
			}

			if (swap)
			{
				SWAP32(arch.cputype);
				SWAP32(arch.cpusubtype);
				SWAP32(arch.offset);
				SWAP32(arch.size);
				SWAP32(arch.align);
			}

			if (cpu_desired == CPU_TYPE_ANY)
			{
				// Find the best arch
#ifdef __x86_64__
				// Hope for x86-64, but also accept i386
				// TODO: x86_64h (Haswell)
				if (arch.cputype == CPU_TYPE_X86_64)
				{
					memcpy(&best_arch, &arch, sizeof(arch));
					break;
				}
				else if (arch.cputype == CPU_TYPE_X86)
					memcpy(&best_arch, &arch, sizeof(arch));
#endif
			}
			else if (arch.cputype == cpu_desired)
			{
				memcpy(&best_arch, &arch, sizeof(arch));
				break;
			}
		}

		if (best_arch.cputype == CPU_TYPE_ANY)
		{
			fprintf(stderr, "No supported architecture found in fat %s.\n", path);
			exit(1);
		}
		if (lseek(fd, best_arch.offset, SEEK_SET) == -1)
		{
			fprintf(stderr, "Cannot seek to selected arch in fat %s.\n", path);
			exit(1);
		}

		
		if (mh_out)
			commpage_setup(best_arch.cputype & CPU_ARCH_ABI64);

		if (best_arch.cputype & CPU_ARCH_ABI64)
			load64(fd, entryPoint_out, mh_out);
		else
		{
			mode_32in64 = true;
			load32(fd, entryPoint_out, mh_out);
		}
	}
	else
	{
		fprintf(stderr, "Unknown file format: %s.\n", path);
		exit(1);
	}

	close(fd);
}

#define GEN_64BIT
#include "loader.c"
#undef GEN_64BIT

#define GEN_32BIT
#include "loader.c"
#undef GEN_32BIT

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

char* apple0_make(const char* filepath)
{
	static char apple0[4096] = "executable_path=";

	if (filepath[0] == '/')
		strcat(apple0, filepath);
	else
	{
		char* path = realpath(filepath, NULL);
		strcat(apple0, path);
		free(path);
	}

	return apple0;
}

