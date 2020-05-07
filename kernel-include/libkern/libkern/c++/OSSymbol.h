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
/* IOSymbol.h created by gvdl on Fri 1998-10-30 */
/* OSSymbol must be created through the factory methods and thus is not subclassable. */

#ifndef _OS_OSSYMBOL_H
#define _OS_OSSYMBOL_H

#include <libkern/c++/OSString.h>
#include <libkern/c++/OSPtr.h>

class OSSymbol;

typedef OSPtr<OSSymbol> OSSymbolPtr;
typedef OSPtr<const OSSymbol> OSSymbolConstPtr;

/*!
 * @header
 *
 * @abstract
 * This header declares the OSSymbol container class.
 */

// xx-review: OSSymbol does not override setChar

/*!
 * @class OSSymbol
 *
 * @abstract
 * OSSymbol wraps a C string in a unique C++ object
 * for use as keys in Libkern collections.
 *
 * @discussion
 * OSSymbol is a container class for managing uniqued strings,
 * for example, those used as dictionary keys.
 * Its static instance-creation functions check
 * for an existing instance of OSSymbol
 * with the requested C string value before creating a new object.
 * If an instance already exists in the pool of unique symbols,
 * its reference count is incremented
 * and the existing instance is returned.
 *
 * While OSSymbol provides for uniquing of a given string value,
 * it makes no effort to enforce immutability of that value.
 * Altering the contents of an OSSymbol should be avoided.
 *
 * <b>Use Restrictions</b>
 *
 * With very few exceptions in the I/O Kit, all Libkern-based C++
 * classes, functions, and macros are <b>unsafe</b>
 * to use in a primary interrupt context.
 * Consult the I/O Kit documentation related to primary interrupts
 * for more information.
 *
 * OSSymbol provides no concurrency protection;
 * it's up to the usage context to provide any protection necessary.
 * Some portions of the I/O Kit, such as
 * @link //apple_ref/doc/class/IORegistryEntry IORegistryEntry@/link,
 * handle synchronization via defined member functions for setting
 * properties.
 */
class OSSymbol : public OSString
{
	friend class OSSymbolPool;

	OSDeclareAbstractStructors(OSSymbol);

private:

