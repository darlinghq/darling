/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

// Host-side test of mldr's Mach-O segment mapping (loader.c).
// Builds small Mach-O images in a temporary file and maps them with the real
// 64-bit loader, then checks what a Mach-O image may rely on:
// - a segment whose filesize < vmsize reads back its file data followed by
//   zeros up to vmsize, including the last, partial page;
// - __PAGEZERO and segments with vmsize 0 (as Go emits for __DWARF, at the
//   same address as __LINKEDIT) are not mapped and do not fail the load;
// - a dynamic linker whose __TEXT starts at vmaddr 0 is mapped at its slide;
// - on arm64 images, the anonymous part of a segment extends to the 16 KB
//   Mach-O page, whatever the host page size;
// - on arm64 hosts, LC_UNIXTHREAD's entry point is read from a validated
//   ARM_THREAD_STATE64 and malformed thread states are refused; images without
//   a preferred address get distinct slots from 64 GB up, below 2^47, and an
//   occupied slot is skipped without replacing what is mapped there.
// Each case runs in a child process, as the loader exits on a failed mapping.
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <mach-o/loader.h>
#include <mach-o/fat.h>
#include "loader.h"

// The definitions mldr.c provides to loader.c.
#define PAGE_SIZE ((size_t) sysconf(_SC_PAGESIZE))
#define PAGE_ALIGN(x) (x & ~(PAGE_SIZE-1))

static int code_signing_gate(int fd, uint64_t image_offset, struct load_results* lr)
{
	(void) image_offset; (void) lr;
	return fd;
}

static void seal_segment(const char* name, uintptr_t address, uint64_t file_size, int prot)
{
	(void) name; (void) address; (void) file_size; (void) prot;
}

static void setup_space(struct load_results* lr, bool is_64_bit)
{
	(void) lr; (void) is_64_bit;
}

static void load(const char* path, cpu_type_t cpu, bool expect_dylinker, char** argv, struct load_results* lr)
{
	(void) cpu; (void) expect_dylinker; (void) argv; (void) lr;
	fprintf(stderr, "unexpected load of %s\n", path);
	exit(2);
}

static void* compatible_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
	return mmap(addr, length, prot, flags, fd, offset);
}

static int native_prot(int prot)
{
	int out = 0;
	if (prot & VM_PROT_READ)
		out |= PROT_READ;
	if (prot & VM_PROT_WRITE)
		out |= PROT_WRITE;
	if (prot & VM_PROT_EXECUTE)
		out |= PROT_EXEC;
	return out;
}

#define GEN_64BIT
#include "loader.c"
#undef GEN_64BIT

#if defined(__aarch64__)
#	define TEST_CPU CPU_TYPE_ARM64
#else
#	define TEST_CPU CPU_TYPE_X86_64
#endif

// Every offset and address below is a multiple of 64 KB, so the fixtures are
// valid for 4, 16 and 64 KB host pages.
#define UNIT 0x10000ULL
#define IMAGE_BASE 0x100000000ULL

#define FILE_BYTE 0x11
#define TRAILING_BYTE 0xAA

#define DATA_FILESIZE 0x100
#define DATA_VMSIZE (2 * UNIT + 0x10) // the last page is partial
#define LINKEDIT_FILESIZE 0x100

static int failures;

static void check(int ok, const char* what)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		failures++;
}

struct fixture {
	uint8_t* bytes;
	size_t size;
	struct mach_header_64* header;
	size_t cmd_offset;
};

static void fixture_init(struct fixture* f, uint32_t filetype, uint32_t flags, size_t size)
{
	f->size = size;
	f->bytes = calloc(1, size);
	if (!f->bytes) {
		perror("calloc");
		exit(1);
	}
	f->header = (struct mach_header_64*) f->bytes;
	f->header->magic = MH_MAGIC_64;
	f->header->cputype = TEST_CPU;
	f->header->filetype = filetype;
	f->header->flags = flags;
	f->cmd_offset = sizeof(*f->header);
}

