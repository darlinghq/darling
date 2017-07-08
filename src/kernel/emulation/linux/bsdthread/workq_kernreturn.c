#include "workq_kernreturn.h"
#include "bsdthread_register.h"
#include "bsdthread_create.h"
#include "../base.h"
#include "../errno.h"
#include "../../../../../platform-include/sys/errno.h"
#include "../../../../startup/threads.h"
#include <linux-syscalls/linux.h>
#include <stddef.h>
#include "../ext/futex.h"
#include "../simple.h"

#define WQ_MAX_THREADS	64

#define WQOPS_QUEUE_ADD 1
#define WQOPS_QUEUE_REMOVE 2
#define WQOPS_THREAD_RETURN 4
#define WQOPS_THREAD_SETCONC  8
#define WQOPS_THREAD_KEVENT_RETURN 0x40
#define WQOPS_QUEUE_NEWSPISUPP  0x10
#define WQOPS_QUEUE_REQTHREADS  0x20
#define WQOPS_QUEUE_REQTHREADS2    0x30
#define WQOPS_SET_EVENT_MANAGER_PRIORITY 0x80

// Flags for the newly spawned thread:
// WQ_FLAG_THREAD_NEWSPI
// WQ_FLAG_THREAD_REUSE (0 if thread is newly spawned)
// item is unused in "new SPI"

#define WQ_FLAG_THREAD_PRIOMASK         0x0000ffff
#define WQ_FLAG_THREAD_OVERCOMMIT       0x00010000
#define WQ_FLAG_THREAD_REUSE            0x00020000
#define WQ_FLAG_THREAD_NEWSPI           0x00040000
#define WQ_FLAG_THREAD_KEVENT           0x00080000 /* thread is response to kevent req */

static int workq_sem = WQ_MAX_THREADS; // max 64 threads in use
static int workq_parked_lock = 1;
// static int workq_parked_threads = 0; // num spawned, but unused threads
static int workq_parked[WQ_MAX_THREADS];
static int workq_parked_prio[WQ_MAX_THREADS];

struct parked_thread workq_parked_head = { NULL, NULL };

extern int thread_self_trap(void);

extern void* pthread_get_stackaddr_np(void* pth);
extern void* memmove(void* dest, const void* src, __SIZE_TYPE__ n);

static void list_add(struct parked_thread* head, struct parked_thread* item);
static void list_remove(struct parked_thread* head, struct parked_thread* item);

struct timespec
{
	long tv_sec;
	long tv_nsec;
};

//void* __attribute__((weak)) __attribute__((visibility("default"))) pthread_getspecific(unsigned long key) { return NULL; }
//int __attribute__((weak)) __attribute__((visibility("default"))) pthread_setspecific(unsigned long key, const void* value) { return 1; }

static int priority_to_class(int prio);

// This is horrible, but it may work
static struct wq_kevent_data* wq_event_pending = NULL;

