/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
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

#ifndef _KDP_PROTOCOL_H_
#define _KDP_PROTOCOL_H_

/*
 * Definition of remote debugger protocol.
 */


#ifdef MACH_KERNEL_PRIVATE
#include        <mach/vm_prot.h>
#include    <mach/boolean.h>
#include        <stdint.h>
#endif

#ifdef KDP_PROXY_PACK_SUPPORT
#pragma pack(1)
#define KDP_PACKED
#else
#define KDP_PACKED __attribute__((packed))
#endif

/*
 * Retransmit parameters
 */
#if     DDEBUG_DEBUG || DEBUG_DEBUG
#define KDP_REXMIT_SECS         20      /* rexmit if no ack in 3 secs */
#else   /* DDEBUG_DEBUG || DEBUG_DEBUG */
#define KDP_REXMIT_SECS         3       /* rexmit if no ack in 3 secs */
#endif  /* DDEBUG_DEBUG || DEBUG_DEBUG */
#define KDP_REXMIT_TRIES        8       /* xmit 8 times, then give up */


/*
 * (NMI) Attention Max Wait Time
 * Remote will resume unless KDP requests is received within this
 * many seconds after an attention (nmi) packet is sent.
 */
#define KDP_MAX_ATTN_WAIT       30      /* wait max of 30 seconds */

/*
 * Well-known UDP port, debugger side.
 * FIXME: This is what the 68K guys use, but beats me how they chose it...
 */
#define KDP_REMOTE_PORT         41139   /* pick one and register it */

/*
 * UDP ports, KDB side. 5 port numbers are reserved for each port (request
 * and exception). This allows multiple KDBs to run on one host.
 */
#define UDP_HOST_COMM_BASE      41140
#define UDP_HOST_EXCEP_BASE     41145
#define NUM_UDP_HOST_PORTS      5

/*
 * Requests
 */
typedef enum {
	/* connection oriented requests */
	KDP_CONNECT, KDP_DISCONNECT,

	/* obtaining client info */
	KDP_HOSTINFO, KDP_VERSION, KDP_MAXBYTES,

	/* memory access */
	KDP_READMEM, KDP_WRITEMEM,

	/* register access */
	KDP_READREGS, KDP_WRITEREGS,

	/* executable image info */
	KDP_LOAD, KDP_IMAGEPATH,

	/* execution control */
	KDP_SUSPEND, KDP_RESUMECPUS,

	/* exception and termination notification, NOT true requests */
	KDP_EXCEPTION, KDP_TERMINATION,

	/* breakpoint control */
	KDP_BREAKPOINT_SET, KDP_BREAKPOINT_REMOVE,

	/* vm regions */
	KDP_REGIONS,

	/* reattach to a connected host */
	KDP_REATTACH,

	/* remote reboot request */
	KDP_HOSTREBOOT,

	/* memory access (64-bit wide addresses). Version 11 protocol */
	KDP_READMEM64, KDP_WRITEMEM64,

	/* breakpoint control (64-bit wide addresses). Version 11 protocol */
	KDP_BREAKPOINT64_SET, KDP_BREAKPOINT64_REMOVE,

	/* kernel version string, like "xnu-1234.5~6". Version 11 protocol */
	KDP_KERNELVERSION,

	/* physical memory access (64-bit wide addresses). Version 12 protocol */
	KDP_READPHYSMEM64, KDP_WRITEPHYSMEM64,

	/* ioport access (8-, 16-, and 32-bit) */
	KDP_READIOPORT, KDP_WRITEIOPORT,

	/* msr access (64-bit) */
	KDP_READMSR64, KDP_WRITEMSR64,

	/* get/dump panic/corefile info */
	KDP_DUMPINFO,

	/* keep this last */
	KDP_INVALID_REQUEST
} kdp_req_t;

