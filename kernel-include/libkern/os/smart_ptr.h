#ifndef _OS_SMART_POINTER_H
#define _OS_SMART_POINTER_H

#include <sys/cdefs.h>
#include <os/cpp_util.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++11-extensions"

#if __has_attribute(trivial_abi)
# define OS_TRIVIAL_ABI __attribute__((trivial_abi))
#else
# error Smart pointers depend on trivial_abi attribute
#endif

#if !OS_HAS_RVALUE_REFERENCES
# error Smart pointers depend on rvalue references
#endif

/* C++98 compatibility */
#if !OS_HAS_NULLPTR && !defined(nullptr)
# define nullptr NULL
#endif

#ifndef OSPTR_LOG
# define OSPTR_LOG(x, ...) do {} while(0)
#endif

namespace os {
static struct no_retain_t {} no_retain;

template<class T, class Policy>
class OS_TRIVIAL_ABI smart_ptr
{
	template<class U, class OtherPolicy> friend class smart_ptr;

public:

/*
 * Default constructor, creates a null pointer
 */
	smart_ptr() : pointer(nullptr)
	{
		OSPTR_LOG("Default construct smart_ptr\n");
	}

#if OS_HAS_NULLPTR
/*
 * Construction from a nullptr
 */
	smart_ptr(os::nullptr_t) : pointer(nullptr)
	{
		OSPTR_LOG("Construct smart_ptr from null\n");
	}
#endif

/*
 * Construct from a raw pointer, taking a reference to the object
 */
	explicit smart_ptr(T *&p) : pointer(p)
	{
		OSPTR_LOG("Construct smart_ptr from raw %p\n", pointer);
		if (pointer != nullptr) {
			_retain(pointer);
		}
	}

/*
 * Construct from a raw pointer, without bumping the refcount
 */
	explicit smart_ptr(T *&p, no_retain_t) : pointer(p)
	{
		OSPTR_LOG("Construct smart_ptr from raw %p no retain\n", pointer);
	}

/*
 * Copy constructor from the same smart_ptr type
 */
	smart_ptr(smart_ptr const &rhs) : pointer(rhs.pointer)
	{
		OSPTR_LOG("Copy construct smart_ptr with %p\n", rhs.pointer);
		if (pointer != nullptr) {
			_retain(pointer);
		}
	}

#if !LIBKERN_NO_MEMBER_TEMPLATES
/*
 * Allows copy of a smart_ptr<T> from a smart_ptr<U>
 * if U is convertible to T. For example, if T is a base class of U
 */
	template<class U>
	smart_ptr(smart_ptr<U, Policy> const &rhs) : pointer(rhs.get())
	{
		OSPTR_LOG("Copy construct smart_ptr with compatible %p\n", rhs.pointer);
		if (pointer != nullptr) {
			_retain(pointer);
		}
	}
#endif

/*
 * Assign to an OSPointer from a raw pointer
 */
	smart_ptr &
	operator=(T *&rhs)
	{
		OSPTR_LOG("Assign smart_ptr with replacing %p with raw %p\n", pointer, rhs);
		smart_ptr(rhs).swap(*this);
		return *this;
	}

#if OS_HAS_NULLPTR
/*
 * Assign to an OSPointer from a null pointer
 */
	smart_ptr &
	operator=(os::nullptr_t)
	{
		OSPTR_LOG("Assign smart_ptr to null replacing %p\n", pointer);
		smart_ptr().swap(*this);
		return *this;
	}
#endif

/*
 * Assign to a smart_ptr from a smart_ptr of the same type
 */
	smart_ptr &
	operator=(smart_ptr &rhs)
	{
		OSPTR_LOG("Assign smart_ptr replacing %p with %p\n", pointer, rhs.pointer);
		smart_ptr(rhs).swap(*this);
		return *this;
	}

#if !LIBKERN_NO_MEMBER_TEMPLATES
/*
 * Allows assignment of a smart_ptr<T> from a smart_ptr<U>
 * if U is convertible to T. For example, if T is a base class of U.
 */
	template <class U>
	smart_ptr &
	operator=(smart_ptr<U, Policy> const &rhs)
	{
		OSPTR_LOG("Assign smart_ptr to compatible replacing %p with %p\n", pointer, rhs.pointer);
		smart_ptr(rhs.get()).swap(*this);
		return *this;
	}
#endif

/*
 * Move support
 */

#if OS_HAS_RVALUE_REFERENCES
/*
 * Move-construct from a different smart_ptr of the same pointer type
 */
	smart_ptr(smart_ptr &&rhs) : pointer(rhs.pointer)
	{
		OSPTR_LOG("Move construct smart_ptr with %p\n", rhs.pointer);
		rhs.pointer = nullptr;
	}

/*
 * Move-construct from a raw pointer
 */
	smart_ptr(T *&&p) : pointer(p)
	{
		OSPTR_LOG("Move construct smart_ptr with %p\n", pointer);
		if (pointer != nullptr) {
			_retain(pointer);
		}
		p = nullptr;
	}

/*
 * Move-construct from a raw pointer without bumping the refcount
 */
	smart_ptr(T *&&p, no_retain_t) : pointer(p)
	{
		OSPTR_LOG("Move construct smart_ptr with %p no retain\n", pointer);
		p = nullptr;
	}

/*
 * Move-assign to a smart_ptr from a raw pointer
 */
	smart_ptr &
	operator=(T *&&rhs)
	{
		OSPTR_LOG("Move assign smart_ptr replacing %p with raw %p\n", pointer, rhs);
		smart_ptr(os::move(rhs)).swap(*this);
		rhs = nullptr;
		return *this;
	}

/*
 * Move-assign from a different smart_ptr of the same type
 */
	smart_ptr &
	operator=(smart_ptr &&rhs)
	{
		OSPTR_LOG("Move assign smart_ptr replacing %p with %p\n", pointer, rhs.pointer);
		smart_ptr(os::move(rhs)).swap(*this);
		return *this;
	}

/*
 * Move from a different smart_ptr with a compatible pointer type
 */
	template<class U>
	smart_ptr(smart_ptr<U, Policy> &&rhs) : pointer(rhs.pointer)
	{
		OSPTR_LOG("Move construct smart_ptr with compatible %p\n", rhs.pointer);
		rhs.pointer = nullptr;
	}

