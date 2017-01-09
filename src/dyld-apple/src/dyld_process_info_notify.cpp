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

#include "dyld_process_info.h"
#include "dyld_process_info_internal.h"
#include "dyld_images.h"
#include "dyld_priv.h"

typedef void (^Notify)(bool unload, uint64_t timestamp, uint64_t machHeader, const uuid_t uuid, const char* path);
typedef void (^NotifyExit)();


//
// Object used for monitoring another processes dyld loads
//
struct __attribute__((visibility("hidden"))) dyld_process_info_notify_base
{
    static dyld_process_info_notify_base* make(task_t task, dispatch_queue_t queue, Notify notify, NotifyExit notifyExit, kern_return_t* kr);
											~dyld_process_info_notify_base();

    uint32_t&           retainCount() const { return _retainCount; }

private:
                        dyld_process_info_notify_base(dispatch_queue_t queue, Notify notify, NotifyExit notifyExit, task_t task);
    kern_return_t       makePorts();
    kern_return_t       pokeSendPortIntoTarget();
	kern_return_t		unpokeSendPortInTarget();
    void				setMachSourceOnQueue();
    void*               operator new (size_t, void* buf) { return buf; }

	mutable uint32_t	_retainCount;
    dispatch_queue_t    _queue;
    Notify              _notify;
    NotifyExit          _notifyExit;
	task_t				_targetTask;
	dispatch_source_t	_machSource;
    uint64_t            _portAddressInTarget;
    mach_port_t         _sendPortInTarget;          // target is process being watched for image loading/unloading
    mach_port_t         _receivePortInMonitor;      // monitor is process being notified of image loading/unloading
};


dyld_process_info_notify_base::dyld_process_info_notify_base(dispatch_queue_t queue, Notify notify, NotifyExit notifyExit, task_t task)
    : _retainCount(1), _queue(queue), _notify(notify), _notifyExit(notifyExit), _targetTask(task), _machSource(NULL), _portAddressInTarget(0), _sendPortInTarget(0), _receivePortInMonitor(0)
{
    dispatch_retain(_queue);
}

dyld_process_info_notify_base::~dyld_process_info_notify_base()
{
	if ( _machSource ) {
		dispatch_release(_machSource);
		_machSource = NULL;
	}
	if ( _portAddressInTarget ) {
		unpokeSendPortInTarget();
		_portAddressInTarget = 0;
	}
	if ( _sendPortInTarget ) {
		_sendPortInTarget = 0;
	}
    dispatch_release(_queue);
	if ( _receivePortInMonitor != 0 ) {
		mach_port_deallocate(mach_task_self(), _receivePortInMonitor);
		_receivePortInMonitor = 0;
	}
}


dyld_process_info_notify_base* dyld_process_info_notify_base::make(task_t task, dispatch_queue_t queue, Notify notify, NotifyExit notifyExit, kern_return_t* kr)
{
    void* storage = malloc(sizeof(dyld_process_info_notify_base));
    dyld_process_info_notify_base* obj = new (storage) dyld_process_info_notify_base(queue, notify, notifyExit, task);

    if ( kern_return_t r = obj->makePorts() ) {
		if ( kr != NULL )
			*kr = r;
        goto fail;
	}

    obj->setMachSourceOnQueue();

    if ( kern_return_t r = obj->pokeSendPortIntoTarget() ) {
		if ( kr != NULL )
			*kr = r;
        goto fail;
	}

	if ( kr != NULL )
		*kr = KERN_SUCCESS;
    return obj;

fail:
    free(obj);
    return NULL;
}


kern_return_t dyld_process_info_notify_base::makePorts()
{
    // Allocate a port to listen on in this monitoring task
    if ( kern_return_t r = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &_receivePortInMonitor) )
        return r;

    // Add send rights for replying
    if ( kern_return_t r = mach_port_insert_right(mach_task_self(), _receivePortInMonitor, _receivePortInMonitor, MACH_MSG_TYPE_MAKE_SEND) )
        return r;

    // Allocate a name in the target. We need a new name to add send rights to
    if ( kern_return_t r = mach_port_allocate(_targetTask, MACH_PORT_RIGHT_DEAD_NAME, &_sendPortInTarget) )
        return r;

    // Deallocate the dead name
    if ( kern_return_t r = mach_port_mod_refs(_targetTask, _sendPortInTarget, MACH_PORT_RIGHT_DEAD_NAME, -1) )
        return r;

    // Make the dead name a send right to our listening port
    if ( kern_return_t r = mach_port_insert_right(_targetTask, _sendPortInTarget, _receivePortInMonitor, MACH_MSG_TYPE_MAKE_SEND) )
        return r;

    // Notify us if the target dies
    mach_port_t previous = MACH_PORT_NULL;
    if ( kern_return_t r = mach_port_request_notification(_targetTask, _sendPortInTarget, MACH_NOTIFY_DEAD_NAME, 0, _receivePortInMonitor, MACH_MSG_TYPE_MAKE_SEND_ONCE, &previous))
        return r;

    //fprintf(stderr, "_sendPortInTarget=%d, _receivePortInMonitor=%d\n", _sendPortInTarget, _receivePortInMonitor);
    return KERN_SUCCESS;
}



