#include "task.h"
#include "debug.h"
#include <linux/spinlock.h>
#include <linux/rbtree.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/printk.h>

static spinlock_t my_lock;
struct rb_root all_tasks = RB_ROOT;

struct task_entry
{
	struct rb_node node;
	int tgid;
	mach_task_t* task;
};

void
darling_task_init(void)
{
	spin_lock_init(&my_lock);
}

mach_task_t*
darling_task_get_current(void)
{
	mach_task_t* ret = NULL;
	struct rb_node* node;
	
	spin_lock(&my_lock);
	
	node = all_tasks.rb_node;
	
	while (node)
	{
		struct task_entry* entry = container_of(node, struct task_entry, node);
		
		if (current->tgid < entry->tgid)
			node = node->rb_left;
		else if (current->tgid > entry->tgid)
			node = node->rb_right;
		else
		{
			ret = entry->task;
			break;
		}
	}
	
	spin_unlock(&my_lock);
	return ret;
}

void
darling_task_set_current(mach_task_t* task)
{
	struct task_entry* entry;
	struct rb_node **new, *parent = NULL;
	
	entry = (struct task_entry*) kmalloc(sizeof(mach_task_t), GFP_KERNEL);
	entry->task = task;
	entry->tgid = current->tgid;
	
	spin_lock(&my_lock);
	new = &all_tasks.rb_node;
	
	while (*new)
	{
		struct task_entry* this = container_of(*new, struct task_entry, node);
		parent = *new;
		
		if (entry->tgid < this->tgid)
			new = &(*new)->rb_left;
		else if (entry->tgid > this->tgid)
			new = &(*new)->rb_right;
		else // Tree already contains this tgid
		{
			kfree(entry);
			
			if (task != NULL)
			{
				// Overwrite existing task entry
				debug_msg("darling_task_set_current() called twice with "
						"non-null task?!\n");
				
				this->task = task;
			}
			else
			{
				// Remove task from tree
				rb_erase(*new, &all_tasks);
				kfree(this);
			}
			
			spin_unlock(&my_lock);
			return;
		}
	}
	
	rb_link_node(&entry->node, parent, new);
	rb_insert_color(&entry->node, &all_tasks);
	
	spin_unlock(&my_lock);
}
