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
#include <linux/sockios.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

#include "private.h"

/*
 * Return the offset from the current position to end of file.
 */
static intptr_t
get_eof_offset(int fd)
{
    off_t curpos;
    struct stat sb;

    curpos = lseek(fd, 0, SEEK_CUR);
    if (curpos == (off_t) -1) {
        dbg_perror("lseek(2)");
        curpos = 0;
    }
    if (fstat(fd, &sb) < 0) {
        dbg_perror("fstat(2)");
        sb.st_size = 1;
    }

    dbg_printf("curpos=%zu size=%zu\n", (size_t)curpos, (size_t)sb.st_size);
    return (sb.st_size - curpos); //FIXME: can overflow
}

int
evfilt_read_copyout(struct kevent *dst, struct knote *src, void *ptr)
{
    struct epoll_event * const ev = (struct epoll_event *) ptr;

    /* Special case: for regular files, return the offset from current position to end of file */
    if (src->kn_flags & KNFL_REGULAR_FILE) {
        memcpy(dst, &src->kev, sizeof(*dst));
        dst->data = get_eof_offset(src->kev.ident);

        if (dst->data == 0) {
            dst->filter = 0;    /* Will cause the kevent to be discarded */
            if (epoll_ctl(src->kn_epollfd, EPOLL_CTL_DEL, src->kdata.kn_eventfd, NULL) < 0) {
                dbg_perror("epoll_ctl(2)");
                return (-1);
            }

#if FIXME
            /* XXX-FIXME Switch to using kn_inotifyfd to monitor for IN_ATTRIB events
                         that may signify the file size has changed.

                         This code is not tested.
             */
            int inofd;
            char path[PATH_MAX];

            inofd = inotify_init();
            if (inofd < 0) {
                dbg_perror("inotify_init(2)");
                (void) close(inofd);
                return (-1);
            } 
            src->kdata.kn_inotifyfd = inofd;
            if (linux_fd_to_path(&path[0], sizeof(path), src->kev.ident) < 0)
                return (-1);
            if (inotify_add_watch(inofd, path, IN_ATTRIB) < 0) {
                dbg_perror("inotify_add_watch");
                return (-1);
            }
            if (epoll_ctl(src->kn_epollfd, EPOLL_CTL_ADD, src->kdata.kn_inotifyfd, NULL) < 0) {
                dbg_perror("epoll_ctl(2)");
                return (-1);
            }
            /* FIXME: race here, should we check the EOF status again ? */
#endif
        }

        return (0);
    }

    dbg_printf("epoll: %s", epoll_event_dump(ev));
    memcpy(dst, &src->kev, sizeof(*dst));
#if defined(HAVE_EPOLLRDHUP)
    if (ev->events & EPOLLRDHUP || ev->events & EPOLLHUP)
        dst->flags |= EV_EOF;
#else
    if (ev->events & EPOLLHUP)
        dst->flags |= EV_EOF;
#endif
    if (ev->events & EPOLLERR)
        dst->fflags = 1; /* FIXME: Return the actual socket error */
          
    if (src->kn_flags & KNFL_PASSIVE_SOCKET) {
        /* On return, data contains the length of the 
           socket backlog. This is not available under Linux.
         */
        dst->data = 1;
    } else {
        /* On return, data contains the number of bytes of protocol
           data available to read.
         */
        if (ioctl(dst->ident, SIOCINQ, &dst->data) < 0) {
            /* race condition with socket close, so ignore this error */
            dbg_puts("ioctl(2) of socket failed");
            dst->data = 0;
        } else {
            if (dst->data == 0)
                dst->flags |= EV_EOF;
        }
    }

    return (0);
}

