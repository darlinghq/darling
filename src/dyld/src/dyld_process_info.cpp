/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
 *
 * Copyright (c) 2016 Apple Inc. All rights reserved.
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

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <libproc.h>
#include <sys/param.h>
#include <mach/shared_region.h>
#include <mach/mach_vm.h>
#include <mach/vm_region.h>
#include <libkern/OSAtomic.h>

#include "dyld_process_info.h"
#include "dyld_process_info_internal.h"
#include "dyld_images.h"
#include "dyld_priv.h"


//
// Opaque object returned by _dyld_process_info_create()
//

struct __attribute__((visibility("hidden"))) dyld_process_info_base {
    static dyld_process_info_base* make(task_t task, const dyld_all_image_infos_64& allImageInfo, const dyld_image_info_64 imageArray[], kern_return_t* kr);
    static dyld_process_info_base* makeSuspended(task_t task, kern_return_t* kr);

    uint32_t&                   retainCount() const { return _retainCount; }
    dyld_process_cache_info*    cacheInfo() const { return (dyld_process_cache_info*)(((char*)this) + _cacheInfoOffset); }
    dyld_process_state_info*    stateInfo() const { return (dyld_process_state_info*)(((char*)this) + _stateInfoOffset); }
    void                        forEachImage(void (^callback)(uint64_t machHeaderAddress, const uuid_t uuid, const char* path)) const;
    void                        forEachSegment(uint64_t machHeaderAddress, void (^callback)(uint64_t segmentAddress, uint64_t segmentSize, const char* segmentName)) const;

private:
    struct ImageInfo {
        uuid_t                  uuid;
        uint64_t                loadAddress;
        const char*             path;
        uint32_t                segmentStartIndex;
        uint32_t                segmentsCount;
    };

    struct SegmentInfo {
        const char*             name;
        uint64_t                addr;
        uint64_t                size;
    };

                                dyld_process_info_base(unsigned imageCount, size_t totalSize);
    void*                       operator new (size_t, void* buf) { return buf; }

    static bool                 inCache(uint64_t addr) { return (addr > SHARED_REGION_BASE) && (addr < SHARED_REGION_BASE+SHARED_REGION_SIZE); }
    kern_return_t               addImage(task_t task, bool sameCacheAsThisProcess, uint64_t imageAddress, uint64_t imagePath, const char* imagePathLocal);
    kern_return_t               addDyldImage(task_t task, uint64_t dyldAddress, uint64_t dyldPathAddress, const char* localPath);

    bool                        invalid() { return ((char*)_stringRevBumpPtr < (char*)_curSegment); }
    const char*                 copyPath(task_t task, uint64_t pathAddr, kern_return_t* kr);
    const char*                 addString(const char*);
    const char*                 copySegmentName(const char*);

    void                        addInfoFromLoadCommands(const mach_header* mh, uint64_t addressInTask, size_t size);

    void                        inspectLocalImageLoadCommands(uint64_t imageAddress, void* func);
    kern_return_t               inspectRemoteImageLoadCommands(task_t task, uint64_t imageAddress, void* func);

    mutable uint32_t            _retainCount;
    const uint32_t              _cacheInfoOffset;
    const uint32_t              _stateInfoOffset;
    const uint32_t              _imageInfosOffset;
    const uint32_t              _segmentInfosOffset;
    ImageInfo* const            _firstImage;
    ImageInfo*                  _curImage;
    SegmentInfo* const          _firstSegment;
    SegmentInfo*                _curSegment;
    uint32_t                    _curSegmentIndex;
    char*                       _stringRevBumpPtr;

    // dyld_process_cache_info  cacheInfo;
    // dyld_process_state_info  stateInfo;
    // ImageInfo                images[];
    // SegmentInfo              segments[];
    // char                     stringPool[]
};

