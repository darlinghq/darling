/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

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

#include <LaunchServices/LaunchServices.h>
#include <LaunchServices/LaunchServicesPriv.h>
#include "LSHostOpen.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <launch_priv.h>
#include <errno.h>
#include <os/log.h>
#include <xpc/xpc.h>
#include <dispatch/dispatch.h>
#include <string.h>
#include "LSDProtocol.h"

static void get_main_executable_info(CFURLRef appURL, CFStringRef *exec, CFStringRef *bundleName);
static Boolean is_file_url(CFURLRef url);
static OSStatus launch_application_pid(CFURLRef appPath, pid_t *outPid);

// One request to launchservicesd and its reply, or NULL when it cannot be reached in time.
static xpc_object_t lsd_request(xpc_object_t request, int64_t timeoutMs)
{
	static dispatch_once_t once;
	static xpc_connection_t connection;

	dispatch_once(&once, ^{
		const char *name = getenv(LSD_SERVICE_NAME_OVERRIDE_ENV);

		connection = xpc_connection_create_mach_service(name != NULL ? name : LSD_SERVICE_NAME,
			dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0), 0);
		if (connection == NULL)
			return;
		xpc_connection_set_event_handler(connection, ^(xpc_object_t event) {
			/* Errors need no action here: a request on a broken connection fails by itself. */
		});
		xpc_connection_resume(connection);
	});
	if (connection == NULL)
		return NULL;

	__block xpc_object_t answer = NULL;
	dispatch_semaphore_t done = dispatch_semaphore_create(0);

	xpc_connection_send_message_with_reply(connection, request, dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0),
		^(xpc_object_t reply) {
			if (xpc_get_type(reply) == XPC_TYPE_DICTIONARY)
				answer = xpc_retain(reply);
			dispatch_semaphore_signal(done);
		});
	if (dispatch_semaphore_wait(done, dispatch_time(DISPATCH_TIME_NOW, timeoutMs * NSEC_PER_MSEC)) != 0)
		return NULL;
	dispatch_release(done);
	return answer;
}

/*
 * Applications are registered in launchservicesd's database, which reads the bundle's Info.plist
 * (document types, URL schemes, ...). Registering is also how an application outside the monitored
 * directories becomes known to the lookups.
 */
OSStatus LSRegisterURL(CFURLRef inURL, Boolean inUpdate)
{
	char path[PATH_MAX];

	if (inURL == NULL || !is_file_url(inURL) || !CFURLGetFileSystemRepresentation(inURL, true, (UInt8 *) path, sizeof(path)))
		return paramErr;

	xpc_object_t request = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_string(request, LSD_KEY_OP, LSD_OP_REGISTER);
	xpc_dictionary_set_string(request, LSD_KEY_PATH, path);
	xpc_object_t reply = lsd_request(request, 30000);
	xpc_release(request);
	if (reply == NULL)
		return kLSServerCommunicationErr;

	OSStatus status = (OSStatus) xpc_dictionary_get_int64(reply, LSD_KEY_ERR);
	xpc_release(reply);
	return status;
}

static Boolean is_application_url(CFURLRef url)
{
	CFStringRef extension = CFURLCopyPathExtension(url);
	Boolean ret = FALSE;

	if (extension != NULL)
	{
		ret = CFStringCompare(extension, CFSTR("app"), kCFCompareCaseInsensitive) == kCFCompareEqualTo;
		CFRelease(extension);
	}
	return ret;
}

static Boolean is_file_url(CFURLRef url)
{
	CFStringRef scheme = CFURLCopyScheme(url);
	Boolean ret = FALSE;

	if (scheme != NULL)
	{
		ret = CFStringCompare(scheme, CFSTR("file"), kCFCompareCaseInsensitive) == kCFCompareEqualTo;
		CFRelease(scheme);
	}
	return ret;
}

