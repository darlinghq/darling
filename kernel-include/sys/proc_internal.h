/*
 * Copyright (c) 2000-2010 Apple Inc. All rights reserved.
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
/*-
 * Copyright (c) 1986, 1989, 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
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
 *	@(#)proc_internal.h	8.15 (Berkeley) 5/19/95
 */
/*
 * NOTICE: This file was modified by SPARTA, Inc. in 2005 to introduce
 * support for mandatory and extensible security protections.  This notice
 * is included in support of clause 2.2 (b) of the Apple Public License,
 * Version 2.0.
 */

#ifndef _SYS_PROC_INTERNAL_H_
#define	_SYS_PROC_INTERNAL_H_

#include <libkern/OSAtomic.h>
#include <sys/proc.h>
__BEGIN_DECLS
#include <kern/locks.h>
#if PSYNCH
#include <kern/thread_call.h>
#endif /* PSYNCH */
__END_DECLS

#if DEBUG
#define	__PROC_INTERNAL_DEBUG 1
#endif

/* 
 * The short form for various locks that protect fields in the data structures.
 * PL = Process Lock
 * PGL = Process Group Lock
 * PFDL = Process File Desc Lock
 * PSL = Process Spin Lock
 * PPL = Parent Process Lock (planed for later usage)
 * LL = List Lock
 * SL = Session Lock
*/
struct label;

/*
 * Added by SPARTA, Inc.
 */
/*
 * Login context.
 */
struct lctx {
	LIST_ENTRY(lctx) lc_list;	/* List of all login contexts. */
	LIST_HEAD(, proc) lc_members;	/* Pointer to lc members. */
	int		lc_mc;		/* Member Count. */
	pid_t		lc_id;		/* Login context ID. */
	lck_mtx_t	lc_mtx;		/* Mutex to protect members */

	struct label	*lc_label;	/* Login context MAC label. */
};

/*
 * One structure allocated per session.
 */
struct	session {
	int			s_count;		/* Ref cnt; pgrps in session. (LL) */
	struct	proc *		s_leader;		/* Session leader.(static) */
	struct	vnode *		s_ttyvp;		/* Vnode of controlling terminal.(SL) */
	int			s_ttyvid;		/* Vnode id of the controlling terminal (SL) */
	struct	tty *		s_ttyp;			/* Controlling terminal. (SL + ttyvp != NULL) */
	pid_t			s_ttypgrpid;		/* tty's pgrp id */
	pid_t			s_sid;			/* Session ID (static) */
	char			s_login[MAXLOGNAME];	/* Setlogin() name.(SL) */
	int			s_flags;		/* Session flags (s_mlock)  */
	LIST_ENTRY(session)	s_hash;			/* Hash chain.(LL) */
	lck_mtx_t		s_mlock;		/* mutex lock to protect session */
	int			s_listflags;
};

#define SESSION_NULL (struct session *)0

/*
 * accessor for s_ttyp which treats it as invalid if s_ttyvp is not valid;
 * note that s_ttyp is not a reference in the session structre, so it can
 * become invalid out from under the session if the device is closed, without
 * this protection.  We can't safely make it into a reference without reflexive
 * close notification of tty devices through cdevsw[].
 *
 * NB:	<sys/tty.h> is not in scope and there is not typedef type enforcement,
 *	or '0' below would be 'TTY_NULL'.
 */
#define SESSION_TP(sp)	(((sp)->s_ttyvp != 0) ? (sp)->s_ttyp : 0)

/*
 * Session flags; used to tunnel information to lower layers and line
 * disciplines, etc.
 */
#define	S_DEFAULT	0x00000000	/* No flags set */
#define	S_NOCTTY	0x00000001	/* Do not associate controlling tty */


#define S_LIST_TERM	1		/* marked for termination */
#define	S_LIST_DEAD	2		/* already dead */
/*
 * One structure allocated per process group.
 */
struct	pgrp {
	LIST_ENTRY(pgrp)	pg_hash;	/* Hash chain. (LL) */
	LIST_HEAD(, proc) 	pg_members;	/* Pointer to pgrp members. (PGL) */
	struct	session *	pg_session;	/* Pointer to session. (LL ) */
	pid_t			pg_id;		/* Pgrp id. (static) */
	int			pg_jobc;	/* # procs qualifying pgrp for job control (PGL) */
	int			pg_membercnt;	/* Number of processes in the pgrocess group (PGL) */
	int			pg_refcount;	/* number of current iterators (LL) */
	unsigned int		pg_listflags;	/* (LL) */
	lck_mtx_t		pg_mlock;	/* mutex lock to protect pgrp */
};

