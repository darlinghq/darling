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


#include <Ink/Ink.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* InkAddStrokeToCurrentPhrase(void)
{
    if (verbose) puts("STUB: InkAddStrokeToCurrentPhrase called");
    return NULL;
}

void* InkIsPhraseInProgress(void)
{
    if (verbose) puts("STUB: InkIsPhraseInProgress called");
    return NULL;
}

void* InkSetApplicationRecognitionMode(void)
{
    if (verbose) puts("STUB: InkSetApplicationRecognitionMode called");
    return NULL;
}

void* InkSetApplicationWritingMode(void)
{
    if (verbose) puts("STUB: InkSetApplicationWritingMode called");
    return NULL;
}

void* InkSetDrawingMode(void)
{
    if (verbose) puts("STUB: InkSetDrawingMode called");
    return NULL;
}

void* InkSetPhraseTerminationMode(void)
{
    if (verbose) puts("STUB: InkSetPhraseTerminationMode called");
    return NULL;
}

void* InkStrokeGetPointCount(void)
{
    if (verbose) puts("STUB: InkStrokeGetPointCount called");
    return NULL;
}

void* InkStrokeGetPoints(void)
{
    if (verbose) puts("STUB: InkStrokeGetPoints called");
    return NULL;
}

void* InkStrokeGetTypeID(void)
{
    if (verbose) puts("STUB: InkStrokeGetTypeID called");
    return NULL;
}

void* InkTerminateCurrentPhrase(void)
{
    if (verbose) puts("STUB: InkTerminateCurrentPhrase called");
    return NULL;
}

void* InkTextAlternatesCount(void)
{
    if (verbose) puts("STUB: InkTextAlternatesCount called");
    return NULL;
}

void* InkTextBounds(void)
{
    if (verbose) puts("STUB: InkTextBounds called");
    return NULL;
}

void* InkTextCopy(void)
{
    if (verbose) puts("STUB: InkTextCopy called");
    return NULL;
}

void* InkTextCreateCFString(void)
{
    if (verbose) puts("STUB: InkTextCreateCFString called");
    return NULL;
}

void* InkTextCreateFromCFData(void)
{
    if (verbose) puts("STUB: InkTextCreateFromCFData called");
    return NULL;
}

void* InkTextDraw(void)
{
    if (verbose) puts("STUB: InkTextDraw called");
    return NULL;
}

void* InkTextFlatten(void)
{
    if (verbose) puts("STUB: InkTextFlatten called");
    return NULL;
}

void* InkTextGetStroke(void)
{
    if (verbose) puts("STUB: InkTextGetStroke called");
    return NULL;
}

void* InkTextGetStrokeCount(void)
{
    if (verbose) puts("STUB: InkTextGetStrokeCount called");
    return NULL;
}

void* InkTextGetTypeID(void)
{
    if (verbose) puts("STUB: InkTextGetTypeID called");
    return NULL;
}

void* InkTextKeyModifiers(void)
{
    if (verbose) puts("STUB: InkTextKeyModifiers called");
    return NULL;
}

void* InkUserWritingMode(void)
{
    if (verbose) puts("STUB: InkUserWritingMode called");
    return NULL;
}
