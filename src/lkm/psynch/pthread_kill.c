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

#include "../mach_includes.h"
#include "pthread_kill.h"
#include <asm/siginfo.h>
#include <linux/uaccess.h>
#include <asm/siginfo.h>
#include <linux/rcupdate.h>
#include <linux/sched.h>
#include "../servers/thread_act.h"
#include "../ipc_types.h"
#include "../ipc_space.h"
#include "../ipc_right.h"
#include "../darling_task.h"

int pthread_kill_trap(mach_task_t* task,
		struct pthread_kill_args* in_args)
{
	struct pthread_kill_args args;
	darling_mach_port_right_t* right = NULL;
	int ret;
	pid_t tid;
	struct siginfo info;
	struct task_struct *t;
	
	if (copy_from_user(&args, in_args, sizeof(args)))
		return -EFAULT;
	
	ipc_space_lock(&task->namespace);
	right = ipc_space_lookup(&task->namespace, args.thread_port);
	
	ipc_space_unlock(&task->namespace);
	if (right == NULL)
	{
		ret = -EINVAL;
		goto err;
	}
	
	tid = get_thread_pid(right->port);
	ipc_port_unlock(right->port);
	
	if (tid == 0)
	{
		// Not a thread port
		ret = -EINVAL;
		goto err;
	}
	
	rcu_read_lock();
	t = pid_task(find_pid_ns(tid, &init_pid_ns), PIDTYPE_PID);
	
	if (t == NULL)
	{
		rcu_read_unlock();
		ret = -ESRCH;
		goto err;
	}
	
	info.si_signo = args.sig;
	info.si_code = SI_TKILL;
	info.si_pid = t->tgid;
	info.si_uid = from_kuid_munged(current_user_ns(), current_uid());
	info.si_errno = 0;
	
	ret = send_sig_info(args.sig, &info, t);
	rcu_read_unlock();
	
err:
	return ret;
}
