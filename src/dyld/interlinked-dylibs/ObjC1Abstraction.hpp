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

#define OBJC_IMAGE_SUPPORTS_GC (1<<1)
#define OBJC_IMAGE_REQUIRES_GC (1<<2)

template <typename P>
struct objc_image_info {
    uint32_t version;
    uint32_t flags;

    uint32_t getFlags()         INLINE { return P::E::get32(flags); }
    
    bool supportsGCFlagSet()    INLINE { return getFlags() & OBJC_IMAGE_SUPPORTS_GC; }
    bool requiresGCFlagSet()    INLINE { return getFlags() & OBJC_IMAGE_REQUIRES_GC; }
    
    void setFlag(uint32_t bits) INLINE { uint32_t old = P::E::get32(flags); P::E::set32(flags, old | bits); }
    void setOptimizedByDyld() INLINE { setFlag(1<<3); }
};

template <typename P>
struct objc_method {
    uint32_t method_name;   // SEL
    uint32_t method_types;  // char *
    uint32_t method_imp;    // IMP     
    
    uint32_t getName() const INLINE { return P::E::get32(method_name); }
    void setName(uint32_t newName) INLINE { P::E::set32(method_name, newName); }
};

template <typename P>
struct objc_method_list {
    enum { OBJC_FIXED_UP = 1771 };
    uint32_t obsolete;      // struct objc_method_list *
    uint32_t method_count;  // int
    struct objc_method<P> method_list[0];
    
    uint32_t getCount() const INLINE { return P::E::get32(method_count); }
    void setFixedUp(bool fixed) INLINE { P::E::set32(obsolete, fixed ? OBJC_FIXED_UP : 0); }
};

template <typename P>
struct objc_class {
    uint32_t isa;            // struct objc_class *
    uint32_t super_class;    // struct objc_class *
    uint32_t name;           // const char *
    uint32_t version;        // long
    uint32_t info;           // long
    uint32_t instance_size;  // long
    uint32_t ivars;          // struct objc_ivar_list *
    uint32_t methodList;     // struct objc_method_list *
    uint32_t method_cache;   // struct objc_cache *
    uint32_t protocols;      // objc_protocol_list *
    uint32_t ivar_layout;    // const char *
    uint32_t ext;            // struct objc_class_ext *

    struct objc_class<P> *getIsa(ContentAccessor* cache) const INLINE { return (struct objc_class<P> *)cache->contentForVMAddr(P::E::get32(isa)); }
    struct objc_method_list<P> *getMethodList(ContentAccessor* cache) const INLINE { return (struct objc_method_list<P> *)cache->contentForVMAddr(P::E::get32(methodList)); }
};

template <typename P>
struct objc_category {
    uint32_t category_name;        // char *
    uint32_t class_name;           // char *
    uint32_t instance_methods;     // struct objc_method_list *
    uint32_t class_methods;        // struct objc_method_list *
    uint32_t protocols;            // objc_protocol_list *
    uint32_t size;                 // uint32_t
    uint32_t instance_properties;  // struct objc_property_list *
    
    struct objc_method_list<P> *getInstanceMethods(ContentAccessor* cache) const INLINE { return (struct objc_method_list<P> *)cache->contentForVMAddr(P::E::get32(instance_methods)); }
    struct objc_method_list<P> *getClassMethods(ContentAccessor* cache) const INLINE { return (struct objc_method_list<P> *)cache->contentForVMAddr(P::E::get32(class_methods)); }
};

template <typename P>
struct objc_symtab {
    uint32_t sel_ref_cnt;  // unsigned long
    uint32_t refs;         // SEL *
    uint16_t cls_def_cnt;  // unsigned short
    uint16_t cat_def_cnt;  // unsigned short
    uint32_t defs[0];      // void *
    
    uint16_t getClassCount(void) const INLINE { return P::E::get16(cls_def_cnt); }
    uint16_t getCategoryCount(void) const INLINE { return P::E::get16(cat_def_cnt); }
    struct objc_class<P> *getClass(ContentAccessor* cache, int index) const INLINE { return (struct objc_class<P> *)cache->contentForVMAddr(P::E::get32(defs[index])); }
    struct objc_category<P> *getCategory(ContentAccessor* cache, int index) const INLINE { return (struct objc_category<P> *)cache->contentForVMAddr(P::E::get32(defs[getClassCount() + index])); }
};

template <typename P>
struct objc_module {
    uint32_t version;  // unsigned long
    uint32_t size;     // unsigned long
    uint32_t name;     // char*
    uint32_t symtab;   // Symtab
    
