/*
 * Copyright (c) 2016-2018 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
/*-
 * Copyright (c) 1999 Michael Smith <msmith@freebsd.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#ifndef _SYS_EVENTHANDLER_H_
#define _SYS_EVENTHANDLER_H_

#include <kern/locks.h>
#include <sys/queue.h>
#include <sys/cdefs.h>
#include <sys/syslog.h>
#include <uuid/uuid.h>

extern int evh_debug;
extern lck_grp_t        *el_lock_grp;
extern lck_attr_t       *el_lock_attr;
extern struct eventhandler_entry_arg eventhandler_entry_dummy_arg;

struct eventhandler_lists_ctxt {
	TAILQ_HEAD(, eventhandler_list)  eventhandler_lists;
	int eventhandler_lists_initted;
	decl_lck_mtx_data(, eventhandler_mutex);
};

struct eventhandler_entry_arg {
	uuid_t ee_fm_uuid;      /* Flow manager UUID */
	uuid_t ee_fr_uuid;      /* Flow route UUID */
};

struct eventhandler_entry {
	TAILQ_ENTRY(eventhandler_entry) ee_link;
	int                             ee_priority;
#define EHE_DEAD_PRIORITY       (-1)
	struct eventhandler_entry_arg   ee_arg;
};

#define EVENTHANDLER_MAX_NAME   32

struct eventhandler_list {
	char                            el_name[EVENTHANDLER_MAX_NAME];
	int                             el_flags;
#define EHL_INITTED     (1<<0)
	u_int                           el_runcount;
	decl_lck_mtx_data(, el_lock);
	TAILQ_ENTRY(eventhandler_list)  el_link;
	TAILQ_HEAD(, eventhandler_entry) el_entries;
};

typedef struct eventhandler_entry       *eventhandler_tag;

#define EHL_LOCK_INIT(p)        lck_mtx_init(&(p)->el_lock, el_lock_grp, el_lock_attr)
#define EHL_LOCK(p)             lck_mtx_lock(&(p)->el_lock)
#define EHL_LOCK_SPIN(p)        lck_mtx_lock_spin(&(p)->el_lock)
#define EHL_LOCK_CONVERT(p)     lck_mtx_convert_spin(&(p)->el_lock)
#define EHL_UNLOCK(p)           lck_mtx_unlock(&(p)->el_lock)
#define EHL_LOCK_ASSERT(p, x)   LCK_MTX_ASSERT(&(p)->el_lock, x)
#define EHL_LOCK_DESTROY(p)     lck_mtx_destroy(&(p)->el_lock, el_lock_grp)

#define evhlog(x)       do { if (evh_debug >= 1) log x; } while (0)

/*
 * Macro to invoke the handlers for a given event.
 */
#define _EVENTHANDLER_INVOKE(name, list, ...) do {                      \
	struct eventhandler_entry *_ep;                                 \
	struct eventhandler_entry_ ## name *_t;                         \
                                                                        \
	VERIFY((list)->el_flags & EHL_INITTED);                         \
	EHL_LOCK_ASSERT((list), LCK_MTX_ASSERT_OWNED);                  \
	(list)->el_runcount++;                                          \
	VERIFY((list)->el_runcount > 0);                                \
	evhlog((LOG_DEBUG, "eventhandler_invoke(\"" __STRING(name) "\")"));     \
	TAILQ_FOREACH(_ep, &((list)->el_entries), ee_link) {            \
	        if (_ep->ee_priority != EHE_DEAD_PRIORITY) {            \
	                EHL_UNLOCK((list));                             \
	                _t = (struct eventhandler_entry_ ## name *)_ep; \
	                evhlog((LOG_DEBUG, "eventhandler_invoke: executing %p", \
	                    VM_KERNEL_UNSLIDE((void *)_t->eh_func)));   \
	                _t->eh_func(_ep->ee_arg , ## __VA_ARGS__);      \
	                EHL_LOCK_SPIN((list));                          \
	        }                                                       \
	}                                                               \
	VERIFY((list)->el_runcount > 0);                                \
	(list)->el_runcount--;                                          \
	if ((list)->el_runcount == 0) {                                 \
	        EHL_LOCK_CONVERT((list));                               \
	        eventhandler_prune_list(list);                          \
	}                                                               \
	EHL_UNLOCK((list));                                             \
} while (0)

/*
 * Slow handlers are entirely dynamic; lists are created
 * when entries are added to them, and thus have no concept of "owner",
 *
 * Slow handlers need to be declared, but do not need to be defined. The
 * declaration must be in scope wherever the handler is to be invoked.
 */
#define EVENTHANDLER_DECLARE(name, type)                                \
struct eventhandler_entry_ ## name                                      \
{                                                                       \
	struct eventhandler_entry	ee;                             \
	type				eh_func;                        \
};                                                                      \
struct __hack

/*
 * XXX EVENTHANDLER_DEFINE by itself doesn't do much on XNU
 * All it does is that it declares the static eventhandler_tag
 * and defines an init routine that still needs to called to put the
 * event and callback on the list.
 */
#define EVENTHANDLER_DEFINE(evthdlr_ref, name, func, arg, priority)                     \
	static eventhandler_tag name ## _tag;                           \
	static void name ## _evh_init(void *ctx)                        \
	{                                                               \
	        name ## _tag = EVENTHANDLER_REGISTER(evthdlr_ref, name, func, ctx,      \
	            priority);                                          \
	}                                                               \
	SYSINIT(name ## _evh_init, SI_SUB_CONFIGURE, SI_ORDER_ANY,      \
	    name ## _evh_init, arg);                                    \
	struct __hack

#define EVENTHANDLER_INVOKE(evthdlr_ref, name, ...)                                     \
do {                                                                    \
	struct eventhandler_list *_el;                                  \
                                                                        \
	if ((_el = eventhandler_find_list(evthdlr_ref, #name)) != NULL)                 \
	        _EVENTHANDLER_INVOKE(name, _el , ## __VA_ARGS__);       \
} while (0)

#define EVENTHANDLER_REGISTER(evthdlr_ref, name, func, arg, priority)           \
	eventhandler_register(evthdlr_ref, NULL, #name, func, arg, priority)

#define EVENTHANDLER_DEREGISTER(evthdlr_ref, name, tag)                                 \
do {                                                                    \
	struct eventhandler_list *_el;                                  \
                                                                        \
	if ((_el = eventhandler_find_list(evthdlr_ref, #name)) != NULL)         \
	        eventhandler_deregister(_el, tag);                      \
} while(0)

void eventhandler_init(void);
extern void eventhandler_reap_caches(boolean_t);
eventhandler_tag eventhandler_register(struct eventhandler_lists_ctxt *evthdlr_lists_ctxt,
    struct eventhandler_list *list, const char *name, void *func, struct eventhandler_entry_arg arg, int priority);
void eventhandler_deregister(struct eventhandler_list *list,
    eventhandler_tag tag);
struct eventhandler_list *eventhandler_find_list(
	struct eventhandler_lists_ctxt *evthdlr_lists_ctxt, const char *name);
void eventhandler_prune_list(struct eventhandler_list *list);
void eventhandler_lists_ctxt_init(struct eventhandler_lists_ctxt *evthdlr_lists_ctxt);
void eventhandler_lists_ctxt_destroy(struct eventhandler_lists_ctxt *evthdlr_lists_ctxt);

/* Generic priority levels */
#define EVENTHANDLER_PRI_FIRST  0
#define EVENTHANDLER_PRI_ANY    10000
#define EVENTHANDLER_PRI_LAST   20000

#endif /* _SYS_EVENTHANDLER_H_ */
