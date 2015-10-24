/*
 * Copyright (c) 2006-2008 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

/*
 * [SPN] Support for _POSIX_SPAWN
 */

#include <sys/types.h> /* for user_size_t */
#include <spawn.h>
#include <spawn_private.h>
#include <sys/spawn_internal.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>	/* for OPEN_MAX, PATH_MAX */
#include <stddef.h>	/* for offsetof() */
#include <string.h>	/* for strlcpy() */
#include <paths.h>	/* for _PATH_DEFPATH */
#include <sys/stat.h>	/* for struct stat */
#include <mach/port.h>
#include <mach/exception_types.h>

#if TARGET_OS_EMBEDDED
#include <sys/kern_memorystatus.h>
#endif

/*
 * posix_spawnattr_init
 *
 * Description:	Initialize a spawn attributes object attr with default values
 *
 * Parameters:	attr			The spawn attributes object to be
 *					initialized
 *
 * Returns:	0			Success
 *		ENOMEM			Insufficient memory exists to
 *					initialize the spawn attributes object.
 *
 * Note:	As an implementation detail, the externally visibily type
 *		posix_spawnattr_t is defined to be a void *, and initialization
 *		involves allocation of a memory object.  Subsequent changes to
 *		the spawn attributes may result in reallocation under the
 *		covers.
 *
 *		Reinitialization of an already initialized spawn attributes
 *		object will result in memory being leaked.  Because spawn
 *		attributes are not required to be used in conjunction with a
 *		static initializer, there is no way to distinguish a spawn
 *		attribute with stack garbage from one that's been initialized.
 *		This is arguably an API design error.
 */
int
posix_spawnattr_init(posix_spawnattr_t *attr)
{
	_posix_spawnattr_t *psattrp = (_posix_spawnattr_t *)attr;
	int	err = 0;

	if ((*psattrp = (_posix_spawnattr_t)malloc(sizeof(struct _posix_spawnattr))) == NULL) {
		err = ENOMEM;
	} else {

		/*
		 * The default value of this attribute shall be as if no
		 * flags were set
		 */
		(*psattrp)->psa_flags = 0;

		/*
		 * The default value of this attribute shall be an empty
		 * signal set
		 */
		(*psattrp)->psa_sigdefault = 0;

		/* The default value of this attribute is unspecified */
		(*psattrp)->psa_sigmask = 0;

		/* The default value of this attribute shall be zero */
		(*psattrp)->psa_pgroup = 0;	/* doesn't matter */

		/* Default is no binary preferences, i.e. use normal grading */
		memset((*psattrp)->psa_binprefs, 0, 
				sizeof((*psattrp)->psa_binprefs));

		/* Default is no port actions to take */
		(*psattrp)->psa_ports = NULL;

		/*
		 * The default value of this attribute shall be an no
		 * process control on resource starvation
		 */
		(*psattrp)->psa_pcontrol = 0;

		/*
		 * The default value of this attribute shall be an no
		 * process control on resource starvation
		 */
		(*psattrp)->psa_apptype = 0;

#if TARGET_OS_EMBEDDED
		/* Jetsam related */
		(*psattrp)->psa_jetsam_flags = 0;
		(*psattrp)->psa_priority = DEFAULT_JETSAM_PRIORITY;
		(*psattrp)->psa_high_water_mark = -1;
#endif

		/* Default is no CPU usage monitor active. */
		(*psattrp)->psa_cpumonitor_percent = 0;
		(*psattrp)->psa_cpumonitor_interval = 0;
	}

	return (err);
}


/*
 * posix_spawnattr_destroy
 *
 * Description:	Destroy a spawn attributes object that was previously
 *		initialized via posix_spawnattr_init() by freeing any
 *		memory associated with it and setting it to an invalid value.
 *
 * Parameters:	attr			The spawn attributes object to be
 *					destroyed.
 *
 * Returns:	0			Success
 *
 * Notes:	The destroyed spawn attribute results in the void * pointer
 *		being set to NULL; subsequent use without reinitialization
 *		will result in explicit program failure (rather than merely
 *		"undefined behaviour").
 *
 * NOTIMP:	Allowed failures (checking NOT required):
 *		EINVAL	The value specified by attr is invalid.
 */
static int posix_spawn_destroyportactions_np(posix_spawnattr_t *);

int
posix_spawnattr_destroy(posix_spawnattr_t *attr)
{
	_posix_spawnattr_t psattr;

	if (attr == NULL || *attr == NULL)
		return EINVAL;

	psattr = *(_posix_spawnattr_t *)attr;
	posix_spawn_destroyportactions_np(attr);

	free(psattr);
	*attr = NULL;

	return (0);
}


/*
 * posix_spawnattr_setflags
 *
 * Description:	Set the spawn flags attribute for the spawn attribute object
 *		referred to by 'attr'.
 *
 * Parameters:	attr			The spawn attributes object whose flags
 *					are to be set
 *		flags			The flags value to set
 *
 * Returns:	0			Success
 *
 * NOTIMP:	Allowed failures (checking NOT required):
 *		EINVAL	The value specified by attr is invalid.
 *		EINVAL	The value of the attribute being set is not valid.
 */
int
posix_spawnattr_setflags(posix_spawnattr_t *attr, short flags)
{
	_posix_spawnattr_t psattr;

	if (attr == NULL || *attr == NULL)
		return EINVAL;

	psattr = *(_posix_spawnattr_t *)attr;
	psattr->psa_flags = flags;

	return (0);
}


/*
 * posix_spawnattr_getflags
 *
 * Description:	Retrieve the spawn attributes flag for the spawn attributes
 *		object referenced by 'attr' and place them in the memory
 *		location referenced by 'flagsp'
 *
 * Parameters:	attr			The spawn attributes object whose flags
 *					are to be retrieved
 *		flagsp			A pointer to a short value to receive
 *					the flags
 *
 * Returns:	0			Success
 *
 * Implicit Returns:
 *		*flagps (modified)	The flags value from the spawn
 *					attributes object
 *
 * NOTIMP:	Allowed failures (checking NOT required):
 *		EINVAL	The value specified by attr is invalid.
 *		EINVAL	The value of the attribute being set is not valid.
 */