static void add_segment(struct fixture* f, const char* name, uint64_t vmaddr, uint64_t vmsize,
		uint64_t fileoff, uint64_t filesize, vm_prot_t prot)
{
	struct segment_command_64* seg = (struct segment_command_64*) (f->bytes + f->cmd_offset);
	seg->cmd = LC_SEGMENT_64;
	seg->cmdsize = sizeof(*seg);
	strncpy(seg->segname, name, sizeof(seg->segname));
	seg->vmaddr = vmaddr;
	seg->vmsize = vmsize;
	seg->fileoff = fileoff;
	seg->filesize = filesize;
	seg->maxprot = prot;
	seg->initprot = prot;
	f->cmd_offset += seg->cmdsize;
	f->header->ncmds++;
	f->header->sizeofcmds += seg->cmdsize;
}

static int fixture_file(const struct fixture* f)
{
	char path[] = "/tmp/mldr-segment-mapping.XXXXXX";
	int fd = mkstemp(path);
	if (fd < 0) {
		perror("mkstemp");
		exit(1);
	}
	unlink(path);
	if (write(fd, f->bytes, f->size) != (ssize_t) f->size) {
		perror("write");
		exit(1);
	}
	lseek(fd, 0, SEEK_SET);
	return fd;
}

static bool all_bytes(const uint8_t* p, size_t length, uint8_t value)
{
	for (size_t i = 0; i < length; i++)
		if (p[i] != value)
			return false;
	return true;
}

// An executable with __PAGEZERO, __TEXT, a __DATA segment whose zero-filled
// part spans pages and ends in a partial page, __LINKEDIT, and a zero-vmsize
// __DWARF segment at __LINKEDIT's address.
static void test_executable(void)
{
	struct fixture f;
	fixture_init(&f, MH_EXECUTE, MH_PIE, 3 * UNIT);

	add_segment(&f, "__PAGEZERO", 0, IMAGE_BASE, 0, 0, VM_PROT_NONE);
	add_segment(&f, "__TEXT", IMAGE_BASE, UNIT, 0, UNIT, VM_PROT_READ | VM_PROT_EXECUTE);
	add_segment(&f, "__DATA", IMAGE_BASE + UNIT, DATA_VMSIZE, UNIT, DATA_FILESIZE, VM_PROT_READ | VM_PROT_WRITE);
	const uint64_t linkedit = IMAGE_BASE + 4 * UNIT;
	add_segment(&f, "__LINKEDIT", linkedit, UNIT, 2 * UNIT, LINKEDIT_FILESIZE, VM_PROT_READ);
	add_segment(&f, "__DWARF", linkedit, 0, 2 * UNIT, LINKEDIT_FILESIZE, VM_PROT_READ);

	// __DATA's file data, then other bytes up to the next segment's data
	memset(f.bytes + UNIT, FILE_BYTE, DATA_FILESIZE);
	memset(f.bytes + UNIT + DATA_FILESIZE, TRAILING_BYTE, UNIT - DATA_FILESIZE);
	memset(f.bytes + 2 * UNIT, 0x22, LINKEDIT_FILESIZE);

	int fd = fixture_file(&f);
	struct load_results lr = {0};
	load64(fd, false, &lr);

	const struct mach_header_64* mh = (const struct mach_header_64*) lr.mh;
	check(mh != NULL && mh->magic == MH_MAGIC_64 && mh->ncmds == 5, "executable: header mapped");
	const uintptr_t slide = (uintptr_t) mh - IMAGE_BASE;

	uint8_t* data = (uint8_t*) (IMAGE_BASE + UNIT + slide);
	check(all_bytes(data, DATA_FILESIZE, FILE_BYTE), "executable: __DATA file data");
	check(all_bytes(data + DATA_FILESIZE, DATA_VMSIZE - DATA_FILESIZE, 0),
		"executable: __DATA zero-filled up to vmsize, last partial page included");
	data[DATA_VMSIZE - 1] = 1;
	check(data[DATA_VMSIZE - 1] == 1, "executable: last byte of __DATA writable");

	const uint8_t* le = (const uint8_t*) (linkedit + slide);
	check(all_bytes(le, LINKEDIT_FILESIZE, 0x22), "executable: __LINKEDIT despite a zero-vmsize segment at its address");
	check(lr.vm_addr_max >= linkedit + slide + UNIT, "executable: vm_addr_max covers __LINKEDIT");

	if (TEST_CPU == CPU_TYPE_ARM64) {
		// arm64 Mach-O pages are 16 KB: the page holding __DATA's end is
		// mapped to its end, also on a 4 KB host.
		uintptr_t end = (uintptr_t) data + DATA_VMSIZE;
		uintptr_t page_end = (end + 0x3fff) & ~(uintptr_t) 0x3fff;
		check(all_bytes((const uint8_t*) end, page_end - end, 0), "executable: arm64 __DATA mapped to its 16 KB page");
	}

	close(fd);
	free(f.bytes);
}

