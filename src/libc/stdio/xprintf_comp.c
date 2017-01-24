/*
 * Copyright (c) 2012 Apple Inc. All rights reserved.
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

#include <xlocale_private.h>
#include <printf.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "xprintf_domain.h"
#include "xprintf_private.h"

void
free_printf_comp(printf_comp_t pc)
{
    if(!pc) return;
    XL_RELEASE(pc->loc);
#ifdef XPRINTF_PERF
    arrayfree(pc->pa);
    free(pc->pa);
    arrayfree(pc->aa);
    free(pc->aa);
    arrayfree(pc->ua);
    free(pc->ua);
#else /* !XPRINTF_PERF */
    free(pc->pi);
    free(pc->argt);
    free(pc->args);
#endif /* !XPRINTF_PERF */
    pthread_mutex_destroy(&pc->mutex);
    free(pc);
}

printf_comp_t
new_printf_comp(printf_domain_t restrict domain, locale_t loc, const char * restrict fmt)
{
    int ret, saverrno;
    printf_comp_t restrict pc;

    if(!domain) {
	errno = EINVAL;
	return NULL;
    }
    pc = MALLOC(sizeof(*pc) + strlen(fmt) + 1);
    if(!pc) return NULL;
    bzero(pc, sizeof(*pc));
    pc->mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    pc->fmt = (const char *)(pc + 1);
    strcpy((char *)pc->fmt, fmt);
    DEFAULT_CURRENT_LOCALE(loc);
    XL_RETAIN(loc);
    pc->loc = loc;
    xprintf_domain_init();
    pthread_rwlock_rdlock(&domain->rwlock);
    ret = __printf_comp(pc, domain);
    saverrno = errno;
    pthread_rwlock_unlock(&domain->rwlock);
    if(ret < 0) {
	XL_RELEASE(loc);
	pthread_mutex_destroy(&pc->mutex);
	free(pc);
	errno = saverrno;
	return NULL;
    }
    return pc;
}
