/*
 * Copyright (c) 2000-2018 Apple Inc. All rights reserved.
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
/* Copyright (c) 1995, 1997 Apple Computer, Inc. All Rights Reserved */
/*
 * Copyright (c) 1982, 1986, 1989, 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)user.h	8.2 (Berkeley) 9/23/93
 */

#ifndef _SYS_USER_H_
#define _SYS_USER_H_

#include <sys/appleapiopts.h>
struct waitq_set;
#ifndef KERNEL
/* stuff that *used* to be included by user.h, or is now needed */
#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/ucred.h>
#include <sys/uio.h>
#endif
#ifdef XNU_KERNEL_PRIVATE
#include <sys/resource.h>
#include <sys/resourcevar.h>
#include <sys/signal.h>
#include <sys/signalvar.h>
#endif
#include <sys/vm.h>             /* XXX */
#include <sys/sysctl.h>

#ifdef KERNEL
#ifdef BSD_KERNEL_PRIVATE
#include <sys/pthread_internal.h> /* for uu_kwe entry */
#include <sys/eventvar.h>
#endif  /* BSD_KERNEL_PRIVATE */
#ifdef __APPLE_API_PRIVATE
#include <sys/eventvar.h>

#if !defined(__LP64__) || defined(XNU_KERNEL_PRIVATE)
/*
 * VFS context structure (part of uthread)
 */
struct vfs_context {
	thread_t        vc_thread;              /* pointer to Mach thread */
	kauth_cred_t    vc_ucred;               /* per thread credential */
};

#endif /* !__LP64 || XNU_KERNEL_PRIVATE */

#ifdef BSD_KERNEL_PRIVATE
/* XXX Deprecated: xnu source compatability */
#define uu_ucred        uu_context.vc_ucred

struct label;           /* MAC label dummy struct */

#define MAXTHREADNAMESIZE 64
/*
 *	Per-thread U area.
 */

struct uthread {
	/* syscall parameters, results and catches */
	u_int64_t uu_arg[8]; /* arguments to current system call */
	int uu_rval[2];
	char uu_cursig; /* p_cursig for exc. */
	/*
	 * uu_workq_pthread_kill_allowed is not modified under a lock and thus
	 * relies on single copy atomicity and cannot be changed to a bitfield.
	 */
	bool uu_workq_pthread_kill_allowed;
	uint16_t syscall_code; /* current syscall code */

	/* thread exception handling */
	int     uu_exception;
	mach_exception_code_t uu_code;  /* ``code'' to trap */
	mach_exception_subcode_t uu_subcode;

	/* support for syscalls which use continuations */
	union {
		struct _select_data {
			u_int64_t abstime;
			uint64_t *wqp;
			int count;
			struct select_nocancel_args *args;  /* original syscall arguments */
			int32_t *retval;                    /* place to store return val */
		} uus_select_data;

		struct kevent_ctx_s uus_kevent;

		struct _kevent_register {
			struct kevent_qos_s kev;            /* the kevent to maybe copy out */
			thread_t handoff_thread;            /* thread we handed off to, has +1 */
			struct kqworkloop *kqwl;
			int eventout;                       /* number of events output */
			user_addr_t ueventlist;             /* the user-address to copyout to */
		} uus_kevent_register;                   /* saved for EVFILT_WORKLOOP wait */

		struct _kauth {
			user_addr_t message;                /* message in progress */
		} uus_kauth;

		struct ksyn_waitq_element uus_kwe;       /* user for pthread synch */

		struct _waitid_data {
			struct waitid_nocancel_args *args;  /* original syscall arguments */
			int32_t *retval;                    /* place to store return val */
		} uus_waitid_data;

		struct _wait4_data {
			struct wait4_nocancel_args *args;   /* original syscall arguments */
			int32_t *retval;                    /* place to store return val */
		} uus_wait4_data;

		struct _workq_park_data {
			uint64_t idle_stamp;
			uint64_t workloop_params;
			uint32_t fulfilled_snapshot;
			uint32_t yields;
			void *thread_request;                /* request being fulfilled, for tracing only */
			uint32_t upcall_flags;
			bool has_stack;
			thread_qos_t qos;
		} uus_workq_park_data;                   /* saved for parked workq threads */

		struct _ulock_wait_data {
			thread_t owner_thread;
			thread_t old_owner;
			int32_t *retval;
			uint flags;
		} uus_ulock_wait_data;
	} uu_save;

	/* Persistent memory allocations across system calls */
	struct _select {
		u_int32_t       *ibits, *obits; /* bits to select on */
		uint    nbytes; /* number of bytes in ibits and obits */
	} uu_select;                    /* saved state for select() */

