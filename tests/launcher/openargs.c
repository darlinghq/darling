// Copyright (C) 2026 Darling Developers
//
// Host-side test of how the launcher tells open(1)'s options and application arguments apart from the
// host files it must name in the guest (src/startup/launcher-openargs.c).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "launcher-openargs.h"

static int failures = 0;

static void expectOperands(const char* what, char* argv[], int argc, const int* want, size_t count)
{
	struct launcher_open_operands got;
	bool ok = launcherOpenOperands(argc, argv, 2, &got);

	if (!ok)
	{
		printf("FAIL: %s: parsing failed\n", what);
		failures++;
		return;
	}
	bool matches = got.count == count;

	if (matches)
	{
		for (size_t i = 0; i < count; i++)
		{
			if (got.indexes[i] != want[i])
			{
				matches = false;
				break;
			}
		}
	}
	if (!matches)
	{
		printf("FAIL: %s: got %zu operands, want %zu\n", what, got.count, count);
		failures++;
	}
	else
		printf("PASS: %s\n", what);
	free(got.indexes);
}

static void expectInvalid(const char* what, char* argv[], int argc)
{
	struct launcher_open_operands got = { (int*) 1, 123 };

	if (launcherOpenOperands(argc, argv, 2, &got) || got.indexes != NULL)
	{
		printf("FAIL: %s: invalid options were accepted\n", what);
		failures++;
	}
	else
		printf("PASS: %s\n", what);
}

#define ARGV(...) (char*[]){ __VA_ARGS__ }

int main(void)
{
	const int two[] = { 2, 3 };
	const int two_after_dashdash[] = { 3, 4 };
	const int one3[] = { 3 };
	const int one4[] = { 4 };

	expectOperands("plain files", ARGV("darling", "open", "a.txt", "b.txt", NULL), 4, two, 2);
	expectOperands("application option", ARGV("darling", "open", "-a", "TextEdit", "a.txt", NULL), 5, &one4[0], 1);
	expectOperands("attached application option", ARGV("darling", "open", "-aTextEdit", "a.txt", NULL), 4, one3, 1);
	expectOperands("bundle identifier option", ARGV("darling", "open", "-b", "com.apple.TextEdit", "a.txt", NULL), 5, &one4[0], 1);
	expectOperands("text editor option", ARGV("darling", "open", "-e", "a.txt", NULL), 4, one3, 1);
	expectOperands("long option", ARGV("darling", "open", "--fresh", "a.txt", NULL), 4, one3, 1);
	expectOperands("abbreviated long option", ARGV("darling", "open", "--wait", "a.txt", NULL), 4, one3, 1);
	expectOperands("cluster without argument", ARGV("darling", "open", "-ng", "a.txt", NULL), 4, one3, 1);
	expectOperands("application arguments", ARGV("darling", "open", "--args", "-x", "not-a-file", NULL), 5, NULL, 0);
	expectOperands("application arguments after end of options", ARGV("darling", "open", "--", "--args", "file", NULL), 5, NULL, 0);
	expectOperands("end of options", ARGV("darling", "open", "--", "-a", "file", NULL), 5, two_after_dashdash, 2);
	expectOperands("no operands", ARGV("darling", "open", NULL), 2, NULL, 0);

	expectInvalid("missing option argument", ARGV("darling", "open", "-a", NULL), 3);
	expectInvalid("--args where an option argument was expected", ARGV("darling", "open", "-a", "--args", "file", NULL), 5);
	expectInvalid("unknown long option", ARGV("darling", "open", "--unknown", "a.txt", NULL), 4);
	expectInvalid("unknown short option", ARGV("darling", "open", "-x", "a.txt", NULL), 4);

	return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
