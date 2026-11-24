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

// vm.loadavg (CTL_VM, VM_LOADAVG) returns struct loadavg, which getloadavg(3)
// is built on, and vm.swapusage is the fixed (CTL_VM, VM_SWAPUSAGE) node.

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
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
	int mib[2] = { CTL_VM, VM_LOADAVG };
	struct loadavg la;
	size_t len = 0;

	// A size query reports the struct of this process' ABI.
	CHECK(sysctl(mib, 2, NULL, &len, NULL, 0) == 0 && len == sizeof(la));
	len = sizeof(la);
	CHECK(sysctl(mib, 2, &la, &len, NULL, 0) == 0 && len == sizeof(la));
	CHECK(la.fscale > 0);
	for (int i = 0; i < 3; i++)
		CHECK(la.ldavg[i] < 100000u * (unsigned)la.fscale);

	// By name, the same value.
	struct loadavg byname;
	len = sizeof(byname);
	CHECK(sysctlbyname("vm.loadavg", &byname, &len, NULL, 0) == 0 && len == sizeof(byname));
	CHECK(byname.fscale == la.fscale);

	// getloadavg() converts the same numbers to doubles.
	double avg[3] = { -1, -1, -1 };
	CHECK(getloadavg(avg, 3) == 3);
	for (int i = 0; i < 3; i++)
		CHECK(avg[i] >= 0 && avg[i] < 100000);
	CHECK(getloadavg(avg, 1) == 1);
	CHECK(getloadavg(avg, 0) == 0);

	// The buffer must hold the struct; the node is read-only.
	char small[1];
	len = sizeof(small);
	errno = 0;
	CHECK(sysctl(mib, 2, small, &len, NULL, 0) == -1 && errno == ENOMEM);
	len = sizeof(la);
	errno = 0;
	CHECK(sysctl(mib, 2, &la, &len, &la, sizeof(la)) == -1 && errno == EPERM);

	// vm.swapusage is reachable through its fixed MIB.
	int smib[2] = { CTL_VM, VM_SWAPUSAGE };
	struct xsw_usage xsu;
	len = sizeof(xsu);
	CHECK(sysctl(smib, 2, &xsu, &len, NULL, 0) == 0 && len == sizeof(xsu));
	CHECK(xsu.xsu_used + xsu.xsu_avail == xsu.xsu_total);

	// And the name maps to those two numbers.
	int nmib[CTL_MAXNAME];
	size_t nlen = CTL_MAXNAME;
	CHECK(sysctlnametomib("vm.swapusage", nmib, &nlen) == 0 && nlen == 2 && nmib[0] == CTL_VM && nmib[1] == VM_SWAPUSAGE);
	nlen = CTL_MAXNAME;
	CHECK(sysctlnametomib("vm.loadavg", nmib, &nlen) == 0 && nlen == 2 && nmib[0] == CTL_VM && nmib[1] == VM_LOADAVG);

	if (failures)
		return 1;
	puts("PASS");
	return 0;
}
