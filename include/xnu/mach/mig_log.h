/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
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
 * @OSF_COPYRIGHT@
 */

#ifndef _mig_log_
#define _mig_log_

#include <sys/appleapiopts.h>

#ifdef __APPLE_API_OBSOLETE

typedef enum {
	MACH_MSG_LOG_USER,
	MACH_MSG_LOG_SERVER
} mig_who_t;

typedef enum {
	MACH_MSG_REQUEST_BEING_SENT,
	MACH_MSG_REQUEST_BEING_RCVD,
	MACH_MSG_REPLY_BEING_SENT,
	MACH_MSG_REPLY_BEING_RCVD
} mig_which_event_t;

typedef enum {
	MACH_MSG_ERROR_WHILE_PARSING,
	MACH_MSG_ERROR_UNKNOWN_ID
} mig_which_error_t;

extern void MigEventTracer
#if     defined(__STDC__)
(
	mig_who_t who,	
	mig_which_event_t what,
	mach_msg_id_t msgh_id,
	unsigned int size,
	unsigned int kpd,
	unsigned int retcode,
	unsigned int ports,
	unsigned int oolports,
	unsigned int ool,
	char *file,
	unsigned int line
);
#else  	/* !defined(__STDC__) */
();
#endif  /* !defined(__STDC__) */

extern void MigEventErrors
#if     defined(__STDC__)
(
	mig_who_t who,	
	mig_which_error_t what,
	void *par,
	char *file,
	unsigned int line
);
#else  	/* !defined(__STDC__) */
();
#endif  /* !defined(__STDC__) */

extern int mig_errors;
extern int mig_tracing;

#define LOG_ERRORS      if (mig_errors)  MigEventErrors
#define LOG_TRACE       if (mig_tracing) MigEventTracer

#endif  /* __APPLE_API_OBSOLETE */

#endif  /* _mach_log_ */

