/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * task_info(TASK_VM_INFO): which fields move for anonymous, file-backed,
 * untouched and reusable memory. Prints the deltas; the checks that must hold
 * on macOS and on Darling make it exit non-zero when they fail.
 *   cc -o task-vm-info task-vm-info.c
 */
#include <fcntl.h>
#include <mach/mach.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define MIB (1024ULL * 1024ULL)
static int failed;
#define CHECK(c) do { if (!(c)) { printf("FAIL: %s (line %d)\n", #c, __LINE__); failed = 1; } } while (0)

static void get(task_vm_info_data_t *i, mach_msg_type_number_t count, kern_return_t *kr_out, mach_msg_type_number_t *count_out)
{
	memset(i, 0xAA, sizeof *i);
	mach_msg_type_number_t c = count;
	kern_return_t kr = task_info(mach_task_self(), TASK_VM_INFO, (task_info_t) i, &c);
	if (kr_out) *kr_out = kr;
	if (count_out) *count_out = c;
}

static void snap(const char *tag, task_vm_info_data_t *i)
{
	kern_return_t kr; mach_msg_type_number_t c;
	get(i, TASK_VM_INFO_COUNT, &kr, &c);
	printf("%-14s kr=%d count=%u virt=%llu res=%llu res_peak=%llu internal=%llu external=%llu reusable=%llu "
	       "compressed=%llu device=%llu purgeable_res=%llu footprint=%llu fp_peak=%lld page=%d\n",
	       tag, kr, c, i->virtual_size / MIB, i->resident_size / MIB, i->resident_size_peak / MIB,
	       i->internal / MIB, i->external / MIB, i->reusable / MIB, i->compressed / MIB, i->device / MIB,
	       i->purgeable_volatile_resident / MIB, i->phys_footprint / MIB,
	       (long long) i->ledger_phys_footprint_peak / (long long) MIB, i->page_size);
}

int main(void)
{
	task_vm_info_data_t a, b, c, d, e;
	kern_return_t kr; mach_msg_type_number_t cnt;

	/* the count: all revisions are accepted, REV0 does not have phys_footprint */
	static const struct { const char *n; mach_msg_type_number_t want; } revs[] = {
		{ "REV0", TASK_VM_INFO_REV0_COUNT }, { "REV1", TASK_VM_INFO_REV1_COUNT },
		{ "REV2", TASK_VM_INFO_REV2_COUNT }, { "REV3", TASK_VM_INFO_REV3_COUNT },
		{ "REV4", TASK_VM_INFO_REV4_COUNT }, { "REV5", TASK_VM_INFO_REV5_COUNT },
	};
	for (int i = 0; i < 6; i++) {
		task_vm_info_data_t t;
		get(&t, revs[i].want, &kr, &cnt);
		printf("%s: kr=%d count_in=%u count_out=%u\n", revs[i].n, kr, revs[i].want, cnt);
		CHECK(kr == KERN_SUCCESS);
		CHECK(cnt == revs[i].want);
	}
	get(&a, TASK_VM_INFO_REV0_COUNT - 1, &kr, &cnt);
	printf("count too small: kr=%d\n", kr);
	CHECK(kr == KERN_INVALID_ARGUMENT);
	/* a larger buffer is clipped to the newest revision */
	{
		mach_msg_type_number_t c2 = TASK_VM_INFO_COUNT + 8;
		task_vm_info_data_t big[2];
		kr = task_info(mach_task_self(), TASK_VM_INFO, (task_info_t) big, &c2);
		printf("oversized count: kr=%d count_out=%u\n", kr, c2);
		CHECK(kr == KERN_SUCCESS);
		CHECK(c2 == TASK_VM_INFO_COUNT);
	}

	snap("baseline", &a);
	CHECK(a.page_size >= 4096);
	CHECK(a.resident_size > 0 && a.virtual_size >= a.resident_size);
	CHECK(a.phys_footprint > 0);
	CHECK(a.internal > 0);

	/* 64 MiB of anonymous memory, touched */
	size_t len = 64 * MIB;
	char *p = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	CHECK(p != MAP_FAILED);
	snap("mmap-untouched", &b);
	for (size_t o = 0; o < len; o += 4096) p[o] = 1;
	snap("anon-touched", &c);
	printf("delta (touched - baseline): virt=%lld res=%lld internal=%lld external=%lld footprint=%lld compressed=%lld MiB\n",
	       (long long) (c.virtual_size - a.virtual_size) / (long long) MIB,
	       (long long) (c.resident_size - a.resident_size) / (long long) MIB,
	       (long long) (c.internal - a.internal) / (long long) MIB,
	       (long long) (c.external - a.external) / (long long) MIB,
	       (long long) (c.phys_footprint - a.phys_footprint) / (long long) MIB,
	       (long long) (c.compressed - a.compressed) / (long long) MIB);
	CHECK(b.phys_footprint - a.phys_footprint < 8 * MIB);          /* untouched memory costs nothing */
	CHECK(b.virtual_size - a.virtual_size >= 60 * MIB);
	CHECK(c.phys_footprint - a.phys_footprint >= 60 * MIB);
	CHECK(c.internal - a.internal >= 60 * MIB);
	CHECK(c.resident_size - a.resident_size >= 60 * MIB);
	CHECK(c.internal <= c.phys_footprint + 8 * MIB);
	CHECK(c.resident_size_peak >= c.resident_size);
	CHECK(c.ledger_phys_footprint_peak >= (int64_t) c.phys_footprint);
	CHECK(c.compressed < 8 * MIB);

	/* the memory goes away again */
	munmap(p, len);
	snap("unmapped", &d);
	CHECK(c.phys_footprint - d.phys_footprint >= 60 * MIB);
	CHECK(d.resident_size_peak >= c.resident_size);                 /* a peak does not go down */
	CHECK(d.ledger_phys_footprint_peak >= (int64_t) c.phys_footprint - (int64_t) (8 * MIB));

	/* 64 MiB of a file mapped and read: counted as resident, not as anonymous memory */
	char path[] = "/tmp/v387-XXXXXX";
	int fd = mkstemp(path);
	unlink(path);
	CHECK(fd >= 0 && ftruncate(fd, len) == 0);
	char *f = mmap(NULL, len, PROT_READ, MAP_SHARED, fd, 0);
	CHECK(f != MAP_FAILED);
	volatile char sink = 0;
	for (size_t o = 0; o < len; o += 4096) sink += f[o];
	(void) sink;
	snap("file-read", &e);
	printf("delta (file-read - unmapped): res=%lld internal=%lld external=%lld footprint=%lld MiB\n",
	       (long long) (e.resident_size - d.resident_size) / (long long) MIB,
	       (long long) (e.internal - d.internal) / (long long) MIB,
	       (long long) (e.external - d.external) / (long long) MIB,
	       (long long) (e.phys_footprint - d.phys_footprint) / (long long) MIB);
	munmap(f, len);
	close(fd);

	puts(failed ? "FAILED" : "PASSED");
	return failed;
}
