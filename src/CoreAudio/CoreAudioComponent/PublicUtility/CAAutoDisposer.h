/*
Copyright (C) 2016 Apple Inc. All Rights Reserved.
See LICENSE.txt for this sample’s licensing information

Abstract:
Part of Core Audio Public Utility Classes
*/

#if !defined(__CAPtr_h__)
#define __CAPtr_h__

#include <stdlib.h>		// for malloc
#include <new>			// for bad_alloc
#include <string.h>		// for memset

inline void* CA_malloc(size_t size)
{
	void* p = malloc(size);
	if (!p && size) throw std::bad_alloc();
	return p;
}

inline void* CA_realloc(void* old, size_t size)
{
#if TARGET_OS_WIN32
	void* p = realloc(old, size);
#else
	void* p = reallocf(old, size); // reallocf ensures the old pointer is freed if memory is full (p is NULL).
#endif
	if (!p && size) throw std::bad_alloc();
	return p;
}

#ifndef UINTPTR_MAX
#if __LP64__
#define UINTPTR_MAX	  18446744073709551615ULL
#else
#define UINTPTR_MAX	  4294967295U
#endif
#endif

inline void* CA_calloc(size_t n, size_t size)
{	
	// ensure that multiplication will not overflow
	if (n && UINTPTR_MAX / n < size) throw std::bad_alloc();
	
	size_t nsize = n*size;
	void* p = malloc(nsize);
	if (!p && nsize) throw std::bad_alloc();

	memset(p, 0, nsize);
	return p;
}


// helper class for automatic conversions
template <typename T>
struct CAPtrRef
{
	T* ptr_;

	explicit CAPtrRef(T* ptr) : ptr_(ptr) {}
};

template <typename T>
class CAAutoFree
{
private:
	T* ptr_;

public:
	
	CAAutoFree() : ptr_(0) {}
	
	explicit CAAutoFree(T* ptr) : ptr_(ptr) {}
	
	template<typename U>
	CAAutoFree(CAAutoFree<U>& that) : ptr_(that.release()) {} // take ownership

	// C++ std says: a template constructor is never a copy constructor
	CAAutoFree(CAAutoFree<T>& that) : ptr_(that.release()) {} // take ownership

	CAAutoFree(size_t n, bool clear = false) 
		// this becomes an ambiguous call if n == 0
		: ptr_(0) 
		{
			size_t maxItems = ~size_t(0) / sizeof(T);
			if (n > maxItems) 
				throw std::bad_alloc();

			ptr_ = static_cast<T*>(clear ? CA_calloc(n, sizeof(T)) : CA_malloc(n * sizeof(T)));
		}
	
	~CAAutoFree() { free(); }
	
	void alloc(size_t numItems, bool clear = false) 
	{
		size_t maxItems = ~size_t(0) / sizeof(T);
		if (numItems > maxItems) throw std::bad_alloc();
		
		free();
		ptr_ = static_cast<T*>(clear ? CA_calloc(numItems, sizeof(T)) : CA_malloc(numItems * sizeof(T)));
	}
	
	void allocBytes(size_t numBytes, bool clear = false) 
	{
		free();
		ptr_ = static_cast<T*>(clear ? CA_calloc(1, numBytes) : CA_malloc(numBytes));
	}
	
	void reallocBytes(size_t numBytes) 
	{
		ptr_ = static_cast<T*>(CA_realloc(ptr_, numBytes));
	}

	void reallocItems(size_t numItems) 
	{
		size_t maxItems = ~size_t(0) / sizeof(T);
		if (numItems > maxItems) throw std::bad_alloc();
		
		ptr_ = static_cast<T*>(CA_realloc(ptr_, numItems * sizeof(T)));
	}
	
	template <typename U>
	CAAutoFree& operator=(CAAutoFree<U>& that) 
	{ 
		set(that.release());	// take ownership
		return *this;
	}
	
	CAAutoFree& operator=(CAAutoFree& that) 
	{ 
		set(that.release());	// take ownership
		return *this;
	}
	
	CAAutoFree& operator=(T* ptr) 
	{
		set(ptr); 
		return *this;
	}
	