int
posix_spawnattr_getflags(const posix_spawnattr_t * __restrict attr,
	short * __restrict flagsp)
{
	_posix_spawnattr_t psattr;

	if (attr == NULL || *attr == NULL)
		return EINVAL;

	psattr = *(_posix_spawnattr_t *)attr;
	*flagsp = psattr->psa_flags;

	return (0);
}


/*
 * posix_spawnattr_getsigdefault
 *
 * Description:	Retrieve the set of signals to be set to default according to
 *		the spawn attribute value referenced by 'attr' and place the
 *		result into the memory containing the sigset_t referenced by
 *		'sigdefault'
 *
 * Parameters:	attr			The spawn attributes object whose
 *					signal set for default signals is to
 *					be retrieved
 *		sigdefault		A pointer to the sigset_t to receive
 *					the signal set
 *
 * Returns:	0			Success
 *
 * Implicit Returns:
 *		*sigdefault (modified)	The signal set of signals to default
 *					from the spawn attributes object
 */
int
posix_spawnattr_getsigdefault(const posix_spawnattr_t * __restrict attr,
		sigset_t * __restrict sigdefault)
{
	_posix_spawnattr_t psattr;

	if (attr == NULL || *attr == NULL)
		return EINVAL;

	psattr = *(_posix_spawnattr_t *)attr;
	*sigdefault = psattr->psa_sigdefault;

	return (0);
}


/*
 * posix_spawnattr_getpgroup
 *
 * Description:	Obtain the value of the spawn process group attribute from the
 *		spawn attributes object referenced by 'attr' and place the
 *		results in the memory location referenced by 'pgroup'
 *
 * Parameters:	attr			The spawn attributes object whose
 *					process group information is to be
 *					retrieved
 *		pgroup			A pointer to the pid_t to receive the
 *					process group
 *
 * Returns:	0			Success
 *
 * Implicit Returns:
 *		*pgroup (modified)	The process group information from the
 *					spawn attributes object
 */
int
posix_spawnattr_getpgroup(const posix_spawnattr_t * __restrict attr,
		pid_t * __restrict pgroup)
{
	_posix_spawnattr_t psattr;

	if (attr == NULL || *attr == NULL)
		return EINVAL;

	psattr = *(_posix_spawnattr_t *)attr;
	*pgroup = psattr->psa_pgroup;

	return (0);
}


/*
 * posix_spawnattr_getsigmask
 *
 * Description:	Obtain the value of the spawn signal mask attribute from the
 *		spawn attributes object referenced by 'attr' and place the
 *		result into the memory containing the sigset_t referenced by
 *		'sigmask'
 *
 * Parameters:	attr			The spawn attributes object whose
 *					signal set for masked signals is to
 *					be retrieved
 *		sigmask		A pointer to the sigset_t to receive
 *					the signal set
 *
 * Returns:	0			Success
 *
 * Implicit Returns:
 *		*sigmask (modified)	The signal set of signals to mask
 *					from the spawn attributes object
 */
int
posix_spawnattr_getsigmask(const posix_spawnattr_t * __restrict attr,
		sigset_t * __restrict sigmask)
{
	_posix_spawnattr_t psattr;

	if (attr == NULL || *attr == NULL)
		return EINVAL;

	psattr = *(_posix_spawnattr_t *)attr;
	*sigmask = psattr->psa_sigmask;

	return (0);
}

/*
 * posix_spawnattr_getbinpref_np
 *
 * Description:	Obtain the value of the spawn binary preferences attribute from 
 * 		the spawn attributes object referenced by 'attr' and place the
 *		result into the memory referenced by 'pref'.
 *
 * Parameters:	attr			The spawn attributes object whose
 *					binary preferences are to be retrieved
 *		count			The size of the cpu_type_t array
 *		pref			An array of cpu types
 *		ocount			The actual number copied
 *
 * Returns:	0			No binary preferences found
 * 		> 0			The number of cpu types (less than 
 * 					count) copied over from 'attr'.
 *
 * Implicit Returns:
 *		*pref (modified)	The binary preferences array 
 *					from the spawn attributes object
 */
int
posix_spawnattr_getbinpref_np(const posix_spawnattr_t * __restrict attr,
		size_t count, cpu_type_t *pref, size_t * __restrict ocount)
{
	_posix_spawnattr_t psattr;
	int i = 0;

	if (attr == NULL || *attr == NULL)
		return EINVAL;

	psattr = *(_posix_spawnattr_t *)attr;
	for (i = 0; i < count && i < 4; i++) {
		pref[i] = psattr->psa_binprefs[i];
	}

	if (ocount)
		*ocount = i;
	return 0;
}


/*
 * posix_spawnattr_getpcontrol_np
 *
 * Description:	Retrieve the  process control property set default according to
 *		the spawn attribute value referenced by 'attr' and place the
 *		result into the memory containing the control  referenced by
 *		'pcontrol'
 *
 * Parameters:	attr			The spawn attributes object whose
 *					signal set for default signals is to
 *					be retrieved
 *		pcontrol		A pointer to an int  to receive
 *					the process control info
 *
 * Returns:	0			Success
 *
 * Implicit Returns:
 *		*pcontrol (modified)	The signal set of signals to default
 *					from the spawn attributes object
 */
int
posix_spawnattr_getpcontrol_np(const posix_spawnattr_t * __restrict attr,
		int * __restrict pcontrol)
{
	_posix_spawnattr_t psattr;

	if (attr == NULL || *attr == NULL)
		return EINVAL;

	psattr = *(_posix_spawnattr_t *)attr;
	*pcontrol = psattr->psa_pcontrol;

	return (0);
}

