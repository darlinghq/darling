/*
 * Copyright (c) 2000-2007 Apple Inc. All rights reserved.
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
/* Copyright (c) 1998 Apple Computer, Inc. All rights reserved */

#ifndef _SYS_EV_H_
#define _SYS_EV_H_


#include <sys/appleapiopts.h>

#include <sys/queue.h>
#include <sys/cdefs.h>

struct eventreq {
  int      er_type;
#define EV_FD 1    // file descriptor
  int      er_handle;
  void    *er_data;
  int      er_rcnt;
  int      er_wcnt;
  int      er_ecnt;
  int      er_eventbits;
#define EV_RE  1
#define EV_WR  2
#define EV_EX  4
#define EV_RM  8
#define EV_MASK 0xf
};

typedef struct eventreq *er_t;

#define EV_RBYTES 0x100
#define EV_WBYTES 0x200
#define EV_RWBYTES (EV_RBYTES|EV_WBYTES)
#define EV_RCLOSED 0x400
#define EV_RCONN   0x800
#define EV_WCLOSED 0x1000
#define EV_WCONN   0x2000
#define EV_OOB     0x4000
#define EV_FIN     0x8000
#define EV_RESET   0x10000
#define EV_TIMEOUT 0x20000
#define EV_DMASK   0xffffff00


#ifndef KERNEL

__BEGIN_DECLS
int	modwatch(er_t, int);
int	watchevent(er_t, int);
int	waitevent(er_t, struct timeval *);
__END_DECLS

#endif


#ifdef BSD_KERNEL_PRIVATE



struct eventreq32 {
  int      er_type;
  int      er_handle;
  uint32_t er_data;
  int      er_rcnt;
  int      er_wcnt;
  int      er_ecnt;
  int      er_eventbits;
};

struct eventreq64 {
  int      er_type;
  int      er_handle;
  user_addr_t er_data;
  int      er_rcnt;
  int      er_wcnt;
  int      er_ecnt;
  int      er_eventbits;
};

struct eventqelt {
  TAILQ_ENTRY(eventqelt)  ee_slist;
  TAILQ_ENTRY(eventqelt)  ee_plist;
  struct eventreq64  ee_req;
  struct proc *    ee_proc;
  u_int            ee_flags;
#define EV_QUEUED	0x01
  u_int            ee_eventmask;
};

int waitevent_close(struct proc *p, struct fileproc *);
#endif /* BSD_KERNEL_PRIVATE */

#endif /* _SYS_EV_H_ */
