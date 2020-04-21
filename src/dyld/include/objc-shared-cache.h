/* 
 * Copyright (c) 2008 Apple Inc. All rights reserved.
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

/*
Portions derived from:

--------------------------------------------------------------------
lookup8.c, by Bob Jenkins, January 4 1997, Public Domain.
hash(), hash2(), hash3, and mix() are externally useful functions.
Routines to test the hash are included if SELF_TEST is defined.
You can use this free for any purpose.  It has no warranty.
--------------------------------------------------------------------

------------------------------------------------------------------------------
perfect.c: code to generate code for a hash for perfect hashing.
(c) Bob Jenkins, September 1996, December 1999
You may use this code in any way you wish, and it is free.  No warranty.
I hereby place this in the public domain.
Source is http://burtleburtle.net/bob/c/perfect.c
------------------------------------------------------------------------------
*/

/*
 * objc-selopt.h
 * Interface between libobjc and dyld 
 * for selector uniquing in the dyld shared cache.
 *
 * When building the shared cache, dyld locates all selectors and selector 
 * references in the cached images. It builds a perfect hash table out of 
 * them and writes the table into the shared cache copy of libobjc.
 * libobjc then uses that table as the builtin selector list.
 *
 * Versioning
 * The table has a version number. dyld and objc can both ignore the table 
 * if the other used the wrong version number.
 *
 * Completeness
 * Not all libraries are in the shared cache. Libraries that are in the 
 * shared cache and were optimized are specially marked. Libraries on 
 * disk never include those marks.
 *
 * Coherency
 * Libraries optimized in the shared cache can be replaced by unoptimized 
 * copies from disk when loaded. The copy from disk is not marked and will 
 * be fixed up by libobjc. The shared cache copy is still mapped into the 
 * process, so the table can point to cstring data in that library's part 
 * of the shared cache without trouble.
 * 
 * Atomicity
 * dyld writes the table itself last. If dyld marks some metadata as 
 * updated but then fails to write a table for some reason, libobjc 
 * fixes up all metadata as if it were not marked.
 */

#ifndef _OBJC_SELOPT_H
#define _OBJC_SELOPT_H

/*
  DO NOT INCLUDE ANY objc HEADERS HERE
  dyld USES THIS FILE AND CANNOT SEE THEM
*/
#include <stdint.h>
#include <stdlib.h>
#ifdef CLOSURE_SELOPT_WRITE
#include "Array.h"
#include "Map.h"
#endif
#ifdef SELOPT_WRITE
#include <unordered_map>
#endif
/*
  DO NOT INCLUDE ANY objc HEADERS HERE
  dyld USES THIS FILE AND CANNOT SEE THEM
*/

#ifndef STATIC_ASSERT
#   define STATIC_ASSERT(x) _STATIC_ASSERT2(x, __LINE__)
#   define _STATIC_ASSERT2(x, line) _STATIC_ASSERT3(x, line)
#   define _STATIC_ASSERT3(x, line)                                     \
        typedef struct {                                                \
            int _static_assert[(x) ? 0 : -1];                           \
        } _static_assert_ ## line __attribute__((unavailable)) 
#endif

#define SELOPT_DEBUG 0

#define S32(x) x = little_endian ? OSSwapHostToLittleInt32(x) : OSSwapHostToBigInt32(x)
#define S64(x) x = little_endian ? OSSwapHostToLittleInt64(x) : OSSwapHostToBigInt64(x)

namespace objc_opt {

typedef int32_t objc_stringhash_offset_t;
typedef uint8_t objc_stringhash_check_t;

static uint64_t lookup8( uint8_t *k, size_t length, uint64_t level);

#if defined(SELOPT_WRITE) || defined(CLOSURE_SELOPT_WRITE)

// Perfect hash code is at the end of this file.

struct __attribute__((packed)) perfect_hash {
    uint32_t capacity;
    uint32_t occupied;
    uint32_t shift;
    uint32_t mask;
    uint64_t salt;

    uint32_t scramble[256];
    dyld3::OverflowSafeArray<uint8_t> tab; // count == mask+1
    
    perfect_hash() { }
    
    ~perfect_hash() { }
};

struct eqstr {
    bool operator()(const char* s1, const char* s2) const {
        return strcmp(s1, s2) == 0;
    }
};

struct hashstr {
    size_t operator()(const char *s) const {
        return (size_t)lookup8((uint8_t *)s, strlen(s), 0);
    }
};

#endif // defined(SELOPT_WRITE) || defined(CLOSURE_SELOPT_WRITE)

#ifdef SELOPT_WRITE

// cstring => cstring's vmaddress
// (used for selector names and class names)
typedef std::unordered_map<const char *, uint64_t, hashstr, eqstr> string_map;

// protocol name => protocol vmaddress
typedef std::unordered_map<const char *, uint64_t, hashstr, eqstr> legacy_protocol_map;

// protocol name => (protocol vmaddress, header_info vmaddress)
typedef std::unordered_multimap<const char *, std::pair<uint64_t, uint64_t>, hashstr, eqstr> protocol_map;

// class name => (class vmaddress, header_info vmaddress)
typedef std::unordered_multimap<const char *, std::pair<uint64_t, uint64_t>, hashstr, eqstr> class_map;

static void make_perfect(const string_map& strings, perfect_hash& phash);

#endif // defined(SELOPT_WRITE)


// Precomputed perfect hash table of strings.
// Base class for precomputed selector table and class table.
// Edit objc-sel-table.s if you change this structure.
struct __attribute__((packed)) objc_stringhash_t {
    uint32_t capacity;
    uint32_t occupied;
    uint32_t shift;
    uint32_t mask;
    uint32_t unused1;  // was zero
    uint32_t unused2;  // alignment pad
    uint64_t salt;

    uint32_t scramble[256];
    uint8_t tab[0];                   /* tab[mask+1] (always power-of-2) */
    // uint8_t checkbytes[capacity];  /* check byte for each string */
    // int32_t offsets[capacity];     /* offsets from &capacity to cstrings */

    objc_stringhash_check_t *checkbytes() { return (objc_stringhash_check_t *)&tab[mask+1]; }
    const objc_stringhash_check_t *checkbytes() const { return (const objc_stringhash_check_t *)&tab[mask+1]; }

    objc_stringhash_offset_t *offsets() { return (objc_stringhash_offset_t *)&checkbytes()[capacity]; }
    const objc_stringhash_offset_t *offsets() const { return (const objc_stringhash_offset_t *)&checkbytes()[capacity]; }

    uint32_t hash(const char *key, size_t keylen) const
    {
        uint64_t val = lookup8((uint8_t*)key, keylen, salt);
        uint32_t index = (uint32_t)(val>>shift) ^ scramble[tab[val&mask]];
        return index;
    }

    uint32_t hash(const char *key) const 
    {
        return hash(key, strlen(key));
    }

    // The check bytes areused to reject strings that aren't in the table
    // without paging in the table's cstring data. This checkbyte calculation 
    // catches 4785/4815 rejects when launching Safari; a perfect checkbyte 
    // would catch 4796/4815.
    objc_stringhash_check_t checkbyte(const char *key, size_t keylen) const
    {
        return 
            ((key[0] & 0x7) << 5)
            |
            ((uint8_t)keylen & 0x1f);
    }

