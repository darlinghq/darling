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
/* IOData.h created by rsulack on Wed 17-Sep-1997 */
/* IOData.h converted to C++ by gvdl on Fri 1998-10-30 */

#ifndef _OS_OSDATA_H
#define _OS_OSDATA_H

#include <libkern/c++/OSObject.h>
#include <libkern/c++/OSPtr.h>

class OSData;
class OSString;

typedef OSPtr<OSData> OSDataPtr;
typedef OSPtr<const OSData> OSDataConstPtr;

/*!
 * @header
 *
 * @abstract
 * This header declares the OSData container class.
 */


/*!
 * @class OSData
 *
 * @abstract
 * OSData wraps an array of bytes in a C++ object
 * for use in Libkern collections.
 *
 * @discussion
 * OSData represents an array of bytes as a Libkern C++ object.
 * OSData objects are mutable:
 * You can add bytes to them and
 * overwrite portions of the byte array.
 *
 * <b>Use Restrictions</b>
 *
 * With very few exceptions in the I/O Kit, all Libkern-based C++
 * classes, functions, and macros are <b>unsafe</b>
 * to use in a primary interrupt context.
 * Consult the I/O Kit documentation related to primary interrupts
 * for more information.
 *
 * OSData provides no concurrency protection;
 * it's up to the usage context to provide any protection necessary.
 * Some portions of the I/O Kit, such as
 * @link //apple_ref/doc/class/IORegistryEntry IORegistryEntry@/link,
 * handle synchronization via defined member functions for setting
 * properties.
 */
class OSData : public OSObject
{
	friend class OSSerialize;

