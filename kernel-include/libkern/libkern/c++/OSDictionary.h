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
 * OSDictionary.h created by rsulack on Wed 17-Sep-1997
 * OSDictionary.h converted to C++ by gvdl on Fri 1998-10-30
 */

#ifndef _IOKIT_IODICTIONARY_H
#define _IOKIT_IODICTIONARY_H

#include <libkern/c++/OSCollection.h>
#include <libkern/c++/OSArray.h>
#include <libkern/c++/OSPtr.h>

class OSArray;
class OSSymbol;
class OSString;
class OSDictionary;

typedef OSPtr<OSDictionary> OSDictionaryPtr;

/*!
 * @header
 *
 * @abstract
 * This header declares the OSDictionary collection class.
 */


/*!
 * @class OSDictionary
 *
 * @abstract
 * OSDictionary provides an associative store using strings for keys.
 *
 * @discussion
 * OSDictionary is a container for Libkern C++ objects
 * (those derived from
 * @link //apple_ref/doc/class/OSMetaClassBase OSMetaClassBase@/link,
 * in particular @link //apple_ref/doc/class/OSObject OSObject@/link).
 * Storage and access are associative, based on string-valued keys
 * (C string, @link //apple_ref/cpp/cl/OSString OSString@/link,
 * or @link //apple_ref/cpp/cl/OSSymbol OSSymbol@/link).
 * When adding an object to an OSDictionary, you provide a string identifier,
 * which can then used to retrieve that object or remove it from the dictionary.
 * Setting an object with a key that already has an associated object
 * replaces the original object.
 *
 * You must generally cast retrieved objects from
 * @link //apple_ref/cpp/cl/OSObject OSObject@/link
 * to the desired class using
 * <code>@link //apple_ref/cpp/macro/OSDynamicCast OSDynamicCast@/link</code>.
 * This macro returns the object cast to the desired class,
 * or <code>NULL</code> if the object isn't derived from that class.
 *
 * When iterating an OSDictionary using
 * @link //apple_ref/doc/class/OSCollectionIterator OSCollectionIterator@/link,
 * the objects returned from
 * <code>@link //apple_ref/doc/function/OSCollectionIterator::getNextObject
 * getNextObject@/link</code>
 * are dictionary keys (not the object values for those keys).
 * You can use the keys to retrieve their associated object values.
 *
 * As with all Libkern collection classes,
 * OSDictionary retains keys and objects added to it,
 * and releases keys and objects removed from it (or replaced).
 * An OSDictionary also grows as necessary to accommodate new key/value pairs,
 * <i>unlike</i> Core Foundation collections (it does not, however, shrink).
 *
 * <b>Note:</b> OSDictionary currently uses a linear search algorithm,
 * and is not designed for high-performance access of many values.
 * It is intended as a simple associative-storage mechanism only.
 *
 * <b>Use Restrictions</b>
 *
 * With very few exceptions in the I/O Kit, all Libkern-based C++
 * classes, functions, and macros are <b>unsafe</b>
 * to use in a primary interrupt context.
 * Consult the I/O Kit documentation related to primary interrupts
 * for more information.
 *
 * OSDictionary provides no concurrency protection;
 * it's up to the usage context to provide any protection necessary.
 * Some portions of the I/O Kit, such as
 * @link //apple_ref/doc/class/IORegistryEntry IORegistryEntry@/link,
 * handle synchronization via defined member functions for setting
 * properties.
 */
class OSDictionary : public OSCollection
{
	friend class OSSerialize;

	OSDeclareDefaultStructors(OSDictionary);

#if APPLE_KEXT_ALIGN_CONTAINERS

protected:
	unsigned int   count;
	unsigned int   capacity;
	unsigned int   capacityIncrement;
	struct dictEntry {
		OSCollectionTaggedPtr<const OSSymbol>        key;
		OSCollectionTaggedPtr<const OSMetaClassBase> value;
#if XNU_KERNEL_PRIVATE
		static int compare(const void *, const void *);
#endif
	};
	dictEntry    * dictionary;

#else /* APPLE_KEXT_ALIGN_CONTAINERS */

protected:
	struct dictEntry {
		OSCollectionTaggedPtr<const OSSymbol>        key;
		OSCollectionTaggedPtr<const OSMetaClassBase> value;
#if XNU_KERNEL_PRIVATE
		static int compare(const void *, const void *);
#endif
	};
	dictEntry    * dictionary;
	unsigned int   count;
	unsigned int   capacity;
	unsigned int   capacityIncrement;

