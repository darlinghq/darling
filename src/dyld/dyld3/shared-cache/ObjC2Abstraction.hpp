/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*- 
 *
 * Copyright (c) 2008-2010 Apple Inc. All rights reserved.
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

#include <iterator>
#include <deque>
#include <set>


// iterate an entsize-based list
// typedef entsize_iterator<P, type_t<P>, type_list_t<P> > type_iterator;
template <typename P, typename T, typename Tlist>
struct entsize_iterator {
    uint32_t entsize;
    uint32_t index;  // keeping track of this saves a divide in operator-
    T* current;    

    typedef std::random_access_iterator_tag iterator_category;
    typedef T value_type;
    typedef ptrdiff_t difference_type;
    typedef T* pointer;
    typedef T& reference;
    
    entsize_iterator() { } 
    
    entsize_iterator(const Tlist& list, uint32_t start = 0)
        : entsize(list.getEntsize()), index(start), current(&list.get(start)) 
    { }
    
    const entsize_iterator<P,T,Tlist>& operator += (ptrdiff_t count) {
        current = (T*)((uint8_t *)current + count*entsize);
        index += count;
        return *this;
    }
    const entsize_iterator<P,T,Tlist>& operator -= (ptrdiff_t count) {
        current = (T*)((uint8_t *)current - count*entsize);
        index -= count;
        return *this;
    }
    const entsize_iterator<P,T,Tlist> operator + (ptrdiff_t count) const {
        return entsize_iterator(*this) += count;
    }
    const entsize_iterator<P,T,Tlist> operator - (ptrdiff_t count) const {
        return entsize_iterator(*this) -= count;
    }
    
    entsize_iterator<P,T,Tlist>& operator ++ () { *this += 1; return *this; }
    entsize_iterator<P,T,Tlist>& operator -- () { *this -= 1; return *this; }
    entsize_iterator<P,T,Tlist> operator ++ (int) { 
        entsize_iterator<P,T,Tlist> result(*this); *this += 1; return result; 
    }
    entsize_iterator<P,T,Tlist> operator -- (int) { 
        entsize_iterator<P,T,Tlist> result(*this); *this -= 1; return result; 
    }
    
    ptrdiff_t operator - (const entsize_iterator<P,T,Tlist>& rhs) const {
        return (ptrdiff_t)this->index - (ptrdiff_t)rhs.index;
    }
    
    T& operator * () { return *current; }
    T& operator * () const { return *current; }
    T& operator -> () { return *current; }
    const T& operator -> () const { return *current; }
    
    operator T& () const { return *current; }
    
    bool operator == (const entsize_iterator<P,T,Tlist>& rhs) {
        return this->current == rhs.current;
    }
    bool operator != (const entsize_iterator<P,T,Tlist>& rhs) {
        return this->current != rhs.current;
    }
    
    bool operator < (const entsize_iterator<P,T,Tlist>& rhs) {
        return this->current < rhs.current;
    }        
    bool operator > (const entsize_iterator<P,T,Tlist>& rhs) {
        return this->current > rhs.current;
    }

    
    static void overwrite(entsize_iterator<P,T,Tlist>& dst, const Tlist* srcList)
    {
        entsize_iterator<P,T,Tlist> src;
        uint32_t ee = srcList->getEntsize();
        for (src = srcList->begin(); src != srcList->end(); ++src) {
            memcpy(&*dst, &*src, ee);
            ++dst;
        }
    }
};

template <typename P> 
class objc_header_info_rw_t {

    typedef typename P::uint_t pint_t;

    pint_t data;   // loaded:1, allRealised:1, objc_header_info *:ptr

public:
    objc_header_info_rw_t(ContentAccessor* cache, const macho_header<P>* mh)
        : data(0) {
    }
};

template <typename P>
class objc_header_info_ro_t {

    typedef typename P::uint_t pint_t;

    pint_t mhdr_offset;     // offset to mach_header or mach_header_64
    pint_t info_offset;     // offset to objc_image_info *

public:
    objc_header_info_ro_t(ContentAccessor* cache, const macho_header<P>* mh)
        : mhdr_offset(0), info_offset(0) {
        P::setP(mhdr_offset, (uint64_t)cache->vmAddrForContent((void*)mh) - (uint64_t)cache->vmAddrForContent(&mhdr_offset));
        assert(header_vmaddr(cache) == (uint64_t)cache->vmAddrForContent((void*)mh));
        const macho_section<P>* sect = mh->getSection("__DATA", "__objc_imageinfo");
        if (sect) {
            P::setP(info_offset, (uint64_t)sect->addr() - (uint64_t)cache->vmAddrForContent(&info_offset));
            // set bit in mach_header.flags to tell dyld that this image has objc content
            macho_header<P>* rwmh = const_cast<macho_header<P>*>(mh);
            rwmh->set_flags(mh->flags() | MH_HAS_OBJC);
        }
        else
            P::setP(info_offset, - (uint64_t)cache->vmAddrForContent(&info_offset));
    }

    pint_t header_vmaddr(ContentAccessor* cache) const {
        return (pint_t)(((uint64_t)cache->vmAddrForContent(&mhdr_offset)) + mhdr_offset);
    }
};


template <typename P>
class objc_method_list_t;  // forward reference


template <typename P>
class objc_method_t {
    typedef typename P::uint_t pint_t;
    pint_t name;   // SEL
    pint_t types;  // const char *
    pint_t imp;    // IMP
    friend class objc_method_list_t<P>;
public:
    pint_t getName() const { return (pint_t)P::getP(name); }
    void setName(pint_t newName) { P::setP(name, newName); }

    struct SortBySELAddress : 
        public std::binary_function<const objc_method_t<P>&, 
                                    const objc_method_t<P>&, bool>
    {
        bool operator() (const objc_method_t<P>& lhs, 
                         const objc_method_t<P>& rhs)
        {
            return lhs.getName() < rhs.getName();
        }
    };
};

