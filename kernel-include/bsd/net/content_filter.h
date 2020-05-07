/*
 * Copyright (c) 2013-2019 Apple Inc. All rights reserved.
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

#ifndef __CONTENT_FILTER_H__
#define __CONTENT_FILTER_H__

#include <sys/param.h>
#include <sys/types.h>
#include <sys/_types/_timeval64.h>
#include <sys/socket.h>
#include <sys/syslog.h>
#include <netinet/in.h>
#include <stdint.h>
#include <corecrypto/ccsha2.h>

#ifdef BSD_KERNEL_PRIVATE
#include <sys/mbuf.h>
#include <sys/socketvar.h>
#endif /* BSD_KERNEL_PRIVATE */

__BEGIN_DECLS

#ifdef PRIVATE

/*
 * Kernel control name for an instance of a Content Filter
 * Use CTLIOCGINFO to find out the corresponding kernel control id
 * to be set in the sc_id field of sockaddr_ctl for connect(2)
 * Note: the sc_unit is ephemeral
 */
#define CONTENT_FILTER_CONTROL_NAME "com.apple.content-filter"

/*
 * Opaque socket identifier
 */
typedef uint64_t cfil_sock_id_t;

#define CFIL_SOCK_ID_NONE UINT64_MAX


/*
 * CFIL_OPT_NECP_CONTROL_UNIT
 * To set or get the NECP filter control unit for the kernel control socket
 * The option level is SYSPROTO_CONTROL
 */
#define CFIL_OPT_NECP_CONTROL_UNIT      1       /* uint32_t */

/*
 * CFIL_OPT_GET_SOCKET_INFO
 * To get information about a given socket that is being filtered.
 */
#define CFIL_OPT_GET_SOCKET_INFO        2       /* uint32_t */

/*
 * struct cfil_opt_sock_info
 *
 * Contains information about a socket that is being filtered.
 */
struct cfil_opt_sock_info {
	cfil_sock_id_t  cfs_sock_id;
	int                             cfs_sock_family;        /* e.g. PF_INET */
	int                             cfs_sock_type;          /* e.g. SOCK_STREAM */
	int                             cfs_sock_protocol;      /* e.g. IPPROTO_TCP */
	union sockaddr_in_4_6   cfs_local;
	union sockaddr_in_4_6   cfs_remote;
	pid_t                   cfs_pid;
	pid_t                   cfs_e_pid;
	uuid_t                  cfs_uuid;
	uuid_t                  cfs_e_uuid;
};

/*
 * How many filter may be active simultaneously
 */
#if !TARGET_OS_OSX && !defined(XNU_TARGET_OS_OSX)
#define CFIL_MAX_FILTER_COUNT   2
#else
#define CFIL_MAX_FILTER_COUNT   8
#endif


/*
 * Crypto Support
 */
#define CFIL_CRYPTO 1
#define CFIL_CRYPTO_SIGNATURE_SIZE 32
#define CFIL_CRYPTO_DATA_EVENT 1

typedef uint8_t cfil_crypto_key[CCSHA256_OUTPUT_SIZE];
typedef uint8_t cfil_crypto_signature[CFIL_CRYPTO_SIGNATURE_SIZE];

typedef struct cfil_crypto_state {
	const struct ccdigest_info *digest_info;
	cfil_crypto_key key;
} *cfil_crypto_state_t;

typedef struct cfil_crypto_data {
	uuid_t flow_id;
	u_int64_t sock_id;
	u_int32_t direction;
	union sockaddr_in_4_6 remote;
	union sockaddr_in_4_6 local;
	u_int32_t socketProtocol;
	pid_t pid;
	pid_t effective_pid;
	uuid_t uuid;
	uuid_t effective_uuid;
	u_int64_t byte_count_in;
	u_int64_t byte_count_out;
} *cfil_crypto_data_t;

/*
 * Types of messages
 *
 * Event messages flow from kernel to user space while action
 * messages flow in the reverse direction.
 * A message in entirely represented by a packet sent or received
 * on a Content Filter kernel control socket.
 */
#define CFM_TYPE_EVENT 1        /* message from kernel */
#define CFM_TYPE_ACTION 2       /* message to kernel */

/*
 * Operations associated with events from kernel
 */
#define CFM_OP_SOCKET_ATTACHED 1        /* a socket has been attached */
#define CFM_OP_SOCKET_CLOSED 2          /* a socket is being closed */
#define CFM_OP_DATA_OUT 3               /* data being sent */
#define CFM_OP_DATA_IN 4                /* data being received */
#define CFM_OP_DISCONNECT_OUT 5         /* no more outgoing data */
#define CFM_OP_DISCONNECT_IN 6          /* no more incoming data */

