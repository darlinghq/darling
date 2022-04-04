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


#include <AssertionServices/AssertionServices.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* BKSAssertionErrorDescription(void)
{
    if (verbose) puts("STUB: BKSAssertionErrorDescription called");
    return NULL;
}

void* BKSProcessAssertionBackgroundTimeRemaining(void)
{
    if (verbose) puts("STUB: BKSProcessAssertionBackgroundTimeRemaining called");
    return NULL;
}

void* BKSProcessAssertionSetExpirationHandler(void)
{
    if (verbose) puts("STUB: BKSProcessAssertionSetExpirationHandler called");
    return NULL;
}

void* BKSProcessDiagnosticReportTypeDescription(void)
{
    if (verbose) puts("STUB: BKSProcessDiagnosticReportTypeDescription called");
    return NULL;
}

void* BKSProcessVisibilityIsForeground(void)
{
    if (verbose) puts("STUB: BKSProcessVisibilityIsForeground called");
    return NULL;
}

void* BKSProcessVisibilityMax(void)
{
    if (verbose) puts("STUB: BKSProcessVisibilityMax called");
    return NULL;
}

void* BKSTerminationAssertionActiveEfficacyForBundleIdentifier(void)
{
    if (verbose) puts("STUB: BKSTerminationAssertionActiveEfficacyForBundleIdentifier called");
    return NULL;
}

void* BKSTerminationAssertionEfficacyDescription(void)
{
    if (verbose) puts("STUB: BKSTerminationAssertionEfficacyDescription called");
    return NULL;
}

void* BKSTerminationAssertionHeldForBundleIdentifier(void)
{
    if (verbose) puts("STUB: BKSTerminationAssertionHeldForBundleIdentifier called");
    return NULL;
}

void* BKSTerminationAssertionRegisterObserver(void)
{
    if (verbose) puts("STUB: BKSTerminationAssertionRegisterObserver called");
    return NULL;
}

void* BKSTerminationAssertionUnregisterObserver(void)
{
    if (verbose) puts("STUB: BKSTerminationAssertionUnregisterObserver called");
    return NULL;
}

void* BKSWatchdogAssertionCreateForPID(void)
{
    if (verbose) puts("STUB: BKSWatchdogAssertionCreateForPID called");
    return NULL;
}

void* BKSWatchdogAssertionGetTypeID(void)
{
    if (verbose) puts("STUB: BKSWatchdogAssertionGetTypeID called");
    return NULL;
}

void* NSStringFromBKSProcessErrorCode(void)
{
    if (verbose) puts("STUB: NSStringFromBKSProcessErrorCode called");
    return NULL;
}

void* NSStringFromBKSProcessExitReason(void)
{
    if (verbose) puts("STUB: NSStringFromBKSProcessExitReason called");
    return NULL;
}

void* NSStringFromBKSProcessTaskState(void)
{
    if (verbose) puts("STUB: NSStringFromBKSProcessTaskState called");
    return NULL;
}

void* NSStringFromBKSProcessVisibility(void)
{
    if (verbose) puts("STUB: NSStringFromBKSProcessVisibility called");
    return NULL;
}

void* RBSProcessLegacyStateDescriptor(void)
{
    if (verbose) puts("STUB: RBSProcessLegacyStateDescriptor called");
    return NULL;
}
