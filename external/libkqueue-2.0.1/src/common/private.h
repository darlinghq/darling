/*
 * Copyright (c) 2009 Mark Heily <mark@heily.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef  _KQUEUE_PRIVATE_H
#define  _KQUEUE_PRIVATE_H

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "tree.h"

/* Maximum events returnable in a single kevent() call */
#define MAX_KEVENT  512

struct kqueue;
struct kevent;
struct knote;
struct map;
struct eventfd;
struct evfilt_data;

#if defined(_WIN32)
# include "../windows/platform.h"
# include "../common/queue.h"
# if !defined(NDEBUG) && !defined(__GNUC__)
#  include <crtdbg.h>
# endif
#elif defined(__linux__)
# include "../posix/platform.h"
# include "../linux/platform.h"
#elif defined(__sun)
# include "../posix/platform.h"
# include "../solaris/platform.h"
#else
# error Unknown platform
#endif

#include "debug.h"

/* Workaround for Android */
#ifndef EPOLLONESHOT
# define EPOLLONESHOT (1 << 30)
#endif

struct eventfd {
    int ef_id;
#if defined(EVENTFD_PLATFORM_SPECIFIC)
    EVENTFD_PLATFORM_SPECIFIC;
#endif
};

/* 
 * Flags used by knote->kn_flags
 */
#define KNFL_PASSIVE_SOCKET  (0x01)  /* Socket is in listen(2) mode */
#define KNFL_REGULAR_FILE    (0x02)  /* File descriptor is a regular file */
#define KNFL_KNOTE_DELETED   (0x10)  /* The knote object is no longer valid */
 
struct knote {
    struct kevent     kev;
    int               kn_flags;       
    union {
        /* OLD */
        int           pfd;       /* Used by timerfd */
        int           events;    /* Used by socket */
        struct {
            nlink_t   nlink;  /* Used by vnode */
            off_t     size;   /* Used by vnode */
        } vnode;
        timer_t       timerid;  
        struct sleepreq *sleepreq; /* Used by posix/timer.c */
		void          *handle;      /* Used by win32 filters */
    } data;
	struct kqueue*	   kn_kq;
    volatile uint32_t  kn_ref;
#if defined(KNOTE_PLATFORM_SPECIFIC)
    KNOTE_PLATFORM_SPECIFIC;
#endif
    RB_ENTRY(knote)   kn_entries;
};

#define KNOTE_ENABLE(ent)           do {                            \
            (ent)->kev.flags &= ~EV_DISABLE;                        \
} while (0/*CONSTCOND*/)

#define KNOTE_DISABLE(ent)          do {                            \
            (ent)->kev.flags |=  EV_DISABLE;                        \
} while (0/*CONSTCOND*/)

struct filter {
    short     kf_id;

    /* filter operations */

    int     (*kf_init)(struct filter *);
    void    (*kf_destroy)(struct filter *);
    int     (*kf_copyout)(struct kevent *, struct knote *, void *);

    /* knote operations */

    int     (*kn_create)(struct filter *, struct knote *);
    int     (*kn_modify)(struct filter *, struct knote *, 
                            const struct kevent *);
    int     (*kn_delete)(struct filter *, struct knote *);
    int     (*kn_enable)(struct filter *, struct knote *);
    int     (*kn_disable)(struct filter *, struct knote *);

    struct eventfd kf_efd;             /* Used by user.c */

    //MOVE TO POSIX?
    int       kf_pfd;                   /* fd to poll(2) for readiness */
    int       kf_wfd;                   /* fd to write when an event occurs */
    //----?

    struct evfilt_data *kf_data;	    /* filter-specific data */
    RB_HEAD(knt, knote) kf_knote;
    pthread_rwlock_t    kf_knote_mtx;
    struct kqueue      *kf_kqueue;
#if defined(FILTER_PLATFORM_SPECIFIC)
    FILTER_PLATFORM_SPECIFIC;
#endif
};

/* Use this to declare a filter that is not implemented */
#define EVFILT_NOTIMPL { 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }

