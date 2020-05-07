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
#ifndef _LIBKERN_OSMETACLASS_H
#define _LIBKERN_OSMETACLASS_H

#include <sys/types.h>

#include <libkern/OSReturn.h>
#include <kern/debug.h>
#include <ptrauth.h>

/*
 * LIBKERN_ macros below can be used to describe the ownership semantics
 * of functions handling subclasses of OSObject.
 * The attributes propagate with inheritance, but can be overriden.
 * New versions of the Clang Static Analyzer can use this knowledge to
 * check the code for leaks or uses-after-free.
 */

/*
 * By default, methods returning OSObjects are assumed to have the following
 * owneship semantics:
 *   - Methods which start with "get" are "Get" and which are not returning
 *  a subclass of OSIterator are assumed to be getters.
 *  They return at "+0" and the caller is not responsible for releasing the
 *  returned object.
 *
 *  - All other methods are assumed to return at "+1", and the caller is
 *  responsible for releasing the returned object.
 *
 *  The semantics implied by the naming convention described above can be
 *  overriden using either LIBKERN_RETURNS_RETAINED or LIBKERN_RETURNS_NOT_RETAINED
 *  attribute applied to a function.
 *  In the former case, it stipulates that the function is returning at "+1",
 *  and in the latter case "+0".
 *
 *  LIBKERN_RETURNS_RETAINED and LIBKERN_RETURNS_NOT_RETAINED attributes
 *  can be also applied to out parameters, in which case they specify
 *  that an out parameter is written into at +1 or +0 respectively.
 *  For out parameters of non-void functions an assumption is
 *  that an out parameter is written into iff the return value is non-zero
 *  unless the function returns a typedef to kern_return_t,
 *  in which case it is assumed to be written into on zero value
 *  (kIOReturnSuccess).
 *  This can be customized using the attributes
 *  LIBKERN_RETURNS_RETAINED_ON_ZERO and LIBKERN_RETURNS_RETAINED_ON_NONZERO.
 */
#if __has_attribute(os_returns_retained)
#define LIBKERN_RETURNS_RETAINED __attribute__((os_returns_retained))
#else
#define LIBKERN_RETURNS_RETAINED
#endif
#if __has_attribute(os_returns_not_retained)
#define LIBKERN_RETURNS_NOT_RETAINED __attribute__((os_returns_not_retained))
#else
#define LIBKERN_RETURNS_NOT_RETAINED
#endif

/*
 * LIBKERN_CONSUMED attribute can be applied to parameters.
 * It specifies that this function call would consume the reference to the
 * annotated parameter.
 */
#if __has_attribute(os_consumed)
#define LIBKERN_CONSUMED __attribute__((os_consumed))
#else
#define LIBKERN_CONSUMED
#endif

/*
 * LIBKERN_CONSUMES_THIS attribute can be applied to methods.
 * It specifies that this method call consumes a reference to "this" (e.g.
 * by storing a reference to "this" in a passed parameter).
 */
#if __has_attribute(os_consumes_this)
#define LIBKERN_CONSUMES_THIS __attribute__((os_consumes_this))
#else
#define LIBKERN_CONSUMES_THIS
#endif

/*
 * LIBKERN_RETURNS_RETAINED_ON_ZERO is an attribute applicable to out
 * parameters.
 * It specifies that an out parameter at +1 is written into an argument iff
 * the function returns a zero return value.
 */
#if __has_attribute(os_returns_retained_on_zero)
#define LIBKERN_RETURNS_RETAINED_ON_ZERO __attribute__((os_returns_retained_on_zero))
#else
#define LIBKERN_RETURNS_RETAINED_ON_ZERO
#endif

/*
 * LIBKERN_RETURNS_RETAINED_ON_NON_ZERO is an attribute applicable to out
 * parameters.
 * It specifies that an out parameter at +1 is written into an argument iff
 * the function returns a non-zero return value.
 */
#if __has_attribute(os_returns_retained_on_non_zero)
#define LIBKERN_RETURNS_RETAINED_ON_NONZERO __attribute__((os_returns_retained_on_non_zero))
#else
#define LIBKERN_RETURNS_RETAINED_ON_NONZERO
#endif

class OSMetaClass;
class OSObject;
class OSString;
class OSSymbol;
class OSDictionary;
class OSSerialize;
#ifdef XNU_KERNEL_PRIVATE
class OSOrderedSet;
class OSCollection;
#endif /* XNU_KERNEL_PRIVATE */
struct IORPC;
class OSInterface
{
};

/*!
 * @header
 *
 * @abstract
 * This header declares the OSMetaClassBase and OSMetaClass classes,
 * which together form the basis of the Libkern and I/O Kit C++ class hierarchy
 * and run-time type information facility.
 */


/*! @parseOnly */
#define APPLE_KEXT_COMPATIBILITY

#ifdef XNU_KERNEL_PRIVATE

#ifdef CONFIG_EMBEDDED
#define APPLE_KEXT_VTABLE_PADDING   0
#else /* CONFIG_EMBEDDED */
/*! @parseOnly */
#define APPLE_KEXT_VTABLE_PADDING   1
#endif /* CONFIG_EMBEDDED */

#else /* XNU_KERNEL_PRIVATE */
#include <TargetConditionals.h>

#if (TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR)
#define APPLE_KEXT_VTABLE_PADDING   0
#else /* (TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR) */
/*! @parseOnly */
#define APPLE_KEXT_VTABLE_PADDING   1
#endif /* (TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR) */

#endif /* XNU_KERNEL_PRIVATE */

#define APPLE_KEXT_ALIGN_CONTAINERS     (0 == APPLE_KEXT_VTABLE_PADDING)

#if defined(__LP64__)
/*! @parseOnly */
#define APPLE_KEXT_LEGACY_ABI  0
#elif defined(__arm__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2))
#define APPLE_KEXT_LEGACY_ABI  0
#else
#define APPLE_KEXT_LEGACY_ABI  1
#endif

#if defined(__LP64__)
/*! @parseOnly */
#define APPLE_KEXT_COMPATIBILITY_VIRTUAL
#else
// private method made virtual only for binary compatibility
#define APPLE_KEXT_COMPATIBILITY_VIRTUAL  virtual
#endif

/*! @parseOnly */
#define APPLE_KEXT_DEPRECATED  __attribute__((deprecated))


/*
 *  <rdar://problem/44872498> AppleUSBAudio builds xnu's libkern headers in user space
 */
#if !defined(BUILD_FOR_USERSPACE) && (__cplusplus >= 201103L)
#define APPLE_KEXT_OVERRIDE                             override
#if defined(__LP64__)
#define APPLE_KEXT_COMPATIBILITY_OVERRIDE
#else
#define APPLE_KEXT_COMPATIBILITY_OVERRIDE       APPLE_KEXT_OVERRIDE
#endif
#else
#define APPLE_KEXT_OVERRIDE
#define APPLE_KEXT_COMPATIBILITY_OVERRIDE
#endif

#define APPLE_KEXT_WSHADOW_PUSH _Pragma("clang diagnostic push") \
	_Pragma("clang diagnostic ignored \"-Wunknown-warning-option\"") \
	_Pragma("clang diagnostic ignored \"-Wshadow-field\"")

#define APPLE_KEXT_WSHADOW_POP _Pragma("clang diagnostic pop")


/*!
 * @class OSMetaClassBase
 *
 * @abstract
 * OSMetaClassBase is the abstract bootstrap class
 * for the Libkern and I/O Kit run-time type information system.
 *
 * @discussion
 * OSMetaClassBase is the abstract C++ root class
 * underlying the entire Libkern and I/O Kit class hierarchy.
 * It defines the run-time type information system,
 * including dynamic class allocation and safe type-casting,
 * as well as the abstract interface for reference counting
 * and a few other utility functions.
 * OSMetaClassBase is the immediate superclass of
 * @link //apple_ref/doc/class/OSObject OSObject@/link and
 * @link //apple_ref/doc/class/OSMetaClass OSMetaClass@/link;
 * no other class should derive from OSMetaClassBase.
 *
 * For more information, see
 * <i>@link //apple_ref/doc/uid/TP40002799
 * I/O Kit Device Driver Design Guidelines@/link</i>.
 *
 * <b>Use by Kernel Extensions</b>
 *
 * Kernel Extensions should never interact directly with OSMetaClassBase,
 * but they will find useful several macros that tie in
 * to the run-time type information system, specifically:
 * <ul>
 * <li><code>@link OSTypeAlloc OSTypeAlloc@/link</code> - allocation of new instances</li>
 * <li><code>@link OSDynamicCast OSDynamicCast@/link</code> - safe type casting</li>
 * <li><code>@link OSCheckTypeInst OSCheckTypeInst@/link</code> -
 *     checking for inheritance/derivation</li>
 * <li><code>@link OSMemberFunctionCast OSMemberFunctionCast@/link</code> -
 *     casting C++ member functions to C function pointers
 *     for registration as callbacks</li>
 * </ul>
 *
 * See @link //apple_ref/doc/class/OSMetaClass OSMetaClass@/link
 * for more run-time type information interfaces.
 *
 * <b>Use Restrictions</b>
 *
 * OSMetaClassBase should not be subclassed by kernel extensions,
 * nor should kernel extensions call its run-time type functions directly.
 *
 * The run-time type functions and macros are <b>not safe</b>
 *  to call in a primary interrupt context.
 *
 * <b>Concurrency Protection</b>
 *
 * The run-time type macros and functions of OSMetaClassBase are thread-safe.
 */