template <typename P>
class objc_method_list_t {
    uint32_t entsize;
    uint32_t count;
    objc_method_t<P> first;

    void* operator new (size_t, void* buf) { return buf; }

public:

    typedef entsize_iterator<P, objc_method_t<P>, objc_method_list_t<P> > method_iterator;

    uint32_t getCount() const { return P::E::get32(count); }

    uint32_t getEntsize() const {return P::E::get32(entsize)&~(uint32_t)3;}

    objc_method_t<P>& get(uint32_t i) const { return *(objc_method_t<P> *)((uint8_t *)&first + i * getEntsize()); }

    uint32_t byteSize() const { 
        return byteSizeForCount(getCount(), getEntsize()); 
    }

    static uint32_t byteSizeForCount(uint32_t c, uint32_t e = sizeof(objc_method_t<P>)) { 
        return sizeof(objc_method_list_t<P>) - sizeof(objc_method_t<P>) + c*e;
    }

    method_iterator begin() { return method_iterator(*this, 0); }
    method_iterator end() { return method_iterator(*this, getCount()); }
    const method_iterator begin() const { return method_iterator(*this, 0); }
    const method_iterator end() const { return method_iterator(*this, getCount()); }

    void setFixedUp() { P::E::set32(entsize, getEntsize() | 3); }

    void getPointers(std::set<void*>& pointersToRemove) {
        for(method_iterator it = begin(); it != end(); ++it) {
            objc_method_t<P>& entry = *it;
            pointersToRemove.insert(&(entry.name));
            pointersToRemove.insert(&(entry.types));
            pointersToRemove.insert(&(entry.imp));
        }
    }
    
    static void addPointers(uint8_t* methodList, CacheBuilder::ASLR_Tracker& aslrTracker) {
        objc_method_list_t<P>* mlist = (objc_method_list_t<P>*)methodList;
        for(method_iterator it = mlist->begin(); it != mlist->end(); ++it) {
            objc_method_t<P>& entry = *it;
            aslrTracker.add(&(entry.name));
            aslrTracker.add(&(entry.types));
            aslrTracker.add(&(entry.imp));
        }
    }

    static objc_method_list_t<P>* newMethodList(size_t newCount, uint32_t newEntsize) {
        void *buf = ::calloc(byteSizeForCount(newCount, newEntsize), 1);
        return new (buf) objc_method_list_t<P>(newCount, newEntsize);
    }

    void operator delete(void * p) { 
        ::free(p); 
    }

    objc_method_list_t(uint32_t newCount, 
                       uint32_t newEntsize = sizeof(objc_method_t<P>))
        : entsize(newEntsize), count(newCount) 
    { }

private:
    // use newMethodList instead
    void* operator new (size_t);
};


template <typename P>
class objc_ivar_t {
    typedef typename P::uint_t pint_t;

    pint_t                    offset;  // uint32_t*  (uint64_t* on x86_64)
    pint_t                    name;    // const char*
    pint_t                    type;    // const char*
    uint32_t                alignment;
    uint32_t                size;
    
public:
    const char* getName(ContentAccessor* cache) const { return (const char *)cache->contentForVMAddr(P::getP(name)); }

    bool hasOffset() const { return offset != 0; }
    uint32_t getOffset(ContentAccessor* cache) const { return P::E::get32(*(uint32_t*)(cache->contentForVMAddr(P::getP(offset)))); }
    void setOffset(ContentAccessor* cache, uint32_t newOffset) { P::E::set32(*(uint32_t*)(cache->contentForVMAddr(P::getP(offset))), newOffset); }


    uint32_t getAlignment() {
        uint32_t a = P::E::get32(alignment);
        return (a == (uint32_t)-1) ? sizeof(pint_t) : 1<<a;
    }
};

template <typename P>
class objc_ivar_list_t {
    typedef typename P::uint_t pint_t;
    uint32_t entsize;
    uint32_t count;
    objc_ivar_t<P> first;

    void* operator new (size_t, void* buf) { return buf; }

public:

    typedef entsize_iterator<P, objc_ivar_t<P>, objc_ivar_list_t<P> > ivar_iterator;

    uint32_t getCount() const { return P::E::get32(count); }

    uint32_t getEntsize() const { return P::E::get32(entsize); }

    objc_ivar_t<P>& get(pint_t i) const { return *(objc_ivar_t<P> *)((uint8_t *)&first + i * P::E::get32(entsize)); }

    uint32_t byteSize() const { 
        return byteSizeForCount(getCount(), getEntsize()); 
    }

    static uint32_t byteSizeForCount(uint32_t c, uint32_t e = sizeof(objc_ivar_t<P>)) { 
        return sizeof(objc_ivar_list_t<P>) - sizeof(objc_ivar_t<P>) + c*e;
    }

    ivar_iterator begin() { return ivar_iterator(*this, 0); }
    ivar_iterator end() { return ivar_iterator(*this, getCount()); }
    const ivar_iterator begin() const { return ivar_iterator(*this, 0); }
    const ivar_iterator end() const { return ivar_iterator(*this, getCount()); }

    static objc_ivar_list_t<P>* newIvarList(size_t newCount, uint32_t newEntsize) {
        void *buf = ::calloc(byteSizeForCount(newCount, newEntsize), 1);
        return new (buf) objc_ivar_list_t<P>(newCount, newEntsize);
    }

    void operator delete(void * p) { 
        ::free(p); 
    }

    objc_ivar_list_t(uint32_t newCount, 
                         uint32_t newEntsize = sizeof(objc_ivar_t<P>))
        : entsize(newEntsize), count(newCount) 
    { }
private:
    // use newIvarList instead
    void* operator new (size_t);
};


template <typename P> class objc_property_list_t; // forward 

template <typename P>
class objc_property_t {
    typedef typename P::uint_t pint_t;
    pint_t name;
    pint_t attributes;
    friend class objc_property_list_t<P>;
public:
    
