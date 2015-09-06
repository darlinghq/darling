#ifndef LINUXMACH_H
#define LINUXMACH_H
#include <linux/fs.h>
#include "ipc_space.h"
#include "api.h"

struct mach_task
{
	pid_t pid;
	struct ipc_space_t namespace;
	
	// TODO: add host and bootstrap ports
	darling_mach_port_t* task_self;
};

typedef struct mach_task mach_task_t;

int mach_dev_open(struct inode* ino, struct file* file);
int mach_dev_release(struct inode* ino, struct file* file);
long mach_dev_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_paramv);

int mach_get_api_version(mach_task_t* task);
mach_port_name_t mach_reply_port_trap(mach_task_t* task);
kern_return_t _kernelrpc_mach_port_mod_refs_trap(mach_task_t* task,
		struct mach_port_mod_refs_args* args);
mach_port_name_t mach_task_self_trap(mach_task_t* task);

#endif
