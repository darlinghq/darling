/**
 * IPC.c - System Starter IPC routines
 * Wilfredo Sanchez  | wsanchez@opensource.apple.com
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

#include <sys/wait.h>
#include <mach/mach.h>
#include <mach/message.h>
#include <mach/mach_error.h>
#include <CoreFoundation/CoreFoundation.h>
#include <syslog.h>

#include "bootstrap.h"

#include "IPC.h"
#include "StartupItems.h"
#include "SystemStarter.h"
#include "SystemStarterIPC.h"

/* Structure to pass StartupContext and anItem to the termination handler. */
typedef struct TerminationContextStorage {
	StartupContext  aStartupContext;
	CFMutableDictionaryRef anItem;
}              *TerminationContext;

/**
 * A CFMachPort invalidation callback that records the termination of
 * a startup item task.  Stops the current run loop to give system_starter
 * another go at running items.
 **/
static void 
startupItemTerminated(CFMachPortRef aMachPort, void *anInfo)
{
	TerminationContext aTerminationContext = (TerminationContext) anInfo;

	if (aMachPort) {
		mach_port_deallocate(mach_task_self(), CFMachPortGetPort(aMachPort));
	}
	if (aTerminationContext && aTerminationContext->anItem) {
		pid_t           aPID = 0;
		pid_t           rPID = 0;
		int             aStatus = 0;
		CFMutableDictionaryRef anItem = aTerminationContext->anItem;
		StartupContext  aStartupContext = aTerminationContext->aStartupContext;

		/* Get the exit status */
		if (anItem) {
			aPID = StartupItemGetPID(anItem);
			if (aPID > 0)
				rPID = waitpid(aPID, &aStatus, 0);
		}
		if (aStartupContext) {
			--aStartupContext->aRunningCount;

			/* Record the item's status */
			if (aStartupContext->aStatusDict) {
				StartupItemExit(aStartupContext->aStatusDict, anItem, (WIFEXITED(aStatus) && WEXITSTATUS(aStatus) == 0));
				if (aStatus) {
					CF_syslog(LOG_WARNING, CFSTR("%@ (%d) did not complete successfully"), CFDictionaryGetValue(anItem, CFSTR("Description")), aPID);
				} else {
					CF_syslog(LOG_DEBUG, CFSTR("Finished %@ (%d)"), CFDictionaryGetValue(anItem, CFSTR("Description")), aPID);
				}
			}
			/*
			 * If the item failed to start, then add it to the
			 * failed list
			 */
			if (WEXITSTATUS(aStatus) || WTERMSIG(aStatus) || WCOREDUMP(aStatus)) {
				CFDictionarySetValue(anItem, kErrorKey, kErrorReturnNonZero);
				AddItemToFailedList(aStartupContext, anItem);
			}
			/*
			 * Remove the item from the waiting list regardless
			 * if it was successful or it failed.
			 */
			RemoveItemFromWaitingList(aStartupContext, anItem);
		}
	}
	if (aTerminationContext)
		free(aTerminationContext);
}

void 
MonitorStartupItem(StartupContext aStartupContext, CFMutableDictionaryRef anItem)
{
	pid_t           aPID = StartupItemGetPID(anItem);
	if (anItem && aPID > 0) {
		mach_port_t     aPort;
		kern_return_t   aResult;
		CFMachPortContext aContext;
		CFMachPortRef   aMachPort;
		CFRunLoopSourceRef aSource;
		TerminationContext aTerminationContext = (TerminationContext) malloc(sizeof(struct TerminationContextStorage));

		aTerminationContext->aStartupContext = aStartupContext;
		aTerminationContext->anItem = anItem;

		aContext.version = 0;
		aContext.info = aTerminationContext;
		aContext.retain = 0;
		aContext.release = 0;

		if ((aResult = task_name_for_pid(mach_task_self(), aPID, &aPort)) != KERN_SUCCESS)
			goto out_bad;

		if (!(aMachPort = CFMachPortCreateWithPort(NULL, aPort, NULL, &aContext, NULL)))
			goto out_bad;

		if (!(aSource = CFMachPortCreateRunLoopSource(NULL, aMachPort, 0))) {
			CFRelease(aMachPort);
			goto out_bad;
		}
		CFMachPortSetInvalidationCallBack(aMachPort, startupItemTerminated);
		CFRunLoopAddSource(CFRunLoopGetCurrent(), aSource, kCFRunLoopCommonModes);
		CFRelease(aSource);
		CFRelease(aMachPort);
		return;
out_bad:
		/*
		 * The assumption is something failed, the task already
		 * terminated.
		 */
		startupItemTerminated(NULL, aTerminationContext);
	}
}
