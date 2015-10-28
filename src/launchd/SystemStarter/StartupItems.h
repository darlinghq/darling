/**
 * StartupItems.h - Startup Item management routines
 * Wilfredo Sanchez | wsanchez@opensource.apple.com
 * Kevin Van Vechten | kevinvv@uclink4.berkeley.edu
 * $Apple$
 **
 * Copyright (c) 1999-2002 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * @APPLE_APACHE_LICENSE_HEADER_END@
 **/

#ifndef _StartupItems_H_
#define _StartupItems_H_

#include <NSSystemDirectories.h>

#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFDictionary.h>

#include "SystemStarter.h"

#define kProvidesKey        CFSTR("Provides")
#define kRequiresKey        CFSTR("Requires")
#define kDescriptionKey     CFSTR("Description")
#define kUsesKey            CFSTR("Uses")
#define kErrorKey           CFSTR("Error")
#define kBundlePathKey      CFSTR("PathToBundle")
#define kPIDKey             CFSTR("ProcessID")
#define kDomainKey          CFSTR("Domain")


#define kErrorPermissions   CFSTR("incorrect permissions")
#define kErrorInternal      CFSTR("SystemStarter internal error")
#define kErrorReturnNonZero CFSTR("execution of Startup script failed")
#define kErrorFork          CFSTR("could not fork() StartupItem")


/*
 * Find all available startup items in NSDomains specified by aMask.
 */
CFMutableArrayRef StartupItemListCreateWithMask (NSSearchPathDomainMask aMask);

/*
 * Returns the item responsible for providing aService.
 */
CFMutableDictionaryRef StartupItemListGetProvider (CFArrayRef anItemList, CFStringRef aService);

/*
 * Creates a list of items in anItemList which depend on anItem, given anAction.
 */
CFMutableArrayRef StartupItemListCreateDependentsList (CFMutableArrayRef anItemList,
						       CFStringRef       aService  ,
						       Action            anAction  );

/*
 * Given aWaitingList of startup items, and aStatusDict describing the current
 * startup state, returns the next startup item to run, if any. Returns nil if
 * none is available.
 * Note that this is not necessarily deterministic; if more than one startup
 * item is ready to run, which item gets returned is not specified. An item is
 * not ready to run if the specified dependencies are not satisfied yet.
 */
CFMutableDictionaryRef StartupItemListGetNext (CFArrayRef      aWaitingList,
                                               CFDictionaryRef aStatusDict ,
					       Action          anAction    );

CFMutableDictionaryRef StartupItemWithPID (CFArrayRef anItemList, pid_t aPID);
pid_t StartupItemGetPID(CFDictionaryRef anItem);

CFStringRef StartupItemCreateDescription(CFMutableDictionaryRef anItem);

/*
 * Returns a list of currently executing startup items.
 */
CFArrayRef StartupItemListCreateFromRunning(CFArrayRef anItemList);

/*
 * Returns the total number of "Provides" entries of all loaded items.
 */
CFIndex StartupItemListCountServices (CFArrayRef anItemList);


/*
 * Utility functions
 */
void RemoveItemFromWaitingList(StartupContext aStartupContext, CFMutableDictionaryRef anItem); 
void AddItemToFailedList(StartupContext aStartupContext, CFMutableDictionaryRef anItem);

/*
 * Run the startup item.
 */
int StartupItemRun   (CFMutableDictionaryRef aStatusDict, CFMutableDictionaryRef anItem, Action  anAction);
void StartupItemExit (CFMutableDictionaryRef aStatusDict, CFMutableDictionaryRef anItem, Boolean aSuccess);		     
void StartupItemSetStatus(CFMutableDictionaryRef aStatusDict, CFMutableDictionaryRef anItem, CFStringRef aServiceName, Boolean aSuccess, Boolean aReplaceFlag);

/*
 * Check whether file was created before boot and has proper permissions to run.
 */
bool StartupItemSecurityCheck(const char *aPath);

#endif /* _StartupItems_H_ */
