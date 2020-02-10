#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <lkm/api.h>

extern int lkm_call(int nr, ...);
extern int __darling_vchroot(int dfd);

int main(int argc, const char** argv)
{
    if (argc < 3)
	{
		fprintf(stderr, "vchroot <dir> <binary> [args...]\n");
		return 1;
	}

	char buf[4096];
	sprintf(buf, "%s%s", argv[1], argv[2]);

	if (access(buf, F_OK) != 0)
	{
		fprintf(stderr, "Target executable not found: %s\n", buf);
		return 5;
	}

	int dfd = open(argv[1], O_RDONLY | O_DIRECTORY);
	if (dfd == -1)
	{
		perror("open");
		return 1;
	}

	if (fchdir(dfd) == -1)
	{
		perror("fchdir");
		return 2;
	}

	if (__darling_vchroot(dfd) < 0)
	{
		perror("vchroot");
		return 3;
	}

	close(dfd);

	// This is only needed for this binary and shouldn't be passed down
	unsetenv("DYLD_ROOT_PATH");

	// printf("Will execv %s\n", argv[2]);
	execv(argv[2], (char * const *) argv+2);
	perror("execv");

	return 4;
}

