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

#include <stdlib.h>
#include <string.h>
#include "launcher-openargs.h"

// The long options of the guest's open(1) that take no argument. --args is handled separately: open(1)
// pre-scans for it and passes everything after it to the launched application.
static const char* const longOptions[] = {
	"fresh",
	"reveal",
	"wait-apps",
	"new",
	"hide",
	"background",
	"header",
};

static bool shortTakesArgument(char c)
{
	return c == 'a' || c == 'b' || c == 's';
}

static bool shortTakesNoArgument(char c)
{
	return strchr("etfFRWnjgh", c) != NULL;
}

// getopt_long(3) accepts an unambiguous prefix of a long option.
static bool matchLongOption(const char* name, size_t length)
{
	bool found = false;

	for (size_t i = 0; i < sizeof(longOptions) / sizeof(*longOptions); i++)
	{
		if (strlen(longOptions[i]) == length && strncmp(longOptions[i], name, length) == 0)
			return true;
		if (strncmp(longOptions[i], name, length) == 0)
		{
			if (found)
				return false;
			found = true;
		}
	}
	return found;
}

bool launcherOpenOperands(int argc, char* const argv[], int start, struct launcher_open_operands* out)
{
	struct launcher_open_operands result = { NULL, 0 };

	if (out == NULL || start < 0 || start > argc)
		return false;
	*out = result;

	// open(1) finds --args before it parses any options, even when --args follows "--" or looks like
	// the argument of another option. Everything from there on belongs to the application.
	for (int i = start; i < argc; i++)
	{
		if (strcmp(argv[i], "--args") == 0)
		{
			argc = i;
			break;
		}
	}

	if (start < argc)
	{
		result.indexes = malloc((size_t)(argc - start) * sizeof(*result.indexes));
		if (result.indexes == NULL)
			return false;
	}

	for (int i = start; i < argc; i++)
	{
		const char* arg = argv[i];

		if (strcmp(arg, "--") == 0)
		{
			for (i++; i < argc; i++)
				result.indexes[result.count++] = i;
			break;
		}

		if (arg[0] == '-' && arg[1] != '\0')
		{
			if (arg[1] == '-')
			{
				const char* name = arg + 2;
				const char* equals = strchr(name, '=');
				size_t length = equals != NULL ? (size_t)(equals - name) : strlen(name);

				// None of open(1)'s long options takes a value.
				if (length == 0 || equals != NULL || !matchLongOption(name, length))
					goto invalid;
				continue;
			}

			for (size_t j = 1; arg[j] != '\0'; j++)
			{
				if (shortTakesArgument(arg[j]))
				{
					if (arg[j + 1] == '\0')
					{
						if (++i >= argc)
							goto invalid;
					}
					break;
				}
				if (!shortTakesNoArgument(arg[j]))
					goto invalid;
			}
			continue;
		}

		result.indexes[result.count++] = i;
	}

	*out = result;
	return true;

invalid:
	free(result.indexes);
	result.indexes = NULL;
	result.count = 0;
	*out = result;
	return false;
}