    const char * getName(ContentAccessor* cache) const { return (const char *)cache->contentForVMAddr(P::getP(name)); }

    const char * getAttributes(ContentAccessor* cache) const { return (const char *)cache->contentForVMAddr(P::getP(attributes)); }
};

template <typename P>
class objc_property_list_t {
    uint32_t entsize;
    uint32_t count;
    objc_property_t<P> first;

    void* operator new (size_t, void* buf) { return buf; }

public:

    typedef entsize_iterator<P, objc_property_t<P>, objc_property_list_t<P> > property_iterator;

    uint32_t getCount() const { return P::E::get32(count); }

    uint32_t getEntsize() const { return P::E::get32(entsize); }

    objc_property_t<P>& get(uint32_t i) const { return *(objc_property_t<P> *)((uint8_t *)&first + i * getEntsize()); }

    uint32_t byteSize() const { 
        return byteSizeForCount(getCount(), getEntsize()); 
    }

    static uint32_t byteSizeForCount(uint32_t c, uint32_t e = sizeof(objc_property_t<P>)) { 
        return sizeof(objc_property_list_t<P>) - sizeof(objc_property_t<P>) + c*e;
    }

    property_iterator begin() { return property_iterator(*this, 0); }
    property_iterator end() { return property_iterator(*this, getCount()); }
    const property_iterator begin() const { return property_iterator(*this, 0); }
    const property_iterator end() const { return property_iterator(*this, getCount()); }

    void getPointers(std::set<void*>& pointersToRemove) {
        for(property_iterator it = begin(); it != end(); ++it) {
            objc_property_t<P>& entry = *it;
            pointersToRemove.insert(&(entry.name));
            pointersToRemove.insert(&(entry.attributes));
        }
    }

    static void addPointers(uint8_t* propertyList, CacheBuilder::ASLR_Tracker& aslrTracker) {
        objc_property_list_t<P>* plist = (objc_property_list_t<P>*)propertyList;
        for(property_iterator it = plist->begin(); it != plist->end(); ++it) {
            objc_property_t<P>& entry = *it;
            aslrTracker.add(&(entry.name));
            aslrTracker.add(&(entry.attributes));
        }
    }

     static objc_property_list_t<P>* newPropertyList(size_t newCount, uint32_t newEntsize) {
        void *buf = ::calloc(byteSizeForCount(newCount, newEntsize), 1);
        return new (buf) objc_property_list_t<P>(newCount, newEntsize);
    }

    void operator delete(void * p) { 
        ::free(p); 
    }

    objc_property_list_t(uint32_t newCount, 
                         uint32_t newEntsize = sizeof(objc_property_t<P>))
        : entsize(newEntsize), count(newCount) 
    { }
private:
    // use newPropertyList instead
    void* operator new (size_t);
};


template <typename A> class objc_protocol_list_t;  // forward reference

template <typename P>
class objc_protocol_t {
    typedef typename P::uint_t pint_t;

    pint_t isa;
    pint_t name;
    pint_t protocols;
    pint_t instanceMethods;
    pint_t classMethods;
    pint_t optionalInstanceMethods;
    pint_t optionalClassMethods;
    pint_t instanceProperties;
    uint32_t size;
    uint32_t flags;
    pint_t extendedMethodTypes;
    pint_t demangledName;
    pint_t classProperties;

public:
    pint_t getIsaVMAddr() const { return (pint_t)P::getP(isa); }
    void setIsaVMAddr(pint_t newIsa) { P::setP(isa, newIsa); }

    const char *getName(ContentAccessor* cache) const { return (const char *)cache->contentForVMAddr(P::getP(name)); }

    uint32_t getSize() const { return P::E::get32(size); }
    void setSize(uint32_t newSize) { P::E::set32(size, newSize); }

    uint32_t getFlags() const { return P::E::get32(flags); }

    void setFixedUp() { P::E::set32(flags, getFlags() | (1<<30)); }
    void setIsCanonical() {
        assert((getFlags() & (1 << 29)) == 0);
        P::E::set32(flags, getFlags() | (1<<29));
    }

    objc_protocol_list_t<P> *getProtocols(ContentAccessor* cache) const { return (objc_protocol_list_t<P> *)cache->contentForVMAddr(P::getP(protocols)); }

    objc_method_list_t<P> *getInstanceMethods(ContentAccessor* cache) const { return (objc_method_list_t<P> *)cache->contentForVMAddr(P::getP(instanceMethods)); }

    objc_method_list_t<P> *getClassMethods(ContentAccessor* cache) const { return (objc_method_list_t<P> *)cache->contentForVMAddr(P::getP(classMethods)); }

    objc_method_list_t<P> *getOptionalInstanceMethods(ContentAccessor* cache) const { return (objc_method_list_t<P> *)cache->contentForVMAddr(P::getP(optionalInstanceMethods)); }

    objc_method_list_t<P> *getOptionalClassMethods(ContentAccessor* cache) const { return (objc_method_list_t<P> *)cache->contentForVMAddr(P::getP(optionalClassMethods)); }

    objc_property_list_t<P> *getInstanceProperties(ContentAccessor* cache) const { return (objc_property_list_t<P> *)cache->contentForVMAddr(P::getP(instanceProperties)); }

    pint_t *getExtendedMethodTypes(ContentAccessor* cache) const {
        if (getSize() < offsetof(objc_protocol_t<P>, extendedMethodTypes) + sizeof(extendedMethodTypes)) {
            return NULL;
        }
        return (pint_t *)cache->contentForVMAddr(P::getP(extendedMethodTypes));
    }

    const char *getDemangledName(ContentAccessor* cache) const {
        if (sizeof(*this) < offsetof(objc_protocol_t<P>, demangledName) + sizeof(demangledName)) {
            return NULL;
        }
        return (const char *)cache->contentForVMAddr(P::getP(demangledName));
    }

