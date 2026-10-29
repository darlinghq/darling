/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Darling Developers
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

// darpath: converts paths between the Linux host and Darling, like cygpath does for Windows. It is
// built twice: for Darling (run inside the container) and for the Linux host.

#include <darling-config.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// Returns the prefix's path on the host
static const char* hostPrefixPath(void)
{
#ifdef __APPLE__
	// set by the launcher
	return getenv("DARLING_PREFIX");
#else
	// same rule as the launcher
	static char path[PATH_MAX];
	const char* prefix = getenv("DPREFIX");
	const char* home = getenv("HOME");

	if (prefix != NULL)
		return prefix;
	if (home == NULL)
		return NULL;
	snprintf(path, sizeof(path), "%s/.darling", home);
	return path;
#endif
}

#ifdef __APPLE__
// where Darling's root and the host's root are, as seen from here
#define DARLING_ROOT ""
#define HOST_ROOT SYSTEM_ROOT
#else
#define DARLING_ROOT hostPrefixPath()
#define HOST_ROOT ""
#endif

static void usage(FILE* out)
{
	fprintf(out,
		"Usage: darpath [-d | -l] PATH...\n"
		"Convert paths between the Linux host and Darling.\n"
		"\n"
		"  -d  convert Linux host paths to Darling paths (default)\n"
		"  -l  convert Darling paths to Linux host paths\n"
		"  -h  show this help\n");
}

// Makes path absolute (relative to the working directory) and removes ".", ".." and repeated slashes
static bool normalize(const char* path, char* out, size_t size)
{
	char buf[PATH_MAX * 2];
	char* save;
	size_t len = 0;

	if (path[0] == '/')
		snprintf(buf, sizeof(buf), "%s", path);
	else
	{
		char cwd[PATH_MAX];
		if (getcwd(cwd, sizeof(cwd)) == NULL)
			return false;
		snprintf(buf, sizeof(buf), "%s/%s", cwd, path);
	}

	out[0] = '\0';
	for (char* part = strtok_r(buf, "/", &save); part != NULL; part = strtok_r(NULL, "/", &save))
	{
		if (strcmp(part, ".") == 0)
			continue;
		if (strcmp(part, "..") == 0)
		{
			char* slash = strrchr(out, '/');
			if (slash != NULL)
				*slash = '\0';
			len = strlen(out);
			continue;
		}
		size_t partLen = strlen(part);
		if (len + 1 + partLen + 1 > size)
			return false;
		out[len++] = '/';
		memcpy(out + len, part, partLen + 1);
		len += partLen;
	}

	if (len == 0)
		snprintf(out, size, "/");
	return true;
}

// If path is base or below it, sets *rest to the remainder ("" for base itself)
static bool underDirectory(const char* path, const char* base, const char** rest)
{
	size_t len = strlen(base);

	if (strncmp(path, base, len) != 0 || (path[len] != '/' && path[len] != '\0'))
		return false;
	*rest = path + len;
	return true;
}

// Returns true if the first component of path is a link to the same directory on the host (see the
// launcher's linkHostDirectories())
static bool inLinkedHostDirectory(const char* path)
{
	char component[PATH_MAX];
	char link[PATH_MAX * 2];
	char target[PATH_MAX];
	char expected[sizeof(SYSTEM_ROOT) + PATH_MAX];
	const char* darlingRoot = DARLING_ROOT;
	const char* end = strchr(path + 1, '/');
	size_t len = end ? (size_t)(end - path) : strlen(path);
	ssize_t targetLen;

	if (darlingRoot == NULL || len <= 1 || len >= sizeof(component))
		return false;
	memcpy(component, path, len);
	component[len] = '\0';

	snprintf(link, sizeof(link), "%s%s", darlingRoot, component);
	targetLen = readlink(link, target, sizeof(target) - 1);
	if (targetLen < 0)
		return false;
	target[targetLen] = '\0';

	snprintf(expected, sizeof(expected), "%s%s", SYSTEM_ROOT, component);
	return strcmp(target, expected) == 0;
}

static void printPath(const char* rest)
{
	puts(rest[0] != '\0' ? rest : "/");
}

static bool toDarling(const char* path)
{
	char norm[PATH_MAX];
	const char* hostPrefix = hostPrefixPath();
	const char* rest;

	// the working directory is the same directory on both sides, so relative paths stay as they are
	if (path[0] != '/')
	{
		puts(path);
		return true;
	}
	if (!normalize(path, norm, sizeof(norm)))
		return false;

	if (hostPrefix != NULL && underDirectory(norm, hostPrefix, &rest))
		printPath(rest);
	else if (underDirectory(norm, LIBEXEC_PATH, &rest))
		printPath(rest);
	else if (inLinkedHostDirectory(norm))
		puts(norm);
	else
		printf("%s%s\n", SYSTEM_ROOT, strcmp(norm, "/") == 0 ? "" : norm);
	return true;
}

static bool toLinux(const char* path)
{
	char norm[PATH_MAX];
	char installed[PATH_MAX * 2];
	char installedHere[PATH_MAX * 3];
	const char* hostPrefix;
	const char* rest;
	const char* sub;
	struct stat st;

	if (!normalize(path, norm, sizeof(norm)))
		return false;

	if (underDirectory(norm, SYSTEM_ROOT, &rest))
	{
		printPath(rest);
		return true;
	}
	if (inLinkedHostDirectory(norm))
	{
		puts(norm);
		return true;
	}

	hostPrefix = hostPrefixPath();
	if (hostPrefix == NULL)
	{
#ifdef __APPLE__
		fprintf(stderr, "darpath: DARLING_PREFIX is not set\n");
#else
		fprintf(stderr, "darpath: cannot determine the prefix; set DPREFIX or HOME\n");
#endif
		return false;
	}

	// The prefix is an overlay of the user's changes (the prefix directory on the host) on the
	// installed files. A path maps to the prefix if the user changed or created it there, otherwise to
	// the installed files.
	sub = strcmp(norm, "/") == 0 ? "" : norm;
	snprintf(installed, sizeof(installed), "%s%s", LIBEXEC_PATH, sub);
#ifndef __APPLE__
	{
		char inPrefix[PATH_MAX * 2];
		snprintf(inPrefix, sizeof(inPrefix), "%s%s", hostPrefix, sub);
		if (lstat(inPrefix, &st) == 0)
		{
			puts(inPrefix);
			return true;
		}
	}
#endif
	// ponytail: inside the container only the merged view is visible, so an installed file modified
	// there maps to the installed copy; telling them apart would need the prefix's upper layer
	snprintf(installedHere, sizeof(installedHere), "%s%s", HOST_ROOT, installed);
	if (lstat(installedHere, &st) == 0)
		puts(installed);
	else
		printf("%s%s\n", hostPrefix, sub);
	return true;
}

int main(int argc, char** argv)
{
	bool toLinuxPaths = false;
	int opt;
	int status = 0;

	while ((opt = getopt(argc, argv, "dlh")) != -1)
	{
		switch (opt)
		{
			case 'd':
				toLinuxPaths = false;
				break;
			case 'l':
				toLinuxPaths = true;
				break;
			case 'h':
				usage(stdout);
				return 0;
			default:
				usage(stderr);
				return 1;
		}
	}

	if (optind >= argc)
	{
		usage(stderr);
		return 1;
	}

	for (int i = optind; i < argc; i++)
	{
		if (!(toLinuxPaths ? toLinux(argv[i]) : toDarling(argv[i])))
		{
			fprintf(stderr, "darpath: cannot convert %s\n", argv[i]);
			status = 1;
		}
	}
	return status;
}
