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

NS_CLASS_AVAILABLE(10_9, NA)
@interface ODAttributeMap : NSObject {
	@protected
	NSString *customQueryFunction;
	NSString *customTranslationFunction;
	NSArray *customAttributes;
	NSString *value;
}

/*
 * Custom functions are in the format of: 
 *		"<module>:<function>"
 *
 * Example: "ldap:query_attribute"
 */

/* Custom function to be used when querying the attribute */
@property(copy) NSString *customQueryFunction NS_AVAILABLE_MAC(10_9);

/* Custom function to be used when translating the result for the client */
@property(copy) NSString *customTranslationFunction NS_AVAILABLE_MAC(10_9);

/* Attributes required for the custom functions */
@property(copy) NSArray *customAttributes NS_AVAILABLE_MAC(10_9);

/* The value used for the mapping.  Static, variable substitution and native are all represented */
@property(copy) NSString *value NS_AVAILABLE_MAC(10_9);

/*!
 * @method attributeMapWithValue:
 *
 * @abstract
 * Returns an initialized and autoreleased ODAttributeMap object with the given value mapped.
 *
 * @discussion
 * Returns an initialized and autoreleased ODAttributeMap object with the given value mapped.
 */
+ (instancetype)attributeMapWithValue:(NSString *)value;

/*!
 * @method attributeMapWithStaticValue:
 *
 * @abstract
 * Returns an initialized and autoreleased ODAttributeMap object with the given static value.
 *
 * @discussion
 * Returns an initialized and autoreleased ODAttributeMap object with the given static value.
 */
+ (instancetype)attributeMapWithStaticValue:(NSString *)staticValue;

/*!
 * @method setStaticValue:
 *
 * @abstract
 * Sets a static value that will always be returned for this mapping.
 *
 * @discussion
 * Sets a static value that will always be returned for this mapping, i.e., "20".
 */
- (void)setStaticValue:(NSString *)staticValue NS_AVAILABLE_MAC(10_9);

/*!
 * @method setVariableSubstitution:
 *
 * @abstract
 * Sets a variable substitution-based value.
 *
 * @discussion
 * Value should be using the syntax '$native$' for all substited values.  For example,
 * to form a home directory using the "cn" of an LDAP record, substitution could be done
 * with "/home/$cn$".
 */
- (void)setVariableSubstitution:(NSString *)variableSubstitution NS_AVAILABLE_MAC(10_9);

@end
