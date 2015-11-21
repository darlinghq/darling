#include "workq_kernreturn.h"
#include "bsdthread_register.h"
#include "bsdthread_create.h"
#include "../base.h"
#include "../errno.h"
#include "../../../../../platform-include/sys/errno.h"
#include "../../../../libdyld/threads.h"
#include <asm/unistd.h>
#include <stddef.h>

#define WQOPS_QUEUE_ADD 1
#define WQOPS_QUEUE_REMOVE 2
#define WQOPS_THREAD_RETURN 4
#define WQOPS_THREAD_SETCONC  8
#define WQOPS_QUEUE_NEWSPISUPP  0x10
#define WQOPS_QUEUE_REQTHREADS  0x20

// Flags for the newly spawned thread:
// WQ_FLAG_THREAD_NEWSPI
// WQ_FLAG_THREAD_REUSE (0 if thread is newly spawned)
// item is unused in "new SPI"

#define WQ_FLAG_THREAD_PRIOMASK         0x0000ffff
#define WQ_FLAG_THREAD_OVERCOMMIT       0x00010000
#define WQ_FLAG_THREAD_REUSE            0x00020000
#define WQ_FLAG_THREAD_NEWSPI           0x00040000

extern int thread_self_trap(void);

long sys_workq_kernreturn(int options, void* item, int affinity, int prio)
{
	// item is only used with WQOPS_QUEUE_ADD
	switch (options)
	{
		case WQOPS_THREAD_RETURN:
			// By returning, we're letting the thread terminate.
			//
			// We can also wait until there are new threads requested
			// and recycle the thread by jumping to wqueue_entry_point
			// again with WQ_FLAG_THREAD_REUSE (TODO).
			return 0;
		case WQOPS_QUEUE_NEWSPISUPP:
			return 0;
		case WQOPS_QUEUE_REQTHREADS:
		{
			// affinity contains thread count

			int i, flags, ret;

			flags = WQ_FLAG_THREAD_NEWSPI;
			flags |= prio;

			for (i = 0; i < affinity; i++)
			{
				//void* stack;

				//stack = thread_stack_allocate(512*1024);
				//ret = darling_thread_create(stack, wqueue_entry_point, stack, NULL, flags, 0);
				ret = __darling_thread_create(512*1024, pthread_obj_size, wqueue_entry_point, 0, NULL, flags, 0,
						thread_self_trap);

				if (ret < 0)
					return ret;
			}
			return 0;
		}
		default:
			return -ENOTSUP;
	}
}

