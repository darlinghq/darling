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

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "private.h"

#include "alloc.h"

int
knote_init(void)
{
    return 0;
//    return (mem_init(sizeof(struct knote), 1024));
}

static int
knote_cmp(struct knote *a, struct knote *b)
{
    return memcmp(&a->kev.ident, &b->kev.ident, sizeof(a->kev.ident)); 
}

RB_GENERATE(knt, knote, kn_entries, knote_cmp)

struct knote *
knote_new(void)
{
	struct knote *res;

    res = calloc(1, sizeof(struct knote));
	if (res == NULL)
        return (NULL);

    res->kn_ref = 1;

    return (res);
}

void
knote_release(struct knote *kn)
{
    assert (kn->kn_ref > 0);

	if (atomic_dec(&kn->kn_ref) == 0) {
        if (kn->kn_flags & KNFL_KNOTE_DELETED) {
            dbg_printf("freeing knote at %p", kn);
            free(kn);
        } else {
            dbg_puts("this should never happen");
        }
    } else {
        dbg_printf("decrementing refcount of knote %p rc=%d", kn, kn->kn_ref);
    }
}

void
knote_insert(struct filter *filt, struct knote *kn)
{
    pthread_rwlock_wrlock(&filt->kf_knote_mtx);
    RB_INSERT(knt, &filt->kf_knote, kn);
    pthread_rwlock_unlock(&filt->kf_knote_mtx);
}

int
knote_delete(struct filter *filt, struct knote *kn)
{
    struct knote query;
    struct knote *tmp;

    if (kn->kn_flags & KNFL_KNOTE_DELETED) {
        dbg_puts("ERROR: double deletion detected");
        return (-1);
    }

    /*
     * Verify that the knote wasn't removed by another
     * thread before we acquired the knotelist lock.
     */
    query.kev.ident = kn->kev.ident;
    pthread_rwlock_wrlock(&filt->kf_knote_mtx);
    tmp = RB_FIND(knt, &filt->kf_knote, &query);
    if (tmp == kn) {
        RB_REMOVE(knt, &filt->kf_knote, kn);
    }
    pthread_rwlock_unlock(&filt->kf_knote_mtx);

    filt->kn_delete(filt, kn); //XXX-FIXME check return value

    kn->kn_flags |= KNFL_KNOTE_DELETED;

    knote_release(kn);

    return (0);
}

struct knote *
knote_lookup(struct filter *filt, uintptr_t ident)
{
    struct knote query;
    struct knote *ent = NULL;

    query.kev.ident = ident;

    pthread_rwlock_rdlock(&filt->kf_knote_mtx);
    ent = RB_FIND(knt, &filt->kf_knote, &query);
    pthread_rwlock_unlock(&filt->kf_knote_mtx);

#ifdef __x86_64__
    dbg_printf("id=%lu ent=%p", ident, ent);
#else
    dbg_printf("id=%u ent=%p", ident, ent);
#endif

    return (ent);
}
    
#if DEADWOOD
struct knote *
knote_get_by_data(struct filter *filt, intptr_t data)
{
    struct knote *kn;

    pthread_rwlock_rdlock(&filt->kf_knote_mtx);
    RB_FOREACH(kn, knt, &filt->kf_knote) {
        if (data == kn->kev.data) 
            break;
    }
    if (kn != NULL) {
        knote_retain(kn);
    }
    pthread_rwlock_unlock(&filt->kf_knote_mtx);

    return (kn);
}
#endif

int
knote_disable(struct filter *filt, struct knote *kn)
{
    assert(!(kn->kev.flags & EV_DISABLE));

    filt->kn_disable(filt, kn); //TODO: Error checking
    KNOTE_DISABLE(kn);
    return (0);
}

//TODO: knote_enable()

