/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */

/*
 * Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#ifndef _SYS_SYSTRACE_H
#define _SYS_SYSTRACE_H

#include <sys/dtrace.h>

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct systrace_sysent {
	dtrace_id_t     stsy_entry;
	dtrace_id_t     stsy_return;
	int32_t         (*stsy_underlying)(struct proc *, void *, int *);
	int32_t         stsy_return_type;
} systrace_sysent_t;

extern systrace_sysent_t *systrace_sysent;
extern systrace_sysent_t *systrace_sysent32;

extern void (*systrace_probe)(dtrace_id_t, uint64_t, uint64_t,
    uint64_t, uint64_t, uint64_t);
extern void systrace_stub(dtrace_id_t, uint64_t, uint64_t,
    uint64_t, uint64_t, uint64_t);

extern int32_t dtrace_systrace_syscall(struct proc *, void *, int *);

extern void dtrace_systrace_syscall_return(unsigned short, int, int *);

#ifdef  __cplusplus
}
#endif

#endif  /* _SYS_SYSTRACE_H */
