/*
 * Copyright (c) 2000-2003 Apple Computer, Inc. All rights reserved.
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
 * Copyright 1996 1995 by Open Software Foundation, Inc. 1997 1996 1995 1994 1993 1992 1991  
 *              All Rights Reserved 
 *  
 * Permission to use, copy, modify, and distribute this software and 
 * its documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation. 
 *  
 * OSF DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE. 
 *  
 * IN NO EVENT SHALL OSF BE LIABLE FOR ANY SPECIAL, INDIRECT, OR 
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT, 
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
 * 
 */
/*
 * MkLinux
 */

/*
 * This program will generate the stuff necessary to "publish" the POSIX
 * header <pthread.h> in a machine dependent fashion.
 */

#include <pthread_internals.h>
#include <stdio.h>

int
main(void)
{
	printf("#ifndef _PTHREAD_IMPL_H_\n");
        printf("#define _PTHREAD_IMPL_H_\n");
        printf("/*\n");
	printf(" * Internal implementation details\n");
	printf(" */\n");
	printf("\n");
	printf("#define __PTHREAD_SIZE__           %zd\n", sizeof(struct _pthread)-sizeof(long));
	printf("#define __PTHREAD_ATTR_SIZE__      %zd\n", sizeof(pthread_attr_t)-sizeof(long));
	printf("#define __PTHREAD_MUTEXATTR_SIZE__ %zd\n", sizeof(pthread_mutexattr_t)-sizeof(long));
	printf("#define __PTHREAD_MUTEX_SIZE__     %zd\n", sizeof(pthread_mutex_t)-sizeof(long));
	printf("#define __PTHREAD_CONDATTR_SIZE__  %zd\n", sizeof(pthread_condattr_t)-sizeof(long));
	printf("#define __PTHREAD_COND_SIZE__      %zd\n", sizeof(pthread_cond_t)-sizeof(long));
	printf("#define __PTHREAD_ONCE_SIZE__      %zd\n", sizeof(pthread_once_t)-sizeof(long));
	printf("#define __PTHREAD_sig_OFFSET__     %zd\n", offsetof(struct _pthread, sig));
	printf("#define __PTHREAD_cleanup_stack_OFFSET__     %zd\n", offsetof(struct _pthread, __cleanup_stack));
	printf("#define __PTHREAD_guardsize_OFFSET__     %zd\n", offsetof(struct _pthread, guardsize));
	printf("#define __PTHREAD_param_OFFSET__     %zd\n", offsetof(struct _pthread, param));
	printf("#define __PTHREAD_mutexes_OFFSET__     %zd\n", offsetof(struct _pthread, mutexes));
	printf("#define __PTHREAD_joiner_OFFSET__     %zd\n", offsetof(struct _pthread, joiner));
	printf("#define __PTHREAD_exit_value_OFFSET__     %zd\n", offsetof(struct _pthread, exit_value));
	printf("#define __PTHREAD_death_OFFSET__     %zd\n", offsetof(struct _pthread, death));
	printf("#define __PTHREAD_kernel_thread_OFFSET__     %zd\n", offsetof(struct _pthread, kernel_thread));
	printf("#define __PTHREAD_fun_OFFSET__     %zd\n", offsetof(struct _pthread, fun));
	printf("#define __PTHREAD_arg_OFFSET__     %zd\n", offsetof(struct _pthread, arg));
	printf("#define __PTHREAD_cancel_state_OFFSET__     %zd\n", offsetof(struct _pthread, cancel_state));
	printf("#define __PTHREAD_err_no_OFFSET__     %zd\n", offsetof(struct _pthread, err_no));
	printf("#define __PTHREAD_tsd_OFFSET__     %zd\n", offsetof(struct _pthread, tsd));
	printf("#define __PTHREAD_stackaddr_OFFSET__     %zd\n", offsetof(struct _pthread, stackaddr));
	printf("#define __PTHREAD_stacksize_OFFSET__     %zd\n", offsetof(struct _pthread, stacksize));
	printf("#define __PTHREAD_reply_port_OFFSET__     %zd\n", offsetof(struct _pthread, reply_port));
	printf("#define __PTHREAD_cthread_self_OFFSET__     %zd\n", offsetof(struct _pthread, cthread_self));
	printf("#define __PTHREAD_freeStackOnExit_OFFSET__     %zd\n", offsetof(struct _pthread, freeStackOnExit));
	printf("#define __PTHREAD_plist_OFFSET__     %zd\n", offsetof(struct _pthread, plist));
	printf("/*\n");
	printf(" * [Internal] data structure signatures\n");
	printf(" */\n");
	printf("#define _PTHREAD_MUTEX_SIG_init		0x%08X\n", _PTHREAD_MUTEX_SIG_init);
	printf("#define _PTHREAD_COND_SIG_init		0x%08X\n", _PTHREAD_COND_SIG_init);
	printf("#define _PTHREAD_ONCE_SIG_init		0x%08X\n", _PTHREAD_ONCE_SIG_init);
	printf("/*\n");
	printf(" * POSIX scheduling policies \n");
	printf(" */\n");
        printf("#define SCHED_OTHER                %d\n", SCHED_OTHER);
        printf("#define SCHED_FIFO                 %d\n", SCHED_FIFO);
        printf("#define SCHED_RR                   %d\n", SCHED_RR);
	printf("\n");
	printf("#define __SCHED_PARAM_SIZE__       %ld\n", (long) sizeof(struct sched_param)-sizeof(int));
        printf("\n");
        printf("#endif _PTHREAD_IMPL_H_\n");

	exit(0);
} 
