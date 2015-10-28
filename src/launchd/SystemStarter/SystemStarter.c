/**
 * System Starter main
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

#include <IOKit/IOKitLib.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/stat.h>
#include <paths.h>
#include <unistd.h>
#include <crt_externs.h>
#include <fcntl.h>
#include <syslog.h>
#include <assert.h>
#include <CoreFoundation/CoreFoundation.h>
#include <DiskArbitration/DiskArbitration.h>
#include <DiskArbitration/DiskArbitrationPrivate.h>
#include <NSSystemDirectories.h>
#include "IPC.h"
#include "StartupItems.h"
#include "SystemStarter.h"
#include "SystemStarterIPC.h"

bool gDebugFlag = false;
bool gVerboseFlag = false;
bool gNoRunFlag = false;

static void     usage(void) __attribute__((noreturn));
static int      system_starter(Action anAction, const char *aService);
static void	displayErrorMessages(StartupContext aStartupContext, Action anAction);
static pid_t	fwexec(const char *cmd, ...) __attribute__((sentinel));
static void	autodiskmount(void);
static void	dummy_sig(int signo __attribute__((unused)))
{
}

int 
main(int argc, char *argv[])
{
	struct kevent	kev;
	Action          anAction = kActionStart;
	int             ch, r, kq = kqueue();

	assert(kq  != -1);

	EV_SET(&kev, SIGTERM, EVFILT_SIGNAL, EV_ADD, 0, 0, 0);
	r = kevent(kq, &kev, 1, NULL, 0, NULL);
	assert(r != -1);
	signal(SIGTERM, dummy_sig);

	while ((ch = getopt(argc, argv, "gvxirdDqn?")) != -1) {
		switch (ch) {
		case 'v':
			gVerboseFlag = true;
			break;
		case 'x':
		case 'g':
		case 'r':
		case 'q':
			break;
		case 'd':
		case 'D':
			gDebugFlag = true;
			break;
		case 'n':
			gNoRunFlag = true;
			break;
		case '?':
		default:
			usage();
			break;
		}
	}
	argc -= optind;
	argv += optind;

	if (argc > 2) {
		usage();
	}

	openlog(getprogname(), LOG_PID|LOG_CONS|(gDebugFlag ? LOG_PERROR : 0), LOG_DAEMON);
	if (gDebugFlag) {
		setlogmask(LOG_UPTO(LOG_DEBUG));
	} else if (gVerboseFlag) {
		setlogmask(LOG_UPTO(LOG_INFO));
	} else {
		setlogmask(LOG_UPTO(LOG_NOTICE));
	}

	if (!gNoRunFlag && (getuid() != 0)) {
		syslog(LOG_ERR, "must be root to run");
		exit(EXIT_FAILURE);
	}

	if (argc > 0) {
		if (strcmp(argv[0], "start") == 0) {
			anAction = kActionStart;
		} else if (strcmp(argv[0], "stop") == 0) {
			anAction = kActionStop;
		} else if (strcmp(argv[0], "restart") == 0) {
			anAction = kActionRestart;
		} else {
			usage();
		}
	}

	if (argc == 2) {
		exit(system_starter(anAction, argv[1]));
	}

	unlink(kFixerPath);

	mach_timespec_t w = { 600, 0 };
	kern_return_t kr;

	/*
	 * Too many old StartupItems had implicit dependancies on "Network" via
	 * other StartupItems that are now no-ops.
	 *
	 * SystemStarter is not on the critical path for boot up, so we'll
	 * stall here to deal with this legacy dependancy problem.
	 */

	if ((kr = IOKitWaitQuiet(kIOMasterPortDefault, &w)) != kIOReturnSuccess) {
		syslog(LOG_NOTICE, "IOKitWaitQuiet: %d\n", kr);
	}

	fwexec("/usr/sbin/ipconfig", "waitall", NULL);
	autodiskmount(); /* wait for Disk Arbitration to report idle */

	system_starter(kActionStart, NULL);

	if (StartupItemSecurityCheck("/etc/rc.local")) {
		fwexec(_PATH_BSHELL, "/etc/rc.local", NULL);
	}

	CFNotificationCenterPostNotificationWithOptions(
			CFNotificationCenterGetDistributedCenter(),
			CFSTR("com.apple.startupitems.completed"),
			NULL, NULL,
			kCFNotificationDeliverImmediately | kCFNotificationPostToAllSessions);

	r = kevent(kq, NULL, 0, &kev, 1, NULL);
	assert(r != -1);
	assert(kev.filter == EVFILT_SIGNAL && kev.ident == SIGTERM);

	if (StartupItemSecurityCheck("/etc/rc.shutdown.local")) {
		fwexec(_PATH_BSHELL, "/etc/rc.shutdown.local", NULL);
	}

	system_starter(kActionStop, NULL);

	exit(EXIT_SUCCESS);
}


