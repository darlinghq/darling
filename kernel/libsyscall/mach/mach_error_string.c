/*
 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
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
/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */

#include <limits.h>

#include <mach/boolean.h>
#include <mach/error.h>
#include <mach/mach_error.h>

#include "errorlib.h"
#include "externs.h"

static void do_compat(mach_error_t *);

static void
do_compat(mach_error_t *org_err)
{
	mach_error_t		err = *org_err;

	/* 
	 * map old error numbers to 
	 * to new error sys & subsystem 
	 */

	if ((-200 < err) && (err <= -100))
		err = -(err + 100) | IPC_SEND_MOD;
	else if ((-300 < err) && (err <= -200))
		err = -(err + 200) | IPC_RCV_MOD;
	else if ((-400 < err) && (err <= -300))
		err = -(err + 300) | MACH_IPC_MIG_MOD;
	else if ((1000 <= err) && (err < 1100))
		err = (err - 1000) | SERV_NETNAME_MOD;
	else if ((1600 <= err) && (err < 1700))
		err = (err - 1600) | SERV_ENV_MOD;
	else if ((27600 <= err) && (err < 27700))
		err = (err - 27600) | SERV_EXECD_MOD;

	*org_err = err;
}

static int
err_sparse_mapit(int old, const struct error_sparse_map *map_table, int mapcnt)
{
	boolean_t found = FALSE;
	int ret = 0, i;

	for (i = 0; i < mapcnt; i++) {
		struct error_sparse_map entry = map_table[i];

		if (entry.start <= old && old <= entry.end) {
			ret += old - entry.start;
			found = TRUE;
			break;
		}
		ret += entry.end - entry.start + 1;
	}

	return (found)? ret : INT_MAX;
}

char *
mach_error_type(mach_error_t err)
{
	const struct error_system *sys_p;
	int sub, system;

	do_compat( &err );

	system = err_get_system(err);
	sys_p = &_mach_errors[system];
	sub = err_get_sub(err);

	if (system <= err_max_system && sys_p->map_table)
		sub = err_sparse_mapit(sub, sys_p->map_table, sys_p->map_count);

	if (system > err_max_system || sub >= sys_p->max_sub)
	    return((char *)"(?/?)");
	return (char *) (sys_p->subsystem[sub].subsys_name);
}

boolean_t mach_error_full_diag = FALSE;

__private_extern__ char *
mach_error_string_int(mach_error_t err, boolean_t *diag)
{
	const struct error_system *sys_p;
	const struct error_subsystem *sub_p;
	int sub, system, code;

	do_compat( &err );

	system = err_get_system(err);
	sys_p = &_mach_errors[system];
	sub = err_get_sub(err);
	code = err_get_code(err);

	*diag = TRUE;

	if (system > err_max_system)
	    return((char *)"(?/?) unknown error system");
	else if (sys_p->map_table)
		sub = err_sparse_mapit(sub, sys_p->map_table, sys_p->map_count);

	if (sub >= sys_p->max_sub)
	    return((char *)sys_p->bad_sub);

	sub_p = &sys_p->subsystem[sub];
	if (sub_p->map_table)
		code = err_sparse_mapit(code, sub_p->map_table, sub_p->map_count);
	if (code >= sub_p->max_code)
	    return ((char *)NO_SUCH_ERROR);

	*diag = mach_error_full_diag;
	return( (char *)sub_p->codes[code] );
}

char *
mach_error_string(mach_error_t err)
{
	boolean_t diag;

	return mach_error_string_int( err, &diag );
}

/* vim: set ts=4: */
