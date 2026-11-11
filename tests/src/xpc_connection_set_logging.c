/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * libxpc exports __xpc_connection_set_logging (Mach-O symbol
 * ___xpc_connection_set_logging), as on macOS 27 (libxpc.tbd).
 * Link test plus a dlsym lookup; the function is not called, since no
 * public header declares its signature.
 */
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

extern void __xpc_connection_set_logging(void);

static int failures;

static void check(const char *what, int cond)
{
	printf("%s: %s\n", cond ? "PASS" : "FAIL", what);
	if (!cond)
		failures++;
}

int main(void)
{
	void *volatile linked = (void *)&__xpc_connection_set_logging;
	Dl_info info;
	void *sym = dlsym(RTLD_DEFAULT, "__xpc_connection_set_logging");

	check("linked against __xpc_connection_set_logging", linked != NULL);
	check("dlsym finds __xpc_connection_set_logging", sym == linked);
	check("it lives in libxpc", sym != NULL && dladdr(sym, &info) && info.dli_fname != NULL &&
	      strstr(info.dli_fname, "libxpc") != NULL);
	return failures != 0;
}
