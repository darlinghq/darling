/*-
 * Copyright (c) 2011, Joakim Johansson <jocke@tbricks.com>
 * Copyright (c) 2010, Mark Heily <mark@heily.com>
 * Copyright (c) 2009, Stacey Son <sson@freebsd.org>
 * Copyright (c) 2000-2008, Apple Inc.
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
#include "private.h"
#include "pthread_workqueue.h"
#include "thread_info.h"
#include "thread_rt.h"

#include <sys/time.h>

/* Environment setting */
unsigned int PWQ_RT_THREADS = 0;
time_t PWQ_SPIN_USEC = 10000; // The number of microseconds we should spin loop if desired
unsigned int PWQ_SPIN_THREADS = 0; // The number of threads that should be kept spinning
unsigned volatile int current_threads_spinning = 0; // The number of threads currently spinning

/* Tunable constants */

#define WORKER_IDLE_SECONDS_THRESHOLD 5

/* Function prototypes */
static unsigned int get_load_average(void);
static void * worker_main(void *arg);
static void * overcommit_worker_main(void *arg);
static unsigned int get_process_limit(void);
static void manager_start(void);

static unsigned int      cpu_count;
static unsigned int      worker_min;
static unsigned int      worker_idle_threshold; // we don't go down below this if we had to increase # workers

/* Overcommit */
static struct _pthread_workqueue *ocwq[PTHREAD_WORKQUEUE_MAX];
static int               ocwq_mask;
static pthread_mutex_t   ocwq_mtx;
static pthread_cond_t    ocwq_has_work;
static unsigned int      ocwq_idle_threads;

/* Non-overcommit */
static struct _pthread_workqueue *wqlist[PTHREAD_WORKQUEUE_MAX];
static volatile unsigned int     wqlist_mask; // mask of currently pending workqueues, atomics used for manipulation
static pthread_mutex_t   wqlist_mtx;

static pthread_cond_t    wqlist_has_work;
static int               wqlist_has_manager;
static pthread_attr_t    detached_attr;

static struct {
    volatile unsigned int load,
                    count,
                    idle;
    unsigned int    sb_wake_pending;
    pthread_mutex_t sb_wake_mtx;
    pthread_cond_t  sb_wake_cond;
} scoreboard;

static unsigned int 
worker_idle_threshold_per_cpu(void)
{
    switch (cpu_count)
    {
        case 0:
        case 1:
        case 2:
        case 4:
          return 2;
        case 6:
          return 3;
        case 8:
        case 12:
          return 4;
        case 16:
        case 24:
          return 6;
        case 32:
        case 64:
          return 8;
        default:
            return cpu_count / 4;
    }
    
    return 2;
}

static void
manager_reinit(void)
{
    if (manager_init() < 0)
        abort();
}

int
manager_init(void)
{
    wqlist_has_manager = 0;
    pthread_cond_init(&wqlist_has_work, NULL);

    pthread_mutex_init(&wqlist_mtx, NULL);
    wqlist_mask = 0;

    pthread_cond_init(&ocwq_has_work, NULL);
    pthread_mutex_init(&ocwq_mtx, NULL);
    ocwq_mask = 0;
    ocwq_idle_threads = 0;

    witem_cache_init();

    cpu_count = (PWQ_ACTIVE_CPU > 0) ? (PWQ_ACTIVE_CPU) : (unsigned int) sysconf(_SC_NPROCESSORS_ONLN);

    pthread_attr_init(&detached_attr);
    pthread_attr_setdetachstate(&detached_attr, PTHREAD_CREATE_DETACHED);

    /* Initialize the scoreboard */
    pthread_cond_init(&scoreboard.sb_wake_cond, NULL);
    pthread_mutex_init(&scoreboard.sb_wake_mtx, NULL);

    /* Determine the initial thread pool constraints */
    worker_min = 2; // we can start with a small amount, worker_idle_threshold will be used as new dynamic low watermark
    worker_idle_threshold = worker_idle_threshold_per_cpu();

    if (pthread_atfork(NULL, NULL, manager_reinit) < 0) {
        dbg_perror("pthread_atfork()");
        return (-1);
    }

    return (0);
}

