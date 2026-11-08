#include "sandbox.h"
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <os/lock.h>
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

static bool sandbox_self_applied(void)
{
	bool applied;
	return sandbox_lookup(getpid(), &applied) && applied;
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

/* Extension tokens.
 *
 * A token names what it grants (a kind, the extension class and a path,
 * service or class name) in a self-contained string, so that it can be
 * consumed by another process than the one that issued it. The format is
 * Darling's own; clients treat tokens as opaque. Nothing is enforced:
 * consuming a token grants nothing a process did not have already.
 */
#define SANDBOX_TOKEN_PREFIX "darling-ext1"

enum sandbox_extension_kind {
	SANDBOX_EXT_FILE = 'f',
	SANDBOX_EXT_MACH = 'm',
	SANDBOX_EXT_GENERIC = 'g',
	SANDBOX_EXT_IOKIT = 'i',
};

static const char sandbox_hex_digits[] = "0123456789abcdef";

static char *sandbox_hex_append(char *out, const char *s)
{
	for (; *s != '\0'; s++) {
		*out++ = sandbox_hex_digits[(unsigned char) *s >> 4];
		*out++ = sandbox_hex_digits[(unsigned char) *s & 0xf];
	}
	return out;
}

static char *sandbox_extension_issue(enum sandbox_extension_kind kind, const char *extension_class, const char *target, uint32_t flags)
{
	if (extension_class == NULL || target == NULL) {
		errno = EINVAL;
		return NULL;
	}

	// prefix ; kind ; flags ; hex(class) ; hex(target)
	size_t length = sizeof(SANDBOX_TOKEN_PREFIX) + 2 + 9 + 2 * strlen(extension_class) + 1 + 2 * strlen(target) + 1;
	char *token = malloc(length);
	if (token == NULL) {
		errno = ENOMEM;
		return NULL;
	}

	char *out = token + snprintf(token, length, SANDBOX_TOKEN_PREFIX ";%c;%08x;", (char) kind, flags);
	out = sandbox_hex_append(out, extension_class);
	*out++ = ';';
	out = sandbox_hex_append(out, target);
	*out = '\0';

	return token;
}

static bool sandbox_hex_field_valid(const char *start, const char *end)
{
	if ((end - start) % 2 != 0) {
		return false;
	}
	for (const char *p = start; p < end; p++) {
		if (strchr(sandbox_hex_digits, *p) == NULL || *p == '\0') {
			return false;
		}
	}
	return true;
}

static bool sandbox_token_valid(const char *token)
{
	const size_t prefix_length = sizeof(SANDBOX_TOKEN_PREFIX) - 1;

	if (strncmp(token, SANDBOX_TOKEN_PREFIX ";", prefix_length + 1) != 0) {
		return false;
	}
	token += prefix_length + 1;

	switch (token[0]) {
		case SANDBOX_EXT_FILE: case SANDBOX_EXT_MACH: case SANDBOX_EXT_GENERIC: case SANDBOX_EXT_IOKIT:
			break;
		default:
			return false;
	}
	if (token[1] != ';') {
		return false;
	}
	token += 2;

	const char *flags_end = strchr(token, ';');
	if (flags_end == NULL || flags_end - token != 8 || !sandbox_hex_field_valid(token, flags_end)) {
		return false;
	}

	const char *class_start = flags_end + 1;
	const char *class_end = strchr(class_start, ';');
	if (class_end == NULL || class_end == class_start || !sandbox_hex_field_valid(class_start, class_end)) {
		return false;
	}

	const char *target_start = class_end + 1;
	const char *target_end = target_start + strlen(target_start);
	return target_end != target_start && sandbox_hex_field_valid(target_start, target_end);
}

/* The extensions this process consumed. Consuming the same token again
 * yields the same handle and needs one more release. */
struct sandbox_consumed_extension {
	char *token;
	int64_t handle;
	unsigned long uses;
};

static struct sandbox_consumed_extension *sandbox_consumed;
static size_t sandbox_consumed_count;
static int64_t sandbox_next_handle = 1;

static os_unfair_lock sandbox_consumed_lock_word = OS_UNFAIR_LOCK_INIT;

static void sandbox_consumed_lock(void)
{
	os_unfair_lock_lock(&sandbox_consumed_lock_word);
}

static void sandbox_consumed_unlock(void)
{
	os_unfair_lock_unlock(&sandbox_consumed_lock_word);
}

static int64_t sandbox_consumed_add(const char *token)
{
	int64_t handle = -1;

	sandbox_consumed_lock();

	for (size_t i = 0; i < sandbox_consumed_count; i++) {
		if (strcmp(sandbox_consumed[i].token, token) == 0) {
			sandbox_consumed[i].uses++;
			handle = sandbox_consumed[i].handle;
			goto out;
		}
	}

	struct sandbox_consumed_extension *grown = realloc(sandbox_consumed, (sandbox_consumed_count + 1) * sizeof(*grown));
	char *copy = strdup(token);
	if (grown != NULL) {
		sandbox_consumed = grown;
	}
	if (grown == NULL || copy == NULL) {
		free(copy);
		errno = ENOMEM;
		goto out;
	}

	handle = sandbox_next_handle++;
	sandbox_consumed[sandbox_consumed_count++] = (struct sandbox_consumed_extension) {
		.token = copy,
		.handle = handle,
		.uses = 1,
	};

out:
	sandbox_consumed_unlock();
	return handle;
}

static bool sandbox_consumed_remove(int64_t handle)
{
	bool found = false;

	sandbox_consumed_lock();

	for (size_t i = 0; i < sandbox_consumed_count; i++) {
		if (sandbox_consumed[i].handle == handle) {
			found = true;
			if (--sandbox_consumed[i].uses == 0) {
				free(sandbox_consumed[i].token);
				sandbox_consumed[i] = sandbox_consumed[--sandbox_consumed_count];
			}
			break;
		}
	}

	sandbox_consumed_unlock();
	return found;
}

char *sandbox_extension_issue_file(const char *extension_class, const char *path, uint32_t flags)
{
	return sandbox_extension_issue(SANDBOX_EXT_FILE, extension_class, path, flags);
}

char *sandbox_extension_issue_file_to_process(const char *extension_class, const char *path, uint32_t flags, audit_token_t target)
{
	// The token is not tied to the target process; nothing checks it
	return sandbox_extension_issue(SANDBOX_EXT_FILE, extension_class, path, flags);
}

char *sandbox_extension_issue_mach(const char *extension_class, const char *name, uint32_t flags)
{
	return sandbox_extension_issue(SANDBOX_EXT_MACH, extension_class, name, flags);
}

char *sandbox_extension_issue_mach_to_process(const char *extension_class, const char *name, uint32_t flags, audit_token_t target)
{
	return sandbox_extension_issue(SANDBOX_EXT_MACH, extension_class, name, flags);
}

char *sandbox_extension_issue_generic(const char *extension_class, uint32_t flags)
{
	// A generic extension names only its class
	return sandbox_extension_issue(SANDBOX_EXT_GENERIC, extension_class, extension_class, flags);
}

char *sandbox_extension_issue_iokit_registry_entry_class(const char *extension_class, const char *registry_entry_class, uint32_t flags)
{
	return sandbox_extension_issue(SANDBOX_EXT_IOKIT, extension_class, registry_entry_class, flags);
}

char *sandbox_extension_issue_iokit_registry_entry_class_to_process(const char *extension_class, const char *registry_entry_class, uint32_t flags, audit_token_t target)
{
	return sandbox_extension_issue(SANDBOX_EXT_IOKIT, extension_class, registry_entry_class, flags);
}

int64_t sandbox_extension_consume(const char *extension_token)
{
	if (extension_token == NULL || !sandbox_token_valid(extension_token)) {
		errno = EINVAL;
		return -1;
	}

	if (!sandbox_self_applied()) {
		// QUIRK: a process that is not sandboxed gets handle 0 for any
		// valid token, and releasing any handle succeeds (see
		// sandbox_extension_release). Not documented. Evidence: black-box
		// observation on macOS 27.0.1 arm64 (tier 2; lol-upstream/impl/P95.md).
		// Test: quirk_unsandboxed_consume_gives_handle_0 in
		// tests/src/sandbox_state.c.
		return 0;
	}

	return sandbox_consumed_add(extension_token);
}

int sandbox_extension_release(int64_t extension_handle)
{
	if (!sandbox_self_applied()) {
		// QUIRK: see sandbox_extension_consume
		return 0;
	}

	// QUIRK: in a sandboxed process too, releasing handle 0 (what an
	// unsandboxed consume returns) succeeds, while any other handle that is
	// not held fails with EINVAL. Not documented. Evidence: black-box
	// observation on macOS 27.0.1 arm64 (tier 2; lol-upstream/impl/P95.md).
	// Test: checkSandboxedExtensions in tests/src/sandbox_state.c.
	if (extension_handle == 0) {
		return 0;
	}

	if (!sandbox_consumed_remove(extension_handle)) {
		errno = EINVAL;
		return -1;
	}
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