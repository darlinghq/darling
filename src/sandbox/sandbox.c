#include "sandbox.h"
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

// DUMMY implementation
const enum sandbox_filter_type SANDBOX_CHECK_NO_REPORT = SANDBOX_FILTER_NONE;

int sandbox_init(const char *profile, uint64_t flags, char **errorbuf)
{
	*errorbuf = strdup("Not implemented");
	return 0;
}

const char kSBXProfileNoInternet[] = "no_internet";

const char kSBXProfileNoNetwork[] = "no_network";

const char kSBXProfileNoWrite[] = "no_write";

const char kSBXProfileNoWriteExceptTemporary[] = "no_write_except_temporary";

const char kSBXProfilePureComputation[] = "pure_computation";

const char APP_SANDBOX_IOKIT_CLIENT[] = "com.apple.app-sandbox.iokit-client";
const char APP_SANDBOX_MACH[] = "com.apple.app-sandbox.mach";
const char APP_SANDBOX_READ[] = "com.apple.app-sandbox.read";
const char APP_SANDBOX_READ_WRITE[] = "com.apple.app-sandbox.read-write";
const char IOS_SANDBOX_APPLICATION_GROUP[] = "com.apple.sandbox.application-group";
const char IOS_SANDBOX_CONTAINER[] = "com.apple.sandbox.container";

// Assuming these are 64-bit only because at least one has a value
// that can't be stored in a 32-bit integer.
#ifdef __LP64__

const uint64_t SANDBOX_EXTENSION_CANONICAL = 0x0000001000000002;
const uint64_t SANDBOX_EXTENSION_DEFAULT = 0x0000000400000000;
const uint64_t SANDBOX_EXTENSION_MAGIC = 0x0000000000000001;
const uint64_t SANDBOX_EXTENSION_NO_REPORT = 0x0000000100000010;
const uint64_t SANDBOX_EXTENSION_PREFIXMATCH = 0x0000000200000004;
const uint64_t SANDBOX_EXTENSION_UNRESOLVED = 0x0000000100000001;

#endif

void sandbox_free_error(char *errorbuf)
{
	free(errorbuf);
}

int sandbox_init_with_parameters(const char *profile, uint64_t flags, const char *const parameters[], char **errorbuf)
{
	*errorbuf = strdup("Not implemented");
	return 0;
}

int sandbox_init_with_extensions(const char *profile, uint64_t flags, const char *const extensions[], char **errorbuf)
{
	*errorbuf = strdup("Not implemented");
	return 0;
}

int sandbox_check(pid_t pid, const char *operation, enum sandbox_filter_type type, ...)
{
	return 0;
}

int sandbox_note(const char *note)
{
	printf("%s\n", note);
	return 0;
}

int sandbox_suspend(pid_t pid)
{
	return -1;
}

int sandbox_unsuspend(void)
{
	return -1;
}

int sandbox_issue_extension(const char *path, char **ext_token)
{
	return -1;
}

int sandbox_issue_fs_extension(const char *path, uint64_t flags, char **ext_token)
{
	return -1;
}

int sandbox_issue_fs_rw_extension(const char *path, char **ext_token)
{
	return -1;
}

int sandbox_issue_mach_extension(const char *name, char **ext_token)
{
	return -1;
}

int sandbox_consume_extension(const char *path, const char *ext_token)
{
	return -1;
}

int sandbox_consume_fs_extension(const char *ext_token, char **path)
{
	return -1;
}

int sandbox_consume_mach_extension(const char *ext_token, char **name)
{
	return -1;
}

int sandbox_release_fs_extension(const char *ext_token)
{
	return -1;
}

int sandbox_container_path_for_pid(pid_t pid, char *buffer, size_t bufsize)
{
	return -1;
}

int sandbox_wakeup_daemon(char **errorbuf)
{
	*errorbuf = strdup("Not implemented");
	return -1;
}

const char *_amkrtemp(const char *path)
{
	size_t len = strlen(path);
	const char suffix[] = ".amkrtempXXXXXX";
	char *template = malloc(len + sizeof(suffix));
	memcpy(template, path, len);
	memcpy(template + len, suffix, sizeof(suffix));
	return mktemp(template);
}

int rootless_allows_task_for_pid(pid_t pid) {
	return 1;
}