class OSMetaClassBase
{
public:


/*!
 * @define OSTypeAlloc
 * @hidecontents
 *
 * @abstract
 * Allocates an instance of the named object class.
 *
 * @param type    The name of the desired class to be created,
 *                as a raw token, <i>not</i> a string or macro.
 *
 * @result
 * A pointer to the new, uninitialized object on success;
 * <code>NULL</code> on failure.
 *
 * @discussion
 * See also
 * <code>@link
 * //apple_ref/cpp/clm/OSMetaClass/allocClassWithName/staticOSObject*\/(constchar*)
 * OSMetaClass::allocClassWithName(const char *)@/link</code>
 * and
 * <code>@link
 * //apple_ref/cpp/instm/OSMetaClass/alloc/virtualOSObject*\/()
 * OSMetaClass::alloc@/link</code>.
 *
 * The OSTypeAlloc macro is used to avoid binary compatibility difficulties
 * presented by the C++ <code>new</code> operator.
 */
#define OSTypeAlloc(type)   ((type *) ((type::metaClass)->alloc()))


/*!
 * @define OSTypeID
 * @hidecontents
 *
 * @abstract
 * Returns the type ID (metaclass) of a class based on its name.
 *
 * @param type    The name of the desired class, as a raw token,
 *                <i>not</i> a string or macro.
 *
 * @result
 * The unique type ID (metaclass) for the class.
 *
 * @discussion
 * It is typically more useful to determine whether a class is derived
 * from another; see
 * <code>@link //apple_ref/cpp/macro/OSDynamicCast OSDynamicCast@/link</code>
 * and
 * <code>@link //apple_ref/cpp/macro/OSCheckTypeInst OSCheckTypeInst@/link</code>.
 */
#define OSTypeID(type)   (type::metaClass)
#define OSMTypeID(type)  ((OSMetaClass *) type::metaClass)


/*!
 * @define OSTypeIDInst
 * @hidecontents
 *
 * @abstract
 * Returns the type ID (metaclass) for the class of an object instance.
 *
 * @param typeinst An instance of an OSObject subclass.
 *
 * @result
 * The type ID of that object's class; that is, its metaclass.
 *
 * @discussion
 * It is typically more useful to determine whether an object is derived
 * from a particular class; see
 * <code>@link //apple_ref/cpp/macro/OSDynamicCast OSDynamicCast@/link</code>
 * and
 * <code>@link //apple_ref/cpp/macro/OSCheckTypeInst OSCheckTypeInst@/link</code>.
 */
#define OSTypeIDInst(typeinst)   ((typeinst)->getMetaClass())


/*!
 * @define OSDynamicCast
 * @hidecontents
 *
 * @abstract
 * Safe type-casting for Libkern C++ objects.
 *
 * @param type    The name of the desired class type, as a raw token,
 *                <i>not</i> a string or macro.
 *                It is assumed you intend to cast to a pointer
 *                to an object of this type.
 *                Type qualifiers, such as <code>const</code>,
 *                are not recognized and will cause
 *                a (usually obscure) compile error.
 * @param inst    A pointer to the object instance to be cast.
 *                May be <code>NULL</code>.
 *
 * @result
 * <code>inst</code> if it is non-<code>NULL</code>
 * and derived from <code>type</code>;
 * otherwise <code>NULL</code>.
 *
 * @discussion
 * <code>OSDynamicCast</code> is a rough equivalent
 * to the standard C++ RTTI <code>dynamic_cast&lt;T&gt;</code> operator.
 * Your code should use this instead of raw C type-casting,
 * and check the resulting value.
 * If the result is non-<code>NULL</code>,
 * the object is safe to use as the type-cast class;
 * if the result is <code>NULL</code>,
 * the object does not derive from the type-cast class
 * and your code should take appropriate steps to handle the error.
 */
#define OSDynamicCast(type, inst)   \
    ((type *) OSMetaClassBase::safeMetaCast((inst), OSTypeID(type)))

/*!
 * @define OSRequiredCast
 * @hidecontents
 *
 * @abstract
 * Safe type-casting for Libkern C++ objects; panics on failure.
 * The input parameters are the same as for the {@code OSDynamicCast} macro.
 *
 * @result {@code inst} if it is NULL or derived from {@code type};
 * otherwise triggers a kernel panic.
 *
 * @discussion
 * This macro should be used in place of C-style casts or
 * <code>@link OSDynamicCast OSDynamicCast@/link</code>.
 * when the caller is absolutely sure that the passed
 * argument is a subclass of a required type.
 * It is equivalent to using {@code OSDynamicCast} and crashing with a kernel
 * panic on cast failure.
 */
#define OSRequiredCast(type, inst)  \
    (type *) OSMetaClassBase::requiredMetaCast((inst), OSTypeID(type))

/*!
 * @define OSCheckTypeInst
 * @hidecontents
 *
 * @abstract
 * Checks whether two objects are type-compatible.
 *
 * @param typeinst The reference object.
 * @param inst     The object to check for type compatibility.
 *
 * @result
 * <code>true</code> if both <code>inst</code> and
 * <code>typeinst</code> are non-<code>NULL</code>
 * and <code>inst</code> is derived from the class of <code>typeinst</code>;
 * otherwise <code>false</code>.
 */
#define OSCheckTypeInst(typeinst, inst) \
    OSMetaClassBase::checkTypeInst(inst, typeinst)

#define OSSafeRelease(inst) \
  do { int OSSafeRelease __attribute__ ((deprecated("Use OSSafeReleaseNULL"))); (OSSafeRelease); \
	if (inst) (inst)->release(); } while (0)

/*! @function OSSafeReleaseNULL
 *  @abstract Release an object if not <code>NULL</code>, then set it to <code>NULL</code>.
 *  @param    inst  Instance of an OSObject, may be <code>NULL</code>.
 */
#define OSSafeReleaseNULL(inst)   do { if (inst != NULL) (inst)->release(); (inst) = NULL; } while (0)

	typedef void (*_ptf_t)(void);

#if defined(__arm__) || defined(__arm64__)

	static _ptf_t _ptmf2ptf(const OSMetaClassBase * self, void (OSMetaClassBase::*func)(void), uintptr_t typeDisc);

#elif defined(__i386__) || defined(__x86_64__)

// Slightly less arcane and slightly less evil code to do
// the same for kexts compiled with the standard Itanium C++
// ABI

