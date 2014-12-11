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

#include "common.h"

static int sigusr1_caught = 0;
static pid_t pid;
static int kqfd;

static void
sig_handler(int signum)
{
    sigusr1_caught = 1;
}

static void
test_kevent_proc_add(struct test_context *ctx)
{
    struct kevent kev;

    test_no_kevents(kqfd);
    kevent_add(kqfd, &kev, pid, EVFILT_PROC, EV_ADD, 0, 0, NULL);
    test_no_kevents(kqfd);
}

static void
test_kevent_proc_delete(struct test_context *ctx)
{
    struct kevent kev;

    test_no_kevents(kqfd);
    kevent_add(kqfd, &kev, pid, EVFILT_PROC, EV_DELETE, 0, 0, NULL);
    if (kill(pid, SIGKILL) < 0)
        die("kill");
    sleep(1);
    test_no_kevents(kqfd);
}

static void
test_kevent_proc_get(struct test_context *ctx)
{
    struct kevent kev, buf;

    /* Create a child that waits to be killed and then exits */
    pid = fork();
    if (pid == 0) {
        pause();
        printf(" -- child caught signal, exiting\n");
        exit(2);
    }
    printf(" -- child created (pid %d)\n", (int) pid);

    test_no_kevents(kqfd);
    kevent_add(kqfd, &kev, pid, EVFILT_PROC, EV_ADD, 0, 0, NULL);

    /* Cause the child to exit, then retrieve the event */
    printf(" -- killing process %d\n", (int) pid);
    if (kill(pid, SIGUSR1) < 0)
        die("kill");
    kevent_get(&buf, kqfd);
    kevent_cmp(&kev, &buf);
    test_no_kevents(kqfd);
}

#ifdef TODO
void
test_kevent_signal_disable(struct test_context *ctx)
{
    const char *test_id = "kevent(EVFILT_SIGNAL, EV_DISABLE)";
    struct kevent kev;

    test_begin(test_id);

    EV_SET(&kev, SIGUSR1, EVFILT_SIGNAL, EV_DISABLE, 0, 0, NULL);
    if (kevent(kqfd, &kev, 1, NULL, 0, NULL) < 0)
        die("%s", test_id);

    /* Block SIGUSR1, then send it to ourselves */
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1)
        die("sigprocmask");
    if (kill(getpid(), SIGKILL) < 0)
        die("kill");

    test_no_kevents();

    success();
}

void
test_kevent_signal_enable(struct test_context *ctx)
{
    const char *test_id = "kevent(EVFILT_SIGNAL, EV_ENABLE)";
    struct kevent kev;

    test_begin(test_id);

    EV_SET(&kev, SIGUSR1, EVFILT_SIGNAL, EV_ENABLE, 0, 0, NULL);
    if (kevent(kqfd, &kev, 1, NULL, 0, NULL) < 0)
        die("%s", test_id);

    /* Block SIGUSR1, then send it to ourselves */
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1)
        die("sigprocmask");
    if (kill(getpid(), SIGUSR1) < 0)
        die("kill");

    kev.flags = EV_ADD | EV_CLEAR;
#if LIBKQUEUE
    kev.data = 1; /* WORKAROUND */
#else
    kev.data = 2; // one extra time from test_kevent_signal_disable()
#endif
    kevent_cmp(&kev, kevent_get(kqfd));

    /* Delete the watch */
    kev.flags = EV_DELETE;
    if (kevent(kqfd, &kev, 1, NULL, 0, NULL) < 0)
        die("%s", test_id);

    success();
}

void
test_kevent_signal_del(struct test_context *ctx)
{
    const char *test_id = "kevent(EVFILT_SIGNAL, EV_DELETE)";
    struct kevent kev;

    test_begin(test_id);

    /* Delete the kevent */
    EV_SET(&kev, SIGUSR1, EVFILT_SIGNAL, EV_DELETE, 0, 0, NULL);
    if (kevent(kqfd, &kev, 1, NULL, 0, NULL) < 0)
        die("%s", test_id);

    /* Block SIGUSR1, then send it to ourselves */
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1)
        die("sigprocmask");
    if (kill(getpid(), SIGUSR1) < 0)
        die("kill");

    test_no_kevents();
    success();
}

void
test_kevent_signal_oneshot(struct test_context *ctx)
{
    const char *test_id = "kevent(EVFILT_SIGNAL, EV_ONESHOT)";
    struct kevent kev;

    test_begin(test_id);

    EV_SET(&kev, SIGUSR1, EVFILT_SIGNAL, EV_ADD | EV_ONESHOT, 0, 0, NULL);
    if (kevent(kqfd, &kev, 1, NULL, 0, NULL) < 0)
        die("%s", test_id);

    /* Block SIGUSR1, then send it to ourselves */
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1)
        die("sigprocmask");
    if (kill(getpid(), SIGUSR1) < 0)
        die("kill");

    kev.flags |= EV_CLEAR;
    kev.data = 1;
    kevent_cmp(&kev, kevent_get(kqfd));

    /* Send another one and make sure we get no events */
    if (kill(getpid(), SIGUSR1) < 0)
        die("kill");
    test_no_kevents();

    success();
}
#endif

void
test_evfilt_proc(struct test_context *ctx)
{
    signal(SIGUSR1, sig_handler);

    /* Create a child that waits to be killed and then exits */
    pid = fork();
    if (pid == 0) {
        pause();
        exit(2);
    }
    printf(" -- child created (pid %d)\n", (int) pid);

    test(kevent_proc_add, ctx);
    test(kevent_proc_delete, ctx);
    test(kevent_proc_get, ctx);

    signal(SIGUSR1, SIG_DFL);

#if TODO
    test_kevent_signal_add();
    test_kevent_signal_del();
    test_kevent_signal_get();
    test_kevent_signal_disable();
    test_kevent_signal_enable();
    test_kevent_signal_oneshot();
#endif
}
