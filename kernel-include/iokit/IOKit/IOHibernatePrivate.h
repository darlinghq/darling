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

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef KERNEL
#include <libkern/crypto/aes.h>
#include <uuid/uuid.h>
#include <kern/debug.h>

extern int kdb_printf(const char *format, ...) __printflike(1, 2);
#endif

#ifndef __IOKIT_IOHIBERNATEPRIVATE_H
#define __IOKIT_IOHIBERNATEPRIVATE_H

struct IOPolledFileExtent {
	uint64_t    start;
	uint64_t    length;
};
typedef struct IOPolledFileExtent IOPolledFileExtent;

struct IOHibernateImageHeader {
	uint64_t    imageSize;
	uint64_t    image1Size;

	uint32_t    restore1CodePhysPage;
	uint32_t    reserved1;
	uint64_t    restore1CodeVirt;
	uint32_t    restore1PageCount;
	uint32_t    restore1CodeOffset;
	uint32_t    restore1StackOffset;

	uint32_t    pageCount;
	uint32_t    bitmapSize;

	uint32_t    restore1Sum;
	uint32_t    image1Sum;
	uint32_t    image2Sum;

	uint32_t    actualRestore1Sum;
	uint32_t    actualImage1Sum;
	uint32_t    actualImage2Sum;

	uint32_t    actualUncompressedPages;
	uint32_t    conflictCount;
	uint32_t    nextFree;

	uint32_t    signature;
	uint32_t    processorFlags;

	uint32_t    runtimePages;
	uint32_t    runtimePageCount;
	uint64_t    runtimeVirtualPages __attribute__ ((packed));

	uint32_t    performanceDataStart;
	uint32_t    performanceDataSize;

	uint64_t    encryptStart __attribute__ ((packed));
	uint64_t    machineSignature __attribute__ ((packed));

	uint32_t    previewSize;
	uint32_t    previewPageListSize;

	uint32_t    diag[4];

	uint32_t    handoffPages;
	uint32_t    handoffPageCount;

	uint32_t    systemTableOffset;

	uint32_t    debugFlags;
	uint32_t    options;
	uint32_t    sleepTime;
	uint32_t    compression;

	uint8_t     bridgeBootSessionUUID[16];

	uint32_t    reserved[54];       // make sizeof == 512
	uint32_t    booterTime0;
	uint32_t    booterTime1;
	uint32_t    booterTime2;

	uint32_t    booterStart;
	uint32_t    smcStart;
	uint32_t    connectDisplayTime;
	uint32_t    splashTime;
	uint32_t    booterTime;
	uint32_t    trampolineTime;

	uint64_t    encryptEnd __attribute__ ((packed));
	uint64_t    deviceBase __attribute__ ((packed));
	uint32_t    deviceBlockSize;

	uint32_t            fileExtentMapSize;
	IOPolledFileExtent  fileExtentMap[2];
};
typedef struct IOHibernateImageHeader IOHibernateImageHeader;

enum{
	kIOHibernateDebugRestoreLogs = 0x00000001
};

// options & IOHibernateOptions property
enum{
	kIOHibernateOptionSSD           = 0x00000001,
	kIOHibernateOptionColor         = 0x00000002,
	kIOHibernateOptionProgress      = 0x00000004,
	kIOHibernateOptionDarkWake      = 0x00000008,
	kIOHibernateOptionHWEncrypt     = 0x00000010,
};

struct hibernate_bitmap_t {
	uint32_t    first_page;
	uint32_t    last_page;
	uint32_t    bitmapwords;
	uint32_t    bitmap[0];
};
typedef struct hibernate_bitmap_t hibernate_bitmap_t;

struct hibernate_page_list_t {
	uint32_t              list_size;
	uint32_t              page_count;
	uint32_t              bank_count;
	hibernate_bitmap_t    bank_bitmap[0];
};
typedef struct hibernate_page_list_t hibernate_page_list_t;

