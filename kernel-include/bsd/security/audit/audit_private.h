/*-
 * Copyright (c) 1999-2016 Apple Inc.
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
 */
/*
 * NOTICE: This file was modified by SPARTA, Inc. in 2005 to introduce
 * support for mandatory and extensible security protections.  This notice
 * is included in support of clause 2.2 (b) of the Apple Public License,
 * Version 2.0.
 */

/*
 * This include file contains function prototypes and type definitions used
 * within the audit implementation.
 */

#ifndef _SECURITY_AUDIT_PRIVATE_H_
#define _SECURITY_AUDIT_PRIVATE_H_

#if defined(_KERNEL) || defined(KERNEL)

#if CONFIG_MACF
#include <sys/queue.h>
#include <security/mac_framework.h>
#endif

#include <sys/ipc.h>
#include <sys/proc_internal.h>  /* for PID_MAX */
#include <sys/socket.h>
#include <sys/ucred.h>

#ifdef MALLOC_DECLARE
MALLOC_DECLARE(M_AUDITBSM);
MALLOC_DECLARE(M_AUDITDATA);
MALLOC_DECLARE(M_AUDITPATH);
MALLOC_DECLARE(M_AUDITTEXT);
#endif

/*
 * Audit control variables that are usually set/read via system calls and
 * used to control various aspects of auditing.
 */
extern struct au_qctrl          audit_qctrl;
extern struct audit_fstat       audit_fstat;
extern struct au_mask           audit_nae_mask;
extern int                      audit_panic_on_write_fail;
extern int                      audit_fail_stop;
extern int                      audit_argv;
extern int                      audit_arge;
extern au_ctlmode_t     audit_ctl_mode;
extern au_expire_after_t        audit_expire_after;

/*
 * Kernel mask that is used to check to see if system calls need to be audited.
 */
extern au_class_t               audit_kevent_mask;

/*
 * The macro used to check to see if the system calls need to be auditing.
 * This will pessimisticly set the audit syscalls flag if the audit kevent
 * mask has not been created yet.  User code should build the event/class
 * mapping table before setting preselection masks to avoid this.
 */
#define AUDIT_CHECK_IF_KEVENTS_MASK(m) do {                             \
	if ((m).am_success || (m).am_failure)                           \
	        if (!audit_kevent_mask ||                               \
	            (audit_kevent_mask & (m).am_success) ||             \
	            (audit_kevent_mask & (m).am_failure))               \
	                audit_syscalls = 1;                             \
} while (0)

/*
 * Success/failure conditions for the conversion of a kernel audit record to
 * BSM format.
 */
#define BSM_SUCCESS     0
#define BSM_FAILURE     1
#define BSM_NOAUDIT     2

/*
 * Defines for the kernel audit record k_ar_commit field.  Flags are set to
 * indicate what sort of record it is, and which preselection mechanism
 * selected it.
 */
#define AR_COMMIT_KERNEL        0x00000001U
#define AR_COMMIT_USER          0x00000010U

#define AR_PRESELECT_TRAIL      0x00001000U
#define AR_PRESELECT_PIPE       0x00002000U

#define AR_PRESELECT_USER_TRAIL 0x00004000U
#define AR_PRESELECT_USER_PIPE  0x00008000U

#define AR_PRESELECT_FILTER     0x00010000U

#define AR_DRAIN_QUEUE          0x80000000U

/*
 * Audit data is generated as a stream of struct audit_record structures,
 * linked by struct kaudit_record, and contain storage for possible audit so
 * that it will not need to be allocated during the processing of a system
 * call, both improving efficiency and avoiding sleeping at untimely moments.
 * This structure is converted to BSM format before being written to disk.
 */
struct vnode_au_info {
	mode_t  vn_mode;
	uid_t   vn_uid;
	gid_t   vn_gid;
	dev_t   vn_dev;
	long    vn_fsid;
	long    vn_fileid;
	long    vn_gen;
};

struct groupset {
	gid_t   gidset[NGROUPS];
	u_int   gidset_size;
};

struct socket_au_info {
	int                     sai_domain;
	int                     sai_type;
	int                     sai_protocol;

	/* Foreign (remote) address/port. */
	struct sockaddr_storage sai_faddr;

	/* Local address/port. */
	struct sockaddr_storage sai_laddr;
};

/*
 *  The following is used for A_OLDSETQCTRL and A_OLDGETQCTRL and a 64-bit
 *  userland.
 */
