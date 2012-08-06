#ifndef MACH_TASK_H
#define MACH_TASK_H
#include <sys/types.h>

struct darwin_task_t
{
  pid_t pid;
};

extern "C"
{
  extern darwin_task_t* mach_task_self_;
  inline darwin_task_t* mach_task_self() { return mach_task_self_; }
}

#endif
