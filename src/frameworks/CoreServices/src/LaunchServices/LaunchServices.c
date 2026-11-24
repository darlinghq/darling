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
#include "LSLaunch.h"
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
#include <sys/stat.h>
#include <uuid/uuid.h>
#include "LSDProtocol.h"
#include "LSDeliver.h"

static void get_main_executable_info(CFURLRef appURL, CFStringRef *exec, CFStringRef *bundleName);
static Boolean is_file_url(CFURLRef url);
/* What a launch hands the new process besides the items: arguments and environment variables. */
struct launch_parameters
{
	CFArrayRef argv;
	CFDictionaryRef environment;
};

static OSStatus launch_application_pid(CFURLRef appPath, const struct launch_parameters *params, pid_t *outPid);

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

/* Largest wait for a running application to take an Apple event (60 ticks per second). */
#define APP_READY_TIMEOUT_TICKS (60 * 15)

static pid_t find_running_instance(const char *bundleID);

pid_t _LSFindRunningApplication(CFURLRef appURL)
{
	CFBundleRef bundle = CFBundleCreate(kCFAllocatorDefault, appURL);
	CFStringRef identifier = bundle != NULL ? CFBundleGetIdentifier(bundle) : NULL;
	char bundleID[512];
	pid_t pid = 0;

	if (identifier != NULL && CFStringGetCString(identifier, bundleID, sizeof(bundleID), kCFStringEncodingUTF8))
		pid = find_running_instance(bundleID);
	if (bundle != NULL)
		CFRelease(bundle);
	return pid;
}

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

/*
 * QUIRK: the events LaunchServices delivers to an application carry an 'aapd' parameter that is true: documents
 * opened in an application (kAEOpenDocuments), at launch or while it runs; an application opened again while it runs
 * (kAEReopenApplication); and the initial event of LSOpenApplication, at launch or while it runs. The kAEOpenApplication
 * of a plain launch does not. Not documented. Evidence: black-box observation on macOS 26.7.3 (tier 2), with open -a,
 * LSOpenFromURLSpec and LSOpenApplication, recorded in darling's tests/volumes.md. URLs (kAEGetURL) were not observed
 * and are left without it.
 * Test: launchservices_open_application.m (the 'aapd' in each expected event).
 */
#define keyLaunchServicesDelivered 'aapd'

static OSStatus mark_delivered(AppleEvent *event)
{
	Boolean yes = true;
	return AEPutParamPtr(event, keyLaunchServicesDelivered, typeBoolean, &yes, sizeof(yes));
}

/* Tells a running application it was opened again, with a kAEReopenApplication event. */
static OSStatus send_reopen_event(pid_t pid)
{
	AppleEvent event;
	AEDesc target;
	SInt32 targetPid = pid;
	OSStatus status = AECreateDesc(typeKernelProcessID, &targetPid, sizeof(targetPid), &target);

	if (status != noErr)
		return status;
	status = AECreateAppleEvent(kCoreEventClass, kAEReopenApplication, &target, kAutoGenerateReturnID, kAnyTransactionID, &event);
	AEDisposeDesc(&target);
	if (status != noErr)
		return status;
	status = mark_delivered(&event);
	if (status == noErr)
		status = send_event(&event, pid, kAEReopenApplication, false);
	AEDisposeDesc(&event);
	return status;
}

OSStatus _LSDeliverReopenEvent(pid_t pid)
{
	return send_reopen_event(pid);
}

OSStatus _LSDeliverEvent(const AppleEvent *event, pid_t pid, Boolean justLaunched)
{
	AppleEvent copy;
	SInt32 targetPid = pid;
	AEEventID eventID = 0;
	DescType type;
	Size size;
	OSStatus status;

	if (event == NULL)
		return paramErr;
	status = AEDuplicateDesc(event, &copy);
	if (status != noErr)
		return status;
	status = AEPutAttributePtr(&copy, keyAddressAttr, typeKernelProcessID, &targetPid, sizeof(targetPid));
	if (status == noErr)
		status = AEGetAttributePtr(&copy, keyEventIDAttr, typeType, &type, &eventID, sizeof(eventID), &size);
	if (status == noErr)
		status = mark_delivered(&copy);
	if (status == noErr)
		status = send_event(&copy, pid, eventID, justLaunched);
	AEDisposeDesc(&copy);
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
		status = mark_delivered(&event);
	if (status == noErr)
		status = send_event(&event, pid, print ? kAEPrintDocuments : kAEOpenDocuments, queue);
	AEDisposeDesc(&event);
	return status;
}

