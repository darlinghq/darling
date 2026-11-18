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
#if __has_include(<mach-o/loader.h>)
#include <mach-o/loader.h>
#endif
#if __has_include(<mach-o/fat.h>)
#include <mach-o/fat.h>
#endif
#include "loader.h"
#include "exec_policy.h"

// The definitions mldr.c provides to loader.c.
#define PAGE_SIZE ((size_t) sysconf(_SC_PAGESIZE))
#define PAGE_ALIGN(x) (x & ~(PAGE_SIZE-1))

#if defined(__aarch64__)
// The thread-pointer-relative TSD slot offset mldr's elfcalls provide.
#define TEST_TSD_SLOT_OFFSET 64
uintptr_t __darling_arm64_tsd_slot_offset(void)
{
	return TEST_TSD_SLOT_OFFSET;
}
// The thread-pointer-relative offset of the rseq cpu_id_start field.
#define TEST_RSEQ_CPU_OFFSET (-32)
bool __darling_arm64_rseq_cpu_offset(intptr_t* offset)
{
	*offset = TEST_RSEQ_CPU_OFFSET;
	return true;
}
#endif

static int code_signing_gate(int fd, uint64_t image_offset, struct load_results* lr)
{
	(void) image_offset; (void) lr;
	return fd;
}

struct seal_record {
	char name[16];
	uintptr_t address;
	uint64_t file_size;
	int prot;
};
#define MAX_SEALS 16
static struct seal_record recorded_seals[MAX_SEALS];
static size_t recorded_seal_count = 0;

static void seal_segment(const char* name, uintptr_t address, uint64_t file_size, int prot)
{
	if (recorded_seal_count < MAX_SEALS) {
		strncpy(recorded_seals[recorded_seal_count].name, name ? name : "", sizeof(recorded_seals[0].name) - 1);
		recorded_seals[recorded_seal_count].name[sizeof(recorded_seals[0].name) - 1] = '\0';
		recorded_seals[recorded_seal_count].address = address;
		recorded_seals[recorded_seal_count].file_size = file_size;
		recorded_seals[recorded_seal_count].prot = prot;
		recorded_seal_count++;
	}
}

static bool setup_space_saw_emulated_x86_64;
static int32_t setup_space_saw_cputype;

