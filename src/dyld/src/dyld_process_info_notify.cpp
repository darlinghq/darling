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
#include <mach/shared_region.h>
#include <mach/mach_vm.h>
#include <libkern/OSAtomic.h>
#include <execinfo.h>
#include <mach-o/dyld_priv.h>
#include <mach-o/dyld_process_info.h>
#include <mach-o/dyld_images.h>


#include "dyld_process_info_internal.h"

#include "Loading.h"
#include "AllImages.h"

extern "C" int _dyld_func_lookup(const char* name, void** address);

typedef void (^Notify)(bool unload, uint64_t timestamp, uint64_t machHeader, const uuid_t uuid, const char* path);
typedef void (^NotifyExit)();
typedef void (^NotifyMain)();

//
// Object used for monitoring another processes dyld loads
//
struct __attribute__((visibility("hidden"))) dyld_process_info_notify_base
{
                        dyld_process_info_notify_base(dispatch_queue_t queue, Notify notify, NotifyExit notifyExit, task_t task, kern_return_t* kr);
                        ~dyld_process_info_notify_base();
    bool                enabled() const;
    void                retain();
    void                release();

	void				setNotifyMain(NotifyMain notifyMain) const { _notifyMain = notifyMain; }

    // override new and delete so we don't need to link with libc++
    static void*        operator new(size_t sz) { return malloc(sz); }
    static void         operator delete(void* p) { free(p); }

private:
    void                handleEvent();
    void                teardown();
    void                replyToMonitoredProcess(mach_msg_header_t& header);

    RemoteBuffer                    _remoteAllImageInfoBuffer;
    uint32_t*                       _notifyMachPorts;
    uint32_t                        _notifySlot;
    mutable std::atomic<int32_t>    _retainCount;
    dispatch_queue_t                _queue;
    Notify                          _notify;
    NotifyExit                      _notifyExit;
	mutable NotifyMain              _notifyMain;
	task_t                          _targetTask;
	dispatch_source_t               _machSource;
    mach_port_t                     _sendPortInTarget;          // target is process being watched for image loading/unloading
    mach_port_t                     _receivePortInMonitor;      // monitor is process being notified of image loading/unloading
    std::atomic<bool>               _disabled;
};