typedef enum {
	KDP_DUMPINFO_GETINFO    = 0x00000000,
	KDP_DUMPINFO_SETINFO    = 0x00000001,
	KDP_DUMPINFO_CORE       = 0x00000102,
	KDP_DUMPINFO_PANICLOG   = 0x00000103,
	KDP_DUMPINFO_SYSTEMLOG  = 0x00000104,
	KDP_DUMPINFO_DISABLE    = 0x00000105,
	KDP_DUMPINFO_MASK       = 0x00000FFF,
	KDP_DUMPINFO_DUMP       = 0x00000100,

	KDP_DUMPINFO_REBOOT     = 0x10000000,
	KDP_DUMPINFO_NORESUME   = 0x20000000,
	KDP_DUMPINFO_RESUME     = 0x00000000, /* default behaviour */
	KDP_DUMPINFO_NOINTR     = 0x40000000, /* don't interrupt */
	KDP_DUMPINFO_INTR       = 0x00000000, /* default behaviour */
} kdp_dumpinfo_t;

/*
 * Common KDP packet header
 * NOTE: kgmacros has a non-symboled version of kdp_hdr_t so that some basic information.
 *       can be gathered from a kernel without any symbols. changes to this structure
 *       need to be reflected in kgmacros as well.
 */
typedef struct {
	kdp_req_t       request:7;      /* kdp_req_t, request type */
	unsigned        is_reply:1;     /* 0 => request, 1 => reply */
	unsigned        seq:8;          /* sequence number within session */
	unsigned        len:16;         /* length of entire pkt including hdr */
	unsigned        key;            /* session key */
} KDP_PACKED kdp_hdr_t;

/*
 * KDP errors
 */
typedef enum {
	KDPERR_NO_ERROR = 0,
	KDPERR_ALREADY_CONNECTED,
	KDPERR_BAD_NBYTES,
	KDPERR_BADFLAVOR,               /* bad flavor in w/r regs */
	KDPERR_BAD_ACCESS,              /* memory reference failure */

	KDPERR_MAX_BREAKPOINTS = 100,
	KDPERR_BREAKPOINT_NOT_FOUND = 101,
	KDPERR_BREAKPOINT_ALREADY_SET = 102
} kdp_error_t;

#if defined(__x86_64__)
#define KDPERR_ACCESS(_req, _ret)  \
	(((_req) == (uint32_t)(_ret)) ? KDPERR_NO_ERROR : KDPERR_BAD_ACCESS)
#else
#define KDPERR_ACCESS(req, cnt)  (KDPERR_NO_ERROR)
#endif /* x86_64 */


/*
 * KDP requests and reply packet formats
 */

/*
 * KDP_CONNECT
 */
typedef struct {                        /* KDP_CONNECT request */
	kdp_hdr_t       hdr;
	uint16_t        req_reply_port; /* udp port which to send replies */
	uint16_t        exc_note_port;  /* udp port which to send exc notes */
	char            greeting[0];    /* "greetings", nul-terminated */
} KDP_PACKED kdp_connect_req_t;

typedef struct {                        /* KDP_CONNECT reply */
	kdp_hdr_t       hdr;
	kdp_error_t     error;
} KDP_PACKED kdp_connect_reply_t;

/*
 * KDP_DISCONNECT
 */
typedef struct {                        /* KDP_DISCONNECT request */
	kdp_hdr_t       hdr;
} KDP_PACKED kdp_disconnect_req_t;

typedef struct {                        /* KDP_DISCONNECT reply */
	kdp_hdr_t       hdr;
} KDP_PACKED kdp_disconnect_reply_t;

/*
 * KDP_REATTACH
 */
typedef struct {
	kdp_hdr_t hdr;
	uint16_t req_reply_port; /* udp port which to send replies */
} KDP_PACKED kdp_reattach_req_t;

/*
 * KDP_HOSTINFO
 */
typedef struct {                        /* KDP_HOSTINFO request */
	kdp_hdr_t       hdr;
} KDP_PACKED kdp_hostinfo_req_t;

typedef struct {
	uint32_t                cpus_mask;      /* bit is 1 if cpu present */
	uint32_t                cpu_type;
	uint32_t                cpu_subtype;
} KDP_PACKED kdp_hostinfo_t;

typedef struct {                        /* KDP_HOSTINFO reply */
	kdp_hdr_t       hdr;
	kdp_hostinfo_t  hostinfo;
} KDP_PACKED kdp_hostinfo_reply_t;

/*
 * KDP_VERSION
 */
