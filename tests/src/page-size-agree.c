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

// Every way of asking for the page size gives the same answer, and that
// answer is the granularity mprotect() actually works with: protecting one
// page leaves the next page writable. On a 16 KiB-page host, a BSD side that
// says 4096 while the Mach side says 16384 fails here.

#include <mach/mach.h>
#include <mach/vm_page_size.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/sysctl.h>
#include <unistd.h>

int main(void)
{
	long sc = sysconf(_SC_PAGESIZE);
	int gp = getpagesize();
	int hw = 0;
	size_t len = sizeof(hw);
	int failures = 0;

	if (sysctlbyname("hw.pagesize", &hw, &len, NULL, 0) != 0) {
		perror("sysctlbyname(hw.pagesize)");
		return 1;
	}

	printf("sysconf %ld, getpagesize %d, hw.pagesize %d, vm_page_size %lu\n",
		sc, gp, hw, (unsigned long)vm_page_size);

	if (sc != (long)vm_page_size || gp != (int)vm_page_size || hw != (int)vm_page_size) {
		printf("FAIL: the page sizes disagree\n");
		failures++;
	}

	// Protect the first page of two; the second must stay usable.
	size_t page = (size_t)sc;
	char* p = mmap(NULL, 2 * page, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	if (p == MAP_FAILED) {
		perror("mmap");
		return 1;
	}
	if (mprotect(p, page, PROT_NONE) != 0) {
		perror("FAIL: mprotect of one page");
		failures++;
	}
	if (mprotect(p + page, page, PROT_READ | PROT_WRITE) != 0) {
		perror("FAIL: mprotect at a page offset");
		failures++;
	}
	memset(p + page, 0x5a, page);
	munmap(p, 2 * page);

	if (failures)
		return 1;
	printf("PASS\n");
	return 0;
}
