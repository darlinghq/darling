#ifndef _OS_OBJECT_PTR_H
#define _OS_OBJECT_PTR_H

#include <sys/cdefs.h>
#include <os/smart_ptr.h>

#if KERNEL
# include <libkern/c++/OSObject.h>
#endif

#ifdef LIBKERN_SMART_POINTERS

/*
 * OSObject pointers (OSPtr)
 */

struct osobject_policy {
	static void
	retain(const OSMetaClassBase *obj)
	{
		obj->retain();
	}
	static void
	release(const OSMetaClassBase *obj)
	{
		obj->release();
	}
	template <class T> static T *
	alloc()
	{
		return OSTypeAlloc(T);
	}
	template <class From, class To> static To *
	dyn_cast(From *p)
	{
		return OSDynamicCast(To, p);
	}
};

template <class T>
using OSPtr = os::smart_ptr<T, osobject_policy>;

/*
 * Tagged OSObject pointers (OSTaggedPtr)
 */

template <class Tag>
struct osobject_tagged_policy {
	static void
	retain(const OSMetaClassBase *obj)
	{
		obj->taggedRetain(OSTypeID(Tag));
	}
	static void
	release(const OSMetaClassBase *obj)
	{
		obj->taggedRelease(OSTypeID(Tag));
	}
	template <class T> static T *
	alloc()
	{
		return OSTypeAlloc(T);
	}
	template <class From, class To> static To *
	dyn_cast(From *p)
	{
		return OSDynamicCast(To, p);
	}
};

template <class T, class Tag>
using OSTaggedPtr = os::smart_ptr<T, osobject_tagged_policy<Tag> >;

/*
 * Dynamic cast
 */

template<class T, class U, class P>
os::smart_ptr<T, P>
OSDynamicCastPtr(os::smart_ptr<U, P> const &from)
{
	return from.template dynamic_pointer_cast<T>();
}

template<class T, class U, class P>
os::smart_ptr<T, P>
OSDynamicCastPtr(os::smart_ptr<U, P> &&from)
{
	return os::move(from).template dynamic_pointer_cast<T>();
}

/*
 * Creation helpers
 */

template<class T, class P>
os::smart_ptr<T, P>
OSNewObject()
{
	return os::smart_ptr<T, P>::alloc();
}

template<class T, class P>
os::smart_ptr<T, P>
OSMakePtr(T *&p)
{
	return os::smart_ptr<T, P>(p);
}

template<class T, class P>
os::smart_ptr<T, P>
OSMakePtr(T *&&p)
{
	return os::smart_ptr<T, P>(os::move(p));
}

template<class T, class P>
os::smart_ptr<T, P>
OSMakePtr(T *&&p, bool retain)
{
	return os::smart_ptr<T, P>(os::move(p), retain);
}

template<class T, class P>
static inline T **
OSOutPtr(os::smart_ptr<T, P> *p)
{
	if (p == nullptr) {
		return nullptr;
	} else {
		return p->get_for_out_param();
	}
}

#else /* LIBKERN_SMART_POINTERS */

/* Fall back to the smart pointer types just being a simple pointer */
template<class T, class policy = void>
using OSPtr = T *;

template <class T, class Tag = void>
using OSTaggedPtr = T *;

#endif /* LIBKERN_SMART_POINTERS */
#endif /* _OS_OBJECT_PTR_H */
