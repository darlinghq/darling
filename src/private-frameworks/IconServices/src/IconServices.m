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


#include <IconServices/IconServices.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;
__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}


extern void* const _kISAttachedResourceInfo = (void*)0;
extern void* _kISFolderCustomIconFileName = (void*)0;
extern void* _kISIconsInvalidatedDarwinNotification = (void*)0;
extern void* _kISIconsInvalidatedNotification = (void*)0;
extern void* const _kISImageRequestTimeoutForeverKey = (void*)0;
extern void* const kISBackgroundResourceKey = (void*)0;
extern void* const kISBadgeResourceKey = (void*)0;
extern void* const kISIcnsDarkVariantName = (void*)0;
extern void* const kISIcnsErrorDomain = (void*)0;
extern void* const kISIcnsPrimaryVariantName = (void*)0;
extern void* const kISIcnsSelectedVariantName = (void*)0;
extern void* const kISIcnsTemplateVariantName = (void*)0;
extern void* const kISIconDecorationAliasType = (void*)0;
extern void* const kISIconDecorationBadgeType = (void*)0;
extern void* const kISIconDecorationCautionAlertType = (void*)0;
extern void* const kISIconDecorationDropFolderBadgeType = (void*)0;
extern void* const kISIconDecorationEmbossType = (void*)0;
extern void* const kISIconDecorationFolderType = (void*)0;
extern void* const kISIconDecorationLockedBadgeType = (void*)0;
extern void* const kISIconDecorationNewFolderType = (void*)0;
extern void* const kISIconDecorationPositionType = (void*)0;
extern void* const kISIconDecorationPositionTypeCenter = (void*)0;
extern void* const kISIconDecorationPositionTypeLeadingBottom = (void*)0;
extern void* const kISIconDecorationPositionTypeTrailingBottom = (void*)0;
extern void* const kISIconDecorationPrivateFolderBadgeType = (void*)0;
extern void* const kISIconDecorationReadOnlyFolderBadgeType = (void*)0;
extern void* const kISIconDecorationSystemType = (void*)0;
extern void* const kISIconDecorationType = (void*)0;
extern void* const kISIconDecorationUnsupportedBadgeType = (void*)0;
extern void* const kISImageDescriptorActivity = (void*)0;
extern void* const kISImageDescriptorBadgedDocumentScalableSize = (void*)0;
extern void* const kISImageDescriptorBadgedDocumentSize = (void*)0;
extern void* const kISImageDescriptorCarLauncher = (void*)0;
extern void* const kISImageDescriptorCarNotification = (void*)0;
extern void* const kISImageDescriptorCustomDocumentCompact = (void*)0;
extern void* const kISImageDescriptorCustomDocumentRegular = (void*)0;
extern void* const kISImageDescriptorCustomDocumentScalableSize = (void*)0;
extern void* const kISImageDescriptorCustomDocumentSize = (void*)0;
extern void* const kISImageDescriptorCustomDocumentTable = (void*)0;
extern void* const kISImageDescriptorHomeScreen = (void*)0;
extern void* const kISImageDescriptorLargeHomeScreen = (void*)0;
extern void* const kISImageDescriptorMessagesExtensionBadge = (void*)0;
extern void* const kISImageDescriptorMessagesExtensionLauncher = (void*)0;
extern void* const kISImageDescriptorMessagesExtensionStatus = (void*)0;
extern void* const kISImageDescriptorNotification = (void*)0;
extern void* const kISImageDescriptorSpotlight = (void*)0;
extern void* const kISImageDescriptorTableUIName = (void*)0;
extern void* const kISPrimaryResourceKey = (void*)0;
extern void* const kISTextResourceKey = (void*)0;
extern void* const kISTintColorResourceKey = (void*)0;
extern void* kIconCompilerImageInfoIsSelectedVariantKey = (void*)0;
extern void* kIconCompilerImageInfoNameKey = (void*)0;
extern void* kIconCompilerImageInfoScaleKey = (void*)0;
extern void* kIconCompilerImageInfoSizeKey = (void*)0;

void *ISCreateCGImageFromBindingWithSizeScaleAndOptions(void) {
    if (verbose) puts("STUB: ISCreateCGImageFromBindingWithSizeScaleAndOptions called");
    return NULL;
}

void *ISIsResourceKey(void) {
    if (verbose) puts("STUB: ISIsResourceKey called");
    return NULL;
}

void *_ISCopyCacheContentsDescription(void) {
    if (verbose) puts("STUB: _ISCopyCacheContentsDescription called");
    return NULL;
}

void *_ISCopyCachePath(void) {
    if (verbose) puts("STUB: _ISCopyCachePath called");
    return NULL;
}

void *_ISCopyICNSDataFromBinding(void) {
    if (verbose) puts("STUB: _ISCopyICNSDataFromBinding called");
    return NULL;
}

void *_ISCreateCGImageForTypeAtScaleInternal(void) {
    if (verbose) puts("STUB: _ISCreateCGImageForTypeAtScaleInternal called");
    return NULL;
}

void *_ISCreateCGImageFromBindingWithSizeScaleAndOptions(void) {
    if (verbose) puts("STUB: _ISCreateCGImageFromBindingWithSizeScaleAndOptions called");
    return NULL;
}

