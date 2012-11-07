#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <string.h>
#include <assert.h>

void test_getpath();

int main()
{
	test_getpath();
	return 0;
}

void test_getpath()
{
	char path[255] = "/tmp/fcntl.XXXXXX";
	char fpath[MAXPATHLEN];
	mkstemp(path);

	int fd = open(path, O_WRONLY|O_CREAT|O_TRUNC, 0666);
	fcntl(fd, F_GETPATH, fpath);

	close(fd);

	assert(strcmp(realpath(path, 0), realpath(fpath, 0)) == 0);
	remove(path);
}

