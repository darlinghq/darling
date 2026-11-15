/*
This file is part of Darling.

Copyright (C) 2020 Lubos Dolezel

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

#include <xpc/xpc.h>
#include <xpc/private.h>
#include <LaunchServices/LaunchServices.h>
#include <LaunchServices/LaunchServicesPriv.h>
#include <CoreFoundation/CFPriv.h>
#include <CoreFoundation/CFBundlePriv.h>
#include <CoreFoundation/CFDate.h>
#include <CoreFoundation/CFXPCBridge.h>
#include <CarbonCore/MacErrors.h>
#include <dispatch/dispatch.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <Block.h>
#include <libproc.h>

static xpc_connection_t g_lsdConnection, g_endpointConnection;

static xpc_connection_t getLSDConnection();
static xpc_object_t createCommandDictionary(uint64_t command);
static mach_port_name_t deathPort();
static CFTypeRef g_ourASN;

static CFStringRef g_applicationType = NULL;
static pthread_mutex_t g_typeLock = PTHREAD_MUTEX_INITIALIZER; // guards g_applicationType
static LSServerConnectionAllowedBlock g_connectionAllowedBlock = NULL;

CFDictionaryRef _LSApplicationCheckIn(int sessionID, CFDictionaryRef applicationInfo)
{
	if (g_connectionAllowedBlock && !g_connectionAllowedBlock(applicationInfo))
		return NULL;

	CFMutableDictionaryRef modAppInfo = CFDictionaryCreateMutableCopy(NULL, 0, applicationInfo);

	if (!CFDictionaryContainsValue(modAppInfo, _kLSArchitectureKey))
	{
#if defined(__x86_64__)
		CFDictionaryAddValue(modAppInfo, _kLSArchitectureKey, _kLSArchitecturex86_64Value);
#elif defined(__i386__)
		CFDictionaryAddValue(modAppInfo, _kLSArchitectureKey, _kLSArchitecturei386Value);
#elif defined(__ppc__)
		CFDictionaryAddValue(modAppInfo, _kLSArchitectureKey, _kLSArchitecturePowerPCValue);
#elif defined(__ppc64__)
		CFDictionaryAddValue(modAppInfo, _kLSArchitectureKey, _kLSArchitecturePowerPC64Value);
#else
		CFDictionaryAddValue(modAppInfo, _kLSArchitectureKey, _kLSArchitectureUnknownValue);
#endif
	}

	if (!CFDictionaryContainsValue(modAppInfo, _kLSExecutablePathKey))
	{
		CFStringRef path = CFStringCreateWithFileSystemRepresentation(NULL, _CFProcessPath());
		CFDictionaryAddValue(modAppInfo, _kLSExecutablePathKey, path);
		CFRelease(path);
	}

	if (!CFDictionaryContainsValue(modAppInfo, _kLSPIDKey))
	{
		// Are we supposed to set this one if not present?
		long pid = getpid();
		CFNumberRef num = CFNumberCreate(NULL, kCFNumberLongType, &pid);
		CFDictionaryAddValue(modAppInfo, _kLSPIDKey, num);
		CFRelease(num);
	}

	if (!CFDictionaryContainsValue(modAppInfo, _kLSDisplayNameKey) &&
		!CFDictionaryContainsValue(modAppInfo, _kCFBundleDisplayNameKey) &&
		!CFDictionaryContainsValue(modAppInfo, kCFBundleNameKey))
	{
		// TODO: Generate app name from bundle path / process path etc.
	}

	if (!CFDictionaryContainsValue(modAppInfo, _kLSCheckInTimeKey))
	{
		CFDateRef now = CFDateCreate(NULL, CFAbsoluteTimeGetCurrent());
		CFDictionaryAddValue(modAppInfo, _kLSCheckInTimeKey, now);
		CFRelease(now);
	}

	if (!CFDictionaryContainsValue(modAppInfo, _kLSApplicationTypeKey))
	{
		pthread_mutex_lock(&g_typeLock);
		if (!g_applicationType)
			g_applicationType = _kLSApplicationForegroundTypeKey;
		CFDictionaryAddValue(modAppInfo, _kLSApplicationTypeKey, g_applicationType);
		pthread_mutex_unlock(&g_typeLock);
	}

	xpc_object_t dict = createCommandDictionary(_kLSXPCCheckIn);

	xpc_object_t info = _CFXPCCreateXPCObjectFromCFObject(modAppInfo);
	CFRelease(modAppInfo);

	xpc_dictionary_set_value(dict, "info", info);
	xpc_release(info);

	xpc_dictionary_set_mach_send(dict, "deathport", deathPort());
	xpc_object_t reply = xpc_connection_send_message_with_reply_sync(getLSDConnection(), dict);
	xpc_release(dict);

	if (reply && reply != XPC_ERROR_CONNECTION_INTERRUPTED && reply != XPC_ERROR_CONNECTION_INVALID)
	{
		if (!xpc_dictionary_get_bool(reply, "success"))
		{
			fprintf(stderr, "LaunchServices: check-in failed\n");
			return NULL;
		}

		CFDictionaryRef result = (CFDictionaryRef) _CFXPCCreateCFObjectFromXPCObject(xpc_dictionary_get_value(reply, "result"));
		g_ourASN = CFDictionaryGetValue(result, _kLSASNKey);
		CFRetain(g_ourASN);

		// Is this really what should be returned?
		return result;
	}
	else
	{
		fprintf(stderr, "LaunchServices: check-in operation didn't execute\n");
	}

	return NULL;
}

CFTypeRef _LSGetCurrentApplicationASN(void)
{
	return g_ourASN;
}

LSASNRef _LSCopyFrontApplication(LSSessionID sessionID)
{
	// Darling has no notion of application focus yet, so there is no
	// frontmost application to report. NULL is the "no front
	// application" answer for this API; callers must already tolerate it.
	return NULL;
}

// Returns the PID an ASN refers to, or -1 if it is not a valid ASN.
static pid_t asnToPid(LSASNRef asn)
{
	long pid;
	if (!asn || CFGetTypeID(asn) != CFNumberGetTypeID() || !CFNumberGetValue(asn, kCFNumberLongType, &pid))
		return -1;
	return (pid > 0 && pid <= 0x7fffffff) ? (pid_t) pid : -1;
}

static bool processExists(pid_t pid)
{
	return kill(pid, 0) == 0 || errno == EPERM;
}

static CFStringRef copyTypeKeyForSelf(void)
{
	CFStringRef type;
	pthread_mutex_lock(&g_typeLock);
	// A process that never checked in as an application is not a foreground one.
	type = g_applicationType ? g_applicationType : _kLSApplicationBackgroundOnlyTypeKey;
	pthread_mutex_unlock(&g_typeLock);
	return type;
}

// The path of the Mach-O executable a process runs. /proc/<pid>/exe would name
// Darling's loader, so the kernel's own answer is used.
static CFStringRef copyProcessExecutablePath(pid_t pid)
{
	char buf[PROC_PIDPATHINFO_MAXSIZE];
	int len = proc_pidpath((int) pid, buf, sizeof(buf));

	if (len <= 0)
		return NULL;
	buf[len < (int) sizeof(buf) ? len : (int) sizeof(buf) - 1] = 0;
	return CFStringCreateWithCString(NULL, buf, kCFStringEncodingUTF8);
}

// The name of a process: the display name of the application bundle the
// executable sits in (<name>.app/Contents/MacOS/<executable>), the bundle
// name when it has no display name, else the executable's file name.
static CFStringRef copyProcessDisplayName(CFStringRef exePath)
{
	CFStringRef name = NULL;
	CFRange found = CFStringFind(exePath, CFSTR(".app/Contents/MacOS/"), kCFCompareBackwards);

	if (found.location != kCFNotFound)
	{
		CFStringRef bundlePath = CFStringCreateWithSubstring(NULL, exePath, CFRangeMake(0, found.location + 4));
		CFURLRef url = CFURLCreateWithFileSystemPath(NULL, bundlePath, kCFURLPOSIXPathStyle, true);
		CFBundleRef bundle = url ? CFBundleCreate(NULL, url) : NULL;

		if (bundle)
		{
			CFTypeRef value = CFBundleGetValueForInfoDictionaryKey(bundle, _kCFBundleDisplayNameKey);
			if (!value || CFGetTypeID(value) != CFStringGetTypeID() || CFStringGetLength(value) == 0)
				value = CFBundleGetValueForInfoDictionaryKey(bundle, kCFBundleNameKey);
			if (value && CFGetTypeID(value) == CFStringGetTypeID() && CFStringGetLength(value) > 0)
				name = CFRetain(value);
			CFRelease(bundle);
		}
		if (url)
			CFRelease(url);
		CFRelease(bundlePath);
	}
	if (!name)
	{
		CFRange slash = CFStringFind(exePath, CFSTR("/"), kCFCompareBackwards);
		CFIndex start = slash.location == kCFNotFound ? 0 : slash.location + 1;
		name = CFStringCreateWithSubstring(NULL, exePath, CFRangeMake(start, CFStringGetLength(exePath) - start));
	}
	return name;
}

OSStatus _LSSetApplicationInformationItem(int sessionID, LSASNRef asn, CFStringRef key, CFStringRef value, CFDictionaryRef *info)
{
	if (info)
		*info = NULL;

	if (asnToPid(asn) != getpid() || !key || !CFEqual(key, _kLSApplicationTypeKey))
		return unimpErr; // changing other processes needs a registry in lsd

	if (!value || !(CFEqual(value, _kLSApplicationForegroundTypeKey)
		|| CFEqual(value, _kLSApplicationBackgroundOnlyTypeKey)
		|| CFEqual(value, _kLSApplicationUIElementTypeKey)))
	{
		return paramErr;
	}

	pthread_mutex_lock(&g_typeLock);
	if (CFEqual(value, _kLSApplicationForegroundTypeKey))
		g_applicationType = _kLSApplicationForegroundTypeKey;
	else if (CFEqual(value, _kLSApplicationBackgroundOnlyTypeKey))
		g_applicationType = _kLSApplicationBackgroundOnlyTypeKey;
	else
		g_applicationType = _kLSApplicationUIElementTypeKey;
	pthread_mutex_unlock(&g_typeLock);

	if (info)
		*info = _LSCopyApplicationInformation(sessionID, asn, 0);
	return noErr;
}

CFTypeRef _LSCopyApplicationInformationItem(int sessionID, CFTypeRef asn, CFStringRef what)
{
	pid_t pid = asnToPid(asn);
	if (pid < 0 || !what || !processExists(pid))
		return NULL;

	if (CFEqual(what, _kLSASNKey))
		return CFRetain(asn);
	if (CFEqual(what, _kLSPIDKey))
	{
		long lpid = pid;
		return CFNumberCreate(NULL, kCFNumberLongType, &lpid);
	}
	if (CFEqual(what, _kLSExecutablePathKey))
		return copyProcessExecutablePath(pid);
	if (CFEqual(what, _kLSDisplayNameKey))
	{
		CFStringRef path = copyProcessExecutablePath(pid);
		CFStringRef name = path ? copyProcessDisplayName(path) : NULL;
		if (path)
			CFRelease(path);
		return name;
	}
	if (CFEqual(what, _kLSApplicationTypeKey) && pid == getpid())
		return CFRetain(copyTypeKeyForSelf());
	return NULL;
}

CFDictionaryRef _LSCopyApplicationInformation(int sessionID, CFTypeRef asn, int unknown)
{
	CFStringRef keys[] = { _kLSASNKey, _kLSPIDKey, _kLSExecutablePathKey, _kLSDisplayNameKey, _kLSApplicationTypeKey };
	CFMutableDictionaryRef dict = NULL;

	if (asnToPid(asn) < 0)
		return NULL;

	for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); i++)
	{
		CFTypeRef value = _LSCopyApplicationInformationItem(sessionID, asn, keys[i]);
		if (!value)
			continue;
		if (!dict)
			dict = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		CFDictionarySetValue(dict, keys[i], value);
		CFRelease(value);
	}
	return dict;
}

Boolean _LSASNExtractHighAndLowParts(LSASNRef asn, UInt32* outHigh, UInt32* outLow)
{
	pid_t pid = asnToPid(asn);

	// An ASN is the PID of the process, so the high part is always 0.
	if (outHigh) *outHigh = 0;
	if (outLow) *outLow = pid > 0 ? (UInt32) pid : 0;
	return pid > 0;
}

static int comparePids(const void* a, const void* b)
{
	pid_t x = *(const pid_t*) a, y = *(const pid_t*) b;
	return (x > y) - (x < y);
}

CFArrayRef _LSCopyApplicationArray(int sessionID)
{
	DIR* dir = opendir("/proc");
	if (!dir)
		return NULL;

	size_t count = 0, cap = 256;
	pid_t* pids = malloc(cap * sizeof(pid_t));
	struct dirent* ent;

	while (pids && (ent = readdir(dir)) != NULL)
	{
		if (!isdigit((unsigned char) ent->d_name[0]))
			continue;
		if (count == cap)
		{
			pid_t* grown = realloc(pids, (cap *= 2) * sizeof(pid_t));
			if (!grown)
			{
				free(pids);
				pids = NULL;
				break;
			}
			pids = grown;
		}
		pids[count++] = (pid_t) atoi(ent->d_name);
	}
	closedir(dir);
	if (!pids)
		return NULL;

	qsort(pids, count, sizeof(pid_t), comparePids);

	CFMutableArrayRef array = CFArrayCreateMutable(NULL, count, &kCFTypeArrayCallBacks);
	for (size_t i = 0; i < count; i++)
	{
		LSASNRef asn = _LSASNCreateWithPid(NULL, pids[i]);
		CFArrayAppendValue(array, asn);
		CFRelease(asn);
	}
	free(pids);
	return array;
}

void _LSSetApplicationLaunchServicesServerConnectionStatus(uint64_t flags, LSServerConnectionAllowedBlock block)
{
	// TODO
	if (flags & 2) // Called from ApplicationServices/HIServices - this disables connecting to lsd???
	{
		pthread_mutex_lock(&g_typeLock);
		g_applicationType = _kLSApplicationBackgroundOnlyTypeKey;
		pthread_mutex_unlock(&g_typeLock);
	}
	
	if (g_connectionAllowedBlock)
		Block_release(g_connectionAllowedBlock);
	
	g_connectionAllowedBlock = NULL;
	if (block)
		g_connectionAllowedBlock = Block_copy(block);
}

LSASNRef _LSASNCreateWithPid(CFAllocatorRef allocator, pid_t pid)
{
	long lpid = pid;
	return CFNumberCreate(NULL, kCFNumberLongType, &lpid);
}

static mach_port_name_t deathPort()
{
	static dispatch_once_t onceDeathPort;
	static mach_port_name_t port;
	dispatch_once(&onceDeathPort, ^{
		kern_return_t kr;
		kr = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &port);

		if (kr == KERN_SUCCESS)
			mach_port_insert_right(mach_task_self(), port, port, MACH_MSG_TYPE_MAKE_SEND);
	});

	return port;
}

static xpc_connection_t getLSDConnection()
{
	static dispatch_once_t once;
	dispatch_once(&once, ^{
		const char* env = getenv("SCDontUseServer");
		if (env != NULL && atoi(env) != 0)
			return;

		const char* serviceName = "com.apple.coreservices.launchservicesd";
		const char* altName = getenv("__LSXPCSERVICENAME");

		if (altName != NULL)
			serviceName = altName;

		xpc_connection_t master = xpc_connection_create_mach_service(serviceName,
			dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), 0);

		xpc_connection_set_event_handler(master, ^(xpc_object_t peer) {
			if (peer == 0)
			{
				// TODO:
				fprintf(stderr, "LaunchServices: Cannot connect to lsd\n");
				return;
			}
			g_lsdConnection = (xpc_connection_t) xpc_retain(peer);

			xpc_object_t dict = createCommandDictionary(_kLSXPCHello);
			xpc_object_t reply = xpc_connection_send_message_with_reply_sync(g_lsdConnection, dict);
			xpc_release(dict);

			if (reply == XPC_ERROR_CONNECTION_INTERRUPTED || reply == XPC_ERROR_CONNECTION_INVALID)
			{
				fprintf(stderr, "LaunchServices: lsd connection broken\n");
				return;
			}
			if (!xpc_dictionary_get_bool(reply, "success"))
			{
				fprintf(stderr, "LaunchServices: lsd hello failed\n");
				return;
			}
			xpc_object_t result = xpc_dictionary_get_value(reply, "result");
			xpc_object_t ep = xpc_dictionary_get_value(result, "endpoint");

			if (xpc_get_type(ep) == XPC_TYPE_ENDPOINT)
			{
				xpc_connection_t conn = g_endpointConnection = xpc_connection_create_from_endpoint((xpc_endpoint_t) ep);
				xpc_connection_set_target_queue(conn, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0));
				xpc_connection_set_event_handler(conn, ^(xpc_object_t conn) {
					// TODO?
				});
				xpc_connection_resume(conn);
			}

			xpc_release(reply);
		});
		xpc_connection_resume(master);
	});

	return g_endpointConnection;
}

static xpc_object_t createCommandDictionary(uint64_t command)
{
	xpc_object_t dict = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_int64(dict, "command", command);
	return dict;
}
