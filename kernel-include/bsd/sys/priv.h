/*
 * Copyright (c) 2010-2017 Apple Inc. All rights reserved.
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
/*-
 * Copyright (c) 2006 nCircle Network Security, Inc.
 * All rights reserved.
 *
 * This software was developed by Robert N. M. Watson for the TrustedBSD
 * Project under contract to nCircle Network Security, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR, NCIRCLE NETWORK SECURITY,
 * INC., OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $FreeBSD: src/sys/sys/priv.h,v 1.38.2.1.2.1 2009/10/25 01:10:29 kensmith Exp $
 */

/*
 * Kernel privilege checking interface.
 */
#ifndef _SYS_PRIV_H_
#define _SYS_PRIV_H_

/*
 * Privilege list, sorted loosely by kernel subsystem.
 *
 * Think carefully before adding or reusing one of these privileges -- are
 * there existing instances referring to the same privilege?  Particular
 * numeric privilege assignments are part of the kernel extension ABI.
 */

/*
 * The remaining privileges typically correspond to one or a small
 * number of specific privilege checks, and have (relatively) precise
 * meanings.  They are loosely sorted into a set of base system
 * privileges, such as the ability to reboot, and then loosely by
 * subsystem, indicated by a subsystem name.
 */
#define PRIV_ADJTIME                    1000    /* Set time adjustment. */
#define PRIV_PROC_UUID_POLICY           1001    /* Change process uuid policy table. */
#define PRIV_GLOBAL_PROC_INFO           1002    /* Query information for processes owned by other users */
#define PRIV_SYSTEM_OVERRIDE            1003    /* Override global system settings for various subsystems for a limited duration/system-mode */
#define PRIV_HW_DEBUG_DATA              1004    /* Extract hw-specific debug data (e.g. ECC data) */
#define PRIV_SELECTIVE_FORCED_IDLE      1005    /* Configure and control Selective Forced Idle (SFI) subsystem */
#define PRIV_PROC_TRACE_INSPECT         1006    /* Request trace memory of arbitrary process to be inspected */
#define PRIV_DARKBOOT                   1007    /* Manipulate the darkboot flag */
#define PRIV_WORK_INTERVAL              1008    /* Express details about a work interval */
#define PRIV_SMB_TIMEMACHINE_CONTROL    1009    /* Control Time Machine properties of an SMB share */
#define PRIV_AUDIO_LATENCY              1010    /* set audio latency requirements for background tracing */
#define PRIV_KTRACE_BACKGROUND          1011    /* Operate ktrace in the background */
#define PRIV_SETPRIORITY_DARWIN_ROLE    1012    /* Allow setpriority(PRIO_DARWIN_ROLE) */
#define PRIV_PACKAGE_EXTENSIONS         1013    /* Push package extension list used by vn_path_package_check() */
#define PRIV_TRIM_ACTIVE_FILE           1014    /* Allow freeing space out from under an active file  */
#define PRIV_PROC_CPUMON_OVERRIDE       1015    /* Allow CPU usage monitor parameters less restrictive than default */
#define PRIV_ENDPOINTSECURITY_CLIENT    1016    /* Allow EndpointSecurity clients to connect */
#define PRIV_AUDIT_SESSION_PORT         1017    /* Obtain send-right for arbitrary audit session's port. */

/*
 * Virtual memory privileges.
 */
#define PRIV_VM_PRESSURE        6000    /* Check VM pressure. */
#define PRIV_VM_JETSAM          6001    /* Adjust jetsam configuration. */
#define PRIV_VM_FOOTPRINT_LIMIT 6002    /* Adjust physical footprint limit. */

/*
 * Network stack privileges.
 */
