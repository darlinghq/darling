/*
 * Copyright (c) 2006, 2010 Apple Inc. All rights reserved.
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

#include <sys/cdefs.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <sys/errno.h>
#include <sys/msgbuf.h>
#define CONFIG_EMBEDDED 1
#include <sys/process_policy.h>

#include "libproc_internal.h"

int __proc_info(int callnum, int pid, int flavor, uint64_t arg, void * buffer, int buffersize);
__private_extern__ int proc_setthreadname(void * buffer, int buffersize);
int __process_policy(int scope, int action, int policy, int policy_subtype, proc_policy_attribute_t * attrp, pid_t target_pid, uint64_t target_threadid);



int 
proc_listpids(uint32_t type, uint32_t typeinfo, void *buffer, int buffersize) 
{
	int retval;
	
	if ((type >= PROC_ALL_PIDS) || (type <= PROC_PPID_ONLY)) {
		if ((retval = __proc_info(1, type, typeinfo,(uint64_t)0, buffer, buffersize)) == -1)
			return(0);
	} else {
		errno = EINVAL;
		retval = 0;
	}
	return(retval);
}


int 
proc_listallpids(void * buffer, int buffersize)
{
	int numpids;
	numpids = proc_listpids(PROC_ALL_PIDS, (uint32_t)0, buffer, buffersize);

	if (numpids == -1)
		return(-1);
	else
		return(numpids/sizeof(int));
}

int 
proc_listpgrppids(pid_t pgrpid, void * buffer, int buffersize)
{
	int numpids;
	numpids = proc_listpids(PROC_PGRP_ONLY, (uint32_t)pgrpid, buffer, buffersize);
	if (numpids == -1)
		return(-1);
	else
		return(numpids/sizeof(int));
}

int 
proc_listchildpids(pid_t ppid, void * buffer, int buffersize)
{
	int numpids;
	numpids = proc_listpids(PROC_PPID_ONLY, (uint32_t)ppid, buffer, buffersize);
	if (numpids == -1)
		return(-1);
	else
		return(numpids/sizeof(int));
}


int 
proc_pidinfo(int pid, int flavor, uint64_t arg,  void *buffer, int buffersize)
{
	int retval;

	if ((retval = __proc_info(2, pid, flavor,  arg,  buffer, buffersize)) == -1)
		return(0);
		
	return(retval);
}


int	
proc_pidfdinfo(int pid, int fd, int flavor, void * buffer, int buffersize)
{
	int retval;

	if ((retval = __proc_info(3, pid,  flavor, (uint64_t)fd, buffer, buffersize)) == -1)
		return(0);
		
	return (retval);
}


int
proc_pidfileportinfo(int pid, uint32_t fileport, int flavor, void *buffer, int buffersize)
{
	int retval;

	if ((retval = __proc_info(6, pid, flavor, (uint64_t)fileport, buffer, buffersize)) == -1)
		return (0);
	return (retval);
}


int
proc_name(int pid, void * buffer, uint32_t buffersize)
{
	int retval = 0, len;
	struct proc_bsdinfo pbsd;
	
	
	if (buffersize < sizeof(pbsd.pbi_name)) {
		errno = ENOMEM;
		return(0);
	}

	retval = proc_pidinfo(pid, PROC_PIDTBSDINFO, (uint64_t)0, &pbsd, sizeof(struct proc_bsdinfo));
	if (retval != 0) {
		if (pbsd.pbi_name[0]) {
			bcopy(&pbsd.pbi_name, buffer, sizeof(pbsd.pbi_name));
		} else {
			bcopy(&pbsd.pbi_comm, buffer, sizeof(pbsd.pbi_comm));
		}
		len = strlen(buffer);
		return(len);
	}
	return(0);
}

int 
proc_regionfilename(int pid, uint64_t address, void * buffer, uint32_t buffersize)
{
	int retval = 0, len;
	struct proc_regionwithpathinfo reginfo;
	
	if (buffersize < MAXPATHLEN) {
		errno = ENOMEM;
		return(0);
	}
	
	retval = proc_pidinfo(pid, PROC_PIDREGIONPATHINFO, (uint64_t)address, &reginfo, sizeof(struct proc_regionwithpathinfo));
	if (retval != -1) {
		len = strlen(&reginfo.prp_vip.vip_path[0]);
		if (len != 0) {
			if (len > MAXPATHLEN)
				len = MAXPATHLEN;
			bcopy(&reginfo.prp_vip.vip_path[0], buffer, len);
			return(len);
		}
		return(0);
	}
	return(0);
			
}

int
proc_kmsgbuf(void * buffer, uint32_t  buffersize)
{
	int retval;

	if ((retval = __proc_info(4, 0,  0, (uint64_t)0, buffer, buffersize)) == -1)
		return(0);
	return (retval);
}

int
proc_pidpath(int pid, void * buffer, uint32_t  buffersize)
{
	int retval, len;

	if (buffersize < PROC_PIDPATHINFO_SIZE) {
		errno = ENOMEM;
		return(0);
	}
	if (buffersize >  PROC_PIDPATHINFO_MAXSIZE) {
		errno = EOVERFLOW;
		return(0);
	}

	retval = __proc_info(2, pid, PROC_PIDPATHINFO,  (uint64_t)0,  buffer, buffersize);
	if (retval != -1) {
		len = strlen(buffer);
		return(len);
	}
	return (0);
}


int 
proc_libversion(int *major, int * minor)
{

	if (major != NULL)
		*major = 1;
	if (minor != NULL)
		*minor = 1;
	return(0);
}

int
proc_setpcontrol(const int control)
{
	int retval ;

	if (control < PROC_SETPC_NONE || control > PROC_SETPC_TERMINATE)
		return(EINVAL);

	if ((retval = __proc_info(5, getpid(), PROC_SELFSET_PCONTROL, (uint64_t)control, NULL, 0)) == -1)
		return(errno);
		
	return(0);
}


__private_extern__ int
proc_setthreadname(void * buffer, int buffersize)
{
	int retval;

        retval = __proc_info(5, getpid(), PROC_SELFSET_THREADNAME, (uint64_t)0, buffer, buffersize);

	if (retval == -1)
                return(errno);
	else
		return(0);
}

int
proc_track_dirty(pid_t pid, uint32_t flags)
{
	if (__proc_info(8, pid, PROC_DIRTYCONTROL_TRACK, flags, NULL, 0) == -1) {
		return errno;
	}
		
	return 0;
}

int
proc_set_dirty(pid_t pid, bool dirty)
{
	if (__proc_info(8, pid, PROC_DIRTYCONTROL_SET, dirty, NULL, 0) == -1) {
		return errno;		
	}

	return 0;
}

int
proc_get_dirty(pid_t pid, uint32_t *flags)
{
	int retval;
	
	if (!flags) {
		return EINVAL;
	}
	
	retval = __proc_info(8, pid, PROC_DIRTYCONTROL_GET, 0, NULL, 0);
	if (retval == -1) {
		return errno;		
	}
	
	*flags = retval;

	return 0;
}

int
proc_terminate(pid_t pid, int *sig)
{
	int retval;
	
	if (!sig) {
		return EINVAL;
	}
	
	retval = __proc_info(7, pid, 0, 0, NULL, 0);
	if (retval == -1) {
		return errno;		
	}
	
	*sig = retval;
	
	return 0;
}

#if TARGET_OS_EMBEDDED

int 
proc_setcpu_percentage(pid_t pid, int action, int percentage)
{
	proc_policy_cpuusage_attr_t attr;

	bzero(&attr, sizeof(proc_policy_cpuusage_attr_t));
	attr.ppattr_cpu_attr = action;
	attr.ppattr_cpu_percentage = percentage;
	if (__process_policy(PROC_POLICY_SCOPE_PROCESS, PROC_POLICY_ACTION_APPLY, PROC_POLICY_RESOURCE_USAGE, PROC_POLICY_RUSAGE_CPU, &attr, pid, (uint64_t)0) != -1)
		return(0);
	else
		return(errno);
}

int
proc_setcpu_deadline(pid_t pid, int action, uint64_t deadline)
{
	proc_policy_cpuusage_attr_t attr;

	bzero(&attr, sizeof(proc_policy_cpuusage_attr_t));
	attr.ppattr_cpu_attr = action;
	attr.ppattr_cpu_attr_deadline = deadline;
	if (__process_policy(PROC_POLICY_SCOPE_PROCESS, PROC_POLICY_ACTION_APPLY, PROC_POLICY_RESOURCE_USAGE, PROC_POLICY_RUSAGE_CPU, &attr, pid, (uint64_t)0) != -1)
		return(0);
	else
		return(errno);

}


int
proc_setcpu_percentage_withdeadline(pid_t pid, int action, int percentage, uint64_t deadline)
{
	proc_policy_cpuusage_attr_t attr;

	bzero(&attr, sizeof(proc_policy_cpuusage_attr_t));
	attr.ppattr_cpu_attr = action;
	attr.ppattr_cpu_percentage = percentage;
	attr.ppattr_cpu_attr_deadline = deadline;
	if (__process_policy(PROC_POLICY_SCOPE_PROCESS, PROC_POLICY_ACTION_APPLY, PROC_POLICY_RESOURCE_USAGE, PROC_POLICY_RUSAGE_CPU, &attr, pid, (uint64_t)0) != -1)
		return(0);
	else
		return(errno);
}

int
proc_clear_cpulimits(pid_t pid)
{
	if (__process_policy(PROC_POLICY_SCOPE_PROCESS, PROC_POLICY_ACTION_RESTORE, PROC_POLICY_RESOURCE_USAGE, PROC_POLICY_RUSAGE_CPU, NULL, pid, (uint64_t)0) != -1)
		return(0);
	else
		return(errno);


}

int
proc_appstate(int pid, int * appstatep)
{
	int state;

	if (__process_policy(PROC_POLICY_SCOPE_PROCESS, PROC_POLICY_ACTION_GET, PROC_POLICY_APP_LIFECYCLE, PROC_POLICY_APPLIFE_STATE, (int *)&state, pid, (uint64_t)0) != -1) {
		if (appstatep != NULL)
			*appstatep = state;
		return(0);
	 } else
		return(errno);

}


int
proc_setappstate(int pid, int appstate)
{
	int state = appstate;

	switch (state) {
		case PROC_APPSTATE_NONE:
		case PROC_APPSTATE_ACTIVE:
		case PROC_APPSTATE_INACTIVE:
		case PROC_APPSTATE_BACKGROUND:
		case PROC_APPSTATE_NONUI:
			break;
		default:
			return(EINVAL);
	}
	if (__process_policy(PROC_POLICY_SCOPE_PROCESS, PROC_POLICY_ACTION_APPLY, PROC_POLICY_APP_LIFECYCLE, PROC_POLICY_APPLIFE_STATE, (int *)&state, pid, (uint64_t)0) != -1)
		return(0);
	else
		return(errno);
}

int 
proc_devstatusnotify(int devicestatus)
{
	int state = devicestatus;

	switch (devicestatus) {
		case PROC_DEVSTATUS_SHORTTERM:
		case PROC_DEVSTATUS_LONGTERM:
			break;
		default:
			return(EINVAL);
	}

	if (__process_policy(PROC_POLICY_SCOPE_PROCESS, PROC_POLICY_ACTION_APPLY, PROC_POLICY_APP_LIFECYCLE, PROC_POLICY_APPLIFE_DEVSTATUS, (int *)&state, getpid(), (uint64_t)0) != -1) {
		return(0);
	 } else
		return(errno);

}

int
proc_pidbind(int pid, uint64_t threadid, int bind)
{
	int state = bind; 
	pid_t passpid = pid;

	switch (bind) {
		case PROC_PIDBIND_CLEAR:
			passpid = getpid();	/* ignore pid on clear */
			break;
		case PROC_PIDBIND_SET:
			break;
		default:
			return(EINVAL);
	}
	if (__process_policy(PROC_POLICY_SCOPE_PROCESS, PROC_POLICY_ACTION_APPLY, PROC_POLICY_APP_LIFECYCLE, PROC_POLICY_APPLIFE_PIDBIND, (int *)&state, passpid, threadid) != -1)
		return(0);
	else
		return(errno);
}

