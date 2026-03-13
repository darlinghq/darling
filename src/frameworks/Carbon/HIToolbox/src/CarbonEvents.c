/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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

#include <HIToolbox/CarbonEvents.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

OSStatus RegisterEventHotKey(UInt32 inHotKeyCode, UInt32 inHotKeyModifiers, EventHotKeyID inHotKeyID, EventTargetRef inTarget, OptionBits inOptions, EventHotKeyRef *outRef)
{
    if (verbose) puts("STUB: RegisterEventHotKey called");

    return 0;
}

OSStatus UnregisterEventHotKey(EventHotKeyRef inHotKey)
{
    if (verbose) puts("STUB: UnregisterEventHotKey called");

    return 0;
}
