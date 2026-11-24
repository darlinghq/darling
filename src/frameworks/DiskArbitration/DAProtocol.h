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

// {} -> {disks: an array of {name, description}, each leaf before its whole disk}. From then on the daemon sends the
// connection an event message for every change, and again after a new hello on a new connection.
#define DA_OP_HELLO "hello"
#define DA_KEY_DISKS "disks"

// Event messages from the daemon: {event, name, description, and for "changed" also keys}. A disk appears leaf
// first, then its whole disk; it disappears whole disk first, then the leaf.
#define DA_KEY_EVENT "event"
#define DA_KEY_KEYS  "keys" // an array of description key strings
#define DA_EVENT_APPEARED    "appeared"
#define DA_EVENT_DISAPPEARED "disappeared"
#define DA_EVENT_CHANGED     "changed"

// {name, options} -> {status, string (optional)}: kDADiskUnmountOption* for unmount, kDADiskEjectOption* for eject.
// The reply comes once the request is done, after the approval round.
#define DA_OP_UNMOUNT "unmount"
#define DA_OP_EJECT   "eject"
#define DA_KEY_OPTIONS "options" // uint64
#define DA_KEY_STRING  "string"  // a dissenter's status string

// An approval request from the daemon, sent to every connection that said hello: {event: "approve", kind, name,
// description} -> {status (0 approves), string (optional)}.
#define DA_EVENT_APPROVE "approve"
#define DA_KEY_KIND      "kind" // DA_OP_UNMOUNT or DA_OP_EJECT

// A description travels as a binary property list of the disk description dictionary, with two differences
// because a property list holds neither URLs nor UUIDs: kDADiskDescriptionVolumePathKey is the mount point as a
// path string, and the UUID keys are UUID strings. The library turns them into a CFURL and CFUUIDs.

#endif
