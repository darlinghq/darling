/* Conservative classification of an existing prefix before initialization. */
#ifndef DARLING_PREFIX_STATE_H
#define DARLING_PREFIX_STATE_H

#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

enum darling_prefix_state {
	DARLING_PREFIX_EMPTY = 0,
	DARLING_PREFIX_INITIALIZED,
	DARLING_PREFIX_UNINITIALIZED,
};

static inline bool darlingPrefixPathIsDirectory(const char* prefix, const char* suffix)
{
	char path[4096];
	int length = snprintf(path, sizeof(path), "%s/%s", prefix, suffix);
	struct stat st;
	return length > 0 && (size_t)length < sizeof(path) && stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

static inline bool darlingPrefixPathIsRegularFile(const char* prefix, const char* suffix)
{
	char path[4096];
	int length = snprintf(path, sizeof(path), "%s/%s", prefix, suffix);
	struct stat st;
	return length > 0 && (size_t)length < sizeof(path) && stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

static inline enum darling_prefix_state darlingClassifyPrefix(const char* prefix)
{
	DIR* directory = opendir(prefix);
	if (!directory)
		return DARLING_PREFIX_UNINITIALIZED;

	bool empty = true;
	struct dirent* entry;
	while ((entry = readdir(directory)) != NULL) {
		if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
			empty = false;
			break;
		}
	}
	closedir(directory);
	if (empty)
		return DARLING_PREFIX_EMPTY;

	/* These files/directories are created by setupPrefix and survive shutdown.
	 * Require all of them before accepting an existing non-empty prefix. */
	if (darlingPrefixPathIsDirectory(prefix, "Volumes") &&
		darlingPrefixPathIsDirectory(prefix, "usr") &&
		darlingPrefixPathIsDirectory(prefix, "var/run") &&
		darlingPrefixPathIsRegularFile(prefix, "private/etc/passwd") &&
		darlingPrefixPathIsRegularFile(prefix, "private/etc/master.passwd") &&
		darlingPrefixPathIsRegularFile(prefix, "private/etc/group"))
		return DARLING_PREFIX_INITIALIZED;

	return DARLING_PREFIX_UNINITIALIZED;
}

#endif
