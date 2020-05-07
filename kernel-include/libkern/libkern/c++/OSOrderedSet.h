/*
 * Copyright (c) 2000-2019 Apple Inc. All rights reserved.
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

#ifndef _OS_OSORDEREDSET_H
#define _OS_OSORDEREDSET_H

#include <libkern/c++/OSCollection.h>
#include <libkern/c++/OSPtr.h>
#include <libkern/OSTypes.h>

class OSOffset;
class OSOrderedSet;

typedef OSPtr<OSOrderedSet> OSOrderedSetPtr;

/*!
 * @header
 *
 * @abstract
 * This header declares the OSOrderedSet collection class.
 */


/*!
 * @class OSOrderedSet
 *
 * @abstract
 * OSOrderedSet provides an ordered set store of objects.
 *
 * @discussion
 * OSOrderedSet is a container for Libkern C++ objects
 * (those derived from
 * @link //apple_ref/doc/class/OSMetaClassBase OSMetaClassBase@/link,
 * in particular @link //apple_ref/doc/class/OSObject OSObject@/link).
 * Storage and access follow ordered set logic.
 * A given object is stored in the set only once, but you can:
 * <ul>
 * <li>Define a sorting function for automated ordering
 *     (upon addition only)</li>
 * <li>Manually insert new objects in the set (overriding sorting)</li>
 * <li>Add and remove objects in the set</li>
 * <li>Test whether the set contains a particular object</li>
 * <li>Get the object stored at a particular index.</li>
 * </ul>
 *
 * Note that automated ordering is performed only upon addition of objects
 * and depends on the existing objects being properly sorted.
 * There is no function to re-sort the contents of an OSOrderedSet
 * or to change the ordering function.
 * In general, you should either use the one ordered-insertion function,
 * or the indexed-insertion functions, and not mix the two.
 *
 * As with all Libkern collection classes,
 * OSOrderedSet retains objects added to it,
 * and releases objects removed from it.
 * An OSOrderedSet also grows as necessary to accommodate new objects,
 * <i>unlike</i> Core Foundation collections (it does not, however, shrink).
 *
 * <b>Use Restrictions</b>
 *
 * With very few exceptions in the I/O Kit, all Libkern-based C++
 * classes, functions, and macros are <b>unsafe</b>
 * to use in a primary interrupt context.
 * Consult the I/O Kit documentation related to primary interrupts
 * for more information.
 *
 * OSOrderedSet provides no concurrency protection;
 * it's up to the usage context to provide any protection necessary.
 * Some portions of the I/O Kit, such as
 * @link //apple_ref/doc/class/IORegistryEntry IORegistryEntry@/link,
 * handle synchronization via defined member functions for setting
 * properties.
 */
class OSOrderedSet : public OSCollection
{
	OSDeclareDefaultStructors(OSOrderedSet);

public:
/*!
 * @typedef OSOrderFunction
 *
 * @abstract
 * The sorting function used by an OSOrderedSet to order objects.
 *
 * @param obj1     An object from the ordered set. May be <code>NULL</code>.
 * @param obj2     The object being ordered within the ordered set.
 *                 May be <code>NULL</code>.
 * @param context  A pointer to a user-provided context. May be <code>NULL</code>.
 *
 * @result
 * A comparison result of the object:
 * <ul>
 *   <li>a negative value if obj2 should precede obj1,</li>
 *   <li>a positive value if obj1 should precede obj2,</li>
 *   <li>and 0 if obj1 and obj2 have an equivalent ordering.</li>
 * </ul>
 */
	typedef SInt32 (*OSOrderFunction)(const OSMetaClassBase * obj1,
	    const OSMetaClassBase * obj2,
	    void * context);

protected:
	struct _Element * array;
	OSOrderFunction   ordering;
	void            * orderingRef;
	unsigned int      count;
	unsigned int      capacity;
	unsigned int      capacityIncrement;