static OSStatus launch_application(CFURLRef appURL, CFURLRef *outLaunchedURL)
{
	OSStatus ret = _LSLaunchApplication(appURL);

	if (ret == noErr && outLaunchedURL != NULL)
	{
		if (*outLaunchedURL != NULL)
			CFRelease(*outLaunchedURL);
		*outLaunchedURL = CFRetain(appURL);
	}
	return ret;
}

/* Largest wait for a running application to take an Apple event (60 ticks per second). */
#define APP_READY_TIMEOUT_TICKS (60 * 15)

/* The running process of an application that can receive Apple events, or 0. */
static pid_t find_running_instance(const char *bundleID)
{
	pid_t pid = 0;
	xpc_object_t request = xpc_dictionary_create(NULL, NULL, 0);

	xpc_dictionary_set_string(request, LSD_KEY_OP, LSD_OP_AE_LOOKUP);
	xpc_dictionary_set_string(request, LSD_KEY_BUNDLE_ID, bundleID);
	xpc_object_t reply = lsd_request(request, 5000);
	xpc_release(request);

	if (reply != NULL)
	{
		if (xpc_dictionary_get_int64(reply, LSD_KEY_ERR) == 0)
			pid = (pid_t) xpc_dictionary_get_int64(reply, LSD_KEY_PID);
		xpc_release(reply);
	}
	return pid;
}

/*
 * Gives the event to the process. A process that was just started (queue) is not waited for:
 * launchservicesd keeps the event and delivers it when the process checks in, as its first event.
 * A running process gets it at once.
 */
static OSStatus send_event(AppleEvent *event, pid_t pid, AEEventID eventID, Boolean queue)
{
	Size size = 0;
	void *flat;
	OSStatus status;

	if (!queue)
		return AESendMessage(event, NULL, kAENoReply, APP_READY_TIMEOUT_TICKS);

	AEFlattenDesc(event, NULL, 0, &size);
	flat = size > 0 ? malloc(size) : NULL;
	if (flat == NULL)
		return memFullErr;
	status = AEFlattenDesc(event, flat, size, &size);
	if (status == noErr)
	{
		xpc_object_t request = xpc_dictionary_create(NULL, NULL, 0);

		xpc_dictionary_set_string(request, LSD_KEY_OP, LSD_OP_AE_QUEUE);
		xpc_dictionary_set_int64(request, LSD_KEY_PID, pid);
		xpc_dictionary_set_int64(request, LSD_KEY_EVENT_ID, eventID);
		xpc_dictionary_set_data(request, LSD_KEY_EVENT, flat, size);

		xpc_object_t reply = lsd_request(request, 10000);

		xpc_release(request);
		if (reply == NULL)
			status = kLSServerCommunicationErr;
		else
		{
			status = (OSStatus) xpc_dictionary_get_int64(reply, LSD_KEY_ERR);
			xpc_release(reply);
		}
	}
	free(flat);
	return status;
}

/* Sends the URL to the process as a kAEGetURL event. */
static OSStatus send_get_url_event(pid_t pid, CFStringRef url, Boolean queue)
{
	AppleEvent event;
	AEDesc target;
	SInt32 targetPid = pid;
	char stack[1024];
	char *utf8 = stack;
	CFIndex length = CFStringGetMaximumSizeForEncoding(CFStringGetLength(url), kCFStringEncodingUTF8) + 1;
	OSStatus status;

	if (length > (CFIndex) sizeof(stack))
		utf8 = malloc(length);
	if (utf8 == NULL)
		return memFullErr;
	if (!CFStringGetCString(url, utf8, length, kCFStringEncodingUTF8))
	{
		status = paramErr;
		goto out;
	}

	status = AECreateDesc(typeKernelProcessID, &targetPid, sizeof(targetPid), &target);
	if (status != noErr)
		goto out;
	status = AECreateAppleEvent(kInternetEventClass, kAEGetURL, &target, kAutoGenerateReturnID, kAnyTransactionID, &event);
	AEDisposeDesc(&target);
	if (status != noErr)
		goto out;
	status = AEPutParamPtr(&event, keyDirectObject, typeUTF8Text, utf8, strlen(utf8));
	if (status == noErr)
		status = send_event(&event, pid, kAEGetURL, queue);
	AEDisposeDesc(&event);
out:
	if (utf8 != stack)
		free(utf8);
	return status;
}

