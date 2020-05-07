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
/* IOSet.h created by rsulack on Thu 11-Jun-1998 */
/* IOSet.h converted to C++ by gvdl on Fri 1998-10-30 */

#ifndef _OS_OSSET_H
#define _OS_OSSET_H

#include <libkern/c++/OSCollection.h>
#include <libkern/c++/OSPtr.h>

class OSArray;
class OSSet;

typedef OSPtr<OSSet> OSSetPtr;
typedef OSPtr<OSArray> OSArrayPtr;

/*!
 * @header
 *
 * @abstract
 * This header declares the OSSet collection class.
 */


/*!
 * @class OSSet
 *
 * @abstract
 * OSSet provides an unordered set store of objects.
 *
 * @discussion
 * OSSet is a container for Libkern C++ objects
 * (those derived from
 * @link //apple_ref/doc/class/OSMetaClassBase OSMetaClassBase@/link,
 * in particular @link //apple_ref/doc/class/OSObject OSObject@/link).
 * Storage and access follow basic set logic: you can add or remove an object,
 * and test whether the set contains a particular object.
 * A given object is only stored in the set once,
 * and there is no ordering of objects in the set.
 * A subclass @link //apple_ref/doc/class/OSOrderedSet OSOrderedSet@/link,
 * provides for ordered set logic.
 *
 * As with all Libkern collection classes,
 * OSSet retains objects added to it,
 * and releases objects removed from it.
 * An OSSet also grows as necessary to accommodate new objects,
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
 * OSSet provides no concurrency protection;
 * it's up to the usage context to provide any protection necessary.
 * Some portions of the I/O Kit, such as
 * @link //apple_ref/doc/class/IORegistryEntry IORegistryEntry@/link,
 * handle synchronization via defined member functions for setting
 * properties.
 */
class OSSet : public OSCollection
{
	friend class OSSerialize;

