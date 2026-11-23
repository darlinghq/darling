// Copyright (C) 2026 Cristian Magherusan-Stanciu
//
// Host-side test of how mldr picks the program to start (after the '!' in
// argv[0], the __mldr_exec_path variable, or argv[1]) and of the arguments it
// builds for running mldr32 under an emulator.
#include <stdio.h>
#include <string.h>
#include "exec_target.h"

static int failures;
static void check(int ok, const char* what)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		failures++;
}

static const char* pick(int argc, char** argv, const char* env, enum mldr_exec_target_source* src)
{
	return mldr_exec_target(argc, argv, env, src);
}

int main(void)
{
	enum mldr_exec_target_source src;
	const char* t;

	{
		char* a[] = { "/usr/libexec/darling/mldr!/host/App", "/App", "arg", NULL };
		t = pick(3, a, NULL, &src);
		check(t && !strcmp(t, "/host/App") && src == MLDR_TARGET_ARGV0, "'!' in argv[0]: the path after it");
		t = pick(3, a, "/other", &src);
		check(t && !strcmp(t, "/host/App") && src == MLDR_TARGET_ARGV0, "'!' in argv[0] beats the variable");
		char* one[] = { "mldr!/host/App", NULL };
		t = pick(1, one, NULL, &src);
		check(t && !strcmp(t, "/host/App") && src == MLDR_TARGET_ARGV0, "'!' with no further arguments");
	}
	{
		char* a[] = { "/emu/mldr32", "/App", "arg", NULL };
		t = pick(3, a, "/host/App", &src);
		check(t && !strcmp(t, "/host/App") && src == MLDR_TARGET_ENV, "variable beats argv[1]");
		t = pick(3, a, "", &src);
		check(t && !strcmp(t, "/App") && src == MLDR_TARGET_ARGV1, "empty variable is ignored");
		t = pick(3, a, NULL, &src);
		check(t && !strcmp(t, "/App") && src == MLDR_TARGET_ARGV1, "no variable: argv[1]");
		char* one[] = { "/emu/mldr32", NULL };
		t = pick(1, one, "/host/App", &src);
		check(t && !strcmp(t, "/host/App") && src == MLDR_TARGET_ENV, "variable with no further arguments");
		t = pick(1, one, NULL, &src);
		check(t == NULL && src == MLDR_TARGET_NONE, "argv[0] only and no variable: nothing to start");
	}
	{
		char* none[] = { NULL };
		t = pick(0, none, "/host/App", &src);
		check(t == NULL && src == MLDR_TARGET_NONE, "no arguments at all: nothing to start, even with the variable");
	}

	{
		char* a[] = { "mldr!/host/App", "/App", "-x", "y z", NULL };
		char* out[6];
		memset(out, 0xff, sizeof out);
		int n = mldr_emulator_argv("/bin/emu", "/lib/mldr32", 4, a, out);
		check(n == 5 && !strcmp(out[0], "/bin/emu") && !strcmp(out[1], "/lib/mldr32") && !strcmp(out[2], "/App") &&
				!strcmp(out[3], "-x") && !strcmp(out[4], "y z") && out[5] == NULL,
				"emulator arguments: emulator, mldr32, argv[1..], NULL; argv[0] dropped");
		char* one[] = { "mldr!/host/App", NULL };
		n = mldr_emulator_argv("/bin/emu", "/lib/mldr32", 1, one, out);
		check(n == 2 && out[2] == NULL, "emulator arguments with only argv[0]");
	}

	printf("%s\n", failures ? "FAILED" : "all passed");
	return failures != 0;
}
