/*-
 * Copyright (c) 2010, Mark Heily <mark@heily.com>
 * Copyright (c) 2009, Stacey Son <sson@freebsd.org>
 * Copyright (c) 2000-2008, Apple Inc.
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

#ifndef _PTHREAD_WORKQUEUE_H
#define _PTHREAD_WORKQUEUE_H

#if _WIN32
#define _PWQ_EXPORT __declspec(dllexport)
#else
#define _PWQ_EXPORT
#endif

typedef struct _pthread_workqueue * pthread_workqueue_t;
typedef void *                      pthread_workitem_handle_t;

/* Pad size to 64 bytes. */
typedef struct {
   unsigned int sig;
   int queueprio;
   int overcommit;
   unsigned int pad[13];
} pthread_workqueue_attr_t;

/* Work queue priority attributes. */
#define WORKQ_HIGH_PRIOQUEUE       0
#define WORKQ_DEFAULT_PRIOQUEUE    1
#define WORKQ_LOW_PRIOQUEUE        2

#if defined(__cplusplus)
	extern "C" {
#endif
		
int _PWQ_EXPORT pthread_workqueue_create_np(pthread_workqueue_t * workqp,
               const pthread_workqueue_attr_t * attr);

int _PWQ_EXPORT pthread_workqueue_additem_np(pthread_workqueue_t workq,
            void (*workitem_func)(void *), void * workitem_arg,
            pthread_workitem_handle_t * itemhandlep, unsigned int *gencountp);

int _PWQ_EXPORT pthread_workqueue_attr_init_np(pthread_workqueue_attr_t * attrp);

int _PWQ_EXPORT pthread_workqueue_attr_destroy_np(pthread_workqueue_attr_t * attr);

int _PWQ_EXPORT pthread_workqueue_attr_setqueuepriority_np(pthread_workqueue_attr_t * attr,
                   int qprio);

int _PWQ_EXPORT pthread_workqueue_attr_getovercommit_np(
               const pthread_workqueue_attr_t * attr, int * ocommp);

int _PWQ_EXPORT pthread_workqueue_attr_setovercommit_np(pthread_workqueue_attr_t * attr,
                   int ocomm);

int _PWQ_EXPORT pthread_workqueue_requestconcurrency_np(pthread_workqueue_t workq,
                   int queue, int request_concurrency);

int _PWQ_EXPORT pthread_workqueue_getovercommit_np(pthread_workqueue_t workq,
                   unsigned int *ocommp);

void _PWQ_EXPORT pthread_workqueue_main_np(void);

#ifdef MAKE_STATIC
int _PWQ_EXPORT pthread_workqueue_init_np(void);
#endif

/* NOTE: this is not part of the Darwin API */
unsigned long _PWQ_EXPORT pthread_workqueue_peek_np(const char *);

#if defined(__cplusplus)
	}
#endif
			
#undef _PWQ_EXPORT

#endif  /* _PTHREAD_WORKQUEUE_H */
