/*
This file is part of Darling.

Copyright (C) 2026 Darling developers

Based on work by lucannez64 (public repository lucannez64/darling-lol,
commit 8f62017), GPL-3.0.

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

#include "LSHostOpenBus.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define DEFAULT_TIMEOUT_USEC (10ULL * 1000 * 1000)
#define HOST_X_OK 1

/* Passed to the transient unit so xdg-open finds the user's display. */
static const char* const forwarded_environment[] = { "DISPLAY", "WAYLAND_DISPLAY", "XDG_CURRENT_DESKTOP" };

const char* _LSHostOpenFindOpener(const struct ls_host_api* api)
{
	static const char* const candidates[] = {
		"/usr/local/bin/xdg-open",
		"/usr/bin/xdg-open",
		"/bin/xdg-open",
	};

	if (api == NULL || api->access == NULL)
		return NULL;
	for (size_t i = 0; i < sizeof(candidates) / sizeof(candidates[0]); i++)
	{
		if (api->access(candidates[i], HOST_X_OK) == 0)
			return candidates[i];
	}
	return NULL;
}

static int api_complete(const struct ls_host_api* api)
{
	return api != NULL && api->sd_bus_new && api->sd_bus_set_address && api->sd_bus_set_bus_client
		&& api->sd_bus_start && api->sd_bus_flush_close_unref && api->sd_bus_message_new_method_call
		&& api->sd_bus_message_append && api->sd_bus_message_append_strv
		&& api->sd_bus_message_open_container && api->sd_bus_message_close_container
		&& api->sd_bus_call && api->sd_bus_message_unref && api->sd_bus_error_free
		&& api->getenv && api->getuid;
}

/*
 * sd-bus separates alternative addresses with ';' and accepts "unixexec:path=...", which runs a
 * host program, so "unix:path=/x;unixexec:path=/bin/sh,..." must not get through. Only one plain
 * socket address (a path or an abstract name) is taken from the environment.
 */
static int session_address_allowed(const char* address)
{
	if (strncmp(address, "unix:path=", 10) != 0 && strncmp(address, "unix:abstract=", 14) != 0)
		return 0;
	for (const unsigned char* c = (const unsigned char*) address; *c != '\0'; c++)
	{
		if (*c <= ' ' || *c >= 0x7f || *c == ';')
			return 0;
	}
	return strstr(address, "unixexec") == NULL;
}

static int open_session_bus(const struct ls_host_api* api, void** ret)
{
	char address[512];
	const char* env = api->getenv("DBUS_SESSION_BUS_ADDRESS");
	void* bus = NULL;
	int r;

	if (env != NULL && env[0] != '\0')
	{
		if (strlen(env) >= sizeof(address) || !session_address_allowed(env))
			return -LS_HOST_LINUX_EINVAL;
		strcpy(address, env);
	}
	else
	{
		const char* root = api->runtime_dir_root ? api->runtime_dir_root : "/run/user";
		if (snprintf(address, sizeof(address), "unix:path=%s/%u/bus", root, api->getuid())
				>= (int) sizeof(address))
			return -LS_HOST_LINUX_EINVAL;
	}

	r = api->sd_bus_new(&bus);
	if (r < 0)
		return r;
	r = api->sd_bus_set_address(bus, address);
	if (r >= 0)
		r = api->sd_bus_set_bus_client(bus, 1);
	if (r >= 0)
		r = api->sd_bus_start(bus);
	if (r < 0)
	{
		api->sd_bus_flush_close_unref(bus);
		return r;
	}
	*ret = bus;
	return 0;
}

/* Copies the D-Bus error name of a failed call (empty if the failure had none) to "name". */
static int call(const struct ls_host_api* api, void* bus, void* m, char* name, size_t name_size)
{
	struct ls_host_bus_error error = { NULL, NULL, 0 };
	void* reply = NULL;
	uint64_t timeout = api->timeout_usec ? api->timeout_usec : DEFAULT_TIMEOUT_USEC;
	int r = api->sd_bus_call(bus, m, timeout, &error, &reply);

	if (name != NULL && name_size > 0)
		snprintf(name, name_size, "%s", error.name != NULL ? error.name : "");
	if (reply != NULL)
		api->sd_bus_message_unref(reply);
	api->sd_bus_error_free(&error);
	return r < 0 ? r : 0;
}

static int open_via_portal(const struct ls_host_api* api, void* bus, const char* url, char* error_name,
	size_t error_name_size)
{
	void* m = NULL;
	int r = api->sd_bus_message_new_method_call(bus, &m, "org.freedesktop.portal.Desktop",
		"/org/freedesktop/portal/desktop", "org.freedesktop.portal.OpenURI", "OpenURI");
	if (r < 0)
		return r;

	r = api->sd_bus_message_append(m, "ss", "", url);
	if (r >= 0)
		r = api->sd_bus_message_append(m, "a{sv}", 0);
	if (r >= 0)
		r = call(api, bus, m, error_name, error_name_size);
	api->sd_bus_message_unref(m);
	return r;
}

