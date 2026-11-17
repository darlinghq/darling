/*
This file is part of Darling.

Copyright (C) 2026 Lubos Dolezel

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

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "muvm-trampoline.h"

// The launcher target is built without the mach-o include directory mldr uses,
// so only the few header fields the decision needs are declared here.
#ifndef MH_MAGIC_64
#define MH_MAGIC_64 0xfeedfacfU
#define MH_CIGAM_64 0xcffaedfeU
#define MH_MAGIC 0xfeedfaceU
#define MH_CIGAM 0xcefaedfeU
#define FAT_MAGIC 0xcafebabeU
#define FAT_CIGAM 0xbebafecaU

#define CPU_TYPE_ARM 12U
#define CPU_TYPE_X86 7U
#define CPU_ARCH_ABI64 0x01000000U
#define CPU_TYPE_ARM64 (CPU_TYPE_ARM | CPU_ARCH_ABI64)
#define CPU_TYPE_X86_64 (CPU_TYPE_X86 | CPU_ARCH_ABI64)
#endif

// A fat header is 8 bytes; each fat_arch is 20. A table larger than this is
// not something a linker produced.
#define FAT_MAX_ARCHS 64U
#define FAT_ARCH_SIZE 20U
#define HEADER_SIZE (8U + FAT_MAX_ARCHS * FAT_ARCH_SIZE)

static uint32_t swap32(uint32_t value)
{
	return (value >> 24) | ((value >> 8) & 0xff00U) | ((value << 8) & 0xff0000U) | (value << 24);
}

static enum darling_target_slice sliceFromCpuType(uint32_t cputype)
{
	switch (cputype)
	{
		case CPU_TYPE_ARM64:
			return DARLING_TARGET_SLICE_ARM64;
		case CPU_TYPE_X86_64:
			return DARLING_TARGET_SLICE_X86_64;
		case CPU_TYPE_ARM:
			return DARLING_TARGET_SLICE_ARM32;
		default:
			return DARLING_TARGET_SLICE_NONE;
	}
}

enum darling_target_slice darling_target_slice_arch(const char* path, const uint32_t bprefs[4])
{
	uint8_t header[HEADER_SIZE];
	ssize_t got;
	uint32_t magic;
	int fd;

	if (!path || !path[0])
		return DARLING_TARGET_SLICE_NONE;

	// The launcher is root at this point and the path comes from the user, so a
	// redirect to a FIFO must not block it and anything but a regular file is
	// left for mldr to reject.
	fd = open(path, O_RDONLY | O_CLOEXEC | O_NONBLOCK);
	if (fd < 0)
		return DARLING_TARGET_SLICE_NONE;

	struct stat st;

	if (fstat(fd, &st) != 0 || !S_ISREG(st.st_mode))
	{
		close(fd);
		return DARLING_TARGET_SLICE_NONE;
	}

	got = read(fd, header, sizeof(header));
	close(fd);

	if (got < 8)
		return DARLING_TARGET_SLICE_NONE;

	memcpy(&magic, header, sizeof(magic));

	if (magic == MH_CIGAM_64 || magic == MH_MAGIC_64)
	{
		uint32_t cputype;

		memcpy(&cputype, header + 4, sizeof(cputype));
		if (magic == MH_CIGAM_64)
			cputype = swap32(cputype);
		return sliceFromCpuType(cputype);
	}

	// A 32-bit thin Mach-O, or something that is not Mach-O at all; mldr reaches
	// the same conclusion on its own.
	if (magic != FAT_CIGAM && magic != FAT_MAGIC)
		return DARLING_TARGET_SLICE_NONE;

	bool swap = magic == FAT_CIGAM;
	uint32_t nfat_arch;

	memcpy(&nfat_arch, header + 4, sizeof(nfat_arch));
	if (swap)
		nfat_arch = swap32(nfat_arch);

	if (nfat_arch == 0 || nfat_arch > FAT_MAX_ARCHS
		|| got < (ssize_t) (8 + nfat_arch * FAT_ARCH_SIZE))
		return DARLING_TARGET_SLICE_NONE;

	uint32_t archs[FAT_MAX_ARCHS];

	for (uint32_t i = 0; i < nfat_arch; i++)
	{
		uint32_t cputype;

		memcpy(&cputype, header + 8 + i * FAT_ARCH_SIZE, sizeof(cputype));
		archs[i] = swap ? swap32(cputype) : cputype;
	}

	// mldr lets the first matching __mldr_bprefs entry choose, which is the
	// lowest bprefs index that names an architecture present in the file.
	for (int j = 0; j < 4; j++)
	{
		if (bprefs && bprefs[j])
		{
			for (uint32_t i = 0; i < nfat_arch; i++)
			{
				if (archs[i] == bprefs[j])
					return sliceFromCpuType(archs[i]);
			}
		}
	}

	// No preference: mldr takes arm64, then x86_64 over a 32-bit arm slice.
	const enum darling_target_slice order[] =
	{
		DARLING_TARGET_SLICE_ARM64,
		DARLING_TARGET_SLICE_X86_64,
		DARLING_TARGET_SLICE_ARM32,
	};
	const uint32_t cputypes[] = { CPU_TYPE_ARM64, CPU_TYPE_X86_64, CPU_TYPE_ARM };

	for (size_t k = 0; k < sizeof(order) / sizeof(order[0]); k++)
	{
		for (uint32_t i = 0; i < nfat_arch; i++)
		{
			if (archs[i] == cputypes[k])
				return order[k];
		}
	}

	return DARLING_TARGET_SLICE_NONE;
}

void darling_read_mldr_bprefs(uint32_t bprefs[4])
{
	const char* value = getenv("__mldr_bprefs");
	unsigned int entries[4] = { 0, 0, 0, 0 };
	int parsed;

	bprefs[0] = bprefs[1] = bprefs[2] = bprefs[3] = 0;

	if (value == NULL)
		return;

	parsed = sscanf(value, "%x,%x,%x,%x", &entries[0], &entries[1], &entries[2], &entries[3]);
	if (parsed < 1)
		return;

	// A preference shorter than four entries leaves the rest at zero, as mldr
	// reads it.
	for (int i = 0; i < parsed; i++)
		bprefs[i] = entries[i];
}

bool darling_muvm_needed(long page_size, enum darling_target_slice slice)
{
	return page_size > 4096 && slice == DARLING_TARGET_SLICE_X86_64;
}

bool darling_muvm_stdio_ok(int stdin_fd, int stdout_fd, int stderr_fd)
{
	return isatty(stdin_fd) && isatty(stdout_fd) && isatty(stderr_fd);
}