    void setDemangledName(ContentAccessor* cache, const char *newName, Diagnostics& diag) {
        if (sizeof(*this) < offsetof(objc_protocol_t<P>, demangledName) + sizeof(demangledName))
            diag.error("objc protocol has the wrong size");
        else
            P::setP(demangledName, cache->vmAddrForContent((void*)newName));
    }

    void addPointers(ContentAccessor* cache, CacheBuilder::ASLR_Tracker& aslrTracker)
    {
        aslrTracker.add(&isa);
        aslrTracker.add(&name);
        if (protocols)               aslrTracker.add(&protocols);
        if (instanceMethods)         aslrTracker.add(&instanceMethods);
        if (classMethods)            aslrTracker.add(&classMethods);
        if (optionalInstanceMethods) aslrTracker.add(&optionalInstanceMethods);
        if (optionalClassMethods)    aslrTracker.add(&optionalClassMethods);
        if (instanceProperties)      aslrTracker.add(&instanceProperties);
        if (extendedMethodTypes)     aslrTracker.add(&extendedMethodTypes);
        if (demangledName)           aslrTracker.add(&demangledName);
    }
};


template <typename P>
class objc_protocol_list_t {
    typedef typename P::uint_t pint_t;
    pint_t count;
    pint_t list[0];

    void* operator new (size_t, void* buf) { return buf; }

public:

    pint_t getCount() const { return (pint_t)P::getP(count); }

    pint_t getVMAddress(pint_t i) {
        return (pint_t)P::getP(list[i]);
    }

    objc_protocol_t<P>* get(ContentAccessor* cache, pint_t i) {
        return (objc_protocol_t<P>*)cache->contentForVMAddr(getVMAddress(i));
    }

    void setVMAddress(pint_t i, pint_t protoVMAddr) {
        P::setP(list[i], protoVMAddr);
    }
    
    void set(ContentAccessor* cache, pint_t i, objc_protocol_t<P>* proto) {
        setVMAddress(i, cache->vmAddrForContent(proto));
    }

    uint32_t byteSize() const {
        return byteSizeForCount(getCount()); 
    }
    static uint32_t byteSizeForCount(pint_t c) { 
        return sizeof(objc_protocol_list_t<P>) + c*sizeof(pint_t);
    }

    void getPointers(std::set<void*>& pointersToRemove) {
        for(int i=0 ; i < count; ++i) {
            pointersToRemove.insert(&list[i]);
        }
    }

     static void addPointers(uint8_t* protocolList, CacheBuilder::ASLR_Tracker& aslrTracker) {
        objc_protocol_list_t<P>* plist = (objc_protocol_list_t<P>*)protocolList;
        for(int i=0 ; i < plist->count; ++i) {
            aslrTracker.add(&plist->list[i]);
        }
    }

    static objc_protocol_list_t<P>* newProtocolList(pint_t newCount) {
        void *buf = ::calloc(byteSizeForCount(newCount), 1);
        return new (buf) objc_protocol_list_t<P>(newCount);
    }

    void operator delete(void * p) { 
        ::free(p); 
    }

    objc_protocol_list_t(uint32_t newCount) : count(newCount) { }
private:
    // use newProtocolList instead
    void* operator new (size_t);
};


template <typename P>
class objc_class_data_t {
    typedef typename P::uint_t pint_t;
    uint32_t flags;
    uint32_t instanceStart;
    // Note there is 4-bytes of alignment padding between instanceSize and ivarLayout
    // on 64-bit archs, but no padding on 32-bit archs.
    // This union is a way to model that.
    union {
        uint32_t                instanceSize;
        pint_t   pad;
    } instanceSize;
    pint_t ivarLayout;
    pint_t name;
    pint_t baseMethods;
    pint_t baseProtocols;
    pint_t ivars;
    pint_t weakIvarLayout;
    pint_t baseProperties;

public:
    bool isMetaClass() { return P::E::get32(flags) & (1 << 0); }
    bool isRootClass() { return P::E::get32(flags) & (1 << 1); }

    uint32_t getInstanceStart() { return P::E::get32(instanceStart); }
    void setInstanceStart(uint32_t newStart) { P::E::set32(instanceStart, newStart); }
    
    uint32_t getInstanceSize() { return P::E::get32(instanceSize.instanceSize); }
    void setInstanceSize(uint32_t newSiz) { P::E::set32(instanceSize.instanceSize, newSiz); }

    objc_method_list_t<P> *getMethodList(ContentAccessor* cache) const { return (objc_method_list_t<P> *)cache->contentForVMAddr(P::getP(baseMethods)); }

    objc_protocol_list_t<P> *getProtocolList(ContentAccessor* cache) const { return (objc_protocol_list_t<P> *)cache->contentForVMAddr(P::getP(baseProtocols)); }

    objc_ivar_list_t<P> *getIvarList(ContentAccessor* cache) const { return (objc_ivar_list_t<P> *)cache->contentForVMAddr(P::getP(ivars)); }
    
    objc_property_list_t<P> *getPropertyList(ContentAccessor* cache) const { return (objc_property_list_t<P> *)cache->contentForVMAddr(P::getP(baseProperties)); }

    const char * getName(ContentAccessor* cache) const { return (const char *)cache->contentForVMAddr(P::getP(name)); }

    void setMethodList(ContentAccessor* cache, objc_method_list_t<P>* mlist) {
        P::setP(baseMethods, cache->vmAddrForContent(mlist));
    }

    void setProtocolList(ContentAccessor* cache, objc_protocol_list_t<P>* protolist) {
        P::setP(baseProtocols, cache->vmAddrForContent(protolist));
    }
 
    void setPropertyList(ContentAccessor* cache, objc_property_list_t<P>* proplist) {
        P::setP(baseProperties, cache->vmAddrForContent(proplist));
    }
    
    void addMethodListPointer(CacheBuilder::ASLR_Tracker& aslrTracker) {
        aslrTracker.add(&this->baseMethods);
    }
    
    void addPropertyListPointer(CacheBuilder::ASLR_Tracker& aslrTracker) {
        aslrTracker.add(&this->baseProperties);
    }
    