dyld_process_info_base::dyld_process_info_base(unsigned imageCount, size_t totalSize)
 :  _retainCount(1), _cacheInfoOffset(sizeof(dyld_process_info_base)),
    _stateInfoOffset(sizeof(dyld_process_info_base) + sizeof(dyld_process_cache_info)),
    _imageInfosOffset(sizeof(dyld_process_info_base) + sizeof(dyld_process_cache_info) + sizeof(dyld_process_state_info)),
    _segmentInfosOffset(sizeof(dyld_process_info_base) + sizeof(dyld_process_cache_info) + sizeof(dyld_process_state_info) + imageCount*sizeof(ImageInfo)),
    _firstImage((ImageInfo*)(((uint8_t*)this) + _imageInfosOffset)),
    _curImage((ImageInfo*)(((uint8_t*)this) + _imageInfosOffset)),
    _firstSegment((SegmentInfo*)(((uint8_t*)this) + _segmentInfosOffset)),
    _curSegment((SegmentInfo*)(((uint8_t*)this) + _segmentInfosOffset)),
    _curSegmentIndex(0),
    _stringRevBumpPtr((char*)(this)+totalSize)
{
}


dyld_process_info_base* dyld_process_info_base::make(task_t task, const dyld_all_image_infos_64& allImageInfo, const dyld_image_info_64 imageArray[], kern_return_t* kr)
{
    // figure out how many path strings will need to be copied and their size
    const dyld_all_image_infos* myInfo = _dyld_get_all_image_infos();
    bool sameCacheAsThisProcess = ((memcmp(myInfo->sharedCacheUUID, allImageInfo.sharedCacheUUID, 16) == 0) && (myInfo->sharedCacheSlide == allImageInfo.sharedCacheSlide));
    unsigned countOfPathsNeedingCopying = 0;
    if ( sameCacheAsThisProcess ) {
        for (int i=0; i < allImageInfo.infoArrayCount; ++i) {
            if ( !inCache(imageArray[i].imageFilePath) )
                ++countOfPathsNeedingCopying;
        }
    }
    else {
        countOfPathsNeedingCopying = allImageInfo.infoArrayCount+1;
    }
    unsigned imageCountWithDyld = allImageInfo.infoArrayCount+1;

    // allocate result object
    size_t allocationSize =   sizeof(dyld_process_info_base)
                            + sizeof(dyld_process_cache_info)
                            + sizeof(dyld_process_state_info)
                            + sizeof(ImageInfo)*(imageCountWithDyld)
                            + sizeof(SegmentInfo)*imageCountWithDyld*5
                            + countOfPathsNeedingCopying*PATH_MAX;
    void* storage = malloc(allocationSize);
    dyld_process_info_base* obj =  new (storage) dyld_process_info_base(imageCountWithDyld, allocationSize); // placement new()

    // fill in base info
    dyld_process_cache_info* cacheInfo = obj->cacheInfo();
    memcpy(cacheInfo->cacheUUID, allImageInfo.sharedCacheUUID, 16);
    cacheInfo->cacheBaseAddress    = allImageInfo.sharedCacheBaseAddress;
    cacheInfo->privateCache        = allImageInfo.processDetachedFromSharedRegion;
    // if no cache is used, allImageInfo has all zeros for cache UUID
    cacheInfo->noCache = true;
    for (int i=0; i < 16; ++i) {
        if ( cacheInfo->cacheUUID[i] != 0 ) {
            cacheInfo->noCache = false;
        }
    }

    dyld_process_state_info* stateInfo = obj->stateInfo();
    stateInfo->timestamp           = allImageInfo.infoArrayChangeTimestamp;
    stateInfo->imageCount          = imageCountWithDyld;
    stateInfo->initialImageCount   = (uint32_t)(allImageInfo.initialImageCount+1);
    if ( allImageInfo.infoArray != 0 )
        stateInfo->dyldState = dyld_process_state_dyld_initialized;
    if ( allImageInfo.libSystemInitialized != 0 ) {
        stateInfo->dyldState = dyld_process_state_libSystem_initialized;
        if ( allImageInfo.initialImageCount != allImageInfo.infoArrayCount )
            stateInfo->dyldState = dyld_process_state_program_running;
    }
    if ( allImageInfo.errorMessage != 0 )
        stateInfo->dyldState = allImageInfo.terminationFlags ? dyld_process_state_terminated_before_inits : dyld_process_state_dyld_terminated;

    // fill in info for dyld
    if ( allImageInfo.dyldPath != 0 ) {
        if ( kern_return_t r = obj->addDyldImage(task, allImageInfo.dyldImageLoadAddress, allImageInfo.dyldPath, NULL) ) {
            if ( kr != NULL )
                *kr = r;
            goto fail;
        }
    }
    
    // fill in info for each image
    for (uint32_t i=0; i < allImageInfo.infoArrayCount; ++i) {
        if ( kern_return_t r = obj->addImage(task, sameCacheAsThisProcess, imageArray[i].imageLoadAddress, imageArray[i].imageFilePath, NULL) ) {
           if ( kr != NULL )
                *kr = r;
            goto fail;
        }
    }

    // sanity check internal data did not overflow
    if ( obj->invalid() )
        goto fail;

    return obj;

fail:
    free(obj);
    return NULL;
}