/*
 * posix_spawnattr_getapptype_np
 *
 * Description:	Retrieve the  process specific behaviors and app launch typea
 *		spawn attribute value referenced by 'attr' and place the
 *		result into the memory containing the control  referenced by
 *		'apptype'
 *
 * Parameters:	attr			The spawn attributes object whose
 *					signal set for default signals is to
 *					be retrieved
 *		apptype			A pointer to an int  to receive
 *					the process control info
 *
 * Returns:	0			Success
 *
 * Implicit Returns:
 *		*pcontrol (modified)	The signal set of signals to default
 *					from the spawn attributes object
 */
int
posix_spawnattr_getapptype_np(const posix_spawnattr_t * __restrict attr,
		int * __restrict apptype)
{
	_posix_spawnattr_t psattr;

	if (attr == NULL || *attr == NULL)
		return EINVAL;

	psattr = *(_posix_spawnattr_t *)attr;
	*apptype = psattr->psa_apptype;

	return (0);
}

/*
 * posix_spawnattr_setsigdefault
 *
 * Description:	Set the set of signals to be set to default for the spawn
 *		attribute value referenced by 'attr' from the memory
 *		containing the sigset_t referenced by 'sigdefault'
 *
 * Parameters:	attr			The spawn attributes object whose
 *					signal set for default signals is to
 *					be set
 *		sigdefault		A pointer to the sigset_t from which to
 *					obtain the signal set
 *
 * Returns:	0			Success
 */
int
posix_spawnattr_setsigdefault(posix_spawnattr_t * __restrict attr,
		const sigset_t * __restrict sigdefault)
{
	_posix_spawnattr_t psattr;

	if (attr == NULL || *attr == NULL)
		return EINVAL;

	psattr = *(_posix_spawnattr_t *)attr;
	psattr->psa_sigdefault = *sigdefault;

	return (0);
}


/*
 * posix_spawnattr_setpgroup
 *
 * Description:	Set the value of the spawn process group attribute for the
 *		spawn attributes object referenced by 'attr' from the value
 *		of 'pgroup'
 *
 * Parameters:	attr			The spawn attributes object for which
 *					the process group information is to be
 *					set
 *		pgroup			The process group to set
 *
 * Returns:	0			Success
 */
int
posix_spawnattr_setpgroup(posix_spawnattr_t * attr, pid_t pgroup)
{
	_posix_spawnattr_t psattr;

	if (attr == NULL || *attr == NULL)
		return EINVAL;

	psattr = *(_posix_spawnattr_t *)attr;
	psattr->psa_pgroup = pgroup;

	return (0);
}


/*
 * posix_spawnattr_setsigmask
 *
 * Description:	Set the set of signals to be masked for the spawn attribute
 *		value referenced by 'attr' from the memory containing the
 *		sigset_t referenced by 'sigmask'
 *
 * Parameters:	attr			The spawn attributes object whose
 *					signal set for masked signals is to
 *					be set
 *		sigmask		A pointer to the sigset_t from which to
 *					obtain the signal set
 *
 * Returns:	0			Success
 */
int
posix_spawnattr_setsigmask(posix_spawnattr_t * __restrict attr,
		const sigset_t * __restrict sigmask)
{
	_posix_spawnattr_t psattr;

	if (attr == NULL || *attr == NULL)
		return EINVAL;

	psattr = *(_posix_spawnattr_t *)attr;
	psattr->psa_sigmask = *sigmask;

	return (0);
}


/*
 * posix_spawnattr_setbinpref_np
 *
 * Description:	Set the universal binary preferences for the spawn attribute
 *		value referenced by 'attr' from the memory containing the
 *		cpu_type_t array referenced by 'pref', size of 'count'
 *
 * Parameters:	attr			The spawn attributes object whose
 * 					binary preferences are to be set
 * 		count			Size of the array pointed to by 'pref'
 * 		pref			cpu_type_t array of binary preferences
 *		ocount			The actual number copied
 *
 * Returns:	0			No preferences copied
 * 		> 0			Number of preferences copied
 *
 * Note:	The posix_spawnattr_t currently only holds four cpu_type_t's. 
 * 		If the caller provides more preferences than this limit, they
 * 		will be ignored, as reflected in the return value.
 */
int
posix_spawnattr_setbinpref_np(posix_spawnattr_t * __restrict attr,
		size_t count, cpu_type_t *pref, size_t * __restrict ocount)
{
	_posix_spawnattr_t psattr;
	int i = 0;

	if (attr == NULL || *attr == NULL)
		return EINVAL;

	psattr = *(_posix_spawnattr_t *)attr;
	for (i = 0; i < count && i < 4; i++) {
		psattr->psa_binprefs[i] = pref[i];
	}

	/* return number of binprefs copied over */
	if (ocount)
		*ocount = i;
	return 0;
}


/*
 * posix_spawnattr_setpcontrol_np
 *
 * Description:	Set the process control property according to
 *		attribute value referenced by 'attr' from the memory
 *		containing the int value 'pcontrol'
 *
 * Parameters:	attr			The spawn attributes object whose
 *					signal set for default signals is to
 *					be set
 *		pcontrol		An int value of the process control info
 *
 * Returns:	0			Success
 */
int
posix_spawnattr_setpcontrol_np(posix_spawnattr_t * __restrict attr,
		const int pcontrol)
{
	_posix_spawnattr_t psattr;

	if (attr == NULL || *attr == NULL)
		return EINVAL;

	psattr = *(_posix_spawnattr_t *)attr;
	psattr->psa_pcontrol = pcontrol;

	return (0);
}


/*
 * posix_spawnattr_setapptype_np
 *
 * Description:	Set the process specific behaviors and app launch type
 *		attribute value referenced by 'attr' from the memory
 *		containing the int value 'apptype'
 *
 * Parameters:	attr			The spawn attributes object whose
 *					signal set for default signals is to
 *					be set
 *		apptype			An int value of the apptype info
 *
 * Returns:	0			Success
 */