void dyld_process_info_notify_base::setMachSourceOnQueue()
{
	NotifyExit exitHandler = _notifyExit;
	_machSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_MACH_RECV, _receivePortInMonitor, 0, _queue);
    dispatch_source_set_event_handler(_machSource, ^{
        uint8_t messageBuffer[DYLD_PROCESS_INFO_NOTIFY_MAX_BUFFER_SIZE];
        mach_msg_header_t* h = (mach_msg_header_t*)messageBuffer;

        kern_return_t r = mach_msg(h, MACH_RCV_MSG, 0, sizeof(messageBuffer), _receivePortInMonitor, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
        if ( r == KERN_SUCCESS ) {
            //fprintf(stderr, "received message id=0x%X, size=%d\n", h->msgh_id, h->msgh_size);
			if ( h->msgh_id == DYLD_PROCESS_INFO_NOTIFY_LOAD_ID || h->msgh_id == DYLD_PROCESS_INFO_NOTIFY_UNLOAD_ID ) {
				// run notifier block for each [un]load image
				const dyld_process_info_notify_header* header = (dyld_process_info_notify_header*)messageBuffer;
				const dyld_process_info_image_entry* entries = (dyld_process_info_image_entry*)&messageBuffer[header->imagesOffset];
				const char* const stringPool = (char*)&messageBuffer[header->stringsOffset];
				for (unsigned i=0; i < header->imageCount; ++i) {
					bool isUnload = (h->msgh_id == DYLD_PROCESS_INFO_NOTIFY_UNLOAD_ID);
					_notify(isUnload, header->timestamp, entries[i].loadAddress, entries[i].uuid, stringPool + entries[i].pathStringOffset);
				}
				// reply to dyld, so it can continue
				mach_msg_header_t replyHeader;
				replyHeader.msgh_bits        = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, MACH_MSG_TYPE_MAKE_SEND);
				replyHeader.msgh_id          = 0;
				replyHeader.msgh_local_port  = MACH_PORT_NULL;
				replyHeader.msgh_remote_port = h->msgh_remote_port;
				replyHeader.msgh_reserved    = 0;
				replyHeader.msgh_size        = sizeof(replyHeader);
				mach_msg(&replyHeader, MACH_SEND_MSG | MACH_SEND_TIMEOUT, replyHeader.msgh_size, 0, MACH_PORT_NULL, 100, MACH_PORT_NULL);
			}
			else if ( h->msgh_id == MACH_NOTIFY_PORT_DELETED ) {
				mach_port_t deadPort = ((mach_port_deleted_notification_t *)h)->not_port;
				//fprintf(stderr, "received message id=MACH_NOTIFY_PORT_DELETED, size=%d, deadPort=%d\n", h->msgh_size, deadPort);
				if ( deadPort == _sendPortInTarget ) {
					// target process died.  Clean up ports
					_sendPortInTarget = 0;
					mach_port_deallocate(mach_task_self(), _receivePortInMonitor);
					_receivePortInMonitor = 0;
					_portAddressInTarget = 0;
					// notify that target is gone
					exitHandler();
				}
			}
			else {
				fprintf(stderr, "received unknown message id=0x%X, size=%d\n", h->msgh_id, h->msgh_size);
			}
        }
    });
    dispatch_resume(_machSource);
}


