/*
 This file is part of Darling.

 Copyright (C) 2023 Darling Team

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


#include <QuartzFilters/QuartzFilters.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;
__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}


extern void* globalUpdateOK = (void*)0;
extern void* kBPCNames = (void*)0;
extern void* kCustomNames = (void*)0;
extern void* kIntermediateNames = (void*)0;
extern void* const kNonCategoryRowIndent = (void*)0;
extern void* kQFilterAppLicationDomain = (void*)0;
extern void* kQFilterApplicationDomain = (void*)0;
extern void* kQFilterAssignCMYKColorSpace = (void*)0;
extern void* kQFilterAssignGrayColorSpace = (void*)0;
extern void* kQFilterAssignProfile = (void*)0;
extern void* kQFilterAssignRGBColorSpace = (void*)0;
extern void* kQFilterBlackPointCompensation = (void*)0;
extern void* kQFilterBlackPointCompensationCMYK = (void*)0;
extern void* kQFilterBlackPointCompensationGray = (void*)0;
extern void* kQFilterBlackPointCompensationRGB = (void*)0;
extern void* kQFilterCIFilter = (void*)0;
extern void* kQFilterCalibrationTargetSettings = (void*)0;
extern void* kQFilterCategories = (void*)0;
extern void* kQFilterColorCategory = (void*)0;
extern void* kQFilterColorSettings = (void*)0;
extern void* kQFilterColorSpaces = (void*)0;
extern void* kQFilterComments = (void*)0;
extern void* kQFilterComponentControlKeys = (void*)0;
extern void* kQFilterComponentKeys = (void*)0;
extern void* kQFilterControlClassName = (void*)0;
extern void* kQFilterControlDisplayName = (void*)0;
extern void* kQFilterConvertToProfile = (void*)0;
extern void* kQFilterCoreImageFilter = (void*)0;
extern void* kQFilterCustomConversion = (void*)0;
extern void* kQFilterCustomCorrection = (void*)0;
extern void* kQFilterCustomCorrectionCMYK = (void*)0;
extern void* kQFilterCustomCorrectionGray = (void*)0;
extern void* kQFilterCustomCorrectionRGB = (void*)0;
extern void* kQFilterData = (void*)0;
extern void* kQFilterDefaultCMYKColorSpace = (void*)0;
extern void* kQFilterDefaultGrayColorSpace = (void*)0;
extern void* kQFilterDefaultProfiles = (void*)0;
extern void* kQFilterDefaultRGBColorSpace = (void*)0;
extern void* kQFilterDocumentColorSettings = (void*)0;
extern void* kQFilterDomains = (void*)0;
extern void* kQFilterDontManageDeviceN = (void*)0;
extern void* kQFilterFlattenTransparency = (void*)0;
extern void* kQFilterFlattenTransparencyRes = (void*)0;
extern void* kQFilterGraphicsColorSettings = (void*)0;
extern void* kQFilterGraphicsObject = (void*)0;
extern void* kQFilterHalfoneBitmapSizeX = (void*)0;
extern void* kQFilterHalfoneBitmapSizeY = (void*)0;
extern void* kQFilterHalfoneColorSpaceModel = (void*)0;
extern void* kQFilterHalftoneSettings = (void*)0;
extern void* kQFilterIdentifier = (void*)0;
extern void* kQFilterImageAutoCompress = (void*)0;
extern void* kQFilterImageBlur = (void*)0;
extern void* kQFilterImageCategory = (void*)0;
extern void* kQFilterImageColorSettings = (void*)0;
extern void* kQFilterImageCompressQuality = (void*)0;
extern void* kQFilterImageCompression = (void*)0;
extern void* kQFilterImageConvolution = (void*)0;
extern void* kQFilterImageConvolutionRadius = (void*)0;
extern void* kQFilterImageFlateCompress = (void*)0;
extern void* kQFilterImageInterpolateNo = (void*)0;
extern void* kQFilterImageInterpolateOverride = (void*)0;
extern void* kQFilterImageInterpolateYes = (void*)0;
extern void* kQFilterImageJPEGCompress = (void*)0;
extern void* kQFilterImageObject = (void*)0;
extern void* kQFilterImageResolution = (void*)0;
extern void* kQFilterImageScaleFactor = (void*)0;
extern void* kQFilterImageScaleInterpolate = (void*)0;
extern void* kQFilterImageScaleSettings = (void*)0;
extern void* kQFilterImageSettings = (void*)0;
extern void* kQFilterImageSharpen = (void*)0;
extern void* kQFilterImageSize = (void*)0;
extern void* kQFilterImageSizeMax = (void*)0;
extern void* kQFilterImageSizeMin = (void*)0;
extern void* kQFilterImageUncompressed = (void*)0;
extern void* kQFilterInfo = (void*)0;
extern void* kQFilterIntermediateColorSpace = (void*)0;
extern void* kQFilterIntermediateColorSpaceCMYK = (void*)0;
extern void* kQFilterIntermediateColorSpaceGray = (void*)0;
extern void* kQFilterIntermediateColorSpaceRGB = (void*)0;
extern void* kQFilterIntermediateProfile = (void*)0;
extern void* kQFilterMonoImageScaleSettings = (void*)0;
extern void* kQFilterMonochromeColorSpace = (void*)0;
extern void* kQFilterName = (void*)0;
extern void* kQFilterNumberBoolean = (void*)0;
extern void* kQFilterNumberDefault = (void*)0;
extern void* kQFilterNumberFloat = (void*)0;
extern void* kQFilterNumberInteger = (void*)0;
extern void* kQFilterNumberMax = (void*)0;
extern void* kQFilterNumberMin = (void*)0;
extern void* kQFilterNumberType = (void*)0;
extern void* kQFilterNumberValue = (void*)0;
extern void* kQFilterObjects = (void*)0;
extern void* kQFilterPDFCategory = (void*)0;
extern void* kQFilterPDFCompression = (void*)0;
extern void* kQFilterPDFImageInterpolation = (void*)0;
extern void* kQFilterPDFMonochrome = (void*)0;
extern void* kQFilterPDFWorkflowDomain = (void*)0;
extern void* kQFilterPDFX3 = (void*)0;
extern void* kQFilterPDFX3AddBleedBox = (void*)0;
extern void* kQFilterPDFX3AddTrimBox = (void*)0;
extern void* kQFilterPDFX3BleedBox = (void*)0;
extern void* kQFilterPDFX3DestinationProfile = (void*)0;
extern void* kQFilterPDFX3IsTrapped = (void*)0;
extern void* kQFilterPDFX3OutputCondition = (void*)0;
extern void* kQFilterPDFX3OutputConditionID = (void*)0;
extern void* kQFilterPDFX3OutputConditionInfo = (void*)0;
extern void* kQFilterPDFX3RegistryName = (void*)0;
extern void* kQFilterPDFX3Settings = (void*)0;
extern void* kQFilterPDFX3TrimBox = (void*)0;
extern void* kQFilterPreserveEPS = (void*)0;
extern void* kQFilterPrinterProfiles = (void*)0;
extern void* kQFilterPrintingDomain = (void*)0;
extern void* kQFilterPrivateInfo = (void*)0;
extern void* kQFilterProfileData = (void*)0;
extern void* kQFilterReadOnly = (void*)0;
extern void* kQFilterRenderingIntent = (void*)0;
extern void* kQFilterShadeInterpolate = (void*)0;
extern void* kQFilterShadeSettings = (void*)0;
extern void* kQFilterShadeSizeX = (void*)0;
extern void* kQFilterShadeSizeY = (void*)0;
extern void* kQFilterShadingColorSettings = (void*)0;
extern void* kQFilterShadingObject = (void*)0;
extern void* kQFilterSourceRenderingIntent = (void*)0;
extern void* kQFilterSourceRenderingIntentCMYK = (void*)0;
extern void* kQFilterSourceRenderingIntentGray = (void*)0;
extern void* kQFilterSourceRenderingIntentRGB = (void*)0;
extern void* kQFilterTargetColorSpace = (void*)0;
extern void* kQFilterTargetColorSpaceCMYK = (void*)0;
extern void* kQFilterTargetColorSpaceGray = (void*)0;
extern void* kQFilterTargetColorSpaceRGB = (void*)0;
extern void* kQFilterTargetRenderingIntent = (void*)0;
extern void* kQFilterTargetRenderingIntentCMYK = (void*)0;
extern void* kQFilterTargetRenderingIntentGray = (void*)0;
extern void* kQFilterTargetRenderingIntentRGB = (void*)0;
extern void* kQFilterTextColorSettings = (void*)0;
extern void* kQFilterTextObject = (void*)0;
extern void* kQFilterType = (void*)0;
extern void* kQFilterURL = (void*)0;
extern void* kQuartzFilterApplicationDomain = (void*)0;
extern void* kQuartzFilterManagerDidAddFilterNotification = (void*)0;
extern void* kQuartzFilterManagerDidModifyFilterNotification = (void*)0;
extern void* kQuartzFilterManagerDidRemoveFilterNotification = (void*)0;
extern void* kQuartzFilterManagerDidSelectFilterNotification = (void*)0;
extern void* kQuartzFilterManagerOtherNotification = (void*)0;
extern void* kQuartzFilterPDFWorkflowDomain = (void*)0;
extern void* kQuartzFilterPrintingDomain = (void*)0;
extern void* kSrcIntentNames = (void*)0;
extern void* kTargetNames = (void*)0;
extern void* kTrgIntentNames = (void*)0;

void *QuartzFilterApplyToContext(void) {
    if (verbose) puts("STUB: QuartzFilterApplyToContext called");
    return NULL;
}

void *QuartzFilterCopyContextDict(void) {
    if (verbose) puts("STUB: QuartzFilterCopyContextDict called");
    return NULL;
}

void *QuartzFilterCreateHostMatchingFilter(void) {
    if (verbose) puts("STUB: QuartzFilterCreateHostMatchingFilter called");
    return NULL;
}

void *QuartzFilterCreateImageCoverter(void) {
    if (verbose) puts("STUB: QuartzFilterCreateImageCoverter called");
    return NULL;
}

void *QuartzFilterCreateWithOutputIntents(void) {
    if (verbose) puts("STUB: QuartzFilterCreateWithOutputIntents called");
    return NULL;
}

void *QuartzFilterCreateWithURL(void) {
    if (verbose) puts("STUB: QuartzFilterCreateWithURL called");
    return NULL;
}

void *QuartzFilterGetName(void) {
    if (verbose) puts("STUB: QuartzFilterGetName called");
    return NULL;
}

void *QuartzFilterOptionsAddPDFX3Info(void) {
    if (verbose) puts("STUB: QuartzFilterOptionsAddPDFX3Info called");
    return NULL;
}

void *QuartzFilterOptionsCreateCalibrationOutputIntents(void) {
    if (verbose) puts("STUB: QuartzFilterOptionsCreateCalibrationOutputIntents called");
    return NULL;
}

void *QuartzFilterOptionsCreateContextFilterInfo(void) {
    if (verbose) puts("STUB: QuartzFilterOptionsCreateContextFilterInfo called");
    return NULL;
}

void *QuartzFilterOptionsCreateFilterData(void) {
    if (verbose) puts("STUB: QuartzFilterOptionsCreateFilterData called");
    return NULL;
}

void *QuartzFilterOptionsCreateOutputIntents(void) {
    if (verbose) puts("STUB: QuartzFilterOptionsCreateOutputIntents called");
    return NULL;
}

void *QuartzFilterOptionsGetAssignCMYKProfileNames(void) {
    if (verbose) puts("STUB: QuartzFilterOptionsGetAssignCMYKProfileNames called");
    return NULL;
}

void *QuartzFilterOptionsGetAssignGrayProfileNames(void) {
    if (verbose) puts("STUB: QuartzFilterOptionsGetAssignGrayProfileNames called");
    return NULL;
}

void *QuartzFilterOptionsGetAssignRGBProfileNames(void) {
    if (verbose) puts("STUB: QuartzFilterOptionsGetAssignRGBProfileNames called");
    return NULL;
}

void *QuartzFilterOptionsGetDefaultCMYKProfileNames(void) {
    if (verbose) puts("STUB: QuartzFilterOptionsGetDefaultCMYKProfileNames called");
    return NULL;
}

void *QuartzFilterOptionsGetDefaultGrayProfileNames(void) {
    if (verbose) puts("STUB: QuartzFilterOptionsGetDefaultGrayProfileNames called");
    return NULL;
}

void *QuartzFilterOptionsGetDefaultRGBProfileNames(void) {
    if (verbose) puts("STUB: QuartzFilterOptionsGetDefaultRGBProfileNames called");
    return NULL;
}

void *QuartzFilterOptionsGetMidProfileNames(void) {
    if (verbose) puts("STUB: QuartzFilterOptionsGetMidProfileNames called");
    return NULL;
}

void *QuartzFilterOptionsGetPDFXProfileNames(void) {
    if (verbose) puts("STUB: QuartzFilterOptionsGetPDFXProfileNames called");
    return NULL;
}

void *QuartzFilterOptionsGetPrinterDeviceIDFromOutputIntents(void) {
    if (verbose) puts("STUB: QuartzFilterOptionsGetPrinterDeviceIDFromOutputIntents called");
    return NULL;
}

void *QuartzFilterOptionsGetPrinterInfo(void) {
    if (verbose) puts("STUB: QuartzFilterOptionsGetPrinterInfo called");
    return NULL;
}

void *QuartzFilterOptionsGetPrinterProfilesFromOutputIntents(void) {
    if (verbose) puts("STUB: QuartzFilterOptionsGetPrinterProfilesFromOutputIntents called");
    return NULL;
}

void *QuartzFilterOptionsGetSummary(void) {
    if (verbose) puts("STUB: QuartzFilterOptionsGetSummary called");
    return NULL;
}

void *QuartzFilterOptionsGetTargetProfileNames(void) {
    if (verbose) puts("STUB: QuartzFilterOptionsGetTargetProfileNames called");
    return NULL;
}

void *QuartzFilterOptionsInitializeWithFilterData(void) {
    if (verbose) puts("STUB: QuartzFilterOptionsInitializeWithFilterData called");
    return NULL;
}

void *QuartzFilterOptionsInitializeWithOptionsInfo(void) {
    if (verbose) puts("STUB: QuartzFilterOptionsInitializeWithOptionsInfo called");
    return NULL;
}

void *QuartzFilterOptionsInitializeWithOutputIntents(void) {
    if (verbose) puts("STUB: QuartzFilterOptionsInitializeWithOutputIntents called");
    return NULL;
}

void *QuartzFilterOptionsInitializeWithPDFDocument(void) {
    if (verbose) puts("STUB: QuartzFilterOptionsInitializeWithPDFDocument called");
    return NULL;
}

void *QuartzFilterOptionsNewInfo(void) {
    if (verbose) puts("STUB: QuartzFilterOptionsNewInfo called");
    return NULL;
}

void *QuartzFilterOptionsNewInfoWithClassProfiles(void) {
    if (verbose) puts("STUB: QuartzFilterOptionsNewInfoWithClassProfiles called");
    return NULL;
}

void *QuartzFilterOptionsRelease(void) {
    if (verbose) puts("STUB: QuartzFilterOptionsRelease called");
    return NULL;
}

void *QuartzFilterOptionsReleaseInfo(void) {
    if (verbose) puts("STUB: QuartzFilterOptionsReleaseInfo called");
    return NULL;
}

void *QuartzFilterOptionsUpdate(void) {
    if (verbose) puts("STUB: QuartzFilterOptionsUpdate called");
    return NULL;
}

void *QuartzFilterRelease(void) {
    if (verbose) puts("STUB: QuartzFilterRelease called");
    return NULL;
}

void *QuartzFilterRemoveFromContext(void) {
    if (verbose) puts("STUB: QuartzFilterRemoveFromContext called");
    return NULL;
}

void *UpdateQFilterInfo(void) {
    if (verbose) puts("STUB: UpdateQFilterInfo called");
    return NULL;
}

void *filterFileIsInPDFServices(void) {
    if (verbose) puts("STUB: filterFileIsInPDFServices called");
    return NULL;
}

void *filterFileIsReadWrite(void) {
    if (verbose) puts("STUB: filterFileIsReadWrite called");
    return NULL;
}

void *localized(void) {
    if (verbose) puts("STUB: localized called");
    return NULL;
}