#if defined(_AES_H)

struct hibernate_cryptwakevars_t {
	uint8_t aes_iv[AES_BLOCK_SIZE];
};
typedef struct hibernate_cryptwakevars_t hibernate_cryptwakevars_t;

struct hibernate_cryptvars_t {
	uint8_t aes_iv[AES_BLOCK_SIZE];
	aes_ctx ctx;
};
typedef struct hibernate_cryptvars_t hibernate_cryptvars_t;

#endif /* defined(_AES_H) */

enum{
	kIOHibernateHandoffType                 = 0x686f0000,
	kIOHibernateHandoffTypeEnd              = kIOHibernateHandoffType + 0,
	kIOHibernateHandoffTypeGraphicsInfo     = kIOHibernateHandoffType + 1,
	kIOHibernateHandoffTypeCryptVars        = kIOHibernateHandoffType + 2,
	kIOHibernateHandoffTypeMemoryMap        = kIOHibernateHandoffType + 3,
	kIOHibernateHandoffTypeDeviceTree       = kIOHibernateHandoffType + 4,
	kIOHibernateHandoffTypeDeviceProperties = kIOHibernateHandoffType + 5,
	kIOHibernateHandoffTypeKeyStore         = kIOHibernateHandoffType + 6,
	kIOHibernateHandoffTypeVolumeCryptKey   = kIOHibernateHandoffType + 7,
};

struct IOHibernateHandoff {
	uint32_t type;
	uint32_t bytecount;
	uint8_t  data[];
};
typedef struct IOHibernateHandoff IOHibernateHandoff;

enum{
	kIOHibernateProgressCount         = 19,
	kIOHibernateProgressWidth         = 7,
	kIOHibernateProgressHeight        = 16,
	kIOHibernateProgressSpacing       = 3,
	kIOHibernateProgressOriginY       = 81,

	kIOHibernateProgressSaveUnderSize = 2 * 5 + 14 * 2,

	kIOHibernateProgressLightGray     = 230,
	kIOHibernateProgressMidGray       = 174,
	kIOHibernateProgressDarkGray      = 92
};

enum{
	kIOHibernatePostWriteSleep   = 0,
	kIOHibernatePostWriteWake    = 1,
	kIOHibernatePostWriteHalt    = 2,
	kIOHibernatePostWriteRestart = 3
};


struct hibernate_graphics_t {
	uint64_t physicalAddress; // Base address of video memory
	int32_t  gfxStatus;     // EFI config restore status
	uint32_t rowBytes;              // Number of bytes per pixel row
	uint32_t width;                 // Width
	uint32_t height;                // Height
	uint32_t depth;                 // Pixel Depth

	uint8_t progressSaveUnder[kIOHibernateProgressCount][kIOHibernateProgressSaveUnderSize];
};
typedef struct hibernate_graphics_t hibernate_graphics_t;

#define DECLARE_IOHIBERNATEPROGRESSALPHA                                \
static const uint8_t gIOHibernateProgressAlpha                  \
[kIOHibernateProgressHeight][kIOHibernateProgressWidth] =       \
{                                                               \
    { 0x00,0x63,0xd8,0xf0,0xd8,0x63,0x00 },                     \
    { 0x51,0xff,0xff,0xff,0xff,0xff,0x51 },                     \
    { 0xae,0xff,0xff,0xff,0xff,0xff,0xae },                     \
    { 0xc3,0xff,0xff,0xff,0xff,0xff,0xc3 },                     \
    { 0xc3,0xff,0xff,0xff,0xff,0xff,0xc3 },                     \
    { 0xc3,0xff,0xff,0xff,0xff,0xff,0xc3 },                     \
    { 0xc3,0xff,0xff,0xff,0xff,0xff,0xc3 },                     \
    { 0xc3,0xff,0xff,0xff,0xff,0xff,0xc3 },                     \
    { 0xc3,0xff,0xff,0xff,0xff,0xff,0xc3 },                     \
    { 0xc3,0xff,0xff,0xff,0xff,0xff,0xc3 },                     \
    { 0xc3,0xff,0xff,0xff,0xff,0xff,0xc3 },                     \
    { 0xc3,0xff,0xff,0xff,0xff,0xff,0xc3 },                     \
    { 0xc3,0xff,0xff,0xff,0xff,0xff,0xc3 },                     \
    { 0xae,0xff,0xff,0xff,0xff,0xff,0xae },                     \
    { 0x54,0xff,0xff,0xff,0xff,0xff,0x54 },                     \
    { 0x00,0x66,0xdb,0xf3,0xdb,0x66,0x00 }                      \
};