// A dynamic linker whose __TEXT starts at vmaddr 0.
static void test_dylinker(void)
{
	struct fixture f;
	fixture_init(&f, MH_DYLINKER, 0, 2 * UNIT);

	add_segment(&f, "__TEXT", 0, UNIT, 0, UNIT, VM_PROT_READ | VM_PROT_EXECUTE);
	add_segment(&f, "__DATA", UNIT, UNIT, UNIT, DATA_FILESIZE, VM_PROT_READ | VM_PROT_WRITE);
	memset(f.bytes + UNIT, FILE_BYTE, DATA_FILESIZE);

	int fd = fixture_file(&f);
	struct load_results lr = {0};
	load64(fd, true, &lr);

	// The image's range ends at lr.base (slide + 2 units).
	const uintptr_t slide = lr.base - 2 * UNIT;
	const struct mach_header_64* mh = (const struct mach_header_64*) slide;
	check(lr.base != 0 && slide != 0, "dylinker: slid away from address 0");
	check(mh->magic == MH_MAGIC_64 && mh->filetype == MH_DYLINKER, "dylinker: zero-based __TEXT mapped at the slide");
	check(all_bytes((const uint8_t*) (slide + UNIT), DATA_FILESIZE, FILE_BYTE), "dylinker: __DATA mapped at the slide");

	close(fd);
	free(f.bytes);
}

#if defined(__aarch64__)
#define ENTRY_OFFSET 0x1000
#define SENTINEL_ADDR 0x1000000000ULL

enum thread_case { THREAD_VALID, THREAD_TRUNCATED, THREAD_BAD_FLAVOR, THREAD_SHORT_COUNT, THREAD_LONG_COUNT };
static enum thread_case thread_case;

struct arm_thread_command {
	uint32_t cmd;
	uint32_t cmdsize;
	uint32_t flavor;
	uint32_t count;
	uint64_t x[29];
	uint64_t fp, lr, sp, pc;
	uint32_t cpsr;
	uint32_t pad;
};

// An executable whose entry point comes from LC_UNIXTHREAD.
static void test_unixthread(void)
{
	struct fixture f;
	fixture_init(&f, MH_EXECUTE, MH_PIE, UNIT);

	add_segment(&f, "__PAGEZERO", 0, IMAGE_BASE, 0, 0, VM_PROT_NONE);
	add_segment(&f, "__TEXT", IMAGE_BASE, UNIT, 0, UNIT, VM_PROT_READ | VM_PROT_EXECUTE);

	struct arm_thread_command* tc = (struct arm_thread_command*) (f.bytes + f.cmd_offset);
	tc->cmd = LC_UNIXTHREAD;
	tc->cmdsize = sizeof(*tc);
	tc->flavor = 6; // ARM_THREAD_STATE64
	tc->count = (sizeof(*tc) - 4 * sizeof(uint32_t)) / sizeof(uint32_t); // ARM_THREAD_STATE64_COUNT, 68
	tc->pc = IMAGE_BASE + ENTRY_OFFSET;
	tc->sp = 0x5555; // not the entry point
	f.cmd_offset += sizeof(*tc);
	f.header->ncmds++;
	f.header->sizeofcmds += sizeof(*tc);

	switch (thread_case) {
		case THREAD_VALID: break;
		case THREAD_TRUNCATED: tc->cmdsize = 16; f.header->sizeofcmds -= sizeof(*tc) - 16; break;
		case THREAD_BAD_FLAVOR: tc->flavor = 1; break;
		case THREAD_SHORT_COUNT: tc->count--; break;
		case THREAD_LONG_COUNT: tc->count++; break;
	}

	int fd = fixture_file(&f);
	struct load_results lr = {0};
	load64(fd, false, &lr);

	check(lr.mh != 0 && lr.entry_point == lr.mh + ENTRY_OFFSET, "unixthread: entry point is the slid pc");

	close(fd);
	free(f.bytes);
}

