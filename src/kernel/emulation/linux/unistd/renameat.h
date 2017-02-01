#ifndef LINUX_RENAMEAT_H
#define LINUX_RENAMEAT_H

long sys_renameat(int oldfd, const char* oldpath, int newfd, const char* newpath);

#endif

