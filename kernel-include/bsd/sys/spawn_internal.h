/*
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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


/*
 * [SPN] Support for _POSIX_SPAWN
 *
 * This file contains internal data structures which are externally represented
 * as opaque void pointers to prevent introspection.  This permits us to
 * change the underlying implementation of the code to maintain it or to
 * support new features, as needed, without the consumer needing to recompile
 * their code because of structure size changes or data reorganization.
 */

#ifndef _SYS_SPAWN_INTERNAL_H_
#define _SYS_SPAWN_INTERNAL_H_

#include <sys/_types.h>         /* __offsetof(), __darwin_size_t */
#include <sys/param.h>
#include <sys/syslimits.h>      /* PATH_MAX */
#include <sys/spawn.h>
#include <mach/machine.h>
#include <mach/port.h>
#include <mach/exception_types.h>
#include <mach/coalition.h>     /* COALITION_NUM_TYPES */
#include <os/overflow.h>

/*
 * Safely compute the size in bytes of a structure, '_type', whose last
 * element, '_member', is a zero-sized array meant to hold 'x' bytes.
 *
 * If the size calculation overflows a size_t value, this macro returns 0.
 */
#define PS_ACTION_SIZE(x, _type, _member_type) ({\
	size_t _ps_count = (size_t)x; \
	size_t _ps_size = 0; \
	/* (count * sizeof(_member_type)) + sizeof(_type) */ \
	if (os_mul_and_add_overflow(_ps_count, \
	                            sizeof(_member_type), \
	                            sizeof(_type), \
	                            &_ps_size)) { \
	        _ps_size = 0; \
	} \
	_ps_size; })

/*
 * Allowable posix_spawn() port action types
 */
typedef enum {
	PSPA_SPECIAL = 0,
	PSPA_EXCEPTION = 1,
	PSPA_AU_SESSION = 2,
	PSPA_IMP_WATCHPORTS = 3,
	PSPA_REGISTERED_PORTS = 4,
} pspa_t;

/*
 * Internal representation of one port to be set on posix_spawn().
 * Currently this is limited to setting special and exception ports,
 * but could be extended to other inheritable port types.
 */
typedef struct _ps_port_action {
	pspa_t                  port_type;
	exception_mask_t        mask;
	mach_port_name_t        new_port;
	exception_behavior_t    behavior;
	thread_state_flavor_t   flavor;
	int                     which;
} _ps_port_action_t;

/*
 * A collection of port actions to take on the newly spawned process.
 */
typedef struct _posix_spawn_port_actions {
	int                     pspa_alloc;
	int                     pspa_count;
	_ps_port_action_t       pspa_actions[];
} *_posix_spawn_port_actions_t;

/*
 * Returns size in bytes of a _posix_spawn_port_actions holding x elements.
 */
#define PS_PORT_ACTIONS_SIZE(x) \
	PS_ACTION_SIZE(x, struct _posix_spawn_port_actions, _ps_port_action_t)

#define NBINPREFS       4

/*
 * Mapping of opaque data pointer to a MAC policy (specified by name).
 */
typedef struct _ps_mac_policy_extension {
	char                    policyname[128];
	union {
		uint64_t        data;
		void            *datap;         /* pointer in kernel memory */
	};
	uint64_t                datalen;
} _ps_mac_policy_extension_t;

/*
 * A collection of extra data passed to MAC policies for the newly spawned process.
 */
typedef struct _posix_spawn_mac_policy_extensions {
	int                     psmx_alloc;
	int                     psmx_count;
	_ps_mac_policy_extension_t psmx_extensions[];
} *_posix_spawn_mac_policy_extensions_t;

/*
 * Returns size in bytes of a _posix_spawn_mac_policy_extensions holding x elements.
 */
#define PS_MAC_EXTENSIONS_SIZE(x)     \
	PS_ACTION_SIZE(x, struct _posix_spawn_mac_policy_extensions, _ps_mac_policy_extension_t)

#define PS_MAC_EXTENSIONS_INIT_COUNT    2

/*
 * Coalition posix spawn attributes
 */