int
posix_spawnattr_setapptype_np(posix_spawnattr_t * __restrict attr,
		const int apptype)
{
	_posix_spawnattr_t psattr;

	if (attr == NULL || *attr == NULL)
		return EINVAL;

	psattr = *(_posix_spawnattr_t *)attr;
	psattr->psa_apptype = apptype;

	return (0);
}

/*
 * posix_spawn_createportactions_np
 * Description: create a new posix_spawn_port_actions struct and link
 * 	it into the posix_spawnattr.
 */
static int
posix_spawn_createportactions_np(posix_spawnattr_t *attr)
{
	_posix_spawnattr_t psattr;
	_posix_spawn_port_actions_t acts;

	if (attr == NULL || *attr == NULL)
		return EINVAL;

	psattr = *(_posix_spawnattr_t *)attr;
	acts = (_posix_spawn_port_actions_t)malloc(PS_PORT_ACTIONS_SIZE(2));
	if (acts == NULL)
		return ENOMEM;
	
	acts->pspa_alloc = 2;
	acts->pspa_count = 0;

	psattr->psa_ports = acts;
	return 0;
}

/*
 * posix_spawn_growportactions_np
 * Description: Enlarge the size of portactions if necessary 
 */
static int
posix_spawn_growportactions_np(posix_spawnattr_t *attr)
{
	_posix_spawnattr_t psattr;
	_posix_spawn_port_actions_t acts; 
	int newnum;

	if (attr == NULL || *attr == NULL)
		return EINVAL;

	psattr = *(_posix_spawnattr_t *)attr;
	acts = psattr->psa_ports; 
	if (acts == NULL)
		return EINVAL;
	
	/* Double number of port actions allocated for */
	newnum = 2 * acts->pspa_alloc;
	acts = realloc(acts, PS_PORT_ACTIONS_SIZE(newnum));
	if (acts == NULL)
		return ENOMEM;
	
	acts->pspa_alloc = newnum;
	return 0;
}

/*
 * posix_spawn_destroyportactions_np
 * Description: clean up portactions struct in posix_spawnattr_t attr
 */
static int
posix_spawn_destroyportactions_np(posix_spawnattr_t *attr)
{
	_posix_spawnattr_t psattr;
	_posix_spawn_port_actions_t acts; 

	if (attr == NULL || *attr == NULL)
		return EINVAL;

	psattr = *(_posix_spawnattr_t *)attr;
	acts = psattr->psa_ports; 
	if (acts == NULL)
		return EINVAL;
	
	free(acts);
	return 0;
}


/*
 * posix_spawnattr_setspecialport_np
 *
 * Description:	Set a new value for a mach special port in the spawned task. 
 *
 * Parameters:	attr			The spawn attributes object for the
 * 					new process
 * 		new_port		The new value for the special port
 * 		which			The particular port to be set
 * 					(see task_set_special_port for details)
 *
 * Returns:	0			Success
 * 		ENOMEM			Couldn't allocate memory
 */
int    
posix_spawnattr_setspecialport_np(
		posix_spawnattr_t *attr,
		mach_port_t      new_port,
		int             which)
{
	_posix_spawnattr_t psattr;
	int err = 0;
	_ps_port_action_t *action;
	_posix_spawn_port_actions_t ports;

	if (attr == NULL || *attr == NULL)
		return EINVAL;

	psattr = *(_posix_spawnattr_t *)attr;
	ports = psattr->psa_ports;
	/* Have any port actions been created yet? */
	if (ports == NULL) {
		err = posix_spawn_createportactions_np(attr);
		if (err) 
			return err;
		ports = psattr->psa_ports;
	}
	
	/* Is there enough room? */
	if (ports->pspa_alloc == ports->pspa_count) {
		err = posix_spawn_growportactions_np(attr);
		if (err)
			return err;
	}

	/* Add this action to next spot in array */
	action = &ports->pspa_actions[ports->pspa_count];
	action->port_type = PSPA_SPECIAL;
	action->new_port = new_port;
	action->which = which;
	
	ports->pspa_count++;
	return err;
}

/*
 * posix_spawnattr_setexceptionports_np
 *
 * Description:	Set a new port for a set of exception ports in the spawned task.
 *
 * Parameters:	attr			The spawn attributes object for the
 * 					new process
 * 		mask			A bitfield indicating which exceptions
 * 					to associate the port with
 * 		new_port		The new value for the exception port
 * 		behavior		The default behavior for the port
 * 		flavor			The default flavor for the port
 * 					(see task_set_exception_ports)
 *
 * Returns:	0			Success
 */
int    
posix_spawnattr_setexceptionports_np(
		posix_spawnattr_t       *attr,
		exception_mask_t        mask,
		mach_port_t              new_port,
		exception_behavior_t    behavior,
		thread_state_flavor_t   flavor)
{
	_posix_spawnattr_t psattr;
	int err = 0;
	_ps_port_action_t *action;
	_posix_spawn_port_actions_t ports;

	if (attr == NULL || *attr == NULL)
		return EINVAL;

	psattr = *(_posix_spawnattr_t *)attr;
	ports = psattr->psa_ports;
	/* Have any port actions been created yet? */
	if (ports == NULL) {
		err = posix_spawn_createportactions_np(attr);
		if (err) 
			return err;
		ports = psattr->psa_ports;
	}
	
	/* Is there enough room? */
	if (ports->pspa_alloc == ports->pspa_count) {
		err = posix_spawn_growportactions_np(attr);
		if (err)
			return err;
	}

	/* Add this action to next spot in array */
	action = &ports->pspa_actions[ports->pspa_count];
	action->port_type = PSPA_EXCEPTION;
	action->mask = mask;
	action->new_port = new_port;
	action->behavior = behavior;
	action->flavor = flavor;
	
	ports->pspa_count++;
	return err;
}

