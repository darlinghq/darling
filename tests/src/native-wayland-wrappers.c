/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Checks the native Wayland library wrappers: wayland-client, wayland-cursor
 * and xkbcommon are either all installed or all absent (they are wrapped as
 * one unit), and an installed wrapper exports the entry points the AppKit
 * Wayland backend binds. Without the wrappers on the host the test reports
 * SKIP and succeeds.
 */
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static const struct {
	const char* path;
	const char* symbols[3];
} libs[] = {
	{ "/usr/lib/native/libwayland-client.dylib", { "wl_display_connect", "wl_proxy_marshal_array_flags", NULL } },
	{ "/usr/lib/native/libwayland-cursor.dylib", { "wl_cursor_theme_load", NULL } },
	{ "/usr/lib/native/libxkbcommon.dylib", { "xkb_context_new", NULL } },
};

int main(void) {
	int present = 0, count = sizeof(libs) / sizeof(libs[0]);
	for (int i = 0; i < count; i++)
		if (access(libs[i].path, R_OK) == 0)
			present++;

	if (present == 0) {
		printf("SKIP: Wayland wrappers not built\n");
		return 0;
	}
	if (present != count) {
		printf("FAIL: only %d of %d Wayland wrappers installed\n", present, count);
		return 1;
	}

	int failed = 0;
	for (int i = 0; i < count; i++) {
		void* h = dlopen(libs[i].path, RTLD_NOW);
		if (!h) {
			printf("FAIL: dlopen %s: %s\n", libs[i].path, dlerror());
			failed = 1;
			continue;
		}
		for (int s = 0; libs[i].symbols[s]; s++) {
			if (!dlsym(h, libs[i].symbols[s])) {
				printf("FAIL: %s lacks %s\n", libs[i].path, libs[i].symbols[s]);
				failed = 1;
			}
		}
	}
	printf(failed ? "FAIL\n" : "PASS\n");
	return failed;
}