dyld_process_info_base* dyld_process_info_base::makeSuspended(task_t task, kern_return_t* kr)
{
    pid_t   pid;
    kern_return_t result = pid_for_task(task, &pid);
    if ( result != KERN_SUCCESS ) {
        if ( kr != NULL )
            *kr = result;
        return NULL;
    }

    unsigned imageCount = 0;  // main executable and dyld
    uint64_t            mainExecutableAddress = 0;
    uint64_t            dyldAddress = 0;
    char                dyldPathBuffer[PATH_MAX+1];
    char                mainExecutablePathBuffer[PATH_MAX+1];
    mach_vm_size_t      size;
    for (mach_vm_address_t address = 0; ; address += size) {
        vm_region_basic_info_data_64_t  info;
        mach_port_t                     objectName;
        unsigned int                    infoCount = VM_REGION_BASIC_INFO_COUNT_64;
        result = mach_vm_region(task, &address, &size, VM_REGION_BASIC_INFO,
                         (vm_region_info_t)&info, &infoCount, &objectName);
        if ( result != KERN_SUCCESS )
            break;
        if ( info.protection == (VM_PROT_READ|VM_PROT_EXECUTE) ) {
            if ( mainExecutableAddress == 0 ) {
                mainExecutableAddress = address;
                int len = proc_regionfilename(pid, mainExecutableAddress, mainExecutablePathBuffer, PATH_MAX);
                if ( len != 0 )
                    mainExecutablePathBuffer[len] = '\0';
                ++imageCount;
            }
            else if ( dyldAddress == 0 ) {
                dyldAddress = address;
                int len = proc_regionfilename(pid, dyldAddress, dyldPathBuffer, PATH_MAX);
                if ( len != 0 )
                    dyldPathBuffer[len] = '\0';
                ++imageCount;
            }
            //fprintf(stderr, "vm region: addr=0x%llX, size=0x%llX, prot=0x%X\n", (uint64_t)address, (uint64_t)size, info.protection);
        }
    }
    //fprintf(stderr, "dyld: addr=0x%llX, path=%s\n", dyldAddress, dyldPathBuffer);
    //fprintf(stderr, "app:  addr=0x%llX, path=%s\n", mainExecutableAddress, mainExecutablePathBuffer);

    // allocate result object
    size_t allocationSize =   sizeof(dyld_process_info_base)
                            + sizeof(dyld_process_cache_info)
                            + sizeof(dyld_process_state_info)
                            + sizeof(ImageInfo)*(imageCount)
                            + sizeof(SegmentInfo)*imageCount*5
                            + imageCount*PATH_MAX;
    void* storage = malloc(allocationSize);
    dyld_process_info_base* obj =  new (storage) dyld_process_info_base(imageCount, allocationSize); // placement new()

    // fill in base info
    dyld_process_cache_info* cacheInfo = obj->cacheInfo();
    bzero(cacheInfo->cacheUUID, 16);
    cacheInfo->cacheBaseAddress    = 0;
    cacheInfo->noCache             = true;
    cacheInfo->privateCache        = false;

    dyld_process_state_info* stateInfo = obj->stateInfo();
    stateInfo->timestamp           = 0;
    stateInfo->imageCount          = imageCount;
    stateInfo->initialImageCount   = imageCount;
    stateInfo->dyldState           = dyld_process_state_not_started;

    // fill in info for dyld
    if ( dyldAddress != 0 ) {
        if ( kern_return_t r = obj->addDyldImage(task, dyldAddress, 0, dyldPathBuffer) ) {
            if ( kr != NULL )
                *kr = r;
            free(obj);
            return NULL;
        }
    }

    // fill in info for each image
    if ( mainExecutableAddress != 0 ) {
        if ( kern_return_t r = obj->addImage(task, false, mainExecutableAddress, 0, mainExecutablePathBuffer) ) {
            if ( kr != NULL )
                *kr = r;
            free(obj);
            return NULL;
        }
    }

    return obj;
}