    objc_stringhash_check_t checkbyte(const char *key) const
    {
        return checkbyte(key, strlen(key));
    }


#define INDEX_NOT_FOUND (~(uint32_t)0)

    uint32_t getIndex(const char *key) const 
    {
        size_t keylen = strlen(key);
        uint32_t h = hash(key, keylen);

        // Use check byte to reject without paging in the table's cstrings
        objc_stringhash_check_t h_check = checkbytes()[h];
        objc_stringhash_check_t key_check = checkbyte(key, keylen);
        bool check_fail = (h_check != key_check);
#if ! SELOPT_DEBUG
        if (check_fail) return INDEX_NOT_FOUND;
#endif

        objc_stringhash_offset_t offset = offsets()[h];
        if (offset == 0) return INDEX_NOT_FOUND;
        const char *result = (const char *)this + offset;
        if (0 != strcmp(key, result)) return INDEX_NOT_FOUND;

#if SELOPT_DEBUG
        if (check_fail) abort();
#endif

        return h;
    }

#ifdef SELOPT_WRITE

    size_t size() 
    {
        return sizeof(objc_stringhash_t) 
            + mask+1 
            + capacity * sizeof(objc_stringhash_check_t) 
            + capacity * sizeof(objc_stringhash_offset_t);
    }

    void byteswap(bool little_endian) 
    {
        // tab and checkbytes are arrays of bytes, no swap needed
        for (uint32_t i = 0; i < 256; i++) {
            S32(scramble[i]);
        }
        objc_stringhash_offset_t *o = offsets();
        for (uint32_t i = 0; i < capacity; i++) {
            S32(o[i]);
        }
        
        S32(capacity);
        S32(occupied);
        S32(shift);
        S32(mask);
        S64(salt);
    }

    const char *write(uint64_t base, size_t remaining, string_map& strings)
    {        
        if (sizeof(objc_stringhash_t) > remaining) {
            return "selector section too small (metadata not optimized)";
        }

        if (strings.size() == 0) {
            bzero(this, sizeof(objc_stringhash_t));
            return NULL;
        }
        
        perfect_hash phash;
        make_perfect(strings, phash);
        if (phash.capacity == 0) {
            return "perfect hash failed (metadata not optimized)";
        }

        // Set header
        capacity = phash.capacity;
        occupied = phash.occupied;
        shift = phash.shift;
        mask = phash.mask;
        unused1 = 0;
        unused2 = 0;
        salt = phash.salt;

        if (size() > remaining) {
            return "selector section too small (metadata not optimized)";
        }
        
        // Set hash data
        for (uint32_t i = 0; i < 256; i++) {
            scramble[i] = phash.scramble[i];
        }
        for (uint32_t i = 0; i < phash.mask+1; i++) {
            tab[i] = phash.tab[i];
        }
        
        // Set offsets to 0
        for (uint32_t i = 0; i < phash.capacity; i++) {
            offsets()[i] = 0;
        }
        // Set checkbytes to 0
        for (uint32_t i = 0; i < phash.capacity; i++) {
            checkbytes()[i] = 0;
        }
        
        // Set real string offsets and checkbytes
#       define SHIFT (64 - 8*sizeof(objc_stringhash_offset_t))
        string_map::const_iterator s;
        for (s = strings.begin(); s != strings.end(); ++s) {
            int64_t offset = s->second - base;
            if ((offset<<SHIFT)>>SHIFT != offset) {
                return "selector offset too big (metadata not optimized)";
            }

            uint32_t h = hash(s->first);
            offsets()[h] = (objc_stringhash_offset_t)offset;
            checkbytes()[h] = checkbyte(s->first);
        }
#       undef SHIFT
        
        return NULL;
    }

// SELOPT_WRITE
#endif
};

// Precomputed selector table.
// Edit objc-sel-table.s if you change this structure.
struct objc_selopt_t : objc_stringhash_t {

    const char* getEntryForIndex(uint32_t index) const {
        return (const char *)this + offsets()[index];
    }

    uint32_t getIndexForKey(const char *key) const {
        return getIndex(key);
    }

    uint32_t getSentinelIndex() const {
        return INDEX_NOT_FOUND;
    }

    const char* get(const char *key) const
    {
        uint32_t h = getIndex(key);
        if (h == INDEX_NOT_FOUND) return NULL;
        return getEntryForIndex(h);
    }

    size_t usedCount() const {
        return capacity;
    }
};

// Precomputed class list.
// Edit objc-sel-table.s if you change these structures.

struct objc_classheader_t {
    objc_stringhash_offset_t clsOffset;
    objc_stringhash_offset_t hiOffset;

    // For duplicate class names:
    // clsOffset = count<<1 | 1
    // duplicated classes are duplicateOffsets[hiOffset..hiOffset+count-1]
    bool isDuplicate() const { return clsOffset & 1; }
    uint32_t duplicateCount() const { return clsOffset >> 1; }
    uint32_t duplicateIndex() const { return hiOffset; }
};


struct objc_clsopt_t : objc_stringhash_t {
    // ...objc_stringhash_t fields...
    // objc_classheader_t classOffsets[capacity]; /* offsets from &capacity to class_t and header_info */
    // uint32_t duplicateCount;
    // objc_classheader_t duplicateOffsets[duplicatedClasses];

    objc_classheader_t *classOffsets() { return (objc_classheader_t *)&offsets()[capacity]; }
    const objc_classheader_t *classOffsets() const { return (const objc_classheader_t *)&offsets()[capacity]; }
    
    uint32_t& duplicateCount() { return *(uint32_t *)&classOffsets()[capacity]; }
    const uint32_t& duplicateCount() const { return *(const uint32_t *)&classOffsets()[capacity]; }

    objc_classheader_t *duplicateOffsets() { return (objc_classheader_t *)(&duplicateCount()+1); }
    const objc_classheader_t *duplicateOffsets() const { return (const objc_classheader_t *)(&duplicateCount()+1); }

    const char* getClassNameForIndex(uint32_t index) const {
        return (const char *)this + offsets()[index];
    }

    void* getClassForIndex(uint32_t index, uint32_t duplicateIndex) const {
        const objc_classheader_t& clshi = classOffsets()[index];
        if (! clshi.isDuplicate()) {
            // class appears in exactly one header
            return (void *)((const char *)this + clshi.clsOffset);
        }
        else {
            // class appears in more than one header - use getClassesAndHeaders
            const objc_classheader_t *list = &duplicateOffsets()[clshi.duplicateIndex()];
            return (void *)((const char *)this + list[duplicateIndex].clsOffset);
        }
    }

    // 0/NULL/NULL: not found
    // 1/ptr/ptr: found exactly one
    // n/NULL/NULL:  found N - use getClassesAndHeaders() instead
    uint32_t getClassHeaderAndIndex(const char *key, void*& cls, void*& hi, uint32_t& index) const
    {
        uint32_t h = getIndex(key);
        if (h == INDEX_NOT_FOUND) { 
            cls = NULL;
            hi = NULL;
            index = 0;
            return 0;
        }

        index = h;

        const objc_classheader_t& clshi = classOffsets()[h];
        if (! clshi.isDuplicate()) {
            // class appears in exactly one header
            cls = (void *)((const char *)this + clshi.clsOffset);
            hi  = (void *)((const char *)this + clshi.hiOffset);
            return 1;
        } 
        else {
            // class appears in more than one header - use getClassesAndHeaders
            cls = NULL;
            hi = NULL;
            return clshi.duplicateCount();
        }
    }