	static inline _ptf_t
	_ptmf2ptf(const OSMetaClassBase *self, void (OSMetaClassBase::*func)(void),
	    uintptr_t typeDisc __attribute__((unused)))
	{
		union {
			void (OSMetaClassBase::*fIn)(void);
			uintptr_t fVTOffset;
			_ptf_t fPFN;
		} map;

		map.fIn = func;

		if (map.fVTOffset & 1) {
			// virtual
			union {
				const OSMetaClassBase *fObj;
				_ptf_t **vtablep;
			} u;
			u.fObj = self;

			// Virtual member function so dereference vtable
			return *(_ptf_t *)(((uintptr_t)*u.vtablep) + map.fVTOffset - 1);
		} else {
			// Not virtual, i.e. plain member func
			return map.fPFN;
		}
	}

#else
#error Unknown architecture.
#endif /* __arm__ */


/*!
 * @define OSMemberFunctionCast
 * @hidecontents
 *
 * @abstract
 * Converts a C++ member function pointer, relative to an instance,
 * to a C-style pointer to function.
 *
 * @param cptrtype The function type declaration to cast to
 *                 (typically provided as a <code>typedef</code> by  I/O KitKit classes).
 * @param self     The <code>this</code> pointer of the object whose function
 *                 you wish to cache.
 * @param func     The pointer to the member function itself,
 *                 something like <code>&Class::function</code>.
 *
 * @result
 * A pointer to a function of the given type referencing <code>self</code>.
 *
 * @discussion
 * This function is used to generate pointers to C++ functions for instances,
 * such that they can be registered as callbacks with I/O Kit objects.
 *
 * No warnings are generated.
 *
 * This function will panic if an attempt is made to call it
 * with a multiply-inheriting class.
 */
#define OSMemberFunctionCast(cptrtype, self, func)         \
    (cptrtype) OSMetaClassBase::                           \
	_ptmf2ptf(self, (void (OSMetaClassBase::*)(void)) func,  \
	          ptrauth_type_discriminator(__typeof__(func)))

protected:
	OSMetaClassBase();
	virtual
	~OSMetaClassBase();

private:
// Disable copy constructors of OSMetaClassBase based objects
/* Not to be included in headerdoc.
 *
 * @function operator =
 *
 * @abstract
 * Disable implicit copy constructor by making private
 *
 * @param src Reference to source object that isn't allowed to be copied.
 */
	void operator =(OSMetaClassBase &src);

/* Not to be included in headerdoc.
 *
 * @function OSMetaClassBase
 *
 * @abstract
 * Disable implicit copy constructor by making private
 *
 * @param src Reference to source object that isn't allowed to be copied.
 */
	OSMetaClassBase(OSMetaClassBase &src);

public:

// xx-review: the original comment for this makes it sound to me like we don't
// xx-review: catch over-releasing an object...?

/*!
 * @function release
 *
 * @abstract
 * Abstract declaration of
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/release/virtualvoid/(int)
 * release(int freeWhen)@/link</code>.
 *
 * @discussion
 * See
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/release/virtualvoid/(int)
 * release(int freeWhen)@/link</code>.
 */
	virtual void release(int freeWhen) const = 0;


/*!
 * @function getRetainCount
 *
 * @abstract
 * Abstract declaration of
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/getRetainCount/virtualint/()
 * getRetainCount()@/link</code>.
 *
 * @discussion
 * See
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/getRetainCount/virtualint/()
 * OSObject::getRetainCount()@/link</code>.
 */
	virtual int getRetainCount() const = 0;


/*!
 * @function retain
 *
 * @abstract
 * Abstract declaration of
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/retain/virtualvoid/()
 * retain()@/link</code>.
 *
 * @discussion
 * See
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/retain/virtualvoid/()
 * OSObject::retain()@/link</code>.
 */
	virtual void retain() const = 0;


/*!
 * @function release
 *
 * @abstract
 * Abstract declaration of
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/release/virtualvoid/()
 * release@/link</code>.
 *
 * @discussion
 * See
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/release/virtualvoid/()
 * OSObject::release@/link</code>.
 */
	virtual void release() const = 0;


/*!
 * @function serialize
 *
 * @abstract
 * Abstract declaration of
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/serialize/virtualbool/(OSSerialize*)
 * serialize@/link</code>.
 *
 * @discussion
 * See
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/serialize/virtualbool/(OSSerialize*)
 * OSObject::serialize@/link</code>.
 */
	virtual bool serialize(OSSerialize * serializer) const = 0;


/*!
 * @function getMetaClass
 *
 * @abstract
 * Returns the OSMetaClass representing
 * an OSMetaClassBase subclass.
 *
 * @discussion
 * OSObject overrides this abstract member function
 * to return the OSMetaClass object that represents
 * each class for run-time typing.
 */
	virtual const OSMetaClass * getMetaClass() const = 0;


/*!
 * @function isEqualTo
 *
 * @abstract
 * Checks whether another object is equal to the receiver.
 *
 * @param anObject The object to copmare to the receiver.
 *
 * @result
 * <code>true</code> if the objects are equal, <code>false</code> otherwise.
 *
 * @discussion
 * OSMetaClassBase implements this as a direct pointer comparison,
 * since it has no other information to judge equality by.
 * Subclasses generally override this function
 * to do a more meaningful comparison.
 * For example, OSString implements it to return
 * <code>true</code> if <code>anObject</code>
 * is derived from OSString and represents the same C string.
 */
	virtual bool isEqualTo(const OSMetaClassBase * anObject) const;


/*!
 * @function metaCast
 *
 * @abstract
 * Casts this object is to the class managed by the given OSMetaClass.
 *
 * @param toMeta A pointer to a constant OSMetaClass
 *               for the desired target type.
 *
 * @result
 * <code>this</code> if the object is derived
 * from the class managed by <code>toMeta</code>,
 * otherwise <code>NULL</code>.
 *
 * @discussion
 * It is far more convenient to use
 * <code>@link OSDynamicCast OSDynamicCast@/link</code>.
 */
	OSMetaClassBase * metaCast(const OSMetaClass * toMeta) const;


/*!
 * @function metaCast
 *
 * @abstract
 * Casts this object is to the class managed by the named OSMetaClass.
 *
 * @param toMeta An OSSymbol naming the desired target type.
 *
 * @result
 * <code>this</code> if the object is derived
 * from the class named by <code>toMeta</code>,
 * otherwise <code>NULL</code>.
 *
 * @discussion
 * It is far more convenient to use
 * <code>@link OSDynamicCast OSDynamicCast@/link</code>.
 */
	OSMetaClassBase * metaCast(const OSSymbol * toMeta) const;


/*!
 * @function metaCast
 *
 * @abstract
 * Casts this object is to the class managed by the named OSMetaClass.
 *
 * @param toMeta An OSString naming the desired target type.
 * @result
 * <code>this</code> if the object is derived
 * from the class named by <code>toMeta</code>,
 * otherwise <code>NULL</code>.
 *
 * @discussion
 * It is far more convenient to use
 * <code>@link OSDynamicCast OSDynamicCast@/link</code>.
 */
	OSMetaClassBase * metaCast(const OSString * toMeta) const;


/*!
 * @function metaCast
 *
 * @abstract
 * Casts this object is to the class managed by the named OSMetaClass.
 *
 * @param toMeta A C string naming the desired target type.
 * @result
 * <code>this</code> if the object is derived
 * from the class named by <code>toMeta</code>,
 * otherwise <code>NULL</code>.
 *
 * @discussion
 * It is far more convenient to use
 * <code>@link OSDynamicCast OSDynamicCast@/link</code>.
 */
	OSMetaClassBase * metaCast(const char * toMeta) const;

// Helper inlines for run-time type preprocessor macros
/*!
 * @function safeMetaCast
 *
 * @abstract
 * Casts an object is to the class managed by the given OSMetaClass.
 *
 * @param anObject A pointer to the object to be cast.
 * @param toMeta   A pointer to a constant OSMetaClass
 *                 for the desired target type.
 *
 * @result
 * <code>anObject</code> if the object is derived
 * from the class managed by <code>toMeta</code>,
 * otherwise <code>NULL</code>.
 *
 * @discussion
 * It is far more convenient to use
 * <code>@link OSDynamicCast OSDynamicCast@/link</code>.
 */
	static OSMetaClassBase * safeMetaCast(
		const OSMetaClassBase * anObject,
		const OSMetaClass     * toMeta);

/*!
 * @function requiredMetaCast
 *
 * @abstract
 * Casts an object to the class managed by the given OSMetaClass or
 * fails with a kernel panic if the cast does not succeed.
 *
 * @param anObject A pointer to the object to be cast.
 * @param toMeta   A pointer to a constant OSMetaClass
 *                 for the desired target type.
 *
 * @result
 * <code>anObject</code> if the object is derived
 * from the class managed by <code>toMeta</code>,
 * <code>NULL</code> if <code>anObject</code> was <code>NULL</code>,
 * kernel panic otherwise.
 *
 * @discussion
 * It is far more convenient to use
 * <code>@link OSRequiredCast OSRequiredCast@/link</code>.
 */
	static OSMetaClassBase *requiredMetaCast(
		const OSMetaClassBase * anObject,
		const OSMetaClass     * toMeta);

/*!
 * @function checkTypeInst
 *
 * @abstract
 * Checks whether an object instance is of the same class
 * as another object instance (or a subclass of that class).
 *
 * @param inst       A pointer to the object to check.
 * @param typeinst   A pointer to an object of the class being checked.
 *
 * @result
 * <code>true</code> if the object is derived
 * from the class of <code>typeinst</code>
 * or a subclass of that class,
 * otherwise <code>false</code>.
 *
 * @discussion
 * It is far more convenient to use
 * <code>@link OSCheckTypeInst OSCheckTypeInst@/link</code>.
 */
	static bool checkTypeInst(
		const OSMetaClassBase * inst,
		const OSMetaClassBase * typeinst);

	static void initialize(void);

public:

/*!
 * @function taggedRetain
 *
 * @abstract
 * Abstract declaration of
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/taggedRetain/virtualvoid/(constvoid*)
 * taggedRetain(const void *)@/link</code>.
 *
 * @discussion
 * See
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/taggedRetain/virtualvoid/(constvoid*)
 * OSObject::taggedRetain(const void *)@/link</code>.
 */
// WAS: virtual void _RESERVEDOSMetaClassBase0();
	virtual void taggedRetain(const void * tag = NULL) const = 0;


/*!
 * @function taggedRelease
 *
 * @abstract
 * Abstract declaration of
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/taggedRelease/virtualvoid/(constvoid*)
 * taggedRelease(const void *)@/link</code>.
 *
 * @discussion
 * See
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/taggedRelease/virtualvoid/(constvoid*)
 * OSObject::taggedRelease(const void *)@/link</code>.
 */
// WAS:  virtual void _RESERVEDOSMetaClassBase1();
	virtual void taggedRelease(const void * tag = NULL) const = 0;

protected:
/*!
 * @function taggedRelease
 *
 * @abstract
 * Abstract declaration of
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/taggedRelease/virtualvoid/(constvoid*,constint)
 * taggedRelease(const void *, const int freeWhen)@/link</code>.
 *
 * @discussion
 * See
 * <code>@link
 * //apple_ref/cpp/instm/OSObject/taggedRelease/virtualvoid/(constvoid*,constint)
 * OSObject::taggedRelease(const void *, const int freeWhen)@/link</code>.
 */
// WAS:  virtual void _RESERVEDOSMetaClassBase2();
	virtual void taggedRelease(
		const void * tag,
		const int    freeWhen) const = 0;

public:
	virtual kern_return_t
	Dispatch(const IORPC rpc);