long sys_workq_kernreturn(int options, void* item, int affinity, int prio)
{
#ifndef VARIANT_DYLD
	struct wq_kevent_data* wq_event = NULL;
	
	// __simple_printf("workq_kernreturn: 0x%x, %p, 0x%x, 0x%x\n", options, item, affinity, prio);

	// item is only used with WQOPS_QUEUE_ADD
	switch (options)
	{
		case WQOPS_THREAD_KEVENT_RETURN:
		{
			struct kevent_qos_s* keventlist = item;
			int nkevents = affinity;
			
			// __simple_printf("kevent return, obj=%p, nkevents=%d\n", wq_event, nkevents);

			if (wq_event_pending != NULL)
			{
				int* sem;
				if (nkevents > 0)
					memmove(wq_event_pending->events, keventlist, nkevents * sizeof(struct kevent_qos_s));
				wq_event_pending->nevents = nkevents;

				sem = &wq_event_pending->sem;
				wq_event_pending = NULL;
				sem_up(sem);
			}
			else
				__simple_printf("wq_event is NULL, something is going to get stuck forever\n");
		}
		case WQOPS_THREAD_RETURN:
		{
			// Signalizes that the thread has completed its job
			// and can be either recycled or terminated

			int thread_self;
			struct parked_thread me;
			void* stack;

			sem_down(&workq_parked_lock, -1);

			// Semaphore locked state (wait for wakeup)
			me.sem = 0;

			// Enqueue for future WQOPS_QUEUE_REQTHREADS
			list_add(&workq_parked_head, &me);

			// Decrease the amount of running threads
			sem_up(&workq_sem);

			sem_up(&workq_parked_lock);

			// Wait until someone calls WQOPS_QUEUE_REQTHREADS
			// and wakes us up
			if (!sem_down(&me.sem, 15))
			{
				// Make sure we haven't just been woken up before locking the queue
				// and remove us from the queue if not.
				sem_down(&workq_parked_lock, -1);
	
				if (me.sem > 0)
				{
					sem_up(&workq_parked_lock);
					goto wakeup;
				}

				list_remove(&workq_parked_head, &me);

				sem_up(&workq_parked_lock);

				// Let the thread terminate (libc will call pthread_exit)
				return 0;
			}
wakeup:
			
			// reset stack and call entry point again with WQ_FLAG_THREAD_REUSE
			thread_self = thread_self_trap();
			stack = __darling_thread_get_stack();

			// __simple_printf("Thread %d woken up, prio=%d\n", thread_self, me.flags & WQ_FLAG_THREAD_PRIOMASK);

			if (me.event)
				wq_event_pending = me.event;
#ifdef __x86_64__
			__asm__ __volatile__ (
					// "int3\n"
					"movq %%rbx, %%r8\n" // 5th argument
					"movl %5, %%r9d\n" // 6th argument
					"movq %0, %%rsp\n"
					"movq %%rsp, %%rdx\n" // 3rd argument
					"xorq %%rcx, %%rcx\n" // 4th argument
					"subq $32, %%rsp\n"
					"jmpq *%2\n"
					:: "D" (stack), "S" (thread_self), "a" (wqueue_entry_point),
					"b" (me.flags | WQ_FLAG_THREAD_REUSE), "c" (me.event ? me.event->events : NULL),
					"r" (me.event ? me.event->nevents : 0)
			);
#elif defined(__i386__)
			// Arguments are in eax, ebx, ecx, edx, edi, esi
			__asm__ __volatile__ (
					"movl %0, %%esp\n"
					"movl %0, %%ecx\n"
					"xorl %%edx, %%edx\n"
					"subl $32, %%esp\n"
					"jmpl *%2\n"
					:: "a" (stack), "b" (thread_self), "S" (wqueue_entry_point),
					"D" (me.flags | WQ_FLAG_THREAD_REUSE), "d" (me.event ? me.event->events : NULL),
					"S" (me.event ? me.event->nevents : 0)
			);
#else
#	error Missing assembly!
#endif
			__builtin_unreachable();

			return 0;
		}
		case WQOPS_SET_EVENT_MANAGER_PRIORITY:
		case WQOPS_QUEUE_NEWSPISUPP:
			return 0;
		case WQOPS_QUEUE_REQTHREAD_FOR_KEVENT:
		{
			wq_event = (struct wq_kevent_data*) item;
			affinity = 1;
		}
		case WQOPS_QUEUE_REQTHREADS2: // with prop bucket
		case WQOPS_QUEUE_REQTHREADS:
		{
			// affinity contains thread count

			int i, flags;

			flags = WQ_FLAG_THREAD_NEWSPI;
			flags |= prio & (WQ_FLAG_THREAD_PRIOMASK | WQ_FLAG_THREAD_OVERCOMMIT); //priority_to_class(prio);

			if (wq_event != NULL)
				flags |= WQ_FLAG_THREAD_KEVENT;
			
			// __simple_printf("Thread requested with prio %d\n", prio & WQ_FLAG_THREAD_PRIOMASK);

			for (i = 0; i < affinity; i++)
			{
				// Increase the amount of running threads
				sem_down(&workq_sem, -1);

				sem_down(&workq_parked_lock, -1);

				if (workq_parked_head.next != NULL)
				{
					struct parked_thread* thread;

					thread = workq_parked_head.next;

					// Resume an existing thread
					// __simple_printf("Resuming thread %d\n", id);

					thread->flags = flags;
					thread->event = wq_event;

					// Dequeue
					list_remove(&workq_parked_head, thread);

					// Resume the thread
					sem_up(&thread->sem);
					sem_up(&workq_parked_lock);

					continue;
				}

				sem_up(&workq_parked_lock);

				// __simple_printf("Spawning a new thread, nevents=%d\n", (wq_event != NULL) ? wq_event->nevents : -1);
				wq_event_pending = wq_event;

				__darling_thread_create(512*1024, pthread_obj_size, wqueue_entry_point, 0,
						(wq_event != NULL) ? wq_event->events : NULL, flags,
						(wq_event != NULL) ? wq_event->nevents : 0,
						thread_self_trap);

				/*if (ret < 0)
				{
					__simple_printf("Failed to spawn a new thread, err %d\n", -ret);
					return ret;
				}*/
			}
			return 0;
		}
		default:
			return -ENOTSUP;
	}
#else
	return -ENOSYS;
#endif
}

