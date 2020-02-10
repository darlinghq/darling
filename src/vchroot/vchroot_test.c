#include <lkm/api.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>

const char* TESTDIR = "/tmp";
//const char* TESTDIR = "/run/user/1000";

int main(int argc, const char** argv)
{
	int lkm = open("/dev/mach", O_RDWR);
	if (lkm == -1)
	{
		perror("open dev mach");
		return 1;
	}
	int dfd = open(TESTDIR, O_RDONLY | O_DIRECTORY);
	if (dfd == -1)
	{
		perror("open dfd");
		return 1;
	}

	int rv = ioctl(lkm, NR_vchroot, dfd);
	if (rv == -1)
	{
		perror("NR_vchroot");
		return 1;
	}

	struct vchroot_expand_args expand;

	//strcpy(expand.path, "link_to_dir_in_root/file"); // link_to_...
	strcpy(expand.path, "/test2/file");
	// strcpy(expand.path, "/proc/self/mounts");
	expand.flags = 0;
	expand.dfd = -100;

	rv = ioctl(lkm, NR_vchroot_expand, &expand);
	if (rv == -1)
	{
		perror("NR_vchroot_expand");
		return 1;
	}

	printf("Path expanded to %s\n", expand.path);

//	int testfilefd = open(expand.path, O_RDONLY);
//	int testfilefd = open("/tmp/dev/null", O_RDONLY);
	int testfilefd = 0;
	if (testfilefd == -1)
	{
		perror("open test file");
		return 1;
	}

	struct vchroot_fdpath_args fdpath;

	fdpath.fd = testfilefd;
	fdpath.path = malloc(512);
	fdpath.maxlen = 512;

	rv = ioctl(lkm, NR_vchroot_fdpath, &fdpath);
	if (rv == -1)
	{
		perror("NR_vchroot_fdpath");
		return 1;
	}

	printf("Reported fdpath: %s\n", fdpath.path);

	return 0;
	
}

