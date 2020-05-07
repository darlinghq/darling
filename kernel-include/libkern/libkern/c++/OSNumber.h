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
/* IOOffset.h created by rsulack on Wed 17-Sep-1997 */
/* IOOffset.h converted to C++ by gvdl on Fri 1998-10-30 */

#ifndef _OS_OSNUMBER_H
#define _OS_OSNUMBER_H

#include <libkern/c++/OSObject.h>
#include <libkern/c++/OSPtr.h>

/*!
 * @header
 *
 * @abstract
 * This header declares the OSNumber container class.
 */

class OSNumber;

typedef OSPtr<OSNumber> OSNumberPtr;

/*!
 * @class OSNumber
 *
 * @abstract
 * OSNumber wraps an integer value in a C++ object
 * for use in Libkern collections.
 *
 * @discussion
 * OSNumber represents an integer of 8, 16, 32, or 64 bits
 * as a Libkern C++ object.
 * OSNumber objects are mutable: you can add to or set their values.
 *
 * <b>Use Restrictions</b>
 *
 * With very few exceptions in the I/O Kit, all Libkern-based C++
 * classes, functions, and macros are <b>unsafe</b>
 * to use in a primary interrupt context.
 * Consult the I/O Kit documentation related to primary interrupts
 * for more information.
 *
 * OSNumber provides no concurrency protection;
 * it's up to the usage context to provide any protection necessary.
 * Some portions of the I/O Kit, such as
 * @link //apple_ref/doc/class/IORegistryEntry IORegistryEntry@/link,
 * handle synchronization via defined member functions for setting
 * properties.
 */
class OSNumber : public OSObject
{
	friend class OSSerialize;

	OSDeclareDefaultStructors(OSNumber);

#if APPLE_KEXT_ALIGN_CONTAINERS

protected:
	unsigned int size;
	unsigned long long value;

#else /* APPLE_KEXT_ALIGN_CONTAINERS */

protected:
	unsigned long long value;
	unsigned int size;