	template <typename U>
	CAAutoFree& operator=(U* ptr) 
	{
		set(ptr); 
		return *this;
	}
		
	T& operator*() const { return *ptr_; }
	T* operator->() const { return ptr_; }
	
	T* operator()() const { return ptr_; }
	T* get() const { return ptr_; }
	operator T*() const { return ptr_; }

	bool operator==(CAAutoFree const& that) const { return ptr_ == that.ptr_; }
	bool operator!=(CAAutoFree const& that) const { return ptr_ != that.ptr_; }
	bool operator==(T* ptr) const { return ptr_ == ptr; }
	bool operator!=(T* ptr) const { return ptr_ != ptr; }
		
	T* release() 
	{
		// release ownership
		T* result = ptr_;
		ptr_ = 0;
		return result;
	}
	
	void set(T* ptr)
	{
		if (ptr != ptr_)
		{
			::free(ptr_);
			ptr_ = ptr;
		}
	}
	
	void free() 
	{
		set(0);
	}


	// automatic conversions to allow assignment from results of functions.
	// hard to explain. see auto_ptr implementation and/or Josuttis' STL book.
	CAAutoFree(CAPtrRef<T> ref) : ptr_(ref.ptr_) { }

	CAAutoFree& operator=(CAPtrRef<T> ref)
	{
		set(ref.ptr_);
		return *this;
	}

	template<typename U>
	operator CAPtrRef<U>()
		{ return CAPtrRef<U>(release()); }

	template<typename U>
	operator CAAutoFree<U>()
		{ return CAAutoFree<U>(release()); }
	
};


template <typename T>
class CAAutoDelete
{
private:
	T* ptr_;

public:
	CAAutoDelete() : ptr_(0) {}

	explicit CAAutoDelete(T* ptr) : ptr_(ptr) {}
	
	template<typename U>
	CAAutoDelete(CAAutoDelete<U>& that) : ptr_(that.release()) {} // take ownership

	// C++ std says: a template constructor is never a copy constructor
	CAAutoDelete(CAAutoDelete<T>& that) : ptr_(that.release()) {} // take ownership
	
	~CAAutoDelete() { free(); }
	
	template <typename U>
	CAAutoDelete& operator=(CAAutoDelete<U>& that) 
	{ 
		set(that.release());	// take ownership
		return *this;
	}
	
	CAAutoDelete& operator=(CAAutoDelete& that) 
	{ 
		set(that.release());	// take ownership
		return *this;
	}
	
	CAAutoDelete& operator=(T* ptr) 
	{
		set(ptr); 
		return *this;
	}
	
	template <typename U>
	CAAutoDelete& operator=(U* ptr) 
	{
		set(ptr); 
		return *this;
	}
		
	T& operator*() const { return *ptr_; }
	T* operator->() const { return ptr_; }
	
	T* operator()() const { return ptr_; }
	T* get() const { return ptr_; }
	operator T*() const { return ptr_; }
	
	bool operator==(CAAutoDelete const& that) const { return ptr_ == that.ptr_; }
	bool operator!=(CAAutoDelete const& that) const { return ptr_ != that.ptr_; }
	bool operator==(T* ptr) const { return ptr_ == ptr; }
	bool operator!=(T* ptr) const { return ptr_ != ptr; }
		
	T* release() 
	{
		// release ownership
		T* result = ptr_;
		ptr_ = 0;
		return result;
	}
	
	void set(T* ptr)
	{
		if (ptr != ptr_)
		{
			delete ptr_;
			ptr_ = ptr;
		}
	}
	
	void free() 
	{
		set(0);
	}


	// automatic conversions to allow assignment from results of functions.
	// hard to explain. see auto_ptr implementation and/or Josuttis' STL book.
	CAAutoDelete(CAPtrRef<T> ref) : ptr_(ref.ptr_) { }

	CAAutoDelete& operator=(CAPtrRef<T> ref)
	{
		set(ref.ptr_);
		return *this;
	}

	template<typename U>
	operator CAPtrRef<U>()
		{ return CAPtrRef<U>(release()); }

	template<typename U>
	operator CAAutoFree<U>()
		{ return CAAutoFree<U>(release()); }
	
};


