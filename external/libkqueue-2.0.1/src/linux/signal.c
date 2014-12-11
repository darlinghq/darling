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

#include "private.h"

#if HAVE_SYS_SIGNALFD_H
# include <sys/signalfd.h>
#else
#define signalfd(a,b,c) syscall(SYS_signalfd, (a), (b), (c))
#define SFD_NONBLOCK 04000
struct signalfd_siginfo
{
  uint32_t ssi_signo;
  int32_t ssi_errno;
  int32_t ssi_code;
  uint32_t ssi_pid;
  uint32_t ssi_uid;
  int32_t ssi_fd;
  uint32_t ssi_tid;
  uint32_t ssi_band;
  uint32_t ssi_overrun;
  uint32_t ssi_trapno;
  int32_t ssi_status;
  int32_t ssi_int;
  uint64_t ssi_ptr;
  uint64_t ssi_utime;
  uint64_t ssi_stime;
  uint64_t ssi_addr;
  uint8_t __pad[48];
};
#endif

static void
signalfd_reset(int sigfd)
{
    struct signalfd_siginfo sig;
    ssize_t n;

    /* Discard any pending signal */
    n = read(sigfd, &sig, sizeof(sig));
    if (n < 0 || n != sizeof(sig)) {
        if (errno == EWOULDBLOCK)
            return;
        //FIXME: eintr?
        dbg_perror("read(2) from signalfd");
        abort();
    }
}

static int
signalfd_add(int epfd, int sigfd, void *ptr)
{
    struct epoll_event ev;
    int rv;

    /* Add the signalfd to the kqueue's epoll descriptor set */
    memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLIN;
    ev.data.ptr = ptr;
    rv = epoll_ctl(epfd, EPOLL_CTL_ADD, sigfd, &ev);
    if (rv < 0) {
        dbg_perror("epoll_ctl(2)");
        return (-1);
    }

    return (0);
}

static int
signalfd_create(int epfd, void *ptr, int signum)
{
    static int flags = SFD_NONBLOCK;
    sigset_t sigmask;
    int sigfd;

    /* Create a signalfd */
    sigemptyset(&sigmask);
    sigaddset(&sigmask, signum);
    sigfd = signalfd(-1, &sigmask, flags);

    /* WORKAROUND: Flags are broken on kernels older than Linux 2.6.27 */
    if (sigfd < 0 && errno == EINVAL && flags != 0) {
        flags = 0;
        sigfd = signalfd(-1, &sigmask, flags);
    }
    if (sigfd < 0) {
        dbg_perror("signalfd(2)");
        goto errout;
    }

    /* Block the signal handler from being invoked */
    if (sigprocmask(SIG_BLOCK, &sigmask, NULL) < 0) {
        dbg_perror("sigprocmask(2)");
        goto errout;
    }

    signalfd_reset(sigfd);

    if (signalfd_add(epfd, sigfd, ptr) < 0)
        goto errout;

    dbg_printf("added sigfd %d to epfd %d (signum=%d)", sigfd, epfd, signum);

    return (sigfd);

errout:
    (void) close(sigfd);
    return (-1);
}

int
evfilt_signal_copyout(struct kevent *dst, struct knote *src, void *x UNUSED)
{
    int sigfd;

    sigfd = src->kdata.kn_signalfd;

    signalfd_reset(sigfd);

    memcpy(dst, &src->kev, sizeof(*dst));
    /* NOTE: dst->data should be the number of times the signal occurred,
       but that information is not available.
     */
    dst->data = 1;  

    return (0);
}

int
evfilt_signal_knote_create(struct filter *filt, struct knote *kn)
{
    int fd;

    fd = signalfd_create(filter_epfd(filt), kn, kn->kev.ident);
    if (fd > 0) {
        kn->kev.flags |= EV_CLEAR;
        kn->kdata.kn_signalfd = fd;
        return (0);
    } else {
        kn->kdata.kn_signalfd = -1;
        return (-1);
    }
}

int
evfilt_signal_knote_modify(struct filter *filt UNUSED, 
        struct knote *kn UNUSED, 
        const struct kevent *kev UNUSED)
{
    /* Nothing to do since the signal number does not change. */

    return (0);
}

int
evfilt_signal_knote_delete(struct filter *filt, struct knote *kn)
{
    const int sigfd = kn->kdata.kn_signalfd;

    /* Needed so that delete() can be called after disable() */
    if (kn->kdata.kn_signalfd == -1)
        return (0);

    if (epoll_ctl(filter_epfd(filt), EPOLL_CTL_DEL, sigfd, NULL) < 0) {
        dbg_perror("epoll_ctl(2)");
        return (-1);
    }

    if (close(sigfd) < 0) {
        dbg_perror("close(2)");
        return (-1);
    }

    /* NOTE: This does not call sigprocmask(3) to unblock the signal. */
    kn->kdata.kn_signalfd = -1;

    return (0);
}

int
evfilt_signal_knote_enable(struct filter *filt, struct knote *kn)
{
    dbg_printf("enabling ident %u", (unsigned int) kn->kev.ident);
    return evfilt_signal_knote_create(filt, kn);
}

int
evfilt_signal_knote_disable(struct filter *filt, struct knote *kn)
{
    dbg_printf("disabling ident %u", (unsigned int) kn->kev.ident);
    return evfilt_signal_knote_delete(filt, kn);
}


const struct filter evfilt_signal = {
    EVFILT_SIGNAL,
    NULL,
    NULL,
    evfilt_signal_copyout,
    evfilt_signal_knote_create,
    evfilt_signal_knote_modify,
    evfilt_signal_knote_delete,
    evfilt_signal_knote_enable,
    evfilt_signal_knote_disable,         
};
