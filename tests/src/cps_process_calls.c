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

// The private CPSGetCurrentProcess / CPSEnableForegroundOperation /
// CPSSetFrontProcess calls are used by programs that are not Cocoa
// applications to become a foreground application and take focus.  They are
// the lower-level counterparts of GetCurrentProcess, TransformProcessType and
// SetFrontProcess.  A CPS process serial number has the same layout as a
// ProcessSerialNumber (two 32-bit words).  The calls are looked up with
// dlsym so the test also builds where the SDK does not declare them.
//
// Checks: the three symbols resolve through ApplicationServices; when
// CPSGetCurrentProcess succeeds it returns the same serial number as
// GetCurrentProcess; CPSSetFrontProcess and CPSEnableForegroundOperation
// accept that serial number.  Return values are printed for each call
// (macOS without a window server session may return an error, which is not
// treated as a failure), and Darling is expected to return 0 from all three.

#include <ApplicationServices/ApplicationServices.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct { uint32_t hi, lo; } CPSSerNum;
typedef int (*get_fn)(CPSSerNum *);
typedef int (*enable_fn)(CPSSerNum *, uint32_t, uint32_t, uint32_t, uint32_t);
typedef int (*front_fn)(CPSSerNum *);

static int failures;
#define CHECK(c) do { if (!(c)) { printf("FAIL: %s (line %d)\n", #c, __LINE__); failures++; } } while (0)

int main(void)
{
	get_fn get = (get_fn) dlsym(RTLD_DEFAULT, "CPSGetCurrentProcess");
	enable_fn enable = (enable_fn) dlsym(RTLD_DEFAULT, "CPSEnableForegroundOperation");
	front_fn front = (front_fn) dlsym(RTLD_DEFAULT, "CPSSetFrontProcess");
	CHECK(get != NULL);
	CHECK(enable != NULL);
	CHECK(front != NULL);
	if (!get || !enable || !front) {
		printf("FAIL: symbols missing\n");
		return 1;
	}

	CPSSerNum psn = { 0xdeadbeef, 0xdeadbeef };
	int r = get(&psn);
	printf("CPSGetCurrentProcess = %d (%u,%u)\n", r, psn.hi, psn.lo);

	ProcessSerialNumber ref;
	CHECK(GetCurrentProcess(&ref) == noErr);
	if (r == 0) {
		CHECK(psn.hi == ref.highLongOfPSN);
		CHECK(psn.lo == ref.lowLongOfPSN);
	} else {
		// no usable serial number: fall back to the Process Manager value
		psn.hi = ref.highLongOfPSN;
		psn.lo = ref.lowLongOfPSN;
	}

	// the four trailing words are opaque flags; zero is accepted
	r = enable(&psn, 0, 0, 0, 0);
	printf("CPSEnableForegroundOperation = %d\n", r);
	r = front(&psn);
	printf("CPSSetFrontProcess = %d\n", r);

	printf(failures ? "FAIL\n" : "PASS\n");
	return failures != 0;
}
