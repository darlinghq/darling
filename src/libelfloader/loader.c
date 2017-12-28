/*
 * Darling
 * Copyright (C) 2017 Lubos Dolezel
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <setjmp.h>
#include <sys/random.h>
#include "elf.h"
#include "auxvec.h"
#include "loader.h"
#include "native/elfcalls.h"

#ifdef __APPLE__
extern int getentropy(void* buf, size_t len);
#endif

#define ElfW(type)	_ElfW (Elf, __ELF_NATIVE_CLASS, type)
#define _ElfW(e,w,t)	_ElfW_1 (e, w, _##t)
#define _ElfW_1(e,w,t)	e##w##t

#if defined(__x86_64__) || defined(__arm64__)
#	define DUMMY_PATH "/usr/libexec/elfloader_dummy64"
#	define MY_ELF_CLASS ELFCLASS64
#	define __ELF_NATIVE_CLASS 64
#else
#	define DUMMY_PATH "/usr/libexec/elfloader_dummy32"
#	define MY_ELF_CLASS ELFCLASS32
#	define __ELF_NATIVE_CLASS 32
#endif

#define ELF_MIN_ALIGN 0x1000
#define ELF_PAGESTART(_v) ((_v) & ~(unsigned long)(ELF_MIN_ALIGN-1))
#define ELF_PAGEOFFSET(_v) ((_v) & (ELF_MIN_ALIGN-1))
#define ELF_PAGEALIGN(_v) (((_v) + ELF_MIN_ALIGN - 1) & ~(ELF_MIN_ALIGN - 1))

extern char** environ;

struct loader_context
{
	uintptr_t interp_entry;
	uintptr_t exec_entry;
	uintptr_t exec_phstart;
	uintptr_t exec_phentsize;
	uintptr_t exec_phnum;
	uintptr_t interp_base;
};

static void run(const char* path);
static void load(const char* path, struct loader_context* lc, bool isInterp);
static void loader_return(void);
static const char SYSTEM_ROOT[] = "/Volumes/SystemRoot";

static jmp_buf jmpbuf;
static struct elf_calls elfcalls;

#ifndef TEST
__attribute__((constructor))
	static void runDummy(void)
{
	run(DUMMY_PATH);
}
#endif

void run(const char* path)
{
	struct loader_context lc;
	unsigned long *stack, *stackmem;
	int ptrcount, pos = 0;
	uint8_t entropy[16];
	char pointer1[20];
	char pointer2[20];

	load(path, &lc, false);
	if (!lc.interp_entry)
		return;

	stackmem = (unsigned long*) mmap(NULL, 4096*4, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANON, -1, 0);

	if (stackmem == MAP_FAILED)
		perror("mmap");

	stack = (unsigned long*) (((char*) stackmem) + 4096*4 - sizeof(unsigned long));

	// AT_*
	ptrcount = 10*2;
	ptrcount++; // argc
	ptrcount += 4; // argv

	// environ
	for (int i = 0; environ[i] != NULL; i++)
		ptrcount++;
	ptrcount++;

	stack -= ptrcount;

	stack[pos++] = 3; // argc
	stack[pos++] = (unsigned long) "elfloader_dummy";

	snprintf(pointer1, sizeof(pointer1), "%lx", (unsigned long) &elfcalls);
	stack[pos++] = (unsigned long) pointer1;

	snprintf(pointer2, sizeof(pointer2), "%lx", (unsigned long) loader_return);
	stack[pos++] = (unsigned long) pointer2;
	stack[pos++] = 0;


	for (int i = 0; environ[i] != NULL; i++)
		stack[pos++] = (unsigned long) environ[i];

#ifdef __linux__ // For testing
	getrandom(entropy, sizeof(entropy), 0);
#else // Darwin
	getentropy(entropy, sizeof(entropy));
#endif

	stack[pos++] = 0;

	stack[pos++] = AT_PHDR;
	stack[pos++] = lc.exec_phstart;

	stack[pos++] = AT_PHENT;
	stack[pos++] = lc.exec_phentsize;

	stack[pos++] = AT_PHNUM;
	stack[pos++] = lc.exec_phnum;

	stack[pos++] = AT_ENTRY;
	stack[pos++] = lc.exec_entry;

	stack[pos++] = AT_BASE;
	stack[pos++] = lc.interp_base;

	stack[pos++] = AT_PAGESZ;
	stack[pos++] = 4096;

	stack[pos++] = AT_FLAGS;
	stack[pos++] = 0;

	stack[pos++] = AT_RANDOM;
	stack[pos++] = (unsigned long) entropy;

	stack[pos++] = AT_NULL;
	stack[pos++] = 0;

	// TODO: AT_EXECFN?
#ifdef __x86_64__
#       define JUMPX(stack, addr) __asm__ volatile("mov %1, %%rsp; jmpq *%0" :: "m"(addr), "r"(stack) :)
#elif defined(__i386__)
#       define JUMPX(stack, addr) __asm__ volatile("mov %1, %%esp; jmp *%0" :: "m"(addr), "r"(stack) :)
#else 
#       error Unsupported platform!
#endif

	if (!setjmp(jmpbuf))
		JUMPX(stack, lc.interp_entry);
	else
	{
		puts("Back from loaded binary");
	}
}

// AT_PHDR -> loadaddr + phoff
// AT_PHENT -> e_phentsize
// AT_PHNUM -> e_phnum
// AT_ENTRY -> exec->e_entry
// AT_BASE -> interp_loadaddr
// AT_EXECFN -> apple[0]
// AT_PAGESZ -> PAGE_SIZE
// AT_FLAGS -> 0
// AT_NULL

void load(const char* path, struct loader_context* lc, bool isInterp)
{
	ElfW(Ehdr) elfHdr;
	void* phdrs = NULL;

	int fd = open(path, O_RDONLY);
	uintptr_t slide, base;

	if (fd == -1)
	{
		perror("open");
		return;
	}

	if (read(fd, &elfHdr, sizeof(elfHdr)) != sizeof(elfHdr))
	{
		perror("read");
		goto out;
	}

	if (memcmp(elfHdr.e_ident, ELFMAG, SELFMAG) != 0 || elfHdr.e_ident[EI_CLASS] != MY_ELF_CLASS)
	{
		fprintf(stderr, "Wrong ELF signature\n");
		goto out;
	}

	if (elfHdr.e_type != ET_DYN)
	{
		fprintf(stderr, "Only position independent ELF are supported\n");
		goto out;
	}
	if (!elfHdr.e_phoff)
	{
		fprintf(stderr, "ELF is not loadable\n");
		goto out;
	}

	phdrs = malloc(elfHdr.e_phentsize * elfHdr.e_phnum);
	if (pread(fd, phdrs, elfHdr.e_phentsize * elfHdr.e_phnum, elfHdr.e_phoff) != elfHdr.e_phentsize * elfHdr.e_phnum)
	{
		fprintf(stderr, "Failed to read Elf phdrs\n");
		goto out;
	}

	// First, get total virtual range needed
	uintptr_t minAddr = UINTPTR_MAX, maxAddr = 0;

	for (int i = 0; i < elfHdr.e_phnum; i++)
	{
		ElfW(Phdr)* phdr = (ElfW(Phdr)*) (((char*) phdrs) + (i * elfHdr.e_phentsize));

		if (phdr->p_type == PT_LOAD)
		{
			if (phdr->p_vaddr < minAddr)
				minAddr = ELF_PAGESTART(phdr->p_vaddr);
			if (phdr->p_vaddr + phdr->p_memsz > maxAddr)
				maxAddr = phdr->p_vaddr + phdr->p_memsz;
		}
		else if (phdr->p_type == PT_INTERP && isInterp)
		{
			fprintf(stderr, "Interp with PT_INTERP?\n");
			goto out;
		}
	}
	if (maxAddr == 0)
	{
		fprintf(stderr, "No PT_LOAD headers?\n");
		goto out;
	}

	base = (uintptr_t) mmap(NULL, maxAddr-minAddr, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
	if (base == (uintptr_t) MAP_FAILED)
	{
		perror("mmap");
		fprintf(stderr, "Cannot reserve 0x%lx bytes in memory\n", maxAddr-minAddr);
		goto out;
	}
	slide = base - minAddr;

	for (int i = 0; i < elfHdr.e_phnum; i++)
	{
		ElfW(Phdr)* phdr = (ElfW(Phdr)*) (((char*) phdrs) + (i * elfHdr.e_phentsize));

		if (phdr->p_type == PT_LOAD)
		{
			int prot = 0;
			int flags = MAP_FIXED;
			void* result;

			uintptr_t addr = phdr->p_vaddr + slide;
			uintptr_t size = phdr->p_filesz + ELF_PAGEOFFSET(phdr->p_vaddr);
			uintptr_t memsize = phdr->p_memsz + ELF_PAGEOFFSET(phdr->p_vaddr);
		    uintptr_t off = phdr->p_offset - ELF_PAGEOFFSET(phdr->p_vaddr);

		    addr = ELF_PAGESTART(addr);
		    size = ELF_PAGEALIGN(size);

			if (phdr->p_flags & PF_X)
				prot |= PROT_EXEC;
			if (phdr->p_flags & PF_W)
				prot |= PROT_WRITE;
			if (phdr->p_flags & PF_R)
				prot |= PROT_READ;

			if (phdr->p_flags & PF_W)
				flags |= MAP_PRIVATE;
			else
				flags |= MAP_SHARED;

			if (mprotect((void*) (addr), memsize, prot) == -1)
			{
				perror("mprotect");
			}

			result = mmap((void*) addr, size, prot, flags, fd, off);
			if (result == MAP_FAILED)
			{
				perror("mmap");
				goto out;
			}

			/*
			if (phdr->p_filesz < phdr->p_memsz)
			{
				if (mprotect((void*) (addr + phdr->p_filesz), phdr->p_memsz - phdr->p_filesz, prot) == -1)
				{
					perror("mprotect");
					goto out;
				}
			}
			*/
		}
		else if (phdr->p_type == PT_INTERP)
		{
			char* interp = malloc(phdr->p_filesz + 1);

			if (pread(fd, interp, phdr->p_filesz, phdr->p_offset) != phdr->p_filesz)
			{
				free(interp);
				perror("reading PT_INTERP");
				goto out;
			}
			interp[phdr->p_filesz] = '\0';

			// Load interpreter
			if (access(interp, F_OK) != 0)
			{
				char* prefixed = (char*) malloc(phdr->p_filesz + sizeof(SYSTEM_ROOT));

				strcpy(prefixed, SYSTEM_ROOT);
				strcat(prefixed, interp);

				free(interp);
				interp = prefixed;
			}

			if (access(interp, F_OK) != 0)
			{
				fprintf(stderr, "Cannot load interpreter at %s\n", interp);
				free(interp);
				goto out;
			}

			load(interp, lc, true);

			free(interp);
		}
	}

	if (isInterp)
	{
		lc->interp_base = base;
		lc->interp_entry = slide + elfHdr.e_entry;
	}
	else
	{
		lc->exec_phstart = slide + elfHdr.e_phoff;
		lc->exec_phentsize = elfHdr.e_phentsize;
		lc->exec_phnum = elfHdr.e_phnum;
		lc->exec_entry = slide + elfHdr.e_entry;
	}

out:
	free(phdrs);
	close(fd);
}

void loader_return(void)
{
	longjmp(jmpbuf, 1);
}

#ifdef TEST

int main(int argc, const char** argv)
{
	if (argc > 1)
		run(argv[1]);
	return 0;
}

#endif

