/*
 * Copyright (c) 2011 Mark Heily <mark@heily.com>
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

#ifndef  _KQUEUE_LINUX_PLATFORM_H
#define  _KQUEUE_LINUX_PLATFORM_H

struct filter;

#include <sys/syscall.h>
#include <sys/epoll.h>
#include <sys/queue.h>
#include <sys/inotify.h>
#if HAVE_SYS_EVENTFD_H
# include <sys/eventfd.h>
#else
# define eventfd(a,b) syscall(SYS_eventfd, (a), (b))

  static inline int eventfd_write(int fd, uint64_t val) {
      if (write(fd, &val, sizeof(val)) < (ssize_t) sizeof(val))
          return (-1);
      else
          return (0);
  }
#endif
#if HAVE_SYS_TIMERFD_H
# include <sys/timerfd.h>
#endif

/*
 * Get the current thread ID
 */
# define _GNU_SOURCE
# include <linux/unistd.h>
# include <unistd.h>
#ifndef __ANDROID__
extern long int syscall (long int __sysno, ...);
#endif
 
/* Convenience macros to access the epoll descriptor for the kqueue */
#define kqueue_epfd(kq)     ((kq)->kq_id)
#define filter_epfd(filt)   ((filt)->kf_kqueue->kq_id)

/*
 * Additional members of struct filter
 */
#undef FILTER_PLATFORM_SPECIFIC 

/*
 * Additional members of struct knote
 */
#define KNOTE_PLATFORM_SPECIFIC \
    int kn_epollfd; /* A copy of filter->epfd */      \
    union { \
        int kn_timerfd; \
        int kn_signalfd; \
        int kn_inotifyfd; \
        int kn_eventfd; \
    } kdata

/*
 * Additional members of struct kqueue
 */
#define KQUEUE_PLATFORM_SPECIFIC \
    struct epoll_event kq_plist[MAX_KEVENT]; \
    size_t kq_nplist

int     linux_kqueue_init(struct kqueue *);
void    linux_kqueue_free(struct kqueue *);

int     linux_kevent_wait(struct kqueue *, int, const struct timespec *);
int     linux_kevent_copyout(struct kqueue *, int, struct kevent *, int);

int     linux_knote_copyout(struct kevent *, struct knote *);

int     linux_eventfd_init(struct eventfd *);
void    linux_eventfd_close(struct eventfd *);
int     linux_eventfd_raise(struct eventfd *);
int     linux_eventfd_lower(struct eventfd *);
int     linux_eventfd_descriptor(struct eventfd *);

/* utility functions */

int     linux_get_descriptor_type(struct knote *);
int     linux_fd_to_path(char *, size_t, int);

/* epoll-related functions */

int     epoll_update(int, struct filter *, struct knote *, struct epoll_event *);
char *  epoll_event_dump(struct epoll_event *);

#endif  /* ! _KQUEUE_LINUX_PLATFORM_H */