struct _posix_spawn_coalition_info {
	struct {
		uint64_t psci_id;
		uint32_t psci_role;
		uint32_t psci_reserved1;
		uint64_t psci_reserved2;
	} psci_info[COALITION_NUM_TYPES];
};

/*
 * UID/GID attributes
 */
struct _posix_spawn_posix_cred_info {
	uint32_t pspci_flags;    /* spawn persona flags */
	uid_t    pspci_uid;      /* alternate posix/unix UID  */
	gid_t    pspci_gid;      /* alternate posix/unix GID */
	uint32_t pspci_ngroups;  /* alternate advisory groups */
	gid_t    pspci_groups[NGROUPS];
	uid_t    pspci_gmuid;    /* group membership UID */
	char     pspci_login[MAXLOGNAME + 1];
};

#define POSIX_SPAWN_POSIX_CRED_UID          0x00010000
#define POSIX_SPAWN_POSIX_CRED_GID          0x00020000
#define POSIX_SPAWN_POSIX_CRED_GROUPS       0x00040000
#define POSIX_SPAWN_POSIX_CRED_LOGIN        0x00080000

/*
 * Persona attributes
 */
struct _posix_spawn_persona_info {
	uid_t    pspi_id;       /* persona ID (unix UID) */
	uint32_t pspi_flags;    /* spawn persona flags */
	uid_t    pspi_uid;      /* alternate posix/unix UID  */
	gid_t    pspi_gid;      /* alternate posix/unix GID */
	uint32_t pspi_ngroups;  /* alternate advisory groups */
	gid_t    pspi_groups[NGROUPS];
	uid_t    pspi_gmuid;    /* group membership UID */
};

#define POSIX_SPAWN_PERSONA_FLAGS_NONE      0x0
#define POSIX_SPAWN_PERSONA_FLAGS_OVERRIDE  0x1
#define POSIX_SPAWN_PERSONA_FLAGS_VERIFY    0x2

#define POSIX_SPAWN_PERSONA_ALL_FLAGS \
	(POSIX_SPAWN_PERSONA_FLAGS_OVERRIDE \
	 | POSIX_SPAWN_PERSONA_FLAGS_VERIFY \
	)

#define POSIX_SPAWN_PERSONA_UID             POSIX_SPAWN_POSIX_CRED_UID
#define POSIX_SPAWN_PERSONA_GID             POSIX_SPAWN_POSIX_CRED_GID
#define POSIX_SPAWN_PERSONA_GROUPS          POSIX_SPAWN_POSIX_CRED_GROUPS


/*
 * A posix_spawnattr structure contains all of the attribute elements that
 * can be set, as well as any metadata whose validity is signalled by the
 * presence of a bit in the flags field.  All fields are initialized to the
 * appropriate default values by posix_spawnattr_init().
 *
 * Fields must be added at the end of this, but before extensions array
 * pointers.
 */

typedef struct _posix_spawnattr {
	short           psa_flags;              /* spawn attribute flags */
	short           flags_padding;  /* get the flags to be int aligned */
	sigset_t        psa_sigdefault;         /* signal set to default */
	sigset_t        psa_sigmask;            /* signal set to mask */
	pid_t           psa_pgroup;             /* pgroup to spawn into */
	cpu_type_t      psa_binprefs[NBINPREFS];   /* cpu affinity prefs*/
	int             psa_pcontrol;           /* process control bits on resource starvation */
	int             psa_apptype;            /* app type and process spec behav */
	uint64_t        psa_cpumonitor_percent; /* CPU usage monitor percentage */
	uint64_t        psa_cpumonitor_interval; /* CPU usage monitor interval, in seconds */
	uint64_t        psa_reserved;

	short       psa_jetsam_flags;           /* jetsam flags */
	short           short_padding;          /* Padding for alignment issues */
	int         psa_priority;               /* jetsam relative importance */
	int         psa_memlimit_active;        /* jetsam memory limit (in MB) when process is active */
	int         psa_memlimit_inactive;      /* jetsam memory limit (in MB) when process is inactive */

	uint64_t        psa_qos_clamp;          /* QoS Clamp to set on the new process */
	uint64_t        psa_darwin_role;           /* PRIO_DARWIN_ROLE to set on the new process */
	int             psa_thread_limit;       /* thread limit */

	uint64_t        psa_max_addr;           /* Max valid VM address */

	/*
	 * NOTE: Extensions array pointers must stay at the end so that
	 * everything above this point stays the same size on different bitnesses
	 * see <rdar://problem/12858307>
	 */
	_posix_spawn_port_actions_t    psa_ports;  /* special/exception ports */
	_posix_spawn_mac_policy_extensions_t psa_mac_extensions; /* MAC policy-specific extensions. */
	struct _posix_spawn_coalition_info *psa_coalition_info;  /* coalition info */
	struct _posix_spawn_persona_info   *psa_persona_info;    /* spawn new process into given persona */
	struct _posix_spawn_posix_cred_info *psa_posix_cred_info; /* posix creds: uid/gid/groups */
} *_posix_spawnattr_t;

