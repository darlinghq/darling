/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * mach_error_string() and mach_error_type() decode Mach error codes through
 * the per-system tables. System 0x07 is the virtual memory error system, with
 * subsystem 0 "(vm/kernel)" and subsystem 1 "(vm/reclaim)". Codes outside the
 * known tables decode as "unknown error code" (code out of range) or
 * "(vm/?) unknown subsystem error" (subsystem out of range).
 * The expected strings were observed on macOS 26.7.3 (x86_64).
 *
 * QUIRK-pinning cases: the (vm/kernel) table is backed by the same strings as
 * the (ipc/send) table, so codes 0 and 1 of vm/kernel decode as the first two
 * (ipc/send) strings while mach_error_type() still names the (vm/kernel)
 * subsystem. Evidence: black-box observation on macOS 26.7.3 (tier 2).
 */
#include <stdio.h>
#include <string.h>
#include <mach/mach.h>
#include <mach/mach_error.h>

#define ERR_VM(sub, code) ((mach_error_t)((0x07u << 26) | ((unsigned)(sub) << 14) | (unsigned)(code)))

static int failures;

static void check(mach_error_t err, const char *str, const char *type)
{
	const char *s = mach_error_string(err);
	const char *t = mach_error_type(err);
	int ok = s && t && strcmp(s, str) == 0 && strcmp(t, type) == 0;
	printf("%s: %08x -> \"%s\" / \"%s\"\n", ok ? "PASS" : "FAIL", (unsigned)err, s ? s : "(null)", t ? t : "(null)");
	if (!ok)
		failures++;
}

int main(void)
{
	/* The VM error system. */
	check(ERR_VM(1, 0), "(vm/reclaim) no error", "(vm/reclaim)");
	check(ERR_VM(1, 1), "(vm/reclaim) invalid argument", "(vm/reclaim)");
	check(ERR_VM(1, 2), "(vm/reclaim) operation not supported", "(vm/reclaim)");
	check(ERR_VM(1, 3), "(vm/reclaim) invalid region size", "(vm/reclaim)");
	check(ERR_VM(1, 4), "(vm/reclaim) invalid ring capacity", "(vm/reclaim)");
	check(ERR_VM(1, 5), "(vm/reclaim) invalid reclaim ID", "(vm/reclaim)");
	check(ERR_VM(1, 6), "(vm/reclaim) ring already instantiated", "(vm/reclaim)");
	check(ERR_VM(1, 7), "(vm/reclaim) invalid ring", "(vm/reclaim)");
	/* Last valid code + 1, unknown subsystems. */
	check(ERR_VM(1, 8), "unknown error code", "(vm/reclaim)");
	check(ERR_VM(0, 2), "unknown error code", "(vm/kernel)");
	check(ERR_VM(2, 0), "(vm/?) unknown subsystem error", "(?/?)");
	check(ERR_VM(0x3ff, 0), "(vm/?) unknown subsystem error", "(?/?)");
	/* QUIRK: vm/kernel strings come from the ipc/send table. */
	check(ERR_VM(0, 0), "(ipc/send) no error", "(vm/kernel)");
	check(ERR_VM(0, 1), "(ipc/send) send in progress", "(vm/kernel)");

	/* Systems that were already registered keep decoding. */
	check(KERN_FAILURE, "(os/kern) failure", "(os/kern)");
	check(KERN_INVALID_ADDRESS, "(os/kern) invalid address", "(os/kern)");
	check(0x10000003u, "(ipc/send) invalid destination port", "(ipc/send)");
	check(0x10004003u, "(ipc/rcv) timed out", "(ipc/rcv)");
	/* An unregistered system and an unknown os code. */
	check(0x00004001, "unknown error code", "(os/?)");

	printf("%s\n", failures ? "FAILED" : "ALL PASSED");
	return failures ? 1 : 0;
}
