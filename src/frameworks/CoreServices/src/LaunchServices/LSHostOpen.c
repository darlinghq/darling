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
 * the Linux desktop. sd-bus is used from the host's libsystemd, loaded through
 * the ELF loader; the guest shares the host's user and network, so the
 * session bus is reachable directly.
 */

#include "LSHostOpen.h"
#include "LSHostOpenBus.h"
#include <CarbonCore/MacErrors.h>
#include <elfcalls.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern struct elf_calls* _elfcalls;

/* Where the host's root file system appears inside the prefix. */
#define HOST_ROOT "/Volumes/SystemRoot"

static struct ls_host_api host_api;
static Boolean host_api_ready;
static pthread_once_t host_api_once = PTHREAD_ONCE_INIT;

static void load_host_api(void)
{
	void *systemd, *libc;
	struct ls_host_api api;

	if (_elfcalls == NULL)
		return;

	systemd = _elfcalls->dlopen("libsystemd.so.0");
	if (systemd == NULL)
		return;
	libc = _elfcalls->dlopen("libc.so.6");
	if (libc == NULL)
	{
		_elfcalls->dlclose(systemd);
		return;
	}

	memset(&api, 0, sizeof(api));
#define BIND(lib, field) \
	if ((*(void**) &api.field = _elfcalls->dlsym(lib, #field)) == NULL) \
		goto fail;
	BIND(systemd, sd_bus_new);
	BIND(systemd, sd_bus_set_address);
	BIND(systemd, sd_bus_set_bus_client);
	BIND(systemd, sd_bus_start);
	BIND(systemd, sd_bus_flush_close_unref);
	BIND(systemd, sd_bus_message_new_method_call);
	BIND(systemd, sd_bus_message_append);
	BIND(systemd, sd_bus_message_append_strv);
	BIND(systemd, sd_bus_message_open_container);
	BIND(systemd, sd_bus_message_close_container);
	BIND(systemd, sd_bus_call);
	BIND(systemd, sd_bus_message_unref);
	BIND(systemd, sd_bus_error_free);
	/* Host IDs and paths, not the prefix's view of them. */
	BIND(libc, getuid);
	BIND(libc, access);
#undef BIND
	api.getenv = getenv;
	api.runtime_dir_root = "/run/user";

	host_api = api;
	host_api_ready = TRUE;
	return;

fail:
	_elfcalls->dlclose(libc);
	_elfcalls->dlclose(systemd);
}

static const struct ls_host_api* get_host_api(void)
{
	pthread_once(&host_api_once, load_host_api);
	return host_api_ready ? &host_api : NULL;
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
	const struct ls_host_api* api;
	char* string = copy_url_string(url);
	int r;

	if (string == NULL || _LSHostOpenValidateURL(string) < 0)
	{
		free(string);
		return paramErr;
	}

	api = get_host_api();
	if (api == NULL)
	{
		fprintf(stderr, "LaunchServices: cannot open URL: host libsystemd is unavailable\n");
		free(string);
		return kLSApplicationNotFoundErr;
	}

	r = _LSHostOpenURL(api, string, NULL);
	free(string);
	if (r < 0)
	{
		fprintf(stderr, "LaunchServices: host desktop did not accept URL (error %d)\n", -r);
		return kLSServerCommunicationErr;
	}
	return noErr;
}

OSStatus _LSGetHostHandlerForURL(CFURLRef url, FSRef* outAppRef, CFURLRef* outAppURL)
{
	const struct ls_host_api* api;
	const char* opener;
	char path[256];

	if (!_LSIsHostOpenURL(url))
		return kLSApplicationNotFoundErr;
	api = get_host_api();
	if (api == NULL)
		return kLSApplicationNotFoundErr;

	opener = _LSHostOpenFindOpener(api);
	snprintf(path, sizeof(path), HOST_ROOT "%s", opener ? opener : "/usr/bin/xdg-open");

	if (outAppRef != NULL)
	{
		OSStatus status = FSPathMakeRef((const UInt8*) path, outAppRef, NULL);
		if (status != noErr)
			return status;
	}
	if (outAppURL != NULL)
	{
		*outAppURL = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8*) path, strlen(path), FALSE);
		if (*outAppURL == NULL)
			return kLSApplicationNotFoundErr;
	}
	return noErr;
}