void
manager_workqueue_create(struct _pthread_workqueue *workq)
{
    pthread_mutex_lock(&wqlist_mtx);
    if (!workq->overcommit && !wqlist_has_manager)
        manager_start();

    if (workq->overcommit) {
        if (ocwq[workq->queueprio] == NULL) {
            ocwq[workq->queueprio] = workq;
            workq->wqlist_index = workq->queueprio;
        } else {
            puts("queue already exists\n");
            abort();
        }
    } else {
        if (wqlist[workq->queueprio] == NULL) {
            wqlist[workq->queueprio] = workq; //FIXME: sort by priority
            workq->wqlist_index = workq->queueprio;
        } else {
            puts("queue already exists\n");
            abort();
        }
    }
    pthread_mutex_unlock(&wqlist_mtx);
}

static struct work *
wqlist_scan(int *queue_priority)
{
    pthread_workqueue_t workq;
    struct work *witem;
    int idx;

    idx = ffs(wqlist_mask);
    if (idx == 0)
        return (NULL);

    workq = wqlist[idx - 1];
 
    pthread_spin_lock(&workq->mtx);

    witem = STAILQ_FIRST(&workq->item_listhead);
    if (witem != NULL) {
        STAILQ_REMOVE_HEAD(&workq->item_listhead, item_entry);
        if (STAILQ_EMPTY(&workq->item_listhead))
        {
            unsigned int wqlist_index_bit = (0x1 << workq->wqlist_index);
            unsigned int new_mask;
            // Remove this now empty wq from the mask, the only contention here is with threads performing the same
            // operation on another workqueue, so we will not be long
            // the 'bit' for this queue is protected by the spin lock, so we will only clear a bit which we have 
            // ownership for (see additem() below for the corresponding part on the producer side)
            do
            {
                new_mask = atomic_and(&wqlist_mask, ~(wqlist_index_bit));
            } while (new_mask & wqlist_index_bit);
        }
        if (queue_priority != NULL)
            *queue_priority = workq->queueprio;

        pthread_spin_unlock(&workq->mtx);
        return (witem);
    } else {
        // this could happen if multiple threads raced and found the same bit with ffs() and
        // emptied the queue completely, so we should just bail out
        pthread_spin_unlock(&workq->mtx);
        return (NULL);
    }
}

static void _wakeup_manager(void)
{
    dbg_puts("asking manager to wake up");

    pthread_mutex_lock(&scoreboard.sb_wake_mtx);
    scoreboard.sb_wake_pending = 1;
    pthread_cond_signal(&scoreboard.sb_wake_cond);
    pthread_mutex_unlock(&scoreboard.sb_wake_mtx);
    return;
}