	struct ExpansionData { };

/* Reserved for future use.  (Internal use only)  */
	ExpansionData * reserved;

#endif /* APPLE_KEXT_ALIGN_CONTAINERS */

// Member functions used by the OSCollectionIterator class.
	virtual unsigned int iteratorSize() const APPLE_KEXT_OVERRIDE;
	virtual bool initIterator(void * iterator) const APPLE_KEXT_OVERRIDE;
	virtual bool getNextObjectForIterator(void * iterator, OSObject ** ret) const APPLE_KEXT_OVERRIDE;

public:

/*!
 * @function withCapacity
 *
 * @abstract
 * Creates and initializes an empty OSDictionary.
 *
 * @param  capacity  The initial storage capacity of the new dictionary object.
 *
 * @result
 * An empty instance of OSDictionary
 * with a retain count of 1;
 * <code>NULL</code> on failure.
 *
 * @discussion
 * <code>capacity</code> must be nonzero.
 * The new dictionary will grow as needed to accommodate more key/object pairs
 * (<i>unlike</i> @link //apple_ref/doc/uid/20001497 CFMutableDictionary@/link,
 * for which the initial capacity is a hard limit).
 */
	static OSDictionaryPtr withCapacity(unsigned int capacity);


/*!
 * @function withObjects
 *
 * @abstract Creates and initializes an OSDictionary
 *           populated with keys and objects provided.
 *
 * @param objects   A C array of OSMetaClassBase-derived objects.
 * @param keys      A C array of OSSymbol keys
 *                  for the corresponding objects in <code>objects</code>.
 * @param count     The number of keys and objects
 *                  to be placed into the dictionary.
 * @param capacity  The initial storage capacity of the new dictionary object.
 *                  If 0, <code>count</code> is used; otherwise this value
 *                  must be greater than or equal to <code>count</code>.
 *
 * @result
 * An instance of OSDictionary
 * containing the key/object pairs provided,
 * with a retain count of 1;
 * <code>NULL</code> on failure.
 *
 * @discussion
 * <code>objects</code> and <code>keys</code> must be non-<code>NULL</code>,
 * and <code>count</code> must be nonzero.
 * If <code>capacity</code> is nonzero,
 * it must be greater than or equal to <code>count</code>.
 * The new dictionary will grow as needed
 * to accommodate more key/object pairs
 * (<i>unlike</i>
 * @link //apple_ref/doc/uid/20001497 CFMutableDictionary@/link,
 * for which the initial capacity is a hard limit).
 */
	static OSDictionaryPtr withObjects(
		const OSObject * objects[],
		const OSSymbol * keys[],
		unsigned int     count,
		unsigned int     capacity = 0);

/*!
 * @function withObjects
 *
 * @abstract
 * Creates and initializes an OSDictionary
 * populated with keys and objects provided.
 *
 * @param objects   A C array of OSMetaClassBase-derived objects.
 * @param keys      A C array of OSString keys for the corresponding objects
 *                  in <code>objects</code>.
 * @param count     The number of keys and objects
 *                  to be placed into the dictionary.
 * @param capacity  The initial storage capacity of the new dictionary object.
 *                  If 0, <code>count</code> is used; otherwise this value
 *                  must be greater than or equal to <code>count</code>.
 *
 * @result
 * An instance of OSDictionary
 * containing the key/object pairs provided,
 * with a retain count of 1;
 * <code>NULL</code> on failure.
 *
 * @discussion
 * <code>objects</code> and <code>keys</code> must be non-<code>NULL</code>,
 * and <code>count</code> must be nonzero.
 * If <code>capacity</code> is nonzero, it must be greater than or equal to <code>count</code>.
 * The new dictionary will grow as needed
 * to accommodate more key/object pairs
 * (<i>unlike</i>
 * @link //apple_ref/doc/uid/20001497 CFMutableDictionary@/link,
 * for which the initial capacity is a hard limit).
 */
	static OSDictionaryPtr withObjects(
		const OSObject * objects[],
		const OSString * keys[],
		unsigned int     count,
		unsigned int     capacity = 0);


/*!
 * @function withDictionary
 *
 * @abstract
 * Creates and initializes an OSDictionary
 * populated with the contents of another dictionary.
 *
 * @param dict      A dictionary whose contents will be stored
 *                  in the new instance.
 * @param capacity  The initial storage capacity of the new dictionary object.
 *                  If 0, the capacity is set to the number of key/value pairs
 *                  in <code>dict</code>;
 *                  otherwise <code>capacity</code> must be greater than or equal to
 *                  the number of key/value pairs in <code>dict</code>.
 *
 * @result
 * An instance of OSDictionary
 * containing the key/value pairs of <code>dict</code>,
 * with a retain count of 1;
 * <code>NULL</code> on failure.
 *
 * @discussion
 * <code>dict</code> must be non-<code>NULL</code>.
 * If <code>capacity</code> is nonzero, it must be greater than or equal to <code>count</code>.
 * The new dictionary will grow as needed
 * to accommodate more key/object pairs
 * (<i>unlike</i>
 *  @link //apple_ref/doc/uid/20001497 CFMutableDictionary@/link,
 * for which the initial capacity is a hard limit).
 *
 * The keys and objects in <code>dict</code> are retained for storage
 * in the new OSDictionary,
 * not copied.
 */
	static OSDictionaryPtr withDictionary(
		const OSDictionary * dict,
		unsigned int         capacity = 0);


/*!
 * @function initWithCapacity
 *
 * @abstract
 * Initializes a new instance of OSDictionary.
 *
 * @param capacity  The initial storage capacity of the new dictionary object.
 * @result
 * <code>true</code> on success, <code>false</code> on failure.
 *
 * @discussion
 * Not for general use. Use the static instance creation method
 * <code>@link //apple_ref/cpp/clm/OSDictionary/withCapacity/staticOSDictionary*\/(unsignedint)
 * withCapacity@/link</code>
 * instead.
 *
 * <code>capacity</code> must be nonzero.
 * The new dictionary will grow as needed
 * to accommodate more key/object pairs
 * (<i>unlike</i>
 * @link //apple_ref/doc/uid/20001497 CFMutableDictionary@/link,
 * for which the initial capacity is a hard limit).
 */
	virtual bool initWithCapacity(unsigned int capacity);


/*!
 * @function initWithObjects
 *
 * @abstract Initializes a new OSDictionary with keys and objects provided.
 *
 * @param objects   A C array of OSMetaClassBase-derived objects.
 * @param keys      A C array of OSSymbol keys
 *                  for the corresponding objects in <code>objects</code>.
 * @param count     The number of keys and objects to be placed
 *                  into the dictionary.
 * @param capacity  The initial storage capacity of the new dictionary object.
 *                  If 0, <code>count</code> is used; otherwise this value
 *                  must be greater than or equal to <code>count</code>.
 *
 * @result
 * <code>true</code> on success, <code>false</code> on failure.
 *
 * @discussion
 * Not for general use. Use the static instance creation method
 * <code>@link
 * //apple_ref/cpp/clm/OSDictionary/withObjects/staticOSDictionary*\/(constOSObject*,constOSString*,unsignedint,unsignedint)
 * withObjects@/link</code>
 * instead.
 *
 * <code>objects</code> and <code>keys</code> must be non-<code>NULL</code>,
 * and <code>count</code> must be nonzero.
 * If <code>capacity</code> is nonzero,
 * it must be greater than or equal to <code>count</code>.
 * The new dictionary will grow as neede
 * to accommodate more key/object pairs
 * (<i>unlike</i>
 * @link //apple_ref/doc/uid/20001497 CFMutableDictionary@/link,
 * for which the initial capacity is a hard limit).
 */
	virtual bool initWithObjects(
		const OSObject * objects[],
		const OSSymbol * keys[],
		unsigned int     count,
		unsigned int     capacity = 0);


/*!
 * @function initWithObjects
 *
 * @abstract
 * Initializes a new OSDictionary with keys and objects provided.
 *
 * @param objects   A C array of OSMetaClassBase-derived objects.
 * @param keys      A C array of OSString keys
 *                  for the corresponding objects in <code>objects</code>.
 * @param count     The number of keys and objects
 *                  to be placed into the dictionary.
 * @param capacity  The initial storage capacity of the new dictionary object.
 *                  If 0, <code>count</code> is used; otherwise this value
 *                  must be greater than or equal to <code>count</code>.
 *
 * @result
 * <code>true</code> on success, <code>false</code> on failure.
 *
 * @discussion
 * Not for general use. Use the static instance creation method
 * <code>@link
 * //apple_ref/cpp/clm/OSDictionary/withObjects/staticOSDictionary*\/(constOSObject*,constOSString*,unsignedint,unsignedint)
 * withObjects@/link</code>
 * instead.
 *
 * <code>objects</code> and <code>keys</code> must be non-<code>NULL</code>,
 * and <code>count</code> must be nonzero.
 * If <code>capacity</code> is nonzero, it must be greater than or equal to <code>count</code>.
 * The new dictionary will grow as needed
 * to accommodate more key/object pairs
 * (<i>unlike</i>
 * @link //apple_ref/doc/uid/20001497 CFMutableDictionary@/link,
 * for which the initial capacity is a hard limit).
 */
	virtual bool initWithObjects(
		const OSObject * objects[],
		const OSString * keys[],
		unsigned int     count,
		unsigned int     capacity = 0);


/*!
 * @function initWithDictionary
 *
 * @abstract
 * Initializes a new OSDictionary
 * with the contents of another dictionary.
 *
 * @param dict      A dictionary whose contents will be placed
 *                  in the new instance.
 * @param capacity  The initial storage capacity of the new dictionary object.
 *                  If 0, the capacity is set to the number of key/value pairs
 *                  in <code>dict</code>;
 *                  otherwise <code>capacity</code> must be greater than or equal to
 *                  the number of key/value pairs in <code>dict</code>.
 *
 * @result
 * <code>true</code> on success, <code>false</code> on failure.
 *
 * @discussion
 * Not for general use. Use the static instance creation method
 * <code>@link withDictionary withDictionary@/link</code> instead.
 *
 * <code>dict</code> must be non-<code>NULL</code>.
 * If <code>capacity</code> is nonzero,
 * it must be greater than or equal to <code>count</code>.
 * The new dictionary will grow as needed
 * to accommodate more key/object pairs
 * (<i>unlike</i>
 * @link //apple_ref/doc/uid/20001497 CFMutableDictionary@/link,
 * for which the initial capacity is a hard limit).
 *
 * The keys and objects in <code>dict</code> are retained for storage
 * in the new OSDictionary,
 * not copied.
 */
	virtual bool initWithDictionary(
		const OSDictionary * dict,
		unsigned int         capacity = 0);


/*!
 * @function free
 *
 * @abstract
 * Deallocates or releases any resources
 * used by the OSDictionary instance.
 *
 * @discussion
 * This function should not be called directly,
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
 * Returns the current number of key/object pairs
 * contained within the dictionary.
 *
 * @result
 * The current number of key/object pairs
 * contained within the dictionary.
 */
	virtual unsigned int getCount() const APPLE_KEXT_OVERRIDE;


/*!
 * @function getCapacity
 *
 * @abstract
 * Returns the number of objects the dictionary can store without reallocating.
 *
 * @result
 * The number objects the dictionary can store without reallocating.
 *
 * @discussion
 * OSDictionary objects grow when full
 * to accommodate additional key/object pairs.
 * See
 * <code>@link
 * //apple_ref/cpp/instm/OSDictionary/getCapacityIncrement/virtualunsignedint/()
 * getCapacityIncrement@/link</code>
 * and
 * <code>@link
 * //apple_ref/cpp/instm/OSDictionary/ensureCapacity/virtualunsignedint/(unsignedint)
 * ensureCapacity@/link</code>.
 */
	virtual unsigned int getCapacity() const APPLE_KEXT_OVERRIDE;


/*!
 * @function getCapacityIncrement
 *
 * @abstract
 * Returns the storage increment of the dictionary.
 *
 * @result
 * The storage increment of the dictionary.
 *
 * @discussion
 * An OSDictionary allocates storage for key/object pairs in multiples
 * of the capacity increment.
 */
	virtual unsigned int getCapacityIncrement() const APPLE_KEXT_OVERRIDE;


/*!
 * @function setCapacityIncrement
 *
 * @abstract
 * Sets the storage increment of the dictionary.
 *
 * @result
 * The new storage increment of the dictionary,
 * which may be different from the number requested.
 *
 * @discussion
 * An OSDictionary allocates storage for key/object pairs in multiples
 * of the capacity increment.
 * Calling this function does not immediately reallocate storage.
 */
	virtual unsigned int setCapacityIncrement(unsigned increment) APPLE_KEXT_OVERRIDE;


/*!
 * @function ensureCapacity
 *
 * @abstract
 * Ensures the dictionary has enough space
 * to store the requested number of key/object  pairs.
 *
 * @param newCapacity  The total number of key/object pairs the dictionary
 *                     should be able to store.
 *
 * @result
 * The new capacity of the dictionary,
 * which may be different from the number requested
 * (if smaller, reallocation of storage failed).
 *
 * @discussion
 * This function immediately resizes the dictionary, if necessary,
 * to accommodate at least <code>newCapacity</code> key/object pairs.
 * If <code>newCapacity</code> is not greater than the current capacity,
 * or if an allocation error occurs, the original capacity is returned.
 *
 * There is no way to reduce the capacity of an OSDictionary.
 */
	virtual unsigned int ensureCapacity(unsigned int newCapacity) APPLE_KEXT_OVERRIDE;


/*!
 * @function flushCollection
 *
 * @abstract
 * Removes and releases all keys and objects within the dictionary.
 *
 * @discussion
 * The dictionary's capacity (and therefore direct memory consumption)
 * is not reduced by this function.
 */
	virtual void flushCollection() APPLE_KEXT_OVERRIDE;


/*!
 * @function setObject
 *
 * @abstract
 * Stores an object in the dictionary under a key.
 *
 * @param aKey      An OSSymbol identifying the object
 *                  placed within the dictionary.
 *                  It is automatically retained.
 * @param anObject  The object to be stored in the dictionary.
 *                  It is automatically retained.
 *
 * @result
 * <code>true</code> if the addition was successful,
 * <code>false</code> otherwise.
 *
 * @discussion
 * An object already stored under <code>aKey</code> is released.
 */
	virtual bool setObject(
		const OSSymbol        * aKey,
		const OSMetaClassBase * anObject);


/*!
 * @function setObject
 *
 * @abstract Stores an object in the dictionary under a key.
 *
 * @param aKey      An OSString identifying the object
 *                  placed within the dictionary.
 * @param anObject  The object to be stored in the dictionary.
 *                  It is automatically retained.
 *
 * @result
 * <code>true</code> if the addition was successful,
 * <code>false</code> otherwise.
 *
 * @discussion
 * An OSSymbol for <code>aKey</code> is created internally.
 * An object already stored under <code>aKey</code> is released.
 */
	virtual bool setObject(
		const OSString        * aKey,
		const OSMetaClassBase * anObject);


/*!
 * @function setObject
 *
 * @abstract
 * Stores an object in the dictionary under a key.
 *
 * @param aKey      A C string identifying the object
 *                  placed within the dictionary.
 * @param anObject  The object to be stored in the dictionary.
 *                  It is automatically retained.
 *
 * @result
 * <code>true</code> if the addition was successful,
 * <code>false</code> otherwise.
 *
 * @discussion
 * An OSSymbol for <code>aKey</code> is created internally.
 * An object already stored under <code>aKey</code> is released.
 */
	virtual bool setObject(
		const char            * aKey,
		const OSMetaClassBase * anObject);


/*!
 * @function removeObject
 *
 * @abstract
 * Removes a key/object pair from the dictionary.
 *
 * @param aKey  An OSSymbol identifying the object
 *              to be removed from the dictionary.
 *
 * @discussion
 * The removed key (not necessarily <code>aKey</code> itself)
 * and object are automatically released.
 */
	virtual void removeObject(const OSSymbol * aKey);


/*!
 * @function removeObject
 *
 * @abstract
 * Removes a key/object pair from the dictionary.
 *
 * @param aKey  A OSString identifying the object
 *              to be removed from the dictionary.
 *
 * @discussion
 * The removed key (not necessarily <code>aKey</code> itself)
 * and object are automatically released.
 */
	virtual void removeObject(const OSString * aKey);


/*!
 * @function removeObject
 *
 * @abstract
 * Removes a key/object pair from the dictionary.
 *
 * @param aKey  A C string identifying the object
 *              to be removed from the dictionary.
 *
 * @discussion
 * The removed key (internally an OSSymbol)
 * and object are automatically released.
 */
	virtual void removeObject(const char * aKey);


/*!
 * @function merge
 *
 * @abstract
 * Merges the contents of a dictionary into the receiver.
 *
 * @param aDictionary  The dictionary whose contents
 *                     are to be merged with the receiver.
 * @result
 * <code>true</code> if the merge succeeds, <code>false</code> otherwise.
 *
 * @discussion
 * If there are keys in <code>aDictionary</code> that match keys
 * in the receiving dictionary,
 * then the objects in the receiver are replaced
 * by those from <code>aDictionary</code>,
 * and the replaced objects are released.
 */
	virtual bool merge(const OSDictionary * aDictionary);


/*!
 * @function getObject
 *
 * @abstract
 * Returns the object stored under a given key.
 *
 * @param aKey  An OSSymbol key identifying the object
 *              to be returned to the caller.
 *
 * @result
 * The object stored under <code>aKey</code>,
 * or <code>NULL</code> if the key does not exist in the dictionary.
 *
 * @discussion
 * The returned object will be released if removed from the dictionary;
 * if you plan to store the reference, you should call
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/retain/virtualvoid/()
 * retain@/link</code>
 * on that object.
 */
	virtual OSObject * getObject(const OSSymbol * aKey) const;


/*!
 * @function getObject
 *
 * @abstract Returns the object stored under a given key.
 *
 * @param aKey  An OSString key identifying the object
 *              to be returned to caller.
 *
 * @result
 * The object stored under <code>aKey</code>,
 * or <code>NULL</code> if the key does not exist in the dictionary.
 *
 * @discussion
 * The returned object will be released if removed from the dictionary;
 * if you plan to store the reference, you should call
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/retain/virtualvoid/()
 * retain@/link</code>
 * on that object.
 */
	virtual OSObject * getObject(const OSString * aKey) const;


/*!
 * @function getObject
 *
 * @abstract
 * Returns the object stored under a given key.
 *
 * @param aKey  A C string key identifying the object
 *              to be returned to caller.
 *
 * @result
 * The object stored under <code>aKey</code>,
 * or <code>NULL</code> if the key does not exist in the dictionary.
 *
 * @discussion
 * The returned object will be released if removed from the dictionary;
 * if you plan to store the reference, you should call
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/retain/virtualvoid/()
 * retain@/link</code>
 * on that object.
 */
	virtual OSObject * getObject(const char * aKey) const;


/*!
 * @function isEqualTo
 *
 * @abstract Tests the equality of two OSDictionary objects
 * over a subset of keys.
 *
 * @param aDictionary  The dictionary to be compared against the receiver.
 * @param keys         An OSArray or OSDictionary containing the keys
 *                     (as @link //apple_ref/cpp/cl/OSString OSStrings@/link or
 *                     @link //apple_ref/cpp/cl/OSSymbol OSSymbols@/link)
 *                     describing the intersection for the comparison.
 *
 * @result
 * <code>true</code> if the intersections
 * of the two dictionaries are equal.
 *
 * @discussion
 * Two OSDictionary objects are considered equal by this function
 * if both have objects stored for all keys provided,
 * and if the objects stored in each under
 * a given key compare as equal using
 * <code>@link
 * //apple_ref/cpp/instm/OSMetaClassBase/isEqualTo/virtualbool/(constOSMetaClassBase*)
 * isEqualTo@/link</code>.
 */
	virtual bool isEqualTo(
		const OSDictionary * aDictionary,
		const OSCollection * keys) const;


/*!
 * @function isEqualTo
 *
 * @abstract Tests the equality of two OSDictionary objects.
 *
 * @param aDictionary  The dictionary to be compared against the receiver.
 *
 * @result
 * <code>true</code> if the dictionaries are equal,
 * <code>false</code> if not.
 *
 * @discussion
 * Two OSDictionary objects are considered equal if they have same count,
 * the same keys, and if the objects stored in each under
 * a given key compare as equal using
 * <code>@link
 * //apple_ref/cpp/instm/OSMetaClassBase/isEqualTo/virtualbool/(constOSMetaClassBase*)
 * isEqualTo@/link</code>.
 */
	virtual bool isEqualTo(const OSDictionary * aDictionary) const;


/*!
 * @function isEqualTo
 *
 * @abstract
 * Tests the equality of an OSDictionary to an arbitrary object.
 *
 * @param anObject An object to be compared against the receiver.
 *
 * @result
 * <code>true</code> if the objects are equal.
 *
 * @discussion
 * An OSDictionary is considered equal to another object
 * if that object is derived from OSDictionary
 * and contains the same or equivalent objects.
 */
	virtual bool isEqualTo(const OSMetaClassBase * anObject) const APPLE_KEXT_OVERRIDE;


/*!
 * @function serialize
 *
 * @abstract
 * Archives the receiver into the provided
 * @link //apple_ref/doc/class/OSSerialize OSSerialize@/link object.
 *
 * @param serializer  The OSSerialize object.
 *
 * @result
 * <code>true</code> if serialization succeeds, <code>false</code> if not.
 */
	virtual bool serialize(OSSerialize * serializer) const APPLE_KEXT_OVERRIDE;


/*!
 * @function setOptions
 *
 * @abstract
 * Recursively sets option bits in the dictionary
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
 * Child collections' options are changed only if the receiving dictionary's
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
 * Creates a deep copy of the dictionary
 * and its child collections.
 *
 * @param cycleDict  A dictionary of all of the collections
 *                   that have been copied so far,
 *                   which is used to track circular references.
 *                   To start the copy at the top level,
 *                   pass <code>NULL</code>.
 *
 * @result
 * The newly copied dictionary, with a retain count of 1,
 * or <code>NULL</code> if there is insufficient memory to do the copy.
 *
 * @discussion
 * The receiving dictionary, and any collections it contains, recursively,
 * are copied.
 * Objects that are not derived from OSCollection are retained
 * rather than copied.
 */
	OSCollectionPtr copyCollection(OSDictionary * cycleDict = NULL) APPLE_KEXT_OVERRIDE;

#if XNU_KERNEL_PRIVATE
	bool setObject(const OSSymbol *aKey, const OSMetaClassBase *anObject, bool onlyAdd);
	void sortBySymbol(void);
	OSArrayPtr copyKeys(void);
#endif /* XNU_KERNEL_PRIVATE */


/*!
 * @function iterateObjects
 *
 * @abstract
 * Invoke a callback for each member of the collection.
 *
 * @param refcon   A reference constant for the callback.
 * @param callback The callback function,
 *                 called with the refcon and each member key & object
 *                 of the dictionary in turn, on the callers thread.
 *                 The callback should return true to early terminate
 *                 the iteration, false otherwise.
 *
 * @result
 * False if the dictionary iteration was made invalid
 * (see OSCollectionIterator::isValid()) otherwise true.
 */
	bool iterateObjects(void * refcon, bool (*callback)(void * refcon, const OSSymbol * key, OSObject * object));

#ifdef __BLOCKS__

/*!
 * @function iterateObjects
 *
 * @abstract
 * Invoke a block for each member of the collection.
 *
 * @param block    The block,
 *                 called with the refcon and each member key & object
 *                 of the dictionary in turn, on the callers thread.
 *                 The callback should return true to early terminate
 *                 the iteration, false otherwise.
 *
 * @result
 * False if the dictionary iteration was made invalid
 * (see OSCollectionIterator::isValid()) otherwise true.
 */
	bool iterateObjects(bool (^block)(const OSSymbol * key, OSObject * object));

#endif /* __BLOCKS__ */

	OSMetaClassDeclareReservedUnused(OSDictionary, 0);
	OSMetaClassDeclareReservedUnused(OSDictionary, 1);
	OSMetaClassDeclareReservedUnused(OSDictionary, 2);
	OSMetaClassDeclareReservedUnused(OSDictionary, 3);
	OSMetaClassDeclareReservedUnused(OSDictionary, 4);
	OSMetaClassDeclareReservedUnused(OSDictionary, 5);
	OSMetaClassDeclareReservedUnused(OSDictionary, 6);
	OSMetaClassDeclareReservedUnused(OSDictionary, 7);
};

#endif /* !_IOKIT_IODICTIONARY_H */
