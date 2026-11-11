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

// kern.secure_kernel exists as a read-only integer that is 0, and a
// successful opendir() does not leave an error in errno. Observed on
// macOS 27 (arm64 and x86_64).

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/sysctl.h>

static int failures;

#define CHECK(cond) do { \
	if (!(cond)) { \
		printf("FAIL line %d: %s (errno %d)\n", __LINE__, #cond, errno); \
		failures++; \
	} \
} while (0)

int main(void)
{
	int value = -1;
	size_t len = sizeof(value);
	CHECK(sysctlbyname("kern.secure_kernel", &value, &len, NULL, 0) == 0);
	CHECK(len == sizeof(int) && value == 0);

	len = 0;
	CHECK(sysctlbyname("kern.secure_kernel", NULL, &len, NULL, 0) == 0 && len == sizeof(int));

	int mib[4];
	size_t miblen = 4;
	CHECK(sysctlnametomib("kern.secure_kernel", mib, &miblen) == 0 && miblen == 2);
	value = -1;
	len = sizeof(value);
	CHECK(sysctl(mib, (u_int) miblen, &value, &len, NULL, 0) == 0 && value == 0);

	int one = 1;
	errno = 0;
	CHECK(sysctlbyname("kern.secure_kernel", NULL, NULL, &one, sizeof(one)) == -1 && errno == EPERM);

	errno = 0;
	DIR* dir = opendir("/");
	CHECK(dir != NULL);
	CHECK(errno == 0);
	if (dir)
	{
		CHECK(readdir(dir) != NULL);
		closedir(dir);
	}

	if (failures)
		return 1;
	puts("PASS");
	return 0;
}
