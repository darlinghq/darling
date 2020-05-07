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
 *	@(#)proc.h	8.15 (Berkeley) 5/19/95
 */

#ifndef _SYS_PROC_H_
#define _SYS_PROC_H_

#include <sys/appleapiopts.h>
#include <sys/cdefs.h>
#include <sys/select.h>                 /* For struct selinfo. */
#include <sys/queue.h>
#include <sys/lock.h>
#include <sys/param.h>
#include <sys/event.h>
#include <sys/time.h>
#ifdef KERNEL
#include <sys/kernel_types.h>
#include <uuid/uuid.h>
#endif
#include <mach/boolean.h>

#ifdef XNU_KERNEL_PRIVATE
#include <mach/coalition.h>             /* COALITION_NUM_TYPES */
#endif

#if defined(XNU_KERNEL_PRIVATE) || !defined(KERNEL)

struct session;
struct pgrp;
struct proc;

/* Exported fields for kern sysctls */
struct extern_proc {
	union {
		struct {
			struct  proc *__p_forw; /* Doubly-linked run/sleep queue. */
			struct  proc *__p_back;
		} p_st1;
		struct timeval __p_starttime;   /* process start time */
	} p_un;
#define p_forw p_un.p_st1.__p_forw
#define p_back p_un.p_st1.__p_back
#define p_starttime p_un.__p_starttime
	struct  vmspace *p_vmspace;     /* Address space. */
	struct  sigacts *p_sigacts;     /* Signal actions, state (PROC ONLY). */
	int     p_flag;                 /* P_* flags. */
	char    p_stat;                 /* S* process status. */
	pid_t   p_pid;                  /* Process identifier. */
	pid_t   p_oppid;         /* Save parent pid during ptrace. XXX */
	int     p_dupfd;         /* Sideways return value from fdopen. XXX */
	/* Mach related  */
	caddr_t user_stack;     /* where user stack was allocated */
	void    *exit_thread;   /* XXX Which thread is exiting? */
	int             p_debugger;             /* allow to debug */
	boolean_t       sigwait;        /* indication to suspend */
	/* scheduling */
	u_int   p_estcpu;        /* Time averaged value of p_cpticks. */
	int     p_cpticks;       /* Ticks of cpu time. */
	fixpt_t p_pctcpu;        /* %cpu for this process during p_swtime */
	void    *p_wchan;        /* Sleep address. */
	char    *p_wmesg;        /* Reason for sleep. */
	u_int   p_swtime;        /* Time swapped in or out. */
	u_int   p_slptime;       /* Time since last blocked. */
	struct  itimerval p_realtimer;  /* Alarm timer. */
	struct  timeval p_rtime;        /* Real time. */
	u_quad_t p_uticks;              /* Statclock hits in user mode. */
	u_quad_t p_sticks;              /* Statclock hits in system mode. */
	u_quad_t p_iticks;              /* Statclock hits processing intr. */
	int     p_traceflag;            /* Kernel trace points. */
	struct  vnode *p_tracep;        /* Trace to vnode. */
	int     p_siglist;              /* DEPRECATED. */
	struct  vnode *p_textvp;        /* Vnode of executable. */
	int     p_holdcnt;              /* If non-zero, don't swap. */
	sigset_t p_sigmask;     /* DEPRECATED. */
	sigset_t p_sigignore;   /* Signals being ignored. */
	sigset_t p_sigcatch;    /* Signals being caught by user. */
	u_char  p_priority;     /* Process priority. */
	u_char  p_usrpri;       /* User-priority based on p_cpu and p_nice. */
	char    p_nice;         /* Process "nice" value. */
	char    p_comm[MAXCOMLEN + 1];
	struct  pgrp *p_pgrp;   /* Pointer to process group. */
	struct  user *p_addr;   /* Kernel virtual addr of u-area (PROC ONLY). */
	u_short p_xstat;        /* Exit status for wait; also stop signal. */
	u_short p_acflag;       /* Accounting flags. */
	struct  rusage *p_ru;   /* Exit information. XXX */
};


/* Status values. */
#define SIDL    1               /* Process being created by fork. */
#define SRUN    2               /* Currently runnable. */
#define SSLEEP  3               /* Sleeping on an address. */
#define SSTOP   4               /* Process debugging or suspension. */
#define SZOMB   5               /* Awaiting collection by parent. */

/* These flags are kept in extern_proc.p_flag. */
#define P_ADVLOCK       0x00000001      /* Process may hold POSIX adv. lock */
#define P_CONTROLT      0x00000002      /* Has a controlling terminal */
#define P_LP64          0x00000004      /* Process is LP64 */
#define P_NOCLDSTOP     0x00000008      /* No SIGCHLD when children stop */

#define P_PPWAIT        0x00000010      /* Parent waiting for chld exec/exit */
#define P_PROFIL        0x00000020      /* Has started profiling */
#define P_SELECT        0x00000040      /* Selecting; wakeup/waiting danger */
#define P_CONTINUED     0x00000080      /* Process was stopped and continued */

#define P_SUGID         0x00000100      /* Has set privileges since last exec */
#define P_SYSTEM        0x00000200      /* Sys proc: no sigs, stats or swap */
#define P_TIMEOUT       0x00000400      /* Timing out during sleep */
#define P_TRACED        0x00000800      /* Debugged process being traced */

#define P_DISABLE_ASLR  0x00001000      /* Disable address space layout randomization */
#define P_WEXIT         0x00002000      /* Working on exiting */
#define P_EXEC          0x00004000      /* Process called exec. */

/* Should be moved to machine-dependent areas. */
#define P_OWEUPC        0x00008000      /* Owe process an addupc() call at next ast. */

#define P_AFFINITY      0x00010000      /* xxx */
#define P_TRANSLATED    0x00020000      /* xxx */
#define P_CLASSIC       P_TRANSLATED    /* xxx */

#define P_DELAYIDLESLEEP 0x00040000     /* Process is marked to delay idle sleep on disk IO */
#define P_CHECKOPENEVT  0x00080000      /* check if a vnode has the OPENEVT flag set on open */

#define P_DEPENDENCY_CAPABLE    0x00100000      /* process is ok to call vfs_markdependency() */
#define P_REBOOT        0x00200000      /* Process called reboot() */
#define P_RESV6         0x00400000      /* used to be P_TBE */
#define P_RESV7         0x00800000      /* (P_SIGEXC)signal exceptions */

#define P_THCWD         0x01000000      /* process has thread cwd  */
#define P_RESV9         0x02000000      /* (P_VFORK)process has vfork children */
#define P_ADOPTPERSONA  0x04000000      /* process adopted a persona (used to be P_NOATTACH) */
#define P_RESV11        0x08000000      /* (P_INVFORK) proc in vfork */

#define P_NOSHLIB       0x10000000      /* no shared libs are in use for proc */
                                        /* flag set on exec */
#define P_FORCEQUOTA    0x20000000      /* Force quota for root */
#define P_NOCLDWAIT     0x40000000      /* No zombies when chil procs exit */
#define P_NOREMOTEHANG  0x80000000      /* Don't hang on remote FS ops */

#define P_INMEM         0               /* Obsolete: retained for compilation */
#define P_NOSWAP        0               /* Obsolete: retained for compilation */
#define P_PHYSIO        0               /* Obsolete: retained for compilation */
#define P_FSTRACE       0               /* Obsolete: retained for compilation */
#define P_SSTEP         0               /* Obsolete: retained for compilation */

#define P_DIRTY_TRACK                           0x00000001      /* track dirty state */
#define P_DIRTY_ALLOW_IDLE_EXIT                 0x00000002      /* process can be idle-exited when clean */
#define P_DIRTY_DEFER                           0x00000004      /* defer initial opt-in to idle-exit */
#define P_DIRTY                                 0x00000008      /* process is dirty */
#define P_DIRTY_SHUTDOWN                        0x00000010      /* process is dirty during shutdown */
#define P_DIRTY_TERMINATED                      0x00000020      /* process has been marked for termination */
#define P_DIRTY_BUSY                            0x00000040      /* serialization flag */
#define P_DIRTY_MARKED                          0x00000080      /* marked dirty previously */
#define P_DIRTY_AGING_IN_PROGRESS               0x00000100      /* aging in one of the 'aging bands' */
#define P_DIRTY_LAUNCH_IN_PROGRESS              0x00000200      /* launch is in progress */
#define P_DIRTY_DEFER_ALWAYS                    0x00000400      /* defer going to idle-exit after every dirty->clean transition.
	                                                         * For legacy jetsam policy only. This is the default with the other policies.*/

#define P_DIRTY_IS_DIRTY                        (P_DIRTY | P_DIRTY_SHUTDOWN)
#define P_DIRTY_IDLE_EXIT_ENABLED               (P_DIRTY_TRACK|P_DIRTY_ALLOW_IDLE_EXIT)

#endif /* XNU_KERNEL_PRIVATE || !KERNEL */

#ifdef KERNEL
__BEGIN_DECLS

extern proc_t kernproc;

extern int proc_is_classic(proc_t p);
proc_t current_proc_EXTERNAL(void);

extern int      msleep(void *chan, lck_mtx_t *mtx, int pri, const char *wmesg, struct timespec * ts );
extern void     wakeup(void *chan);
extern void wakeup_one(caddr_t chan);

/* proc kpis */
/* this routine returns the pid of the current process */
extern int proc_selfpid(void);
/* this routine returns the pid of the parent of the current process */
extern int proc_selfppid(void);
/* this routine returns the csflags of the current process */
extern int proc_selfcsflags(void);
/* this routine returns sends a signal signum to the process identified by the pid */
extern void proc_signal(int pid, int signum);
/* this routine checks whether any signal identified by the mask are pending in the  process identified by the pid. The check is  on all threads of the process. */
extern int proc_issignal(int pid, sigset_t mask);
/* this routine returns 1 if the pid1 is inferior of pid2 */
extern int proc_isinferior(int pid1, int pid2);
/* this routine copies the process's name of the executable to the passed in buffer. It
 * is always null terminated. The size of the buffer is to be passed in as well. This
 * routine is to be used typically for debugging
 */
void proc_name(int pid, char * buf, int size);
/* returns the 32-byte name if it exists, otherwise returns the 16-byte name */
extern char *proc_best_name(proc_t p);
/* This routine is simillar to proc_name except it returns for current process */
void proc_selfname(char * buf, int size);

/* find a process with a given pid. This comes with a reference which needs to be dropped by proc_rele */
extern proc_t proc_find(int pid);
/* returns a handle to current process which is referenced. The reference needs to be dropped with proc_rele */
extern proc_t proc_self(void);
/* releases the held reference on the process */
extern int proc_rele(proc_t p);
/* returns the pid of the given process */
extern int proc_pid(proc_t);
/* returns the pid of the parent of a given process */
extern int proc_ppid(proc_t);
/* returns the original pid of the parent of a given process */
extern int proc_original_ppid(proc_t);
/* returns the platform (macos, ios, watchos, tvos, ...) of the given process */
extern uint32_t proc_platform(proc_t);
/* returns the sdk version used by the current process */
extern uint32_t proc_sdk(proc_t);
/* returns 1 if the process is marked for no remote hangs */
extern int proc_noremotehang(proc_t);
/* returns 1 if the process is marked for force quota */
extern int proc_forcequota(proc_t);
/* returns 1 if the process is chrooted */
extern int proc_chrooted(proc_t);
/* returns TRUE if a sync EXC_RESOURCE should be sent for the process */
extern boolean_t proc_send_synchronous_EXC_RESOURCE(proc_t p);

/* this routine returns 1 if the process is running with a 64bit address space, else 0 */
extern int proc_is64bit(proc_t);
/* this routine returns 1 if the process is running with a 64bit register state, else 0 */
extern int proc_is64bit_data(proc_t);
/* is this process exiting? */
extern int proc_exiting(proc_t);
/* returns whether the process has started down proc_exit() */
extern int proc_in_teardown(proc_t);
/* this routine returns error if the process is not one with super user privileges */
int proc_suser(proc_t p);
/* returns the cred assicaited with the process; temporary api */
kauth_cred_t proc_ucred(proc_t p);
/* returns 1 if the process is tainted by uid or gid changes,e else 0 */
extern int proc_issetugid(proc_t p);

extern int proc_tbe(proc_t);

/*!
 *  @function proc_selfpgrpid
 *  @abstract Get the process group id for the current process, as with proc_pgrpid().
 *  @return pgrpid of current process.
 */
pid_t proc_selfpgrpid(void);

/*!
 *  @function proc_pgrpid
 *  @abstract Get the process group id for the passed-in process.
 *  @param p Process whose pgrpid to grab.
 *  @return pgrpid for "p".
 */
pid_t proc_pgrpid(proc_t p);

/*!
 *  @function proc_sessionid
 *  @abstract Get the process session id for the passed-in process.
 *  @param p Process whose session id to grab.
 *  @return session id for "p", or -1 on failure
 */
pid_t proc_sessionid(proc_t p);

#ifdef KERNEL_PRIVATE
// mark a process as being allowed to call vfs_markdependency()
void bsd_set_dependency_capable(task_t task);
#ifdef  __arm__
static inline int
IS_64BIT_PROCESS(__unused proc_t p)
{
	return 0;
}
#else
extern int IS_64BIT_PROCESS(proc_t);
#endif /* __arm__ */

extern int      tsleep(void *chan, int pri, const char *wmesg, int timo);
extern int      msleep1(void *chan, lck_mtx_t *mtx, int pri, const char *wmesg, u_int64_t timo);

task_t proc_task(proc_t);
extern int proc_pidversion(proc_t);
extern uint32_t proc_persona_id(proc_t);
extern uint32_t proc_getuid(proc_t);
extern uint32_t proc_getgid(proc_t);
extern int proc_getcdhash(proc_t, unsigned char *);

/*!
 *  @function    proc_pidbackgrounded
 *  @abstract    KPI to determine if a process is currently backgrounded.
 *  @discussion  The process may move into or out of background state at any time,
 *             so be prepared for this value to be outdated immediately.
 *  @param pid   PID of the process to be queried.
 *  @param state Pointer to a value which will be set to 1 if the process
 *             is currently backgrounded, 0 otherwise.
 *  @return      ESRCH if pid cannot be found or has started exiting.
 *
 *             EINVAL if state is NULL.
 */
extern int proc_pidbackgrounded(pid_t pid, uint32_t* state);

/*
 * This returns an unique 64bit id of a given process.
 * Caller needs to hold proper reference on the
 * passed in process strucutre.
 */
extern uint64_t proc_uniqueid(proc_t);

/* unique 64bit id for process's original parent */
extern uint64_t proc_puniqueid(proc_t);

extern void proc_set_responsible_pid(proc_t target_proc, pid_t responsible_pid);

/* return 1 if process is forcing case-sensitive HFS+ access, 0 for default */
extern int proc_is_forcing_hfs_case_sensitivity(proc_t);

/*!
 *  @function    proc_exitstatus
 *  @abstract    KPI to determine a process's exit status.
 *  @discussion  This function is not safe to call if the process could be
 *               concurrently stopped or started, but it can be called from a
 *               mpo_proc_notify_exit callback.
 *  @param p     The process to be queried.
 *  @return      Value in the same format as wait()'s output parameter.
 */
extern int proc_exitstatus(proc_t p);

#endif /* KERNEL_PRIVATE */

#ifdef XNU_KERNEL_PRIVATE

extern void proc_getexecutableuuid(proc_t, unsigned char *, unsigned long);
extern int proc_get_originatorbgstate(uint32_t *is_backgrounded);

/* Kernel interface to get the uuid of the originator of the work.*/
extern int proc_pidoriginatoruuid(uuid_t uuid_buf, uint32_t buffersize);

extern uint64_t proc_was_throttled(proc_t);
extern uint64_t proc_did_throttle(proc_t);

extern void proc_coalitionids(proc_t, uint64_t[COALITION_NUM_TYPES]);

#ifdef CONFIG_32BIT_TELEMETRY
extern void proc_log_32bit_telemetry(proc_t p);
#endif /* CONFIG_32BIT_TELEMETRY */
extern uint64_t get_current_unique_pid(void);
#endif /* XNU_KERNEL_PRIVATE*/

#ifdef KERNEL_PRIVATE
/* If buf argument is NULL, the necessary length to allocate will be set in buflen */
extern int proc_selfexecutableargs(uint8_t *buf, size_t *buflen);
extern off_t proc_getexecutableoffset(proc_t p);
extern vnode_t proc_getexecutablevnode(proc_t); /* Returned with iocount, use vnode_put() to drop */
extern int networking_memstatus_callout(proc_t p, uint32_t);

#define SYSCALL_MASK_UNIX 0
extern size_t proc_get_syscall_filter_mask_size(int which);
extern int proc_set_syscall_filter_mask(proc_t p, int which, unsigned char *maskptr, size_t masklen);
#endif /* KERNEL_PRIVATE */

__END_DECLS

#endif  /* KERNEL */

#ifdef PRIVATE

/* Values for pid_shutdown_sockets */
#define SHUTDOWN_SOCKET_LEVEL_DISCONNECT_SVC            0x00000001
#define SHUTDOWN_SOCKET_LEVEL_DISCONNECT_ALL            0x00000002

#ifdef KERNEL
#define SHUTDOWN_SOCKET_LEVEL_DISCONNECT_INTERNAL       0x10000000
#define SHUTDOWN_SOCKET_LEVEL_NECP                      0x20000000
#define SHUTDOWN_SOCKET_LEVEL_CONTENT_FILTER            0x40000000
#endif

#ifndef KERNEL

__BEGIN_DECLS

int pid_suspend(int pid);
int pid_resume(int pid);

#if defined(__arm__) || defined(__arm64__)
int pid_hibernate(int pid);
#endif /* defined(__arm__) || defined(__arm64__)  */
int pid_shutdown_sockets(int pid, int level);
int pid_shutdown_networking(int pid, int level);
__END_DECLS

#endif /* !KERNEL */

/* Entitlement to allow non-root processes to suspend/resume any task */
#define PROCESS_RESUME_SUSPEND_ENTITLEMENT "com.apple.private.process.suspend-resume.any"

#endif /* PRIVATE */

#endif  /* !_SYS_PROC_H_ */
