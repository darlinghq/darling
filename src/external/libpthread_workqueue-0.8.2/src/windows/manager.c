/*-
 * Copyright (c) 2011, Mark Heily <mark@heily.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "platform.h"
#include "../private.h"
#include "pthread_workqueue.h"

#ifdef PROVIDE_LEGACY_XP_SUPPORT

static LIST_HEAD(, _pthread_workqueue) wqlist[WORKQ_NUM_PRIOQUEUE];
static pthread_rwlock_t wqlist_mtx;

int
manager_init(void)
{
	pthread_rwlock_init(&wqlist_mtx, NULL);
    return (0);
}

void
manager_workqueue_create(struct _pthread_workqueue *workq)
{
    pthread_rwlock_wrlock(&wqlist_mtx);
    LIST_INSERT_HEAD(&wqlist[workq->queueprio], workq, wqlist_entry);
	pthread_rwlock_unlock(&wqlist_mtx);

	pthread_spin_init(&workq->mtx, PTHREAD_PROCESS_PRIVATE);
}

/* The caller must hold the wqlist_mtx. */
static struct work *
wqlist_scan(void)
{
    pthread_workqueue_t workq;
    struct work *witem = NULL;
    int i;

    pthread_rwlock_rdlock(&wqlist_mtx);
    for (i = 0; i < WORKQ_NUM_PRIOQUEUE; i++) {
        LIST_FOREACH(workq, &wqlist[i], wqlist_entry) {
			pthread_spin_lock(&workq->mtx);

            if (STAILQ_EMPTY(&workq->item_listhead)) {
				pthread_spin_unlock(&workq->mtx);
				continue;
			}

            witem = STAILQ_FIRST(&workq->item_listhead);
            if (witem != NULL)
                STAILQ_REMOVE_HEAD(&workq->item_listhead, item_entry);

			pthread_spin_unlock(&workq->mtx);
            goto out;
        }
    }

out:
    pthread_rwlock_unlock(&wqlist_mtx);
    return (witem);
}

DWORD WINAPI
worker_main(LPVOID arg)
{
    struct work *witem;

    witem = wqlist_scan();
    if (witem == NULL)
	    return (0);

    witem->func(witem->func_arg);
    free(witem);
    return (0);
}

void
manager_workqueue_additem(struct _pthread_workqueue *workq, struct work *witem)
{
    pthread_spin_lock(&workq->mtx);
    STAILQ_INSERT_TAIL(&workq->item_listhead, witem, item_entry);
    pthread_spin_unlock(&workq->mtx);
    if (!QueueUserWorkItem(worker_main, NULL, WT_EXECUTELONGFUNCTION))
	    abort();
}

#else

int
manager_init(void)
{
    return (0);
}

void
manager_workqueue_create(struct _pthread_workqueue *workq)
{
	PTP_POOL pool;
	PTP_CALLBACK_ENVIRON callback;
	SYSTEM_INFO sysinfo;

	pool = CreateThreadpool(NULL);
	if(pool == NULL){
		dbg_lasterror("CreateThreadpool()");
		return;
	}

	InitializeThreadpoolEnvironment(&workq->win_callback_env);
	callback = &workq->win_callback_env;
	SetThreadpoolCallbackPool(callback, pool);

	switch(workq->queueprio){
	case WORKQ_HIGH_PRIOQUEUE:
		// weird but this seems the only valid solution !?
		SetThreadpoolCallbackPriority(callback, TP_CALLBACK_PRIORITY_LOW);
		break;
	case WORKQ_LOW_PRIOQUEUE:
		// see above
		SetThreadpoolCallbackPriority(callback, TP_CALLBACK_PRIORITY_HIGH);
		break;
	default:
		SetThreadpoolCallbackPriority(callback, TP_CALLBACK_PRIORITY_NORMAL);
		break;
	}

	// we need a proper way to implement overcommitting on windows
	if(workq->overcommit){
		GetSystemInfo(&sysinfo);
		SetThreadpoolThreadMaximum(pool, sysinfo.dwNumberOfProcessors * 2);
	}

	workq->win_thread_pool = pool;
}

VOID CALLBACK 
worker_main( PTP_CALLBACK_INSTANCE instance, PVOID Parameter, PTP_WORK work )
{
	struct work* witem = (struct work*)Parameter;

    assert(witem);
	witem->func(witem->func_arg);
    free(witem);
	CloseThreadpoolWork(work);
}

void
manager_workqueue_additem(struct _pthread_workqueue *workq, struct work *witem)
{
	PTP_WORK work = CreateThreadpoolWork(worker_main, witem, &workq->win_callback_env);
	if(work == NULL) {
		dbg_lasterror("CreateThreadpoolWork()");
		return;
	}
	SubmitThreadpoolWork(work);
}

// TODO: We need to cleanly close the environment and threadpools!

#endif

unsigned long
manager_peek(const char *key)
{
    unsigned long rv;

    if (strcmp(key, "combined_idle") == 0) {
        dbg_puts("TODO");
        abort();
    } else {
        dbg_printf("invalid key: ", key);
        abort();
    }

    return rv;
}
