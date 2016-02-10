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
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/kernel.h>
#include <linux/seq_file.h>
#include "ipc_msg.h"
#include "ipc_port.h"
#include "proc_entry.h"
#include "darling_task.h"
#include "api.h"
#include "primitives/semaphore.h"

static struct proc_dir_entry *proc_mach;

static int mach_proc_open(struct inode *inode, struct file *file);

static const struct file_operations proc_file_fops = {
	.owner = THIS_MODULE,
	.open  = mach_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int mach_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "Darling Mach\n");
	seq_printf(m, "API level:\t" DARLING_MACH_API_VERSION_STR "\n");
	
	seq_printf(m, "Task count:\t%u\n", darling_get_task_count());
	seq_printf(m, "Port count:\t%d\n", ipc_port_count());
	seq_printf(m, "Sem count:\t%d\n", mach_semaphore_count());
	seq_printf(m, "Msgs sent:\t%d\n", ipc_msg_count());
	
	return 0;
}

static int mach_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, mach_proc_show, NULL);
}

void setup_proc_entry(void)
{
	proc_mach = proc_create("driver/darling_mach", 0400, NULL,
			&proc_file_fops);
}

void cleanup_proc_entry(void)
{
	if (proc_mach != NULL)
		remove_proc_entry("driver/darling_mach", NULL);
}
