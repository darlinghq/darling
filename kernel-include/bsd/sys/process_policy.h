/*
 * Copyright (c) 2010 Apple Computer, Inc. All rights reserved.
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

#ifndef _SYS_PROCESS_POLICY_H
#define _SYS_PROCESS_POLICY_H

#include <sys/cdefs.h>
#include <sys/param.h>
#include <sys/types.h>
#include <stdint.h>

#ifndef XNU_KERNEL_PRIVATE
#include <TargetConditionals.h>
#endif

__BEGIN_DECLS

/* defns of scope */
#define PROC_POLICY_SCOPE_PROCESS       1       /* the policy setting is for process wide effect */
#define PROC_POLICY_SCOPE_THREAD        2       /* the policy setting is for thread inside a proc */

/* defns  of actions with no attributes */
#define PROC_POLICY_ACTION_APPLY        1       /* enforce the set policy */
#define PROC_POLICY_ACTION_RESTORE      2       /* revert the applied action back */
#define PROC_POLICY_ACTION_DENYINHERIT  3       /* set for no inheritence of the specified policy */
#define PROC_POLICY_ACTION_DENYSELFSET  4       /* set for the process to set its own policy */
#define PROC_POLICY_ACTION_ENABLE       5       /* enable policy and its actions */
#define PROC_POLICY_ACTION_DISABLE      6       /* disable policy and its actions, also clears any actions that have already happened */
/* defns  of actions  with attributes */
#define PROC_POLICY_ACTION_SET          10      /* set the policy attributes */
#define PROC_POLICY_ACTION_GET          11      /* get the policy attributes */
#define PROC_POLICY_ACTION_ADD          12      /* add a policy attribute */
#define PROC_POLICY_ACTION_REMOVE       13      /* remove a policy attribute */
#define PROC_POLICY_ACTION_HOLD         14      /* hold an importance boost assertion */
#define PROC_POLICY_ACTION_DROP         15      /* drop an importance boost assertion */

/* policies */
#define PROC_POLICY NONE		0
#define PROC_POLICY_BACKGROUND          1       /* darwin background policy */
#define PROC_POLICY_HARDWARE_ACCESS     2       /* access to various hardware */
#define PROC_POLICY_RESOURCE_STARVATION 3       /* behavior on resource starvation */
#define PROC_POLICY_RESOURCE_USAGE      4       /* behavior on resource consumption */
#if CONFIG_EMBEDDED || (TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR)
#define PROC_POLICY_APP_LIFECYCLE       5       /* app life cycle management */
#else /* CONFIG_EMBEDDED */
#define PROC_POLICY_RESERVED            5       /* behavior on resource consumption */
#endif /* CONFIG_EMBEDDED */
#define PROC_POLICY_APPTYPE             6       /* behavior on resource consumption */
#define PROC_POLICY_BOOST               7       /* importance boost/drop */

/* sub policies for background policy */
#define PROC_POLICY_BG_NONE             0       /* none */
#define PROC_POLICY_BG_LOWCPUPRI        1       /* Low cpu priority */
#define PROC_POLICY_BG_DISKTHROTTLE     2       /* disk accesses throttled */
#define PROC_POLICY_BG_NETTHROTTLE      4       /* network accesses throttled */
#define PROC_POLICY_BG_GPUDENY          8       /* no access to GPU */
#if CONFIG_EMBEDDED || (TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR)
#define PROC_POLICY_BG_ALL            0x0F
#else /* CONFIG_EMBEDDED */
#define PROC_POLICY_BG_ALL            0x07
#endif /* CONFIG_EMBEDDED */
#define PROC_POLICY_BG_DEFAULT          PROC_POLICY_BG_ALL

/* sub policies for hardware */
#define PROC_POLICY_HWACCESS_NONE       0
#define PROC_POLICY_HWACCESS_DISK       1       /* disk access */
#define PROC_POLICY_HWACCESS_GPU        2       /* GPU access */
#define PROC_POLICY_HWACCESS_NETWORK    3       /* network access */
#define PROC_POLICY_HWACCESS_CPU        4       /* cpu access */

/* attribute values for disk hardware access, bit different as it should reflect IOPOL_XXX */
#define PROC_POLICY_DISKACC_NONE        0
#define PROC_POLICY_DISKACC_NORMAL      1       /* normal access to the disk */
#define PROC_POLICY_DISKACC_FULLACCESS  1       /* normal access to the disk */
#define PROC_POLICY_DISKACC_PASSIVE     2       /* treat the I/Os as passive */
#define PROC_POLICY_DISKACC_THROTTLE    3       /* throttle the disk IOs */
#define PROC_POLICY_DISKACC_DEFAULT     PROC_POLICY_DISKACC_FULLACCESS

/* attribute values for GPU hardware access */
#define PROC_POLICY_GPUACC_NONE 0
#define PROC_POLICY_GPUACC_FULLACCESS   0       /* complete access to the GPU */
#define PROC_POLICY_GPUACC_DENYACCESS   1       /* deny any access to the GPU */
#define PROC_POLICY_GPUACC_DEFAULT      PROC_POLICY_GPUACC_FULLACCESS /*  default is complete access */

/* atrribute values for  network hardware access */
#define PROC_POLICY_NETACC_NONE 0
#define PROC_POLICY_NETACC_FULLACCESS   0       /* complete access to the network */
#define PROC_POLICY_NETACC_THROTTLE     1       /* throttle access to network */
#define PROC_POLICY_NETACC_DEFAULT      PROC_POLICY_NETACC_FULLACCESS /*  default is complete access */