#define PGRP_FLAG_TERMINATE 	1
#define PGRP_FLAG_WAITTERMINATE 2
#define PGRP_FLAG_DEAD 		4
#define PGRP_FLAG_ITERABEGIN 	8
#define PGRP_FLAG_ITERWAIT 	0x10

#define PGRP_NULL (struct pgrp *)0
struct proc;

#define PROC_NULL (struct proc *)0

#define PROC_UPDATE_CREDS_ONPROC(p) { \
	p->p_uid =  kauth_cred_getuid(p->p_ucred); \
	p->p_gid =  kauth_cred_getgid(p->p_ucred); \
	p->p_ruid =  kauth_cred_getruid(p->p_ucred); \
	p->p_rgid =  kauth_cred_getrgid(p->p_ucred); \
	p->p_svuid =  kauth_cred_getsvuid(p->p_ucred); \
	p->p_svgid =  kauth_cred_getsvgid(p->p_ucred); \
	}
/*
 * Description of a process.
 *
 * This structure contains the information needed to manage a thread of
 * control, known in UN*X as a process; it has references to substructures
 * containing descriptions of things that the process uses, but may share
 * with related processes.  The process structure and the substructures
 * are always addressible except for those marked "(PROC ONLY)" below,
 * which might be addressible only on a processor on which the process
 * is running.
 */
struct	proc {
	LIST_ENTRY(proc) p_list;		/* List of all processes. */

	pid_t		p_pid;			/* Process identifier. (static)*/
	void * 		task;			/* corresponding task (static)*/
	struct	proc *	p_pptr;		 	/* Pointer to parent process.(LL) */
	pid_t		p_ppid;			/* process's parent pid number */
	pid_t		p_pgrpid;		/* process group id of the process (LL)*/
	uid_t		p_uid;
	gid_t		p_gid;
	uid_t		p_ruid;
	gid_t		p_rgid;
	uid_t		p_svuid;
	gid_t		p_svgid;
	uint64_t	p_uniqueid;		/* process uniqe ID */

	lck_mtx_t 	p_mlock;		/* mutex lock for proc */

	char		p_stat;			/* S* process status. (PL)*/
	char		p_shutdownstate;
	char		p_kdebug;		/* P_KDEBUG eq (CC)*/ 
	char		p_btrace;		/* P_BTRACE eq (CC)*/

	LIST_ENTRY(proc) p_pglist;		/* List of processes in pgrp.(PGL) */
	LIST_ENTRY(proc) p_sibling;		/* List of sibling processes. (LL)*/
	LIST_HEAD(, proc) p_children;		/* Pointer to list of children. (LL)*/
	TAILQ_HEAD( , uthread) p_uthlist; 	/* List of uthreads  (PL) */

	LIST_ENTRY(proc) p_hash;		/* Hash chain. (LL)*/
	TAILQ_HEAD( ,eventqelt) p_evlist;	/* (PL) */

	lck_mtx_t	p_fdmlock;		/* proc lock to protect fdesc */

	/* substructures: */
	kauth_cred_t	p_ucred;		/* Process owner's identity. (PL) */
	struct	filedesc *p_fd;			/* Ptr to open files structure. (PFDL) */
	struct	pstats *p_stats;		/* Accounting/statistics (PL). */
	struct	plimit *p_limit;		/* Process limits.(PL) */

	struct	sigacts *p_sigacts;		/* Signal actions, state (PL) */
	 int		p_siglist;		/* signals captured back from threads */
	lck_spin_t	p_slock;		/* spin lock for itimer/profil protection */

#define	p_rlimit	p_limit->pl_rlimit

	struct	plimit *p_olimit;		/* old process limits  - not inherited by child  (PL) */
	unsigned int	p_flag;			/* P_* flags. (atomic bit ops) */
	unsigned int	p_lflag;		/* local flags  (PL) */
	unsigned int	p_listflag;		/* list flags (LL) */
	unsigned int	p_ladvflag;		/* local adv flags (atomic) */
	int		p_refcount;		/* number of outstanding users(LL) */
	int		p_childrencnt;		/* children holding ref on parent (LL) */
	int		p_parentref;		/* children lookup ref on parent (LL) */

	pid_t		p_oppid;	 	/* Save parent pid during ptrace. XXX */
	u_int		p_xstat;		/* Exit status for wait; also stop signal. */

#ifdef _PROC_HAS_SCHEDINFO_
	/* may need cleanup, not used */
	u_int		p_estcpu;	 	/* Time averaged value of p_cpticks.(used by aio and proc_comapre) */
	fixpt_t		p_pctcpu;	 	/* %cpu for this process during p_swtime (used by aio)*/
	u_int		p_slptime;		/* used by proc_compare */
#endif /* _PROC_HAS_SCHEDINFO_ */

