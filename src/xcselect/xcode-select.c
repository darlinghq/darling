/*
This file is part of Darling.

Copyright (C) 2017-2020 Lubos Dolezel

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
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include "xcselect.h"

void printUsage(void);
void doReset(void);
void doSwitch(const char* path);
void doPrintManPaths(void);

int main(int argc, const char** argv)
{
	if (argc == 1)
	{
		fprintf(stderr, "xcode-select: error: no command option given\n");
		printUsage();
	}
	else if (argc == 2 || argc == 3)
	{
		if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
		{
			printUsage();
		}
		else if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-version") == 0)
		{
			printf("xcode-select for Darling, version 1.0\n");
		}
		else if (strcmp(argv[1], "-p") == 0 || strcmp(argv[1], "--print-path") == 0 || strcmp(argv[1], "-print-path") == 0)
		{
			char path[1024];
			bool is_cmd_line;

			if (xcselect_get_developer_dir_path(path, sizeof(path), &is_cmd_line))
			{
				printf("%s\n", path);
			}
			else
			{
				fprintf(stderr, "xcode-select: error: unable to get active developer directory\n");
				return 1;
			}
		}
		else if (strcmp(argv[1], "--show-manpaths") == 0)
		{
			doPrintManPaths();
		}
		else if (strcmp(argv[1], "--install") == 0)
		{
			if (access("/Library/Developer/CommandLineTools/usr/lib/libxcrun.dylib", F_OK) == 0)
			{
				fprintf(stderr, "xcode-select: error: command line tools are already installed\n");
				return 1;
			}
			else
			{
				int status = system("/usr/libexec/darling/clt_install.py");
				return WEXITSTATUS(status);
			}
		}
		else if (strcmp(argv[1], "-r") == 0 || strcmp(argv[1], "--reset") == 0)
		{
			doReset();
		}
		else if (strcmp(argv[1], "-s") == 0 || strcmp(argv[1], "--switch") == 0 || strcmp(argv[1], "-switch") == 0)
		{
			if (argc != 3)
			{
				fprintf(stderr, "xcode-select: error: missing argument to '%s'\n", argv[1]);
				return 1;
			}

			doSwitch(argv[2]);
		}
		else
		{
			fprintf(stderr, "xcode-select: error: unknown option: %s\n", argv[1]);
			printUsage();
			return 1;
		}

		return 0;
	}
	else
	{
		fprintf(stderr, "xcode-select: error: bad argument count\n");
		printUsage();
	}
	return 1;
}

void printUsage(void)
{
	fprintf(stderr, "Usage: xcode-select [options]\n\n");
	fprintf(stderr, "xcode-select is used to set up path to the active developer directory.\n"
			"This affects both toolchain commands (such as clang or make) and Xcode-specific\n"
			"tools (such as xcodebuild).\n\n");
	fprintf(stderr, "Options:\n"
			"  -h, --help                  print this help message\n"
			"  -p, --print-path            print the path of the active developer directory\n"
			"  -s <path>, --switch <path>  change the path of the active developer directory\n"
			"  --install                   trigger the installation of command line developer tools\n"
			"  -v, --version               print the version of this tool\n"
			"  -r, --reset                 reset to the default developer directory\n");
}

static void killLink(const char* path)
{
	if (unlink(path) != 0)
	{
		if (errno != ENOENT)
		{
			fprintf(stderr, "xcode-select: error: cannot remove existing link at '%s': %s\n",
					path, strerror(errno));
			exit(1);
		}
	}
}

void doReset(void)
{
	killLink("/var/db/xcode_select_link");
	killLink("/usr/share/xcode-select/xcode_dir_link");
	killLink("/usr/share/xcode-select/xcode_dir_path");
}

void doSwitch(const char* path)
{
	char buffer[1024];
	bool unused;

	if (!xcselect_find_developer_contents_from_path(path, buffer, &unused, sizeof(buffer)))
	{
		fprintf(stderr, "xcode-select: error: invalid developer directory '%s'\n", path);
		exit(1);
	}

	doReset();

	umask(022);
	if (symlink(buffer, "/var/db/xcode_select_link") != 0)
	{
		fprintf(stderr, "xcode-select: error: unable to create symlink: %s\n",
				strerror(errno));
		exit(1);
	}
}

void doPrintManPaths(void)
{
	xcselect_manpaths *xcp;
	const char *path;
	unsigned i, count;

	xcp = xcselect_get_manpaths(NULL);
	if (xcp != NULL) {
		count = xcselect_manpaths_get_num_paths(xcp);
		for (i = 0; i < count; i++) {
			path = xcselect_manpaths_get_path(xcp, i);
			if (path != NULL) {
				puts(path);
			}
		}
		xcselect_manpaths_free(xcp);
	}
}