const char* dyld_process_info_base::addString(const char* str)
{
    size_t len = strlen(str) + 1;
    _stringRevBumpPtr -= len;
    strcpy(_stringRevBumpPtr, str);
    return _stringRevBumpPtr;
}

const char* dyld_process_info_base::copyPath(task_t task, uint64_t stringAddressInTask, kern_return_t* kr)
{
    char temp[PATH_MAX+8]; // +8 is to allow '\0' at temp[PATH_MAX]
    mach_vm_size_t readSize = PATH_MAX;
    if ( ((stringAddressInTask & 0xFFF) + PATH_MAX) < 4096 ) {
        // string fits within page, only one vm_read needed
        if ( kern_return_t r = mach_vm_read_overwrite(task, stringAddressInTask, PATH_MAX, (vm_address_t)&temp, &readSize) ) {
            if ( kr != NULL )
                *kr = r;
            return NULL;
        }
    }
    else {
        // string may cross page boundary, split into two reads
        size_t firstLen = 4096 - (stringAddressInTask & 0xFFF);
        readSize = firstLen;
        if ( kern_return_t r = mach_vm_read_overwrite(task, stringAddressInTask, firstLen, (vm_address_t)&temp, &readSize) ) {
            if ( kr != NULL )
                *kr = r;
            return NULL;
        }
        temp[firstLen] = '\0';
        if ( strlen(temp) >= firstLen ) {
            readSize = PATH_MAX-firstLen;
            if ( kern_return_t r = mach_vm_read_overwrite(task, stringAddressInTask+firstLen, PATH_MAX-firstLen, (vm_address_t)&temp+firstLen, &readSize) ) {
                if ( kr != NULL )
                    *kr = r;
                return NULL;
                temp[PATH_MAX] = '\0'; // truncate any string that is too long
            }
        }
    }
    if ( kr != NULL )
        *kr = KERN_SUCCESS;
    return addString(temp);
}


kern_return_t dyld_process_info_base::addImage(task_t task, bool sameCacheAsThisProcess, uint64_t imageAddress, uint64_t imagePath, const char* imagePathLocal)
{
    _curImage->loadAddress = imageAddress;
    _curImage->segmentStartIndex = _curSegmentIndex;
    if ( imagePathLocal != NULL ) {
        _curImage->path = addString(imagePathLocal);
    }
    else if ( sameCacheAsThisProcess && inCache(imagePath) ) {
        _curImage->path = (const char*)imagePath;
    }
    else {
        kern_return_t kr;
        _curImage->path = copyPath(task, imagePath, &kr);
        if ( kr )
            return kr;
    }
    if ( sameCacheAsThisProcess && inCache(imageAddress) ) {
        addInfoFromLoadCommands((mach_header*)imageAddress, imageAddress, 32*1024);
    }
    else {
        mach_vm_size_t readSize = sizeof(mach_header_64);
        mach_header_64 mhBuffer;
        if ( kern_return_t r = mach_vm_read_overwrite(task, imageAddress, sizeof(mach_header_64), (vm_address_t)&mhBuffer, &readSize) ) {
            return r;
        }
        size_t          headerPagesSize = (sizeof(mach_header_64) + mhBuffer.sizeofcmds + 4095) & (-4096);
        vm_address_t    localCopyBuffer;
        unsigned int    localCopyBufferSize;
        if ( kern_return_t r = mach_vm_read(task, imageAddress, headerPagesSize, &localCopyBuffer, &localCopyBufferSize) ) {
            return r;
        }
        addInfoFromLoadCommands((mach_header*)localCopyBuffer, imageAddress, localCopyBufferSize);
        vm_deallocate(mach_task_self(), localCopyBuffer, localCopyBufferSize);
    }
    _curImage->segmentsCount = _curSegmentIndex - _curImage->segmentStartIndex;
    _curImage++;
    return KERN_SUCCESS;
}