    void addProtocolListPointer(CacheBuilder::ASLR_Tracker& aslrTracker) {
        aslrTracker.add(&this->baseProtocols);
    }
};

template <typename P>
class objc_class_t {
    typedef typename P::uint_t pint_t;

    pint_t isa;
    pint_t superclass;
    pint_t method_cache;
    pint_t vtable;
    pint_t data;

public:
    bool isMetaClass(ContentAccessor* cache) const { return getData(cache)->isMetaClass(); }
    bool isRootClass(ContentAccessor* cache) const { return getData(cache)->isRootClass(); }

    objc_class_t<P> *getIsa(ContentAccessor* cache) const { return (objc_class_t<P> *)cache->contentForVMAddr(P::getP(isa)); }

    objc_class_t<P> *getSuperclass(ContentAccessor* cache) const { return (objc_class_t<P> *)cache->contentForVMAddr(P::getP(superclass)); }
    
    const pint_t* getSuperClassAddress() const { return &superclass; }

    // Low bit marks Swift classes.
    objc_class_data_t<P> *getData(ContentAccessor* cache) const { return (objc_class_data_t<P> *)cache->contentForVMAddr(P::getP(data & ~0x3LL)); }

    objc_method_list_t<P> *getMethodList(ContentAccessor* cache) const {
        objc_class_data_t<P>* d = getData(cache);
        return d->getMethodList(cache);
    }

    objc_protocol_list_t<P> *getProtocolList(ContentAccessor* cache) const { return getData(cache)->getProtocolList(cache); }

    objc_property_list_t<P> *getPropertyList(ContentAccessor* cache) const { return getData(cache)->getPropertyList(cache); }

    const char* getName(ContentAccessor* cache) const {
        return getData(cache)->getName(cache);
    }

    void setMethodList(ContentAccessor* cache, objc_method_list_t<P>* mlist) {
        getData(cache)->setMethodList(cache, mlist);
    }

    void setProtocolList(ContentAccessor* cache, objc_protocol_list_t<P>* protolist) {
        getData(cache)->setProtocolList(cache, protolist);
    }

    void setPropertyList(ContentAccessor* cache, objc_property_list_t<P>* proplist) {
        getData(cache)->setPropertyList(cache, proplist);
    }
    
    void addMethodListPointer(ContentAccessor* cache, CacheBuilder::ASLR_Tracker& aslrTracker) {
        getData(cache)->addMethodListPointer(aslrTracker);
    }
    
    void addPropertyListPointer(ContentAccessor* cache, CacheBuilder::ASLR_Tracker& aslrTracker) {
        getData(cache)->addPropertyListPointer(aslrTracker);
    }
    
    void addProtocolListPointer(ContentAccessor* cache, CacheBuilder::ASLR_Tracker& aslrTracker) {
        getData(cache)->addProtocolListPointer(aslrTracker);
    }
    
};



template <typename P>
class objc_category_t {
    typedef typename P::uint_t pint_t;

    pint_t name;
    pint_t cls;
    pint_t instanceMethods;
    pint_t classMethods;
    pint_t protocols;
    pint_t instanceProperties;

public:

    const char * getName(ContentAccessor* cache) const { return (const char *)cache->contentForVMAddr(P::getP(name)); }

    objc_class_t<P> *getClass(ContentAccessor* cache) const { return (objc_class_t<P> *)cache->contentForVMAddr(P::getP(cls)); }

    objc_method_list_t<P> *getInstanceMethods(ContentAccessor* cache) const { return (objc_method_list_t<P> *)cache->contentForVMAddr(P::getP(instanceMethods)); }

    objc_method_list_t<P> *getClassMethods(ContentAccessor* cache) const { return (objc_method_list_t<P> *)cache->contentForVMAddr(P::getP(classMethods)); }

    objc_protocol_list_t<P> *getProtocols(ContentAccessor* cache) const { return (objc_protocol_list_t<P> *)cache->contentForVMAddr(P::getP(protocols)); }
 
    objc_property_list_t<P> *getInstanceProperties(ContentAccessor* cache) const { return (objc_property_list_t<P> *)cache->contentForVMAddr(P::getP(instanceProperties)); }

    void getPointers(std::set<void*>& pointersToRemove) {
        pointersToRemove.insert(&name);
        pointersToRemove.insert(&cls);
        pointersToRemove.insert(&instanceMethods);
        pointersToRemove.insert(&classMethods);
        pointersToRemove.insert(&protocols);
        pointersToRemove.insert(&instanceProperties);
    }


};

template <typename P>
class objc_message_ref_t {
    typedef typename P::uint_t pint_t;

    pint_t imp;
    pint_t sel;

public:
    pint_t getName() const { return (pint_t)P::getP(sel); }

    void setName(pint_t newName) { P::setP(sel, newName); }
};

// Call visitor.visitIvar() on every ivar in a given class.
template <typename P, typename V>
class IvarWalker {
    typedef typename P::uint_t pint_t;
    V& ivarVisitor;
public:
    
    IvarWalker(V& visitor) : ivarVisitor(visitor) { }
    
    void walk(ContentAccessor* cache, const macho_header<P>* header, objc_class_t<P> *cls)
    {
        objc_class_data_t<P> *data = cls->getData(cache);
        objc_ivar_list_t<P> *ivars = data->getIvarList(cache);
        if (ivars) {
            for (pint_t i = 0; i < ivars->getCount(); i++) {
                objc_ivar_t<P>& ivar = ivars->get(i);
                //fprintf(stderr, "visiting ivar: %s\n", ivar.getName(cache));
                ivarVisitor.visitIvar(cache, header, cls, &ivar);
            }
        } else {
            //fprintf(stderr, "no ivars\n");
        }
    }
    
    void visitClass(ContentAccessor* cache, const macho_header<P>* header, objc_class_t<P> *cls)
    {
        walk(cache, header, cls);
    }
};