/*
 * Jetsam flags  eg: psa_jetsam_flags
 */
#define POSIX_SPAWN_JETSAM_SET                      0x8000

#define POSIX_SPAWN_JETSAM_USE_EFFECTIVE_PRIORITY       0x01
#define POSIX_SPAWN_JETSAM_HIWATER_BACKGROUND           0x02  /* to be deprecated */
#define POSIX_SPAWN_JETSAM_MEMLIMIT_FATAL               0x04  /* to be deprecated */

/*
 * Additional flags available for use with
 * the posix_spawnattr_setjetsam_ext() call
 */
#define POSIX_SPAWN_JETSAM_MEMLIMIT_ACTIVE_FATAL        0x04  /* if set, limit is fatal when the process is active   */
#define POSIX_SPAWN_JETSAM_MEMLIMIT_INACTIVE_FATAL      0x08  /* if set, limit is fatal when the process is inactive */


/*
 * Flags set based on posix_spawnattr_set_jetsam_ttr_np().
 * Indicate relaunch behavior of process when jetsammed
 */
/* Mask and bucket counts for relaunch behavior */
#define POSIX_SPAWN_JETSAM_RELAUNCH_BEHAVIOR_BUCKETS    (0x3)
#define POSIX_SPAWN_JETSAM_RELAUNCH_BEHAVIOR_MASK       (0x30)

/* Actual buckets based on behavior data */
#define POSIX_SPAWN_JETSAM_RELAUNCH_BEHAVIOR_HIGH       (0x30)
#define POSIX_SPAWN_JETSAM_RELAUNCH_BEHAVIOR_MED        (0x20)
#define POSIX_SPAWN_JETSAM_RELAUNCH_BEHAVIOR_LOW        (0x10)

/*
 * Deprecated posix_spawn psa_flags values
 *
 * POSIX_SPAWN_OSX_TALAPP_START         0x0400
 * POSIX_SPAWN_IOS_RESV1_APP_START      0x0400
 * POSIX_SPAWN_IOS_APPLE_DAEMON_START   0x0800
 * POSIX_SPAWN_IOS_APP_START            0x1000
 * POSIX_SPAWN_OSX_WIDGET_START         0x0800
 * POSIX_SPAWN_OSX_DBCLIENT_START       0x0800
 * POSIX_SPAWN_OSX_RESVAPP_START        0x1000
 */

/*
 * Deprecated posix_spawn psa_apptype values
 *
 * POSIX_SPAWN_PROCESS_TYPE_APPLEDAEMON             0x00000001
 * POSIX_SPAWN_PROCESS_TYPE_UIAPP                   0x00000002
 * POSIX_SPAWN_PROCESS_TYPE_ADAPTIVE                0x00000004
 * POSIX_SPAWN_PROCESS_TYPE_TAL                     0x00000001
 * POSIX_SPAWN_PROCESS_TYPE_WIDGET                  0x00000002
 * POSIX_SPAWN_PROCESS_TYPE_DELAYIDLESLEEP          0x10000000
 *
 * POSIX_SPAWN_PROCESS_FLAG_IMPORTANCE_DONOR        0x00000010
 * POSIX_SPAWN_PROCESS_FLAG_ADAPTIVE                0x00000020
 * POSIX_SPAWN_PROCESS_FLAG_START_BACKGROUND        0x00000040
 * POSIX_SPAWN_PROCESS_FLAG_START_LIGHT_THROTTLE    0x00000080
 */

