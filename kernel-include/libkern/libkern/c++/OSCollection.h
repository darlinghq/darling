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
/* IOCollection.h created by gvdl on Thu 1998-10-22 */

#ifndef _OS_OSCOLLECTION_H
#define _OS_OSCOLLECTION_H

#include <libkern/c++/OSObject.h>
#include <libkern/c++/OSPtr.h>

class OSDictionary;
class OSCollection;

typedef OSPtr<OSCollection> OSCollectionPtr;

template <typename T>
using OSCollectionTaggedPtr = OSTaggedPtr<T, OSCollection>;


/*!
 * @header
 *
 * @abstract
 * This header declares the OSDictionary collection class.
 */


/*!
 * @class OSCollection
 *
 * @abstract
 * The abstract superclass for Libkern collections.
 *
 * @discussion
 * OSCollection is the abstract superclass
 * for all Libkern C++ object collections.
 * It defines the necessary interfaces for managing storage space
 * and iterating through an arbitrary collection
 * (see the
 * @link //apple_ref/cpp/class/OSIterator OSIterator@/link
 * and
 * @link //apple_ref/cpp/class/OSCollectionIterator OSCollectionIterator@/link
 * classes).
 * It is up to concrete subclasses
 * to define their specific content management functions.
 *
 * <b>Use Restrictions</b>
 *
 * With very few exceptions in the I/O Kit, all Libkern-based C++
 * classes, functions, and macros are <b>unsafe</b>
 * to use in a primary interrupt context.
 * Consult the I/O Kit documentation related to primary interrupts
 * for more information.
 *
 * OSCollection provides no concurrency protection;
 * it's up to the usage context to provide any protection necessary.
 * Some portions of the I/O Kit, such as
 * @link //apple_ref/doc/class/IORegistryEntry IORegistryEntry@/link,
 * handle synchronization via defined member functions for setting
 * properties.
 */
class OSCollection : public OSObject
{
	friend class OSCollectionIterator;

	OSDeclareAbstractStructors(OSCollection);

