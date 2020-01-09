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


#include <DataDetectors/DataDetectors.h>
#include <stdlib.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* DDActionLogHandle(void) {
    if (verbose) puts("STUB: DDActionLogHandle called");
    return NULL;
}

void* DDHighlightCreateWithRectsInVisibleRect(void) {
    if (verbose) puts("STUB: DDHighlightCreateWithRectsInVisibleRect called");
    return NULL;
}

void* DDHighlightCreateWithRectsInVisibleRectWithStyle(void) {
    if (verbose) puts("STUB: DDHighlightCreateWithRectsInVisibleRectWithStyle called");
    return NULL;
}

void* DDHighlightCreateWithRectsInVisibleRectWithStyleAndDirection(void) {
    if (verbose) puts("STUB: DDHighlightCreateWithRectsInVisibleRectWithStyleAndDirection called");
    return NULL;
}

void* DDHighlightGetActionRect(void) {
    if (verbose) puts("STUB: DDHighlightGetActionRect called");
    return NULL;
}

void* DDHighlightGetBoundingRect(void) {
    if (verbose) puts("STUB: DDHighlightGetBoundingRect called");
    return NULL;
}

void* DDHighlightGetLayerWithContext(void) {
    if (verbose) puts("STUB: DDHighlightGetLayerWithContext called");
    return NULL;
}

void* DDHighlightGetTrackingRects(void) {
    if (verbose) puts("STUB: DDHighlightGetTrackingRects called");
    return NULL;
}

void* DDHighlightPointIsOnButton(void) {
    if (verbose) puts("STUB: DDHighlightPointIsOnButton called");
    return NULL;
}

void* DDHighlightPointIsOnHighlight(void) {
    if (verbose) puts("STUB: DDHighlightPointIsOnHighlight called");
    return NULL;
}

void* DDHighlightWantsRollover(void) {
    if (verbose) puts("STUB: DDHighlightWantsRollover called");
    return NULL;
}

void* DDMapsGetBestAddressForResults(void) {
    if (verbose) puts("STUB: DDMapsGetBestAddressForResults called");
    return NULL;
}

void* DDMapsGetDictionaryForFullAddress(void) {
    if (verbose) puts("STUB: DDMapsGetDictionaryForFullAddress called");
    return NULL;
}

void* DDMapsURLStringForAddressResult(void) {
    if (verbose) puts("STUB: DDMapsURLStringForAddressResult called");
    return NULL;
}

void* DDMapsURLStringForString(void) {
    if (verbose) puts("STUB: DDMapsURLStringForString called");
    return NULL;
}

void* DDMenuItemGetActionUTI(void) {
    if (verbose) puts("STUB: DDMenuItemGetActionUTI called");
    return NULL;
}

void* DDServiceLogHandle(void) {
    if (verbose) puts("STUB: DDServiceLogHandle called");
    return NULL;
}

void* DDUILogHandle(void) {
    if (verbose) puts("STUB: DDUILogHandle called");
    return NULL;
}

void* _DDUseRTLLayout(void) {
    if (verbose) puts("STUB: _DDUseRTLLayout called");
    return NULL;
}

void* dd_can_connect_to_service(void) {
    if (verbose) puts("STUB: dd_can_connect_to_service called");
    return NULL;
}

void* dd_has_bool_entitlement(void) {
    if (verbose) puts("STUB: dd_has_bool_entitlement called");
    return NULL;
}

void* separatorColorForDarkMode(void) {
    if (verbose) puts("STUB: separatorColorForDarkMode called");
    return NULL;
}