// Call visitor.visitClass() on every class.
template <typename P, typename V>
class ClassWalker {
    typedef typename P::uint_t pint_t;
    V& _visitor;
public:
    
    ClassWalker(V& visitor) : _visitor(visitor) { }
    
    void walk(ContentAccessor* cache, const macho_header<P>* header)
    {   
        PointerSection<P, objc_class_t<P>*> classList(cache, header, "__DATA", "__objc_classlist");
        
        for (pint_t i = 0; i < classList.count(); i++) {
            objc_class_t<P>* cls = classList.get(i);
            //fprintf(stderr, "visiting class: %s\n", cls->getName(cache));
            if (cls) _visitor.visitClass(cache, header, cls);
        }
    }
};

// Call visitor.visitProtocol() on every protocol.
template <typename P, typename V>
class ProtocolWalker {
    typedef typename P::uint_t pint_t;
    V& _protocolVisitor;
public:
    
    ProtocolWalker(V& visitor) : _protocolVisitor(visitor) { }
    
    void walk(ContentAccessor* cache, const macho_header<P>* header)
    {   
        PointerSection<P, objc_protocol_t<P> *>
            protocols(cache, header, "__DATA", "__objc_protolist");
        
        for (pint_t i = 0; i < protocols.count(); i++) {
            objc_protocol_t<P> *proto = protocols.get(i);
            _protocolVisitor.visitProtocol(cache, header, proto);
        }
    }
};

// Call visitor.visitProtocolReference() on every protocol.
template <typename P, typename V>
class ProtocolReferenceWalker {
    typedef typename P::uint_t pint_t;
    V& _visitor;

    void visitProtocolList(ContentAccessor* cache,
                           objc_protocol_list_t<P>* protolist)
    {
        if (!protolist) return;
        for (pint_t i = 0; i < protolist->getCount(); i++) {
            pint_t oldValue = protolist->getVMAddress(i);
            pint_t newValue = _visitor.visitProtocolReference(cache, oldValue);
            protolist->setVMAddress(i, newValue);
        }
    }

    friend class ClassWalker<P, ProtocolReferenceWalker<P, V>>;

    void visitClass(ContentAccessor* cache, const macho_header<P>*,
                    objc_class_t<P>* cls)
    {
        visitProtocolList(cache, cls->getProtocolList(cache));
        visitProtocolList(cache, cls->getIsa(cache)->getProtocolList(cache));
    }

public:
    
    ProtocolReferenceWalker(V& visitor) : _visitor(visitor) { }
    void walk(ContentAccessor* cache, const macho_header<P>* header)
    {
        // @protocol expressions
        PointerSection<P, objc_protocol_t<P> *>
            protorefs(cache, header, "__DATA", "__objc_protorefs");
        for (pint_t i = 0; i < protorefs.count(); i++) {
            pint_t oldValue = protorefs.getVMAddress(i);
            pint_t newValue = _visitor.visitProtocolReference(cache, oldValue);
            protorefs.setVMAddress(i, newValue);
        }

        // protocol lists in classes
        ClassWalker<P, ProtocolReferenceWalker<P, V>> classes(*this);
        classes.walk(cache, header);

        // protocol lists from categories
        PointerSection<P, objc_category_t<P> *>
        cats(cache, header, "__DATA", "__objc_catlist");
        for (pint_t i = 0; i < cats.count(); i++) {
            objc_category_t<P> *cat = cats.get(i);
            visitProtocolList(cache, cat->getProtocols(cache));
        }

        // protocol lists in protocols
        // __objc_protolists itself is NOT updated
        PointerSection<P, objc_protocol_t<P> *>
            protocols(cache, header, "__DATA", "__objc_protolist");
        for (pint_t i = 0; i < protocols.count(); i++) {
            objc_protocol_t<P>* proto = protocols.get(i);
            visitProtocolList(cache, proto->getProtocols(cache));
            // not recursive: every old protocol object 
            // must be in some protolist section somewhere
        }
    }
};

// Call visitor.visitMethodList(mlist) on every
// class and category method list in a header.
// Call visitor.visitProtocolMethodList(mlist, typelist) on every
// protocol method list in a header.
template <typename P, typename V>
class MethodListWalker {

    typedef typename P::uint_t pint_t;

    V& mVisitor;

public: 
    
    MethodListWalker(V& visitor) : mVisitor(visitor) { }

    void walk(ContentAccessor* cache, const macho_header<P>* header)
    {   
        // Method lists in classes
        PointerSection<P, objc_class_t<P> *> 
            classes(cache, header, "__DATA", "__objc_classlist");
            
        for (pint_t i = 0; i < classes.count(); i++) {
            objc_class_t<P> *cls = classes.get(i);
            objc_method_list_t<P> *mlist;
            if ((mlist = cls->getMethodList(cache))) {
                mVisitor.visitMethodList(mlist);
            }
            if ((mlist = cls->getIsa(cache)->getMethodList(cache))) {
                mVisitor.visitMethodList(mlist);
            }
        }
        
        // Method lists from categories
        PointerSection<P, objc_category_t<P> *> 
            cats(cache, header, "__DATA", "__objc_catlist");
        for (pint_t i = 0; i < cats.count(); i++) {
            objc_category_t<P> *cat = cats.get(i);
            objc_method_list_t<P> *mlist;
            if ((mlist = cat->getInstanceMethods(cache))) {
                mVisitor.visitMethodList(mlist);
            }
            if ((mlist = cat->getClassMethods(cache))) {
                mVisitor.visitMethodList(mlist);
            }
        }

        // Method description lists from protocols
        PointerSection<P, objc_protocol_t<P> *>
            protocols(cache, header, "__DATA", "__objc_protolist");
        for (pint_t i = 0; i < protocols.count(); i++) {
            objc_protocol_t<P> *proto = protocols.get(i);
            objc_method_list_t<P> *mlist;
            pint_t *typelist = proto->getExtendedMethodTypes(cache);

            if ((mlist = proto->getInstanceMethods(cache))) {
                mVisitor.visitProtocolMethodList(mlist, typelist);
                if (typelist) typelist += mlist->getCount();
            }
            if ((mlist = proto->getClassMethods(cache))) {
                mVisitor.visitProtocolMethodList(mlist, typelist);
                if (typelist) typelist += mlist->getCount();
            }
            if ((mlist = proto->getOptionalInstanceMethods(cache))) {
                mVisitor.visitProtocolMethodList(mlist, typelist);
                if (typelist) typelist += mlist->getCount();
            }
            if ((mlist = proto->getOptionalClassMethods(cache))) {
                mVisitor.visitProtocolMethodList(mlist, typelist);
                if (typelist) typelist += mlist->getCount();
            }
        }
    }
};

