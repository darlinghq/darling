/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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
#ifndef _SYS_PERSONA_H_
#define _SYS_PERSONA_H_

#ifdef PRIVATE
#include <sys/param.h>

enum {
	PERSONA_INVALID      = 0,
	PERSONA_GUEST        = 1,
	PERSONA_MANAGED      = 2,
	PERSONA_PRIV         = 3,
	PERSONA_SYSTEM       = 4,
	PERSONA_DEFAULT      = 5,
	PERSONA_SYSTEM_PROXY = 6,
	PERSONA_SYS_EXT      = 7,
	PERSONA_ENTERPRISE   = 8,

	PERSONA_TYPE_MAX     = PERSONA_ENTERPRISE,
};

#define PERSONA_ID_NONE ((uid_t)-1)

struct kpersona_info {
	uint32_t persona_info_version;

	uid_t    persona_id; /* overlaps with UID */
	int      persona_type;
	gid_t    persona_gid;
	uint32_t persona_ngroups;
	gid_t    persona_groups[NGROUPS];
	uid_t    persona_gmuid;
	char     persona_name[MAXLOGNAME + 1];

	/* TODO: MAC policies?! */
};

#define PERSONA_INFO_V1       1
#define PERSONA_INFO_V1_SIZE  (sizeof(struct kpersona_info))


#define PERSONA_OP_ALLOC    1
#define PERSONA_OP_PALLOC   2
#define PERSONA_OP_DEALLOC  3
#define PERSONA_OP_GET      4
#define PERSONA_OP_INFO     5
#define PERSONA_OP_PIDINFO  6
#define PERSONA_OP_FIND     7
#define PERSONA_OP_GETPATH  8
#define PERSONA_OP_FIND_BY_TYPE  9

#define PERSONA_MGMT_ENTITLEMENT "com.apple.private.persona-mgmt"

#ifndef KERNEL
/*
 * user space persona interface
 */

/*
 * kpersona_alloc: Allocate a new in-kernel persona
 *
 * Parameters:
 *       info: Pointer to persona info structure describing the
 *             attributes of the persona to create / allocate.
 *
 *         id: output: set to the ID of the created persona
 *
 * Note:
 *      The 'persona_id' field of the 'info' parameter is ignored.
 *
 * Return:
 *        != 0: ERROR
 *        == 0: Success
 */
int kpersona_alloc(struct kpersona_info *info, uid_t *id);

/*
 * kpersona_palloc: Allocate a new in-kernel persona with a directory
 *                 pathname
 *
 * Parameters:
 *       info: Pointer to persona info structure describing the
 *             attributes of the persona to create / allocate.
 *
 *       path: Pointer to directory name that stores persona specific
 *             data. Assumes path buffer length = MAXPATHLEN and is a
 *             null-terminated string.
 *
 *         id: output: set to the ID of the created persona
 *
 * Note:
 *      The 'persona_id' field of the 'info' parameter is ignored.
 *
 * Return:
 *        != 0: ERROR
 *        == 0: Success
 */
int kpersona_palloc(struct kpersona_info *info, uid_t *id, char path[MAXPATHLEN]);

/*
 * kpersona_dealloc: delete / destroy an in-kernel persona
 *
 * Parameters:
 *         id: the ID of the persona to destroy
 *
 * Return:
 *        < 0: ERROR
 *          0: Success
 */
int kpersona_dealloc(uid_t id);

/*
 * kpersona_get: retrieve the persona with which the current thread is running
 *
 * To find the proc's persona id use kpersona_pidinfo
 *
 * Parameters:
 *         id: output: will be filled with the persona id from the voucher adopted
 *             on the current thread. If that voucher contains no persona information
 *             or there is no such voucher, then it defaults to the proc's persona id.
 *
 * Return:
 *        < 0: Thread is not running under any persona
 *          0: Success (uuid is filled with running persona UUID)
 */
int kpersona_get(uid_t *id);

/*
 * kpersona_get_path: retrieve the given persona's path
 *
 * Parameters:
 *         id: ID of the persona
 *
 *         path: output: filled in with path on success.
 *               Assumes path buffer length = MAXPATHLEN
 *
 * Return:
 *        < 0: Error
 *          0: Success
 */
int kpersona_getpath(uid_t id, char path[MAXPATHLEN]);

/*
 * kpersona_info: gather info about the given persona
 *
 * Parameters:
 *         id: ID of the persona to investigate
 *             If set to 0, it uses persona id from the voucher adopted on the current
 *             thread. If that voucher contains no persona information or there is no
 *             such voucher, then it defaults to the proc's persona id.
 *
 *       info: output: filled in with persona attributes on success.
 *
 * Return:
 *        < 0: ERROR
 *          0: Success
 */
int kpersona_info(uid_t id, struct kpersona_info *info);

/*
 * kpersona_pidinfo: gather persona info about the given PID
 *
 * Parameters:
 *        pid: PID of the process whose persona info we're to return
 *
 *       info: output: filled in with persona attributes on success.
 *
 * Return:
 *        < 0: ERROR
 *          0: Success
 */