	struct	itimerval p_realtimer;		/* Alarm timer. (PSL) */
	struct	timeval p_rtime;		/* Real time.(PSL)  */
	struct	itimerval p_vtimer_user;	/* Virtual timers.(PSL)  */
	struct	itimerval p_vtimer_prof;	/* (PSL) */

	struct	timeval	p_rlim_cpu;		/* Remaining rlim cpu value.(PSL) */
	int		p_debugger;		/*  NU 1: can exec set-bit programs if suser */
	boolean_t	sigwait;	/* indication to suspend (PL) */
	void	*sigwait_thread;	/* 'thread' holding sigwait(PL)  */
	void	*exit_thread;		/* Which thread is exiting(PL)  */
	int	p_vforkcnt;		/* number of outstanding vforks(PL)  */
    	void *  p_vforkact;     	/* activation running this vfork proc)(static)  */
	int	p_fpdrainwait;		/* (PFDL) */
	pid_t	p_contproc;	/* last PID to send us a SIGCONT (PL) */

	/* Following fields are info from SIGCHLD (PL) */
	pid_t	si_pid;			/* (PL) */
	u_int   si_status;		/* (PL) */
	u_int	si_code;		/* (PL) */
	uid_t	si_uid;			/* (PL) */

	void * vm_shm;			/* (SYSV SHM Lock) for sysV shared memory */

#if CONFIG_DTRACE
	user_addr_t			p_dtrace_argv;			/* (write once, read only after that) */
	user_addr_t			p_dtrace_envp;			/* (write once, read only after that) */
	lck_mtx_t			p_dtrace_sprlock;		/* sun proc lock emulation */
	int				p_dtrace_probes;		/* (PL) are there probes for this proc? */
	u_int				p_dtrace_count;			/* (sprlock) number of DTrace tracepoints */
        uint8_t                         p_dtrace_stop;                  /* indicates a DTrace-desired stop */
	struct dtrace_ptss_page*	p_dtrace_ptss_pages;		/* (sprlock) list of user ptss pages */
	struct dtrace_ptss_page_entry*	p_dtrace_ptss_free_list;	/* (atomic) list of individual ptss entries */
	struct dtrace_helpers*		p_dtrace_helpers;		/* (dtrace_lock) DTrace per-proc private */
	struct dof_ioctl_data*		p_dtrace_lazy_dofs;		/* (sprlock) unloaded dof_helper_t's */
#endif /* CONFIG_DTRACE */

/* XXXXXXXXXXXXX BCOPY'ed on fork XXXXXXXXXXXXXXXX */
/* The following fields are all copied upon creation in fork. */
#define	p_startcopy	p_argslen

	u_int	p_argslen;	 /* Length of process arguments. */
	int  	p_argc;			/* saved argc for sysctl_procargs() */
	user_addr_t user_stack;		/* where user stack was allocated */
	struct	vnode *p_textvp;	/* Vnode of executable. */
	off_t	p_textoff;		/* offset in executable vnode */

	sigset_t p_sigmask;		/* DEPRECATED */
	sigset_t p_sigignore;	/* Signals being ignored. (PL) */
	sigset_t p_sigcatch;	/* Signals being caught by user.(PL)  */

	u_char	p_priority;	/* (NU) Process priority. */
	u_char	p_resv0;	/* (NU) User-priority based on p_cpu and p_nice. */
	char	p_nice;		/* Process "nice" value.(PL) */
	u_char	p_resv1;	/* (NU) User-priority based on p_cpu and p_nice. */

#if CONFIG_MACF
	int	p_mac_enforce;			/* MAC policy enforcement control */
#endif

	char	p_comm[MAXCOMLEN+1];
	char	p_name[(2*MAXCOMLEN)+1];	/* PL */