struct  au_qctrl64 {
	u_int64_t               aq64_hiwater;
	u_int64_t               aq64_lowater;
	u_int64_t               aq64_bufsz;
	u_int64_t               aq64_delay;
	int64_t                 aq64_minfree;
};
typedef struct au_qctrl64 au_qctrl64_t;

union auditon_udata {
	char                    *au_path;
	int                     au_cond;
	int                     au_policy;
	int64_t                 au_cond64;
	int64_t                 au_policy64;
	int                     au_trigger;
	au_evclass_map_t        au_evclass;
	au_mask_t               au_mask;
	au_asflgs_t             au_flags;
	auditinfo_t             au_auinfo;
	auditpinfo_t            au_aupinfo;
	auditpinfo_addr_t       au_aupinfo_addr;
	au_qctrl_t              au_qctrl;
	au_qctrl64_t            au_qctrl64;
	au_stat_t               au_stat;
	au_fstat_t              au_fstat;
	auditinfo_addr_t        au_kau_info;
	au_ctlmode_t    au_ctl_mode;
	au_expire_after_t       au_expire_after;
};

struct posix_ipc_perm {
	uid_t   pipc_uid;
	gid_t   pipc_gid;
	mode_t  pipc_mode;
};

struct au_identity_info {
	u_int32_t       signer_type;
	char            *signing_id;
	u_char          signing_id_trunc;
	char            *team_id;
	u_char          team_id_trunc;
	u_int8_t        *cdhash;
	u_int16_t       cdhash_len;
};

struct audit_record {
	/* Audit record header. */
	u_int32_t               ar_magic;
	int                     ar_event;
	int                     ar_retval; /* value returned to the process */
	int                     ar_errno;  /* return status of system call */
	struct timespec         ar_starttime;
	struct timespec         ar_endtime;
	u_int64_t               ar_valid_arg;  /* Bitmask of valid arguments */

	/* Audit subject information. */
	struct xucred           ar_subj_cred;
	uid_t                   ar_subj_ruid;
	gid_t                   ar_subj_rgid;
	gid_t                   ar_subj_egid;
	uid_t                   ar_subj_auid; /* Audit user ID */
	pid_t                   ar_subj_asid; /* Audit session ID */
	pid_t                   ar_subj_pid;
	struct au_tid           ar_subj_term;
	struct au_tid_addr      ar_subj_term_addr;
	struct au_mask          ar_subj_amask;

	/* Operation arguments. */
	uid_t                   ar_arg_euid;
	uid_t                   ar_arg_ruid;
	uid_t                   ar_arg_suid;
	gid_t                   ar_arg_egid;
	gid_t                   ar_arg_rgid;
	gid_t                   ar_arg_sgid;
	pid_t                   ar_arg_pid;
	pid_t                   ar_arg_asid;
	struct au_tid           ar_arg_termid;
	struct au_tid_addr      ar_arg_termid_addr;
	uid_t                   ar_arg_uid;
	uid_t                   ar_arg_auid;
	gid_t                   ar_arg_gid;
	struct groupset         ar_arg_groups;
	int                     ar_arg_fd;
	int                     ar_arg_fflags;
	mode_t                  ar_arg_mode;
	uint32_t                ar_arg_value32;
	uint64_t                ar_arg_value64;
	user_addr_t             ar_arg_addr;
	user_size_t             ar_arg_len;
	int                     ar_arg_mask;
	u_int                   ar_arg_signum;
	char                    ar_arg_login[MAXLOGNAME];
	int                     ar_arg_ctlname[CTL_MAXNAME];
	struct socket_au_info   ar_arg_sockinfo;
	char                    *ar_arg_upath1;
	char                    *ar_arg_upath2;
	char                    *ar_arg_kpath1;         /* darwin-only */
	char                    *ar_arg_kpath2;         /* darwin-only */
#if CONFIG_MACF
	char                    *ar_vnode1_mac_labels;
	char                    *ar_vnode2_mac_labels;
	char                    *ar_cred_mac_labels;
	char                    *ar_arg_mac_string;
#endif
	char                    *ar_arg_text;
	void                    *ar_arg_opaque;         /* darwin-only */
	void                    *ar_arg_data;           /* darwin-only */
	u_int16_t               ar_arg_opq_size;        /* darwin-only */
	u_char                  ar_arg_data_type;       /* darwin-only */
	u_char                  ar_arg_data_count;      /* darwin-only */
	struct au_mask          ar_arg_amask;
	struct vnode_au_info    ar_arg_vnode1;
	struct vnode_au_info    ar_arg_vnode2;
	int                     ar_arg_cmd;
	int                     ar_arg_svipc_cmd;
	struct ipc_perm         ar_arg_svipc_perm;
	int                     ar_arg_svipc_id;
	user_addr_t             ar_arg_svipc_addr;
	struct posix_ipc_perm   ar_arg_pipc_perm;
	mach_port_name_t        ar_arg_mach_port1;      /* darwin-only */
	mach_port_name_t        ar_arg_mach_port2;      /* darwin-only */
	union auditon_udata     ar_arg_auditon;
	char                    *ar_arg_argv;
	int                     ar_arg_argc;
	char                    *ar_arg_envv;
	int                     ar_arg_envc;
	int                     ar_arg_exitstatus;
	int                     ar_arg_exitretval;
	struct sockaddr_storage ar_arg_sockaddr;
	int                     ar_arg_fd2;

#if CONFIG_MACF
	/*
	 * MAC security related fields added by MAC policies ar_forced_by_mac
	 * is 1 if mac_audit_check_preselect() forced this call to be audited,
	 * 0 otherwise.
	 */
	LIST_HEAD(mac_audit_record_list_t, mac_audit_record)    * ar_mac_records;
	int                     ar_forced_by_mac;
#endif
	struct au_identity_info ar_arg_identity;
};