dyld_process_info_notify_base::dyld_process_info_notify_base(dispatch_queue_t queue, Notify notify, NotifyExit notifyExit,
                                                             task_t task, kern_return_t* kr) :
        _notifyMachPorts(nullptr), _notifySlot(0), _retainCount(1), _queue(queue), _notify(notify), _notifyExit(notifyExit),
        _notifyMain(nullptr), _targetTask(task), _machSource(nullptr), _sendPortInTarget(0), _receivePortInMonitor(0),
        _disabled(false)
{
    assert(_disabled == false);
    dispatch_retain(_queue);
    // Allocate a port to listen on in this monitoring task
    mach_port_options_t options = { .flags = MPO_IMPORTANCE_RECEIVER | MPO_CONTEXT_AS_GUARD | MPO_STRICT,
        .mpl = { MACH_PORT_QLIMIT_DEFAULT }};
    if ((*kr = mach_port_construct(mach_task_self(), &options, (mach_port_context_t)this, &_receivePortInMonitor))) {
        teardown();
        return;
    }
    if (_targetTask == mach_task_self()) {
        _sendPortInTarget = _receivePortInMonitor;
        (void)mach_port_insert_right(_targetTask, _sendPortInTarget, _receivePortInMonitor, MACH_MSG_TYPE_MAKE_SEND);
    } else {
        // Insert a deadname right into the port to trigger notifications
        kern_return_t r = KERN_NAME_EXISTS;
        while (r == KERN_NAME_EXISTS) {
            _sendPortInTarget = MACH_PORT_NULL;
            //FIXME file radar
            r = mach_port_allocate(_targetTask, MACH_PORT_RIGHT_DEAD_NAME, &_sendPortInTarget);
            if (r != KERN_SUCCESS) {
                *kr = r;
                return;
            }
            (void)mach_port_deallocate(_targetTask, _sendPortInTarget);
            r = mach_port_insert_right(_targetTask, _sendPortInTarget, _receivePortInMonitor, MACH_MSG_TYPE_MAKE_SEND);
        }
        if (r != KERN_SUCCESS) {
            *kr = r;
            return;
        }

        // Notify us if the target dies
        mach_port_t previous = MACH_PORT_NULL;
        if ((*kr = mach_port_request_notification(_targetTask, _sendPortInTarget, MACH_NOTIFY_DEAD_NAME, 0, _receivePortInMonitor, MACH_MSG_TYPE_MAKE_SEND_ONCE, &previous))) {
            (void)mach_port_deallocate(_targetTask, _sendPortInTarget);
            (void)mach_port_destruct(mach_task_self(), _receivePortInMonitor, 0, (mach_port_context_t)this);
            teardown();
            return;
        }
        // This is a new port, if there is a previous notifier attached then something is wrong... abort.
        if (previous != MACH_PORT_NULL) {
            (void)mach_port_deallocate(mach_task_self(), previous);
            (void)mach_port_deallocate(_targetTask, _sendPortInTarget);
            (void)mach_port_destruct(mach_task_self(), _receivePortInMonitor, 0, (mach_port_context_t)this);
            teardown();
            return;
        }
    }

    // Setup the event handler for the port
    _machSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_MACH_RECV, _receivePortInMonitor, 0, _queue);
    if (_machSource == nullptr) {
        (void)mach_port_deallocate(_targetTask, _sendPortInTarget);
        (void)mach_port_destruct(mach_task_self(), _receivePortInMonitor, 0, (mach_port_context_t)this);
        teardown();
        return;
    }
    dispatch_source_set_event_handler(_machSource, ^{
        handleEvent();
    });
    dispatch_source_set_cancel_handler(_machSource, ^{
        if ( _receivePortInMonitor != 0 ) {
            (void)mach_port_destruct(mach_task_self(), _receivePortInMonitor, 0, (mach_port_context_t)this);
            _receivePortInMonitor = 0;
        }
    });
    dispatch_activate(_machSource);

    // get location on all_image_infos in the target task
    task_dyld_info_data_t taskDyldInfo;
    mach_msg_type_number_t taskDyldInfoCount = TASK_DYLD_INFO_COUNT;
    if ((*kr = task_info(_targetTask, TASK_DYLD_INFO, (task_info_t)&taskDyldInfo, &taskDyldInfoCount))) {
        (void)mach_port_deallocate(_targetTask, _sendPortInTarget);
        teardown();
        return;
    }
    // Poke the portname of our port into the target task
    _remoteAllImageInfoBuffer = RemoteBuffer(_targetTask, taskDyldInfo.all_image_info_addr, (size_t)taskDyldInfo.all_image_info_size, true, false);
    *kr = _remoteAllImageInfoBuffer.getKernelReturn();
    if (*kr) {
        (void)mach_port_deallocate(_targetTask, _sendPortInTarget);
        teardown();
        return;
    }

    static_assert(sizeof(mach_port_t) == sizeof(uint32_t), "machport size not 32-bits");
    if ( taskDyldInfo.all_image_info_format == TASK_DYLD_ALL_IMAGE_INFO_32 ) {
        _notifyMachPorts = (uint32_t *)((uint8_t *)_remoteAllImageInfoBuffer.getLocalAddress() + offsetof(dyld_all_image_infos_32,notifyMachPorts));
    } else {
        _notifyMachPorts = (uint32_t *)((uint8_t *)_remoteAllImageInfoBuffer.getLocalAddress() + offsetof(dyld_all_image_infos_64,notifyMachPorts));
    }

