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
#include <darling_dbus.h>
#include <CarbonCore/MacErrors.h>
#include <fcntl.h>
#include <limits.h>
#include <os/log.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static struct ls_host_dbus host_dbus;
static Boolean host_dbus_ready;
static pthread_once_t host_dbus_once = PTHREAD_ONCE_INIT;

static void load_host_dbus(void)
{
#if defined(__x86_64__)
	struct ls_host_dbus api;
	const struct darling_dbus* shared = darling_dbus_get(0);

	if (shared == NULL)
		return;

	memset(&api, 0, sizeof(api));
	api.connection_open_private = shared->connection_open_private;
	api.bus_register = shared->bus_register;
	api.connection_close = shared->connection_close;
	api.connection_unref = shared->connection_unref;
	api.message_new_method_call = shared->message_new_method_call;
	api.message_iter_init_append = shared->message_iter_init_append;
	api.message_iter_append_basic = shared->message_iter_append_basic;
	api.message_iter_open_container = shared->message_iter_open_container;
	api.message_iter_close_container = shared->message_iter_close_container;
	api.connection_send_with_reply_and_block = shared->connection_send_with_reply_and_block;
	api.message_unref = shared->message_unref;
	api.error_init = shared->error_init;
	api.error_free = shared->error_free;
	api.address_escape_value = shared->address_escape_value;
	api.free = shared->free;
	/*
	 * The guest's environment: DBUS_SESSION_BUS_ADDRESS and XDG_RUNTIME_DIR
	 * arrive from the host unchanged. A guest process can set them, but only a
	 * single unix socket address is used (see LSHostOpenBus.c), so it can at
	 * most pick a socket that it could open itself as the same host user.
	 */
	api.getenv = getenv;

	host_dbus = api;
	host_dbus_ready = TRUE;
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

OSStatus _LSOpenFileOnHost(CFURLRef url, Boolean reveal)
{
	const struct ls_host_dbus* api;
	char path[PATH_MAX];
	int fd, r;

	if (url == NULL || !CFURLGetFileSystemRepresentation(url, true, (UInt8*) path, sizeof(path)))
		return paramErr;

	/* The desktop gets the descriptor and finds the item from it, so no path has to be translated to the host's. */
	fd = open(path, O_RDONLY | O_CLOEXEC);
	if (fd < 0)
		return fnfErr;

	api = get_host_dbus();
	if (api == NULL)
	{
		os_log_error(OS_LOG_DEFAULT, "LaunchServices: cannot open the item: the host D-Bus library is unavailable");
		close(fd);
		return kLSApplicationNotFoundErr;
	}

	r = _LSHostOpenFileDescriptor(api, fd, reveal);
	close(fd);
	switch (r)
	{
		case LS_HOST_OPEN_OK:
			return noErr;
		case LS_HOST_OPEN_INVALID_URL:
			return paramErr;
		case LS_HOST_OPEN_NO_PORTAL:
			os_log_error(OS_LOG_DEFAULT, "LaunchServices: cannot open the item: no desktop portal on the session bus");
			return kLSApplicationNotFoundErr;
		default:
			os_log_error(OS_LOG_DEFAULT, "LaunchServices: the desktop portal did not accept the item");
			return kLSServerCommunicationErr;
	}
}
