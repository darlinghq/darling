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


#include <AVFoundation/AVFoundation.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* AVMakeRectWithAspectRatioInsideRect(void) {
    if (verbose) puts("STUB: AVMakeRectWithAspectRatioInsideRect called");
    return NULL;
}

void* AVMediaSubtypesForAC3(void) {
    if (verbose) puts("STUB: AVMediaSubtypesForAC3 called");
    return NULL;
}

void* AVMediaSubtypesForEnhancedAC3(void) {
    if (verbose) puts("STUB: AVMediaSubtypesForEnhancedAC3 called");
    return NULL;
}

void* AVMediaTypesForCaptionEditing(void) {
    if (verbose) puts("STUB: AVMediaTypesForCaptionEditing called");
    return NULL;
}

void* AVOutputDeviceGetFigRouteDescriptor(void) {
    if (verbose) puts("STUB: AVOutputDeviceGetFigRouteDescriptor called");
    return NULL;
}

void* CGAffineTransformFromNSArray(void) {
    if (verbose) puts("STUB: CGAffineTransformFromNSArray called");
    return NULL;
}

void* MediaIODeviceConfigureToBestMatchPixelBufferAttributesAndMinimumFrameDuration(void) {
    if (verbose) puts("STUB: MediaIODeviceConfigureToBestMatchPixelBufferAttributesAndMinimumFrameDuration called");
    return NULL;
}

void* MediaIOGraphConnectNodeToFanOutNode(void) {
    if (verbose) puts("STUB: MediaIOGraphConnectNodeToFanOutNode called");
    return NULL;
}

void* MediaIOGraphRemoveAllNodes(void) {
    if (verbose) puts("STUB: MediaIOGraphRemoveAllNodes called");
    return NULL;
}

void* MediaIOGraphUnusedNodeIndexForNodeFunctionalDesignation(void) {
    if (verbose) puts("STUB: MediaIOGraphUnusedNodeIndexForNodeFunctionalDesignation called");
    return NULL;
}

void* NSArrayFromCGAffineTransform(void) {
    if (verbose) puts("STUB: NSArrayFromCGAffineTransform called");
    return NULL;
}

void* NSArrayFromRGBAComponents(void) {
    if (verbose) puts("STUB: NSArrayFromRGBAComponents called");
    return NULL;
}

void* NSArrayGetRGBAComponents(void) {
    if (verbose) puts("STUB: NSArrayGetRGBAComponents called");
    return NULL;
}
