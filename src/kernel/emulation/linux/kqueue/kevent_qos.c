#include "kevent_qos.h"
#include "../base.h"
#include "../errno.h"
#include "../simple.h"
#include "../fcntl/fcntl.h"
#include <linux-syscalls/linux.h>
#include <stddef.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include "kqueue.h"
#include "../bsdthread/workq_kernreturn.h"

// not static because `mach_init.c` needs to reset it on fork
// (but since we're using symbol visibility, no one else will be able to see it, so it's ok)
// we can remove this once our LKM supports kqueue workqs and workloops (at that point, this syscall will turn into a simple trap into the LKM)
int default_kq = -1;

extern void* memmove(void* dst, const void* src, __SIZE_TYPE__ n);

// we have an LKM trap for kevent_qos, but due to our LKM not supporting workqs and workloops yet,
// it's easier to keep our old behavior and translate this into a kevent64 call and handle workq stuff ourselves.
// TODO: support workqs and workloops in the LKM (requires in-kernel pthread operations)

static void kevent_qos_to_64(const struct kevent_qos_s *ev, struct kevent64_s* ev64);
static void kevent_64_to_qos(const struct kevent64_s* ev64, struct kevent_qos_s *ev);

long sys_kevent_qos(int	kq, const struct kevent_qos_s *changelist, int nchanges,
			struct	kevent_qos_s *eventlist, int nevents,
			void* data_out, unsigned long* data_available, unsigned int flags)
{
	int rv;
	struct kevent64_s* changelist64 = NULL;
	struct kevent64_s* eventlist64 = NULL;
	struct wq_kevent_data wq_kevent;
	struct timespec polling_timeout = {};

	if ((kq == -1) != !!(flags & KEVENT_FLAG_WORKQ))
		return -EINVAL;

	if (kq < 0) {
		if (default_kq == -1) {
			default_kq = sys_kqueue();
		}
		kq = default_kq;
	}

	if (changelist != NULL && nchanges > 0)
	{
		int i;

		changelist64 = (struct kevent64_s*) __builtin_alloca(nchanges * sizeof(struct kevent64_s));
		for (i = 0; i < nchanges; i++)
			kevent_qos_to_64(&changelist[i], &changelist64[i]);
	}
	if (eventlist != NULL && nevents > 0)
		eventlist64 = (struct kevent64_s*) __builtin_alloca(nevents * sizeof(struct kevent64_s));
	if (nevents == 0 && (flags & KEVENT_FLAG_WORKQ))
	{
		eventlist = (struct kevent_qos_s*) __builtin_alloca(nevents * sizeof(struct kevent_qos_s));
		nevents = 1;
		eventlist64 = (struct kevent64_s*) __builtin_alloca(nevents * sizeof(struct kevent64_s));
	}

	rv = kevent64(kq, changelist64, nchanges, eventlist64, nevents, flags, (flags & KEVENT_FLAG_IMMEDIATE) ? &polling_timeout : NULL);

	if (rv > 0)
	{
		int i;

		for (i = 0; i < rv; i++)
			kevent_64_to_qos(&eventlist64[i], &eventlist[i]);
	}

	if (flags & KEVENT_FLAG_WORKQ)
	{
		// Pass to workqueue and wait
		wq_kevent.sem = 0;
		wq_kevent.events = eventlist;
		wq_kevent.nevents = rv;

		sys_workq_kernreturn(WQOPS_QUEUE_REQTHREAD_FOR_KEVENT, &wq_kevent, 1, 0);
		
		// __simple_printf("waiting for workq processing\n");
		sem_down(&wq_kevent.sem, -1);

		rv = wq_kevent.nevents;
		memmove(eventlist, wq_kevent.events, rv * sizeof(struct kevent_qos_s));
		// __simple_printf("workq done processing, returning %d to caller\n", rv);
	}
	
	return rv;
}


void kevent_qos_to_64(const struct kevent_qos_s *ev, struct kevent64_s* ev64)
{
	ev64->ident = ev->ident;
	ev64->filter = ev->filter;
	ev64->flags = ev->flags;
	ev64->fflags = ev->fflags;
	ev64->data = ev->data;
	ev64->udata = ev->udata;
	ev64->ext[0] = ev->ext[0];
	ev64->ext[1] = ev->ext[1];
}

void kevent_64_to_qos(const struct kevent64_s* ev64, struct kevent_qos_s *ev)
{
	ev->ident = ev64->ident;
	ev->filter = ev64->filter;
	ev->flags = ev64->flags;
	ev->qos = 0;
	ev->fflags = ev64->fflags;
	ev->data = ev64->data;
	ev->udata = ev64->udata;
	ev->ext[0] = ev64->ext[0];
	ev->ext[1] = ev64->ext[1];
	ev->ext[2] = ev->ext[3] = 0;
}