static void *
overcommit_worker_main(void *arg)
{
    struct timespec ts;
    pthread_workqueue_t workq;
    void (*func)(void *);
    void *func_arg;
    struct work *witem;
    int rv, idx;

    (void)arg;
     
    pthread_mutex_lock(&ocwq_mtx);

    for (;;) {
        /* Find the highest priority workqueue that is non-empty */
        idx = ffs(ocwq_mask);
        if (idx > 0) {
            workq = ocwq[idx - 1];
            witem = STAILQ_FIRST(&workq->item_listhead);
            if (witem != NULL) {
                /* Remove the first work item */
                STAILQ_REMOVE_HEAD(&workq->item_listhead, item_entry);
                if (STAILQ_EMPTY(&workq->item_listhead))
                    ocwq_mask &= ~(0x1 << workq->wqlist_index);
                /* Execute the work item */
                pthread_mutex_unlock(&ocwq_mtx);
                func = witem->func;
                func_arg = witem->func_arg;
                witem_free(witem);
                func(func_arg);    
                pthread_mutex_lock(&ocwq_mtx);
                continue;
            }
        }

        /* Wait for more work to be available. */
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 15;
        ocwq_idle_threads++;
        dbg_printf("waiting for work (idle=%d)", ocwq_idle_threads);
        rv = pthread_cond_timedwait(&ocwq_has_work, &ocwq_mtx, &ts);
        if (rv != 0) {
            /* Normally, the signaler will decrement the idle counter,
               but this path is not taken in response to a signaler.
             */
            ocwq_idle_threads--;
	    pthread_mutex_unlock(&ocwq_mtx);

            if (rv == ETIMEDOUT) {
                dbg_puts("timeout, no work available");
                break;
            } else {
                dbg_perror("pthread_cond_timedwait");
                //TODO: some kind of crash mechanism
                break;
            }
        }
    }

    dbg_printf("worker exiting (idle=%d)", ocwq_idle_threads);
    pthread_exit(NULL);
}

static void *
worker_main(void *arg)
{
    struct work *witem;
    void (*func)(void *);
    void *func_arg;
    int queue_priority = 0;
    struct timespec ts_start, ts_now;
    
    (void) arg;
    dbg_puts("worker thread started");

    if (PWQ_RT_THREADS)
        ptwq_set_current_thread_priority(WORKQ_HIGH_PRIOQUEUE); // start at highest priority possible
        
    for (;;) {

        witem = wqlist_scan(&queue_priority);

        // Only take overhead of sleeping and/or spinning if we 
        // could not get a witem cheaply using the spinlock above
        if (slowpath(!witem))
        {
            // Optional busy loop for getting the next item for a while if so configured
            // We'll only spin limited thread at a time (this is really mostly useful when running
            // in low latency configurations using dedicated processor sets)
            if ((PWQ_SPIN_THREADS > 0) && (current_threads_spinning <= PWQ_SPIN_THREADS))
            {
                atomic_inc(&current_threads_spinning);

                // If we are racing with another thread, let's skip
                // spinning and instead go through the slowpath below

                if (current_threads_spinning <= PWQ_SPIN_THREADS)
                {
                    clock_gettime(CLOCK_REALTIME, &ts_start);
                    ts_now.tv_sec = ts_start.tv_sec;
                    ts_now.tv_nsec = ts_start.tv_nsec;
                    
                    // Spin until we get an item or PWQ_SPIN_USEC microseconds passes
                    while (!witem && (((ts_now.tv_sec - ts_start.tv_sec) * 1000000) + (((ts_now.tv_nsec - ts_start.tv_nsec) / 1000)) <= PWQ_SPIN_USEC))
                    {
                        witem = wqlist_scan(&queue_priority);
                        if (!witem)
                        {
                            // Perhaps a hardware pause
                            // instruction could be used here to keep the pace down, probably not needed though
                            clock_gettime(CLOCK_REALTIME, &ts_now);
                        }
                    }                
                }
                
                atomic_dec(&current_threads_spinning);
            }

            // No witem from the busy loop, let's wait for wakeup
            if (!witem)
            {
                pthread_mutex_lock(&wqlist_mtx);

                /*
                  TODO: Consider using pthread_cond_timedwait() so that
                  workers can self-terminate if they are idle too long.
                  This would also be a failsafe in case there are bugs
                  with the scoreboard that cause us to "leak" workers.
                 */
                while ((witem = wqlist_scan(&queue_priority)) == NULL)
                    pthread_cond_wait(&wqlist_has_work, &wqlist_mtx);

                pthread_mutex_unlock(&wqlist_mtx);
            }
        }

        atomic_dec(&scoreboard.idle);

        if (slowpath(witem->func == NULL)) {
            dbg_puts("worker exiting..");
            atomic_dec(&scoreboard.count);
            witem_free(witem);
            pthread_exit(0);
        }

        dbg_printf("count=%u idle=%u wake_pending=%u", 
            scoreboard.count, scoreboard.idle,  scoreboard.sb_wake_pending);
        
        /* Force the manager thread to wakeup if all workers are busy */
        if (slowpath(scoreboard.idle == 0 && !scoreboard.sb_wake_pending))
            _wakeup_manager();

        // If using RT threads, decrease thread prio if we aren't a high prio queue
        if (PWQ_RT_THREADS && (queue_priority != WORKQ_HIGH_PRIOQUEUE))
            ptwq_set_current_thread_priority(queue_priority);
        
        /* Invoke the callback function, free witem first for possible reuse */
        func = witem->func;
        func_arg = witem->func_arg;
        witem_free(witem);
        
        func(func_arg);    
        
        atomic_inc(&scoreboard.idle); // initial inc was one in worker_start, this is to avoid a race

        // Only take the overhead and change RT priority back if it was not a high priority queue being serviced
        if (PWQ_RT_THREADS && (queue_priority != WORKQ_HIGH_PRIOQUEUE))
            ptwq_set_current_thread_priority(WORKQ_HIGH_PRIOQUEUE);
    }

    /* NOTREACHED */
    return (NULL);
}