	template<class U>
	smart_ptr &
	operator=(smart_ptr<U, Policy> &&rhs)
	{
		OSPTR_LOG("Move assign smart_ptr replacing %p with compatible %p\n", pointer, rhs.pointer);
		smart_ptr(os::move(rhs)).swap(*this);
		return *this;
	}
#endif

/*
 * Destructor - decreases the object's reference count
 */
	~smart_ptr()
	{
		OSPTR_LOG("Destroy smart_ptr with %p\n", pointer);
		if (pointer) {
			_release(pointer);
		}
	}

/*
 * Create a new object of type T and wrap it in a smart_ptr. The object will have
 * a reference count of 1, so destruction of the smart_ptr will result in the
 * object being freed if the smart_ptr wasn't copied first.
 */
	static inline smart_ptr
	alloc()
	{
		return smart_ptr(_alloc(), no_retain);
	}

	void
	reset()
	{
		smart_ptr().swap(*this);
	}

	T *
	get() const
	{
		return pointer;
	}

	T **
	get_for_out_param()
	{
		reset();
		return &pointer;
	}

/*
 * Take ownership of object from raw pointer
 */
	void
	attach(T *&p)
	{
		OSPTR_LOG("Attach smart_ptr with %p\n", p);
		smart_ptr(p, no_retain).swap(*this);
	}

	void
	attach(T *&&p)
	{
		OSPTR_LOG("Move attach smart_ptr with %p\n", p);
		smart_ptr(os::move(p), no_retain).swap(*this);
	}

/* Return and drop ownership of pointer with NO release() */
	T *
	detach()
	{
		OSPTR_LOG("Detach smart_ptr with %p\n", pointer);
		T *ret = pointer;
		pointer = nullptr;
		return ret;
	}

	T *
	operator->() const
	{
		OSPTR_LOG("Dereference smart_ptr with %p\n", pointer);
		return pointer;
	}

	explicit
	operator bool() const
	{
		return pointer != nullptr;
	}

	inline void
	swap(smart_ptr &p)
	{
		T *temp = pointer;
		pointer = p.pointer;
		p.pointer = temp;
	}

/* swap pointers to the same type but with different policies */
	template<class OtherPolicy>
	void
	swap(smart_ptr<T, OtherPolicy> &p)
	{
		if (p.pointer) {
			_retain(p.pointer);
		}
		if (pointer) {
			smart_ptr<T, OtherPolicy>::_retain(pointer);
		}

		T *temp = pointer;
		pointer = p.pointer;
		p.pointer = temp;

		if (p.pointer) {
			_release(p.pointer);
		}
		if (pointer) {
			smart_ptr<T, OtherPolicy>::_release(pointer);
		}
	}

	template<class U>
	smart_ptr<U, Policy>
	const_pointer_cast() const &
	{
		OSPTR_LOG("const_pointer_cast smart_ptr with %p\n", pointer);
		return smart_ptr<U, Policy>(const_cast<U *>(pointer));
	}

