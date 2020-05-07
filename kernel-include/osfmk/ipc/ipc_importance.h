/*
 * Copyright (c) 2013 Apple Computer, Inc. All rights reserved.
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
#ifndef _IPC_IPC_IMPORTANCE_H_
#define _IPC_IPC_IMPORTANCE_H_

#include <mach/mach_types.h>
#include <mach/mach_voucher_types.h>
#include <mach/boolean.h>
#include <ipc/ipc_types.h>
#include <ipc/ipc_voucher.h>

/*
 * IPC Importance - All definitions are MACH_KERNEL_PRIVATE
 */
#ifdef MACH_KERNEL_PRIVATE

#include <kern/locks.h>
#include <kern/simple_lock.h>

/*
 * IPC Importance Value Element
 *
 * This element represents a single task's (base) importance,
 * or in the case of inherited importance, the inheritance
 * linkage from the source to the destination task.  In the
 * inheritance case, this source can be a base importance or
 * another inherited importace.
 *
 * Whenever the task importance is adjusted, it walks the
 * list of IPC-related items it influences (ports and downstream
 * tasks doing work on its behalf) and makes adjustments to
 * their importance attributes accordingly.
 *
 */

struct ipc_importance_elem {
	uint32_t                                iie_bits;       /* type and refs */
	mach_voucher_attr_value_reference_t     iie_made;       /* references given to vouchers */
	queue_head_t                            iie_kmsgs;      /* list of kmsgs inheriting from this */
	uint32_t                                iie_externcnt;  /* number of externalized boosts */
	uint32_t                                iie_externdrop; /* number of those dropped already */
#define IIE_REF_DEBUG 0
#if IIE_REF_DEBUG
	uint32_t iie_refs_added;                        /* all refs added via all means */
	uint32_t iie_refs_dropped;                      /* all refs dropped via all means */
	uint32_t iie_kmsg_refs_added;                   /* all refs added by kmsgs taking a ref */
	uint32_t iie_kmsg_refs_inherited;               /* kmsg refs consumed by a new inherit */
	uint32_t iie_kmsg_refs_coalesced;               /* kmsg refs coalesced into an existing inherit */
	uint32_t iie_kmsg_refs_dropped;                 /* kmsg refs dropped by not accepting msg importance */
	uint32_t iie_task_refs_added;                   /* refs added by a task reference call */
	uint32_t iie_task_refs_added_inherit_from;      /* task references added by inherit from */
	uint32_t iie_task_refs_added_transition;        /* task references added by imp transition code */
	uint32_t iie_task_refs_self_added;              /* task refs added by self-boost */
	uint32_t iie_task_refs_inherited;               /* task refs consumed by a new inherit */
	uint32_t iie_task_refs_coalesced;               /* task refs coalesced into an existing inherit */
	uint32_t iie_task_refs_dropped;                 /* all refs dropped via all task means */
#endif
};

#define IIE_TYPE_MASK           0x80000000      /* Just the high bit for now */
#define IIE_TYPE_TASK           0x00000000      /* Element is a task element */
#define IIE_TYPE_INHERIT        0x80000000      /* Element inherits from a previous element */
#define IIE_TYPE(e)             ((e)->iie_bits & IIE_TYPE_MASK)

#define IIE_REFS_MASK           0x7FFFFFFF      /* Mask to extract references */
#define IIE_REFS_MAX            0x7FFFFFFF
#define IIE_REFS(e)             ((e)->iie_bits & IIE_REFS_MASK)

#define IIE_EXTERN(e)           ((e)->iie_externcnt - (e)->iie_externdrop)

#if !IIE_REF_DEBUG
#define ipc_importance_reference_internal(elem)         \
	(os_atomic_inc(&(elem)->iie_bits, relaxed) & IIE_REFS_MASK)

#define ipc_importance_release_internal(elem)           \
	(os_atomic_dec(&(elem)->iie_bits, relaxed) & IIE_REFS_MASK)
#endif

