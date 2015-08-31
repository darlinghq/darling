#include "mach_includes.h"
#include <linux/init.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include "linuxmach.h"
#include "ipc_space.h"
#include "ipc_port.h"
#include "api.h"

MODULE_LICENSE("GPL");

typedef long (*trap_handler)(struct mach_task_t*, ...);

static struct file_operations mach_chardev_ops = {
	.open           = mach_dev_open,
	.release        = mach_dev_release,
	.unlocked_ioctl = mach_dev_ioctl,
	.compat_ioctl   = mach_dev_ioctl,
	.owner          = THIS_MODULE
};
static const trap_handler mach_traps[20] = {
	[NR_mach_reply_port] = (trap_handler) mach_reply_port_trap
};

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
	int err = 0;
	struct mach_task_t* task;
	
	task = (struct mach_task_t*) kmalloc(sizeof(struct mach_task_t), GFP_KERNEL);
	ipc_space_init(&task->namespace);
	task->pid = current->pid;
	
	file->private_data = task;

	return 0;
fail:

	return err;
}

int mach_dev_release(struct inode* ino, struct file* file)
{
	struct mach_task_t* task;
	
	task = (struct mach_task_t*) file->private_data;
	ipc_space_put(&task->namespace);
	
	return 0;
}

long mach_dev_ioctl(struct file* file, unsigned int ioctl_num, unsigned long ioctl_paramv)
{
	struct mach_task_t* mpdata = (struct mach_task_t*) file->private_data;
	const unsigned int num_traps = sizeof(mach_traps) / sizeof(mach_traps[0]);
	
	if (ioctl_num >= num_traps)
		return -ENOSYS;

	if (!mach_traps[ioctl_num])
		return -ENOSYS;
	
	return mach_traps[ioctl_num](mpdata, ioctl_paramv);
}

mach_port_name_t mach_reply_port_trap(struct mach_task_t* task)
{
	mach_msg_return_t ret;
	mach_port_name_t name;
	struct mach_port_t* port;
	
	ret = mach_port_new(&port);
	if (ret != KERN_SUCCESS)
		return 0;
	
	ret = ipc_space_make_receive(&task->namespace, port, &name);
	if (ret != KERN_SUCCESS)
	{
		mach_port_put(port);
		return 0;
	}
	
	return name;
}

module_init(mach_init);
module_exit(mach_exit);

