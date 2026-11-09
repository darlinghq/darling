// Copyright (C) 2026 Lubos Dolezel
//
// Host-side test of the guard below the thread stacks mldr allocates: it is one
// whole host page, it is inaccessible, and it ends exactly where the stack
// begins, so no part of the stack is inaccessible. (With a guard smaller than a
// page, mprotect rounds the length up and protects the start of the stack; that
// only shows on hosts with pages larger than 4 KiB.)
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include "thread_stack.h"

static int failures;

static void check(int ok, const char* what)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		failures++;
}

// Finds the mapping containing `addr` in /proc/self/maps and returns its
// bounds and whether it is readable. Returns 0 if there is none.
static int mapping_of(uintptr_t addr, uintptr_t* start, uintptr_t* end, int* readable)
{
	FILE* maps = fopen("/proc/self/maps", "r");
	char line[512];
	int found = 0;

	if (!maps)
		return 0;
	while (fgets(line, sizeof(line), maps))
	{
		unsigned long s, e;
		char perms[5];
		if (sscanf(line, "%lx-%lx %4s", &s, &e, perms) != 3)
			continue;
		if (addr >= s && addr < e)
		{
			*start = s;
			*end = e;
			*readable = perms[0] == 'r';
			found = 1;
			break;
		}
	}
	fclose(maps);
	return found;
}

int main(void)
{
	size_t page = (size_t) sysconf(_SC_PAGESIZE);
	size_t guard = dthread_guard_size();
	size_t stack_size = 8 * page;
	void* stack_top = NULL;

	check(guard >= page && guard % page == 0, "the guard is a whole number of pages");

	dthread_t dthread = dthread_structure_allocate(stack_size, guard, &stack_top);
	check(dthread != NULL, "allocation succeeds");
	if (!dthread)
		return 1;

	uintptr_t base = (uintptr_t) dthread->freeaddr;
	uintptr_t stack_bottom = (uintptr_t) stack_top - stack_size;
	uintptr_t start, end;
	int readable;

	check(dthread->guardsize == guard, "the dthread records the guard size");
	check(stack_bottom == base + guard, "the stack starts right above the guard");

	check(mapping_of(base, &start, &end, &readable) && !readable
		&& start == base && end == base + guard,
		"exactly the guard is inaccessible");

	check(mapping_of(stack_bottom, &start, &end, &readable) && readable
		&& start == stack_bottom,
		"the lowest stack page is accessible");

	// Touch both ends of the stack
	*(volatile char*) stack_bottom = 1;
	*(volatile char*) ((char*) stack_top - 1) = 1;

	munmap(dthread->freeaddr, dthread->freesize);

	if (failures)
	{
		printf("FAILED %d\n", failures);
		return 1;
	}
	printf("ALL PASSED\n");
	return 0;
}
