/*
 * Copyright (c) 2018 Apple Inc. All rights reserved.
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

#import <os/log.h>

@class EFLogEventParser;

os_log_t __log_Spectacles(void);
#define specs_log_err(format, ...)	os_log_error(__log_Spectacles(), format, ##__VA_ARGS__)
#define specs_log_notice(format, ...)	os_log      (__log_Spectacles(), format, ##__VA_ARGS__)
#define specs_log_info(format, ...)	os_log_info (__log_Spectacles(), format, ##__VA_ARGS__)
#define specs_log_debug(format, ...)	os_log_debug(__log_Spectacles(), format, ##__VA_ARGS__)

#define TokenInterfaceName "ifname"

@interface SCLogParser: NSObject
- (instancetype)initWithCategory:(NSString *)category eventParser:(EFLogEventParser *)eventParser;
- (NSData *)createSubsystemIdentifier;
- (NSArray<NSString *> *)addUniqueString:(NSString *)newString toArray:(NSArray<NSString *> *)array;
- (NSArray<NSString *> *)addUniqueStrings:(NSArray<NSString *> *)strings toArray:(NSArray<NSString *> *)array;
- (EFNetworkControlPathEvent *)createInterfaceEventWithLogEvent:(EFLogEvent *)logEvent matchResult:(NSTextCheckingResult *)matchResult;
- (EFNetworkControlPathEvent *)createInterfaceEventWithLogEvent:(EFLogEvent *)logEvent interfaceName:(NSString *)interfaceName;
- (void)addAddress:(NSString *)addressString toInterfaceEvent:(EFNetworkControlPathEvent *)event;
- (BOOL)removeAddress:(NSString *)addressString fromInterfaceEvent:(EFNetworkControlPathEvent *)event;
- (NSString *)substringOfString:(NSString *)matchedString forCaptureGroup:(NSString *)groupName inMatchResult:(NSTextCheckingResult *)result;
- (sa_family_t)getAddressFamilyOfAddress:(NSString *)addressString;
@property (readonly) EFLogEventParser *eventParser;
@property (readonly) NSString *category;
@property (class, readonly) NSMutableDictionary<NSString *, NSArray<NSString *> *> *interfaceMap;
@end