#if 0
    //If all the slots are filled we will sleep and retry a few times before giving up
    for (uint32_t i=0; i<10; ++i) {
        for (_notifySlot=0; _notifySlot < DYLD_MAX_PROCESS_INFO_NOTIFY_COUNT; ++_notifySlot) {
            if (OSAtomicCompareAndSwap32(0, _sendPortInTarget, (volatile int32_t*)&_notifyMachPorts[_notifySlot])) {
                break;
            }
        }
        if (_notifySlot == DYLD_MAX_PROCESS_INFO_NOTIFY_COUNT) {
            // all the slots are filled, sleep and try again
            usleep(1000 * 50); // 50ms
        } else {
            // if _notifySlot is set we are done
            break;
        }
    }
#else
    for (_notifySlot=0; _notifySlot < DYLD_MAX_PROCESS_INFO_NOTIFY_COUNT; ++_notifySlot) {
        if (OSAtomicCompareAndSwap32(0, _sendPortInTarget, (volatile int32_t*)&_notifyMachPorts[_notifySlot])) {
            break;
        }
    }
#endif

    if (_notifySlot == DYLD_MAX_PROCESS_INFO_NOTIFY_COUNT) {
        (void)mach_port_deallocate(_targetTask, _sendPortInTarget);
        teardown();
        *kr = KERN_UREFS_OVERFLOW;
        return;
    }

    *kr = KERN_SUCCESS;
}

dyld_process_info_notify_base::~dyld_process_info_notify_base() {
    if (!_disabled) {
        fprintf(stderr, "dyld: ~dyld_process_info_notify_base called while still enabled\n");
    }
    dispatch_release(_queue);
}

void dyld_process_info_notify_base::teardown() {
    if (!_disabled) {
        _disabled = true;
        // The connection to the target is dead.  Clean up ports
        if ( _remoteAllImageInfoBuffer.getLocalAddress() != 0 && _notifySlot < DYLD_MAX_PROCESS_INFO_NOTIFY_COUNT) {
            mach_port_t extractedPort = MACH_PORT_NULL;
            mach_msg_type_name_t extractedPortType;
            kern_return_t kr = mach_port_extract_right(_targetTask, _sendPortInTarget, MACH_MSG_TYPE_COPY_SEND, &extractedPort, &extractedPortType);
            if (kr == KERN_SUCCESS) {
                if (extractedPort == _receivePortInMonitor) {
                    if (OSAtomicCompareAndSwap32(_sendPortInTarget, 0, (volatile int32_t*)&_notifyMachPorts[_notifySlot])) {
                        (void)mach_port_deallocate(_targetTask, _sendPortInTarget);
                    }
                }
                (void)mach_port_deallocate(mach_task_self(), extractedPort);
            }
        }
        _sendPortInTarget = 0;
        if ( _machSource ) {
            dispatch_source_cancel(_machSource);
            dispatch_release(_machSource);
            _machSource = NULL;
        }
        if (_notifyExit) {
            dispatch_async(_queue, ^{
                _notifyExit();
            });
        }
    }
}

bool dyld_process_info_notify_base::enabled() const
{
    return !_disabled;
}

void dyld_process_info_notify_base::retain()
{
    _retainCount++;
}

void dyld_process_info_notify_base::release()
{
    uint32_t newCount = --_retainCount;
    
    if ( newCount == 0 ) {
        teardown();
    }
    dispatch_async(_queue, ^{
        delete this;
    });
}

void dyld_process_info_notify_base::replyToMonitoredProcess(mach_msg_header_t& header) {
    mach_msg_header_t replyHeader;
    replyHeader.msgh_bits        = MACH_MSGH_BITS_SET(MACH_MSGH_BITS_REMOTE(header.msgh_bits), 0, 0, 0);
    replyHeader.msgh_id          = 0;
    replyHeader.msgh_local_port  = MACH_PORT_NULL;
    replyHeader.msgh_remote_port  = header.msgh_remote_port;
    replyHeader.msgh_reserved    = 0;
    replyHeader.msgh_size        = sizeof(replyHeader);
    kern_return_t r = mach_msg(&replyHeader, MACH_SEND_MSG, replyHeader.msgh_size, 0, MACH_PORT_NULL, 0, MACH_PORT_NULL);
    if (r == KERN_SUCCESS) {
        header.msgh_remote_port = MACH_PORT_NULL;
    } else {
        teardown();
    }
}

