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

#ifndef	_SYS_SPAWN_INTERNAL_H_
#define	_SYS_SPAWN_INTERNAL_H_

#include <sys/_types.h>		/* __offsetof(), __darwin_size_t */
#include <sys/syslimits.h>	/* PATH_MAX */
#include <sys/spawn.h>
#include <mach/machine.h>
#include <mach/port.h>
#include <mach/exception_types.h>

/*
 * Allowable posix_spawn() port action types
 */
typedef enum {
	PSPA_SPECIAL = 0,
	PSPA_EXCEPTION = 1,
	PSPA_AU_SESSION = 2,
} pspa_t;

/*
 * Internal representation of one port to be set on posix_spawn().
 * Currently this is limited to setting special and exception ports,
 * but could be extended to other inheritable port types.
 */
typedef struct _ps_port_action {
	pspa_t			port_type;
	exception_mask_t	mask;
	mach_port_name_t	new_port;
	exception_behavior_t	behavior;
	thread_state_flavor_t	flavor;
	int			which;
} _ps_port_action_t;

/*
 * A collection of port actions to take on the newly spawned process.
 */
typedef struct _posix_spawn_port_actions {
	int			pspa_alloc;
	int			pspa_count;
	_ps_port_action_t 	pspa_actions[];
} *_posix_spawn_port_actions_t;

/*
 * Returns size in bytes of a _posix_spawn_port_actions holding x elements.
 */
#define	PS_PORT_ACTIONS_SIZE(x)	\
	__offsetof(struct _posix_spawn_port_actions, pspa_actions[(x)])

#define NBINPREFS	4

/*
 * A posix_spawnattr structure contains all of the attribute elements that
 * can be set, as well as any metadata whose validity is signalled by the
 * presence of a bit in the flags field.  All fields are initialized to the
 * appropriate default values by posix_spawnattr_init().
 */
typedef struct _posix_spawnattr {
	short		psa_flags;		/* spawn attribute flags */
	sigset_t	psa_sigdefault;		/* signal set to default */
	sigset_t	psa_sigmask;		/* signal set to mask */
	pid_t		psa_pgroup;		/* pgroup to spawn into */
	cpu_type_t	psa_binprefs[NBINPREFS];   /* cpu affinity prefs*/
	int		psa_pcontrol;		/* process control bits on resource starvation */
	int		psa_apptype;		/* app type and process spec behav */
	uint64_t 	psa_cpumonitor_percent; /* CPU usage monitor percentage */
	uint64_t 	psa_cpumonitor_interval; /* CPU usage monitor interval, in seconds */
	_posix_spawn_port_actions_t	psa_ports; /* special/exception ports */
	/* XXX - k64/u32 unaligned below here */
#if CONFIG_MEMORYSTATUS || CONFIG_EMBEDDED || TARGET_OS_EMBEDDED
	/* Jetsam related */
	short       psa_jetsam_flags; /* flags */
	int         psa_priority;   /* relative importance */
	int         psa_high_water_mark; /* resident page count limit */
#endif
} *_posix_spawnattr_t;

/*
 * Jetsam flags
 */
#if CONFIG_MEMORYSTATUS || CONFIG_EMBEDDED || TARGET_OS_EMBEDDED
#define	POSIX_SPAWN_JETSAM_USE_EFFECTIVE_PRIORITY	0x1
#endif

/*
 * DEPRECATED: maintained for transition purposes only
 * posix_spawn apptype settings.
 */
#if TARGET_OS_EMBEDDED || CONFIG_EMBEDDED
/* for compat sake */
#define POSIX_SPAWN_OSX_TALAPP_START    0x0400
#define POSIX_SPAWN_IOS_RESV1_APP_START 0x0400
#define POSIX_SPAWN_IOS_APPLE_DAEMON_START      0x0800          /* not a bug, same as widget just rename */
#define POSIX_SPAWN_IOS_APP_START       0x1000
#else /* TARGET_OS_EMBEDDED */
#define POSIX_SPAWN_OSX_TALAPP_START    0x0400
#define POSIX_SPAWN_OSX_WIDGET_START    0x0800
#define POSIX_SPAWN_OSX_DBCLIENT_START  0x0800          /* not a bug, same as widget just rename */
#define POSIX_SPAWN_OSX_RESVAPP_START   0x1000          /* reserved for app start usages */
#endif /* TARGET_OS_EMBEDDED */


