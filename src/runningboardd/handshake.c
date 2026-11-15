/**
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Darling developers
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

// The reply to handshakeWithRequest:. Layout (every key, type and nesting) as observed on macOS 26.7.3 x86_64 with a
// client program of our own that sent the request and recorded the reply; the
// observation covers an unbundled command-line caller.
//
// The caller's own pid, uid and audit token are filled in from the XPC connection, Darling-side. The success and fail
// arrays are empty, as observed. Not reproduced: "_data" of the coded handle, a 64-bit value that the observation could
// not explain (it differs per process); a client that needs it is not served.

#include "handshake.h"
#include "archive.h"

#include <libproc.h>
#include <mach/message.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

// declared in xpc/private.h, which the plain C build of this daemon does not include
void xpc_connection_get_audit_token(xpc_connection_t connection, audit_token_t* token);

static xpc_object_t class_only(const char* name) {
	xpc_object_t dictionary = xpc_dictionary_create(NULL, NULL, 0);
	xpc_dictionary_set_string(dictionary, "bsx_class", name);
	return dictionary;
}

static void set_and_release(xpc_object_t dictionary, const char* key, xpc_object_t value) {
	xpc_dictionary_set_value(dictionary, key, value);
	xpc_release(value);
}

xpc_object_t handshake_response_create(xpc_connection_t caller) {
	audit_token_t token;
	memset(&token, 0, sizeof(token));
	xpc_connection_get_audit_token(caller, &token);
	pid_t pid = xpc_connection_get_pid(caller);
	// The audit token's first value is the audit user id. Darling's token carries 0 for a non-root caller, and -1 means
	// "none", so both fall back to the connection's effective uid (on macOS the observed caller had both equal).
	uid_t euid = xpc_connection_get_euid(caller);
	uid_t auid = (uid_t)token.val[0];
	if (auid == (uid_t)-1 || (auid == 0 && euid != 0)) {
		auid = euid;
	}

	// Observed description of an unbundled command-line caller: "anon<name>(uid)", the process name then the audit uid.
	char name[64] = "unknown";
	proc_name(pid, name, sizeof(name));
	char description[160];
	snprintf(description, sizeof(description), "anon<%s>(%u)", name, (unsigned)auid);

	xpc_object_t identity = class_only("RBSOpaqueProcessIdentity");
	xpc_dictionary_set_int64(identity, "_pid", pid);
	xpc_dictionary_set_int64(identity, "_auid", auid);
	xpc_dictionary_set_string(identity, "_description", description);

	xpc_object_t coded = xpc_dictionary_create(NULL, NULL, 0);
	set_and_release(coded, "_identity", identity);
	xpc_dictionary_set_uint64(coded, "_pid", (uint64_t)pid);
	set_and_release(coded, "_bundle", class_only("RBSProcessBundle"));
	// The audit token travels as an archive, see archive.m. Its first word is the audit uid and the pid is the caller's;
	// the other words come from the connection, as far as Darling reports them.
	audit_token_t archived = token;
	archived.val[0] = auid;
	archived.val[1] = euid;
	archived.val[2] = xpc_connection_get_egid(caller);
	archived.val[3] = euid;
	archived.val[4] = xpc_connection_get_egid(caller);
	archived.val[5] = (unsigned)pid;
	set_and_release(coded, "_bsAuditToken", archive_audit_token_create(coded, archived));

	xpc_object_t handle = class_only("RBSProcessHandle");
	set_and_release(handle, "_codedHandle", coded);

	xpc_object_t response = class_only("RBSHandshakeResponse");
	static const char* const empty_arrays[] = { "success-keys", "success-values", "fail-keys", "fail-values" };
	for (size_t i = 0; i < sizeof(empty_arrays) / sizeof(empty_arrays[0]); ++i) {
		set_and_release(response, empty_arrays[i], xpc_array_create(NULL, 0));
	}
	set_and_release(response, "_handle", handle);
	return response;
}