    void getClassesAndHeaders(const char *key, void **cls, void **hi) const 
    {
        uint32_t h = getIndex(key);
        if (h == INDEX_NOT_FOUND) return;

        const objc_classheader_t& clshi = classOffsets()[h];
        if (! clshi.isDuplicate()) {
            // class appears in exactly one header
            cls[0] = (void *)((const char *)this + clshi.clsOffset);
            hi[0]  = (void *)((const char *)this + clshi.hiOffset);
        } 
        else {
            // class appears in more than one header
            uint32_t count = clshi.duplicateCount();
            const objc_classheader_t *list = 
                &duplicateOffsets()[clshi.duplicateIndex()];
            for (uint32_t i = 0; i < count; i++) {
                cls[i] = (void *)((const char *)this + list[i].clsOffset);
                hi[i]  = (void *)((const char *)this + list[i].hiOffset);
            }
        }
    }

    // 0/NULL/NULL: not found
    // 1/ptr/ptr: found exactly one
    // n/NULL/NULL:  found N - use getClassesAndHeaders() instead
    uint32_t getClassAndHeader(const char *key, void*& cls, void*& hi) const
    {
        uint32_t unusedIndex = 0;
        return getClassHeaderAndIndex(key, cls, hi, unusedIndex);
    }

#ifdef SELOPT_WRITE

    size_t size() 
    {
        return
            objc_stringhash_t::size()
            + capacity * sizeof(objc_classheader_t)
            + sizeof(duplicateCount())
            + duplicateCount() * sizeof(objc_classheader_t);
    }

    size_t sizeWithoutDups()
    {
        return
            objc_stringhash_t::size()
            + capacity * sizeof(objc_classheader_t);
    }

   void byteswap(bool little_endian)
    {
        objc_classheader_t *o;
        
        o = classOffsets();
        for (uint32_t i = 0; i < capacity; i++) {
            S32(o[i].clsOffset);
            S32(o[i].hiOffset);
        }

        o = duplicateOffsets();
        for (uint32_t i = 0; i < duplicateCount(); i++) {
            S32(o[i].clsOffset);
            S32(o[i].hiOffset);
        }

        S32(duplicateCount());

        objc_stringhash_t::byteswap(little_endian);
    }
    
    const char *write(uint64_t base, size_t remaining, 
                      string_map& strings, class_map& classes, bool verbose)
    {
        const char *err;
        err = objc_stringhash_t::write(base, remaining, strings);
        if (err) return err;
   
        if (sizeWithoutDups() > remaining) {
            return "selector section too small (metadata not optimized)";
        }
        if (size() > remaining) {
            return "selector section too small (metadata not optimized)";
        }

        // Set class offsets to 0
        for (uint32_t i = 0; i < capacity; i++) {
            classOffsets()[i].clsOffset = 0;
            classOffsets()[i].hiOffset = 0;
        }
        
        // Set real class offsets
#       define SHIFT (64 - 8*sizeof(objc_stringhash_offset_t))
        class_map::const_iterator c;
        for (c = classes.begin(); c != classes.end(); ++c) {
            uint32_t h = getIndex(c->first);
            if (h == INDEX_NOT_FOUND) {
                return "class list busted (metadata not optimized)";
            }

            if (classOffsets()[h].clsOffset != 0) {
                // already did this class
                continue;
            }

            uint32_t count = (uint32_t)classes.count(c->first);
            if (count == 1) {
                // only one class with this name

                int64_t coff = c->second.first - base;
                int64_t hoff = c->second.second - base;
                if ((coff<<SHIFT)>>SHIFT != coff) {
                    return "class offset too big (metadata not optimized)";
                }
                if ((hoff<<SHIFT)>>SHIFT != hoff) {
                    return "header offset too big (metadata not optimized)";
                }

                classOffsets()[h].clsOffset = (objc_stringhash_offset_t)coff;
                classOffsets()[h].hiOffset  = (objc_stringhash_offset_t)hoff;
            }
            else {
                // class name has duplicates - write them all now
                if (verbose) {
                    fprintf(stderr, "update_dyld_shared_cache: %u duplicates of Objective-C class %s\n", count, c->first);
                }

                uint32_t dest = duplicateCount();
                duplicateCount() += count;                
                if (size() > remaining) {
                    return "selector section too small (metadata not optimized)";
                }

                // classOffsets() instead contains count and array index
                classOffsets()[h].clsOffset = count*2 + 1;
                classOffsets()[h].hiOffset = dest;

                std::pair<class_map::const_iterator, class_map::const_iterator>
                    duplicates = classes.equal_range(c->first);
                class_map::const_iterator dup;
                for (dup = duplicates.first; dup != duplicates.second; ++dup) {
                    int64_t coff = dup->second.first - base;
                    int64_t hoff = dup->second.second - base;
                    if ((coff<<SHIFT)>>SHIFT != coff) {
                        return "class offset too big (metadata not optimized)";
                    }
                    if ((hoff<<SHIFT)>>SHIFT != hoff) {
                        return "header offset too big (metadata not optimized)";
                    }
                    
                    duplicateOffsets()[dest].clsOffset = (objc_stringhash_offset_t)coff;
                    duplicateOffsets()[dest].hiOffset  = (objc_stringhash_offset_t)hoff;
                    dest++;
                }
            } 
        }
#       undef SHIFT
        
        return NULL;
    }

// SELOPT_WRITE
#endif
};


struct objc_protocolopt_t : objc_stringhash_t {
    // ...objc_stringhash_t fields...
    // uint32_t protocolOffsets[capacity]; /* offsets from &capacity to protocol_t */

    objc_stringhash_offset_t *protocolOffsets() { return (objc_stringhash_offset_t *)&offsets()[capacity]; }
    const objc_stringhash_offset_t *protocolOffsets() const { return (const objc_stringhash_offset_t *)&offsets()[capacity]; }

    void* getProtocol(const char *key) const
    {
        uint32_t h = getIndex(key);
        if (h == INDEX_NOT_FOUND) {
            return NULL;
        }

        return (void *)((const char *)this + protocolOffsets()[h]);
    }

#ifdef SELOPT_WRITE

    size_t size()
    {
        return
            objc_stringhash_t::size() + capacity * sizeof(objc_stringhash_offset_t);
    }

    void byteswap(bool little_endian)
    {
        objc_stringhash_offset_t *o;

        o = protocolOffsets();
        for (objc_stringhash_offset_t i = 0; i < (int)capacity; i++) {
            S32(o[i]);
        }

        objc_stringhash_t::byteswap(little_endian);
    }

