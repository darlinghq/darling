/*
 * Copyright (c) 1998-2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 * 
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

#ifndef __IOKIT_IOMAPPER_H
#define __IOKIT_IOMAPPER_H

#include <sys/cdefs.h>

__BEGIN_DECLS
#include <IOKit/IOTypes.h>
#include <mach/vm_types.h>

// These are C accessors to the system mapper for non-IOKit clients
ppnum_t IOMapperIOVMAlloc(unsigned pages);
void IOMapperIOVMFree(ppnum_t addr, unsigned pages);
ppnum_t IOMapperInsertPage(ppnum_t addr, unsigned offset, ppnum_t page);

__END_DECLS

#if __cplusplus

#include <IOKit/IOService.h>
#include <IOKit/IOMemoryDescriptor.h>
#include <IOKit/IODMACommand.h>

class OSData;

extern const OSSymbol * gIOMapperIDKey;

class IOMapper : public IOService
{
    OSDeclareAbstractStructors(IOMapper);

    // Give the platform expert access to setMapperRequired();
    friend class IOPlatformExpert;
    friend class IOMemoryDescriptor;

private:
    enum SystemMapperState {
        kNoMapper  = 0,
        kUnknown   = 1,
        kHasMapper = 2,	// Any other value is pointer to a live mapper
        kWaitMask  = 3,
    };
protected:
#ifdef XNU_KERNEL_PRIVATE
    uint64_t   __reservedA[7];
    uint32_t   __reservedB;
    uint32_t   fPageSize;
#else
    uint64_t __reserved[8];
#endif
    bool fIsSystem;

    static void setMapperRequired(bool hasMapper);
    static void waitForSystemMapper();

    virtual bool initHardware(IOService *provider) = 0;

public:
    virtual bool start(IOService *provider) APPLE_KEXT_OVERRIDE;
    virtual void free() APPLE_KEXT_OVERRIDE;

    // To get access to the system mapper IOMapper::gSystem 
    static IOMapper *gSystem;

    static void checkForSystemMapper()
        { if ((uintptr_t) gSystem & kWaitMask) waitForSystemMapper(); };

    static IOMapper * copyMapperForDevice(IOService * device);
    static IOMapper * copyMapperForDeviceWithIndex(IOService * device, unsigned int index);

    // { subclasses

    virtual uint64_t getPageSize(void) const = 0;

    virtual IOReturn iovmMapMemory(IOMemoryDescriptor          * memory,
				   uint64_t                      descriptorOffset,
				   uint64_t                      length,
				   uint32_t                      mapOptions,
				   const IODMAMapSpecification * mapSpecification,
				   IODMACommand                * dmaCommand,
				   const IODMAMapPageList      * pageList,
				   uint64_t                    * mapAddress,
				   uint64_t                    * mapLength) = 0;

    virtual IOReturn iovmUnmapMemory(IOMemoryDescriptor * memory, 
				     IODMACommand       * dmaCommand, 
				     uint64_t             mapAddress,
				     uint64_t             mapLength) = 0;

    virtual IOReturn iovmInsert(uint32_t options,
				uint64_t mapAddress,
				uint64_t offset, 
				uint64_t physicalAddress, 
				uint64_t length) = 0;

    virtual uint64_t mapToPhysicalAddress(uint64_t mappedAddress) = 0;

    // }

private:
    OSMetaClassDeclareReservedUnused(IOMapper, 0);
    OSMetaClassDeclareReservedUnused(IOMapper, 1);
    OSMetaClassDeclareReservedUnused(IOMapper, 2);
    OSMetaClassDeclareReservedUnused(IOMapper, 3);
    OSMetaClassDeclareReservedUnused(IOMapper, 4);
    OSMetaClassDeclareReservedUnused(IOMapper, 5);
    OSMetaClassDeclareReservedUnused(IOMapper, 6);
    OSMetaClassDeclareReservedUnused(IOMapper, 7);
    OSMetaClassDeclareReservedUnused(IOMapper, 8);
    OSMetaClassDeclareReservedUnused(IOMapper, 9);
    OSMetaClassDeclareReservedUnused(IOMapper, 10);
    OSMetaClassDeclareReservedUnused(IOMapper, 11);
    OSMetaClassDeclareReservedUnused(IOMapper, 12);
    OSMetaClassDeclareReservedUnused(IOMapper, 13);
    OSMetaClassDeclareReservedUnused(IOMapper, 14);
    OSMetaClassDeclareReservedUnused(IOMapper, 15);
};

#endif /* __cplusplus */

#endif /* !__IOKIT_IOMAPPER_H */
