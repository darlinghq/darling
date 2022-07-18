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


#include <ContactsUI/ContactsUI.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* CNUIDebugLoggingEnabled(void)
{
    if (verbose) puts("STUB: CNUIDebugLoggingEnabled called");
    return NULL;
}

void* CNUILoggingContextDebug(void)
{
    if (verbose) puts("STUB: CNUILoggingContextDebug called");
    return NULL;
}

void* CNUILoggingContextPerformance(void)
{
    if (verbose) puts("STUB: CNUILoggingContextPerformance called");
    return NULL;
}

void* CNUILoggingSubsystem(void)
{
    if (verbose) puts("STUB: CNUILoggingSubsystem called");
    return NULL;
}

void* CNUIPerformanceLoggingEnabled(void)
{
    if (verbose) puts("STUB: CNUIPerformanceLoggingEnabled called");
    return NULL;
}

void* sharedSessionQueue(void)
{
    if (verbose) puts("STUB: sharedSessionQueue called");
    return NULL;
}
