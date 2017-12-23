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


#include <CoreImage/CoreImage.h>
#include <stdlib.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* AVCameraCalibrationDataClass(void) {
    if (verbose) puts("STUB: AVCameraCalibrationDataClass called");
    return NULL;
}

void* AVCameraCalibrationDataIntrinsicMatrix(void) {
    if (verbose) puts("STUB: AVCameraCalibrationDataIntrinsicMatrix called");
    return NULL;
}

void* AVCameraCalibrationDataIntrinsicMatrixReferenceDimensions(void) {
    if (verbose) puts("STUB: AVCameraCalibrationDataIntrinsicMatrixReferenceDimensions called");
    return NULL;
}

void* AVDepthDataClass(void) {
    if (verbose) puts("STUB: AVDepthDataClass called");
    return NULL;
}

void* AVDepthDataGetAccuracy(void) {
    if (verbose) puts("STUB: AVDepthDataGetAccuracy called");
    return NULL;
}

void* AVDepthDataGetPortraitScore(void) {
    if (verbose) puts("STUB: AVDepthDataGetPortraitScore called");
    return NULL;
}

void* AVFDepthCameraCalibrationData(void) {
    if (verbose) puts("STUB: AVFDepthCameraCalibrationData called");
    return NULL;
}

void* AVFDepthDataByConvertingToDepthDataType(void) {
    if (verbose) puts("STUB: AVFDepthDataByConvertingToDepthDataType called");
    return NULL;
}

void* AVFDepthDataByReplacingDepthDataMapWithPixelBuffer(void) {
    if (verbose) puts("STUB: AVFDepthDataByReplacingDepthDataMapWithPixelBuffer called");
    return NULL;
}

void* AVFDepthDataDictionaryRepresentation(void) {
    if (verbose) puts("STUB: AVFDepthDataDictionaryRepresentation called");
    return NULL;
}

void* AVFDepthDataFromDictionary(void) {
    if (verbose) puts("STUB: AVFDepthDataFromDictionary called");
    return NULL;
}

void* AVFDepthDataMap(void) {
    if (verbose) puts("STUB: AVFDepthDataMap called");
    return NULL;
}

void* AVFDepthDataType(void) {
    if (verbose) puts("STUB: AVFDepthDataType called");
    return NULL;
}

void* CI_GenericRGBLinear_to_TempTint(void) {
    if (verbose) puts("STUB: CI_GenericRGBLinear_to_TempTint called");
    return NULL;
}

void* CI_TempTint_to_xy(void) {
    if (verbose) puts("STUB: CI_TempTint_to_xy called");
    return NULL;
}

void* CI_xy_to_TempTint(void) {
    if (verbose) puts("STUB: CI_xy_to_TempTint called");
    return NULL;
}

void* open_memstream(void) {
    if (verbose) puts("STUB: open_memstream called");
    return NULL;
}
