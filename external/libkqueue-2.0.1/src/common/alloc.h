/*
 * Copyright (c) 2011 Mark Heily <mark@heily.com>
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


/*
 * A simple fixed-size memory allocator.
 *
 * Each translation unit in a program can include this header and
 * have access to it's own private memory allocator. This can be useful
 * for improving the performance of programs which frequently allocate 
 * and deallocate objects with a fixed size.
 *
 * The allocator must be initialized by calling mem_init(). This
 * function takes two arguments: the object size, and the maximum
 * number of objects in the cache.
 *
 * The functions mem_alloc() and mem_free() have similar semantics
 * to the traditional malloc() and free() calls. The main difference
 * is that mem_alloc() does not allow you to specify a specific size.
 *
 */

#include <stdlib.h>

#ifndef _WIN32
# include <unistd.h>
#endif

static __thread struct {
    void  **ac_cache;       /* An array of reusable memory objects */
    size_t  ac_count;       /* The number of objects in the cache */
    size_t  ac_max;         /* The maximum number of cached objects */
    size_t  ac_size;        /* The size, in bytes, of each object */
} _ma;

static inline int
mem_init(size_t objsize, size_t cachesize)
{
    _ma.ac_size = objsize;
    _ma.ac_cache = malloc(cachesize * sizeof(void *));
    return (_ma.ac_cache == NULL ? -1 : 0);
}

static inline void *
mem_alloc(void)
{
    if (_ma.ac_count > 0) 
        return (_ma.ac_cache[_ma.ac_count--]);
    else 
        return (malloc(_ma.ac_size));
}

static inline void *
mem_calloc(void)
{
    void *p;

    p = mem_alloc();
    if (p != NULL)
        memset(p, 0, _ma.ac_size);
    return (p);
}

static inline void
mem_free(void *ptr)
{
    if (_ma.ac_count < _ma.ac_max)
        _ma.ac_cache[_ma.ac_count++] = ptr;
    else 
        free(ptr);
}