struct kqueue {
    int             kq_id;
    struct filter   kq_filt[EVFILT_SYSCOUNT];
    fd_set          kq_fds, kq_rfds; 
    int             kq_nfds;
    tracing_mutex_t kq_mtx;
    volatile uint32_t kq_ref;
#if defined(KQUEUE_PLATFORM_SPECIFIC)
    KQUEUE_PLATFORM_SPECIFIC;
#endif
    RB_ENTRY(kqueue) entries;
};

struct kqueue_vtable {
    int  (*kqueue_init)(struct kqueue *);
    void (*kqueue_free)(struct kqueue *);
    // @param timespec can be given as timeout
    // @param int the number of events to wait for
    // @param kqueue the queue to wait on
	int  (*kevent_wait)(struct kqueue *, int, const struct timespec *);
    // @param kqueue the queue to look at
    // @param int The number of events that should be ready
    // @param kevent the structure to copy the events into
    // @param int The number of events to copy
    // @return the actual number of events copied
    int  (*kevent_copyout)(struct kqueue *, int, struct kevent *, int);
    int  (*filter_init)(struct kqueue *, struct filter *);
    void (*filter_free)(struct kqueue *, struct filter *);
    int  (*eventfd_init)(struct eventfd *);
    void (*eventfd_close)(struct eventfd *);
    int  (*eventfd_raise)(struct eventfd *);
    int  (*eventfd_lower)(struct eventfd *);
    int  (*eventfd_descriptor)(struct eventfd *);
};
extern const struct kqueue_vtable kqops;

/*
 * kqueue internal API
 */
#define kqueue_lock(kq)     tracing_mutex_lock(&(kq)->kq_mtx)
#define kqueue_unlock(kq)   tracing_mutex_unlock(&(kq)->kq_mtx)

/*
 * knote internal API
 */
struct knote * knote_lookup(struct filter *, uintptr_t);
//DEADWOOD: struct knote * knote_get_by_data(struct filter *filt, intptr_t);
struct knote * knote_new(void);
#define knote_retain(kn) atomic_inc(&kn->kn_ref)
void knote_release(struct knote *);
void knote_insert(struct filter *, struct knote *);
int  knote_delete(struct filter *, struct knote *);
int  knote_init(void);
int  knote_disable(struct filter *, struct knote *);
#define knote_get_filter(knt) &((knt)->kn_kq->kq_filt[(knt)->kev.filter])

int         filter_lookup(struct filter **, struct kqueue *, short);
int      	filter_register_all(struct kqueue *);
void     	filter_unregister_all(struct kqueue *);
const char *filter_name(short);

int         kevent_wait(struct kqueue *, const struct timespec *);
int         kevent_copyout(struct kqueue *, int, struct kevent *, int);
void 		kevent_free(struct kqueue *);
const char *kevent_dump(const struct kevent *);
struct kqueue * kqueue_lookup(int);
int         kqueue_validate(struct kqueue *);

struct map *map_new(size_t);
int         map_insert(struct map *, int, void *);
int         map_remove(struct map *, int, void *);
int         map_replace(struct map *, int, void *, void *);
void       *map_lookup(struct map *, int);
void       *map_delete(struct map *, int);
void        map_free(struct map *);

/* DEADWOOD: No longer needed due to the un-smerging of POSIX and Linux

int  posix_evfilt_user_init(struct filter *);
void posix_evfilt_user_destroy(struct filter *);
int  posix_evfilt_user_copyout(struct kevent *, struct knote *, void *ptr UNUSED);
int  posix_evfilt_user_knote_create(struct filter *, struct knote *);
int  posix_evfilt_user_knote_modify(struct filter *, struct knote *, const struct kevent *);
int  posix_evfilt_user_knote_delete(struct filter *, struct knote *);
int  posix_evfilt_user_knote_enable(struct filter *, struct knote *);
int  posix_evfilt_user_knote_disable(struct filter *, struct knote *);

*/

#endif  /* ! _KQUEUE_PRIVATE_H */