struct hibernate_preview_t {
	uint32_t  imageCount;   // Number of images
	uint32_t  width;        // Width
	uint32_t  height;       // Height
	uint32_t  depth;        // Pixel Depth
	uint32_t  lockTime; // Lock time
	uint32_t  reservedG[8];// reserved
	uint32_t  reservedK[8];// reserved
};
typedef struct hibernate_preview_t hibernate_preview_t;

struct hibernate_statistics_t {
	uint64_t image1Size;
	uint64_t imageSize;
	uint32_t image1Pages;
	uint32_t imagePages;
	uint32_t booterStart;
	uint32_t smcStart;
	uint32_t booterDuration;
	uint32_t booterConnectDisplayDuration;
	uint32_t booterSplashDuration;
	uint32_t booterDuration0;
	uint32_t booterDuration1;
	uint32_t booterDuration2;
	uint32_t trampolineDuration;
	uint32_t kernelImageReadDuration;

	uint32_t graphicsReadyTime;
	uint32_t wakeNotificationTime;
	uint32_t lockScreenReadyTime;
	uint32_t hidReadyTime;

	uint32_t wakeCapability;
	uint32_t resvA[15];
};
typedef struct hibernate_statistics_t hibernate_statistics_t;

#define kIOSysctlHibernateStatistics    "kern.hibernatestatistics"
#define kIOSysctlHibernateGraphicsReady "kern.hibernategraphicsready"
#define kIOSysctlHibernateWakeNotify    "kern.hibernatewakenotification"
#define kIOSysctlHibernateScreenReady   "kern.hibernatelockscreenready"
#define kIOSysctlHibernateHIDReady      "kern.hibernatehidready"

#ifdef KERNEL

#ifdef __cplusplus

void     IOHibernateSystemInit(IOPMrootDomain * rootDomain);

IOReturn IOHibernateSystemSleep(void);
IOReturn IOHibernateIOKitSleep(void);
IOReturn IOHibernateSystemHasSlept(void);
IOReturn IOHibernateSystemWake(void);
IOReturn IOHibernateSystemPostWake(bool now);
uint32_t IOHibernateWasScreenLocked(void);
void     IOHibernateSetScreenLocked(uint32_t lockState);
void     IOHibernateSetWakeCapabilities(uint32_t capability);
void     IOHibernateSystemRestart(void);

#endif /* __cplusplus */

void
vm_compressor_do_warmup(void);


hibernate_page_list_t *
hibernate_page_list_allocate(boolean_t log);

kern_return_t
hibernate_alloc_page_lists(
	hibernate_page_list_t ** page_list_ret,
	hibernate_page_list_t ** page_list_wired_ret,
	hibernate_page_list_t ** page_list_pal_ret);

kern_return_t
hibernate_setup(IOHibernateImageHeader * header,
    boolean_t vmflush,
    hibernate_page_list_t * page_list,
    hibernate_page_list_t * page_list_wired,
    hibernate_page_list_t * page_list_pal);

kern_return_t
hibernate_teardown(hibernate_page_list_t * page_list,
    hibernate_page_list_t * page_list_wired,
    hibernate_page_list_t * page_list_pal);

kern_return_t
hibernate_pin_swap(boolean_t begin);

