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

extern int kqfd;

/* Checks if any events are pending, which is an error. */
void 
_test_no_kevents(int kqfd, const char *file, int line)
{
    int nfds;
    struct timespec timeo;
    struct kevent kev;

    memset(&timeo, 0, sizeof(timeo));
    nfds = kevent(kqfd, NULL, 0, &kev, 1, &timeo);
    if (nfds < 0)
        err(1, "kevent(2)");
    if (nfds > 0) {
        printf("\n[%s:%d]: Unexpected event:", file, line);
        err(1, kevent_to_str(&kev));
    }
}

/* Retrieve a single kevent */
void
kevent_get(struct kevent *kev, int kqfd)
{
    struct kevent buf;
    int nfds;

    if (kev == NULL)
       kev = &buf;

    nfds = kevent(kqfd, NULL, 0, kev, 1, NULL);
    if (nfds < 1)
        err(1, "kevent(2)");
}

/* In Linux, a kevent() call with less than 1ms resolution
   will perform a pselect() call to obtain the higer resolution.
   This test exercises that codepath.
 */
void
kevent_get_hires(struct kevent *kev, int kqfd)
{
    int nfds;
    struct timespec timeo;

    timeo.tv_sec = 0;
    timeo.tv_nsec = 500000;
    nfds = kevent(kqfd, NULL, 0, kev, 1, &timeo);
    if (nfds < 1)
        die("kevent(2)");
}

char *
kevent_fflags_dump(struct kevent *kev)
{
    char *buf;

#define KEVFFL_DUMP(attrib) \
    if (kev->fflags & attrib) \
	strncat(buf, #attrib" ", 64);

    if ((buf = calloc(1, 1024)) == NULL)
	abort();

    /* Not every filter has meaningful fflags */
    if (kev->filter != EVFILT_VNODE) {
    	snprintf(buf, 1024, "fflags = %d", kev->fflags);
	return (buf);
    }

    snprintf(buf, 1024, "fflags = %d (", kev->fflags);
    KEVFFL_DUMP(NOTE_DELETE);
    KEVFFL_DUMP(NOTE_WRITE);
    KEVFFL_DUMP(NOTE_EXTEND);
#if HAVE_NOTE_TRUNCATE
    KEVFFL_DUMP(NOTE_TRUNCATE);
#endif
    KEVFFL_DUMP(NOTE_ATTRIB);
    KEVFFL_DUMP(NOTE_LINK);
    KEVFFL_DUMP(NOTE_RENAME);
#if HAVE_NOTE_REVOKE
    KEVFFL_DUMP(NOTE_REVOKE);
#endif
    buf[strlen(buf) - 1] = ')';

    return (buf);
}

char *
kevent_flags_dump(struct kevent *kev)
{
    char *buf;

#define KEVFL_DUMP(attrib) \
    if (kev->flags & attrib) \
	strncat(buf, #attrib" ", 64);

    if ((buf = calloc(1, 1024)) == NULL)
	abort();

    snprintf(buf, 1024, "flags = %d (", kev->flags);
    KEVFL_DUMP(EV_ADD);
    KEVFL_DUMP(EV_ENABLE);
    KEVFL_DUMP(EV_DISABLE);
    KEVFL_DUMP(EV_DELETE);
    KEVFL_DUMP(EV_ONESHOT);
    KEVFL_DUMP(EV_CLEAR);
    KEVFL_DUMP(EV_EOF);
    KEVFL_DUMP(EV_ERROR);
#ifdef EV_DISPATCH
    KEVFL_DUMP(EV_DISPATCH);
#endif
#ifdef EV_RECEIPT
    KEVFL_DUMP(EV_RECEIPT);
#endif
    buf[strlen(buf) - 1] = ')';

    return (buf);
}

/* TODO - backport changes from src/common/kevent.c kevent_dump() */
const char *
kevent_to_str(struct kevent *kev)
{
    char buf[512];

    snprintf(&buf[0], sizeof(buf), 
            "[ident=%d, filter=%d, %s, %s, data=%d, udata=%p]",
            (u_int) kev->ident,
            kev->filter,
            kevent_flags_dump(kev),
            kevent_fflags_dump(kev),
            (int) kev->data,
            kev->udata);

    return (strdup(buf));
}

void
kevent_update(int kqfd, struct kevent *kev)
{
    if (kevent(kqfd, kev, 1, NULL, 0, NULL) < 0) {
        printf("Unable to add the following kevent:\n%s\n",
                kevent_to_str(kev));
        die("kevent");
    }
}

void
kevent_add(int kqfd, struct kevent *kev, 
        uintptr_t ident,
        short     filter,
        u_short   flags,
        u_int     fflags,
        intptr_t  data,
        void      *udata)
{
    EV_SET(kev, ident, filter, flags, fflags, data, NULL);    
    if (kevent(kqfd, kev, 1, NULL, 0, NULL) < 0) {
        printf("Unable to add the following kevent:\n%s\n",
                kevent_to_str(kev));
        die("kevent");
    }
}

void
_kevent_cmp(struct kevent *k1, struct kevent *k2, const char *file, int line)
{
/* XXX-
   Workaround for inconsistent implementation of kevent(2) 
 */
#if defined (__FreeBSD_kernel__) || defined (__FreeBSD__) 
    if (k1->flags & EV_ADD)
        k2->flags |= EV_ADD;
#endif
    if (memcmp(k1, k2, sizeof(*k1)) != 0) {
        printf("[%s:%d]: kevent_cmp() failed:\n  expected %s\n  but got  %s\n", 
              file, line, kevent_to_str(k1), kevent_to_str(k2));
        abort();
    }
}