	struct 	pgrp *p_pgrp;	/* Pointer to process group. (LL) */
	uint32_t	p_csflags;	/* flags for codesign (PL) */
	uint32_t	p_pcaction;	/* action  for process control on starvation */
	uint8_t p_uuid[16];		/* from LC_UUID load command */

#if !CONFIG_EMBEDDED
#define PROC_LEGACY_BEHAVIOR_IOTHROTTLE (0x00000001)
	 uint32_t	p_legacy_behavior;
#endif

/* End area that is copied on creation. */
/* XXXXXXXXXXXXX End of BCOPY'ed on fork (AIOLOCK)XXXXXXXXXXXXXXXX */
#define	p_endcopy	p_aio_total_count
	int		p_aio_total_count;		/* all allocated AIO requests for this proc */
	int		p_aio_active_count;		/* all unfinished AIO requests for this proc */
	TAILQ_HEAD( , aio_workq_entry ) p_aio_activeq; 	/* active async IO requests */
	TAILQ_HEAD( , aio_workq_entry ) p_aio_doneq;	/* completed async IO requests */

	struct klist p_klist;  /* knote list (PL ?)*/

	struct	rusage *p_ru;	/* Exit information. (PL) */
	int		p_sigwaitcnt;
	thread_t 	p_signalholder;
	thread_t 	p_transholder;

	/* DEPRECATE following field  */
	u_short	p_acflag;	/* Accounting flags. */

	struct lctx *p_lctx;		/* Pointer to login context. */
	LIST_ENTRY(proc) p_lclist;	/* List of processes in lctx. */
	user_addr_t 	p_threadstart;		/* pthread start fn */
	user_addr_t 	p_wqthread;		/* pthread workqueue fn */
	int 	p_pthsize;			/* pthread size */
	user_addr_t	p_targconc;		/* target concurrency ptr */
	void * 	p_wqptr;			/* workq ptr */
	int 	p_wqsize;			/* allocated size */
	boolean_t       p_wqiniting;            /* semaphore to serialze wq_open */
	lck_spin_t	p_wqlock;		/* lock to protect work queue */
	struct  timeval p_start;        	/* starting time */
	void *	p_rcall;
	int		p_ractive;
	int	p_idversion;		/* version of process identity */
	void *	p_pthhash;			/* pthread waitqueue hash */
#if DIAGNOSTIC
	unsigned int p_fdlock_pc[4];
	unsigned int p_fdunlock_pc[4];
#if SIGNAL_DEBUG
	unsigned int lockpc[8];
	unsigned int unlockpc[8];
#endif /* SIGNAL_DEBUG */
#endif /* DIAGNOSTIC */
	uint64_t	p_dispatchqueue_offset;
#if VM_PRESSURE_EVENTS
	struct timeval	vm_pressure_last_notify_tstamp;
#endif
	int		p_dirty;			/* dirty state */ 
};

#define PGRPID_DEAD 0xdeaddead

/* p_listflag */ 
#define P_LIST_DRAIN   			0x00000001
#define P_LIST_DRAINWAIT 		0x00000002
#define P_LIST_DRAINED   		0x00000004
#define P_LIST_DEAD				0x00000008
#define P_LIST_WAITING   		0x00000010
#define P_LIST_EXITED    		0x00000040 
#define P_LIST_CHILDDRSTART    	0x00000080 
#define P_LIST_CHILDDRAINED    	0x00000100 
#define P_LIST_CHILDDRWAIT    	0x00000200 
#define P_LIST_CHILDLKWAIT    	0x00000400 
#define P_LIST_DEADPARENT    	0x00000800 
#define P_LIST_PARENTREFWAIT   	0x00001000 
#define P_LIST_INCREATE   		0x00002000 
/* 0x4000 &  0x8000 Not used */
#define P_LIST_INHASH	 		0x00010000	/* process is in hash */
#define P_LIST_INPGRP	 		0x00020000	/* process is in pgrp */
#define P_LIST_PGRPTRANS 		0x00040000	/* pgrp is getting replaced */
#define P_LIST_PGRPTRWAIT 		0x00080000	/* wait for pgrp replacement */
#define P_LIST_EXITCOUNT 		0x00100000	/* counted for process exit */ 
 

/* local flags */
#define	P_LDELAYTERM	0x00000001	/* */
#define	P_LNOZOMB	0x00000002 	/* */
#define P_LTERM		0x00000004      /* */ 
#define P_LEXIT		0x00000008      /* */
#define P_LPEXIT	0x00000010
#define	P_LTRANSCOMMIT	0x00000020	/* process is committed to trans */
#define	P_LINTRANSIT	0x00000040	/* process in exec or in creation */
#define	P_LTRANSWAIT	0x00000080	/* waiting for trans to complete */
#define P_LVFORK        0x00000100      /* */
#define P_LINVFORK      0x00000200      /* */
#define P_LTRACED       0x00000400      /* */
#define P_LSIGEXC       0x00000800      /* */
#define P_LNOATTACH     0x00001000      /* */
#define P_LPPWAIT       0x00002000      /* */
#define	P_LKQWDRAIN	0x00004000
#define	P_LKQWDRAINWAIT	0x00008000
#define	P_LKQWDEAD	0x00010000
#define	P_LLIMCHANGE	0x00020000
#define	P_LLIMWAIT	0x00040000
#define P_LWAITED   	0x00080000 
#define P_LINSIGNAL    	0x00100000 
#define P_UNUSED  	0x00200000 	/* Unused */
#define P_LRAGE_VNODES	0x00400000
#define P_LREGISTER	0x00800000	/* thread start fns registered  */
#define P_LVMRSRCOWNER	0x01000000	/* can handle the resource ownership of  */
#define P_LPTERMINATE	0x02000000	/* can handle the resource ownership of  */

