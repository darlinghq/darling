/*
 * Copyright (c) 2018 Apple Inc. All rights reserved.
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


#ifndef _IORPC_H
#define _IORPC_H

#include <stdint.h>

#ifndef PLATFORM_DriverKit

#include <mach/message.h>

#else /* !PLATFORM_DriverKit */

#ifndef _MACH_MESSAGE_H_
#define _MACH_MESSAGE_H_

#define MACH_MSG_TYPE_MOVE_RECEIVE      16      /* Must hold receive right */
#define MACH_MSG_TYPE_MOVE_SEND         17      /* Must hold send right(s) */
#define MACH_MSG_TYPE_MOVE_SEND_ONCE    18      /* Must hold sendonce right */
#define MACH_MSG_TYPE_COPY_SEND         19      /* Must hold send right(s) */
#define MACH_MSG_TYPE_MAKE_SEND         20      /* Must hold receive right */
#define MACH_MSG_TYPE_MAKE_SEND_ONCE    21      /* Must hold receive right */
#define MACH_MSG_TYPE_COPY_RECEIVE      22      /* NOT VALID */
#define MACH_MSG_TYPE_DISPOSE_RECEIVE   24      /* must hold receive right */
#define MACH_MSG_TYPE_DISPOSE_SEND      25      /* must hold send right(s) */
#define MACH_MSG_TYPE_DISPOSE_SEND_ONCE 26      /* must hold sendonce right */

#define MACH_MSG_TYPE_PORT_NONE         0

#define MACH_MSG_PORT_DESCRIPTOR                0
#define MACH_MSG_OOL_DESCRIPTOR                 1

typedef unsigned int mach_msg_copy_options_t;

#define MACH_MSG_PHYSICAL_COPY          0
#define MACH_MSG_VIRTUAL_COPY           1
#define MACH_MSG_ALLOCATE               2

typedef uint32_t natural_t;
typedef int32_t integer_t;

typedef unsigned int mach_msg_type_name_t;
typedef unsigned int mach_msg_descriptor_type_t;

#if KERNEL
typedef void * mach_port_t;
#define MACH_PORT_NULL  NULL
#else /* !KERNEL */
typedef natural_t mach_port_t;
#define MACH_PORT_NULL  0
#endif /* !KERNEL */

typedef natural_t mach_port_name_t;

typedef unsigned int mach_msg_bits_t;
typedef natural_t mach_msg_size_t;
typedef integer_t mach_msg_id_t;

#pragma pack(push, 4)

typedef struct{
	mach_msg_bits_t       msgh_bits;
	mach_msg_size_t       msgh_size;
	mach_port_t           msgh_remote_port;
	mach_port_t           msgh_local_port;
	mach_port_name_t      msgh_voucher_port;
	mach_msg_id_t         msgh_id;
} mach_msg_header_t;

typedef struct{
	mach_msg_size_t msgh_descriptor_count;
} mach_msg_body_t;

typedef struct{
	mach_port_t                   name;
#if !(defined(KERNEL) && defined(__LP64__))
// Pad to 8 bytes everywhere except the K64 kernel where mach_port_t is 8 bytes
	mach_msg_size_t               pad1;
#endif
	unsigned int                  pad2 : 16;
	mach_msg_type_name_t          disposition : 8;
	mach_msg_descriptor_type_t    type : 8;
#if defined(KERNEL)
	uint32_t          pad_end;
#endif
} mach_msg_port_descriptor_t;

typedef struct{
	void *                        address;
#if !defined(__LP64__)
	mach_msg_size_t               size;
#endif
	int                           deallocate: 8;
	mach_msg_copy_options_t       copy: 8;
	unsigned int                  pad1: 8;
	mach_msg_descriptor_type_t    type: 8;
#if defined(__LP64__)
	mach_msg_size_t               size;
#endif
#if defined(KERNEL) && !defined(__LP64__)
	uint32_t          pad_end;
#endif
} mach_msg_ool_descriptor_t;

typedef struct{
	unsigned int                  val[80 / sizeof(int)];
} mach_msg_max_trailer_t;

#pragma pack(pop)

#endif  /* _MACH_MESSAGE_H_ */

#endif /* PLATFORM_DriverKit */

#if KERNEL
class IOUserServer;
#endif /* KERNEL */

typedef uint64_t OSObjectRef;

enum {
	kIORPCVersion190615       = (mach_msg_id_t) 0x4da2b68c,
	kIORPCVersion190615Reply  = (mach_msg_id_t) 0x4da2b68d,

#if DRIVERKIT_PRIVATE
	kIORPCVersion190501       = (mach_msg_id_t) 0xfe316a7a,
	kIORPCVersion190501Reply  = (mach_msg_id_t) 0xfe316a7b,

	kIORPCVersionCurrent      = kIORPCVersion190615,
	kIORPCVersionCurrentReply = kIORPCVersion190615Reply
#endif /* DRIVERKIT_PRIVATE */
};

enum{
	kIORPCMessageRemote     = 0x00000001,
	kIORPCMessageLocalHost  = 0x00000002,
	kIORPCMessageKernel     = 0x00000004,
	kIORPCMessageOneway     = 0x00000008,
	kIORPCMessageObjectRefs = 0x00000010,
	kIORPCMessageOnqueue    = 0x00000020,
	kIORPCMessageError      = 0x00000040,
	kIORPCMessageSimpleReply = 0x00000080,
};

enum{
	kIORPCMessageIDKernel   = (1ULL << 63),
};

struct IORPCMessageMach {
	mach_msg_header_t          msgh;
	mach_msg_body_t            msgh_body;
	mach_msg_port_descriptor_t objects[0];
};
typedef struct IORPCMessageMach IORPCMessageMach;

struct IORPCMessage {
	uint64_t         msgid;
	uint64_t         flags;
	uint64_t         objectRefs;
	OSObjectRef      objects[0];
};
typedef struct IORPCMessage IORPCMessage;

extern "C" IORPCMessage *
IORPCMessageFromMach(IORPCMessageMach * msg, bool reply);

struct IORPCMessageErrorReturnContent {
	IORPCMessage  hdr;
	kern_return_t result;
	uint32_t      pad;
};

#pragma pack(4)
struct IORPCMessageErrorReturn {
	IORPCMessageMach mach;
	IORPCMessageErrorReturnContent content;
};
#pragma pack()


class OSMetaClassBase;
struct IORPC;
typedef kern_return_t (*OSDispatchMethod)(OSMetaClassBase * self, const IORPC rpc);

struct IORPC {
	IORPCMessageMach * message;
	IORPCMessageMach * reply;
	uint32_t           sendSize;
	uint32_t           replySize;
};
typedef struct IORPC IORPC;

enum {
	kOSClassCanRemote   = 0x00000001,
};

struct OSClassDescription {
	uint32_t    descriptionSize;

	char        name[96];
	char        superName[96];

	uint32_t    methodOptionsSize;
	uint32_t    methodOptionsOffset;
	uint32_t    metaMethodOptionsSize;
	uint32_t    metaMethodOptionsOffset;
	uint32_t    queueNamesSize;
	uint32_t    queueNamesOffset;
	uint32_t    methodNamesSize;
	uint32_t    methodNamesOffset;
	uint32_t    metaMethodNamesSize;
	uint32_t    metaMethodNamesOffset;

	uint64_t    flags;

	uint64_t    resv1[8];

	uint64_t    methodOptions[0];
	uint64_t    metaMethodOptions[0];

	char        dispatchNames[0];
	char        methodNames[0];
	char        metaMethodNames[0];
};

#endif /* _IORPC_H */