/* atrribute values for  network hardware access */
#define PROC_POLICY_CPUACC_NONE         0
#define PROC_POLICY_CPUACC_FULLACCESS   0       /* access to all avialable cpus */
#define PROC_POLICY_CPUACC_ONE          1       /* access to only one available cpu */
#define PROC_POLICY_CPUACC_LLCACHE      2       /* access to only one last level cache */
#define PROC_POLICY_CPUACC_DEFAULT      PROC_POLICY_CPUACC_FULLACCESS /*  default is access to all cpus */


/* System Resource management (ie usage and starvation related) definitions */

/* sub policies for resource starvation */
#define PROC_POLICY_RS_NONE             0
#define PROC_POLICY_RS_VIRTUALMEM       1       /* virtual memory starvation */

/* sub policies for resource usage */
#define PROC_POLICY_RUSAGE_NONE         0
#define PROC_POLICY_RUSAGE_WIREDMEM     1       /* wired memory usages */
#define PROC_POLICY_RUSAGE_VIRTMEM      2       /* virtual memory usage */
#define PROC_POLICY_RUSAGE_CPU          3       /* amount of cpu usage */
#define PROC_POLICY_RUSAGE_DISK         4       /* amount of disk usage */
#define PROC_POLICY_RUSAGE_NETWORK      5       /* amount of network usage */
#define PROC_POLICY_RUSAGE_POWER        6       /* amount of power/battery consumption */

/* attribute values for the resource usage and low resource - MUST match corresponding task definitions */
#define PROC_POLICY_RSRCACT_NONE        0
#define PROC_POLICY_RSRCACT_THROTTLE    1       /* throttle on resource condition */
#define PROC_POLICY_RSRCACT_SUSPEND     2       /* suspend on resource condition */
#define PROC_POLICY_RSRCACT_TERMINATE   3       /* kill on resource condition */
#define PROC_POLICY_RSRCACT_NOTIFY_KQ   4       /* send kqueue notification */
#define PROC_POLICY_RSRCACT_NOTIFY_EXC  5       /* send exception */

#define PROC_POLICY_CPUMON_DISABLE      0xFF    /* Disable CPU usage monitor */
#define PROC_POLICY_CPUMON_DEFAULTS     0xFE    /* Set default CPU usage monitor params */

/* sub policies for importance boost/drop */
#define PROC_POLICY_IMP_IMPORTANT       1       /* Important-level boost */
#define PROC_POLICY_IMP_STANDARD        2       /* Standard-level boost */
#define PROC_POLICY_IMP_DONATION        3       /* Mark a task as an importance source */

typedef struct proc_policy_attribute {
	uint32_t        ppattr_attribute;  /* the policy attribute to be modified or returned */
	uint32_t        ppattr_resv;       /* pad field */
	uint64_t        ppattr_value1;     /* 64bit policy specific attribute */
	uint64_t        ppattr_value2;     /* 64bit policy specific attribute */
	uint64_t        ppattr_value3;     /* 64bit policy specific attribute */
	uint64_t        ppattr_resv1[4];    /* reserved for future use */
} proc_policy_attribute_t;


typedef struct proc_policy_cpuusage_attr {
	uint32_t        ppattr_cpu_attr;  /* specified action as in PROC_POLICY_RSRCACT_xx */
	uint32_t        ppattr_cpu_percentage;       /* percentage of interval */
	uint64_t        ppattr_cpu_attr_interval;     /* 64bit interval in nsecs */
	uint64_t        ppattr_cpu_attr_deadline;     /* 64bit deadline in nsecs */
} proc_policy_cpuusage_attr_t;

#if CONFIG_EMBEDDED || (TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR)
/* sub policies for app lifecycle management */
#define PROC_POLICY_APPLIFE_NONE        0       /* does nothing.. */
#define PROC_POLICY_APPLIFE_STATE       1       /* sets the app to various lifecycle states */
#define PROC_POLICY_APPLIFE_DEVSTATUS   2       /* notes the device in inactive or short/long term */
#define PROC_POLICY_APPLIFE_PIDBIND     3       /* a thread is to be bound to another processes app state */
#endif /* CONFIG_EMBEDDED */

/* sub policies for PROC_POLICY_APPTYPE */
#define PROC_POLICY_APPTYPE_NONE        0       /* does nothing.. */
#define PROC_POLICY_APPTYPE_MODIFY      1       /* sets the app to various lifecycle states */
#if CONFIG_EMBEDDED || (TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR)
#define PROC_POLICY_APPTYPE_THREADTHR   2       /* notes the device in inactive or short/long term */
#endif /* CONFIG_EMBEDDED */

/* exported apptypes for PROC_POLICY_APPTYPE */
#define PROC_POLICY_OSX_APPTYPE_TAL             1       /* TAL-launched app */

#define PROC_POLICY_OSX_APPTYPE_DASHCLIENT      2       /* Dashboard client (deprecated) */
#define PROC_POLICY_IOS_DONATEIMP               6       /* donates priority imp (deprecated) */
#define PROC_POLICY_IOS_HOLDIMP                 7       /* hold importance assertion (deprecated) */
#define PROC_POLICY_IOS_DROPIMP                 8       /* drop importance assertion (deprecated) */

#ifndef KERNEL
int process_policy(int scope, int action, int policy, int policy_subtype, proc_policy_attribute_t * attrp, pid_t target_pid, uint64_t target_threadid);
#endif /* KERNEL */


__END_DECLS

#endif /*_SYS_PROCESS_POLICY_H */