/* Process control state for resource starvation */
#define P_PCTHROTTLE	1
#define P_PCSUSP	2
#define P_PCKILL	3
#define P_PCMAX		3

/* Process control action state on resrouce starvation */
#define PROC_ACTION_MASK 0xffff0000;
#define PROC_CONTROL_STATE(p) (p->p_pcaction & P_PCMAX)
#define PROC_ACTION_STATE(p) ((p->p_pcaction >> 16) & P_PCMAX)
#define PROC_SETACTION_STATE(p) (p->p_pcaction = (PROC_CONTROL_STATE(p) | (PROC_CONTROL_STATE(p) << 16)))
#define PROC_RESETACTION_STATE(p) (p->p_pcaction = PROC_CONTROL_STATE(p))

/* additional process flags */
#define P_LADVLOCK		0x01

/* defns for proc_iterate */
#define PROC_ALLPROCLIST        1		/* walk the allproc list (procs not exited yet) */
#define PROC_ZOMBPROCLIST       2		/*  walk the zombie list */
#define PROC_NOWAITTRANS       4		/* do not wait for transitions (checkdirs only)  */
 
/* defns for pgrp_iterate */ 
#define PGRP_DROPREF    	1
#define	PGRP_BLOCKITERATE 	2
 
/* return values of the proc iteration callback routine */ 
#define PROC_RETURNED           0
#define PROC_RETURNED_DONE      1
#define PROC_CLAIMED            2
#define PROC_CLAIMED_DONE       3

/* process creation arguments */
#define	PROC_CREATE_FORK	0	/* independent child (running) */
#define	PROC_CREATE_SPAWN	1	/* independent child (suspended) */
#define	PROC_CREATE_VFORK	2	/* child borrows context */


/* LP64 version of extern_proc.  all pointers 
 * grow when we're dealing with a 64-bit process.
 * WARNING - keep in sync with extern_proc
 * but use native alignment of 64-bit process.
 */

#ifdef KERNEL
#include <sys/time.h>	/* user_timeval, user_itimerval */

