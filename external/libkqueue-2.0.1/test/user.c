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

static void
test_kevent_user_add_and_delete(struct test_context *ctx)
{
    struct kevent kev;

    kevent_add(ctx->kqfd, &kev, 1, EVFILT_USER, EV_ADD, 0, 0, NULL);
    test_no_kevents(ctx->kqfd);

    kevent_add(ctx->kqfd, &kev, 1, EVFILT_USER, EV_DELETE, 0, 0, NULL);
    test_no_kevents(ctx->kqfd);
}

static void
test_kevent_user_get(struct test_context *ctx)
{
    struct kevent kev, ret;

    test_no_kevents(ctx->kqfd);

    /* Add the event, and then trigger it */
    kevent_add(ctx->kqfd, &kev, 1, EVFILT_USER, EV_ADD | EV_CLEAR, 0, 0, NULL);    
    kevent_add(ctx->kqfd, &kev, 1, EVFILT_USER, 0, NOTE_TRIGGER, 0, NULL);    

    kev.fflags &= ~NOTE_FFCTRLMASK;
    kev.fflags &= ~NOTE_TRIGGER;
    kev.flags = EV_CLEAR;
    kevent_get(&ret, ctx->kqfd);
    kevent_cmp(&kev, &ret);

    test_no_kevents(ctx->kqfd);
}

static void
test_kevent_user_get_hires(struct test_context *ctx)
{
    struct kevent kev, ret;

    test_no_kevents(ctx->kqfd);

    /* Add the event, and then trigger it */
    kevent_add(ctx->kqfd, &kev, 1, EVFILT_USER, EV_ADD | EV_CLEAR, 0, 0, NULL);    
    kevent_add(ctx->kqfd, &kev, 1, EVFILT_USER, 0, NOTE_TRIGGER, 0, NULL);    

    kev.fflags &= ~NOTE_FFCTRLMASK;
    kev.fflags &= ~NOTE_TRIGGER;
    kev.flags = EV_CLEAR;
    kevent_get_hires(&ret, ctx->kqfd);
    kevent_cmp(&kev, &ret);

    test_no_kevents(ctx->kqfd);
}

static void
test_kevent_user_disable_and_enable(struct test_context *ctx)
{
    struct kevent kev, ret;

    test_no_kevents(ctx->kqfd);

    kevent_add(ctx->kqfd, &kev, 1, EVFILT_USER, EV_ADD, 0, 0, NULL); 
    kevent_add(ctx->kqfd, &kev, 1, EVFILT_USER, EV_DISABLE, 0, 0, NULL); 

    /* Trigger the event, but since it is disabled, nothing will happen. */
    kevent_add(ctx->kqfd, &kev, 1, EVFILT_USER, 0, NOTE_TRIGGER, 0, NULL); 
    test_no_kevents(ctx->kqfd);

    kevent_add(ctx->kqfd, &kev, 1, EVFILT_USER, EV_ENABLE, 0, 0, NULL); 
    kevent_add(ctx->kqfd, &kev, 1, EVFILT_USER, 0, NOTE_TRIGGER, 0, NULL); 

    kev.flags = EV_CLEAR;
    kev.fflags &= ~NOTE_FFCTRLMASK;
    kev.fflags &= ~NOTE_TRIGGER;
    kevent_get(&ret, ctx->kqfd);
    kevent_cmp(&kev, &ret);
}

static void
test_kevent_user_oneshot(struct test_context *ctx)
{
    struct kevent kev, ret;

    test_no_kevents(ctx->kqfd);

    kevent_add(ctx->kqfd, &kev, 2, EVFILT_USER, EV_ADD | EV_ONESHOT, 0, 0, NULL);

    puts("  -- event 1");
    kevent_add(ctx->kqfd, &kev, 2, EVFILT_USER, 0, NOTE_TRIGGER, 0, NULL);    

    kev.flags = EV_ONESHOT;
    kev.fflags &= ~NOTE_FFCTRLMASK;
    kev.fflags &= ~NOTE_TRIGGER;
    kevent_get(&ret, ctx->kqfd);
    kevent_cmp(&kev, &ret);

    test_no_kevents(ctx->kqfd);
}

#ifdef EV_DISPATCH
void
test_kevent_user_dispatch(struct test_context *ctx)
{
    struct kevent kev, ret;

    test_no_kevents(ctx->kqfd);

    /* Add the event, and then trigger it */
    kevent_add(ctx->kqfd, &kev, 1, EVFILT_USER, EV_ADD | EV_CLEAR | EV_DISPATCH, 0, 0, NULL);
    kevent_add(ctx->kqfd, &kev, 1, EVFILT_USER, 0, NOTE_TRIGGER, 0, NULL);

    /* Retrieve one event */
    kev.fflags &= ~NOTE_FFCTRLMASK;
    kev.fflags &= ~NOTE_TRIGGER;
    kev.flags = EV_CLEAR;
    kevent_get(&ret, ctx->kqfd);
    kevent_cmp(&kev, &ret);

    /* Confirm that the knote is disabled automatically */
    test_no_kevents(ctx->kqfd);

    /* Re-enable the kevent */
    /* FIXME- is EV_DISPATCH needed when rearming ? */
    kevent_add(ctx->kqfd, &kev, 1, EVFILT_USER, EV_ENABLE | EV_CLEAR | EV_DISPATCH, 0, 0, NULL);
    test_no_kevents(ctx->kqfd);

    /* Trigger the event */
    kevent_add(ctx->kqfd, &kev, 1, EVFILT_USER, 0, NOTE_TRIGGER, 0, NULL);
    kev.fflags &= ~NOTE_FFCTRLMASK;
    kev.fflags &= ~NOTE_TRIGGER;
    kev.flags = EV_CLEAR;
    kevent_get(&ret, ctx->kqfd);
    kevent_cmp(&kev, &ret);
    test_no_kevents(ctx->kqfd);

    /* Delete the watch */
    kevent_add(ctx->kqfd, &kev, 1, EVFILT_USER, EV_DELETE, 0, 0, NULL);
    test_no_kevents(ctx->kqfd);
}
#endif 	/* EV_DISPATCH */

void
test_evfilt_user(struct test_context *ctx)
{
    test(kevent_user_add_and_delete, ctx);
    test(kevent_user_get, ctx);
    test(kevent_user_get_hires, ctx);
    test(kevent_user_disable_and_enable, ctx);
    test(kevent_user_oneshot, ctx);
#ifdef EV_DISPATCH
    test(kevent_user_dispatch, ctx);
#endif
    /* TODO: try different fflags operations */
}
