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


#include <Vision/Vision.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* CCRectLowLevel(void)
{
    if (verbose) puts("STUB: CCRectLowLevel called");
    return NULL;
}

void* VNCropAndScaleFromURLToPixelBuffers(void)
{
    if (verbose) puts("STUB: VNCropAndScaleFromURLToPixelBuffers called");
    return NULL;
}

void* VNCropAndScalePixelBufferToPixelBuffers(void)
{
    if (verbose) puts("STUB: VNCropAndScalePixelBufferToPixelBuffers called");
    return NULL;
}

void* VNElementCountForScenePrintRequestRevision(void)
{
    if (verbose) puts("STUB: VNElementCountForScenePrintRequestRevision called");
    return NULL;
}

void* VNElementTypeSize(void)
{
    if (verbose) puts("STUB: VNElementTypeSize called");
    return NULL;
}

void* VNImagePointForFaceLandmarkPoint(void)
{
    if (verbose) puts("STUB: VNImagePointForFaceLandmarkPoint called");
    return NULL;
}

void* VNImagePointForNormalizedPoint(void)
{
    if (verbose) puts("STUB: VNImagePointForNormalizedPoint called");
    return NULL;
}

void* VNImageRectForNormalizedRect(void)
{
    if (verbose) puts("STUB: VNImageRectForNormalizedRect called");
    return NULL;
}

void* VNIntersectionOverUnion(void)
{
    if (verbose) puts("STUB: VNIntersectionOverUnion called");
    return NULL;
}

void* VNLargestCentralSquareOfRect(void)
{
    if (verbose) puts("STUB: VNLargestCentralSquareOfRect called");
    return NULL;
}

void* VNLengthInBytesForScenePrintRequestRevision(void)
{
    if (verbose) puts("STUB: VNLengthInBytesForScenePrintRequestRevision called");
    return NULL;
}

void* VNNormalizedFaceBoundingBoxPointForLandmarkPoint(void)
{
    if (verbose) puts("STUB: VNNormalizedFaceBoundingBoxPointForLandmarkPoint called");
    return NULL;
}

void* VNNormalizedImageRectForNormalizedRegionOfInterestRect(void)
{
    if (verbose) puts("STUB: VNNormalizedImageRectForNormalizedRegionOfInterestRect called");
    return NULL;
}

void* VNNormalizedRectForImageRect(void)
{
    if (verbose) puts("STUB: VNNormalizedRectForImageRect called");
    return NULL;
}

void* VNNormalizedRectIsIdentityRect(void)
{
    if (verbose) puts("STUB: VNNormalizedRectIsIdentityRect called");
    return NULL;
}

void* VNScenePrintsFromPixelBuffers(void)
{
    if (verbose) puts("STUB: VNScenePrintsFromPixelBuffers called");
    return NULL;
}

void* VNScenePrintsFromPixelBuffersUsesCPUOnly(void)
{
    if (verbose) puts("STUB: VNScenePrintsFromPixelBuffersUsesCPUOnly called");
    return NULL;
}

void* VNScenePrintsFromURL(void)
{
    if (verbose) puts("STUB: VNScenePrintsFromURL called");
    return NULL;
}

void* VNScenePrintsFromURLUsesCPUOnly(void)
{
    if (verbose) puts("STUB: VNScenePrintsFromURLUsesCPUOnly called");
    return NULL;
}

void* VNSceneprintSupportedRevisions(void)
{
    if (verbose) puts("STUB: VNSceneprintSupportedRevisions called");
    return NULL;
}

void* VNVerticallyFlippedNormalizedPoint(void)
{
    if (verbose) puts("STUB: VNVerticallyFlippedNormalizedPoint called");
    return NULL;
}

void* VNVerticallyFlippedNormalizedRect(void)
{
    if (verbose) puts("STUB: VNVerticallyFlippedNormalizedRect called");
    return NULL;
}

void* createPerMeshForFFTSIZE(void)
{
    if (verbose) puts("STUB: createPerMeshForFFTSIZE called");
    return NULL;
}

void* horizonDetectionFFT(void)
{
    if (verbose) puts("STUB: horizonDetectionFFT called");
    return NULL;
}

void* horizonDetectionFFTAngles(void)
{
    if (verbose) puts("STUB: horizonDetectionFFTAngles called");
    return NULL;
}

void* initializePerMesh(void)
{
    if (verbose) puts("STUB: initializePerMesh called");
    return NULL;
}
