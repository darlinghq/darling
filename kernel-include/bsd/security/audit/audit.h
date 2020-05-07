/*-
 * Copyright (c) 2004-2019 Apple Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */
/*
 * NOTICE: This file was modified by SPARTA, Inc. in 2005 to introduce
 * support for mandatory and extensible security protections.  This notice
 * is included in support of clause 2.2 (b) of the Apple Public License,
 * Version 2.0.
 */

/*
 * This header includes function prototypes and type definitions that are
 * necessary for the kernel as a whole to interact with the audit subsystem.
 */

#ifndef _SECURITY_AUDIT_AUDIT_H
#define _SECURITY_AUDIT_AUDIT_H

#if defined(_KERNEL) || defined(KERNEL)

#include <bsm/audit.h>

#include <sys/sysctl.h>
#include <sys/user.h>
#include <sys/ipc.h>

/*
 * Audit subsystem condition flags.  The audit_enabled flag is set and
 * removed automatically as a result of configuring log files, and can be
 * observed but should not be directly manipulated.  The audit suspension
 * flag permits audit to be temporarily disabled without reconfiguring the
 * audit target. The audit syscalls flag is set at the first hint that kernel
 * events (system and mach calls) need to be audited.  It is used for
 * performance so an event class map table lookup doesn't have be done for
 * every system call if only user events are being audited.
 */
extern int      audit_enabled;
extern int      audit_suspended;
extern int      audit_syscalls;

/*
 * Define the masks for the audited arguments.
 *
 * XXXRW: These need to remain in audit.h for now because our vnode and name
 * lookup audit calls rely on passing in flags to indicate which name or
 * vnode is being logged.  These should move to audit_private.h when that is
 * fixed.
 */
#define ARG_EUID                0x0000000000000001ULL
#define ARG_RUID                0x0000000000000002ULL
#define ARG_SUID                0x0000000000000004ULL
#define ARG_EGID                0x0000000000000008ULL
#define ARG_RGID                0x0000000000000010ULL
#define ARG_SGID                0x0000000000000020ULL
#define ARG_PID                 0x0000000000000040ULL
#define ARG_UID                 0x0000000000000080ULL
#define ARG_AUID                0x0000000000000100ULL
#define ARG_GID                 0x0000000000000200ULL
#define ARG_FD                  0x0000000000000400ULL
#define ARG_FD1                 ARG_FD
#define ARG_POSIX_IPC_PERM      0x0000000000000800ULL
#define ARG_FFLAGS              0x0000000000001000ULL
#define ARG_MODE                0x0000000000002000ULL
#define ARG_VALUE32             0x0000000000004000ULL
#define ARG_ADDR32              0x0000000000008000ULL
#define ARG_ADDR                ARG_ADDR32
#define ARG_LEN                 0x0000000000010000ULL
#define ARG_MASK                0x0000000000020000ULL
#define ARG_SIGNUM              0x0000000000040000ULL
#define ARG_LOGIN               0x0000000000080000ULL
#define ARG_SADDRINET           0x0000000000100000ULL
#define ARG_SADDRINET6          0x0000000000200000ULL
#define ARG_SADDRUNIX           0x0000000000400000ULL
#define ARG_TERMID_ADDR         ARG_SADDRUNIX
#define ARG_KPATH1              0x0000000000800000ULL   /* darwin-only */
#define ARG_KPATH2              0x0000000001000000ULL   /* darwin-only */
#define ARG_UPATH1              0x0000000002000000ULL
#define ARG_UPATH2              0x0000000004000000ULL
#define ARG_TEXT                0x0000000008000000ULL
#define ARG_VNODE1              0x0000000010000000ULL
#define ARG_VNODE2              0x0000000020000000ULL
#define ARG_SVIPC_CMD           0x0000000040000000ULL
#define ARG_SVIPC_PERM          0x0000000080000000ULL
#define ARG_SVIPC_ID            0x0000000100000000ULL
#define ARG_SVIPC_ADDR          0x0000000200000000ULL
#define ARG_GROUPSET            0x0000000400000000ULL
#define ARG_CMD                 0x0000000800000000ULL
#define ARG_SOCKINFO            0x0000001000000000ULL
#define ARG_ASID                0x0000002000000000ULL
#define ARG_TERMID              0x0000004000000000ULL
#define ARG_AUDITON             0x0000008000000000ULL
#define ARG_VALUE64             0x0000010000000000ULL   /* darwin-only */
#define ARG_AMASK               0x0000020000000000ULL
#define ARG_CTLNAME             0x0000040000000000ULL
#define ARG_PROCESS             0x0000080000000000ULL
#define ARG_MACHPORT1           0x0000100000000000ULL
#define ARG_MACHPORT2           0x0000200000000000ULL
#define ARG_MAC_STRING          0x0000400000000000ULL
#define ARG_EXIT                0x0000800000000000ULL
#define ARG_IOVECSTR            0x0001000000000000ULL
#define ARG_ARGV                0x0002000000000000ULL
#define ARG_ENVV                0x0004000000000000ULL
#define ARG_OPAQUE              0x0008000000000000ULL   /* darwin-only */
#define ARG_DATA                0x0010000000000000ULL   /* darwin-only */
#define ARG_ADDR64              0x0020000000000000ULL   /* darwin-only */
#define ARG_FD2                 0x0040000000000000ULL   /* darwin-only */
#define ARG_IDENTITY            0x0080000000000000ULL   /* darwin-only */
#define ARG_NONE                0x0000000000000000ULL
#define ARG_ALL                 0xFFFFFFFFFFFFFFFFULL