static int
worker_start(void) 
{
    pthread_t tid;

    dbg_puts("Spawning another worker");

    atomic_inc(&scoreboard.idle);
    atomic_inc(&scoreboard.count);

    if (pthread_create(&tid, &detached_attr, worker_main, NULL) != 0) {
        dbg_perror("pthread_create(3)");
        atomic_dec(&scoreboard.idle);
        atomic_dec(&scoreboard.count);
        return (-1);
    }

    return (0);
}

static int
worker_stop(void) 
{
    struct work *witem;
    pthread_workqueue_t workq;
    int i;
    unsigned int wqlist_index_bit, new_mask;

    witem = witem_alloc(NULL, NULL);

    pthread_mutex_lock(&wqlist_mtx);
    for (i = 0; i < PTHREAD_WORKQUEUE_MAX; i++) {
        workq = wqlist[i];
        if (workq == NULL)
            continue;

        wqlist_index_bit = (0x1 << workq->wqlist_index);

        pthread_spin_lock(&workq->mtx);
        
        do
        {
            new_mask = atomic_or(&wqlist_mask, wqlist_index_bit);
        } while (!(new_mask & wqlist_index_bit));

        STAILQ_INSERT_TAIL(&workq->item_listhead, witem, item_entry);

        pthread_spin_unlock(&workq->mtx);
        
        pthread_cond_signal(&wqlist_has_work);
        pthread_mutex_unlock(&wqlist_mtx);

        return (0);
    }

    /* FIXME: this means there are no workqueues.. should never happen */
    dbg_puts("Attempting to add a workitem without a workqueue");
    abort();

    return (-1);
}

