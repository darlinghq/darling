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

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

#include "sys/event.h"
#include "private.h"

/* NOTE: copy+pasted from linux_eventfd_raise() */
static int
eventfd_raise(int evfd)
{
    uint64_t counter;
    int rv = 0;

    dbg_puts("raising event level");
    counter = 1;
    if (write(evfd, &counter, sizeof(counter)) < 0) {
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

/* NOTE: copy+pasted from linux_eventfd_lower() */
static int
eventfd_lower(int evfd)
{
    uint64_t cur;
    ssize_t n;
    int rv = 0;

    /* Reset the counter */
    dbg_puts("lowering event level");
    n = read(evfd, &cur, sizeof(cur));
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
linux_evfilt_user_copyout(struct kevent *dst, struct knote *src, void *ptr UNUSED)
{
    memcpy(dst, &src->kev, sizeof(*dst));
    dst->fflags &= ~NOTE_FFCTRLMASK;     //FIXME: Not sure if needed
    dst->fflags &= ~NOTE_TRIGGER;
    if (src->kev.flags & EV_ADD) {
        /* NOTE: True on FreeBSD but not consistent behavior with
           other filters. */
        dst->flags &= ~EV_ADD;
    }
    if (src->kev.flags & EV_CLEAR)
        src->kev.fflags &= ~NOTE_TRIGGER;
    if (src->kev.flags & (EV_DISPATCH | EV_CLEAR | EV_ONESHOT)) {
        if (eventfd_lower(src->kdata.kn_eventfd) < 0)
            return (-1);
    }

    if (src->kev.flags & EV_DISPATCH)
        src->kev.fflags &= ~NOTE_TRIGGER;

    return (0);
}

int
linux_evfilt_user_knote_create(struct filter *filt, struct knote *kn)
{
    struct epoll_event ev;
    int evfd;

    /* Create an eventfd */
    evfd = eventfd(0, 0);
    if (evfd < 0) {
        dbg_perror("eventfd");
        goto errout;
    }

    /* Add the eventfd to the epoll set */
    memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLIN;
    ev.data.ptr = kn;
    if (epoll_ctl(filter_epfd(filt), EPOLL_CTL_ADD, evfd, &ev) < 0) {
        dbg_perror("epoll_ctl(2)");
        goto errout;
    }

    kn->kdata.kn_eventfd = evfd;

    return (0);

errout:
    (void) close(evfd);
    kn->kdata.kn_eventfd = -1;
    return (-1);
}

int
linux_evfilt_user_knote_modify(struct filter *filt UNUSED, struct knote *kn, 
        const struct kevent *kev)
{
    unsigned int ffctrl;
    unsigned int fflags;

    /* Excerpted from sys/kern/kern_event.c in FreeBSD HEAD */
    ffctrl = kev->fflags & NOTE_FFCTRLMASK;
    fflags = kev->fflags & NOTE_FFLAGSMASK;
    switch (ffctrl) {
        case NOTE_FFNOP:
            break;

        case NOTE_FFAND:
            kn->kev.fflags &= fflags;
            break;

        case NOTE_FFOR:
            kn->kev.fflags |= fflags;
            break;

        case NOTE_FFCOPY:
            kn->kev.fflags = fflags;
            break;

        default:
            /* XXX Return error? */
            break;
    }

    if ((!(kn->kev.flags & EV_DISABLE)) && kev->fflags & NOTE_TRIGGER) {
        kn->kev.fflags |= NOTE_TRIGGER;
        if (eventfd_raise(kn->kdata.kn_eventfd) < 0)
            return (-1);
    }

    return (0);
}

int
linux_evfilt_user_knote_delete(struct filter *filt, struct knote *kn)
{
    if (epoll_ctl(filter_epfd(filt), EPOLL_CTL_DEL, 
                kn->kdata.kn_eventfd, NULL) < 0) {
        dbg_perror("epoll_ctl(2)");
        return (-1);
    }
    if (close(kn->kdata.kn_eventfd) < 0) {
        dbg_perror("close(2)");
        return (-1);
    }
    dbg_printf("removed eventfd %d from the epollfd", kn->kdata.kn_eventfd); 
    kn->kdata.kn_eventfd = -1;

    return (0);
}

int
linux_evfilt_user_knote_enable(struct filter *filt, struct knote *kn)
{
    /* FIXME: what happens if NOTE_TRIGGER is in fflags?
       should the event fire? */
    return linux_evfilt_user_knote_create(filt, kn);
}

int
linux_evfilt_user_knote_disable(struct filter *filt, struct knote *kn)
{
    return linux_evfilt_user_knote_delete(filt, kn);
}

const struct filter evfilt_user = {
    EVFILT_USER,
    NULL,
    NULL,
    linux_evfilt_user_copyout,
    linux_evfilt_user_knote_create,
    linux_evfilt_user_knote_modify,
    linux_evfilt_user_knote_delete,
    linux_evfilt_user_knote_enable,
    linux_evfilt_user_knote_disable,   
};
