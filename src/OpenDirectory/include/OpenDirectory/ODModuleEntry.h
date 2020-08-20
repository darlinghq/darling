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

@class ODMappings;

NS_CLASS_AVAILABLE(10_9, NA)
@interface ODModuleEntry : NSObject {
	@protected
	ODMappings *mappings;
	NSArray *supportedOptions;
	NSString *name;
	NSString *xpcServiceName;
	NSMutableDictionary *options;
	NSString *uuidString;
};

/* mappings that are specific to this module configuration */
@property(strong) ODMappings *mappings NS_AVAILABLE_MAC(10_9);

/* A list supported options for a module.  The list will contain dictionaries with keys ODModuleOptionName, ODModuleOptionType, etc. */
@property(readonly, copy) NSArray *supportedOptions NS_AVAILABLE_MAC(10_9);

/* Name of the module, used in logging, etc. */
@property(copy) NSString *name NS_AVAILABLE_MAC(10_9);

/* XPCService to be used for this module */
@property(copy) NSString *xpcServiceName NS_AVAILABLE_MAC(10_9);

/* A UUID in string form that uniquely identifies this configuration, will be assigned automatically if missing */
@property(copy) NSString *uuidString NS_AVAILABLE_MAC(10_9);

/*!
 * @method moduleEntryWithName:xpcServiceName:
 *
 * @abstract
 * Creates a new module entry with a given name and service.
 *
 * @discussion
 * Creates a new module entry with a given name and service.
 */
+ (instancetype)moduleEntryWithName:(NSString *)name xpcServiceName:(NSString *)xpcServiceName NS_AVAILABLE_MAC(10_9);

/*!
 * @method setOption:value:
 *
 * @abstract
 * Assigns a particular option for this module.
 *
 * @discussion
 * Options are dictated by the module and can be queried via [module supportedOptions].
 */
- (void) setOption:(NSString *)optionName value:(id)value NS_AVAILABLE_MAC(10_9);

/*!
 * @method option:
 *
 * @abstract
 * Fetches the current setting for the requested option.
 *
 * @discussion
 * Fetches the current setting for the requested option.
 */
- (id) option:(NSString *)optionName NS_AVAILABLE_MAC(10_9);

@end