	struct ExpansionData { };

/* Reserved for future use.  (Internal use only)  */
	ExpansionData * reserved;

#endif /* APPLE_KEXT_ALIGN_CONTAINERS */

public:

/*!
 * @function withNumber
 *
 * @abstract
 * Creates and initializes an instance of OSNumber
 * with an integer value.
 *
 * @param value        The numeric integer value for the OSNumber to store.
 * @param numberOfBits The number of bits to limit storage to.
 *
 * @result
 * An instance of OSNumber with a reference count of 1;
 * <code>NULL</code> on failure.
 *
 * @discussion
 * <code>value</code> is masked to the provided <code>numberOfBits</code>
 * when the OSNumber object is initialized.
 *
 * You can change the value of an OSNumber later
 * using <code>@link setValue setValue@/link</code>
 * and <code>@link addValue addValue@/link</code>,
 * but you can't change the bit size.
 */
	static OSNumberPtr withNumber(
		unsigned long long value,
		unsigned int       numberOfBits);


/*!
 * @function withNumber
 *
 * @abstract
 * Creates and initializes an instance of OSNumber
 * with an unsigned integer value represented as a C string.
 *
 * @param valueString  A C string representing a numeric value
 *                     for the OSNumber to store.
 * @param numberOfBits The number of bits to limit storage to.
 *
 * @result
 * An instance of OSNumber with a reference count of 1;
 * <code>NULL</code> on failure.
 *
 * @discussion
 * This function does not work in I/O Kit versions prior to 8.0 (Mac OS X 10.4).
 * In I/O Kit version 8.0 and later, it works
 * but is limited to parsing unsigned 32 bit quantities.
 * The format of the C string may be decimal, hexadecimal ("0x" prefix),
 * binary ("0b" prefix), or octal ("0" prefix).
 *
 * The parsed value is masked to the provided <code>numberOfBits</code>
 * when the OSNumber object is initialized.
 *
 * You can change the value of an OSNumber later
 * using <code>@link setValue setValue@/link</code>
 * and <code>@link addValue addValue@/link</code>,
 * but you can't change the bit size.
 */
	static OSNumberPtr withNumber(
		const char   * valueString,
		unsigned int   numberOfBits);


/*!
 * @function init
 *
 * @abstract
 * Initializes an instance of OSNumber with an integer value.
 *
 * @param value        The numeric integer value for the OSNumber to store.
 * @param numberOfBits The number of bits to limit storage to.
 *
 * @result
 * <code>true</code> if initialization succeeds,
 * <code>false</code> on failure.
 *
 * @discussion
 * Not for general use. Use the static instance creation method
 * <code>@link
 * //apple_ref/cpp/clm/OSNumber/withNumber/staticOSNumber*\/(constchar*,unsignedint)
 * withNumber(unsigned long long, unsigned int)@/link</code>
 * instead.
 */
	virtual bool init(
		unsigned long long value,
		unsigned int       numberOfBits);


/*!
 * @function init
 *
 * @abstract
 * Initializes an instance of OSNumber
 * with an unsigned integer value represented as a C string.
 *
 * @param valueString  A C string representing a numeric value
 *                     for the OSNumber to store.
 * @param numberOfBits The number of bits to limit storage to.
 *
 * @result
 * <code>true</code> if initialization succeeds,
 * <code>false</code> on failure.
 *
 * @discussion
 * Not for general use. Use the static instance creation method
 * <code>@link
 * //apple_ref/cpp/clm/OSNumber/withNumber/staticOSNumber*\/(constchar*,unsignedint)
 * withNumber(const char *, unsigned int)@/link</code>
 * instead.
 */
	virtual bool init(
		const char   * valueString,
		unsigned int   numberOfBits);


/*!
 * @function free
 *
 * @abstract
 * Deallocates or releases any resources
 * used by the OSNumber instance.
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
 * @function numberOfBits
 *
 * @abstract
 * Returns the number of bits used to represent
 * the OSNumber object's integer value.
 *
 * @result
 * The number of bits used to represent
 * the OSNumber object's integer value.
 *
 * @discussion
 * The number of bits is used to limit the stored value of the OSNumber.
 * Any change to its value is performed as an <code>unsigned long long</code>
 * and then truncated to the number of bits.
 */
	virtual unsigned int numberOfBits() const;


/*!
 * @function numberOfBytes
 *
 * @abstract
 * Returns the number of bytes used to represent
 * the OSNumber object's integer value.
 *
 * @result
 * The number of bytes used to represent
 * the OSNumber object's integer value.
 * See <code>@link numberOfBits numberOfBits@/link</code>.
 */
	virtual unsigned int numberOfBytes() const;


// xx-review: should switch to explicitly-sized int types
// xx-review: but that messes up C++ mangled symbols :-(


/*!
 * @function unsigned8BitValue
 *
 * @abstract
 * Returns the OSNumber object's integer value
 * cast as an unsigned 8-bit integer.
 *
 * @result
 * The OSNumber object's integer value
 * cast as an unsigned 8-bit integer.
 *
 * @discussion
 * This function merely casts the internal integer value,
 * giving no indication of truncation or other potential conversion problems.
 */
	virtual unsigned char unsigned8BitValue() const;


/*!
 * @function unsigned16BitValue
 *
 * @abstract
 * Returns the OSNumber object's integer value
 * cast as an unsigned 16-bit integer.
 *
 * @result
 * Returns the OSNumber object's integer value
 * cast as an unsigned 16-bit integer.
 *
 * @discussion
 * This function merely casts the internal integer value,
 * giving no indication of truncation or other potential conversion problems.
 */
	virtual unsigned short unsigned16BitValue() const;


/*!
 * @function unsigned32BitValue
 *
 * @abstract
 * Returns the OSNumber object's integer value
 * cast as an unsigned 32-bit integer.
 *
 * @result
 * Returns the OSNumber object's integer value
 * cast as an unsigned 32-bit integer.
 *
 * @discussion
 * This function merely casts the internal integer value,
 * giving no indication of truncation or other potential conversion problems.
 */
	virtual unsigned int unsigned32BitValue() const;


/*!
 * @function unsigned64BitValue
 *
 * @abstract
 * Returns the OSNumber object's integer value
 * cast as an unsigned 64-bit integer.
 *
 * @result
 * Returns the OSNumber object's integer value
 * cast as an unsigned 64-bit integer.
 *
 * @discussion
 * This function merely casts the internal integer value,
 * giving no indication of truncation or other potential conversion problems.
 */
	virtual unsigned long long unsigned64BitValue() const;

// xx-review: wow, there's no addNumber(OSNumber *)!

/*!
 * @function addValue
 *
 * @abstract
 * Adds a signed integer value to the internal integer value
 * of the OSNumber object.
 *
 * @param value  The value to be added.
 *
 * @discussion
 * This function adds values as 64-bit integers,
 * but masks the result by the bit size
 * (see <code>@link numberOfBits numberOfBits@/link</code>),
 * so addition overflows will not necessarily
 * be the same as for plain C integers.
 */
	virtual void addValue(signed long long value);


/*!
 * @function setValue
 *
 * @abstract
 * Replaces the current internal integer value
 * of the OSNumber object by the value given.
 *
 * @param value  The new value for the OSNumber object,
 *               which is truncated by the bit size of the OSNumber object
 *               (see <code>@link numberOfBits numberOfBits@/link</code>).
 */
	virtual void setValue(unsigned long long value);


/*!
 * @function isEqualTo
 *
 * @abstract
 * Tests the equality of two OSNumber objects.
 *
 * @param aNumber     The OSNumber to be compared against the receiver.
 *
 * @result
 * <code>true</code> if the OSNumber objects are equal,
 * <code>false</code> if not.
 *
 * @discussion
 * Two OSNumber objects are considered equal
 * if they represent the same C integer value.
 */
	virtual bool isEqualTo(const OSNumber * aNumber) const;


/*!
 * @function isEqualTo
 *
 * @abstract
 * Tests the equality an OSNumber to an arbitrary object.
 *
 * @param anObject An object to be compared against the receiver.
 *
 * @result
 * <code>true</code> if the objects are equal,
 * <code>false</code> if not.
 *
 * @discussion
 * An OSNumber is considered equal to another object if that object is
 * derived from OSNumber and represents the same C integer value.
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


	OSMetaClassDeclareReservedUnused(OSNumber, 0);
	OSMetaClassDeclareReservedUnused(OSNumber, 1);
	OSMetaClassDeclareReservedUnused(OSNumber, 2);
	OSMetaClassDeclareReservedUnused(OSNumber, 3);
	OSMetaClassDeclareReservedUnused(OSNumber, 4);
	OSMetaClassDeclareReservedUnused(OSNumber, 5);
	OSMetaClassDeclareReservedUnused(OSNumber, 6);
	OSMetaClassDeclareReservedUnused(OSNumber, 7);
};

#endif /* !_OS_OSNUMBER_H */
