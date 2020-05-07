/*
 * Copyright (c) 2000-2004 Apple Computer, Inc. All rights reserved.
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
 * Internal definitions for kdp module
 */

#include <kdp/kdp.h>
#include <kern/machine.h>
#include <kdp/kdp_protocol.h>
#include <mach/vm_types.h>
#include <mach/boolean.h>
#include <mach/mach_types.h>
#include <libsa/types.h>

typedef struct {
	void                        *saved_state;
	thread_t                    kdp_thread;
	int                         kdp_cpu;
	uint32_t                    session_key;
	unsigned int                conn_seq;
	unsigned short              reply_port;
	unsigned short              exception_port;
	boolean_t                   is_conn;
	boolean_t                   is_halted;
	unsigned char               exception_seq;
	boolean_t                   exception_ack_needed;
} kdp_glob_t;

extern kdp_glob_t       kdp;

extern volatile int     kdp_flag;
extern int            noresume_on_disconnect;

extern char kdp_kernelversion_string[256];

#define KDP_READY       0x1
#define KDP_ARP         0x2
#define KDP_BP_DIS      0x4
#define KDP_GETC_ENA    0x8
#define KDP_PANIC_DUMP_ENABLED 0x10
#define PANIC_CORE_ON_NMI 0x20
#define DBG_POST_CORE     0x40
#define PANIC_LOG_DUMP    0x80
#define REBOOT_POST_CORE  0x100
#define SYSTEM_LOG_DUMP   0x200
typedef boolean_t
(*kdp_dispatch_t) (
	kdp_pkt_t *,
	int  *,
	unsigned short *
	);

extern
boolean_t
kdp_packet(
	unsigned char *,
	int *,
	unsigned short *
	);

extern
boolean_t
kdp_remove_all_breakpoints(void);

extern
void
kdp_exception(
	unsigned char *,
	int *,
	unsigned short *,
	unsigned int,
	unsigned int,
	unsigned int
	);

extern
boolean_t
kdp_exception_ack(
	unsigned char *,
	int
	);

extern
void
kdp_panic(
	const char          *msg
	);

extern
void
kdp_machine_reboot(
	void
	);

extern
void
kdp_us_spin(
	int usec
	);

extern
int
kdp_intr_disbl(
	void
	);

extern
void
kdp_intr_enbl(
	int s
	);

extern
kdp_error_t
kdp_machine_read_regs(
	unsigned int cpu,
	unsigned int flavor,
	char *data,
	int *size
	);

extern
kdp_error_t
kdp_machine_write_regs(
	unsigned int cpu,
	unsigned int flavor,
	char *data,
	int *size
	);

extern
void
kdp_machine_hostinfo(
	kdp_hostinfo_t *hostinfo
	);

extern
void
kdp_sync_cache(
	void
	);

/* Return a byte array that can be byte-copied to a memory address
 * to trap into the debugger. Must be 4 bytes or less in the current
 * implementation
 */
#define MAX_BREAKINSN_BYTES 4

void
kdp_machine_get_breakinsn(
	uint8_t *bytes,
	uint32_t *size
	);

extern void
kdp_ml_enter_debugger(
	void
	);

mach_vm_size_t
    kdp_machine_vm_read( mach_vm_address_t, caddr_t, mach_vm_size_t);

mach_vm_size_t
    kdp_machine_vm_write( caddr_t, mach_vm_address_t, mach_vm_size_t);

mach_vm_size_t
    kdp_machine_phys_read(kdp_readphysmem64_req_t * rq, caddr_t /* data */,
    uint16_t /* lcpu */);

mach_vm_size_t
    kdp_machine_phys_write(kdp_writephysmem64_req_t * rq, caddr_t /* data */,
    uint16_t /* lcpu */);

int
    kdp_machine_ioport_read(kdp_readioport_req_t *, caddr_t /* data */, uint16_t /* lcpu */);

int
    kdp_machine_ioport_write(kdp_writeioport_req_t *, caddr_t /* data */, uint16_t /* lcpu */);

int
    kdp_machine_msr64_read(kdp_readmsr64_req_t *, caddr_t /* data */, uint16_t /* lcpu */);

int
    kdp_machine_msr64_write(kdp_writemsr64_req_t *, caddr_t /* data */, uint16_t /* lcpu */);
