/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define CLT_LABEL "Command Line Tools for Xcode-13.4"
#define CLT_GIT "/Library/Developer/CommandLineTools/usr/bin/git"
#define CLT_INSTALLER "/usr/libexec/darling/clt_install.py"

static int is_flag(const char* arg, const char* shortName, const char* longName)
{
	return strcmp(arg, shortName) == 0 || strcmp(arg, longName) == 0;
}

int main(int argc, char **argv)
{
	int agree = 0;
	int list = 0;
	int install = 0;
	const char* label = NULL;

	for (int i = 1; i < argc; i++)
	{
		if (is_flag(argv[i], "-l", "--list"))
			list = 1;
		else if (is_flag(argv[i], "-i", "--install"))
			install = 1;
		else if (strcmp(argv[i], "--agree-to-license") == 0)
			agree = 1;
		else if (argv[i][0] != '-')
			label = argv[i];
	}

	printf("Software Update Tool\n\n");

	if (list)
	{
		printf("Finding available software\n");
		if (access(CLT_GIT, F_OK) != 0)
		{
			printf("Software Update found the following new or updated software:\n");
			printf("* Label: " CLT_LABEL "\n");
			printf("\tTitle: Command Line Tools for Xcode, Version: 13.4, Size: 2500000KiB, Recommended: YES,\n");
		}
		else
		{
			printf("No new software available.\n");
		}
		return 0;
	}

	if (install && label != NULL && strstr(label, "Command Line Tools") != NULL)
	{
		printf("Installing %s\n", label);

		// The installer asks for the license acceptance unless the caller
		// agreed to it on the command line.
		if (agree)
			setenv("AUTO_ACCEPT_LICENSE", "1", 1);
		else
			unsetenv("AUTO_ACCEPT_LICENSE");

		return system(CLT_INSTALLER) == 0 ? 0 : 1;
	}

	return 0;
}
