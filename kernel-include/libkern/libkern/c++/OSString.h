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
/* IOString.h created by rsulack on Wed 17-Sep-1997 */
/* IOString.h converted to C++ by gvdl on Fri 1998-10-30 */

#ifndef _OS_OSSTRING_H
#define _OS_OSSTRING_H

#include <libkern/c++/OSObject.h>
#include <libkern/c++/OSPtr.h>

class OSData;
class OSString;

typedef OSPtr<OSString> OSStringPtr;
typedef OSPtr<const OSString> OSStringConstPtr;


/*!
 * @header
 *
 * @abstract
 * This header declares the OSString container class.
 */


/* Not to be included in headerdoc.
 *
 * For internal use.
 */
enum { kOSStringNoCopy = 0x00000001 };


/*!
 * @class OSString
 *
 * @abstract
 * OSString wraps a C string in a C++ object for use in Libkern collections.
 *
 * @discussion
 * OSString is a container class for managing arrays of characters.
 * An OSString normally maintains its own character buffer and allows changes,
 * but you can create an "immutable" OSString
 * that references an external C string
 * buffer using the "NoCopy" creator functions.
 * Functions called to change the contents of an immutable OSString will fail.
 *
 * <b>Encodings</b>
 *
 * OSString makes no provisions for different character encodings and
 * assumes that a string is a nul-terminated sequence of single-byte characters.
 * User-space code must either assume an encoding (typically ASCII or UTF-8)
 * or determine it in some other way (such as an IORegistryEntry property).
 *
 * <b>Altering Strings</b>
 *
 * OSString's indended use is as a reference-counted object container
 * for a C string and little more.
 * While OSString provides full access to the underlying C string,
 * it provides little in the way of string object manipulation;
 * there are no append or insert functions,
 * only a set-character function.
 * If you need to manipulate OSStrings,
 * it's generally best to get the C strings,
 * alter them as necessary, and create a new OSString object
 * from the resulting C string.
 *
 * <b>Use Restrictions</b>
 *
 * With very few exceptions in the I/O Kit, all Libkern-based C++
 * classes, functions, and macros are <b>unsafe</b>
 * to use in a primary interrupt context.
 * Consult the I/O Kit documentation related to primary interrupts
 * for more information.
 *
 * OSString provides no concurrency protection;
 * it's up to the usage context to provide any protection necessary.
 * Some portions of the I/O Kit, such as
 * @link //apple_ref/doc/class/IORegistryEntry IORegistryEntry@/link,
 * handle synchronization via defined member functions for setting
 * properties.
 */
class OSString : public OSObject
{
	OSDeclareDefaultStructors(OSString);

	enum { kMaxStringLength  = 262142 };

#if APPLE_KEXT_ALIGN_CONTAINERS

protected:

	unsigned int   flags:14,
	    length:18;
	char         * string;

#else /* APPLE_KEXT_ALIGN_CONTAINERS */

protected:
	char         * string;
	unsigned int   flags;
	unsigned int   length;

#endif /* APPLE_KEXT_ALIGN_CONTAINERS */

public:

/*!
 * @function withString
 *
 * @abstract
 * Creates and initializes an OSString from another OSString.
 *
 * @param aString   The OSString object whose contents to copy.
 *
 * @result
 * An instance of OSString representing
 * the same characters as <code>aString</code>,
 * and with a reference count of 1;
 * <code>NULL</code> on failure.
 *
 * @discussion
 * The new OSString is a distinct instance from <code>aString</code>,
 * and is not merely the original object
 * with the reference count incremented.
 * Changes to one will not be reflected in the other.
 */
	static OSStringPtr withString(const OSString * aString);


/*!
 * @function withCString
 *
 * @abstract
 * Creates and initializes an OSString from a C string.
 *
 * @param cString   The C string to copy into the new OSString.
 *
 * @result
 * An instance of OSString representing
 * the same characters as <code>aString</code>,
 * and with a reference count of 1;
 * <code>NULL</code> on failure.
 */
	static OSStringPtr withCString(const char * cString);


/*!
 * @function withCStringNoCopy
 *
 * @abstract
 * Creates and initializes an immutable OSString
 * that shares the provided C string buffer.
 *
 * @param cString   The C string to reference.
 *
 * @result
 * An instance of OSString containing <code>cString</code>,
 * and with a reference count of 1;
 * <code>NULL</code> on failure.
 *
 * @discussion
 * An OSString object created with this function
 * does not claim ownership of the C string,
 * but shares it with the caller.
 * When the caller determines that the OSString object has actually been freed,
 * it can safely dispose of the data buffer.
 * Conversely, if it frees the shared data buffer,
 * it must not attempt to use the OSString object and should release it.
 *
 * An OSString object created with this function does not
 * allow changing the string via <code>@link setChar setChar@/link</code>.
 */
	static OSStringPtr withCStringNoCopy(const char * cString);

#if XNU_KERNEL_PRIVATE
	static OSStringPtr withStringOfLength(const char *cString, size_t length);
#endif  /* XNU_KERNEL_PRIVATE */

/*!
 * @function initWithString
 *
 * @abstract
 * Initializes an OSString from another OSString.
 *
 * @param aString   The OSString object whose contents to copy.
 *
 * @result
 * <code>true</code> on success, <code>false</code> on failure.
 *
 * @discussion
 * Not for general use. Use the static instance creation method
 * <code>@link withString withString@/link</code> instead.
 */
	virtual bool initWithString(const OSString * aString);


/*!
 * @function initWithCString
 *
 * @abstract
 * Initializes an OSString from a C string.
 *
 * @param cString   The C string to copy into the new OSString.
 *
 * @result
 * <code>true</code> on success, <code>false</code> on failure.
 *
 * @discussion
 * Not for general use. Use the static instance creation method
 * <code>@link withCString withCString@/link</code> instead.
 */
	virtual bool initWithCString(const char * cString);


/*!
 * @function initWithCStringNoCopy
 *
 * @abstract
 * Initializes an immutable OSString
 * to share the provided C string buffer.
 *
 * @param cString   The C string to reference.
 *
 * @result
 * <code>true</code> on success, <code>false</code> on failure.
 *
 * @discussion
 * Not for general use. Use the static instance creation method
 * <code>@link withCStringNoCopy withCStringNoCopy@/link</code> instead.
 *
 * An OSString object initialized with this function
 * does not claim ownership of the C string,
 * but shares it with the caller.
 * When the caller determines that the OSString object has actually been freed,
 * it can safely dispose of the data buffer.
 * Conversely, if it frees the shared data buffer,
 * it must not attempt to use the OSString object and should release it.
 *
 * An OSString object created with this function does not
 * allow changing the string via <code>@link setChar setChar@/link</code>.
 */
	virtual bool initWithCStringNoCopy(const char * cString);

#if XNU_KERNEL_PRIVATE
	bool initWithStringOfLength(const char *cString, size_t inlength);
#endif  /* XNU_KERNEL_PRIVATE */

/*!
 * @function free
 *
 * @abstract
 * Deallocates or releases any resources
 * used by the OSString instance.
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
 * Returns the number of characters in the OSString object.
 *
 * @result
 * The number of characters in the OSString object.
 */
	virtual unsigned int getLength() const;


/*!
 * @function getChar
 *
 * @abstract
 * Returns the character at a given index in the string object.
 *
 * @param index The index into the string.
 *
 * @result
 * The character at <code>index</code> within the string,
 * or <code>'\0'</code> if index is past the end of the string.
 */
	virtual char getChar(unsigned int index) const;


/*!
 * @function setChar
 *
 * @abstract
 * Replaces a character at a given index in the string object.
 *
 * @param aChar The character value to set.
 * @param index The index into the string.
 *
 * @result
 * <code>true</code> if the character was replaced,
 * <code>false</code> if the was created "NoCopy"
 * or <code>index</code> is past the end of the string.
 */
	virtual bool setChar(char aChar, unsigned int index);


/*!
 * @function getCStringNoCopy
 *
 * @abstract
 * Returns a pointer to the internal C string buffer.
 *
 * @result
 * A pointer to the internal C string buffer.
 */
	virtual const char * getCStringNoCopy() const;


/*!
 * @function isEqualTo
 *
 * @abstract
 * Tests the equality of two OSString objects.
 *
 * @param aString  The OSString object being compared against the receiver.
 *
 * @result
 * <code>true</code> if the two OSString objects are equivalent,
 * <code>false</code> otherwise.
 *
 * @discussion
 * Two OSString objects are considered equal if they have same length
 * and if their byte buffers hold the same contents.
 */
	virtual bool isEqualTo(const OSString * aString) const;


/*!
 * @function isEqualTo
 *
 * @abstract
 * Tests the equality of an OSString object with a C string.
 *
 * @param cString  The C string to compare against the receiver.
 *
 * @result
 * <code>true</code> if the OSString's characters
 * are equivalent to the C string's,
 * <code>false</code> otherwise.
 */
	virtual bool isEqualTo(const char * cString) const;


/*!
 * @function isEqualTo
 *
 * @abstract
 * Tests the equality of an OSString object to an arbitrary object.
 *
 * @param anObject The object to be compared against the receiver.
 *
 * @result
 * Returns <code>true</code> if the two objects are equivalent,
 * <code>false</code> otherwise.
 *
 * @discussion
 * An OSString is considered equal to another object
 * if that object is derived from OSString
 * and contains the equivalent bytes of the same length.
 */
	virtual bool isEqualTo(const OSMetaClassBase * anObject) const APPLE_KEXT_OVERRIDE;


/*!
 * @function isEqualTo
 *
 * @abstract
 * Tests the equality of an OSData object and the OSString instance.
 *
 * @param aDataObject An OSData object.
 *
 * @result
 * <code>true</code> if the two objects are equivalent, <code>false</code> otherwise.
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
	virtual bool isEqualTo(const OSData * aDataObject) const;


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

	OSMetaClassDeclareReservedUnused(OSString, 0);
	OSMetaClassDeclareReservedUnused(OSString, 1);
	OSMetaClassDeclareReservedUnused(OSString, 2);
	OSMetaClassDeclareReservedUnused(OSString, 3);
	OSMetaClassDeclareReservedUnused(OSString, 4);
	OSMetaClassDeclareReservedUnused(OSString, 5);
	OSMetaClassDeclareReservedUnused(OSString, 6);
	OSMetaClassDeclareReservedUnused(OSString, 7);
	OSMetaClassDeclareReservedUnused(OSString, 8);
	OSMetaClassDeclareReservedUnused(OSString, 9);
	OSMetaClassDeclareReservedUnused(OSString, 10);
	OSMetaClassDeclareReservedUnused(OSString, 11);
	OSMetaClassDeclareReservedUnused(OSString, 12);
	OSMetaClassDeclareReservedUnused(OSString, 13);
	OSMetaClassDeclareReservedUnused(OSString, 14);
	OSMetaClassDeclareReservedUnused(OSString, 15);
};

#endif /* !_OS_OSSTRING_H */
