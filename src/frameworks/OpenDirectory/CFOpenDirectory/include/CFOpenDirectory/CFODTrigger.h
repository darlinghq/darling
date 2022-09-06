/*
 * Copyright (c) 2010 Apple Inc. All rights reserved.
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

#if !defined(__OPENDIRECTORY_CFODTRIGGER__)
#define __OPENDIRECTORY_CFODTRIGGER__

typedef struct __ODTrigger *ODTriggerRef;

/* ODTriggerCreateForRecords */
enum {
	kODTriggerRecordEventAdd = 1,
	kODTriggerRecordEventDelete = 2,
	kODTriggerRecordEventModify = 4,
};

/* ODTriggerCreateForNodes */
enum {
	kODTriggerNodeRegister = 1,
	kODTriggerNodeUnregister = 2,
	kODTriggerNodeOnline = 4,
	kODTriggerNodeOffline = 8,
};

/* ODTriggerCreateForSearch */
enum {
	kODTriggerSearchAdd = 1,
	kODTriggerSearchDelete = 2,
	kODTriggerSearchOnline = 4,
	kODTriggerSearchOffline = 8,
};

typedef CFOptionFlags ODTriggerEventFlags;

__BEGIN_DECLS

/*!
    @function   ODTriggerGetTypeID
    @abstract   returns the type identifier for the ODTrigger type
    @result     The type identifer for the ODTrigger type.
*/
CF_EXPORT CFTypeID
ODTriggerGetTypeID(void) __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);

/*!
    @function   ODTriggerCreateForRecords
    @abstract   creates a trigger for a particular record event
    @discussion Creates a trigger for a particular record event.  nodenames, recordnames and attributes is optional. Pass a CFArrayRef or CFStringRef for each list.
    @param      events mask of events to monitor: kODTriggerRecordEventAdd, kODTriggerRecordEventDelete, kODTriggerRecordEventModify
    @param      nodenames (optional) node names to monitor. Pass NULL to monitor all nodes.
    @param      recordtypes record types of interest.
    @param      recordnames (optional) record names to watch. Pass NULL to receive an event for any record.
    @param      queue a queue to deliver this event on
    @param      block a block to execute when event is triggered
    @result     returns an ODTriggerRef that may be cleaned up with ODTriggerCancel and CFRelease
*/
CF_EXPORT ODTriggerRef
ODTriggerCreateForRecords(CFAllocatorRef allocator, ODTriggerEventFlags events, CFTypeRef nodenames, CFTypeRef recordtypes, CFTypeRef recordnames,
	dispatch_queue_t queue, void (^block)(ODTriggerRef trigger, CFStringRef node, CFStringRef type, CFStringRef name))
	__OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);

/*!
    @function   ODTriggerCreateForRecordAttributes
    @abstract   creates a trigger for record attribute modifications
    @discussion Creates a trigger for modification of record attributes. Pass a CFArrayRef or CFStringRef for each list.
    @param      nodenames (optional) node names to watch. Pass NULL to watch all nodes.
    @param      recordtypes record types to monitor (required)
    @param      recordnames (optional) record names to monitor. Pass NULL to monitor all records.
    @param      attributes (optional) attributes of interest. Pass NULL to see all attribute changes.
    @param      queue a queue to deliver this event on
    @param      block a block to execute when event is triggered
    @result     returns an ODTriggerRef that may be cleaned up with ODTriggerCancel and CFRelease
*/
CF_EXPORT ODTriggerRef
ODTriggerCreateForRecordAttributes(CFAllocatorRef allocator, CFTypeRef nodenames, CFTypeRef recordtypes, CFTypeRef recordnames, CFTypeRef attributes,
	dispatch_queue_t queue, void (^block)(ODTriggerRef trigger, CFStringRef node, CFStringRef type, CFStringRef rec, CFStringRef attr))
	__OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);

/*!
    @function   nodes_create
    @abstract   creates a trigger for a particular node event
    @discussion Creates a trigger for a particular node event. Pass a CFArrayRef or CFStringRef for each list.
    @param      events mask of events to monitor: kODTriggerNodeOffline, kODTriggerNodeOnline, kODTriggerNodeRegister, kODTriggerNodeUnregister
    @param      nodenames (optional) node names to watch. Pass NULL to watch all nodes.
    @param      queue a queue to deliver this event on
    @param      block a block to execute when event is triggered
    @result     returns an ODTriggerRef that may be cleaned up with ODTriggerCancel and CFRelease
*/
CF_EXPORT ODTriggerRef
ODTriggerCreateForNodes(CFAllocatorRef allocator, ODTriggerEventFlags events, CFTypeRef nodenames,
	dispatch_queue_t queue, void (^block)(ODTriggerRef trigger, CFStringRef node))
	__OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);

/*!
    @function   ODTriggerCreateForSearch
    @abstract   creates a trigger for a search node event
    @discussion Creates a trigger for a search node event. Pass a CFArrayRef or CFStringRef for each list.
    @param      events mask of events to monitor: kODTriggerSearchAdd, kODTriggerSearchDelete, kODTriggerSearchOffline, kODTriggerSearchOnline
    @param      nodenames (optional) interesting node names on the search policy. Pass NULL to monitor all search policy nodes.
    @param      queue a queue to deliver this event on
    @param      block a block to execute when event is triggered
    @result     returns an ODTriggerRef that may be cleaned up with ODTriggerCancel and CFRelease
*/
CF_EXPORT ODTriggerRef
ODTriggerCreateForSearch(CFAllocatorRef allocator, ODTriggerEventFlags events, CFTypeRef nodenames,
	dispatch_queue_t queue, void (^block)(ODTriggerRef trigger, CFStringRef node))
	__OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);

/*!
    @function   ODTriggerCancel
    @abstract   cancel the trigger
    @discussion Permanently stops all this trigger from firing. Must call CFRelease as well.
*/
CF_EXPORT void
ODTriggerCancel(ODTriggerRef trigger) __OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_NA);

__END_DECLS

#endif /* !__OPENDIRECTORY_CFODTRIGGER__ */
