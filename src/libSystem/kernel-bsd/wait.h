#ifndef BSD_WAIT_H
#define BSD_WAIT_H
#include <sys/wait.h>

#ifdef __cplusplus
extern "C"
{
#endif

int __darwin_kill(pid_t pid, int sig);
pid_t __darwin_wait(int *stat_loc);
pid_t __darwin_wait3(int *stat_loc, int options, struct rusage *rusage);
pid_t __darwin_wait4(pid_t pid, int *stat_loc, int options, struct rusage *rusage);
pid_t __darwin_waitpid(pid_t pid, int *stat_loc, int options);

#ifdef __cplusplus
}
#endif

#endif