typedef struct {                        /* KDP_VERSION request */
	kdp_hdr_t       hdr;
} KDP_PACKED kdp_version_req_t;

#define KDP_FEATURE_BP  0x1     /* local breakpoint support */

typedef struct {                        /* KDP_VERSION reply */
	kdp_hdr_t       hdr;
	uint32_t        version;
	uint32_t        feature;
	uint32_t        pad0;
	uint32_t        pad1;
} KDP_PACKED kdp_version_reply_t;

#define VM_PROT_VOLATILE        ((vm_prot_t) 0x08)      /* not cacheable */
#define VM_PROT_SPARSE          ((vm_prot_t) 0x10)      /* sparse addr space */

/*
 * KDP_REGIONS
 */
typedef struct {                        /* KDP_REGIONS request */
	kdp_hdr_t       hdr;
} KDP_PACKED kdp_regions_req_t;

typedef struct {
	uint32_t        address;
	uint32_t        nbytes;
	uint32_t        protection;     /* vm_prot_t */
} KDP_PACKED kdp_region_t;

typedef struct {                        /* KDP_REGIONS reply */
	kdp_hdr_t       hdr;
	uint32_t        nregions;
	kdp_region_t    regions[0];
} KDP_PACKED kdp_regions_reply_t;

/*
 * KDP_MAXBYTES
 */
typedef struct {                        /* KDP_MAXBYTES request */
	kdp_hdr_t       hdr;
} KDP_PACKED kdp_maxbytes_req_t;

typedef struct {                        /* KDP_MAXBYTES reply */
	kdp_hdr_t       hdr;
	uint32_t        max_bytes;
} KDP_PACKED kdp_maxbytes_reply_t;

/*
 * KDP_READMEM
 */
typedef struct {                        /* KDP_READMEM request */
	kdp_hdr_t       hdr;
	uint32_t        address;
	uint32_t        nbytes;
} KDP_PACKED kdp_readmem_req_t;

typedef struct {                        /* KDP_READMEM reply */
	kdp_hdr_t       hdr;
	kdp_error_t     error;
	char            data[0];
} KDP_PACKED kdp_readmem_reply_t;

/*
 * KDP_READMEM64
 */
typedef struct {                        /* KDP_READMEM64 request */
	kdp_hdr_t       hdr;
	uint64_t        address;
	uint32_t        nbytes;
} KDP_PACKED kdp_readmem64_req_t;

typedef struct {                        /* KDP_READMEM64 reply */
	kdp_hdr_t       hdr;
	kdp_error_t     error;
	char            data[0];
} KDP_PACKED kdp_readmem64_reply_t;

/*
 * KDP_READPHYSMEM64
 */
typedef struct {                        /* KDP_READPHYSMEM64 request */
	kdp_hdr_t       hdr;
	uint64_t        address;
	uint32_t        nbytes;
	uint16_t        lcpu;
} KDP_PACKED kdp_readphysmem64_req_t;

typedef struct {                        /* KDP_READPHYSMEM64 reply */
	kdp_hdr_t       hdr;
	kdp_error_t     error;
	char            data[0];
} KDP_PACKED kdp_readphysmem64_reply_t;

/*
 * KDP_WRITEMEM
 */
typedef struct {                        /* KDP_WRITEMEM request */
	kdp_hdr_t       hdr;
	uint32_t        address;
	uint32_t        nbytes;
	char            data[0];
} KDP_PACKED kdp_writemem_req_t;

typedef struct {                        /* KDP_WRITEMEM reply */
	kdp_hdr_t       hdr;
	kdp_error_t     error;
} KDP_PACKED kdp_writemem_reply_t;

/*
 * KDP_WRITEMEM64
 */
typedef struct {                        /* KDP_WRITEMEM64 request */
	kdp_hdr_t       hdr;
	uint64_t        address;
	uint32_t        nbytes;
	char            data[0];
} KDP_PACKED kdp_writemem64_req_t;

typedef struct {                        /* KDP_WRITEMEM64 reply */
	kdp_hdr_t       hdr;
	kdp_error_t     error;
} KDP_PACKED kdp_writemem64_reply_t;

/*
 * KDP_WRITEPHYSMEM64
 */
