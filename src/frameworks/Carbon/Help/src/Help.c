/*
 This file is part of Darling.

 Copyright (C) 2021 Lubos Dolezel

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


#include <Help/Help.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* AHGotoApplicationHelp(void)
{
    if (verbose) puts("STUB: AHGotoApplicationHelp called");
    return NULL;
}

void* AHGotoMainTOC(void)
{
    if (verbose) puts("STUB: AHGotoMainTOC called");
    return NULL;
}

void* AHGotoPage(void)
{
    if (verbose) puts("STUB: AHGotoPage called");
    return NULL;
}

void* AHLookupAnchor(void)
{
    if (verbose) puts("STUB: AHLookupAnchor called");
    return NULL;
}

void* AHRegisterHelpBook(void)
{
    if (verbose) puts("STUB: AHRegisterHelpBook called");
    return NULL;
}

void* AHRegisterHelpBookWithURL(void)
{
    if (verbose) puts("STUB: AHRegisterHelpBookWithURL called");
    return NULL;
}

void* AHSearch(void)
{
    if (verbose) puts("STUB: AHSearch called");
    return NULL;
}

void* AppendEncodingHint(void)
{
    if (verbose) puts("STUB: AppendEncodingHint called");
    return NULL;
}

void* CFStringToCString(void)
{
    if (verbose) puts("STUB: CFStringToCString called");
    return NULL;
}

void* CopyBookDetails(void)
{
    if (verbose) puts("STUB: CopyBookDetails called");
    return NULL;
}

void* CreateBookDetailsFromAppBundle(void)
{
    if (verbose) puts("STUB: CreateBookDetailsFromAppBundle called");
    return NULL;
}

void* CreateBookDetailsFromAppBundleAtIndex(void)
{
    if (verbose) puts("STUB: CreateBookDetailsFromAppBundleAtIndex called");
    return NULL;
}

void* CreateBookDetailsWithBookNameAndID(void)
{
    if (verbose) puts("STUB: CreateBookDetailsWithBookNameAndID called");
    return NULL;
}

void* EscapeMacCharacters(void)
{
    if (verbose) puts("STUB: EscapeMacCharacters called");
    return NULL;
}

void* MacPathToURLPath(void)
{
    if (verbose) puts("STUB: MacPathToURLPath called");
    return NULL;
}