static void *
manager_main(void *unused __attribute__ ((unused)))
{
    unsigned int load_max = cpu_count;
    unsigned int worker_max, current_thread_count = 0;
    unsigned int worker_idle_seconds_accumulated = 0;
    unsigned int max_threads_to_stop = 0;
    unsigned int i;
    int cond_wait_rv = 0;
    sigset_t sigmask;
    struct timespec   ts;
    struct timeval    tp;

    worker_max = get_process_limit();
    scoreboard.load = get_load_average();

    /* Block all signals */
    sigfillset(&sigmask);
    pthread_sigmask(SIG_BLOCK, &sigmask, NULL);

    /* Create the minimum number of workers */
    scoreboard.count = 0;
    for (i = 0; i < worker_min; i++)
        worker_start();

    for (;;) {

        pthread_mutex_lock(&scoreboard.sb_wake_mtx);

        dbg_puts("manager is sleeping");
        
        (void) gettimeofday(&tp, NULL); // TODO - error checking
        
        /* Convert from timeval to timespec */
        ts.tv_sec  = tp.tv_sec;
        ts.tv_nsec = tp.tv_usec * 1000;
        ts.tv_sec += 1; // wake once per second and check if we have too many idle threads...

        // We should only sleep on the condition if there are no pending signal, spurious wakeup is also ok
        if (scoreboard.sb_wake_pending == 0)
            cond_wait_rv = pthread_cond_timedwait(&scoreboard.sb_wake_cond, &scoreboard.sb_wake_mtx, &ts);

        scoreboard.sb_wake_pending = 0; // we must set this before spawning any new threads below, or we race...

        dbg_puts("manager is awake");

        dbg_printf("load=%u idle=%u workers=%u max_workers=%u worker_min = %u",
                   scoreboard.load, scoreboard.idle, scoreboard.count, worker_max, worker_min);
                
        // If no workers available, check if we should create a new one
        if (scoreboard.idle == 0 && (scoreboard.count > 0)) // last part required for an extremely unlikely race at startup
        {
            scoreboard.load = get_load_average();
            
            if ((scoreboard.load < load_max) && (scoreboard.count < worker_max)) 
            {
                if (scoreboard.count < worker_idle_threshold) // allow cheap rampup up to worker_idle_threshold without going to /proc
                {
                    worker_start();
                }
                else // check through /proc, will be a bit more expensive in terms of latency
                if (threads_runnable(&current_thread_count) == 0)
                {
                    // only start thread if we have less runnable threads than cpus
                    if (current_thread_count >= cpu_count)
                    {
                        dbg_printf("Not spawning worker thread, thread_runnable = %d >= cpu_count = %d", 
                                   current_thread_count, cpu_count);
                    }
                    else
                    {
                        worker_start();
                    }
                }
                else // always start thread if we can't get runnable count
                {
                    worker_start();
                }
            }
            else // high load, allow rampup up to worker_idle_threshold regardless of this
            {
                if (scoreboard.count < worker_idle_threshold) 
                {
                    worker_start();
                }                
            }
        }
        else
        {
            if (cond_wait_rv == ETIMEDOUT) // Only check for ramp down on the 'timer tick'
            {
                if ((scoreboard.idle - worker_idle_threshold) > 0) // only accumulate if there are 'too many' idle threads
                {
                    worker_idle_seconds_accumulated += scoreboard.idle; // keep track of many idle 'thread seconds' we have
                
                    dbg_printf("worker_idle_seconds_accumulated = %d, scoreboard.idle = %d, scoreboard.count = %d\n",
                       worker_idle_seconds_accumulated, scoreboard.idle, scoreboard.count);
                }
                
                // Only consider ramp down if we have accumulated enough thread 'idle seconds'
                // this logic will ensure that a large number of idle threads will ramp down faster
                max_threads_to_stop = worker_idle_seconds_accumulated / WORKER_IDLE_SECONDS_THRESHOLD;

                if (max_threads_to_stop > 0)
                {
                    worker_idle_seconds_accumulated = 0; 

                    if (max_threads_to_stop > (scoreboard.idle - worker_idle_threshold))
                        max_threads_to_stop = (scoreboard.idle - worker_idle_threshold);

                    // Only stop threads if we actually have 'too many' idle ones in the pool
                    if (scoreboard.idle > worker_idle_threshold)
                    {
                        for (i = 0; i < max_threads_to_stop; i++)
                        {
                            dbg_puts("Removing one thread from the thread pool");
                            worker_stop();
                        }                    
                    }
                }
            }            
        }
        
        pthread_mutex_unlock(&scoreboard.sb_wake_mtx);
    }

    /*NOTREACHED*/
    return (NULL);
}

