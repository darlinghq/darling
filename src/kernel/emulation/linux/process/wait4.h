#ifndef LINUX_WAIT4_H
#define LINUX_WAIT4_H

long sys_wait4(int pid, int* status, int options, void* rusage);

int waitopts_bsd_to_linux(int options);

#define LINUX_WNOHANG 1
#define LINUX_WUNTRACED 2
#define LINUX_WSTOPPED 2
#define LINUX_WEXITED 4
#define LINUX_WCONTINUED 8
#define LINUX_WNOWAIT 0x1000000

#define BSD_WNOHANG 1
#define BSD_WUNTRACED 2
#define BSD_WEXITED 4
#define BSD_WSTOPPED 8
#define BSD_WCONTINUED 0x10
#define BSD_WNOWAIT 0x20

#endif

