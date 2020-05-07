/*
 * Copyright (c) 2008 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

#ifndef _PRELINK_H_
#define _PRELINK_H_

#define kPrelinkTextSegment                "__PRELINK_TEXT"
#define kPrelinkTextSection                "__text"

#define kPrelinkDataSegment                "__PRELINK_DATA"
#define kPrelinkDataSection                "__data"

#define kPrelinkInfoSegment                "__PRELINK_INFO"
#define kPrelinkInfoSection                "__info"
#define kBuiltinInfoSection                "__kmod_info"
#define kBuiltinStartSection               "__kmod_start"

// __DATA segment
#define kBuiltinInitSection                "__kmod_init"
#define kBuiltinTermSection                "__kmod_term"

#define kPrelinkBundlePathKey              "_PrelinkBundlePath"
#define kPrelinkExecutableRelativePathKey  "_PrelinkExecutableRelativePath"
#define kPrelinkExecutableLoadKey          "_PrelinkExecutableLoadAddr"
#define kPrelinkExecutableSourceKey        "_PrelinkExecutableSourceAddr"
#define kPrelinkExecutableSizeKey          "_PrelinkExecutableSize"
#define kPrelinkInfoDictionaryKey          "_PrelinkInfoDictionary"
#define kPrelinkInterfaceUUIDKey           "_PrelinkInterfaceUUID"
#define kPrelinkKmodInfoKey                "_PrelinkKmodInfo"
#define kPrelinkLinkStateKey               "_PrelinkLinkState"
#define kPrelinkLinkStateSizeKey           "_PrelinkLinkStateSize"
#define kPrelinkLinkKASLROffsetsKey        "_PrelinkLinkKASLROffsets"
#define kPrelinkInfoKCIDKey                "_PrelinkKCID"

#endif /* _PRELINK_H_ */