#pragma pack(4)
struct user32_extern_proc {
	union {
		struct {
			uint32_t __p_forw;	/* Doubly-linked run/sleep queue. */
			uint32_t __p_back;
		} p_st1;
		struct user32_timeval __p_starttime; 	/* process start time */
	} p_un;
	uint32_t 	p_vmspace;	/* Address space. */
	uint32_t	p_sigacts;	/* Signal actions, state (PROC ONLY). */
	int		p_flag;			/* P_* flags. */
	char	p_stat;			/* S* process status. */
	pid_t	p_pid;			/* Process identifier. */
	pid_t	p_oppid;		/* Save parent pid during ptrace. XXX */
	int		p_dupfd;		/* Sideways return value from fdopen. XXX */
	/* Mach related  */
	uint32_t user_stack;	/* where user stack was allocated */
	uint32_t exit_thread;  /* XXX Which thread is exiting? */
	int		p_debugger;		/* allow to debug */
	boolean_t	sigwait;	/* indication to suspend */
	/* scheduling */
	u_int	p_estcpu;	 /* Time averaged value of p_cpticks. */
	int		p_cpticks;	 /* Ticks of cpu time. */
	fixpt_t	p_pctcpu;	 /* %cpu for this process during p_swtime */
	uint32_t	p_wchan;	 /* Sleep address. */
	uint32_t	p_wmesg;	 /* Reason for sleep. */
	u_int	p_swtime;	 /* Time swapped in or out. */
	u_int	p_slptime;	 /* Time since last blocked. */
	struct	user32_itimerval p_realtimer;	/* Alarm timer. */
	struct	user32_timeval p_rtime;	/* Real time. */
	u_quad_t p_uticks;		/* Statclock hits in user mode. */
	u_quad_t p_sticks;		/* Statclock hits in system mode. */
	u_quad_t p_iticks;		/* Statclock hits processing intr. */
	int		p_traceflag;		/* Kernel trace points. */
	uint32_t	p_tracep;	/* Trace to vnode. */
	int		p_siglist;		/* DEPRECATED */
	uint32_t	p_textvp;	/* Vnode of executable. */
	int		p_holdcnt;		/* If non-zero, don't swap. */
	sigset_t p_sigmask;	/* DEPRECATED. */
	sigset_t p_sigignore;	/* Signals being ignored. */
	sigset_t p_sigcatch;	/* Signals being caught by user. */
	u_char	p_priority;	/* Process priority. */
	u_char	p_usrpri;	/* User-priority based on p_cpu and p_nice. */
	char	p_nice;		/* Process "nice" value. */
	char	p_comm[MAXCOMLEN+1];
	uint32_t	p_pgrp;	/* Pointer to process group. */
	uint32_t	p_addr;	/* Kernel virtual addr of u-area (PROC ONLY). */
	u_short	p_xstat;	/* Exit status for wait; also stop signal. */
	u_short	p_acflag;	/* Accounting flags. */
	uint32_t	p_ru;	/* Exit information. XXX */
};
#pragma pack()
struct user64_extern_proc {
	union {
		struct {
			user_addr_t __p_forw;	/* Doubly-linked run/sleep queue. */
			user_addr_t __p_back;
		} p_st1;
		struct user64_timeval __p_starttime; 	/* process start time */
	} p_un;
	user_addr_t 	p_vmspace;	/* Address space. */
	user_addr_t		p_sigacts;	/* Signal actions, state (PROC ONLY). */
	int		p_flag;			/* P_* flags. */
	char	p_stat;			/* S* process status. */
	pid_t	p_pid;			/* Process identifier. */
	pid_t	p_oppid;		/* Save parent pid during ptrace. XXX */
	int		p_dupfd;		/* Sideways return value from fdopen. XXX */
	/* Mach related  */
	user_addr_t user_stack __attribute((aligned(8)));	/* where user stack was allocated */
	user_addr_t exit_thread;  /* XXX Which thread is exiting? */
	int		p_debugger;		/* allow to debug */
	boolean_t	sigwait;	/* indication to suspend */
	/* scheduling */
	u_int	p_estcpu;	 /* Time averaged value of p_cpticks. */
	int		p_cpticks;	 /* Ticks of cpu time. */
	fixpt_t	p_pctcpu;	 /* %cpu for this process during p_swtime */
	user_addr_t	p_wchan __attribute((aligned(8)));	 /* Sleep address. */
	user_addr_t	p_wmesg;	 /* Reason for sleep. */
	u_int	p_swtime;	 /* Time swapped in or out. */
	u_int	p_slptime;	 /* Time since last blocked. */
	struct	user64_itimerval p_realtimer;	/* Alarm timer. */
	struct	user64_timeval p_rtime;	/* Real time. */
	u_quad_t p_uticks;		/* Statclock hits in user mode. */
	u_quad_t p_sticks;		/* Statclock hits in system mode. */
	u_quad_t p_iticks;		/* Statclock hits processing intr. */
	int		p_traceflag;		/* Kernel trace points. */
	user_addr_t	p_tracep __attribute((aligned(8)));	/* Trace to vnode. */
	int		p_siglist;		/* DEPRECATED */
	user_addr_t	p_textvp __attribute((aligned(8)));	/* Vnode of executable. */
	int		p_holdcnt;		/* If non-zero, don't swap. */
	sigset_t p_sigmask;	/* DEPRECATED. */
	sigset_t p_sigignore;	/* Signals being ignored. */
	sigset_t p_sigcatch;	/* Signals being caught by user. */
	u_char	p_priority;	/* Process priority. */
	u_char	p_usrpri;	/* User-priority based on p_cpu and p_nice. */
	char	p_nice;		/* Process "nice" value. */
	char	p_comm[MAXCOMLEN+1];
	user_addr_t	p_pgrp __attribute((aligned(8)));	/* Pointer to process group. */
	user_addr_t	p_addr;	/* Kernel virtual addr of u-area (PROC ONLY). */
	u_short	p_xstat;	/* Exit status for wait; also stop signal. */
	u_short	p_acflag;	/* Accounting flags. */
	user_addr_t	p_ru __attribute((aligned(8)));	/* Exit information. XXX */
};
#endif	/* KERNEL */

/*
 * We use process IDs <= PID_MAX; PID_MAX + 1 must also fit in a pid_t,
 * as it is used to represent "no process group".
 */