int sem_down(int* sem, int timeout)
{
	int result;
	struct timespec ts = { timeout, 0 };

try_again:
	result = __sync_add_and_fetch(sem, -1);
	if (result < 0)
	{
		*sem = -1;
		result = __linux_futex(sem, FUTEX_WAIT, -1, (timeout != -1) ? &ts : NULL, 0, 0);

		if (result == -1)
			return 0;

		goto try_again;
	}

	return 1;
}

void sem_up(int* sem)
{
	int result;
	result = __sync_add_and_fetch(sem, 1);
	if (result == 0)
	{
		*sem = 1;
		__linux_futex(sem, FUTEX_WAKE, 1, NULL, 0, 0);
	}
}

static void list_add(struct parked_thread* head, struct parked_thread* item)
{
	if (head->next != NULL)
	{
		item->prev = head->next->prev;
		item->next = head->next;
		head->next->prev = item;
	}
	else
	{
		item->next = NULL;
		item->prev = NULL;
	}
	head->next = item;

	if (head->prev == NULL)
		head->prev = item;
}

static void list_remove(struct parked_thread* head, struct parked_thread* item)
{
	if (item->prev != NULL)
		item->prev->next = item->next;
	if (item->next != NULL)
		item->next->prev = item->prev;
	if (head->next == item)
		head->next = item->next;
	if (head->prev == item)
		head->prev = item->prev;
}

#define __PTHREAD_PRIORITY_CBIT_USER_INTERACTIVE 0x20
#define __PTHREAD_PRIORITY_CBIT_USER_INITIATED 0x10
#define __PTHREAD_PRIORITY_CBIT_DEFAULT 0x8
#define __PTHREAD_PRIORITY_CBIT_UTILITY 0x4
#define __PTHREAD_PRIORITY_CBIT_BACKGROUND 0x2
#define __PTHREAD_PRIORITY_CBIT_MAINTENANCE 0x1
#define __PTHREAD_PRIORITY_CBIT_UNSPECIFIED 0x0

#define QOS_CLASS_USER_INTERACTIVE 0x21
#define QOS_CLASS_USER_INITIATED 0x19
#define QOS_CLASS_DEFAULT 0x15
#define QOS_CLASS_UTILITY 0x11
#define QOS_CLASS_BACKGROUND 0x09
#define QOS_CLASS_MAINTENANCE 0x05
#define QOS_CLASS_UNSPECIFIED 0x0

static int priority_to_class(int prio)
{
	int dec_prio = (prio & 0xff00) >> 8;
	switch (dec_prio)
	{
		case __PTHREAD_PRIORITY_CBIT_USER_INTERACTIVE:
			return __PTHREAD_PRIORITY_CBIT_MAINTENANCE;
		case __PTHREAD_PRIORITY_CBIT_USER_INITIATED:
			return QOS_CLASS_USER_INITIATED;
		case __PTHREAD_PRIORITY_CBIT_DEFAULT:
			return QOS_CLASS_DEFAULT;
		case __PTHREAD_PRIORITY_CBIT_UTILITY:
			return QOS_CLASS_UTILITY;
		case __PTHREAD_PRIORITY_CBIT_BACKGROUND:
			return QOS_CLASS_BACKGROUND;
		case __PTHREAD_PRIORITY_CBIT_MAINTENANCE:
			return QOS_CLASS_MAINTENANCE;
		default:
			return QOS_CLASS_UNSPECIFIED;
	}
}

