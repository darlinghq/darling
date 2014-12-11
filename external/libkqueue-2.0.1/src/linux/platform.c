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

# define _GNU_SOURCE
# include <poll.h>
#include "../common/private.h"

//XXX-FIXME TEMP
const struct filter evfilt_proc = EVFILT_NOTIMPL;

/*
 * Per-thread epoll event buffer used to ferry data between
 * kevent_wait() and kevent_copyout().
 */
static __thread struct epoll_event epevt[MAX_KEVENT];

const struct kqueue_vtable kqops = {
    linux_kqueue_init,
    linux_kqueue_free,
	linux_kevent_wait,
	linux_kevent_copyout,
	NULL,
	NULL,
    linux_eventfd_init,
    linux_eventfd_close,
    linux_eventfd_raise,
    linux_eventfd_lower,
    linux_eventfd_descriptor
};

int
linux_kqueue_init(struct kqueue *kq)
{
    kq->kq_id = epoll_create(1);
    if (kq->kq_id < 0) {
        dbg_perror("epoll_create(2)");
        return (-1);
    }

    if (filter_register_all(kq) < 0) {
        close(kq->kq_id);
        return (-1);
    }


 #if DEADWOOD
    //might be useful in posix

    /* Add each filter's pollable descriptor to the epollset */
    for (i = 0; i < EVFILT_SYSCOUNT; i++) {
        filt = &kq->kq_filt[i];

        if (filt->kf_id == 0)
            continue;

        memset(&ev, 0, sizeof(ev));
        ev.events = EPOLLIN;
        ev.data.ptr = filt;

        if (epoll_ctl(kq->kq_id, EPOLL_CTL_ADD, filt->kf_pfd, &ev) < 0) {
            dbg_perror("epoll_ctl(2)");
            close(kq->kq_id);
            return (-1);
        }
    }
#endif

    return (0);
}

void
linux_kqueue_free(struct kqueue *kq UNUSED)
{
    abort();//FIXME
}

static int
linux_kevent_wait_hires(
        struct kqueue *kq, 
        const struct timespec *timeout)
{
    int n;
#if HAVE_DECL_PPOLL
    struct pollfd fds;

    dbg_printf("waiting for events (timeout=%ld sec %ld nsec)",
            timeout->tv_sec, timeout->tv_nsec);
    fds.fd = kqueue_epfd(kq);
    fds.events = POLLIN;

    n = ppoll(&fds, 1, timeout, NULL);
#else
    int epfd;
    fd_set fds;

    dbg_printf("waiting for events (timeout=%ld sec %ld nsec)",
            timeout->tv_sec, timeout->tv_nsec);

    epfd = kqueue_epfd(kq);
    FD_ZERO(&fds);
    FD_SET(epfd, &fds);
    n = pselect(epfd + 1, &fds, NULL , NULL, timeout, NULL);
#endif

    if (n < 0) {
        if (errno == EINTR) {
            dbg_puts("signal caught");
            return (-1);
        }
        dbg_perror("ppoll(2) or pselect(2)");
        return (-1);
    }
    return (n);
}

int
linux_kevent_wait(
        struct kqueue *kq, 
        int nevents,
        const struct timespec *ts)
{
    int timeout, nret;

    /* Use a high-resolution syscall if the timeout value is less than one millisecond.  */
    if (ts != NULL && ts->tv_sec == 0 && ts->tv_nsec > 0 && ts->tv_nsec < 1000000) {
        nret = linux_kevent_wait_hires(kq, ts);
        if (nret <= 0)
            return (nret);

        /* epoll_wait() should have ready events */
        timeout = 0;
    } else {
        /* Convert timeout to the format used by epoll_wait() */
        if (ts == NULL) 
            timeout = -1;
        else
            timeout = (1000 * ts->tv_sec) + (ts->tv_nsec / 1000000);
    }

    dbg_puts("waiting for events");
    nret = epoll_wait(kqueue_epfd(kq), &epevt[0], nevents, timeout);
    if (nret < 0) {
        dbg_perror("epoll_wait");
        return (-1);
    }

    return (nret);
}

int
linux_kevent_copyout(struct kqueue *kq, int nready,
        struct kevent *eventlist, int nevents UNUSED)
{
    struct epoll_event *ev;
    struct filter *filt;
    struct knote *kn;
    int i, nret, rv;

    nret = nready;
    for (i = 0; i < nready; i++) {
        ev = &epevt[i];
        kn = (struct knote *) ev->data.ptr;
        filt = &kq->kq_filt[~(kn->kev.filter)];
        rv = filt->kf_copyout(eventlist, kn, ev);
        if (slowpath(rv < 0)) {
            dbg_puts("knote_copyout failed");
            /* XXX-FIXME: hard to handle this without losing events */
            abort();
        }

        /*
         * Certain flags cause the associated knote to be deleted
         * or disabled.
         */
        if (eventlist->flags & EV_DISPATCH) 
            knote_disable(filt, kn); //FIXME: Error checking
        if (eventlist->flags & EV_ONESHOT) {
            knote_delete(filt, kn); //FIXME: Error checking
        }

        /* If an empty kevent structure is returned, the event is discarded. */
        /* TODO: add these semantics to windows + solaris platform.c */
        if (fastpath(eventlist->filter != 0)) {
            eventlist++;
        } else {
            dbg_puts("spurious wakeup, discarding event");
            nret--;
        }
    }

