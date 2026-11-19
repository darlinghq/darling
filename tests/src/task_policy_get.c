/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
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

// task_policy_get()/task_policy_set() on the calling task and on another
// task: the category role, the QoS tiers, the suppression policy and the
// policy state read back as documented, short buffers and unknown flavours
// fail with KERN_INVALID_ARGUMENT, and a role stored with task_policy_set()
// is returned by a later get (but not by a get with get_default set).

#include <mach/mach.h>
#include <mach/task_policy.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

// kernel-private sizes, in integers, of the structures behind these flavours
#define SUPPRESSION_COUNT 16
#define STATE_COUNT 16

static int failures;

static void check(const char *name, int ok)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", name);
	failures += !ok;
}

static kern_return_t get(task_t t, task_policy_flavor_t fl, integer_t *buf, mach_msg_type_number_t cnt, boolean_t def)
{
	mach_msg_type_number_t c = cnt;
	return task_policy_get(t, fl, buf, &c, &def);
}

static void run(const char *who, task_t t)
{
	char n[96];
	integer_t buf[32];
	kern_return_t kr;
	mach_msg_type_number_t c;
	boolean_t def;

	memset(buf, 0x5a, sizeof buf);
	c = TASK_CATEGORY_POLICY_COUNT;
	def = FALSE;
	kr = task_policy_get(t, TASK_CATEGORY_POLICY, buf, &c, &def);
	snprintf(n, sizeof n, "%s: category get succeeds with role unspecified", who);
	check(n, kr == KERN_SUCCESS && c == TASK_CATEGORY_POLICY_COUNT && buf[0] == TASK_UNSPECIFIED);

	def = TRUE;
	memset(buf, 0x5a, sizeof buf);
	c = TASK_CATEGORY_POLICY_COUNT;
	kr = task_policy_get(t, TASK_CATEGORY_POLICY, buf, &c, &def);
	snprintf(n, sizeof n, "%s: category get_default succeeds with role unspecified", who);
	check(n, kr == KERN_SUCCESS && buf[0] == TASK_UNSPECIFIED);

	snprintf(n, sizeof n, "%s: category get with count 0 fails with KERN_INVALID_ARGUMENT", who);
	check(n, get(t, TASK_CATEGORY_POLICY, buf, 0, FALSE) == KERN_INVALID_ARGUMENT);

	for (int d = 0; d < 2; d++) {
		memset(buf, 0x5a, sizeof buf);
		kr = get(t, TASK_BASE_QOS_POLICY, buf, TASK_QOS_POLICY_COUNT, d);
		snprintf(n, sizeof n, "%s: base QoS get_default=%d is unspecified", who, d);
		check(n, kr == KERN_SUCCESS && buf[0] == 0 && buf[1] == 0);

		memset(buf, 0x5a, sizeof buf);
		kr = get(t, TASK_OVERRIDE_QOS_POLICY, buf, TASK_QOS_POLICY_COUNT, d);
		snprintf(n, sizeof n, "%s: override QoS get_default=%d is unspecified", who, d);
		check(n, kr == KERN_SUCCESS && buf[0] == 0 && buf[1] == 0);

		memset(buf, 0x5a, sizeof buf);
		kr = get(t, TASK_SUPPRESSION_POLICY, buf, SUPPRESSION_COUNT, d);
		snprintf(n, sizeof n, "%s: suppression get_default=%d succeeds with nothing active", who, d);
		check(n, kr == KERN_SUCCESS && buf[0] == 0 && buf[1] == 0 && buf[3] == 0 && buf[5] == 0);

		// macOS reserves the policy state for privileged callers
		memset(buf, 0x5a, sizeof buf);
		kr = get(t, TASK_POLICY_STATE, buf, STATE_COUNT, d);
		snprintf(n, sizeof n, "%s: policy state get_default=%d succeeds or is refused as privileged", who, d);
		check(n, kr == KERN_SUCCESS || kr == KERN_PROTECTION_FAILURE);

		kr = get(t, TASK_BASE_QOS_POLICY, buf, 0, d);
		snprintf(n, sizeof n, "%s: QoS get with count 0 fails with KERN_INVALID_ARGUMENT", who);
		check(n, kr == KERN_INVALID_ARGUMENT);

		kr = get(t, TASK_BASE_LATENCY_QOS_POLICY, buf, 1, d);
		snprintf(n, sizeof n, "%s: base latency QoS flavour cannot be read", who);
		check(n, kr == KERN_INVALID_ARGUMENT);

		kr = get(t, 99, buf, 4, d);
		snprintf(n, sizeof n, "%s: unknown flavour fails with KERN_INVALID_ARGUMENT", who);
		check(n, kr == KERN_INVALID_ARGUMENT);
	}
}

