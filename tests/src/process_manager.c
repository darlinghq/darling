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

// Process Manager calls (GetCurrentProcess, GetProcessPID, GetProcessForPID,
// SameProcess, GetNextProcess, CopyProcessName, GetProcessInformation,
// TransformProcessType, SetFrontProcess[WithOptions], GetFrontProcess).
//
// The expected results (error codes for unknown or invalid processes and
// types, the rules for transforming between process types) are the documented
// Process Manager contract, confirmed by observing macOS 10.14.4 (i386,
// x86_64) and 26.7.3 (x86_64) from a command-line process. A command-line
// process starts out as a background-only application, which the transform
// sequence below relies on.
//
// Arguments: "skip-info" skips GetProcessInformation (it crashes in 64-bit
// processes on macOS 10.14.4 and 26.7.3 with this struct; i386 works); "darling" additionally requires that SetFrontProcess on
// this process succeeds and makes it the front process, which needs no window
// server in Darling. On macOS over ssh (no window server session) the
// SetFrontProcess result is an error that is only printed.

#include <ApplicationServices/ApplicationServices.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int failures;
#define CHECK(c) do { if (!(c)) { printf("FAIL: %s (line %d)\n", #c, __LINE__); failures++; } } while (0)

int main(int argc, char **argv)
{
	int skipInfo = 0, inDarling = 0;
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "skip-info")) skipInfo = 1;
		if (!strcmp(argv[i], "darling")) inDarling = 1;
	}
	setvbuf(stdout, NULL, _IONBF, 0);

	ProcessSerialNumber me = { 9, 9 }, cur = { 0, kCurrentProcess }, none = { 0, kNoProcess };
	ProcessSerialNumber bad = { 0, 0x7ffffff0 }, psn;
	pid_t pid;
	Boolean same;

	CHECK(GetCurrentProcess(&me) == noErr);
	CHECK(me.highLongOfPSN == 0 || me.lowLongOfPSN != 0);

	pid = -1;
	CHECK(GetProcessPID(&me, &pid) == noErr && pid == getpid());
	pid = -1;
	CHECK(GetProcessPID(&cur, &pid) == noErr && pid == getpid());
	pid = -1;
	CHECK(GetProcessPID(&bad, &pid) == procNotFound && pid == -1);
	CHECK(GetProcessPID(&none, &pid) == procNotFound);

	psn.highLongOfPSN = psn.lowLongOfPSN = 7;
	CHECK(GetProcessForPID(getpid(), &psn) == noErr);
	CHECK(psn.highLongOfPSN == me.highLongOfPSN && psn.lowLongOfPSN == me.lowLongOfPSN);
	psn.highLongOfPSN = psn.lowLongOfPSN = 7;
	CHECK(GetProcessForPID(0x7ffffff0, &psn) == procNotFound);
	CHECK(psn.highLongOfPSN == 7 && psn.lowLongOfPSN == 7);

	same = 2;
	CHECK(SameProcess(&me, &me, &same) == noErr && same);
	same = 2;
	CHECK(SameProcess(&me, &cur, &same) == noErr && same);
	same = 2;
	CHECK(SameProcess(&me, &bad, &same) == noErr && !same);

	int count = 0, result;
	ProcessSerialNumber next = none;
	while ((result = GetNextProcess(&next)) == noErr && count < 100000)
		count++;
	CHECK(count > 0);
	CHECK(result == procNotFound);

	CFStringRef name = NULL;
	char buf[256] = "";
	const char *base = strrchr(argv[0], '/');
	base = base ? base + 1 : argv[0];
	CHECK(CopyProcessName(&me, &name) == noErr && name != NULL);
	if (name) {
		CFStringGetCString(name, buf, sizeof buf, kCFStringEncodingUTF8);
		CFRelease(name);
	}
	// the kernel truncates process names to 15 characters on some systems
	CHECK(strncmp(buf, base, 15) == 0 && buf[0] != 0);
	name = NULL;
	CHECK(CopyProcessName(&bad, &name) == procNotFound);

	if (!skipInfo) {
		struct ProcessInfoRec info;
		Str255 pname;
		FSRef appRef;
		memset(&info, 0, sizeof info);
		info.processInfoLength = sizeof info;
		info.processName = pname;
		info.processAppRef = &appRef;
		CHECK(GetProcessInformation(&me, &info) == noErr);
		CHECK(info.processNumber.highLongOfPSN == me.highLongOfPSN && info.processNumber.lowLongOfPSN == me.lowLongOfPSN);
		CHECK(pname[0] > 0);
		info.processInfoLength = sizeof info;
		CHECK(GetProcessInformation(&bad, &info) == procNotFound);
	}

	// transforms: invalid type, unknown process
	CHECK(TransformProcessType(&me, 0) == paramErr);
	CHECK(TransformProcessType(&me, 99) == paramErr);
	CHECK(TransformProcessType(&bad, kProcessTransformToForegroundApplication) == procNotFound);
	// QUIRK: transforming to the type the process already has is paramErr
	// (undocumented; observed on macOS 10.14.4 and 26.7.3).
	CHECK(TransformProcessType(&me, kProcessTransformToBackgroundApplication) == paramErr);
	CHECK(TransformProcessType(&me, kProcessTransformToUIElementApplication) == noErr);
	CHECK(TransformProcessType(&me, kProcessTransformToForegroundApplication) == noErr);
	CHECK(TransformProcessType(&me, kProcessTransformToForegroundApplication) == paramErr);
	CHECK(TransformProcessType(&me, kProcessTransformToBackgroundApplication) == noErr);

	// front process
	CHECK(SetFrontProcess(&bad) == procNotFound);
	CHECK(SetFrontProcessWithOptions(&me, 0x80) == paramErr);
	CHECK(SetFrontProcess(&(ProcessSerialNumber){ 0, kSystemProcess }) == noErr);
	OSErr r = SetFrontProcess(&me);
	printf("SetFrontProcess(self) = %d\n", (int) r);
	r = SetFrontProcessWithOptions(&me, kSetFrontProcessFrontWindowOnly);
	printf("SetFrontProcessWithOptions(self, FrontWindowOnly) = %d\n", (int) r);
	ProcessSerialNumber front = { 5, 5 };
	CHECK(GetFrontProcess(&front) == noErr);
	if (inDarling) {
		CHECK(SetFrontProcess(&me) == noErr);
		CHECK(SetFrontProcessWithOptions(&me, kSetFrontProcessFrontWindowOnly) == noErr);
		CHECK(GetFrontProcess(&front) == noErr);
		same = 0;
		CHECK(SameProcess(&front, &me, &same) == noErr && same);
	}

	printf(failures ? "FAIL\n" : "PASS\n");
	return failures != 0;
}