	template <class U>
	smart_ptr<U, Policy>
	const_pointer_cast() &&
	{
		OSPTR_LOG("const_pointer_cast move smart_ptr with %p\n", pointer);
		U *newPointer = const_cast<U *>(detach());
		return smart_ptr<U, Policy>(os::move(newPointer), no_retain);
	}

	template <class U>
	smart_ptr<U, Policy>
	static_pointer_cast() const &
	{
		OSPTR_LOG("static_pointer_cast smart_ptr with %p\n", pointer);
		return smart_ptr<U, Policy>(static_cast<U *>(pointer));
	}

	template <class U>
	smart_ptr<U, Policy>
	static_pointer_cast() &&
	{
		OSPTR_LOG("static_pointer_cast move smart_ptr with %p\n", pointer);
		return smart_ptr<U, Policy>(static_cast<U *>(detach()), no_retain);
	}

	template <class U>
	smart_ptr<U, Policy>
	dynamic_pointer_cast() const &
	{
		OSPTR_LOG("dynamic_pointer_cast smart_ptr with %p\n", pointer);
		return smart_ptr<U, Policy>(Policy::template dyn_cast<T, U>(pointer));
	}

	template <class U>
	smart_ptr<U, Policy>
	dynamic_pointer_cast() &&
	{
		OSPTR_LOG("dynamic_pointer_cast move smart_ptr with %p\n", pointer);
		U *newPointer = Policy::template dyn_cast<T, U>(pointer);

		if (newPointer != nullptr) {
			detach();
		} else {
			reset();
		}
		return smart_ptr<U, Policy>(os::move(newPointer), no_retain);
	}

private:
	static inline void
	_retain(T *obj)
	{
		OSPTR_LOG("    %s with %p\n", __FUNCTION__, obj);
		Policy::retain(obj);
	}

	static inline void
	_release(T *obj)
	{
		OSPTR_LOG("    %s with %p\n", __FUNCTION__, obj);
		Policy::release(obj);
	}

	static inline T *
	_alloc()
	{
		OSPTR_LOG("    %s\n", __FUNCTION__);
		return Policy::template alloc<T>();
	}

	T *pointer;
};

/*
 * Comparison
 */

template<class T, class Policy>
inline bool
operator==(smart_ptr<T, Policy> const &a, smart_ptr<T, Policy> const &b)
{
	return a.get() == b.get();
}

template<class T, class Policy>
inline bool
operator!=(smart_ptr<T, Policy> const &a, smart_ptr<T, Policy> const &b)
{
	return a.get() != b.get();
}

template<class A, class A_policy, class B, class B_policy>
inline bool
operator==(smart_ptr<A, A_policy> const &a, smart_ptr<B, B_policy> const &b)
{
	return a.get() == b.get();
}

template<class A, class A_policy, class B, class B_policy>
inline bool
operator!=(smart_ptr<A, A_policy> const &a, smart_ptr<B, B_policy> const &b)
{
	return a.get() != b.get();
}

/*
 * Comparison with nullptr
 */

#if OS_HAS_NULLPTR
template<class T, class Policy>
inline bool
operator==(smart_ptr<T, Policy> const &p, os::nullptr_t)
{
	return p.get() == nullptr;
}

template<class T, class Policy> inline bool
operator==(os::nullptr_t, smart_ptr<T, Policy> const &p)
{
	return p.get() == nullptr;
}

template<class T, class Policy>
inline bool
operator!=(smart_ptr<T, Policy> const &p, os::nullptr_t)
{
	return p.get() != nullptr;
}

template<class T, class Policy>
inline bool
operator!=(os::nullptr_t, smart_ptr<T, Policy> const &p)
{
	return p.get() != nullptr;
}
#endif

/*
 * Comparison with raw pointer
 */

template<class T, class Policy>
inline bool
operator==(smart_ptr<T, Policy> const &p, const os::remove_const_t<T> *other)
{
	return p.get() == other;
}

template<class T, class Policy>
inline bool
operator==(const os::remove_const_t<T> *other, smart_ptr<T, Policy> const &p)
{
	return other == p.get();
}

template<class T, class Policy>
inline bool
operator!=(smart_ptr<T, Policy> const &p, const os::remove_const_t<T> *other)
{
	return p.get() != other;
}

template<class T, class Policy>
inline bool
operator!=(const os::remove_const_t<T> *other, smart_ptr<T, Policy> const &p)
{
	return other != p.get();
}
};

#pragma clang diagnostic pop
#endif /* _OS_SMART_POINTER_H */
