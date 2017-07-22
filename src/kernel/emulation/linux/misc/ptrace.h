#ifndef LINUX_PTRACE_H
#define LINUX_PTRACE_H

#define PT_PTRACE_ME 0
#define PT_CONTINUE 7
#define PT_KILL 8
#define PT_ATTACH 10
#define PT_ATTACHEXC 14
#define PT_DETACH 11
#define PT_SIGEXC 12
#define PT_THUPDATE 13

long sys_ptrace(int request, int pid, void* addr, int data);

#endif

