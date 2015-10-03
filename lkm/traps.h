#ifndef LINUXMACH_H
#define LINUXMACH_H
#include <linux/fs.h>
#include "ipc_space.h"
#include "api.h"

int mach_dev_open(struct inode* ino, struct file* file);
int mach_dev_release(struct inode* ino, struct file* file);
long mach_dev_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_paramv);

int mach_get_api_version(mach_task_t* task);
mach_port_name_t mach_reply_port_trap(mach_task_t* task);
kern_return_t _kernelrpc_mach_port_mod_refs_trap(mach_task_t* task,
		struct mach_port_mod_refs_args* args);
kern_return_t _kernelrpc_mach_port_deallocate_trap(mach_task_t* task,
		struct mach_port_deallocate_args* args);
kern_return_t _kernelrpc_mach_port_destroy_trap(mach_task_t* task,
		struct mach_port_destroy_args* args);
mach_port_name_t mach_task_self_trap(mach_task_t* task);
mach_port_name_t mach_host_self_trap(mach_task_t* task);
kern_return_t _kernelrpc_mach_port_allocate_trap(mach_task_t* task,
												 struct mach_port_allocate_args* args);
kern_return_t mach_msg_overwrite_trap(mach_task_t* task,
		struct mach_msg_overwrite_args* args);

#endif
