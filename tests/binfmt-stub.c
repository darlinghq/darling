// Copyright (C) 2026 Darling Developers
//
// Stand-in for the launcher and mldr in tests/binfmt-helper.py. Unlike a script, an ELF executable
// receives argv[0] exactly as the helper passed it, which is what that test needs to observe.
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
	const char* path = getenv("DARLING_BINFMT_TEST_OUTPUT");
	const char* launch = getenv("__darling_binfmt_launch");
	const char* guest = getenv("__darling_binfmt_guest");
	FILE* out;

	if (path == NULL || (out = fopen(path, "w")) == NULL)
		return 1;

	fprintf(out, "argv0=%s\n", argv[0] != NULL ? argv[0] : "");
	for (int i = 1; i < argc; i++)
		fprintf(out, "arg%d=%s\n", i - 1, argv[i]);
	fprintf(out, "launch=%s\n", launch != NULL ? launch : "unset");
	fprintf(out, "guest=%s\n", guest != NULL ? guest : "unset");
	return fclose(out) == 0 ? 0 : 1;
}
