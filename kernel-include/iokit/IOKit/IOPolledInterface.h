/*
 * Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
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

#ifndef _IOPOLLEDINTERFACE_H_
#define _IOPOLLEDINTERFACE_H_

enum
{
    kIOPolledPreflightState           = 1,
    kIOPolledBeforeSleepState         = 2,
    kIOPolledAfterSleepState          = 3,
    kIOPolledPostflightState          = 4,

    kIOPolledPreflightCoreDumpState   = 5,
    kIOPolledPostflightCoreDumpState  = 6,

    kIOPolledBeforeSleepStateAborted  = 7,
};

#if defined(__cplusplus)

#include <libkern/c++/OSObject.h>
#include <IOKit/IOMemoryDescriptor.h>

#define kIOPolledInterfaceSupportKey "IOPolledInterface"
#define kIOPolledInterfaceActiveKey  "IOPolledInterfaceActive"
#define kIOPolledInterfaceStackKey   "IOPolledInterfaceStack"

enum
{
    kIOPolledWrite = 1,
    kIOPolledRead  = 2
};

typedef void (*IOPolledCompletionAction)( void *   target,
                                          void *   parameter,
                                          IOReturn status,
                                          uint64_t actualByteCount);
struct IOPolledCompletion
{
    void *                    target;
    IOPolledCompletionAction  action;
    void *                    parameter;
};

class IOPolledInterface : public OSObject
{
    OSDeclareAbstractStructors(IOPolledInterface);

protected:
    struct ExpansionData { };
    ExpansionData * reserved;

public:
    virtual IOReturn probe(IOService * target) = 0;

    virtual IOReturn open( IOOptionBits state, IOMemoryDescriptor * buffer) = 0;
    virtual IOReturn close(IOOptionBits state) = 0;

    virtual IOReturn startIO(uint32_t 	        operation,
                             uint32_t           bufferOffset,
                             uint64_t	        deviceOffset,
                             uint64_t	        length,
                             IOPolledCompletion completion) = 0;

    virtual IOReturn checkForWork(void) = 0;

    OSMetaClassDeclareReservedUnused(IOPolledInterface, 0);
    OSMetaClassDeclareReservedUnused(IOPolledInterface, 1);
    OSMetaClassDeclareReservedUnused(IOPolledInterface, 2);
    OSMetaClassDeclareReservedUnused(IOPolledInterface, 3);
    OSMetaClassDeclareReservedUnused(IOPolledInterface, 4);
    OSMetaClassDeclareReservedUnused(IOPolledInterface, 5);
    OSMetaClassDeclareReservedUnused(IOPolledInterface, 6);
    OSMetaClassDeclareReservedUnused(IOPolledInterface, 7);
    OSMetaClassDeclareReservedUnused(IOPolledInterface, 8);
    OSMetaClassDeclareReservedUnused(IOPolledInterface, 9);
    OSMetaClassDeclareReservedUnused(IOPolledInterface, 10);
    OSMetaClassDeclareReservedUnused(IOPolledInterface, 11);
    OSMetaClassDeclareReservedUnused(IOPolledInterface, 12);
    OSMetaClassDeclareReservedUnused(IOPolledInterface, 13);
    OSMetaClassDeclareReservedUnused(IOPolledInterface, 14);
    OSMetaClassDeclareReservedUnused(IOPolledInterface, 15);
};

#endif /* defined(__cplusplus) */

#ifdef XNU_KERNEL_PRIVATE

#include <libkern/crypto/aes.h>
#include <IOKit/IOTypes.h>
#include <IOKit/IOHibernatePrivate.h>

enum
{
    kIOPolledFileSSD = 0x00000001
};

#if !defined(__cplusplus)
typedef struct IORegistryEntry IORegistryEntry;
typedef struct OSData OSData;
typedef struct OSArray OSArray;
typedef struct IOMemoryDescriptor IOMemoryDescriptor;
typedef struct IOPolledFilePollers IOPolledFilePollers;
#else
class IOPolledFilePollers;
#endif

