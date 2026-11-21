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

#include <QTKit/QTKit.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* QTErrorFromOSStatus(void)
{
    if (verbose) puts("STUB: QTErrorFromOSStatus called");
    return NULL;
}

void* QTGetComponentsFromFloatTime(void)
{
    if (verbose) puts("STUB: QTGetComponentsFromFloatTime called");
    return NULL;
}

void* QTGetComponentsFromRationalTime(void)
{
    if (verbose) puts("STUB: QTGetComponentsFromRationalTime called");
    return NULL;
}

void* QTGetResolvedImageConsumerAttributes(void)
{
    if (verbose) puts("STUB: QTGetResolvedImageConsumerAttributes called");
    return NULL;
}

void* QTKitCanUseIOSurface(void)
{
    if (verbose) puts("STUB: QTKitCanUseIOSurface called");
    return NULL;
}

void* QTLocalizedStringFromErrorCode(void)
{
    if (verbose) puts("STUB: QTLocalizedStringFromErrorCode called");
    return NULL;
}

void* QTLocalizedStringFromKey(void)
{
    if (verbose) puts("STUB: QTLocalizedStringFromKey called");
    return NULL;
}

void* QTLocalizedStringFromTable(void)
{
    if (verbose) puts("STUB: QTLocalizedStringFromTable called");
    return NULL;
}

void* QTSetThreadName(void)
{
    if (verbose) puts("STUB: QTSetThreadName called");
    return NULL;
}

void* QTStringFromSMPTETime(void)
{
    if (verbose) puts("STUB: QTStringFromSMPTETime called");
    return NULL;
}

void* QTTimeFormatterCopyProperty(void)
{
    if (verbose) puts("STUB: QTTimeFormatterCopyProperty called");
    return NULL;
}

void* QTTimeFormatterCreate(void)
{
    if (verbose) puts("STUB: QTTimeFormatterCreate called");
    return NULL;
}

void* QTTimeFormatterCreateStringFromFloatTime(void)
{
    if (verbose) puts("STUB: QTTimeFormatterCreateStringFromFloatTime called");
    return NULL;
}

void* QTTimeFormatterCreateStringFromRationalTime(void)
{
    if (verbose) puts("STUB: QTTimeFormatterCreateStringFromRationalTime called");
    return NULL;
}

void* QTTimeFormatterGetFloatTimeFromString(void)
{
    if (verbose) puts("STUB: QTTimeFormatterGetFloatTimeFromString called");
    return NULL;
}

void* QTTimeFormatterGetLocale(void)
{
    if (verbose) puts("STUB: QTTimeFormatterGetLocale called");
    return NULL;
}

void* QTTimeFormatterGetRationalTimeFromString(void)
{
    if (verbose) puts("STUB: QTTimeFormatterGetRationalTimeFromString called");
    return NULL;
}

void* QTTimeFormatterGetTypeID(void)
{
    if (verbose) puts("STUB: QTTimeFormatterGetTypeID called");
    return NULL;
}

void* QTTimeFormatterSetProperty(void)
{
    if (verbose) puts("STUB: QTTimeFormatterSetProperty called");
    return NULL;
}

void* QTUICopyMeasurements(void)
{
    if (verbose) puts("STUB: QTUICopyMeasurements called");
    return NULL;
}

void* QTUIDraw(void)
{
    if (verbose) puts("STUB: QTUIDraw called");
    return NULL;
}

void* QTUIHitTest(void)
{
    if (verbose) puts("STUB: QTUIHitTest called");
    return NULL;
}