kern_return_t dyld_process_info_base::addDyldImage(task_t task, uint64_t dyldAddress, uint64_t dyldPathAddress, const char* localPath)
{
    kern_return_t kr;
    _curImage->loadAddress = dyldAddress;
    _curImage->segmentStartIndex = _curSegmentIndex;
    if ( localPath != NULL ) {
        _curImage->path = addString(localPath);
    }
    else {
        _curImage->path = copyPath(task, dyldPathAddress, &kr);
        if ( kr )
            return kr;
    }

    mach_vm_size_t readSize = sizeof(mach_header_64);
    mach_header_64 mhBuffer;
    if ( kern_return_t r = mach_vm_read_overwrite(task, dyldAddress, sizeof(mach_header_64), (vm_address_t)&mhBuffer, &readSize) ) {
        return r;
    }
    size_t          headerPagesSize = (sizeof(mach_header_64) + mhBuffer.sizeofcmds + 4095) & (-4096);
    vm_address_t    localCopyBuffer;
    unsigned int    localCopyBufferSize;
    if ( kern_return_t r = mach_vm_read(task, dyldAddress, headerPagesSize, &localCopyBuffer, &localCopyBufferSize) ) {
        return r;
    }
    addInfoFromLoadCommands((mach_header*)localCopyBuffer, dyldAddress, localCopyBufferSize);
    vm_deallocate(mach_task_self(), localCopyBuffer, localCopyBufferSize);
    _curImage->segmentsCount = _curSegmentIndex - _curImage->segmentStartIndex;
    _curImage++;
    return KERN_SUCCESS;
}


void dyld_process_info_base::addInfoFromLoadCommands(const mach_header* mh, uint64_t addressInTask, size_t size)
{
    const load_command* startCmds = NULL;
    if ( mh->magic == MH_MAGIC_64 )
        startCmds = (load_command*)((char *)mh + sizeof(mach_header_64));
    else if ( mh->magic == MH_MAGIC )
        startCmds = (load_command*)((char *)mh + sizeof(mach_header));
    else
        return;  // not a mach-o file, or wrong endianness
        
    const load_command* const cmdsEnd = (load_command*)((char*)startCmds + mh->sizeofcmds);
    const load_command* cmd = startCmds;
    for(uint32_t i = 0; i < mh->ncmds; ++i) {
        const load_command* nextCmd = (load_command*)((char *)cmd + cmd->cmdsize);
        if ( (cmd->cmdsize < 8) || (nextCmd > cmdsEnd) || (nextCmd < startCmds) ) {
            return;  // malformed load command
        }
        if ( cmd->cmd == LC_UUID ) {
            const uuid_command* uuidCmd = (uuid_command*)cmd;
            memcpy(_curImage->uuid, uuidCmd->uuid, 16);
        }
        else if ( cmd->cmd == LC_SEGMENT ) {
            const segment_command* segCmd = (segment_command*)cmd;
            _curSegment->name = copySegmentName(segCmd->segname);
            _curSegment->addr = segCmd->vmaddr;
            _curSegment->size = segCmd->vmsize;
            _curSegment++;
            _curSegmentIndex++;
        }
        else if ( cmd->cmd == LC_SEGMENT_64 ) {
            const segment_command_64* segCmd = (segment_command_64*)cmd;
            _curSegment->name = copySegmentName(segCmd->segname);
            _curSegment->addr = segCmd->vmaddr;
            _curSegment->size = segCmd->vmsize;
            _curSegment++;
            _curSegmentIndex++;
        }
        cmd = nextCmd;
    }
}

const char* dyld_process_info_base::copySegmentName(const char* name)
{
    // don't copy names of standard segments into string pool
    static const char* stdSegNames[] = {"__TEXT", "__DATA", "__LINKEDIT", "__DATA_DIRTY", "__DATA_CONST", "__OBJC", NULL };
    for (const char** s=stdSegNames; *s != NULL; ++s) {
        if ( strcmp(name, *s) == 0 )
        return *s;
    }
    // copy custom segment names into string pool
    return addString(name);
}