template <typename T>
class CAAutoArrayDelete
{
private:
	T* ptr_;

public:
	CAAutoArrayDelete() : ptr_(0) {}

	explicit CAAutoArrayDelete(T* ptr) : ptr_(ptr) {}
	
	template<typename U>
	CAAutoArrayDelete(CAAutoArrayDelete<U>& that) : ptr_(that.release()) {} // take ownership

	// C++ std says: a template constructor is never a copy constructor
	CAAutoArrayDelete(CAAutoArrayDelete<T>& that) : ptr_(that.release()) {} // take ownership
	
	// this becomes an ambiguous call if n == 0
	CAAutoArrayDelete(size_t n) : ptr_(new T[n]) {}
	
	~CAAutoArrayDelete() { free(); }
	
	void alloc(size_t numItems) 
	{
		free();
		ptr_ = new T [numItems];
	}
	
	template <typename U>
	CAAutoArrayDelete& operator=(CAAutoArrayDelete<U>& that) 
	{ 
		set(that.release());	// take ownership
		return *this;
	}
	
	CAAutoArrayDelete& operator=(CAAutoArrayDelete& that) 
	{ 
		set(that.release());	// take ownership
		return *this;
	}
	
	CAAutoArrayDelete& operator=(T* ptr) 
	{
		set(ptr); 
		return *this;
	}
	
	template <typename U>
	CAAutoArrayDelete& operator=(U* ptr) 
	{
		set(ptr); 
		return *this;
	}
		
	T& operator*() const { return *ptr_; }
	T* operator->() const { return ptr_; }
	
	T* operator()() const { return ptr_; }
	T* get() const { return ptr_; }
	operator T*() const { return ptr_; }

	bool operator==(CAAutoArrayDelete const& that) const { return ptr_ == that.ptr_; }
	bool operator!=(CAAutoArrayDelete const& that) const { return ptr_ != that.ptr_; }
	bool operator==(T* ptr) const { return ptr_ == ptr; }
	bool operator!=(T* ptr) const { return ptr_ != ptr; }
		
	T* release() 
	{
		// release ownership
		T* result = ptr_;
		ptr_ = 0;
		return result;
	}
	
	void set(T* ptr)
	{
		if (ptr != ptr_)
		{
			delete [] ptr_;
			ptr_ = ptr;
		}
	}
	
	void free() 
	{
		set(0);
	}


	// automatic conversions to allow assignment from results of functions.
	// hard to explain. see auto_ptr implementation and/or Josuttis' STL book.
	CAAutoArrayDelete(CAPtrRef<T> ref) : ptr_(ref.ptr_) { }

	CAAutoArrayDelete& operator=(CAPtrRef<T> ref)
	{
		set(ref.ptr_);
		return *this;
	}

	template<typename U>
	operator CAPtrRef<U>()
		{ return CAPtrRef<U>(release()); }

	template<typename U>
	operator CAAutoArrayDelete<U>()
		{ return CAAutoFree<U>(release()); }
	
};





// convenience function
template <typename T>
void free(CAAutoFree<T>& p)
{
	p.free();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#if 0
// example program showing ownership transfer

CAAutoFree<char> source()
{
	// source allocates and returns ownership to the caller.
	const char* str = "this is a test";
	size_t size = strlen(str) + 1;
	CAAutoFree<char> captr(size, false);
	strlcpy(captr(), str, size);
	printf("source %08X %08X '%s'\n", &captr, captr(), captr());
	return captr;
}

void user(CAAutoFree<char> const& captr)
{
	// passed by const reference. user can access the pointer but does not take ownership.
	printf("user: %08X %08X '%s'\n", &captr, captr(), captr());
}

void sink(CAAutoFree<char> captr)
{
	// passed by value. sink takes ownership and frees the pointer on return.
	printf("sink: %08X %08X '%s'\n", &captr, captr(), captr());
}


int main (int argc, char * const argv[]) 
{

	CAAutoFree<char> captr(source());
	printf("main captr A %08X %08X\n", &captr, captr());
	user(captr);
	sink(captr);
	printf("main captr B %08X %08X\n", &captr, captr());
    return 0;
}
#endif

#endif
