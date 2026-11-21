/*
This file is part of Darling.

Copyright (C) 2026 Lubos Dolezel

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

#include "darling_dbus.h"
#include <dlfcn.h>
#include <pthread.h>
#include <string.h>

static int bind_functions(void* lib, struct darling_dbus* table)
{
	memset(table, 0, sizeof(*table));
#define DARLING_DBUS_BIND(name, ret, params) \
	if ((*(void**) &table->name = dlsym(lib, "dbus_" #name)) == NULL) \
		return -1;
	DARLING_DBUS_FUNCTIONS(DARLING_DBUS_BIND)
#undef DARLING_DBUS_BIND
	return 0;
}

static void* open_host_library(void)
{
	void* lib = dlopen("libdbus-1.so.3", RTLD_NOW | RTLD_LOCAL);

	if (lib == NULL)
		lib = dlopen("libdbus-1.so", RTLD_NOW | RTLD_LOCAL);
	return lib;
}

static int load_from(void* lib, struct darling_dbus* table)
{
	struct darling_dbus loaded;

	if (lib == NULL)
		return -1;
	if (bind_functions(lib, &loaded) != 0)
	{
		dlclose(lib);
		return -2;
	}
	*table = loaded;
	return 0;
}

int darling_dbus_load(const char* path, unsigned int flags, struct darling_dbus* table)
{
	void* lib = dlopen(path != NULL ? path : DARLING_DBUS_WRAPPER_PATH, RTLD_NOW | RTLD_LOCAL);

	if (lib == NULL && (flags & DARLING_DBUS_HOST_FALLBACK) != 0)
		lib = open_host_library();
	return load_from(lib, table);
}

/* The wrapper is tried once; the host library is only tried for callers that allow it, also once. */
static struct darling_dbus wrapper_table;
static int wrapper_loaded;
static pthread_once_t wrapper_once = PTHREAD_ONCE_INIT;

static struct darling_dbus host_table;
static int host_loaded;
static pthread_once_t host_once = PTHREAD_ONCE_INIT;

static void load_wrapper(void)
{
	wrapper_loaded = darling_dbus_load(NULL, 0, &wrapper_table) == 0;
}

static void load_host(void)
{
	host_loaded = load_from(open_host_library(), &host_table) == 0;
}

const struct darling_dbus* darling_dbus_get(unsigned int flags)
{
	pthread_once(&wrapper_once, load_wrapper);
	if (wrapper_loaded)
		return &wrapper_table;

	if ((flags & DARLING_DBUS_HOST_FALLBACK) != 0)
	{
		pthread_once(&host_once, load_host);
		if (host_loaded)
			return &host_table;
	}
	return NULL;
}
