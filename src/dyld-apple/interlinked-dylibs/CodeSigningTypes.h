/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- 
 *
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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

#ifndef _CODE_SIGNING_TYPES_
#define _CODE_SIGNING_TYPES_

#include <stdint.h>
#include <stddef.h>


//
// Magic numbers used by Code Signing
//
enum {
    CSMAGIC_REQUIREMENT         = 0xfade0c00,    // single Requirement blob
    CSMAGIC_REQUIREMENTS        = 0xfade0c01,    // Requirements vector (internal requirements)
    CSMAGIC_CODEDIRECTORY       = 0xfade0c02,    // CodeDirectory blob
    CSMAGIC_EMBEDDED_SIGNATURE  = 0xfade0cc0,    // embedded form of signature data
    CSMAGIC_DETACHED_SIGNATURE  = 0xfade0cc1,    // multi-arch collection of embedded signatures
    CSMAGIC_BLOBWRAPPER         = 0xfade0b01,    // used for the cms blob
};

enum {
    CS_PAGE_SIZE                = 4096,

    CS_HASHTYPE_SHA1              = 1,
    CS_HASHTYPE_SHA256            = 2,
    CS_HASHTYPE_SHA256_TRUNCATED  = 3,

    CS_HASH_SIZE_SHA1             = 20,
    CS_HASH_SIZE_SHA256           = 32,
    CS_HASH_SIZE_SHA256_TRUNCATED = 20,

    CSSLOT_CODEDIRECTORY        = 0,
    CSSLOT_INFOSLOT             = 1,
    CSSLOT_REQUIREMENTS         = 2,
    CSSLOT_RESOURCEDIR          = 3,
    CSSLOT_APPLICATION          = 4,
    CSSLOT_ENTITLEMENTS         = 5,
    CSSLOT_CMS_SIGNATURE        = 0x10000,

	kSecCodeSignatureAdhoc		= 2
};



//
// Structure of a SuperBlob
//
struct CS_BlobIndex {
    uint32_t type;                  // type of entry
    uint32_t offset;                // offset of entry
};

struct CS_SuperBlob {
    uint32_t magic;                 // magic number
    uint32_t length;                // total length of SuperBlob
    uint32_t count;                 // number of index entries following
    CS_BlobIndex index[];           // (count) entries
    // followed by Blobs in no particular order as indicated by offsets in index
};

//
// C form of a CodeDirectory.
//
struct CS_CodeDirectory {
    uint32_t magic;                 // magic number (CSMAGIC_CODEDIRECTORY) */
    uint32_t length;                // total length of CodeDirectory blob
    uint32_t version;               // compatibility version
    uint32_t flags;                 // setup and mode flags
    uint32_t hashOffset;            // offset of hash slot element at index zero
    uint32_t identOffset;           // offset of identifier string
    uint32_t nSpecialSlots;         // number of special hash slots
    uint32_t nCodeSlots;            // number of ordinary (code) hash slots
    uint32_t codeLimit;             // limit to main image signature range
    uint8_t  hashSize;              // size of each hash in bytes
    uint8_t  hashType;              // type of hash (cdHashType* constants)
    uint8_t  platform;              // platform identifier; zero if not platform binary
    uint8_t  pageSize;              // log2(page size in bytes); 0 => infinite
    uint32_t spare2;                // unused (must be zero)
    // Version 0x20100 or later
    uint32_t scatterOffset;         // offset of optional scatter vector
    // followed by dynamic content as located by offset fields above
};

struct CS_Blob {
    uint32_t magic;                 // magic number
    uint32_t length;                // total length of blob
};

struct CS_RequirementsBlob {
    uint32_t magic;                 // magic number
    uint32_t length;                // total length of blob
    uint32_t data;                  // zero for dyld shared cache
};


struct CS_Scatter {
    uint32_t count;                 // number of pages; zero for sentinel (only)
    uint32_t base;                  // first page number
    uint64_t targetOffset;          // byte offset in target
    uint64_t spare;                 // reserved (must be zero)
};


#endif // _CODE_SIGNING_TYPES_



