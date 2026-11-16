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

// The pasteboard service of the container (Mach service com.apple.pasteboard.1). It keeps the named pasteboards, their change
// counts and data for the user's processes, so that windowless programs (pbcopy, pbpaste) and the find pasteboard
// work without a display. The store and the protocol live in CoreFoundation's CFPasteboard; this program only
// provides the process. launchd starts it on the first lookup of the service; the container has only the system domain, so it is a
// LaunchDaemon, and every process of the container may use it.

#include <CoreFoundation/CFPasteboard_Private.h>

int main(void) {
	__CFPasteboardRunService();
	return 0;
}