	kern_return_t
	Invoke(const IORPC rpc);

private:
#if APPLE_KEXT_VTABLE_PADDING
// Virtual Padding
	virtual void _RESERVEDOSMetaClassBase4();
	virtual void _RESERVEDOSMetaClassBase5();
	virtual void _RESERVEDOSMetaClassBase6();
	virtual void _RESERVEDOSMetaClassBase7();
#endif
} APPLE_KEXT_COMPATIBILITY;


#ifdef XNU_KERNEL_PRIVATE
typedef bool (*OSMetaClassInstanceApplierFunction)(const OSObject * instance,
    void * context);
#endif /* XNU_KERNEL_PRIVATE */

/*!
 * @class OSMetaClass
 *
 * @abstract
 * OSMetaClass manages run-time type information
 * for Libkern and I/O Kit C++ classes.
 *
 * @discussion OSMetaClass manages run-time type information
 * for Libkern and I/O Kit C++ classes.
 * An instance of OSMetaClass exists for (nearly) every such C++ class,
 * keeping track of inheritance relationships, class lookup by name,
 * instance counts, and more.
 * OSMetaClass operates almost entirely behind the scenes,
 * and kernel extensions should rarely, if ever,
 * have to interact directly with OSMetaClass.
 *
 * <b>Use by Kernel Extensions</b>
 *
 * While kernel extensions rarey interact directly with OSMetaClass at run time,
 * they must register their classes with the metaclass system
 * using the macros declared here.
 * The class declaration should use one of these two macros
 * before its first member function declaration:
 * <ul>
 * <li><code>@link OSDeclareDefaultStructors OSDeclareDefaultStructors@/link</code> -
 *     for classes with no abstract member function declarations</li>
 * <li><code>@link OSDeclareAbstractStructors OSDeclareAbstractStructors@/link</code> -
 *     for classes with at least one abstract member function declaration</li>
 * <li><code>@link OSDeclareFinalStructors OSDeclareFinalStructors@/link</code> -
 *     for classes that should not be subclassable by another kext</li>
 * </ul>
 *
 * The class implementation should then use one of these macros:
 * <ul>
 * <li><code>@link OSDefineMetaClassAndStructors
 *           OSDefineMetaClassAndStructors@/link</code> -
 *     for classes with no abstract member function declarations</li>
 * <li><code>@link OSDefineMetaClassAndAbstractStructors
 *           OSDefineMetaClassAndAbstractStructors@/link</code> -
 *     for classes with at least one abstract member function declaration</li>
 * <li><code>@link OSDefineMetaClassAndFinalStructors
 *           OSDefineMetaClassAndFinalStructors@/link</code> -
 *     for classes that should not be subclassable by another kext</li>
 * </ul>
 *
 * Classes in kernel extensions that are intended for use as libraries
 * may need to reserve vtable slots to preserve binary compatibility
 * as new functions are added. They may do so with these macros:
 * <ul>
 * <li><code>@link OSMetaClassDeclareReservedUnused
 *           OSMetaClassDeclareReservedUnused@/link</code> -
 *     reserves a vtable slot</li>
 * <li><code>@link OSMetaClassDefineReservedUnused
 *           OSMetaClassDefineReservedUnused@/link</code> -
 *     defines the reserved vtable slot as an unimplemented function</li>
 * <li><code>@link OSMetaClassDeclareReservedUsed
 *           OSMetaClassDeclareReservedUsed@/link</code> -
 *     documents that a formerly reserved slot is now used</li>
 * <li><code>@link OSMetaClassDefineReservedUsed
 *           OSMetaClassDefineReservedUsed@/link</code> -
 *    documents that a formerly reserved slot is now used</li>
 * </ul>
 *
 * <b>Use Restrictions</b>
 *
 * OSMetaClass should not be explicitly subclassed by kernel extensions
 * (the declare/define macros do that),
 * nor should kernel extensions call its run-time type functions directly.
 *
 * OSMetaClass functions should be considered
 * <b>unsafe</b> to call in a primary interrupt context.
 *
 * <b>Concurrency Protection</b>
 *
 * Kernel extensions should in general not interact
 * with OSMetaClass objects directly,
 * instead using the run-time type macros.
 * Much of OSMetaClass's interface is intended for use
 * by the run-time type information system,
 * which handles concurrency and locking internally.
 */
