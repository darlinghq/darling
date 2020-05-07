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
/* IOArray.h created by rsulack on Thu 11-Sep-1997 */
/* IOArray.h converted to C++ by gvdl on Fri 1998-10-30 */

#ifndef _OS_OSARRAY_H
#define _OS_OSARRAY_H

#include <libkern/c++/OSCollection.h>
#include <libkern/c++/OSPtr.h>

class OSSerialize;
class OSArray;

typedef OSPtr<OSArray> OSArrayPtr;

/*!
 * @header
 *
 * @abstract
 * This header declares the OSArray collection class.
 */


/*!
 * @class OSArray
 *
 * @abstract
 * OSArray provides an indexed store of objects.
 *
 * @discussion
 * OSArray is a container for Libkern C++ objects
 * (those derived from
 * @link //apple_ref/doc/class/OSMetaClassBase OSMetaClassBase@/link,
 * in particular
 * @link //apple_ref/doc/class/OSObject OSObject@/link).
 * Storage and access are by array index.
 *
 * You must generally cast retrieved objects from
 * @link //apple_ref/cpp/cl/OSObject OSObject@/link
 * to the desired class using
 * <code>@link //apple_ref/cpp/macro/OSDynamicCast OSDynamicCast@/link</code>.
 * This macro returns the object cast to the desired class,
 * or <code>NULL</code> if the object isn't derived from that class.
 *
 * As with all Libkern collection classes,
 * OSArray retains objects added to it,
 * and releases objects removed from it (or replaced).
 * An OSArray also grows as necessary to accommodate new objects,
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
 * OSArray provides no concurrency protection;
 * it's up to the usage context to provide any protection necessary.
 * Some portions of the I/O Kit, such as
 * @link //apple_ref/doc/class/IORegistryEntry IORegistryEntry@/link,
 * handle synchronization via defined member functions for setting
 * properties.
 */
class OSArray : public OSCollection
{
	friend class OSSet;
	friend class OSSerialize;

	OSDeclareDefaultStructors(OSArray);

#if APPLE_KEXT_ALIGN_CONTAINERS

protected:
	unsigned int             count;
	unsigned int             capacity;
	unsigned int             capacityIncrement;
	OSCollectionTaggedPtr<const OSMetaClassBase> *array;

#else /* APPLE_KEXT_ALIGN_CONTAINERS */

protected:
	OSCollectionTaggedPtr<const OSMetaClassBase> *array;
	unsigned int             count;
	unsigned int             capacity;
	unsigned int             capacityIncrement;

