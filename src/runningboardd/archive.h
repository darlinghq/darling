/**
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

#ifndef RUNNINGBOARDD_ARCHIVE_H
#define RUNNINGBOARDD_ARCHIVE_H

#include <xpc/xpc.h>
#include <xpc/connection.h>
#include <mach/message.h>

// The reply dictionary key that marks an error reply, and the key of an archive that goes with a value or an error.
#define ERROR_KEY "__RBSXPCError__"
#define ARCHIVE_KEY "bsx_archive"
#define INDEX_KEY "bsx_index"

// Adds to `dictionary` the archive of the audit token (under ARCHIVE_KEY) and returns the dictionary
// {INDEX_KEY: <name of the archived object>} that refers to it.
xpc_object_t archive_audit_token_create(xpc_object_t dictionary, audit_token_t token);

// Fills `reply` as an error reply: ERROR_KEY with an index, and ARCHIVE_KEY holding an archived NSError of the
// given domain, code and failure reason.
void archive_error_reply(xpc_object_t reply, const char* domain, int code, const char* reason);

#endif
