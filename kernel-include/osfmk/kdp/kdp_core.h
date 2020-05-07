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

/* HISTORY
 * 8 Aug. 2003 - Created (Derek Kumar)
 */

/* Various protocol definitions
 * for the core transfer protocol, which is a variant of TFTP
 */
#ifndef __KDP_CORE_H
#define __KDP_CORE_H

#include <kern/thread.h>
#include <kdp/kdp_protocol.h>
#include <string.h>

/*
 * Packet types.
 */
#define KDP_RRQ   1                     /* read request */
#define KDP_WRQ   2                     /* write request */
#define KDP_DATA  3                     /* data packet */
#define KDP_ACK   4                     /* acknowledgement */
#define KDP_ERROR 5                     /* error code */
#define KDP_SEEK  6                     /* Seek to specified offset */
#define KDP_EOF   7                     /* signal end of file */
#define KDP_FLUSH 8                     /* flush outstanding data */
#define KDP_FEATURE_MASK_STRING         "features"

enum    {KDP_FEATURE_LARGE_CRASHDUMPS = 1, KDP_FEATURE_LARGE_PKT_SIZE = 2};
extern  uint32_t        kdp_feature_large_crashdumps, kdp_feature_large_pkt_size;

struct  corehdr {
	short   th_opcode;              /* packet type */
	union {
		unsigned int    tu_block;       /* block # */
		unsigned int    tu_code;        /* error code */
		char    tu_rpl[1];      /* request packet payload */
	} th_u;
	char    th_data[0];             /* data or error string */
}__attribute__((packed));

#define th_block        th_u.tu_block
#define th_code         th_u.tu_code
#define th_stuff        th_u.tu_rpl
#define th_msg          th_data

/*
 * Error codes.
 */
#define EUNDEF          0               /* not defined */
#define ENOTFOUND       1               /* file not found */
#define EACCESS         2               /* access violation */
#define ENOSPACE        3               /* disk full or allocation exceeded */
#define EBADOP          4               /* illegal TFTP operation */
#define EBADID          5               /* unknown transfer ID */
#define EEXISTS         6               /* file already exists */
#define ENOUSER         7               /* no such user */

#define CORE_REMOTE_PORT 1069 /* hardwired, we can't really query the services file */

#if CONFIG_EMBEDDED
/*
 * xnu shared memory hardware debugger support
 *
 * A hardware debugger can connect, read the consistent debug
 * header to determine the physical location of the handshake
 * structure and communicate using commands in the structure as
 * defined below.
 *
 * Currently used for sending compressed coredumps to
 * astris.
 */
struct xnu_hw_shmem_dbg_command_info {
	volatile uint32_t xhsdci_status;
	uint32_t xhsdci_seq_no;
	volatile uint64_t xhsdci_buf_phys_addr;
	volatile uint32_t xhsdci_buf_data_length;
	/* end of version 0 structure */
	uint64_t xhsdci_coredump_total_size_uncomp;
	uint64_t xhsdci_coredump_total_size_sent_uncomp;
	uint32_t xhsdci_page_size;
} __attribute__((packed));

#define CUR_XNU_HWSDCI_STRUCT_VERS 1

#define XHSDCI_STATUS_NONE              0 /* default status */
#define XHSDCI_STATUS_KERNEL_BUSY       1 /* kernel is busy with other procedure */
#define XHSDCI_STATUS_KERNEL_READY      2 /* kernel ready to begin command */
#define XHSDCI_COREDUMP_BEGIN           3 /* indicates hardware debugger is ready to begin consuming coredump info */
#define XHSDCI_COREDUMP_BUF_READY       4 /* indicates the kernel has populated the buffer */
#define XHSDCI_COREDUMP_BUF_EMPTY       5 /* indicates hardware debugger is done consuming the current data */
#define XHSDCI_COREDUMP_STATUS_DONE     6 /* indicates last compressed data is in buffer */
#define XHSDCI_COREDUMP_ERROR           7 /* indicates an error was encountered */
#define XHSDCI_COREDUMP_REMOTE_DONE     8 /* indicates that hardware debugger is done */

void panic_spin_shmcon(void);

#endif /* CONFIG_EMBEDDED */

void kdp_panic_dump(void);
void begin_panic_transfer(void);
void abort_panic_transfer(void);
void kdp_set_dump_info(const uint32_t flags, const char *file, const char *destip,
    const char *routerip, const uint32_t port);
void kdp_get_dump_info(kdp_dumpinfo_reply_t *rp);

enum kern_dump_type {
	KERN_DUMP_DISK, /* local, on device core dump */
	KERN_DUMP_NET, /* kdp network core dump */
#if CONFIG_EMBEDDED
	KERN_DUMP_HW_SHMEM_DBG, /* coordinated hardware shared memory debugger core dump */
#endif
	KERN_DUMP_STACKSHOT_DISK, /* local, stackshot on device coredump */
};

int kern_dump(enum kern_dump_type kd_variant);

boolean_t dumped_kernel_core(void);

struct corehdr *create_panic_header(unsigned int request, const char *corename, unsigned length, unsigned block);

int     kdp_send_crashdump_pkt(unsigned int request, char *corename,
    uint64_t length, void *panic_data);

int     kdp_send_crashdump_data(unsigned int request, char *corename,
    uint64_t length, void * txstart);

void kern_collectth_state_size(uint64_t * tstate_count, uint64_t * tstate_size);

void kern_collectth_state(thread_t thread, void *buffer, uint64_t size, void **iter);

boolean_t kdp_has_polled_corefile(void);
kern_return_t kdp_polled_corefile_error(void);

void kdp_core_init(void);

extern boolean_t kdp_corezip_disabled;

#define KDP_CRASHDUMP_POLL_COUNT (2500)

#if PRIVATE
kern_return_t kdp_core_output(void *kdp_core_out_vars, uint64_t length, void * data);

kern_return_t kdp_reset_output_vars(void *kdp_core_out_vars, uint64_t totalbytes);

int kern_dump_record_file(void *kdp_core_out_vars, const char *filename, uint64_t file_offset, uint64_t *out_file_length);

int kern_dump_seek_to_next_file(void *kdp_core_out_varss, uint64_t next_file_offset);

extern boolean_t bootloader_valid_page(ppnum_t ppn);

#endif /* PRIVATE */

#endif /* __KDP_CORE_H */