	OSDeclareDefaultStructors(OSData);

#if APPLE_KEXT_ALIGN_CONTAINERS

protected:
	unsigned int   length;
	unsigned int   capacity;
	unsigned int   capacityIncrement;
	void         * data;

#else /* APPLE_KEXT_ALIGN_CONTAINERS */

protected:
	void         * data;
	unsigned int   length;
	unsigned int   capacity;
	unsigned int   capacityIncrement;

#endif /* APPLE_KEXT_ALIGN_CONTAINERS */

#ifdef XNU_KERNEL_PRIVATE
/* Available within xnu source only */
public:
	typedef void (*DeallocFunction)(void * ptr, unsigned int length);
protected:
	struct ExpansionData {
		DeallocFunction deallocFunction;
		bool            disableSerialization;
	};
#else /* XNU_KERNEL_PRIVATE */
private:
	typedef void (*DeallocFunction)(void * ptr, unsigned int length);
protected:
	struct ExpansionData;
#endif /* XNU_KERNEL_PRIVATE */

/* Reserved for future use. (Internal use only)  */
	ExpansionData * reserved;

public:

/*!
 * @function withCapacity
 *
 * @abstract
 * Creates and initializes an empty instance of OSData.
 *
 * @param capacity  The initial capacity of the OSData object in bytes.
 *
 * @result
 * An instance of OSData with a reference count of 1;
 * <code>NULL</code> on failure.
 *
 * @discussion
 * <code>capacity</code> may be zero.
 * The OSData object will allocate a buffer internally
 * when necessary, and will grow as needed to accommodate more bytes
 * (<i>unlike</i> @link //apple_ref/doc/uid/20001498 CFMutableData@/link,
 * for which a nonzero initial capacity is a hard limit).
 */
	static OSDataPtr withCapacity(unsigned int capacity);


/*!
 * @function withBytes
 *
 * @abstract
 * Creates and initializes an instance of OSData
 * with a copy of the provided data buffer.
 *
 * @param bytes     The buffer of data to copy.
 * @param numBytes  The length of <code>bytes</code>.
 *
 * @result
 * An instance of OSData containing a copy of the provided byte array,
 * with a reference count of 1;
 * <code>NULL</code> on failure.
 *
 * @discussion
 * The new OSData object will grow as needed to accommodate more bytes
 * (<i>unlike</i> @link //apple_ref/doc/uid/20001498 CFMutableData@/link,
 * for which a nonzero initial capacity is a hard limit).
 */
	static OSDataPtr withBytes(
		const void   * bytes,
		unsigned int   numBytes);


/*!
 * @function withBytesNoCopy
 *
 * @abstract
 * Creates and initializes an instance of OSData
 * that shares the provided data buffer.
 *
 * @param bytes     The buffer of data to represent.
 * @param numBytes  The length of <code>bytes</code>.
 *
 * @result
 * A instance of OSData that shares the provided byte array,
 * with a reference count of 1;
 * <code>NULL</code> on failure.
 *
 * @discussion
 * An OSData object created with this function
 * does not claim ownership
 * of the data buffer, but shares it with the caller.
 * When the caller determines that the OSData object has actually been freed,
 * it can safely dispose of the data buffer.
 * Conversely, if it frees the shared data buffer,
 * it must not attempt to use the OSData object and should release it.
 *
 * An OSData object created with shared external data cannot append bytes,
 * but you can get the byte pointer and
 * modify bytes within the shared buffer.
 */
	static OSDataPtr withBytesNoCopy(
		void         * bytes,
		unsigned int   numBytes);


/*!
 * @function withData
 *
 * @abstract
 * Creates and initializes an instance of OSData
 * with contents copied from another OSData object.
 *
 * @param inData An OSData object that provides the initial data.
 *
 * @result
 * An instance of OSData containing a copy of the data in <code>inData</code>,
 * with a reference count of 1;
 * <code>NULL</code> on failure.
 *
 * @discussion
 * The new OSData object will grow as needed to accommodate more bytes
 * (<i>unlike</i> @link //apple_ref/doc/uid/20001498 CFMutableData@/link,
 * for which a nonzero initial capacity is a hard limit).
 */
	static OSDataPtr withData(const OSData * inData);


/*!
 * @function withData
 *
 * @abstract
 * Creates and initializes an instance of OSData
 * with contents copied from a range within another OSData object.
 *
 * @param inData    An OSData object that provides the initial data.
 * @param start     The starting index from which bytes will be copied.
 * @param numBytes  The number of bytes to be copied from <code>start</code>.
 *
 * @result
 * An instance of OSData containing a copy
 * of the specified data range from <code>inData</code>,
 * with a reference count of 1;
 * <code>NULL</code> on failure.
 *
 * @discussion
 * The new OSData object will grow as needed to accommodate more bytes
 * (<i>unlike</i> @link //apple_ref/doc/uid/20001498 CFMutableData@/link,
 * for which a nonzero initial capacity is a hard limit).
 */
	static OSDataPtr withData(
		const OSData * inData,
		unsigned int   start,
		unsigned int   numBytes);


/*!
 * @function initWithCapacity
 *
 * @abstract
 * Initializes an instance of OSData.
 *
 * @param capacity The initial capacity of the OSData object in bytes.
 *
 * @result
 * <code>true</code> on success, <code>false</code> on failure.
 *
 * @discussion
 * Not for general use. Use the static instance creation method
 * <code>@link
 * //apple_ref/cpp/clm/OSData/withCapacity/staticOSData*\/(unsignedint)
 * withCapacity@/link</code> instead.
 *
 * <code>capacity</code> may be zero.
 * The OSData object will allocate a buffer internally
 * when necessary, and will grow as needed to accommodate more bytes
 * (<i>unlike</i> @link //apple_ref/doc/uid/20001498 CFMutableData@/link,
 * for which a nonzero initial capacity is a hard limit).
 */
	virtual bool initWithCapacity(unsigned int capacity);


/*!
 * @function initWithBytes
 *
 * @abstract
 * Initializes an instance of OSData
 * with a copy of the provided data buffer.
 *
 * @param bytes     The buffer of data to copy.
 * @param numBytes  The length of <code>bytes</code>.
 *
 * @result
 * <code>true</code> on success, <code>false</code> on failure.
 *
 * @discussion
 * Not for general use. Use the static instance creation method
 * <code>@link withBytes withBytes@/link</code> instead.
 *
 * The new OSData object will grow as needed to accommodate more bytes
 * (<i>unlike</i> @link //apple_ref/doc/uid/20001498 CFMutableData@/link,
 * for which a nonzero initial capacity is a hard limit).
 */
	virtual bool initWithBytes(
		const void   * bytes,
		unsigned int   numBytes);


/*!
 * @function initWithBytesNoCopy
 *
 * @abstract
 * Initializes an instance of OSData
 * to share the provided data buffer.
 *
 * @param bytes     The buffer of data to represent.
 * @param numBytes  The length of <code>bytes</code>.
 *
 * @result
 * <code>true</code> on success, <code>false</code> on failure.
 *
 * @discussion
 * Not for general use. Use the static instance creation method
 * <code>@link withBytesNoCopy withBytesNoCopy@/link</code> instead.
 *
 * An OSData object initialized with this function
 * does not claim ownership
 * of the data buffer, but merely shares it with the caller.
 *
 * An OSData object created with shared external data cannot append bytes,
 * but you can get the byte pointer and
 * modify bytes within the shared buffer.
 */
	virtual bool initWithBytesNoCopy(
		void         * bytes,
		unsigned int   numBytes);


/*!
 * @function initWithData
 *
 * @abstract
 * Creates and initializes an instance of OSData
 * with contents copied from another OSData object.
 *
 * @param inData An OSData object that provides the initial data.
 *
 * @result
 * <code>true</code> on success, <code>false</code> on failure.
 *
 * @discussion
 * Not for general use. Use the static instance creation method
 * <code>@link
 * //apple_ref/cpp/clm/OSData/withData/staticOSData*\/(constOSData*)
 * withData(OSData *)@/link</code>
 * instead.
 *
 * The new OSData object will grow as needed to accommodate more bytes
 * (<i>unlike</i> @link //apple_ref/doc/uid/20001498 CFMutableData@/link,
 * for which a nonzero initial capacity is a hard limit).
 */
	virtual bool initWithData(const OSData * inData);


/*!
 * @function initWithData
 *
 * @abstract
 * Initializes an instance of OSData
 * with contents copied from a range within another OSData object.
 *
 * @param inData    An OSData object that provides the initial data.
 * @param start     The starting index from which bytes will be copied.
 * @param numBytes  The number of bytes to be copied from <code>start</code>.
 *
 * @result
 * Returns <code>true</code> on success, <code>false</code> on failure.
 *
 * @discussion
 * Not for general use. Use the static instance creation method
 * <code>@link
 * //apple_ref/cpp/clm/OSData/withData/staticOSData*\/(constOSData*,unsignedint,unsignedint)
 * withData(OSData *, unsigned int, unsigned int)@/link</code>
 * instead.
 *
 * The new OSData object will grow as needed to accommodate more bytes
 * (<i>unlike</i> @link //apple_ref/doc/uid/20001498 CFMutableData@/link,
 * for which a nonzero initial capacity is a hard limit).
 */
	virtual bool initWithData(
		const OSData * inData,
		unsigned int   start,
		unsigned int   numBytes);


/*!
 * @function free
 *
 * @abstract
 * Deallocates or releases any resources
 * used by the OSData instance.
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
 * @function getLength
 *
 * @abstract
 * Returns the number of bytes in or referenced by the OSData object.
 *
 * @result
 * The number of bytes in or referenced by the OSData object.
 */
	virtual unsigned int getLength() const;


/*!
 * @function getCapacity
 *
 * @abstract
 * Returns the total number of bytes the OSData can store without reallocating.
 *
 * @result
 * The total number bytes the OSData can store without reallocating.
 *
 * @discussion
 * OSData objects grow when full to accommodate additional bytes.
 * See
 * <code>@link
 * //apple_ref/cpp/instm/OSData/getCapacityIncrement/virtualunsignedint/()
 * getCapacityIncrement@/link</code>
 * and
 * <code>@link
 * //apple_ref/cpp/instm/OSData/ensureCapacity/virtualunsignedint/(unsignedint)
 * ensureCapacity@/link</code>.
 *
 * OSData objects created or initialized to use a shared buffer
 * do not make use of this attribute, and return -1 from this function.
 */
	virtual unsigned int getCapacity() const;


/*!
 * @function getCapacityIncrement
 *
 * @abstract
 * Returns the storage increment of the OSData object.
 *
 * @result
 * The storage increment of the OSData object.
 *
 * @discussion
 * An OSData object allocates storage for bytes in multiples
 * of the capacity increment.
 *
 * OSData objects created or initialized to use a shared buffer
 * do not make use of this attribute.
 */
	virtual unsigned int getCapacityIncrement() const;


/*!
 * @function setCapacityIncrement
 *
 * @abstract
 * Sets the storage increment of the array.
 *
 * @result
 * The original storage increment of the array.
 *
 * @discussion
 * An OSArray allocates storage for objects in multiples
 * of the capacity increment.
 *
 * OSData objects created or initialized to use a shared buffer
 * do not make use of this attribute.
 */
	virtual unsigned int setCapacityIncrement(unsigned increment);


// xx-review: does not check for capacity == EXTERNAL

/*!
 * @function ensureCapacity
 *
 * @abstract
 * Ensures the array has enough space
 * to store the requested number of bytes.
 *
 * @param newCapacity The total number of bytes the OSData object
 *                    should be able to store.
 *
 * @result
 * Returns the new capacity of the OSData object,
 * which may be different from the number requested
 * (if smaller, reallocation of storage failed).
 *
 * @discussion
 * This function immediately resizes the OSData's buffer, if necessary,
 * to accommodate at least <code>newCapacity</code> bytes.
 * If <code>newCapacity</code> is not greater than the current capacity,
 * or if an allocation error occurs, the original capacity is returned.
 *
 * There is no way to reduce the capacity of an OSData.
 *
 * An OSData object created "NoCopy" does not allow resizing.
 */
	virtual unsigned int ensureCapacity(unsigned int newCapacity);


/*!
 * @function appendBytes
 *
 * @abstract
 * Appends a buffer of bytes to the OSData object's internal data buffer.
 *
 * @param bytes     A pointer to the data to append.
 *                  If <code>bytes</code> is <code>NULL</code>
 *                  then a zero-filled buffer of length <code>numBytes</code>
 *                  is appended.
 * @param numBytes  The number of bytes from <code>bytes</code> to append.
 *
 * @result
 * <code>true</code> if the new data was successfully added,
 * <code>false</code> on failure.
 *
 * @discussion
 * This function immediately resizes the OSData's buffer, if necessary,
 * to accommodate the new total size.
 *
 * An OSData object created "NoCopy" does not allow bytes
 * to be appended.
 */
	virtual bool appendBytes(
		const void   * bytes,
		unsigned int   numBytes);


/*!
 * @function appendBytes
 *
 * @abstract
 * Appends the data contained in another OSData object.
 *
 * @param aDataObj  The OSData object whose contents will be appended.
 *
 * @result
 * <code>true</code> if the new data was successfully added,
 * <code>false</code> on failure.
 *
 * @discussion
 * This function immediately resizes the OSData's buffer, if necessary,
 * to accommodate the new total size.
 *
 * An OSData object created "NoCopy" does not allow bytes
 * to be appended.
 */
	virtual bool appendBytes(const OSData * aDataObj);


/*!
 * @function getBytesNoCopy
 *
 * @abstract
 * Returns a pointer to the OSData object's internal data buffer.
 *
 * @result
 * A pointer to the OSData object's internal data buffer.
 *
 * @discussion
 * You can modify the existing contents of an OSData object
 * via this function.
 * It works with OSData objects that have their own data buffers
 * as well as with OSData objects that have shared buffers.
 *
 * If you append bytes or characters to an OSData object,
 * it may have to reallocate its internal storage,
 * rendering invalid an extrated pointer to that storage.
 */
	virtual const void * getBytesNoCopy() const;


/*!
 * @function getBytesNoCopy
 *
 * @abstract
 * Returns a pointer into the OSData object's internal data buffer
 * with a given offset and length.
 *
 * @param start    The offset from the base of the internal data buffer.
 * @param numBytes The length of the  window.
 *
 * @result
 * A pointer to the bytes in the specified range
 * within the OSData object,
 * or 0 if that range does not lie completely
 * within the object's buffer.
 *
 * @discussion
 * You can modify the existing contents of an OSData object
 * via this function.
 * It works with OSData objects that have their own data buffers
 * as well as with OSData objects that have shared buffers.
 *
 * If you append bytes or characters to an OSData object,
 * it may have to reallocate its internal storage,
 * rendering invalid an extrated pointer to that storage.
 */
	virtual const void * getBytesNoCopy(
		unsigned int start,
		unsigned int numBytes) const;


/*!
 * @function isEqualTo
 *
 * @abstract
 * Tests the equality of two OSData objects.
 *
 * @param aDataObj The OSData object being compared against the receiver.
 *
 * @result
 * <code>true</code> if the two OSData objects are equivalent,
 * <code>false</code> otherwise.
 *
 * @discussion
 * Two OSData objects are considered equal
 * if they have same length and if their
 * byte buffers hold the same contents.
 */
	virtual bool isEqualTo(const OSData * aDataObj) const;


/*!
 * @function isEqualTo
 *
 * @abstract
 * Tests the equality of an OSData object's contents
 * to a C array of bytes.
 *
 * @param bytes    A pointer to the bytes to compare.
 * @param numBytes The number of bytes to compare.
 *
 * @result
 * <code>true</code> if the data buffers are equal
 * over the given length,
 * <code>false</code> otherwise.
 */
	virtual bool isEqualTo(
		const void   * bytes,
		unsigned int   numBytes) const;


/*!
 * @function isEqualTo
 *
 * @abstract
 * Tests the equality of an OSData object to an arbitrary object.
 *
 * @param anObject The object to be compared against the receiver.
 *
 * @result
 * <code>true</code> if the two objects are equivalent,
 * <code>false</code> otherwise.
 *
 * @discussion
 * An OSData is considered equal to another object
 * if that object is derived from OSData
 * and contains the equivalent bytes of the same length.
 */
	virtual bool isEqualTo(const OSMetaClassBase * anObject) const APPLE_KEXT_OVERRIDE;


/*!
 * @function isEqualTo
 *
 * @abstract
 * Tests the equality of an OSData object to an OSString.
 *
 * @param aString  The string object to be compared against the receiver.
 *
 * @result
 * <code>true</code> if the two objects are equivalent,
 * <code>false</code> otherwise.
 *
 * @discussion
 * This function compares the bytes of the OSData object
 * against those of the OSString,
 * accounting for the possibility that an OSData
 * might explicitly include a nul
 * character as part of its total length.
 * Thus, for example, an OSData object containing
 * either the bytes <'u', 's', 'b', '\0'>
 * or  <'u', 's', 'b'>
 * will compare as equal to the OSString containing "usb".
 */
	virtual bool isEqualTo(const OSString * aString) const;


/*!
 * @function serialize
 *
 * @abstract
 * Archives the receiver into the provided
 * @link //apple_ref/doc/class/IORegistryEntry OSSerialize@/link object.
 *
 * @param serializer The OSSerialize object.
 *
 * @result
 * <code>true</code> if serialization succeeds, <code>false</code> if not.
 */
	virtual bool serialize(OSSerialize * serializer) const APPLE_KEXT_OVERRIDE;


/*!
 * @function appendByte
 *
 * @abstract
 * Appends a single byte value
 * to the OSData object's internal data buffer
 * a specified number of times.
 *
 * @param byte     The byte value to append.
 * @param numBytes The number of copies of <code>byte</code> to append.
 *
 * @result
 * <code>true</code> if the new data was successfully added,
 * <code>false</code> if not.
 *
 * @discussion
 * This function immediately resizes the OSData's buffer, if necessary,
 * to accommodate the new total size.
 *
 * An OSData object created "NoCopy" does not allow bytes
 * to be appended.
 */
	virtual bool appendByte(
		unsigned char byte,
		unsigned int  numBytes);


	void setSerializable(bool serializable);

#ifdef XNU_KERNEL_PRIVATE
/* Available within xnu source only */
public:
#else
private:
#endif
	virtual void setDeallocFunction(DeallocFunction func);
	OSMetaClassDeclareReservedUsed(OSData, 0);
	bool isSerializable(void);

private:
	OSMetaClassDeclareReservedUnused(OSData, 1);
	OSMetaClassDeclareReservedUnused(OSData, 2);
	OSMetaClassDeclareReservedUnused(OSData, 3);
	OSMetaClassDeclareReservedUnused(OSData, 4);
	OSMetaClassDeclareReservedUnused(OSData, 5);
	OSMetaClassDeclareReservedUnused(OSData, 6);
	OSMetaClassDeclareReservedUnused(OSData, 7);
};

#endif /* !_OS_OSDATA_H */