struct IOPolledFileIOVars
{
    IOPolledFilePollers              *  pollers;
    struct kern_direct_file_io_ref_t *	fileRef;
    OSData *		 		fileExtents;
    uint64_t				block0;
    IOByteCount				blockSize;
    uint64_t			        maxiobytes;
    IOByteCount 			bufferLimit;
    uint8_t *  				buffer;
    IOByteCount 			bufferSize;
    IOByteCount 			bufferOffset;
    IOByteCount 			bufferHalf;
    IOByteCount				extentRemaining;
    IOByteCount				lastRead;
    IOByteCount				readEnd;
    uint32_t                            flags;
    uint64_t				fileSize;
    uint64_t				position;
    uint64_t				extentPosition;
    uint64_t				encryptStart;
    uint64_t				encryptEnd;
    uint64_t                            cryptBytes;
    AbsoluteTime                        cryptTime;
    IOPolledFileExtent * 		extentMap;
    IOPolledFileExtent * 		currentExtent;
    bool				allocated;
};

typedef struct IOPolledFileIOVars IOPolledFileIOVars;

struct IOPolledFileCryptVars
{
    uint8_t aes_iv[AES_BLOCK_SIZE];
    aes_ctx ctx;
};
typedef struct IOPolledFileCryptVars IOPolledFileCryptVars;

#if defined(__cplusplus)

IOReturn IOPolledFileOpen(const char * filename, 
			  uint64_t setFileSize, uint64_t fsFreeSize,
			  void * write_file_addr, size_t write_file_len,
			  IOPolledFileIOVars ** fileVars,
			  OSData ** imagePath,
			  uint8_t * volumeCryptKey, size_t keySize);

IOReturn IOPolledFileClose(IOPolledFileIOVars ** pVars,
			   off_t write_offset, void * addr, size_t write_length,
			   off_t discard_offset, off_t discard_end);

IOReturn IOPolledFilePollersSetup(IOPolledFileIOVars * vars, uint32_t openState);

IOMemoryDescriptor * IOPolledFileGetIOBuffer(IOPolledFileIOVars * vars);

#endif /* defined(__cplusplus) */

#if defined(__cplusplus)
#define __C	"C"
#else
#define __C
#endif

extern __C IOReturn IOPolledFileSeek(IOPolledFileIOVars * vars, uint64_t position);

extern __C IOReturn IOPolledFileWrite(IOPolledFileIOVars * vars,
			   const uint8_t * bytes, IOByteCount size,
			   IOPolledFileCryptVars * cryptvars);
extern __C IOReturn IOPolledFileRead(IOPolledFileIOVars * vars,
			  uint8_t * bytes, IOByteCount size,
			  IOPolledFileCryptVars * cryptvars);

extern __C IOReturn IOPolledFilePollersOpen(IOPolledFileIOVars * vars, uint32_t state, bool abortable);

extern __C IOReturn IOPolledFilePollersClose(IOPolledFileIOVars * vars, uint32_t state);

extern __C IOPolledFileIOVars * gCoreFileVars;

#ifdef _SYS_CONF_H_

__BEGIN_DECLS

typedef void (*kern_get_file_extents_callback_t)(void * ref, uint64_t start, uint64_t size);

struct kern_direct_file_io_ref_t *
kern_open_file_for_direct_io(const char * name, boolean_t create_file,
			     kern_get_file_extents_callback_t callback, 
			     void * callback_ref,
                             off_t set_file_size,
                             off_t fs_free_size,
                             off_t write_file_offset,
                             void * write_file_addr,
                             size_t write_file_len,
			     dev_t * partition_device_result,
			     dev_t * image_device_result,
                             uint64_t * partitionbase_result,
                             uint64_t * maxiocount_result,
                             uint32_t * oflags);
void
kern_close_file_for_direct_io(struct kern_direct_file_io_ref_t * ref,
			      off_t write_offset, void * addr, size_t write_length,
			      off_t discard_offset, off_t discard_end);
int
kern_write_file(struct kern_direct_file_io_ref_t * ref, off_t offset, void * addr, size_t len, int ioflag);
int
kern_read_file(struct kern_direct_file_io_ref_t * ref, off_t offset, void * addr, size_t len, int ioflag);

struct mount *
kern_file_mount(struct kern_direct_file_io_ref_t * ref);

enum 
{
    kIOPolledFileMountChangeMount = 0x00000101,
    kIOPolledFileMountChangeUnmount = 0x00000102,
    kIOPolledFileMountChangeWillResize = 0x00000201,
    kIOPolledFileMountChangeDidResize = 0x00000202,
};
extern void IOPolledFileMountChange(struct mount * mp, uint32_t op);

__END_DECLS

#endif /* _SYS_CONF_H_ */

#endif /* XNU_KERNEL_PRIVATE */

#endif /* _IOPOLLEDINTERFACE_H_ */