int kpersona_pidinfo(pid_t pid, struct kpersona_info *info);

/*
 * kpersona_find: lookup the kernel's UUID of a persona
 *
 * Parameters:
 *       name: Local login name of the persona.
 *             Set this to NULL to find personas by 'uid'.
 *
 *        uid: UID of the persona.
 *             Set this to -1 to find personas by 'name'
 *
 *         id: output: the ID(s) matching the input parameters
 *             This can be NULL
 *
 *      idlen: input - size of 'id' buffer (in number of IDs)
 *             output - the total required size of the 'id' buffer
 *                      (in number of IDs) - may be larger than input size
 * Note:
 *      At least one of 'name' or 'uid' must be set.
 *
 * Return:
 *         < 0: ERROR
 *        >= 0: The number of IDs found to match the input parameters
 */
int kpersona_find(const char *name, uid_t uid, uid_t *id, size_t *idlen);

/*
 * kpersona_find_by_type: lookup the persona ids by type
 *
 * Parameters:
 *  persona_type: Type of persona id (see enum)
 *
 *           id: output: the ID(s) matching the input parameters
 *               This can be NULL
 *
 *        idlen: input - size of 'id' buffer (in number of IDs)
 *               output - the total required size of the 'id' buffer
 *                      (in number of IDs) - may be larger than input size
 * Return:
 *         < 0: ERROR
 *        >= 0: The number of IDs found to match the input parameters
 */
int kpersona_find_by_type(int persona_type, uid_t *id, size_t *idlen);
#endif /* !KERNEL */

#ifdef KERNEL_PRIVATE
/* XNU + kext private interface */
#include <sys/cdefs.h>
#include <sys/kauth.h>
#include <libkern/libkern.h>
#include <os/refcnt.h>

