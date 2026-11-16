// The private libdarling-dmg API that hdiutil and DiskImageMounter attach disk images with.
// Usage: dmg-api <unmount-test.dmg> <a file that is not a disk image> <empty directory to mount in>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>

#include <darling-dmg/darling_dmg.h>

static int failures;

static void check(int ok, const char* what)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		failures++;
}

int main(int argc, char** argv)
{
	if (argc != 4) {
		fprintf(stderr, "usage: %s <image> <not an image> <mount root>\n", argv[0]);
		return 2;
	}

	darling_dmg_image* image = NULL;
	check(darling_dmg_open("/nonexistent-dmg-api.dmg", &image) == ENOENT && image == NULL, "open of a missing file: ENOENT");
	check(darling_dmg_open(argv[2], &image) == EFTYPE && image == NULL, "open of a file that is not an image: EFTYPE");

	int err = darling_dmg_open(argv[1], &image);
	check(err == 0 && image != NULL, "open of the fixture");
	if (err != 0)
		return 1;

	const char* name = darling_dmg_volume_name(image);
	check(name != NULL && name[0] != 0, "volume name is not empty");

	struct darling_dmg_attach_options options = { .mount_root = argv[3], .mount_point = NULL };
	char mp[MAXPATHLEN];
	err = darling_dmg_attach(image, &options, mp, sizeof(mp));
	check(err == 0, "attach under the given mount root");
	check(strncmp(mp, argv[3], strlen(argv[3])) == 0, "mount point is under the mount root");

	char file[MAXPATHLEN];
	snprintf(file, sizeof(file), "%s/hello.txt", mp);
	char buf[16] = { 0 };
	int fd = open(file, O_RDONLY);
	check(fd >= 0 && read(fd, buf, sizeof(buf)) == 6 && strcmp(buf, "hello\n") == 0, "file on the volume is readable");
	if (fd >= 0)
		close(fd);

	struct statfs sfs;
	check(statfs(mp, &sfs) == 0 && strcmp(sfs.f_mntonname, mp) == 0, "mount point is a mount point");

	check(darling_dmg_detach(mp, 0) == 0, "detach");
	struct stat st;
	check(stat(mp, &st) != 0 && errno == ENOENT, "mount point directory removed");
	err = darling_dmg_detach(mp, 0);
	check(err == ENOENT || err == EINVAL, "detach again fails (no such mount)");

	darling_dmg_close(image);
	return failures ? 1 : 0;
}