void dyld_process_info_notify_base::handleEvent() {
    // References object may still exist even after the ports are dead. Disable event dispatching
    // if the ports have been torn down.
    if (_disabled) {
        return;
    }
    // This event handler block has an implicit reference to "this"
    // if incrementing the count goes to one, that means the object may have already been destroyed
    uint8_t messageBuffer[DYLD_PROCESS_INFO_NOTIFY_MAX_BUFFER_SIZE] = {};
    mach_msg_header_t* h = (mach_msg_header_t*)messageBuffer;

    kern_return_t r = mach_msg(h, MACH_RCV_MSG | MACH_RCV_VOUCHER| MACH_RCV_TRAILER_ELEMENTS(MACH_RCV_TRAILER_AUDIT) | MACH_RCV_TRAILER_TYPE(MACH_MSG_TRAILER_FORMAT_0), 0, sizeof(messageBuffer)-sizeof(mach_msg_audit_trailer_t), _receivePortInMonitor, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
    if ( r == KERN_SUCCESS && !(h->msgh_bits & MACH_MSGH_BITS_COMPLEX)) {
        //fprintf(stderr, "received message id=0x%X, size=%d\n", h->msgh_id, h->msgh_size);

        if ( h->msgh_id == DYLD_PROCESS_INFO_NOTIFY_LOAD_ID || h->msgh_id == DYLD_PROCESS_INFO_NOTIFY_UNLOAD_ID ) {
            // run notifier block for each [un]load image
            const dyld_process_info_notify_header* header = (dyld_process_info_notify_header*)messageBuffer;
            if (sizeof(*header) <= h->msgh_size
                && header->imagesOffset <= h->msgh_size
                && header->stringsOffset <= h->msgh_size
                && (header->imageCount * sizeof(dyld_process_info_image_entry)) <= (h->msgh_size - header->imagesOffset)) {
                const dyld_process_info_image_entry* entries = (dyld_process_info_image_entry*)&messageBuffer[header->imagesOffset];
                const char* const stringPool = (char*)&messageBuffer[header->stringsOffset];
                for (unsigned i=0; i < header->imageCount; ++i) {
                    bool isUnload = (h->msgh_id == DYLD_PROCESS_INFO_NOTIFY_UNLOAD_ID);
                    if (entries[i].pathStringOffset <= h->msgh_size - header->stringsOffset) {
                        //fprintf(stderr, "Notifying about: %s\n", stringPool + entries[i].pathStringOffset);
                        _notify(isUnload, header->timestamp, entries[i].loadAddress, entries[i].uuid, stringPool + entries[i].pathStringOffset);
                    } else {
                        teardown();
                        break;
                    }
                }
                // reply to dyld, so it can continue
                replyToMonitoredProcess(*h);
            } else {
                teardown();
            }
        }
        else if ( h->msgh_id == DYLD_PROCESS_INFO_NOTIFY_MAIN_ID ) {
            if (h->msgh_size != sizeof(mach_msg_header_t)) {
                teardown();
            } else if ( _notifyMain != NULL )  {
                _notifyMain();
            }
            replyToMonitoredProcess(*h);
        } else if ( h->msgh_id == MACH_NOTIFY_PORT_DELETED ) {
            mach_port_t deadPort = ((mach_port_deleted_notification_t *)h)->not_port;
            // Validate this notification came from the kernel
            const mach_msg_audit_trailer_t *audit_tlr = (mach_msg_audit_trailer_t *)((uint8_t *)h + round_msg(h->msgh_size));
            if (audit_tlr->msgh_trailer_type == MACH_MSG_TRAILER_FORMAT_0
                && audit_tlr->msgh_trailer_size >= sizeof(mach_msg_audit_trailer_t)
                // We cannot link to libbsm, so we are hardcoding the audit token offset (5)
                // And the value the represents the kernel (0)
                && audit_tlr->msgh_audit.val[5] == 0
                && deadPort == _sendPortInTarget ) {
                teardown();
            }
        }
        else {
            fprintf(stderr, "dyld: received unknown message id=0x%X, size=%d\n", h->msgh_id, h->msgh_size);
        }
    }
    mach_msg_destroy(h);
}

dyld_process_info_notify _dyld_process_info_notify(task_t task, dispatch_queue_t queue,
                                                   void (^notify)(bool unload, uint64_t timestamp, uint64_t machHeader, const uuid_t uuid, const char* path),
                                                   void (^notifyExit)(),
                                                   kern_return_t* kr)
{
    kern_return_t krSink = KERN_SUCCESS;
    if (kr == nullptr) {
        kr = &krSink;
    }
    *kr = KERN_SUCCESS;
    
    dyld_process_info_notify result = new dyld_process_info_notify_base(queue, notify, notifyExit, task, kr);
    if (result->enabled())
        return result;
    const_cast<dyld_process_info_notify_base*>(result)->release();
    return nullptr;
}

void _dyld_process_info_notify_main(dyld_process_info_notify object, void (^notifyMain)())
{
	object->setNotifyMain(notifyMain);
}

void _dyld_process_info_notify_retain(dyld_process_info_notify object)
{
    const_cast<dyld_process_info_notify_base*>(object)->retain();
}

void _dyld_process_info_notify_release(dyld_process_info_notify object)
{
    const_cast<dyld_process_info_notify_base*>(object)->release();
}

static void (*sNotifyMonitoringDyldMain)() = nullptr;
static void (*sNotifyMonitoringDyld)(bool unloading, unsigned imageCount, const struct mach_header* loadAddresses[],
                                     const char* imagePaths[]) = nullptr;

void setNotifyMonitoringDyldMain(void (*func)())
{
    sNotifyMonitoringDyldMain = func;
}

void setNotifyMonitoringDyld(void (*func)(bool unloading, unsigned imageCount,
                                          const struct mach_header* loadAddresses[],
                                          const char* imagePaths[]))
{
    sNotifyMonitoringDyld = func;
}

namespace dyld3 {

void AllImages::notifyMonitorMain()
{
#if !TARGET_OS_DRIVERKIT
    assert(sNotifyMonitoringDyldMain != nullptr);
    sNotifyMonitoringDyldMain();
#endif
}

void AllImages::notifyMonitorLoads(const Array<LoadedImage>& newImages)
{
#if !TARGET_OS_DRIVERKIT
    assert(sNotifyMonitoringDyld != nullptr);
    const struct mach_header* loadAddresses[newImages.count()];
    const char* loadPaths[newImages.count()];
    for(uint32_t i = 0; i<newImages.count(); ++i) {
        loadAddresses[i] = newImages[i].loadedAddress();
        loadPaths[i] = newImages[i].image()->path();
    }
    sNotifyMonitoringDyld(false, (unsigned)newImages.count(), loadAddresses, loadPaths);
#endif
}

void AllImages::notifyMonitorUnloads(const Array<LoadedImage>& unloadingImages)
{
#if !TARGET_OS_DRIVERKIT
    assert(sNotifyMonitoringDyld != nullptr);
    const struct mach_header* loadAddresses[unloadingImages.count()];
    const char* loadPaths[unloadingImages.count()];
    for(uint32_t i = 0; i<unloadingImages.count(); ++i) {
        loadAddresses[i] = unloadingImages[i].loadedAddress();
        loadPaths[i] = unloadingImages[i].image()->path();
    }
    sNotifyMonitoringDyld(true, (unsigned)unloadingImages.count(), loadAddresses, loadPaths);
#endif
}

} // namespace dyld3