/**
 * checkForActivity checks to see if any items have completed since the last invokation.
 * If not, a message is displayed showing what item(s) are being waited on.
 **/
static void 
checkForActivity(StartupContext aStartupContext)
{
	static CFIndex  aLastStatusDictionaryCount = -1;
	static CFStringRef aWaitingForString = NULL;

	if (aStartupContext && aStartupContext->aStatusDict) {
		CFIndex         aCount = CFDictionaryGetCount(aStartupContext->aStatusDict);

		if (!aWaitingForString) {
			aWaitingForString = CFSTR("Waiting for %@");
		}
		if (aLastStatusDictionaryCount == aCount) {
			CFArrayRef aRunningList = StartupItemListCreateFromRunning(aStartupContext->aWaitingList);
			if (aRunningList && CFArrayGetCount(aRunningList) > 0) {
				CFMutableDictionaryRef anItem = (CFMutableDictionaryRef) CFArrayGetValueAtIndex(aRunningList, 0);
				CFStringRef     anItemDescription = StartupItemCreateDescription(anItem);
				CFStringRef     aString = aWaitingForString && anItemDescription ?
				CFStringCreateWithFormat(NULL, NULL, aWaitingForString, anItemDescription) : NULL;

				if (aString) {
					CF_syslog(LOG_INFO, CFSTR("%@"), aString);
					CFRelease(aString);
				}
				if (anItemDescription)
					CFRelease(anItemDescription);
			}
			if (aRunningList)
				CFRelease(aRunningList);
		}
		aLastStatusDictionaryCount = aCount;
	}
}

/*
 * print out any error messages to the log regarding non starting StartupItems
 */
void 
displayErrorMessages(StartupContext aStartupContext, Action anAction)
{
	if (aStartupContext->aFailedList && CFArrayGetCount(aStartupContext->aFailedList) > 0) {
		CFIndex         anItemCount = CFArrayGetCount(aStartupContext->aFailedList);
		CFIndex         anItemIndex;


		syslog(LOG_WARNING, "The following StartupItems failed to %s properly:", (anAction == kActionStart) ? "start" : "stop");

		for (anItemIndex = 0; anItemIndex < anItemCount; anItemIndex++) {
			CFMutableDictionaryRef anItem = (CFMutableDictionaryRef) CFArrayGetValueAtIndex(aStartupContext->aFailedList, anItemIndex);
			CFStringRef     anErrorDescription = CFDictionaryGetValue(anItem, kErrorKey);
			CFStringRef     anItemPath = CFDictionaryGetValue(anItem, kBundlePathKey);

			if (anItemPath) {
				CF_syslog(LOG_WARNING, CFSTR("%@"), anItemPath);
			}
			if (anErrorDescription) {
				CF_syslog(LOG_WARNING, CFSTR(" - %@"), anErrorDescription);
			} else {
				CF_syslog(LOG_WARNING, CFSTR(" - %@"), kErrorInternal);
			}
		}
	}
	if (CFArrayGetCount(aStartupContext->aWaitingList) > 0) {
		CFIndex         anItemCount = CFArrayGetCount(aStartupContext->aWaitingList);
		CFIndex         anItemIndex;

		syslog(LOG_WARNING, "The following StartupItems were not attempted due to failure of a required service:");

		for (anItemIndex = 0; anItemIndex < anItemCount; anItemIndex++) {
			CFMutableDictionaryRef anItem = (CFMutableDictionaryRef) CFArrayGetValueAtIndex(aStartupContext->aWaitingList, anItemIndex);
			CFStringRef     anItemPath = CFDictionaryGetValue(anItem, kBundlePathKey);
			if (anItemPath) {
				CF_syslog(LOG_WARNING, CFSTR("%@"), anItemPath);
			}
		}
	}
}


