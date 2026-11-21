// The private libdarling-dmg API that hdiutil and DiskImageMounter attach disk images with.
// Usage: dmg-api <image> <its volume name> <a file that is not a disk image> <empty directory to mount in>
// The image must contain hello.txt with "hello\n" (fixtures/unmount-test.dmg under any file name).
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
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
	if (argc != 5) {
		fprintf(stderr, "usage: %s <image> <volume name> <not an image> <mount root>\n", argv[0]);
		return 2;
	}

	darling_dmg_image* image = NULL;
	check(darling_dmg_open("/nonexistent-dmg-api.dmg", &image) == ENOENT && image == NULL, "open of a missing file: ENOENT");
	check(darling_dmg_open(argv[3], &image) == EFTYPE && image == NULL, "open of a file that is not an image: EFTYPE");

	int err = darling_dmg_open(argv[1], &image);
	check(err == 0 && image != NULL, "open of the fixture");
	if (err != 0)
		return 1;

	const char* name = darling_dmg_volume_name(image);
	check(name != NULL && strcmp(name, argv[2]) == 0, "volume name is the name stored in the image");
	if (name != NULL && strcmp(name, argv[2]) != 0)
		printf("  volume name: '%s'\n", name);

	char attached[MAXPATHLEN];
	check(darling_dmg_attached_mount_point(image, attached, sizeof(attached)) == ENOENT,
		"an image that is not attached has no mount point");

	struct darling_dmg_attach_options options = { .mount_root = argv[4], .mount_point = NULL };
	char mp[MAXPATHLEN];
	err = darling_dmg_attach(image, &options, mp, sizeof(mp));
	check(err == 0, "attach under the given mount root");
	check(strncmp(mp, argv[4], strlen(argv[4])) == 0, "mount point is under the mount root");

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

	// Attached again, also through a symbolic link to it, the image is not mounted a second time: its mount point
	// comes back (as hdiutil attach does on macOS 26.7.3, see dmg-attach.md). Under /Volumes: getfsstat does not list
	// volumes mounted on the host side (/Volumes/SystemRoot), like the mount root above.
	err = darling_dmg_attach(image, NULL, mp, sizeof(mp));
	check(err == 0, "attach under /Volumes");
	check(darling_dmg_attached_mount_point(image, attached, sizeof(attached)) == 0 && strcmp(attached, mp) == 0,
		"an attached image has its mount point");
	check(darling_dmg_attached_mount_point(image, attached, 2) == ENAMETOOLONG, "a mount point that does not fit: ENAMETOOLONG");
	char link[MAXPATHLEN];
	snprintf(link, sizeof(link), "%s/../dmg-api-link.dmg", argv[4]);
	unlink(link);
	darling_dmg_image* linked = NULL;
	char again[MAXPATHLEN] = { 0 };
	char target[MAXPATHLEN];
	if (realpath(argv[1], target) != NULL && symlink(target, link) == 0 && darling_dmg_open(link, &linked) == 0)
	{
		check(darling_dmg_attach(linked, NULL, again, sizeof(again)) == 0 && strcmp(again, mp) == 0,
			"attaching an attached image, through a link, gives its mount point");
		darling_dmg_close(linked);
	}
	else
		check(0, "a link to the image can be opened");
	unlink(link);
	int count = getfsstat(NULL, 0, MNT_NOWAIT), mounts = 0;
	struct statfs* all = count > 0 ? calloc(count, sizeof(*all)) : NULL;
	count = all != NULL ? getfsstat(all, count * sizeof(*all), MNT_NOWAIT) : 0;
	char source[MAXPATHLEN];
	if (realpath(argv[1], source) == NULL)
		source[0] = 0;
	for (int i = 0; i < count; i++)
		mounts += strcmp(all[i].f_mntfromname, source) == 0;
	free(all);
	check(mounts == 1, "the image is mounted once");

	check(darling_dmg_detach(mp, 0) == 0, "detach from /Volumes");

	darling_dmg_close(image);
	return failures ? 1 : 0;
}