/*
 * posix_spawnattr_setauditsessionport_np
 *
 * Description:	Set the audit session port rights attribute in the spawned task.
 *		This is used to securely set the audit session information for
 *		the new task.
 *
 * Parameters:	attr			The spawn attributes object for the
 * 					new process
 * 		au_sessionport		The audit session send port right
 *
 * Returns:	0			Success
 */
int    
posix_spawnattr_setauditsessionport_np(
		posix_spawnattr_t       *attr,
		mach_port_t              au_sessionport)
{
	_posix_spawnattr_t psattr;
	int err = 0;
	_ps_port_action_t *action;
	_posix_spawn_port_actions_t ports;

	if (attr == NULL || *attr == NULL)
		return EINVAL;

	psattr = *(_posix_spawnattr_t *)attr;
	ports = psattr->psa_ports;
	/* Have any port actions been created yet? */
	if (ports == NULL) {
		err = posix_spawn_createportactions_np(attr);
		if (err) 
			return err;
		ports = psattr->psa_ports;
	}
	
	/* Is there enough room? */
	if (ports->pspa_alloc == ports->pspa_count) {
		err = posix_spawn_growportactions_np(attr);
		if (err)
			return err;
	}

	/* Add this action to next spot in array */
	action = &ports->pspa_actions[ports->pspa_count];
	action->port_type = PSPA_AU_SESSION;
	action->new_port = au_sessionport;
	
	ports->pspa_count++;
	return err;
}


/*
 * posix_spawn_file_actions_init
 *
 * Description:	Initialize a spawn file actions object attr with default values
 *
 * Parameters:	file_actions		The spawn file actions object to be
 *					initialized
 *
 * Returns:	0			Success
 *		ENOMEM			Insufficient memory exists to
 *					initialize the spawn file actions
 *					object.
 *
 * Note:	As an implementation detail, the externally visibily type
 *		posix_spawn_file_actions_t is defined to be a void *, and
 *		initialization involves allocation of a memory object.
 *		Subsequent changes to the spawn file actions may result in
 *		reallocation under the covers.
 *
 *		Reinitialization of an already initialized spawn file actions
 *		object will result in memory being leaked.  Because spawn
 *		file actions are not required to be used in conjunction with a
 *		static initializer, there is no way to distinguish a spawn
 *		file actions with stack garbage from one that's been
 *		initialized.  This is arguably an API design error.
 */
int
posix_spawn_file_actions_init(posix_spawn_file_actions_t *file_actions)
{
	_posix_spawn_file_actions_t *psactsp = (_posix_spawn_file_actions_t *)file_actions;
	int	err = 0;

	if ((*psactsp = (_posix_spawn_file_actions_t)malloc(PSF_ACTIONS_SIZE(PSF_ACTIONS_INIT_COUNT))) == NULL) {
		err = ENOMEM;
	} else {
		(*psactsp)->psfa_act_alloc = PSF_ACTIONS_INIT_COUNT;
		(*psactsp)->psfa_act_count = 0;
	}

	return (err);
}


/*
 * posix_spawn_file_actions_destroy
 *
 * Description:	Destroy a spawn file actions object that was previously
 *		initialized via posix_spawn_file_actions_init() by freeing any
 *		memory associated with it and setting it to an invalid value.
 *
 * Parameters:	attr			The spawn file actions object to be
 *					destroyed.
 *
 * Returns:	0			Success
 *
 * Notes:	The destroyed spawn file actions results in the void * pointer
 *		being set to NULL; subsequent use without reinitialization
 *		will result in explicit program failure (rather than merely
 *		"undefined behaviour").
 *
 * NOTIMP:	Allowed failures (checking NOT required):
 *		EINVAL	The value specified by file_actions is invalid.
 */
int
posix_spawn_file_actions_destroy(posix_spawn_file_actions_t *file_actions)
{
	_posix_spawn_file_actions_t psacts;

	if (file_actions == NULL || *file_actions == NULL)
		return EINVAL;

	psacts = *(_posix_spawn_file_actions_t *)file_actions;
	free(psacts);
	*file_actions = NULL;

	return (0);
}


/*
 * _posix_spawn_file_actions_grow
 *
 * Description:	Grow the available list of file actions associated with the
 *		pointer to the structure provided; replace the contents of the
 *		pointer as a side effect.
 *
 * Parameters:	psactsp			Pointer to _posix_spawn_file_actions_t
 *					to grow
 *
 * Returns:	0			Success
 *		ENOMEM			Insufficient memory for operation
 *
 * Notes:	This code is common to all posix_spawn_file_actions_*()
 *		functions, since we use a naieve data structure implementation
 *		at present.  Future optimization will likely change this.
 */
static int
_posix_spawn_file_actions_grow(_posix_spawn_file_actions_t *psactsp)
{
	int	new_alloc = (*psactsp)->psfa_act_alloc * 2;
	_posix_spawn_file_actions_t new_psacts;

	/*
	 * XXX may want to impose an administrative limit here; POSIX does
	 * XXX not provide for an administrative error return in this case,
	 * XXX so it's probably acceptable to just fail catastrophically
	 * XXX instead of implementing one.
	 */
	if ((new_psacts = (_posix_spawn_file_actions_t)realloc((*psactsp), PSF_ACTIONS_SIZE(new_alloc))) == NULL) {
		return (ENOMEM);
	}
	new_psacts->psfa_act_alloc = new_alloc;
	*psactsp = new_psacts;

	return (0);
}


