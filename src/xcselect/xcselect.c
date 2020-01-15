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
#include "xcselect.h"

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

bool xcselect_find_developer_contents_from_path(const char* p, char* dst, bool* is_cmd_line, size_t dst_size)
{
	size_t length;

	if (*p != '/')
	{
		getcwd(dst, dst_size);
		strcat(dst, "/");
		strcat(dst, p);
	}
	else
		strlcpy(dst, p, dst_size);

	length = strlen(dst);
	if (valid_dev_path(dst))
		return true;

	dst[length++] = '/';
	dst[length] = 0;

	strcat(dst, "Library/Developer/CommandLineTools");
	if (valid_dev_path(dst))
	{
		*is_cmd_line = true;
		return true;
	}

	dst[length] = 0;
	strcat(dst, "CommandLineTools");
	if (valid_dev_path(dst))
	{
		*is_cmd_line = true;
		return true;
	}

	dst[length] = 0;
	strcat(dst, "Contents/Developer");
	if (valid_dev_path(dst))
		return true;

	return false;
}

bool xcselect_get_developer_dir_path(char* path, size_t path_len, bool* is_cmd_line)
{
	const char* p;
	char* slash;

	*is_cmd_line = false;

	p = getenv("DEVELOPER_DIR");
	if (p)
	{
		if (xcselect_find_developer_contents_from_path(p, path_buffer, is_cmd_line, sizeof(path_buffer)))
		{
			p = path_buffer;
			goto have_path;
		}
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
	else if (dir_exists("/Library/Developer/DarlingCLT"))
	{
		p = "/Library/Developer/DarlingCLT";
		*is_cmd_line = true;
		goto have_path;
	}

	return false;

have_path:
	slash = strrchr(p, '/');
	if (slash != NULL)
	{
		if (strcmp(slash+1, "CommandLineTools") == 0)
			*is_cmd_line = true;
	}

	strlcpy(path, p, path_len);
	strlcat(path, "/", path_len);
	return true;
}


int xcselect_invoke_xcrun(const char* tool, int argc, char* argv[], int flags)
{
	char dev_dir[1024];
	bool is_cmdline;

	if (xcselect_get_developer_dir_path(dev_dir, sizeof(dev_dir), &is_cmdline))
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
		if (buf[strlen(buf) - 1] != '/')
		{
			strcat(buf, "/");
		}
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
				fn(tool, argc, argv, dev_dir);
				fprintf(stderr, "xcrun: xcrun_main unexpectedly exited\n");
			}
		}
		else
		{
			char** argv2;
			int j = 0;

			buf[length] = 0;
			strcat(buf, "usr/bin/xcrun");

			// +3: buf, tool, NULL
			argv2 = (char**) __builtin_alloca((argc+3) * sizeof(char*));
			argv2[j++] = buf;

			if (tool != NULL)
				argv2[j++] = (char*) tool;

			for (int i = 0; i < argc; i++, j++)
				argv2[j] = argv[i];
			argv2[j] = NULL;

			execv(buf, argv2);
			fprintf(stderr, "xcrun: developer path '%s' is invalid, failed to execute '%s': %s\n",
					dev_dir, buf, strerror(errno));
		}
	}
	else 
	{
		if (dir_exists("/usr/libexec/DeveloperTools") && tool != NULL)
		{
			char* buf = __builtin_alloca(strlen(tool) + 30);
			strcpy(buf, "/usr/libexec/DeveloperTools/");
			strcat(buf, tool);

			if (access(buf, F_OK) == 0)
			{
				char** argv2 = (char **) __builtin_alloca((argc+1+1) * sizeof(char*));
				argv2[0] = buf;
				for (int i = 0; i < argc; i++)
					argv2[i+1] = argv[i];
				argv2[argc+1] = NULL;

				execv(buf, argv2);
				fprintf(stderr, "xcrun: failed to exec '%s': %s\n", buf, strerror(errno));
				exit(1);
			}
		}

		fprintf(stderr, "xcrun: cannot find developer tools, set DEVELOPER_DIR if you are using a non-standard location.\n");
	}

	exit(1);
}

struct __xcselect_manpaths
{
	unsigned int count;
	char** paths;
};

void xcselect_manpaths_append(xcselect_manpaths* paths, const char* path)
{
	paths->count++;
	paths->paths = realloc(paths->paths, sizeof(char*) * paths->count);
	paths->paths[paths->count - 1] = strdup(path);
}

xcselect_manpaths* xcselect_get_manpaths(const char* sdkname)
{
	char path[1024];
	bool unused;
	size_t len;
	xcselect_manpaths* rv;

	if (!xcselect_get_developer_dir_path(path, sizeof(path), &unused))
		return NULL;

	len = strlen(path);
	strcat(path, "usr/lib/libxcrun.dylib");

	rv = (xcselect_manpaths*) malloc(sizeof(*rv));
	memset(rv, 0, sizeof(*rv));

	if (access(path, F_OK) == 0)
	{
		void* module = dlopen(path, RTLD_LAZY);
		void (*fn)(const char* devpath, const char* sdkname, void (^)(const char*));

		if (!module)
		{
			free(rv);
			fprintf(stderr, "%s: error: cannot load libxcrun (%s)\n", getprogname(), dlerror());
			return NULL;
		}

		*((void**)&fn) = dlsym(module, "xcrun_iter_manpaths");

		if (fn != NULL)
		{
			path[len] = 0;
			fn(path, sdkname, ^(const char* path) {
				xcselect_manpaths_append(rv, path);
			});
		}

		dlclose(module);
	}

	// Add standard paths
	path[len] = 0;
	strcat(path, "usr/share/man");
	xcselect_manpaths_append(rv, path);

	path[len] = 0;
	strcat(path, "usr/llvm-gcc-4,2/share/man");
	xcselect_manpaths_append(rv, path);

	path[len] = 0;
	strcat(path, "Toolchains/XcodeDefault.xctoolchain/usr/share/man");
	xcselect_manpaths_append(rv, path);

	return rv;
}

unsigned int xcselect_manpaths_get_num_paths(xcselect_manpaths* p)
{
	return p->count;
}

const char* xcselect_manpaths_get_path(xcselect_manpaths* p, unsigned int idx)
{
	if (idx < xcselect_manpaths_get_num_paths(p))
		return p->paths[idx];
	else
		return NULL;
}

void xcselect_manpaths_free(xcselect_manpaths* p)
{
	for (unsigned int i = 0; i < p->count; i++)
		free(p->paths[i]);
	free(p->paths);
	free(p);
}