/*
 * Operations associated with action from filter to kernel
 */
#define CFM_OP_DATA_UPDATE 16           /* update pass or peek offsets */
#define CFM_OP_DROP 17                  /* shutdown socket, no more data */
#define CFM_OP_BLESS_CLIENT 18          /* mark a client flow as already filtered, passes a uuid */
#define CFM_OP_SET_CRYPTO_KEY 19        /* assign client crypto key for message signing */

/*
 * struct cfil_msg_hdr
 *
 * Header common to all messages
 */
struct cfil_msg_hdr {
	uint32_t        cfm_len;        /* total length */
	uint32_t        cfm_version;
	uint32_t        cfm_type;
	uint32_t        cfm_op;
	cfil_sock_id_t  cfm_sock_id;
};

#define CFM_VERSION_CURRENT 1

/*
 * Connection Direction
 */
#define CFS_CONNECTION_DIR_IN  0
#define CFS_CONNECTION_DIR_OUT 1

#define CFS_AUDIT_TOKEN            1

/*
 * struct cfil_msg_sock_attached
 *
 * Information about a new socket being attached to the content filter
 *
 * Action: No reply is expected as this does not block the creation of the
 * TCP/IP but timely action must be taken to avoid user noticeable delays.
 *
 * Valid Types: CFM_TYPE_EVENT
 *
 * Valid Op: CFM_OP_SOCKET_ATTACHED
 */
struct cfil_msg_sock_attached {
	struct cfil_msg_hdr     cfs_msghdr;
	int                     cfs_sock_family;        /* e.g. PF_INET */
	int                     cfs_sock_type;          /* e.g. SOCK_STREAM */
	int                     cfs_sock_protocol;      /* e.g. IPPROTO_TCP */
	int                     cfs_unused;             /* padding */
	pid_t                   cfs_pid;
	pid_t                   cfs_e_pid;
	uuid_t                  cfs_uuid;
	uuid_t                  cfs_e_uuid;
	union sockaddr_in_4_6   cfs_src;
	union sockaddr_in_4_6   cfs_dst;
	int                     cfs_conn_dir;
	unsigned int            cfs_audit_token[8];             /* Must match audit_token_t */
	cfil_crypto_signature   cfs_signature;
	uint32_t                cfs_signature_length;
};

/*
 * struct cfil_msg_data_event
 *
 * Event for the content fiter to act on a span of data
 * A data span is described by a pair of offsets over the cumulative
 * number of bytes sent or received on the socket.
 *
 * Action: The event must be acted upon but the filter may buffer
 * data spans until it has enough content to make a decision.
 * The action must be timely to avoid user noticeable delays.
 *
 * Valid Type: CFM_TYPE_EVENT
 *
 * Valid Ops: CFM_OP_DATA_OUT, CFM_OP_DATA_IN
 */
struct cfil_msg_data_event {
	struct cfil_msg_hdr     cfd_msghdr;
	union sockaddr_in_4_6   cfc_src;
	union sockaddr_in_4_6   cfc_dst;
	uint64_t                cfd_start_offset;
	uint64_t                cfd_end_offset;
	cfil_crypto_signature   cfd_signature;
	uint32_t                cfd_signature_length;
	/* Actual content data immediatly follows */
};

#define CFI_MAX_TIME_LOG_ENTRY 6
/*
 * struct cfil_msg_sock_closed
 *
 * Information about a socket being closed to the content filter
 *
 * Action: No reply is expected as this does not block the closing of the
 * TCP/IP.
 *
 * Valid Types: CFM_TYPE_EVENT
 *
 * Valid Op: CFM_OP_SOCKET_CLOSED
 */
struct cfil_msg_sock_closed {
	struct cfil_msg_hdr     cfc_msghdr;
	struct timeval64        cfc_first_event;
	uint32_t                cfc_op_list_ctr;
	uint32_t                cfc_op_time[CFI_MAX_TIME_LOG_ENTRY];    /* time interval in microseconds since first event */
	unsigned char           cfc_op_list[CFI_MAX_TIME_LOG_ENTRY];
	uint64_t                cfc_byte_inbound_count;
	uint64_t                cfc_byte_outbound_count;
	cfil_crypto_signature   cfc_signature;
	uint32_t                cfc_signature_length;
} __attribute__((aligned(8)));

