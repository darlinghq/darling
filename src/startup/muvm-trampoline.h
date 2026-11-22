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

// Stock muvm forwards only the host's X11 display. A muvm that serves the host's
// Wayland compositor in the guest (it starts wl-cross-domain-proxy under the
// host's WAYLAND_DISPLAY name) makes AppKit's Wayland backend usable, and AppKit
// prefers that backend whenever the guest has a WAYLAND_DISPLAY and falls back to
// X11 otherwise, so the host's DISPLAY is left to muvm as it always was.

// True when muvm would serve this WAYLAND_DISPLAY in the guest: a plain socket
// name, not empty, not a path, not "." or "..". NULL (unset) is not.
bool darling_wayland_display_is_plain(const char* wayland_display);

// True when the muvm found through path_env (a PATH value, may be NULL) can
// forward Wayland: wl-cross-domain-proxy is in that path, and so is a
// muvm-guest that names the proxy (a stock muvm-guest never does). muvm finds
// both through PATH too. Reads files, runs and starts nothing.
bool darling_muvm_wayland_capable(const char* path_env);

// Whether the guest gets the host's Wayland session: a plain WAYLAND_DISPLAY
// (NULL when unset) and a muvm that probe says is capable. The probe, usually
// darling_muvm_wayland_capable, is not called without a plain name, so an X11
// only host never reads muvm-guest.
bool darling_muvm_wayland_wanted(const char* wayland_display, const char* path_env,
	bool (*probe)(const char* path_env));

// Re-executes this launcher under muvm, with the launcher path made absolute,
// argv[target_index] replaced by target_path, the arguments before the target
// kept as they are, and the variables muvm needs forwarded with -e. Must be
// called with privileges already dropped: execve preserves the saved set-uid.
// Returns only on failure, with errno set.
bool darling_reexec_under_muvm(char* const argv[], size_t target_index, const char* target_path);

#endif // _MUVM_TRAMPOLINE_H