#if CONFIG_MACF

#define MAC_AUDIT_LABEL_LEN 1024
#define MAC_AUDIT_DATA_TYPE 0
#define MAC_AUDIT_TEXT_TYPE 1

struct mac_audit_record {
	int type;               /* one of the types defined above */
	int length;             /* byte length of the data field */
	u_char *data;           /* the payload */
	LIST_ENTRY(mac_audit_record) records;
};

#endif

struct proc;
struct vnode;
struct componentname;

int                      kau_will_audit(void);
void                     audit_init(void);
void                     audit_shutdown(void);
void                     audit_syscall_enter(unsigned int code,
    struct proc *proc, struct uthread *uthread);
#if CONFIG_MACF
/*
 * The parameter list of audit_syscall_exit() was modified to also take the
 * Darwin syscall number, which is required by mac_audit_check_postselect().
 */
void    audit_syscall_exit(unsigned int code, int error,
    struct proc *proc, struct uthread *uthread);
#else
void    audit_syscall_exit(int error, struct proc *proc,
    struct uthread *uthread);
#endif
void    audit_mach_syscall_enter(unsigned short audit_event);
void    audit_mach_syscall_exit(int retval, struct uthread *uthread);

void                    audit_subcall_enter(au_event_t event,
    struct proc *proc, struct uthread *uthread);
void                    audit_subcall_exit(int error,
    struct uthread *uthread);

extern struct auditinfo_addr *audit_default_aia_p;

/*
 * The remaining kernel functions are conditionally compiled in as they are
 * wrapped by a macro, and the macro should be the only place in the source
 * tree where these functions are referenced.
 */
#if CONFIG_AUDIT
struct ipc_perm;
struct sockaddr;
union auditon_udata;
void     audit_arg_addr(struct kaudit_record *ar, user_addr_t addr);
void     audit_arg_exit(struct kaudit_record *ar, int status, int retval);
void     audit_arg_len(struct kaudit_record *ar, user_size_t len);
void     audit_arg_fd(struct kaudit_record *ar, int fd);
void     audit_arg_fd2(struct kaudit_record *ar, int fd);
void     audit_arg_fflags(struct kaudit_record *ar, int fflags);
void     audit_arg_gid(struct kaudit_record *ar, gid_t gid);
void     audit_arg_uid(struct kaudit_record *ar, uid_t uid);
void     audit_arg_egid(struct kaudit_record *ar, gid_t egid);
void     audit_arg_euid(struct kaudit_record *ar, uid_t euid);
void     audit_arg_rgid(struct kaudit_record *ar, gid_t rgid);
void     audit_arg_ruid(struct kaudit_record *ar, uid_t ruid);
void     audit_arg_sgid(struct kaudit_record *ar, gid_t sgid);
void     audit_arg_suid(struct kaudit_record *ar, uid_t suid);
void     audit_arg_groupset(struct kaudit_record *ar, gid_t *gidset,
    u_int gidset_size);
void     audit_arg_login(struct kaudit_record *ar, char *login);
void     audit_arg_ctlname(struct kaudit_record *ar, int *name, int namelen);
void     audit_arg_mask(struct kaudit_record *ar, int mask);
void     audit_arg_mode(struct kaudit_record *ar, mode_t mode);
void     audit_arg_value32(struct kaudit_record *ar, uint32_t value32);
void     audit_arg_value64(struct kaudit_record *ar, uint64_t value64);
void     audit_arg_owner(struct kaudit_record *ar, uid_t uid, gid_t gid);
void     audit_arg_pid(struct kaudit_record *ar, pid_t pid);
void     audit_arg_process(struct kaudit_record *ar, proc_t p);
void     audit_arg_signum(struct kaudit_record *ar, u_int signum);
void     audit_arg_socket(struct kaudit_record *ar, int sodomain, int sotype,
    int soprotocol);
void     audit_arg_sockaddr(struct kaudit_record *ar, struct vnode *cwd_vp,
    struct sockaddr *so);