void *_ISCreateCGImageFromBindingWithSizeScaleAndOptionsAsync(void) {
    if (verbose) puts("STUB: _ISCreateCGImageFromBindingWithSizeScaleAndOptionsAsync called");
    return NULL;
}

void *_ISDefaultLog(void) {
    if (verbose) puts("STUB: _ISDefaultLog called");
    return NULL;
}

void *_ISGDBIconsCGCacheList(void) {
    if (verbose) puts("STUB: _ISGDBIconsCGCacheList called");
    return NULL;
}

void *_ISGenerateCacheImagesFromBindingWithSizeRangeScaleAndOptions(void) {
    if (verbose) puts("STUB: _ISGenerateCacheImagesFromBindingWithSizeRangeScaleAndOptions called");
    return NULL;
}

void *_ISGetGlobalIconImagesCacheMaxEntriesAndMaxDataSize(void) {
    if (verbose) puts("STUB: _ISGetGlobalIconImagesCacheMaxEntriesAndMaxDataSize called");
    return NULL;
}

void *_ISGetIcnsIndex(void) {
    if (verbose) puts("STUB: _ISGetIcnsIndex called");
    return NULL;
}

void *_ISGetIconFamilyData(void) {
    if (verbose) puts("STUB: _ISGetIconFamilyData called");
    return NULL;
}

void *_ISGetIconRefVariant(void) {
    if (verbose) puts("STUB: _ISGetIconRefVariant called");
    return NULL;
}

void *_ISIcnsIndexGetInfo(void) {
    if (verbose) puts("STUB: _ISIcnsIndexGetInfo called");
    return NULL;
}

void *_ISIcnsIndexNext(void) {
    if (verbose) puts("STUB: _ISIcnsIndexNext called");
    return NULL;
}

void *_ISIcnsIndexPrev(void) {
    if (verbose) puts("STUB: _ISIcnsIndexPrev called");
    return NULL;
}

void *_ISIconDataForResourceProxy(void) {
    if (verbose) puts("STUB: _ISIconDataForResourceProxy called");
    return NULL;
}

void *_ISIconRefContainsCGPoint(void) {
    if (verbose) puts("STUB: _ISIconRefContainsCGPoint called");
    return NULL;
}

void *_ISIconRefIntersectsCGRect(void) {
    if (verbose) puts("STUB: _ISIconRefIntersectsCGRect called");
    return NULL;
}

void *_ISIconRefToHIShape(void) {
    if (verbose) puts("STUB: _ISIconRefToHIShape called");
    return NULL;
}

void *_ISIconRefToIconFamily(void) {
    if (verbose) puts("STUB: _ISIconRefToIconFamily called");
    return NULL;
}

void *_ISIconServicesGetCGImageRefFromIconRef(void) {
    if (verbose) puts("STUB: _ISIconServicesGetCGImageRefFromIconRef called");
    return NULL;
}

void *_ISIconServicesGetCGImageRefFromURL(void) {
    if (verbose) puts("STUB: _ISIconServicesGetCGImageRefFromURL called");
    return NULL;
}

void *_ISIndexIsValid(void) {
    if (verbose) puts("STUB: _ISIndexIsValid called");
    return NULL;
}

void *_ISInvalidateCacheEntriesForBundleIdentifier(void) {
    if (verbose) puts("STUB: _ISInvalidateCacheEntriesForBundleIdentifier called");
    return NULL;
}

void *_ISIsIconRefMaskEmpty(void) {
    if (verbose) puts("STUB: _ISIsIconRefMaskEmpty called");
    return NULL;
}

void *_ISPlotIconRefInContext(void) {
    if (verbose) puts("STUB: _ISPlotIconRefInContext called");
    return NULL;
}

void *_ISPlotInContext(void) {
    if (verbose) puts("STUB: _ISPlotInContext called");
    return NULL;
}

void *_ISPrepareISIconSignpostLog(void) {
    if (verbose) puts("STUB: _ISPrepareISIconSignpostLog called");
    return NULL;
}

void *_ISPrimaryIconDataForApplicationProxy(void) {
    if (verbose) puts("STUB: _ISPrimaryIconDataForApplicationProxy called");
    return NULL;
}

void *_ISProcessIsRoot(void) {
    if (verbose) puts("STUB: _ISProcessIsRoot called");
    return NULL;
}

void *_ISSetGlobalIconImagesCacheMaxEntriesAndMaxDataSize(void) {
    if (verbose) puts("STUB: _ISSetGlobalIconImagesCacheMaxEntriesAndMaxDataSize called");
    return NULL;
}

void *_ISSetIconFamilyData(void) {
    if (verbose) puts("STUB: _ISSetIconFamilyData called");
    return NULL;
}

void *_ISTraceLog(void) {
    if (verbose) puts("STUB: _ISTraceLog called");
    return NULL;
}

void *_ISURLCacheLog(void) {
    if (verbose) puts("STUB: _ISURLCacheLog called");
    return NULL;
}

void *getFocusRingTint(void) {
    if (verbose) puts("STUB: getFocusRingTint called");
    return NULL;
}

