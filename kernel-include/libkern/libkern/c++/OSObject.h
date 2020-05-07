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
/*
 *  Copyright (c) 1998 Apple Computer, Inc.  All rights reserved.
 *  HISTORY
 *   1998-10-30  Godfrey van der Linden(gvdl)
 *   Created
 */
#ifndef _LIBKERN_OSOBJECT_H
#define _LIBKERN_OSOBJECT_H

#include <libkern/c++/OSMetaClass.h>
#include <IOKit/IORPC.h>
#include <DriverKit/OSObject.h>
#include <libkern/c++/OSPtr.h>

#if defined(__clang__)
#pragma clang diagnostic ignored "-Woverloaded-virtual"
#endif

class OSSymbol;
class OSString;
class OSObject;

typedef OSPtr<OSObject> OSObjectPtr;


/*!
 * @header
 *
 * @abstract
 * This header declares the OSObject class,
 * which is the concrete root of the Libkern C++ class hierarchy.
 */


/*!
 * @class OSObject
 *
 * @abstract
 * OSObject is the concrete root class
 * of the Libkern and I/O Kit C++ class hierarchy.
 *
 * @discussion
 * OSObject defines the minimal functionality
 * required of Libkern and I/O Kit C++ classes:
 * tie-in to the run-time type information facility,
 * the dynamic allocation/initialization paradigm,
 * and reference counting.
 * While kernel extensions are free to use their own C++ classes internally,
 * any interaction they have with Libkern or the I/O Kit will require
 * classes ultimately derived from OSObject.
 *
 * <b>Run-Time Type Information</b>
 *
 * OSObject is derived from the abstract root class
 * @link //apple_ref/doc/class/OSMetaClassBase OSMetaClassBase@/link,
 * which declares (and defines many of) the primitives
 * on which the run-time type information facility is based.
 * A parallel inheritance hierarchy of metaclass objects
 * provides run-time introspection, including access to class names,
 * inheritance, and safe type-casting.
 * See @link //apple_ref/doc/class/OSMetaClass OSMetaClass@/link
 * for more information.
 *
 * <b>Dynamic Allocation/Initialization</b>
 *
 * The kernel-resident C++ runtime does not support exceptions,
 * so Libkern classes cannot use standard C++ object
 * constructors and destructors,
 * which use exceptions to report errors.
 * To support error-handling during instance creation, then,
 * OSObject separates object allocation from initialization.
 * You can create a new OSObject-derived instance
 * with the <code>new</code> operator,
 * but this does nothing more than allocate memory
 * and initialize the reference count to 1.
 * Following this, you must call a designated initialization function
 * and check its <code>bool</code> return value.
 * If the initialization fails,
 * you must immediately call
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/release/virtualvoid/()
 * release@/link</code>
 * on the instance and handle the failure in whatever way is appropriate.
 * Many Libkern and I/O Kit classes define static instance-creation functions
 * (beginning with the word "with")
 * to make construction a one-step process for clients.
 *
 * <b>Reference Counting</b>
 *
 * OSObject provides reference counting services using the
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/retain/virtualvoid/()
 * retain@/link</code>,
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/release/virtualvoid/()
 * release()@/link</code>,
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/release/virtualvoid/(int)
 * release(int freeWhen)@/link</code>
 * and
 *<code> @link
 * //apple_ref/cpp/instm/OSObject/free/virtualvoid/()
 * free@/link</code>
 * functions.
 * The public interface to the reference counting is
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/retain/virtualvoid/()
 * retain@/link</code>,
 * and
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/release/virtualvoid/()
 * release@/link</code>;
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/release/virtualvoid/(int)
 * release(int freeWhen)@/link</code>
 * is provided
 * for objects that have internal retain cycles.
 *
 * In general, a subclass is expected to only override
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/free/virtualvoid/()
 * free@/link</code>.
 * It may also choose to override
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/release/virtualvoid/(int)
 * release(int freeWhen)@/link</code>
 * if the object has a circular retain count, as noted above.
 *
 * <b>Use Restrictions</b>
 *
 * With very few exceptions in the I/O Kit, all Libkern-based C++
 * classes, functions, and macros are <b>unsafe</b>
 * to use in a primary interrupt context.
 * Consult the I/O Kit documentation related to primary interrupts
 * for more information.
 *
 * <b>Concurrency Protection</b>
 *
 * The basic features of OSObject are thread-safe.
 * Most Libkern subclasses are not, and require locking or other protection
 * if instances are shared between threads.
 * I/O Kit driver objects are either designed for use within thread-safe contexts
 * or designed to inherently be thread-safe.
 * Always check the individual class documentation to see what
 * steps are necessary for concurrent use of instances.
 */