/*
 * Sends the documents to the process as one kAEOpenDocuments event (kAEPrintDocuments to print
 * them), whose direct object is a list of file URLs.
 */
static OSStatus send_documents_event(pid_t pid, CFArrayRef urls, Boolean print, Boolean queue)
{
	AppleEvent event;
	AEDescList list;
	AEDesc target;
	SInt32 targetPid = pid;
	OSStatus status;

	status = AECreateDesc(typeKernelProcessID, &targetPid, sizeof(targetPid), &target);
	if (status != noErr)
		return status;
	status = AECreateAppleEvent(kCoreEventClass, print ? kAEPrintDocuments : kAEOpenDocuments, &target, kAutoGenerateReturnID, kAnyTransactionID, &event);
	AEDisposeDesc(&target);
	if (status != noErr)
		return status;
	status = AECreateList(NULL, 0, false, &list);
	for (CFIndex i = 0; status == noErr && i < CFArrayGetCount(urls); i++)
	{
		CFStringRef string = CFURLGetString(CFArrayGetValueAtIndex(urls, i));
		char stack[1024];
		char *utf8 = stack;
		CFIndex length = string != NULL ? CFStringGetMaximumSizeForEncoding(CFStringGetLength(string), kCFStringEncodingUTF8) + 1 : 0;

		if (length == 0)
		{
			status = paramErr;
			break;
		}
		if (length > (CFIndex) sizeof(stack))
			utf8 = malloc(length);
		if (utf8 == NULL)
			status = memFullErr;
		else if (!CFStringGetCString(string, utf8, length, kCFStringEncodingUTF8))
			status = paramErr;
		else
			status = AEPutPtr(&list, 0, typeFileURL, utf8, strlen(utf8));
		if (utf8 != stack)
			free(utf8);
	}
	if (status == noErr)
		status = AEPutParamDesc(&event, keyDirectObject, &list);
	AEDisposeDesc(&list);
	if (status == noErr)
		status = send_event(&event, pid, print ? kAEPrintDocuments : kAEOpenDocuments, queue);
	AEDisposeDesc(&event);
	return status;
}

/*
 * Opens items with an application: file URLs go to it together as a kAEOpenDocuments event (or
 * kAEPrintDocuments, with print), every other URL as its own kAEGetURL event. A running instance
 * receives them. Otherwise the application is launched (always, with kLSLaunchNewInstance) and the
 * events are queued in launchservicesd without waiting: the new process gets them as its first events
 * when it checks in, and a process that never does is simply launched.
 */
static OSStatus deliver_urls(CFURLRef appURL, CFArrayRef urls, Boolean newInstance, Boolean print, CFURLRef *outLaunchedURL)
{
	CFBundleRef bundle = CFBundleCreate(kCFAllocatorDefault, appURL);
	CFStringRef identifier = bundle != NULL ? CFBundleGetIdentifier(bundle) : NULL;
	char bundleID[512] = "";
	pid_t pid = 0;
	Boolean started = false;
	OSStatus status;

	if (identifier != NULL)
		CFStringGetCString(identifier, bundleID, sizeof(bundleID), kCFStringEncodingUTF8);
	if (bundle != NULL)
		CFRelease(bundle);

	if (!newInstance && bundleID[0] != 0)
		pid = find_running_instance(bundleID);
	if (pid == 0)
	{
		status = launch_application_pid(appURL, &pid);
		if (status != noErr)
			return status;
		started = true;
	}

	CFMutableArrayRef files = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);

	for (CFIndex i = 0; i < CFArrayGetCount(urls); i++)
	{
		CFURLRef item = CFArrayGetValueAtIndex(urls, i);
		CFStringRef string = CFURLGetString(item);

		if (is_file_url(item))
		{
			CFArrayAppendValue(files, item);
			continue;
		}
		status = string != NULL ? send_get_url_event(pid, string, started) : paramErr;
		if (status != noErr)
			os_log_error(OS_LOG_DEFAULT, "LaunchServices: could not deliver a URL to process %d: %d", (int) pid, (int) status);
	}
	if (CFArrayGetCount(files) > 0)
	{
		status = send_documents_event(pid, files, print, started);
		if (status != noErr)
			os_log_error(OS_LOG_DEFAULT, "LaunchServices: could not deliver documents to process %d: %d", (int) pid, (int) status);
	}
	CFRelease(files);

	if (outLaunchedURL != NULL)
	{
		if (*outLaunchedURL != NULL)
			CFRelease(*outLaunchedURL);
		*outLaunchedURL = CFRetain(appURL);
	}
	return noErr;
}

