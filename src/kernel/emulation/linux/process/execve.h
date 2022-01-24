#ifndef LINUX_EXECVE_H
#define LINUX_EXECVE_H

long sys_execve(const char* fname, const char** argvp, const char** envp);

#endif

