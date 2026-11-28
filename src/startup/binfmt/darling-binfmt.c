/*
This file is part of Darling.

Copyright (C) 2026 Darling Developers

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT WARRANTY OF ANY KIND; see the GNU General Public License
for more details.
*/

// binfmt_misc interpreter for Mach-O executables. The host-facing configuration in
// share/darling/binfmt is optional: without it the kernel never calls this program.
//
// The registration uses the P flag. The kernel then runs this program as
//   darling-binfmt <path of the Mach-O file> <argv[0] the caller gave it> <arguments...>
// (see the binfmt_misc administrator's guide). Outside the container we hand the path and the
// arguments to the launcher, which starts a container if needed and maps host files into it. Inside
// one, the guest marker set by the launcher tells us to go straight to mldr: starting a second
// container from inside the first would recurse forever.
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef INSTALL_PREFIX
#error "darling-binfmt needs the installation prefix"
#endif

#ifndef HOST_LAUNCHER
#define HOST_LAUNCHER INSTALL_PREFIX "/bin/darling"
#endif
#ifndef GUEST_MLDR
#define GUEST_MLDR "/bin/mldr"
#endif

static int fail(const char* what)
{
	fprintf(stderr, "darling-binfmt: %s: %s\n", what, strerror(errno));
	return 127;
}

// argv[1] is the executable and argv[2] the name the caller used; "darling exec" takes the path
// followed by the arguments, so the caller's own argv[0] cannot be forwarded.
static int launchInContainer(int argc, char** argv)
{
	char** launcherArgv = calloc((size_t) argc + 1, sizeof(*launcherArgv));

	if (launcherArgv == NULL)
		return fail("cannot allocate launcher arguments");

	launcherArgv[0] = "darling";
	launcherArgv[1] = "exec";
	launcherArgv[2] = argv[1];
	for (int i = 3; i < argc; i++)
		launcherArgv[i] = argv[i];
	launcherArgv[argc] = NULL;

	execv(HOST_LAUNCHER, launcherArgv);
	free(launcherArgv);
	return fail("cannot execute the Darling launcher");
}

// mldr takes the executable path after a "!" in its own argv[0]; the remaining arguments are then
// exactly what the Mach-O program sees, starting with the name the caller used.
static int execInGuest(int argc, char** argv)
{
	char* mldrArgv0;
	char** mldrArgv;

	if (asprintf(&mldrArgv0, "mldr!%s", argv[1]) < 0)
		return fail("cannot allocate the mldr argument");

	mldrArgv = calloc((size_t) argc, sizeof(*mldrArgv));
	if (mldrArgv == NULL)
	{
		free(mldrArgv0);
		return fail("cannot allocate mldr arguments");
	}

	mldrArgv[0] = mldrArgv0;
	for (int i = 2; i < argc; i++)
		mldrArgv[i - 1] = argv[i];
	mldrArgv[argc - 1] = NULL;

	execv(GUEST_MLDR, mldrArgv);
	free(mldrArgv0);
	free(mldrArgv);
	return fail("cannot execute mldr");
}

int main(int argc, char** argv)
{
	const char* marker = getenv("__darling_binfmt_guest");

	if (argc < 3 || argv[1] == NULL || argv[1][0] == '\0')
	{
		fprintf(stderr, "darling-binfmt: not called by binfmt_misc with the P flag\n");
		return 127;
	}

	if (marker != NULL && strcmp(marker, "1") == 0)
		return execInGuest(argc, argv);

	return launchInContainer(argc, argv);
}
