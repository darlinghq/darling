// Host helper for diskarbitration.sh: an empty, read-only FUSE file system whose source (fsname) is a file, like a
// disk image's, but which is not darling-dmg (it names no volume kind). DiskArbitration must not take it for a disk.
// Build: cc -o fuse-file-source-host fuse-file-source-host.c $(pkg-config --cflags --libs fuse3)
// Usage: fuse-file-source-host <source file> <mount point>   (runs in the foreground until it is unmounted)
#define FUSE_USE_VERSION 31
#include <errno.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

static int getattr(const char* path, struct stat* st, struct fuse_file_info* fi)
{
	(void) fi;
	if (strcmp(path, "/") != 0)
		return -ENOENT;
	memset(st, 0, sizeof(*st));
	st->st_mode = S_IFDIR | 0555;
	st->st_nlink = 2;
	return 0;
}

static int readdir(const char* path, void* buf, fuse_fill_dir_t fill, off_t offset, struct fuse_file_info* fi,
	enum fuse_readdir_flags flags)
{
	(void) offset; (void) fi; (void) flags;
	if (strcmp(path, "/") != 0)
		return -ENOENT;
	fill(buf, ".", NULL, 0, 0);
	fill(buf, "..", NULL, 0, 0);
	return 0;
}

static const struct fuse_operations operations = { .getattr = getattr, .readdir = readdir };

int main(int argc, char** argv)
{
	if (argc != 3)
	{
		fprintf(stderr, "usage: %s <source file> <mount point>\n", argv[0]);
		return 2;
	}
	char options[4096];
	snprintf(options, sizeof(options), "ro,fsname=%s,subtype=filesource", argv[1]);
	char* args[] = { argv[0], "-f", "-o", options, argv[2], NULL };
	return fuse_main(5, args, &operations, NULL);
}