	struct proc *uu_proc;
	thread_t uu_thread;
	void * uu_userstate;
	struct waitq_set *uu_wqset;             /* waitq state cached across select calls */
	size_t uu_wqstate_sz;                   /* ...size of uu_wqset buffer */
	int uu_flag;
	sigset_t uu_siglist;                            /* signals pending for the thread */
	sigset_t uu_sigwait;                            /*  sigwait on this thread*/
	sigset_t uu_sigmask;                            /* signal mask for the thread */
	sigset_t uu_oldmask;                            /* signal mask saved before sigpause */
	sigset_t uu_vforkmask;                          /* saved signal mask during vfork */
	struct vfs_context uu_context;                  /* thread + cred */

	TAILQ_ENTRY(uthread) uu_list;           /* List of uthreads in proc */

	struct kaudit_record    *uu_ar;                 /* audit record */
	struct task*    uu_aio_task;                    /* target task for async io */

	union {
		lck_mtx_t  *uu_mtx;
		struct knote_lock_ctx *uu_knlock;
	};

	lck_spin_t      uu_rethrottle_lock;     /* locks was_rethrottled and is_throttled */
	TAILQ_ENTRY(uthread) uu_throttlelist;   /* List of uthreads currently throttled */
	void    *       uu_throttle_info;       /* pointer to throttled I/Os info */
	int8_t          uu_on_throttlelist;
	bool            uu_lowpri_window;
	/* These boolean fields are protected by different locks */
	bool            uu_was_rethrottled;
	bool            uu_is_throttled;
	bool            uu_throttle_bc;
	bool            uu_defer_reclaims;

	/* internal support for continuation framework */
	uint16_t uu_pri;                        /* pri | PCATCH | PVFS, ... */
	caddr_t uu_wchan;                       /* sleeping thread wait channel */
	int (*uu_continuation)(int);
	const char *uu_wmesg;                   /* ... wait message */

	u_int32_t       uu_network_marks;       /* network control flow marks */

	struct kern_sigaltstack uu_sigstk;
	vnode_t         uu_vreclaims;
	vnode_t         uu_cdir;                /* per thread CWD */
	int             uu_dupfd;               /* fd in fdesc_open/dupfdopen */

	/*
	 * Bound kqueue request. This field is only cleared by the current thread,
	 * hence can be dereferenced safely by the current thread without locks.
	 */
	struct workq_threadreq_s *uu_kqr_bound;
	TAILQ_ENTRY(uthread) uu_workq_entry;
	vm_offset_t uu_workq_stackaddr;
	mach_port_name_t uu_workq_thport;
	struct uu_workq_policy {
		uint16_t qos_req : 4;         /* requested QoS */
		uint16_t qos_max : 4;         /* current acked max qos */
		uint16_t qos_override : 4;    /* received async override */
		uint16_t qos_bucket : 4;      /* current acked bucket */
	} uu_workq_pri;
	uint8_t uu_workq_flags;
	kq_index_t uu_kqueue_override;

#ifdef JOE_DEBUG
	int             uu_iocount;
	int             uu_vpindex;
	void    *uu_vps[32];
	void    *uu_pcs[32][10];
#endif
#if CONFIG_WORKLOOP_DEBUG
#define UU_KEVENT_HISTORY_COUNT 32
#define UU_KEVENT_HISTORY_WRITE_ENTRY(uth, ...)  ({ \
	        struct uthread *__uth = (uth); \
	        unsigned int __index = __uth->uu_kevent_index++; \
	        __uth->uu_kevent_history[__index % UU_KEVENT_HISTORY_COUNT] = \
	                        (struct uu_kevent_history)__VA_ARGS__; \
	})
	struct uu_kevent_history {
		uint64_t uu_kqid;
		struct kqueue *uu_kq;
		int uu_error, uu_nchanges, uu_nevents;
		unsigned int uu_flags;
	} uu_kevent_history[UU_KEVENT_HISTORY_COUNT];
	unsigned int uu_kevent_index;
#endif
	int             uu_proc_refcount;
#if PROC_REF_DEBUG
#define NUM_PROC_REFS_TO_TRACK 32
#define PROC_REF_STACK_DEPTH 10
	int             uu_pindex;
	void    *       uu_proc_ps[NUM_PROC_REFS_TO_TRACK];
	uintptr_t       uu_proc_pcs[NUM_PROC_REFS_TO_TRACK][PROC_REF_STACK_DEPTH];
#endif