	static void initialize();

/*!
 * @function initWithString
 *
 * @abstract
 * Overridden to prevent creation of duplicate symbols.
 *
 * @param aString   Unused.
 *
 * @result
 * <code>false</code>.
 *
 * @discussion
 * Overrides OSString's implementation to prevent creation
 * of distinct OSSymbols with the same string value.
 */
	virtual bool initWithString(const OSString * aString) APPLE_KEXT_OVERRIDE;


/*!
 * @function initWithCString
 *
 * @abstract
 * Overridden to prevent creation of duplicate symbols.
 *
 * @param cString   Unused.
 *
 * @result
 * <code>false</code>.
 *
 * @discussion
 * Overrides OSString's implementation to prevent creation
 * of distinct OSSymbols with the same string value.
 */
	virtual bool initWithCString(const char * cString) APPLE_KEXT_OVERRIDE;


/*!
 * @function initWithCStringNoCopy
 *
 * @abstract
 * Overridden to prevent creation of duplicate symbols.
 *
 * @param cString   Unused.
 *
 * @result
 * <code>false</code>.
 *
 * @discussion
 * Overrides OSString's implementation to prevent creation
 * of distinct OSSymbols with the same string value.
 */
	virtual bool initWithCStringNoCopy(const char *cString) APPLE_KEXT_OVERRIDE;

protected:

// xx-review: should we just omit this from headerdoc?
/*!
 * @function taggedRelease
 *
 * @abstract
 * Overrides
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/taggedRelease/virtualvoid/(constvoid*,constint)
 * OSObject::taggedRelease(const void *, const int)@/link</code>
 * to synchronize with the symbol pool.
 *
 * @param tag      Used for tracking collection references.
 * @param freeWhen If decrementing the reference count makes it
 *                 >= <code>freeWhen</code>, the object is immediately freed.
 *
 * @discussion
 * Because OSSymbol shares instances, the reference-counting functions
 * must synchronize access to the class-internal tables
 * used to track those instances.
 */
	virtual void taggedRelease(
		const void * tag,
		const int    freeWhen) const APPLE_KEXT_OVERRIDE;


// xx-review: should we just omit this from headerdoc?
/*!
 * @function free
 *
 * @abstract
 * Overrides
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/free/virtualvoid/()
 * OSObject::free@/link</code>
 * to synchronize with the symbol pool.
 *
 * @discussion
 * Because OSSymbol shares instances, the reference-counting functions
 * must synchronize access to the class-internal tables
 * used to track those instances.
 */
	virtual void free() APPLE_KEXT_OVERRIDE;

public:

// xx-review: should we just omit this from headerdoc?
/*!
 * @function taggedRelease
 *
 * @abstract
 * Overrides
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/taggedRelease/virtualvoid/(constvoid*)
 * OSObject::taggedRelease(const void *)@/link</code>
 * to synchronize with the symbol pool.
 *
 * @param tag      Used for tracking collection references.
 *
 * @discussion
 * Because OSSymbol shares instances, the reference-counting functions
 * must synchronize access to the class-internal tables
 * used to track those instances.
 */

/* Original note (not for headerdoc):
 * The C++ language has forced me to override this method
 * even though I have implemented it as
 * <code>{ super::taggedRelease(tag) }</code>.
 * It seems that C++ is confused about the appearance of the protected
 * taggedRelease with 2 parameters and refuses to only inherit one function.
 * See
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/taggedRelease/virtualvoid/(constvoid*,constint)
 * OSObject::taggedRelease(const void *, const int)@/link</code>.
 */
	virtual void taggedRelease(const void * tag) const  APPLE_KEXT_OVERRIDE;


/*!
 * @function withString
 *
 * @abstract
 * Returns an OSSymbol created from an OSString,
 * or the existing unique instance of the same value.
 *
 * @param aString   The OSString object to look up or copy.
 *
 * @result
 * An instance of OSSymbol
 * representing the same characters  as <code>aString</code>;
 * <code>NULL</code> on failure.
 *
 * @discussion
 * This function creates or returns the unique OSSymbol instance
 * representing the string value of <code>aString</code>.
 * You can compare it with other OSSymbols using the <code>==</code> operator.
 *
 * OSSymbols are reference-counted normally.
 * This function either returns a
 * new OSSymbol with a retain count of 1,
 * or increments the retain count of the existing instance.
 */
	static OSSymbolConstPtr withString(const OSString * aString);


/*!
 * @function withCString
 *
 * @abstract
 * Returns an OSSymbol created from a C string,
 * or the existing unique instance of the same value.
 *
 * @param cString   The C string to look up or copy.
 *
 * @result
 * An instance of OSSymbol representing
 * the same characters as <code>cString</code>;
 * <code>NULL</code> on failure.
 *
 * @discussion
 * This function returns the unique OSSymbol instance
 * representing the string value of <code>cString</code>.
 * You can compare it with other OSSymbols using the <code>==</code> operator.
 *
 * OSSymbols are reference-counted normally.
 * This function either returns a
 * new OSSymbol with a retain count of 1,
 * or increments the retain count of the existing instance.
 */
	static OSSymbolConstPtr withCString(const char * cString);


/*!
 * @function withCStringNoCopy
 *
 * @abstract
 * Returns an OSSymbol created from a C string,
 * without copying that string,
 * or the existing unique instance of the same value.
 *
 * @param cString   The C string to look up or use.
 * @result
 * An instance of OSSymbol representing
 * the same characters as <code>cString</code>;
 * <code>NULL</code>.
 *
 * @discussion
 * Avoid using this function;
 * OSSymbols should own their internal string buffers.
 *
 * This function returns the unique OSSymbol instance
 * representing the string value of <code>cString</code>.
 * You can compare it with other OSSymbols using the <code>==</code> operator.
 *
 * OSSymbols are reference-counted normally.
 * This function either returns a
 * new OSSymbol with a retain count of 1,
 * or increments the retain count of the existing instance.
 */
	static OSSymbolConstPtr withCStringNoCopy(const char * cString);

/*!
 * @function existingSymbolForString
 *
 * @abstract
 * Returns an existing OSSymbol for the given OSString.
 *
 * @param aString   The OSString Object to look up.
 *
 * @result
 * An existing instance of OSSymbol representing
 * the same characters as <code>aString</code>;
 * <code>NULL</code> if none is found.
 *
 * @discussion
 * The returned OSSymbol object is returned with an incremented refcount
 * that needs to be released.
 */
	static OSSymbolConstPtr existingSymbolForString(const OSString *aString);

/*!
 * @function existingSymbolForCString
 *
 * @abstract
 * Returns an existing OSSymbol for the given C string.
 *
 * @param aCString   The C string to look up.
 *
 * @result
 * An existing instance of OSSymbol representing
 * the same characters as <code>aString</code>;
 * <code>NULL</code> if none is found.
 *
 * @discussion
 * The returned OSSymbol object is returned with an incremented refcount
 * that needs to be released.
 */
	static OSSymbolConstPtr existingSymbolForCString(const char *aCString);

/*!
 * @function isEqualTo
 *
 * @abstract
 * Tests the equality of two OSSymbol objects.
 *
 * @param aSymbol The OSSymbol object being compared against the receiver.
 *
 * @result
 * <code>true</code> if the two OSSymbol objects are equivalent,
 * <code>false</code> otherwise.
 *
 * @discussion
 * Two OSSymbol objects are considered equal if they have the same address;
 * that is, this function is equivalent to the <code>==</code> operator.
 */
	virtual bool isEqualTo(const OSSymbol * aSymbol) const;


/*!
 * @function isEqualTo
 *
 * @abstract Tests the equality of an OSSymbol object with a C string.
 *
 * @param cString  The C string to compare against the receiver.
 *
 * @result
 * <code>true</code> if the OSSymbol's characters
 * are equivalent to the C string's,
 * <code>false</code> otherwise.
 */
	virtual bool isEqualTo(const char * cString) const APPLE_KEXT_OVERRIDE;


/*!
 * @function isEqualTo
 *
 * @abstract Tests the equality of an OSSymbol object to an arbitrary object.
 *
 * @param anObject The object to be compared against the receiver.
 * @result         Returns <code>true</code> if the two objects are equivalent,
 *                 <code>false</code> otherwise.
 *
 * @discussion
 * An OSSymbol is considered equal to another object
 * if that object is derived from
 * @link //apple_ref/doc/class/OSMetaClassBase OSString@/link
 * and contains the equivalent bytes of the same length.
 */
	virtual bool isEqualTo(const OSMetaClassBase * anObject) const APPLE_KEXT_OVERRIDE;


#ifdef XNU_KERNEL_PRIVATE
/* OSRuntime only INTERNAL API - DO NOT USE */
/* Not to be included in headerdoc. */
// xx-review: this should be removed from the symbol set.

	static void checkForPageUnload(
		void * startAddr,
		void * endAddr);

	static unsigned int bsearch(
		const void *  key,
		const void *  array,
		unsigned int  arrayCount,
		size_t        memberSize);
#endif /* XNU_KERNEL_PRIVATE */

	OSMetaClassDeclareReservedUnused(OSSymbol, 0);
	OSMetaClassDeclareReservedUnused(OSSymbol, 1);
	OSMetaClassDeclareReservedUnused(OSSymbol, 2);
	OSMetaClassDeclareReservedUnused(OSSymbol, 3);
	OSMetaClassDeclareReservedUnused(OSSymbol, 4);
	OSMetaClassDeclareReservedUnused(OSSymbol, 5);
	OSMetaClassDeclareReservedUnused(OSSymbol, 6);
	OSMetaClassDeclareReservedUnused(OSSymbol, 7);
};

#endif /* !_OS_OSSYMBOL_H */
