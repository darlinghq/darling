/*
 * Copyright (c) 2000-2005, 2008-2014 Apple Inc. All rights reserved.
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

/*
 * Modification History
 *
 * June 1, 2001			Allan Nathanson <ajn@apple.com>
 * - public API conversion
 *
 * November 9, 2000		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */

#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <unistd.h>

#include "scutil.h"
#include "notifications.h"


static int			osig;
static struct sigaction		*oact	= NULL;


static char *
elapsed()
{
	int			n;
	static char		str[128];
	struct tm		tm_diff;
	struct tm		tm_now;
	struct timeval		tv_diff;
	struct timeval		tv_now;
	static struct timeval	tv_then	= { 0, 0 };

	(void)gettimeofday(&tv_now, NULL);

	(void)localtime_r(&tv_now.tv_sec, &tm_now);

	timersub(&tv_now, &tv_then, &tv_diff);
	(void)localtime_r(&tv_diff.tv_sec, &tm_diff);
	n = snprintf(str, sizeof(str), "%2d:%02d:%02d.%03d",
		     tm_now.tm_hour,
		     tm_now.tm_min,
		     tm_now.tm_sec,
		     tv_now.tv_usec / 1000);
	if (((tv_then.tv_sec != 0) || (tv_then.tv_usec != 0)) && (n < sizeof(str))) {
		snprintf(&str[n], sizeof(str) - n, " (+%ld.%03d)",
			 tv_diff.tv_sec,
			 tv_diff.tv_usec / 1000);
	}

	tv_then = tv_now;
	return str;
}


static CFComparisonResult
sort_keys(const void *p1, const void *p2, void *context) {
	CFStringRef key1 = (CFStringRef)p1;
	CFStringRef key2 = (CFStringRef)p2;
	return CFStringCompare(key1, key2, 0);
}


__private_extern__
void
storeCallback(SCDynamicStoreRef store, CFArrayRef changedKeys, void *info)
{
	int		i;
	CFIndex		n;

	SCPrint(TRUE, stdout, CFSTR("notification callback (store address = %p).\n"), store);

	n = CFArrayGetCount(changedKeys);
	if (n > 0) {
		for (i = 0; i < n; i++) {
			SCPrint(TRUE,
				stdout,
				CFSTR("  %s changedKey [%d] = %@\n"),
				elapsed(),
				i,
				CFArrayGetValueAtIndex(changedKeys, i));
		}
	} else {
		SCPrint(TRUE, stdout, CFSTR("  no changed key's.\n"));
	}

	return;
}


__private_extern__
void
do_notify_list(int argc, char **argv)
{
	int			i;
	CFArrayRef		list;
	CFIndex			listCnt;
	Boolean			isRegex = FALSE;
	CFMutableArrayRef	sortedList;

	if (store == NULL) {
		SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(kSCStatusNoStoreSession));
		return;
	}

	if (argc == 1)
		isRegex = TRUE;

	list = isRegex ? watchedPatterns : watchedKeys;
	if (!list) {
		SCPrint(TRUE,
			stdout,
			CFSTR("  no notifier %s.\n"),
			isRegex ? "patterns" : "keys");
		return;
	}

	listCnt = CFArrayGetCount(list);
	sortedList = CFArrayCreateMutableCopy(NULL, listCnt, list);
	CFArraySortValues(sortedList,
			  CFRangeMake(0, listCnt),
			  sort_keys,
			  NULL);

	if (listCnt > 0) {
		for (i = 0; i < listCnt; i++) {
			SCPrint(TRUE,
				stdout,
				CFSTR("  notifier %s [%d] = %@\n"),
				isRegex ? "pattern" : "key",
				i,
				CFArrayGetValueAtIndex(sortedList, i));
		}
	} else {
		SCPrint(TRUE,
			stdout,
			CFSTR("  no notifier %s.\n"),
			isRegex ? "patterns" : "keys");
	}
	CFRelease(sortedList);

	return;
}


__private_extern__
void
do_notify_add(int argc, char **argv)
{
	CFStringRef		key;
	CFMutableArrayRef	keys;
	Boolean			isRegex = FALSE;

	if (store == NULL) {
		SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(kSCStatusNoStoreSession));
		return;
	}

	key = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);

	if (argc == 2)
		isRegex = TRUE;

	keys = isRegex ? watchedPatterns : watchedKeys;
	if (CFArrayContainsValue(keys, CFRangeMake(0, CFArrayGetCount(keys)), key)) {
		SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(kSCStatusKeyExists));
		CFRelease(key);
		return;
	}

	CFArrayAppendValue(keys, key);
	CFRelease(key);

	if (!SCDynamicStoreSetNotificationKeys(store, watchedKeys, watchedPatterns)) {
		SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(SCError()));
	}

	return;
}


