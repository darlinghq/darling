// Copyright (C) 2026 Lubos Dolezel
//
// The signpost identifier functions of <os/signpost.h>, callable from C and
// C++: os_signpost_id_generate() returns valid identifiers that differ from
// each other, os_signpost_id_make_with_pointer() returns the same valid
// identifier for the same pointer and different ones for different pointers,
// and os_signpost_enabled() answers without failing (Darling does not record
// signposts, so it says false).
//   clang -o /tmp/os-signpost-ids tests/os-signpost-ids.c   (run inside Darling; any arch)
// Exits non-zero on failure.
#include <os/log.h>
#include <os/signpost.h>
#include <stdbool.h>
#include <stdio.h>

static int fail;

static void check(bool ok, const char* what)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
	fail |= !ok;
}

static bool valid(os_signpost_id_t spid)
{
	return spid != OS_SIGNPOST_ID_NULL && spid != OS_SIGNPOST_ID_INVALID && spid != OS_SIGNPOST_ID_EXCLUSIVE;
}

int main(void)
{
	os_log_t log = os_log_create("org.darlinghq.test", "signpost");

	os_signpost_id_t a = os_signpost_id_generate(log);
	os_signpost_id_t b = os_signpost_id_generate(log);
	check(valid(a) && valid(b), "generated identifiers are valid");
	check(a != b, "generated identifiers differ");

	int x, y;
	os_signpost_id_t px = os_signpost_id_make_with_pointer(log, &x);
	check(valid(px), "identifier made from a pointer is valid");
	check(os_signpost_id_make_with_pointer(log, &x) == px, "same pointer gives the same identifier");
	check(os_signpost_id_make_with_pointer(log, &y) != px, "different pointers give different identifiers");

	check(!os_signpost_enabled(log), "signposts are not enabled");
	return fail;
}
