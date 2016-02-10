/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License, Version 1.0 only
 * (the "License").  You may not use this file except in compliance
 * with the License.
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
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#ifndef _SYS_SDT_H
#define	_SYS_SDT_H

/*
 * This is a wrapper header that wraps the mach visible sdt.h header so that
 * the header file ends up visible where software expects it to be.  We also
 * do the C/C++ symbol wrapping here, since Mach headers are technically C
 * interfaces.
 *
 * Note:  The process of adding USDT probes to code is slightly different
 * than documented in the "Solaris Dynamic Tracing Guide".
 * The DTRACE_PROBE*() macros are not supported on Mac OS X -- instead see
 * "BUILDING CODE CONTAINING USDT PROBES" in the dtrace(1) manpage
 *
 */
#include <sys/cdefs.h>
__BEGIN_DECLS
#include <mach/sdt.h>
__END_DECLS

#endif	/* _SYS_SDT_H */