/*
 * posix_spawn_file_actions_addopen
 *
 * Description:	Add an open action to the object referenced by 'file_actions'
 *		that will cause the file named by 'path' to be attempted to be
 *		opened with flags 'oflag' and mode 'mode', and, if successful,
 *		return as descriptor 'filedes' to the spawned process.
 *
 * Parameters:	file_actions		File action object to augment
 *		filedes			fd that open is to use
 *		path			path to file to open
 *		oflag			open file flags
 *		mode			open file mode
 *
 * Returns:	0			Success
 *		EBADF			The value specified by fildes is
 *					negative or greater than or equal to
 *					{OPEN_MAX}.
 *		ENOMEM			Insufficient memory exists to add to
 *					the spawn file actions object.
 *
 * NOTIMP:	Allowed failures (checking NOT required):
 *		EINVAL	The value specified by file_actions is invalid.
 */
int
posix_spawn_file_actions_addopen(
		posix_spawn_file_actions_t * __restrict file_actions,
		int filedes, const char * __restrict path, int oflag,
		mode_t mode)
{
	_posix_spawn_file_actions_t *psactsp;
	_psfa_action_t *psfileact;

	if (file_actions == NULL || *file_actions == NULL)
		return EINVAL;

	psactsp = (_posix_spawn_file_actions_t *)file_actions;
	/* Range check; required by POSIX */
	if (filedes < 0 || filedes >= OPEN_MAX)
		return (EBADF);

	/* If we do not have enough slots, grow the structure */
	if ((*psactsp)->psfa_act_count == (*psactsp)->psfa_act_alloc) {
		/* need to grow file actions structure */
		if (_posix_spawn_file_actions_grow(psactsp))
			return (ENOMEM);
	}

	/*
	 * Allocate next available slot and fill it out
	 */
	psfileact = &(*psactsp)->psfa_act_acts[(*psactsp)->psfa_act_count++];

	psfileact->psfaa_type = PSFA_OPEN;
	psfileact->psfaa_filedes = filedes;
	psfileact->psfaa_openargs.psfao_oflag = oflag;
	psfileact->psfaa_openargs.psfao_mode = mode;
	strlcpy(psfileact->psfaa_openargs.psfao_path, path, PATH_MAX);

	return (0);
}


/*
 * posix_spawn_file_actions_addclose
 *
 * Description:	Add a close action to the object referenced by 'file_actions'
 *		that will cause the file referenced by 'filedes' to be
 *		attempted to be closed in the spawned process.
 *
 * Parameters:	file_actions		File action object to augment
 *		filedes			fd to close
 *
 * Returns:	0			Success
 *		EBADF			The value specified by fildes is
 *					negative or greater than or equal to
 *					{OPEN_MAX}.
 *		ENOMEM			Insufficient memory exists to add to
 *					the spawn file actions object.
 *
 * NOTIMP:	Allowed failures (checking NOT required):
 *		EINVAL	The value specified by file_actions is invalid.
 */
int
posix_spawn_file_actions_addclose(posix_spawn_file_actions_t *file_actions,
		int filedes)
{
	_posix_spawn_file_actions_t *psactsp;
	_psfa_action_t *psfileact;

	if (file_actions == NULL || *file_actions == NULL)
		return EINVAL;

	psactsp = (_posix_spawn_file_actions_t *)file_actions;
	/* Range check; required by POSIX */
	if (filedes < 0 || filedes >= OPEN_MAX)
		return (EBADF);

	/* If we do not have enough slots, grow the structure */
	if ((*psactsp)->psfa_act_count == (*psactsp)->psfa_act_alloc) {
		/* need to grow file actions structure */
		if (_posix_spawn_file_actions_grow(psactsp))
			return (ENOMEM);
	}

	/*
	 * Allocate next available slot and fill it out
	 */
	psfileact = &(*psactsp)->psfa_act_acts[(*psactsp)->psfa_act_count++];

	psfileact->psfaa_type = PSFA_CLOSE;
	psfileact->psfaa_filedes = filedes;

	return (0);
}


/*
 * posix_spawn_file_actions_adddup2
 *
 * Description:	Add a dup2 action to the object referenced by 'file_actions'
 *		that will cause the file referenced by 'filedes' to be
 *		attempted to be dup2'ed to the descriptor 'newfiledes' in the
 *		spawned process.
 *
 * Parameters:	file_actions		File action object to augment
 *		filedes			fd to dup2
 *		newfiledes		fd to dup2 it to
 *
 * Returns:	0			Success
 *		EBADF			The value specified by either fildes
 *					or by newfiledes is negative or greater
 *					than or equal to {OPEN_MAX}.
 *		ENOMEM			Insufficient memory exists to add to
 *					the spawn file actions object.
 *
 * NOTIMP:	Allowed failures (checking NOT required):
 *		EINVAL	The value specified by file_actions is invalid.
 */
int
posix_spawn_file_actions_adddup2(posix_spawn_file_actions_t *file_actions,
		int filedes, int newfiledes)
{
	_posix_spawn_file_actions_t *psactsp;
	_psfa_action_t *psfileact;

	if (file_actions == NULL || *file_actions == NULL)
		return EINVAL;

	psactsp = (_posix_spawn_file_actions_t *)file_actions;
	/* Range check; required by POSIX */
	if (filedes < 0 || filedes >= OPEN_MAX ||
	    newfiledes < 0 || newfiledes >= OPEN_MAX)
		return (EBADF);

	/* If we do not have enough slots, grow the structure */
	if ((*psactsp)->psfa_act_count == (*psactsp)->psfa_act_alloc) {
		/* need to grow file actions structure */
		if (_posix_spawn_file_actions_grow(psactsp))
			return (ENOMEM);
	}

	/*
	 * Allocate next available slot and fill it out
	 */
	psfileact = &(*psactsp)->psfa_act_acts[(*psactsp)->psfa_act_count++];

	psfileact->psfaa_type = PSFA_DUP2;
	psfileact->psfaa_filedes = filedes;
	psfileact->psfaa_openargs.psfao_oflag = newfiledes;

	return (0);
}