typedef struct {                        /* KDP_WRITEPHYSMEM64 request */
	kdp_hdr_t       hdr;
	uint64_t        address;
	uint32_t        nbytes;
	uint16_t        lcpu;
	char            data[0];
} KDP_PACKED kdp_writephysmem64_req_t;

typedef struct {                        /* KDP_WRITEPHYSMEM64 reply */
	kdp_hdr_t       hdr;
	kdp_error_t     error;
} KDP_PACKED kdp_writephysmem64_reply_t;

/*
 * KDP_WRITEIOPORT
 */
typedef struct {                        /* KDP_WRITEIOPORT request */
	kdp_hdr_t       hdr;
	uint16_t        lcpu;
	uint16_t        address;
	uint16_t        nbytes;
	char            data[0];
} KDP_PACKED kdp_writeioport_req_t;

typedef struct {                        /* KDP_WRITEIOPORT reply */
	kdp_hdr_t       hdr;
	kdp_error_t     error;
} KDP_PACKED kdp_writeioport_reply_t;

/*
 * KDP_READIOPORT
 */
typedef struct {                        /* KDP_READIOPORT request */
	kdp_hdr_t       hdr;
	uint16_t        lcpu;
	uint16_t        address;
	uint16_t        nbytes;
} KDP_PACKED kdp_readioport_req_t;

typedef struct {                        /* KDP_READIOPORT reply */
	kdp_hdr_t       hdr;
	kdp_error_t     error;
	char            data[0];
} KDP_PACKED kdp_readioport_reply_t;


/*
 * KDP_WRITEMSR64
 */
typedef struct {                        /* KDP_WRITEMSR64 request */
	kdp_hdr_t       hdr;
	uint32_t        address;
	uint16_t        lcpu;
	char            data[0];
} KDP_PACKED kdp_writemsr64_req_t;

typedef struct {                        /* KDP_WRITEMSR64 reply */
	kdp_hdr_t       hdr;
	kdp_error_t     error;
} KDP_PACKED kdp_writemsr64_reply_t;

/*
 * KDP_READMSR64
 */
typedef struct {                        /* KDP_READMSR64 request */
	kdp_hdr_t       hdr;
	uint32_t        address;
	uint16_t        lcpu;
} KDP_PACKED kdp_readmsr64_req_t;

typedef struct {                        /* KDP_READMSR64 reply */
	kdp_hdr_t       hdr;
	kdp_error_t     error;
	char            data[0];
} KDP_PACKED kdp_readmsr64_reply_t;


/*
 * KDP_READREGS
 */
typedef struct {                        /* KDP_READREGS request */
	kdp_hdr_t       hdr;
	uint32_t        cpu;
	uint32_t        flavor;
} KDP_PACKED kdp_readregs_req_t;

typedef struct {                        /* KDP_READREGS reply */
	kdp_hdr_t       hdr;
	kdp_error_t     error;          /* could be KDPERR_BADFLAVOR */
	char            data[0];
} KDP_PACKED kdp_readregs_reply_t;

/*
 * KDP_WRITEREGS
 */
typedef struct {                        /* KDP_WRITEREGS request */
	kdp_hdr_t       hdr;
	uint32_t        cpu;
	uint32_t        flavor;
	char            data[0];
} KDP_PACKED kdp_writeregs_req_t;

typedef struct {                        /* KDP_WRITEREGS reply */
	kdp_hdr_t       hdr;
	kdp_error_t     error;
} KDP_PACKED kdp_writeregs_reply_t;

/*
 * KDP_LOAD
 */
typedef struct {                        /* KDP_LOAD request */
	kdp_hdr_t       hdr;
	char            file_args[0];
} KDP_PACKED kdp_load_req_t;

typedef struct {                        /* KDP_LOAD reply */
	kdp_hdr_t       hdr;
	kdp_error_t     error;
} KDP_PACKED kdp_load_reply_t;

/*
 * KDP_IMAGEPATH
 */
typedef struct {                        /* KDP_IMAGEPATH request */
	kdp_hdr_t       hdr;
} KDP_PACKED kdp_imagepath_req_t;