    struct objc_symtab<P> *getSymtab(ContentAccessor* cache) const INLINE { return (struct objc_symtab<P> *)cache->contentForVMAddr(P::E::get32(symtab)); }
};

template <typename P>
struct objc_method_description {
    uint32_t name;   // SEL
    uint32_t types;  // char *
    
    uint32_t getName() const INLINE { return P::E::get32(name); }
    void setName(uint32_t newName) INLINE { P::E::set32(name, newName); }
};

template <typename P>
struct objc_method_description_list {
    uint32_t count;  // int
    struct objc_method_description<P> list[0];
    
    uint32_t getCount() const INLINE { return P::E::get32(count); }
};

template <typename P>
struct objc_protocol {
    uint32_t isa;               // danger! contains strange values!
    uint32_t protocol_name;     // const char *
    uint32_t protocol_list;     // struct objc_protocol_list
    uint32_t instance_methods;  // struct objc_method_description_list *
    uint32_t class_methods;     // struct objc_method_description_list *
    
    struct objc_method_description_list<P> *getInstanceMethodDescriptions(ContentAccessor* cache) const INLINE { return (struct objc_method_description_list<P> *)cache->contentForVMAddr(P::E::get32(instance_methods)); }
    struct objc_method_description_list<P> *getClassMethodDescriptions(ContentAccessor* cache) const INLINE { return (struct objc_method_description_list<P> *)cache->contentForVMAddr(P::E::get32(class_methods)); }
};


template <typename P, typename V>
class LegacySelectorUpdater {
    typedef typename P::uint_t pint_t;

    static void visitMethodList(objc_method_list<P> *mlist, V& visitor)
    {
        for (uint32_t m = 0; m < mlist->getCount(); m++) {
            pint_t oldValue = mlist->method_list[m].getName();
            pint_t newValue = visitor.visit(oldValue);
            mlist->method_list[m].setName((uint32_t)newValue);
        }
        mlist->setFixedUp(true);
    }

    static void visitMethodDescriptionList(objc_method_description_list<P> *mlist, V& visitor)
    {
        for (pint_t m = 0; m < mlist->getCount(); m++) {
            pint_t oldValue = mlist->list[m].getName();
            pint_t newValue = visitor.visit(oldValue);
            mlist->list[m].setName((uint32_t)newValue);
        }
    }

public:

    static void update(ContentAccessor* cache, const macho_header<P>* header, V& visitor)
    {
        ArraySection<P, objc_module<P> >
            modules(cache, header, "__OBJC", "__module_info");
        for (uint64_t m = 0; m < modules.count(); m++) {
            objc_symtab<P> *symtab = modules.get(m).getSymtab(cache);
            if (!symtab) continue;

            // Method lists in classes
            for (int c = 0; c < symtab->getClassCount(); c++) {
                objc_class<P> *cls = symtab->getClass(cache, c);
                objc_class<P> *isa = cls->getIsa(cache);
                objc_method_list<P> *mlist;
                if ((mlist = cls->getMethodList(cache))) {
                    visitMethodList(mlist, visitor);
                }
                if ((mlist = isa->getMethodList(cache))) {
                    visitMethodList(mlist, visitor);
                }
            }
            
            // Method lists from categories
            for (int c = 0; c < symtab->getCategoryCount(); c++) {
                objc_category<P> *cat = symtab->getCategory(cache, c);
                objc_method_list<P> *mlist;
                if ((mlist = cat->getInstanceMethods(cache))) {
                    visitMethodList(mlist, visitor);
                }
                if ((mlist = cat->getClassMethods(cache))) {
                    visitMethodList(mlist, visitor);
                }
            }
        }

        // Method description lists from protocols        
        ArraySection<P, objc_protocol<P>>
            protocols(cache, header, "__OBJC", "__protocol");
        for (uint64_t p = 0; p < protocols.count(); p++) {
            objc_protocol<P>& protocol = protocols.get(p);
            objc_method_description_list<P> *mlist;
            if ((mlist = protocol.getInstanceMethodDescriptions(cache))) {
                visitMethodDescriptionList(mlist, visitor);
            }
            if ((mlist = protocol.getClassMethodDescriptions(cache))) {
                visitMethodDescriptionList(mlist, visitor);
            }
        }

        // Message refs
        PointerSection<P, const char *> selrefs(cache, header, "__OBJC", "__message_refs");
        for (pint_t s = 0; s < selrefs.count(); s++) {
            pint_t oldValue = selrefs.getVMAddress(s);
            pint_t newValue = visitor.visit(oldValue);
            selrefs.setVMAddress(s, newValue);
        }
    }
};
