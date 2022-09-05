/*
 * Copyright (c) 2012 Apple Inc. All rights reserved.
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

#import <Foundation/Foundation.h>

@class ODRecordMap;

NS_CLASS_AVAILABLE(10_9, NA)
@interface ODMappings : NSObject {
	@protected
	NSString *comment;
	NSString *templateName;
	NSString *identifier;
	NSString *function;
	NSArray *functionAttributes;
	NSMutableDictionary *recordTypes;
}

/* Short comment about the mappings */
@property(copy) NSString *comment NS_AVAILABLE_MAC(10_9);

/*
 * Name of the mappings template to be used for this configuration from /System/Library/OpenDirectory/Mappings or /Library/OpenDirectory/Mappings
 * if a template is used, then no other values should be set, template is an exclusive setting
 */
@property(copy) NSString *templateName NS_AVAILABLE_MAC(10_9);

/* RFC1034 identifier for this mapping, com.company.mapping */
@property(copy) NSString *identifier NS_AVAILABLE_MAC(10_9);

/* Returns a list of NSString values for all attributes currently configured */
@property(readonly, copy) NSArray *recordTypes NS_AVAILABLE_MAC(10_9);

/* function to translate record types */
@property(copy) NSString *function NS_AVAILABLE_MAC(10_9);

/* native attributes required for translation function to work */
@property(copy) NSArray *functionAttributes NS_AVAILABLE_MAC(10_9);

/*!
 * @method mappings
 *
 * @abstract
 * Returns an initialized and autoreleased ODMappings object.
 *
 * @discussion
 * Returns an initialized and autoreleased ODMappings object.
 */
+ (instancetype)mappings;

/*!
 * @method recordType:
 *
 * @abstract
 * Returns an ODRecordMap associated with the provided recordtype.
 *
 * @discussion
 * Returns an ODRecordMap associated with the provided recordtype.
 */
- (ODRecordMap *) recordMapForStandardRecordType:(NSString *)stdType NS_AVAILABLE_MAC(10_9);

/*!
 * @method setRecordMap:forRecordType:
 *
 * @abstract
 * Sets a particular ODRecordMap for a given standard record type.
 *
 * @discussion
 * Sets a particular ODRecordMap for a given standard record type.
 */
- (void) setRecordMap:(ODRecordMap *)map forStandardRecordType:(NSString *)stdType NS_AVAILABLE_MAC(10_9);

@end
