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


#include <Spotlight/Spotlight.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* SPKStringFromResponseKind(void) {
    if (verbose) puts("STUB: SPKStringFromResponseKind called");
    return NULL;
}

void* SPLogInit(void) {
    if (verbose) puts("STUB: SPLogInit called");
    return NULL;
}

void* SPRoundToNSignificantDigits(void) {
    if (verbose) puts("STUB: SPRoundToNSignificantDigits called");
    return NULL;
}

void* bootDriveIsFast(void) {
    if (verbose) puts("STUB: bootDriveIsFast called");
    return NULL;
}

void* enumerateQueryResultsUsingBlock(void) {
    if (verbose) puts("STUB: enumerateQueryResultsUsingBlock called");
    return NULL;
}

void* mt_log_spotlight_category_orderings(void) {
    if (verbose) puts("STUB: mt_log_spotlight_category_orderings called");
    return NULL;
}

void* mt_log_spotlight_search_session(void) {
    if (verbose) puts("STUB: mt_log_spotlight_search_session called");
    return NULL;
}