/*
 * posix_spawn psa_apptype process type settings.
 * when POSIX_SPAWN_PROC_TYPE is set, old psa_apptype bits are ignored
 */
#define POSIX_SPAWN_PROCESS_TYPE_NORMAL             0x00000000
#define POSIX_SPAWN_PROCESS_TYPE_DEFAULT            POSIX_SPAWN_PROCESS_TYPE_NORMAL

#define POSIX_SPAWN_PROC_TYPE_MASK                  0x00000F00

#define POSIX_SPAWN_PROC_TYPE_APP_DEFAULT           0x00000100
#define POSIX_SPAWN_PROC_TYPE_APP_TAL               0x00000200

#define POSIX_SPAWN_PROC_TYPE_DAEMON_STANDARD       0x00000300
#define POSIX_SPAWN_PROC_TYPE_DAEMON_INTERACTIVE    0x00000400
#define POSIX_SPAWN_PROC_TYPE_DAEMON_BACKGROUND     0x00000500
#define POSIX_SPAWN_PROC_TYPE_DAEMON_ADAPTIVE       0x00000600

#define POSIX_SPAWN_PROC_TYPE_DRIVER                0x00000700

#define POSIX_SPAWN_PROC_CLAMP_NONE                 0x00000000
#define POSIX_SPAWN_PROC_CLAMP_UTILITY              0x00000001
#define POSIX_SPAWN_PROC_CLAMP_BACKGROUND           0x00000002
#define POSIX_SPAWN_PROC_CLAMP_MAINTENANCE          0x00000003
#define POSIX_SPAWN_PROC_CLAMP_LAST                 0x00000004

#define POSIX_SPAWN_ENTITLEMENT_DRIVER "com.apple.private.spawn-driver"
/* Setting to indicate no change to darwin role */
#define POSIX_SPAWN_DARWIN_ROLE_NONE                0x00000000
/* Other possible values are specified by PRIO_DARWIN_ROLE in sys/resource.h */

/*
 * Allowable posix_spawn() file actions
 */
typedef enum {
	PSFA_OPEN = 0,
	PSFA_CLOSE = 1,
	PSFA_DUP2 = 2,
	PSFA_INHERIT = 3,
	PSFA_FILEPORT_DUP2 = 4,
	PSFA_CHDIR = 5,
	PSFA_FCHDIR = 6
} psfa_t;


/*
 * A posix_spawn() file action record for a single action
 *
 * Notes:	We carry around the full open arguments for both the open
 *		and the close to permit the use of a single array of action
 *		elements to be associated with a file actions object.
 *
 *		A possible future optimization would be to break this into
 *		a variable sized vector list to save space (i.e. a separate
 *		string area, allocation of least amount of path buffer per
 *		open action, etc.).
 */
typedef struct _psfa_action {
	psfa_t  psfaa_type;                         /* file action type */
	union {
		int psfaa_filedes;                  /* fd to operate on */
		mach_port_name_t psfaa_fileport;    /* fileport to operate on */
	};
	union {
		struct _psfaa_open {
			int     psfao_oflag;            /* open flags to use */
			mode_t  psfao_mode;             /* mode for open */
			char    psfao_path[PATH_MAX];   /* path to open */
		} psfaa_openargs;
		struct {
			int psfad_newfiledes;           /* new file descriptor to use */
		} psfaa_dup2args;
		struct {
			char    psfac_path[PATH_MAX];   /* path to chdir */
		} psfaa_chdirargs;
	};
} _psfa_action_t;


/*
 * Internal representation of posix_spawn() file actions structure
 *
 * Notes:	This is implemented as a structure followed by an array of
 *		file action records.  The psfa_act_alloc value is the number
 *		of elements allocated in this array, and the psfa_act_count is
 *		the number of elements currently in use (to permit some form
 *		of preallocation, e.g. a power of 2 growth for reallocation,
 *		etc.).
 *
 *		A possible future optimization would keep a size value and
 *		a structure base reference pointer to permit copyin to the
 *		kernel directly as a single blob, without damaging relative
 *		internal pointer math.  It's probably better that this be a
 *		long long rather than a true pointer, to make it invariant
 *		for 32 vs. 64 bt programming SPIs.
 */