__private_extern__
void
do_notify_remove(int argc, char **argv)
{
	CFStringRef		key;
	CFMutableArrayRef	keys;
	CFIndex			i;
	Boolean			isRegex = FALSE;

	if (store == NULL) {
		SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(kSCStatusNoStoreSession));
		return;
	}

	key   = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);

	if (argc == 2)
		isRegex = TRUE;

	keys = isRegex ? watchedPatterns : watchedKeys;
	i = CFArrayGetFirstIndexOfValue(keys, CFRangeMake(0, CFArrayGetCount(keys)), key);
	CFRelease(key);

	if (i == kCFNotFound) {
		SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(kSCStatusNoKey));
		return;
	}

	CFArrayRemoveValueAtIndex(keys, i);

	if (!SCDynamicStoreSetNotificationKeys(store, watchedKeys, watchedPatterns)) {
		SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(SCError()));
	}

	return;
}


__private_extern__
void
do_notify_changes(int argc, char **argv)
{
	CFArrayRef	list;
	CFIndex		listCnt;
	int		i;

	list = SCDynamicStoreCopyNotifiedKeys(store);
	if (!list) {
		SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(SCError()));
		return;
	}

	listCnt = CFArrayGetCount(list);
	if (listCnt > 0) {
		for (i = 0; i < listCnt; i++) {
			SCPrint(TRUE,
				stdout,
				CFSTR("  %s changedKey [%d] = %@\n"),
				elapsed(),
				i,
				CFArrayGetValueAtIndex(list, i));
		}
	} else {
		SCPrint(TRUE, stdout, CFSTR("  no changedKey's.\n"));
	}
	CFRelease(list);

	return;
}


static void *
_watcher(void *arg)
{
	notifyRl = CFRunLoopGetCurrent();
	if (notifyRl == NULL) {
		SCPrint(TRUE, stdout, CFSTR("  CFRunLoopGetCurrent() failed\n"));
		return NULL;
	}

	if (doDispatch) {
		if (!SCDynamicStoreSetDispatchQueue(store, dispatch_get_main_queue())) {
			SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(SCError()));
			notifyRl = NULL;
			return NULL;
		}
		notifyRls = (CFRunLoopSourceRef)kCFNull;
	} else {
		notifyRls = SCDynamicStoreCreateRunLoopSource(NULL, store, 0);
		if (notifyRls == NULL) {
			SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(SCError()));
			notifyRl = NULL;
			return NULL;
		}
		CFRunLoopAddSource(notifyRl, notifyRls, kCFRunLoopDefaultMode);
	}

	pthread_setname_np("n.watch");
	CFRunLoopRun();
	notifyRl = NULL;
	return NULL;
}

__private_extern__
void
do_notify_watch(int argc, char **argv)
{
	pthread_attr_t	tattr;
	pthread_t	tid;

	if (notifyRl != NULL) {
		SCPrint(TRUE, stdout, CFSTR("already active\n"));
		return;
	}

	pthread_attr_init(&tattr);
	pthread_attr_setscope(&tattr, PTHREAD_SCOPE_SYSTEM);
	pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
//      pthread_attr_setstacksize(&tattr, 96 * 1024); // each thread gets a 96K stack
	pthread_create(&tid, &tattr, _watcher, NULL);
	pthread_attr_destroy(&tattr);

	return;
}


__private_extern__
void
do_notify_wait(int argc, char **argv)
{
	if (!SCDynamicStoreNotifyWait(store)) {
		SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(SCError()));
		return;
	}

	return;
}


