#ifndef LIBC_DIR_H
#define LIBC_DIR_H
#include "common/path.h"
#include <sys/types.h>
#include <dirent.h>
#include <stdint.h>

struct darwin_dirent64
{
	uint64_t d_ino;
	uint64_t d_seekoff;
	uint16_t d_reclen;
	uint16_t d_namlen;
	uint8_t d_type;
	char d_name[DARWIN_MAXPATHLEN];
};

#pragma pack(4)
struct darwin_dirent {
	uint32_t d_ino;
	uint16_t d_reclen;
	uint8_t d_type;
	uint8_t d_namlen;
	char d_name[DARWIN_MAXNAMLEN + 1];
};
#pragma pack()

#ifdef __cplusplus
extern "C"
{
#endif

darwin_dirent64* __darwin_readdir64(DIR* dirp) asm("__darwin_readdir$INODE64");
darwin_dirent* __darwin_readdir(DIR* dirp);
int __darwin_readdir_r(DIR *dirp, struct darwin_dirent *entry, struct darwin_dirent **result);
int __darwin_readdir_r64(DIR *dirp, struct darwin_dirent64 *entry, struct darwin_dirent64 **result) asm("__darwin_readdir_r$INODE64");

DIR* __darwin_opendir(const char *name); // opendir$INODE64 in aliases
// DIR* __darwin_fdopendir(int fd); // not present on Darwin
int __darwin_closedir(DIR *dirp);

int __darwin_scandir(const char *dirp, struct darwin_dirent ***namelist,
			int (*filter)(const struct darwin_dirent *),
			int (*compar)(const struct darwin_dirent **, const struct darwin_dirent **));
int __darwin_scandir64(const char *dirp, struct darwin_dirent64 ***namelist,
			int (*filter)(const struct darwin_dirent64 *),
			int (*compar)(const struct darwin_dirent64 **, const struct darwin_dirent64 **)) asm("__darwin_scandir$INODE64");

// dirfs is compatible (no errors)
// rewinddir is compatible (no errors)
// seekdir is compatible
// getdirentries macro-mapped

// alphasort
// versionsort

int __darwin_alphasort(const void *a, const void *b);
int __darwin_alphasort64(const void *a, const void *b) asm("__darwin_alphasort$INODE64");

int __darwin_versionsort(const void *a, const void *b);
int __darwin_versionsort64(const void *a, const void *b) asm("__darwin_versionsort$INODE64");

#ifdef __cplusplus
}
#endif

#endif