void dyld_process_info_base::forEachImage(void (^callback)(uint64_t machHeaderAddress, const uuid_t uuid, const char* path)) const
{
    for (const ImageInfo* p = _firstImage; p < _curImage; ++p) {
        callback(p->loadAddress, p->uuid, p->path);
    }
}

void dyld_process_info_base::forEachSegment(uint64_t machHeaderAddress, void (^callback)(uint64_t segmentAddress, uint64_t segmentSize, const char* segmentName)) const
{
    for (const ImageInfo* p = _firstImage; p < _curImage; ++p) {
        if ( p->loadAddress == machHeaderAddress ) {
            uint64_t slide = 0;
            for (int i=0; i < p->segmentsCount; ++i) {
                const SegmentInfo* seg = &_firstSegment[p->segmentStartIndex+i];
                if ( strcmp(seg->name, "__TEXT") == 0 ) {
                    slide = machHeaderAddress - seg->addr;
                    break;
                }
            }
            for (int i=0; i < p->segmentsCount; ++i) {
                const SegmentInfo* seg = &_firstSegment[p->segmentStartIndex+i];
                callback(seg->addr + slide, seg->size, seg->name);
            }
            break;
        }
    }
}





// Implementation that works with existing dyld data structures
dyld_process_info _dyld_process_info_create(task_t task, uint64_t timestamp, kern_return_t* kr)
{
    if ( kr != NULL )
        *kr = KERN_SUCCESS;

    task_dyld_info_data_t task_dyld_info;
    mach_msg_type_number_t count = TASK_DYLD_INFO_COUNT;
    if ( kern_return_t r = task_info(task, TASK_DYLD_INFO, (task_info_t)&task_dyld_info, &count) ) {
        if ( kr != NULL )
            *kr = r;
        return  NULL;
    }

    //The kernel will return MACH_VM_MIN_ADDRESS for an executable that has not had dyld loaded
    if (task_dyld_info.all_image_info_addr == MACH_VM_MIN_ADDRESS)
        return NULL;

    if ( task_dyld_info.all_image_info_size > sizeof(dyld_all_image_infos_64) )
        return NULL;

    // read all_image_infos struct
    dyld_all_image_infos_64 allImageInfo64;
    mach_vm_size_t readSize = task_dyld_info.all_image_info_size;
    if ( kern_return_t r = mach_vm_read_overwrite(task, task_dyld_info.all_image_info_addr, task_dyld_info.all_image_info_size, (vm_address_t)&allImageInfo64, &readSize) ) {
        if ( kr != NULL )
            *kr = r;
        return  NULL;
    }
    if ( allImageInfo64.infoArrayCount == 0 ) {
        // could be task was launch suspended or still launching, wait a moment to see
        usleep(1000 * 50); // 50ms
        if ( kern_return_t r = mach_vm_read_overwrite(task, task_dyld_info.all_image_info_addr, task_dyld_info.all_image_info_size, (vm_address_t)&allImageInfo64, &readSize) ) {
            if ( kr != NULL )
                *kr = r;
            return  NULL;
        }
        // if infoArrayCount is still zero, then target was most likely launched suspended
        if ( allImageInfo64.infoArrayCount == 0 )
            return dyld_process_info_base::makeSuspended(task, kr);
    }

    // bail out of dyld is too old
    if ( allImageInfo64.version < 15 ) {
        if ( kr != NULL )
            *kr = KERN_INVALID_HOST;
        return NULL;
    }

    // normalize by expanding 32-bit all_image_infos into 64-bit one
    uint32_t imageCount = allImageInfo64.infoArrayCount;
    size_t imageArraySize = imageCount * sizeof(dyld_image_info_64);
    if ( task_dyld_info.all_image_info_format == TASK_DYLD_ALL_IMAGE_INFO_32 ) {
        const dyld_all_image_infos_32* allImageInfo32 = (dyld_all_image_infos_32*)&allImageInfo64;
        dyld_all_image_infos_64 info64;
        bzero(&info64, sizeof(info64));
        info64.version                  = allImageInfo32->version;
        info64.infoArrayCount           = allImageInfo32->infoArrayCount;
        info64.infoArray                = allImageInfo32->infoArray;
        info64.processDetachedFromSharedRegion = allImageInfo32->processDetachedFromSharedRegion;
        info64.libSystemInitialized     = allImageInfo32->libSystemInitialized;
        info64.dyldImageLoadAddress     = allImageInfo32->dyldImageLoadAddress;
        info64.initialImageCount        = allImageInfo32->initialImageCount;
        info64.uuidArrayCount           = allImageInfo32->uuidArrayCount;
        info64.uuidArray                = allImageInfo32->uuidArray;
        info64.dyldAllImageInfosAddress = allImageInfo32->dyldAllImageInfosAddress;
        info64.sharedCacheSlide         = allImageInfo32->sharedCacheSlide;
        info64.infoArrayChangeTimestamp = allImageInfo32->infoArrayChangeTimestamp;
        info64.sharedCacheBaseAddress   = allImageInfo32->sharedCacheBaseAddress;
        info64.dyldPath                 = allImageInfo32->dyldPath;
        memcpy((void*)(info64.sharedCacheUUID), (void*)(allImageInfo32->sharedCacheUUID), 16);
        allImageInfo64 = info64;
        imageCount = allImageInfo64.infoArrayCount;
        imageArraySize = imageCount * sizeof(dyld_image_info_32);
    }

    // don't do any (more) work if target process's dyld timestamp has not changed since previous query
    if ( (timestamp != 0) && (timestamp == allImageInfo64.infoArrayChangeTimestamp) ) {
        if ( kr != NULL )
            *kr = KERN_SUCCESS;
        return NULL;
    }

    // For the moment we are going to truncate any image list longer than 8192 because some programs do
    // terrible things that corrupt their own image lists and we need to stop clients from crashing
    // reading them. We can try to do something more advanced in the future. rdar://27446361
    imageCount = MIN(imageCount, 8192);

    // read image array
    dyld_image_info_64 imageArray64[imageCount];
    if ( kern_return_t r = mach_vm_read_overwrite(task, allImageInfo64.infoArray, imageArraySize, (vm_address_t)&imageArray64, &readSize) ) {
         if ( kr != NULL )
            *kr = r;
        return  NULL;
    }
    // normalize by expanding 32-bit image_infos into 64-bit ones
    if ( task_dyld_info.all_image_info_format == TASK_DYLD_ALL_IMAGE_INFO_32 ) {
        const dyld_image_info_32* imageArray32 = (dyld_image_info_32*)&imageArray64;
        dyld_image_info_64 tempArray[imageCount];
        for (uint32_t i=0; i < imageCount; ++i) {
            tempArray[i].imageLoadAddress = imageArray32[i].imageLoadAddress;
            tempArray[i].imageFilePath    = imageArray32[i].imageFilePath;
            tempArray[i].imageFileModDate = imageArray32[i].imageFileModDate;
        }
        memcpy(imageArray64, tempArray, sizeof(dyld_image_info_64)*imageCount);
    }

    // create object based on local copy of all image infos and image array
    return dyld_process_info_base::make(task, allImageInfo64, imageArray64, kr);
}


void _dyld_process_info_get_state(dyld_process_info info, dyld_process_state_info* stateInfo)
{
    *stateInfo = *info->stateInfo();
}

void _dyld_process_info_get_cache(dyld_process_info info, dyld_process_cache_info* cacheInfo)
{
    *cacheInfo = *info->cacheInfo();
}

void _dyld_process_info_retain(dyld_process_info info)
{
    info->retainCount() += 1;
}

void _dyld_process_info_release(dyld_process_info info)
{
    info->retainCount() -= 1;
    if ( info->retainCount() == 0 )
        free((void*)info);
}

void _dyld_process_info_for_each_image(dyld_process_info info, void (^callback)(uint64_t machHeaderAddress, const uuid_t uuid, const char* path))
{
    info->forEachImage(callback);
}


void _dyld_process_info_for_each_segment(dyld_process_info info, uint64_t machHeaderAddress, void (^callback)(uint64_t segmentAddress, uint64_t segmentSize, const char* segmentName))
{
    info->forEachSegment(machHeaderAddress, callback);
}



