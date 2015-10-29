/*
 * Darling Mach Linux Kernel Module
 * Copyright (C) 2015 Lubos Dolezel
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "mach_includes.h"
#include <linux/init.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/file.h>
#include <linux/dcache.h>
#include "darling_task.h"
#include "traps.h"
#include "ipc_space.h"
#include "ipc_port.h"
#include "ipc_msg.h"
#include "api.h"
#include "debug.h"
#include "ipc_server.h"
#include "proc_entry.h"
#include "bsd_ioctl.h"
#include "servers/mach_host.h"
#include "servers/thread_act.h"
#include "primitives/semaphore.h"

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
	[sc(NR__kernelrpc_mach_port_allocate)] = (trap_handler) _kernelrpc_mach_port_allocate_trap,
	[sc(NR_mach_msg_overwrite_trap)] = (trap_handler) mach_msg_overwrite_trap,
	[sc(NR_host_self_trap)] = (trap_handler) mach_host_self_trap,
	[sc(NR__kernelrpc_mach_port_deallocate)] = (trap_handler) _kernelrpc_mach_port_deallocate_trap,
	[sc(NR__kernelrpc_mach_port_destroy)] = (trap_handler) _kernelrpc_mach_port_destroy_trap,
	[sc(NR_semaphore_signal_trap)] = (trap_handler) semaphore_signal_trap,
	[sc(NR_semaphore_signal_all_trap)] = (trap_handler) semaphore_signal_all_trap,
	[sc(NR_semaphore_wait_trap)] = (trap_handler) semaphore_wait_trap,
	[sc(NR_semaphore_wait_signal_trap)] = (trap_handler) semaphore_wait_signal_trap,
	[sc(NR_semaphore_timedwait_signal_trap)] = (trap_handler) semaphore_timedwait_trap,
	[sc(NR_semaphore_timedwait_trap)] = (trap_handler) semaphore_timedwait_signal_trap,
	[sc(NR_bsd_ioctl_trap)] = (trap_handler) bsd_ioctl_trap,
	[sc(NR_thread_self_trap)] = (trap_handler) mach_thread_self_trap,
};
#undef sc

static struct miscdevice mach_dev = {
	MISC_DYNAMIC_MINOR,
	"mach",
	&mach_chardev_ops,
};

darling_mach_port_t* host_port;
ipc_namespace_t kernel_namespace;

static int mach_init(void)
{
	// mach_port_name_t name;
	int err;
	
	err = misc_register(&mach_dev);
	if (err < 0)
		goto fail;
	
	darling_task_init();
	setup_proc_entry();
	ipc_space_init(&kernel_namespace);

	if (ipc_port_new(&host_port) != KERN_SUCCESS)
	{
		err = -ENOMEM;
		goto fail;
	}
	ipc_port_make_host(host_port);
	// ipc_space_make_receive(&kernel_namespace, host_port, &name);

	printk(KERN_INFO "Darling Mach kernel emulation loaded\n");
	return 0;

fail:
	printk(KERN_WARNING "Error loading Darling Mach: %d\n", err);
	return err;
}
static void mach_exit(void)
{
	ipc_port_put(host_port);
	ipc_space_put(&kernel_namespace);
	cleanup_proc_entry();
	misc_deregister(&mach_dev);
	printk(KERN_INFO "Darling Mach kernel emulation unloaded\n");
}

int mach_dev_open(struct inode* ino, struct file* file)
{
	darling_mach_port_t* task_port;
	darling_mach_port_t* thread_port;
	mach_task_t* task;
	
	if (ipc_port_new(&task_port) != KERN_SUCCESS)
		return -ENOMEM;
	
	if (ipc_port_new(&thread_port) != KERN_SUCCESS)
	{
		ipc_port_put(task_port);
		return -ENOMEM;
	}
	
	task = ipc_port_make_task(task_port, current->pid);
	file->private_data = task_port;
	
	ipc_port_make_thread(thread_port);
	
	darling_task_set_current(ipc_port_get_task(task_port));
	darling_task_register_thread(task, thread_port);

	return 0;
}

int mach_dev_release(struct inode* ino, struct file* file)
{
	darling_mach_port_t* task_port;
	
	task_port = (darling_mach_port_t*) file->private_data;
	ipc_port_put(task_port);
	
	darling_task_set_current(NULL);
	
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
	
	ipc_space_lock(&task_self->namespace);
	
	// We behave like XNU here
	if (port_name_to_task(task_self, args.task_right_name) != task_self)
	{
		debug_msg("_kernelrpc_mach_port_mod_refs_trap() -> MACH_SEND_INVALID_DEST\n");
		ret = MACH_SEND_INVALID_DEST;
		goto err;
	}
	
	right = ipc_space_lookup(&task_self->namespace, args.port_right_name);
	if (right == NULL)
	{
		debug_msg("_kernelrpc_mach_port_mod_refs_trap(%d) -> KERN_INVALID_NAME\n", args.port_right_name);
		ret = KERN_INVALID_NAME;
		goto err;
	}
	
	ret = ipc_right_mod_refs(right, args.right_type, args.delta);
	
	if (ipc_right_put_if_noref(right, &task_self->namespace, args.port_right_name))
		right = NULL;
	
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
	
	ipc_port_lock(task->task_self);
	
	ret = ipc_space_make_send(&task->namespace, task->task_self, false, &name);
	
	ipc_port_unlock(task->task_self);
	
	if (ret == KERN_SUCCESS)
		return name;
	else
		return 0;
}

mach_port_name_t mach_thread_self_trap(mach_task_t* task)
{
	mach_port_name_t name;
	kern_return_t ret;
	darling_mach_port_t* thread_port;
	
	ipc_port_lock(task->task_self);
	
	thread_port = darling_task_lookup_thread(task, current->pid);
	
	if (thread_port != NULL)
		ret = ipc_space_make_send(&task->namespace, thread_port, false, &name);
	else
		ret = KERN_FAILURE;
	
	ipc_port_unlock(task->task_self);
	
	if (ret == KERN_SUCCESS)
		return name;
	else
		return 0;
}

mach_port_name_t mach_host_self_trap(mach_task_t* task)
{
	mach_port_name_t name;
	kern_return_t ret;
	
	ipc_port_lock(host_port);
	
	ret = ipc_space_make_send(&task->namespace, host_port, false, &name);
	
	ipc_port_unlock(host_port);
	
	if (ret == KERN_SUCCESS)
		return name;
	else
		return 0;
}

kern_return_t _kernelrpc_mach_port_allocate_trap(mach_task_t* task,
		struct mach_port_allocate_args* in_out_args)
{
	struct mach_port_allocate_args args;
	darling_mach_port_t* port;
	kern_return_t ret = KERN_SUCCESS;
	
	if (copy_from_user(&args, in_out_args, sizeof(args)))
		return KERN_INVALID_ADDRESS;
	
	ipc_space_lock(&task->namespace);
	
	if (port_name_to_task(task, args.task_right_name) != task)
	{
		debug_msg("_kernelrpc_mach_port_allocate_trap() -> MACH_SEND_INVALID_DEST\n");
		
		ipc_space_unlock(&task->namespace);
		ret = MACH_SEND_INVALID_DEST;
		goto err;
	}
	
	ipc_space_unlock(&task->namespace);
	
	switch (args.right_type)
	{
		case MACH_PORT_RIGHT_RECEIVE:
		{
			ret = ipc_port_new(&port);
			if (ret != KERN_SUCCESS)
				return ret;
			
			break;
		}
		case MACH_PORT_RIGHT_DEAD_NAME:
		{
			port = PORT_DEAD;
			break;
		}
		// TODO: missing MACH_PORT_RIGHT_PORT_SET
		default:
		{
			return KERN_INVALID_VALUE;
		}
	}
	
	ret = ipc_space_make_receive(&task->namespace, port,
								 &args.out_right_name);
	if (ret != KERN_SUCCESS)
	{
		ipc_port_put(port);
		goto err;
	}
	
	if (copy_to_user(in_out_args, &args, sizeof(args)))
	{
		ipc_port_put(port);
		ret = KERN_PROTECTION_FAILURE;
		goto err;
	}
	
err:
	return ret;
}

kern_return_t _kernelrpc_mach_port_deallocate_trap(mach_task_t* task,
		struct mach_port_deallocate_args* in_args)
{
	struct mach_port_deallocate_args args;
	struct mach_port_right* right = NULL;
	kern_return_t ret = KERN_SUCCESS;
	
	if (copy_from_user(&args, in_args, sizeof(args)))
		return KERN_INVALID_ADDRESS;
	
	ipc_space_lock(&task->namespace);
	
	if (port_name_to_task(task, args.task_right_name) != task)
	{
		debug_msg("_kernelrpc_mach_port_deallocate_trap() -> MACH_SEND_INVALID_DEST\n");
		
		ret = MACH_SEND_INVALID_DEST;
		goto err;
	}
	
	right = ipc_space_lookup(&task->namespace, args.port_right_name);
	if (right == NULL || right->type == MACH_PORT_RIGHT_RECEIVE)
	{
		debug_msg("_kernelrpc_mach_port_deallocate_trap() -> KERN_INVALID_RIGHT\n");
		
		ret = KERN_INVALID_RIGHT;
		goto err;
	}
	
	if (right->port != NULL)
	{
		ipc_right_mod_refs(right, right->type, -1);
		
		if (ipc_right_put_if_noref(right, &task->namespace, args.port_right_name))
			right = NULL;
	}
	
err:
	if (right != NULL)
		ipc_port_unlock(right->port);

	ipc_space_unlock(&task->namespace);
	return ret;
}

kern_return_t _kernelrpc_mach_port_destroy_trap(mach_task_t* task,
		struct mach_port_destroy_args* in_args)
{
	struct mach_port_destroy_args args;
	
	if (copy_from_user(&args, in_args, sizeof(args)))
		return KERN_INVALID_ADDRESS;
	
	return _kernelrpc_mach_port_destroy(task, args.task_right_name,
			args.port_right_name);
}

kern_return_t _kernelrpc_mach_port_destroy(mach_task_t* task,
		mach_port_name_t task_name, mach_port_name_t right_name)
{
	kern_return_t ret = KERN_SUCCESS;
	
	ipc_space_lock(&task->namespace);
	
	if (port_name_to_task(task, task_name) != task)
	{
		debug_msg("_kernelrpc_mach_port_destroy_trap() -> MACH_SEND_INVALID_DEST\n");
		
		ret = MACH_SEND_INVALID_DEST;
		goto err;
	}
	
	ret = ipc_space_right_put(&task->namespace, right_name);
	
err:

	ipc_space_unlock(&task->namespace);
	return ret;
}

kern_return_t mach_msg_overwrite_trap(mach_task_t* task,
		struct mach_msg_overwrite_args* in_args)
{
	struct mach_msg_overwrite_args args;
	kern_return_t ret = MACH_MSG_SUCCESS;
	
	debug_msg("mach_msg_overwrite_trap()");
	
	if (copy_from_user(&args, in_args, sizeof(args)))
		return KERN_INVALID_ADDRESS;
	
	if (args.option & MACH_SEND_MSG)
	{
		mach_msg_header_t* msg;
		
		if (args.send_size > 10*4096)
		{
			debug_msg("\t-> MACH_SEND_NO_BUFFER\n");
			return MACH_SEND_NO_BUFFER;
		}
		if (args.send_size < sizeof(mach_msg_header_t))
		{
			debug_msg("\t-> MACH_SEND_MSG_TOO_SMALL\n");
			return MACH_SEND_MSG_TOO_SMALL;
		}
		
		msg = (mach_msg_header_t*) kmalloc(args.send_size, GFP_KERNEL);
		if (msg == NULL)
			return MACH_SEND_NO_BUFFER;
		
		if (copy_from_user(msg, args.msg, args.send_size))
		{
			kfree(msg);
			return KERN_INVALID_ADDRESS;
		}
		
		msg->msgh_size = args.send_size;
		msg->msgh_bits &= MACH_MSGH_BITS_USER;
		
		ret = ipc_msg_send(&task->namespace, msg,
				(args.option & MACH_SEND_TIMEOUT) ? args.timeout : MACH_MSG_TIMEOUT_NONE,
				args.option);
		
		if (ret != MACH_MSG_SUCCESS)
			return ret;
	}
	
	if (args.option & MACH_RCV_MSG)
	{
		ret = ipc_msg_recv(task, args.rcv_name, args.rcv_msg, args.recv_size,
				(args.option & MACH_RCV_TIMEOUT) ? args.timeout : MACH_MSG_TIMEOUT_NONE,
				args.option);
	}
	return ret;
}

kern_return_t semaphore_signal_trap(mach_task_t* task,
		struct semaphore_signal_args* in_args)
{
	struct semaphore_signal_args args;
	darling_mach_port_right_t* right;
	
	if (copy_from_user(&args, in_args, sizeof(args)))
		return KERN_INVALID_ADDRESS;
	
	ipc_space_lock(&task->namespace);
	
	right = ipc_space_lookup(&task->namespace, args.signal);
	
	ipc_space_unlock(&task->namespace);
	
	if (right == NULL || !PORT_IS_VALID(right->port))
	{
		if (right != NULL)
			ipc_port_unlock(right->port);
		return KERN_INVALID_ARGUMENT;
	}
	
	return mach_semaphore_signal(right->port);
}

kern_return_t semaphore_signal_all_trap(mach_task_t* task,
		struct semaphore_signal_all_args* in_args)
{
	struct semaphore_signal_all_args args;
	darling_mach_port_right_t* right;
	
	if (copy_from_user(&args, in_args, sizeof(args)))
		return KERN_INVALID_ADDRESS;
	
	ipc_space_lock(&task->namespace);
	
	right = ipc_space_lookup(&task->namespace, args.signal);
	
	ipc_space_unlock(&task->namespace);
	
	if (right == NULL || !PORT_IS_VALID(right->port))
	{
		if (right != NULL)
			ipc_port_unlock(right->port);
		return KERN_INVALID_ARGUMENT;
	}
	
	return mach_semaphore_signal_all(right->port);
}

kern_return_t semaphore_wait_trap(mach_task_t* task,
		struct semaphore_wait_args* in_args)
{
	struct semaphore_wait_args args;
	darling_mach_port_right_t* right;
	
	if (copy_from_user(&args, in_args, sizeof(args)))
		return KERN_INVALID_ADDRESS;
	
	ipc_space_lock(&task->namespace);
	
	right = ipc_space_lookup(&task->namespace, args.signal);
	
	ipc_space_unlock(&task->namespace);
	
	if (right == NULL || !PORT_IS_VALID(right->port))
	{
		if (right != NULL)
			ipc_port_unlock(right->port);
		return KERN_INVALID_ARGUMENT;
	}
	
	return mach_semaphore_wait(right->port);
}

kern_return_t semaphore_wait_signal_trap(mach_task_t* task,
		struct semaphore_wait_signal_args* in_args)
{
	struct semaphore_wait_signal_args args;
	darling_mach_port_right_t *right_wait, *right_signal;
	kern_return_t ret = KERN_SUCCESS;
	
	if (copy_from_user(&args, in_args, sizeof(args)))
		return KERN_INVALID_ADDRESS;
	
	ipc_space_lock(&task->namespace);
	
	right_signal = ipc_space_lookup(&task->namespace, args.signal);
	if (right_signal == NULL)
	{
		ipc_space_unlock(&task->namespace);
		return KERN_INVALID_ARGUMENT;
	}
	
	if (args.wait != 0)
		right_wait = ipc_space_lookup(&task->namespace, args.wait);
	else
		right_wait = NULL;
	
	ipc_space_unlock(&task->namespace);
	
	if (right_wait != NULL)
		ret = mach_semaphore_wait(right_wait->port);
	
	mach_semaphore_signal(right_signal->port);
	
	return ret;
}

kern_return_t semaphore_timedwait_trap(mach_task_t* task,
		struct semaphore_timedwait_args* in_args)
{
	struct semaphore_timedwait_args args;
	darling_mach_port_right_t* right;
	
	if (copy_from_user(&args, in_args, sizeof(args)))
		return KERN_INVALID_ADDRESS;
	
	ipc_space_lock(&task->namespace);
	
	right = ipc_space_lookup(&task->namespace, args.wait);
	
	ipc_space_unlock(&task->namespace);
	
	if (right == NULL || !PORT_IS_VALID(right->port))
	{
		if (right != NULL)
			ipc_port_unlock(right->port);
		return KERN_INVALID_ARGUMENT;
	}
	
	return mach_semaphore_timedwait(right->port, args.sec, args.nsec);
}

kern_return_t semaphore_timedwait_signal_trap(mach_task_t* task,
		struct semaphore_timedwait_signal_args* in_args)
{
	struct semaphore_timedwait_signal_args args;
	darling_mach_port_right_t *right_wait, *right_signal;
	kern_return_t ret = KERN_SUCCESS;
	
	if (copy_from_user(&args, in_args, sizeof(args)))
		return KERN_INVALID_ADDRESS;
	
	ipc_space_lock(&task->namespace);
	
	right_signal = ipc_space_lookup(&task->namespace, args.signal);
	if (right_signal == NULL)
	{
		ipc_space_unlock(&task->namespace);
		return KERN_INVALID_ARGUMENT;
	}
	
	if (args.wait != 0)
		right_wait = ipc_space_lookup(&task->namespace, args.wait);
	else
		right_wait = NULL;
	
	ipc_space_unlock(&task->namespace);
	
	if (right_wait != NULL)
		ret = mach_semaphore_timedwait(right_wait->port, args.sec, args.nsec);
	
	mach_semaphore_signal(right_signal->port);
	
	return ret;
}

long bsd_ioctl_trap(mach_task_t* task, struct bsd_ioctl_args* in_args)
{
	struct bsd_ioctl_args args;
	struct file* f;
	char name[60];
	long retval = 0;
	int handled;
	
	if (copy_from_user(&args, in_args, sizeof(args)))
		return -EFAULT;
	
	f = fget(args.fd);
	if (f == NULL)
		return -EBADF;
	
	if (f->f_op->unlocked_ioctl == NULL)
	{
		fput(f);
		return -ENOTTY;
	}
	
	if (d_path(&f->f_path, name, sizeof(name)) == NULL)
	{
		fput(f);
		return -EBADF;
	}
	
	// Perform ioctl translation based on name
	if (strncmp(name, "socket:", 7) == 0)
		handled = bsd_ioctl_xlate_socket(f, &args, &retval);
	else if (strncmp(name, "/dev/tty", 8) == 0)
		handled = bsd_ioctl_xlate_tty(f, &args, &retval);
	else if (strncmp(name, "/dev/pts", 8) == 0)
		handled = bsd_ioctl_xlate_pts(f, &args, &retval);
	else
		handled = bsd_ioctl_xlate_generic(f, &args, &retval);
	
	if (!handled)
		retval = f->f_op->unlocked_ioctl(f, args.request, args.arg);
	
	fput(f);
	
	return retval;
}

module_init(mach_init);
module_exit(mach_exit);