static void setup_space(struct load_results* lr, bool is_64_bit)
{
	(void) is_64_bit;
	setup_space_saw_emulated_x86_64 = lr->emulated_x86_64;
	setup_space_saw_cputype = lr->cputype;
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
	char path[512];
	const char* tmp = getenv("TMPDIR");
	snprintf(path, sizeof(path), "%s/mldr-segment-mapping.XXXXXX", (tmp && tmp[0]) ? tmp : "/tmp");
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

struct x86_thread_command {
	uint32_t cmd;
	uint32_t cmdsize;
	uint32_t flavor;
	uint32_t count;
	uint64_t rax, rbx, rcx, rdx, rdi, rsi, rbp, rsp;
	uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
	uint64_t rip, rflags;
	uint64_t cs, fs, gs;
};

static void test_unixthread_x86_64_on_arm64(void)
{
	struct fixture f;
	fixture_init(&f, MH_EXECUTE, MH_PIE, UNIT);
	f.header->cputype = CPU_TYPE_X86_64;

	add_segment(&f, "__PAGEZERO", 0, IMAGE_BASE, 0, 0, VM_PROT_NONE);
	add_segment(&f, "__TEXT", IMAGE_BASE, UNIT, 0, UNIT, VM_PROT_READ | VM_PROT_EXECUTE);

	struct x86_thread_command* tc = (struct x86_thread_command*) (f.bytes + f.cmd_offset);
	tc->cmd = LC_UNIXTHREAD;
	tc->cmdsize = sizeof(*tc);
	tc->flavor = 4; // x86_THREAD_STATE64
	tc->count = (sizeof(*tc) - 4 * sizeof(uint32_t)) / sizeof(uint32_t); // 42
	tc->rip = IMAGE_BASE + ENTRY_OFFSET;
	tc->rsp = 0x7777;
	f.cmd_offset += sizeof(*tc);
	f.header->ncmds++;
	f.header->sizeofcmds += sizeof(*tc);

	switch (thread_case) {
		case THREAD_VALID: break;
		case THREAD_TRUNCATED: tc->cmdsize = 16; f.header->sizeofcmds -= sizeof(*tc) - 16; break;
		case THREAD_BAD_FLAVOR: tc->flavor = 6; break;
		case THREAD_SHORT_COUNT: tc->count--; break;
		case THREAD_LONG_COUNT: tc->count++; break;
	}

	setup_space_saw_emulated_x86_64 = false;
	setup_space_saw_cputype = 0;

	int fd = fixture_file(&f);
	struct load_results lr = {0};
	load64(fd, false, &lr);

	check(setup_space_saw_emulated_x86_64 && setup_space_saw_cputype == CPU_TYPE_X86_64,
		"x86_64 unixthread: setup_space sees emulated_x86_64 and CPU_TYPE_X86_64");
	check(lr.emulated_x86_64 && lr.cputype == CPU_TYPE_X86_64,
		"x86_64 unixthread: load_results records emulated_x86_64");
	check(lr.mh != 0 && lr.entry_point == lr.mh + ENTRY_OFFSET,
		"x86_64 unixthread: entry point is the slid rip");

	close(fd);
	free(f.bytes);
}

static void test_dylinker_arch_mismatch(void)
{
	struct fixture f;
	fixture_init(&f, MH_DYLINKER, 0, UNIT);
	f.header->cputype = CPU_TYPE_ARM64;
	add_segment(&f, "__TEXT", 0, UNIT, 0, UNIT, VM_PROT_READ | VM_PROT_EXECUTE);

	int fd = fixture_file(&f);
	struct load_results lr = {0};
	lr.cputype = CPU_TYPE_X86_64;
	load64(fd, true, &lr);
	close(fd);
	free(f.bytes);
}

static void test_fat_arch_selection_arm64(void)
{
	const struct fat_arch slices_both[] = {
		{ .cputype = CPU_TYPE_X86_64, .offset = 0x4000 },
		{ .cputype = CPU_TYPE_ARM64,  .offset = 0x8000 },
	};
	const struct fat_arch slices_x86_only[] = {
		{ .cputype = CPU_TYPE_ARM,    .offset = 0x2000 },
		{ .cputype = CPU_TYPE_X86_64, .offset = 0x6000 },
	};
	const uint32_t no_bprefs[4] = {0};
	const uint32_t x86_bprefs[4] = { CPU_TYPE_X86_64, 0, 0, 0 };

	struct fat_arch best = { .cputype = CPU_TYPE_ANY };
	int bpref_idx = -1;
	for (size_t i = 0; i < 2; i++)
		mldr_consider_fat_arch(&slices_both[i], 0, no_bprefs, &best, &bpref_idx);
	check(best.cputype == CPU_TYPE_ARM64 && best.offset == 0x8000,
		"fat selection: arm64 preferred over x86_64 by default");

	best = (struct fat_arch){ .cputype = CPU_TYPE_ANY };
	bpref_idx = -1;
	for (size_t i = 0; i < 2; i++)
		mldr_consider_fat_arch(&slices_x86_only[i], 0, no_bprefs, &best, &bpref_idx);
	check(best.cputype == CPU_TYPE_X86_64 && best.offset == 0x6000,
		"fat selection: x86_64 selected when arm64 slice is absent");

	best = (struct fat_arch){ .cputype = CPU_TYPE_ANY };
	bpref_idx = -1;
	for (size_t i = 0; i < 2; i++)
		mldr_consider_fat_arch(&slices_both[i], 0, x86_bprefs, &best, &bpref_idx);
	check(best.cputype == CPU_TYPE_X86_64 && best.offset == 0x4000,
		"fat selection: bprefs selects x86_64 over arm64");
}
#endif


#if defined(__aarch64__)
// The Darwin TSD base read pair (mrs Xn, TPIDRRO_EL0; and Xn, Xn, #~7) in an
// instruction section is rewritten to read the native TLS slot; the same words
// in a data section, and a lone TPIDRRO_EL0 CPU number read, are left alone. The
// CPU number read pair (mrs Xn, TPIDR_EL0; and Wd, Wn, #0xfff) loads the rseq
// CPU number instead. The code pages are executable and not writable afterwards.
static void test_tsd_reads(void)
{
	const uint32_t mrs_ro = 0xD53BD068u; // mrs x8, TPIDRRO_EL0
	const uint32_t and_not7 = 0x927DF108u; // and x8, x8, #~7
	const uint32_t mrs_el0 = 0xD53BD048u; // mrs x8, TPIDR_EL0
	const uint32_t ldr_slot = 0xF9400108u | ((TEST_TSD_SLOT_OFFSET / 8) << 10); // ldr x8, [x8, #slot]
	const uint32_t and7 = 0x92400908u; // and x8, x8, #7 (CPU number)
	const uint32_t and_low12 = 0x12002D00u; // and w0, w8, #0xfff
	const uint32_t ldur_cpu = 0xB85E0100u; // ldur w0, [x8, #-32]

	struct fixture f;
	fixture_init(&f, MH_EXECUTE, MH_PIE, UNIT);
	struct segment_command_64* seg = (struct segment_command_64*) (f.bytes + f.cmd_offset);
	seg->cmd = LC_SEGMENT_64;
	seg->cmdsize = sizeof(*seg) + 2 * sizeof(struct section_64);
	strncpy(seg->segname, "__TEXT", sizeof(seg->segname));
	seg->vmaddr = IMAGE_BASE;
	seg->vmsize = UNIT;
	seg->filesize = UNIT;
	seg->maxprot = seg->initprot = VM_PROT_READ | VM_PROT_EXECUTE;
	seg->nsects = 2;
	struct section_64* sect = (struct section_64*) (seg + 1);
	strncpy(sect[0].sectname, "__text", sizeof(sect[0].sectname));
	strncpy(sect[0].segname, "__TEXT", sizeof(sect[0].segname));
	sect[0].addr = IMAGE_BASE + 0x1000;
	sect[0].size = 0x18;
	sect[0].offset = 0x1000;
	sect[0].flags = S_REGULAR | S_ATTR_PURE_INSTRUCTIONS | S_ATTR_SOME_INSTRUCTIONS;
	strncpy(sect[1].sectname, "__const", sizeof(sect[1].sectname));
	strncpy(sect[1].segname, "__TEXT", sizeof(sect[1].segname));
	sect[1].addr = IMAGE_BASE + 0x2000;
	sect[1].size = 0x18;
	sect[1].offset = 0x2000;
	sect[1].flags = S_REGULAR;
	f.cmd_offset += seg->cmdsize;
	f.header->ncmds++;
	f.header->sizeofcmds += seg->cmdsize;

	uint32_t* text = (uint32_t*) (f.bytes + 0x1000);
	text[0] = mrs_ro; text[1] = and_not7; text[2] = mrs_ro; text[3] = and7; text[4] = mrs_el0; text[5] = and_low12;
	uint32_t* data = (uint32_t*) (f.bytes + 0x2000);
	data[0] = mrs_ro; data[1] = and_not7; data[2] = mrs_ro; data[3] = and7; data[4] = mrs_el0; data[5] = and_low12;

	int fd = fixture_file(&f);
	struct load_results lr = {0};
	load64(fd, false, &lr);
	const uintptr_t slide = lr.mh - IMAGE_BASE;
	const uint32_t* t = (const uint32_t*) (IMAGE_BASE + 0x1000 + slide);
	const uint32_t* d = (const uint32_t*) (IMAGE_BASE + 0x2000 + slide);

	check(t[0] == mrs_el0 && t[1] == ldr_slot, "tsd reads: the pair in an instruction section reads the native TLS slot");
	check(t[2] == mrs_ro && t[3] == and7, "tsd reads: a CPU number read is left alone");
	check(t[4] == mrs_el0 && t[5] == ldur_cpu, "tsd reads: the TPIDR_EL0 CPU number pair loads the rseq CPU number");
	check(d[0] == mrs_ro && d[1] == and_not7 && d[4] == mrs_el0 && d[5] == and_low12, "tsd reads: the same words in a data section are left alone");

	// the code pages are r-x
	char line[256];
	bool found = false, exec_only = false;
	FILE* maps = fopen("/proc/self/maps", "r");
	while (maps && fgets(line, sizeof(line), maps)) {
		unsigned long lo, hi;
		char perms[8];
		if (sscanf(line, "%lx-%lx %7s", &lo, &hi, perms) == 3 && lo <= (uintptr_t) t && (uintptr_t) t < hi) {
			found = true;
			exec_only = strncmp(perms, "r-x", 3) == 0;
		}
	}
	if (maps)
		fclose(maps);
	check(found && exec_only, "tsd reads: the rewritten page is readable and executable, not writable");

	close(fd);
	free(f.bytes);
}
#endif

static bool get_mapping_perms(uintptr_t addr, char perms_out[5])
{
	FILE* f = fopen("/proc/self/maps", "r");
	if (!f) return false;
	char line[512];
	bool found = false;
	while (fgets(line, sizeof(line), f))
	{
		uintptr_t start, end;
		char perms[5];
		if (sscanf(line, "%lx-%lx %4s", &start, &end, perms) == 3)
		{
			if (addr >= start && addr < end)
			{
				memcpy(perms_out, perms, 4);
				perms_out[4] = '\0';
				found = true;
				break;
			}
		}
	}
	fclose(f);
	return found;
}

static void test_subpage_isolated_rw(void)
{
	const size_t host_ps = sysconf(_SC_PAGESIZE);
	const size_t sub_off = host_ps / 4;
	const size_t sub_sz = host_ps / 4;

	struct fixture f;
	fixture_init(&f, MH_EXECUTE, MH_PIE, 4 * host_ps);

	add_segment(&f, "__PAGEZERO", 0, IMAGE_BASE, 0, 0, VM_PROT_NONE);
	add_segment(&f, "__TEXT", IMAGE_BASE, host_ps, 0, host_ps, VM_PROT_READ | VM_PROT_EXECUTE);
	add_segment(&f, "__DATA", IMAGE_BASE + 2 * host_ps + sub_off, sub_sz, host_ps, 0x100, VM_PROT_READ | VM_PROT_WRITE);

	memset(f.bytes + host_ps, 0x33, 0x100);

	int fd = fixture_file(&f);
	struct load_results lr = {0};
	load64(fd, false, &lr);

	const uintptr_t slide = lr.mh - IMAGE_BASE;
	uintptr_t data_page = (IMAGE_BASE + 2 * host_ps + sub_off + slide) & ~(host_ps - 1);
	char perms[5] = {0};
	bool ok = get_mapping_perms(data_page, perms);
	check(ok, "isolated subpage rw: mapping found in /proc/self/maps");
	check(strcmp(perms, "rw-p") == 0, "isolated subpage rw: has rw-p and no x");

	close(fd);
	free(f.bytes);
}

static void test_subpage_isolated_rx(void)
{
	const size_t host_ps = sysconf(_SC_PAGESIZE);
	const size_t sub_off = host_ps / 4;
	const size_t sub_sz = host_ps / 4;

	struct fixture f;
	fixture_init(&f, MH_EXECUTE, MH_PIE, 4 * host_ps);

	add_segment(&f, "__PAGEZERO", 0, IMAGE_BASE, 0, 0, VM_PROT_NONE);
	add_segment(&f, "__TEXT", IMAGE_BASE, host_ps, 0, host_ps, VM_PROT_READ | VM_PROT_EXECUTE);
	add_segment(&f, "__SUBTEXT", IMAGE_BASE + 2 * host_ps + sub_off, sub_sz, host_ps, 0x100, VM_PROT_READ | VM_PROT_EXECUTE);

	memset(f.bytes + host_ps, 0x44, 0x100);

	int fd = fixture_file(&f);
	struct load_results lr = {0};
	load64(fd, false, &lr);

	const uintptr_t slide = lr.mh - IMAGE_BASE;
	uintptr_t sub_page = (IMAGE_BASE + 2 * host_ps + sub_off + slide) & ~(host_ps - 1);
	char perms[5] = {0};
	bool ok = get_mapping_perms(sub_page, perms);
	check(ok, "isolated subpage rx: mapping found in /proc/self/maps");
	check(strcmp(perms, "r-xp") == 0, "isolated subpage rx: has r-xp and no w");

	close(fd);
	free(f.bytes);
}

static void test_subpage_shared_rx_rw(void)
{
	const size_t host_ps = sysconf(_SC_PAGESIZE);
	const size_t sub_off = host_ps / 2;

	struct fixture f;
	fixture_init(&f, MH_EXECUTE, MH_PIE, 5 * host_ps);

	add_segment(&f, "__PAGEZERO", 0, IMAGE_BASE, 0, 0, VM_PROT_NONE);
	add_segment(&f, "__TEXT", IMAGE_BASE + host_ps, sub_off, 0, sub_off, VM_PROT_READ | VM_PROT_EXECUTE);
	add_segment(&f, "__DATA", IMAGE_BASE + host_ps + sub_off, 2 * host_ps, sub_off, 0x100, VM_PROT_READ | VM_PROT_WRITE);

	memset(f.bytes + sub_off, 0x55, 0x100);

	int fd = fixture_file(&f);
	struct load_results lr = {0};
	load64(fd, false, &lr);

	const uintptr_t slide = lr.mh - (IMAGE_BASE + host_ps);
	uintptr_t shared_page = IMAGE_BASE + host_ps + slide;
	uintptr_t pure_rw_page = shared_page + host_ps;

	char perms[5] = {0};
	check(get_mapping_perms(shared_page, perms), "shared rx+rw: shared page in maps");
	check(strcmp(perms, "rwxp") == 0, "shared rx+rw: shared page retains union rwxp");

	check(get_mapping_perms(pure_rw_page, perms), "shared rx+rw: pure rw page in maps");
	check(strcmp(perms, "rw-p") == 0, "shared rx+rw: non-shared page stays rw-p (no x)");

	close(fd);
	free(f.bytes);
}

static void test_subpage_shared_rw_r(void)
{
	const size_t host_ps = sysconf(_SC_PAGESIZE);
	const size_t sub_off = host_ps / 2;

	struct fixture f;
	fixture_init(&f, MH_EXECUTE, MH_PIE, 5 * host_ps);

	add_segment(&f, "__PAGEZERO", 0, IMAGE_BASE, 0, 0, VM_PROT_NONE);
	add_segment(&f, "__TEXT", IMAGE_BASE, host_ps, 0, host_ps, VM_PROT_READ | VM_PROT_EXECUTE);
	add_segment(&f, "__DATA", IMAGE_BASE + 2 * host_ps, sub_off, host_ps, sub_off, VM_PROT_READ | VM_PROT_WRITE);
	add_segment(&f, "__RO", IMAGE_BASE + 2 * host_ps + sub_off, host_ps - sub_off, host_ps + sub_off, 0x100, VM_PROT_READ);

	int fd = fixture_file(&f);
	struct load_results lr = {0};
	load64(fd, false, &lr);

	const uintptr_t slide = lr.mh - IMAGE_BASE;
	uintptr_t shared_page = IMAGE_BASE + 2 * host_ps + slide;

	char perms[5] = {0};
	check(get_mapping_perms(shared_page, perms), "shared rw+r: shared page in maps");
	check(strcmp(perms, "rw-p") == 0, "shared rw+r: shared page retains union rw-p (no x)");

	close(fd);
	free(f.bytes);
}

static void test_subpage_unaligned_bss(void)
{
	const size_t host_ps = sysconf(_SC_PAGESIZE);
	const size_t sub_off = host_ps / 4;

	struct fixture f;
	fixture_init(&f, MH_EXECUTE, MH_PIE, 4 * host_ps);

	add_segment(&f, "__PAGEZERO", 0, IMAGE_BASE, 0, 0, VM_PROT_NONE);
	add_segment(&f, "__TEXT", IMAGE_BASE, host_ps, 0, host_ps, VM_PROT_READ | VM_PROT_EXECUTE);
	add_segment(&f, "__BSS", IMAGE_BASE + 2 * host_ps + sub_off, host_ps, host_ps, 0, VM_PROT_READ | VM_PROT_WRITE);

	int fd = fixture_file(&f);
	struct load_results lr = {0};
	load64(fd, false, &lr);

	const uintptr_t slide = lr.mh - IMAGE_BASE;
	uintptr_t bss_page = (IMAGE_BASE + 2 * host_ps + sub_off + slide) & ~(host_ps - 1);
	char perms[5] = {0};
	check(get_mapping_perms(bss_page, perms), "unaligned bss: mapped in maps");
	check(strcmp(perms, "rw-p") == 0, "unaligned bss: has rw-p");

	close(fd);
	free(f.bytes);
}

static void test_seal_readonly_segments(void)
{
	const size_t host_ps = sysconf(_SC_PAGESIZE);
	struct fixture f;
	fixture_init(&f, MH_EXECUTE, MH_PIE, 4 * host_ps);

	add_segment(&f, "__PAGEZERO", 0, IMAGE_BASE, 0, 0, VM_PROT_NONE);
	add_segment(&f, "__TEXT", IMAGE_BASE, host_ps, 0, host_ps, VM_PROT_READ | VM_PROT_EXECUTE);
	add_segment(&f, "__DATA", IMAGE_BASE + 2 * host_ps, host_ps, host_ps, host_ps, VM_PROT_READ | VM_PROT_WRITE);

	int fd = fixture_file(&f);
	struct load_results lr = {0};
	lr.seal_images = true;
	recorded_seal_count = 0;
	load64(fd, false, &lr);

	const uintptr_t slide = lr.mh - IMAGE_BASE;
	check(recorded_seal_count == 1, "seal readonly: exactly 1 segment sealed");
	if (recorded_seal_count >= 1) {
		check(strcmp(recorded_seals[0].name, "__TEXT") == 0, "seal readonly: __TEXT sealed");
		check(recorded_seals[0].address == IMAGE_BASE + slide, "seal readonly: __TEXT address matches");
		check(recorded_seals[0].file_size == host_ps, "seal readonly: __TEXT length matches");
		check(recorded_seals[0].prot == (PROT_READ | PROT_EXEC), "seal readonly: __TEXT prot is R-X");
	}

	close(fd);
	free(f.bytes);
}

static void test_seal_shared_page_with_writable_not_sealed(void)
{
	const size_t host_ps = sysconf(_SC_PAGESIZE);
	const size_t sub_off = host_ps / 2;

	struct fixture f;
	fixture_init(&f, MH_EXECUTE, MH_PIE, 4 * host_ps);

	add_segment(&f, "__PAGEZERO", 0, IMAGE_BASE, 0, 0, VM_PROT_NONE);
	add_segment(&f, "__TEXT", IMAGE_BASE + host_ps, sub_off, 0, sub_off, VM_PROT_READ | VM_PROT_EXECUTE);
	add_segment(&f, "__DATA", IMAGE_BASE + host_ps + sub_off, host_ps, sub_off, 0x100, VM_PROT_READ | VM_PROT_WRITE);

	int fd = fixture_file(&f);
	struct load_results lr = {0};
	lr.seal_images = true;
	recorded_seal_count = 0;
	load64(fd, false, &lr);

	check(recorded_seal_count == 0, "seal shared with writable: no segments sealed");

	close(fd);
	free(f.bytes);
}

static void test_seal_contiguous_runs_differing_prot(void)
{
	const size_t host_ps = sysconf(_SC_PAGESIZE);
	const size_t sub_off = host_ps / 2;

	struct fixture f;
	fixture_init(&f, MH_EXECUTE, MH_PIE, 6 * host_ps);

	add_segment(&f, "__PAGEZERO", 0, IMAGE_BASE, 0, 0, VM_PROT_NONE);
	add_segment(&f, "__TEXT", IMAGE_BASE + host_ps, host_ps + sub_off, 0, host_ps + sub_off, VM_PROT_READ);
	add_segment(&f, "__SUBEXEC", IMAGE_BASE + 2 * host_ps + sub_off, host_ps - sub_off, host_ps + sub_off, host_ps - sub_off, VM_PROT_READ | VM_PROT_EXECUTE);

	int fd = fixture_file(&f);
	struct load_results lr = {0};
	lr.seal_images = true;
	recorded_seal_count = 0;
	load64(fd, false, &lr);

	const uintptr_t slide = lr.mh - (IMAGE_BASE + host_ps);
	check(recorded_seal_count == 2, "seal contiguous differing prot: split into 2 runs");
	if (recorded_seal_count >= 2) {
		check(recorded_seals[0].address == IMAGE_BASE + host_ps + slide, "seal run 0 address");
		check(recorded_seals[0].file_size == host_ps, "seal run 0 len is 1 page");
		check(recorded_seals[0].prot == PROT_READ, "seal run 0 prot is PROT_READ");

		check(recorded_seals[1].address == IMAGE_BASE + 2 * host_ps + slide, "seal run 1 address");
		check(recorded_seals[1].file_size == host_ps, "seal run 1 len is 1 page");
		check(recorded_seals[1].prot == (PROT_READ | PROT_EXEC), "seal run 1 prot is PROT_READ | PROT_EXEC");
	}

	close(fd);
	free(f.bytes);
}

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
	run(test_subpage_isolated_rw, "subpage isolated rw");
	run(test_subpage_isolated_rx, "subpage isolated rx");
	run(test_subpage_shared_rx_rw, "subpage shared rx+rw");
	run(test_subpage_shared_rw_r, "subpage shared rw+r");
	run(test_subpage_unaligned_bss, "subpage unaligned bss");
	run(test_seal_readonly_segments, "seal readonly segments");
	run(test_seal_shared_page_with_writable_not_sealed, "seal shared page with writable not sealed");
	run(test_seal_contiguous_runs_differing_prot, "seal contiguous runs differing prot");
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
	thread_case = THREAD_VALID;
	run(test_unixthread_x86_64_on_arm64, "x86_64 unixthread on arm64");
	thread_case = THREAD_TRUNCATED;
	run_expect(test_unixthread_x86_64_on_arm64, "x86_64 unixthread truncated command refused", 1);
	thread_case = THREAD_BAD_FLAVOR;
	run_expect(test_unixthread_x86_64_on_arm64, "x86_64 unixthread other flavor refused", 1);
	thread_case = THREAD_SHORT_COUNT;
	run_expect(test_unixthread_x86_64_on_arm64, "x86_64 unixthread short count refused", 1);
	thread_case = THREAD_LONG_COUNT;
	run_expect(test_unixthread_x86_64_on_arm64, "x86_64 unixthread count past the command refused", 1);
	run_expect(test_dylinker_arch_mismatch, "dylinker arch mismatch refused", 1);
	run(test_fat_arch_selection_arm64, "fat arch selection on arm64");
	run(test_low_slots, "low slots");
	run(test_tsd_reads, "tsd reads");
#endif
	return failures ? 1 : 0;
}