class OSObject : public OSMetaClassBase
{
	OSDeclareAbstractStructorsWithDispatch(OSObject);

#if IOKITSTATS
	friend class IOStatistics;
#endif

#ifdef LIBKERN_SMART_POINTERS
	template<class T, class OSPtrPolicy>
	friend class os::smart_ptr;
#endif

private:
/* Not to be included in headerdoc.
 *
 * @var retainCount Number of references held on this instance.
 */
	mutable int retainCount;

protected:

// xx-review: seems not to be used, should we deprecate?

/*!
 * @function release
 *
 * @abstract
 * Releases a reference to an object,
 * freeing it immediately if the reference count
 * drops below the specified threshold.
 *
 * @param freeWhen If decrementing the reference count makes it
 *                 < <code>freeWhen</code>, the object is immediately freed.
 *
 * @discussion
 * If the receiver has fewer than <code>freeWhen</code> references
 * after its reference count is decremented,
 * it is immediately freed.
 *
 * This version of <code>release</code>
 * can be used to break certain retain cycles in object graphs.
 * In general, however, it should be avoided.
 */
	virtual void release(int freeWhen) const APPLE_KEXT_OVERRIDE;

/*!
 * @function taggedRelease
 *
 * @abstract
 * Releases a tagged reference to an object,
 * freeing it immediately if the reference count
 * drops below the specified threshold.
 *
 * @param tag      Used for tracking collection references.
 * @param freeWhen If decrementing the reference count makes it
 *                 < <code>freeWhen</code>, the object is immediately freed.
 *
 * @discussion
 * Kernel extensions should not use this function.
 * It is for use by OSCollection and subclasses to track
 * inclusion in collections.
 *
 * If the receiver has fewer than <code>freeWhen</code> references
 * after its reference count is decremented,
 * it is immediately freed.
 *
 * This version of <code>release</code>
 * can be used to break certain retain cycles in object graphs.
 * In general, however, it should be avoided.
 */
	virtual void taggedRelease(const void * tag, const int freeWhen) const APPLE_KEXT_OVERRIDE;


/*!
 * @function init
 *
 * @abstract
 * Initializes a newly-allocated object.
 *
 * @result
 * <code>true</code> on success, <code>false</code> on failure.
 *
 * @discussion
 * Classes derived from OSObject must override the primary init method
 * of their parent.
 * In general most implementations call
 * <code><i>super</i>::init()</code>
 * before doing local initialisation.
 * If the superclass call fails then return <code>false</code> immediately.
 * If the subclass encounters a failure then it should return <code>false</code>.
 */
	virtual bool init();


/*!
 * @function free
 *
 * @abstract
 * Deallocates/releases resources held by the object.
 *
 * @discussion
 * Classes derived from OSObject should override this function
 * to deallocate or release all dynamic resources held by the instance,
 * then call the superclass's implementation.
 *
 * <b>Caution:</b>
 * <ol>
 * <li>You can not assume that you have completed initialization
 *     before <code>free</code> is called,
 *     so be very careful in your implementation.</li>
 * <li>OSObject's implementation performs the C++ <code>delete</code>
 *     of the instance, so be sure that you call the superclass
 *     implementation <i>last</i> in your implementation.</li>
 * <li><code>free</code> must not fail;
 *     all resources must be deallocated or released on completion.</li>
 * </ol>
 */
	virtual void free();


/*!
 * @function operator delete
 *
 * @abstract
 * Frees the memory of the object itself.
 *
 * @param mem   A pointer to the object's memory.
 * @param size  The size of the object's block of memory.
 *
 * @discussion
 * Never use <code>delete</code> on objects derived from OSObject;
 * use
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/release/virtualvoid/()
 * release@/link</code>
 * instead.
 */
	static void operator delete(void * mem, size_t size);

// XXX: eventually we can flip this switch
//#ifdef LIBKERN_SMART_POINTERS
#if 0
private:
#else
public:
#endif

/*!
 * @function operator new
 *
 * @abstract
 * Allocates memory for an instance of the class.
 *
 * @param size The number of bytes to allocate
 *
 * @result
 * A pointer to block of memory if available, <code>NULL</code> otherwise.
 */
	static void * operator new(size_t size);

public:

/*!
 * @function getRetainCount
 *
 * @abstract
 * Returns the reference count of the object.
 *
 * @result
 * The reference count of the object.
 */
	virtual int getRetainCount() const APPLE_KEXT_OVERRIDE;


/*!
 * @function retain
 *
 * @abstract
 * Retains a reference to the object.
 *
 * @discussion
 * This function increments the reference count of the receiver by 1.
 * If you need to maintain a reference to an object
 * outside the context in which you received it,
 * you should always retain it immediately.
 */
	virtual void retain() const APPLE_KEXT_OVERRIDE;


/*!
 * @function release
 *
 * @abstract
 * Releases a reference to the object,
 * freeing it immediately if the reference count drops to zero.
 *
 * @discussion
 * This function decrements the reference count of the receiver by 1.
 * If the reference count drops to zero,
 * the object is immediately freed using
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/free/virtualvoid/()
 * free@/link</code>.
 */
	virtual void release() const APPLE_KEXT_OVERRIDE;


/*!
 * @function taggedRetain
 *
 * @abstract
 * Retains a reference to the object with an optional
 * tag used for reference-tracking.
 *
 * @param tag      Used for tracking collection references.
 *
 * @discussion
 * Kernel extensions should not use this function.
 * It is for use by OSCollection and subclasses to track
 * inclusion in collections.
 *
 * If you need to maintain a reference to an object
 * outside the context in which you received it,
 * you should always retain it immediately.
 */
	virtual void taggedRetain(const void * tag = NULL) const APPLE_KEXT_OVERRIDE;


/*!
 * @function taggedRelease
 *
 * @abstract
 * Releases a tagged reference to an object,
 * freeing it immediately if the reference count
 * drops to zero.
 *
 * @param tag      Used for tracking collection references.
 *
 * @discussion
 * Kernel extensions should not use this function.
 * It is for use by OSCollection and subclasses to track
 * inclusion in collections.
 */
	virtual void taggedRelease(const void * tag = NULL) const APPLE_KEXT_OVERRIDE;
// xx-review: used to say, "Remove a reference on this object with this tag, if an attempt is made to remove a reference that isn't associated with this tag the kernel will panic immediately", but I don't see that in the implementation


/*!
 * @function serialize
 *
 * @abstract
 * Overridden by subclasses to archive the receiver into the provided
 * @link //apple_ref/doc/class/OSSerialize OSSerialize@/link object.
 *
 * @param serializer The OSSerialize object.
 *
 * @result
 * <code>true</code> if serialization succeeds, <code>false</code> if not.
 *
 * @discussion
 * OSObject's implementation writes a string indicating that
 * the class of the object receiving the function call
 * is not serializable.
 * Subclasses that can meaningfully encode themselves
 * in I/O Kit-style property list XML can override this function to do so.
 * See
 * @link //apple_ref/doc/class/OSSerialize OSSerialize@/link
 * for more information.
 */
	virtual bool serialize(OSSerialize * serializer) const APPLE_KEXT_OVERRIDE;

#ifdef XNU_KERNEL_PRIVATE
#if IOTRACKING
	void trackingAccumSize(size_t size);
#endif