extern int nprocs, maxproc;		/* Current and max number of procs. */
extern int maxprocperuid;		/* Current number of procs per uid */
__private_extern__ int hard_maxproc;	/* hard limit */
extern unsigned int proc_shutdown_exitcount;

#define	PID_MAX		99999
#define	NO_PID		100000
extern lck_mtx_t * proc_list_mlock;
extern lck_mtx_t * proc_klist_mlock;

#define BSD_SIMUL_EXECS		33 /* 32 , allow for rounding */
#define	BSD_PAGEABLE_SIZE_PER_EXEC	(NCARGS + PAGE_SIZE + PAGE_SIZE) /* page for apple vars, page for executable header */
extern int execargs_cache_size;
extern int execargs_free_count;
extern vm_offset_t * execargs_cache;

#define SESS_LEADER(p, sessp)	((sessp)->s_leader == (p))

/* Lock and unlock a login context. */
#define LCTX_LOCK(lc)	lck_mtx_lock(&(lc)->lc_mtx)
#define LCTX_UNLOCK(lc)	lck_mtx_unlock(&(lc)->lc_mtx)
#define LCTX_LOCKED(lc)
#define LCTX_LOCK_ASSERT(lc, type)
#define ALLLCTX_LOCK	lck_mtx_lock(&alllctx_lock)
#define ALLLCTX_UNLOCK	lck_mtx_unlock(&alllctx_lock)
extern lck_mtx_t alllctx_lock;
extern lck_grp_t * lctx_lck_grp;
extern lck_grp_attr_t * lctx_lck_grp_attr;
extern lck_attr_t * lctx_lck_attr;

#define	PIDHASH(pid)	(&pidhashtbl[(pid) & pidhash])
extern LIST_HEAD(pidhashhead, proc) *pidhashtbl;
extern u_long pidhash;

#define	PGRPHASH(pgid)	(&pgrphashtbl[(pgid) & pgrphash])
extern LIST_HEAD(pgrphashhead, pgrp) *pgrphashtbl;
extern u_long pgrphash;
#define	SESSHASH(sessid) (&sesshashtbl[(sessid) & sesshash])
extern LIST_HEAD(sesshashhead, session) *sesshashtbl;
extern u_long sesshash;

extern lck_grp_t * proc_lck_grp;
#if CONFIG_FINE_LOCK_GROUPS
extern lck_grp_t * proc_mlock_grp;
extern lck_grp_t * proc_fdmlock_grp;
extern lck_grp_t * proc_slock_grp;
#endif
extern lck_grp_attr_t * proc_lck_grp_attr;
extern lck_attr_t * proc_lck_attr;

LIST_HEAD(proclist, proc);
extern struct proclist allproc;		/* List of all processes. */
extern struct proclist zombproc;	/* List of zombie processes. */
extern struct proc *initproc;
extern void	procinit(void) __attribute__((section("__TEXT, initcode")));
extern void proc_lock(struct proc *);
extern void proc_unlock(struct proc *);
extern void proc_spinlock(struct proc *);
extern void proc_spinunlock(struct proc *);
extern void proc_list_lock(void);
extern void proc_list_unlock(void);
extern void proc_klist_lock(void);
extern void proc_klist_unlock(void);
extern void proc_fdlock(struct proc *);
extern void proc_fdlock_spin(struct proc *);
extern void proc_fdunlock(struct proc *);
extern void proc_fdlock_assert(proc_t p, int assertflags);
__private_extern__ int proc_core_name(const char *name, uid_t uid, pid_t pid,
		char *cr_name, size_t cr_name_len);
extern int isinferior(struct proc *, struct proc *);
__private_extern__ struct proc *pzfind(pid_t);	/* Find zombie by id. */
__private_extern__ struct proc *proc_find_zombref(pid_t);	/* Find zombie by id. */
__private_extern__ void proc_drop_zombref(struct proc * p);	/* Find zombie by id. */


extern struct	lctx *lcfind(pid_t);		/* Find a login context by id */
extern struct	lctx *lccreate(void);		/* Create a new login context */

