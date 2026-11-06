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

// Secure event input is a nesting count: every EnableSecureEventInput()
// needs a matching DisableSecureEventInput(). On macOS the window server
// combines the counts of all processes; Darling has no such server, so
// IsSecureEventInputEnabled() only reflects the calling process, and
// nothing restricts the delivery of keyboard events yet.
static int g_secureEventInputCount;

OSStatus EnableSecureEventInput(void)
{
	__atomic_add_fetch(&g_secureEventInputCount, 1, __ATOMIC_SEQ_CST);
	return noErr;
}

OSStatus DisableSecureEventInput(void)
{
	int count = __atomic_load_n(&g_secureEventInputCount, __ATOMIC_SEQ_CST);

	// An unbalanced call must not leave the count negative, which would
	// swallow the next EnableSecureEventInput().
	while (count > 0)
	{
		if (__atomic_compare_exchange_n(&g_secureEventInputCount, &count, count - 1,
				false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST))
			break;
	}
	return noErr;
}

Boolean IsSecureEventInputEnabled(void)
{
	return __atomic_load_n(&g_secureEventInputCount, __ATOMIC_SEQ_CST) > 0;
}