    const char *write(uint64_t base, size_t remaining,
                      string_map& strings, legacy_protocol_map& protocols,
                      bool verbose)
    {
        const char *err;
        err = objc_stringhash_t::write(base, remaining, strings);
        if (err) return err;

        if (size() > remaining) {
            return "selector section too small (metadata not optimized)";
        }

        // Set protocol offsets to 0
        for (uint32_t i = 0; i < capacity; i++) {
            protocolOffsets()[i] = 0;
        }

        // Set real protocol offsets
#       define SHIFT (64 - 8*sizeof(objc_stringhash_offset_t))
        legacy_protocol_map::const_iterator c;
        for (c = protocols.begin(); c != protocols.end(); ++c) {
            uint32_t h = getIndex(c->first);
            if (h == INDEX_NOT_FOUND) {
                return "protocol list busted (metadata not optimized)";
            }

            int64_t offset = c->second - base;
            if ((offset<<SHIFT)>>SHIFT != offset) {
                return "protocol offset too big (metadata not optimized)";
            }

            protocolOffsets()[h] = (objc_stringhash_offset_t)offset;
        }
#       undef SHIFT

        return NULL;
    }

// SELOPT_WRITE
#endif
};

struct objc_protocolopt2_t : objc_clsopt_t {

    void* getProtocol(const char *key,
                      bool (*callback)(const void* header_info)) const
    {
        uint32_t h = getIndex(key);
        if (h == INDEX_NOT_FOUND) {
            return NULL;
        }

        const objc_classheader_t& clshi = classOffsets()[h];
        if (! clshi.isDuplicate()) {
            // protocol appears in exactly one header
            void* cls = (void *)((const char *)this + clshi.clsOffset);
            void* hi  = (void *)((const char *)this + clshi.hiOffset);
            return callback(hi) ? cls : NULL;
        }
        else {
            // protocol appears in more than one header
            uint32_t count = clshi.duplicateCount();
            const objc_classheader_t *list = &duplicateOffsets()[clshi.duplicateIndex()];
            for (uint32_t i = 0; i < count; i++) {
                void* cls = (void *)((const char *)this + list[i].clsOffset);
                void* hi  = (void *)((const char *)this + list[i].hiOffset);
                if (callback(hi))
                    return cls;
            }
            return NULL;
        }
    }

};


// Precomputed image list.
struct objc_headeropt_ro_t;

// Precomputed image list.
struct objc_headeropt_rw_t;

// Precomputed class list.
struct objc_clsopt_t;

// Edit objc-sel-table.s if you change this value.
// lldb and Symbolication read these structures. Inform them of any changes.
enum { VERSION = 15 };

// Values for objc_opt_t::flags
enum : uint32_t {
    IsProduction = (1 << 0),               // never set in development cache
    NoMissingWeakSuperclasses = (1 << 1)   // set in development cache and customer
};

// Top-level optimization structure.
// Edit objc-sel-table.s if you change this structure.
struct alignas(alignof(void*)) objc_opt_t {
    uint32_t version;
    uint32_t flags;
    int32_t selopt_offset;
    int32_t headeropt_ro_offset;
    int32_t clsopt_offset;
    int32_t unused_protocolopt_offset; // This is now 0 as we've moved to the new protocolopt_offset
    int32_t headeropt_rw_offset;
    int32_t protocolopt_offset;

    const objc_selopt_t* selopt() const {
        if (selopt_offset == 0) return NULL;
        return (objc_selopt_t *)((uint8_t *)this + selopt_offset);
    }
    objc_selopt_t* selopt() { 
        if (selopt_offset == 0) return NULL;
        return (objc_selopt_t *)((uint8_t *)this + selopt_offset);
    }

    struct objc_headeropt_ro_t* headeropt_ro() const {
        if (headeropt_ro_offset == 0) return NULL;
        return (struct objc_headeropt_ro_t *)((uint8_t *)this + headeropt_ro_offset);
    }

    struct objc_clsopt_t* clsopt() const { 
        if (clsopt_offset == 0) return NULL;
        return (objc_clsopt_t *)((uint8_t *)this + clsopt_offset);
    }

    struct objc_protocolopt_t* protocolopt() const {
        if (unused_protocolopt_offset == 0) return NULL;
        return (objc_protocolopt_t *)((uint8_t *)this + unused_protocolopt_offset);
    }

    struct objc_protocolopt2_t* protocolopt2() const {
        if (protocolopt_offset == 0) return NULL;
        return (objc_protocolopt2_t *)((uint8_t *)this + protocolopt_offset);
    }

    struct objc_headeropt_rw_t* headeropt_rw() const {
        if (headeropt_rw_offset == 0) return NULL;
        return (struct objc_headeropt_rw_t *)((uint8_t *)this + headeropt_rw_offset);
    }
};

// sizeof(objc_opt_t) must be pointer-aligned
STATIC_ASSERT(sizeof(objc_opt_t) % sizeof(void*) == 0);


// List of offsets in libobjc that the shared cache optimization needs to use.
template <typename T>
struct objc_opt_pointerlist_tt {
    T protocolClass;
};
typedef struct objc_opt_pointerlist_tt<uintptr_t> objc_opt_pointerlist_t;


/*
--------------------------------------------------------------------
mix -- mix 3 64-bit values reversibly.
mix() takes 48 machine instructions, but only 24 cycles on a superscalar
  machine (like Intel's new MMX architecture).  It requires 4 64-bit
  registers for 4::2 parallelism.
All 1-bit deltas, all 2-bit deltas, all deltas composed of top bits of
  (a,b,c), and all deltas of bottom bits were tested.  All deltas were
  tested both on random keys and on keys that were nearly all zero.
  These deltas all cause every bit of c to change between 1/3 and 2/3
  of the time (well, only 113/400 to 287/400 of the time for some
  2-bit delta).  These deltas all cause at least 80 bits to change
  among (a,b,c) when the mix is run either forward or backward (yes it
  is reversible).
This implies that a hash using mix64 has no funnels.  There may be
  characteristics with 3-bit deltas or bigger, I didn't test for
  those.
--------------------------------------------------------------------
*/
#define mix64(a,b,c) \
{ \
  a -= b; a -= c; a ^= (c>>43); \
  b -= c; b -= a; b ^= (a<<9); \
  c -= a; c -= b; c ^= (b>>8); \
  a -= b; a -= c; a ^= (c>>38); \
  b -= c; b -= a; b ^= (a<<23); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>35); \
  b -= c; b -= a; b ^= (a<<49); \
  c -= a; c -= b; c ^= (b>>11); \
  a -= b; a -= c; a ^= (c>>12); \
  b -= c; b -= a; b ^= (a<<18); \
  c -= a; c -= b; c ^= (b>>22); \
}

/*
--------------------------------------------------------------------
hash() -- hash a variable-length key into a 64-bit value
  k     : the key (the unaligned variable-length array of bytes)
  len   : the length of the key, counting by bytes
  level : can be any 8-byte value
Returns a 64-bit value.  Every bit of the key affects every bit of
the return value.  No funnels.  Every 1-bit and 2-bit delta achieves
avalanche.  About 41+5len instructions.

The best hash table sizes are powers of 2.  There is no need to do
mod a prime (mod is sooo slow!).  If you need less than 64 bits,
use a bitmask.  For example, if you need only 10 bits, do
  h = (h & hashmask(10));
In which case, the hash table should have hashsize(10) elements.

If you are hashing n strings (uint8_t **)k, do it like this:
  for (i=0, h=0; i<n; ++i) h = hash( k[i], len[i], h);

By Bob Jenkins, Jan 4 1997.  bob_jenkins@burtleburtle.net.  You may
use this code any way you wish, private, educational, or commercial,
but I would appreciate if you give me credit.

See http://burtleburtle.net/bob/hash/evahash.html
Use for hash table lookup, or anything where one collision in 2^^64
is acceptable.  Do NOT use for cryptographic purposes.
--------------------------------------------------------------------
*/