struct ipc_importance_task {
	struct ipc_importance_elem iit_elem;    /* common element parts */
	task_t                  iit_task;       /* task associated with */
	queue_head_t            iit_inherits;   /* list of inherit elems hung off this */
	queue_t                 iit_updateq;    /* queue chained on for task policy updates */
	queue_chain_t           iit_updates;    /* link on update chain */
	queue_chain_t           iit_props;      /* link on propagation chain */
	uint64_t                iit_updatetime; /* timestamp of our last policy update request */
	uint64_t                iit_transitions;/* total number of boost transitions (lifetime) */
	uint32_t                iit_assertcnt;  /* net number of boost assertions (internal, external and legacy) */
	uint32_t                iit_legacy_externcnt;  /* Legacy external boost count */
	uint32_t                iit_legacy_externdrop; /* Legacy external boost drop count */
	uint32_t                iit_receiver:1, /* the task can receive importance boost */
	    iit_denap:1,                        /* the task can be awaked from App Nap */
	    iit_donor:1,                        /* the task always sends boosts regardless of boost status */
	    iit_live_donor:1,                          /* the task temporarily sends boosts regardless of boost status */
	    iit_updatepolicy:1,                        /* enqueue for policy update at the end of propagation */
	    iit_reserved:3,                            /* reserved for future use */
	    iit_filelocks:24;                          /* number of file lock boosts */
#if DEVELOPMENT || DEBUG
	char                    iit_procname[20];      /* name of proc */
	uint32_t                iit_bsd_pid;           /* pid of proc creating this iit */
	queue_chain_t           iit_allocation;        /* link on global iit allocation chain */
#endif
};
#define iit_bits                iit_elem.iie_bits
#define iit_made                iit_elem.iie_made
#define iit_kmsgs               iit_elem.iie_kmsgs
#define iit_externcnt           iit_elem.iie_externcnt
#define iit_externdrop          iit_elem.iie_externdrop

#define IIT_REFS_MAX            IIE_REFS_MAX
#define IIT_REFS(t)             IIE_REFS(&(t)->iit_elem)
#define IIT_EXTERN(t)           IIE_EXTERN(&(t)->iit_elem)
#define IIT_LEGACY_EXTERN(t)    ((t)->iit_legacy_externcnt - (t)->iit_legacy_externdrop)

#if !IIE_REF_DEBUG
#define ipc_importance_task_reference_internal(task_imp)        \
	(ipc_importance_reference_internal(&(task_imp)->iit_elem))

#define ipc_importance_task_release_internal(task_imp)          \
	(assert(1 < IIT_REFS(task_imp)), ipc_importance_release_internal(&(task_imp)->iit_elem))
#endif

typedef int iit_update_type_t;
#define IIT_UPDATE_HOLD                 ((iit_update_type_t)1)
#define IIT_UPDATE_DROP                 ((iit_update_type_t)2)

struct ipc_importance_inherit {
	struct ipc_importance_elem      iii_elem;         /* common element partss */
	boolean_t                       iii_donating;     /* is this donating importance */
	uint32_t                        iii_depth;        /* nesting depth */
	ipc_importance_task_t           iii_to_task;      /* donating to */
	ipc_importance_elem_t           iii_from_elem;    /* other elem contributing */
	queue_chain_t                   iii_inheritance;  /* inherited from link */
};
#define iii_bits                        iii_elem.iie_bits
#define iii_made                        iii_elem.iie_made
#define iii_kmsgs                       iii_elem.iie_kmsgs
#define iii_externcnt                   iii_elem.iie_externcnt
#define iii_externdrop                  iii_elem.iie_externdrop
#define III_REFS_MAX                    IIE_REFS_MAX
#define III_REFS(i)                     IIE_REFS(&(i)->iii_elem)
#define III_EXTERN(i)                   IIE_EXTERN(&(i)->iii_elem)

#define III_DEPTH_RESET                 0x80000000
#define III_DEPTH_MASK                  0x000000FF
#define III_DEPTH(i)                    ((i)->iii_depth & III_DEPTH_MASK)
#define III_DEPTH_MAX                   32      /* maximum inherit->inherit chain depth */

#define ipc_importance_inherit_reference_internal(inherit)      \
	(ipc_importance_reference_internal(&(inherit)->iii_elem))

__BEGIN_DECLS

/* add a reference to an importance attribute */
extern void ipc_importance_reference(ipc_importance_elem_t elem);

/* release an importance attribute reference */
extern void ipc_importance_release(ipc_importance_elem_t elem);

