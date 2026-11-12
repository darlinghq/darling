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

/* Largest wait for a launched application to be ready to receive Apple events (60 ticks per second). */
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

/* Sends the URL to the process as a kAEGetURL event; the process is waited for until it can receive it. */
static OSStatus send_get_url_event(pid_t pid, CFStringRef url)
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
		status = AESendMessage(&event, NULL, kAENoReply, APP_READY_TIMEOUT_TICKS);
	AEDisposeDesc(&event);
out:
	if (utf8 != stack)
		free(utf8);
	return status;
}

/*
 * Opens URLs with an application: each goes to it as a kAEGetURL event. A running instance
 * receives them, otherwise the application is launched first (or always, with kLSLaunchNewInstance).
 * Once the application has been started it is not an error if it never takes the events; an
 * application that installs no handler is just launched.
 */
static OSStatus deliver_urls(CFURLRef appURL, CFArrayRef urls, Boolean newInstance, CFURLRef *outLaunchedURL)
{
	CFBundleRef bundle = CFBundleCreate(kCFAllocatorDefault, appURL);
	CFStringRef identifier = bundle != NULL ? CFBundleGetIdentifier(bundle) : NULL;
	char bundleID[512] = "";
	pid_t pid = 0;
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
	}

	for (CFIndex i = 0; i < CFArrayGetCount(urls); i++)
	{
		CFStringRef string = CFURLGetString(CFArrayGetValueAtIndex(urls, i));

		status = string != NULL ? send_get_url_event(pid, string) : paramErr;
		if (status != noErr)
			os_log_error(OS_LOG_DEFAULT, "LaunchServices: could not deliver a URL to process %d: %d", (int) pid, (int) status);
	}

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

/* Items that are URLs for a scheme (not files, not links the host desktop opens), grouped by handling application. */
static OSStatus open_scheme_urls(CFArrayRef urls, Boolean newInstance, CFURLRef *outLaunchedURL)
{
	CFMutableDictionaryRef groups = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	OSStatus ret = noErr;

	for (CFIndex i = 0; i < CFArrayGetCount(urls); i++)
	{
		CFURLRef url = CFArrayGetValueAtIndex(urls, i);
		CFURLRef app = copy_scheme_handler_app(url);
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
				OSStatus temp = deliver_urls(apps[i], lists[i], newInstance, outLaunchedURL);
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
 * Apple Events: URLs for a URL scheme are delivered as kAEGetURL events. Not supported yet:
 * - an application is started without 'oapp' and documents opened with an explicit application
 *   are not sent to it as 'odoc', so only the application is launched for them.
 * - passThruParams: it is ignored. Without kLSLaunchNewInstance a URL goes to a running instance of
 *   its handler; every other call starts a new instance.
 */
OSStatus LSOpenFromURLSpec(const LSLaunchURLSpec *inLaunchSpec, CFURLRef *outLaunchedURL)
{
	CFIndex count, i;
	CFURLRef url;
	OSStatus ret = noErr, temp;
	CFMutableArrayRef schemeURLs;
	Boolean newInstance;

	if (outLaunchedURL != NULL)
		*outLaunchedURL = NULL;

	if (inLaunchSpec == NULL)
		return paramErr;

	count = inLaunchSpec->itemURLs != NULL ? CFArrayGetCount(inLaunchSpec->itemURLs) : 0;
	newInstance = (inLaunchSpec->launchFlags & kLSLaunchNewInstance) != 0;

	if (inLaunchSpec->appURL != NULL)
	{
		/* The application opens all items, so it is launched once; URLs that are not files are
		   handed to it as events. */
		schemeURLs = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
		for (i = 0; i < count; i++)
		{
			url = CFArrayGetValueAtIndex(inLaunchSpec->itemURLs, i);
			if (!is_file_url(url))
				CFArrayAppendValue(schemeURLs, url);
		}
		if (CFArrayGetCount(schemeURLs) > 0)
			ret = deliver_urls(inLaunchSpec->appURL, schemeURLs, newInstance, outLaunchedURL);
		else
			ret = launch_application(inLaunchSpec->appURL, outLaunchedURL);
		CFRelease(schemeURLs);
		return ret;
	}

	if (count == 0)
		return paramErr;

	schemeURLs = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
	for (i = 0; i < count; i++)
	{
		url = CFArrayGetValueAtIndex(inLaunchSpec->itemURLs, i);

		if (is_file_url(url))
		{
			/* First check if it's a "mac thing", if not forward to xdg-open */
			if (is_application_url(url))
				temp = launch_application(url, outLaunchedURL);
			else
				temp = kLSApplicationNotFoundErr;
		}
		else if (_LSIsHostOpenURL(url))
		{
			temp = _LSOpenURLOnHost(url);
		}
		else
		{
			CFArrayAppendValue(schemeURLs, url);
			continue;
		}

		if (temp != noErr)
			ret = temp;
	}

	if (CFArrayGetCount(schemeURLs) > 0)
	{
		temp = open_scheme_urls(schemeURLs, newInstance, outLaunchedURL);
		if (temp != noErr)
			ret = temp;
	}
	CFRelease(schemeURLs);
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