static uint64_t lookup8( uint8_t *k, size_t length, uint64_t level)
// uint8_t *k;        /* the key */
// uint64_t  length;   /* the length of the key */
// uint64_t  level;    /* the previous hash, or an arbitrary value */
{
  uint64_t a,b,c;
  size_t len;

  /* Set up the internal state */
  len = length;
  a = b = level;                         /* the previous hash value */
  c = 0x9e3779b97f4a7c13LL; /* the golden ratio; an arbitrary value */

  /*---------------------------------------- handle most of the key */
  while (len >= 24)
  {
    a += (k[0]        +((uint64_t)k[ 1]<< 8)+((uint64_t)k[ 2]<<16)+((uint64_t)k[ 3]<<24)
     +((uint64_t)k[4 ]<<32)+((uint64_t)k[ 5]<<40)+((uint64_t)k[ 6]<<48)+((uint64_t)k[ 7]<<56));
    b += (k[8]        +((uint64_t)k[ 9]<< 8)+((uint64_t)k[10]<<16)+((uint64_t)k[11]<<24)
     +((uint64_t)k[12]<<32)+((uint64_t)k[13]<<40)+((uint64_t)k[14]<<48)+((uint64_t)k[15]<<56));
    c += (k[16]       +((uint64_t)k[17]<< 8)+((uint64_t)k[18]<<16)+((uint64_t)k[19]<<24)
     +((uint64_t)k[20]<<32)+((uint64_t)k[21]<<40)+((uint64_t)k[22]<<48)+((uint64_t)k[23]<<56));
    mix64(a,b,c);
    k += 24; len -= 24;
  }

  /*------------------------------------- handle the last 23 bytes */
  c += length;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-fallthrough"
  switch(len)              /* all the case statements fall through */
  {
  case 23: c+=((uint64_t)k[22]<<56);
  case 22: c+=((uint64_t)k[21]<<48);
  case 21: c+=((uint64_t)k[20]<<40);
  case 20: c+=((uint64_t)k[19]<<32);
  case 19: c+=((uint64_t)k[18]<<24);
  case 18: c+=((uint64_t)k[17]<<16);
  case 17: c+=((uint64_t)k[16]<<8);
    /* the first byte of c is reserved for the length */
  case 16: b+=((uint64_t)k[15]<<56);
  case 15: b+=((uint64_t)k[14]<<48);
  case 14: b+=((uint64_t)k[13]<<40);
  case 13: b+=((uint64_t)k[12]<<32);
  case 12: b+=((uint64_t)k[11]<<24);
  case 11: b+=((uint64_t)k[10]<<16);
  case 10: b+=((uint64_t)k[ 9]<<8);
  case  9: b+=((uint64_t)k[ 8]);
  case  8: a+=((uint64_t)k[ 7]<<56);
  case  7: a+=((uint64_t)k[ 6]<<48);
  case  6: a+=((uint64_t)k[ 5]<<40);
  case  5: a+=((uint64_t)k[ 4]<<32);
  case  4: a+=((uint64_t)k[ 3]<<24);
  case  3: a+=((uint64_t)k[ 2]<<16);
  case  2: a+=((uint64_t)k[ 1]<<8);
  case  1: a+=((uint64_t)k[ 0]);
    /* case 0: nothing left to add */
  }
#pragma clang diagnostic pop
  mix64(a,b,c);
  /*-------------------------------------------- report the result */
  return c;
}


#if defined(SELOPT_WRITE) || defined(CLOSURE_SELOPT_WRITE)

/*
------------------------------------------------------------------------------
This generates a minimal perfect hash function.  That means, given a
set of n keys, this determines a hash function that maps each of
those keys into a value in 0..n-1 with no collisions.

The perfect hash function first uses a normal hash function on the key
to determine (a,b) such that the pair (a,b) is distinct for all
keys, then it computes a^scramble[tab[b]] to get the final perfect hash.
tab[] is an array of 1-byte values and scramble[] is a 256-term array of 
2-byte or 4-byte values.  If there are n keys, the length of tab[] is a 
power of two between n/3 and n.

I found the idea of computing distinct (a,b) values in "Practical minimal 
perfect hash functions for large databases", Fox, Heath, Chen, and Daoud, 
Communications of the ACM, January 1992.  They found the idea in Chichelli 
(CACM Jan 1980).  Beyond that, our methods differ.

The key is hashed to a pair (a,b) where a in 0..*alen*-1 and b in
0..*blen*-1.  A fast hash function determines both a and b
simultaneously.  Any decent hash function is likely to produce
hashes so that (a,b) is distinct for all pairs.  I try the hash
using different values of *salt* until all pairs are distinct.

The final hash is (a XOR scramble[tab[b]]).  *scramble* is a
predetermined mapping of 0..255 into 0..smax-1.  *tab* is an
array that we fill in in such a way as to make the hash perfect.

First we fill in all values of *tab* that are used by more than one
key.  We try all possible values for each position until one works.

This leaves m unmapped keys and m values that something could hash to.
If you treat unmapped keys as lefthand nodes and unused hash values
as righthand nodes, and draw a line connecting each key to each hash
value it could map to, you get a bipartite graph.  We attempt to
find a perfect matching in this graph.  If we succeed, we have
determined a perfect hash for the whole set of keys.

*scramble* is used because (a^tab[i]) clusters keys around *a*.
------------------------------------------------------------------------------
*/

typedef uint64_t  ub8;
#define UB8MAXVAL 0xffffffffffffffffLL
#define UB8BITS 64
typedef uint32_t  ub4;
#define UB4MAXVAL 0xffffffff
#define UB4BITS 32
typedef uint16_t  ub2;
#define UB2MAXVAL 0xffff
#define UB2BITS 16
typedef uint8_t ub1;
#define UB1MAXVAL 0xff
#define UB1BITS 8

#define TRUE  1
#define FALSE 0

#define SCRAMBLE_LEN 256 // ((ub4)1<<16)                    /* length of *scramble* */
#define RETRY_INITKEY 2048  /* number of times to try to find distinct (a,b) */
#define RETRY_PERFECT 4     /* number of times to try to make a perfect hash */


/* representation of a key */
struct key
{
  ub1        *name_k;                                      /* the actual key */
  ub4         len_k;                         /* the length of the actual key */
  ub4         hash_k;                 /* the initial hash value for this key */
/* beyond this point is mapping-dependent */
  ub4         a_k;                            /* a, of the key maps to (a,b) */
  ub4         b_k;                            /* b, of the key maps to (a,b) */
  struct key *nextb_k;                               /* next key with this b */
};
typedef  struct key  key;

