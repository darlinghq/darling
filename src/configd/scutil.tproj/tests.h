/*
 * Copyright (c) 2000, 2001, 2004, 2007, 2011, 2012, 2014 Apple Inc. All rights reserved.
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
 * Modification History
 *
 * July 9, 2001			Allan Nathanson <ajn@apple.com>
 * - added "-r" option for checking network reachability
 * - added "-w" option to check/wait for the presence of a
 *   dynamic store key.
 *
 * November 9, 2000		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */

#ifndef _TESTS_H
#define _TESTS_H

#include <sys/cdefs.h>

__BEGIN_DECLS

void	do_checkReachability		(int argc, char **argv);
void	do_watchReachability		(int argc, char **argv);
void	do_renew			(char *interface);
void	do_showDNSConfiguration		(int argc, char **argv);
void	do_watchDNSConfiguration	(int argc, char **argv);
void	do_showProxyConfiguration	(int argc, char **argv);
void	do_snapshot			(int argc, char **argv);
void	do_wait				(char *waitKey, int timeout);
void	do_showNWI			(int argc, char **argv);
void	do_watchNWI			(int argc, char **argv);

__END_DECLS

#endif /* !_TESTS_H */
