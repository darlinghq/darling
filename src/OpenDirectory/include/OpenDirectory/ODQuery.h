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

#import <OpenDirectory/OpenDirectory.h>

@class ODQuery;
@class ODNode;

/*!
    @protocol    ODQueryDelegate
    @abstract    The delegate method for receiving query results from the NSRunLoop-based queries
    @discussion  The delegate method called as results are returned from an NSRunLoop-based query.  These results are only partial
                 and delegate is called repeatedly as results are available.  The incoming result must be retained or copied.  The 
                 array will be released by the NSRunLoop upon return.  Incoming results do not include previous results,
                 only the most resent results are returned.  inResults can be nil if an error occurs or the query is complete.  If 
                 inResults and inError are nil then the query has completed.
*/
@protocol ODQueryDelegate <NSObject>
@required
- (void)query:(ODQuery *)inQuery foundResults:(NSArray *)inResults error:(NSError *)inError NS_AVAILABLE(10_6, NA);
@end

/*!
    @class       ODQuery
    @abstract    Class used for querying OpenDirectory.
    @discussion  OpenDirectory queries may be used to search for different types of records, e.g. users, groups.
*/
@interface ODQuery : NSObject <NSCopying>
{
	@private
	void *_internal;
}

/*!
    @method     queryWithNode:forRecordTypes:attribute:matchType:queryValues:returnAttributes:maximumResults:error:
    @abstract   Creates an autoreleased query with the node using the parameters provided
    @discussion Creates an autoreleased query with the node using the supplied query parameters.  Some parameters
                can either be NSString or NSData or an NSArray of either NSString or NSData.  Passing nil for 
                returnAttributes is equivalent to passing kODAttributeTypeStandardOnly.  outError is optional parameter,
                nil can be passed if error details are not needed.  
*/
+ (ODQuery *)queryWithNode:(ODNode *)inNode forRecordTypes:(id)inRecordTypeOrList attribute:(ODAttributeType)inAttribute
                 matchType:(ODMatchType)inMatchType queryValues:(id)inQueryValueOrList 
          returnAttributes:(id)inReturnAttributeOrList maximumResults:(NSInteger)inMaximumResults
                     error:(NSError **)outError NS_AVAILABLE(10_6, NA);

/*!
    @method     initWithNode:forRecordTypes:attribute:matchType:queryValues:returnAttributes:maximumResults:error:
    @abstract   Creates a query with the node using the parameters provided
    @discussion Creates a query with the node using the supplied query parameters.  Some parameters
                can either be NSString or NSData or an NSArray of either NSString or NSData.  Passing nil for 
                returnAttributes is equivalent to passing kODAttributeTypeStandardOnly. outError is optional parameter,
                nil can be passed if error details are not needed.
*/
- (instancetype)initWithNode:(ODNode *)inNode forRecordTypes:(id)inRecordTypeOrList attribute:(ODAttributeType)inAttribute
         matchType:(ODMatchType)inMatchType queryValues:(id)inQueryValueOrList 
  returnAttributes:(id)inReturnAttributeOrList maximumResults:(NSInteger)inMaximumResults error:(NSError **)outError NS_AVAILABLE(10_6, NA);

/*!
    @method     resultsAllowingPartial:error:
    @abstract   Returns results from a provided ODQuery synchronously
    @discussion Returns results from a provided ODQuery synchronously.  Passing NO to inAllowPartialResults
                will block the call until all results are returned or an error occurs.  YES can be passed at any time
                even if previous calls were made with NO.  outError is optional parameter, nil can be passed if error 
                details are not needed.
*/
- (NSArray *)resultsAllowingPartial:(BOOL)inAllowPartialResults error:(NSError **)outError NS_AVAILABLE(10_6, NA);

/*!
    @property   delegate
    @abstract   The currently set delegate
    @discussion The query delegate which will receive asynchronous query results.
*/
@property (nonatomic, readwrite, assign) id <ODQueryDelegate> delegate NS_AVAILABLE(10_6, NA);

/*!
    @method     scheduleInRunLoop:forMode:
    @abstract   Adds the query object to the specified NSRunLoop to receive asynchronous results
    @discussion Adds the query object to the specified NSRunLoop to receive asynchronous results.  A delegate must be set
                in advance otherwise results may be lost due to the lack of a receiver.
*/
- (void)scheduleInRunLoop:(NSRunLoop *)inRunLoop forMode:(NSString *)inMode NS_AVAILABLE(10_6, NA);

/*!
    @method     removeFromRunLoop:forMode:
    @abstract   Removes the query object from the specified NSRunLoop
    @discussion Removes the query object from the specified NSRunLoop.
*/
- (void)removeFromRunLoop:(NSRunLoop *)inRunLoop forMode:(NSString *)inMode NS_AVAILABLE(10_6, NA);

/*!
    @method     synchronize
    @abstract   Will dispose of any results and restart the query.
    @discussion Will dispose of any results and restart the query for subsequent resultsAllowingPartial: calls.  If the query
                is currently scheduled on a RunLoop, then the delegate will be called with inResults == nil and
                [inError code] == kODErrorQuerySynchronize and [inError domain] == ODFrameworkErrorDomain, signifying that
                all existing results should be thrown away in preparation for new results.
*/
- (void)synchronize NS_AVAILABLE(10_6, NA);

/*!
    @property   operationQueue
    @abstract   The NSOperationQueue on which asynchronous results are delivered to the delegate.
    @discussion The NSOperationQueue on which asynchronous results are delivered to the delegate.
 */
@property (readwrite, retain) NSOperationQueue * operationQueue NS_AVAILABLE(10_6, NA);

@end
