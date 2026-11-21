/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

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

#include <QD/QD.h>
#include <ATS/ATS.h>
#include <stdlib.h>
#include <stdio.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CGDirectDisplay.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

// Number of HideCursor calls not yet balanced by ShowCursor. The display-level hide count it
// feeds is not queryable, so this is what InitCursor uses to bring the cursor back.
static int gCursorHideLevel;

// Cursor visibility is a counter: every HideCursor is balanced by a ShowCursor, as for the
// display-level calls these are built on.
void HideCursor(void)
{
    __atomic_add_fetch(&gCursorHideLevel, 1, __ATOMIC_SEQ_CST);
    CGDisplayHideCursor(CGMainDisplayID());
}

void ShowCursor(void)
{
    int level = __atomic_load_n(&gCursorHideLevel, __ATOMIC_SEQ_CST);
    while (level > 0 && !__atomic_compare_exchange_n(&gCursorHideLevel, &level, level - 1, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST))
        ;
    CGDisplayShowCursor(CGMainDisplayID());
}

// Resets the hide level to zero, so the cursor is visible again however many HideCursor calls
// were outstanding. Only the visibility is reset: no layer below AppKit can set the cursor shape.
void InitCursor(void)
{
    int level = __atomic_exchange_n(&gCursorHideLevel, 0, __ATOMIC_SEQ_CST);
    for (; level > 0; level--)
        CGDisplayShowCursor(CGMainDisplayID());
}

static uint32_t gFakeScreenDevice;

GDHandle DMGetFirstScreenDevice(Boolean a)
{
    if (verbose) puts("STUB: DMGetFirstScreenDevice called");
	return (GDHandle)&gFakeScreenDevice;
}

GDHandle DMGetNextScreenDevice (GDHandle a, Boolean b)
{
    if (verbose) puts("STUB: DMGetNextScreenDevice called");
	return (GDHandle)0;
}

GDHandle GetMainDevice()
{
    if (verbose) puts("STUB: GetMainDevice called");
	return (GDHandle)0;
}

CGDirectDisplayID QDGetCGDirectDisplayID(GDHandle a)
{
    if (verbose) puts("STUB: QDGetCGDirectDisplayID called");
	return (CGDirectDisplayID)a;
}

FMFont FMGetFontFromATSFontRef(ATSFontRef a)
{
    if (verbose) puts("STUB: FMGetFontFromATSFontRef called");
	return (FMFont)0;
}
