#ifndef MACH_STUB_H
#define MACH_STUB_H
#include <cstdio>

#define MACH_STUB() { fprintf(stderr, "MACH_STUB(): %s\n", __PRETTY_FUNCTION__); return KERN_FAILURE; }
#define CHECK_TASK_SELF(task) { if (task->pid != getpid()) MACH_STUB(); }

#endif