/* The application for the URL's scheme, or NULL. */
static CFURLRef copy_scheme_handler_app(CFURLRef url)
{
	CFStringRef scheme = CFURLCopyScheme(url);
	CFStringRef handler = scheme != NULL ? LSCopyDefaultHandlerForURLScheme(scheme) : NULL;
	CFURLRef app = NULL;

	if (handler != NULL)
	{
		CFArrayRef apps = LSCopyApplicationURLsForBundleIdentifier(handler, NULL);

		if (apps != NULL)
		{
			/* Registrations of bundles that were removed stay in the database; skip them. */
			for (CFIndex i = 0; i < CFArrayGetCount(apps) && app == NULL; i++)
			{
				CFURLRef candidate = CFArrayGetValueAtIndex(apps, i);
				char path[PATH_MAX];

				if (CFURLGetFileSystemRepresentation(candidate, true, (UInt8 *) path, sizeof(path)) && access(path, F_OK) == 0)
					app = CFRetain(candidate);
			}
			CFRelease(apps);
		}
		CFRelease(handler);
	}
	if (scheme != NULL)
		CFRelease(scheme);
	return app;
}

/* The application that opens the item: for a document the handler registered for its type, for another URL the one for its scheme. */
static CFURLRef copy_handler_app(CFURLRef url)
{
	if (is_file_url(url))
		return LSCopyDefaultApplicationURLForURL(url, kLSRolesAll, NULL);
	return copy_scheme_handler_app(url);
}

/* Items that are documents or URLs for a scheme (not links the host desktop opens), grouped by handling application. */
static OSStatus open_items_with_handlers(CFArrayRef urls, Boolean newInstance, Boolean print, CFURLRef *outLaunchedURL)
{
	CFMutableDictionaryRef groups = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	OSStatus ret = noErr;

	for (CFIndex i = 0; i < CFArrayGetCount(urls); i++)
	{
		CFURLRef url = CFArrayGetValueAtIndex(urls, i);
		CFURLRef app = copy_handler_app(url);
		CFMutableArrayRef group;

		if (app == NULL)
		{
			ret = kLSApplicationNotFoundErr;
			continue;
		}
		group = (CFMutableArrayRef) CFDictionaryGetValue(groups, app);
		if (group == NULL)
		{
			group = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
			CFDictionarySetValue(groups, app, group);
			CFRelease(group);
		}
		CFArrayAppendValue(group, url);
		CFRelease(app);
	}

	CFIndex n = CFDictionaryGetCount(groups);
	if (n > 0)
	{
		const void **apps = malloc(n * sizeof(void *)), **lists = malloc(n * sizeof(void *));

		if (apps != NULL && lists != NULL)
		{
			CFDictionaryGetKeysAndValues(groups, apps, lists);
			for (CFIndex i = 0; i < n; i++)
			{
				OSStatus temp = deliver_urls(apps[i], lists[i], newInstance, print, outLaunchedURL);
				if (temp != noErr)
					ret = temp;
			}
		}
		else
			ret = memFullErr;
		free(apps);
		free(lists);
	}
	CFRelease(groups);
	return ret;
}

