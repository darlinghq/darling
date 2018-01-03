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


#include <SpotlightDaemon/SpotlightDaemon.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* BulkDeleteAttributes(void) {
    if (verbose) puts("STUB: BulkDeleteAttributes called");
    return NULL;
}

void* SDTraceAdd(void) {
    if (verbose) puts("STUB: SDTraceAdd called");
    return NULL;
}

void* SDTraceDescription(void) {
    if (verbose) puts("STUB: SDTraceDescription called");
    return NULL;
}

void* SDTransactionCreate(void) {
    if (verbose) puts("STUB: SDTransactionCreate called");
    return NULL;
}

void* SDTransactionDone(void) {
    if (verbose) puts("STUB: SDTransactionDone called");
    return NULL;
}

void* SISynchedOpWithBlock(void) {
    if (verbose) puts("STUB: SISynchedOpWithBlock called");
    return NULL;
}

void* SISynchedOpWithBlockPropagatingPriority(void) {
    if (verbose) puts("STUB: SISynchedOpWithBlockPropagatingPriority called");
    return NULL;
}

void* blockRoutine(void) {
    if (verbose) puts("STUB: blockRoutine called");
    return NULL;
}

void* delayedOpBlockCompleteCallback(void) {
    if (verbose) puts("STUB: delayedOpBlockCompleteCallback called");
    return NULL;
}

void* startAgents(void) {
    if (verbose) puts("STUB: startAgents called");
    return NULL;
}

void* startAllAgents(void) {
    if (verbose) puts("STUB: startAllAgents called");
    return NULL;
}

void* startIndexAgent(void) {
    if (verbose) puts("STUB: startIndexAgent called");
    return NULL;
}

void* syncFinished(void) {
    if (verbose) puts("STUB: syncFinished called");
    return NULL;
}

void* unpauseCallback(void) {
    if (verbose) puts("STUB: unpauseCallback called");
    return NULL;
}
