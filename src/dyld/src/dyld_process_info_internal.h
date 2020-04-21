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

#ifndef _DYLD_PROCESS_INFO_INTERNAL_H_
#define _DYLD_PROCESS_INFO_INTERNAL_H_

#define VIS_HIDDEN __attribute__((visibility("hidden")))

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <uuid/uuid.h>

#include <array>

struct dyld_all_image_infos_32 {
    uint32_t                        version;
    uint32_t                        infoArrayCount;
    std::atomic<uint32_t>           infoArray;
    uint32_t                        notification;
    bool                            processDetachedFromSharedRegion;
    bool                            libSystemInitialized;
    uint32_t                        dyldImageLoadAddress;
    uint32_t                        jitInfo;
    uint32_t                        dyldVersion;
    uint32_t                        errorMessage;
    uint32_t                        terminationFlags;
    uint32_t                        coreSymbolicationShmPage;
    uint32_t                        systemOrderFlag;
    uint32_t                        uuidArrayCount;
    uint32_t                        uuidArray;
    uint32_t                        dyldAllImageInfosAddress;
    uint32_t                        initialImageCount;
    uint32_t                        errorKind;
    uint32_t                        errorClientOfDylibPath;
    uint32_t                        errorTargetDylibPath;
    uint32_t                        errorSymbol;
    uint32_t                        sharedCacheSlide;
    std::array<uint8_t, 16>         sharedCacheUUID;
    uint32_t                        sharedCacheBaseAddress;
    std::atomic<uint64_t>           infoArrayChangeTimestamp;
    uint32_t                        dyldPath;
    uint32_t                        notifyMachPorts[8];
    uint32_t                        reserved[5];
    uint32_t                        compact_dyld_image_info_addr;
    uint32_t                        compact_dyld_image_info_size;
    uint32_t                        platform;
};

struct dyld_all_image_infos_64 {
    uint32_t                version;
    uint32_t                infoArrayCount;
    std::atomic<uint64_t>   infoArray;
    uint64_t                notification;
    bool                    processDetachedFromSharedRegion;
    bool                    libSystemInitialized;
    uint32_t                paddingToMakeTheSizeCorrectOn32bitAndDoesntAffect64b; // NOT PART OF DYLD_ALL_IMAGE_INFOS!
    uint64_t                dyldImageLoadAddress;
    uint64_t                jitInfo;
    uint64_t                dyldVersion;
    uint64_t                errorMessage;
    uint64_t                terminationFlags;
    uint64_t                coreSymbolicationShmPage;
    uint64_t                systemOrderFlag;
    uint64_t                uuidArrayCount;
    uint64_t                uuidArray;
    uint64_t                dyldAllImageInfosAddress;
    uint64_t                initialImageCount;
    uint64_t                errorKind;
    uint64_t                errorClientOfDylibPath;
    uint64_t                errorTargetDylibPath;
    uint64_t                errorSymbol;
    uint64_t                sharedCacheSlide;
    std::array<uint8_t, 16> sharedCacheUUID;
    uint64_t                sharedCacheBaseAddress;
    std::atomic<uint64_t>   infoArrayChangeTimestamp;
    uint64_t                dyldPath;
    uint32_t                notifyMachPorts[8];
    uint64_t                reserved[9];
    uint64_t                compact_dyld_image_info_addr;
    uint64_t                compact_dyld_image_info_size;
    uint32_t                platform;
};

struct dyld_image_info_32 {
    uint32_t                    imageLoadAddress;
    uint32_t                    imageFilePath;
    uint32_t                    imageFileModDate;
};
struct dyld_image_info_64 {
    uint64_t                    imageLoadAddress;
    uint64_t                    imageFilePath;
    uint64_t                    imageFileModDate;
};

#define DYLD_PROCESS_INFO_NOTIFY_MAX_BUFFER_SIZE	(32*1024)
#define DYLD_PROCESS_INFO_NOTIFY_LOAD_ID			0x1000
#define DYLD_PROCESS_INFO_NOTIFY_UNLOAD_ID			0x2000
#define DYLD_PROCESS_INFO_NOTIFY_MAIN_ID			0x3000


struct dyld_process_info_image_entry {
    uuid_t						uuid;
    uint64_t                    loadAddress;
    uint32_t                    pathStringOffset;
    uint32_t                    pathLength;
};

struct dyld_process_info_notify_header {
	mach_msg_header_t			header;
    uint32_t                    version;
    uint32_t                    imageCount;
    uint32_t                    imagesOffset;
    uint32_t                    stringsOffset;
    uint64_t                    timestamp;
};

//FIXME: Refactor this out into a seperate file
struct VIS_HIDDEN RemoteBuffer {
    RemoteBuffer();
    RemoteBuffer(task_t task, mach_vm_address_t remote_address, size_t remote_size, bool shared, bool allow_truncation);
    RemoteBuffer& operator=(RemoteBuffer&& other);
    ~RemoteBuffer();
    void *getLocalAddress() const;
    kern_return_t getKernelReturn() const;
    size_t getSize() const;
private:
    static std::pair<mach_vm_address_t, kern_return_t> map( task_t task, mach_vm_address_t remote_address,
                                                            vm_size_t _size, bool shared);
    static std::tuple<mach_vm_address_t,vm_size_t,kern_return_t,bool>create(    task_t task,
                                                                                mach_vm_address_t remote_address,
                                                                                size_t remote_size,
                                                                                bool shared,
                                                                                bool allow_truncation);
    RemoteBuffer(std::tuple<mach_vm_address_t,vm_size_t,kern_return_t,bool> T);

    mach_vm_address_t _localAddress;
    vm_size_t _size;
    kern_return_t _kr;
    bool _shared;
};

// only called during libdyld set up
void setNotifyMonitoringDyldMain(void (*func)()) VIS_HIDDEN;
void setNotifyMonitoringDyld(void (*func)(bool unloading, unsigned imageCount,
                                          const struct mach_header* loadAddresses[],
                                          const char* imagePaths[])) VIS_HIDDEN;

void withRemoteBuffer(task_t task, mach_vm_address_t remote_address, size_t remote_size, bool shared, bool allow_truncation, kern_return_t *kr, void (^block)(void *buffer, size_t size)) __attribute__((visibility("hidden")));

template<typename T>
VIS_HIDDEN void withRemoteObject(task_t task, mach_vm_address_t remote_address, bool shared, kern_return_t *kr, void (^block)(T t))
{
    withRemoteBuffer(task, remote_address, sizeof(T), shared, false, kr, ^(void *buffer, size_t size) {
        block(*reinterpret_cast<T *>(buffer));
    });
}


#endif // _DYLD_PROCESS_INFO_INTERNAL_H_


