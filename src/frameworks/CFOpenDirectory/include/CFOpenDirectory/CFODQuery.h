/*
 * Copyright (c) 2009 Apple Inc. All rights reserved.
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

#if !defined(__OPENDIRECTORY_CFODQUERY__)
#define __OPENDIRECTORY_CFODQUERY__ 1

#include <CFOpenDirectory/CFOpenDirectory.h>

/*!
    @header CFODQuery
    @abstract   ODQuery functions
    @discussion This header describes functions for working with ODQueryRefs.
*/

/*!
    @typedef    ODQueryCallback
    @abstract   Is called as results are returned from a query.  The incoming result must be retained or copied.
    @discussion Is called as results are returned from an CFRunLoop-based query.  These results are only partial
                and the callback is called repeatedly as results are available.  The incoming result must be retained or copied.  The 
                array will be released by the CFRunLoop upon return.  Incoming results do not include previous results,
                only the most recent results are returned.  inResults can be NULL if an error occurs or the query is complete.  If 
                inError and inResults are NULL then the query has completed.
*/
typedef void (*ODQueryCallback)(ODQueryRef query, CFArrayRef results, CFErrorRef error, void *context);

__BEGIN_DECLS

/*!
    @function   ODQueryGetTypeID
    @abstract   Standard GetTypeID function support for CF-based objects
    @discussion Returns the typeID for the ODQuery object
    @result     a valid CFTypeID for the ODQuery object
*/
CF_EXPORT
CFTypeID ODQueryGetTypeID(void) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODQueryCreateWithNode
    @abstract   Creates a query with the node using the parameters provided
    @discussion Creates a query with the node using the supplied query parameters.  Some parameters can either be CFString or
                CFData or a CFArray of either CFString or CFData.
    @param      allocator a memory allocator to use for this object
    @param      node an ODNodeRef to use
    @param      recordTypeOrList a CFString of a type or CFArray with a list of record types
    @param      attribute a CFStringRef of the attribute name to query
    @param      matchType an ODMatchType value that signifies the type of query
    @param      queryValueOrList a CFStringRef, CFDataRef or CFArrayRef of either type for values to query in attribute
    @param      returnAttributeOrList a CFStringRef or CFArrayRef of CFStrings with the list of attributes to be returned
                from the query.  Passing NULL is equivalent to passing kODAttributeTypeStandardOnly.
    @param      maxResults a CFIndex of the total number of values the caller wants to be returned
    @param      error an optional CFErrorRef reference for error details
    @result     an ODQueryRef which should be passed into ODQueryCopyResults for immediate results or
                ODQueryScheduleWithRunLoop for background behavior
