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
