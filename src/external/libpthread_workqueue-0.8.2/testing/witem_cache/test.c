/*-
 * Copyright (c) 2011, Mark Heily <mark@heily.com>
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

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <time.h>

#include "config.h"
#include "src/private.h"

#if HAVE_ERR_H
# include <err.h>
#else
# define err(rc,msg,...) do { perror(msg); exit(rc); } while (0)
# define errx(rc,msg,...) do { puts(msg); exit(rc); } while (0)
#endif

#include "pthread_workqueue.h"

pthread_workqueue_t wq;

void additem(void (*func)(void *), 
             void * arg)
{
    int rv;
    
    rv = pthread_workqueue_additem_np(wq, *func, arg, NULL, NULL);
    if (rv != 0)
        errx(1, "unable to add item: %s", strerror(rv));
}


void
feedback(void *arg)
{
    int *i = (int *) arg;
    struct timespec tv;

	(*i)--;
    if ((*i) <= 0) {
        puts("All tests completed.\n");
        exit(0);
    } else {
        additem(feedback, arg);
        tv.tv_sec = 0;
        tv.tv_nsec = 750;
        nanosleep(&tv, NULL);
    }
}

int main() {
    int i = 10000;

    pthread_workqueue_create_np(&wq, NULL);
    additem(feedback, &i);
    pause();
}
