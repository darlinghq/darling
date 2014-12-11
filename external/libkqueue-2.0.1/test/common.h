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

#ifndef _COMMON_H
#define _COMMON_H


#if HAVE_ERR_H
# include <err.h>
#else
# define err(rc,msg,...) do { perror(msg); exit(rc); } while (0)
# define errx(rc,msg,...) do { puts(msg); exit(rc); } while (0)
#endif

#define die(str)   do { \
    fprintf(stderr, "%s(): %s: %s\n", __func__,str, strerror(errno));\
    abort();\
} while (0)

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#ifndef _WIN32
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/event.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <poll.h>
#include "../config.h"
#else
# include "../include/sys/event.h"
# include "../src/windows/platform.h"
#endif

struct test_context;

struct unit_test {
    const char *ut_name;
    int         ut_enabled;
    void      (*ut_func)(struct test_context *);
};

#define MAX_TESTS 50
struct test_context {
    struct unit_test tests[MAX_TESTS];
    char *cur_test_id;
    int iterations;
    int iteration;
    int kqfd;

    /* EVFILT_READ and EVFILT_WRITE */
    int client_fd;
    int server_fd;

    /* EVFILT_VNODE */
    int vnode_fd;
    char testfile[1024];
};

void test_evfilt_read(struct test_context *);
void test_evfilt_signal(struct test_context *);
void test_evfilt_vnode(struct test_context *);
void test_evfilt_timer(struct test_context *);
void test_evfilt_proc(struct test_context *);
#ifdef EVFILT_USER
void test_evfilt_user(struct test_context *);
#endif

#define test(f,ctx,...) do {                                            \
    assert(ctx != NULL); \
    test_begin(ctx, "test_"#f"()\t"__VA_ARGS__); \
    test_##f(ctx); \
    test_end(ctx); \
} while (/*CONSTCOND*/0)

extern const char * kevent_to_str(struct kevent *);
void kevent_get(struct kevent *, int);
void kevent_get_hires(struct kevent *, int);
void kevent_update(int kqfd, struct kevent *kev);

#define kevent_cmp(a,b) _kevent_cmp(a,b, __FILE__, __LINE__)
void _kevent_cmp(struct kevent *, struct kevent *, const char *, int);

void
kevent_add(int kqfd, struct kevent *kev, 
        uintptr_t ident,
        short     filter,
        u_short   flags,
        u_int     fflags,
        intptr_t  data,
        void      *udata);

/* DEPRECATED: */
#define KEV_CMP(kev,_ident,_filter,_flags) do {                 \
    if (kev.ident != (_ident) ||                                \
            kev.filter != (_filter) ||                          \
            kev.flags != (_flags)) \
        err(1, "kevent mismatch: got [%d,%d,%d] but expecting [%d,%d,%d]", \
                (int)_ident, (int)_filter, (int)_flags,\
                (int)kev.ident, kev.filter, kev.flags);\
} while (0);

/* Checks if any events are pending, which is an error. */
#define test_no_kevents(a) _test_no_kevents(a, __FILE__, __LINE__)
void _test_no_kevents(int, const char *, int);

/* From test.c */
void    test_begin(struct test_context *, const char *);
void    test_end(struct test_context *);
void    test_atexit(void);
void    testing_begin(void);
void    testing_end(void);
int     testing_make_uid(void);

#endif  /* _COMMON_H */