static void
manager_start(void)
{
    pthread_t tid;
    int rv;

    dbg_puts("starting the manager thread");

    do {
        rv = pthread_create(&tid, &detached_attr, manager_main, NULL);
        if (rv == EAGAIN) {
            sleep(1);
        } else if (rv != 0) {
            /* FIXME: not nice */
            dbg_printf("thread creation failed, rv=%d", rv);
            abort();
        }
    } while (rv != 0);

    wqlist_has_manager = 1;
}

void
manager_workqueue_additem(struct _pthread_workqueue *workq, struct work *witem)
{
    unsigned int wqlist_index_bit = (0x1 << workq->wqlist_index);
    
    if (workq->overcommit) {
        pthread_t tid;

        pthread_mutex_lock(&ocwq_mtx);
        pthread_spin_lock(&workq->mtx);
        STAILQ_INSERT_TAIL(&workq->item_listhead, witem, item_entry);
        pthread_spin_unlock(&workq->mtx);
        ocwq_mask |= wqlist_index_bit;
        if (ocwq_idle_threads > 0) {
            dbg_puts("signaling an idle worker");
            pthread_cond_signal(&ocwq_has_work);
            ocwq_idle_threads--;
        } else {
            (void)pthread_create(&tid, &detached_attr, overcommit_worker_main, NULL);
        }
        pthread_mutex_unlock(&ocwq_mtx);
    } else {
        pthread_spin_lock(&workq->mtx);

        // Only set the mask for the first item added to the workqueue. 
        if (STAILQ_EMPTY(&workq->item_listhead))
        {
            unsigned int new_mask;
            
            // The only possible contention here are with threads performing the same
            // operation on another workqueue, so we will not be blocked long... 
            // Threads operating on the same workqueue will be serialized by the spinlock so it is very unlikely.
            do
            {
                new_mask = atomic_or(&wqlist_mask, wqlist_index_bit);
            } while (!(new_mask & wqlist_index_bit));
        }
        
        STAILQ_INSERT_TAIL(&workq->item_listhead, witem, item_entry);

        pthread_spin_unlock(&workq->mtx);

        // Only signal thread wakeup if there are idle threads available
        // and no other thread have managed to race us and empty the wqlist on our behalf already
        if ((scoreboard.idle > 0)) // && ((wqlist_mask & wqlist_index_bit) != 0)) // disabling this fringe optimization for now
        {
            pthread_mutex_lock(&wqlist_mtx);
            pthread_cond_signal(&wqlist_has_work);
            pthread_mutex_unlock(&wqlist_mtx);
        }
    }
}


static unsigned int
get_process_limit(void)
{
#if __linux__
    struct rlimit rlim;

    if (getrlimit(RLIMIT_NPROC, &rlim) < 0) {
        dbg_perror("getrlimit(2)");
        return (50);
    } else {
        return (rlim.rlim_max);
    }
#else
    /* Solaris doesn't define this limit anywhere I can see.. */
    return (64);
#endif
}

static unsigned int
get_load_average(void)
{
    double loadavg;

    /* TODO: proper error handling */
    if (getloadavg(&loadavg, 1) != 1) {
        dbg_perror("getloadavg(3)");
        return (1);
    }
    if (loadavg > INT_MAX || loadavg < 0)
        loadavg = 1;

    return ((int) loadavg);
}

unsigned long 
manager_peek(const char *key)
{
    uint64_t rv;

    if (strcmp(key, "combined_idle") == 0) {
        rv = scoreboard.idle;
        if (scoreboard.idle > worker_min)
            rv -= worker_min;
        rv += ocwq_idle_threads;
    } else if (strcmp(key, "idle") == 0) {
        rv = scoreboard.idle;
        if (scoreboard.idle > worker_min)
            rv -= worker_min;
    } else if (strcmp(key, "ocomm_idle") == 0) {
        rv = ocwq_idle_threads;
    } else {
        dbg_printf("invalid key: %s", key);
        abort();
    }

    return rv;
}