	bool taggedTryRetain(const void *tag) const;

	bool iterateObjects(void * refcon, bool (*callback)(void * refcon, OSObject * object));
#ifdef __BLOCKS__
	bool iterateObjects(bool (^block)(OSObject * object));
#endif /* __BLOCKS__ */

#endif /* XNU_KERNEL_PRIVATE */

// Unused Padding
	OSMetaClassDeclareReservedUnused(OSObject, 0);
	OSMetaClassDeclareReservedUnused(OSObject, 1);
	OSMetaClassDeclareReservedUnused(OSObject, 2);
	OSMetaClassDeclareReservedUnused(OSObject, 3);
	OSMetaClassDeclareReservedUnused(OSObject, 4);
	OSMetaClassDeclareReservedUnused(OSObject, 5);
	OSMetaClassDeclareReservedUnused(OSObject, 6);
	OSMetaClassDeclareReservedUnused(OSObject, 7);
	OSMetaClassDeclareReservedUnused(OSObject, 8);
	OSMetaClassDeclareReservedUnused(OSObject, 9);
	OSMetaClassDeclareReservedUnused(OSObject, 10);
	OSMetaClassDeclareReservedUnused(OSObject, 11);
	OSMetaClassDeclareReservedUnused(OSObject, 12);
	OSMetaClassDeclareReservedUnused(OSObject, 13);
	OSMetaClassDeclareReservedUnused(OSObject, 14);
	OSMetaClassDeclareReservedUnused(OSObject, 15);
};

#endif /* !_LIBKERN_OSOBJECT_H */
