#include "workq_kernreturn.h"
#include "bsdthread_register.h"
#include "bsdthread_create.h"
#include "../base.h"
#include "../errno.h"
#include <sys/errno.h>
#include <linux-syscalls/linux.h>
#include <stddef.h>
#include <pthread/tsd_private.h>
#include "../ext/futex.h"
#include "../simple.h"
#include <sys/queue.h>
#include <os/lock.h>
#include <elfcalls/threads.h>
#include "../machdep/tls.h"
#include "../mach/mach_traps.h"

#define __PTHREAD_EXPOSE_INTERNALS__ 1
#include <pthread/priority_private.h>

// much easier to include than libpthread's `internal.h`
#include "../../../../startup/mldr/elfcalls/dthreads.h"

#include <darlingserver/rpc.h>

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
#define WQOPS_SETUP_DISPATCH 0x400

// Flags for the newly spawned thread:
// WQ_FLAG_THREAD_NEWSPI
// WQ_FLAG_THREAD_REUSE (0 if thread is newly spawned)
// item is unused in "new SPI"

#define WQ_FLAG_THREAD_PRIO_SCHED    0x00008000
#define WQ_FLAG_THREAD_PRIO_QOS      0x00004000
#define WQ_FLAG_THREAD_PRIO_MASK     0x00000fff

#define WQ_FLAG_THREAD_OVERCOMMIT    0x00010000
#define WQ_FLAG_THREAD_REUSE         0x00020000
#define WQ_FLAG_THREAD_NEWSPI        0x00040000
#define WQ_FLAG_THREAD_KEVENT        0x00080000
#define WQ_FLAG_THREAD_EVENT_MANAGER 0x00100000
#define WQ_FLAG_THREAD_TSD_BASE_SET  0x00200000
#define WQ_FLAG_THREAD_WORKLOOP      0x00400000
#define WQ_FLAG_THREAD_OUTSIDEQOS    0x00800000

#define WORKQ_EXIT_THREAD_NKEVENT (-1)

static int workq_sem = WQ_MAX_THREADS; // max 64 threads in use
static os_unfair_lock workq_parked_lock = OS_UNFAIR_LOCK_INIT;
// static int workq_parked_threads = 0; // num spawned, but unused threads
static int workq_parked[WQ_MAX_THREADS];
static int workq_parked_prio[WQ_MAX_THREADS];

TAILQ_HEAD(tailhead, parked_thread) workq_parked_head = TAILQ_HEAD_INITIALIZER(workq_parked_head);

struct parked_thread
{
	int sem, flags;
	struct wq_kevent_data* event;
	TAILQ_ENTRY(parked_thread) entries;
};

static void list_add(struct parked_thread* head, struct parked_thread* item);
static void list_remove(struct parked_thread* head, struct parked_thread* item);

struct timespec
{
	long tv_sec;
	long tv_nsec;
};

void __attribute__((weak)) os_unfair_lock_unlock(os_unfair_lock_t lock) {}
void __attribute__((weak)) os_unfair_lock_lock(os_unfair_lock_t lock) {}

//void* __attribute__((weak)) __attribute__((visibility("default"))) pthread_getspecific(unsigned long key) { return NULL; }
//int __attribute__((weak)) __attribute__((visibility("default"))) pthread_setspecific(unsigned long key, const void* value) { return 1; }

static int priority_to_class(int prio);

// This is horrible, but it may work
static struct wq_kevent_data* wq_event_pending = NULL;

static const struct darling_thread_create_callbacks callbacks = {
	.thread_self_trap = &thread_self_trap_impl,
	.thread_set_tsd_base = &sys_thread_set_tsd_base,
	.dserver_rpc_checkin = &dserver_rpc_checkin,
	.dserver_rpc_checkout = &dserver_rpc_checkout,
	.kprintf = &__simple_kprintf,
};

static int extract_wq_flags(int priority) {
	int flags = 0;
	int qos = _pthread_priority_thread_qos(priority);

	flags = WQ_FLAG_THREAD_NEWSPI | qos | WQ_FLAG_THREAD_PRIO_QOS;

	if (priority & _PTHREAD_PRIORITY_OVERCOMMIT_FLAG) {
		flags |= WQ_FLAG_THREAD_OVERCOMMIT;
	}

	return flags;
};

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
			dthread_t dthread;
			bool terminating = false;

			os_unfair_lock_lock(&workq_parked_lock);

			// Semaphore locked state (wait for wakeup)
			me.sem = 0;

			// extract initial flags
			// (in case we only get created and used once and then terminate; `_pthread_wqthread` requires a valid `flags` argument)
			dthread = _pthread_getspecific_direct(_PTHREAD_TSD_SLOT_PTHREAD_SELF);
			prio = _pthread_getspecific_direct(_PTHREAD_TSD_SLOT_PTHREAD_QOS_CLASS);
			// doesn't extract `WQ_FLAG_THREAD_KEVENT` if we had it set, but that shouldn't matter
			// like i said before, the only case where we actually need these flags to be set here is when the thread is going to die immediately after creation
			me.flags = extract_wq_flags(prio);

			// Enqueue for future WQOPS_QUEUE_REQTHREADS
			TAILQ_INSERT_HEAD(&workq_parked_head, &me, entries);

			// Decrease the amount of running threads
			sem_up(&workq_sem);

			os_unfair_lock_unlock(&workq_parked_lock);

			// Wait until someone calls WQOPS_QUEUE_REQTHREADS
			// and wakes us up
			if (!sem_down(&me.sem, 15))
			{
				// Make sure we haven't just been woken up before locking the queue
				// and remove us from the queue if not.
				os_unfair_lock_lock(&workq_parked_lock);
	
				if (me.sem > 0)
				{
					os_unfair_lock_unlock(&workq_parked_lock);
					goto wakeup;
				}

				TAILQ_REMOVE(&workq_parked_head, &me, entries);

				os_unfair_lock_unlock(&workq_parked_lock);

				terminating = true;

				// resume the thread; it'll call `_pthread_wqthread_exit` and kill itself
				goto resume_thread;
			}

