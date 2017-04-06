/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- 
 *
 * Copyright (c) 2014 Apple Inc. All rights reserved.
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


#include "mega-dylib-utils.h"
#include "Logging.h"
#include "MachOFileAbstraction.hpp"


#include <dirent.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <mach-o/loader.h>
#include <mach-o/fat.h>
#include <assert.h>


// Scan a C++ or Swift length-mangled field.
static bool scanMangledField(const char *&string, const char *end, 
                             const char *&field, int& length)
{
    // Leading zero not allowed.
    if (*string == '0') return false;

    length = 0;
    field = string;
    while (field < end) {
        char c = *field;
        if (!isdigit(c)) break;
        field++;
        if (__builtin_smul_overflow(length, 10, &length)) return false;
        if (__builtin_sadd_overflow(length, c - '0', &length)) return false;
    }

    string = field + length;
    return length > 0  &&  string <= end;
}


// copySwiftDemangledName
// Returns the pretty form of the given Swift-mangled class or protocol name. 
// Returns nullptr if the string doesn't look like a mangled Swift name.
// The result must be freed with free().
static char *copySwiftDemangledName(const char *string, bool isProtocol = false)
{
    if (!string) return nullptr;

    // Swift mangling prefix.
    if (strncmp(string, isProtocol ? "_TtP" : "_TtC", 4) != 0) return nullptr;
    string += 4;

    const char *end = string + strlen(string);

    // Module name.
    const char *prefix;
    int prefixLength;
    if (string[0] == 's') {
        // "s" is the Swift module.
        prefix = "Swift";
        prefixLength = 5;
        string += 1;
    } else {
        if (! scanMangledField(string, end, prefix, prefixLength)) return nullptr;
    }

    // Class or protocol name.
    const char *suffix;
    int suffixLength;
    if (! scanMangledField(string, end, suffix, suffixLength)) return nullptr;

    if (isProtocol) {
        // Remainder must be "_".
        if (strcmp(string, "_") != 0) return nullptr;
    } else {
        // Remainder must be empty.
        if (string != end) return nullptr;
    }

    char *result;
    asprintf(&result, "%.*s.%.*s", prefixLength,prefix, suffixLength,suffix);
    return result;
}


class ContentAccessor {
public:
    ContentAccessor(SharedCache& cache) {
        cache.forEachRegion([&] (void* content, uint64_t vmAddr, uint64_t size, uint32_t permissions) {
            Info info = { (uint8_t*)content, (uint8_t*)content+size, vmAddr, vmAddr+size };
            _regions.push_back(info);
        });
    }

    void* contentForVMAddr(uint64_t vmaddr) {
        for (Info& info : _regions) {
            if ( (info.startAddr <= vmaddr) && (vmaddr < info.endAddr) )
                return (void*)(info.contentStart + vmaddr - info.startAddr);
        }
        if ( vmaddr == 0 )
            return nullptr;
        terminate("contentForVMAddr(0x%0llX) invalid vmaddr in ObjC data", vmaddr);
    }

    uint64_t vmAddrForContent(const void* content) {
        for (Info& info : _regions) {
            if ( (info.contentStart <= content) && (content < info.contentEnd) )
                return info.startAddr + ((uint8_t*)content - (uint8_t*)info.contentStart);
        }
        terminate("vmAddrForContent(%p) invalid content pointer in ObjC data", content);
    }

private:
    struct Info { uint8_t* contentStart; uint8_t* contentEnd; uint64_t startAddr; uint64_t endAddr; };
    std::vector<Info> _regions;
};


// Access a section containing a list of pointers
template <typename P, typename T>
class PointerSection 
{
    typedef typename P::uint_t   pint_t;
public:
    PointerSection(ContentAccessor* cache, const macho_header<P>* mh,
                   const char* segname, const char* sectname)
        : _cache(cache),
          _section(mh->getSection(segname, sectname)),
          _base(_section ? (pint_t*)cache->contentForVMAddr(_section->addr()) : 0),
          _count(_section ? (pint_t)(_section->size() / sizeof(pint_t)) : 0) {
    }

    pint_t count() const { return _count; }

