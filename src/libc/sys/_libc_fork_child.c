/*
 * Copyright (c) 2010 Apple Inc. All rights reserved.
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
 * _libc_fork_child() is called from Libsystem's libSystem_atfork_child()
 */
#include <TargetConditionals.h>
#include <CrashReporterClient.h>

extern void _arc4_fork_child();
extern void _init_clock_port(void);
extern void __environ_lock_fork_child();

void _libc_fork_child(void); // todo: private_extern?
void
_libc_fork_child(void)
{
	CRSetCrashLogMessage("crashed on child side of fork pre-exec");

	_arc4_fork_child();
	_init_clock_port();
	__environ_lock_fork_child();
}
