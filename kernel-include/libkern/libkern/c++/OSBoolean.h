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
/* OSBoolean.cpp created by rsulack on Tue Oct 12 1999 */

#ifndef _OS_OSBOOLEAN_H
#define _OS_OSBOOLEAN_H

#include <libkern/c++/OSObject.h>
#include <libkern/c++/OSPtr.h>

class OSString;
class OSBoolean;

typedef OSPtr<OSBoolean> OSBooleanPtr;

/*!
 * @header
 *
 * @abstract
 * This header declares the OSBoolean container class.
 */


/*!
 * @class OSBoolean
 *
 * @abstract
 * OSBoolean wraps a boolean value in a C++ object
 * for use in Libkern collections.
 *
 * @discussion
 * OSBoolean represents a boolean <code>true</code>/<code>false</code> value
 * as a Libkern C++ object.
 * There are only two instances of OSBoolean,
 * <code>@link kOSBooleanTrue kOSBooleanTrue@/link</code>
 * and <code>@link kOSBooleanFalse kOSBooleanFalse@/link</code>.
 * These are shared globally and returned by the instance-creation function
 * <code>@link withBoolean withBoolean@/link</code>.
 * Thus, you can use pointer comparison
 * to test whether two OSBoolean objects are equal.
 */
class OSBoolean : public OSObject
{
	OSDeclareDefaultStructors(OSBoolean);
	friend class OSSerialize;

protected:
	bool value;

/*!
 * @function taggedRelease
 *
 * @abstract
 * Overrides the reference counting mechanism
 * for the shared global instances.
 *
 * @param tag  Unused.
 * @param when Unused.
 */
	virtual void taggedRelease(
		const void * tag,
		const int    when) const APPLE_KEXT_OVERRIDE;

public:
	static void initialize();

/*!
 * @function withBoolean
 *
 * @abstract
 * Returns one of the global instances of OSBoolean.
 *
 * @param value   A boolean value.
 *
 * @result
 * The global instance of OSBoolean with the boolean <code>value</code>.
 *
 * @discussion
 * This function actually returns either
 * <code>@link kOSBooleanTrue kOSBooleanTrue@/link</code> or
 * <code>@link kOSBooleanFalse kOSBooleanFalse@/link</code>,
 * so that you can always use pointer comparison with OSBoolean objects.
 */
	static OSBooleanPtr withBoolean(bool value);

/*!
 * @function free
 *
 * @abstract
 * Overridden to prevent deallocation of the shared global instances.
 *
 * @discussion
 * This function should never be called.
 */
	virtual void free() APPLE_KEXT_OVERRIDE;


/*!
 * @function taggedRetain
 *
 * @abstract
 * Overrides the reference counting mechanism for the shared global instances.
 *
 * @param tag  Unused.
 */
	virtual void taggedRetain(const void * tag) const APPLE_KEXT_OVERRIDE;


/*!
 * @function isTrue
 *
 * @abstract
 * Checks whether the OSBoolean object
 * represents a <code>true</code> <code>bool</code> value.
 *
 * @result
 * <code>true</code> if the OSBoolean object is <code>true</code>,
 * <code>false</code> otherwise.
 *
 * @discussion
 * You can also use <code>==</code> against
 * <code>@link kOSBooleanTrue kOSBooleanTrue@/link</code>.
 */
	virtual bool isTrue() const;


/*!
 * @function isFalse
 *
 * @abstract
 * Checks whether the OSBoolean object
 * represents a <code>false</code> <code>bool</code> value.
 *
 * @result
 * <code>true</code> if the OSBoolean object is <code>false</code>,
 * <code>true</code> otherwise.
 *
 * @discussion
 * You can also use <code>==</code> against
 * <code>@link kOSBooleanFalse kOSBooleanFalse@/link</code>.
 */
	virtual bool isFalse() const;


/*!
 * @function getValue
 *
 * @abstract
 * Returns the C++ <code>bool</code> value for the OSBoolean object.
 *
 * @result
 * Returns the C++ <code>bool</code> value of the OSBoolean object.
 */
	virtual bool getValue() const;


/*!
 * @function isEqualTo
 *
 * @abstract
 * Tests the equality of two OSBoolean objects.
 *
 * @param aBoolean    The OSBoolean to be compared against the receiver.
 *
 * @result
 * <code>true</code> if the OSBoolean objects are equal,
 * <code>false</code> if not.
 *
 * @discussion
 * Two OSBoolean objects are considered equal
 * if they are the same exact object (pointer equality).
 */
	virtual bool isEqualTo(const OSBoolean * aBoolean) const;


/*!
 * @function isEqualTo
 *
 * @abstract
 * Tests the equality an OSBoolean to an arbitrary object.
 *
 * @param anObject  An object to be compared against the receiver.
 *
 * @result
 * <code>true</code> if the objects are equal, <code>false</code> if not.
 *
 * @discussion
 * An OSBoolean is considered equal to another object
 * if that object is derived from OSBoolean
 * and represents the same C++ <code>bool</code> value.
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

	OSMetaClassDeclareReservedUnused(OSBoolean, 0);
	OSMetaClassDeclareReservedUnused(OSBoolean, 1);
	OSMetaClassDeclareReservedUnused(OSBoolean, 2);
	OSMetaClassDeclareReservedUnused(OSBoolean, 3);
	OSMetaClassDeclareReservedUnused(OSBoolean, 4);
	OSMetaClassDeclareReservedUnused(OSBoolean, 5);
	OSMetaClassDeclareReservedUnused(OSBoolean, 6);
	OSMetaClassDeclareReservedUnused(OSBoolean, 7);
};

/*!
 * @const kOSBooleanTrue
 *
 * @abstract
 * The OSBoolean constant for <code>true</code>.
 *
 * @discussion
 * kOSBooleanTrue is the OSBoolean constant for <code>true</code>.
 * This object does not need to be retained or released (but it can be).
 * Comparisons of the form
 * <code>booleanObject == kOSBooleanTrue</code>
 * are acceptable and are equivalent to
 * <code>booleanObject->getValue() == true</code>.
 */
extern OSBoolean * const & kOSBooleanTrue;

/*!
 * @const kOSBooleanFalse
 *
 * @abstract
 * The OSBoolean constant for <code>false</code>.
 *
 * @discussion
 * kOSBooleanFalse is the OSBoolean constant for <code>false</code>.
 * This object does not need to be retained or released (but it can be).
 * Comparisons of the form
 * <code>booleanObject == kOSBooleanFalse</code>
 * are acceptable and are equivalent to
 * <code>booleanObject->getValue() == false</code>.
 */
extern OSBoolean * const & kOSBooleanFalse;

#endif /* !_OS_OSBOOLEAN_H */