/* things indexed by b of original (a,b) pair */
struct bstuff
{
  ub2  val_b;                                        /* hash=a^tabb[b].val_b */
  key *list_b;                   /* tabb[i].list_b is list of keys with b==i */
  ub4  listlen_b;                                        /* length of list_b */
  ub4  water_b;           /* high watermark of who has visited this map node */
};
typedef  struct bstuff  bstuff;

/* things indexed by final hash value */
struct hstuff
{
  key *key_h;                   /* tabh[i].key_h is the key with a hash of i */
};
typedef  struct hstuff hstuff;

/* things indexed by queue position */
struct qstuff
{
  bstuff *b_q;                        /* b that currently occupies this hash */
  ub4     parent_q;     /* queue position of parent that could use this hash */
  ub2     newval_q;      /* what to change parent tab[b] to to use this hash */
  ub2     oldval_q;                              /* original value of tab[b] */
};
typedef  struct qstuff  qstuff;


/*
------------------------------------------------------------------------------
Find the mapping that will produce a perfect hash
------------------------------------------------------------------------------
*/

/* return the ceiling of the log (base 2) of val */
static ub4  log2u(ub4 val)
{
  ub4 i;
  for (i=0; ((ub4)1<<i) < val; ++i)
    ;
  return i;
}

/* compute p(x), where p is a permutation of 0..(1<<nbits)-1 */
/* permute(0)=0.  This is intended and useful. */
static ub4  permute(ub4 x, ub4 nbits)
// ub4 x;                                       /* input, a value in some range */
// ub4 nbits;                                 /* input, number of bits in range */
{
  int i;
  int mask   = ((ub4)1<<nbits)-1;                                /* all ones */
  int const2 = 1+nbits/2;
  int const3 = 1+nbits/3;
  int const4 = 1+nbits/4;
  int const5 = 1+nbits/5;
  for (i=0; i<20; ++i)
  {
    x = (x+(x<<const2)) & mask; 
    x = (x^(x>>const3));
    x = (x+(x<<const4)) & mask;
    x = (x^(x>>const5));
  }
  return x;
}

/* initialize scramble[] with distinct random values in 0..smax-1 */
static void scrambleinit(ub4 *scramble, ub4 smax)
// ub4      *scramble;                            /* hash is a^scramble[tab[b]] */
// ub4       smax;                    /* scramble values should be in 0..smax-1 */
{
  ub4 i;

  /* fill scramble[] with distinct random integers in 0..smax-1 */
  for (i=0; i<SCRAMBLE_LEN; ++i)
  {
    scramble[i] = permute(i, log2u(smax));
  }
}


/* 
 * put keys in tabb according to key->b_k
 * check if the initial hash might work 
 */
static int inittab(dyld3::OverflowSafeArray<bstuff>& tabb, dyld3::OverflowSafeArray<key>& keys, int complete)
// bstuff   *tabb;                     /* output, list of keys with b for (a,b) */
// ub4       blen;                                            /* length of tabb */
// key      *keys;                               /* list of keys already hashed */
// int       complete;        /* TRUE means to complete init despite collisions */
{
  int  nocollision = TRUE;
  ub4 i;

  memset((void *)tabb.begin(), 0, (size_t)(sizeof(bstuff)*tabb.maxCount()));

  /* Two keys with the same (a,b) guarantees a collision */
  for (i = 0; i < keys.count(); i++) {
    key *mykey = &keys[i];
    key *otherkey;

    for (otherkey=tabb[mykey->b_k].list_b; 
	 otherkey; 
	 otherkey=otherkey->nextb_k)
    {
      if (mykey->a_k == otherkey->a_k)
      {
        nocollision = FALSE;
	if (!complete)
	  return FALSE;
      }
    }
    ++tabb[mykey->b_k].listlen_b;
    mykey->nextb_k = tabb[mykey->b_k].list_b;
    tabb[mykey->b_k].list_b = mykey;
  }

  /* no two keys have the same (a,b) pair */
  return nocollision;
}


/* Do the initial hash for normal mode (use lookup and checksum) */
static void initnorm(dyld3::OverflowSafeArray<key>& keys, ub4 alen, ub4 blen, ub4 smax, ub8 salt)
// key      *keys;                                          /* list of all keys */
// ub4       alen;                    /* (a,b) has a in 0..alen-1, a power of 2 */
// ub4       blen;                    /* (a,b) has b in 0..blen-1, a power of 2 */
// ub4       smax;                   /* maximum range of computable hash values */
// ub4       salt;                     /* used to initialize the hash function */
// gencode  *final;                          /* output, code for the final hash */
{
  ub4 loga = log2u(alen);                            /* log based 2 of blen */
#if BUILDING_CACHE_BUILDER
  dispatch_apply(keys.count(), DISPATCH_APPLY_AUTO, ^(size_t index) {
    ub4 i = (ub4)index;
    key *mykey = &keys[i];
    ub8 hash = lookup8(mykey->name_k, mykey->len_k, salt);
    mykey->a_k = (loga > 0) ? (ub4)(hash >> (UB8BITS-loga)) : 0;
    mykey->b_k = (blen > 1) ? (hash & (blen-1)) : 0;
  });
#else
  for (size_t index = 0; index != keys.count(); ++index) {
    ub4 i = (ub4)index;
    key *mykey = &keys[i];
    ub8 hash = lookup8(mykey->name_k, mykey->len_k, salt);
    mykey->a_k = (loga > 0) ? (ub4)(hash >> (UB8BITS-loga)) : 0;
    mykey->b_k = (blen > 1) ? (hash & (blen-1)) : 0;
  };
#endif
}


/* Try to apply an augmenting list */
static int apply(dyld3::OverflowSafeArray<bstuff>& tabb,
                 dyld3::OverflowSafeArray<hstuff>& tabh,
                 dyld3::OverflowSafeArray<qstuff>& tabq,
                 ub4 *scramble, ub4 tail, int rollback)
// bstuff *tabb;
// hstuff *tabh;
// qstuff *tabq;
// ub4     blen;
// ub4    *scramble;
// ub4     tail;
// int     rollback;          /* FALSE applies augmenting path, TRUE rolls back */
{
    ub4     hash;
    key    *mykey;
    bstuff *pb;
    ub4     child;
    ub4     parent;
    ub4     stabb;                                         /* scramble[tab[b]] */

    /* walk from child to parent */
    for (child=tail-1; child; child=parent)
    {
        parent = tabq[child].parent_q;                    /* find child's parent */
        pb     = tabq[parent].b_q;             /* find parent's list of siblings */

        /* erase old hash values */
        stabb = scramble[pb->val_b];
        for (mykey=pb->list_b; mykey; mykey=mykey->nextb_k)
        {
            hash = mykey->a_k^stabb;
            if (mykey == tabh[hash].key_h)
            {                            /* erase hash for all of child's siblings */
                tabh[hash].key_h = (key *)0;
            }
        }

        /* change pb->val_b, which will change the hashes of all parent siblings */
        pb->val_b = (rollback ? tabq[child].oldval_q : tabq[child].newval_q);

        /* set new hash values */
        stabb = scramble[pb->val_b];
        for (mykey=pb->list_b; mykey; mykey=mykey->nextb_k)
        {
            hash = mykey->a_k^stabb;
            if (rollback)
            {
                if (parent == 0) continue;                  /* root never had a hash */
            }
            else if (tabh[hash].key_h)
            {
                /* very rare: roll back any changes */
                apply(tabb, tabh, tabq, scramble, tail, TRUE);
                return FALSE;                                  /* failure, collision */
            }
            tabh[hash].key_h = mykey;
        }
    }
    return TRUE;
}


