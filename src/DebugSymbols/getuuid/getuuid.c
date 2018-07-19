/*
This file is part of Darling.

Copyright (C) 2018 Lubos Dolezel

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
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <mach-o/loader.h>
#include <mach-o/fat.h>
#include <string.h>
#include <errno.h>

bool printUuidAny(const void* mem);
bool printUuidMH(const struct mach_header* mhdr);
bool printUuidMH64(const struct mach_header_64* mhdr);
bool printFat(const struct fat_header* fhdr);
bool printTaf(const struct fat_header* fhdr);

int main(int argc, const char** argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "getuuid: Prints the UUID(s) of a Mach-O file, separated by semicolons (in case of fat files)\n");
		fprintf(stderr, "Usage: getuuid <macho-file>\n");
		return EXIT_FAILURE;
	}

	int fd = open(argv[1], O_RDONLY);
	if (fd == -1)
	{
		fprintf(stderr, "Cannot open %s: %s", argv[1], strerror(errno));
		return EXIT_FAILURE;
	}

	size_t length = lseek(fd, 0, SEEK_END);

	void* mem = mmap(NULL, length, PROT_READ, MAP_SHARED, fd, 0);
	if (mem == MAP_FAILED)
	{
		perror("mmap");
		return EXIT_FAILURE;
	}

	close(fd);

	int rv = EXIT_FAILURE;

	if (printUuidAny(mem))
		rv = EXIT_SUCCESS;

	munmap(mem, length);

	return rv;
}

bool printUuidAny(const void* mem)
{
	const struct mach_header* mhdr = (struct mach_header*) mem;

	if (mhdr->magic == MH_MAGIC)
		return printUuidMH(mhdr);
	else if (mhdr->magic == MH_MAGIC_64)
		return printUuidMH64((struct mach_header_64*) mhdr);
	else if (mhdr->magic == FAT_MAGIC)
		return printFat((struct fat_header*) mhdr);
	else if (mhdr->magic == FAT_CIGAM)
		return printTaf((struct fat_header*) mhdr);
	else
	{
		fprintf(stderr, "File format not recognized\n");
		exit(1);
	}
}

bool printUuid(const struct load_command* lc)
{
	static bool hadUuid = false;
	if (lc->cmd == LC_UUID)
	{
		const struct uuid_command* uuid = (const struct uuid_command*) lc;

		if (hadUuid)
			putchar(';');

		printf("%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
				uuid->uuid[0],
				uuid->uuid[1],
				uuid->uuid[2],
				uuid->uuid[3],
				uuid->uuid[4],
				uuid->uuid[5],
				uuid->uuid[6],
				uuid->uuid[7],
				uuid->uuid[8],
				uuid->uuid[9],
				uuid->uuid[10],
				uuid->uuid[11],
				uuid->uuid[12],
				uuid->uuid[13],
				uuid->uuid[14],
				uuid->uuid[15]);

		hadUuid = true;
		return true;
	}
	return false;
}

bool printUuidMH(const struct mach_header* mhdr)
{
	const uint8_t* command = (const uint8_t*)(mhdr + 1);
	for (uint32_t i = 0; i < mhdr->ncmds; i++)
	{
		if (printUuid((const struct load_command*) command))
			return true;
		else
			command += ((const struct load_command*)command)->cmdsize;
	}
	return false;
}

bool printUuidMH64(const struct mach_header_64* mhdr)
{
	const uint8_t* command = (const uint8_t*)(mhdr + 1);
	for (uint32_t i = 0; i < mhdr->ncmds; i++)
	{
		if (printUuid((const struct load_command*) command))
			return true;
		else
			command += ((const struct load_command*)command)->cmdsize;
	}
	return false;
}

bool printFat(const struct fat_header* fhdr)
{
	const struct fat_arch* fa = ((const struct fat_arch*) (fhdr+1));
	bool rv = false;

	for (uint32_t i = 0; i < fhdr->nfat_arch; i++)
		rv |= printUuidAny(((char*) fhdr) + fa[i].offset);

	return rv;
}

bool printTaf(const struct fat_header* fhdr)
{
	const struct fat_arch* fa = ((const struct fat_arch*) (fhdr+1));
	bool rv = false;

	for (uint32_t i = 0; i < __builtin_bswap32(fhdr->nfat_arch); i++)
		rv |= printUuidAny(((char*) fhdr) + __builtin_bswap32(fa[i].offset));

	return rv;
}