__private_extern__
void
do_notify_file(int argc, char **argv)
{
	int32_t		reqID = 0;
	int		fd;
	union {
		char	data[4];
		int32_t	gotID;
	} buf;
	char		*bufPtr;
	int		needed;

	if (argc == 1) {
		if ((sscanf(argv[0], "%d", &reqID) != 1)) {
			SCPrint(TRUE, stdout, CFSTR("invalid identifier.\n"));
			return;
		}
	}

	if (!SCDynamicStoreNotifyFileDescriptor(store, reqID, &fd)) {
		SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(SCError()));
		return;
	}

	bzero(buf.data, sizeof(buf.data));
	bufPtr = &buf.data[0];
	needed = sizeof(buf.gotID);
	while (needed > 0) {
		ssize_t	got;

		got = read(fd, bufPtr, needed);
		if (got == -1) {
			/* if error detected */
			SCPrint(TRUE, stdout, CFSTR("read() failed: %s.\n"), strerror(errno));
			break;
		}

		if (got == 0) {
			/* if end of file detected */
			SCPrint(TRUE, stdout, CFSTR("read(): detected end of file.\n"));
			break;
		}

		SCPrint(TRUE, stdout, CFSTR("Received %ld bytes.\n"), got);
		bufPtr += got;
		needed -= got;
	}

	if (needed != sizeof(buf.gotID)) {
		SCPrint(TRUE, stdout, CFSTR("  Received notification, identifier = %d.\n"), buf.gotID);
	}

	/* report the keys that changed */
	do_notify_changes(0, NULL);

	/* this utility only allows processes one notification per "n.file" request */
	(void) SCDynamicStoreNotifyCancel(store);

	(void) close(fd);	/* close my side of the file descriptor */

	return;
}


static void
signalCatcher(int signum)
{
	static int	n = 0;

	SCPrint(TRUE, stdout, CFSTR("Received sig%s (#%d).\n"), sys_signame[signum], n++);
	return;
}


__private_extern__
void
do_notify_signal(int argc, char **argv)
{
	int			sig;
	pid_t			pid;
	struct sigaction	nact;

	if (isdigit(*argv[0])) {
		if ((sscanf(argv[0], "%d", &sig) != 1) || (sig <= 0) || (sig >= NSIG)) {
			SCPrint(TRUE, stdout, CFSTR("signal must be in the range of 1 .. %d.\n"), NSIG-1);
			return;
		}
	} else {
		for (sig = 1; sig < NSIG; sig++) {
			if (strcasecmp(argv[0], sys_signame[sig]) == 0)
				break;
		}
		if (sig >= NSIG) {
			CFMutableStringRef	str;

			SCPrint(TRUE, stdout, CFSTR("Signal must be one of the following:\n"));

			str = CFStringCreateMutable(NULL, 0);
			for (sig = 1; sig < NSIG; sig++) {
				CFStringAppendFormat(str, NULL, CFSTR(" %-6s"), sys_signame[sig]);
				if ((sig % 10) == 0) {
					CFStringAppendFormat(str, NULL, CFSTR("\n"));
				}
			}
			if ((sig % 10) != 0) {
				CFStringAppendFormat(str, NULL, CFSTR("\n"));
			}
			SCPrint(TRUE, stdout, CFSTR("%@"), str);
			CFRelease(str);
			return;
		}

	}

	if ((argc != 2) || (sscanf(argv[1], "%d", &pid) != 1)) {
		pid = getpid();
	}

	if (oact != NULL) {
		(void) sigaction(osig, oact, NULL);	/* restore original signal handler */
	} else {
		oact = malloc(sizeof(struct sigaction));
	}

	nact.sa_handler = signalCatcher;
	sigemptyset(&nact.sa_mask);
	nact.sa_flags = SA_RESTART;
	(void) sigaction(sig, &nact, oact);
	osig = sig;
	SCPrint(TRUE, stdout, CFSTR("signal handler started.\n"));

	if (!SCDynamicStoreNotifySignal(store, pid, sig)) {
		SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(SCError()));
		return;
	}

	return;
}


__private_extern__
void
do_notify_cancel(int argc, char **argv)
{
	if (notifyRls != NULL) {
		if (doDispatch) {
			if (!SCDynamicStoreSetDispatchQueue(store, NULL)) {
				SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(SCError()));
				return;
			}
		} else {
			CFRunLoopSourceInvalidate(notifyRls);
			CFRelease(notifyRls);
		}
		notifyRls = NULL;
	} else {
		if (!SCDynamicStoreNotifyCancel(store)) {
			SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(SCError()));
			return;
		}
	}

	if (notifyRl != NULL) {
		CFRunLoopStop(notifyRl);
	}

	if (oact != NULL) {
		(void) sigaction(osig, oact, NULL);	/* restore original signal handler */
		free(oact);
		oact = NULL;
	}

	return;
}