static int load_dylinker_slot(uintptr_t* start, uintptr_t* end)
{
	struct fixture f;
	fixture_init(&f, MH_DYLINKER, 0, UNIT);
	add_segment(&f, "__TEXT", 0, UNIT, 0, UNIT, VM_PROT_READ | VM_PROT_EXECUTE);

	int fd = fixture_file(&f);
	struct load_results lr = {0};
	load64(fd, true, &lr);
	close(fd);
	free(f.bytes);

	*end = lr.base;
	*start = lr.base - UNIT;
	return ((const struct mach_header_64*) *start)->magic == MH_MAGIC_64;
}

// Images without a preferred address (base 0) on arm64.
static void test_low_slots(void)
{
	const size_t page = sysconf(_SC_PAGESIZE);
	volatile char* sentinel = mmap((void*) SENTINEL_ADDR, page, PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED_NOREPLACE, -1, 0);
	if (sentinel != (void*) SENTINEL_ADDR) {
		perror("mmap sentinel");
		exit(1);
	}
	sentinel[0] = 42;

	uintptr_t start1, end1, start2, end2;
	check(load_dylinker_slot(&start1, &end1), "low slots: first image mapped");
	check(start1 > SENTINEL_ADDR && end1 <= 0x800000000000ULL, "low slots: first slot from 64 GB up, below 2^47, past an occupied slot");
	check(sentinel[0] == 42, "low slots: the occupied slot keeps its mapping");
	check(load_dylinker_slot(&start2, &end2), "low slots: second image mapped");
	check(start2 >= end1 && end2 <= 0x800000000000ULL, "low slots: second image gets the next slot");
}
#endif

static void run_expect(void (*test)(void), const char* name, int expected_status)
{
	fflush(stdout);
	pid_t pid = fork();
	if (pid < 0) {
		perror("fork");
		exit(1);
	}
	if (pid == 0) {
		failures = 0;
		test();
		fflush(stdout);
		_exit(failures ? 1 : 0);
	}

	int status;
	while (waitpid(pid, &status, 0) < 0 && errno == EINTR)
		;
	char what[128];
	if (WIFEXITED(status))
		snprintf(what, sizeof(what), "%s (exit status %d)", name, WEXITSTATUS(status));
	else
		snprintf(what, sizeof(what), "%s (signal %d)", name, WTERMSIG(status));
	check(WIFEXITED(status) && WEXITSTATUS(status) == expected_status, what);
}

static void run(void (*test)(void), const char* name)
{
	run_expect(test, name, 0);
}

int main(void)
{
	run(test_executable, "executable");
	run(test_dylinker, "dylinker");
#if defined(__aarch64__)
	thread_case = THREAD_VALID;
	run(test_unixthread, "unixthread");
	// a malformed thread state stops the load with exit status 1
	thread_case = THREAD_TRUNCATED;
	run_expect(test_unixthread, "unixthread truncated command refused", 1);
	thread_case = THREAD_BAD_FLAVOR;
	run_expect(test_unixthread, "unixthread other flavor refused", 1);
	thread_case = THREAD_SHORT_COUNT;
	run_expect(test_unixthread, "unixthread short count refused", 1);
	thread_case = THREAD_LONG_COUNT;
	run_expect(test_unixthread, "unixthread count past the command refused", 1);
	run(test_low_slots, "low slots");
#endif
	return failures ? 1 : 0;
}
