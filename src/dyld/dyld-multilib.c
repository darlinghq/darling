/*
This file is part of Darling.

Copyright (C) 2015 Lubos Dolezel

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
#include <sys/wait.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

struct fat_arch
{
	uint32_t cputype;
	uint32_t cpusubtype;
	uint32_t offset;
	uint32_t size;
	uint32_t align;
};

void junction(const char* target, int argc, char** argv);
const char* decideFat(int fd, bool swapEndian);
int registerDeregister(const char* argv0, bool reg);
int registerDeregisterRun(const char* argv0, const char* bits, bool reg);

int main(int argc, char** argv)
{
	uint32_t signature;
	int fd = -1;
	const char* target = "64";
	bool reg;
	
	if (argc == 2 && (reg = !strcmp(argv[1], "--register") || !strcmp(argv[1], "--deregister")))
		return registerDeregister(argv[0], reg);

	if (argc > 1)
		fd = open(argv[1], O_RDONLY | O_CLOEXEC);

	// We let real dyld output all serious error messages
	// not to duplicate the functionality.
	if (fd < 0)
		junction(target, argc, argv);

	if (read(fd, &signature, 4) != 4)
		junction(target, argc, argv);

	if (signature == 0xfeedface || signature == 0xcefaedfe)
		target = "32";
	else if (signature == 0xcafebabe || signature == 0xbebafeca)
		target = decideFat(fd, signature == 0xbebafeca);

	junction(target, argc, argv);
	return 1;
}

void junction(const char* target, int argc, char** argv)
{
	char* targetPath = malloc(strlen(target) + 3);
	
	strcpy(targetPath, argv[0]);
	strcat(targetPath, target);

	argv[0] = targetPath;

	execvp(targetPath, argv);

	fprintf(stderr, "Darling dyld multilib junction failed to execute %s!\n", targetPath);
	free(targetPath);

	exit(errno);
}

const char* decideFat(int fd, bool swapEndian)
{
	uint32_t nArchs;
	struct fat_arch* archs;
	size_t bytes;
	const char* rv = "32";
	
	if (read(fd, &nArchs, 4) != 4)
		return "64";
	
	if (swapEndian)
		nArchs = __builtin_bswap32(nArchs);
	
	bytes = nArchs*sizeof(struct fat_arch);
	archs = (struct fat_arch*) malloc(bytes);
	
	if (read(fd, archs, bytes) != bytes)
		return "64";
	
	for (uint32_t i = 0; i < nArchs; i++)
	{
		uint32_t cputype = archs[i].cputype;
		
		if (swapEndian)
			cputype = __builtin_bswap32(cputype);
		
		if (cputype & 0x01000000) // 64-bit
			rv = "64";
	}
	
	free(archs);
	
	return rv;
}

int registerDeregister(const char* argv0, bool reg)
{
	int ec, rv = 0;
	
	if ((ec = registerDeregisterRun(argv0, "64", reg)))
		rv = ec;
	if ((ec = registerDeregisterRun(argv0, "32", reg)))
		rv = ec;
	
	return rv;
}

int registerDeregisterRun(const char* argv0, const char* bits, bool reg)
{
	char path[PATH_MAX];
	const char* argument = (reg) ? "--register" : "--deregister";
	
	strncpy(path, argv0, PATH_MAX-1);
	path[PATH_MAX-1] = 0;
	
	strcat(path, bits);
	
	if (!fork())
	{
		execl(path, path, argument, NULL);
		exit(1);
	}
	else
	{
		int status;
		
		wait(&status);
		return WEXITSTATUS(status);
	}
}


