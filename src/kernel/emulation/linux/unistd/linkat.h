#ifndef LINUX_LINKAT_H
#define LINUX_LINKAT_H

long sys_linkat(int fd, const char* path, int fdlink, const char* link, int flag);

#endif