extern int	chgproccnt(uid_t uid, int diff);
extern void	enterlctx(struct proc *p, struct lctx *l, int create);
extern void	pinsertchild(struct proc *parent, struct proc *child);
extern int	enterpgrp(struct proc *p, pid_t pgid, int mksess);
extern void	fixjobc(struct proc *p, struct pgrp *pgrp, int entering);
extern int	inferior(struct proc *p);
extern int	leavepgrp(struct proc *p);
extern void	leavelctx(struct proc *p);
extern void	resetpriority(struct proc *);
extern void	setrunnable(struct proc *);
extern void	setrunqueue(struct proc *);
extern int	sleep(void *chan, int pri);
extern int	tsleep0(void *chan, int pri, const char *wmesg, int timo, int (*continuation)(int));
extern int	tsleep1(void *chan, int pri, const char *wmesg, u_int64_t abstime, int (*continuation)(int));
extern int	msleep0(void *chan, lck_mtx_t *mtx, int pri, const char *wmesg, int timo, int (*continuation)(int));
extern void	vfork_return(struct proc *child, int32_t *retval, int rval);
extern int	exit1(struct proc *, int, int *);
extern int	exit1_internal(struct proc *, int, int *, boolean_t, boolean_t);
extern int	fork1(proc_t, thread_t *, int);
extern void vfork_exit_internal(struct proc *p, int rv, int forced);
extern void proc_reparentlocked(struct proc *child, struct proc * newparent, int cansignal, int locked);
extern int pgrp_iterate(struct pgrp * pgrp, int flags, int (*callout)(proc_t , void *), void *arg, int (*filterfn)(proc_t , void *), void *filterarg);
extern int proc_iterate(int flags, int (*callout)(proc_t , void *), void *arg, int (*filterfn)(proc_t , void *), void *filterarg);
extern int proc_rebootscan(int (*callout)(proc_t , void *), void *arg, int (*filterfn)(proc_t , void *), void *filterarg);
extern int proc_childrenwalk(proc_t p, int (*callout)(proc_t , void *), void *arg);
extern proc_t proc_findinternal(int pid, int funneled);
extern proc_t proc_findthread(thread_t thread);
extern void proc_refdrain(proc_t);
extern void proc_childdrainlocked(proc_t);
extern void proc_childdrainstart(proc_t);
extern void proc_childdrainend(proc_t);
extern void  proc_checkdeadrefs(proc_t);
struct proc *pfind_locked(pid_t);
extern struct pgrp *pgfind(pid_t);
extern void pg_rele(struct pgrp * pgrp);
extern struct session * session_find_internal(pid_t sessid);
extern struct pgrp * proc_pgrp(proc_t);
extern struct pgrp * tty_pgrp(struct tty * tp);
extern struct pgrp * pgfind_internal(pid_t);
extern struct session * proc_session(proc_t);
extern void pgrp_lock(struct pgrp * pgrp);
extern void pgrp_unlock(struct pgrp * pgrp);
extern void session_lock(struct session * sess);
extern void session_unlock(struct session * sess);
extern struct session * pgrp_session(struct pgrp * pgrp);
extern void	session_rele(struct session *sess);
extern int isbackground(proc_t p, struct tty  *tp);
extern proc_t proc_parent(proc_t);
extern proc_t proc_parentholdref(proc_t);
extern int proc_parentdropref(proc_t, int);
int     itimerfix(struct timeval *tv);
int     itimerdecr(struct proc * p, struct itimerval *itp, int usec);
void proc_signalstart(struct proc *, int locked);
void proc_signalend(struct proc *, int locked);
int  proc_transstart(struct proc *, int locked);
void proc_transcommit(struct proc *, int locked);
void proc_transend(struct proc *, int locked);
int  proc_transwait(struct proc *, int locked);
void  proc_rele_locked(struct proc *  p);
struct proc *proc_ref_locked(struct proc *  p);
void proc_knote(struct proc * p, long hint);
void proc_knote_drain(struct proc *p);
void workqueue_init_lock(proc_t p);
void workqueue_destroy_lock(proc_t p);
void proc_setregister(proc_t p);
void proc_resetregister(proc_t p);
/* returns the first thread_t in the process, or NULL XXX for NFS, DO NOT USE */
thread_t proc_thread(proc_t);
extern int proc_pendingsignals(proc_t, sigset_t);
int proc_getpcontrol(int pid, int * pcontrolp);
int proc_dopcontrol(proc_t p, void *unused_arg);
int proc_resetpcontrol(int pid);
extern void proc_removethrottle(proc_t);
#if PSYNCH
void pth_proc_hashinit(proc_t);
void pth_proc_hashdelete(proc_t);
void pth_global_hashinit(void);
extern thread_call_t psynch_thcall;
void psynch_wq_cleanup(__unused void *  param, __unused void * param1);
extern lck_mtx_t * pthread_list_mlock;
#endif /* PSYNCH */
struct uthread * current_uthread(void);
#endif	/* !_SYS_PROC_INTERNAL_H_ */
