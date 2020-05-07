/*
 * Copyright (c) 2006, 2008, 2010 Apple Inc. All rights reserved.
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

#ifndef _GSSD_MACH_TYPES_H_
#define _GSSD_MACH_TYPES_H_

#define MAX_DISPLAY_STR 128
#define MAX_PRINC_STR 1024

typedef enum gssd_mechtype {
	GSSD_NO_MECH = -1,
	GSSD_KRB5_MECH = 0,
	GSSD_SPNEGO_MECH,
	GSSD_NTLM_MECH,
	GSSD_IAKERB_MECH
} gssd_mechtype;

typedef enum gssd_nametype {
	GSSD_STRING_NAME = 0,
	GSSD_EXPORT,
	GSSD_ANONYMOUS,
	GSSD_HOSTBASED,
	GSSD_USER,
	GSSD_MACHINE_UID,
	GSSD_STRING_UID,
	GSSD_KRB5_PRINCIPAL,
	GSSD_KRB5_REFERRAL,
	GSSD_NTLM_PRINCIPAL,
	GSSD_NTLM_BLOB,
	GSSD_UUID
} gssd_nametype;

typedef char *gssd_string;
typedef char *gssd_dstring;
typedef uint8_t *gssd_byte_buffer;
typedef uint32_t *gssd_gid_list;
typedef uint64_t gssd_ctx;
typedef uint64_t gssd_cred;
typedef int32_t *gssd_etype_list;

/* The following need to correspond to GSS_C_*_FLAG in gssapi.h */
#define GSSD_DELEG_FLAG         1
#define GSSD_MUTUAL_FLAG        2
#define GSSD_REPLAY_FLAG        4
#define GSSD_SEQUENCE_FLAG      8
#define GSSD_CONF_FLAG          16
#define GSSD_INTEG_FLAG         32
#define GSSD_ANON_FLAG          64
#define GSSD_PROT_FLAG          128
#define GSSD_TRANS_FLAG         256
#define GSSD_DELEG_POLICY_FLAG  32768

#define GSSD_NO_DEFAULT         1  // Only use the supplied principal, do not fallback to the default.
#define GSSD_NO_CANON           2  // Don't canononicalize host names
#define GSSD_HOME_ACCESS_OK     4  // OK to access home directory
#define GSSD_GUEST_ONLY         8  // NTLM Server is forcing guest access
#define GSSD_RESTART            16 // Destroy the supplied context and start over
#define GSSD_NFS_1DES           64 // Only get single DES session keys
#define GSSD_WIN2K_HACK         128 // Hack for Win2K
#define GSSD_LUCID_CONTEXT      256 // Export Lucid context

#endif /* _GSSD_MACH_TYPES_H_ */
