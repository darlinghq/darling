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

void* QTEqualTimeRanges(void)
{
    if (verbose) puts("STUB: QTEqualTimeRanges called");
    return NULL;
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

void* QTGetTimeInterval(void)
{
    if (verbose) puts("STUB: QTGetTimeInterval called");
    return NULL;
}

void* QTGetTimeRecord(void)
{
    if (verbose) puts("STUB: QTGetTimeRecord called");
    return NULL;
}

void* QTIntersectionTimeRange(void)
{
    if (verbose) puts("STUB: QTIntersectionTimeRange called");
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

void* QTMakeTime(void)
{
    if (verbose) puts("STUB: QTMakeTime called");
    return NULL;
}

void* QTMakeTimeRange(void)
{
    if (verbose) puts("STUB: QTMakeTimeRange called");
    return NULL;
}

void* QTMakeTimeScaled(void)
{
    if (verbose) puts("STUB: QTMakeTimeScaled called");
    return NULL;
}

void* QTMakeTimeWithTimeInterval(void)
{
    if (verbose) puts("STUB: QTMakeTimeWithTimeInterval called");
    return NULL;
}

void* QTMakeTimeWithTimeRecord(void)
{
    if (verbose) puts("STUB: QTMakeTimeWithTimeRecord called");
    return NULL;
}

void* QTMediaTypeStringForOSType(void)
{
    if (verbose) puts("STUB: QTMediaTypeStringForOSType called");
    return NULL;
}

void* QTOSTypeForString(void)
{
    if (verbose) puts("STUB: QTOSTypeForString called");
    return NULL;
}

void* QTSetThreadName(void)
{
    if (verbose) puts("STUB: QTSetThreadName called");
    return NULL;
}

void* QTStringForOSType(void)
{
    if (verbose) puts("STUB: QTStringForOSType called");
    return NULL;
}

void* QTStringFromSMPTETime(void)
{
    if (verbose) puts("STUB: QTStringFromSMPTETime called");
    return NULL;
}

void* QTStringFromTime(void)
{
    if (verbose) puts("STUB: QTStringFromTime called");
    return NULL;
}

void* QTStringFromTimeRange(void)
{
    if (verbose) puts("STUB: QTStringFromTimeRange called");
    return NULL;
}

void* QTTimeCompare(void)
{
    if (verbose) puts("STUB: QTTimeCompare called");
    return NULL;
}

void* QTTimeDecrement(void)
{
    if (verbose) puts("STUB: QTTimeDecrement called");
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

void* QTTimeFromString(void)
{
    if (verbose) puts("STUB: QTTimeFromString called");
    return NULL;
}

void* QTTimeInTimeRange(void)
{
    if (verbose) puts("STUB: QTTimeInTimeRange called");
    return NULL;
}

void* QTTimeIncrement(void)
{
    if (verbose) puts("STUB: QTTimeIncrement called");
    return NULL;
}

void* QTTimeIsIndefinite(void)
{
    if (verbose) puts("STUB: QTTimeIsIndefinite called");
    return NULL;
}

void* QTTimeRangeEnd(void)
{
    if (verbose) puts("STUB: QTTimeRangeEnd called");
    return NULL;
}

void* QTTimeRangeFromString(void)
{
    if (verbose) puts("STUB: QTTimeRangeFromString called");
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

void* QTUnionTimeRange(void)
{
    if (verbose) puts("STUB: QTUnionTimeRange called");
    return NULL;
}
