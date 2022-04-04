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


#include <OSAKit/OSAKit.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* ActiveProc(void)
{
    if (verbose) puts("STUB: ActiveProc called");
    return NULL;
}

void* FilterProc(void)
{
    if (verbose) puts("STUB: FilterProc called");
    return NULL;
}

void* IdleProc(void)
{
    if (verbose) puts("STUB: IdleProc called");
    return NULL;
}

void* OSA_IsHardLineBreakUnichar(void)
{
    if (verbose) puts("STUB: OSA_IsHardLineBreakUnichar called");
    return NULL;
}

void* OSA_attributedStringFromDescriptor(void)
{
    if (verbose) puts("STUB: OSA_attributedStringFromDescriptor called");
    return NULL;
}

void* OSA_attributedStringWithCarbonTEData(void)
{
    if (verbose) puts("STUB: OSA_attributedStringWithCarbonTEData called");
    return NULL;
}

void* OSA_carbonTEDataFromAttributedString(void)
{
    if (verbose) puts("STUB: OSA_carbonTEDataFromAttributedString called");
    return NULL;
}

void* OSA_findMatchingBraceForRangeInString(void)
{
    if (verbose) puts("STUB: OSA_findMatchingBraceForRangeInString called");
    return NULL;
}

void* OSA_isClosingBrace(void)
{
    if (verbose) puts("STUB: OSA_isClosingBrace called");
    return NULL;
}

void* OSA_isOpeningBrace(void)
{
    if (verbose) puts("STUB: OSA_isOpeningBrace called");
    return NULL;
}

void* OSA_matchingDelimiter(void)
{
    if (verbose) puts("STUB: OSA_matchingDelimiter called");
    return NULL;
}

void* OSA_numberOfLeadingSpacesFromRangeInString(void)
{
    if (verbose) puts("STUB: OSA_numberOfLeadingSpacesFromRangeInString called");
    return NULL;
}

void* OSA_tabStopArrayForFontAndTabWidth(void)
{
    if (verbose) puts("STUB: OSA_tabStopArrayForFontAndTabWidth called");
    return NULL;
}

void* OSA_tabbifiedStringWithNumberOfSpaces(void)
{
    if (verbose) puts("STUB: OSA_tabbifiedStringWithNumberOfSpaces called");
    return NULL;
}

void* SendProc(void)
{
    if (verbose) puts("STUB: SendProc called");
    return NULL;
}

void* SetSegmentDescriptions(void)
{
    if (verbose) puts("STUB: SetSegmentDescriptions called");
    return NULL;
}

void* _OSA_identifierChars(void)
{
    if (verbose) puts("STUB: _OSA_identifierChars called");
    return NULL;
}

void* _osa_fileRefFromPath(void)
{
    if (verbose) puts("STUB: _osa_fileRefFromPath called");
    return NULL;
}

void* _osa_pathFromFSRef(void)
{
    if (verbose) puts("STUB: _osa_pathFromFSRef called");
    return NULL;
}

void* scriptAssistantCaseCompareItems(void)
{
    if (verbose) puts("STUB: scriptAssistantCaseCompareItems called");
    return NULL;
}