#ifdef PERSONA_DEBUG
#include <os/log.h>
#define persona_dbg(fmt, ...) \
	os_log(OS_LOG_DEFAULT, "[%4d] %s:  " fmt "\n", \
	       current_proc() ? current_proc()->p_pid : -1, \
	       __func__, ## __VA_ARGS__)
#else
#define persona_dbg(fmt, ...) do { } while (0)
#endif

/*
 * Persona
 */
#ifdef XNU_KERNEL_PRIVATE
/* only XNU proper needs to see the persona structure */
struct persona {
	os_refcnt_t  pna_refcount;
	int32_t      pna_valid;

	uid_t        pna_id;
	int          pna_type;
	char         pna_login[MAXLOGNAME + 1];
	char         *pna_path;

	kauth_cred_t pna_cred;
	uid_t        pna_pgid;

	int          pna_cred_locked; /* set upon first adoption */

	LIST_ENTRY(persona) pna_list;

	/* this could go away if we used a coalition */
	LIST_HEAD(, proc)   pna_members;

	lck_mtx_t    pna_lock;

	/*
	 * We can add things here such as PID maps, UID maps, etc.
	 */
#ifdef PERSONA_DEBUG
	char         pna_desc[128];
#endif
};

#define persona_lock(persona)     lck_mtx_lock(&(persona)->pna_lock)
#define persona_unlock(persona)   lck_mtx_unlock(&(persona)->pna_lock)
#define persona_try_lock(persona) lck_mtx_try_lock(&(persona)->pna_lock)

#define persona_lock_assert_held(persona) \
	LCK_MTX_ASSERT(&(persona)->pna_lock, LCK_MTX_ASSERT_OWNED)

#ifdef PERSONA_DEBUG
static inline const char *
persona_desc(struct persona *persona, int locked)
{
	if (!persona) {
		return "<none>";
	}

	if (persona->pna_desc[0] != 0) {
		return persona->pna_desc;
	}

	if (!locked) {
		persona_lock(persona);
	}
	if (persona->pna_desc[0] != 0) {
		goto out_unlock;
	}

	char *p = &persona->pna_desc[0];
	char *end = p + sizeof(persona->pna_desc) - 1;

	*end = 0;
	p += scnprintf(p, end - p, "%s/%d:%d",
	    persona->pna_login,
	    kauth_cred_getuid(persona->pna_cred),
	    kauth_cred_getgid(persona->pna_cred));

	if (p <= end) {
		*p = 0;
	}
out_unlock:
	if (!locked) {
		persona_unlock(persona);
	}

	return persona->pna_desc;
}
#else /* !PERSONA_DEBUG */
static inline const char *
persona_desc(struct persona *persona, int locked)
{
	(void)persona;
	(void)locked;
	return "<persona>";
}
#endif

#else /* !XNU_KERNEL_PRIVATE */
/* kexts should only see an opaque persona structure */
struct persona;
#endif

__BEGIN_DECLS

#ifndef _KAUTH_CRED_T
#define _KAUTH_CRED_T
typedef struct ucred *kauth_cred_t;
#endif  /* !_KAUTH_CRED_T */

/* returns the persona ID for the given pesona structure */
uid_t persona_get_id(struct persona *persona);

/* returns the type of the persona (see enum above: PERSONA_GUEST, etc.) */
int persona_get_type(struct persona *persona);

/* returns ref on kauth_cred_t that must be dropped via kauth_cred_unref() */
kauth_cred_t persona_get_cred(struct persona *persona);

/* returns a reference that must be released with persona_put() */
struct persona *persona_lookup(uid_t id);

/*
 * Search for personas based on name or uid
 *
 * Parameters:
 *       name: Local login name of the persona.
 *             Set this to NULL to find personas by 'uid'.
 *
 *        uid: UID of the persona.
 *             Set this to -1 to find personas by 'name'
 *
 *    persona: output - array of persona pointers. Each non-NULL value
 *             must* be released with persona_put. This can be NULL.
 *
 *       plen: input - size of 'persona' buffer (in number of pointers)
 *             output - the total required size of the 'persona' buffer (could be larger than input value)
 *
 * Return:
 *           0: Success
 *        != 0: failure (BSD errno value ESRCH or EINVAL)
 */
int persona_find(const char *login, uid_t uid,
    struct persona **persona, size_t *plen);

/* returns a reference that must be released with persona_put() */
struct persona *persona_proc_get(pid_t pid);

/* returns a reference to the persona tied to the current thread (also uses adopted voucher) */
struct persona *current_persona_get(void);

/* get a reference to a persona structure */
struct persona *persona_get(struct persona *persona);

/* release a reference to a persona structure */
void persona_put(struct persona *persona);

/*
 * Search for personas of a given type, 'persona_type'.
 *
 * Parameters:
 *   persona_type: Type of persona (see enum)
 *
 *        persona: output - array of persona pointers. Each non-NULL value
 *        must* be released with persona_put. This can be NULL.
 *
 *           plen: input - size of 'persona' buffer (in number of pointers)
 *                 output - the total required size of the 'persona' buffer (could be larger than input value)
 *
 * Return:
 *           0: Success
 *        != 0: failure (BSD errno value ESRCH or EINVAL)
 */
int persona_find_by_type(int persona_type, struct persona **persona,
    size_t *plen);

#ifdef XNU_KERNEL_PRIVATE

#if CONFIG_PERSONAS
#include <sys/proc_internal.h>

/*
 * In-kernel persona API
 */
extern uint32_t g_max_personas;

void personas_bootstrap(void);

struct persona *persona_alloc(uid_t id, const char *login,
    int type, char *path, int *error);

int persona_init_begin(struct persona *persona);
void persona_init_end(struct persona *persona, int error);

struct persona *persona_lookup_and_invalidate(uid_t id);
int persona_verify_and_set_uniqueness(struct persona *persona);
boolean_t persona_is_unique(struct persona *persona);

static inline int
proc_has_persona(proc_t p)
{
	if (p && p->p_persona) {
		return 1;
	}
	return 0;
}

static inline uid_t
persona_id_from_proc(proc_t p)
{
	if (p && p->p_persona) {
		return p->p_persona->pna_id;
	}
	return PERSONA_ID_NONE;
}

int persona_proc_inherit(proc_t child, proc_t parent);

int persona_proc_adopt_id(proc_t p, uid_t id,
    kauth_cred_t auth_override);
int persona_proc_adopt(proc_t p, struct persona *persona,
    kauth_cred_t auth_override);
int persona_proc_drop(proc_t p);

int persona_set_cred(struct persona *persona, kauth_cred_t cred);
int persona_set_cred_from_proc(struct persona *persona, proc_t proc);

uid_t persona_get_uid(struct persona *persona);

int persona_set_gid(struct persona *persona, gid_t gid);
gid_t persona_get_gid(struct persona *persona);

int persona_set_groups(struct persona *persona, gid_t *groups, unsigned ngroups, uid_t gmuid);
int persona_get_groups(struct persona *persona, unsigned *ngroups, gid_t *groups, unsigned groups_sz);

uid_t persona_get_gmuid(struct persona *persona);

int persona_get_login(struct persona *persona, char login[MAXLOGNAME + 1]);

/* returns a reference that must be released with persona_put() */
struct persona *persona_proc_get(pid_t pid);

int persona_find_all(const char *login, uid_t uid, int persona_type,
    struct persona **persona, size_t *plen);

#else /* !CONFIG_PERSONAS */

static inline int
proc_has_persona(__unused proc_t p)
{
	return 0;
}

static inline uid_t
persona_id_from_proc(__unused proc_t p)
{
	return PERSONA_ID_NONE;
}

#endif /* CONFIG_PERSONAS */
#endif /* XNU_KERNEL_PRIVATE */
__END_DECLS

#endif /* KERNEL_PRIVATE */

#endif /* PRIVATE */
#endif /* _SYS_PERSONA_H_ */