void     audit_arg_auid(struct kaudit_record *ar, uid_t auid);
void     audit_arg_auditinfo(struct kaudit_record *ar,
    struct auditinfo *au_info);
void     audit_arg_auditinfo_addr(struct kaudit_record *ar,
    struct auditinfo_addr *au_info);
void     audit_arg_upath(struct kaudit_record *ar, struct vnode *cwd_vp,
    char *upath, u_int64_t flags);
void     audit_arg_kpath(struct kaudit_record *ar,
    char *kpath, u_int64_t flags);
void     audit_arg_vnpath(struct kaudit_record *ar, struct vnode *vp,
    u_int64_t flags);
void     audit_arg_vnpath_withref(struct kaudit_record *ar, struct vnode *vp,
    u_int64_t flags);
void     audit_arg_text(struct kaudit_record *ar, char *text);
void     audit_arg_opaque(struct kaudit_record *ar, void *data, size_t size);
void     audit_arg_data(struct kaudit_record *ar, void *data, size_t size,
    size_t number);
void     audit_arg_cmd(struct kaudit_record *ar, int cmd);
void     audit_arg_svipc_cmd(struct kaudit_record *ar, int cmd);
void     audit_arg_svipc_perm(struct kaudit_record *ar, struct ipc_perm *perm);
void     audit_arg_svipc_id(struct kaudit_record *ar, int id);
void     audit_arg_svipc_addr(struct kaudit_record *ar, user_addr_t addr);
void     audit_arg_posix_ipc_perm(struct kaudit_record *ar, uid_t uid,
    gid_t gid, mode_t mode);
void     audit_arg_auditon(struct kaudit_record *ar,
    union auditon_udata *udata);
void     audit_arg_file(struct kaudit_record *ar, struct proc *p,
    struct fileproc *fp);
void     audit_arg_argv(struct kaudit_record *ar, char *argv, int argc,
    int length);
void     audit_arg_envv(struct kaudit_record *ar, char *envv, int envc,
    int length);
void    audit_arg_identity(struct kaudit_record *ar);

void     audit_arg_mach_port1(struct kaudit_record *ar, mach_port_name_t port);
void     audit_arg_mach_port2(struct kaudit_record *ar, mach_port_name_t port);
void     audit_sysclose(struct kaudit_record *ar, struct proc *p, int fd);

void     audit_proc_coredump(proc_t proc, char *path, int errcode);
void     audit_proc_init(struct proc *p);
void     audit_proc_fork(struct proc *parent, struct proc *child);
void     audit_proc_free(struct proc *p);

#ifndef _KAUTH_CRED_T
#define _KAUTH_CRED_T
struct ucred;
typedef struct ucred *kauth_cred_t;
#endif /* !_KAUTH_CRED_T */

void     audit_session_ref(kauth_cred_t cred);
void     audit_session_unref(kauth_cred_t cred);
void     audit_session_procnew(proc_t p);
void     audit_session_procexit(proc_t p);
int      audit_session_spawnjoin(proc_t p, task_t task, ipc_port_t port);

void     audit_sdev_submit(au_id_t auid, au_asid_t asid, void *record,
    u_int record_len);

/*
 * Audit session macros.
 */
#define IS_VALID_SESSION(a)     ((a) != NULL && (a) != audit_default_aia_p)

#define AUDIT_SESSION_REF(cred)         audit_session_ref(cred)
#define AUDIT_SESSION_UNREF(cred)       audit_session_unref(cred)

#define AUDIT_SESSION_PROCNEW(p)        audit_session_procnew(p)
#define AUDIT_SESSION_PROCEXIT(p)       audit_session_procexit(p)

#if CONFIG_MACF
/*
 * audit_mac_data() is the MAC Framework's entry point to the audit subsystem.
 * It currently creates only text and data audit tokens.
 */
int      audit_mac_data(int type, int len, u_char *data);
void     audit_arg_mac_string(struct kaudit_record *ar, char *string);

#endif

extern au_event_t sys_au_event[];

#define AUDIT_RECORD() \
	((struct uthread*)get_bsdthread_info(current_thread()))->uu_ar

#ifndef AUDIT_USE_BUILTIN_EXPECT
#define AUDIT_USE_BUILTIN_EXPECT
#endif

#ifdef  AUDIT_USE_BUILTIN_EXPECT
/*
 * Use branch prediction for the case of auditing enabled but not
 * auditing system calls.
 */
#define AUDIT_SYSCALLS()        __builtin_expect(audit_syscalls, 0)
#define AUDIT_ENABLED()         __builtin_expect(audit_syscalls &&      \
	                                audit_enabled, 0)
#define AUDIT_AUDITING(x)       __builtin_expect(NULL != (x), 0)

#else

#define AUDIT_SYSCALLS()        (audit_syscalls)
#define AUDIT_ENABLED()         (audit_syscalls && audit_enabled)
#define AUDIT_AUDITING(x)       (NULL != (x))