    pint_t getVMAddress(pint_t index) const {
        if ( index >= _count )
            terminate("index out of range in section %s", _section->sectname());
        return (pint_t)P::getP(_base[index]);
    }

    T get(pint_t index) const {
        return (T)_cache->contentForVMAddr(getVMAddress(index));
    }

    void setVMAddress(pint_t index, pint_t value) {
        if (index >= _count)
            terminate("index out of range in section %s", _section->sectname());
        P::setP(_base[index], value);
    }

    void removeNulls() {
        pint_t shift = 0;
        for (pint_t i = 0; i < _count; i++) {
            pint_t value = _base[i];
            if (value) {
                _base[i-shift] = value;
            } else {
                shift++;
            }
        }
        _count -= shift;
        const_cast<macho_section<P>*>(_section)->set_size(_count * sizeof(pint_t));
    }

private:
    ContentAccessor* const         _cache;
    const macho_section<P>* const  _section;
    pint_t* const                  _base;
    pint_t const                   _count;
};


// Access a section containing an array of structures
template <typename P, typename T>
class ArraySection 
{
public:
    ArraySection(ContentAccessor* cache, const macho_header<P>* mh,
                 const char *segname, const char *sectname)
        : _cache(cache),
          _section(mh->getSection(segname, sectname)),
          _base(_section ? (T *)cache->contentForVMAddr(_section->addr()) : 0),
          _count(_section ? _section->size() / sizeof(T) : 0) {
    }

    uint64_t count() const { return _count; }

    T& get(uint64_t index) const { 
        if (index >= _count)
            terminate("index out of range in section %s", _section->sectname());
        return _base[index];
    }

private:
    ContentAccessor* const         _cache;
    const macho_section<P>* const  _section;
    T * const                      _base;
    uint64_t const                 _count;
};


#define SELOPT_WRITE
#include "objc-shared-cache.h"
#include "ObjC1Abstraction.hpp"
#include "ObjC2Abstraction.hpp"


namespace {



template <typename P>
class ObjCSelectorUniquer
{
public:
    typedef typename P::uint_t  pint_t;

    ObjCSelectorUniquer(ContentAccessor* cache) : _cache(cache) { }

    pint_t visit(pint_t oldValue)
    {
        _count++;
        const char *s = (const char *)_cache->contentForVMAddr(oldValue);
        objc_opt::string_map::iterator element = 
            _selectorStrings.insert(objc_opt::string_map::value_type(s, oldValue)).first;
        return (pint_t)element->second;
    }

    objc_opt::string_map& strings() { 
        return _selectorStrings;
    }

    size_t count() const { return _count; }

private:
    objc_opt::string_map    _selectorStrings;
    ContentAccessor*        _cache;
    size_t                  _count = 0;
};


template <typename P>
class ClassListBuilder
{
private:
    objc_opt::string_map    _classNames;
    objc_opt::class_map     _classes;
    size_t                  _count = 0;
    HeaderInfoOptimizer<P, objc_header_info_ro_t<P>>& _hInfos;

public:

    ClassListBuilder(HeaderInfoOptimizer<P, objc_header_info_ro_t<P>>& hinfos) : _hInfos(hinfos) { }

    void visitClass(ContentAccessor* cache,
                    const macho_header<P>* header,
                    objc_class_t<P>* cls)
    {
        if (cls->isMetaClass(cache)) return;

        const char *name = cls->getName(cache);
        uint64_t name_vmaddr = cache->vmAddrForContent((void*)name);
        uint64_t cls_vmaddr = cache->vmAddrForContent(cls);
        uint64_t hinfo_vmaddr = cache->vmAddrForContent(_hInfos.hinfoForHeader(cache, header));
        _classNames.insert(objc_opt::string_map::value_type(name, name_vmaddr));
        _classes.insert(objc_opt::class_map::value_type(name, std::pair<uint64_t, uint64_t>(cls_vmaddr, hinfo_vmaddr)));
        _count++;
    }

    objc_opt::string_map& classNames() { 
        return _classNames;
    }

    objc_opt::class_map& classes() { 
        return _classes;
    }

    size_t count() const { return _count; }
};

template <typename P>
class ProtocolOptimizer
{
private:
    typedef typename P::uint_t pint_t;