    return (nret);
}

int
linux_eventfd_init(struct eventfd *e)
{
    int evfd;

    evfd = eventfd(0, 0);
    if (evfd < 0) {
        dbg_perror("eventfd");
        return (-1);
    }
    if (fcntl(evfd, F_SETFL, O_NONBLOCK) < 0) {
        dbg_perror("fcntl");
        close(evfd);
        return (-1);
    }
    e->ef_id = evfd;

    return (0);
}

void
linux_eventfd_close(struct eventfd *e)
{
    close(e->ef_id);
    e->ef_id = -1;
}

int
linux_eventfd_raise(struct eventfd *e)
{
    uint64_t counter;
    int rv = 0;

    dbg_puts("raising event level");
    counter = 1;
    if (write(e->ef_id, &counter, sizeof(counter)) < 0) {
        switch (errno) {
            case EAGAIN:    
                /* Not considered an error */
                break;

            case EINTR:
                rv = -EINTR;
                break;

            default:
                dbg_printf("write(2): %s", strerror(errno));
                rv = -1;
        }
    }
    return (rv);
}

int
linux_eventfd_lower(struct eventfd *e)
{
    uint64_t cur;
    ssize_t n;
    int rv = 0;

    /* Reset the counter */
    dbg_puts("lowering event level");
    n = read(e->ef_id, &cur, sizeof(cur));
    if (n < 0) {
        switch (errno) {
            case EAGAIN:    
                /* Not considered an error */
                break;

            case EINTR:
                rv = -EINTR;
                break;

            default:
                dbg_printf("read(2): %s", strerror(errno));
                rv = -1;
        }
    } else if (n != sizeof(cur)) {
        dbg_puts("short read");
        rv = -1;
    }

    return (rv);
}

int
linux_eventfd_descriptor(struct eventfd *e)
{
    return (e->ef_id);
}

int
linux_get_descriptor_type(struct knote *kn)
{
    socklen_t slen;
    struct stat sb;
    int i, lsock;

    /*
     * Test if the descriptor is a socket.
     */
    if (fstat(kn->kev.ident, &sb) < 0) {
        dbg_perror("fstat(2)");
        return (-1);
    }
    if (S_ISREG(sb.st_mode)) {
        kn->kn_flags |= KNFL_REGULAR_FILE;
        dbg_printf("fd %d is a regular file\n", (int)kn->kev.ident);
        return (0);
    }

    /*
     * Test if the socket is active or passive.
     */
    if (! S_ISSOCK(sb.st_mode))
        return (0);

    slen = sizeof(lsock);
    lsock = 0;
    i = getsockopt(kn->kev.ident, SOL_SOCKET, SO_ACCEPTCONN, (char *) &lsock, &slen);
    if (i < 0) {
        switch (errno) {
            case ENOTSOCK:   /* same as lsock = 0 */
                return (0);
                break;
            default:
                dbg_perror("getsockopt(3)");
                return (-1);
        }
    } else {
        if (lsock) 
            kn->kn_flags |= KNFL_PASSIVE_SOCKET;
        return (0);
    }
}

char *
epoll_event_dump(struct epoll_event *evt)
{
    static __thread char buf[128];

    if (evt == NULL)
        return "(null)";

#define EPEVT_DUMP(attrib) \
    if (evt->events & attrib) \
       strcat(&buf[0], #attrib" ");

    snprintf(&buf[0], 128, " { data = %p, events = ", evt->data.ptr);
    EPEVT_DUMP(EPOLLIN);
    EPEVT_DUMP(EPOLLOUT);
#if defined(HAVE_EPOLLRDHUP)
    EPEVT_DUMP(EPOLLRDHUP);
#endif
    EPEVT_DUMP(EPOLLONESHOT);
    EPEVT_DUMP(EPOLLET);
    strcat(&buf[0], "}\n");

    return (&buf[0]);
#undef EPEVT_DUMP
}

int
epoll_update(int op, struct filter *filt, struct knote *kn, struct epoll_event *ev)
{
    dbg_printf("op=%d fd=%d events=%s", op, (int)kn->kev.ident, 
            epoll_event_dump(ev));
    if (epoll_ctl(filter_epfd(filt), op, kn->kev.ident, ev) < 0) {
        dbg_printf("epoll_ctl(2): %s", strerror(errno));
        return (-1);
    }

    return (0);
}

/*
 * Given a file descriptor, return the path to the file it refers to.
 */
int
linux_fd_to_path(char *buf, size_t bufsz, int fd)
{
    char path[1024];    //TODO: Maxpathlen, etc.

    if (snprintf(&path[0], sizeof(path), "/proc/%d/fd/%d", getpid(), fd) < 0)
        return (-1);

    memset(buf, 0, bufsz);
    return (readlink(path, buf, bufsz));
}