class OSMetaClass : public OSMetaClassBase
{
	friend class OSKext;
#if IOKITSTATS
	friend class IOStatistics;
#endif

private:
// Can never be allocated must be created at compile time
	static void * operator new(size_t size);

/* Reserved for future use.  (Internal use only) */
	struct ExpansionData *reserved;

/* superClass Handle to the superclass's meta class. */
	const OSMetaClass *superClassLink;

/* className OSSymbol of the class' name. */
	const OSSymbol *className;

/* classSize How big is a single instance of this class. */
	unsigned int classSize;

/* instanceCount Roughly number of instances of the object,
 * +1 for each direct subclass with a nonzero refcount.
 * Used primarily as a code-in-use flag.
 */
	mutable unsigned int instanceCount;

/* Not to be included in headerdoc.
 *
 * @function OSMetaClass
 *
 * @abstract
 * The default private constructor.
 */
	OSMetaClass();

// Called by postModLoad
/* Not to be included in headerdoc.
 *
 * @function logError
 *
 * @abstract
 * Logs an error string for an <code>OSReturn</code> value
 * using <code>printf</code>.
 *
 * @param result  The <code>OSReturn</code> value for which to log a message.
 *
 * @discussion
 * This function is used to log errors loading kernel extensions.
 * Kernel extensions themselves should not call it.
 */
	static void logError(OSReturn result);

public:

/*!
 * @function getMetaClassWithName
 *
 * @abstract
 * Look up a metaclass in the run-time type information system.
 *
 * @param name The name of the desired class's metaclass.
 *
 * @result
 * A pointer to the metaclass object if found, <code>NULL</code> otherwise.
 */
	static const OSMetaClass * getMetaClassWithName(const OSSymbol * name);

#if XNU_KERNEL_PRIVATE

/*!
 * @function copyMetaClassWithName
 *
 * @abstract
 * Look up a metaclass in the run-time type information system.
 *
 * @param name The name of the desired class's metaclass.
 *
 * @result
 * A pointer to the metaclass object if found, <code>NULL</code> otherwise.
 * The metaclass will be protected from unloading until releaseMetaClass()
 * is called.
 */
	static const OSMetaClass * copyMetaClassWithName(const OSSymbol * name);
/*!
 * @function releaseMetaClass
 *
 * @abstract
 * Releases reference obtained from copyMetaClassWithName().
 *
 * @discussion
 * The metaclass will be protected from unloading until releaseMetaClass()
 * is called.
 */
	void releaseMetaClass() const;

#endif /* XNU_KERNEL_PRIVATE */

protected:
/*!
 * @function retain
 *
 * @abstract
 * Implements the abstract <code>retain</code> function to do nothing.
 *
 * @discussion
 * Since an OSMetaClass instance must remain in existence
 * for as long as its kernel extension is loaded,
 * OSMetaClass does not use reference-counting.
 */
	virtual void retain() const;


/*!
 * @function release
 *
 * @abstract
 * Implements the abstract <code>release</code> function to do nothing.
 *
 * @discussion
 * Since an OSMetaClass instance must remain in existence
 * for as long as its kernel extension is loaded,
 * OSMetaClass does not use reference-counting.
 */
	virtual void release() const;


/*!
 * @function release
 *
 * @abstract
 * Implements the abstract <code>release(int freeWhen)</code>
 * function to do nothing.
 *
 * @param freeWhen  Unused.
 *
 * @discussion
 * Since an OSMetaClass instance must remain in existence
 * for as long as its kernel extension is loaded,
 * OSMetaClass does not use reference-counting.
 */
	virtual void release(int freeWhen) const;


/*!
 * @function taggedRetain
 *
 * @abstract
 * Implements the abstract <code>taggedRetain(const void *)</code>
 * function to do nothing.
 *
 * @param tag  Unused.
 *
 * @discussion
 * Since an OSMetaClass instance must remain in existence
 * for as long as its kernel extension is loaded,
 * OSMetaClass does not use reference-counting.
 */
	virtual void taggedRetain(const void * tag = NULL) const;


/*!
 * @function taggedRelease
 *
 * @abstract
 * Implements the abstract <code>taggedRelease(const void *)</code>
 * function to do nothing.
 *
 * @param tag  Unused.
 *
 * @discussion
 * Since an OSMetaClass instance must remain in existence
 * for as long as its kernel extension is loaded,
 * OSMetaClass does not use reference-counting.
 */
	virtual void taggedRelease(const void * tag = NULL) const;


/*!
 * @function taggedRelease
 *
 * @abstract
 * Implements the abstract <code>taggedRelease(const void *, cont int)</code>
 * function to do nothing.
 *
 * @param tag       Unused.
 * @param freeWhen  Unused.
 *
 * @discussion
 * Since an OSMetaClass instance must remain in existence
 * for as long as its kernel extension is loaded,
 * OSMetaClass does not use reference-counting.
 */
	virtual void taggedRelease(
		const void * tag,
		const int    freeWhen) const;


/*!
 * @function getRetainCount
 *
 * @abstract
 * Implements the abstract <code>getRetainCount</code>
 * function to return 0.
 *
 * @result
 * Always returns 0.
 *
 * @discussion
 * Since an OSMetaClass instance must remain in existence
 * for as long as its kernel extension is loaded,
 * OSMetaClass does not use reference-counting.
 */
	virtual int getRetainCount() const;


/* Not to be included in headerdoc.
 *
 * @function getMetaClass
 *
 * @abstract
 * Returns the meta-metaclass.
 *
 * @result
 * The metaclass of the OSMetaClass object.
 */
	virtual const OSMetaClass * getMetaClass() const;


/*!
 * @function OSMetaClass
 *
 * @abstract
 * Constructor for OSMetaClass objects.
 *
 * @param className  A C string naming the C++ class
 *                   that this OSMetaClass represents.
 * @param superclass The OSMetaClass object representing the superclass
 *                   of this metaclass's class.
 * @param classSize  The allocation size of the represented C++ class.
 *
 * @discussion
 * This constructor is protected and cannot be used
 * to instantiate OSMetaClass directly, as OSMetaClass is an abstract class.
 * This function is called during kext loading
 * to queue C++ classes for registration.
 * See <code>@link preModLoad preModLoad@/link</code> and
 * <code>@link postModLoad postModLoad@/link</code>.
 */
	OSMetaClass(const char * className,
	    const OSMetaClass  * superclass,
	    unsigned int         classSize);


/*!
 * @function ~OSMetaClass
 *
 * @abstract
 * Destructor for OSMetaClass objects.
 *
 * @discussion
 * This function is called when the kernel extension that implements
 * the metaclass's class is unloaded.
 * The destructor removes all references to the class
 * from the run-time type information system.
 */
	virtual
	~OSMetaClass();

// Needs to be overriden as NULL as all OSMetaClass objects are allocated
// statically at compile time, don't accidently try to free them.
	void
	operator delete(void *, size_t)
	{
	}

public:
	static const OSMetaClass * const metaClass;

/*!
 * @function preModLoad
 *
 * @abstract
 * Prepares the run-time type system
 * for the creation of new metaclasses
 * during loading of a kernel extension (module).
 *
 * @param kextID  The bundle ID of the kext being loaded.
 *
 * @result
 * An opaque handle to the load context
 * for the kernel extension on success;
 * <code>NULL</code> on failure.
 *
 * @discussion
 * <i>Not for use by kernel extensions.</i>
 *
 * Prepares the run-time type information system to record and register
 * metaclasses created by static constructors until a subsequent call to
 * <code>@link postModLoad postModLoad@/link</code>.
 * <code>preModLoad</code> takes a lock to ensure processing of a single
 * load operation at a time; the lock is released by
 * <code>@link postModLoad postModLoad@/link</code>.
 * Any OSMetaClass constructed between these two function calls
 * will be associated with <code>kextID</code>.
 */
	static void * preModLoad(const char * kextID);


/*!
 * @function checkModLoad
 *
 * @abstract
 * Checks whether the current kext load operation can proceed.
 *
 * @param loadHandle The opaque handle returned
 *                   by <code>@link preModLoad preModLoad@/link</code>.
 * @result
 * <code>true</code> if no errors are outstanding
 * and the system is ready to process more metaclasses.
 *
 * @discussion
 * <i>Not for use by kernel extensions.</i>
 */
	static bool checkModLoad(void * loadHandle);


/*!
 * @function postModLoad
 *
 * @abstract
 * Registers the metaclasses created during loading of a kernel extension.
 *
 * @param loadHandle The opaque handle returned
 *                   by <code>@link preModLoad preModLoad@/link</code>.
 * @result
 * The error code of the first error encountered,
 * or
 * <code>@link
 * //apple_ref/cpp/macro/kOSReturnSuccess
 * kOSReturnSuccess@/link</code>
 * if no error occurred.
 *
 * @discussion
 * <i>Not for use by kernel extensions.</i>
 *
 * Called after all static constructors in a kernel extension
 * have created metaclasses,
 * this function checks for duplicate class names,
 * then registers the new metaclasses under the kext ID
 * that @link preModLoad preModLoad@/link was called with,
 * so that they can be dynamically allocated
 * and have their instance counts tracked.
 * <code>postModLoad</code> releases the lock taken by
 * <code>@link preModLoad preModLoad@/link</code>.
 */
	static OSReturn postModLoad(void * loadHandle);

/*!
 * @function modHasInstance
 *
 * @abstract
 * Returns whether any classes defined by the named
 * kernel extension (or their subclasses) have existing instances.
 *
 * @param kextID   The bundle ID of the kernel extension to check.
 *
 * @result
 * <code>true</code> if the kext is found and
 * if any class defined by that kext
 * has a nonzero instance count,
 * <code>false</code> otherwise.
 *
 * @discussion
 * This function is called before a kernel extension's static destructors
 * are invoked, prior to unloading the extension.
 * If any classes stil have instances or subclasses with instances,
 * those classes are logged
 * (using <code>@link reportModInstances reportModInstances@/link</code>) and
 * the kernel extension is not be unloaded.
 */
	static bool modHasInstance(const char * kextID);


/*!
 * @function reportModInstances
 *
 * @abstract
 * Logs the instance counts for classes
 * defined by a kernel extension.
 *
 * @param kextID   The bundle ID of the kernel extension to report on.
 *
 * @discussion
 * This function prints the names and instance counts
 * of any class defined by <code>kextID</code>
 * that has a nonzero instance count.
 * It's called by <code>@link modHasInstance modHasInstance@/link</code>
 * to help diagnose problems unloading kernel extensions.
 */
	static void reportModInstances(const char * kextID);


/*!
 * @function considerUnloads
 *
 * @abstract
 * Schedule automatic unloading of unused kernel extensions.
 *
 * @discussion
 * This function schedules a check for kernel extensions
 * that can be automatically unloaded,
 * canceling any currently scheduled check.
 * At that time, any such kexts with no Libkern C++ instances
 * and no external references are unloaded.
 *
 * The I/O Kit calls this function when matching goes idle.
 *
 * Kernel extensions that define subclasses of
 * @link //apple_ref/doc/class/IOService IOService@/link
 * are eligible for automatic unloading.
 *
 * (On releases of Mac OS X prior to Snow Leopard (10.6),
 * any kernel extension defining any Libkern C++ class
 * was eligible for automatic unloading,
 * but that unload did not call the module stop routine.
 * Non-I/O Kit kernel extensions that define Libkern C++ subclasses
 * should be sure to have OSBundleLibraries declarations that ensure
 * they will not load on releases prior to Snow Leopard.)
 */
	static void considerUnloads();

#if XNU_KERNEL_PRIVATE
	static bool removeClasses(OSCollection * metaClasses);
#endif /* XNU_KERNEL_PRIVATE */

/*!
 * @function allocClassWithName
 *
 * @abstract
 * Allocates an instance of a named OSObject-derived class.
 *
 * @param name The name of the desired class.
 *
 * @result
 * A pointer to the newly-allocated, uninitialized object on success;
 * <code>NULL</code> on failure.
 *
 * @discussion
 * Kernel extensions should not need to use this function
 * directly, instead using static instance-creation functions
 * defined by classes.
 *
 * This function consults the run-time type information system
 * to find the metaclass for the named class.
 * If it exists, it calls the metaclass's <code>@link alloc alloc@/link</code>
 * function and returns the result.
 */
	static OSObject * allocClassWithName(const OSSymbol * name);


/*!
 * function allocClassWithName
 *
 * @abstract
 * Allocates an instance of a named OSObject-derived class.
 *
 * @param name The name of the desired class.
 *
 * @result
 * A pointer to the newly-allocated, uninitialized object on success;
 * <code>NULL</code> on failure.
 *
 * @discussion
 * Kernel extensions should not need to use this function
 * directly, instead using static instance-creation functions
 * defined by classes.
 *
 * This function consults the run-time type information system
 * to find the metaclass for the named class.
 * If it exists, it calls the metaclass's <code>@link alloc alloc@/link</code>
 * function and returns the result.
 */
	static OSObject * allocClassWithName(const OSString * name);


/*!
 * function allocClassWithName
 *
 * @abstract
 * Allocates an instance of a named OSObject-derived class.
 *
 * @param name The name of the desired class.
 *
 * @result
 * A pointer to the newly-allocated, uninitialized object on success;
 * <code>NULL</code> on failure.
 *
 * @discussion
 * Kernel extensions should not need to use this function
 * directly, instead using static instance-creation functions
 * defined by classes.
 *
 * This function consults the run-time type information system
 * to find the metaclass for the named class.
 * If it exists, it calls the metaclass's <code>@link alloc alloc@/link</code>
 * function and returns the result.
 */
	static OSObject * allocClassWithName(const char * name);


/*!
 * @function checkMetaCastWithName
 *
 * @abstract
 * Search the metaclass inheritance hierarchy by name for an object instance.
 *
 * @param className The name of the desired class or superclass.
 * @param object    The object whose metaclass begins the search.
 *
 * @result
 * <code>object</code> if it's derived from <code>className</code>;
 * <code>NULL</code> otherwise.
 *
 * @discussion
 * This function is the basis of the Libkern run-time type-checking system.
 * Kernel extensions should not use it directly,
 * instead using <code>@link OSDynamicCast OSDynamicCast@/link</code> or
 * <code>@link OSCheckTypeInst OSCheckTypeInst@/link</code>.
 */
	static OSMetaClassBase * checkMetaCastWithName(
		const OSSymbol        * className,
		const OSMetaClassBase * object);

/*!
 * @function checkMetaCastWithName
 *
 * @abstract
 * Search the metaclass inheritance hierarchy by name for an object instance.
 *
 * @param className The name of the desired class or superclass.
 * @param object    The object whose metaclass begins the search.
 *
 * @result
 * <code>object</code> if it's derived from <code>className</code>;
 * <code>NULL</code> otherwise.
 *
 * @discussion
 * Kernel extensions should not use this function directly,
 * instead using <code>@link OSDynamicCast OSDynamicCast@/link</code> or
 * <code>@link OSCheckTypeInst OSCheckTypeInst@/link</code>.
 */
	static OSMetaClassBase * checkMetaCastWithName(
		const OSString        * className,
		const OSMetaClassBase * object);

/*!
 * @function checkMetaCastWithName
 *
 * @abstract
 * Search the metaclass inheritance hierarchy by name for an object instance.
 *
 * @param className The name of the desired class or superclass.
 * @param object    The object whose metaclass begins the search.
 *
 * @result
 * <code>object</code> if it's derived from <code>className</code>;
 * <code>NULL</code> otherwise.
 *
 * @discussion
 * Kernel extensions should not use this function directly,
 * instead using <code>@link OSDynamicCast OSDynamicCast@/link</code> or
 * <code>@link OSCheckTypeInst OSCheckTypeInst@/link</code>.
 */
	static OSMetaClassBase * checkMetaCastWithName(
		const char            * className,
		const OSMetaClassBase * object);


/*!
 * @function instanceConstructed
 *
 * @abstract
 * Counts the instances of the class managed by this metaclass.
 *
 * @discussion
 * <i>Not for use by kernel extensions.</i>
 *
 * Every non-abstract class that inherits from OSObject
 * has a default constructor that calls it's own metaclass's
 * <code>instanceConstructed</code> function.
 * This constructor is defined by the
 * <code>@link
 * OSDefineMetaClassAndStructors
 * OSDefineMetaClassAndStructors@/link</code>
 * macro that all OSObject subclasses must use.
 *
 * If a class's instance count goes from 0 to 1--that is,
 * upon the creation of the first instance of that class--the
 * superclass's instance count is also incremented.
 * This propagates reference counts up the inheritance chain so that
 * superclasses are counted as "in use" when subclasses have instances.
 */
	void instanceConstructed() const;


/*!
 * @function instanceDestructed
 *
 * @abstract
 * Counts the instances of the class managed by this metaclass.
 *
 * @discussion
 * Every non-abstract class that inherits from OSObject
 * has a default destructor that calls it's own metaclass's
 * <code>instanceDestructed</code> function.
 * This constructor is defined by the
 * @link OSDefineMetaClassAndStructors OSDefineMetaClassAndStructors@/link
 * macro that all OSObject subclasses must use.
 *
 * If a class's instance count goes from 1 to 0--that is,
 * upon the destruction of the last instance of that class--the
 * superclass's instance count is also decremented.
 * This reduces "in use" counts from superclasses when their subclasses
 * no longer have instances.
 */
	void instanceDestructed() const;


/*!
 * @function checkMetaCast
 *
 * @abstract
 * Check whether a given object is an instance of the receiving
 * metaclass's class or one derived from it.
 *
 * @param object The object to check for inheritance.
 *
 * @result
 * <code>object</code> if it is derived from the receiver's class,
 * <code>NULL</code> if not.
 */
	OSMetaClassBase * checkMetaCast(const OSMetaClassBase * object) const;


/*!
 * @function getInstanceCount
 *
 * @abstract
 * Returns the number of existing instances of the metaclass's class.
 *
 * @result
 * The number of existing instances of the metaclass's class,
 * plus 1 for each subclass with any instance.
 */
	unsigned int getInstanceCount() const;


/*!
 * @function getSuperClass
 *
 * @abstract
 * Returns the super-metaclass of the receiver.
 *
 * @result
 * Returns a pointer to the super-metaclass of the receiving
 * OSMetaClass, or <code>NULL</code> for OSObject's metaclass.
 */
	const OSMetaClass * getSuperClass() const;

/*!
 * @function getKmodName
 *
 * @abstract
 * Returns the bundle identifier of the kernel extension
 * that defines this metaclass.
 *
 * @result
 * The bundle identifier of the kernel extension that defines this metaclass.
 *
 * @discussion
 * "Kmod" is an older term for kernel extension.
 */
	const OSSymbol * getKmodName() const;


/*!
 * @function getClassName
 *
 * @abstract
 * Returns the name of the C++ class managed by this metaclass.
 *
 * @result
 * Returns the name of the C++ class managed by this metaclass.
 */
	const char * getClassName() const;
	const OSSymbol * getClassNameSymbol() const;


/*!
 * @function getClassSize
 *
 * @abstract
 * Returns the allocation size of the C++ class managed by this metaclass.
 *
 * @result
 * The allocation size of the C++ class managed by this metaclass.
 */
	unsigned int getClassSize() const;


/*!
 * @function alloc
 *
 * @abstract
 * Allocates an instance of the C++ class managed by this metaclass.
 *
 * @result
 * A pointer to the newly allocated, uninitialized instance,
 * with a retain count of 1; <code>NULL</code> on allocation failure.
 *
 * @discussion
 * This function is automatically created by the metaclass-registration macros
 * to enable dynamic instance allocation.
 */
	virtual OSObject * alloc() const = 0;

#ifdef XNU_KERNEL_PRIVATE
	void addInstance(const OSObject * instance, bool super = false) const;
	void removeInstance(const OSObject * instance, bool super = false) const;
	void applyToInstances(OSMetaClassInstanceApplierFunction applier,
	    void * context) const;
	static void applyToInstancesOfClassName(
		const OSSymbol * name,
		OSMetaClassInstanceApplierFunction  applier,
		void * context);
private:
	static void applyToInstances(OSOrderedSet * set,
	    OSMetaClassInstanceApplierFunction  applier,
	    void * context);
public:
#endif /* XNU_KERNEL_PRIVATE */

/* Not to be included in headerdoc.
 *
 * @define OSDeclareCommonStructors
 * @hidecontents
 *
 * @abstract
 * Helper macro for for the standard metaclass-registration macros.
 * DO NOT USE.
 *
 * @param className The name of the C++ class, as a raw token,
 *                  <i>not</i> a string or macro.
 */

#define _OS_ADD_METAMETHODS(b) _OS_ADD_METAMETHODS_ ## b
#define _OS_ADD_METAMETHODS_
#define _OS_ADD_METAMETHODS_dispatch                            \
    virtual kern_return_t Dispatch(const IORPC rpc) APPLE_KEXT_OVERRIDE;

#define _OS_ADD_METHODS(className, b) _OS_ADD_METHODS_ ## b(className)
#define _OS_ADD_METHODS_(className)
#define _OS_ADD_METHODS_dispatch(className)                     \
    className ## _Methods                                       \
    className ## _KernelMethods

#define SUPERDISPATCH ((OSDispatchMethod)&super::_Dispatch)

#define OSDeclareCommonStructors(className, dispatch)           \
    private:                                                    \
    static const OSMetaClass * const superClass;                \
    public:                                                     \
    static const OSMetaClass * const metaClass;                 \
	static class MetaClass : public OSMetaClass {           \
	public:                                                 \
	    MetaClass();                                        \
	    virtual OSObject *alloc() const APPLE_KEXT_OVERRIDE;\
	    _OS_ADD_METAMETHODS(dispatch);                      \
	} gMetaClass;                                           \
	friend class className ::MetaClass;                     \
	virtual const OSMetaClass * getMetaClass() const APPLE_KEXT_OVERRIDE; \
    protected:                                                  \
    className (const OSMetaClass *);                            \
    virtual ~ className () APPLE_KEXT_OVERRIDE;                 \
    _OS_ADD_METHODS(className, dispatch)


/*!
 * @define OSDeclareDefaultStructors
 * @hidecontents
 *
 * @abstract
 * Declares run-time type information and functions
 * for a final (non-subclassable) Libkern C++ class.
 *
 * @param className The name of the C++ class, as a raw token,
 *                  <i>not</i> a string or macro.
 *
 * @discussion
 * Concrete Libkern C++ classes should "call" this macro
 * immediately after the opening brace in a class declaration.
 * It leaves the current privacy state as <code>protected:</code>.
 */
#define _OSDeclareDefaultStructors(className, dispatch)    \
    OSDeclareCommonStructors(className, dispatch);        \
    public:                                     \
    className (void);                           \
    protected:


#define OSDeclareDefaultStructors(className)   \
_OSDeclareDefaultStructors(className, )

#define OSDeclareDefaultStructorsWithDispatch(className)   \
_OSDeclareDefaultStructors(className, dispatch)


/*!
 * @define OSDeclareAbstractStructors
 * @hidecontents
 *
 * @abstract
 * Declares run-time type information and functions
 * for an abstract Libkern C++ class.
 *
 * @param className The name of the C++ class, as a raw token,
 *                  <i>not</i> a string or macro.
 *
 * @discussion
 * Abstract Libkern C++ classes--those with at least one
 * pure virtual method--should "call" this macro
 * immediately after the opening brace in a class declaration.
 * It leaves the current privacy state as <code>protected:</code>.
 */
#define _OSDeclareAbstractStructors(className, dispatch)                        \
    OSDeclareCommonStructors(className, dispatch);                              \
    private:                                                                    \
    className (void); /* Make primary constructor private in abstract */            \
    protected:

#define OSDeclareAbstractStructors(className)                                   \
_OSDeclareAbstractStructors(className, )

#define OSDeclareAbstractStructorsWithDispatch(className)                       \
_OSDeclareAbstractStructors(className, dispatch)

/*!
 * @define OSDeclareFinalStructors
 * @hidecontents
 *
 * @abstract
 * Declares run-time type information and functions
 * for a concrete Libkern C++ class.
 *
 * @param className The name of the C++ class, as a raw token,
 *                  <i>not</i> a string or macro.
 *
 * @discussion
 * Final Libkern C++ classes--those that do not allow subclassing--should
 * "call" this macro immediately after the opening brace in a class declaration.
 * (Final classes in the kernel may actually have subclasses in the kernel,
 * but kexts cannot define any subclasses of a final class.)
 * It leaves the current privacy state as <code>protected:</code>.
 *
 * <b>Note:</b> If the class is exported by a pseudokext (symbol set),
 * the final symbol generated by this macro must be exported
 * for the final-class attribute to be enforced.
 *
 * <b>Warning:</b> Changing a class from "Default" to "Final" will break
 * binary compatibility.
 */
#define _OSDeclareFinalStructors(className, dispatch)                           \
	_OSDeclareDefaultStructors(className, dispatch)                         \
    private:                                                                    \
	void __OSFinalClass(void);                                              \
    protected:


#define OSDeclareFinalStructors(className)                                      \
_OSDeclareFinalStructors(className, )

#define OSDeclareFinalStructorsWithDispatch(className)                          \
_OSDeclareFinalStructors(className, dispatch)


/* Not to be included in headerdoc.
 *
 * @define OSDefineMetaClassWithInit
 * @hidecontents
 *
 * @abstract
 * Helper macro for for the standard metaclass-registration macros.
 * DO NOT USE.
 *
 * @param className      The name of the C++ class, as a raw token,
 *                       <i>not</i> a string or macro.
 * @param superclassName The name of the superclass of the C++ class,
 *                       as a raw token,
 *                       <i>not</i> a string or macro.
 * @param init           A function to call in the constructor
 *                       of the class's OSMetaClass.
 */
#define OSDefineMetaClassWithInit(className, superclassName, init)            \
	/* Class global data */                                                   \
    className ::MetaClass className ::gMetaClass;                             \
    const OSMetaClass * const className ::metaClass =                         \
	& className ::gMetaClass;                                             \
    const OSMetaClass * const className ::superClass =                        \
	& superclassName ::gMetaClass;                                        \
	/* Class member functions */                                              \
    className :: className(const OSMetaClass *meta)                           \
	: superclassName (meta) { }                                           \
    className ::~ className() { }                                             \
    const OSMetaClass * className ::getMetaClass() const                      \
	{ return &gMetaClass; }                                               \
	/* The ::MetaClass constructor */                                         \
    className ::MetaClass::MetaClass()                                        \
	: OSMetaClass(#className, className::superClass, sizeof(className))   \
	{ init; }


/* Not to be included in headerdoc.
 *
 * @define OSDefineAbstractStructors
 * @hidecontents
 *
 * @abstract
 * Helper macro for for the standard metaclass-registration macros.
 * DO NOT USE.
 *
 * @param className      The name of the C++ class, as a raw token,
 *                       <i>not</i> a string or macro.
 * @param superclassName The name of the superclass of the C++ class,
 *                       as a raw token,
 *                       <i>not</i> a string or macro.
 */
#define OSDefineAbstractStructors(className, superclassName)        \
    OSObject * className ::MetaClass::alloc() const { return NULL; }


/* Not to be included in headerdoc.
 *
 * @define OSDefineDefaultStructors
 * @hidecontents
 *
 * @abstract
 * Helper macro for for the standard metaclass-registration macros.
 * DO NOT USE.
 *
 * @param className      The name of the C++ class, as a raw token,
 *                       <i>not</i> a string or macro.
 * @param superclassName The name of the superclass of the C++ class,
 *                       as a raw token,
 *                       <i>not</i> a string or macro.
 */
#define OSDefineDefaultStructors(className, superclassName)     \
    OSObject * className ::MetaClass::alloc() const             \
    { return new className; }                                   \
    className :: className () : superclassName (&gMetaClass)    \
    { gMetaClass.instanceConstructed(); }

/* Not to be included in headerdoc.
 *
 * @define OSDefineDefaultStructors
 * @hidecontents
 *
 * @abstract
 * Helper macro for for the standard metaclass-registration macros.
 * DO NOT USE.
 *
 * @param className      The name of the C++ class, as a raw token,
 *                       <i>not</i> a string or macro.
 * @param superclassName The name of the superclass of the C++ class,
 *                       as a raw token,
 *                       <i>not</i> a string or macro.
 */
#define OSDefineFinalStructors(className, superclassName)               \
    OSDefineDefaultStructors(className, superclassName)                 \
    void className ::__OSFinalClass(void) { }


/* Not to be included in headerdoc.
 *
 * @define OSDefineMetaClassAndStructorsWithInit
 * @hidecontents
 *
 * @abstract
 * Helper macro for for the standard metaclass-registration macros.
 * DO NOT USE.
 *
 * @param className      The name of the C++ class, as a raw token,
 *                       <i>not</i> a string or macro.
 * @param superclassName The name of the superclass of the C++ class,
 *                       as a raw token,
 *                       <i>not</i> a string or macro.
 * @param init           A function to call in the constructor
 *                       of the class's OSMetaClass.
 */
#define OSDefineMetaClassAndStructorsWithInit(className, superclassName, init) \
    OSDefineMetaClassWithInit(className, superclassName, init)        \
    OSDefineDefaultStructors(className, superclassName)


/* Not to be included in headerdoc.
 *
 * @define OSDefineMetaClassAndAbstractStructorsWithInit
 * @hidecontents
 *
 * @abstract
 * Helper macro for for the standard metaclass-registration macros.
 * DO NOT USE.
 *
 * @param className      The name of the C++ class, as a raw token,
 *                       <i>not</i> a string or macro.
 * @param superclassName The name of the superclass of the C++ class,
 *                       as a raw token,
 *                       <i>not</i> a string or macro.
 * @param init           A function to call in the constructor
 *                       of the class's OSMetaClass.
 */
#define OSDefineMetaClassAndAbstractStructorsWithInit(className, superclassName, init) \
    OSDefineMetaClassWithInit(className, superclassName, init)        \
    OSDefineAbstractStructors(className, superclassName)


/* Not to be included in headerdoc.
 *
 * @define OSDefineMetaClassAndFinalStructorsWithInit
 * @hidecontents
 *
 * @abstract
 * Helper macro for for the standard metaclass-registration macros.
 * DO NOT USE.
 *
 * @param className      The name of the C++ class, as a raw token,
 *                       <i>not</i> a string or macro.
 * @param superclassName The name of the superclass of the C++ class,
 *                       as a raw token,
 *                       <i>not</i> a string or macro.
 * @param init           A function to call in the constructor
 *                       of the class's OSMetaClass.
 */
#define OSDefineMetaClassAndFinalStructorsWithInit(className, superclassName, init) \
    OSDefineMetaClassWithInit(className, superclassName, init)                      \
    OSDefineFinalStructors(className, superclassName)


/* Helpers */

/* Not to be included in headerdoc.
 *
 * @define OSDefineMetaClass
 * @hidecontents
 *
 * @abstract
 * Helper macro for for the standard metaclass-registration macros.
 * DO NOT USE.
 *
 * @param className      The name of the C++ class, as a raw token,
 *                       <i>not</i> a string or macro.
 * @param superclassName The name of the superclass of the C++ class,
 *                       as a raw token,
 *                       <i>not</i> a string or macro.
 * @param init           A function to call in the constructor
 *                       of the class's OSMetaClass.
 */
#define OSDefineMetaClass(className, superclassName)            \
    OSDefineMetaClassWithInit(className, superclassName, )


/*!
 * @define OSDefineMetaClassAndStructors
 * @hidecontents
 *
 * @abstract
 * Defines an OSMetaClass and associated routines
 * for a concrete Libkern C++ class.
 *
 * @param className      The name of the C++ class, as a raw token,
 *                       <i>not</i> a string or macro.
 * @param superclassName The name of the superclass of the C++ class,
 *                       as a raw token,
 *                       <i>not</i> a string or macro.
 *
 * @discussion
 * Concrete Libkern C++ classes should "call" this macro
 * at the beginning of their implementation files,
 * before any function implementations for the class.
 */
#define OSDefineMetaClassAndStructors(className, superclassName)    \
    OSDefineMetaClassAndStructorsWithInit(className, superclassName, )


/*!
 * @define OSDefineMetaClassAndAbstractStructors
 * @hidecontents
 *
 * @abstract
 * Defines an OSMetaClass and associated routines
 * for an abstract Libkern C++ class.
 *
 * @param className      The name of the C++ class, as a raw token,
 *                       <i>not</i> a string or macro.
 * @param superclassName The name of the superclass of the C++ class,
 *                       as a raw token,
 *                       <i>not</i> a string or macro.
 *
 * @discussion
 * Abstract Libkern C++ classes--those with at least one
 * pure virtual method--should "call" this macro
 * at the beginning of their implementation files,
 * before any function implementations for the class.
 */
#define OSDefineMetaClassAndAbstractStructors(className, superclassName) \
    OSDefineMetaClassAndAbstractStructorsWithInit (className, superclassName, )


/*!
 * @define OSDefineMetaClassAndFinalStructors
 * @hidecontents
 *
 * @abstract
 * Defines an OSMetaClass and associated routines
 * for concrete Libkern C++ class.
 *
 * @param className      The name of the C++ class, as a raw token,
 *                       <i>not</i> a string or macro.
 * @param superclassName The name of the superclass of the C++ class,
 *                       as a raw token,
 *                       <i>not</i> a string or macro.
 *
 * @discussion
 * Final Libkern C++ classes--those that do not allow
 * subclassing--should "call" this macro at the beginning
 * of their implementation files,
 * before any function implementations for the class.
 * (Final classes in the kernel may actually have subclasses in the kernel,
 * but kexts cannot define any subclasses of a final class.)
 *
 * <b>Note:</b> If the class is exported by a pseudokext (symbol set),
 * the final symbol generated by this macro must be exported
 * for the final-class attribute to be enforced.
 *
 * <b>Warning:</b> Changing a class from "Default" to "Final" will break
 * binary compatibility.
 */
#define OSDefineMetaClassAndFinalStructors(className, superclassName)   \
    OSDefineMetaClassAndFinalStructorsWithInit(className, superclassName, )


// Dynamic vtable patchup support routines and types
	void reservedCalled(int ind) const;


/*!
 * @define OSMetaClassDeclareReservedUnused
 * @hidecontents
 *
 * @abstract
 * Reserves vtable space for new virtual functions
 * in a Libkern C++ class.
 *
 * @param className      The name of the C++ class, as a raw token,
 *                       <i>not</i> a string or macro.
 * @param index          The numeric index of the vtable slot,
 *                       as a raw constant, beginning from 0.
 *
 * @discussion
 * Libkern C++ classes in kernel extensions that can be used as libraries
 * can provide for backward compatibility by declaring a number
 * of reserved vtable slots
 * that can be replaced with new functions as they are added.
 * Each reserved declaration must be accompanied in the implementation
 * by a corresponding reference to
 * <code>@link OSMetaClassDefineReservedUnused
 *       OSMetaClassDefineReservedUnused@/link</code>.
 *
 * When replacing a reserved slot, change the macro from "Unused"
 * to "Used" to document the fact that the slot used to be reserved,
 * and declare the new function immediately after the "Used" macro
 * to preserve vtable ordering.
 * See
 * <code>@link OSMetaClassDeclareReservedUsed
 *       OSMetaClassDeclareReservedUsed@/link</code>.
 */
#if APPLE_KEXT_VTABLE_PADDING
#define OSMetaClassDeclareReservedUnused(className, index)        \
    private:                                                      \
    virtual void _RESERVED ## className ## index ()
#else
#define OSMetaClassDeclareReservedUnused(className, index)
#endif


/*!
 * @define OSMetaClassDeclareReservedUsed
 * @hidecontents
 *
 * @abstract
 * Documents use of reserved vtable space for new virtual functions
 * in a Libkern C++ class.
 *
 * @param className      The name of the C++ class, as a raw token,
 *                       <i>not</i> a string or macro.
 * @param index          The numeric index of the vtable slot,
 *                       as a raw constant, beginning from 0.
 *
 * @discussion
 * This macro evaluates to nothing, and is used to document reserved
 * vtable slots as they are filled.
 * See
 * <code>@link OSMetaClassDeclareReservedUnused
 *       OSMetaClassDeclareReservedUnused@/link</code>.
 */
#define OSMetaClassDeclareReservedUsed(className, index)


/*!
 * @define OSMetaClassDefineReservedUnused
 * @hidecontents
 *
 * @abstract
 * Defines a reserved vtable slot for a Libkern C++ class.
 *
 * @param className      The name of the C++ class, as a raw token,
 *                       <i>not</i> a string or macro.
 * @param index          The numeric index of the vtable slot,
 *                       as a raw constant, beginning from 0.
 *
 * @discussion
 * Libkern C++ classes in kernel extensions that can be used as libraries
 * can provide for backward compatibility by declaring a number
 * of reserved vtable slots
 * that can be replaced with new functions as they are added.
 * Each reserved defintion accompanies
 * a corresponding declaration created with
 * <code>@link OSMetaClassDeclareReservedUnused
 *       OSMetaClassDeclareReservedUnused@/link</code>.
 *
 * This macro is used in the implementation file
 * to provide a placeholder definition for the reserved vtable slot,
 * as a function that calls <code>panic</code> with an error message.
 *
 * When replacing a reserved slot, change the macro from "Unused"
 * to "Used" to document the fact that the slot used to be reserved,
 * and declare the new function immediately after the "Used" macro
 * to preserve vtable ordering.
 * See
 * <code>@link OSMetaClassDefineReservedUsed
 *       OSMetaClassDefineReservedUsed@/link</code>.
 */
#if APPLE_KEXT_VTABLE_PADDING
#define OSMetaClassDefineReservedUnused(className, index)       \
void className ::_RESERVED ## className ## index ()             \
	{ gMetaClass.reservedCalled(index); }
#else
#define OSMetaClassDefineReservedUnused(className, index)
#endif


/*!
 * @define OSMetaClassDefineReservedUsed
 * @hidecontents
 *
 * @abstract
 * Reserves vtable space for new virtual functions in a Libkern C++ class.
 *
 * @param className      The name of the C++ class, as a raw token,
 *                       <i>not</i> a string or macro.
 * @param index          The numeric index of the vtable slot,
 *                       as a raw constant, beginning from 0.
 *
 * @discussion
 * This macro evaluates to nothing, and is used to document reserved
 * vtable slots as they are filled.
 * See
 * <code>@link OSMetaClassDefineReservedUnused
 *       OSMetaClassDefineReservedUnused@/link</code>.
 */
#define OSMetaClassDefineReservedUsed(className, index)

// I/O Kit debug internal routines.
	static void printInstanceCounts();
	static void serializeClassDictionary(OSDictionary * dict);
#ifdef XNU_KERNEL_PRIVATE
#if IOTRACKING
public:
	static void * trackedNew(size_t size);
	static void trackedDelete(void * mem, size_t size);
	void trackedInstance(OSObject * instance) const;
	void trackedFree(OSObject * instance) const;
	void trackedAccumSize(OSObject * instance, size_t size) const;
	struct IOTrackingQueue * getTracking() const;
#endif /* IOTRACKING */
#endif /* XNU_KERNEL_PRIVATE */

private:
// Obsolete APIs
	static OSDictionary * getClassDictionary();
	virtual bool serialize(OSSerialize * serializer) const;

// Virtual Padding functions for MetaClass's
	OSMetaClassDeclareReservedUnused(OSMetaClass, 0);
	OSMetaClassDeclareReservedUnused(OSMetaClass, 1);
	OSMetaClassDeclareReservedUnused(OSMetaClass, 2);
	OSMetaClassDeclareReservedUnused(OSMetaClass, 3);
	OSMetaClassDeclareReservedUnused(OSMetaClass, 4);
	OSMetaClassDeclareReservedUnused(OSMetaClass, 5);
	OSMetaClassDeclareReservedUnused(OSMetaClass, 6);
	OSMetaClassDeclareReservedUnused(OSMetaClass, 7);
};

#endif /* !_LIBKERN_OSMETACLASS_H */