static int append_environment(const struct ls_host_api* api, void* m)
{
	char storage[3][1024];
	char* entries[4];
	size_t count = 0;
	int r;

	for (size_t i = 0; i < sizeof(forwarded_environment) / sizeof(forwarded_environment[0]); i++)
	{
		const char* value = api->getenv(forwarded_environment[i]);
		int n;

		if (value == NULL || value[0] == '\0')
			continue;
		n = snprintf(storage[count], sizeof(storage[count]), "%s=%s", forwarded_environment[i], value);
		if (n < 0 || n >= (int) sizeof(storage[count]))
			continue;
		entries[count] = storage[count];
		count++;
	}
	entries[count] = NULL;
	if (count == 0)
		return 0;

	r = api->sd_bus_message_open_container(m, 'r', "sv");
	if (r >= 0)
		r = api->sd_bus_message_append(m, "s", "Environment");
	if (r >= 0)
		r = api->sd_bus_message_open_container(m, 'v', "as");
	if (r >= 0)
		r = api->sd_bus_message_append_strv(m, entries);
	if (r >= 0)
		r = api->sd_bus_message_close_container(m);
	if (r >= 0)
		r = api->sd_bus_message_close_container(m);
	return r;
}

static int open_via_systemd(const struct ls_host_api* api, void* bus, const char* url)
{
	static unsigned int counter;
	struct timespec now;
	char unit[128];
	const char* opener = _LSHostOpenFindOpener(api);
	void* m = NULL;
	int r;

	/* Bare names are resolved by systemd >= 248. */
	if (opener == NULL)
		opener = "xdg-open";

	clock_gettime(CLOCK_REALTIME, &now);
	snprintf(unit, sizeof(unit), "app-darling-openurl-%d-%llx-%u.service", (int) getpid(),
		(unsigned long long) now.tv_sec * 1000000000ULL + (unsigned long long) now.tv_nsec,
		__atomic_add_fetch(&counter, 1, __ATOMIC_RELAXED));

	r = api->sd_bus_message_new_method_call(bus, &m, "org.freedesktop.systemd1",
		"/org/freedesktop/systemd1", "org.freedesktop.systemd1.Manager", "StartTransientUnit");
	if (r < 0)
		return r;

	r = api->sd_bus_message_append(m, "ss", unit, "fail");
	if (r >= 0)
		r = api->sd_bus_message_open_container(m, 'a', "(sv)");
	if (r >= 0)
		r = api->sd_bus_message_append(m, "(sv)(sv)(sv)(sv)",
			"Description", "s", "Open URL from Darling",
			/* Keep the handler running after xdg-open exits. */
			"KillMode", "s", "process",
			"CollectMode", "s", "inactive-or-failed",
			"ExecStart", "a(sasb)", 1, opener, 2, "xdg-open", url, 0);
	if (r >= 0)
		r = append_environment(api, m);
	if (r >= 0)
		r = api->sd_bus_message_close_container(m);
	if (r >= 0)
		r = api->sd_bus_message_append(m, "a(sa(sv))", 0);
	if (r >= 0)
		r = call(api, bus, m, NULL, 0);
	api->sd_bus_message_unref(m);
	return r;
}

/* True when the portal is simply not there; a denial or any other failure must not be bypassed. */
static int portal_unavailable(const char* error_name)
{
	return strcmp(error_name, "org.freedesktop.DBus.Error.ServiceUnknown") == 0
		|| strcmp(error_name, "org.freedesktop.DBus.Error.NameHasNoOwner") == 0;
}

int _LSHostOpenURL(const struct ls_host_api* api, const char* url, int* method)
{
	void* bus = NULL;
	char error_name[128] = "";
	int r;

	r = _LSHostOpenValidateURL(url);
	if (r < 0)
		return r;
	if (!api_complete(api))
		return -LS_HOST_LINUX_ENOSYS;

	r = open_session_bus(api, &bus);
	if (r < 0)
		return r;

	r = open_via_portal(api, bus, url, error_name, sizeof(error_name));
	if (r >= 0)
	{
		if (method)
			*method = LS_HOST_OPEN_VIA_PORTAL;
	}
	/* Only an absent portal is bypassed: a timed-out one may still open the URL and a denial is a
	   decision, so neither is retried. */
	else if (portal_unavailable(error_name))
	{
		r = open_via_systemd(api, bus, url);
		if (r >= 0 && method)
			*method = LS_HOST_OPEN_VIA_SYSTEMD;
	}

	api->sd_bus_flush_close_unref(bus);
	return r < 0 ? r : 0;
}
