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

@class ODAttributeMap;

NS_CLASS_AVAILABLE_MAC(10_9)
@interface ODRecordMap : NSObject {
	@protected
	NSString *native;
	NSDictionary *odPredicate;
	NSMutableDictionary *attributes;
}

/* native value for this attribute, if not present, translation function or preformed OD predicate must be present */
@property(copy) NSString *native NS_AVAILABLE_MAC(10_9);

/*
 * preformed OD-style predicate that defines a query for a specific record type
 * 
 * predicate dictionary:
 *		attribute : <native attribute>
 *		values : [ <value> ] # note multiple values will be treated as AND
 *		matchtype : <value from kODMatchEqualTo(0x2001) | kODMatchBeginsWith(0x2002) | kODMatchContains(0x2003) | kODMatchEndsWith(0x2004)>
 *
 * nested predicates
 *		operator : { OR | AND | NOT }
 *		predicates : [ <predicate>, ... ]
 */
@property(copy) NSDictionary *odPredicate NS_AVAILABLE_MAC(10_9);

/* dictionary of OpenDirectory standard attributes as a key and a value of ODAttributeMap */
@property(readonly, copy) NSDictionary *attributes NS_AVAILABLE_MAC(10_9);

/* returns an array of NSStrings that list the attributes for this recordmap */
@property(readonly, copy) NSArray *standardAttributeTypes NS_AVAILABLE_MAC(10_9);

/*!
 * @method recordMap
 *
 * @abstract
 * Returns an initialized and autoreleased ODRecordMap object.
 *
 * @discussion
 * Returns an initialized and autoreleased ODRecordMap object.
 */
+ (instancetype)recordMap;

/*!
 * @method attributeMapForStandardAttribute:
 *
 * @abstract
 * Returns an ODAttributeMap object for the given OD standard attribute.
 *
 * @discussion
 * Returns an ODAttributeMap object for the given OD standard attribute.
 */
- (ODAttributeMap *)attributeMapForStandardAttribute:(NSString *)standardAttribute NS_AVAILABLE_MAC(10_9);

/*!
 * @method setAttributeMap:forStandardAttribute:
 *
 * @abstract
 * Sets an ODAttributeMap object for a given OD standard attribute.
 *
 * @discussion
 * Sets an ODAttributeMap object for a given OD standard attribute.
 */
- (void)setAttributeMap:(ODAttributeMap *)attributeMap forStandardAttribute:(NSString *)standardAttribute NS_AVAILABLE_MAC(10_9);

@end