	OSDeclareDefaultStructors(OSSet);

#if APPLE_KEXT_ALIGN_CONTAINERS

private:
	OSArrayPtr members;

#else /* APPLE_KEXT_ALIGN_CONTAINERS */

private:
	OSArrayPtr members;

protected:
	struct ExpansionData { };

/* Reserved for future use.  (Internal use only)  */
	ExpansionData * reserved;

#endif /* APPLE_KEXT_ALIGN_CONTAINERS */

/*
 * OSCollectionIterator interfaces.
 */
	virtual unsigned int iteratorSize() const APPLE_KEXT_OVERRIDE;
	virtual bool initIterator(void * iterator) const APPLE_KEXT_OVERRIDE;
	virtual bool getNextObjectForIterator(void * iterator, OSObject ** ret) const APPLE_KEXT_OVERRIDE;

public:


/*!
 * @function withCapacity
 *
 * @abstract
 * Creates and initializes an empty OSSet.
 *
 * @param   capacity The initial storage capacity of the new set object.
 *
 * @result
 * An empty instance of OSSet
 *         with a retain count of 1;
 * <code>NULL</code> on failure.
 *
 * @discussion
 * <code>capacity</code> must be nonzero.
 * The new OSSet will grow as needed to accommodate more key/object pairs
 * (<i>unlike</i> @link //apple_ref/doc/uid/20001503 CFMutableSet@/link,
 * for which the initial capacity is a hard limit).
 */
	static OSSetPtr withCapacity(unsigned int capacity);


/*!
 * @function withObjects
 *
 * @abstract
 * Creates and initializes an OSSet
 * populated with objects provided.
 *
 * @param objects   A C array of OSMetaClassBase-derived objects.
 * @param count     The number of objects to be placed into the set.
 * @param capacity  The initial storage capacity of the new set object.
 *                  If 0, <code>count</code> is used; otherwise this value
 *                  must be greater than or equal to <code>count</code>.
 *
 * @result
 * An instance of OSSet
 * containing the objects provided,
 * with a retain count of 1;
 * <code>NULL</code> on failure.
 *
 * @discussion
 * <code>objects</code> must be non-<code>NULL</code>,
 * and <code>count</code> must be nonzero.
 * If <code>capacity</code> is nonzero,
 * it must be greater than or equal to <code>count</code>.
 * The new OSSet will grow as needed to accommodate more objects
 * (<i>unlike</i> @link //apple_ref/doc/uid/20001503 CFMutableSet@/link,
 * for which the initial capacity is a hard limit).
 *
 * The objects in <code>objects</code> are retained for storage in the new set,
 * not copied.
 */
	static OSSetPtr withObjects(
		const OSObject * objects[],
		unsigned int     count,
		unsigned int     capacity = 0);


/*!
 * @function withArray
 *
 * @abstract
 * Creates and initializes an OSSet
 * populated with the contents of an OSArray.
 *
 * @param array     An array whose objects will be stored in the new OSSet.
 * @param capacity  The initial storage capacity of the new set object.
 *                  If 0, the capacity is set to the number of objects
 *                  in <code>array</code>;
 *                  otherwise <code>capacity</code> must be greater than or equal to
 *                  the number of objects in <code>array</code>.
 * @result
 * An instance of OSSet containing
 * the objects of <code>array</code>,
 * with a retain count of 1;
 * <code>NULL</code> on failure.
 *
 * @discussion
 * Each distinct object in <code>array</code> is added to the new set.
 *
 * <code>array</code> must be non-<code>NULL</code>.
 * If <code>capacity</code> is nonzero,
 * it must be greater than or equal to <code>count</code>.
 * The new OSSet will grow as needed to accommodate more key-object pairs
 * (<i>unlike</i> @link //apple_ref/doc/uid/20001503 CFMutableSet@/link,
 * for which the initial capacity is a hard limit).
 *
 * The objects in <code>array</code> are retained for storage in the new set,
 * not copied.
 */
	static OSSetPtr withArray(
		const OSArray * array,
		unsigned int    capacity = 0);


/*!
 * @function withSet
 *
 * @abstract
 * Creates and initializes an OSSet
 * populated with the contents of another OSSet.
 *
 * @param set       An OSSet whose contents will be stored
 *                  in the new instance.
 * @param capacity  The initial storage capacity of the set object.
 *                  If 0, the capacity is set to the number of objects
 *                  in <code>set</code>;
 *                  otherwise <code>capacity</code> must be greater than or equal to
 *                  the number of objects in <code>array</code>.
 * @result
 * An instance of OSArray
 * containing the objects of <code>set</code>,
 * with a retain count of 1;
 * <code>NULL</code> on failure.
 *
 * @discussion
 * <code>set</code> must be non-<code>NULL</code>.
 * If <code>capacity</code> is nonzero,
 * it must be greater than or equal to <code>count</code>.
 * The array will grow as needed to accommodate more key-object pairs
 * (<i>unlike</i> @link //apple_ref/doc/uid/20001503 CFMutableSet@/link,
 * for which the initial capacity is a hard limit).
 *
 * The objects in <code>set</code> are retained for storage in the new set,
 * not copied.
 */
	static OSSetPtr withSet(const OSSet * set,
	    unsigned int capacity = 0);


/*!
 * @function initWithCapacity
 *
 * @abstract
 * Initializes a new instance of OSSet.
 *
 * @param capacity  The initial storage capacity of the new set object.
 *
 * @result
 * <code>true</code> on success, <code>false</code> on failure.
 *
 * @discussion
 * Not for general use. Use the static instance creation method
 * <code>@link
 * //apple_ref/cpp/clm/OSSet/withCapacity/staticOSSet*\/(unsignedint)
 * withCapacity@/link</code>
 * instead.
 *
 * <code>capacity</code> must be nonzero.
 * The new set will grow as needed to accommodate more key/object pairs
 * (<i>unlike</i> @link //apple_ref/doc/uid/20001503 CFMutableSet@/link,
 * for which the initial capacity is a hard limit).
 */
	virtual bool initWithCapacity(unsigned int capacity);


/*!
 * @function initWithObjects
 *
 * @abstract
 * Initializes a new OSSet populated with objects provided.
 *
 * @param objects   A C array of OSObject-derived objects.
 * @param count     The number of objects to be placed into the set.
 * @param capacity  The initial storage capacity of the new set object.
 *                  If 0, <code>count</code> is used; otherwise this value
 *                  must be greater than or equal to <code>count</code>.
 *
 * @result
 * <code>true</code> on success, <code>false</code> on failure.
 *
 * @discussion
 * Not for general use. Use the static instance creation method
 * <code>@link
 * //apple_ref/cpp/clm/OSSet/withObjects/staticOSSet*\/(constOSObject*,unsignedint,unsignedint)
 * withObjects@/link</code>
 * instead.
 *
 * <code>objects</code> must be non-<code>NULL</code>,
 * and <code>count</code> must be nonzero.
 * If <code>capacity</code> is nonzero, it must be greater than or equal to <code>count</code>.
 * The new array will grow as needed to accommodate more key-object pairs
 * (<i>unlike</i> @link //apple_ref/doc/uid/20001503 CFMutableSet@/link,
 * for which the initial capacity is a hard limit).
 *
 * The objects in <code>objects</code> are retained for storage in the new set,
 * not copied.
 */
	virtual bool initWithObjects(
		const OSObject * objects[],
		unsigned int     count,
		unsigned int     capacity = 0);


/*!
 * @function initWithArray
 *
 * @abstract Initializes a new OSSet
 *           populated with the contents of an OSArray.
 *
 * @param array     An OSAray whose contents will be placed
 *                  in the new instance.
 * @param capacity  The initial storage capacity of the new set object.
 *                  If 0, the capacity is set
 *                  to the number of objects in <code>array</code>;
 *                  otherwise <code>capacity</code> must be greater than or equal to
 *                  the number of objects in <code>array</code>.
 *
 * @result
 * <code>true</code> on success, <code>false</code> on failure.
 *
 * @discussion
 * Not for general use. Use the static instance creation method
 * <code>@link
 * //apple_ref/cpp/clm/OSSet/withArray/staticOSSet*\/(constOSArray*,unsignedint)
 * withArray@/link</code>
 * instead.
 *
 * <code>array</code> must be non-<code>NULL</code>.
 * If <code>capacity</code> is nonzero,
 * it must be greater than or equal to <code>count</code>.
 * The new array will grow as needed to accommodate more key-object pairs
 * (<i>unlike</i> @link //apple_ref/doc/uid/20001503 CFMutableSet@/link,
 * for which the initial capacity is a hard limit).
 *
 * The objects in <code>array</code> are retained for storage in the new set,
 * not copied.
 */
	virtual bool initWithArray(
		const OSArray * array,
		unsigned int capacity = 0);


/*!
 * @function initWithSet
 *
 * @abstract
 * Initializes a new OSSet
 * populated with the contents of another OSSet.
 *
 * @param set       A set whose contents will be placed in the new instance.
 * @param capacity  The initial storage capacity of the new set object.
 *                  If 0, the capacity is set
 *                  to the number of objects in <code>set</code>;
 *                  otherwise <code>capacity</code> must be greater than or equal to
 *                  the number of objects in <code>set</code>.
 *
 * @result
 * <code>true</code> on success, <code>false</code> on failure.
 *
 * @discussion
 * Not for general use. Use the static instance creation method
 * <code>@link withSet withSet@/link</code> instead.
 *
 * <code>set</code> must be non-<code>NULL</code>.
 * If <code>capacity</code> is nonzero,
 * it must be greater than or equal to <code>count</code>.
 * The new set will grow as needed to accommodate more key-object pairs
 * (<i>unlike</i> @link //apple_ref/doc/uid/20001503 CFMutableSet@/link,
 * for which the initial capacity is a hard limit).
 *
 * The objects in <code>set</code> are retained for storage in the new set,
 * not copied.
 */
	virtual bool initWithSet(const OSSet *set,
	    unsigned int capacity = 0);


/*!
 * @function free
 *
 * @abstract
 * Deallocates or releases any resources
 * used by the OSSet instance.
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
 * Returns the current number of objects within the set.
 *
 * @result
 * The current number of objects within the set.
 */
	virtual unsigned int getCount() const APPLE_KEXT_OVERRIDE;


/*!
 * @function getCapacity
 *
 * @abstract
 * Returns the number of objects the set
 * can store without reallocating.
 *
 * @result
 * The number objects the set
 * can store without reallocating.
 *
 * @discussion
 * OSSet objects grow when full to accommodate additional objects.
 * See
 * <code>@link
 * //apple_ref/cpp/instm/OSSet/getCapacityIncrement/virtualunsignedint/()
 * getCapacityIncrement@/link</code>
 * and
 * <code>@link
 * //apple_ref/cpp/instm/OSSet/ensureCapacity/virtualunsignedint/(unsignedint)
 * ensureCapacity@/link</code>.
 */
	virtual unsigned int getCapacity() const APPLE_KEXT_OVERRIDE;


/*!
 * @function getCapacityIncrement
 *
 * @abstract
 * Returns the storage increment of the set.
 *
 * @result
 * The storage increment of the set.
 *
 * @discussion
 * An OSSet allocates storage for objects in multiples
 * of the capacity increment.
 */
	virtual unsigned int getCapacityIncrement() const APPLE_KEXT_OVERRIDE;


/*!
 * @function setCapacityIncrement
 *
 * @abstract
 * Sets the storage increment of the set.
 *
 * @result
 * The new storage increment of the set,
 * which may be different from the number requested.
 *
 * @discussion
 * An OSSet allocates storage for objects in multiples
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
 * @param newCapacity  The total number of distinct objects the set
 *                     should be able to store.
 * @result
 * The new capacity of the set,
 * which may be different from the number requested
 * (if smaller, reallocation of storage failed).
 *
 * @discussion
 * This function immediately resizes the set, if necessary,
 * to accommodate at least <code>newCapacity</code> distinct objects.
 * If <code>newCapacity</code> is not greater than the current capacity,
 * or if an allocation error occurs, the original capacity is returned.
 *
 * There is no way to reduce the capacity of an OSSet.
 */
	virtual unsigned int ensureCapacity(unsigned int newCapacity) APPLE_KEXT_OVERRIDE;


/*!
 * @function flushCollection
 *
 * @abstract
 * Removes and releases all objects within the set.
 *
 * @discussion
 * The set's capacity (and therefore direct memory consumption)
 * is not reduced by this function.
 */
	virtual void flushCollection() APPLE_KEXT_OVERRIDE;


/*!
 * @function setObject
 *
 * @abstract
 * Adds an object to the OSSet if it is not already present.
 *
 * @param anObject  The OSMetaClassBase-derived object to be added to the set.
 *
 * @result
 * <code>true</code> if <code>anObject</code> was successfully
 * added to the set, <code>false</code> otherwise
 * (including if it was already in the set).
 *
 * @discussion
 * The set adds storage to accomodate the new object, if necessary.
 * If successfully added, the object is retained.
 *
 * A <code>false</code> return value can mean either
 * that <code>anObject</code> is already present in the set,
 * or that a memory allocation failure occurred.
 * If you need to know whether the object
 * is already present, use
 * <code>@link containsObject containsObject@/link</code>.
 */
	virtual bool setObject(const OSMetaClassBase * anObject);


/*!
 * @function merge
 *
 * @abstract
 * Adds the contents of an OSArray to the set.
 *
 * @param array  The OSArray object containing the objects to be added.
 *
 * @result
 * <code>true</code> if all objects from <code>array</code>
 * are successfully added the receiver (or were already present),
 * <code>false</code> otherwise.
 *
 * @discussion
 * This functions adds to the receiving set
 * all objects from <code>array</code>
 * that are not already in the receiving set.
 * Objects added to the receiver are retained.
 *
 * In  releases prior to 10.7, this function would return <code>false</code>
 * if an object from <code>array</code> was already present in the set,
 * or if <code>array</code> was empty.
 * This is no longer the case, so this function correctly returns <code>true</code>
 * when the semantic of merging is met.
 */
	virtual bool merge(const OSArray * array);


/*!
 * @function merge
 *
 * @abstract
 * Adds the contents of an OSet to the set.
 *
 * @param set    The OSSet object containing the objects to be added.
 *
 * @result
 * <code>true</code> if any object from <code>set</code>
 * are successfully added the receiver (or were already present),
 * <code>false</code> otherwise.
 *
 * @discussion
 * This functions adds to the receiving set
 * all objects from <code>set</code>
 * that are not already in the receiving set.
 * Objects  added to the receiver are retained.
 *
 * In  releases prior to 10.7, this function would return <code>false</code>
 * if an object from <code>set</code> was already present in the set,
 * or if <code>set</code> was empty.
 * This is no longer the case, so this function correctly returns <code>true</code>
 * when the semantic of merging is met.
 */
	virtual bool merge(const OSSet * set);


/*!
 * @function removeObject
 *
 * @abstract
 * Removes an object from the set.
 *
 * @param anObject  The OSMetaClassBase-derived object
 *                  to be removed from the set.
 *
 * @discussion
 * The object removed from the set is released.
 */
	virtual void removeObject(const OSMetaClassBase * anObject);


/*!
 * @function containsObject
 *
 * @abstract
 * Checks the set for the presence of an object.
 *
 * @param anObject  The OSMetaClassBase-derived object
 *                  to check for in the set.
 *
 * @result
 * <code>true</code> if <code>anObject</code> is present within the set,
 * <code>false</code> otherwise.
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
 * Checks the set for the presence of an object.
 *
 * @param anObject  The OSMetaClassBase-derived object
 *                  to check for in the set.
 *
 * @result
 * <code>true</code> if <code>anObject</code> is present
 * within the set, <code>false</code> otherwise.
 *
 * @discussion
 * Pointer equality is used. This function returns <code>false</code>
 * if passed <code>NULL</code>.
 *
 * <code>@link containsObject containsObject@/link</code>
 * checks for <code>NULL</code> first,
 * and is therefore more efficient than this function.
 */
	virtual bool member(const OSMetaClassBase * anObject) const;


/*!
 * @function getAnyObject
 *
 * @abstract
 * Returns an arbitrary (not random) object from the set.
 *
 * @result
 * An arbitrary (not random) object
 * if one exists within the set.
 *
 * @discussion
 * The returned object will be released if removed from the set;
 * if you plan to store the reference, you should call
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/retain/virtualvoid/()
 * retain@/link</code>
 * on that object.
 */
	virtual OSObject * getAnyObject() const;


/*!
 * @function isEqualTo
 *
 * @abstract
 * Tests the equality of two OSSet objects.
 *
 * @param aSet  The set object being compared against the receiver.
 * @result
 * <code>true</code> if the two sets are equivalent,
 * <code>false</code> otherwise.
 *
 * @discussion
 * Two OSSet objects are considered equal if they have same count
 * and the same object pointer values.
 */
	virtual bool isEqualTo(const OSSet * aSet) const;


/*!
 * @function isEqualTo
 *
 * @abstract
 * Tests the equality of an OSSet against an arbitrary object.
 *
 * @param anObject  The object being compared against the receiver.
 * @result
 * <code>true</code> if the two objects are equivalent,
 * <code>false</code> otherwise.
 *
 * @discussion
 * An OSSet object is considered equal to another object if the other object
 * is derived from OSSet and compares equal as a set.
 */
	virtual bool isEqualTo(const OSMetaClassBase * anObject) const APPLE_KEXT_OVERRIDE;


/*!
 * @function serialize
 *
 * @abstract
 * Archives the receiver into the provided
 * @link //apple_ref/doc/class/OSSerialize OSSerialize@/link object.
 *
 * @param serializer The OSSerialize object.
 *
 * @result
 * <code>true</code> if serialization succeeds, <code>false</code> if not.
 */
	virtual bool serialize(OSSerialize * serializer) const APPLE_KEXT_OVERRIDE;


/*!
 * @function setOptions
 *
 * @abstract
 * Recursively sets option bits in the set
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
 * Child collections' options are changed only if the receiving set's
 * options actually change.
 */
	virtual unsigned setOptions(unsigned options, unsigned mask, void * context = NULL) APPLE_KEXT_OVERRIDE;


/*!
 * @function copyCollection
 *
 * @abstract
 * Creates a deep copy of this set and its child collections.
 *
 * @param cycleDict  A dictionary of all of the collections
 *                   that have been copied so far,
 *                   which is used to track circular references.
 *                   To start the copy at the top level,
 *                   pass <code>NULL</code>.
 *
 * @result
 * The newly copied set, with a retain count of 1,
 * or <code>NULL</code> if there is insufficient memory to do the copy.
 *
 * @discussion
 * The receiving set, and any collections it contains,
 * recursively, are copied.
 * Objects that are not derived from OSCollection are retained
 * rather than copied.
 */
	OSCollectionPtr copyCollection(OSDictionary *cycleDict = NULL) APPLE_KEXT_OVERRIDE;

	OSMetaClassDeclareReservedUnused(OSSet, 0);
	OSMetaClassDeclareReservedUnused(OSSet, 1);
	OSMetaClassDeclareReservedUnused(OSSet, 2);
	OSMetaClassDeclareReservedUnused(OSSet, 3);
	OSMetaClassDeclareReservedUnused(OSSet, 4);
	OSMetaClassDeclareReservedUnused(OSSet, 5);
	OSMetaClassDeclareReservedUnused(OSSet, 6);
	OSMetaClassDeclareReservedUnused(OSSet, 7);
};

#endif /* !_OS_OSSET_H */
