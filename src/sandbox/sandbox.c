#include "sandbox.h"
#include <stddef.h>
#include <string.h>

// DUMMY implementation
const enum sandbox_filter_type SANDBOX_CHECK_NO_REPORT = SANDBOX_FILTER_NONE;

int sandbox_init(const char *profile, uint64_t flags, char **errorbuf)
{
	*errorbuf = strdup("Not implemented");
	return -1;
}

const char kSBXProfileNoInternet[] = "no_internet";

const char kSBXProfileNoNetwork[] = "no_network";

const char kSBXProfileNoWrite[] = "no_write";

const char kSBXProfileNoWriteExceptTemporary[] = "no_write_except_temporary";

const char kSBXProfilePureComputation[] = "pure_computation";

void sandbox_free_error(char *errorbuf)
{
	free(errorbuf);
}

int sandbox_init_with_parameters(const char *profile, uint64_t flags, const char *const parameters[], char **errorbuf)
{
	*errorbuf = strdup("Not implemented");
	return -1;
}

int sandbox_init_with_extensions(const char *profile, uint64_t flags, const char *const extensions[], char **errorbuf)
{
	*errorbuf = strdup("Not implemented");
	return -1;
}

int sandbox_check(pid_t pid, const char *operation, enum sandbox_filter_type type, ...)
{
	return -1;
}

int sandbox_note(const char *note)
{
	return -1;
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

const char *_amkrtemp(const char *unused)
{
	return NULL;
}

