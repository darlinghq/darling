/*
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
#ifndef _DYLD_PROCESS_INFO_
#define _DYLD_PROCESS_INFO_

#include <stdbool.h>
#include <unistd.h>
#include <mach/mach.h>
#include <dispatch/dispatch.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// Beginning in iOS 10.0 and Mac OS X 10.12, this is how lldb figures out mach-o binaries are in a process:
//
// When attaching to an existing process, lldb uses _dyld_process_info_create() to get the current list of images
// in a process, then falls into the start up case.
//
// When starting a process, lldb starts the process suspended, finds the "_dyld_debugger_notification" symbol in
// dyld, sets a break point on it, then resumes the process.  Dyld will call _dyld_debugger_notification() with
// a list of images that were just added or removed from the process.  Dyld calls this function before running
// any initializers in the image, so the debugger will have a chance to set break points in the image.
//

enum dyld_notify_mode { dyld_notify_adding=0, dyld_notify_removing=1, dyld_notify_remove_all=2 };

void _dyld_debugger_notification(enum dyld_notify_mode, unsigned long count, uint64_t machHeaders[]);


struct dyld_process_cache_info {
    uuid_t      cacheUUID;              // UUID of cache used by process
    uint64_t    cacheBaseAddress;       // load address of dyld shared cache
    bool        noCache;                // process is running without a dyld cache
    bool        privateCache;           // process is using a private copy of its dyld cache
};
typedef struct dyld_process_cache_info dyld_process_cache_info;

enum {
    dyld_process_state_not_started              = 0x00,     // process is suspended, dyld has not started running yet
    dyld_process_state_dyld_initialized         = 0x10,     // dyld has initialzed itself
    dyld_process_state_terminated_before_inits  = 0x20,     // process was terminated due missing library or symbol before it got to main()
    dyld_process_state_libSystem_initialized    = 0x30,     // dyld has run libSystem's initializer
    dyld_process_state_running_initializers     = 0x40,     // dyld is running other initializers
    dyld_process_state_program_running          = 0x50,     // dyld has finished and jumped into main()
    dyld_process_state_dyld_terminated          = 0x60      // process was terminated by dyld post-main (e.g. bad lazying binding info)
};

struct dyld_process_state_info {
    uint64_t    timestamp;              // mach_absolute_time of last time dyld change to image list
    uint32_t    imageCount;             // number of images currently loaded into process
    uint32_t    initialImageCount;      // number of images statically loaded into process (before any dlopen() calls)
    uint8_t     dyldState;              // one of dyld_process_state_* values
};
typedef struct dyld_process_state_info dyld_process_state_info;


typedef const struct dyld_process_info_base* dyld_process_info;

//
// Generate a dyld_process_info object for specified task.
//
// The timestamp parameter is an optimization to not spend the time to gather all the image information
// if the process image list has not changed since the last call.  If timestamp is zero, this function
// always gathers the full process info.  If timestamp is non-zero, this function will check if the target
// task's image list has changed since that time.  If is has not changed, the function returns NULL and
// kern_return_t is KERN_SUCCESS.  If it has changed, the function gathers the full image info.
// The kernelError parameter can be NULL for clients that don't care why it failed.
//
extern dyld_process_info _dyld_process_info_create(task_t task, uint64_t timestamp, kern_return_t* kernelError);

// retain/release dyld_process_info for specified task
extern void  _dyld_process_info_release(dyld_process_info info);
extern void  _dyld_process_info_retain(dyld_process_info info);

// fill in struct with basic info about dyld in the process
extern void  _dyld_process_info_get_state(dyld_process_info info, dyld_process_state_info* stateInfo);

// fill in struct with info about dyld cache in use by process
extern void  _dyld_process_info_get_cache(dyld_process_info info, dyld_process_cache_info* cacheInfo);

// iterate all images in process
extern void _dyld_process_info_for_each_image(dyld_process_info info, void (^callback)(uint64_t machHeaderAddress, const uuid_t uuid, const char* path));

// iterate all segments in an image
extern void _dyld_process_info_for_each_segment(dyld_process_info info, uint64_t machHeaderAddress, void (^callback)(uint64_t segmentAddress, uint64_t segmentSize, const char* segmentName));




typedef const struct dyld_process_info_notify_base* dyld_process_info_notify;

//
// Request notifications if image list changes in target process.  Each time a load or unload happens in the target taks,
// the notify block will be called in this process.  If the process exits, the notifyExit block will be called.
// If the notifications cannot be set up, this function will return NULL, and the reason in the kernError parameter.
// The kernelError parameter can be NULL for clients that don't care why it failed.
// If you want to stop receiving notifications, call _dyld_process_info_notify_release().
//
extern dyld_process_info_notify _dyld_process_info_notify(task_t task, dispatch_queue_t queue,
                                                          void (^notify)(bool unload, uint64_t timestamp, uint64_t machHeader, const uuid_t uuid, const char* path),
                                                          void (^notifyExit)(),
                                                          kern_return_t* kernelError);

// stop notifications and invalid dyld_process_info_notify object
extern void  _dyld_process_info_notify_release(dyld_process_info_notify object);
extern void  _dyld_process_info_notify_retain(dyld_process_info_notify object);


#ifdef __cplusplus
}
#endif

#endif /* _DYLD_PROCESS_INFO_ */