/*
 * Arguments in the audit record are initially not defined; flags are set to
 * indicate if they are present so they can be included in the audit log
 * stream only if defined.
 */
#define ARG_IS_VALID(kar, arg)  ((kar)->k_ar.ar_valid_arg & (arg))
#define ARG_SET_VALID(kar, arg) do {                                    \
	(kar)->k_ar.ar_valid_arg |= (arg);                              \
} while (0)

/*
 * Current thread macro.  get_bsdthread_info() returns a void ptr for some
 * reason.
 */
#define curthread()     ((struct uthread *)get_bsdthread_info(current_thread()))

/*
 * In-kernel version of audit record; the basic record plus queue meta-data.
 * This record can also have a pointer set to some opaque data that will be
 * passed through to the audit writing mechanism.
 */
struct kaudit_record {
	struct audit_record              k_ar;
	u_int32_t                        k_ar_commit;
	void                            *k_udata;       /* User data. */
	u_int                            k_ulen;        /* User data length. */
	struct uthread                  *k_uthread;     /* Audited thread. */
	TAILQ_ENTRY(kaudit_record)       k_q;
};
TAILQ_HEAD(kaudit_queue, kaudit_record);

/*
 * Functions to manage the allocation, release, and commit of kernel audit
 * records.
 */
void                     audit_abort(struct kaudit_record *ar);
void                     audit_commit(struct kaudit_record *ar, int error,
    int retval);
struct kaudit_record    *audit_new(int event, proc_t p, struct uthread *td);

/*
 * Functions relating to the conversion of internal kernel audit records to
 * the BSM file format.
 */
struct au_record;
int      kaudit_to_bsm(struct kaudit_record *kar, struct au_record **pau);
int      bsm_rec_verify(void *rec, int length, boolean_t kern_events_allowed);

/*
 * Kernel versions of the libbsm audit record functions.
 */
void     kau_free(struct au_record *rec);
void     kau_init(void);

/*
 * Return values for pre-selection and post-selection decisions.
 */
#define AU_PRS_SUCCESS  1
#define AU_PRS_FAILURE  2
#define AU_PRS_BOTH     (AU_PRS_SUCCESS|AU_PRS_FAILURE)

/*
 * Data structures relating to the kernel audit queue.  Ideally, these might
 * be abstracted so that only accessor methods are exposed.
 */
extern struct mtx               audit_mtx;
extern struct cv                audit_watermark_cv;
extern struct cv                audit_worker_cv;
extern struct cv                audit_drain_cv;
extern struct kaudit_queue      audit_q;
extern int                      audit_q_len;
extern int                      audit_pre_q_len;
extern int                      audit_in_failure;

/*
 * Flags to use on audit files when opening and closing.
 */
#define AUDIT_OPEN_FLAGS        (FWRITE | O_APPEND)
#define AUDIT_CLOSE_FLAGS       (FWRITE | O_APPEND)

#include <sys/fcntl.h>
#include <sys/kernel.h>
#include <sys/malloc.h>

/*
 * Some of the BSM tokenizer functions take different parameters in the
 * kernel implementations in order to save the copying of large kernel data
 * structures.  The prototypes of these functions are declared here.
 */
token_t         *kau_to_socket(struct socket_au_info *soi);

/*
 * audit_klib prototypes
 */
int              au_preselect(au_event_t event, au_class_t class,
    au_mask_t *mask_p, int sorf);