kern_return_t dyld_process_info_notify_base::pokeSendPortIntoTarget()
{
    // get location on all_image_infos in target task
    task_dyld_info_data_t taskDyldInfo;
    mach_msg_type_number_t count = TASK_DYLD_INFO_COUNT;
    kern_return_t r = task_info(_targetTask, TASK_DYLD_INFO, (task_info_t)&taskDyldInfo, &count);
    if ( r )
        return  r;

    // remap the page containing all_image_infos into this process r/w
    mach_vm_address_t mappedAddress = 0;
    mach_vm_size_t    mappedSize = taskDyldInfo.all_image_info_size;
    vm_prot_t curProt = VM_PROT_NONE;
    vm_prot_t maxProt = VM_PROT_NONE;
    r = mach_vm_remap(mach_task_self(), &mappedAddress, mappedSize, 0, VM_FLAGS_ANYWHERE | VM_FLAGS_RETURN_DATA_ADDR,
    					_targetTask, taskDyldInfo.all_image_info_addr, false, &curProt, &maxProt, VM_INHERIT_NONE);
    if ( r )
        return r;
    if ( curProt != (VM_PROT_READ|VM_PROT_WRITE) )
        return KERN_PROTECTION_FAILURE;

    // atomically set port into all_image_info_struct
    static_assert(sizeof(mach_port_t) == sizeof(uint32_t), "machport size not 32-bits");

    mach_vm_address_t mappedAddressToPokePort = 0;
    if ( taskDyldInfo.all_image_info_format == TASK_DYLD_ALL_IMAGE_INFO_32 )
        mappedAddressToPokePort = mappedAddress + offsetof(dyld_all_image_infos_32,notifyMachPorts);
    else
        mappedAddressToPokePort = mappedAddress + offsetof(dyld_all_image_infos_64,notifyMachPorts);

    // use first available slot
	bool slotFound = false;
	for (int slotIndex=0; slotIndex < DYLD_MAX_PROCESS_INFO_NOTIFY_COUNT; ++slotIndex) {
		if ( OSAtomicCompareAndSwap32Barrier(0, _sendPortInTarget, (volatile int32_t*)mappedAddressToPokePort) ) {
			slotFound = true;
			break;
		}
		mappedAddressToPokePort += sizeof(uint32_t);
     }
	if ( !slotFound ) {
		mach_vm_deallocate(mach_task_self(), mappedAddress, mappedSize);
		return KERN_UREFS_OVERFLOW;
	}
    _portAddressInTarget = taskDyldInfo.all_image_info_addr + mappedAddressToPokePort - mappedAddress;
    //fprintf(stderr, "poked port %d into target at address 0x%llX\n", _sendPortInTarget, _portAddressInTarget);
    mach_vm_deallocate(mach_task_self(), mappedAddress, mappedSize);
    return r;
}



kern_return_t dyld_process_info_notify_base::unpokeSendPortInTarget()
{
    // remap the page containing all_image_infos into this process r/w
    mach_vm_address_t mappedAddress = 0;
    mach_vm_size_t    mappedSize = sizeof(mach_port_t);
    vm_prot_t curProt = VM_PROT_NONE;
    vm_prot_t maxProt = VM_PROT_NONE;
    kern_return_t r = mach_vm_remap(mach_task_self(), &mappedAddress, mappedSize, 0, VM_FLAGS_ANYWHERE | VM_FLAGS_RETURN_DATA_ADDR,
									_targetTask, _portAddressInTarget, false, &curProt, &maxProt, VM_INHERIT_NONE);
    if ( r )
        return r;
    if ( curProt != (VM_PROT_READ|VM_PROT_WRITE) )
        return KERN_PROTECTION_FAILURE;

    OSAtomicCompareAndSwap32Barrier(_sendPortInTarget, 0, (volatile int32_t*)mappedAddress);

    //fprintf(stderr, "cleared port %d from target\n", _sendPortInTarget);
    mach_vm_deallocate(mach_task_self(), mappedAddress, mappedSize);
    return r;
}



dyld_process_info_notify _dyld_process_info_notify(task_t task, dispatch_queue_t queue,
                                                   void (^notify)(bool unload, uint64_t timestamp, uint64_t machHeader, const uuid_t uuid, const char* path),
                                                   void (^notifyExit)(),
                                                   kern_return_t* kr)
{
    return dyld_process_info_notify_base::make(task, queue, notify, notifyExit, kr);
}

void _dyld_process_info_notify_retain(dyld_process_info_notify object)
{
    object->retainCount() += 1;
}

void _dyld_process_info_notify_release(dyld_process_info_notify object)
{
    object->retainCount() -= 1;
    if ( object->retainCount() == 0 ) {
		object->~dyld_process_info_notify_base();
		free((void*)object);
	}
}



