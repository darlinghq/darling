/*
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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

#ifndef	_FSLOG_H_
#define _FSLOG_H_

#include <sys/syslog.h>
#include <sys/mount.h>
#include <machine/limits.h>

#ifdef KERNEL
/* Log file system related error in key-value format identified by Apple 
 * system log (ASL) facility.  The key-value pairs are string pointers 
 * (char *) and are provided as variable arguments list.  A NULL value 
 * indicates end of the list.
 *
 * Keys can not contain '[', ']', space, and newline.  Values can not 
 * contain '[', ']', and newline.  If any key-value contains any of the 
 * reserved characters, the behavior is undefined.  The caller of the 
 * function should escape any occurrences of '[' and ']' by prefixing
 * it with '\'.
 * 
 * The function takes a message ID which can be used to logically group
 * different ASL messages.  Messages in same logical group have same message
 * ID and have information to describe order of the message --- first, 
 * middle, or last.
 *
 * The following message IDs have special meaning - 
 * FSLOG_MSG_FIRST - This message is the first message in its logical
 *	group.  This generates a unique message ID, creates two key-value 
 *	pairs with message ID and order of the message as "First".
 * FSLOG_MSG_LAST - This is really a MASK which should be logically OR'ed 
 *	with message ID to indicate the last message for a logical group.  
 *	This also creates two key-value pairs with message ID and order of 
 *	message as "Last".
 * FSLOG_MSG_SINGLE - This signifies that the message is the only message
 * 	in its logical group.  Therefore no extra key-values are generated 
 *	for this option.
 * For all other values of message IDs, it regards them as intermediate 
 * message and generates two key-value pairs with message ID and order of
 * message as "Middle".
 * 
 * Returns - 
 *	Message ID of the ASL message printed.  The caller should use
 * 	this value to print intermediate messages or end the logical message
 *	group.
 *	For FSLOG_MSG_SINGLE option, it returns FSLOG_MSG_SINGLE. 
 */
unsigned long fslog_err(unsigned long msg_id, ... );

/* Reserved message IDs to determine message order */
#define FSLOG_MSG_SINGLE	ULONG_MAX
#define FSLOG_MSG_FIRST		0x0
#define FSLOG_MSG_LAST		(~(ULONG_MAX >> 1)) 

#ifdef BSD_KERNEL_PRIVATE

/* Log information about runtime file system corruption detected */
void fslog_fs_corrupt(struct mount *mnt);

/* Log information about IO error detected */
void fslog_io_error(const buf_t bp);

#endif /* BSD_KERNEL_PRIVATE */

#ifdef XNU_KERNEL_PRIVATE

/* Log information about external modification of a target process */
void fslog_extmod_msgtracer(proc_t caller, proc_t target);

#endif /* XNU_KERNEL_PRIVATE */

#endif /* KERNEL */

/* Keys used by FSLog */
#define FSLOG_KEY_FACILITY	"Facility"	/* Facility generating messages */
#define FSLOG_KEY_LEVEL		"Level"		/* Priority level */
#define FSLOG_KEY_MSG_ID	"FSLogMsgID"	/* Message ID */
#define FSLOG_KEY_MSG_ORDER	"FSLogMsgOrder"	/* Message Order */
#define FSLOG_KEY_READ_UID	"ReadUID"	/* Allow read access to this UID only */

/* Values for message order (FSLOG_KEY_MSG_ORDER) */
#define FSLOG_VAL_ORDER_FIRST	"First"		
#define FSLOG_VAL_ORDER_MIDDLE	"Middle"
#define FSLOG_VAL_ORDER_LAST 	"Last"

/* Values used by FSLog */
#define FSLOG_VAL_FACILITY	"com.apple.system.fs" /* Facility generating messages */
#define FSLOG_VAL_LEVEL		LOG_ERR		/* Priority level */
#define FSLOG_VAL_READ_UID	0		/* Allow read access to root only */

/* Keys for IO/FS logging using FSLog */
#define	FSLOG_KEY_ERR_TYPE	"ErrType"	/* Type of problem (IO, FS Corruption) */
#define	FSLOG_KEY_ERRNO		"ErrNo"		/* Error number (Integer) */
#define FSLOG_KEY_IOTYPE	"IOType"	/* Type of IO (Read/Write) */
#define FSLOG_KEY_PHYS_BLKNUM	"PBlkNum"	/* Physical block number */
#define FSLOG_KEY_LOG_BLKNUM	"LBlkNum"	/* Logical block number */
#define FSLOG_KEY_DEVNODE	"DevNode"	/* Device node (f_mntfromname) */
#define FSLOG_KEY_PATH		"Path"		/* File system path */
#define FSLOG_KEY_MNTPT		"MountPt"	/* Mount point */

/* Values for type of error (FSLOG_KEY_ERR_TYPE) */
#define FSLOG_VAL_ERR_TYPE_IO	"IO"		/* IO error */
#define FSLOG_VAL_ERR_TYPE_FS	"FS"		/* FS error */

/* Values for type of operation (FSLOG_KEY_IOTYPE) */
#define FSLOG_VAL_IOTYPE_READ	"Read"
#define FSLOG_VAL_IOTYPE_WRITE	"Write"

#endif /* !_FSLOG_H_ */