typedef struct {                        /* KDP_IMAGEPATH reply */
	kdp_hdr_t       hdr;
	char            path[0];
} KDP_PACKED kdp_imagepath_reply_t;

/*
 * KDP_SUSPEND
 */
typedef struct {                        /* KDP_SUSPEND request */
	kdp_hdr_t       hdr;
} KDP_PACKED kdp_suspend_req_t;

typedef struct {                        /* KDP_SUSPEND reply */
	kdp_hdr_t       hdr;
} KDP_PACKED kdp_suspend_reply_t;

/*
 * KDP_RESUMECPUS
 */
typedef struct {                        /* KDP_RESUMECPUS request */
	kdp_hdr_t       hdr;
	uint32_t        cpu_mask;
} KDP_PACKED kdp_resumecpus_req_t;

typedef struct {                        /* KDP_RESUMECPUS reply */
	kdp_hdr_t       hdr;
} KDP_PACKED kdp_resumecpus_reply_t;

/*
 * KDP_BREAKPOINT_SET and KDP_BREAKPOINT_REMOVE
 */

typedef struct {
	kdp_hdr_t hdr;
	uint32_t      address;
} KDP_PACKED kdp_breakpoint_req_t;

typedef struct {
	kdp_hdr_t hdr;
	kdp_error_t error;
} KDP_PACKED kdp_breakpoint_reply_t;

/*
 * KDP_BREAKPOINT64_SET and KDP_BREAKPOINT64_REMOVE
 */

typedef struct {
	kdp_hdr_t hdr;
	uint64_t        address;
} KDP_PACKED kdp_breakpoint64_req_t;

typedef struct {
	kdp_hdr_t hdr;
	kdp_error_t error;
} KDP_PACKED kdp_breakpoint64_reply_t;

/*
 * Exception notifications
 * (Exception notifications are not requests, and in fact travel from
 * the remote debugger to the gdb agent KDB.)
 */
typedef struct {                        /* exc. info for one cpu */
	uint32_t        cpu;
	/*
	 * Following info is defined as
	 * per <mach/exception.h>
	 */
	uint32_t        exception;
	uint32_t        code;
	uint32_t        subcode;
} KDP_PACKED kdp_exc_info_t;

typedef struct {                        /* KDP_EXCEPTION notification */
	kdp_hdr_t       hdr;
	uint32_t        n_exc_info;
	kdp_exc_info_t  exc_info[0];
} KDP_PACKED kdp_exception_t;

typedef struct {                        /* KDP_EXCEPTION acknowledgement */
	kdp_hdr_t       hdr;
} KDP_PACKED kdp_exception_ack_t;

/*
 * KDP_KERNELVERSION
 */
typedef struct {                        /* KDP_KERNELVERSION request */
	kdp_hdr_t       hdr;
} KDP_PACKED kdp_kernelversion_req_t;

typedef struct {                        /* KDP_KERNELVERSION reply */
	kdp_hdr_t       hdr;
	char            version[0];
} KDP_PACKED kdp_kernelversion_reply_t;


/*
 * Child termination messages
 */
typedef enum {
	KDP_FAULT = 0,          /* child took fault (internal use) */
	KDP_EXIT,               /* child exited */
	KDP_POWEROFF,           /* child power-off */
	KDP_REBOOT,             /* child reboot */
	KDP_COMMAND_MODE        /* child exit to mon command_mode */
} kdp_termination_code_t;

typedef struct {                        /* KDP_TERMINATION notification */
	kdp_hdr_t               hdr;
	uint32_t                term_code;      /* kdp_termination_code_t */
	uint32_t                exit_code;
} KDP_PACKED kdp_termination_t;

typedef struct {
	kdp_hdr_t       hdr;
} KDP_PACKED kdp_termination_ack_t;

/*
 * KDP_DUMPINFO
 */
typedef struct {                        /* KDP_DUMPINFO request */
	kdp_hdr_t       hdr;
	char            name[50];
	char            destip[16];
	char            routerip[16];
	uint32_t        port;
	kdp_dumpinfo_t  type;
} KDP_PACKED kdp_dumpinfo_req_t;

