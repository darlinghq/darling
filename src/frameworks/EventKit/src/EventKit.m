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


#include <EventKit/EventKit.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* EKAutoCommentPrefix(void)
{
    if (verbose) puts("STUB: EKAutoCommentPrefix called");
    return NULL;
}

void* EKAvailabilitySpanTypeAsString(void)
{
    if (verbose) puts("STUB: EKAvailabilitySpanTypeAsString called");
    return NULL;
}

void* EKBundle(void)
{
    if (verbose) puts("STUB: EKBundle called");
    return NULL;
}

void* EKTravelEngineLogInitialize(void)
{
    if (verbose) puts("STUB: EKTravelEngineLogInitialize called");
    return NULL;
}

void* EKUIAttendeeUtils_AttendeeHasResponded(void)
{
    if (verbose) puts("STUB: EKUIAttendeeUtils_AttendeeHasResponded called");
    return NULL;
}

void* EKUtils_AdjustedAttendeeAddress(void)
{
    if (verbose) puts("STUB: EKUtils_AdjustedAttendeeAddress called");
    return NULL;
}

void* _bitmaskContainsOption(void)
{
    if (verbose) puts("STUB: _bitmaskContainsOption called");
    return NULL;
}

void* _bitmaskExclusivelyMatchesOption(void)
{
    if (verbose) puts("STUB: _bitmaskExclusivelyMatchesOption called");
    return NULL;
}

void* _bitmaskMatchesExactlyOneOptionFromList(void)
{
    if (verbose) puts("STUB: _bitmaskMatchesExactlyOneOptionFromList called");
    return NULL;
}

void* _boolToString(void)
{
    if (verbose) puts("STUB: _boolToString called");
    return NULL;
}

void* _validMoveEventRequirementsCombination(void)
{
    if (verbose) puts("STUB: _validMoveEventRequirementsCombination called");
    return NULL;
}

void* barrierAsyncIfQueueNotNil(void)
{
    if (verbose) puts("STUB: barrierAsyncIfQueueNotNil called");
    return NULL;
}

void* barrierSyncBool(void)
{
    if (verbose) puts("STUB: barrierSyncBool called");
    return NULL;
}

void* barrierSyncIfQueueNotNil(void)
{
    if (verbose) puts("STUB: barrierSyncIfQueueNotNil called");
    return NULL;
}

void* defaultValueForKey(void)
{
    if (verbose) puts("STUB: defaultValueForKey called");
    return NULL;
}

void* dispatch_async_main_if_necessary(void)
{
    if (verbose) puts("STUB: dispatch_async_main_if_necessary called");
    return NULL;
}

void* dispatch_queue_create_recursive_concurrent(void)
{
    if (verbose) puts("STUB: dispatch_queue_create_recursive_concurrent called");
    return NULL;
}

void* dispatch_sync_recursive(void)
{
    if (verbose) puts("STUB: dispatch_sync_recursive called");
    return NULL;
}

void* resetStaticObjectCreationVariables(void)
{
    if (verbose) puts("STUB: resetStaticObjectCreationVariables called");
    return NULL;
}

void* stringFromEKEntityType(void)
{
    if (verbose) puts("STUB: stringFromEKEntityType called");
    return NULL;
}

void* stringFromEKSharingStatus(void)
{
    if (verbose) puts("STUB: stringFromEKSharingStatus called");
    return NULL;
}

void* syncBoolIfQueueNotNil(void)
{
    if (verbose) puts("STUB: syncBoolIfQueueNotNil called");
    return NULL;
}

void* syncIfQueueNotNil(void)
{
    if (verbose) puts("STUB: syncIfQueueNotNil called");
    return NULL;
}
