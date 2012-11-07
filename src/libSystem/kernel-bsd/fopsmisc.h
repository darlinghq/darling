#ifndef FOPSMISC_H
#define FOPSMISC_H
#include <sys/stat.h>

#ifdef __cplusplus
extern "C"
{
#endif

int __darwin_access(const char *pathname, int mode);
int __darwin_chown(const char *path, uid_t owner, gid_t group);
// int fchown(int fd, uid_t owner, gid_t group);
int __darwin_lchown(const char *path, uid_t owner, gid_t group);
int __darwin_chmod(const char *path, mode_t mode);
// int __darwin_fchmod(int fd, mode_t mode);

ssize_t __darwin_readlink(const char *path, char *buf, size_t bufsiz);
int __darwin_symlink(const char *oldpath, const char *newpath);
int __darwin_link(const char *oldpath, const char *newpath);

int __darwin_unlink(const char *pathname);
int __darwin_rmdir(const char *pathname);
int __darwin_mknod(const char *pathname, mode_t mode, dev_t dev);
int __darwin_mkdir(const char *pathname, mode_t mode);


int __darwin_chdir(const char *path);
int __darwin_acct(const char *filename);

#ifdef __cplusplus
}
#endif

#endif

