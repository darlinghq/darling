/*
 * Copyright (c) 1998-2019 Apple Inc. All rights reserved.
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
 * Copyright (c) 1999 Apple Computer, Inc.  All rights reserved.
 *
 * HISTORY
 *
 */

#ifndef _IOKIT_IONOTIFIER_H
#define _IOKIT_IONOTIFIER_H

#include <libkern/c++/OSObject.h>

/*! @class IONotifier : public OSObject
 *   @abstract An abstract base class defining common methods for controlling a notification request.
 *   @discussion IOService notification requests are represented as implementations of the IONotifier object. It defines methods to enable, disable and remove notification requests. These actions are synchronized with invocations of the notification handler, so removing a notification request will guarantee the handler is not being executed. */

class IONotifier : public OSObject
{
	OSDeclareAbstractStructors(IONotifier);

public:

/*! @function remove
 *   @abstract Removes the notification request and releases it.
 *   @discussion Removes the notification request and release it. Since creating an IONotifier instance will leave it with a retain count of one, creating an IONotifier and then removing it will destroy it. This method is synchronous with any handler invocations, so when this method returns its guaranteed the handler will not be in entered. */

	virtual void remove() = 0;

/*! @function disable
 *   @abstract Disables the notification request.
 *   @discussion Disables the notification request. This method is synchronous with any handler invocations, so when this method returns its guaranteed the handler will not be in entered.
 *   @result Returns the previous enable state of the IONotifier. */

	virtual bool disable() = 0;

/*! @function enable
 *   @abstract Sets the enable state of the notification request.
 *   @discussion Restores the enable state of the notification request, given the previous state passed in.
 *   @param was The enable state of the notifier to restore. */

	virtual void enable( bool was ) = 0;
};

#endif /* ! _IOKIT_IONOTIFIER_H */
