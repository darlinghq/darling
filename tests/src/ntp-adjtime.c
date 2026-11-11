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

// ntp_adjtime() reading the clock state and the mode bits it accepts.
// Checked on macOS 27 (arm64 and x86_64).

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/timex.h>
#include <unistd.h>

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

static int failures;

#define CHECK(cond) do { \
	if (!(cond)) { \
		printf("FAIL line %d: %s (errno %d)\n", __LINE__, #cond, errno); \
		failures++; \
	} \
} while (0)

int main(void)
{
	struct timex tx;
	memset(&tx, 0, sizeof(tx));
	errno = 0;
	int state = ntp_adjtime(&tx);
	CHECK(state >= TIME_OK && state <= TIME_ERROR);
	CHECK(tx.modes == 0);
	CHECK(tx.tolerance > 0);
	CHECK(tx.precision > 0);

	errno = 0;
	CHECK(ntp_adjtime(NULL) == -1 && errno == EFAULT);

	if (geteuid() != 0)
	{
		memset(&tx, 0, sizeof(tx));
		tx.modes = MOD_FREQUENCY;
		errno = 0;
		CHECK(ntp_adjtime(&tx) == -1 && errno == EPERM);
		CHECK(tx.modes == MOD_FREQUENCY);
	}

	if (failures)
		return 1;
	puts("PASS");
	return 0;
}