/*
-------------------------------------------------------------------------------
augment(): Add item to the mapping.

Construct a spanning tree of *b*s with *item* as root, where each
parent can have all its hashes changed (by some new val_b) with 
at most one collision, and each child is the b of that collision.

I got this from Tarjan's "Data Structures and Network Algorithms".  The
path from *item* to a *b* that can be remapped with no collision is 
an "augmenting path".  Change values of tab[b] along the path so that 
the unmapped key gets mapped and the unused hash value gets used.

Assuming 1 key per b, if m out of n hash values are still unused, 
you should expect the transitive closure to cover n/m nodes before 
an unused node is found.  Sum(i=1..n)(n/i) is about nlogn, so expect
this approach to take about nlogn time to map all single-key b's.
-------------------------------------------------------------------------------
*/
static int augment(dyld3::OverflowSafeArray<bstuff>& tabb,
                   dyld3::OverflowSafeArray<hstuff>& tabh,
                   dyld3::OverflowSafeArray<qstuff>& tabq,
                   ub4 *scramble, ub4 smax, bstuff *item, ub4 nkeys,
                   ub4 highwater)
// bstuff   *tabb;                                        /* stuff indexed by b */
// hstuff   *tabh;  /* which key is associated with which hash, indexed by hash */
// qstuff   *tabq;            /* queue of *b* values, this is the spanning tree */
// ub4      *scramble;                      /* final hash is a^scramble[tab[b]] */
// ub4       smax;                                 /* highest value in scramble */
// bstuff   *item;                           /* &tabb[b] for the b to be mapped */
// ub4       nkeys;                         /* final hash must be in 0..nkeys-1 */
// ub4       highwater;        /* a value higher than any now in tabb[].water_b */
{
    ub4  q;                      /* current position walking through the queue */
    ub4  tail;              /* tail of the queue.  0 is the head of the queue. */
    ub4  limit=UB1MAXVAL+1;
    ub4  highhash = smax;

    /* initialize the root of the spanning tree */
    tabq[0].b_q = item;
    tail = 1;

    /* construct the spanning tree by walking the queue, add children to tail */
    for (q=0; q<tail; ++q)
    {
        bstuff *myb = tabq[q].b_q;                        /* the b for this node */
        ub4     i;                              /* possible value for myb->val_b */

        if (q == 1)
            break;                                  /* don't do transitive closure */

        for (i=0; i<limit; ++i)
        {
            bstuff *childb = (bstuff *)0;             /* the b that this i maps to */
            key    *mykey;                       /* for walking through myb's keys */

            for (mykey = myb->list_b; mykey; mykey=mykey->nextb_k)
            {
                key    *childkey;
                ub4 hash = mykey->a_k^scramble[i];

                if (hash >= highhash) break;                        /* out of bounds */
                childkey = tabh[hash].key_h;

                if (childkey)
                {
                    bstuff *hitb = &tabb[childkey->b_k];

                    if (childb)
                    {
                        if (childb != hitb) break;            /* hit at most one child b */
                    }
                    else
                    {
                        childb = hitb;                        /* remember this as childb */
                        if (childb->water_b == highwater) break;     /* already explored */
                    }
                }
            }
            if (mykey) continue;             /* myb with i has multiple collisions */

            /* add childb to the queue of reachable things */
            if (childb) childb->water_b = highwater;
            tabq[tail].b_q      = childb;
            tabq[tail].newval_q = i;     /* how to make parent (myb) use this hash */
            tabq[tail].oldval_q = myb->val_b;            /* need this for rollback */
            tabq[tail].parent_q = q;
            ++tail;

            if (!childb)
            {                                  /* found an *i* with no collisions? */
                /* try to apply the augmenting path */
                if (apply(tabb, tabh, tabq, scramble, tail, FALSE))
                    return TRUE;        /* success, item was added to the perfect hash */

                --tail;                    /* don't know how to handle such a child! */
            }
        }
    }
    return FALSE;
}


/* find a mapping that makes this a perfect hash */
static int perfect(dyld3::OverflowSafeArray<bstuff>& tabb,
                   dyld3::OverflowSafeArray<hstuff>& tabh,
                   dyld3::OverflowSafeArray<qstuff>& tabq,
                   ub4 smax, ub4 *scramble, ub4 nkeys)
{
    ub4 maxkeys;                           /* maximum number of keys for any b */
    ub4 i, j;

    const ub4 blen = (ub4)tabb.count();

#if SELOPT_DEBUG
    fprintf(stderr, "           blen %d smax %d nkeys %d\n", blen, smax, nkeys);
#endif

    /* clear any state from previous attempts */
    memset((void *)tabh.begin(), 0, sizeof(hstuff)*smax);
    memset((void *)tabq.begin(), 0, sizeof(qstuff)*(blen+1));

    for (maxkeys=0,i=0; i<blen; ++i)
        if (tabb[i].listlen_b > maxkeys)
            maxkeys = tabb[i].listlen_b;

    /* In descending order by number of keys, map all *b*s */
    for (j=maxkeys; j>0; --j)
        for (i=0; i<blen; ++i)
            if (tabb[i].listlen_b == j)
                if (!augment(tabb, tabh, tabq, scramble, smax, &tabb[i], nkeys,
                             i+1))
                {
                    return FALSE;
                }

    /* Success!  We found a perfect hash of all keys into 0..nkeys-1. */
    return TRUE;
}


/* guess initial values for alen and blen */
static void initalen(ub4 *alen, ub4 *blen, ub4 smax, ub4 nkeys)
// ub4      *alen;                                      /* output, initial alen */
// ub4      *blen;                                      /* output, initial blen */
// ub4      smax;    /* input, power of two greater or equal to max hash value */
// ub4       nkeys;                              /* number of keys being hashed */
{
  /*
   * Find initial *alen, *blen
   * Initial alen and blen values were found empirically.  Some factors:
   *
   * If smax<256 there is no scramble, so tab[b] needs to cover 0..smax-1.
   *
   * alen and blen must be powers of 2 because the values in 0..alen-1 and
   * 0..blen-1 are produced by applying a bitmask to the initial hash function.
   *
   * alen must be less than smax, in fact less than nkeys, because otherwise
   * there would often be no i such that a^scramble[i] is in 0..nkeys-1 for
   * all the *a*s associated with a given *b*, so there would be no legal
   * value to assign to tab[b].  This only matters when we're doing a minimal
   * perfect hash.
   *
   * It takes around 800 trials to find distinct (a,b) with nkey=smax*(5/8)
   * and alen*blen = smax*smax/32.
   *
   * Values of blen less than smax/4 never work, and smax/2 always works.
   *
   * We want blen as small as possible because it is the number of bytes in
   * the huge array we must create for the perfect hash.
   *
   * When nkey <= smax*(5/8), blen=smax/4 works much more often with 
   * alen=smax/8 than with alen=smax/4.  Above smax*(5/8), blen=smax/4
   * doesn't seem to care whether alen=smax/8 or alen=smax/4.  I think it
   * has something to do with 5/8 = 1/8 * 5.  For example examine 80000, 
   * 85000, and 90000 keys with different values of alen.  This only matters
   * if we're doing a minimal perfect hash.
   *
   * When alen*blen <= 1<<UB4BITS, the initial hash must produce one integer.
   * Bigger than that it must produce two integers, which increases the
   * cost of the hash per character hashed.
   */
  *alen = smax;                     /* no reason to restrict alen to smax/2 */
  *blen = ((nkeys <= smax*0.6) ? smax/16 : 
           (nkeys <= smax*0.8) ? smax/8 : smax/4);
  
  if (*alen < 1) *alen = 1;
  if (*blen < 1) *blen = 1;

#if SELOPT_DEBUG
  fprintf(stderr, "alen %d blen %d smax %d nkeys %d\n", *alen, *blen, smax, nkeys);
#endif
}