kern_return_t
hibernate_processor_setup(IOHibernateImageHeader * header);

void
hibernate_gobble_pages(uint32_t gobble_count, uint32_t free_page_time);
void
hibernate_free_gobble_pages(void);

void
hibernate_vm_lock_queues(void);
void
hibernate_vm_unlock_queues(void);

void
hibernate_vm_lock(void);
void
hibernate_vm_unlock(void);
void
hibernate_vm_lock_end(void);
boolean_t
hibernate_vm_locks_are_safe(void);

// mark pages not to be saved, based on VM system accounting
void
hibernate_page_list_setall(hibernate_page_list_t * page_list,
    hibernate_page_list_t * page_list_wired,
    hibernate_page_list_t * page_list_pal,
    boolean_t preflight,
    boolean_t discard_all,
    uint32_t * pagesOut);

// mark pages to be saved, or pages not to be saved but available
// for scratch usage during restore
void
hibernate_page_list_setall_machine(hibernate_page_list_t * page_list,
    hibernate_page_list_t * page_list_wired,
    boolean_t preflight,
    uint32_t * pagesOut);

// mark pages not to be saved and not for scratch usage during restore
void
hibernate_page_list_set_volatile( hibernate_page_list_t * page_list,
    hibernate_page_list_t * page_list_wired,
    uint32_t * pagesOut);

void
hibernate_page_list_discard(hibernate_page_list_t * page_list);

int
hibernate_should_abort(void);

void
hibernate_set_page_state(hibernate_page_list_t * page_list, hibernate_page_list_t * page_list_wired,
    vm_offset_t ppnum, vm_offset_t count, uint32_t kind);

void
hibernate_page_bitset(hibernate_page_list_t * list, boolean_t set, uint32_t page);

boolean_t
hibernate_page_bittst(hibernate_page_list_t * list, uint32_t page);

hibernate_bitmap_t *
hibernate_page_bitmap_pin(hibernate_page_list_t * list, uint32_t * page);

uint32_t
hibernate_page_bitmap_count(hibernate_bitmap_t * bitmap, uint32_t set, uint32_t page);

uintptr_t
hibernate_restore_phys_page(uint64_t src, uint64_t dst, uint32_t len, uint32_t procFlags);

void
hibernate_machine_init(void);

uint32_t
hibernate_write_image(void);

long
hibernate_machine_entrypoint(uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4);
long
hibernate_kernel_entrypoint(uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4);
void
hibernate_newruntime_map(void * map, vm_size_t map_size,
    uint32_t system_table_offset);


extern uint32_t    gIOHibernateState;
extern uint32_t    gIOHibernateMode;
extern uint32_t    gIOHibernateDebugFlags;
extern uint32_t    gIOHibernateFreeTime;        // max time to spend freeing pages (ms)
extern boolean_t   gIOHibernateStandbyDisabled;
extern uint8_t     gIOHibernateRestoreStack[];
extern uint8_t     gIOHibernateRestoreStackEnd[];
extern IOHibernateImageHeader *    gIOHibernateCurrentHeader;

