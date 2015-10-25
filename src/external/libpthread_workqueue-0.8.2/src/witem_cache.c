/*
 * Copyright (c) 2011, Joakim Johansson <jocke@tbricks.com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "private.h"


/* no witem cache */

#if (WITEM_CACHE_TYPE == 1)

int
witem_cache_init(void)
{
   return (0);
}

struct work *
witem_alloc(void (*func)(void *), void *func_arg)
{
	struct work *witem;
    
	while (!(witem = fastpath(malloc(ROUND_UP_TO_CACHELINE_SIZE(sizeof(*witem)))))) {
		sleep(1);
	}

    witem->gencount = 0;
    witem->flags = 0;
    witem->item_entry.stqe_next = 0;
    witem->func = func;
    witem->func_arg = func_arg;

	return witem;
}

void 
witem_free(struct work *wi)
{
    dbg_printf("freed work item %p", wi);
    free(wi);
}

void
witem_cache_cleanup(void *value)
{
    (void) value;
}

/* libumem based object cache */

#elif (WITEM_CACHE_TYPE == 2)

#include <umem.h>

static umem_cache_t  *witem_cache;

int
witem_cache_init(void)
{
    witem_cache = umem_cache_create((char *) "witem_cache",   
                                    sizeof(struct work),   
                                    CACHELINE_SIZE,  
                                    NULL,
                                    NULL, 
                                    NULL, 
                                    NULL, 
                                    NULL, 
                                    0);
    return (0);
}

struct work *
witem_alloc(void (*func)(void *), void *func_arg)
{
	struct work *witem;
    
	while (!(witem = fastpath(umem_cache_alloc(witem_cache, UMEM_DEFAULT)))) {
		sleep(1);
	}
    
    witem->gencount = 0;
    witem->flags = 0;
    witem->item_entry.stqe_next = 0;
    witem->func = func;
    witem->func_arg = func_arg;

	return witem;
}

void 
witem_free(struct work *wi)
{
    umem_cache_free(witem_cache, wi);
    return;
}

void
witem_cache_cleanup(void *value)
{
    void * p;
    p = value;
}

/* TSD based cacheing per thread */

#elif (WITEM_CACHE_TYPE == 3)

pthread_key_t witem_cache_key;

int
witem_cache_init(void)
{
    pthread_key_create(&witem_cache_key, witem_cache_cleanup);
    return (0);
}

static struct work *
witem_alloc_from_heap(void)
{
	struct work *witem;
    
	while (!(witem = fastpath(malloc(ROUND_UP_TO_CACHELINE_SIZE(sizeof(*witem)))))) {
		sleep(1);
	}
    
    witem->gencount = 0;
    witem->flags = 0;
    witem->item_entry.stqe_next = 0;

	return witem;
}

struct work *
witem_alloc(void (*func)(void *), void *func_arg)
{
    struct work *witem = fastpath(pthread_getspecific(witem_cache_key));
	if (witem) 
    {
		pthread_setspecific(witem_cache_key, witem->wi_next);
	}
    else
    {
        witem = witem_alloc_from_heap();
    }

    witem->func = func;
    witem->func_arg = func_arg;
    
	return witem;
}

void 
witem_free(struct work *witem)
{
	struct work *prev_wi = pthread_getspecific(witem_cache_key);

	witem->wi_next = prev_wi;
    
    // We need to initialize here also...
    witem->gencount = 0;
    witem->flags = 0;
    witem->item_entry.stqe_next = 0;
    witem->func = NULL;
    witem->func_arg = NULL;

	pthread_setspecific(witem_cache_key, witem);
}

void
witem_cache_cleanup(void *value)
{
	struct work *wi, *next_wi = value;
    
	while ((wi = next_wi)) {
		next_wi = wi->wi_next;
		free(wi);
	}
}
#else

#error Invalid witem cache type specified

#endif