    objc_opt::string_map    _protocolNames;
    objc_opt::protocol_map  _protocols;
    size_t                  _protocolCount;
    size_t                  _protocolReferenceCount;

    friend class ProtocolReferenceWalker<P, ProtocolOptimizer<P>>;

    pint_t visitProtocolReference(ContentAccessor* cache, pint_t oldValue)
    {
        objc_protocol_t<P>* proto = (objc_protocol_t<P>*)
            cache->contentForVMAddr(oldValue);
        pint_t newValue = (pint_t)_protocols[proto->getName(cache)];
        if (oldValue != newValue) _protocolReferenceCount++;
        return newValue;
    }

public:

    ProtocolOptimizer()
        : _protocolNames()
        , _protocols()
        , _protocolCount(0)
        , _protocolReferenceCount(0)
    { }

    void addProtocols(ContentAccessor* cache,
                      const macho_header<P>* header)
    {
        PointerSection<P, objc_protocol_t<P> *>
            protocols(cache, header, "__DATA", "__objc_protolist");
        
        for (pint_t i = 0; i < protocols.count(); i++) {
            objc_protocol_t<P> *proto = protocols.get(i);

            const char *name = proto->getName(cache);
            if (_protocolNames.count(name) == 0) {
                if (proto->getSize() > sizeof(objc_protocol_t<P>)) {
                    terminate("objc protocol is too big");
                }

                uint64_t name_vmaddr = cache->vmAddrForContent((void*)name);
                uint64_t proto_vmaddr = cache->vmAddrForContent(proto);
                _protocolNames.insert(objc_opt::string_map::value_type(name, name_vmaddr));
                _protocols.insert(objc_opt::protocol_map::value_type(name, proto_vmaddr));
                _protocolCount++;
            }
        }
    }

    const char *writeProtocols(ContentAccessor* cache,
                               uint8_t *& rwdest, size_t& rwremaining,
                               uint8_t *& rodest, size_t& roremaining, 
                               std::vector<void*>& pointersInData, 
                               pint_t protocolClassVMAddr)
    {
        if (_protocolCount == 0) return NULL;

        if (protocolClassVMAddr == 0) {
            return "libobjc's Protocol class symbol not found (metadata not optimized)";
        }

        size_t rwrequired = _protocolCount * sizeof(objc_protocol_t<P>);
        if (rwremaining < rwrequired) {
            return "libobjc's read-write section is too small (metadata not optimized)";
        }

        for (objc_opt::protocol_map::iterator iter = _protocols.begin();
             iter != _protocols.end();
             ++iter)
        {
            objc_protocol_t<P>* oldProto = (objc_protocol_t<P>*)
                cache->contentForVMAddr(iter->second);

            // Create a new protocol object.
            objc_protocol_t<P>* proto = (objc_protocol_t<P>*)rwdest;
            rwdest += sizeof(*proto);
            rwremaining -= sizeof(*proto);

            // Initialize it.
            uint32_t oldSize = oldProto->getSize();
            memcpy(proto, oldProto, oldSize);
            if (!proto->getIsaVMAddr()) {
                proto->setIsaVMAddr(protocolClassVMAddr);
            }
            if (oldSize < sizeof(*proto)) {
                // Protocol object is old. Populate new fields.
                proto->setSize(sizeof(objc_protocol_t<P>));
                // missing extendedMethodTypes is already nil
            }
            // Some protocol objects are big enough to have the 
            // demangledName field but don't initialize it.
            // Initialize it here if it is not already set.
            if (!proto->getDemangledName(cache)) {
                const char *roName = proto->getName(cache);
                char *demangledName = copySwiftDemangledName(roName, true);
                if (demangledName) {
                    size_t length = 1 + strlen(demangledName);
                    if (roremaining < length) {
                        return "libobjc's read-only section is too small (metadata not optimized)";
                    }

                    memmove(rodest, demangledName, length);
                    roName = (const char *)rodest;
                    rodest += length;
                    roremaining -= length;

                    free(demangledName);
                }
                proto->setDemangledName(cache, roName);
            }
            proto->setFixedUp();

            // Redirect the protocol table at our new object.
            iter->second = cache->vmAddrForContent(proto);

            // Add new rebase entries.
            proto->addPointers(pointersInData);
        }
        
        return NULL;
    }

