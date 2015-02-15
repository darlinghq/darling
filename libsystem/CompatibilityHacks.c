/*
 * Copyright (c) 2010-2011 Apple Inc. All rights reserved.
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
 * See: <rdar://problem/8289209>, <rdar://problem/8351271>, <rdar://problem/8359348>
 */

#if defined(__i386__) || defined(__x86_64__)

#define SYM(sym) \
  __asm__(".globl R8289209$_" #sym "; R8289209$_" #sym ": jmp _" #sym)

/****************/

SYM(close);
SYM(fork);
SYM(fsync);
SYM(getattrlist);
SYM(getrlimit);
SYM(getxattr);
SYM(open);
SYM(pthread_attr_destroy);
SYM(pthread_attr_init);
SYM(pthread_attr_setdetachstate);
SYM(pthread_create);
SYM(pthread_mutex_lock);
SYM(pthread_mutex_unlock);
SYM(pthread_self);
SYM(ptrace);
SYM(read);
SYM(setattrlist);
SYM(setrlimit);
SYM(sigaction);
SYM(stat);
SYM(sysctl);
SYM(time);
SYM(unlink);
SYM(write);

#endif /* defined(__i386__) || defined(__x86_64__) */

