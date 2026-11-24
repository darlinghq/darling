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

// diskarbitrationd's disks, made from the mount table. Every function is called on the daemon's queue.
//
// The disks are the root volume ("disk0", its volume "disk0s1") and every volume whose source (f_mntfromname) is a
// path outside /dev, i.e. a file: a disk image (darling-dmg mounts an image under its path). Such a volume is the
// leaf "diskNs1" of a whole disk "diskN", N being the lowest unit no other disk has. Other mounts have no disk.
#ifndef DA_MODEL_H
#define DA_MODEL_H

#include <CoreFoundation/CoreFoundation.h>
#include <stdbool.h>
#include <sys/param.h>
#include <sys/types.h>

// A whole disk and the leaf that holds its volume.
typedef struct DAMedia
{
	int unit;
	unsigned long id;             // never reused, unlike the unit
	bool root;                    // the root volume's disk
	bool mounted;                 // the leaf's volume is mounted on mountPoint
	bool seen;                    // found by the current rescan
	char mountPoint[MAXPATHLEN];
	char source[MAXPATHLEN];      // f_mntfromname
	uid_t owner;                  // f_owner of the volume
	CFMutableDictionaryRef whole; // the descriptions, as DAProtocol.h sends them
	CFMutableDictionaryRef leaf;
	struct DAMedia* next;
} DAMedia;

// What a rescan reports; any callback may be NULL. Names are BSD names.
typedef struct DAModelEvents
{
	void* context;
	void (*appeared)(void* context, const char* name, CFDictionaryRef description);
	void (*disappeared)(void* context, const char* name, CFDictionaryRef description);
	void (*changed)(void* context, const char* name, CFArrayRef keys, CFDictionaryRef description);
} DAModelEvents;

// Brings the disks in line with the mount table. A disk whose volume went away first changes (its leaf loses
// DAVolumePath), then disappears (whole disk first), and its mount point in /Volumes is removed; new volumes appear
// (leaf first, as on macOS); changed descriptions are reported with the keys that changed. A disk whose volume DiskArbitration unmounted (mounted == false) is left
// alone: it stays until it is ejected.
void DAModelRescan(const DAModelEvents* events);

// The leaf's volume was unmounted through DiskArbitration: it loses DAVolumePath (reported as changed), and its
// mount point in /Volumes is removed. The disk stays until it is ejected.
void DAModelUnmounted(DAMedia* media, const DAModelEvents* events);

// The disk is ejected: it disappears (whole disk first) and is freed.
void DAModelRemove(DAMedia* media, const DAModelEvents* events);

// The media a BSD name ("diskN" or "diskNs1", optionally after "/dev/") names, or NULL; *whole tells which.
DAMedia* DAModelFindName(const char* name, bool* whole);

// The media whose volume is mounted exactly on path (no trailing slash, except for "/"), or NULL.
DAMedia* DAModelFindMountPoint(const char* path);

// The first media; walk on with ->next.
DAMedia* DAModelFirst(void);

// The BSD name of the whole disk or of the leaf.
void DAModelName(const DAMedia* media, bool whole, char* name, size_t size);

// Removes a mount point directly in /Volumes if it is an empty directory with nothing mounted on it, while holding
// the lock the attach library takes on /Volumes while it names and mounts a new volume.
void DAModelRemoveMountPoint(const char* mountPoint);

#endif
