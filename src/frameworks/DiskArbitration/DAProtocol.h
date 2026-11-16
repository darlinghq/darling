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

// The messages between the DiskArbitration library and diskarbitrationd: XPC dictionaries whose DA_KEY_OP names
// the request. This is Darling's own protocol; only the service name is the one macOS uses.
#ifndef DA_PROTOCOL_H
#define DA_PROTOCOL_H

#define DA_SERVICE_NAME "com.apple.DiskArbitration.diskarbitrationd"

#define DA_KEY_OP          "op"
#define DA_KEY_NAME        "name"        // a BSD name, "diskN" or "diskNsM"
#define DA_KEY_PATH        "path"
#define DA_KEY_STATUS      "status"      // a DAReturn, int64
#define DA_KEY_DESCRIPTION "description" // data: a binary property list (see below)

// {name} -> {description} | {status: kDAReturnNotFound}
#define DA_OP_DESCRIBE "describe"
// {path: an exact mount point, no trailing slash} -> {name} | {status: kDAReturnNotFound}
#define DA_OP_LOOKUP_PATH "lookup-path"

// A description travels as a binary property list of the disk description dictionary, with two differences
// because a property list holds neither URLs nor UUIDs: kDADiskDescriptionVolumePathKey is the mount point as a
// path string, and the UUID keys are UUID strings. The library turns them into a CFURL and CFUUIDs.

#endif