void             au_evclassmap_init(void);
void             au_evclassmap_insert(au_event_t event, au_class_t class);
au_class_t       au_event_class(au_event_t event);
au_event_t       audit_ctlname_to_sysctlevent(int name[], uint64_t valid_arg);
au_event_t       audit_flags_and_error_to_openevent(int oflags, int error);
au_event_t       audit_flags_and_error_to_openextendedevent(int oflags,
    int error);
au_event_t       audit_flags_and_error_to_openatevent(int oflags,
    int error);
au_event_t       audit_flags_and_error_to_openbyidevent(int oflags,
    int error);
au_event_t       audit_msgctl_to_event(int cmd);
au_event_t       audit_semctl_to_event(int cmr);
int              audit_canon_path(struct vnode *cwd_vp, char *path,
    char *cpath);
au_event_t       auditon_command_event(int cmd);
au_event_t       audit_fcntl_command_event(int cmd, int oflags, int error);

/*
 * Audit trigger events notify user space of kernel audit conditions
 * asynchronously.
 */
int              audit_send_trigger(unsigned int trigger);

/*
 * Accessor functions to manage global audit state.
 */
void            audit_set_kinfo(struct auditinfo_addr *);
void            audit_get_kinfo(struct auditinfo_addr *);

/*
 * General audit related functions.
 */
struct kaudit_record    *currecord(void);
void                     audit_free(struct kaudit_record *ar);
void                     audit_rotate_vnode(struct ucred *cred,
    struct vnode *vp);
void                     audit_worker_init(void);
void                     audit_identity_info_construct(
	struct au_identity_info *id_info);
void                     audit_identity_info_destruct(
	struct au_identity_info *id_info);

/*
 * Audit pipe functions.
 */
int      audit_pipe_init(void);
int      audit_pipe_shutdown(void);
int      audit_pipe_preselect(au_id_t auid, au_event_t event,
    au_class_t class, int sorf, int trail_select);
void     audit_pipe_submit(au_id_t auid, au_event_t event, au_class_t class,
    int sorf, int trail_select, void *record, u_int record_len);
void     audit_pipe_submit_user(void *record, u_int record_len);

/*
 * Audit MAC prototypes.
 */
void    audit_mac_init(void);
int     audit_mac_new(proc_t p, struct kaudit_record *ar);
void    audit_mac_free(struct kaudit_record *ar);
int     audit_mac_syscall_enter(unsigned short code, proc_t p,
    struct uthread *uthread, kauth_cred_t my_cred, au_event_t event);
int     audit_mac_syscall_exit(unsigned short code, struct uthread *uthread,
    int error, int retval);

/*
 * Audit Session.
 */
void    audit_session_init(void);
int     audit_session_setaia(proc_t p, auditinfo_addr_t *aia_p);
auditinfo_addr_t *audit_session_update(auditinfo_addr_t *new_aia);
int     audit_session_lookup(au_asid_t asid, auditinfo_addr_t *ret_aia);

/*
 * Kernel assigned audit session IDs start at PID_MAX + 1 and ends at
 * ASSIGNED_ASID_MAX.
 */
#define ASSIGNED_ASID_MIN       (PID_MAX + 1)
#define ASSIGNED_ASID_MAX       (0xFFFFFFFF - 1)

/*
 * Entitlement required to control various audit subsystem settings
 */
#define AU_CLASS_RESERVED_ENTITLEMENT "com.apple.private.dz.audit"

/*
 * Entitlement required to control auditctl sys call
 */
#define AU_AUDITCTL_RESERVED_ENTITLEMENT "com.apple.private.protected-audit-control"

/*
 * Entitlement required to control auditctl sys call
 */
#define AU_AUDIT_USER_ENTITLEMENT "com.apple.private.audit.user"

/*
 * Max sizes used by the kernel for signing id and team id values of the
 * identity tokens. These lengths include space for the null terminator.
 */
#define MAX_AU_IDENTITY_SIGNING_ID_LENGTH 129
#define MAX_AU_IDENTITY_TEAM_ID_LENGTH 17

struct __attribute__((__packed__)) hdr_tok_partial {
	u_char type;
	uint32_t len;
	u_char ver;
	uint16_t e_type;
};
static_assert(sizeof(struct hdr_tok_partial) == 8);

struct __attribute__((__packed__)) trl_tok_partial {
	u_char type;
	uint16_t magic;
	uint32_t len;
};
static_assert(sizeof(struct trl_tok_partial) == 7);

#endif /* defined(KERNEL) || defined(_KERNEL) */

#endif /* ! _SECURITY_AUDIT_PRIVATE_H_ */