/* retain a task importance attribute reference */
extern void ipc_importance_task_reference(ipc_importance_task_t task_elem);

/* release a task importance attribute reference */
extern void ipc_importance_task_release(ipc_importance_task_t task_imp);

/* reset the influence of the task on the importance */
extern void ipc_importance_reset(ipc_importance_task_t task_imp, boolean_t donor);

extern ipc_importance_task_t ipc_importance_for_task(task_t task, boolean_t made);
extern void ipc_importance_disconnect_task(task_t task);
extern ipc_importance_inherit_t ipc_importance_exec_switch_task(task_t old_task, task_t new_task);

extern boolean_t ipc_importance_task_is_donor(ipc_importance_task_t task_imp);
extern boolean_t ipc_importance_task_is_never_donor(ipc_importance_task_t task_imp);
extern boolean_t ipc_importance_task_is_marked_donor(ipc_importance_task_t task_imp);
extern boolean_t ipc_importance_task_is_marked_live_donor(ipc_importance_task_t task_imp);

extern void ipc_importance_task_mark_donor(ipc_importance_task_t task_imp, boolean_t donating);
extern void ipc_importance_task_mark_live_donor(ipc_importance_task_t task_imp, boolean_t live_donating);
extern void ipc_importance_task_update_live_donor(ipc_importance_task_t task_imp);

extern boolean_t ipc_importance_task_is_marked_receiver(ipc_importance_task_t task_imp);
extern void ipc_importance_task_mark_receiver(ipc_importance_task_t task_imp, boolean_t receiving);

extern boolean_t ipc_importance_task_is_denap_receiver(ipc_importance_task_t task_imp);
extern boolean_t ipc_importance_task_is_marked_denap_receiver(ipc_importance_task_t task_imp);
extern void ipc_importance_task_mark_denap_receiver(ipc_importance_task_t task_imp, boolean_t receiving);

extern boolean_t ipc_importance_task_is_any_receiver_type(ipc_importance_task_t task_imp);

extern kern_return_t ipc_importance_task_hold_internal_assertion(ipc_importance_task_t task_imp, uint32_t count);
extern kern_return_t ipc_importance_task_drop_internal_assertion(ipc_importance_task_t task_imp, uint32_t count);

extern kern_return_t ipc_importance_task_hold_file_lock_assertion(ipc_importance_task_t task_imp, uint32_t count);
extern kern_return_t ipc_importance_task_drop_file_lock_assertion(ipc_importance_task_t task_imp, uint32_t count);

extern kern_return_t ipc_importance_task_hold_legacy_external_assertion(ipc_importance_task_t task_imp, uint32_t count);
extern kern_return_t ipc_importance_task_drop_legacy_external_assertion(ipc_importance_task_t task_imp, uint32_t count);

extern boolean_t ipc_importance_check_circularity(ipc_port_t port, ipc_port_t dest);

/* prepare importance attributes for sending */
extern boolean_t ipc_importance_send(
	ipc_kmsg_t              kmsg,
	mach_msg_option_t       option);

/* receive importance attributes from message */
extern void ipc_importance_receive(
	ipc_kmsg_t              kmsg,
	mach_msg_option_t       option);

/* undo receive of importance attributes from message */
extern void ipc_importance_unreceive(
	ipc_kmsg_t              kmsg,
	mach_msg_option_t       option);

/* clean importance attributes out of destroyed message */
extern void ipc_importance_clean(ipc_kmsg_t kmsg);

/* assert a message is clean w.r.t. importance attributes */
extern void ipc_importance_assert_clean(ipc_kmsg_t kmsg);

/* initialize the ipc importance subsystem */
extern void ipc_importance_init(void);

/* initialize the ipc importance delayed calls */
extern void ipc_importance_thread_call_init(void);

#if DEVELOPMENT || DEBUG
extern void task_importance_update_owner_info(task_t task);
#endif

#if XNU_KERNEL_PRIVATE
#define TASK_IMP_LIST_DONATING_PIDS  0x1
extern int task_importance_list_pids(task_t task, int flags, char *pid_list, unsigned int max_count);
#endif

__END_DECLS

#endif /* MACH_KERNEL_PRIVATE */

#endif  /* _IPC_IPC_IMPORTANCE_H_ */
