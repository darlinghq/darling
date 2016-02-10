/**
 * IPC.h - System Starter IPC routines
 * Wilfredo Sanchez  | wsanchez@opensource.apple.com
 * Kevin Van Vechten | kevinvv@uclink4.berkeley.edu
 * $Apple$
 **
 * Copyright (c) 1999-2001 Apple Computer, Inc. All rights reserved.
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

#ifndef _IPC_H_
#define _IPC_H_

#include "SystemStarter.h"

/**
 * Monitor a startup item task.  Creates a mach port and uses the
 * invalidation callback to notify system starter when the process 
 * terminates.
 **/
void MonitorStartupItem (StartupContext aStartupContext, CFMutableDictionaryRef anItem);

#endif /* _IPC_H_ */
