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
 * Copyright (c) 1998-1999 Apple Computer, Inc.  All rights reserved.
 *
 * HISTORY
 *
 */

#ifndef _OS_OSITERATOR_H
#define _OS_OSITERATOR_H

#include <libkern/c++/OSObject.h>

/*!
 * @header
 *
 * @abstract
 * This header declares the OSIterator collection class.
 */


/*!
 * @class OSIterator
 * @abstract
 * The abstract superclass for Libkern iterators.
 *
 * @discussion
 * OSIterator is the abstract superclass for all Libkern C++ object iterators.
 * It defines the basic interface for iterating and resetting.
 * See @link //apple_ref/cpp/macro/OSCollection OSCollection@/link and
 * @link //apple_ref/cpp/macro/OSCollectionIterator OSCollectionIterator@/link
 * for more information.
 *
 * With very few exceptions in the I/O Kit, all Libkern-based C++
 * classes, functions, and macros are <b>unsafe</b>
 * to use in a primary interrupt context.
 * Consult the I/O Kit documentation related to primary interrupts
 * for more information.
 *
 * OSIterator provides no concurrency protection.
 */
class OSIterator : public OSObject
{
	OSDeclareAbstractStructors(OSIterator);

public:
/*!
 * @function reset
 *
 * @abstract
 * Resets the iterator to the beginning of the collection,
 * as if it had just been created.
 *
 * @discussion
 * Subclasses must implement this pure virtual member function.
 */
	virtual void reset() = 0;


/*!
 * @function isValid
 *
 * @abstract
 * Check that the collection hasn't been modified during iteration.
 *
 * @result
 * <code>true</code> if the iterator is valid for continued use,
 * <code>false</code> otherwise
 * (typically because the collection being iterated has been modified).
 *
 * @discussion
 * Subclasses must implement this pure virtual member function.
 */
	virtual bool isValid() = 0;


/*!
 * @function getNextObject
 *
 * @abstract
 * Advances to and returns the next object in the iteration.
 *
 * @return
 * The next object in the iteration context,
 * <code>NULL</code> if there is no next object
 * or if the iterator is no longer valid.
 *
 * @discussion
 * The returned object will be released if removed from the collection;
 * if you plan to store the reference, you should call
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/retain/virtualvoid/()
 * retain@/link</code>
 * on that object.
 *
 * Subclasses must implement this pure virtual function
 * to check for validity with
 * <code>@link isValid isValid@/link</code>,
 * and then to advance the iteration context to the next object (if any)
 * and return that next object, or <code>NULL</code> if there is none.
 */
	virtual OSObject *getNextObject() = 0;

	OSMetaClassDeclareReservedUnused(OSIterator, 0);
	OSMetaClassDeclareReservedUnused(OSIterator, 1);
	OSMetaClassDeclareReservedUnused(OSIterator, 2);
	OSMetaClassDeclareReservedUnused(OSIterator, 3);
};

#endif /* ! _OS_OSITERATOR_H */