#if CONFIG_DTRACE
	uint32_t        t_dtrace_errno; /* Most recent errno */
	siginfo_t       t_dtrace_siginfo;
	uint64_t        t_dtrace_resumepid; /* DTrace's pidresume() pid */
	uint8_t         t_dtrace_stop;  /* indicates a DTrace desired stop */
	uint8_t         t_dtrace_sig;   /* signal sent via DTrace's raise() */

	union __tdu {
		struct __tds {
			uint8_t _t_dtrace_on;   /* hit a fasttrap tracepoint */
			uint8_t _t_dtrace_step; /* about to return to kernel */
			uint8_t _t_dtrace_ret;  /* handling a return probe */
			uint8_t _t_dtrace_ast;  /* saved ast flag */
#if __sol64 || defined(__APPLE__)
			uint8_t _t_dtrace_reg;  /* modified register */
#endif
		} _tds;
		u_int32_t _t_dtrace_ft;           /* bitwise or of these flags */
	} _tdu;
#define t_dtrace_ft     _tdu._t_dtrace_ft
#define t_dtrace_on     _tdu._tds._t_dtrace_on
#define t_dtrace_step   _tdu._tds._t_dtrace_step
#define t_dtrace_ret    _tdu._tds._t_dtrace_ret
#define t_dtrace_ast    _tdu._tds._t_dtrace_ast
#if __sol64 || defined(__APPLE__)
#define t_dtrace_reg    _tdu._tds._t_dtrace_reg
#endif

	user_addr_t     t_dtrace_pc;    /* DTrace saved pc from fasttrap */
	user_addr_t     t_dtrace_npc;   /* DTrace next pc from fasttrap */
	user_addr_t     t_dtrace_scrpc; /* DTrace per-thread scratch location */
	user_addr_t     t_dtrace_astpc; /* DTrace return sequence location */

	struct dtrace_ptss_page_entry*  t_dtrace_scratch; /* scratch space entry */

#if __sol64 || defined(__APPLE__)
	uint64_t        t_dtrace_regv;  /* DTrace saved reg from fasttrap */
#endif
	void *t_dtrace_syscall_args;
#endif /* CONFIG_DTRACE */
	char *pth_name;

	/* Document Tracking struct used to track a "tombstone" for a document */
	struct doc_tombstone *t_tombstone;

	struct os_reason *uu_exit_reason;
};

typedef struct uthread * uthread_t;

/* Definition of uu_flag */
#define UT_SAS_OLDMASK  0x00000001      /* need to restore mask before pause */
#define UT_NO_SIGMASK   0x00000002      /* exited thread; invalid sigmask */
#define UT_NOTCANCELPT  0x00000004             /* not a cancelation point */
#define UT_CANCEL       0x00000008             /* thread marked for cancel */
#define UT_CANCELED     0x00000010            /* thread cancelled */
#define UT_CANCELDISABLE 0x00000020            /* thread cancel disabled */
#define UT_ALTSTACK     0x00000040      /* this thread has alt stack for signals */
#define UT_THROTTLE_IO  0x00000080      /* this thread issues throttle I/O */
#define UT_PASSIVE_IO   0x00000100      /* this thread issues passive I/O */
#define UT_PROCEXIT     0x00000200      /* this thread completed the  proc exit */
#define UT_RAGE_VNODES  0x00000400      /* rapid age any vnodes created by this thread */
#define UT_KERN_RAGE_VNODES        0x00000800 /* rapid age any vnodes created by this thread (kernel set) */
#define UT_NSPACE_NODATALESSFAULTS 0x00001000 /* thread does not materialize dataless files */
#define UT_ATIME_UPDATE 0x00002000      /* don't update atime for files accessed by this thread */
#define UT_NSPACE_FORCEDATALESSFAULTS  0x00004000 /* thread always materializes dataless files */
#define UT_VFORK        0x02000000      /* thread has vfork children */
#define UT_SETUID       0x04000000      /* thread is settugid() */
#define UT_WASSETUID    0x08000000      /* thread was settugid() (in vfork) */
#define UT_VFORKING     0x10000000      /* thread in vfork() syscall */

#endif /* BSD_KERNEL_PRIVATE */

#endif /* __APPLE_API_PRIVATE */

#endif  /* KERNEL */

/*
 * Per process structure containing data that isn't needed in core
 * when the process isn't running (esp. when swapped out).
 * This structure may or may not be at the same kernel address
 * in all processes.
 */

struct  user {
	/* NOT USED ANYMORE */
};

#endif  /* !_SYS_USER_H_ */
