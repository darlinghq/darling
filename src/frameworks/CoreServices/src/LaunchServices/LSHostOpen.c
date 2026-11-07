/*
This file is part of Darling.

Copyright (C) 2026 Darling developers

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

/*
 * Web and mail URLs have no handler inside the prefix, so they are handed to
 * the host desktop through the XDG desktop portal (see LSHostOpenBus.h). The
 * guest shares the host's user, so the session bus is reachable directly.
 *
 * libdbus-1 is the host's, used through Darling's wrapper library, which is
 * loaded only when a URL is opened. The wrapper is part of the GUI
 * component; without it, or on a slice where the host library is not used
 * (only x86_64 is), no URL is opened and kLSApplicationNotFoundErr is
 * returned.
 */

#include "LSHostOpen.h"
#include "LSHostOpenBus.h"
#include <CarbonCore/MacErrors.h>
#include <dlfcn.h>
#include <os/log.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#define DBUS_WRAPPER_PATH "/usr/lib/native/libdbus.dylib"

static struct ls_host_dbus host_dbus;
static Boolean host_dbus_ready;
static pthread_once_t host_dbus_once = PTHREAD_ONCE_INIT;

static void load_host_dbus(void)
{
#if defined(__x86_64__)
	struct ls_host_dbus api;
	void* lib = dlopen(DBUS_WRAPPER_PATH, RTLD_NOW | RTLD_LOCAL);

	if (lib == NULL)
		return;

	memset(&api, 0, sizeof(api));
#define BIND(field, name) \
	if ((*(void**) &api.field = dlsym(lib, name)) == NULL) \
		goto fail;
	BIND(connection_open_private, "dbus_connection_open_private");
	BIND(bus_register, "dbus_bus_register");
	BIND(connection_close, "dbus_connection_close");
	BIND(connection_unref, "dbus_connection_unref");
	BIND(message_new_method_call, "dbus_message_new_method_call");
	BIND(message_iter_init_append, "dbus_message_iter_init_append");
	BIND(message_iter_append_basic, "dbus_message_iter_append_basic");
	BIND(message_iter_open_container, "dbus_message_iter_open_container");
	BIND(message_iter_close_container, "dbus_message_iter_close_container");
	BIND(connection_send_with_reply_and_block, "dbus_connection_send_with_reply_and_block");
	BIND(message_unref, "dbus_message_unref");
	BIND(error_init, "dbus_error_init");
	BIND(error_free, "dbus_error_free");
	BIND(address_escape_value, "dbus_address_escape_value");
	BIND(free, "dbus_free");
#undef BIND
	/*
	 * The guest's environment: DBUS_SESSION_BUS_ADDRESS and XDG_RUNTIME_DIR
	 * arrive from the host unchanged. A guest process can set them, but only a
	 * single unix socket address is used (see LSHostOpenBus.c), so it can at
	 * most pick a socket that it could open itself as the same host user.
	 */
	api.getenv = getenv;

	host_dbus = api;
	host_dbus_ready = TRUE;
	return;

fail:
	dlclose(lib);
#endif
}

static const struct ls_host_dbus* get_host_dbus(void)
{
	pthread_once(&host_dbus_once, load_host_dbus);
	return host_dbus_ready ? &host_dbus : NULL;
}

/* Returns the absolute URL as a malloc'd UTF-8 string. */
static char* copy_url_string(CFURLRef url)
{
	CFURLRef absolute;
	CFStringRef string;
	CFIndex size;
	char* buffer = NULL;

	if (url == NULL)
		return NULL;
	absolute = CFURLCopyAbsoluteURL(url);
	if (absolute == NULL)
		return NULL;

	string = CFURLGetString(absolute);
	size = CFStringGetMaximumSizeForEncoding(CFStringGetLength(string), kCFStringEncodingUTF8) + 1;
	if (size <= LS_HOST_OPEN_MAX_URL_LENGTH * 4 + 1)
	{
		buffer = malloc(size);
		if (buffer != NULL && !CFStringGetCString(string, buffer, size, kCFStringEncodingUTF8))
		{
			free(buffer);
			buffer = NULL;
		}
	}
	CFRelease(absolute);
	return buffer;
}

Boolean _LSIsHostOpenURL(CFURLRef url)
{
	CFStringRef scheme;
	Boolean result = FALSE;

	if (url == NULL)
		return FALSE;
	scheme = CFURLCopyScheme(url);
	if (scheme == NULL)
		return FALSE;

	result = CFStringCompare(scheme, CFSTR("http"), kCFCompareCaseInsensitive) == kCFCompareEqualTo
		|| CFStringCompare(scheme, CFSTR("https"), kCFCompareCaseInsensitive) == kCFCompareEqualTo
		|| CFStringCompare(scheme, CFSTR("mailto"), kCFCompareCaseInsensitive) == kCFCompareEqualTo;
	CFRelease(scheme);
	return result;
}

OSStatus _LSOpenURLOnHost(CFURLRef url)
{
	const struct ls_host_dbus* api;
	char* string = copy_url_string(url);
	int r;

	if (string == NULL || _LSHostOpenValidateURL(string) < 0)
	{
		free(string);
		return paramErr;
	}

	api = get_host_dbus();
	if (api == NULL)
	{
		os_log_error(OS_LOG_DEFAULT, "LaunchServices: cannot open URL: the host D-Bus library is unavailable");
		free(string);
		return kLSApplicationNotFoundErr;
	}

	r = _LSHostOpenURL(api, string);
	free(string);
	switch (r)
	{
		case LS_HOST_OPEN_OK:
			return noErr;
		case LS_HOST_OPEN_INVALID_URL:
			return paramErr;
		case LS_HOST_OPEN_NO_PORTAL:
			os_log_error(OS_LOG_DEFAULT, "LaunchServices: cannot open URL: no desktop portal on the session bus");
			return kLSApplicationNotFoundErr;
		default:
			os_log_error(OS_LOG_DEFAULT, "LaunchServices: the desktop portal did not accept the URL");
			return kLSServerCommunicationErr;
	}
}