wakeup: // we actually want to go back into the thread to do work
			// __simple_printf("Thread %d woken up, prio=%d\n", thread_self, me.flags & WQ_FLAG_THREAD_PRIOMASK);

			if (me.event)
				wq_event_pending = me.event;

resume_thread: // we want the thread to resume, but it might be just to die
			// reset stack and call entry point again with WQ_FLAG_THREAD_REUSE
			thread_self = _pthread_getspecific_direct(_PTHREAD_TSD_SLOT_MACH_THREAD_SELF);
			dthread = _pthread_getspecific_direct(_PTHREAD_TSD_SLOT_PTHREAD_SELF);

#ifdef __x86_64__
			// arguments are in rdi, rsi, rdx, rcx, r8, r9
			__asm__ __volatile__ (
					// "int3\n"
					"movl %3, %%r8d\n" // 5th argument
					"movl %5, %%r9d\n" // 6th argument
					"movq %0, %%rsp\n"
					"subq $32, %%rsp\n"
					"jmpq *%2\n"
					:: "D" (dthread), "S" (thread_self), "a" (wqueue_entry_point),
					"r" (me.flags | WQ_FLAG_THREAD_REUSE), "c" ((!terminating && me.event) ? me.event->events : NULL),
					"r" (terminating ? WORKQ_EXIT_THREAD_NKEVENT : (me.event ? me.event->nevents : 0)), "d" (dthread->stackbottom)
			);
#elif defined(__i386__)
			// Arguments are in eax, ebx, ecx, edx, edi, esi
			__asm__ __volatile__ (
					"movl %0, %%esp\n"
					"subl $32, %%esp\n"
					"jmpl *%2\n"
					:: "a" (dthread), "b" (thread_self), "S" (wqueue_entry_point),
					"D" (me.flags | WQ_FLAG_THREAD_REUSE), "d" ((!terminating && me.event) ? me.event->events : NULL),
					"S" (terminating ? WORKQ_EXIT_THREAD_NKEVENT : (me.event ? me.event->nevents : 0)), "c" (dthread->stackbottom)
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

			int i, flags = extract_wq_flags(prio);

			if (wq_event != NULL)
				flags |= WQ_FLAG_THREAD_KEVENT;
			
			// __simple_printf("Thread requested with prio %d\n", prio & WQ_FLAG_THREAD_PRIOMASK);

			for (i = 0; i < affinity; i++)
			{
				// Increase the amount of running threads
				sem_down(&workq_sem, -1);

				os_unfair_lock_lock(&workq_parked_lock);

				if (workq_parked_head.tqh_first != NULL)
				{
					struct parked_thread* thread;

					thread = workq_parked_head.tqh_first;

					// Resume an existing thread
					// __simple_printf("Resuming thread %d\n", id);

					thread->flags = flags;
					thread->event = wq_event;

					// Dequeue
					TAILQ_REMOVE(&workq_parked_head, thread, entries);

					// Resume the thread
					sem_up(&thread->sem);
					os_unfair_lock_unlock(&workq_parked_lock);

					continue;
				}

				os_unfair_lock_unlock(&workq_parked_lock);

				// __simple_printf("Spawning a new thread, nevents=%d\n", (wq_event != NULL) ? wq_event->nevents : -1);
				wq_event_pending = wq_event;

				__darling_thread_create(512*1024, pthread_obj_size, wqueue_entry_point_wrapper, 0,
						(wq_event != NULL) ? wq_event->events : NULL, flags,
						(wq_event != NULL) ? wq_event->nevents : 0,
						&callbacks, NULL);

				/*if (ret < 0)
				{
					__simple_printf("Failed to spawn a new thread, err %d\n", -ret);
					return ret;
				}*/
			}
			return 0;
		}
		case WQOPS_SETUP_DISPATCH: {
			// TODO: actually do something with the info passed in
			// XNU just tacks it onto the process its called for and uses it later when someone asks for it

			/*
			struct workq_dispatch_config cfg = {0};

			// `affinity` contains the size of the config structure passed in
			memcpy(&cfg, item, (affinity < sizeof(struct workq_dispatch_config)) ? affinity : sizeof(struct workq_dispatch_config));
			*/

			return 0;
		} break;
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

