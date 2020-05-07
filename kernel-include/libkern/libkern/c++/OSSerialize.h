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
/* OSSerialize.h created by rsulack on Wen 25-Nov-1998 */

#ifndef _OS_OSSERIALIZE_H
#define _OS_OSSERIALIZE_H

#include <libkern/c++/OSObject.h>

class OSCollection;
class OSSet;
class OSDictionary;
class OSArray;
class OSData;

class OSSerializer;
typedef OSPtr<OSSerializer> OSSerializerPtr;

class OSSerialize;
typedef OSPtr<OSSerialize> OSSerializePtr;

/*!
 * @header
 *
 * @abstract
 * This header declares the OSSerialize class.
 */

OSObjectPtr
OSUnserializeBinary(const void *buffer, size_t bufferSize);

/*!
 * @class OSSerialize
 *
 * @abstract
 * OSSerialize coordinates serialization of Libkern C++ objects
 * into an XML stream.
 *
 * @discussion
 * This class is for the most part internal to the OSContainer classes,
 * used for transferring property tables between the kernel and user space.
 * It should not be used directly.
 * Classes that participate in serialization
 * override the
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/serialize/virtualbool/(OSSerialize*)
 * OSObject::serialize@/link</code> .
 * function.
 *
 * <b>Use Restrictions</b>
 *
 * With very few exceptions in the I/O Kit, all Libkern-based C++
 * classes, functions, and macros are <b>unsafe</b>
 * to use in a primary interrupt context.
 * Consult the I/O Kit documentation related to primary interrupts
 * for more information.
 *
 * OSSerialize provides no concurrency protection;
 * it's up to the usage context to provide any protection necessary.
 * Some portions of the I/O Kit, such as
 * @link //apple_ref/doc/class/IORegistryEntry IORegistryEntry@/link,
 * handle synchronization via defined member functions
 * for serializing properties.
 */

class OSSerialize : public OSObject
{
	OSDeclareDefaultStructors(OSSerialize);
	friend class OSBoolean;

private:
	char         * data;           // container for serialized data
	unsigned int   length;         // of serialized data (counting NULL)
	unsigned int   capacity;       // of container
	unsigned int   capacityIncrement;// of container

	OSArray * tags;                    // tags for all objects seen

#ifdef XNU_KERNEL_PRIVATE
public:
	typedef const OSMetaClassBase * (*Editor)(void                  * reference,
	    OSSerialize           * s,
	    OSCollection          * container,
	    const OSSymbol        * name,
	    const OSMetaClassBase * value);
#else
	typedef void * Editor;
#endif

	bool     binary;
	bool     endCollection;
	Editor   editor;
	void   * editRef;
	OSData * indexData;

	bool binarySerialize(const OSMetaClassBase *o);
	bool binarySerializeInternal(const OSMetaClassBase *o);
	bool addBinary(const void * data, size_t size);
	bool addBinaryObject(const OSMetaClassBase * o, uint32_t key, const void * _bits, size_t size,
	    uint32_t * startCollection);
	void endBinaryCollection(uint32_t startCollection);

public:

/*!
 * @function withCapacity
 *
 * @abstract
 * Creates and initializes an empty OSSerialize object.
 *
 * @param  capacity The initial size of the XML buffer.
 *
 * @result
 * A new instance of OSSerialize
 * with a retain count of 1;
 * <code>NULL</code> on failure.
 *
 * @discussion
 * The serializer will grow as needed to accommodate more data.
 */
	static OSSerializePtr withCapacity(unsigned int capacity);

