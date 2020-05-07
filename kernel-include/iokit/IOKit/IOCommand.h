/*
 * Copyright (c) 2019 Apple Inc. All rights reserved.
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
 *
 *	Copyright (c) 2000 Apple Computer, Inc.  All rights reserved.
 *
 *	HISTORY
 *
 *	2001-01-18 gvdl	Made the primary queue pointer public, to be used when
 *			Ownership is clear.
 *	11/13/2000 CJS	Created IOCommand class and implementation
 *
 */

/*!
 * @header IOCommand
 * @abstract
 * This header contains the IOCommand class definition.
 */

#ifndef _IOKIT_IO_COMMAND_H_
#define _IOKIT_IO_COMMAND_H_

/*
 * Kernel
 */

#if defined(KERNEL) && defined(__cplusplus)

#include <kern/queue.h>
#include <libkern/c++/OSObject.h>

/*!
 * @class IOCommand
 * @abstract
 * This class is an abstract class which represents an I/O command.
 * @discussion
 * This class is an abstract class which represents an I/O command passed
 * from a device driver to a controller. All controller commands (e.g. IOATACommand)
 * should inherit from this class.
 */

class IOCommand : public OSObject
{
	OSDeclareDefaultStructors(IOCommand);

public:
	virtual bool init(void) APPLE_KEXT_OVERRIDE;

/*! @var fCommandChain
 *   This variable is used by the current 'owner' to queue the command.  During the life cycle of a command it moves through a series of queues.  This is the queue pointer for it.  Only valid while 'ownership' is clear.  For instance a IOCommandPool uses this pointer to maintain its list of free commands.  May be manipulated using the kern/queue.h macros */
	queue_chain_t fCommandChain;    /* used to queue commands */
};

#endif /* defined(KERNEL) && defined(__cplusplus) */

#endif  /* _IOKIT_IO_COMMAND_H_ */
