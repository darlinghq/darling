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

#ifndef _SYSTEMCONFIGURATIONINTERNAL_H
#define _SYSTEMCONFIGURATIONINTERNAL_H

#include <os/availability.h>
#include <TargetConditionals.h>
#include <sys/cdefs.h>

#include "SCDynamicStoreInternal.h"

#if	!TARGET_OS_IPHONE
extern const CFStringRef	kSCEntNetAppleTalk;
extern const CFStringRef	kSCEntNetNetInfo;
extern const CFStringRef	kSCPropNetAppleTalkConfigMethod;
extern const CFStringRef	kSCPropNetAppleTalkDefaultZone;
extern const CFStringRef	kSCPropNetAppleTalkNetworkID;
extern const CFStringRef	kSCPropNetAppleTalkNodeID;
extern const CFStringRef	kSCValNetAppleTalkConfigMethodNode;
#endif	// !TARGET_OS_IPHONE

__BEGIN_DECLS

void
_SCDPluginExecInit		(void);

__END_DECLS

#endif	// _SYSTEMCONFIGURATIONINTERNAL_H
