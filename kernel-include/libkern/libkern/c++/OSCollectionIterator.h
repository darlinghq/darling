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
/* IOCollectionIterator.h created by gvdl on Fri 1998-10-30 */

#ifndef _OS_OSCOLLECTIONITERATOR_H
#define _OS_OSCOLLECTIONITERATOR_H

#include <libkern/c++/OSIterator.h>
#include <libkern/c++/OSCollection.h>
#include <libkern/c++/OSPtr.h>

class OSCollectionIterator;

typedef OSPtr<OSCollectionIterator> OSCollectionIteratorPtr;

/*!
 * @header
 *
 * @abstract
 * This header declares the OSCollectionIterator collection class.
 */


/*!
 * @class OSCollectionIterator
 *
 * @discussion
 * OSCollectionIterator defines a consistent mechanism to iterate
 * through the objects of an OSCollection.
 * It expands on the basic interface of
 * @link //apple_ref/cpp/class/OSIterator OSIterator@/link
 * to allow association of an iterator with a specific collection.
 *
 * To use an OSCollectionIterator, you create it with the collection
 * to be iterated, then call
 * @link //apple_ref/cpp/class/OSIterator OSIterator@/link
 * as long as it returns an object:
 *
 * @textblock
 * <pre>
 *     OSCollectionIterator * iterator =
 *         OSCollectionIterator::withCollection(myCollection);
 *     OSObject * object;
 *     while (object = iterator->getNextObject()) {
 *         // do something with object
 *     }
 *     // optional
 *     if (!iterator->isValid()) {
 *         // report that collection changed during iteration
 *     }
 *     iterator->release();
 * </pre>
 * @/textblock
 *
 * Note that when iterating associative collections,
 * the objects returned by <code>getNextObject</code> are keys;
 * if you want to work with the associated values,
 * simply look them up in the collection with the keys.
 *
 * <b>Use Restrictions</b>
 *
 * With very few exceptions in the I/O Kit, all Libkern-based C++
 * classes, functions, and macros are <b>unsafe</b>
 * to use in a primary interrupt context.
 * Consult the I/O Kit documentation related to primary interrupts
 * for more information.
 *
 * OSCollectionIterator provides no concurrency protection.
 */
class OSCollectionIterator : public OSIterator
{
	OSDeclareDefaultStructors(OSCollectionIterator);

protected:
// xx-review: Do we want to document these?
	OSPtr<const OSCollection> collection;
	void               * collIterator;
	unsigned int         initialUpdateStamp;
	bool                 valid;

public:
/*!
 * @function withCollection
 *
 * @abstract
 * Creates and initializes an OSCollectionIterator
 * for the provided collection object.
 *
 * @param inColl  The OSCollection-derived collection object to be iteratated.
 *
 * @result
 * A new instance of OSCollectionIterator, or <code>NULL</code> on failure.
 */
	static OSCollectionIteratorPtr withCollection(const OSCollection * inColl);


/*!
 * @function initWithCollection
 *
 * @abstract
 * Initializes an OSCollectionIterator
 * for the provided collection object.
 *
 * @param inColl  The OSCollection-derived collection object to be iteratated.
 * @result
 * <code>true</code> if the initialization was successful,
 * or <code>false</code> on failure.
 *
 * @discussion
 * Not for general use. Use the static instance creation method
 * <code>@link withCollection withCollection@/link</code> instead.
 */
	virtual bool initWithCollection(const OSCollection * inColl);


/*!
 * @function free
 *
 * @abstract
 * Releases or deallocates any resources used
 * by the OSCollectionIterator object.
 *
 * @discussion
 * This function should not be called directly;
 * use
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/release/virtualvoid/()
 * release@/link</code>
 * instead.
 */
	virtual void free() APPLE_KEXT_OVERRIDE;


/*!
 * @function reset
 *
 * @abstract
 * Resets the iterator to the beginning of the collection,
 * as if it had just been created.
 */
	virtual void reset() APPLE_KEXT_OVERRIDE;


/*!
 * @function isValid
 *
 * @abstract
 * Checks that the collection hasn't been modified during iteration.
 *
 * @return
 * <code>true</code> if the iterator is valid for continued use,
 * <code>false</code> otherwise
 * (typically because the iteration context has been modified).
 */
	virtual bool isValid() APPLE_KEXT_OVERRIDE;


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
 * This function first calls
 * <code>@link //apple_ref/cpp/instm/OSCollectionIterator/isValid/virtualbool/()
 * isValid@/link</code>
 * and returns <code>NULL</code> if that function
 * returns <code>false</code>.
 *
 * Subclasses must implement this pure virtual function
 * to check for validity with
 * <code>@link
 * //apple_ref/cpp/instm/OSCollectionIterator/isValid/virtualbool/()
 * isValid@/link</code>,
 * and then to advance the iteration context to the next object (if any)
 * and return that next object, or <code>NULL</code> if there is none.
 */
	virtual OSObject * getNextObject() APPLE_KEXT_OVERRIDE;
};

#endif /* !_OS_OSCOLLECTIONITERATOR_H */
