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

#include <OSServices/Power.h>
#include <CarbonCore/MacErrors.h>

// QUIRK: UpdateSystemActivity succeeds for every documented activity type (0 to 4) and fails with
// paramErr for any other value, whatever the system's power features. Darling has no power
// management that idle timers could feed, so a valid call only validates the activity type.
// Evidence: black-box observation on macOS 10.14.4, i386 and x86_64 (tier 2),
// vd-upstream/mac-probes/v665-sm.c.
// Test: tests/src/script_manager_variables.c
OSErr UpdateSystemActivity(UInt8 activity)
{
	return (activity <= IdleActivity) ? noErr : paramErr;
}