#else /* TARGET_OS_EMBEDDED */

int
proc_clear_vmpressure(pid_t pid)
{
	if (__process_policy(PROC_POLICY_SCOPE_PROCESS, PROC_POLICY_ACTION_RESTORE, PROC_POLICY_RESOURCE_STARVATION, PROC_POLICY_RS_VIRTUALMEM, NULL, pid, (uint64_t)0) != -1)
		return(0);
	else
		return(errno);
}

/* set the current process as one who can resume suspended processes due to low virtual memory. Need to be root */
int 
proc_set_owner_vmpressure(void)
{
	int retval;

	if ((retval = __proc_info(5, getpid(), PROC_SELFSET_VMRSRCOWNER, (uint64_t)0, NULL, 0)) == -1)
		return(errno);
		
	return(0);
}

/* mark yourself to delay idle sleep on disk IO */
int 
proc_set_delayidlesleep(void)
{
	int retval;

	if ((retval = __proc_info(5, getpid(), PROC_SELFSET_DELAYIDLESLEEP, (uint64_t)1, NULL, 0)) == -1)
		return(errno);

	return(0);
}

/* Reset yourself to delay idle sleep on disk IO, if already set */
int 
proc_clear_delayidlesleep(void)
{
	int retval;

	if ((retval = __proc_info(5, getpid(), PROC_SELFSET_DELAYIDLESLEEP, (uint64_t)0, NULL, 0)) == -1)
		return(errno);

	return(0);
}