static int 
system_starter(Action anAction, const char *aService_cstr)
{
	CFStringRef     aService = NULL;
	NSSearchPathDomainMask aMask;

	if (aService_cstr)
		aService = CFStringCreateWithCString(kCFAllocatorDefault, aService_cstr, kCFStringEncodingUTF8);

	StartupContext  aStartupContext = (StartupContext) malloc(sizeof(struct StartupContextStorage));
	if (!aStartupContext) {
		syslog(LOG_ERR, "Not enough memory to allocate startup context");
		return (1);
	}
	if (gDebugFlag && gNoRunFlag)
		sleep(1);

	/**
         * Get a list of Startup Items which are in /Local and /System.
         * We can't search /Network yet because the network isn't up.
         **/
	aMask = NSSystemDomainMask | NSLocalDomainMask;

	aStartupContext->aWaitingList = StartupItemListCreateWithMask(aMask);
	aStartupContext->aFailedList = NULL;
	aStartupContext->aStatusDict = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks,
					  &kCFTypeDictionaryValueCallBacks);
	aStartupContext->aServicesCount = 0;
	aStartupContext->aRunningCount = 0;

	if (aService) {
		CFMutableArrayRef aDependentsList = StartupItemListCreateDependentsList(aStartupContext->aWaitingList, aService, anAction);

		if (aDependentsList) {
			CFRelease(aStartupContext->aWaitingList);
			aStartupContext->aWaitingList = aDependentsList;
		} else {
			CF_syslog(LOG_ERR, CFSTR("Unknown service: %@"), aService);
			return (1);
		}
	}
	aStartupContext->aServicesCount = StartupItemListCountServices(aStartupContext->aWaitingList);

	/**
         * Do the run loop
         **/
	while (1) {
		CFMutableDictionaryRef anItem = StartupItemListGetNext(aStartupContext->aWaitingList, aStartupContext->aStatusDict, anAction);

		if (anItem) {
			int             err = StartupItemRun(aStartupContext->aStatusDict, anItem, anAction);
			if (!err) {
				++aStartupContext->aRunningCount;
				MonitorStartupItem(aStartupContext, anItem);
			} else {
				/* add item to failed list */
				AddItemToFailedList(aStartupContext, anItem);

				/* Remove the item from the waiting list. */
				RemoveItemFromWaitingList(aStartupContext, anItem);
			}
		} else {
			/*
			 * If no item was selected to run, and if no items
			 * are running, startup is done.
			 */
			if (aStartupContext->aRunningCount == 0) {
				syslog(LOG_DEBUG, "none left");
				break;
			}
			/*
			 * Process incoming IPC messages and item
			 * terminations
			 */
			switch (CFRunLoopRunInMode(kCFRunLoopDefaultMode, 3.0, true)) {
			case kCFRunLoopRunTimedOut:
				checkForActivity(aStartupContext);
				break;
			case kCFRunLoopRunFinished:
				break;
			case kCFRunLoopRunStopped:
				break;
			case kCFRunLoopRunHandledSource:
				break;
			default:
				/* unknown return value */
				break;
			}
		}
	}

	/**
         * Good-bye.
         **/
	displayErrorMessages(aStartupContext, anAction);

	/* clean up  */
	if (aStartupContext->aStatusDict)
		CFRelease(aStartupContext->aStatusDict);
	if (aStartupContext->aWaitingList)
		CFRelease(aStartupContext->aWaitingList);
	if (aStartupContext->aFailedList)
		CFRelease(aStartupContext->aFailedList);

	free(aStartupContext);
	return (0);
}

void 
CF_syslog(int level, CFStringRef message,...)
{
	char            buf[8192];
	CFStringRef     cooked_msg;
	va_list         ap;

	va_start(ap, message);
	cooked_msg = CFStringCreateWithFormatAndArguments(NULL, NULL, message, ap);
	va_end(ap);

	if (CFStringGetCString(cooked_msg, buf, sizeof(buf), kCFStringEncodingUTF8))
		syslog(level, "%s", buf);

	CFRelease(cooked_msg);
}

static void 
usage(void)
{
	fprintf(stderr, "usage: %s [-vdqn?] [ <action> [ <item> ] ]\n"
	"\t<action>: action to take (start|stop|restart); default is start\n"
		"\t<item>  : name of item to act on; default is all items\n"
		"options:\n"
		"\t-v: verbose startup\n"
		"\t-d: print debugging output\n"
		"\t-q: be quiet (disable debugging output)\n"
	     "\t-n: don't actually perform action on items (pretend mode)\n"
		"\t-?: show this help\n",
		getprogname());
	exit(EXIT_FAILURE);
}

pid_t
fwexec(const char *cmd, ...)
{
	const char *argv[100] = { cmd };
	va_list ap;
	int wstatus, i = 1;
	pid_t p;

	va_start(ap, cmd);
	do {
		argv[i] = va_arg(ap, char *);
	} while (argv[i++]);
	va_end(ap);

	switch ((p = fork())) {
	case -1:
		return -1;
	case 0:
		execvp(argv[0], (char *const *)argv);
		_exit(EXIT_FAILURE);
		break;
	default:
		if (waitpid(p, &wstatus, 0) == -1) {
			return -1;
		} else if (WIFEXITED(wstatus)) {
			if (WEXITSTATUS(wstatus) == 0) {
				return 0;
			} else {
				syslog(LOG_WARNING, "%s exit status: %d", argv[0], WEXITSTATUS(wstatus));
			}
		} else {
			/* must have died due to signal */
			syslog(LOG_WARNING, "%s died: %s", argv[0], strsignal(WTERMSIG(wstatus)));
		}
		break;
	}

	return -1;
}

static void
autodiskmount_idle(void* context __attribute__((unused)))
{
	CFRunLoopStop(CFRunLoopGetCurrent());
}

static void
autodiskmount(void)
{
	DASessionRef session = DASessionCreate(NULL);
	if (session) {
		DASessionScheduleWithRunLoop(session, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
		DARegisterIdleCallback(session, autodiskmount_idle, NULL);
		CFRunLoopRun();
		CFRelease(session);
	}
}
