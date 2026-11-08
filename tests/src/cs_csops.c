// Copyright (C) 2026 Lubos Dolezel
//
// csops() and csops_audittoken() against the documented contract, for this
// program, which is not signed. Before mldr registers images, darlingserver
// knows nothing about its signature; after, it knows it has none. Under the
// default policy (`off`) both report the same status. Exits by itself.
//
// - CS_OPS_STATUS reports the process as valid (without enforcement, an
//   unsigned image keeps the status an unregistered one has);
// - an unknown process gives ESRCH, an unknown operation EINVAL;
// - CDHASH needs a 20-byte buffer (EINVAL), and the image has no CDHash
//   (EINVAL while unregistered, EBADEXEC once registered as unsigned);
// - IDENTITY and BLOB need room for the 8-byte header (ERANGE); without an
//   identifier, IDENTITY and TEAMID give ENOENT; BLOB gives an empty header;
// - QUIRK test_quirk_no_entitlements_leaves_buffer: without entitlements,
//   ENTITLEMENTS_BLOB succeeds and leaves the buffer untouched (macOS 27);
// - CLEARPLATFORM gives ENOTSUP; CLEAR_LV works for the process itself and is
//   refused (EPERM) for another one;
// - the flag-changing operations on another process are refused for a
//   process that is not root (EPERM), and allowed for root;
// - csops_audittoken needs a token (EINVAL) that names the process (ESRCH).
#include <errno.h>
#include <mach/mach.h>
#include <pwd.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/codesign.h>
#include <sys/wait.h>
#include <unistd.h>

static int failures;

static void check(int ok, const char* what)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		failures++;
}

static void finish(void)
{
	printf("%s\n", failures ? "FAILED" : "ALL PASSED");
	fflush(stdout);
	_exit(failures ? 1 : 0);
}

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	failures++;
	finish();
}

static int fails_with(int rc, int err)
{
	return rc == -1 && errno == err;
}

static void test_quirk_no_entitlements_leaves_buffer(void)
{
	unsigned char buffer[64];
	int untouched = 1;

	memset(buffer, 0xaa, sizeof(buffer));
	errno = 0;
	int rc = csops(0, CS_OPS_ENTITLEMENTS_BLOB, buffer, sizeof(buffer));
	for (size_t i = 0; i < sizeof(buffer); i++)
		untouched = untouched && buffer[i] == 0xaa;
	check(rc == 0, "QUIRK ENTITLEMENTS_BLOB without entitlements succeeds");
	check(untouched, "QUIRK ENTITLEMENTS_BLOB without entitlements leaves the buffer untouched");
}

// runs csops(target, op) in a child as `uid`; returns its errno (0 on success)
static int csops_as(uid_t uid, pid_t target, unsigned int op)
{
	int channel[2];
	if (pipe(channel) != 0)
		return -1;

	fflush(stdout);
	pid_t child = fork();
	if (child == 0) {
		close(channel[0]);
		int result = -1;
		if (uid == 0 || (setuid(uid) == 0 && getuid() == uid)) {
			errno = 0;
			result = (csops(target, op, NULL, 0) == 0) ? 0 : errno;
		}
		write(channel[1], &result, sizeof(result));
		_exit(0);
	}
	close(channel[1]);

	int result = -1;
	if (child <= 0 || read(channel[0], &result, sizeof(result)) != sizeof(result))
		result = -1;
	close(channel[0]);
	if (child > 0)
		waitpid(child, NULL, 0);
	return result;
}

