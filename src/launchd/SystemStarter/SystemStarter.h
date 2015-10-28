/**
 * SystemStarter.h - System Starter driver
 * Wilfredo Sanchez | wsanchez@opensource.apple.com
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

#ifndef _SYSTEM_STARTER_H_
#define _SYSTEM_STARTER_H_

/* Structure to pass common objects from system_starter to the IPC handlers */
typedef struct StartupContextStorage {
    CFMutableArrayRef           aWaitingList;
    CFMutableArrayRef           aFailedList;
    CFMutableDictionaryRef      aStatusDict;
    int                         aServicesCount;
    int                         aRunningCount;
} *StartupContext;

#define kFixerDir	"/var/db/fixer"
#define kFixerPath	"/var/db/fixer/StartupItems"

/* Action types */
typedef enum {
  kActionNone = 0,
  kActionStart,
  kActionStop,
  kActionRestart
} Action;

void CF_syslog(int level, CFStringRef message, ...);
extern bool gVerboseFlag;

#endif /* _SYSTEM_STARTER_H_ */