	struct ExpansionData { };

/* Reserved for future use. (Internal use only) */
	ExpansionData          * reserved;

#endif /* APPLE_KEXT_ALIGN_CONTAINERS */

/* OSCollectionIterator interfaces. */
	virtual unsigned int iteratorSize() const APPLE_KEXT_OVERRIDE;
	virtual bool initIterator(void * iterator) const APPLE_KEXT_OVERRIDE;
	virtual bool getNextObjectForIterator(void * iterator, OSObject ** ret) const APPLE_KEXT_OVERRIDE;

public:

/*!
 * @function withCapacity
 *
 * @abstract
 * Creates and initializes an empty OSArray.
 *
 * @param  capacity  The initial storage capacity of the array object.
 *
 * @result
 * An empty instance of OSArray with a retain count of 1;
 * <code>NULL</code> on failure.
 *
 * @discussion
 * <code>capacity</code> must be nonzero.
 * The new array will grow as needed to accommodate more objects
 * (<i>unlike</i> @link //apple_ref/doc/uid/20001502 CFMutableArray@/link,
 * for which the initial capacity is a hard limit).
 */
	static OSArrayPtr withCapacity(unsigned int capacity);


/*!
 * @function withObjects
 *
 * @abstract
 * Creates and initializes an OSArray populated with objects provided.
 *
 * @param objects   A C array of OSObject-derived instances.
 * @param count     The number of objects to be placed into the array.
 * @param capacity  The initial storage capacity of the array object.
 *                  If 0, <code>count</code> is used; otherwise this value
 *                  must be greater than or equal to <code>count</code>.
 *
 * @result
 * An instance of OSArray containing the objects provided,
 * with a retain count of 1;
 * <code>NULL</code> on failure.
 *
 * @discussion
 * <code>objects</code> must be non-<code>NULL</code>, and <code>count</code> must be nonzero.
 * If <code>capacity</code> is nonzero,
 * it must be greater than or equal to <code>count</code>.
 * The new array will grow as needed to accommodate more objects
 * (<i>unlike</i> @link //apple_ref/doc/uid/20001502 CFMutableArray@/link,
 * for which the initial capacity is a hard limit).
 */
	static OSArrayPtr withObjects(
		const OSObject * objects[],
		unsigned int     count,
		unsigned int     capacity = 0);


/*!
 * @function withArray
 *
 * @abstract
 * Creates and initializes an OSArray populated with the contents of another array.
 *
 * @param array     An OSArray whose contents will be stored
 *                  in the new instance.
 * @param capacity  The initial storage capacity of the array object.
 *                  If 0, the capacity is set to the number of objects
 *                  in <code>array</code>;
 *                  otherwise <code>capacity</code> must be
 *                  greater than or equal to the number of objects
 *                  in <code>array</code>.
 *
 * @result
 * An instance of OSArray containing the objects of <code>array</code>,
 * with a retain count of 1;
 * <code>NULL</code> on failure.
 *
 * @discussion
 * <code>array</code> must be non-<code>NULL</code>.
 * If <code>capacity</code> is nonzero,
 * it must be greater than or equal to <code>count</code>.
 * The new array will grow as needed to accommodate more objects
 * (<i>unlike</i> @link //apple_ref/doc/uid/20001502 CFMutableArray@/link,
 * for which the initial capacity is a hard limit).
 *
 * The objects in <code>array</code> are retained
 * for storage in the new OSArray,
 * not copied.
 */
	static OSArrayPtr withArray(
		const OSArray * array,
		unsigned int    capacity = 0);


/*!
 * @function initWithCapacity
 *
 * @abstract
 * Initializes a new instance of OSArray.
 *
 * @param capacity  The initial storage capacity of the array object.
 *
 * @result
 * <code>true</code> on success, <code>false</code> on failure.
 *
 * @discussion
 * Not for general use. Use the static instance creation method
 * <code>@link //apple_ref/cpp/clm/OSArray/withCapacity/staticOSArray*\/(unsignedint)
 * withCapacity@/link</code>
 * instead.
 *
 * <code>capacity</code> must be nonzero.
 * The new array will grow as needed to accommodate more objects
 * (<i>unlike</i> @link //apple_ref/doc/uid/20001502 CFMutableArray@/link,
 * for which the initial capacity is a hard limit).
 */
	virtual bool initWithCapacity(unsigned int capacity);


/*!
 * @function initWithObjects
 *
 * @abstract
 * Initializes a new OSArray populated with objects provided.
 *
 * @param objects   A C array of OSObject-derived objects.
 * @param count     The number of objects to be placed into the array.
 * @param capacity  The initial storage capacity of the array object.
 *                  If 0, <code>count</code> is used; otherwise this value
 *                  must be greater than or equal to <code>count</code>.
 *
 * @result
 * <code>true</code> on success, <code>false</code> on failure.
 *
 * @discussion
 * Not for general use. Use the static instance creation method
 * <code>@link
 * //apple_ref/cpp/clm/OSArray/withObjects/staticOSArray*\/(constOSObject*,unsignedint,unsignedint)
 * withObjects@/link</code>
 * instead.
 *
 * <code>objects</code> must be non-<code>NULL</code>,
 * and <code>count</code> must be nonzero.
 * If <code>capacity</code> is nonzero,
 * it must be greater than or equal to <code>count</code>.
 * The new array will grow as needed to accommodate more objects
 * (<i>unlike</i> @link //apple_ref/doc/uid/20001502 CFMutableArray@/link,
 * for which the initial capacity is a hard limit).
 */
	virtual bool initWithObjects(
		const OSObject * objects[],
		unsigned int     count,
		unsigned int     capacity = 0);

/*!
 * @function initWithArray
 *
 * @abstract
 * Initializes a new OSArray populated with the contents of another array.
 *
 * @param anArray  The array whose contents will be placed
 *                 in the new instance.
 * @param capacity The initial storage capacity of the array object.
 *                 If 0, the capacity is set to the number of objects
 *                 in <code>array</code>;
 *                 otherwise <code>capacity</code> must be
 *                 greater than or equal to the number of objects
 *                 in <code>array</code>.
 *
 * @result
 * <code>true</code> on success, <code>false</code> on failure.
 *
 * @discussion
 * Not for general use. Use the static instance creation method
 * <code>@link //apple_ref/cpp/clm/OSArray/withArray/staticOSArray*\/(constOSArray*,unsignedint)
 * withArray@/link</code> instead.
 *
 * <code>array</code> must be non-<code>NULL</code>.
 * If <code>capacity</code> is nonzero,
 * it must be greater than or equal to <code>count</code>.
 * The new array will grow as needed to accommodate more objects
 * (<i>unlike</i> @link //apple_ref/doc/uid/20001502 CFMutableArray@/link,
 * for which the initial capacity is a hard limit).
 *
 * The objects in <code>array</code> are retained for storage in the new OSArray,
 * not copied.
 */
	virtual bool initWithArray(
		const OSArray * anArray,
		unsigned int    capacity = 0);


/*!
 * @function free
 *
 * @abstract
 * Deallocates or releases any resources
 * used by the OSArray instance.
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
 * Returns the current number of objects within the array.
 *
 * @result
 * The current number of objects within the array.
 */
	virtual unsigned int getCount() const APPLE_KEXT_OVERRIDE;


/*!
 * @function getCapacity
 *
 * @abstract
 * Returns the number of objects the array can store
 * without reallocating.
 *
 * @result
 * The number objects the array can store
 * without reallocating.
 *
 * @discussion
 * OSArray objects grow when full to accommodate additional objects.
 * See
 * <code>@link
 * //apple_ref/cpp/instm/OSArray/getCapacity/virtualunsignedint/()
 * getCapacityIncrement@/link</code>
 * and
 * @link
 * //apple_ref/cpp/instm/OSArray/ensureCapacity/virtualunsignedint/(unsignedint)
 * <code>ensureCapacity</code>.@/link
 */
	virtual unsigned int getCapacity() const APPLE_KEXT_OVERRIDE;


/*!
 * @function getCapacityIncrement
 *
 * @abstract
 * Returns the storage increment of the array.
 *
 * @result
 * The storage increment of the array.
 *
 * @discussion
 * An OSArray allocates storage for objects in multiples
 * of the capacity increment.
 */
	virtual unsigned int getCapacityIncrement() const APPLE_KEXT_OVERRIDE;


/*!
 * @function setCapacityIncrement
 *
 * @abstract
 * Sets the storage increment of the array.
 *
 * @result
 * The new storage increment of the array,
 * which may be different from the number requested.
 *
 * @discussion
 * An OSArray allocates storage for objects in multiples
 * of the capacity increment.
 * Calling this function does not immediately reallocate storage.
 */
	virtual unsigned int setCapacityIncrement(unsigned increment) APPLE_KEXT_OVERRIDE;


/*!
 * @function ensureCapacity
 *
 * @abstract
 * Ensures the array has enough space
 * to store the requested number of objects.
 *
 * @param newCapacity  The total number of objects the array
 *                     should be able to store.
 *
 * @result
 * The new capacity of the array,
 * which may be different from the number requested
 * (if smaller, reallocation of storage failed).
 *
 * @discussion
 * This function immediately resizes the array, if necessary,
 * to accommodate at least <code>newCapacity</code> objects.
 * If <code>newCapacity</code> is not greater than the current capacity,
 * or if an allocation error occurs, the original capacity is returned.
 *
 * There is no way to reduce the capacity of an OSArray.
 */
	virtual unsigned int ensureCapacity(unsigned int newCapacity) APPLE_KEXT_OVERRIDE;


/*!
 * @function flushCollection
 *
 * @abstract
 * Removes and releases all objects within the array.
 *
 * @discussion
 * The array's capacity (and therefore direct memory consumption)
 * is not reduced by this function.
 */
	virtual void flushCollection() APPLE_KEXT_OVERRIDE;


/*!
 * @function setObject
 *
 * @abstract
 * Appends an object onto the end of the array,
 * increasing storage if necessary.
 *
 * @param anObject  The object to add to the OSArray instance.
 *
 * @result
 * <code>true</code> if the addition of <code>anObject</code> was successful,
 * <code>false</code> if not.
 *
 * @discussion
 * The array adds storage to accomodate the new object, if necessary.
 * If successfully added, the object is retained.
 */
	virtual bool setObject(const OSMetaClassBase * anObject);


/*!
 * @function setObject
 *
 * @abstract
 * Inserts or appends an object into the array
 * at a particular index.
 *
 * @param index     The index in the array at which to insert the object.
 *                  Must be less than or equal to the array's count.
 * @param anObject  The object to add to the array.
 *
 * @result
 * <code>true</code> if the addition of <code>anObject</code>
 * was successful, <code>false</code> if not.
 *
 * @discussion
 * This function moves existing objects from <code>index</code> on,
 * in order to accommodate the new object;
 * it does not replace an existing object at <code>index</code>. See
 * <code>@link
 * //apple_ref/cpp/instm/OSArray/replaceObject/virtualvoid/(unsignedint,constOSMetaClassBase*)
 * replaceObject@/link</code>.
 * If successfully added, the object is retained.
 *
 * The array adds storage to accomodate the new object, if necessary.
 * Note, however, that this function does not allow for arbirtrary growth
 * of an array by specifying an index larger than the current count.
 * If you need to immediately grow an array by an arbitrary amount,
 * use
 * <code>@link
 * //apple_ref/cpp/instm/OSArray/ensureCapacity/virtualunsignedint/(unsignedint)
 * ensureCapacity@/link</code>.
 */
	virtual bool setObject(
		unsigned int            index,
		const OSMetaClassBase * anObject);


/*!
 * @function merge
 *
 * @abstract
 * Appends the contents of an array onto the receiving array.
 *
 * @param  otherArray  The array whose contents will be appended
 *                     to the receiving array.
 * @result
 * <code>true</code> if merging was successful, <code>false</code> otherwise.
 *
 * @discussion
 * This function merely appends one array onto another.
 * Duplicates are not avoided and no sorting is performed.
 * Objects successfully added to the receiver are retained.
 */
	virtual bool merge(const OSArray * otherArray);


/*!
 * @function replaceObject
 *
 * @abstract
 * Replaces an object in an array at a given index.
 *
 * @param index     The index of the object to be replaced.
 *                  Must be less than the array's count.
 * @param anObject  The object to be placed into the array.
 *
 * @discussion
 * The original object is released and the new object is retained.
 */
	virtual void replaceObject(
		unsigned int            index,
		const OSMetaClassBase * anObject);


/*!
 * @function removeObject
 *
 * @abstract
 * Removes an object from the array.
 *
 * @param index  The index of the object to be removed.
 *
 * @discussion
 * This function moves existing objects to fill the vacated index
 * so that there are no gaps.
 * The object removed is released.
 */
	virtual void removeObject(unsigned int index);


/*!
 * @function isEqualTo
 *
 * @abstract
 * Tests the equality of two OSArray objects.
 *
 * @param anArray  The array object being compared against the receiver.
 *
 * @result
 * <code>true</code> if the two arrays are equivalent,
 *<code>false</code> otherwise.
 *
 * @discussion
 * Two OSArray objects are considered equal if they have same count
 * and if the objects at corresponding indices compare as equal using
 * <code>@link
 * //apple_ref/cpp/instm/OSMetaClassBase/isEqualTo/virtualbool/(constOSMetaClassBase*)
 * isEqualTo@/link</code>.
 */
	virtual bool isEqualTo(const OSArray * anArray) const;


/*!
 * @function isEqualTo
 *
 * @abstract
 * Tests the equality of an OSArray to an arbitrary object.
 *
 * @param anObject  The object to be compared against the receiver.
 *
 * @result
 * <code>true</code> if the two objects are equivalent,
 * <code>false</code> otherwise.
 *
 * @discussion
 * An OSArray is considered equal to another object
 * if that object is derived from OSArray
 * and contains the same or equivalent objects.
 */
	virtual bool isEqualTo(const OSMetaClassBase * anObject) const APPLE_KEXT_OVERRIDE;


/*!
 * @function getObject
 *
 * @abstract
 * Return the object stored at a given index.
 *
 * @param index The index of the object to be returned to caller.
 *
 * @result
 * The object stored at <code>index</code>,
 * or <code>NULL</code> if <code>index</code> lies past the end of the array.
 *
 * @discussion
 * The returned object will be released if removed from the array;
 * if you plan to store the reference, you should call
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/retain/virtualvoid/()
 * retain@/link</code>
 * on that object.
 */
	virtual OSObject * getObject(unsigned int index) const;


/*!
 * @function getLastObject
 *
 * @abstract
 * Returns the last object in the array.
 *
 * @result
 * The last object in the array,
 * or <code>NULL</code> if the array is empty.
 *
 * @discussion
 * The returned object will be released if removed from the array;
 * if you plan to store the reference, you should call
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/retain/virtualvoid/()
 * retain@/link</code>
 * on that object.
 */
	virtual OSObject * getLastObject() const;


/*!
 * @function getNextIndexOfObject
 *
 * @abstract
 * Scans the array for the next instance of a specific object
 * at or beyond a given index.
 *
 * @param anObject  The object to scan for.
 * @param index     The index at which to begin the scan.
 *
 * @result
 * The next index of <code>anObject</code> in the array or (-1)
 * if none is found.
 *
 * @discussion
 * This function uses pointer equivalence, and does not use
 * <code>@link
 * //apple_ref/cpp/instm/OSMetaClassBase/isEqualTo/virtualbool/(constOSMetaClassBase*)
 * isEqualTo@/link</code>.
 */
	virtual unsigned int getNextIndexOfObject(
		const OSMetaClassBase * anObject,
		unsigned int            index) const;

/*!
 * @function serialize
 *
 * @abstract
 * Archives the receiver into the provided
 * @link //apple_ref/doc/class/OSSerialize OSSerialize@/link object.
 *
 * @param serializer  The OSSerialize object.
 * @result
 * <code>true</code> if serialization succeeds, <code>false</code> if not.
 */
	virtual bool serialize(OSSerialize * serializer) const APPLE_KEXT_OVERRIDE;


/*!
 * @function setOptions
 *
 * @abstract
 * Recursively sets option bits in an array
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
 * Child collections' options are changed only if the receiving array's
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
 * Creates a deep copy of an array and its child collections.
 *
 * @param cycleDict  A dictionary of all of the collections
 *                   that have been copied so far,
 *                   which is used to track circular references.
 *                   To start the copy at the top level,
 *                   pass <code>NULL</code>.
 *
 * @result
 * The newly copied array, with a retain count of 1,
 * or <code>NULL</code> if there is insufficient memory to do the copy.
 *
 * @discussion
 * The receiving array, and any collections it contains,
 * recursively, are copied.
 * Objects that are not derived from OSCollection are retained
 * rather than copied.
 */
	OSCollectionPtr copyCollection(OSDictionary * cycleDict = NULL) APPLE_KEXT_OVERRIDE;

	OSMetaClassDeclareReservedUnused(OSArray, 0);
	OSMetaClassDeclareReservedUnused(OSArray, 1);
	OSMetaClassDeclareReservedUnused(OSArray, 2);
	OSMetaClassDeclareReservedUnused(OSArray, 3);
	OSMetaClassDeclareReservedUnused(OSArray, 4);
	OSMetaClassDeclareReservedUnused(OSArray, 5);
	OSMetaClassDeclareReservedUnused(OSArray, 6);
	OSMetaClassDeclareReservedUnused(OSArray, 7);
};

#endif /* !_OS_OSARRAY_H */
