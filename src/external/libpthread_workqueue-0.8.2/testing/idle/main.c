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

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread_workqueue.h>

void f(void *arg)
{
    long x = (long) arg;

    printf("worker %ld running\n", x);
    sleep(1);
    printf("worker %ld finished\n", x);
}

void run_idle_test(pthread_workqueue_t wq)
{
    long i;
    int rv;

    for (i = 0; i < 100; i++) {
        rv = pthread_workqueue_additem_np(wq, f, (void *) i, NULL, NULL);
        if (rv != 0) abort();
    }

    sleep(2);

    int rounds = 0;
    for (;;) {
        unsigned long idle = pthread_workqueue_peek_np("combined_idle");
        unsigned long norml_idle = pthread_workqueue_peek_np("idle");
        unsigned long ocomm_idle = pthread_workqueue_peek_np("ocomm_idle");
        printf("idle = %lu (overcommit = %lu non-overcommit = %lu)\n", 
                idle, ocomm_idle, norml_idle);
        if (idle == 0 || (norml_idle == 1 && ocomm_idle == 0))
            break;

        sleep(1);
        if (rounds++ > 240) {
            printf("\n*** ERROR: idle threads were not reaped properly\n");
            exit(1);
        }
    }
}

/*
 * Enqueue a large number of short-lived workitems, to allow observation
 * of how idle threads are terminated.
 */
int main(int argc, char *argv[]) 
{
    pthread_workqueue_t wq;
    pthread_workqueue_t ocwq;
    pthread_workqueue_attr_t attr;
    pthread_workqueue_attr_t ocattr;
    int i, rounds;
    int rv;

    if (argc == 2) 
        rounds = atoi(argv[1]);
    else
        rounds = 1;

    pthread_workqueue_attr_init_np(&attr);
    pthread_workqueue_attr_setovercommit_np(&attr, 0);
    rv = pthread_workqueue_create_np(&wq, &attr);
    if (rv != 0) abort();

    pthread_workqueue_attr_init_np(&ocattr);
    pthread_workqueue_attr_setovercommit_np(&ocattr, 1);
    rv = pthread_workqueue_create_np(&ocwq, &ocattr);
    if (rv != 0) abort();

    for (i = 0; i < rounds; i++) {
        run_idle_test(wq);
        run_idle_test(ocwq);
    }
    printf("\n---\nOK: all excess idle threads have been terminated after %d rounds.\n", rounds);
    exit(0);
}