typedef struct _posix_spawn_file_actions {
	int             psfa_act_alloc;         /* available actions space */
	int             psfa_act_count;         /* count of defined actions */
	_psfa_action_t  psfa_act_acts[];        /* actions array (uses c99) */
} *_posix_spawn_file_actions_t;

/*
 * Calculate the size of a structure, given the number of elements that it is
 * capable of containing.
 */
#define PSF_ACTIONS_SIZE(x)     \
	PS_ACTION_SIZE(x, struct _posix_spawn_file_actions, _psfa_action_t)

/*
 * Initial count of actions in a struct _posix_spawn_file_actions after it is
 * first allocated; this should be non-zero, since we expect that one would not
 * have been allocated unless there was an intent to use it.
 */
#define PSF_ACTIONS_INIT_COUNT  2

/*
 * Structure defining the true third argument to the posix_spawn() system call
 * entry point; we wrap it and pass a descriptor so that we can know the
 * copyin size ahead of time, and deal with copying in variant lists of things
 * as single monolithic units, instead of many individual elements.  This is a
 * performance optimization.
 */
struct _posix_spawn_args_desc {
	__darwin_size_t         attr_size;      /* size of attributes block */
	_posix_spawnattr_t      attrp;          /* pointer to block */
	__darwin_size_t file_actions_size;      /* size of file actions block */
	_posix_spawn_file_actions_t
	    file_actions;                       /* pointer to block */
	__darwin_size_t port_actions_size;      /* size of port actions block */
	_posix_spawn_port_actions_t
	    port_actions;                       /* pointer to port block */
	__darwin_size_t mac_extensions_size;
	_posix_spawn_mac_policy_extensions_t
	    mac_extensions;                     /* pointer to policy-specific
	                                         * attributes */
	__darwin_size_t coal_info_size;
	struct _posix_spawn_coalition_info *coal_info;  /* pointer to coalition info */

	__darwin_size_t persona_info_size;
	struct _posix_spawn_persona_info   *persona_info;

	__darwin_size_t posix_cred_info_size;
	struct _posix_spawn_posix_cred_info *posix_cred_info;
};

#ifdef KERNEL
#include <sys/appleapiopts.h>
#ifdef __APPLE_API_PRIVATE

#if __DARWIN_ALIGN_NATURAL
#pragma options align=natural
#endif

struct user32__posix_spawn_args_desc {
	uint32_t        attr_size;              /* size of attributes block */
	uint32_t        attrp;                  /* pointer to block */
	uint32_t        file_actions_size;      /* size of file actions block */
	uint32_t        file_actions;           /* pointer to block */
	uint32_t        port_actions_size;      /* size of port actions block */
	uint32_t        port_actions;           /* pointer to block */
	uint32_t        mac_extensions_size;
	uint32_t        mac_extensions;
	uint32_t        coal_info_size;
	uint32_t        coal_info;
	uint32_t        persona_info_size;
	uint32_t        persona_info;
	uint32_t        posix_cred_info_size;
	uint32_t        posix_cred_info;
};

struct user__posix_spawn_args_desc {
	user_size_t     attr_size;              /* size of attributes block */
	user_addr_t     attrp;                  /* pointer to block */
	user_size_t     file_actions_size;      /* size of file actions block */
	user_addr_t     file_actions;           /* pointer to block */
	user_size_t     port_actions_size;      /* size of port actions block */
	user_addr_t     port_actions;           /* pointer to block */
	user_size_t     mac_extensions_size;    /* size of MAC-specific attrs. */
	user_addr_t     mac_extensions;         /* pointer to block */
	user_size_t     coal_info_size;
	user_addr_t     coal_info;
	user_size_t     persona_info_size;
	user_addr_t     persona_info;
	user_size_t     posix_cred_info_size;
	user_addr_t     posix_cred_info;
};


#if __DARWIN_ALIGN_NATURAL
#pragma options align=reset
#endif

#endif  /* __APPLE_API_PRIVATE */
#endif  /* KERNEL */

#endif  /* _SYS_SPAWN_INTERNAL_H_ */
