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
#include <CoreServices/MacErrors.h>
#include <dispatch/dispatch.h>
#include <cstdlib>
#include <iostream>
#include <Block.h>

static xpc_connection_t g_lsdConnection, g_endpointConnection;

static xpc_connection_t getLSDConnection();
static xpc_object_t createCommandDictionary(uint64_t command);
static mach_port_name_t deathPort();
static CFTypeRef g_ourASN;

static CFStringRef g_applicationType = _kLSApplicationForegroundTypeKey;
static LSServerConnectionAllowedBlock g_connectionAllowedBlock = nullptr;

CFDictionaryRef _LSApplicationCheckIn(int sessionID, CFDictionaryRef applicationInfo)
{
	if (g_connectionAllowedBlock && !g_connectionAllowedBlock(applicationInfo))
		return nullptr;

	CFMutableDictionaryRef modAppInfo = CFDictionaryCreateMutableCopy(nullptr, 0, applicationInfo);

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
		CFStringRef path = CFStringCreateWithFileSystemRepresentation(nullptr, _CFProcessPath());
		CFDictionaryAddValue(modAppInfo, _kLSExecutablePathKey, path);
		CFRelease(path);
	}

	if (!CFDictionaryContainsValue(modAppInfo, _kLSPIDKey))
	{
		// Are we supposed to set this one if not present?
		long pid = getpid();
		CFNumberRef num = CFNumberCreate(nullptr, kCFNumberLongType, &pid);
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
		CFDateRef now = CFDateCreate(nullptr, CFAbsoluteTimeGetCurrent());
		CFDictionaryAddValue(modAppInfo, _kLSCheckInTimeKey, now);
		CFRelease(now);
	}

	if (!CFDictionaryContainsValue(modAppInfo, _kLSApplicationTypeKey))
	{
		CFDictionaryAddValue(modAppInfo, _kLSApplicationTypeKey, g_applicationType);
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
			std::cerr << "LaunchServices: check-in failed\n";
			return nullptr;
		}

		CFDictionaryRef result = (CFDictionaryRef) _CFXPCCreateCFObjectFromXPCObject(xpc_dictionary_get_value(reply, "result"));
		g_ourASN = CFDictionaryGetValue(result, _kLSASNKey);
		CFRetain(g_ourASN);

		// Is this really what should be returned?
		return result;
	}
	else
	{
		std::cerr << "LaunchServices: check-in operation didn't execute\n";
	}

	return nullptr;
}

CFTypeRef _LSGetCurrentApplicationASN(void)
{
	return g_ourASN;
}

OSStatus _LSSetApplicationInformationItem(int sessionID, LSASNRef asn, CFStringRef key, CFStringRef value, CFDictionaryRef *info)
{
	// TODO: This talks to lsd to change the property of *any* app
	return unimpErr;
}

void _LSSetApplicationLaunchServicesServerConnectionStatus(uint64_t flags, LSServerConnectionAllowedBlock block)
{
	// TODO
	if (flags & 2) // Called from ApplicationServices/HIServices - this disables connecting to lsd???
		g_applicationType = _kLSApplicationBackgroundOnlyTypeKey;
	
	if (g_connectionAllowedBlock)
		Block_release(g_connectionAllowedBlock);
	
	g_connectionAllowedBlock = nullptr;
	if (block)
		g_connectionAllowedBlock = Block_copy(block);
}

LSASNRef _LSASNCreateWithPid(CFAllocatorRef allocator, pid_t pid)
{
	long lpid = pid;
	return CFNumberCreate(nullptr, kCFNumberLongType, &lpid);
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
		if (const char* env = getenv("SCDontUseServer"); env != nullptr && atoi(env) != 0)
			return;

		const char* serviceName = "com.apple.coreservices.launchservicesd";
		if (const char* altName = getenv("__LSXPCSERVICENAME"); altName != nullptr)
			serviceName = altName;

		xpc_connection_t master = xpc_connection_create_mach_service(serviceName,
			dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), 0);

		xpc_connection_set_event_handler(master, ^(xpc_object_t peer) {
			if (peer == 0)
			{
				// TODO:
				std::cerr << "LaunchServices: Cannot connect to lsd\n";
				return;
			}
			g_lsdConnection = (xpc_connection_t) xpc_retain(peer);

			xpc_object_t dict = createCommandDictionary(_kLSXPCHello);
			xpc_object_t reply = xpc_connection_send_message_with_reply_sync(g_lsdConnection, dict);
			xpc_release(dict);

			if (reply == XPC_ERROR_CONNECTION_INTERRUPTED || reply == XPC_ERROR_CONNECTION_INVALID)
			{
				std::cerr << "LaunchServices: lsd connection broken\n";
				return;
			}
			if (!xpc_dictionary_get_bool(reply, "success"))
			{
				std::cerr << "LaunchServices: lsd hello failed\n";
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
	xpc_object_t dict = xpc_dictionary_create(nullptr, nullptr, 0);
	xpc_dictionary_set_int64(dict, "command", command);
	return dict;
}
