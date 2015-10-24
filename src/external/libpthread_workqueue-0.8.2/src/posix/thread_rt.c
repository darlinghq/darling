/*
 * Copyright (c) 2011, Joakim Johansson <jocke@tbricks.com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "platform.h"
#include "private.h"

#if defined(__sun)

#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/procset.h>
#include <sys/priocntl.h>
#include <sys/rtpriocntl.h>
#include <sys/tspriocntl.h>
#include <sys/iapriocntl.h>
#include <sys/fsspriocntl.h>
#include <sys/fxpriocntl.h>

// Set the RT priority - it is inveresed from the queue priorities, higher is better
// We give '0' to low prio queues and the highest possible for the 'high' prio queue (currently 2)

void ptwq_set_current_thread_priority(int priority)
{
    long retval = 0;

    dbg_printf("reconfiguring thread for priority level=%u", priority);

    switch (priority)
    {
        case WORKQ_LOW_PRIOQUEUE:
            retval = priocntl(P_LWPID, P_MYID, PC_SETXPARMS, "TS", 0); // run low prio queues as time sharing
            break;
        case WORKQ_DEFAULT_PRIOQUEUE:
            retval = priocntl(P_LWPID, P_MYID, PC_SETXPARMS, "RT", RT_KY_PRI, WORKQ_NUM_PRIOQUEUE - priority - 1, 0);
            break;
        case WORKQ_HIGH_PRIOQUEUE:
            retval = priocntl(P_LWPID, P_MYID, PC_SETXPARMS, "RT", RT_KY_PRI, WORKQ_NUM_PRIOQUEUE - priority - 1, 0);
            break;
        default:
            dbg_printf("Unknown priority level = %u", priority);
            break;
    }
    

    if (retval != 0)
        dbg_perror("priocntl()");

    return;
}

#else

void ptwq_set_current_thread_priority(int priority  __attribute__ ((unused)))
{    
    return;
}

#endif