	struct ExpansionData { };

protected:
/* Not to be included in headerdoc.
 *
 * @var updateStamp
 *
 * @abstract
 * A counter for changes to the collection object.
 *
 * @discussion
 * The update stamp is used primarily to track validity
 * of iteration contexts.
 * See @link //apple_ref/cpp/class/OSIterator OSIterator@/link and
 * @link //apple_ref/cpp/class/OSCollectionIterator OSCollectionIterator@/link
 * for more information.
 */
	unsigned int updateStamp;

#ifdef XNU_KERNEL_PRIVATE
protected:
#else
private:
#endif /* XNU_KERNEL_PRIVATE */
/* Reserved for future use.  (Internal use only)  */
// ExpansionData * reserved;
	unsigned int fOptions;

protected:
// Member functions used by the OSCollectionIterator class.


/*!
 * @function iteratorSize
 *
 * @abstract
 * Returns the size in bytes of a subclass's iteration context.
 *
 * @result
 * The size in bytes of the iteration context
 * needed by the subclass of OSCollection.
 *
 * @discussion
 * This pure virtual member function, which subclasses must implement,
 * is called by an
 * @link //apple_ref/doc/class/OSCollectionIterator OSCollectionIterator@/link
 * object so that it can allocate the storage needed
 * for the iteration context.
 * An iteration context contains the data necessary
 * to iterate through the collection.
 */
	virtual unsigned int iteratorSize() const = 0;


/*!
 * @function initIterator
 *
 * @abstract
 * Initializes the iteration context for a collection subclass.
 *
 * @param iterationContext  The iteration context to initialize.
 *
 * @result
 * <code>true</code> if initialization was successful,
 * <code>false</code> otherwise.
 *
 * @discussion
 * This pure virtual member function, which subclasses must implement,
 * is called by an
 * @link //apple_ref/doc/class/OSCollectionIterator OSCollectionIterator@/link
 * object to initialize an iteration context for a collection.
 * The collection object should interpret <code>iterationContext</code> appropriately
 * and initialize its contents to begin an iteration.
 *
 * This function can be called repeatedly for a given context,
 * whenever the iterator is reset via the
 * @link //apple_ref/cpp/instm/OSCollectionIterator/reset/virtualvoid/()
 * OSCollectionIterator::reset@/link
 * function.
 */
	virtual bool initIterator(void * iterationContext) const = 0;


/*!
 * @function getNextObjectForIterator
 *
 * @abstract
 * Returns the next member of a collection.
 *
 * @param iterationContext  The iteration context.
 * @param nextObject        The object returned by reference to the caller.
 *
 * @result
 * <code>true</code> if an object was found, <code>false</code> otherwise.
 *
 * @discussion
 * This pure virtual member function, which subclasses must implement,
 * is called by an
 * @link //apple_ref/doc/class/OSCollectionIterator OSCollectionIterator@/link
 * to get the next object for a given iteration context.
 * The collection object should interpret
 * <code>iterationContext</code> appropriately,
 * advance the context from its current object
 * to the next object (if it exists),
 * return that object by reference in <code>nextObject</code>,
 * and return <code>true</code> for the function call.
 * If there is no next object, the collection object must return <code>false</code>.
 *
 * For associative collections, the object returned should be the key
 * used to access its associated value, and not the value itself.
 */
	virtual bool getNextObjectForIterator(
		void      * iterationContext,
		OSObject ** nextObject) const = 0;


/*!
 * @function init
 *
 * @abstract
 * Initializes the OSCollection object.
 *
 * @result
 * <code>true</code> on success, <code>false</code> otherwise.
 *
 * @discussion
 * This function is used to initialize state
 * within a newly created OSCollection object.
 */
	virtual bool init() APPLE_KEXT_OVERRIDE;

public:

/*!
 * @typedef _OSCollectionFlags
 *
 * @const kImmutable
 * @discussion
 * Used with <code>@link setOptions setOptions@/link</code>
 * to indicate the collection's contents should
 * or should not change.
 *
 * An @link //apple_ref/doc/class/IORegistryEntry IORegistryEntry@/link
 * object marks collections immutable when set
 * as properties of a registry entry that's attached to a plane.
 * This is generally an advisory flag, used for debugging;
 * setting it does not mean a collection will in fact
 * disallow modifications.
 */
	typedef enum {
		kImmutable  = 0x00000001,
		kSort       = 0x00000002,
		kMASK       = (unsigned) - 1
	} _OSCollectionFlags;

// xx-review: should be protected, not public

/*!
 * @function haveUpdated
 *
 * @abstract
 * Tracks updates to the collection.
 *
 * @discussion
 * Subclasses call this function <i>before</i>
 * making any change to their contents (not after, as the name implies).
 * Update tracking is used for collection iterators,
 * and to enforce certain protections in the IORegistry.
 */
	void haveUpdated();


/*!
 * @function getCount
 *
 * @abstract
 * Returns the number of objects in the collection.
 *
 * @result
 * The number of objects in the collection.
 *
 * @discussion
 * Subclasses must implement this pure virtual member function.
 */
	virtual unsigned int getCount() const = 0;


/*!
 * @function getCapacity
 *
 * @abstract
 * Returns the number of objects the collection
 * can store without reallocating.
 *
 * @result
 * The number objects the collection
 * can store without reallocating.
 *
 * @discussion
 * Subclasses must implement this pure virtual member function.
 */
	virtual unsigned int getCapacity() const = 0;


/*!
 * @function getCapacityIncrement
 *
 * @abstract
 * Returns the storage increment of the collection.
 *
 * @result
 * The storage increment of the collection.
 *
 * @discussion
 * Subclasses must implement this pure virtual member function.
 * Most collection subclasses allocate their storage
 * in multiples of the capacity increment.
 *
 * See
 * <code>@link
 * //apple_ref/cpp/instm/OSCollection/ensureCapacity/virtualunsignedint/(unsignedint)
 * ensureCapacity@/link</code>
 * for how the capacity increment is used.
 */
	virtual unsigned int getCapacityIncrement() const = 0;


/*!
 * @function setCapacityIncrement
 *
 * @abstract
 * Sets the storage increment of the collection.
 *
 * @result
 * The new storage increment of the collection,
 * which may be different from the number requested.
 *
 * @discussion
 * Subclasses must implement this pure virtual member function.
 * Most collection subclasses allocate their storage
 * in multiples of the capacity increment.
 *
 * Collection subclasses should gracefully handle
 * an <code>increment</code> of zero
 * by applying (and returning) a positive minimum capacity.
 *
 * Setting the capacity increment does not trigger an immediate adjustment
 * of a collection's storage.
 *
 * See
 * @link
 * //apple_ref/cpp/instm/OSCollection/ensureCapacity/virtualunsignedint/(unsignedint)
 * ensureCapacity@/link
 * for how the capacity increment is used.
 */
	virtual unsigned int setCapacityIncrement(unsigned increment) = 0;


/*!
 * @function ensureCapacity
 *
 * @abstract
 * Ensures the collection has enough space to store
 * the requested number of objects.
 *
 * @param newCapacity  The total number of objects the collection
 *                     should be able to store.
 *
 * @result
 * The new capacity of the collection,
 * which may be different from the number requested
 * (if smaller, reallocation of storage failed).
 *
 * @discussion
 * Subclasses implement this pure virtual member function
 * to adjust their storage so that they can hold
 * at least <code>newCapacity</code> objects.
 * Libkern collections generally allocate storage
 * in multiples of their capacity increment.
 *
 * Subclass methods that add objects to the collection
 * should call this function before adding any object,
 * and should check the return value for success.
 *
 * Collection subclasses may reduce their storage
 * when the number of contained objects falls below some threshold,
 * but no Libkern collections currently do.
 */
	virtual unsigned int ensureCapacity(unsigned int newCapacity) = 0;


/*!
 * @function flushCollection
 *
 * @abstract
 * Empties the collection, releasing any objects retained.
 *
 * @discussion
 * Subclasses implement this pure virtual member function
 * to remove their entire contents.
 * This must not release the collection itself.
 */
	virtual void flushCollection() = 0;


/*!
 * @function setOptions
 *
 * @abstract
 * Recursively sets option bits in this collection
 * and all child collections.
 *
 * @param options  A bitfield whose values turn the options on (1) or off (0).
 * @param mask     A mask indicating which bits
 *                 in <code>options</code> to change.
 *                 Pass 0 to get the whole current options bitfield
 *                 without changing any settings.
 * @param context  Unused.
 *
 * @result
 * The options bitfield as it was before the set operation.
 *
 * @discussion
 * Kernel extensions should not call this function.
 *
 * The only option currently in use is
 * <code>@link //apple_ref/doc/title:econst/OSCollectionFlags/kImmutable
 * kImmutable@/link</code>.
 *
 * Subclasses should override this function to recursively apply
 * the options to their contents if the options actually change.
 */
	virtual unsigned setOptions(
		unsigned   options,
		unsigned   mask,
		void     * context = NULL);
	OSMetaClassDeclareReservedUsed(OSCollection, 0);

/*!
 * @function copyCollection
 *
 * @abstract
 * Creates a deep copy of a collection.
 *
 * @param cycleDict  A dictionary of all of the collections
 *                   that have been copied so far,
 *                   to start the copy at the top level
 *                   pass <code>NULL</code> for <code>cycleDict</code>.
 *
 * @result
 * The newly copied collecton,
 * <code>NULL</code> on failure.
 *
 * @discussion
 * This function copies the collection
 * and all of the contained collections recursively.
 * Objects that are not derived from OSCollection are retained
 * rather than copied.
 *
 * Subclasses of OSCollection must override this function
 * to properly support deep copies.
 */
	virtual OSCollectionPtr copyCollection(OSDictionary * cycleDict = NULL);
	OSMetaClassDeclareReservedUsed(OSCollection, 1);

/*!
 * @function iterateObjects
 *
 * @abstract
 * Invoke a callback for each member of the collection.
 *
 * @param refcon   A reference constant for the callback.
 * @param callback The callback function,
 *                 called with the refcon and each member object
 *                 of the collection in turn, on the callers thread.
 *                 The callback should return true to early terminate
 *                 the iteration, false otherwise.
 *
 * @result
 * False if the collection iteration was made invalid
 * (see OSCollectionIterator::isValid()) otherwise true.
 */
	bool iterateObjects(void * refcon, bool (*callback)(void * refcon, OSObject * object));

#ifdef __BLOCKS__

/*!
 * @function iterateObjects
 *
 * @abstract
 * Invoke a block for each member of the collection.
 *
 * @param block    The block,
 *                 called with the refcon and each member object
 *                 of the collection in turn, on the callers thread.
 *                 The block should return true to early terminate
 *                 the iteration, false otherwise.
 *
 * @result
 * False if the collection iteration was made invalid
 * (see OSCollectionIterator::isValid()) otherwise true.
 */
	bool iterateObjects(bool (^block)(OSObject * object));

#endif /* __BLOCKS__ */

	OSMetaClassDeclareReservedUnused(OSCollection, 2);
	OSMetaClassDeclareReservedUnused(OSCollection, 3);
	OSMetaClassDeclareReservedUnused(OSCollection, 4);
	OSMetaClassDeclareReservedUnused(OSCollection, 5);
	OSMetaClassDeclareReservedUnused(OSCollection, 6);
	OSMetaClassDeclareReservedUnused(OSCollection, 7);
};

#endif /* !_OS_OSCOLLECTION_H */
