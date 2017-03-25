/*
This file is part of Darling.

Copyright (C) 2017 Lubos Dolezel

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
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <dlfcn.h>
#include <stdio.h>
#include "libxcselect.h"

static char path_buffer[1024];

static const char* get_developer_dir_from_file(const char* file)
{
	int fd = open(file, O_RDONLY);
	if (fd == -1)
		return NULL;

	int len = read(fd, path_buffer, sizeof(path_buffer)-1);
	if (len <= 0)
	{
		close(fd);
		return NULL;
	}

	path_buffer[len] = 0;
	close(fd);

	return path_buffer;
}

const char* get_developer_dir_from_symlink(const char* link)
{
	ssize_t len;

	len = readlink(link, path_buffer, sizeof(path_buffer)-1);
	if (len <= 0)
		return NULL;

	path_buffer[len] = 0;
	return path_buffer;
}

static bool dir_exists(const char* dir)
{
	struct stat st;

	if (stat(dir, &st) != 0)
		return false;

	return S_ISDIR(st.st_mode);
}

static bool valid_dev_path(const char* path)
{
	char buffer[1024];
	size_t length;

	strcpy(buffer, path);
	strcat(buffer, "/");
	
	length = strlen(buffer);
	strcat(buffer, "usr/lib/libxcrun.dylib");
	if (access(buffer, F_OK) == 0)
		return true;

	buffer[length] = 0;
	strcat(buffer, "usr/bin/xcrun");
	if (access(buffer, F_OK) == 0)
		return true;

	return false;
}

const char* xcselect_find_developer_contents_from_path(const char* p, bool* is_cmd_line)
{
	size_t length;

	if (*p != '/')
	{
		getcwd(path_buffer, sizeof(path_buffer));
		strcat(path_buffer, "/");
		strcat(path_buffer, p);
	}
	else
		strcpy(path_buffer, p);

	length = strlen(path_buffer);
	if (valid_dev_path(path_buffer))
		return path_buffer;

	path_buffer[length++] = '/';
	path_buffer[length] = 0;

	strcat(path_buffer, "Library/Developer/CommandLineTools");
	if (valid_dev_path(path_buffer))
	{
		*is_cmd_line = true;
		return path_buffer;
	}

	path_buffer[length] = 0;
	strcat(path_buffer, "CommandLineTools");
	if (valid_dev_path(path_buffer))
	{
		*is_cmd_line = true;
		return path_buffer;
	}

	path_buffer[length] = 0;
	strcat(path_buffer, "Contents/Developer");
	if (valid_dev_path(path_buffer))
		return path_buffer;

	return NULL;
}

const char* xcselect_get_developer_dir_path(bool* is_cmd_line)
{
	const char* p;
	char* slash;

	*is_cmd_line = false;

	p = getenv("DEVELOPER_DIR");
	if (p)
	{
		p = xcselect_find_developer_contents_from_path(p, is_cmd_line);
		if (p)
			goto have_path;
	}

	p = get_developer_dir_from_symlink("/var/db/xcode_select_link");
	if (p)
		goto have_path;

	p = get_developer_dir_from_symlink("/usr/share/xcode-select/xcode_dir_link");
	if (p)
		goto have_path;

	p = get_developer_dir_from_file("/usr/share/xcode-select/xcode_dir_path");
	if (p)
		goto have_path;

	if (dir_exists("/Applications/Xcode.app"))
	{
		p = "/Applications/Xcode.app/Contents/Developer";
		goto have_path;
	}
	else if (dir_exists("/Library/Developer/CommandLineTools"))
	{
		p = "/Library/Developer/CommandLineTools";
		goto have_path;
	}

	return NULL;

have_path:
	slash = strrchr(p, '/');
	if (slash != NULL)
	{
		if (strcmp(slash+1, "CommandLineTools") == 0)
			*is_cmd_line = true;
	}

	return p;
}


int xcselect_invoke_xcrun(const char* tool, int argc, char* argv[], int flags)
{
	const char* dev_dir;
	bool is_cmdline;

	dev_dir = xcselect_get_developer_dir_path(&is_cmdline);

	if (dev_dir != NULL)
	{
		char* buf = (char*) malloc(2048);
		size_t length;
		void* lib;

		if (is_cmdline && (flags & XCSELECT_FLAG_REQUIRE_XCODE))
		{
			fprintf(stderr, "xcrun: tool '%s' requires Xcode installation, command line tools are insufficient.\n", tool);
			exit(1);
		}

		strcpy(buf, dev_dir);
		strcat(buf, "/");
		length = strlen(buf);

		strcat(buf, "usr/lib/libxcrun.dylib");
		lib = dlopen(buf, RTLD_LAZY);

		if (lib != NULL)
		{
			void(*fn)(const char*, int, char**, const char*);

			*((void**)&fn) = dlsym(lib, "xcrun_main");
			if (!fn)
			{
				fprintf(stderr, "xcrun: broken libxcrun.dylib at '%s'\n", buf);
			}
			else
			{
				fn(getprogname(), argc, argv, dev_dir);
			}
		}
		else
		{
			char** argv2;
			int j = 0;

			buf[length] = 0;
			strcat(buf, "usr/bin/xcrun");

			argv2 = (char**) __builtin_alloca((argc+2) * sizeof(char*));
			argv2[j++] = buf;

			if (tool != NULL)
				argv2[j++] = (char*) tool;

			for (int i = 0; i < argc; i++, j++)
				argv2[j] = argv[i];

			execv(buf, argv2);
			fprintf(stderr, "xcrun: developer path '%s' is invalid, failed to execute '%s': %s\n",
					dev_dir, buf, strerror(errno));
		}
	}
	else if (dir_exists("/usr/libexec/DeveloperTools") && tool != NULL)
	{
		char* buf = __builtin_alloca(strlen(tool) + 30);
		strcpy(buf, "/usr/libexec/DeveloperTools/");
		strcat(buf, tool);

		execv(buf, argv);
		fprintf(stderr, "xcrun: failed to exec '%s': %s\n", buf, strerror(errno));
	}
	else
	{
		fprintf(stderr, "xcrun: cannot find developer tools, set DEVELOPER_DIR if you are using a non-standard location.\n");
	}

	exit(1);
}