*/
CF_EXPORT
ODQueryRef ODQueryCreateWithNode(CFAllocatorRef allocator, ODNodeRef node, CFTypeRef recordTypeOrList, ODAttributeType attribute, ODMatchType matchType, CFTypeRef queryValueOrList, CFTypeRef returnAttributeOrList, CFIndex maxResults, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODQueryCreateWithNodeType
    @abstract   Creates a query object that is initialized to a particular node type.
    @discussion Creates a query object that is initialized to a particular node type using the supplied
                query options.
    @param      allocator a memory allocator to use for this object
    @param      nodeType an ODNodeType to use when doing a query
    @param      recordTypeOrList a ODRecordType of a type or CFArray with a list of record types
    @param      attribute a ODAttributeType or CFStringRef of the attribute name to query
    @param      matchType an ODMatchType value that signifies the type of query
    @param      queryValueOrList a CFStringRef, CFDataRef or CFArrayRef of either type for values to query in attribute
    @param      returnAttributeOrList a CFStringRef or CFArrayRef of CFStrings with the list of attributes to be returned
                from the query.  Passing NULL is equivalent to passing kODAttributeTypeStandardOnly.
    @param      maxResults a CFIndex of the total number of values the caller wants to be returned
    @param      error an optional CFErrorRef reference for error details
    @result     an ODQueryRef which should be passed into ODQueryCopyResults for immediate results or
                ODQueryScheduleWithRunLoop for background behavior, see ODQueryCallback for details on RunLoop
                behavior.
*/
CF_EXPORT
ODQueryRef ODQueryCreateWithNodeType(CFAllocatorRef allocator, ODNodeType nodeType, CFTypeRef recordTypeOrList, ODAttributeType attribute, ODMatchType matchType, CFTypeRef queryValueOrList, CFTypeRef returnAttributeOrList, CFIndex maxResults, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODQueryCopyResults
    @abstract   Returns results from a provided ODQueryRef synchronously
    @discussion Returns results from a provided ODQueryRef synchronously.  Passing false to inAllowPartialResults
                will block the call until all results are returned or an error occurs.  true can be passed at any time
                even if previous calls were made with false.
    @param      query an ODQueryRef to use
    @param      allowPartialResults a bool, passing true to retrieve any currently available results, or false to
                wait for all results
    @param      error an optional CFErrorRef reference for error details
    @result     a CFArrayRef comprised of ODRecord objects.  If partial results were requested but are complete, then
                NULL will be returned with outError set to NULL. If an error occurs, NULL will be returned and 
                outError should be checked accordingly.
*/
CF_EXPORT
CFArrayRef ODQueryCopyResults(ODQueryRef query, bool allowPartialResults, CFErrorRef *error) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODQuerySynchronize
    @abstract   Will dispose of any results and restart the query.
    @discussion Will dispose of any results and restart the query for subsequent ODQueryCopyResults.  If the query
                is currently scheduled on a RunLoop, then the callback function will be called with inResults == NULL and
                inError.error == kODErrorQuerySynchronize and inError.domain == kODErrorDomainFramework, signifying that
                all existing results should be thrown away in preparation for new results.
    @param      query an ODQueryRef to use
*/
CF_EXPORT
void ODQuerySynchronize(ODQueryRef query) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODQuerySetCallback
    @abstract   This call is used to set the callback function for an asynchronous query
    @discussion This call is used to set the callback function for an asynchronous query, using a
                CFRunLoop or a dispatch queue.
    @param      query an ODQueryRef to use
    @param      callback a function to call when a query has results to return
    @param      userInfo a user-defined pointer to be passed back to the Query callback function
*/
CF_EXPORT
void ODQuerySetCallback(ODQueryRef query, ODQueryCallback callback, void *userInfo) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODQueryScheduleWithRunLoop
    @abstract   Allows a query to run off of a runloop, though it will spawn a thread to handle the work
    @discussion Allows a query to run off of a runloop, though it will spawn a thread to handle the work.
                When query is complete or stopped the callback function will be called with NULL results 
                and inError set to NULL.  ODQueryUnscheduleFromRunLoop() must be called to remove this query
                from Runloops if necessary.
    @param      query an ODQueryRef to put on the runloop
    @param      runLoop a CFRunLoopRef to put the ODQueryRef source onto
    @param      runLoopMode a CFStringRef with the runloop mode to add the ODQueryRef to
*/
CF_EXPORT
void ODQueryScheduleWithRunLoop(ODQueryRef query, CFRunLoopRef runLoop, CFStringRef runLoopMode) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODQueryUnscheduleFromRunLoop
    @abstract   Removes the ODQueryRef from the provided runloop
    @discussion Removes the ODQueryRef from the provided runloop
    @param      query an ODQueryRef to remove from the runloop
    @param      runLoop a CFRunLoopRef to remove the ODQuery source from
    @param      runLoopMode a CFStringRef of the mode to remove the ODQuery from
*/
CF_EXPORT
void ODQueryUnscheduleFromRunLoop(ODQueryRef query, CFRunLoopRef runLoop, CFStringRef runLoopMode) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

/*!
    @function   ODQuerySetDispatchQueue
    @abstract   Performs the query and sends the results using the specified dispatch queue
    @discussion Schedule the query to run and deliver its results using the specified dispatch queue.
                The previously set callback will be called using the same semantics as
                ODQueryScheduleWithRunLoop
    @param      query an ODQueryRef to perform
    @param      queue a dispatch queue to receive the query results
*/
CF_EXPORT
void ODQuerySetDispatchQueue(ODQueryRef query, dispatch_queue_t queue) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_NA);

__END_DECLS

#endif /* ! __OPENDIRECTORY_CFODQUERY__ */