// Update selector references. The visitor performs recording and uniquing.
template <typename P, typename V>
class SelectorOptimizer {

    typedef typename P::uint_t pint_t;

    V& mVisitor;

    std::set<pint_t> selectorRefVMAddrs;

    friend class MethodListWalker<P, SelectorOptimizer<P,V> >;
    void visitMethodList(objc_method_list_t<P> *mlist)
    {
        // Gather selectors. Update method names.
        for (uint32_t m = 0; m < mlist->getCount(); m++) {
            pint_t oldValue = mlist->get(m).getName();
            pint_t newValue = mVisitor.visit(oldValue);
            mlist->get(m).setName(newValue);
        }
        // Do not setFixedUp: the methods are not yet sorted.
    }

    void visitProtocolMethodList(objc_method_list_t<P> *mlist, pint_t *types)
    {
        visitMethodList(mlist);
    }

public:

    SelectorOptimizer(V& visitor) : mVisitor(visitor) { }

    void visitCoalescedStrings(const CacheBuilder::CacheCoalescedText& coalescedText) {
        mVisitor.visitCoalescedStrings(coalescedText);
    }

    void optimize(ContentAccessor* cache, const macho_header<P>* header)
    {
        // method lists in classes, categories, and protocols
        MethodListWalker<P, SelectorOptimizer<P,V> > mw(*this);
        mw.walk(cache, header);
        
        // @selector references
        PointerSection<P, const char *> 
            selrefs(cache, header, "__DATA", "__objc_selrefs");
        for (pint_t i = 0; i < selrefs.count(); i++) {
            pint_t oldValue = selrefs.getVMAddress(i);
            pint_t newValue = mVisitor.visit(oldValue);
            selrefs.setVMAddress(i, newValue);
            selectorRefVMAddrs.insert(selrefs.getSectionVMAddress() + (i * sizeof(pint_t)));
        }

        // message references
        ArraySection<P, objc_message_ref_t<P> > 
            msgrefs(cache, header, "__DATA", "__objc_msgrefs");
        for (pint_t i = 0; i < msgrefs.count(); i++) {
            objc_message_ref_t<P>& msg = msgrefs.get(i);
            pint_t oldValue = msg.getName();
            pint_t newValue = mVisitor.visit(oldValue);
            msg.setName(newValue);
        }
    }

    bool isSelectorRefAddress(pint_t vmAddr) const {
        return selectorRefVMAddrs.count(vmAddr);
    }
};


// Update selector references. The visitor performs recording and uniquing.
template <typename P>
class IvarOffsetOptimizer {
    uint32_t    _slide;
    uint32_t    _maxAlignment;
    uint32_t    _optimized;

public:
    
    IvarOffsetOptimizer() : _optimized(0) { }

    size_t optimized() const { return _optimized; }
    
    // dual purpose ivar visitor function
    // if slide!=0 then slides the ivar by that amount, otherwise computes _maxAlignment
    void visitIvar(ContentAccessor* cache, const macho_header<P>* /*unused, may be NULL*/, objc_class_t<P> *cls, objc_ivar_t<P> *ivar)
    {
        if (_slide == 0) {
            uint32_t alignment = ivar->getAlignment();
            if (alignment > _maxAlignment) _maxAlignment = alignment;
        } else {
            // skip anonymous bitfields
            if (ivar->hasOffset()) {
                uint32_t oldOffset = (uint32_t)ivar->getOffset(cache);
                ivar->setOffset(cache, oldOffset + _slide);
                _optimized++;
                //fprintf(stderr, "%d -> %d for %s.%s\n", oldOffset, oldOffset + _slide, cls->getName(cache), ivar->getName(cache));
            } else {
                //fprintf(stderr, "NULL offset\n");
            }
        }
    }
    
    // Class visitor function. Evaluates whether to slide ivars and performs slide if needed.
    // The slide algorithm is also implemented in objc. Any changes here should be reflected there also.
    void visitClass(ContentAccessor* cache, const macho_header<P>* /*unused, may be NULL*/, objc_class_t<P> *cls)
    {
        objc_class_t<P> *super = cls->getSuperclass(cache);
        if (super) {
            // Recursively visit superclasses to ensure we have the correct superclass start
            // Note that we don't need the macho_header, so just pass NULL.
            visitClass(cache, nullptr, super);

            objc_class_data_t<P> *data = cls->getData(cache);
            objc_class_data_t<P> *super_data = super->getData(cache);
            int32_t diff = super_data->getInstanceSize() - data->getInstanceStart();
            if (diff > 0) {
                IvarWalker<P, IvarOffsetOptimizer<P> > ivarVisitor(*this);
                _maxAlignment = 1;
                _slide = 0;
                
                // This walk computes _maxAlignment
                ivarVisitor.walk(cache, nullptr, cls);

                // Compute a slide value that preserves that alignment
                uint32_t alignMask = _maxAlignment - 1;
                if (diff & alignMask) diff = (diff + alignMask) & ~alignMask;

                // Slide all of this class's ivars en masse
                _slide = diff;
                if (_slide != 0) {
                    //fprintf(stderr, "Sliding ivars in %s by %u (superclass was %d, now %d)\n", cls->getName(cache), _slide, data->getInstanceStart(), super_data->getInstanceSize());
                    ivarVisitor.walk(cache, nullptr, cls);
                    data->setInstanceStart(data->getInstanceStart() + _slide);
                    data->setInstanceSize(data->getInstanceSize() + _slide);
                }
            }
        }
    }
    
