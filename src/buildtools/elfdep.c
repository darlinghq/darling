/*
This file is part of Darling.

Copyright (C) 2018-2020 Lubos Dolezel

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

bool printElfdepAny(const void* mem);
bool printElfdepMH(const struct mach_header* mhdr);
bool printElfdepMH64(const struct mach_header_64* mhdr);
bool printFat(const struct fat_header* fhdr);
bool printTaf(const struct fat_header* fhdr);

int main(int argc, const char** argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "elfdep: Prints the ELF dependency (SONAME) of a Mach-O file, if any\n");
		fprintf(stderr, "Usage: elfdep <macho-file>\n");
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

	printElfdepAny(mem);

	munmap(mem, length);

	return EXIT_SUCCESS;
}

bool printElfdepAny(const void* mem)
{
	const struct mach_header* mhdr = (struct mach_header*) mem;

	if (mhdr->magic == MH_MAGIC)
		return printElfdepMH(mhdr);
	else if (mhdr->magic == MH_MAGIC_64)
		return printElfdepMH64((struct mach_header_64*) mhdr);
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

bool printElfdep(const struct load_command* lc, const void* base)
{
	if (lc->cmd == LC_SEGMENT)
	{
		const struct segment_command* sc = (const struct segment_command*) lc;

		if (strcmp(sc->segname, "__TEXT") == 0)
		{
			const struct section* sect = (const struct section*)(sc+1);
			for (int i = 0; i < sc->nsects; i++)
			{
				if (strcmp(sect->sectname, "__elfname") == 0)
				{
					printf("%s\n", ((const char*)base) + sect->offset);
					return true;
				}
				sect++;
			}
		}
	}
	else if (lc->cmd == LC_SEGMENT_64)
	{
		const struct segment_command_64* sc = (const struct segment_command_64*) lc;

		if (strcmp(sc->segname, "__TEXT") == 0)
		{
			const struct section_64* sect = (const struct section_64*)(sc+1);
			for (int i = 0; i < sc->nsects; i++)
			{
				if (strcmp(sect->sectname, "__elfname") == 0)
				{
					printf("%s\n", ((const char*)base) + sect->offset);
					return true;
				}
				sect++;
			}
		}
	}
	return false;
}

bool printElfdepMH(const struct mach_header* mhdr)
{
	const uint8_t* command = (const uint8_t*)(mhdr + 1);
	for (uint32_t i = 0; i < mhdr->ncmds; i++)
	{
		if (printElfdep((const struct load_command*) command, mhdr))
			return true;
		else
			command += ((const struct load_command*)command)->cmdsize;
	}
	return false;
}

bool printElfdepMH64(const struct mach_header_64* mhdr)
{
	const uint8_t* command = (const uint8_t*)(mhdr + 1);
	for (uint32_t i = 0; i < mhdr->ncmds; i++)
	{
		if (printElfdep((const struct load_command*) command, mhdr))
			return true;
		else
			command += ((const struct load_command*)command)->cmdsize;
	}
	return false;
}

bool printFat(const struct fat_header* fhdr)
{
	const struct fat_arch* fa = ((const struct fat_arch*) (fhdr+1));

	for (uint32_t i = 0; i < fhdr->nfat_arch; i++)
	{
		if (printElfdepAny(((char*) fhdr) + fa[i].offset))
			return true;
	}

	return false;
}

bool printTaf(const struct fat_header* fhdr)
{
	const struct fat_arch* fa = ((const struct fat_arch*) (fhdr+1));

	for (uint32_t i = 0; i < __builtin_bswap32(fhdr->nfat_arch); i++)
	{
		if (printElfdepAny(((char*) fhdr) + __builtin_bswap32(fa[i].offset)))
			return true;
	}

	return false;
}