int
evfilt_read_knote_create(struct filter *filt, struct knote *kn)
{
    struct epoll_event ev;

    if (linux_get_descriptor_type(kn) < 0)
        return (-1);

    /* Convert the kevent into an epoll_event */
#if defined(HAVE_EPOLLRDHUP)
    kn->data.events = EPOLLIN | EPOLLRDHUP;
#else
    kn->data.events = EPOLLIN;
#endif
    if (kn->kev.flags & EV_ONESHOT || kn->kev.flags & EV_DISPATCH)
        kn->data.events |= EPOLLONESHOT;
    if (kn->kev.flags & EV_CLEAR)
        kn->data.events |= EPOLLET;

    memset(&ev, 0, sizeof(ev));
    ev.events = kn->data.events;
    ev.data.ptr = kn;

    /* Special case: for regular files, add a surrogate eventfd that is always readable */
    if (kn->kn_flags & KNFL_REGULAR_FILE) {
        int evfd;

        kn->kn_epollfd = filter_epfd(filt);
        evfd = eventfd(0, 0);
        if (evfd < 0) {
            dbg_perror("eventfd(2)");
            return (-1);
        }
        if (eventfd_write(evfd, 1) < 0) {
            dbg_perror("eventfd_write(3)");
            (void) close(evfd);
            return (-1);
        }

        kn->kdata.kn_eventfd = evfd;

        if (epoll_ctl(kn->kn_epollfd, EPOLL_CTL_ADD, kn->kdata.kn_eventfd, &ev) < 0) {
            dbg_printf("epoll_ctl(2): %s", strerror(errno));
            return (-1);
        }
        return (0);
    }

    return epoll_update(EPOLL_CTL_ADD, filt, kn, &ev);
}

int
evfilt_read_knote_modify(struct filter *filt, struct knote *kn, 
        const struct kevent *kev)
{
    (void) filt;
    (void) kn;
    (void) kev;
    return (-1); /* STUB */
}

int
evfilt_read_knote_delete(struct filter *filt, struct knote *kn)
{
    if (kn->kev.flags & EV_DISABLE)
        return (0);

    if ((kn->kn_flags & KNFL_REGULAR_FILE && kn->kdata.kn_eventfd != -1) < 0) {
        if (epoll_ctl(kn->kn_epollfd, EPOLL_CTL_DEL, kn->kdata.kn_eventfd, NULL) < 0) {
            dbg_perror("epoll_ctl(2)");
            return (-1);
        }
        (void) close(kn->kdata.kn_eventfd);
        kn->kdata.kn_eventfd = -1;
    } else {
        return epoll_update(EPOLL_CTL_DEL, filt, kn, NULL);
    }

    // clang will complain about not returning a value otherwise
    return (-1);
}

int
evfilt_read_knote_enable(struct filter *filt, struct knote *kn)
{
    struct epoll_event ev;

    memset(&ev, 0, sizeof(ev));
    ev.events = kn->data.events;
    ev.data.ptr = kn;

    if (kn->kn_flags & KNFL_REGULAR_FILE) {
        if (epoll_ctl(kn->kn_epollfd, EPOLL_CTL_ADD, kn->kdata.kn_eventfd, &ev) < 0) {
            dbg_perror("epoll_ctl(2)");
            return (-1);
        }
        return (0);
    } else {
        return epoll_update(EPOLL_CTL_ADD, filt, kn, &ev);
    }

    // clang will complain about not returning a value otherwise
    return (-1);
}

int
evfilt_read_knote_disable(struct filter *filt, struct knote *kn)
{
    if (kn->kn_flags & KNFL_REGULAR_FILE) {
        if (epoll_ctl(kn->kn_epollfd, EPOLL_CTL_DEL, kn->kdata.kn_eventfd, NULL) < 0) {
            dbg_perror("epoll_ctl(2)");
            return (-1);
        }
        return (0);
    } else {
        return epoll_update(EPOLL_CTL_DEL, filt, kn, NULL);
    }
}

const struct filter evfilt_read = {
    EVFILT_READ,
    NULL,
    NULL,
    evfilt_read_copyout,
    evfilt_read_knote_create,
    evfilt_read_knote_modify,
    evfilt_read_knote_delete,
    evfilt_read_knote_enable,
    evfilt_read_knote_disable,         
};
