/*
 * Copyright (c) 2017 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef Array_h
#define Array_h

#include <algorithm>
#include <stdint.h>
#include <stddef.h>
#include <mach/mach.h>
#include <assert.h>

#if !TARGET_OS_DRIVERKIT && (BUILDING_LIBDYLD || BUILDING_DYLD)
  #include <CrashReporterClient.h>
#else
  #define CRSetCrashLogMessage(x)
  #define CRSetCrashLogMessage2(x)
#endif

#define VIS_HIDDEN __attribute__((visibility("hidden")))

namespace dyld3 {


//
// Similar to std::vector<> but storage is pre-allocated and cannot be re-allocated.
// Storage is normally stack allocated.
//
// Use push_back() to add elements and range based for loops to iterate and [] to access by index.
//
template <typename T>
class VIS_HIDDEN Array
{
public:
                    Array()                                 : _elements(nullptr), _allocCount(0), _usedCount(0) {}
                    Array(T* storage, uintptr_t allocCount, uintptr_t usedCount=0) : _elements(storage), _allocCount(allocCount), _usedCount(usedCount) {}
    void            setInitialStorage(T* storage, uintptr_t allocCount) { assert(_usedCount == 0); _elements=storage; _allocCount=allocCount; }

    T&              operator[](size_t idx)       { assert(idx < _usedCount); return _elements[idx]; }
    const T&        operator[](size_t idx) const { assert(idx < _usedCount); return _elements[idx]; }
    T&              back()                       { assert(_usedCount > 0); return _elements[_usedCount-1]; }
    uintptr_t       count() const                { return _usedCount; }
    uintptr_t       maxCount() const             { return _allocCount; }
    uintptr_t       freeCount() const            { return _allocCount - _usedCount; }
    bool            empty() const                { return (_usedCount == 0); }
    uintptr_t       index(const T& element)      { return &element - _elements; }
    void            push_back(const T& t)        { assert(_usedCount < _allocCount); _elements[_usedCount++] = t; }
    void            default_constuct_back()      { assert(_usedCount < _allocCount); new (&_elements[_usedCount++])T(); }
    void            pop_back()                   { assert(_usedCount > 0); _usedCount--; }
    T*              begin()                      { return &_elements[0]; }
    T*              end()                        { return &_elements[_usedCount]; }
    const T*        begin() const                { return &_elements[0]; }
    const T*        end() const                  { return &_elements[_usedCount]; }
    const Array<T>  subArray(uintptr_t start, uintptr_t size) const { assert(start+size <= _usedCount);
                                                                      return Array<T>(&_elements[start], size, size); }
    bool            contains(const T& targ) const { for (const T& a : *this) { if ( a == targ ) return true; } return false; }
    void            remove(size_t idx)          { assert(idx < _usedCount); ::memmove(&_elements[idx], &_elements[idx+1], sizeof(T)*(_usedCount-idx-1)); }

protected:
    T*          _elements;
    uintptr_t   _allocCount;
    uintptr_t   _usedCount;
};


// If an Array<>.setInitialStorage() is used, the array may out live the stack space of the storage.
// To allow cleanup to be done to array elements when the stack goes away, you can make a local
// variable of ArrayFinalizer<>.
template <typename T>
class VIS_HIDDEN ArrayFinalizer
{
public:
    typedef void (^CleanUp)(T& element);
                    ArrayFinalizer(Array<T>& array, CleanUp handler) : _array(array), _handler(handler) { }
                    ~ArrayFinalizer() { for(T& element : _array) _handler(element); }
private:
    Array<T>&   _array;
    CleanUp     _handler;
};



//
// Similar to Array<> but if the array overflows, it is re-allocated using vm_allocate().
// When the variable goes out of scope, any vm_allocate()ed storage is released.
// if MAXCOUNT is specified, then only one one vm_allocate() to that size is done.
//
template <typename T, uintptr_t MAXCOUNT=0xFFFFFFFF>
class VIS_HIDDEN OverflowSafeArray : public Array<T>
{
public:
                    OverflowSafeArray() : Array<T>(nullptr, 0) {}
                    OverflowSafeArray(T* stackStorage, uintptr_t stackAllocCount) : Array<T>(stackStorage, stackAllocCount) {}
                    ~OverflowSafeArray();

                    OverflowSafeArray(OverflowSafeArray&) = default;
                    OverflowSafeArray& operator=(OverflowSafeArray&& other);

    void            push_back(const T& t)        { verifySpace(1); this->_elements[this->_usedCount++] = t; }
    void            default_constuct_back()      { verifySpace(1); new (&this->_elements[this->_usedCount++])T(); }
    void            clear() { this->_usedCount = 0; }
    void            reserve(uintptr_t n) { if (this->_allocCount < n) growTo(n); }
    void            resize(uintptr_t n) {
        if (n == this->_usedCount)
            return;
        if (n < this->_usedCount) {
            this->_usedCount = n;
            return;
        }
        reserve(n);
        this->_usedCount = n;
    }

protected:
    void            growTo(uintptr_t n);
    void            verifySpace(uintptr_t n)     { if (this->_usedCount+n > this->_allocCount) growTo(this->_usedCount + n); }

private:
    vm_address_t    _overflowBuffer         = 0;
    vm_size_t       _overflowBufferSize     = 0;
};


template <typename T, uintptr_t MAXCOUNT>
inline void OverflowSafeArray<T,MAXCOUNT>::growTo(uintptr_t n)
{
    vm_address_t    oldBuffer      = _overflowBuffer;
    vm_size_t       oldBufferSize  = _overflowBufferSize;
    if ( MAXCOUNT != 0xFFFFFFFF ) {
        assert(oldBufferSize == 0); // only re-alloc once
        // MAXCOUNT is specified, so immediately jump to that size
        _overflowBufferSize = round_page(std::max(MAXCOUNT, n) * sizeof(T));
    }
    else {
       // MAXCOUNT is not specified, keep doubling size
       _overflowBufferSize = round_page(std::max(this->_allocCount * 2, n) * sizeof(T));
    }
    kern_return_t kr = ::vm_allocate(mach_task_self(), &_overflowBuffer, _overflowBufferSize, VM_FLAGS_ANYWHERE);
    if (kr != KERN_SUCCESS) {
#if BUILDING_LIBDYLD
        //FIXME We should figure out a way to do this in dyld
        char crashString[256];
        snprintf(crashString, 256, "OverflowSafeArray failed to allocate %lu bytes, vm_allocate returned: %d\n",
                _overflowBufferSize, kr);
        CRSetCrashLogMessage(crashString);
#endif
        assert(0);
    }
    ::memcpy((void*)_overflowBuffer, this->_elements, this->_usedCount*sizeof(T));
    this->_elements = (T*)_overflowBuffer;
    this->_allocCount = _overflowBufferSize / sizeof(T);

    if ( oldBuffer != 0 )
        ::vm_deallocate(mach_task_self(), oldBuffer, oldBufferSize);
}

template <typename T, uintptr_t MAXCOUNT>
inline OverflowSafeArray<T,MAXCOUNT>::~OverflowSafeArray()
{
    if ( _overflowBuffer != 0 )
        ::vm_deallocate(mach_task_self(), _overflowBuffer, _overflowBufferSize);
}

template <typename T, uintptr_t MAXCOUNT>
inline OverflowSafeArray<T,MAXCOUNT>& OverflowSafeArray<T,MAXCOUNT>::operator=(OverflowSafeArray<T,MAXCOUNT>&& other)
{
    if (this == &other)
        return *this;

    // Free our buffer if we have one
    if ( _overflowBuffer != 0 )
        ::vm_deallocate(mach_task_self(), _overflowBuffer, _overflowBufferSize);

    // Now take the buffer from the other array
    this->_elements     = other._elements;
    this->_allocCount   = other._allocCount;
    this->_usedCount    = other._usedCount;
    _overflowBuffer     = other._overflowBuffer;
    _overflowBufferSize = other._overflowBufferSize;

    // Now reset the other object so that it doesn't try to deallocate the memory later.
    other._elements             = nullptr;
    other._allocCount           = 0;
    other._usedCount            = 0;
    other._overflowBuffer       = 0;
    other._overflowBufferSize   = 0;
    return *this;
}



#if BUILDING_LIBDYLD
//
// Similar to std::vector<> but storage is initially allocated in the object. But if it needs to
// grow beyond, it will use malloc.  The QUANT template arg is the "quantum" size for allocations.
// When the allocation needs to be grown, it is re-allocated at the required size rounded up to
// the next quantum.
//
// Use push_back() to add elements and range based for loops to iterate and [] to access by index.
//
// Note: this should be a subclass of Array<T> but doing so disables the compiler from optimizing away static constructors
//
template <typename T, int QUANT=4, int INIT=1>
class VIS_HIDDEN GrowableArray
{
public:

    T&              operator[](size_t idx)       { assert(idx < _usedCount); return _elements[idx]; }
    const T&        operator[](size_t idx) const { assert(idx < _usedCount); return _elements[idx]; }
    T&              back()                       { assert(_usedCount > 0); return _elements[_usedCount-1]; }
    uintptr_t       count() const                { return _usedCount; }
    uintptr_t       maxCount() const             { return _allocCount; }
    bool            empty() const                { return (_usedCount == 0); }
    uintptr_t       index(const T& element)      { return &element - _elements; }
    void            push_back(const T& t)        { verifySpace(1); _elements[_usedCount++] = t; }
    void            append(const Array<T>& a);
    void            pop_back()                   { assert(_usedCount > 0); _usedCount--; }
    T*              begin()                      { return &_elements[0]; }
    T*              end()                        { return &_elements[_usedCount]; }
    const T*        begin() const                { return &_elements[0]; }
    const T*        end() const                  { return &_elements[_usedCount]; }
    const Array<T>  subArray(uintptr_t start, uintptr_t size) const { assert(start+size <= _usedCount);
                                                                      return Array<T>(&_elements[start], size, size); }
    const Array<T>& array() const                 { return *((Array<T>*)this); }
    bool            contains(const T& targ) const { for (const T& a : *this) { if ( a == targ ) return true; } return false; }
    void            erase(T& targ);

protected:
    void            growTo(uintptr_t n);
    void            verifySpace(uintptr_t n)     { if (this->_usedCount+n > this->_allocCount) growTo(this->_usedCount + n); }

private:
    T*              _elements               = _initialAlloc;
    uintptr_t       _allocCount             = INIT;
    uintptr_t       _usedCount              = 0;
    T               _initialAlloc[INIT]     = { };
};


template <typename T, int QUANT, int INIT>
inline void GrowableArray<T,QUANT,INIT>::growTo(uintptr_t n)
{
    uintptr_t newCount = (n + QUANT - 1) & (-QUANT);
    T* newArray = (T*)::malloc(sizeof(T)*newCount);
    T* oldArray = this->_elements;
    if ( this->_usedCount != 0 )
        ::memcpy(newArray, oldArray, sizeof(T)*this->_usedCount);
    this->_elements   = newArray;
    this->_allocCount = newCount;
    if ( oldArray != this->_initialAlloc )
        ::free(oldArray);
}

template <typename T, int QUANT, int INIT>
inline void GrowableArray<T,QUANT,INIT>::append(const Array<T>& a)
{
    verifySpace(a.count());
    ::memcpy(&_elements[_usedCount], a.begin(), a.count()*sizeof(T));
    _usedCount += a.count();
}

template <typename T, int QUANT, int INIT>
inline void GrowableArray<T,QUANT,INIT>::erase(T& targ)
{
    intptr_t index = &targ - _elements;
    assert(index >= 0);
    assert(index < (intptr_t)_usedCount);
    intptr_t moveCount = _usedCount-index-1;
    if ( moveCount > 0 )
        ::memcpy(&_elements[index], &_elements[index+1], moveCount*sizeof(T));
    _usedCount -= 1;
}

#endif // BUILDING_LIBDYLD



//  STACK_ALLOC_ARRAY(foo, myarray, 10);
//  myarray is of type Array<foo>
#define STACK_ALLOC_ARRAY(_type, _name, _count)  \
    uintptr_t __##_name##_array_alloc[1 + ((sizeof(_type)*(_count))/sizeof(uintptr_t))]; \
    __block dyld3::Array<_type> _name((_type*)__##_name##_array_alloc, _count);


//  STACK_ALLOC_OVERFLOW_SAFE_ARRAY(foo, myarray, 10);
//  myarray is of type OverflowSafeArray<foo>
#define STACK_ALLOC_OVERFLOW_SAFE_ARRAY(_type, _name, _count)  \
    uintptr_t __##_name##_array_alloc[1 + ((sizeof(_type)*(_count))/sizeof(uintptr_t))]; \
    __block dyld3::OverflowSafeArray<_type> _name((_type*)__##_name##_array_alloc, _count);


//  work around compiler bug where:
//       __block type name[count];
//  is not accessible in a block
#define BLOCK_ACCCESSIBLE_ARRAY(_type, _name, _count)  \
    _type __##_name##_array_alloc[_count]; \
    _type* _name = __##_name##_array_alloc;


} // namespace dyld3

#endif /* Array_h */
