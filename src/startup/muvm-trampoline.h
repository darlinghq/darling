/*
This file is part of Darling.

Copyright (C) 2026 Lubos Dolezel

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _MUVM_TRAMPOLINE_H
#define _MUVM_TRAMPOLINE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// FEXCore assumes 4 KB guest pages, so an x86_64 Mach-O cannot be loaded by mldr
// on a host with larger pages. Such a run is handed to muvm, whose guest has
// 4 KB pages; inside the guest this whole decision is false and nothing loops.

enum darling_target_slice
{
	DARLING_TARGET_SLICE_NONE,
	DARLING_TARGET_SLICE_ARM64,
	DARLING_TARGET_SLICE_ARM32,
	DARLING_TARGET_SLICE_X86_64,
};

// The slice mldr would run for this file, mirroring mldr_consider_fat_arch
// (src/startup/mldr/loader.h) on aarch64: an entry of bprefs that matches wins
// first, then arm64, then x86_64 over a 32-bit arm slice. bprefs may be NULL.
// Anything that is not a readable regular Mach-O file is DARLING_TARGET_SLICE_NONE.
enum darling_target_slice darling_target_slice_arch(const char* path, const uint32_t bprefs[4]);

// mldr reads its architecture preference from __mldr_bprefs; so must the
// decision above, or a fat binary pinned to x86_64 would skip muvm and abort.
void darling_read_mldr_bprefs(uint32_t bprefs[4]);

// True only when the run cannot be loaded natively and muvm is the way in.
bool darling_muvm_needed(long page_size, enum darling_target_slice slice);

// muvm moves the guest's stdio onto its virtio console, and only routes that
// console to a terminal; a redirected descriptor would silently lose output.
bool darling_muvm_stdio_ok(int stdin_fd, int stdout_fd, int stderr_fd);

#endif // _MUVM_TRAMPOLINE_H
