/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

// Delivery of Apple Events to applications, shared by LaunchServices' open calls. Internal to LaunchServices.
#ifndef _LS_DELIVER_H_
#define _LS_DELIVER_H_

#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// The running process of the application bundle that can receive Apple Events, or 0.
pid_t _LSFindRunningApplication(CFURLRef appURL);

// Gives a copy of the event, addressed to the process, as LaunchServices delivers it. A process that was just
// launched (justLaunched) gets it as its first event when it checks in; a running one gets it at once.
OSStatus _LSDeliverEvent(const AppleEvent *event, pid_t pid, Boolean justLaunched);

// Tells a running application it was opened again (kAEReopenApplication).
OSStatus _LSDeliverReopenEvent(pid_t pid);

#ifdef __cplusplus
}
#endif

#endif
