#ifndef BSD_WAIT_H
#define BSD_WAIT_H
#include <sys/wait.h>

extern "C"
{

pid_t __darwin_wait(int *stat_loc);
pid_t __darwin_wait3(int *stat_loc, int options, struct rusage *rusage);
pid_t __darwin_wait4(pid_t pid, int *stat_loc, int options, struct rusage *rusage);
pid_t __darwin_waitpid(pid_t pid, int *stat_loc, int options);

}

#endif
