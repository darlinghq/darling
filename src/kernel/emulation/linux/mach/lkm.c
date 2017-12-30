#include "lkm.h"
#include "../../lkm/api.h"
#include "../signal/sigexc.h"
#include "../base.h"
#include "../linux-syscalls/linux.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/resource.h>
#include "../../libsyscall/wrappers/_libkernel_init.h"

extern int __real_ioctl(int fd, int cmd, void* arg);
extern int sys_open(const char*, int, int);
extern int sys_close(int);
extern int sys_write(int, const void*, int);
extern int sys_kill(int, int);
extern int sys_getrlimit(int, struct rlimit*);
extern int sys_dup2(int, int);
extern int sys_fcntl(int, int, int);
extern _libkernel_functions_t _libkernel_functions;

extern void sigexc_setup1(void);
extern void sigexc_setup2(void);

void mach_driver_init(void)
{
}