/* disable the launch time backgroudn policy and restore the process to default group */
int 
proc_disable_apptype(pid_t pid, int apptype)
{
	switch (apptype) {
		case PROC_POLICY_OSX_APPTYPE_TAL:
		case PROC_POLICY_OSX_APPTYPE_DASHCLIENT:
			break;
		default:
			return(EINVAL);

	}

	if (__process_policy(PROC_POLICY_SCOPE_PROCESS, PROC_POLICY_ACTION_DISABLE, PROC_POLICY_APPTYPE, apptype, NULL, pid, (uint64_t)0) != -1)
		return(0);
	else
		return(errno);

}

/* re-enable the launch time background policy if it had been disabled. */
int 
proc_enable_apptype(pid_t pid, int apptype)
{
	switch (apptype) {
		case PROC_POLICY_OSX_APPTYPE_TAL:
		case PROC_POLICY_OSX_APPTYPE_DASHCLIENT:
			break;
		default:
			return(EINVAL);

	}

	if (__process_policy(PROC_POLICY_SCOPE_PROCESS, PROC_POLICY_ACTION_ENABLE, PROC_POLICY_APPTYPE, apptype, NULL, pid, (uint64_t)0) != -1)
		return(0);
	else
		return(errno);
 
}

#endif /* TARGET_OS_EMBEDDED */

