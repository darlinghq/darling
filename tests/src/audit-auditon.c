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

// auditon(2) and audit(2) in Darling. Darling does not restrict these to
// root (on macOS 27 an unprivileged caller gets EPERM from both), so this
// test is Darling-only: it checks the session state auditon keeps, that
// audit() appends to /var/audit/trail, and that the trail descriptor does
// not get in the program's way.

#include <bsm/audit.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

static int failures;

#define CHECK(cond) do { \
	if (!(cond)) { \
		printf("FAIL line %d: %s (errno %d)\n", __LINE__, #cond, errno); \
		failures++; \
	} \
} while (0)

static off_t trail_size(void)
{
	struct stat st;
	if (stat("/var/audit/trail", &st) != 0)
		return 0;
	return st.st_size;
}

int main(void)
{
	int cond = -1;
	CHECK(auditon(A_GETCOND, &cond, sizeof(cond)) == 0);
	CHECK(cond == AUC_DISABLED);
	int newcond = AUC_NOAUDIT;
	CHECK(auditon(A_SETCOND, &newcond, sizeof(newcond)) == 0);
	CHECK(auditon(A_GETCOND, &cond, sizeof(cond)) == 0 && cond == AUC_NOAUDIT);

	// A_SETPMASK changes the mask getaudit_addr reports.
	auditpinfo_t api;
	memset(&api, 0, sizeof(api));
	api.ap_pid = getpid();
	api.ap_mask.am_success = 0x1234;
	api.ap_mask.am_failure = 0x5678;
	CHECK(auditon(A_SETPMASK, &api, sizeof(api)) == 0);
	auditinfo_addr_t aia;
	CHECK(getaudit_addr(&aia, sizeof(aia)) == 0);
	CHECK(aia.ai_mask.am_success == 0x1234 && aia.ai_mask.am_failure == 0x5678);
	errno = 0;
	CHECK(auditon(A_SETPMASK, &api, sizeof(api) - 1) == -1 && errno == EINVAL);

	int64_t policy = 0x10000000;
	errno = 0;
	CHECK(auditon(A_SETPOLICY, &policy, sizeof(policy)) == -1 && errno == EINVAL);

	// audit() appends the record to the trail.
	char record[] = "darling audit test record";
	off_t before = trail_size();
	CHECK(audit(record, sizeof(record)) == 0);
	off_t after = trail_size();
	CHECK(after == before + (off_t) sizeof(record));
	errno = 0;
	CHECK(audit(NULL, 0) == -1 && errno == EINVAL);

	// Close every descriptor above stderr, open a file, and audit again:
	// the record must not land in that file.
	for (int fd = 3; fd < 256; fd++)
		close(fd);
	char path[] = "/tmp/audit-testXXXXXX";
	int fd = mkstemp(path);
	CHECK(fd >= 0);
	CHECK(audit(record, sizeof(record)) == 0);
	struct stat st;
	CHECK(fstat(fd, &st) == 0 && st.st_size == 0);
	CHECK(trail_size() == after + (off_t) sizeof(record));
	close(fd);
	unlink(path);

	if (failures)
		return 1;
	puts("PASS");
	return 0;
}