    void updateReferences(ContentAccessor* cache, const macho_header<P>* header)
    {
        ProtocolReferenceWalker<P, ProtocolOptimizer<P>> refs(*this);
        refs.walk(cache, header);
    }

    objc_opt::string_map& protocolNames() { 
        return _protocolNames;
    }

    objc_opt::protocol_map& protocols() { 
        return _protocols;
   }

    size_t protocolCount() const { return _protocolCount; }
    size_t protocolReferenceCount() const { return _protocolReferenceCount; }
};


static int percent(size_t num, size_t denom) {
    if (denom)
        return (int)(num / (double)denom * 100);
    else
        return 100;
}


template <typename P>
void optimizeObjC(SharedCache& cache, std::vector<void*>& pointersForASLR, bool forProduction)
{
    typedef typename P::E           E;
    typedef typename P::uint_t      pint_t;

    verboseLog("Optimizing objc metadata:");
    verboseLog("  cache type is %s", 
               forProduction ? "production" : "development");

    ContentAccessor cacheAccessor(cache);

    size_t headerSize = P::round_up(sizeof(objc_opt::objc_opt_t));
    if (headerSize != sizeof(objc_opt::objc_opt_t)) {
        warning("libobjc's optimization structure size is wrong (metadata not optimized)");
    }

    //
    // Find libobjc's empty sections and build list of images with objc metadata
    //
    const macho_section<P> *optROSection = nullptr;
    const macho_section<P> *optRWSection = nullptr;
    const macho_section<P> *optPointerListSection = nullptr;
    std::vector<const macho_header<P>*> objcDylibs;
    cache.forEachImage([&](const void* machHeader, const char* installName,
                        time_t, ino_t, const std::vector<MachOProxy::Segment>& segments) {
        const macho_header<P>* mh = (const macho_header<P>*)machHeader;
        if ( strstr(installName, "/libobjc.") != nullptr ) {
            optROSection = mh->getSection("__TEXT", "__objc_opt_ro");
            optRWSection = mh->getSection("__DATA", "__objc_opt_rw");
            optPointerListSection = mh->getSection("__DATA", "__objc_opt_ptrs");
        }
        if ( mh->getSection("__DATA", "__objc_imageinfo") || mh->getSection("__OBJC", "__image_info") ) {
            objcDylibs.push_back(mh);
        }
        // log("installName %s at mhdr 0x%016lx", installName, (uintptr_t)cacheAccessor.vmAddrForContent((void*)mh));
    });
    if ( optROSection == nullptr ) {
        warning("libobjc's read-only section missing (metadata not optimized)");
        return;
    }
    if ( optRWSection == nullptr ) {
        warning("libobjc's read/write section missing (metadata not optimized)");
        return;
    }
    if ( optPointerListSection == nullptr ) {
        warning("libobjc's pointer list section missing (metadata not optimized)");
        return;
    }

    uint8_t* optROData = (uint8_t*)cacheAccessor.contentForVMAddr(optROSection->addr());
    size_t optRORemaining = optROSection->size();
    uint8_t* optRWData = (uint8_t*)cacheAccessor.contentForVMAddr(optRWSection->addr());
    size_t optRWRemaining = optRWSection->size();
    if (optRORemaining < headerSize) {
        warning("libobjc's read-only section is too small (metadata not optimized)");
        return;
    }
    objc_opt::objc_opt_t* optROHeader = (objc_opt::objc_opt_t *)optROData;
    optROData += headerSize;
    optRORemaining -= headerSize;
    if (E::get32(optROHeader->version) != objc_opt::VERSION) {
        warning("libobjc's read-only section version is unrecognized (metadata not optimized)");
        return;
    }

    if (optPointerListSection->size() < sizeof(objc_opt::objc_opt_pointerlist_tt<pint_t>)) {
        warning("libobjc's pointer list section is too small (metadata not optimized)");
        return;
    }
    const objc_opt::objc_opt_pointerlist_tt<pint_t> *optPointerList = (const objc_opt::objc_opt_pointerlist_tt<pint_t> *)cacheAccessor.contentForVMAddr(optPointerListSection->addr());

    // Write nothing to optROHeader until everything else is written.
    // If something fails below, libobjc will not use the section.


    //
    // Make copy of objcList and sort that list.
    //
    std::vector<const macho_header<P>*> addressSortedDylibs = objcDylibs;
    std::sort(addressSortedDylibs.begin(), addressSortedDylibs.end(), [](const macho_header<P>* lmh, const macho_header<P>* rmh) -> bool {
        return lmh < rmh;
    });

    //
    // Build HeaderInfo list in cache
    //
    // First the RO header info
    // log("writing out %d RO dylibs at offset %d", (uint32_t)objcDylibs.size(), (uint32_t)(optROSection->size() - optRORemaining));
    uint64_t hinfoROVMAddr = optROSection->addr() + optROSection->size() - optRORemaining;
    HeaderInfoOptimizer<P, objc_header_info_ro_t<P>> hinfoROOptimizer;
    const char* err = hinfoROOptimizer.init((uint32_t)objcDylibs.size(), optROData, optRORemaining);
    if (err) {
        warning("%s", err);
        return;
    }
    else {
        for (const macho_header<P>* mh : addressSortedDylibs) {
            hinfoROOptimizer.update(&cacheAccessor, mh, pointersForASLR);
        }
    }

    // Then the RW header info
    // log("writing out %d RW dylibs at offset %d", (uint32_t)objcDylibs.size(), (uint32_t)(optRWSection->size() - optRWRemaining));
    uint64_t hinfoRWVMAddr = (uint64_t)optRWSection->addr() + (uint64_t)optRWSection->size() - optRWRemaining;
    HeaderInfoOptimizer<P, objc_header_info_rw_t<P>> hinfoRWOptimizer;
    err = hinfoRWOptimizer.init((uint32_t)objcDylibs.size(), optRWData, optRWRemaining);
    if (err) {
        warning("%s", err);
        return;
    }
    else {
        for (const macho_header<P>* mh : addressSortedDylibs) {
            hinfoRWOptimizer.update(&cacheAccessor, mh, pointersForASLR);
        }
    }

    //
    // Update selector references and build selector list
    //
    // This is SAFE: if we run out of room for the selector table, 
    // the modified binaries are still usable.
    //
    // Heuristic: choose selectors from libraries with more selector cstring data first.
    // This tries to localize selector cstring memory.
    //
    ObjCSelectorUniquer<P> uniq(&cacheAccessor);
    std::vector<const macho_header<P>*> sizeSortedDylibs = objcDylibs;
    std::sort(sizeSortedDylibs.begin(), sizeSortedDylibs.end(),  [](const macho_header<P>* lmh, const macho_header<P>* rmh) -> bool {
        const macho_section<P>* lSection = lmh->getSection("__TEXT", "__objc_methname");
        const macho_section<P>* rSection = rmh->getSection("__TEXT", "__objc_methname");
        uint64_t lSelectorSize = (lSection ? lSection->size() : 0);
        uint64_t rSelectorSize = (rSection ? rSection->size() : 0);
        return lSelectorSize > rSelectorSize;
    });

    SelectorOptimizer<P, ObjCSelectorUniquer<P> > selOptimizer(uniq);
    for (const macho_header<P>* mh : sizeSortedDylibs) {
        LegacySelectorUpdater<P, ObjCSelectorUniquer<P>>::update(&cacheAccessor, mh, uniq);
        selOptimizer.optimize(&cacheAccessor, mh);
    }

    verboseLog("  uniqued  % 6ld selectors",
               uniq.strings().size());
    verboseLog("  updated  % 6ld selector references",
               uniq.count());

    uint64_t seloptVMAddr = optROSection->addr() + optROSection->size() - optRORemaining;
    objc_opt::objc_selopt_t *selopt = new(optROData) objc_opt::objc_selopt_t;
    err = selopt->write(seloptVMAddr, optRORemaining, uniq.strings());
    if (err) {
        warning("%s", err);
        return;
    }
    optROData += selopt->size();
    optRORemaining -= selopt->size();
    uint32_t seloptCapacity = selopt->capacity;
    uint32_t seloptOccupied = selopt->occupied;
    selopt->byteswap(E::little_endian), selopt = nullptr;

    verboseLog("  selector table occupancy %u/%u (%u%%)",
               seloptOccupied, seloptCapacity, 
               (unsigned)(seloptOccupied/(double)seloptCapacity*100));


    // 
    // Detect classes that have missing weak-import superclasses.
    // 
    // Production only. Development cache does not do this: a replacement 
    // library could omit a class at runtime that was present during 
    // cache construction.
    // 
    // This is SAFE: the binaries themselves are unmodified.
    bool noMissingWeakSuperclasses = false; // dev cache can't promise otherwise
    if (forProduction) {
        WeakClassDetector<P> weakopt;
        noMissingWeakSuperclasses = 
            weakopt.noMissingWeakSuperclasses(&cacheAccessor, sizeSortedDylibs);

        // Shared cache does not currently support unbound weak references. 
        // Here we assert that there are none. If support is added later then 
        // this assertion needs to be removed and this path needs to be tested.
        if (!noMissingWeakSuperclasses) {
            terminate("Some Objective-C class has a superclass that is "
                      "weak-import and missing from the cache.");
        }
    }


    //
    // Build class table.
    //
    // This is SAFE: the binaries themselves are unmodified.
    ClassListBuilder<P> classes(hinfoROOptimizer);
    ClassWalker<P, ClassListBuilder<P>> classWalker(classes);
    for (const macho_header<P>* mh : sizeSortedDylibs) {
        classWalker.walk(&cacheAccessor, mh);
    }

    verboseLog("  recorded % 6ld classes",
               classes.classNames().size());

    uint64_t clsoptVMAddr = optROSection->addr() + optROSection->size() - optRORemaining;
    objc_opt::objc_clsopt_t *clsopt = new(optROData) objc_opt::objc_clsopt_t;
    err = clsopt->write(clsoptVMAddr, optRORemaining, 
                        classes.classNames(), classes.classes(), false);
    if (err) {
        warning("%s", err);
        return;
    }
    optROData += clsopt->size();
    optRORemaining -= clsopt->size();
    size_t duplicateCount = clsopt->duplicateCount();
    uint32_t clsoptCapacity = clsopt->capacity;
    uint32_t clsoptOccupied = clsopt->occupied;
    clsopt->byteswap(E::little_endian);
    clsopt = nullptr;

    verboseLog("  found    % 6ld duplicate classes",
               duplicateCount);
    verboseLog("  class table occupancy %u/%u (%u%%)",
               clsoptOccupied, clsoptCapacity, 
               (unsigned)(clsoptOccupied/(double)clsoptCapacity*100));


    //
    // Sort method lists.
    //
    // This is SAFE: modified binaries are still usable as unsorted lists.
    // This must be done AFTER uniquing selectors.
    MethodListSorter<P> methodSorter;
    for (const macho_header<P>* mh : sizeSortedDylibs) {
        methodSorter.optimize(&cacheAccessor, mh);
    }

    verboseLog("  sorted   % 6ld method lists",
               methodSorter.optimized());


    // Unique protocols and build protocol table.

    // This is SAFE: no protocol references are updated yet
    // This must be done AFTER updating method lists.

    ProtocolOptimizer<P> protocolOptimizer;
    for (const macho_header<P>* mh : sizeSortedDylibs) {
        protocolOptimizer.addProtocols(&cacheAccessor, mh);
    }

    verboseLog("  uniqued  % 6ld protocols",
               protocolOptimizer.protocolCount());

    pint_t protocolClassVMAddr = (pint_t)P::getP(optPointerList->protocolClass);
    err = protocolOptimizer.writeProtocols(&cacheAccessor,
                                           optRWData, optRWRemaining,
                                           optROData, optRORemaining,
                                           pointersForASLR, protocolClassVMAddr);
    if (err) {
        warning("%s", err);
        return;
    }

    uint64_t protocoloptVMAddr = optROSection->addr() + optROSection->size() - optRORemaining;
    objc_opt::objc_protocolopt_t *protocolopt = new (optROData) objc_opt::objc_protocolopt_t;
    err = protocolopt->write(protocoloptVMAddr, optRORemaining, 
                             protocolOptimizer.protocolNames(), 
                             protocolOptimizer.protocols(), true);
    if (err) {
        warning("%s", err);
        return;
    }
    optROData += protocolopt->size();
    optRORemaining -= protocolopt->size();
    uint32_t protocoloptCapacity = protocolopt->capacity;
    uint32_t protocoloptOccupied = protocolopt->occupied;
    protocolopt->byteswap(E::little_endian), protocolopt = NULL;

    verboseLog("  protocol table occupancy %u/%u (%u%%)",
               protocoloptOccupied, protocoloptCapacity, 
               (unsigned)(protocoloptOccupied/(double)protocoloptCapacity*100));


    // Redirect protocol references to the uniqued protocols.

    // This is SAFE: the new protocol objects are still usable as-is.
    for (const macho_header<P>* mh : sizeSortedDylibs) {
        protocolOptimizer.updateReferences(&cacheAccessor, mh);
    }

    verboseLog("  updated  % 6ld protocol references",
               protocolOptimizer.protocolReferenceCount());


    //
    // Repair ivar offsets.
    //
    // This is SAFE: the runtime always validates ivar offsets at runtime.
    IvarOffsetOptimizer<P> ivarOffsetOptimizer;
    for (const macho_header<P>* mh : sizeSortedDylibs) {
        ivarOffsetOptimizer.optimize(&cacheAccessor, mh);
    }
    
    verboseLog("  updated  % 6ld ivar offsets",
               ivarOffsetOptimizer.optimized());


    // Collect flags.
    uint32_t headerFlags = 0;
    if (forProduction) {
        headerFlags |= objc_opt::IsProduction;
    }
    if (noMissingWeakSuperclasses) {
        headerFlags |= objc_opt::NoMissingWeakSuperclasses;
    }


    // Success. Mark dylibs as optimized.
    for (const macho_header<P>* mh : sizeSortedDylibs) {
        const macho_section<P>* imageInfoSection = mh->getSection("__DATA", "__objc_imageinfo");
        if (!imageInfoSection) {
            imageInfoSection = mh->getSection("__OBJC", "__image_info");
        }
        if (imageInfoSection) {
            objc_image_info<P>* info = (objc_image_info<P>*)cacheAccessor.contentForVMAddr(imageInfoSection->addr());
            info->setOptimizedByDyld();
        }
    }


    // Success. Update RO header last.
    E::set32(optROHeader->flags, headerFlags);
    E::set32(optROHeader->selopt_offset, (uint32_t)(seloptVMAddr - optROSection->addr()));
    E::set32(optROHeader->clsopt_offset, (uint32_t)(clsoptVMAddr - optROSection->addr()));
    E::set32(optROHeader->protocolopt_offset, (uint32_t)(protocoloptVMAddr - optROSection->addr()));
    E::set32(optROHeader->headeropt_ro_offset, (uint32_t)(hinfoROVMAddr - optROSection->addr()));
    E::set32(optROHeader->headeropt_rw_offset, (uint32_t)(hinfoRWVMAddr - optROSection->addr()));

    // Log statistics.
    size_t roSize = optROSection->size() - optRORemaining;
    size_t rwSize = optRWSection->size() - optRWRemaining;
    verboseLog("  %zu/%llu bytes "
               "(%d%%) used in libobjc read-only optimization section",
                roSize, optROSection->size(),
                percent(roSize, optROSection->size()));
    verboseLog("  %zu/%llu bytes "
               "(%d%%) used in libobjc read/write optimization section",
                rwSize, optRWSection->size(),
                percent(rwSize, optRWSection->size()));
    verboseLog("  wrote objc metadata optimization version %d",
                objc_opt::VERSION);
}


} // anon namespace


void SharedCache::optimizeObjC(bool forProduction)
{
     switch ( _arch.arch ) {
        case CPU_TYPE_ARM:
        case CPU_TYPE_I386:
            ::optimizeObjC<Pointer32<LittleEndian>>(*this, _pointersForASLR, forProduction);
            break;
        case CPU_TYPE_X86_64:
        case CPU_TYPE_ARM64:
            ::optimizeObjC<Pointer64<LittleEndian>>(*this, _pointersForASLR, forProduction);
            break;
        default:
            terminate("unsupported arch 0x%08X", _arch.arch);
    }
}
