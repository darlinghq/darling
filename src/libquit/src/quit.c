/*
 This file is part of Darling.

 Copyright (C) 2017 Lubos Dolezel

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


#include <quit/quit.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* LQCachePort(void) {
    if (verbose) puts("STUB: LQCachePort called");
    return NULL;
}

void* LQForceQuit(void) {
    if (verbose) puts("STUB: LQForceQuit called");
    return NULL;
}

void* LQNotifyCpuResource(void) {
    if (verbose) puts("STUB: LQNotifyCpuResource called");
    return NULL;
}

void* LQNotifyFacelessQuitHappening(void) {
    if (verbose) puts("STUB: LQNotifyFacelessQuitHappening called");
    return NULL;
}

void* LQNotifyQuitLikely(void) {
    if (verbose) puts("STUB: LQNotifyQuitLikely called");
    return NULL;
}

void* LQNotifySpinLikely(void) {
    if (verbose) puts("STUB: LQNotifySpinLikely called");
    return NULL;
}

void* LQNotifyStuckApp(void) {
    if (verbose) puts("STUB: LQNotifyStuckApp called");
    return NULL;
}

void* LQNotifyUiQuitHappening(void) {
    if (verbose) puts("STUB: LQNotifyUiQuitHappening called");
    return NULL;
}

void* LQNotifyUnstuckApp(void) {
    if (verbose) puts("STUB: LQNotifyUnstuckApp called");
    return NULL;
}

void* LQNotifyWakeupsResource(void) {
    if (verbose) puts("STUB: LQNotifyWakeupsResource called");
    return NULL;
}

void* LQReportCpuResource(void) {
    if (verbose) puts("STUB: LQReportCpuResource called");
    return NULL;
}

void* LQReportPotentialHang(void) {
    if (verbose) puts("STUB: LQReportPotentialHang called");
    return NULL;
}

void* LQReportPotentialSpin(void) {
    if (verbose) puts("STUB: LQReportPotentialSpin called");
    return NULL;
}

void* LQReportStuckApp(void) {
    if (verbose) puts("STUB: LQReportStuckApp called");
    return NULL;
}

void* LQReportUnstuckApp(void) {
    if (verbose) puts("STUB: LQReportUnstuckApp called");
    return NULL;
}

void* LQReportWakeupsResource(void) {
    if (verbose) puts("STUB: LQReportWakeupsResource called");
    return NULL;
}
