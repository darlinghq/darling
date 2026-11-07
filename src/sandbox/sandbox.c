#include "sandbox.h"
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <darling/emulation/linux_premigration/ext/for-sandbox.h>

// Darling enforces no sandbox profile. What it keeps is the state macOS
// reports: whether a process applied a profile (held by darlingserver, so it
// is inherited by fork and kept across exec), and the extensions a process
// consumed.
const enum sandbox_filter_type SANDBOX_CHECK_NO_REPORT = SANDBOX_FILTER_NONE;

static void sandbox_record_applied(void)
{
	/* Only the reported state depends on this. If darlingserver cannot
	 * record it, the process is reported as not sandboxed; applying the
	 * profile still succeeds, as nothing is enforced either way. */
	(void) __darling_sandbox_mark_applied();
}

/* Looks up whether the process applied a sandbox profile.
 * Returns false if there is no such process. */
static bool sandbox_lookup(pid_t pid, bool *applied)
{
	*applied = false;

	// pid 0 is the kernel, which is never sandboxed
	if (pid == 0) {
		return true;
	}

	// Other failures (e.g. an older darlingserver) report "not sandboxed"
	return __darling_sandbox_is_applied(pid, applied) != ESRCH;
}

int sandbox_init(const char *profile, uint64_t flags, char **errorbuf)
{
	/* Darling does not enforce seatbelt profiles: this succeeds and only
	 * records that the process is sandboxed (see sandbox_check). errorbuf
	 * may be NULL, and a successful call reports no error. */
	sandbox_record_applied();
	if (errorbuf != NULL) {
		*errorbuf = NULL;
	}
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
	/* No enforcement; see sandbox_init. */
	sandbox_record_applied();
	if (errorbuf != NULL) {
		*errorbuf = NULL;
	}
	return 0;
}

int sandbox_init_with_extensions(const char *profile, uint64_t flags, const char *const extensions[], char **errorbuf)
{
	/* No enforcement; see sandbox_init. Extensions are accepted and ignored. */
	sandbox_record_applied();
	if (errorbuf != NULL) {
		*errorbuf = NULL;
	}
	return 0;
}

int sandbox_check(pid_t pid, const char *operation, enum sandbox_filter_type type, ...)
{
	bool applied;

	if (!sandbox_lookup(pid, &applied)) {
		// QUIRK: for a pid that names no process (none, an exited one, or
		// a negative one), sandbox_check returns 1 and leaves errno alone,
		// with or without an operation. Not documented. Evidence: black-box
		// observation on macOS 27.0.1 arm64 (tier 2; lol-upstream/impl/P95.md).
		// Test: quirk_unknown_pid_reports_1 in tests/src/sandbox_state.c.
		return 1;
	}

	// The NULL operation asks whether the process is sandboxed
	if (operation == NULL) {
		return applied ? 1 : 0;
	}

	// Nothing is enforced, so every operation is allowed
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
	/* No enforcement: hand out a placeholder token that callers can consume
	 * and release. */
	if (ext_token != NULL) {
		*ext_token = strdup("darling-sandbox-stub");
	}
	return 0;
}

int sandbox_issue_fs_extension(const char *path, uint64_t flags, char **ext_token)
{
	if (ext_token != NULL) {
		*ext_token = strdup("darling-sandbox-stub");
	}
	return 0;
}

int sandbox_issue_fs_rw_extension(const char *path, char **ext_token)
{
	if (ext_token != NULL) {
		*ext_token = strdup("darling-sandbox-stub");
	}
	return 0;
}

int sandbox_issue_mach_extension(const char *name, char **ext_token)
{
	if (ext_token != NULL) {
		*ext_token = strdup("darling-sandbox-stub");
	}
	return 0;
}

int sandbox_consume_extension(const char *path, const char *ext_token)
{
	return 0;
}

int sandbox_consume_fs_extension(const char *ext_token, char **path)
{
	return 0;
}

int sandbox_consume_mach_extension(const char *ext_token, char **name)
{
	return 0;
}

int sandbox_release_fs_extension(const char *ext_token)
{
	return 0;
}

/* No enforcement: issued tokens are placeholders that can be consumed and
 * released, and a consumed token yields a positive handle. */
static int64_t sandbox_extension_next_handle = 1;

static char *sandbox_extension_placeholder_token(const char *extension_class)
{
	if (extension_class == NULL) {
		return NULL;
	}
	return strdup("darling-sandbox-stub");
}

char *sandbox_extension_issue_file(const char *extension_class, const char *path, uint32_t flags)
{
	if (path == NULL) {
		return NULL;
	}
	return sandbox_extension_placeholder_token(extension_class);
}

char *sandbox_extension_issue_mach(const char *extension_class, const char *name, uint32_t flags)
{
	if (name == NULL) {
		return NULL;
	}
	return sandbox_extension_placeholder_token(extension_class);
}

int64_t sandbox_extension_consume(const char *extension_token)
{
	if (extension_token == NULL) {
		return -1;
	}
	return __atomic_fetch_add(&sandbox_extension_next_handle, 1, __ATOMIC_RELAXED);
}

int sandbox_extension_release(int64_t extension_handle)
{
	return 0;
}

int sandbox_container_path_for_pid(pid_t pid, char *buffer, size_t bufsize)
{
	return -1;
}

int sandbox_wakeup_daemon(char **errorbuf)
{
	/* There is no sandbox daemon to wake, so this fails, and a failure
	 * comes with a message for sandbox_free_error. */
	if (errorbuf != NULL) {
		*errorbuf = strdup("Not implemented");
	}
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

int sandbox_check_by_audit_token(audit_token_t tok, const char* operation, enum sandbox_filter_type filt, ...) {
	// The process ID is the sixth word of an audit token (see audit_token_to_au32(3))
	return sandbox_check((pid_t) tok.val[5], operation, filt);
};

int rootless_check_trusted(const char* path) {
	// also unimplemented
	// Darling doesn't have rootless anyways so just return a value indicating trust
	return 0;
};

int rootless_restricted_environment() {
	// also unimplemented
	// Darling doesn't have rootless anyways, so just return a value indicating an unrestricted environment
	return 0;
};

int rootless_check_datavault_flag(const char* path, const char* storage_class) {
	// also unimplemented
	// not quite sure what a "data vault" is here, but its usage in `JSScript.mm` in JavaScriptCore seems to indicate that we should return `0`
	return 0;
};

int sandbox_query_approval_policy_for_path(const char* query, const char* path, char **approval) {
	return -1;
}