/*
 * Opens items with an application (none: just opens it, which reopens a running instance): file URLs go to it together as a kAEOpenDocuments event (or
 * kAEPrintDocuments, with print), every other URL as its own kAEGetURL event. A running instance
 * receives them. Otherwise the application is launched (always, with kLSLaunchNewInstance) and the
 * events are queued in launchservicesd without waiting: the new process gets them as its first events
 * when it checks in, and a process that never does is simply launched.
 */
static OSStatus deliver_items(CFURLRef appURL, CFArrayRef urls, Boolean newInstance, Boolean print,
	const struct launch_parameters *params, CFURLRef *outLaunchedURL);

/* Whether two file URLs name the same path (a trailing slash does not matter). */
static Boolean same_file_path(CFURLRef a, CFURLRef b)
{
	char pathA[PATH_MAX], pathB[PATH_MAX];
	size_t lengthA, lengthB;

	if (!CFURLGetFileSystemRepresentation(a, true, (UInt8 *) pathA, sizeof(pathA)) ||
			!CFURLGetFileSystemRepresentation(b, true, (UInt8 *) pathB, sizeof(pathB)))
		return false;
	lengthA = strlen(pathA);
	lengthB = strlen(pathB);
	while (lengthA > 1 && pathA[lengthA - 1] == '/')
		pathA[--lengthA] = 0;
	while (lengthB > 1 && pathB[lengthB - 1] == '/')
		pathB[--lengthB] = 0;
	return strcmp(pathA, pathB) == 0;
}

static OSStatus deliver_urls(CFURLRef appURL, CFArrayRef itemURLs, Boolean newInstance, Boolean print,
	const struct launch_parameters *params, CFURLRef *outLaunchedURL)
{
	/* The application among its own items (how open -a asks for just the application) is not a document. */
	CFMutableArrayRef urls = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
	OSStatus result;

	for (CFIndex i = 0; itemURLs != NULL && i < CFArrayGetCount(itemURLs); i++)
	{
		CFURLRef item = CFArrayGetValueAtIndex(itemURLs, i);

		if (!(is_file_url(item) && same_file_path(item, appURL)))
			CFArrayAppendValue(urls, item);
	}
	result = deliver_items(appURL, urls, newInstance, print, params, outLaunchedURL);
	CFRelease(urls);
	return result;
}

static OSStatus deliver_items(CFURLRef appURL, CFArrayRef urls, Boolean newInstance, Boolean print,
	const struct launch_parameters *params, CFURLRef *outLaunchedURL)
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
		status = launch_application_pid(appURL, params, &pid);
		if (status != noErr)
			return status;
		started = true;
	}

	CFMutableArrayRef files = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);

	/* Opening a running application without items is a reopen. */
	if (!started && (urls == NULL || CFArrayGetCount(urls) == 0))
		send_reopen_event(pid);

	for (CFIndex i = 0; urls != NULL && i < CFArrayGetCount(urls); i++)
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
static OSStatus open_items_with_handlers(CFArrayRef urls, Boolean newInstance, Boolean print,
	const struct launch_parameters *params, CFURLRef *outLaunchedURL)
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
				OSStatus temp = deliver_urls(apps[i], lists[i], newInstance, print, params, outLaunchedURL);
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
 * with kLSLaunchAndPrint), URLs for a URL scheme as kAEGetURL events, a running application opened
 * without items as kAEReopenApplication. A new process gets them as its first events, through
 * launchservicesd. Without kLSLaunchNewInstance items, and an application bundle among the items, go
 * to a running instance of their application; otherwise a new instance is started. Not supported yet:
 * - passThruParams: it is ignored.
 */