/*
 * Apple Events: documents are delivered as one kAEOpenDocuments event per application (kAEPrintDocuments
 * with kLSLaunchAndPrint), URLs for a URL scheme as kAEGetURL events. Not supported yet:
 * - an application is started without 'oapp'.
 * - passThruParams: it is ignored. Without kLSLaunchNewInstance items go to a running instance of
 *   their handler; every other call starts a new instance.
 */
OSStatus LSOpenFromURLSpec(const LSLaunchURLSpec *inLaunchSpec, CFURLRef *outLaunchedURL)
{
	CFIndex count, i;
	CFURLRef url;
	OSStatus ret = noErr, temp;
	CFMutableArrayRef items;
	Boolean newInstance, print;

	if (outLaunchedURL != NULL)
		*outLaunchedURL = NULL;

	if (inLaunchSpec == NULL)
		return paramErr;

	count = inLaunchSpec->itemURLs != NULL ? CFArrayGetCount(inLaunchSpec->itemURLs) : 0;
	newInstance = (inLaunchSpec->launchFlags & kLSLaunchNewInstance) != 0;
	print = (inLaunchSpec->launchFlags & kLSLaunchAndPrint) != 0;

	if (inLaunchSpec->appURL != NULL)
	{
		/* The application opens all items, so it is launched once and handed the items as events. */
		if (count == 0)
			return launch_application(inLaunchSpec->appURL, outLaunchedURL);
		return deliver_urls(inLaunchSpec->appURL, inLaunchSpec->itemURLs, newInstance, print, outLaunchedURL);
	}

	if (count == 0)
		return paramErr;

	items = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
	for (i = 0; i < count; i++)
	{
		url = CFArrayGetValueAtIndex(inLaunchSpec->itemURLs, i);

		if (is_file_url(url))
		{
			/* An application bundle is launched, any other file is opened by its handler. */
			if (is_application_url(url))
				temp = launch_application(url, outLaunchedURL);
			else
			{
				CFArrayAppendValue(items, url);
				continue;
			}
		}
		else if (_LSIsHostOpenURL(url))
		{
			temp = _LSOpenURLOnHost(url);
		}
		else
		{
			CFArrayAppendValue(items, url);
			continue;
		}

		if (temp != noErr)
			ret = temp;
	}

	if (CFArrayGetCount(items) > 0)
	{
		temp = open_items_with_handlers(items, newInstance, print, outLaunchedURL);
		if (temp != noErr)
			ret = temp;
	}
	CFRelease(items);
	return ret;
}

OSStatus _LSLaunchApplication(CFURLRef appPath)
{
	return launch_application_pid(appPath, NULL);
}

