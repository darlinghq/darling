#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

void junction(const char* target, int argc, char** argv);

int main(int argc, char** argv)
{
	uint32_t signature;
	int fd;
	const char* target = "64";

	fd = open(argv[1], O_RDONLY);

	// We let real dyld output all serious error messages
	// not to duplicate the functionality.
	if (fd < 0)
		junction(target, argc, argv);

	if (read(fd, &signature, 4) != 4)
		junction(target, argc, argv);

	if (signature == 0xfeedface)
		target = "32";

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