#endif /* AUDIT_USE_BUILTIN_EXPECT */

#define AUDIT_RECORD_EXISTS() (AUDIT_ENABLED() && AUDIT_AUDITING(AUDIT_RECORD()))

/*
 * Define a macro to wrap the audit_arg_* calls by checking the global
 * audit_enabled flag before performing the actual call.
 */
#define AUDIT_ARG(op, args...)  do {                                    \
	if (AUDIT_SYSCALLS()) {                                         \
	        struct kaudit_record *__ar = AUDIT_RECORD();            \
	        if (AUDIT_AUDITING(__ar))                               \
	                audit_arg_ ## op (__ar, ## args);               \
	}                                                               \
} while (0)

#define AUDIT_SYSCALL_ENTER(args...)    do {                            \
	if (AUDIT_ENABLED()) {                                  \
	        audit_syscall_enter(args);                              \
	}                                                               \
} while (0)

/*
 * Wrap the audit_syscall_exit() function so that it is called only when
 * we have a audit record on the thread.  Audit records can persist after
 * auditing is disabled, so we don't just check audit_enabled here.
 */
#define AUDIT_SYSCALL_EXIT(code, proc, uthread, error)  do {            \
	        if (AUDIT_AUDITING(uthread->uu_ar))                     \
	                audit_syscall_exit(code, error, proc, uthread); \
} while (0)

#define AUDIT_SUBCALL_ENTER(event, proc, uthread)  do {                 \
	if (AUDIT_ENABLED())                                            \
	        audit_subcall_enter(AUE_ ## event, proc, uthread);      \
} while (0)

#define AUDIT_SUBCALL_EXIT(uthread, error)  do {                        \
	        if (AUDIT_AUDITING(uthread->uu_ar))                     \
	                audit_subcall_exit(error, uthread);             \
} while (0)

/*
 * Wrap the audit_mach_syscall_enter() and audit_mach_syscall_exit()
 * functions in a manner similar to other system call enter/exit functions.
 */
#define AUDIT_MACH_SYSCALL_ENTER(args...)       do {                    \
	if (AUDIT_ENABLED()) {                                  \
	        audit_mach_syscall_enter(args);                         \
	}                                                               \
} while (0)

#define AUDIT_MACH_SYSCALL_EXIT(retval)         do {                    \
	if (AUDIT_SYSCALLS()) {                                         \
	        struct uthread *__uthread =                             \
	                get_bsdthread_info(current_thread());           \
	        if (AUDIT_AUDITING(__uthread->uu_ar))                   \
	                audit_mach_syscall_exit(retval, __uthread);     \
	}                                                               \
} while (0)

/*
 * A Macro to wrap the audit_sysclose() function.
 */
#define AUDIT_SYSCLOSE(args...) do {                                    \
	if (AUDIT_SYSCALLS()) {                                         \
	        struct kaudit_record *__ar = AUDIT_RECORD();            \
	        if (AUDIT_AUDITING(__ar))                               \
	                audit_sysclose(__ar, args);                     \
	}                                                               \
} while (0)

#else /* !CONFIG_AUDIT */

#define AUDIT_ARG(op, args...)  do {                                    \
} while (0)

#define AUDIT_SYSCALL_ENTER(args...)    do {                            \
} while (0)

#define AUDIT_SYSCALL_EXIT(code, proc, uthread, error)  do {            \
} while (0)

#define AUDIT_SUBCALL_ENTER(event, proc, uthread)    do {               \
} while (0)

#define AUDIT_SUBCALL_EXIT(uthread, error)  do {                        \
} while (0)

#define AUDIT_MACH_SYSCALL_ENTER(args...)       do {                    \
} while (0)

#define AUDIT_MACH_SYSCALL_EXIT(retval)         do {                    \
} while (0)

#define AUDIT_SYSCLOSE(op, args...)     do {                            \
} while (0)

#define AUDIT_SESSION_REF(cred)         do {                            \
} while (0)

#define AUDIT_SESSION_UNREF(cred)       do {                            \
} while (0)

#define AUDIT_SESSION_PROCNEW(cred)     do {                            \
} while (0)

#define AUDIT_SESSION_PROCEXIT(cred)    do {                            \
} while (0)

#define AUDIT_SESSION_REF(cred)         do {                            \
} while (0)

#define AUDIT_SESSION_UNREF(cred)       do {                            \
} while (0)

#define AUDIT_SESSION_PROCNEW(cred)     do {                            \
} while (0)

#define AUDIT_SESSION_PROCEXIT(cred)    do {                            \
} while (0)

#endif /* CONFIG_AUDIT */

#endif /* KERNEL */

#endif /* !_SECURITY_AUDIT_ADUIT_H */