/*
 * struct cfil_msg_action
 *
 * Valid Type: CFM_TYPE_ACTION
 *
 * Valid Ops: CFM_OP_DATA_UPDATE, CFM_OP_DROP
 *
 * For CFM_OP_DATA_UPDATE:
 *
 * cfa_in_pass_offset and cfa_out_pass_offset indicates how much data is
 * allowed to pass. A zero value does not modify the corresponding pass offset.
 *
 * cfa_in_peek_offset and cfa_out_peek_offset lets the filter specify how much
 * data it needs to make a decision: the kernel will deliver data up to that
 * offset (if less than cfa_pass_offset it is ignored). Use CFM_MAX_OFFSET
 * if you don't value the corresponding peek offset to be updated.
 */
struct cfil_msg_action {
	struct cfil_msg_hdr     cfa_msghdr;
	uint64_t                cfa_in_pass_offset;
	uint64_t                cfa_in_peek_offset;
	uint64_t                cfa_out_pass_offset;
	uint64_t                cfa_out_peek_offset;
};

/*
 * struct cfil_msg_bless_client
 *
 * Marks a client UUID as already filtered at a higher level.
 *
 * Valid Type: CFM_TYPE_ACTION
 *
 * Valid Ops: CFM_OP_BLESS_CLIENT
 */
struct cfil_msg_bless_client {
	struct cfil_msg_hdr     cfb_msghdr;
	uuid_t cfb_client_uuid;
};

/*
 * struct cfil_msg_set_crypto_key
 *
 * Filter assigning client crypto key to CFIL for message signing
 *
 * Valid Type: CFM_TYPE_ACTION
 *
 * Valid Ops: CFM_OP_SET_CRYPTO_KEY
 */
struct cfil_msg_set_crypto_key {
	struct cfil_msg_hdr     cfb_msghdr;
	cfil_crypto_key         crypto_key;
};

#define CFM_MAX_OFFSET  UINT64_MAX

/*
 * Statistics retrieved via sysctl(3)
 */
struct cfil_filter_stat {
	uint32_t        cfs_len;
	uint32_t        cfs_filter_id;
	uint32_t        cfs_flags;
	uint32_t        cfs_sock_count;
	uint32_t        cfs_necp_control_unit;
};

struct cfil_entry_stat {
	uint32_t                ces_len;
	uint32_t                ces_filter_id;
	uint32_t                ces_flags;
	uint32_t                ces_necp_control_unit;
	struct timeval64        ces_last_event;
	struct timeval64        ces_last_action;
	struct cfe_buf_stat {
		uint64_t        cbs_pending_first;
		uint64_t        cbs_pending_last;
		uint64_t        cbs_ctl_first;
		uint64_t        cbs_ctl_last;
		uint64_t        cbs_pass_offset;
		uint64_t        cbs_peek_offset;
		uint64_t        cbs_peeked;
	} ces_snd, ces_rcv;
};

struct cfil_sock_stat {
	uint32_t        cfs_len;
	int             cfs_sock_family;
	int             cfs_sock_type;
	int             cfs_sock_protocol;
	cfil_sock_id_t  cfs_sock_id;
	uint64_t        cfs_flags;
	pid_t           cfs_pid;
	pid_t           cfs_e_pid;
	uuid_t          cfs_uuid;
	uuid_t          cfs_e_uuid;
	struct cfi_buf_stat {
		uint64_t        cbs_pending_first;
		uint64_t        cbs_pending_last;
		uint64_t        cbs_pass_offset;
		uint64_t        cbs_inject_q_len;
	} cfs_snd, cfs_rcv;
	struct cfil_entry_stat  ces_entries[CFIL_MAX_FILTER_COUNT];
};

/*
 * Global statistics
 */
struct cfil_stats {
	int32_t cfs_ctl_connect_ok;
	int32_t cfs_ctl_connect_fail;
	int32_t cfs_ctl_disconnect_ok;
	int32_t cfs_ctl_disconnect_fail;
	int32_t cfs_ctl_send_ok;
	int32_t cfs_ctl_send_bad;
	int32_t cfs_ctl_rcvd_ok;
	int32_t cfs_ctl_rcvd_bad;
	int32_t cfs_ctl_rcvd_flow_lift;
	int32_t cfs_ctl_action_data_update;
	int32_t cfs_ctl_action_drop;
	int32_t cfs_ctl_action_bad_op;
	int32_t cfs_ctl_action_bad_len;

	int32_t cfs_sock_id_not_found;

	int32_t cfs_cfi_alloc_ok;
	int32_t cfs_cfi_alloc_fail;