/* 
** Try to find a perfect hash function.  
** Return the successful initializer for the initial hash. 
** Return 0 if no perfect hash could be found.
*/
static int findhash(dyld3::OverflowSafeArray<bstuff>& tabb,
                    ub4 *alen, ub8 *salt,
                    ub4 *scramble, ub4 smax, dyld3::OverflowSafeArray<key>& keys)
// bstuff  **tabb;           /* output, tab[] of the perfect hash, length *blen */
// ub4      *alen;                 /* output, 0..alen-1 is range for a of (a,b) */
// ub4      *blen;                 /* output, 0..blen-1 is range for b of (a,b) */
// ub4      *salt;                         /* output, initializes initial hash */
// ub4      *scramble;                      /* input, hash = a^scramble[tab[b]] */
// ub4      smax;                           /* input, scramble[i] in 0..smax-1 */
// key      *keys;                                       /* input, keys to hash */
// ub4       nkeys;                       /* input, number of keys being hashed */
{
    ub4 bad_initkey;                       /* how many times did initkey fail? */
    ub4 bad_perfect;                       /* how many times did perfect fail? */
    ub4 si;                        /* trial initializer for initial hash */
    ub4 maxalen;
    dyld3::OverflowSafeArray<hstuff>tabh;   /* table of keys indexed by hash value */
    dyld3::OverflowSafeArray<qstuff>tabq;   /* table of stuff indexed by queue value, used by augment */

    /* guess initial values for alen and blen */
    ub4 blen = 0;
    initalen(alen, &blen, smax, (ub4)keys.count());

    scrambleinit(scramble, smax);

    maxalen = smax;

    /* allocate working memory */
    tabb.resize(blen);
    tabq.resize(blen+1);
    tabh.resize(smax);

    /* Actually find the perfect hash */
    *salt = 0;
    bad_initkey = 0;
    bad_perfect = 0;
    for (si=1; ; ++si)
    {
        ub4 rslinit;
        /* Try to find distinct (A,B) for all keys */
        *salt = si * 0x9e3779b97f4a7c13LL; /* golden ratio (arbitrary value) */
        initnorm(keys, *alen, blen, smax, *salt);
        rslinit = inittab(tabb, keys, FALSE);
        if (rslinit == 0)
        {
            /* didn't find distinct (a,b) */
            if (++bad_initkey >= RETRY_INITKEY)
            {
                /* Try to put more bits in (A,B) to make distinct (A,B) more likely */
                if (*alen < maxalen)
                {
                    *alen *= 2;
                }
                else if (blen < smax)
                {
                    blen *= 2;
                    tabb.resize(blen);
                    tabq.resize(blen+1);
                }
                bad_initkey = 0;
                bad_perfect = 0;
            }
            continue;                             /* two keys have same (a,b) pair */
        }

        /* Given distinct (A,B) for all keys, build a perfect hash */
        if (!perfect(tabb, tabh, tabq, smax, scramble, (ub4)keys.count()))
        {
            if (++bad_perfect >= RETRY_PERFECT)
            {
                if (blen < smax)
                {
                    blen *= 2;
                    tabb.resize(blen);
                    tabq.resize(blen+1);
                    --si;               /* we know this salt got distinct (A,B) */
                }
                else
                {
                    return 0;
                }
                bad_perfect = 0;
            }
            continue;
        }

        break;
    }

    return 1;
}

/*
------------------------------------------------------------------------------
Input/output type routines
------------------------------------------------------------------------------
*/


static void
make_perfect(dyld3::OverflowSafeArray<key>& keys, perfect_hash& result)
{
    dyld3::OverflowSafeArray<bstuff>  tab;                /* table indexed by b */
    ub4       smax;            /* scramble[] values in 0..smax-1, a power of 2 */
    ub4       alen;                            /* a in 0..alen-1, a power of 2 */
    ub8       salt;                       /* a parameter to the hash function */
    ub4       scramble[SCRAMBLE_LEN];           /* used in final hash function */
    int ok;
    uint32_t i;

    /* find the hash */
    smax = ((ub4)1<<log2u((ub4)keys.count()));
    ok = findhash(tab, &alen, &salt,
                  scramble, smax, keys);
    if (!ok) {
        smax = 2 * ((ub4)1<<log2u((ub4)keys.count()));
        ok = findhash(tab, &alen, &salt,
                      scramble, smax, keys);
    }
    if (!ok) {
        bzero(&result, sizeof(result));
    } else {
        /* build the tables */
        result.capacity = smax;
        result.occupied = (ub4)keys.count();
        result.shift = UB8BITS - log2u(alen);
        result.mask = (ub4)tab.count() - 1;
        result.salt = salt;

        result.tab.resize(tab.count());
        for (i = 0; i < tab.count(); i++) {
            result.tab[i] = tab[i].val_b;
        }
        for (i = 0; i < 256; i++) {
            result.scramble[i] = scramble[i];
        }
    }
}

// SELOPT_WRITE || CLOSURE_SELOPT_WRITE
#endif

#ifdef SELOPT_WRITE

static void
make_perfect(const string_map& strings, perfect_hash& phash)
{
    dyld3::OverflowSafeArray<key> keys;

    /* read in the list of keywords */
    keys.reserve(strings.size());
    size_t i;
    string_map::const_iterator s;
    for (i = 0, s = strings.begin(); s != strings.end(); ++s, ++i) {
        key mykey;
        mykey.name_k = (ub1 *)s->first;
        mykey.len_k  = (ub4)strlen(s->first);
        keys.push_back(mykey);
    }

    make_perfect(keys, phash);
}

// SELOPT_WRITE
#endif

#ifdef CLOSURE_SELOPT_WRITE

static void
make_perfect(const dyld3::OverflowSafeArray<const char*>& strings, perfect_hash& phash)
{
    dyld3::OverflowSafeArray<key> keys;

    /* read in the list of keywords */
    keys.reserve(strings.count());
    for (const char* s : strings) {
        key mykey;
        mykey.name_k = (ub1 *)s;
        mykey.len_k  = (ub4)strlen(s);
        keys.push_back(mykey);
    }

    make_perfect(keys, phash);
}

// CLOSURE_SELOPT_WRITE
#endif

// namespace objc_selopt
};

#undef S32
#undef S64

#endif