#define HIBLOGFROMPANIC(fmt, args...) \
    { if (kernel_debugger_entry_count) { kdb_printf(fmt, ## args); } }

#define HIBLOG(fmt, args...)    \
    { if (kernel_debugger_entry_count) { kdb_printf(fmt, ## args); } else { kprintf(fmt, ## args); printf(fmt, ## args); } }

#define HIBPRINT(fmt, args...)  \
    { if (kernel_debugger_entry_count) { kdb_printf(fmt, ## args); } else { kprintf(fmt, ## args); } }


#endif /* KERNEL */

// gIOHibernateState, kIOHibernateStateKey
enum{
	kIOHibernateStateInactive            = 0,
	kIOHibernateStateHibernating         = 1,/* writing image */
	kIOHibernateStateWakingFromHibernate = 2 /* booted and restored image */
};

// gIOHibernateMode, kIOHibernateModeKey
enum{
	kIOHibernateModeOn      = 0x00000001,
	kIOHibernateModeSleep   = 0x00000002,
	kIOHibernateModeEncrypt = 0x00000004,
	kIOHibernateModeDiscardCleanInactive = 0x00000008,
	kIOHibernateModeDiscardCleanActive   = 0x00000010,
	kIOHibernateModeSwitch      = 0x00000020,
	kIOHibernateModeRestart     = 0x00000040,
	kIOHibernateModeSSDInvert   = 0x00000080,
	kIOHibernateModeFileResize  = 0x00000100,
};

// IOHibernateImageHeader.signature
enum{
	kIOHibernateHeaderSignature        = 0x73696d65,
	kIOHibernateHeaderInvalidSignature = 0x7a7a7a7a,
	kIOHibernateHeaderOpenSignature    = 0xf1e0be9d,
	kIOHibernateHeaderDebugDataSignature = 0xfcddfcdd
};

// kind for hibernate_set_page_state()
enum{
	kIOHibernatePageStateFree        = 0,
	kIOHibernatePageStateWiredSave   = 1,
	kIOHibernatePageStateUnwiredSave = 2
};

#define kIOHibernateModeKey             "Hibernate Mode"
#define kIOHibernateFileKey             "Hibernate File"
#define kIOHibernateFileMinSizeKey      "Hibernate File Min"
#define kIOHibernateFileMaxSizeKey      "Hibernate File Max"
#define kIOHibernateFreeRatioKey        "Hibernate Free Ratio"
#define kIOHibernateFreeTimeKey         "Hibernate Free Time"

#define kIOHibernateStateKey            "IOHibernateState"
#define kIOHibernateFeatureKey          "Hibernation"
#define kIOHibernatePreviewBufferKey    "IOPreviewBuffer"

#ifndef kIOHibernatePreviewActiveKey
#define kIOHibernatePreviewActiveKey    "IOHibernatePreviewActive"
// values for kIOHibernatePreviewActiveKey
enum {
	kIOHibernatePreviewActive  = 0x00000001,
	kIOHibernatePreviewUpdates = 0x00000002
};
#endif

#define kIOHibernateOptionsKey      "IOHibernateOptions"
#define kIOHibernateGfxStatusKey    "IOHibernateGfxStatus"
enum {
	kIOHibernateGfxStatusUnknown = ((int32_t) 0xFFFFFFFF)
};

#define kIOHibernateBootImageKey        "boot-image"
#define kIOHibernateBootImageKeyKey     "boot-image-key"
#define kIOHibernateBootSignatureKey    "boot-signature"

#define kIOHibernateMemorySignatureKey    "memory-signature"
#define kIOHibernateMemorySignatureEnvKey "mem-sig"
#define kIOHibernateMachineSignatureKey   "machine-signature"

#define kIOHibernateRTCVariablesKey     "IOHibernateRTCVariables"
#define kIOHibernateSMCVariablesKey     "IOHibernateSMCVariables"

#define kIOHibernateBootSwitchVarsKey   "boot-switch-vars"

#define kIOHibernateBootNoteKey         "boot-note"


#define kIOHibernateUseKernelInterpreter    0x80000000

enum{
	kIOPreviewImageIndexDesktop = 0,
	kIOPreviewImageIndexLockScreen = 1,
	kIOPreviewImageCount = 2
};

enum{
	kIOScreenLockNoLock          = 1,
	kIOScreenLockUnlocked        = 2,
	kIOScreenLockLocked          = 3,
	kIOScreenLockFileVaultDialog = 4,
};

#define kIOScreenLockStateKey       "IOScreenLockState"
#define kIOBooterScreenLockStateKey "IOBooterScreenLockState"

#endif /* ! __IOKIT_IOHIBERNATEPRIVATE_H */

#ifdef __cplusplus
}
#endif