	int32_t cfs_sock_userspace_only;
	int32_t cfs_sock_attach_in_vain;
	int32_t cfs_sock_attach_already;
	int32_t cfs_sock_attach_no_mem;
	int32_t cfs_sock_attach_failed;
	int32_t cfs_sock_attached;
	int32_t cfs_sock_detached;

	int32_t cfs_attach_event_ok;
	int32_t cfs_attach_event_flow_control;
	int32_t cfs_attach_event_fail;

	int32_t cfs_closed_event_ok;
	int32_t cfs_closed_event_flow_control;
	int32_t cfs_closed_event_fail;

	int32_t cfs_data_event_ok;
	int32_t cfs_data_event_flow_control;
	int32_t cfs_data_event_fail;

	int32_t cfs_disconnect_in_event_ok;
	int32_t cfs_disconnect_out_event_ok;
	int32_t cfs_disconnect_event_flow_control;
	int32_t cfs_disconnect_event_fail;

	int32_t cfs_ctl_q_not_started;

	int32_t cfs_close_wait;
	int32_t cfs_close_wait_timeout;

	int32_t cfs_flush_in_drop;
	int32_t cfs_flush_out_drop;
	int32_t cfs_flush_in_close;
	int32_t cfs_flush_out_close;
	int32_t cfs_flush_in_free;
	int32_t cfs_flush_out_free;

	int32_t cfs_inject_q_nomem;
	int32_t cfs_inject_q_nobufs;
	int32_t cfs_inject_q_detached;
	int32_t cfs_inject_q_in_fail;
	int32_t cfs_inject_q_out_fail;

	int32_t cfs_inject_q_in_retry;
	int32_t cfs_inject_q_out_retry;

	int32_t cfs_data_in_control;
	int32_t cfs_data_in_oob;
	int32_t cfs_data_out_control;
	int32_t cfs_data_out_oob;

	int64_t cfs_ctl_q_in_enqueued __attribute__((aligned(8)));
	int64_t cfs_ctl_q_out_enqueued __attribute__((aligned(8)));
	int64_t cfs_ctl_q_in_peeked __attribute__((aligned(8)));
	int64_t cfs_ctl_q_out_peeked __attribute__((aligned(8)));

	int64_t cfs_pending_q_in_enqueued __attribute__((aligned(8)));
	int64_t cfs_pending_q_out_enqueued __attribute__((aligned(8)));

	int64_t cfs_inject_q_in_enqueued __attribute__((aligned(8)));
	int64_t cfs_inject_q_out_enqueued __attribute__((aligned(8)));
	int64_t cfs_inject_q_in_passed __attribute__((aligned(8)));
	int64_t cfs_inject_q_out_passed __attribute__((aligned(8)));
};
#endif /* PRIVATE */

#ifdef BSD_KERNEL_PRIVATE

#define M_SKIPCFIL      M_PROTO5

extern int cfil_log_level;

#define CFIL_LOG(level, fmt, ...) \
do { \
	if (cfil_log_level >= level) \
	        printf("%s:%d " fmt "\n",\
	                __FUNCTION__, __LINE__, ##__VA_ARGS__); \
} while (0)


extern void cfil_init(void);

extern boolean_t cfil_filter_present(void);
extern boolean_t cfil_sock_connected_pending_verdict(struct socket *so);
extern errno_t cfil_sock_attach(struct socket *so,
    struct sockaddr *local, struct sockaddr *remote, int dir);
extern errno_t cfil_sock_detach(struct socket *so);

extern int cfil_sock_data_out(struct socket *so, struct sockaddr  *to,
    struct mbuf *data, struct mbuf *control,
    uint32_t flags);
extern int cfil_sock_data_in(struct socket *so, struct sockaddr *from,
    struct mbuf *data, struct mbuf *control,
    uint32_t flags);

extern int cfil_sock_shutdown(struct socket *so, int *how);
extern void cfil_sock_is_closed(struct socket *so);
extern void cfil_sock_notify_shutdown(struct socket *so, int how);
extern void cfil_sock_close_wait(struct socket *so);

extern boolean_t cfil_sock_data_pending(struct sockbuf *sb);
extern int cfil_sock_data_space(struct sockbuf *sb);
extern void cfil_sock_buf_update(struct sockbuf *sb);

extern cfil_sock_id_t cfil_sock_id_from_socket(struct socket *so);

extern struct m_tag *cfil_udp_get_socket_state(struct mbuf *m, uint32_t *state_change_cnt,
    short *options, struct sockaddr **faddr);
#endif /* BSD_KERNEL_PRIVATE */

__END_DECLS

#endif /* __CONTENT_FILTER_H__ */