/*
 * posix_spawn apptype and process attribute settings.
 */
#if TARGET_OS_EMBEDDED || CONFIG_EMBEDDED
#define POSIX_SPAWN_APPTYPE_IOS_APPLEDAEMON    0x0001          /* it is an iOS apple daemon  */
#else /* TARGET_OS_EMBEDDED */
#define POSIX_SPAWN_APPTYPE_OSX_TAL	0x0001		/* it is a TAL app */
#define POSIX_SPAWN_APPTYPE_OSX_WIDGET	0x0002		/* it is a widget */
#define POSIX_SPAWN_APPTYPE_DELAYIDLESLEEP   0x10000000	/* Process is marked to delay idle sleep on disk IO */
#endif /* TARGET_OS_EMBEDDED */

/*
 * Allowable posix_spawn() file actions
 */
typedef enum {
	PSFA_OPEN = 0,
	PSFA_CLOSE = 1,
	PSFA_DUP2 = 2,
	PSFA_INHERIT = 3
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
 *
 * XXX:		Currently overloading psfao_oflag for PSFA_DUP2
 */
typedef struct _psfa_action {
	psfa_t	psfaa_type;			/* file action type */
	int	psfaa_filedes;			/* fd to operate on */
	struct _psfaa_open {
		int	psfao_oflag;		/* open flags to use */
		mode_t	psfao_mode;		/* mode for open */
		char	psfao_path[PATH_MAX];	/* path to open */
	} psfaa_openargs;
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
	int		psfa_act_alloc;		/* available actions space */
	int		psfa_act_count;		/* count of defined actions */
	_psfa_action_t	psfa_act_acts[];	/* actions array (uses c99) */
} *_posix_spawn_file_actions_t;

/*
 * Calculate the size of a structure, given the number of elements that it is
 * capable of containing.
 */
#define	PSF_ACTIONS_SIZE(x)	\
	__offsetof(struct _posix_spawn_file_actions, psfa_act_acts[(x)])

/*
 * Initial count of actions in a struct _posix_spawn_file_actions after it is
 * first allocated; this should be non-zero, since we expect that one would not
 * have been allocated unless there was an intent to use it.
 */
#define	PSF_ACTIONS_INIT_COUNT	2

/*
 * Structure defining the true third argument to the posix_spawn() system call
 * entry point; we wrap it and pass a descriptor so that we can know the
 * copyin size ahead of time, and deal with copying in variant lists of things
 * as single monolithic units, instead of many individual elements.  This is a
 * performance optimization.
 */
struct _posix_spawn_args_desc {
	__darwin_size_t		attr_size;	/* size of attributes block */
	_posix_spawnattr_t	attrp;		/* pointer to block */
	__darwin_size_t	file_actions_size;	/* size of file actions block */
	_posix_spawn_file_actions_t
				file_actions;	/* pointer to block */
	__darwin_size_t	port_actions_size; 	/* size of port actions block */
	_posix_spawn_port_actions_t
				port_actions; 	/* pointer to port block */
};

#ifdef KERNEL
#include <sys/appleapiopts.h>
#ifdef __APPLE_API_PRIVATE

#if __DARWIN_ALIGN_NATURAL
#pragma options align=natural
#endif

struct user32__posix_spawn_args_desc {
	uint32_t		attr_size;	/* size of attributes block */
	uint32_t		attrp;		/* pointer to block */
	uint32_t	file_actions_size;	/* size of file actions block */
	uint32_t		file_actions;	/* pointer to block */
	uint32_t	port_actions_size;	/* size of port actions block */
	uint32_t		port_actions;	/* pointer to block */
};

struct user__posix_spawn_args_desc {
	user_size_t		attr_size;	/* size of attributes block */
	user_addr_t		attrp;		/* pointer to block */
	user_size_t	file_actions_size;	/* size of file actions block */
	user_addr_t		file_actions;	/* pointer to block */
	user_size_t	port_actions_size;	/* size of port actions block */
	user_addr_t		port_actions;	/* pointer to block */
};


#if __DARWIN_ALIGN_NATURAL
#pragma options align=reset
#endif

#endif	/* __APPLE_API_PRIVATE */
#endif	/* KERNEL */
	
#endif	/* _SYS_SPAWN_INTERNAL_H_ */