/*
 * posix_spawn_file_actions_addinherit_np
 *
 * Description:	Add the "inherit" action to the object referenced by
 *		'file_actions' that will cause the file referenced by
 *		'filedes' to continue to be available in the spawned
 *		process via the same descriptor.
 *
 *		Inheritance is the normal default behaviour for
 *		file descriptors across exec and spawn; but if the
 *		POSIX_SPAWN_CLOEXEC_DEFAULT flag is set, the usual
 *		default is reversed for the purposes of the spawn
 *		invocation.  Any pre-existing descriptors that
 *		need to be made available to the spawned process can
 *		be marked explicitly as 'inherit' via this interface.
 *		Otherwise they will be automatically closed.
 *
 *		Note that any descriptors created via the other file
 *		actions interfaces are automatically marked as 'inherit'.
 *
 * Parameters:	file_actions		File action object to augment
 *		filedes			fd to inherit.
 *
 * Returns:	0			Success
 *		EBADF			The value specified by fildes is
 *					negative or greater than or equal to
 *					{OPEN_MAX}.
 *		ENOMEM			Insufficient memory exists to add to
 *					the spawn file actions object.
 *
 * NOTIMP:	Allowed failures (checking NOT required):
 *		EINVAL	The value specified by file_actions is invalid.
 */
int
posix_spawn_file_actions_addinherit_np(posix_spawn_file_actions_t *file_actions,
		int filedes)
{
	_posix_spawn_file_actions_t *psactsp;
	_psfa_action_t *psfileact;

	if (file_actions == NULL || *file_actions == NULL)
		return (EINVAL);

	psactsp = (_posix_spawn_file_actions_t *)file_actions;
	/* Range check; required by POSIX */
	if (filedes < 0 || filedes >= OPEN_MAX)
		return (EBADF);

#if defined(POSIX_SPAWN_CLOEXEC_DEFAULT)	// TODO: delete this check
	/* If we do not have enough slots, grow the structure */
	if ((*psactsp)->psfa_act_count == (*psactsp)->psfa_act_alloc) {
		/* need to grow file actions structure */
		if (_posix_spawn_file_actions_grow(psactsp))
			return (ENOMEM);
	}

	/*
	 * Allocate next available slot and fill it out
	 */
	psfileact = &(*psactsp)->psfa_act_acts[(*psactsp)->psfa_act_count++];

	psfileact->psfaa_type = PSFA_INHERIT;
	psfileact->psfaa_filedes = filedes;
#endif
	return (0);
}

int
posix_spawnattr_setcpumonitor(posix_spawnattr_t * __restrict attr,
		uint64_t percent, uint64_t interval)
{
	_posix_spawnattr_t psattr;

	if (attr == NULL || *attr == NULL || percent == 0 || percent > 100)
		return (EINVAL);

	psattr = *(_posix_spawnattr_t *)attr;

	psattr->psa_cpumonitor_percent = percent;
	psattr->psa_cpumonitor_interval = interval;

	return (0);			
}

int
posix_spawnattr_getcpumonitor(posix_spawnattr_t * __restrict attr,
		uint64_t *percent, uint64_t *interval)
{
	_posix_spawnattr_t psattr;

	if (attr == NULL || *attr == NULL)
		return (EINVAL);

	psattr = *(_posix_spawnattr_t *)attr;

	*percent = psattr->psa_cpumonitor_percent;
	*interval = psattr->psa_cpumonitor_interval;

	return (0);
}

#if TARGET_OS_EMBEDDED
/*
 * posix_spawnattr_setjetsam
 *
 * Description:	Set jetsam attributes for the spawn attribute object
 *		referred to by 'attr'.
 *
 * Parameters:	flags			The flags value to set
 *		priority		Relative jetsam priority
 *		high_water_mark		Value in pages; resident page
 *					counts above this level can
 *					result in termination
 *
 * Returns:	0			Success
 */
int
posix_spawnattr_setjetsam(posix_spawnattr_t * __restrict attr,
		short flags, int priority, int high_water_mark)
{
	_posix_spawnattr_t psattr;

	if (attr == NULL || *attr == NULL)
		return EINVAL;

	psattr = *(_posix_spawnattr_t *)attr;
	
	psattr->psa_jetsam_flags = flags;
	psattr->psa_priority = priority;
	psattr->psa_high_water_mark = high_water_mark;

	return (0);
}
#endif

/*
 * posix_spawnp
 *
 * Description:	Create a new process from the process image corresponding to
 *		the supplied 'file' argument and the parent processes path
 *		environment.
 *
 * Parameters:	pid				Pointer to pid_t to receive the
 *						PID of the spawned process, if
 *						successful and 'pid' != NULL
 *		file				Name of image file to spawn
 *		file_actions			spawn file actions object which
 *						describes file actions to be
 *						performed during the spawn
 *		attrp				spawn attributes object which
 *						describes attributes to be
 *						applied during the spawn
 *		argv				argument vector array; NULL
 *						terminated
 *		envp				environment vector array; NULL
 *						terminated
 *
 * Returns:	0				Success
 *		!0				An errno value indicating the
 *						cause of the failure to spawn
 *
 * Notes:	Much of this function is derived from code from execvP() from
 *		exec.c in libc; this common code should be factored out at
 *		some point to prevent code duplication or desynchronization vs.
 *		bug fixes applied to one set of code but not the other.
 */