int main(void)
{
	run("self", mach_task_self());

	struct task_category_policy p = { TASK_FOREGROUND_APPLICATION };
	integer_t out[8];
	kern_return_t kr;

	kr = task_policy_set(mach_task_self(), TASK_CATEGORY_POLICY, (task_policy_t)&p, TASK_CATEGORY_POLICY_COUNT);
	check("set foreground role succeeds", kr == KERN_SUCCESS);
	kr = get(mach_task_self(), TASK_CATEGORY_POLICY, out, TASK_CATEGORY_POLICY_COUNT, FALSE);
	check("get returns the foreground role", kr == KERN_SUCCESS && out[0] == TASK_FOREGROUND_APPLICATION);
	kr = get(mach_task_self(), TASK_CATEGORY_POLICY, out, TASK_CATEGORY_POLICY_COUNT, TRUE);
	check("get_default still returns the unspecified role", kr == KERN_SUCCESS && out[0] == TASK_UNSPECIFIED);

	p.role = (task_role_t)77;
	kr = task_policy_set(mach_task_self(), TASK_CATEGORY_POLICY, (task_policy_t)&p, TASK_CATEGORY_POLICY_COUNT);
	check("set invalid role fails with KERN_INVALID_ARGUMENT", kr == KERN_INVALID_ARGUMENT);
	p.role = TASK_BACKGROUND_APPLICATION;
	kr = task_policy_set(mach_task_self(), TASK_CATEGORY_POLICY, (task_policy_t)&p, 0);
	check("set with count 0 fails with KERN_INVALID_ARGUMENT", kr == KERN_INVALID_ARGUMENT);
	kr = get(mach_task_self(), TASK_CATEGORY_POLICY, out, TASK_CATEGORY_POLICY_COUNT, FALSE);
	check("failed sets leave the role alone", kr == KERN_SUCCESS && out[0] == TASK_FOREGROUND_APPLICATION);
	kr = task_policy_set(mach_task_self(), TASK_CATEGORY_POLICY, (task_policy_t)&p, TASK_CATEGORY_POLICY_COUNT);
	kr |= get(mach_task_self(), TASK_CATEGORY_POLICY, out, TASK_CATEGORY_POLICY_COUNT, FALSE);
	check("set background role is returned by get", kr == KERN_SUCCESS && out[0] == TASK_BACKGROUND_APPLICATION);

	// another process: its task port is only obtainable by a privileged caller
	int fds[2];
	if (pipe(fds) == 0) {
		pid_t child = fork();
		if (child == 0) {
			char c;
			close(fds[1]);
			read(fds[0], &c, 1);
			_exit(0);
		}
		close(fds[0]);
		task_t ct = MACH_PORT_NULL;
		if (child > 0 && task_for_pid(mach_task_self(), child, &ct) == KERN_SUCCESS) {
			run("child", ct);
			kr = get(ct, TASK_CATEGORY_POLICY, out, TASK_CATEGORY_POLICY_COUNT, FALSE);
			check("child role is independent of the parent role", kr == KERN_SUCCESS && out[0] == TASK_UNSPECIFIED);
		} else {
			printf("SKIP: task_for_pid not permitted for a child\n");
		}
		close(fds[1]);
		if (child > 0) {
			waitpid(child, NULL, 0);
		}
	}

	printf("%s\n", failures ? "FAILED" : "PASSED");
	return failures ? 1 : 0;
}