	struct ExpansionData { };

/* Reserved for future use.  (Internal use only)  */
	ExpansionData *reserved;

protected:
/* OSCollectionIterator interfaces. */
	virtual unsigned int iteratorSize() const APPLE_KEXT_OVERRIDE;
	virtual bool initIterator(void *iterator) const APPLE_KEXT_OVERRIDE;
	virtual bool getNextObjectForIterator(void *iterator, OSObject **ret) const APPLE_KEXT_OVERRIDE;

public:

/*!
 * @function withCapacity
 *
 * @abstract
 * Creates and initializes an empty OSOrderedSet.
 *
 * @param capacity         The initial storage capacity
 *                         of the new ordered set object.
 * @param orderFunc        A C function that implements the sorting algorithm
 *                         for the set.
 * @param orderingContext  An ordering context,
 *                         which is passed to <code>orderFunc</code>.
 * @result
 * An empty instance of OSOrderedSet
 * with a retain count of 1;
 * <code>NULL</code> on failure.
 *
 * @discussion
 * <code>capacity</code> must be nonzero.
 * The new OSOrderedSet will grow as needed
 * to accommodate more key/object pairs
 * (<i>unlike</i> Core Foundation collections,
 * for which the initial capacity is a hard limit).
 *
 * If <code>orderFunc</code> is provided, it is used by
 * <code>@link
 * //apple_ref/cpp/instm/OSOrderedSet/setObject/virtualbool/(constOSMetaClassBase*)
 * setObject(const OSMetaClassBase *)@/link</code>
 * to determine where to insert a new object.
 * Other object-setting functions ignore ordering.
 *
 * <code>orderingContext</code> is not retained or otherwise memory-managed
 * by the ordered set.
 * If it needs to be deallocated,
 * you must track references to it and the ordered set
 * in order to deallocate it appropriately.
 * See
 * <code>@link getOrderingRef getOrderingRef@/link</code>.
 */
	static OSOrderedSetPtr withCapacity(
		unsigned int      capacity,
		OSOrderFunction   orderFunc = NULL,
		void            * orderingContext = NULL);


/*!
 * @function initWithCapacity
 *
 * @abstract
 * Initializes a new instance of OSOrderedSet.
 *
 * @param capacity         The initial storage capacity
 *                         of the new ordered set object.
 * @param orderFunc        A C function that implements the sorting algorithm
 *                         for the set.
 * @param orderingContext  An ordering context,
 *                         which is passed to <code>orderFunc</code>.
 *
 * @result
 * <code>true</code> on success, <code>false</code> on failure.
 *
 * @discussion
 * Not for general use. Use the static instance creation method
 * <code>@link
 * //apple_ref/cpp/clm/OSOrderedSet/withCapacity/staticOSOrderedSet*\/(unsignedint,OSOrderFunction,void*)
 * withCapacity@/link</code>
 * instead.
 *
 * <code>capacity</code> must be nonzero.
 * The new set will grow as needed to accommodate more key/object pairs
 * (<i>unlike</i> Core Foundation collections,
 * for which the initial capacity is a hard limit).
 *
 * If <code>orderFunc</code> is provided, it is used by
 * <code>@link
 * //apple_ref/cpp/instm/OSOrderedSet/setObject/virtualbool/(constOSMetaClassBase*)
 * setObject(const OSMetaClassBase *)@/link</code>
 * to determine where to insert a new object.
 * Other object-setting functions ignore ordering.
 *
 * <code>orderingContext</code> is not retained or otherwise memory-managed
 * by the ordered set.
 * If it needs to be deallocated,
 * you must track references to it and the ordered set
 * in order to deallocate it appropriately.
 * See
 * <code>@link getOrderingRef getOrderingRef@/link</code>.
 */
	virtual bool initWithCapacity(
		unsigned int      capacity,
		OSOrderFunction   orderFunc = NULL,
		void            * orderingContext = NULL);


/*!
 * @function free
 *
 * @abstract
 * Deallocatesand releases any resources
 * used by the OSOrderedSet instance.
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
 * @function getCount
 *
 * @abstract
 * Returns the current number of objects within the ordered set.
 *
 * @result
 * The current number of objects within the ordered set.
 */
	virtual unsigned int getCount() const APPLE_KEXT_OVERRIDE;


/*!
 * @function getCapacity
 *
 * @abstract
 * Returns the number of objects the ordered set
 * can store without reallocating.
 *
 * @result
 * The number objects the ordered set
 * can store without reallocating.
 *
 * @discussion
 * OSOrderedSet objects grow when full to accommodate additional objects.
 * See
 * <code>@link
 * //apple_ref/cpp/instm/OSOrderedSet/getCapacityIncrement/virtualunsignedint/()
 * getCapacityIncrement@/link</code>
 * and
 * <code>@link
 * //apple_ref/cpp/instm/OSOrderedSet/ensureCapacity/virtualunsignedint/(unsignedint)
 * ensureCapacity@/link</code>.
 */
	virtual unsigned int getCapacity() const APPLE_KEXT_OVERRIDE;


/*!
 * @function getCapacityIncrement
 *
 * @abstract
 * Returns the storage increment of the ordered set.
 *
 * @result
 * The storage increment of the ordered set.
 *
 * @discussion
 * An OSOrderedSet allocates storage for objects in multiples
 * of the capacity increment.
 */
	virtual unsigned int getCapacityIncrement() const APPLE_KEXT_OVERRIDE;


/*!
 * @function setCapacityIncrement
 *
 * @abstract
 * Sets the storage increment of the ordered set.
 *
 * @result
 * The new storage increment of the ordered set,
 * which may be different from the number requested.
 *
 * @discussion
 * An OSOrderedSet allocates storage for objects in multiples
 * of the capacity increment.
 * Calling this function does not immediately reallocate storage.
 */
	virtual unsigned int setCapacityIncrement(unsigned increment) APPLE_KEXT_OVERRIDE;


/*!
 * @function ensureCapacity
 *
 * @abstract
 * Ensures the set has enough space
 * to store the requested number of distinct objects.
 *
 * @param newCapacity  The total number of distinct objects the ordered set
 *                     should be able to store.
 *
 * @result
 * The new capacity of the ordered set,
 * which may be different from the number requested
 * (if smaller, reallocation of storage failed).
 *
 * @discussion
 * This function immediately resizes the ordered set, if necessary,
 * to accommodate at least <code>newCapacity</code> distinct objects.
 * If <code>newCapacity</code> is not greater than the current capacity,
 * or if an allocation error occurs, the original capacity is returned.
 *
 * There is no way to reduce the capacity of an OSOrderedSet.
 */
	virtual unsigned int ensureCapacity(unsigned int newCapacity) APPLE_KEXT_OVERRIDE;


/*!
 * @function flushCollection
 *
 * @abstract
 * Removes and releases all objects within the ordered set.
 *
 * @discussion
 * The ordered set's capacity (and therefore direct memory consumption)
 * is not reduced by this function.
 */
	virtual void flushCollection() APPLE_KEXT_OVERRIDE;


/*!
 * @function setObject
 *
 * @abstract
 * Adds an object to the OSOrderedSet if it is not already present,
 * storing it in sorted order if there is an order function.
 *
 * @param anObject  The OSMetaClassBase-derived object to be added
 *                  to the ordered set.
 * @result
 * <code>true</code> if <code>anObject</code> was successfully
 * added to the ordered set, <code>false</code> otherwise
 * (including if it was already in the ordered set).
 *
 * @discussion
 * The set adds storage to accomodate the new object, if necessary.
 * If successfully added, the object is retained.
 *
 * If <code>anObject</code> is not already in the ordered set
 * and there is an order function,
 * this function loops through the existing objects,
 * calling the @link OSOrderFunction order function@/link
 * with arguments each existingObject, <code>anObject</code>,
 * and the ordering context
 * (or <code>NULL</code> if none was set),
 * until the order function returns
 * a value <i>greater than</i> or equal to 0.
 * It then inserts <code>anObject</code> at the index of the existing object.
 *
 * If there is no order function, the object is inserted at index 0.
 *
 * A <code>false</code> return value can mean either
 * that <code>anObject</code> is already present in the set,
 * or that a memory allocation failure occurred.
 * If you need to know whether the object
 * is already present, use
 * <code>@link
 * //apple_ref/cpp/instm/OSOrderedSet/containsObject/virtualbool/(constOSMetaClassBase*)
 * containsObject(const OSMetaClassBase *)@/link</code>.
 */
	virtual bool setObject(const OSMetaClassBase * anObject);


/*!
 * @function setFirstObject
 *
 * @abstract
 * Adds an object to the OSOrderedSet at index 0
 * if it is not already present.
 *
 * @param anObject  The OSMetaClassBase-derived object
 *                  to be added to the ordered set.
 * @result
 * <code>true</code> if <code>anObject</code> was successfully added
 * to the ordered set, <code>false</code> otherwise
 * (including if it was already in the ordered set at any index).
 *
 * @discussion
 * The set adds storage to accomodate the new object, if necessary.
 * If successfully added, the object is retained.
 *
 * This function ignores any ordering function of the ordered set,
 * and can disrupt the automatic sorting mechanism.
 * Only call this function if you are managing the ordered set directly.
 *
 * A <code>false</code> return value can mean either that <code>anObject</code>
 * is already present in the set,
 * or that a memory allocation failure occurred.
 * If you need to know whether the object
 * is already present, use
 * <code>@link
 * //apple_ref/cpp/instm/OSOrderedSet/containsObject/virtualbool/(constOSMetaClassBase*)
 * containsObject(const OSMetaClassBase *)@/link</code>.
 */
	virtual bool setFirstObject(const OSMetaClassBase * anObject);


/*!
 * @function setLastObject
 *
 * @abstract
 * Adds an object at the end of the OSOrderedSet
 * if it is not already present.
 *
 * @param anObject  The OSMetaClassBase-derived object to be added
 *                  to the ordered set.
 * @result
 * <code>true</code> if <code>anObject</code> was successfully added
 * to the ordered set, <code>false</code> otherwise
 * (including if it was already in the ordered set at any index).
 *
 * @discussion
 * The set adds storage to accomodate the new object, if necessary.
 * If successfully added, the object is retained.
 *
 * This function ignores any ordering function of the ordered set,
 * and can disrupt the automatic sorting mechanism.
 * Only call this function if you are managing the ordered set directly.
 *
 * A <code>false</code> return value can mean either that <code>anObject</code>
 * is already present in the set,
 * or that a memory allocation failure occurred.
 * If you need to know whether the object
 * is already present, use
 * <code>@link
 * //apple_ref/cpp/instm/OSOrderedSet/containsObject/virtualbool/(constOSMetaClassBase*)
 * containsObject(const OSMetaClassBase *)@/link</code>.
 */
	virtual bool setLastObject(const OSMetaClassBase * anObject);


/*!
 * @function removeObject
 *
 * @abstract
 * Removes an object from the ordered set.
 *
 * @param anObject  The OSMetaClassBase-derived object
 *                  to be removed from the ordered set.
 *
 * @discussion
 * The object removed from the ordered set is released.
 */
	virtual void removeObject(const OSMetaClassBase * anObject);


/*!
 * @function containsObject
 *
 * @abstract
 * Checks the ordered set for the presence of an object.
 *
 * @param anObject  The OSMetaClassBase-derived object to check for
 *                  in the ordered set.
 *
 * @result
 * <code>true</code> if <code>anObject</code> is present
 * within the ordered set, <code>false</code> otherwise.
 *
 * @discussion
 * Pointer equality is used.
 * This function returns <code>false</code> if passed <code>NULL</code>.
 */
	virtual bool containsObject(const OSMetaClassBase * anObject) const;


/*!
 * @function member
 *
 * @abstract
 * Checks the ordered set for the presence of an object.
 *
 * @param anObject  The OSMetaClassBase-derived object to check for
 *                  in the ordered set.
 *
 * @result
 * <code>true</code> if <code>anObject</code> is present
 *                 within the ordered set, <code>false</code> otherwise.
 *
 * @discussion
 * Pointer equality is used.
 * Returns <code>false</code> if passed <code>NULL</code>.
 *
 * <code>@link
 * //apple_ref/cpp/instm/OSOrderedSet/containsObject/virtualbool/(constOSMetaClassBase*)
 * containsObject(const OSMetaClassBase *)@/link</code>
 * checks for <code>NULL</code> before scanning the contents,
 * and is therefore more efficient than this function.
 */
	virtual bool member(const OSMetaClassBase * anObject) const;


/*!
 * @function getFirstObject
 *
 * @abstract
 * The object at index 0 in the ordered set if there is one,
 * otherwise <code>NULL</code>.
 *
 * @discussion
 * The returned object will be released if removed from the ordered set;
 * if you plan to store the reference, you should call
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/retain/virtualvoid/()
 * retain@/link</code>
 * on that object.
 */
	virtual OSObject * getFirstObject() const;


/*!
 * @function getLastObject
 *
 * @abstract
 * The last object in the ordered set if there is one,
 * otherwise <code>NULL</code>.
 *
 * @discussion
 * The returned object will be released if removed from the ordered set;
 * if you plan to store the reference, you should call
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/retain/virtualvoid/()
 * retain@/link</code>
 * on that object.
 */
	virtual OSObject * getLastObject() const;


/*!
 * @function orderObject
 *
 * @abstract
 * Calls the ordered set's order function against a <code>NULL</code> object.
 *
 * @param anObject  The object to be ordered.
 *
 * @result
 * The ordering value for the object.
 *
 * @discussion
 * This function calls the ordered set's
 * @link OSOrderFunction order function@/link
 * with <code>anObject</code>, <code>NULL</code>, and the ordering context
 * (or <code>NULL</code> if none was set),
 * and returns the result of that function.
 */
	virtual SInt32 orderObject(const OSMetaClassBase * anObject);


/*!
 * @function setObject
 *
 * @abstract
 * Adds an object to an OSOrderedSet at a specified index
 * if it is not already present.
 *
 * @param index     The index at which to insert the new object.
 * @param anObject  The OSMetaClassBase-derived object to be added
 *                  to the ordered set.
 *
 * @result
 * <code>true</code> if the object was successfully added
 * to the ordered set, <code>false</code> otherwise
 * (including if it was already in the set).
 *
 * @discussion
 * The set adds storage to accomodate the new object, if necessary.
 * If successfully added, the object is retained.
 *
 * This function ignores any ordering function of the ordered set,
 * and can disrupt the automatic sorting mechanism.
 * Only call this function if you are managing the ordered set directly.
 *
 * A <code>false</code> return value can mean either that the object
 * is already present in the set,
 * or that a memory allocation failure occurred.
 * If you need to know whether the object
 * is already present, use
 * <code>@link //apple_ref/cpp/instm/OSOrderedSet/containsObject/virtualbool/(constOSMetaClassBase*)
 * containsObject containsObject@/link</code>.
 */
	virtual bool setObject(
		unsigned int            index,
		const OSMetaClassBase * anObject);


/*!
 * @function getObject
 *
 * @abstract
 * Gets the object at a particular index.
 *
 * @param index  The index into the set.
 * @result
 * The object at the given index,
 * or <code>NULL</code> if none exists at that location.
 *
 * @discussion
 * The returned object will be released if removed from the set;
 * if you plan to store the reference, you should call
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/retain/virtualvoid/()
 * retain@/link</code>
 * on that object.
 */
	virtual OSObject * getObject(unsigned int index) const;


/*!
 * @function getOrderingRef
 *
 * @abstract
 * Returns the ordering context the ordered set was created with.
 *
 * @result
 * The ordered set's ordering context,
 * or <code>NULL</code> if it doesn't have one.
 */
	virtual void * getOrderingRef();


/*!
 * @function isEqualTo
 *
 * @abstract
 * Tests the equality of two OSOrderedSet objects.
 *
 * @param anOrderedSet  The ordered set object being compared
 *                      against the receiver.
 * @result
 * <code>true</code> if the two sets are equivalent,
 * <code>false</code> otherwise.
 *
 * @discussion
 * Two OSOrderedSet objects are considered equal if they have same count
 * and the same object pointer values in the same order.
 */
	virtual bool isEqualTo(const OSOrderedSet * anOrderedSet) const;


/*!
 * @function isEqualTo
 *
 * @abstract
 * Tests the equality of an OSOrderedSet
 * against an arbitrary object.
 *
 * @param anObject  The object being compared against the receiver.
 * @result
 * <code>true</code> if the two objects are equivalent,
 * <code>false</code> otherwise.
 *
 * @discussion
 * An OSOrderedSet object is considered equal to another object
 * if the other object is derived from OSOrderedSet
 * and compares equal as an OSOrderedSet.
 */
	virtual bool isEqualTo(const OSMetaClassBase * anObject) const APPLE_KEXT_OVERRIDE;


/*!
 * @function setOptions
 *
 * Recursively sets option bits in the ordered set
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
 * Child collections' options are changed only if the receiving ordered set's
 * options actually change.
 */
	virtual unsigned setOptions(
		unsigned   options,
		unsigned   mask,
		void     * context = NULL) APPLE_KEXT_OVERRIDE;


/*!
 * @function copyCollection
 *
 * @abstract
 * Creates a deep copy of this ordered set and its child collections.
 *
 * @param cycleDict  A dictionary of all of the collections
 *                   that have been copied so far,
 *                   which is used to track circular references.
 *                   To start the copy at the top level,
 *                   pass <code>NULL</code>.
 *
 * @result
 * The newly copied ordered set, with a retain count of 1,
 * or <code>NULL</code> if there is insufficient memory to do the copy.
 *
 * @discussion
 * The receiving ordered set, and any collections it contains,
 * recursively, are copied.
 * Objects that are not derived from OSCollection are retained
 * rather than copied.
 */
	OSCollectionPtr copyCollection(OSDictionary * cycleDict = NULL) APPLE_KEXT_OVERRIDE;

	OSMetaClassDeclareReservedUnused(OSOrderedSet, 0);
	OSMetaClassDeclareReservedUnused(OSOrderedSet, 1);
	OSMetaClassDeclareReservedUnused(OSOrderedSet, 2);
	OSMetaClassDeclareReservedUnused(OSOrderedSet, 3);
	OSMetaClassDeclareReservedUnused(OSOrderedSet, 4);
	OSMetaClassDeclareReservedUnused(OSOrderedSet, 5);
	OSMetaClassDeclareReservedUnused(OSOrderedSet, 6);
	OSMetaClassDeclareReservedUnused(OSOrderedSet, 7);
};

#endif /* ! _OS_OSORDEREDSET_H */
