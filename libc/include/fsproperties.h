/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
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

#ifndef _FSPROPERTIES_H_
#define _FSPROPERTIES_H_

/* Info plist keys */
#define kFSMediaTypesKey             "FSMediaTypes"
#define kFSPersonalitiesKey          "FSPersonalities"

/* Sub-keys for FSMediaTypes dictionaries */
#define kFSMediaPropertiesKey        "FSMediaProperties"
#define kFSProbeArgumentsKey         "FSProbeArguments"
#define kFSProbeExecutableKey        "FSProbeExecutable"
#define kFSProbeOrderKey             "FSProbeOrder"

/* Sub-keys for FSPersonalities dictionaries */
#define kFSFormatArgumentsKey        "FSFormatArguments"
#define kFSFormatContentMaskKey      "FSFormatContentMask"
#define kFSFormatExecutableKey       "FSFormatExecutable"
#define kFSFormatInteractiveKey      "FSFormatInteractive"
#define kFSFormatMinimumSizeKey      "FSFormatMinimumSize"
#define kFSFormatMaximumSizeKey      "FSFormatMaximumSize"
#define kFSMountArgumentsKey         "FSMountArguments"
#define kFSMountExecutableKey        "FSMountExecutable"
#define kFSNameKey                   "FSName"
#define kFSRepairArgumentsKey        "FSRepairArguments"
#define kFSRepairExecutableKey       "FSRepairExecutable"
#define kFSVerificationArgumentsKey  "FSVerificationArguments"
#define kFSVerificationExecutableKey "FSVerificationExecutable"
#define kFSSubTypeKey                "FSSubType"
#define kFSXMLOutputArgumentKey      "FSXMLOutputArgument"

#define	kFSCoreStorageEncryptNameKey "FSCoreStorageEncryptionName"

#endif /* _FSPROPERTIES_H_ */
