#ifndef LINUXMACH_H
#define LINUXMACH_H
#include <linux/fs.h>
#include "ipc_space.h"

struct mach_task_t
{
	pid_t pid;
	struct ipc_space_t namespace;
};

int mach_dev_open(struct inode* ino, struct file* file);
int mach_dev_release(struct inode* ino, struct file* file);
long mach_dev_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_paramv);

mach_port_name_t mach_reply_port_trap(struct mach_task_t* task);

#endif