typedef struct {                        /* KDP_DUMPINFO reply */
	kdp_hdr_t       hdr;
	char            name[50];
	char            destip[16];
	char            routerip[16];
	uint32_t        port;
	kdp_dumpinfo_t  type;
} KDP_PACKED kdp_dumpinfo_reply_t;


typedef union {
	kdp_hdr_t               hdr;
	kdp_connect_req_t       connect_req;
	kdp_connect_reply_t     connect_reply;
	kdp_disconnect_req_t    disconnect_req;
	kdp_disconnect_reply_t  disconnect_reply;
	kdp_hostinfo_req_t      hostinfo_req;
	kdp_hostinfo_reply_t    hostinfo_reply;
	kdp_version_req_t       version_req;
	kdp_version_reply_t     version_reply;
	kdp_maxbytes_req_t      maxbytes_req;
	kdp_maxbytes_reply_t    maxbytes_reply;
	kdp_readmem_req_t       readmem_req;
	kdp_readmem_reply_t     readmem_reply;
	kdp_readmem64_req_t     readmem64_req;
	kdp_readmem64_reply_t   readmem64_reply;
	kdp_readphysmem64_req_t readphysmem64_req;
	kdp_readphysmem64_reply_t       readphysmem64_reply;
	kdp_writemem_req_t      writemem_req;
	kdp_writemem_reply_t    writemem_reply;
	kdp_writemem64_req_t    writemem64_req;
	kdp_writemem64_reply_t  writemem64_reply;
	kdp_writephysmem64_req_t        writephysmem64_req;
	kdp_writephysmem64_reply_t      writephysmem64_reply;
	kdp_readregs_req_t      readregs_req;
	kdp_readregs_reply_t    readregs_reply;
	kdp_writeregs_req_t     writeregs_req;
	kdp_writeregs_reply_t   writeregs_reply;
	kdp_load_req_t          load_req;
	kdp_load_reply_t        load_reply;
	kdp_imagepath_req_t     imagepath_req;
	kdp_imagepath_reply_t   imagepath_reply;
	kdp_suspend_req_t       suspend_req;
	kdp_suspend_reply_t     suspend_reply;
	kdp_resumecpus_req_t    resumecpus_req;
	kdp_resumecpus_reply_t  resumecpus_reply;
	kdp_exception_t         exception;
	kdp_exception_ack_t     exception_ack;
	kdp_termination_t       termination;
	kdp_termination_ack_t   termination_ack;
	kdp_breakpoint_req_t    breakpoint_req;
	kdp_breakpoint_reply_t  breakpoint_reply;
	kdp_breakpoint64_req_t  breakpoint64_req;
	kdp_breakpoint64_reply_t        breakpoint64_reply;
	kdp_reattach_req_t      reattach_req;
	kdp_regions_req_t       regions_req;
	kdp_regions_reply_t     regions_reply;
	kdp_kernelversion_req_t kernelversion_req;
	kdp_kernelversion_reply_t       kernelversion_reply;
	kdp_readioport_req_t    readioport_req;
	kdp_readioport_reply_t  readioport_reply;
	kdp_writeioport_req_t   writeioport_req;
	kdp_writeioport_reply_t writeioport_reply;
	kdp_readmsr64_req_t     readmsr64_req;
	kdp_readmsr64_reply_t   readmsr64_reply;
	kdp_writemsr64_req_t    writemsr64_req;
	kdp_writemsr64_reply_t  writemsr64_reply;
	kdp_dumpinfo_req_t      dumpinfo_req;
	kdp_dumpinfo_reply_t    dumpinfo_reply;
} kdp_pkt_t;

#define MAX_KDP_PKT_SIZE        1200    /* max packet size */
#define MAX_KDP_DATA_SIZE       1024    /* max r/w data per packet */

/*
 * Support relatively small request/responses here.
 * If kgmacros needs to make a larger request, increase
 * this buffer size
 */
#define KDP_MANUAL_PACKET_SIZE 128
struct kdp_manual_pkt {
	unsigned char       data[KDP_MANUAL_PACKET_SIZE];
	unsigned int        len;
	boolean_t           input;
} KDP_PACKED;

#ifdef KDP_PROXY_PACK_SUPPORT
#pragma pack()
#endif

#endif // _KDP_PROTOCOL_H_
