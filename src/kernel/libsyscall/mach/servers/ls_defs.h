/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
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
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */

/*
 * Definitions for the logstat module.
 */


#ifndef _LS_DEFS_
#define _LS_DEFS_

#include <sys/types.h>

/*
 * Definition for a log record.
 */
typedef struct  {
	long    code;
	long    thread;
	long    a1;
	long    a2;
	long    a3;
	long    a4;
	long    a5;
	long    a6;
} log_rec_t;

typedef log_rec_t       *log_ptr_t;

/*
 * Statistics record.
 */
typedef struct {
	int     datagram_pkts_sent;
	int     datagram_pkts_rcvd;
	int     srr_requests_sent;
	int     srr_bcasts_sent;
	int     srr_requests_rcvd;
	int     srr_bcasts_rcvd;
	int     srr_replies_sent;
	int     srr_replies_rcvd;
	int     srr_retries_sent;
	int     srr_retries_rcvd;
	int     srr_cfailures_sent;
	int     srr_cfailures_rcvd;
	int     deltat_dpkts_sent;
	int     deltat_acks_rcvd;
	int     deltat_dpkts_rcvd;
	int     deltat_acks_sent;
	int     deltat_oldpkts_rcvd;
	int     deltat_oospkts_rcvd;
	int     deltat_retries_sent;
	int     deltat_retries_rcvd;
	int     deltat_cfailures_sent;
	int     deltat_cfailures_rcvd;
	int     deltat_aborts_sent;
	int     deltat_aborts_rcvd;
	int     vmtp_requests_sent;
	int     vmtp_requests_rcvd;
	int     vmtp_replies_sent;
	int     vmtp_replies_rcvd;
	int     ipc_in_messages;
	int     ipc_out_messages;
	int     ipc_unblocks_sent;
	int     ipc_unblocks_rcvd;
	int     pc_requests_sent;
	int     pc_requests_rcvd;
	int     pc_replies_rcvd;
	int     pc_startups_rcvd;
	int     nn_requests_sent;
	int     nn_requests_rcvd;
	int     nn_replies_rcvd;
	int     po_ro_hints_sent;
	int     po_ro_hints_rcvd;
	int     po_token_requests_sent;
	int     po_token_requests_rcvd;
	int     po_token_replies_rcvd;
	int     po_xfer_requests_sent;
	int     po_xfer_requests_rcvd;
	int     po_xfer_replies_rcvd;
	int     po_deaths_sent;
	int     po_deaths_rcvd;
	int     ps_requests_sent;
	int     ps_requests_rcvd;
	int     ps_replies_rcvd;
	int     ps_auth_requests_sent;
	int     ps_auth_requests_rcvd;
	int     ps_auth_replies_rcvd;
	int     mallocs_or_vm_allocates;
	int     mem_allocs;
	int     mem_deallocs;
	int     mem_allocobjs;
	int     mem_deallocobjs;
	int     pkts_encrypted;
	int     pkts_decrypted;
	int     vmtp_segs_encrypted;
	int     vmtp_segs_decrypted;
	int     tcp_requests_sent;
	int     tcp_replies_sent;
	int     tcp_requests_rcvd;
	int     tcp_replies_rcvd;
	int     tcp_send;
	int     tcp_recv;
	int     tcp_connect;
	int     tcp_accept;
	int     tcp_close;
} stat_t;

typedef stat_t  *stat_ptr_t;


/*
 * Debugging flags record.
 */
typedef struct {
	int     print_level;
	int     ipc_in;
	int     ipc_out;
	int     tracing;
	int     vmtp;
	int     netname;
	int     deltat;
	int     tcp;
	int     mem;
} debug_t;

typedef debug_t *debug_ptr_t;


/*
 * Parameters record.
 */
typedef struct {
	int     srr_max_tries;
	int     srr_retry_sec;
	int     srr_retry_usec;
	int     deltat_max_tries;
	int     deltat_retry_sec;
	int     deltat_retry_usec;
	int     deltat_msg_life;
	int     pc_checkup_interval;
	int     crypt_algorithm;
	int     transport_default;
	int     conf_network;
	int     conf_netport;
	int     timer_quantum;
	int     tcp_conn_steady;
	int     tcp_conn_opening;
	int     tcp_conn_max;
	int     compat;
	int     syslog;
	int     old_nmmonitor;
} param_t;

typedef param_t *param_ptr_t;


/*
 * Port statistics record.
 */
typedef struct {
	u_int   port_id;
	u_int   alive;
	u_int   nport_id_high;
	u_int   nport_id_low;
	u_int   nport_receiver;
	u_int   nport_owner;
	u_int   messages_sent;
	u_int   messages_rcvd;
	u_int   send_rights_sent;
	u_int   send_rights_rcvd_sender;
	u_int   send_rights_rcvd_recown;
	u_int   rcv_rights_xferd;
	u_int   own_rights_xferd;
	u_int   all_rights_xferd;
	u_int   tokens_sent;
	u_int   tokens_requested;
	u_int   xfer_hints_sent;
	u_int   xfer_hints_rcvd;
} port_stat_t, *port_stat_ptr_t;

extern port_stat_ptr_t  port_stat_cur;
extern port_stat_ptr_t  port_stat_end;
extern struct mutex     port_stat_lock;


/*
 * Types for the mem_list operation.
 *
 * XXX These must be faked, because we cannot include mem.h here
 * (mutual includes).
 */
typedef char                    *mem_class_ptr_t;
typedef char                    *mem_nam_ptr_t;
typedef int                     *mem_bucket_ptr_t;


/*
 * Definitions for print_level.
 */
#define LS_PRINT_NEVER          5
#define LS_PRINT_LOG            3
#define LS_PRINT_ALWAYS         0

#endif  /* _LS_DEFS_ */
