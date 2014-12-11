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
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>

#include "private.h"

int DEBUG_KQUEUE = 0;
char *KQUEUE_DEBUG_IDENT = "KQ";

#ifdef _WIN32
static LONG kq_init_begin = 0;
static int kq_init_complete = 0;
#else
pthread_mutex_t kq_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_once_t kq_is_initialized = PTHREAD_ONCE_INIT;
#endif

static unsigned int
get_fd_limit(void)
{
#ifdef _WIN32
    /* actually windows should be able to hold
       way more, as they use HANDLEs for everything.
       Still this number should still be sufficient for
       the provided number of kqueue fds.
       */
    return 65536;
#else
    struct rlimit rlim;
    
    if (getrlimit(RLIMIT_NOFILE, &rlim) < 0) {
        dbg_perror("getrlimit(2)");
        return (65536);
    } else {
        return (rlim.rlim_max);
    }
#endif
}

static struct map *kqmap;

void
libkqueue_init(void)
{
#ifdef NDEBUG
    DEBUG_KQUEUE = 0;
#else
    char *s = getenv("KQUEUE_DEBUG");
    if (s != NULL && strlen(s) > 0) {
        DEBUG_KQUEUE = 1;

#ifdef _WIN32
    /* Initialize the Winsock library */
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) 
        abort();
#endif

# if defined(_WIN32) && !defined(__GNUC__)
	/* Enable heap surveillance */
	{
		int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
		tmpFlag |= _CRTDBG_CHECK_ALWAYS_DF;
		_CrtSetDbgFlag(tmpFlag);
	}
# endif /* _WIN32 */
    }
#endif

   kqmap = map_new(get_fd_limit()); // INT_MAX
   if (kqmap == NULL)
       abort(); 
   if (knote_init() < 0)
       abort();
   dbg_puts("library initialization complete");
#ifdef _WIN32
   kq_init_complete = 1;
#endif
}

#if DEADWOOD
static int
kqueue_cmp(struct kqueue *a, struct kqueue *b)
{
    return memcmp(&a->kq_id, &b->kq_id, sizeof(int)); 
}

/* Must hold the kqtree_mtx when calling this */
void
kqueue_free(struct kqueue *kq)
{
    RB_REMOVE(kqt, &kqtree, kq);
    filter_unregister_all(kq);
    kqops.kqueue_free(kq);
    free(kq);
}

#endif

struct kqueue *
kqueue_lookup(int kq)
{
    return ((struct kqueue *) map_lookup(kqmap, kq));
}

int VISIBLE
kqueue(void)
{
	struct kqueue *kq;
    struct kqueue *tmp;

#ifdef _WIN32
    if (InterlockedCompareExchange(&kq_init_begin, 0, 1) == 0) {
        libkqueue_init();
    } else {
        while (kq_init_complete == 0) {
            sleep(1);
        }
    }
#else
    (void) pthread_mutex_lock(&kq_mtx);
    (void) pthread_once(&kq_is_initialized, libkqueue_init);
    (void) pthread_mutex_unlock(&kq_mtx);
#endif

    kq = calloc(1, sizeof(*kq));
    if (kq == NULL)
        return (-1);

	tracing_mutex_init(&kq->kq_mtx, NULL);

    if (kqops.kqueue_init(kq) < 0) {
        free(kq);
        return (-1);
    }

    dbg_printf("created kqueue, fd=%d", kq->kq_id);

    tmp = map_delete(kqmap, kq->kq_id);
    if (tmp != NULL) {
        dbg_puts("FIXME -- memory leak here");
        // TODO: kqops.kqueue_free(tmp), or (better yet) decrease it's refcount
    }
    if (map_insert(kqmap, kq->kq_id, kq) < 0) {
        dbg_puts("map insertion failed");
        kqops.kqueue_free(kq);
        return (-1);
    }

    return (kq->kq_id);
}