    // Enumerates objc classes in the module and performs any ivar slides
    void optimize(ContentAccessor* cache, const macho_header<P>* header)
    {
        // The slide code cannot fix up GC layout strings so skip modules that support or require GC
        const macho_section<P> *imageInfoSection = header->getSection("__DATA", "__objc_imageinfo");
        if (imageInfoSection) {
            objc_image_info<P> *info = (objc_image_info<P> *)cache->contentForVMAddr(imageInfoSection->addr());
            if (!info->supportsGCFlagSet() && !info->requiresGCFlagSet()) {
                ClassWalker<P, IvarOffsetOptimizer<P> > classVisitor(*this);
                classVisitor.walk(cache, header);
            } else {
                //fprintf(stderr, "GC support present - skipped module\n");
            }
        }
    }
};


// Detect classes that have missing weak-import superclasses.
template <typename P>
class WeakClassDetector {
    bool                                noMissing;
    const std::map<void*, std::string>* missingWeakImports = nullptr;

    friend class ClassWalker<P, WeakClassDetector<P>>;
    void visitClass(ContentAccessor* cache, const macho_header<P>*, 
                    objc_class_t<P>* cls)
    {
        auto supercls = cls->getSuperclass(cache);
        if (supercls) {
            // okay: class with superclass
            // Note that the superclass itself might have a missing superclass.
            // That is fine for mere detection because we will visit the 
            // superclass separately.
        } else if (cls->isRootClass(cache)) {
            // okay: root class is expected to have no superclass
        } else {
            // bad: cls's superclass is missing.
            // See if we can find the name from the missing weak import map
            auto it = missingWeakImports->find((void*)cls->getSuperClassAddress());
            const char* dylibName = "unknown dylib";
            if (it != missingWeakImports->end()) {
                dylibName = it->second.c_str();
            }
            cache->diagnostics().warning("Superclass of class '%s' is weak-import and missing.  Expected in %s",
                                         cls->getName(cache), dylibName);
            noMissing = false;
        }
    }

public:
    bool noMissingWeakSuperclasses(ContentAccessor* cache,
                                   const std::map<void*, std::string>& missingWeakImportsMap,
                                   std::vector<const macho_header<P>*> dylibs)
    {
        noMissing           = true;
        missingWeakImports  = &missingWeakImportsMap;
        ClassWalker<P, WeakClassDetector<P>> classes(*this);
        for (auto mh : dylibs) {
            classes.walk(cache, mh);
        }
        return noMissing;
    }
};


// Sort methods in place by selector.
template <typename P>
class MethodListSorter {

    typedef typename P::uint_t pint_t;

    uint32_t _optimized;

    friend class MethodListWalker<P, MethodListSorter<P> >;
    void visitMethodList(objc_method_list_t<P> *mlist)
    {
        typename objc_method_t<P>::SortBySELAddress sorter;
        std::stable_sort(mlist->begin(), mlist->end(), sorter);
        mlist->setFixedUp();
        _optimized++;
    }

    void visitProtocolMethodList(objc_method_list_t<P> *mlist, pint_t *typelist)
    {
        typename objc_method_t<P>::SortBySELAddress sorter;
        // can't easily use std::stable_sort here
        for (uint32_t i = 0; i < mlist->getCount(); i++) {
            for (uint32_t j = i+1; j < mlist->getCount(); j++) {
                objc_method_t<P>& mi = mlist->get(i);
                objc_method_t<P>& mj = mlist->get(j);
                if (! sorter(mi, mj)) {
                    std::swap(mi, mj);
                    if (typelist) std::swap(typelist[i], typelist[j]);
                }
            }
        }

        mlist->setFixedUp();
        _optimized++;
    }

public:
    MethodListSorter() : _optimized(0) { }

    size_t optimized() const { return _optimized; }

    void optimize(ContentAccessor* cache, const macho_header<P>* header)
    {
        MethodListWalker<P, MethodListSorter<P> > mw(*this);
        mw.walk(cache, header);
    }
};


template <typename P, typename InfoT>
class HeaderInfoOptimizer {
public:

    typedef typename P::uint_t pint_t;

    HeaderInfoOptimizer() : _hInfos(0), _count(0) { }

    const char* init(uint32_t count, uint8_t*& buf, size_t& bufSize) {
        if (count == 0)
            return nullptr;

        size_t requiredSize = 
            2*sizeof(uint32_t) + count*sizeof(InfoT);
        if (bufSize < requiredSize) {
            return "libobjc's read/write section is too small (metadata not optimized)";
        }

        uint32_t *buf32 = (uint32_t *)buf;
        P::E::set32(buf32[0], count);
        P::E::set32(buf32[1], sizeof(InfoT));
        _hInfos = (InfoT*)(buf32+2);

        buf += requiredSize;
        bufSize -= requiredSize;

        return nullptr;
    }

    void update(ContentAccessor* cache, const macho_header<P>* mh, CacheBuilder::ASLR_Tracker& aslrTracker) {
        InfoT* hi = new(&_hInfos[_count++]) InfoT(cache, mh);
        (void)hi;
    }

    InfoT* hinfoForHeader(ContentAccessor* cache, const macho_header<P>* mh) {
        // FIXME: could be binary search
        uint64_t mh_vmaddr = cache->vmAddrForContent((void*)mh);
        for (size_t i = 0; i < _count; i++) {
            InfoT* hi = &_hInfos[i];
            if (hi->header_vmaddr(cache) == mh_vmaddr) return hi;
        }
        return nullptr;
    }
private:
    InfoT*                    _hInfos;
    size_t                    _count;
};