int
posix_spawnp(pid_t * __restrict pid, const char * __restrict file,
		const posix_spawn_file_actions_t *file_actions,
		const posix_spawnattr_t * __restrict attrp,
		char *const argv[ __restrict], char *const envp[ __restrict])
{
	const char *env_path;
	char *bp;
	char *cur;
	char *p;
	char **memp;
	int lp;
	int ln;
	int cnt;
	int err = 0;
	int eacces = 0;
	struct stat sb;
	char path_buf[PATH_MAX];

	if ((env_path = getenv("PATH")) == NULL)
		env_path = _PATH_DEFPATH;

	/* If it's an absolute or relative path name, it's easy. */
	if (index(file, '/')) {
		bp = (char *)file;
		cur = NULL;
		goto retry;
	}
	bp = path_buf;

	/* If it's an empty path name, fail in the usual POSIX way. */
	if (*file == '\0')
		return (ENOENT);

	if ((cur = alloca(strlen(env_path) + 1)) == NULL)
		return ENOMEM;
	strcpy(cur, env_path);
	while ((p = strsep(&cur, ":")) != NULL) {
		/*
		 * It's a SHELL path -- double, leading and trailing colons
		 * mean the current directory.
		 */
		if (*p == '\0') {
			p = ".";
			lp = 1;
		} else {
			lp = strlen(p);
		}
		ln = strlen(file);

		/*
		 * If the path is too long complain.  This is a possible
		 * security issue; given a way to make the path too long
		 * the user may spawn the wrong program.
		 */
		if (lp + ln + 2 > sizeof(path_buf)) {
			err = ENAMETOOLONG;
			goto done;
		}
		bcopy(p, path_buf, lp);
		path_buf[lp] = '/';
		bcopy(file, path_buf + lp + 1, ln);
		path_buf[lp + ln + 1] = '\0';

retry:		err = posix_spawn(pid, bp, file_actions, attrp, argv, envp);
		switch (err) {
		case E2BIG:
		case ENOMEM:
		case ETXTBSY:
			goto done;
		case ELOOP:
		case ENAMETOOLONG:
		case ENOENT:
		case ENOTDIR:
			break;
		case ENOEXEC:
			for (cnt = 0; argv[cnt]; ++cnt)
				;
			memp = alloca((cnt + 2) * sizeof(char *));
			if (memp == NULL) {
				/* errno = ENOMEM; XXX override ENOEXEC? */
				goto done;
			}
			memp[0] = "sh";
			memp[1] = bp;
			bcopy(argv + 1, memp + 2, cnt * sizeof(char *));
			err = posix_spawn(pid, _PATH_BSHELL, file_actions, attrp, memp, envp);
			goto done;
		default:
			/*
			 * EACCES may be for an inaccessible directory or
			 * a non-executable file.  Call stat() to decide
			 * which.  This also handles ambiguities for EFAULT
			 * and EIO, and undocumented errors like ESTALE.
			 * We hope that the race for a stat() is unimportant.
			 */
			if (stat(bp, &sb) != 0)
				break;
			if (err == EACCES) {
				eacces = 1;
				continue;
			}
			goto done;
		}
	}
	if (eacces)
		err = EACCES;
	else
		err = ENOENT;
done:
	return (err);
}


/*
 * posix_spawn
 *
 * Description:	Create a new process from the process image corresponding to
 *		the supplied 'path' argument.
 *
 * Parameters:	pid				Pointer to pid_t to receive the
 *						PID of the spawned process, if
 *						successful and 'pid' != NULL
 *		path				Path of image file to spawn
 *		file_actions			spawn file actions object which
 *						describes file actions to be
 *						performed during the spawn
 *		attrp				spawn attributes object which
 *						describes attributes to be
 *						applied during the spawn
 *		argv				argument vector array; NULL
 *						terminated
 *		envp				environment vector array; NULL
 *						terminated
 *
 * Returns:	0				Success
 *		!0				An errno value indicating the
 *						cause of the failure to spawn
 *
 * Notes:	Unlike other system calls, the return value of this system
 *		call is expected to either be a 0 or an errno, rather than a
 *		0 or a -1, with the 'errno' variable being set.
 */
extern int __posix_spawn(pid_t * __restrict, const char * __restrict,
		struct _posix_spawn_args_desc *,
		char *const argv[ __restrict], char *const envp[ __restrict]);

int
posix_spawn(pid_t * __restrict pid, const char * __restrict path,
		const posix_spawn_file_actions_t *file_actions,
		const posix_spawnattr_t * __restrict attrp,
		char *const argv[ __restrict], char *const envp[ __restrict])
{
	int saveerrno = errno;
	int ret;
	/*
	 * Only do extra work if we have file actions or attributes to push
	 * down.  We use a descriptor to push this information down, since we
	 * want to have size information, which will let us (1) preallocate a
	 * single chunk of memory for the copyin(), and (2) allow us to do a
	 * single copyin() per attributes or file actions as a monlithic block.
	 *
	 * Note:	A future implementation may attempt to do the same
	 *		thing for the argv/envp data, which could potentially
	 *		result in a performance improvement due to increased
	 *		kernel efficiency, even though it would mean copying
	 *		the data in user space.
	 */
	if ((file_actions != NULL && (*file_actions != NULL) &&	(*(_posix_spawn_file_actions_t *)file_actions)->psfa_act_count > 0) || attrp != NULL) {
		struct _posix_spawn_args_desc	ad;

		memset(&ad, 0, sizeof(ad));
		if (attrp != NULL && *attrp != NULL) {
			_posix_spawnattr_t psattr = *(_posix_spawnattr_t *)attrp;
			ad.attr_size = sizeof(struct _posix_spawnattr);
			ad.attrp = psattr;

			if (psattr->psa_ports != NULL) {
				ad.port_actions = psattr->psa_ports;
				ad.port_actions_size = PS_PORT_ACTIONS_SIZE(
						ad.port_actions->pspa_count);
			}
		}
		if (file_actions != NULL && *file_actions != NULL) {
			_posix_spawn_file_actions_t psactsp =
				*(_posix_spawn_file_actions_t *)file_actions;

			if (psactsp->psfa_act_count > 0) {
				ad.file_actions_size = PSF_ACTIONS_SIZE(psactsp->psfa_act_count);
				ad.file_actions = psactsp;
			}
		}

		ret = __posix_spawn(pid, path, &ad, argv, envp);
	} else
		ret = __posix_spawn(pid, path, NULL, argv, envp);

	if (ret < 0)
		ret = errno;
	errno = saveerrno;
	return ret;
}

