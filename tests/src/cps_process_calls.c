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
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

typedef struct { uint32_t hi, lo; } CPSSerNum;
typedef int (*get_fn)(CPSSerNum *);
typedef int (*enable_fn)(CPSSerNum *, uint32_t, uint32_t, uint32_t, uint32_t);
typedef int (*front_fn)(CPSSerNum *);

static int failures;
#define CHECK(c) do { if (!(c)) { printf("FAIL: %s (line %d)\n", #c, __LINE__); failures++; } } while (0)

// A fresh process (no policy published yet) that transforms itself to a foreground application publishes the
// regular policy, since TransformProcessType() is built on CPSEnableForegroundOperation(). Exit status 0 when so.
static int transformChild(void)
{
	typedef int (*policy_fn)(pid_t, int *);
	policy_fn policyOf = (policy_fn) dlsym(RTLD_DEFAULT, "_CGSGetProcessActivationPolicy");
	ProcessSerialNumber me;
	int policy = -1;

	if (!policyOf || GetCurrentProcess(&me) != noErr)
		return 2;
	if (policyOf(getpid(), &policy) == 0)
		return 3; // nothing was published yet
	if (TransformProcessType(&me, kProcessTransformToForegroundApplication) != noErr)
		return 4;
	return policyOf(getpid(), &policy) == 0 && policy == 0 ? 0 : 5;
}

int main(int argc, char **argv)
{
	if (argc > 1 && !strcmp(argv[1], "transform-child"))
		return transformChild();
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

	// A NULL serial number fails all three calls with 1003 (macOS 10.14.4 and 26.7.3, no window server
	// session).
	CHECK(get(NULL) == 1003);
	CHECK(enable(NULL, 0, 0, 0, 0) == 1003);
	CHECK(front(NULL) == 1003);

	// The CPS calls belong to the window system layer: they resolve through CoreGraphics' own image (on macOS
	// CoreGraphics re-exports them), not only through whatever else is loaded.
	void *cg = dlopen("/System/Library/Frameworks/CoreGraphics.framework/CoreGraphics", RTLD_LAZY);
	CHECK(cg != NULL);
	if (cg) {
		CHECK(dlsym(cg, "CPSGetCurrentProcess") == (void *) get);
		CHECK(dlsym(cg, "CPSEnableForegroundOperation") == (void *) enable);
		CHECK(dlsym(cg, "CPSSetFrontProcess") == (void *) front);
	}

	// Darling: the activation policy the process publishes through CoreGraphics follows CPSEnableForegroundOperation
	// and the Process Manager's transformation to a foreground application built on it (regular).
	typedef int (*policy_fn)(pid_t, int *);
	policy_fn policyOf = (policy_fn) dlsym(RTLD_DEFAULT, "_CGSGetProcessActivationPolicy");
	if (policyOf) {
		int policy = -1;
		CPSSerNum self = { 0, (uint32_t) getpid() };
		CHECK(enable(&self, 0, 0, 0, 0) == 0);
		CHECK(policyOf(getpid(), &policy) == 0 && policy == 0);
		CHECK(front(&self) == 0);
		CHECK(SetFrontProcess(&ref) == noErr);
		pid_t child = fork();
		if (child == 0) {
			execl(argv[0], argv[0], "transform-child", (char *) NULL);
			_exit(127);
		}
		int status = -1;
		CHECK(child > 0 && waitpid(child, &status, 0) == child && WIFEXITED(status) && WEXITSTATUS(status) == 0);
		// The system process is accepted and left alone, a serial number of no process is not.
		CPSSerNum system = { 0, 1 }, none = { 0, 0 }, gone = { 1, 1 };
		CHECK(front(&system) == 0);
		CHECK(front(&none) != 0);
		CHECK(front(&gone) != 0);
		// Only the calling process can publish its own policy.
		CPSSerNum init = { 0, 1 + (uint32_t) getpid() };
		CHECK(enable(&init, 0, 0, 0, 0) != 0);
	}

	printf(failures ? "FAIL\n" : "PASS\n");
	return failures != 0;
}