	static OSSerializePtr binaryWithCapacity(unsigned int inCapacity, Editor editor = NULL, void * reference = NULL);
	void setIndexed(bool index);

/*!
 * @function text
 *
 * @abstract
 * Returns the XML text serialized so far.
 *
 * @result
 * The nul-terminated XML data serialized so far.
 */
	virtual char * text() const;


/*!
 * @function clearText
 *
 * @abstract
 * Resets the OSSerialize object.
 *
 * @discussion
 * This function is a useful optimization if you are serializing
 * the same object repeatedly.
 */
	virtual void clearText();

// stuff to serialize your object

/*!
 * @function previouslySerialized
 *
 * @abstract
 * Checks whether the object has already been serialized
 * into the XML stream, emitting a reference if it has.
 *
 * @param object The object to check.
 *
 * @result
 * <code>true</code> if <code>object</code> has already been serialized
 * by this OSSerialize object and a reference
 * to it is successfully added to the XML stream,
 * <code>false</code> otherwise.
 *
 *
 * @discussion
 * This function both reduces the size of generated XML
 * by emitting shorter references to existing objects with the same
 * value (particularly for OSString, OSSymbol, and OSData),
 * and also preserves instance references
 * so that the user-space I/O Kit library can reconstruct
 * an identical graph of object relationships.
 *
 * All classes that override
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/serialize/virtualbool/(OSSerialize*)
 * OSObject::serialize@/link</code>.
 * should call this function before doing any actual serialization;
 * if it returns <code>true</code>, the <code>serialize</code> implementation
 * can immediately return <code>true</code>.
 */
	virtual bool previouslySerialized(const OSMetaClassBase * object);


/*!
 * @function addXMLStartTag
 *
 * @abstract
 * Appends an XML start tag to the XML stream.
 *
 * @param object    The object being serialized.
 * @param tagString The name of the XML tag to emit; for example, "string".
 *
 * @result
 * <code>true</code> if an XML start tag for <code>tagString</code>
 * is successfully added to the XML stream, <code>false</code> otherwise.
 *
 * @discussion
 * This function emits the named tag,
 * enclosed within a pair of angle brackets.
 *
 * A class that implements serialization should call this function
 * with the name of the XML tag that best represents the serialized
 * contents of the object.
 * A limited number of tags are supported by the user-space
 * I/O Kit library:
 * <ul>
 * <li>array</li>
 * <li>dict</li>
 * <li>integer</li>
 * <li>key</li>
 * <li>set</li>
 * <li>string</li>
 * </ul>
 *
 * A call to this function must be balanced with one to
 * <code>@link addXMLEndTag addXMLEndTag@/link</code>
 * using the same <code>tagString</code>.
 */
	virtual bool addXMLStartTag(
		const OSMetaClassBase * object,
		const char            * tagString);


/*!
 * @function addXMLEndTag
 *
 * @abstract
 * Appends an XML end tag to the XML stream.
 *
 * @param tagString The name of the XML tag to emit; for example, "string".
 *
 * @result
 * <code>true</code> if an XML end tag for <code>tagString</code>
 * is successfully added to the XML stream, <code>false</code> otherwise.
 *
 * @discussion
 * This function emits the named tag,
 * preceded by a slash character to indicate the closing of an entity,
 * all enclosed within a pair of angle brackets.
 *
 * A call to this function must balance an earlier call to
 * <code>@link addXMLStartTag addXMLStartTag@/link</code>
 * using the same <code>tagString</code>.
 */
	virtual bool addXMLEndTag(const char * tagString);


/*!
 * @function addChar
 *
 * @abstract
 * Appends a single character to the XML stream.
 *
 * @param aChar The character to append to the XML stream.
 *
 * @result
 * <code>true</code> if <code>char</code>
 * is successfully added to the XML stream, <code>false</code> otherwise.
 */
	virtual bool addChar(const char aChar);


/*!
 * @function addString
 *
 * @abstract
 * Appends a C string to the XML stream.
 *
 * @param cString The C string to append to the XML stream.
 *
 * @result
 *  <code>true</code> if <code>cString</code>
 * is successfully added to the XML stream, <code>false</code> otherwise.
 */
	virtual bool addString(const char * cString);

// stuff you should never have to use (in theory)

	virtual bool initWithCapacity(unsigned int inCapacity);
	virtual unsigned int getLength() const;
	virtual unsigned int getCapacity() const;
	virtual unsigned int getCapacityIncrement() const;
	virtual unsigned int setCapacityIncrement(unsigned increment);
	virtual unsigned int ensureCapacity(unsigned int newCapacity);
	virtual void free() APPLE_KEXT_OVERRIDE;

	OSMetaClassDeclareReservedUnused(OSSerialize, 0);
	OSMetaClassDeclareReservedUnused(OSSerialize, 1);
	OSMetaClassDeclareReservedUnused(OSSerialize, 2);
	OSMetaClassDeclareReservedUnused(OSSerialize, 3);
	OSMetaClassDeclareReservedUnused(OSSerialize, 4);
	OSMetaClassDeclareReservedUnused(OSSerialize, 5);
	OSMetaClassDeclareReservedUnused(OSSerialize, 6);
	OSMetaClassDeclareReservedUnused(OSSerialize, 7);
};


typedef bool (*OSSerializerCallback)(void * target, void * ref,
    OSSerialize * serializer);

#ifdef __BLOCKS__
typedef bool (^OSSerializerBlock)(OSSerialize * serializer);
#endif /* __BLOCKS__ */


class OSSerializer : public OSObject
{
	OSDeclareDefaultStructors(OSSerializer);

	void * target;
	void * ref;
	OSSerializerCallback callback;

public:

	static OSSerializerPtr forTarget(
		void * target,
		OSSerializerCallback callback,
		void * ref = NULL);

#ifdef __BLOCKS__
	static OSSerializerPtr withBlock(
		OSSerializerBlock callback);
#endif

	virtual void free( void ) APPLE_KEXT_OVERRIDE;

#if XNU_KERNEL_PRIVATE
	static bool callbackToBlock(void * target, void * ref,
	    OSSerialize * serializer);
#endif /* XNU_KERNEL_PRIVATE */

	virtual bool serialize(OSSerialize * serializer) const APPLE_KEXT_OVERRIDE;
};

#endif /* _OS_OSSERIALIZE_H */
