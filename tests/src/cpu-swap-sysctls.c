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

// hw.packages, vm.swapusage and machdep.cpu.feature_bits, with the types
// and errors macOS 27 reports (arm64, and x86_64 under Rosetta).

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
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
	int packages = 0;
	size_t len = 0;
	CHECK(sysctlbyname("hw.packages", NULL, &len, NULL, 0) == 0 && len == sizeof(int));
	len = sizeof(packages);
	CHECK(sysctlbyname("hw.packages", &packages, &len, NULL, 0) == 0 && packages >= 1);

	struct xsw_usage usage;
	len = 0;
	CHECK(sysctlbyname("vm.swapusage", NULL, &len, NULL, 0) == 0 && len == sizeof(usage));
	len = sizeof(usage);
	CHECK(sysctlbyname("vm.swapusage", &usage, &len, NULL, 0) == 0);
	CHECK(usage.xsu_used + usage.xsu_avail == usage.xsu_total);
	CHECK(usage.xsu_pagesize >= 4096);

	// A short buffer is ENOMEM; these are read-only.
	char small[1];
	len = sizeof(small);
	errno = 0;
	CHECK(sysctlbyname("hw.packages", small, &len, NULL, 0) == -1 && errno == ENOMEM);
	len = sizeof(packages);
	errno = 0;
	CHECK(sysctlbyname("hw.packages", &packages, &len, &packages, sizeof(packages)) == -1 && errno == EPERM);

	// There is no machdep.cpu.signature on macOS.
	uint32_t sig;
	len = sizeof(sig);
	errno = 0;
	CHECK(sysctlbyname("machdep.cpu.signature", &sig, &len, NULL, 0) == -1 && errno == ENOENT);

#if defined(__x86_64__) || defined(__i386__)
	uint64_t bits = 0;
	len = 0;
	CHECK(sysctlbyname("machdep.cpu.feature_bits", NULL, &len, NULL, 0) == 0 && len == sizeof(bits));
	len = sizeof(bits);
	CHECK(sysctlbyname("machdep.cpu.feature_bits", &bits, &len, NULL, 0) == 0);
	// EDX bit 0 (FPU) and bit 25 (SSE) are set on every x86_64 CPU.
	CHECK((bits & 1) && (bits & (1u << 25)));
#endif

	if (failures)
		return 1;
	puts("PASS");
	return 0;
}
