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
// binfmt_misc is used without the P flag here, so argv[0] is the path of the Mach-O executable and
// argv[1..] are the arguments the caller gave it. Outside the container we hand that to the launcher,
// which starts a container if needed and maps host files into it. Inside one, the guest marker set by
// the launcher tells us to go straight to mldr: starting a second container from inside the first
// would recurse forever.
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

// argv[0] is the executable, so insert "darling exec" before it and keep every argument as it was.
static int launchInContainer(int argc, char** argv)
{
	char** launcherArgv = calloc((size_t) argc + 3, sizeof(*launcherArgv));

	if (launcherArgv == NULL)
		return fail("cannot allocate launcher arguments");

	launcherArgv[0] = "darling";
	launcherArgv[1] = "exec";
	launcherArgv[2] = argv[0];
	for (int i = 1; i < argc; i++)
		launcherArgv[i + 2] = argv[i];
	launcherArgv[argc + 2] = NULL;

	execv(HOST_LAUNCHER, launcherArgv);
	free(launcherArgv);
	return fail("cannot execute the Darling launcher");
}

// mldr expects its own path in argv[0] and the executable in argv[1]. The "!" form puts both in
// argv[0], which keeps the arguments that the Mach-O program itself sees unchanged.
static int execInGuest(int argc, char** argv)
{
	char* mldrArgv0;
	char** mldrArgv;

	if (asprintf(&mldrArgv0, "mldr!%s", argv[0]) < 0)
		return fail("cannot allocate the mldr argument");

	mldrArgv = calloc((size_t) argc + 2, sizeof(*mldrArgv));
	if (mldrArgv == NULL)
	{
		free(mldrArgv0);
		return fail("cannot allocate mldr arguments");
	}

	mldrArgv[0] = mldrArgv0;
	for (int i = 0; i < argc; i++)
		mldrArgv[i + 1] = argv[i];
	mldrArgv[argc + 1] = NULL;

	execv(GUEST_MLDR, mldrArgv);
	free(mldrArgv0);
	free(mldrArgv);
	return fail("cannot execute mldr");
}

int main(int argc, char** argv)
{
	const char* marker = getenv("__darling_binfmt_guest");

	if (argc < 1 || argv[0] == NULL || argv[0][0] == '\0')
	{
		fprintf(stderr, "darling-binfmt: no executable was named by binfmt_misc\n");
		return 127;
	}

	if (marker != NULL && strcmp(marker, "1") == 0)
		return execInGuest(argc, argv);

	return launchInContainer(argc, argv);
}