int main(void)
{
	signal(SIGALRM, on_alarm);
	alarm(20);

	uint32_t flags = 0;
	unsigned char buffer[512];
	int rc;

	errno = 0;
	rc = csops(0, CS_OPS_STATUS, &flags, sizeof(flags));
	check(rc == 0 && (flags & CS_VALID), "STATUS of an unsigned process: CS_VALID");
	flags = 0;
	rc = csops(getpid(), CS_OPS_STATUS, &flags, sizeof(flags));
	check(rc == 0 && (flags & CS_VALID), "STATUS by PID");
	check(csops(0, CS_OPS_STATUS, NULL, 0) == 0, "STATUS without a buffer succeeds");

	errno = 0;
	check(fails_with(csops(999999, CS_OPS_STATUS, &flags, sizeof(flags)), ESRCH), "STATUS of an unknown process: ESRCH");
	errno = 0;
	check(fails_with(csops(0, 9999, buffer, sizeof(buffer)), EINVAL), "unknown operation: EINVAL");

	errno = 0;
	check(fails_with(csops(0, CS_OPS_CDHASH, buffer, 19), EINVAL), "CDHASH with 19 bytes: EINVAL");
	errno = 0;
	rc = csops(0, CS_OPS_CDHASH, buffer, 20);
	check(fails_with(rc, EINVAL) || fails_with(rc, EBADEXEC), "CDHASH of an unsigned image: EINVAL (unregistered) or EBADEXEC");

	errno = 0;
	check(fails_with(csops(0, CS_OPS_IDENTITY, buffer, 4), ERANGE), "IDENTITY with 4 bytes: ERANGE");
	errno = 0;
	check(fails_with(csops(0, CS_OPS_IDENTITY, buffer, sizeof(buffer)), ENOENT), "IDENTITY without an identifier: ENOENT");
	errno = 0;
	check(fails_with(csops(0, CS_OPS_TEAMID, buffer, sizeof(buffer)), ENOENT), "TEAMID without a team: ENOENT");

	errno = 0;
	check(fails_with(csops(0, CS_OPS_BLOB, buffer, 4), ERANGE), "BLOB with 4 bytes: ERANGE");
	memset(buffer, 0xaa, sizeof(buffer));
	rc = csops(0, CS_OPS_BLOB, buffer, sizeof(buffer));
	check(rc == 0 && buffer[0] == 0 && buffer[7] == 0 && buffer[8] == 0xaa, "BLOB: an empty 8-byte header");

	test_quirk_no_entitlements_leaves_buffer();

	int64_t offset = -1;
	check(csops(0, CS_OPS_PIDOFFSET, &offset, sizeof(offset)) == 0 && offset == 0, "PIDOFFSET: 0 for a thin image");

	errno = 0;
	check(fails_with(csops(0, CS_OPS_CLEARPLATFORM, NULL, 0), ENOTSUP), "CLEARPLATFORM: ENOTSUP");
	check(csops(0, CS_OPS_CLEAR_LV, NULL, 0) == 0, "CLEAR_LV for the process itself succeeds (not refused by default)");
	errno = 0;
	check(fails_with(csops(1, CS_OPS_CLEAR_LV, NULL, 0), EPERM), "CLEAR_LV for another process: EPERM");
	check(csops(0, CS_OPS_MARKRESTRICT, NULL, 0) == 0, "MARKRESTRICT of the process itself succeeds");

	if (getuid() == 0) {
		// macOS's "nobody" is uid -2; use it when the user database has no entry. A negative
		// uid does not reach darlingserver yet (its uidgid call takes it for "unchanged"),
		// so then any other unprivileged user is used: daemon, uid 1
		struct passwd* nobody = getpwnam("nobody");
		uid_t other = nobody ? nobody->pw_uid : (uid_t) -2;
		if ((int32_t) other < 0) {
			struct passwd* daemon_user = getpwnam("daemon");
			other = daemon_user ? daemon_user->pw_uid : 1;
		}
		check(csops_as(other, 1, CS_OPS_MARKRESTRICT) == EPERM, "MARKRESTRICT of another process, not root: EPERM");
		check(csops_as(0, 1, CS_OPS_MARKRESTRICT) == 0, "MARKRESTRICT of another process, root: allowed");
	} else {
		printf("SKIP: the cross-process checks need root\n");
	}

	audit_token_t token;
	mach_msg_type_number_t count = TASK_AUDIT_TOKEN_COUNT;
	check(task_info(mach_task_self(), TASK_AUDIT_TOKEN, (task_info_t) &token, &count) == KERN_SUCCESS, "TASK_AUDIT_TOKEN");
	errno = 0;
	check(fails_with(csops_audittoken(0, CS_OPS_STATUS, &flags, sizeof(flags), NULL), EINVAL), "csops_audittoken without a token: EINVAL");
	check(csops_audittoken(getpid(), CS_OPS_STATUS, &flags, sizeof(flags), &token) == 0, "csops_audittoken with the process's token");
	check(csops_audittoken(0, CS_OPS_STATUS, &flags, sizeof(flags), &token) == 0, "csops_audittoken with PID 0");
	token.val[5]++;
	errno = 0;
	check(fails_with(csops_audittoken(getpid(), CS_OPS_STATUS, &flags, sizeof(flags), &token), ESRCH), "csops_audittoken with another process's token: ESRCH");

	finish();
	return 0;
}