static OSStatus launch_application_pid(CFURLRef appPath, pid_t *outPid)
{
	CFStringRef executable = NULL, bundleName = NULL;
	CFIndex nameSize = 0, execSize = 0;
	CFURLRef pre = NULL, url_plus_exec = NULL;
	pid_t pid = -1;
	char *name = NULL, *exec = NULL, *argv[2];
	OSStatus status = kLSApplicationNotFoundErr;

	/* Read info.plist, find path to main executable */
	get_main_executable_info(appPath, &executable, &bundleName);
	if (executable == NULL)
	{
		goto cleanup;
	}

	url_plus_exec = CFURLCreateCopyAppendingPathComponent(kCFAllocatorDefault, appPath, CFSTR("Contents"), TRUE);
	if (!url_plus_exec)
		goto cleanup;

	pre = CFURLCreateCopyAppendingPathComponent(kCFAllocatorDefault, url_plus_exec, CFSTR("MacOS"), TRUE);
	CFRelease(url_plus_exec);
	url_plus_exec = NULL;
	if (!pre)
		goto cleanup;

	url_plus_exec = CFURLCreateCopyAppendingPathComponent(kCFAllocatorDefault, pre, executable, FALSE);
	CFRelease(pre);
	pre = NULL;
	if (!url_plus_exec)
		goto cleanup;

	/* Launch the executable */
	CFStringRef execPath = CFURLCopyFileSystemPath(url_plus_exec, kCFURLPOSIXPathStyle);
	CFRelease(url_plus_exec);
	url_plus_exec = NULL;
	if (!execPath)
		goto cleanup;

	/* Try to use CFBundleName first */
	if (bundleName == NULL)
		bundleName = CFStringCreateCopy(kCFAllocatorDefault, execPath);
	if (!bundleName)
	{
		CFRelease(execPath);
		goto cleanup;
	}

	nameSize = CFStringGetMaximumSizeOfFileSystemRepresentation(bundleName);
	name = malloc(nameSize);
	if (!name)
	{
		CFRelease(execPath);
		goto cleanup;
	}
	CFStringGetFileSystemRepresentation(bundleName, name, nameSize);

	execSize = CFStringGetMaximumSizeOfFileSystemRepresentation(execPath);
	exec = malloc(execSize);
	if (!exec)
	{
		CFRelease(execPath);
		goto cleanup;
	}
	CFStringGetFileSystemRepresentation(execPath, exec, execSize);
	CFRelease(execPath);

	argv[0] = exec;
	argv[1] = NULL;
	/* Applications are launchd jobs, as on macOS. */
	pid = spawn_via_launchd(name, (const char *const *)argv, NULL);
	if (pid < 0)
	{
		os_log_error(OS_LOG_DEFAULT, "LaunchServices: launchd could not spawn %{public}s: %{darwin.errno}d", exec, errno);
		status = kLSApplicationNotFoundErr;
		goto cleanup;
	}

	if (outPid != NULL)
		*outPid = pid;
	status = noErr;

cleanup:
	if (name)
		free(name);
	if (exec)
		free(exec);
	if (executable)
		CFRelease(executable);
	if (bundleName)
		CFRelease(bundleName);

	return status;
}

static void get_main_executable_info(CFURLRef appURL, CFStringRef *exec, CFStringRef *bundleName)
{
	CFURLRef pre = CFURLCreateCopyAppendingPathComponent(kCFAllocatorDefault, appURL, CFSTR("Contents"), TRUE);
	CFURLRef info_plist_url = CFURLCreateCopyAppendingPathComponent(kCFAllocatorDefault, pre, CFSTR("Info.plist"), FALSE);
	CFReadStreamRef rs = CFReadStreamCreateWithFile(kCFAllocatorDefault, info_plist_url);
	CFDictionaryRef info_plist;
	CFStringRef ret = NULL;

	/* TODO: Simplify by using CFBundleGetInfoDictionary */

	*exec = NULL;
	*bundleName = NULL;

	CFRelease(pre);

	if (!CFReadStreamOpen(rs))
		goto err_pre_info;

	info_plist = CFPropertyListCreateWithStream(kCFAllocatorDefault, rs, 0, 0, NULL, NULL);
	if (info_plist == NULL)
		goto err_close;

	if (CFGetTypeID(info_plist) != CFDictionaryGetTypeID())
		goto err;

	ret = CFDictionaryGetValue(info_plist, CFSTR("CFBundleExecutable"));
	if (ret != NULL)
		ret = CFStringCreateCopy(kCFAllocatorDefault, ret);

	*exec = ret;

	ret = CFDictionaryGetValue(info_plist, CFSTR("CFBundleName"));
	if (ret != NULL)
		*bundleName = CFStringCreateCopy(kCFAllocatorDefault, ret);

err:
	CFRelease(info_plist);
err_close:
	CFReadStreamClose(rs);
err_pre_info:
	CFRelease(rs);
	CFRelease(info_plist_url);
}