#define PRIV_NET_PRIVILEGED_TRAFFIC_CLASS       10000   /* Set SO_PRIVILEGED_TRAFFIC_CLASS. */
#define PRIV_NET_PRIVILEGED_SOCKET_DELEGATE     10001   /* Set delegate on a socket */
#define PRIV_NET_INTERFACE_CONTROL              10002   /* Enable interface debug logging. */
#define PRIV_NET_PRIVILEGED_NETWORK_STATISTICS  10003   /* Access to all sockets */
#define PRIV_NET_PRIVILEGED_NECP_POLICIES       10004   /* Access to privileged Network Extension policies */
#define PRIV_NET_RESTRICTED_AWDL                10005   /* Access to restricted AWDL mode */
#define PRIV_NET_PRIVILEGED_NECP_MATCH          10006   /* Privilege verified by Network Extension policies */
#define PRIV_NET_QOSMARKING_POLICY_OVERRIDE     10007   /* Privilege verified by Network Extension policies */
#define PRIV_NET_RESTRICTED_INTCOPROC           10008   /* Access to internal co-processor network interfaces */

#define PRIV_NET_PRIVILEGED_MULTIPATH           10009   /* Multipath usage */
#define PRIV_NET_RESTRICTED_MULTIPATH_EXTENDED  10010   /* Extended multipath (more aggressive on cell) */
#define PRIV_NET_RESTRICTED_ROUTE_NC_READ       10011   /* Enable route neighbhor cache read operations */

#define PRIV_NET_PRIVILEGED_CLIENT_ACCESS       10012   /* Allow client networking access on restricted platforms */
#define PRIV_NET_PRIVILEGED_SERVER_ACCESS       10013   /* Allow server networking access on restricted platforms */

#define PRIV_NET_VALIDATED_RESOLVER             10014   /* Privilege to sign DNS resolver results for validation */

#define PRIV_NET_CUSTOM_PROTOCOL                10015   /* Privilege to use custom protocol APIs */
#define PRIV_NET_PRIVILEGED_NECP_DROP_ALL_BYPASS 10016  /* Privilege to bypass NECP drop-all */
#define PRIV_NET_PRIVILEGED_IPSEC_WAKE_PACKET   10017   /* Privilege to get IPsec wake packet */

/*
 * IPv4 and IPv6 privileges.
 */
#define PRIV_NETINET_RESERVEDPORT       11000   /* Bind low port number. */
#define PRIV_NETINET_TCP_KA_OFFLOAD     11001   /* Can set TCP keep alive offload option */


/*
 * VFS privileges
 */
#define PRIV_VFS_OPEN_BY_ID             14000   /* Allow calling openbyid_np() */
#define PRIV_VFS_MOVE_DATA_EXTENTS      14001   /* Allow F_MOVEDATAEXTENTS fcntl */
#define PRIV_VFS_SNAPSHOT               14002   /* Allow create/rename/delete of snapshots */
#define PRIV_VFS_SNAPSHOT_REVERT        14003   /* Allow reverting filesystem to a previous snapshot */
#define PRIV_VFS_DATALESS_RESOLVER      14004   /* Allow registration as dataless file resolver */
#define PRIV_VFS_DATALESS_MANIPULATION  14005   /* Allow process to inspect dataless directories / manipulate dataless objects */

#define PRIV_APFS_EMBED_DRIVER          14100   /* Allow embedding an EFI driver into the APFS container */
#define PRIV_APFS_FUSION_DEBUG      14101   /* Allow getting internal statistics and controlling the APFS Fusion container */
#define PRIV_APFS_FUSION_ALLOW_PIN_FASTPROMOTE  14102   /* Allow changing pinned/fastPromote inode flags in APFS Fusion container */
// #define PRIV_APFS_UNUSED              14103
#define PRIV_APFS_SET_FREE_SPACE_CHANGE_THRESHOLD       14104   /* Allow setting the free space change notification threshold */
#define PRIV_APFS_SET_FIRMLINK       14105   /* Allow setting the SF_FIRM_LINK bsd flag */

#ifdef KERNEL
/*
 * Privilege check interface.  No flags are currently defined for the API.
 */
#include <sys/cdefs.h>
#include <sys/kauth.h>

/*
 * flags for priv_check_cred
 */
#define PRIVCHECK_DEFAULT_UNPRIVILEGED_FLAG (1) /* Don't grant root privilege by default */

__BEGIN_DECLS
int     priv_check_cred(kauth_cred_t cred, int priv, int flags);
__END_DECLS
#endif

#endif /* !_SYS_PRIV_H_ */
