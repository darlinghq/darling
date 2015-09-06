#include "mach_includes.h"
#include <linux/init.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include "linuxmach.h"
#include "ipc_space.h"
#include "ipc_port.h"
#include "api.h"
#include "debug.h"
#include "ipc_server.h"

MODULE_LICENSE("GPL");

typedef long (*trap_handler)(mach_task_t*, ...);

static struct file_operations mach_chardev_ops = {
	.open           = mach_dev_open,
	.release        = mach_dev_release,
	.unlocked_ioctl = mach_dev_ioctl,
	.compat_ioctl   = mach_dev_ioctl,
	.owner          = THIS_MODULE
};

#define sc(n) n-DARLING_MACH_API_BASE

static const trap_handler mach_traps[20] = {
	[sc(NR_get_api_version)] = (trap_handler) mach_get_api_version,
	[sc(NR_mach_reply_port)] = (trap_handler) mach_reply_port_trap,
	[sc(NR__kernelrpc_mach_port_mod_refs)] = (trap_handler) _kernelrpc_mach_port_mod_refs_trap,
	[sc(NR_task_self_trap)] = (trap_handler) mach_task_self_trap,
};
#undef sc

static struct miscdevice mach_dev = {
	MISC_DYNAMIC_MINOR,
	"mach",
	&mach_chardev_ops,
};

static int mach_init(void)
{
	int err = misc_register(&mach_dev);
	if (err < 0)
		goto fail;

	printk(KERN_INFO "Darling Mach kernel emulation loaded\n");
	return 0;

fail:
	printk(KERN_WARNING "Error loading Darling Mach: %d\n", err);
	return err;
}
static void mach_exit(void)
{
	misc_deregister(&mach_dev);
	printk(KERN_INFO "Darling Mach kernel emulation unloaded\n");
}

int mach_dev_open(struct inode* ino, struct file* file)
{
	darling_mach_port_t* port;
	
	if (ipc_port_new(&port) != KERN_SUCCESS)
		return -ENOMEM;
	
	ipc_port_make_task(port, current->pid);
	file->private_data = port;

	return 0;
}

int mach_dev_release(struct inode* ino, struct file* file)
{
	darling_mach_port_t* task_port;
	mach_task_t* task;
	
	task_port = (darling_mach_port_t*) file->private_data;
	ipc_port_put(task_port);
	
	return 0;
}

long mach_dev_ioctl(struct file* file, unsigned int ioctl_num, unsigned long ioctl_paramv)
{
	const unsigned int num_traps = sizeof(mach_traps) / sizeof(mach_traps[0]);
	
	darling_mach_port_t* task_port = (darling_mach_port_t*) file->private_data;
	mach_task_t* task;
	
	debug_msg("function 0x%x called...\n", ioctl_num);
	
	ioctl_num -= DARLING_MACH_API_BASE;
	
	if (ioctl_num >= num_traps)
		return -ENOSYS;

	if (!mach_traps[ioctl_num])
		return -ENOSYS;
	
	task = ipc_port_get_task(task_port);
	
	return mach_traps[ioctl_num](task, ioctl_paramv);
}

int mach_get_api_version(mach_task_t* task)
{
	return DARLING_MACH_API_VERSION;
}

mach_port_name_t mach_reply_port_trap(mach_task_t* task)
{
	mach_msg_return_t ret;
	mach_port_name_t name;
	darling_mach_port_t* port;
	
	ret = ipc_port_new(&port);
	if (ret != KERN_SUCCESS)
		return 0;
	
	ret = ipc_space_make_receive(&task->namespace, port, &name);
	if (ret != KERN_SUCCESS)
	{
		ipc_port_put(port);
		return 0;
	}
	
	return name;
}

static mach_task_t* port_name_to_task(mach_task_t* task_self, mach_port_name_t name)
{
	struct mach_port_right* right;
	mach_task_t* task;
	
	right = ipc_space_lookup(&task_self->namespace, name);
	if (right == NULL)
		return NULL;
	
	// NOTE: If XNU were to support accessing other tasks,
	// we would need to safely add a reference to the task at this point.
	task = ipc_port_get_task(right->port);
	
	ipc_port_unlock(right->port);
	
	return task;
}

kern_return_t _kernelrpc_mach_port_mod_refs_trap(mach_task_t* task_self,
		struct mach_port_mod_refs_args* in_args)
{
	struct mach_port_mod_refs_args args;
	struct mach_port_right* right = NULL;
	kern_return_t ret;
	
	if (copy_from_user(&args, in_args, sizeof(args)))
		return KERN_INVALID_ADDRESS;
	
	// We behave like XNU here
	if (port_name_to_task(task_self, args.task_right_name) != task_self)
	{
		debug_msg("_kernelrpc_mach_port_mod_refs_trap() -> MACH_SEND_INVALID_DEST\n");
		ret = MACH_SEND_INVALID_DEST;
		goto err;
	}
	
	ipc_space_lock(&task_self->namespace);
	
	right = ipc_space_lookup(&task_self->namespace, args.port_right_name);
	if (right == NULL)
	{
		debug_msg("_kernelrpc_mach_port_mod_refs_trap() -> KERN_INVALID_NAME\n");
		ret = KERN_INVALID_NAME;
		goto err;
	}
	
	ret = ipc_right_mod_refs(right, args.right_type, args.delta);
	
	if (right->num_refs == 0)
		ipc_right_put(right);
	
err:
	if (right != NULL)
		ipc_port_unlock(right->port);
	
	ipc_space_unlock(&task_self->namespace);
	
	return ret;
}

mach_port_name_t mach_task_self_trap(mach_task_t* task)
{
	mach_port_name_t name;
	kern_return_t ret;
	
	ipc_space_lock(&task->namespace);
	
	ret = ipc_space_make_send(&task->namespace, task->task_self, false, &name);
	
	ipc_space_unlock(&task->namespace);
	
	if (ret == KERN_SUCCESS)
		return name;
	else
		return 0;
}

module_init(mach_init);
module_exit(mach_exit);