static Boolean is_folder_without_handler(CFURLRef url)
{
	char path[PATH_MAX];
	struct stat st;
	CFURLRef app;

	if (!CFURLGetFileSystemRepresentation(url, true, (UInt8 *) path, sizeof(path)) || stat(path, &st) != 0 || !S_ISDIR(st.st_mode))
		return FALSE;
	app = copy_handler_app(url);
	if (app == NULL)
		return TRUE;
	CFRelease(app);
	return FALSE;
}

OSStatus LSOpenFromURLSpec(const LSLaunchURLSpec *inLaunchSpec, CFURLRef *outLaunchedURL)
{
	return _LSOpenFromURLSpecWithParameters(inLaunchSpec, NULL, NULL, outLaunchedURL);
}

OSStatus _LSOpenFromURLSpecWithParameters(const LSLaunchURLSpec *inLaunchSpec, CFArrayRef argv,
	CFDictionaryRef environment, CFURLRef *outLaunchedURL)
{
	const struct launch_parameters params = { argv, environment };
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
		return deliver_urls(inLaunchSpec->appURL, inLaunchSpec->itemURLs, newInstance, print, &params, outLaunchedURL);
	}

	if (count == 0)
		return paramErr;

	items = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
	for (i = 0; i < count; i++)
	{
		url = CFArrayGetValueAtIndex(inLaunchSpec->itemURLs, i);

		if (is_file_url(url))
		{
			/*
			 * An application bundle is opened like an application without items: a running instance
			 * is reopened unless a new one is asked for. Any other file is opened by its handler.
			 */
			if (is_application_url(url))
				temp = deliver_items(url, NULL, newInstance, print, &params, outLaunchedURL);
			else if (is_folder_without_handler(url))
				/* There is no Finder in the prefix, so a folder is shown by the host's file manager. */
				temp = _LSOpenFileOnHost(url, FALSE);
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
		temp = open_items_with_handlers(items, newInstance, print, &params, outLaunchedURL);
		if (temp != noErr)
			ret = temp;
	}
	CFRelease(items);
	return ret;
}

OSStatus _LSLaunchApplication(CFURLRef appPath)
{
	return launch_application_pid(appPath, NULL, NULL);
}

/*
 * The variables that name the host's window system (X11 or Wayland) and the AppKit backend for it.
 * On macOS an application opened through LaunchServices joins the caller's login session and so its
 * window server; here the window server is the host display the caller's own variables name, which
 * the container's launchd lacks when the container was started from a terminal without them.
 */
static const char *const window_system_variables[] = {
	"DISPLAY",
	"WAYLAND_DISPLAY",
	"XAUTHORITY",
	"XDG_RUNTIME_DIR",
	"DARLING_APPKIT_BACKEND",
};

static char *copy_cfstring_utf8(CFStringRef string)
{
	CFIndex size;
	char *result;

	if (string == NULL || CFGetTypeID(string) != CFStringGetTypeID())
		return NULL;
	size = CFStringGetMaximumSizeForEncoding(CFStringGetLength(string), kCFStringEncodingUTF8) + 1;
	result = malloc(size);
	if (result != NULL && !CFStringGetCString(string, result, size, kCFStringEncodingUTF8))
	{
		free(result);
		result = NULL;
	}
	return result;
}

static void free_string_list(char **list)
{
	if (list == NULL)
		return;
	for (char **p = list; *p != NULL; p++)
		free(*p);
	free(list);
}

/* The new process's arguments: the executable, then the CFStrings of argv. NULL if one is not a string. */
static char **copy_arguments(const char *exec, CFArrayRef argv)
{
	CFIndex count = argv != NULL ? CFArrayGetCount(argv) : 0;
	char **result = calloc(count + 2, sizeof(char *));

	if (result == NULL)
		return NULL;
	result[0] = strdup(exec);
	if (result[0] == NULL)
		goto fail;
	for (CFIndex i = 0; i < count; i++)
	{
		result[i + 1] = copy_cfstring_utf8(CFArrayGetValueAtIndex(argv, i));
		if (result[i + 1] == NULL)
			goto fail;
	}
	return result;

fail:
	free_string_list(result);
	return NULL;
}

/*
 * NAME=value entries set in the new process on top of launchd's environment: the caller's window
 * system variables, then the requested environment (which wins). NULL if an entry is not a string.
 */
static char **copy_environment(CFDictionaryRef environment)
{
	const size_t nvars = sizeof(window_system_variables) / sizeof(window_system_variables[0]);
	CFIndex count = environment != NULL ? CFDictionaryGetCount(environment) : 0;
	char **result = calloc(nvars + count + 1, sizeof(char *));
	size_t n = 0;

	if (result == NULL)
		return NULL;

	for (size_t i = 0; i < nvars; i++)
	{
		const char *value = getenv(window_system_variables[i]);
		CFStringRef key;
		Boolean requested;

		if (value == NULL)
			continue;
		key = CFStringCreateWithCString(kCFAllocatorDefault, window_system_variables[i], kCFStringEncodingUTF8);
		requested = environment != NULL && key != NULL && CFDictionaryContainsKey(environment, key);
		if (key != NULL)
			CFRelease(key);
		if (requested)
			continue;
		if (asprintf(&result[n], "%s=%s", window_system_variables[i], value) < 0)
		{
			result[n] = NULL;
			goto fail;
		}
		n++;
	}

	if (count > 0)
	{
		const void **keys = malloc(count * sizeof(void *)), **values = malloc(count * sizeof(void *));

		if (keys == NULL || values == NULL)
		{
			free(keys);
			free(values);
			goto fail;
		}
		CFDictionaryGetKeysAndValues(environment, keys, values);
		for (CFIndex i = 0; i < count; i++)
		{
			char *key = copy_cfstring_utf8(keys[i]), *value = copy_cfstring_utf8(values[i]);

			if (key == NULL || value == NULL || key[0] == 0 || strchr(key, '=') != NULL ||
					asprintf(&result[n], "%s=%s", key, value) < 0)
			{
				result[n] = NULL;
				free(key);
				free(value);
				free(keys);
				free(values);
				goto fail;
			}
			n++;
			free(key);
			free(value);
		}
		free(keys);
		free(values);
	}
	return result;

fail:
	free_string_list(result);
	return NULL;
}

/*
 * The launchd label of a new application process. Every launch is its own job, so that a new
 * instance never finds the job of a running (or exited but not yet removed) one with the same
 * name: launchd would answer with that process instead of starting a new one.
 */
static char *copy_job_label(CFURLRef appPath, const char *name)
{
	CFBundleRef bundle = CFBundleCreate(kCFAllocatorDefault, appPath);
	CFStringRef identifier = bundle != NULL ? CFBundleGetIdentifier(bundle) : NULL;
	char *base = copy_cfstring_utf8(identifier), *label = NULL;
	uuid_t uuid;
	uuid_string_t uuidString;

	if (bundle != NULL)
		CFRelease(bundle);
	uuid_generate_random(uuid);
	uuid_unparse_upper(uuid, uuidString);
	if (asprintf(&label, "application.%s.%s", base != NULL ? base : name, uuidString) < 0)
		label = NULL;
	free(base);
	return label;
}

static OSStatus launch_application_pid(CFURLRef appPath, const struct launch_parameters *params, pid_t *outPid)
{
	CFStringRef executable = NULL, bundleName = NULL;
	CFIndex nameSize = 0, execSize = 0;
	CFURLRef pre = NULL, url_plus_exec = NULL;
	pid_t pid = -1;
	char *name = NULL, *exec = NULL, *label = NULL, **argv = NULL, **env = NULL;
	struct spawn_via_launchd_attr attr;
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

	/* Arguments and environment variables that are not strings are a bad parameter. */
	status = paramErr;
	argv = copy_arguments(exec, params != NULL ? params->argv : NULL);
	env = copy_environment(params != NULL ? params->environment : NULL);
	if (argv == NULL || env == NULL)
		goto cleanup;

	status = memFullErr;
	label = copy_job_label(appPath, name);
	if (label == NULL)
		goto cleanup;

	memset(&attr, 0, sizeof(attr));
	attr.spawn_env = (const char *const *) env;

	/* Applications are launchd jobs, as on macOS. */
	pid = spawn_via_launchd(label, (const char *const *) argv, &attr);
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
	free(label);
	free_string_list(argv);
	free_string_list(env);
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
