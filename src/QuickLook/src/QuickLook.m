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


#include <QuickLook/QuickLook.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* QLAdaptContextCTMToFitRectInRect(void) {
    if (verbose) puts("STUB: QLAdaptContextCTMToFitRectInRect called");
    return NULL;
}

void* QLAdaptContextCTMToScaleRectToRect(void) {
    if (verbose) puts("STUB: QLAdaptContextCTMToScaleRectToRect called");
    return NULL;
}

void* QLAdaptSizeInSize(void) {
    if (verbose) puts("STUB: QLAdaptSizeInSize called");
    return NULL;
}

void* QLAdaptSizeOutSize(void) {
    if (verbose) puts("STUB: QLAdaptSizeOutSize called");
    return NULL;
}

void* QLAdaptSizeToRect(void) {
    if (verbose) puts("STUB: QLAdaptSizeToRect called");
    return NULL;
}

void* QLAdaptSizeToRectWithRounding(void) {
    if (verbose) puts("STUB: QLAdaptSizeToRectWithRounding called");
    return NULL;
}

void* QLAdjustRectInRect(void) {
    if (verbose) puts("STUB: QLAdjustRectInRect called");
    return NULL;
}

void* QLBitmapContextLengthWithSize(void) {
    if (verbose) puts("STUB: QLBitmapContextLengthWithSize called");
    return NULL;
}

void* QLBitmapGetPixelSizeForPointSize(void) {
    if (verbose) puts("STUB: QLBitmapGetPixelSizeForPointSize called");
    return NULL;
}

void* QLCacheObserverCancel(void) {
    if (verbose) puts("STUB: QLCacheObserverCancel called");
    return NULL;
}

void* QLCacheObserverCreate(void) {
    if (verbose) puts("STUB: QLCacheObserverCreate called");
    return NULL;
}

void* QLCacheObserverGetTypeID(void) {
    if (verbose) puts("STUB: QLCacheObserverGetTypeID called");
    return NULL;
}

void* QLCacheObserverResume(void) {
    if (verbose) puts("STUB: QLCacheObserverResume called");
    return NULL;
}

void* QLCacheObserverSetInvalidationBlock(void) {
    if (verbose) puts("STUB: QLCacheObserverSetInvalidationBlock called");
    return NULL;
}

void* QLCacheObserverSuspend(void) {
    if (verbose) puts("STUB: QLCacheObserverSuspend called");
    return NULL;
}

void* QLCachedDrawingCreate(void) {
    if (verbose) puts("STUB: QLCachedDrawingCreate called");
    return NULL;
}

void* QLCachedDrawingDispose(void) {
    if (verbose) puts("STUB: QLCachedDrawingDispose called");
    return NULL;
}

void* QLCachedDrawingDraw(void) {
    if (verbose) puts("STUB: QLCachedDrawingDraw called");
    return NULL;
}

void* QLCheckAccessForURL(void) {
    if (verbose) puts("STUB: QLCheckAccessForURL called");
    return NULL;
}

void* QLCheckAppleSignature(void) {
    if (verbose) puts("STUB: QLCheckAppleSignature called");
    return NULL;
}

void* QLCompareVersion(void) {
    if (verbose) puts("STUB: QLCompareVersion called");
    return NULL;
}

void* QLContextCreateGrayColorSpace(void) {
    if (verbose) puts("STUB: QLContextCreateGrayColorSpace called");
    return NULL;
}

void* QLContextCreateRGBColorSpace(void) {
    if (verbose) puts("STUB: QLContextCreateRGBColorSpace called");
    return NULL;
}

void* QLContextSetGrayFillColor(void) {
    if (verbose) puts("STUB: QLContextSetGrayFillColor called");
    return NULL;
}

void* QLContextSetGrayStrokeColor(void) {
    if (verbose) puts("STUB: QLContextSetGrayStrokeColor called");
    return NULL;
}

void* QLContextSetRGBFillColor(void) {
    if (verbose) puts("STUB: QLContextSetRGBFillColor called");
    return NULL;
}

void* QLContextSetRGBStrokeColor(void) {
    if (verbose) puts("STUB: QLContextSetRGBStrokeColor called");
    return NULL;
}

void* QLCopyIconImageFromURL(void) {
    if (verbose) puts("STUB: QLCopyIconImageFromURL called");
    return NULL;
}

void* QLCopyURLQueryDictionary(void) {
    if (verbose) puts("STUB: QLCopyURLQueryDictionary called");
    return NULL;
}

void* QLCreateImageWithBitmapContext(void) {
    if (verbose) puts("STUB: QLCreateImageWithBitmapContext called");
    return NULL;
}

void* QLCreateImageWithBlock(void) {
    if (verbose) puts("STUB: QLCreateImageWithBlock called");
    return NULL;
}

void* QLCreateImageWithMallocedBitmapContextTransferData(void) {
    if (verbose) puts("STUB: QLCreateImageWithMallocedBitmapContextTransferData called");
    return NULL;
}

void* QLCreateMallocedBitmapContextWithSize(void) {
    if (verbose) puts("STUB: QLCreateMallocedBitmapContextWithSize called");
    return NULL;
}

void* QLCreateMallocedBitmapContextWithSizeSupportingSubPixelAntialiasing(void) {
    if (verbose) puts("STUB: QLCreateMallocedBitmapContextWithSizeSupportingSubPixelAntialiasing called");
    return NULL;
}

void* QLCreateMallocedGrayscaleContextWithSize(void) {
    if (verbose) puts("STUB: QLCreateMallocedGrayscaleContextWithSize called");
    return NULL;
}

void* QLCreateReservedBitmapContextWithSize(void) {
    if (verbose) puts("STUB: QLCreateReservedBitmapContextWithSize called");
    return NULL;
}

void* QLCreateURLByResolvingAlias(void) {
    if (verbose) puts("STUB: QLCreateURLByResolvingAlias called");
    return NULL;
}

void* QLCreateURLByResolvingSymlink(void) {
    if (verbose) puts("STUB: QLCreateURLByResolvingSymlink called");
    return NULL;
}

void* QLCreateVMBitmapContextWithSize(void) {
    if (verbose) puts("STUB: QLCreateVMBitmapContextWithSize called");
    return NULL;
}

void* QLDeletePaths(void) {
    if (verbose) puts("STUB: QLDeletePaths called");
    return NULL;
}

void* QLDiskStoreAcceptsThumbnails(void) {
    if (verbose) puts("STUB: QLDiskStoreAcceptsThumbnails called");
    return NULL;
}

void* QLDiskStoreCopyDiskStoreForURL(void) {
    if (verbose) puts("STUB: QLDiskStoreCopyDiskStoreForURL called");
    return NULL;
}

void* QLDiskStoreCopyDiskStores(void) {
    if (verbose) puts("STUB: QLDiskStoreCopyDiskStores called");
    return NULL;
}

void* QLDiskStoreExecuteBlock(void) {
    if (verbose) puts("STUB: QLDiskStoreExecuteBlock called");
    return NULL;
}

void* QLDiskStoreGetAvailableDiskSpace(void) {
    if (verbose) puts("STUB: QLDiskStoreGetAvailableDiskSpace called");
    return NULL;
}

void* QLDiskStoreGetMountPath(void) {
    if (verbose) puts("STUB: QLDiskStoreGetMountPath called");
    return NULL;
}

void* QLDiskStoreGetTypeID(void) {
    if (verbose) puts("STUB: QLDiskStoreGetTypeID called");
    return NULL;
}

void* QLDiskStoreHasThumbnailCache(void) {
    if (verbose) puts("STUB: QLDiskStoreHasThumbnailCache called");
    return NULL;
}

void* QLDiskStoreIsDistant(void) {
    if (verbose) puts("STUB: QLDiskStoreIsDistant called");
    return NULL;
}

void* QLDiskStoreIsEncrypted(void) {
    if (verbose) puts("STUB: QLDiskStoreIsEncrypted called");
    return NULL;
}

void* QLDrawCGPDFPage(void) {
    if (verbose) puts("STUB: QLDrawCGPDFPage called");
    return NULL;
}

void* QLDrawCGPDFPageInRect(void) {
    if (verbose) puts("STUB: QLDrawCGPDFPageInRect called");
    return NULL;
}

void* QLDrawDebugRect(void) {
    if (verbose) puts("STUB: QLDrawDebugRect called");
    return NULL;
}

void* QLDrawGrayLinearGradient(void) {
    if (verbose) puts("STUB: QLDrawGrayLinearGradient called");
    return NULL;
}

void* QLDrawIconImageForURL(void) {
    if (verbose) puts("STUB: QLDrawIconImageForURL called");
    return NULL;
}

void* QLDrawLinearGradient(void) {
    if (verbose) puts("STUB: QLDrawLinearGradient called");
    return NULL;
}

void* QLDrawPDFPage(void) {
    if (verbose) puts("STUB: QLDrawPDFPage called");
    return NULL;
}

void* QLDrawPDFPageInRect(void) {
    if (verbose) puts("STUB: QLDrawPDFPageInRect called");
    return NULL;
}

void* QLError(void) {
    if (verbose) puts("STUB: QLError called");
    return NULL;
}

void* QLGeneratorDispatchBlock(void) {
    if (verbose) puts("STUB: QLGeneratorDispatchBlock called");
    return NULL;
}

void* QLGeneratorGetTypeID(void) {
    if (verbose) puts("STUB: QLGeneratorGetTypeID called");
    return NULL;
}

void* QLGetCGPDFDrawRect(void) {
    if (verbose) puts("STUB: QLGetCGPDFDrawRect called");
    return NULL;
}

void* QLGetCGPDFDrawingTransform(void) {
    if (verbose) puts("STUB: QLGetCGPDFDrawingTransform called");
    return NULL;
}

void* QLGetCGPDFPageRect(void) {
    if (verbose) puts("STUB: QLGetCGPDFPageRect called");
    return NULL;
}

void* QLGetPDFDrawRect(void) {
    if (verbose) puts("STUB: QLGetPDFDrawRect called");
    return NULL;
}

void* QLGetPDFDrawingTransform(void) {
    if (verbose) puts("STUB: QLGetPDFDrawingTransform called");
    return NULL;
}

void* QLGetPDFPageRect(void) {
    if (verbose) puts("STUB: QLGetPDFPageRect called");
    return NULL;
}

void* QLGetRealPath(void) {
    if (verbose) puts("STUB: QLGetRealPath called");
    return NULL;
}

void* QLIconContextFinish(void) {
    if (verbose) puts("STUB: QLIconContextFinish called");
    return NULL;
}

void* QLIconContextNextPage(void) {
    if (verbose) puts("STUB: QLIconContextNextPage called");
    return NULL;
}

void* QLIconContextStart(void) {
    if (verbose) puts("STUB: QLIconContextStart called");
    return NULL;
}

void* QLIconCreateBackAndFrontLayerImages(void) {
    if (verbose) puts("STUB: QLIconCreateBackAndFrontLayerImages called");
    return NULL;
}

void* QLIconCreateMaskImage(void) {
    if (verbose) puts("STUB: QLIconCreateMaskImage called");
    return NULL;
}

void* QLIconCreateMaskImageNoFlavor(void) {
    if (verbose) puts("STUB: QLIconCreateMaskImageNoFlavor called");
    return NULL;
}

void* QLIconCreateWithImage(void) {
    if (verbose) puts("STUB: QLIconCreateWithImage called");
    return NULL;
}

void* QLIconCreateWithImageAndOptions(void) {
    if (verbose) puts("STUB: QLIconCreateWithImageAndOptions called");
    return NULL;
}

void* QLIconDrawWithOptions(void) {
    if (verbose) puts("STUB: QLIconDrawWithOptions called");
    return NULL;
}

void* QLIconGetMaxImageSize(void) {
    if (verbose) puts("STUB: QLIconGetMaxImageSize called");
    return NULL;
}

void* QLIconSetCopyImageURLForSize(void) {
    if (verbose) puts("STUB: QLIconSetCopyImageURLForSize called");
    return NULL;
}

void* QLIconSetIsValid(void) {
    if (verbose) puts("STUB: QLIconSetIsValid called");
    return NULL;
}

void* QLImageCopySampleColors(void) {
    if (verbose) puts("STUB: QLImageCopySampleColors called");
    return NULL;
}

void* QLImageIOCleanupContext(void) {
    if (verbose) puts("STUB: QLImageIOCleanupContext called");
    return NULL;
}

void* QLImageIOCreateThumbnailFromData(void) {
    if (verbose) puts("STUB: QLImageIOCreateThumbnailFromData called");
    return NULL;
}

void* QLImageIOCreateThumbnailFromURL(void) {
    if (verbose) puts("STUB: QLImageIOCreateThumbnailFromURL called");
    return NULL;
}

void* QLImageIOCreateThumbnailWithImageSource(void) {
    if (verbose) puts("STUB: QLImageIOCreateThumbnailWithImageSource called");
    return NULL;
}

void* QLImageIODrawImageInContext(void) {
    if (verbose) puts("STUB: QLImageIODrawImageInContext called");
    return NULL;
}

void* QLImageIOFillContextForData(void) {
    if (verbose) puts("STUB: QLImageIOFillContextForData called");
    return NULL;
}

void* QLImageIOFillContextForURL(void) {
    if (verbose) puts("STUB: QLImageIOFillContextForURL called");
    return NULL;
}

void* QLImageIOGetAffineTransformFromCGImage(void) {
    if (verbose) puts("STUB: QLImageIOGetAffineTransformFromCGImage called");
    return NULL;
}

void* QLInitLogging(void) {
    if (verbose) puts("STUB: QLInitLogging called");
    return NULL;
}

void* QLIsDiskCacheEnabled(void) {
    if (verbose) puts("STUB: QLIsDiskCacheEnabled called");
    return NULL;
}

void* QLIsFileURL(void) {
    if (verbose) puts("STUB: QLIsFileURL called");
    return NULL;
}

void* QLLoadImageAtPath(void) {
    if (verbose) puts("STUB: QLLoadImageAtPath called");
    return NULL;
}

void* QLLoadImageFromBundle(void) {
    if (verbose) puts("STUB: QLLoadImageFromBundle called");
    return NULL;
}

void* QLLoadImageFromBundleWithID(void) {
    if (verbose) puts("STUB: QLLoadImageFromBundleWithID called");
    return NULL;
}

void* QLLoadQuickLookUISymbol(void) {
    if (verbose) puts("STUB: QLLoadQuickLookUISymbol called");
    return NULL;
}

void* QLLogSetLoggingFunction(void) {
    if (verbose) puts("STUB: QLLogSetLoggingFunction called");
    return NULL;
}

void* QLMiniCacheGetSizeInBytes(void) {
    if (verbose) puts("STUB: QLMiniCacheGetSizeInBytes called");
    return NULL;
}

void* QLMiniCachePurgeEntriesBySize(void) {
    if (verbose) puts("STUB: QLMiniCachePurgeEntriesBySize called");
    return NULL;
}

void* QLMiniCachePurgeLeastRecentlyUsedEntries(void) {
    if (verbose) puts("STUB: QLMiniCachePurgeLeastRecentlyUsedEntries called");
    return NULL;
}

void* QLMiniCachePurgeURL(void) {
    if (verbose) puts("STUB: QLMiniCachePurgeURL called");
    return NULL;
}

void* QLMiniCachePurgeURLs(void) {
    if (verbose) puts("STUB: QLMiniCachePurgeURLs called");
    return NULL;
}

void* QLMiniCacheRegisterCache(void) {
    if (verbose) puts("STUB: QLMiniCacheRegisterCache called");
    return NULL;
}

void* QLMiniCacheUnregisterCache(void) {
    if (verbose) puts("STUB: QLMiniCacheUnregisterCache called");
    return NULL;
}

void* QLPreviewBubbleCopyDocumentURL(void) {
    if (verbose) puts("STUB: QLPreviewBubbleCopyDocumentURL called");
    return NULL;
}

void* QLPreviewBubbleCopyLaunchURL(void) {
    if (verbose) puts("STUB: QLPreviewBubbleCopyLaunchURL called");
    return NULL;
}

void* QLPreviewBubbleCopySeamlessOpeningInformation(void) {
    if (verbose) puts("STUB: QLPreviewBubbleCopySeamlessOpeningInformation called");
    return NULL;
}

void* QLPreviewBubbleCreate(void) {
    if (verbose) puts("STUB: QLPreviewBubbleCreate called");
    return NULL;
}

void* QLPreviewBubbleGetTypeID(void) {
    if (verbose) puts("STUB: QLPreviewBubbleGetTypeID called");
    return NULL;
}

void* QLPreviewBubbleGetUsedEdge(void) {
    if (verbose) puts("STUB: QLPreviewBubbleGetUsedEdge called");
    return NULL;
}

void* QLPreviewBubbleGetWindowID(void) {
    if (verbose) puts("STUB: QLPreviewBubbleGetWindowID called");
    return NULL;
}

void* QLPreviewBubbleHide(void) {
    if (verbose) puts("STUB: QLPreviewBubbleHide called");
    return NULL;
}

void* QLPreviewBubbleInvalidate(void) {
    if (verbose) puts("STUB: QLPreviewBubbleInvalidate called");
    return NULL;
}

void* QLPreviewBubbleInvertShowBubbleEffect(void) {
    if (verbose) puts("STUB: QLPreviewBubbleInvertShowBubbleEffect called");
    return NULL;
}

void* QLPreviewBubbleInvokeShowBubbleEffect(void) {
    if (verbose) puts("STUB: QLPreviewBubbleInvokeShowBubbleEffect called");
    return NULL;
}

void* QLPreviewBubbleIsVisible(void) {
    if (verbose) puts("STUB: QLPreviewBubbleIsVisible called");
    return NULL;
}

void* QLPreviewBubbleSetAnimationProgress(void) {
    if (verbose) puts("STUB: QLPreviewBubbleSetAnimationProgress called");
    return NULL;
}

void* QLPreviewBubbleSetButtonClickHandler(void) {
    if (verbose) puts("STUB: QLPreviewBubbleSetButtonClickHandler called");
    return NULL;
}

void* QLPreviewBubbleSetDispatchQueue(void) {
    if (verbose) puts("STUB: QLPreviewBubbleSetDispatchQueue called");
    return NULL;
}

void* QLPreviewBubbleSetDisplayPathAndAdditionalString(void) {
    if (verbose) puts("STUB: QLPreviewBubbleSetDisplayPathAndAdditionalString called");
    return NULL;
}

void* QLPreviewBubbleSetDoubleClickHandler(void) {
    if (verbose) puts("STUB: QLPreviewBubbleSetDoubleClickHandler called");
    return NULL;
}

void* QLPreviewBubbleSetEventCallbacks(void) {
    if (verbose) puts("STUB: QLPreviewBubbleSetEventCallbacks called");
    return NULL;
}

void* QLPreviewBubbleSetLoadedHandler(void) {
    if (verbose) puts("STUB: QLPreviewBubbleSetLoadedHandler called");
    return NULL;
}

void* QLPreviewBubbleSetWillOpenURLHandler(void) {
    if (verbose) puts("STUB: QLPreviewBubbleSetWillOpenURLHandler called");
    return NULL;
}

void* QLPreviewBubbleShow(void) {
    if (verbose) puts("STUB: QLPreviewBubbleShow called");
    return NULL;
}

void* QLPreviewCancel(void) {
    if (verbose) puts("STUB: QLPreviewCancel called");
    return NULL;
}

void* QLPreviewClose(void) {
    if (verbose) puts("STUB: QLPreviewClose called");
    return NULL;
}

void* QLPreviewCopyBitmapImage(void) {
    if (verbose) puts("STUB: QLPreviewCopyBitmapImage called");
    return NULL;
}

void* QLPreviewCopyData(void) {
    if (verbose) puts("STUB: QLPreviewCopyData called");
    return NULL;
}

void* QLPreviewCopyDictionaryRepresentation(void) {
    if (verbose) puts("STUB: QLPreviewCopyDictionaryRepresentation called");
    return NULL;
}

void* QLPreviewCopyDocumentURL(void) {
    if (verbose) puts("STUB: QLPreviewCopyDocumentURL called");
    return NULL;
}

void* QLPreviewCopyNextData(void) {
    if (verbose) puts("STUB: QLPreviewCopyNextData called");
    return NULL;
}

void* QLPreviewCopyOptions(void) {
    if (verbose) puts("STUB: QLPreviewCopyOptions called");
    return NULL;
}

void* QLPreviewCopyPreviewContentType(void) {
    if (verbose) puts("STUB: QLPreviewCopyPreviewContentType called");
    return NULL;
}

void* QLPreviewCopyProperties(void) {
    if (verbose) puts("STUB: QLPreviewCopyProperties called");
    return NULL;
}

void* QLPreviewCopyURLRepresentation(void) {
    if (verbose) puts("STUB: QLPreviewCopyURLRepresentation called");
    return NULL;
}

void* QLPreviewCreate(void) {
    if (verbose) puts("STUB: QLPreviewCreate called");
    return NULL;
}

void* QLPreviewCreateFromSearchItem(void) {
    if (verbose) puts("STUB: QLPreviewCreateFromSearchItem called");
    return NULL;
}

void* QLPreviewCreateUniqueAttachmentURL(void) {
    if (verbose) puts("STUB: QLPreviewCreateUniqueAttachmentURL called");
    return NULL;
}

void* QLPreviewDiscardAllPendingActions(void) {
    if (verbose) puts("STUB: QLPreviewDiscardAllPendingActions called");
    return NULL;
}

void* QLPreviewDocumentComputePreview(void) {
    if (verbose) puts("STUB: QLPreviewDocumentComputePreview called");
    return NULL;
}

void* QLPreviewDocumentCopyContentType(void) {
    if (verbose) puts("STUB: QLPreviewDocumentCopyContentType called");
    return NULL;
}

void* QLPreviewDocumentCopyData(void) {
    if (verbose) puts("STUB: QLPreviewDocumentCopyData called");
    return NULL;
}

void* QLPreviewDocumentCopyPageAsImage(void) {
    if (verbose) puts("STUB: QLPreviewDocumentCopyPageAsImage called");
    return NULL;
}

void* QLPreviewDocumentCopyPageText(void) {
    if (verbose) puts("STUB: QLPreviewDocumentCopyPageText called");
    return NULL;
}

void* QLPreviewDocumentCopyProperties(void) {
    if (verbose) puts("STUB: QLPreviewDocumentCopyProperties called");
    return NULL;
}

void* QLPreviewDocumentCopySuggestedFilename(void) {
    if (verbose) puts("STUB: QLPreviewDocumentCopySuggestedFilename called");
    return NULL;
}

void* QLPreviewDocumentCopyURL(void) {
    if (verbose) puts("STUB: QLPreviewDocumentCopyURL called");
    return NULL;
}

void* QLPreviewDocumentCreateWithURL(void) {
    if (verbose) puts("STUB: QLPreviewDocumentCreateWithURL called");
    return NULL;
}

void* QLPreviewDocumentDrawPage(void) {
    if (verbose) puts("STUB: QLPreviewDocumentDrawPage called");
    return NULL;
}

void* QLPreviewDocumentGetPageCount(void) {
    if (verbose) puts("STUB: QLPreviewDocumentGetPageCount called");
    return NULL;
}

void* QLPreviewDocumentGetPageSize(void) {
    if (verbose) puts("STUB: QLPreviewDocumentGetPageSize called");
    return NULL;
}

void* QLPreviewDocumentGetTypeID(void) {
    if (verbose) puts("STUB: QLPreviewDocumentGetTypeID called");
    return NULL;
}

void* QLPreviewDocumentIsPaginated(void) {
    if (verbose) puts("STUB: QLPreviewDocumentIsPaginated called");
    return NULL;
}

void* QLPreviewDocumentSetServer(void) {
    if (verbose) puts("STUB: QLPreviewDocumentSetServer called");
    return NULL;
}

void* QLPreviewFreePreviewData(void) {
    if (verbose) puts("STUB: QLPreviewFreePreviewData called");
    return NULL;
}

void* QLPreviewGetDisplayBundleID(void) {
    if (verbose) puts("STUB: QLPreviewGetDisplayBundleID called");
    return NULL;
}

void* QLPreviewGetInlinePreviewSupportedContentTypes(void) {
    if (verbose) puts("STUB: QLPreviewGetInlinePreviewSupportedContentTypes called");
    return NULL;
}

void* QLPreviewGetLastError(void) {
    if (verbose) puts("STUB: QLPreviewGetLastError called");
    return NULL;
}

void* QLPreviewGetLoadingPlaceholder(void) {
    if (verbose) puts("STUB: QLPreviewGetLoadingPlaceholder called");
    return NULL;
}

void* QLPreviewGetPreviewSizeHint(void) {
    if (verbose) puts("STUB: QLPreviewGetPreviewSizeHint called");
    return NULL;
}

void* QLPreviewGetRepresentedObject(void) {
    if (verbose) puts("STUB: QLPreviewGetRepresentedObject called");
    return NULL;
}

void* QLPreviewGetTypeID(void) {
    if (verbose) puts("STUB: QLPreviewGetTypeID called");
    return NULL;
}

void* QLPreviewIsCancelled(void) {
    if (verbose) puts("STUB: QLPreviewIsCancelled called");
    return NULL;
}

void* QLPreviewIsGenericFolder(void) {
    if (verbose) puts("STUB: QLPreviewIsGenericFolder called");
    return NULL;
}

void* QLPreviewIsPreferringDarkLoading(void) {
    if (verbose) puts("STUB: QLPreviewIsPreferringDarkLoading called");
    return NULL;
}

void* QLPreviewIsProgressive(void) {
    if (verbose) puts("STUB: QLPreviewIsProgressive called");
    return NULL;
}

void* QLPreviewIsRawImage(void) {
    if (verbose) puts("STUB: QLPreviewIsRawImage called");
    return NULL;
}

void* QLPreviewIsWithinContentTypeLimit(void) {
    if (verbose) puts("STUB: QLPreviewIsWithinContentTypeLimit called");
    return NULL;
}

void* QLPreviewProcessProgressiveRendering(void) {
    if (verbose) puts("STUB: QLPreviewProcessProgressiveRendering called");
    return NULL;
}

void* QLPreviewRequestAppendDataToAttachment(void) {
    if (verbose) puts("STUB: QLPreviewRequestAppendDataToAttachment called");
    return NULL;
}

void* QLPreviewRequestCopyContentUTI(void) {
    if (verbose) puts("STUB: QLPreviewRequestCopyContentUTI called");
    return NULL;
}

void* QLPreviewRequestCopyOptions(void) {
    if (verbose) puts("STUB: QLPreviewRequestCopyOptions called");
    return NULL;
}

void* QLPreviewRequestCopyURL(void) {
    if (verbose) puts("STUB: QLPreviewRequestCopyURL called");
    return NULL;
}

void* QLPreviewRequestCreateAttachmentURL(void) {
    if (verbose) puts("STUB: QLPreviewRequestCreateAttachmentURL called");
    return NULL;
}

void* QLPreviewRequestCreateContext(void) {
    if (verbose) puts("STUB: QLPreviewRequestCreateContext called");
    return NULL;
}

void* QLPreviewRequestCreatePDFContext(void) {
    if (verbose) puts("STUB: QLPreviewRequestCreatePDFContext called");
    return NULL;
}

void* QLPreviewRequestFlushContext(void) {
    if (verbose) puts("STUB: QLPreviewRequestFlushContext called");
    return NULL;
}

void* QLPreviewRequestGetDocumentObject(void) {
    if (verbose) puts("STUB: QLPreviewRequestGetDocumentObject called");
    return NULL;
}

void* QLPreviewRequestGetGeneratorBundle(void) {
    if (verbose) puts("STUB: QLPreviewRequestGetGeneratorBundle called");
    return NULL;
}

void* QLPreviewRequestGetTypeID(void) {
    if (verbose) puts("STUB: QLPreviewRequestGetTypeID called");
    return NULL;
}

void* QLPreviewRequestIsCancelled(void) {
    if (verbose) puts("STUB: QLPreviewRequestIsCancelled called");
    return NULL;
}

void* QLPreviewRequestSetData(void) {
    if (verbose) puts("STUB: QLPreviewRequestSetData called");
    return NULL;
}

void* QLPreviewRequestSetDataRepresentation(void) {
    if (verbose) puts("STUB: QLPreviewRequestSetDataRepresentation called");
    return NULL;
}

void* QLPreviewRequestSetDocumentObject(void) {
    if (verbose) puts("STUB: QLPreviewRequestSetDocumentObject called");
    return NULL;
}

void* QLPreviewRequestSetPreviewType(void) {
    if (verbose) puts("STUB: QLPreviewRequestSetPreviewType called");
    return NULL;
}

void* QLPreviewRequestSetURLRepresentation(void) {
    if (verbose) puts("STUB: QLPreviewRequestSetURLRepresentation called");
    return NULL;
}

void* QLPreviewRequestSetWebContent(void) {
    if (verbose) puts("STUB: QLPreviewRequestSetWebContent called");
    return NULL;
}

void* QLPreviewRequestStartDataRepresentation(void) {
    if (verbose) puts("STUB: QLPreviewRequestStartDataRepresentation called");
    return NULL;
}

void* QLPreviewSetApplicationBundleIdentifier(void) {
    if (verbose) puts("STUB: QLPreviewSetApplicationBundleIdentifier called");
    return NULL;
}

void* QLPreviewSetContentTypeLimits(void) {
    if (verbose) puts("STUB: QLPreviewSetContentTypeLimits called");
    return NULL;
}

void* QLPreviewSetDisplayBundleID(void) {
    if (verbose) puts("STUB: QLPreviewSetDisplayBundleID called");
    return NULL;
}

void* QLPreviewSetExtensionPath(void) {
    if (verbose) puts("STUB: QLPreviewSetExtensionPath called");
    return NULL;
}

void* QLPreviewSetForceContentTypeUTI(void) {
    if (verbose) puts("STUB: QLPreviewSetForceContentTypeUTI called");
    return NULL;
}

void* QLPreviewSetPreviewURL(void) {
    if (verbose) puts("STUB: QLPreviewSetPreviewURL called");
    return NULL;
}

void* QLPreviewSetProperties(void) {
    if (verbose) puts("STUB: QLPreviewSetProperties called");
    return NULL;
}

void* QLPreviewSetRepresentedObject(void) {
    if (verbose) puts("STUB: QLPreviewSetRepresentedObject called");
    return NULL;
}

void* QLPreviewSetSearchableItemUniqueIdentifier(void) {
    if (verbose) puts("STUB: QLPreviewSetSearchableItemUniqueIdentifier called");
    return NULL;
}

void* QLPreviewSetServer(void) {
    if (verbose) puts("STUB: QLPreviewSetServer called");
    return NULL;
}

void* QLPreviewSupportsContentUTI(void) {
    if (verbose) puts("STUB: QLPreviewSupportsContentUTI called");
    return NULL;
}

void* QLPreviewSupportsFileAtURL(void) {
    if (verbose) puts("STUB: QLPreviewSupportsFileAtURL called");
    return NULL;
}

void* QLPreviewTypeConfigureGenerator(void) {
    if (verbose) puts("STUB: QLPreviewTypeConfigureGenerator called");
    return NULL;
}

void* QLPreviewTypeDisplayBundleWantsToBeResized(void) {
    if (verbose) puts("STUB: QLPreviewTypeDisplayBundleWantsToBeResized called");
    return NULL;
}

void* QLPreviewTypeGetDisplayBundleCount(void) {
    if (verbose) puts("STUB: QLPreviewTypeGetDisplayBundleCount called");
    return NULL;
}

void* QLPreviewTypeGetDisplayBundleIDForUTI(void) {
    if (verbose) puts("STUB: QLPreviewTypeGetDisplayBundleIDForUTI called");
    return NULL;
}

void* QLPreviewTypeGetDisplayBundleIDsAndNames(void) {
    if (verbose) puts("STUB: QLPreviewTypeGetDisplayBundleIDsAndNames called");
    return NULL;
}

void* QLPreviewTypeGetRawImageContentType(void) {
    if (verbose) puts("STUB: QLPreviewTypeGetRawImageContentType called");
    return NULL;
}

void* QLPreviewTypeGetRawImageDisplayBundleID(void) {
    if (verbose) puts("STUB: QLPreviewTypeGetRawImageDisplayBundleID called");
    return NULL;
}

void* QLPreviewTypeGetStaticDisplayBundleIDForUTI(void) {
    if (verbose) puts("STUB: QLPreviewTypeGetStaticDisplayBundleIDForUTI called");
    return NULL;
}

void* QLPreviewTypeIsDisplayBundleIDSafe(void) {
    if (verbose) puts("STUB: QLPreviewTypeIsDisplayBundleIDSafe called");
    return NULL;
}

void* QLPreviewTypeProcessThirdPartyGeneratorType(void) {
    if (verbose) puts("STUB: QLPreviewTypeProcessThirdPartyGeneratorType called");
    return NULL;
}

void* QLPreviewUpdateWithDictionaryRepresentation(void) {
    if (verbose) puts("STUB: QLPreviewUpdateWithDictionaryRepresentation called");
    return NULL;
}

void* QLPushCustomRoundedRect(void) {
    if (verbose) puts("STUB: QLPushCustomRoundedRect called");
    return NULL;
}

void* QLPushRoundedRect(void) {
    if (verbose) puts("STUB: QLPushRoundedRect called");
    return NULL;
}

void* QLRequestCheckAccess(void) {
    if (verbose) puts("STUB: QLRequestCheckAccess called");
    return NULL;
}

void* QLRequestCopyContentUTI(void) {
    if (verbose) puts("STUB: QLRequestCopyContentUTI called");
    return NULL;
}

void* QLRequestCopyURL(void) {
    if (verbose) puts("STUB: QLRequestCopyURL called");
    return NULL;
}

void* QLRequestDispatchBlock(void) {
    if (verbose) puts("STUB: QLRequestDispatchBlock called");
    return NULL;
}

void* QLRequestFillUUID(void) {
    if (verbose) puts("STUB: QLRequestFillUUID called");
    return NULL;
}

void* QLRequestGetGeneratorBundle(void) {
    if (verbose) puts("STUB: QLRequestGetGeneratorBundle called");
    return NULL;
}

void* QLRequestIsCancelled(void) {
    if (verbose) puts("STUB: QLRequestIsCancelled called");
    return NULL;
}

void* QLRequestIsThumbnailRequest(void) {
    if (verbose) puts("STUB: QLRequestIsThumbnailRequest called");
    return NULL;
}

void* QLRequestMarkAsCancelled(void) {
    if (verbose) puts("STUB: QLRequestMarkAsCancelled called");
    return NULL;
}

void* QLRequestReplyWithDiscardError(void) {
    if (verbose) puts("STUB: QLRequestReplyWithDiscardError called");
    return NULL;
}

void* QLRequestSessionCopyAndNotifyPortDeathWithPort(void) {
    if (verbose) puts("STUB: QLRequestSessionCopyAndNotifyPortDeathWithPort called");
    return NULL;
}

void* QLRequestSessionDisableInstantOff(void) {
    if (verbose) puts("STUB: QLRequestSessionDisableInstantOff called");
    return NULL;
}

void* QLRequestSessionDiscardAllSessions(void) {
    if (verbose) puts("STUB: QLRequestSessionDiscardAllSessions called");
    return NULL;
}

void* QLRequestSessionEnableInstantOff(void) {
    if (verbose) puts("STUB: QLRequestSessionEnableInstantOff called");
    return NULL;
}

void* QLRequestSessionGetReplyPort(void) {
    if (verbose) puts("STUB: QLRequestSessionGetReplyPort called");
    return NULL;
}

void* QLRequestSessionGetRequest(void) {
    if (verbose) puts("STUB: QLRequestSessionGetRequest called");
    return NULL;
}

void* QLRequestSessionGetSessionCount(void) {
    if (verbose) puts("STUB: QLRequestSessionGetSessionCount called");
    return NULL;
}

void* QLRequestSessionGetTypeID(void) {
    if (verbose) puts("STUB: QLRequestSessionGetTypeID called");
    return NULL;
}

void* QLRequestSessionIsInstantOffEnabled(void) {
    if (verbose) puts("STUB: QLRequestSessionIsInstantOffEnabled called");
    return NULL;
}

void* QLRequestSessionPerformAsync(void) {
    if (verbose) puts("STUB: QLRequestSessionPerformAsync called");
    return NULL;
}

void* QLRequestSessionPerformSync(void) {
    if (verbose) puts("STUB: QLRequestSessionPerformSync called");
    return NULL;
}

void* QLRequestSessionSetupCrashReportMessageForRequest(void) {
    if (verbose) puts("STUB: QLRequestSessionSetupCrashReportMessageForRequest called");
    return NULL;
}

void* QLRequestSessionSetupSessions(void) {
    if (verbose) puts("STUB: QLRequestSessionSetupSessions called");
    return NULL;
}

void* QLRequestSetDiscardError(void) {
    if (verbose) puts("STUB: QLRequestSetDiscardError called");
    return NULL;
}

void* QLRequestSetupSession(void) {
    if (verbose) puts("STUB: QLRequestSetupSession called");
    return NULL;
}

void* QLRequestUnsetupSession(void) {
    if (verbose) puts("STUB: QLRequestUnsetupSession called");
    return NULL;
}

void* QLSandboxReleaseHandle(void) {
    if (verbose) puts("STUB: QLSandboxReleaseHandle called");
    return NULL;
}

void* QLSandboxTokenConsume(void) {
    if (verbose) puts("STUB: QLSandboxTokenConsume called");
    return NULL;
}

void* QLSandboxTokenCreateFileRead(void) {
    if (verbose) puts("STUB: QLSandboxTokenCreateFileRead called");
    return NULL;
}

void* QLScaleRect(void) {
    if (verbose) puts("STUB: QLScaleRect called");
    return NULL;
}

void* QLSeamlessClosingRequestCreate(void) {
    if (verbose) puts("STUB: QLSeamlessClosingRequestCreate called");
    return NULL;
}

void* QLSeamlessClosingRequestDiscard(void) {
    if (verbose) puts("STUB: QLSeamlessClosingRequestDiscard called");
    return NULL;
}

void* QLSeamlessClosingRequestDiscardRequestsWithAssociatedObject(void) {
    if (verbose) puts("STUB: QLSeamlessClosingRequestDiscardRequestsWithAssociatedObject called");
    return NULL;
}

void* QLSeamlessClosingRequestGetAssociatedObject(void) {
    if (verbose) puts("STUB: QLSeamlessClosingRequestGetAssociatedObject called");
    return NULL;
}

void* QLSeamlessClosingRequestGetTypeID(void) {
    if (verbose) puts("STUB: QLSeamlessClosingRequestGetTypeID called");
    return NULL;
}

void* QLSeamlessClosingRequestRegister(void) {
    if (verbose) puts("STUB: QLSeamlessClosingRequestRegister called");
    return NULL;
}

void* QLSeamlessClosingRequestSetAssociatedObject(void) {
    if (verbose) puts("STUB: QLSeamlessClosingRequestSetAssociatedObject called");
    return NULL;
}

void* QLSeamlessClosingRequestSetDispatchQueue(void) {
    if (verbose) puts("STUB: QLSeamlessClosingRequestSetDispatchQueue called");
    return NULL;
}

void* QLSeamlessClosingRequestSetHandler(void) {
    if (verbose) puts("STUB: QLSeamlessClosingRequestSetHandler called");
    return NULL;
}

void* QLSeamlessOpeningRequestBindToPID(void) {
    if (verbose) puts("STUB: QLSeamlessOpeningRequestBindToPID called");
    return NULL;
}

void* QLSeamlessOpeningRequestCreate(void) {
    if (verbose) puts("STUB: QLSeamlessOpeningRequestCreate called");
    return NULL;
}

void* QLSeamlessOpeningRequestDiscard(void) {
    if (verbose) puts("STUB: QLSeamlessOpeningRequestDiscard called");
    return NULL;
}

void* QLSeamlessOpeningRequestGetClosingRequest(void) {
    if (verbose) puts("STUB: QLSeamlessOpeningRequestGetClosingRequest called");
    return NULL;
}

void* QLSeamlessOpeningRequestGetTypeID(void) {
    if (verbose) puts("STUB: QLSeamlessOpeningRequestGetTypeID called");
    return NULL;
}

void* QLSeamlessOpeningRequestOpenItem(void) {
    if (verbose) puts("STUB: QLSeamlessOpeningRequestOpenItem called");
    return NULL;
}

void* QLSeamlessOpeningRequestSetCompletionHandler(void) {
    if (verbose) puts("STUB: QLSeamlessOpeningRequestSetCompletionHandler called");
    return NULL;
}

void* QLSeamlessOpeningRequestSetDispatchQueue(void) {
    if (verbose) puts("STUB: QLSeamlessOpeningRequestSetDispatchQueue called");
    return NULL;
}

void* QLServerCheckIn(void) {
    if (verbose) puts("STUB: QLServerCheckIn called");
    return NULL;
}

void* QLServerCheckOut(void) {
    if (verbose) puts("STUB: QLServerCheckOut called");
    return NULL;
}

void* QLServerCheckUsage(void) {
    if (verbose) puts("STUB: QLServerCheckUsage called");
    return NULL;
}

void* QLServerCopyPortToServer(void) {
    if (verbose) puts("STUB: QLServerCopyPortToServer called");
    return NULL;
}

void* QLServerCopyServerWithName(void) {
    if (verbose) puts("STUB: QLServerCopyServerWithName called");
    return NULL;
}

void* QLServerCreate(void) {
    if (verbose) puts("STUB: QLServerCreate called");
    return NULL;
}

void* QLServerCreateHelperServer(void) {
    if (verbose) puts("STUB: QLServerCreateHelperServer called");
    return NULL;
}

void* QLServerCreateHelperServerWithOptions(void) {
    if (verbose) puts("STUB: QLServerCreateHelperServerWithOptions called");
    return NULL;
}

void* QLServerCreateServerToMachPort(void) {
    if (verbose) puts("STUB: QLServerCreateServerToMachPort called");
    return NULL;
}

void* QLServerCreateWithName(void) {
    if (verbose) puts("STUB: QLServerCreateWithName called");
    return NULL;
}

void* QLServerGetMemoryUsage(void) {
    if (verbose) puts("STUB: QLServerGetMemoryUsage called");
    return NULL;
}

void* QLServerGetNumberOfAvailableFDs(void) {
    if (verbose) puts("STUB: QLServerGetNumberOfAvailableFDs called");
    return NULL;
}

void* QLServerGetQuickLookClient(void) {
    if (verbose) puts("STUB: QLServerGetQuickLookClient called");
    return NULL;
}

void* QLServerGetTimeoutForRequest(void) {
    if (verbose) puts("STUB: QLServerGetTimeoutForRequest called");
    return NULL;
}

void* QLServerGetTypeID(void) {
    if (verbose) puts("STUB: QLServerGetTypeID called");
    return NULL;
}

void* QLServerResetCachedPort(void) {
    if (verbose) puts("STUB: QLServerResetCachedPort called");
    return NULL;
}

void* QLServerSetDefaultServer(void) {
    if (verbose) puts("STUB: QLServerSetDefaultServer called");
    return NULL;
}

void* QLServerSetup32bitServerWithName(void) {
    if (verbose) puts("STUB: QLServerSetup32bitServerWithName called");
    return NULL;
}

void* QLServerShutDownServer(void) {
    if (verbose) puts("STUB: QLServerShutDownServer called");
    return NULL;
}

void* QLThumbnailCancel(void) {
    if (verbose) puts("STUB: QLThumbnailCancel called");
    return NULL;
}

void* QLThumbnailComputeAsync(void) {
    if (verbose) puts("STUB: QLThumbnailComputeAsync called");
    return NULL;
}

void* QLThumbnailCopyCacheIDForURL(void) {
    if (verbose) puts("STUB: QLThumbnailCopyCacheIDForURL called");
    return NULL;
}

void* QLThumbnailCopyClientProperties(void) {
    if (verbose) puts("STUB: QLThumbnailCopyClientProperties called");
    return NULL;
}

void* QLThumbnailCopyDocumentURL(void) {
    if (verbose) puts("STUB: QLThumbnailCopyDocumentURL called");
    return NULL;
}

void* QLThumbnailCopyImage(void) {
    if (verbose) puts("STUB: QLThumbnailCopyImage called");
    return NULL;
}

void* QLThumbnailCopyImageMask(void) {
    if (verbose) puts("STUB: QLThumbnailCopyImageMask called");
    return NULL;
}

void* QLThumbnailCopyOptions(void) {
    if (verbose) puts("STUB: QLThumbnailCopyOptions called");
    return NULL;
}

void* QLThumbnailCopyProperties(void) {
    if (verbose) puts("STUB: QLThumbnailCopyProperties called");
    return NULL;
}

void* QLThumbnailCopySpecialGenericImage(void) {
    if (verbose) puts("STUB: QLThumbnailCopySpecialGenericImage called");
    return NULL;
}

void* QLThumbnailCreate(void) {
    if (verbose) puts("STUB: QLThumbnailCreate called");
    return NULL;
}

void* QLThumbnailCreateSpecialGenericIcon(void) {
    if (verbose) puts("STUB: QLThumbnailCreateSpecialGenericIcon called");
    return NULL;
}

void* QLThumbnailDeleteCacheForVolume(void) {
    if (verbose) puts("STUB: QLThumbnailDeleteCacheForVolume called");
    return NULL;
}

void* QLThumbnailDispatchAsync(void) {
    if (verbose) puts("STUB: QLThumbnailDispatchAsync called");
    return NULL;
}

void* QLThumbnailGetContentRect(void) {
    if (verbose) puts("STUB: QLThumbnailGetContentRect called");
    return NULL;
}

void* QLThumbnailGetMaximumSize(void) {
    if (verbose) puts("STUB: QLThumbnailGetMaximumSize called");
    return NULL;
}

void* QLThumbnailGetRepresentedObject(void) {
    if (verbose) puts("STUB: QLThumbnailGetRepresentedObject called");
    return NULL;
}

void* QLThumbnailGetTypeID(void) {
    if (verbose) puts("STUB: QLThumbnailGetTypeID called");
    return NULL;
}

void* QLThumbnailImageCreate(void) {
    if (verbose) puts("STUB: QLThumbnailImageCreate called");
    return NULL;
}

void* QLThumbnailInvalidateCacheForURL(void) {
    if (verbose) puts("STUB: QLThumbnailInvalidateCacheForURL called");
    return NULL;
}

void* QLThumbnailInvalidateCacheForURLs(void) {
    if (verbose) puts("STUB: QLThumbnailInvalidateCacheForURLs called");
    return NULL;
}

void* QLThumbnailIsCancelled(void) {
    if (verbose) puts("STUB: QLThumbnailIsCancelled called");
    return NULL;
}

void* QLThumbnailIsGenericFolder(void) {
    if (verbose) puts("STUB: QLThumbnailIsGenericFolder called");
    return NULL;
}

void* QLThumbnailIsLowQuality(void) {
    if (verbose) puts("STUB: QLThumbnailIsLowQuality called");
    return NULL;
}

void* QLThumbnailNotifyWhenAdditionIsAvailable(void) {
    if (verbose) puts("STUB: QLThumbnailNotifyWhenAdditionIsAvailable called");
    return NULL;
}

void* QLThumbnailQueueAddThumbnail(void) {
    if (verbose) puts("STUB: QLThumbnailQueueAddThumbnail called");
    return NULL;
}

void* QLThumbnailQueueAddThumbnailsFromArray(void) {
    if (verbose) puts("STUB: QLThumbnailQueueAddThumbnailsFromArray called");
    return NULL;
}

void* QLThumbnailQueueCancelAll(void) {
    if (verbose) puts("STUB: QLThumbnailQueueCancelAll called");
    return NULL;
}

void* QLThumbnailQueueCancelThumbnails(void) {
    if (verbose) puts("STUB: QLThumbnailQueueCancelThumbnails called");
    return NULL;
}

void* QLThumbnailQueueCreate(void) {
    if (verbose) puts("STUB: QLThumbnailQueueCreate called");
    return NULL;
}

void* QLThumbnailQueueGetTypeID(void) {
    if (verbose) puts("STUB: QLThumbnailQueueGetTypeID called");
    return NULL;
}

void* QLThumbnailQueueInvalidate(void) {
    if (verbose) puts("STUB: QLThumbnailQueueInvalidate called");
    return NULL;
}

void* QLThumbnailQueueIsProcessing(void) {
    if (verbose) puts("STUB: QLThumbnailQueueIsProcessing called");
    return NULL;
}

void* QLThumbnailQueueRun(void) {
    if (verbose) puts("STUB: QLThumbnailQueueRun called");
    return NULL;
}

void* QLThumbnailQueueSetClient(void) {
    if (verbose) puts("STUB: QLThumbnailQueueSetClient called");
    return NULL;
}

void* QLThumbnailQueueSetCompletionBlock(void) {
    if (verbose) puts("STUB: QLThumbnailQueueSetCompletionBlock called");
    return NULL;
}

void* QLThumbnailQueueSetMaximumQueuePushCount(void) {
    if (verbose) puts("STUB: QLThumbnailQueueSetMaximumQueuePushCount called");
    return NULL;
}

void* QLThumbnailQueueSetMinimumQueuePushCount(void) {
    if (verbose) puts("STUB: QLThumbnailQueueSetMinimumQueuePushCount called");
    return NULL;
}

void* QLThumbnailQueueWakeUp(void) {
    if (verbose) puts("STUB: QLThumbnailQueueWakeUp called");
    return NULL;
}

void* QLThumbnailRequestCopyBackPageContext(void) {
    if (verbose) puts("STUB: QLThumbnailRequestCopyBackPageContext called");
    return NULL;
}

void* QLThumbnailRequestCopyContentUTI(void) {
    if (verbose) puts("STUB: QLThumbnailRequestCopyContentUTI called");
    return NULL;
}

void* QLThumbnailRequestCopyOptions(void) {
    if (verbose) puts("STUB: QLThumbnailRequestCopyOptions called");
    return NULL;
}

void* QLThumbnailRequestCopyURL(void) {
    if (verbose) puts("STUB: QLThumbnailRequestCopyURL called");
    return NULL;
}

void* QLThumbnailRequestCreateContext(void) {
    if (verbose) puts("STUB: QLThumbnailRequestCreateContext called");
    return NULL;
}

void* QLThumbnailRequestFlushContext(void) {
    if (verbose) puts("STUB: QLThumbnailRequestFlushContext called");
    return NULL;
}

void* QLThumbnailRequestGetBitmapMaximumSize(void) {
    if (verbose) puts("STUB: QLThumbnailRequestGetBitmapMaximumSize called");
    return NULL;
}

void* QLThumbnailRequestGetDimension(void) {
    if (verbose) puts("STUB: QLThumbnailRequestGetDimension called");
    return NULL;
}

void* QLThumbnailRequestGetDocumentObject(void) {
    if (verbose) puts("STUB: QLThumbnailRequestGetDocumentObject called");
    return NULL;
}

void* QLThumbnailRequestGetGeneratorBundle(void) {
    if (verbose) puts("STUB: QLThumbnailRequestGetGeneratorBundle called");
    return NULL;
}

void* QLThumbnailRequestGetMaximumSize(void) {
    if (verbose) puts("STUB: QLThumbnailRequestGetMaximumSize called");
    return NULL;
}

void* QLThumbnailRequestGetTypeID(void) {
    if (verbose) puts("STUB: QLThumbnailRequestGetTypeID called");
    return NULL;
}

void* QLThumbnailRequestIsCancelled(void) {
    if (verbose) puts("STUB: QLThumbnailRequestIsCancelled called");
    return NULL;
}

void* QLThumbnailRequestSetDocumentObject(void) {
    if (verbose) puts("STUB: QLThumbnailRequestSetDocumentObject called");
    return NULL;
}

void* QLThumbnailRequestSetImage(void) {
    if (verbose) puts("STUB: QLThumbnailRequestSetImage called");
    return NULL;
}

void* QLThumbnailRequestSetImageAtURL(void) {
    if (verbose) puts("STUB: QLThumbnailRequestSetImageAtURL called");
    return NULL;
}

void* QLThumbnailRequestSetImageWithData(void) {
    if (verbose) puts("STUB: QLThumbnailRequestSetImageWithData called");
    return NULL;
}

void* QLThumbnailRequestSetThumbnailWithDataRepresentation(void) {
    if (verbose) puts("STUB: QLThumbnailRequestSetThumbnailWithDataRepresentation called");
    return NULL;
}

void* QLThumbnailRequestSetThumbnailWithURLRepresentation(void) {
    if (verbose) puts("STUB: QLThumbnailRequestSetThumbnailWithURLRepresentation called");
    return NULL;
}

void* QLThumbnailSetForceContentTypeUTI(void) {
    if (verbose) puts("STUB: QLThumbnailSetForceContentTypeUTI called");
    return NULL;
}

void* QLThumbnailSetRepresentedObject(void) {
    if (verbose) puts("STUB: QLThumbnailSetRepresentedObject called");
    return NULL;
}

void* QLThumbnailSetServer(void) {
    if (verbose) puts("STUB: QLThumbnailSetServer called");
    return NULL;
}

void* QLThumbnailSupportsContentUTIAtSize(void) {
    if (verbose) puts("STUB: QLThumbnailSupportsContentUTIAtSize called");
    return NULL;
}

void* QLUIHelperCreateCallbackSource(void) {
    if (verbose) puts("STUB: QLUIHelperCreateCallbackSource called");
    return NULL;
}

void* QLUIHelperExecuteMachBlock(void) {
    if (verbose) puts("STUB: QLUIHelperExecuteMachBlock called");
    return NULL;
}

void* QLUIHelperIsAvailable(void) {
    if (verbose) puts("STUB: QLUIHelperIsAvailable called");
    return NULL;
}

void* QLUIHelperRegisterForHelperDeath(void) {
    if (verbose) puts("STUB: QLUIHelperRegisterForHelperDeath called");
    return NULL;
}

void* QLUIHelperServerDemux(void) {
    if (verbose) puts("STUB: QLUIHelperServerDemux called");
    return NULL;
}

void* QLUIHelperServerPort(void) {
    if (verbose) puts("STUB: QLUIHelperServerPort called");
    return NULL;
}

void* QLUIHelperUnregisterFromHelperDeath(void) {
    if (verbose) puts("STUB: QLUIHelperUnregisterFromHelperDeath called");
    return NULL;
}

void* QLUTIIsGenericFolder(void) {
    if (verbose) puts("STUB: QLUTIIsGenericFolder called");
    return NULL;
}

void* _DrawPSDGradient(void) {
    if (verbose) puts("STUB: _DrawPSDGradient called");
    return NULL;
}

void* _QLAppPreferencesBooleanValueForKey(void) {
    if (verbose) puts("STUB: _QLAppPreferencesBooleanValueForKey called");
    return NULL;
}

void* _QLAppPreferencesCFIndexValueForKey(void) {
    if (verbose) puts("STUB: _QLAppPreferencesCFIndexValueForKey called");
    return NULL;
}

void* _QLAppPreferencesFloatValueForKey(void) {
    if (verbose) puts("STUB: _QLAppPreferencesFloatValueForKey called");
    return NULL;
}

void* _QLAppPreferencesIntValueForKey(void) {
    if (verbose) puts("STUB: _QLAppPreferencesIntValueForKey called");
    return NULL;
}

void* _QLAttributedStringCopyPlistRepresentation(void) {
    if (verbose) puts("STUB: _QLAttributedStringCopyPlistRepresentation called");
    return NULL;
}

void* _QLBubbleFromPort(void) {
    if (verbose) puts("STUB: _QLBubbleFromPort called");
    return NULL;
}

void* _QLBubbleRegister(void) {
    if (verbose) puts("STUB: _QLBubbleRegister called");
    return NULL;
}

void* _QLBubbleUnregister(void) {
    if (verbose) puts("STUB: _QLBubbleUnregister called");
    return NULL;
}

void* _QLCFDataGetBytes(void) {
    if (verbose) puts("STUB: _QLCFDataGetBytes called");
    return NULL;
}

void* _QLCacheDidInvalidate(void) {
    if (verbose) puts("STUB: _QLCacheDidInvalidate called");
    return NULL;
}

void* _QLCacheInvalidationNotificationCopyURLs(void) {
    if (verbose) puts("STUB: _QLCacheInvalidationNotificationCopyURLs called");
    return NULL;
}

void* _QLCacheVersionCopyDescription(void) {
    if (verbose) puts("STUB: _QLCacheVersionCopyDescription called");
    return NULL;
}

void* _QLCacheVersionCopySerializedData(void) {
    if (verbose) puts("STUB: _QLCacheVersionCopySerializedData called");
    return NULL;
}

void* _QLCacheVersionCreateFromURL(void) {
    if (verbose) puts("STUB: _QLCacheVersionCreateFromURL called");
    return NULL;
}

void* _QLCacheVersionCreateFromURLWithGenerator(void) {
    if (verbose) puts("STUB: _QLCacheVersionCreateFromURLWithGenerator called");
    return NULL;
}

void* _QLCacheVersionCreateWithBytes(void) {
    if (verbose) puts("STUB: _QLCacheVersionCreateWithBytes called");
    return NULL;
}

void* _QLCacheVersionCreateWithData(void) {
    if (verbose) puts("STUB: _QLCacheVersionCreateWithData called");
    return NULL;
}

void* _QLCacheVersionIsEqual(void) {
    if (verbose) puts("STUB: _QLCacheVersionIsEqual called");
    return NULL;
}

void* _QLCacheVersionIsEqualIncludingGeneratorVersion(void) {
    if (verbose) puts("STUB: _QLCacheVersionIsEqualIncludingGeneratorVersion called");
    return NULL;
}

void* _QLCopyAllThumbnailInfo(void) {
    if (verbose) puts("STUB: _QLCopyAllThumbnailInfo called");
    return NULL;
}

void* _QLCopyCacheInfo(void) {
    if (verbose) puts("STUB: _QLCopyCacheInfo called");
    return NULL;
}

void* _QLCopyCachedThumbnailsForURL(void) {
    if (verbose) puts("STUB: _QLCopyCachedThumbnailsForURL called");
    return NULL;
}

void* _QLCopyPluginMapping(void) {
    if (verbose) puts("STUB: _QLCopyPluginMapping called");
    return NULL;
}

void* _QLCopyServerStatistics(void) {
    if (verbose) puts("STUB: _QLCopyServerStatistics called");
    return NULL;
}

void* _QLCopyUTITypeForFileAtURL(void) {
    if (verbose) puts("STUB: _QLCopyUTITypeForFileAtURL called");
    return NULL;
}

void* _QLCrash(void) {
    if (verbose) puts("STUB: _QLCrash called");
    return NULL;
}

void* _QLCrashForGoodReason(void) {
    if (verbose) puts("STUB: _QLCrashForGoodReason called");
    return NULL;
}

void* _QLCreateArrayFromPlistBytesFromMIG(void) {
    if (verbose) puts("STUB: _QLCreateArrayFromPlistBytesFromMIG called");
    return NULL;
}

void* _QLCreateAttributedStringFromPlistRepresentation(void) {
    if (verbose) puts("STUB: _QLCreateAttributedStringFromPlistRepresentation called");
    return NULL;
}

void* _QLCreateCFStringFromQLStringFromMIG(void) {
    if (verbose) puts("STUB: _QLCreateCFStringFromQLStringFromMIG called");
    return NULL;
}

void* _QLCreateCFURLFromQLPathFromMIG(void) {
    if (verbose) puts("STUB: _QLCreateCFURLFromQLPathFromMIG called");
    return NULL;
}

void* _QLCreateCFURLFromQLURLFromMIG(void) {
    if (verbose) puts("STUB: _QLCreateCFURLFromQLURLFromMIG called");
    return NULL;
}

void* _QLCreateDictionaryFromPlistBytesFromMIG(void) {
    if (verbose) puts("STUB: _QLCreateDictionaryFromPlistBytesFromMIG called");
    return NULL;
}

void* _QLCreateImageDataFromCGImageForXPC(void) {
    if (verbose) puts("STUB: _QLCreateImageDataFromCGImageForXPC called");
    return NULL;
}

void* _QLCreateImageFromBitmapDataFromMIG(void) {
    if (verbose) puts("STUB: _QLCreateImageFromBitmapDataFromMIG called");
    return NULL;
}

void* _QLCreateImageFromBitmapDataFromXPC(void) {
    if (verbose) puts("STUB: _QLCreateImageFromBitmapDataFromXPC called");
    return NULL;
}

void* _QLCreateQLImageFromCGBitmapContextForMIG(void) {
    if (verbose) puts("STUB: _QLCreateQLImageFromCGBitmapContextForMIG called");
    return NULL;
}

void* _QLCreateQLImageFromCGImageForMIG(void) {
    if (verbose) puts("STUB: _QLCreateQLImageFromCGImageForMIG called");
    return NULL;
}

void* _QLCreateQLPathFromCFURLForMIG(void) {
    if (verbose) puts("STUB: _QLCreateQLPathFromCFURLForMIG called");
    return NULL;
}

void* _QLCreateQLPathFromCFURLForMIGWithResolution(void) {
    if (verbose) puts("STUB: _QLCreateQLPathFromCFURLForMIGWithResolution called");
    return NULL;
}

void* _QLCreateQLStringFromCFStringForMIG(void) {
    if (verbose) puts("STUB: _QLCreateQLStringFromCFStringForMIG called");
    return NULL;
}

void* _QLCreateQLURLFromCFURLForMIG(void) {
    if (verbose) puts("STUB: _QLCreateQLURLFromCFURLForMIG called");
    return NULL;
}

void* _QLDebugWriteBitmapContextToDisk(void) {
    if (verbose) puts("STUB: _QLDebugWriteBitmapContextToDisk called");
    return NULL;
}

void* _QLDebugWriteImageToDisk(void) {
    if (verbose) puts("STUB: _QLDebugWriteImageToDisk called");
    return NULL;
}

void* _QLDebugWriteLayerToDisk(void) {
    if (verbose) puts("STUB: _QLDebugWriteLayerToDisk called");
    return NULL;
}

void* _QLDestroyMallocedBitmapContext(void) {
    if (verbose) puts("STUB: _QLDestroyMallocedBitmapContext called");
    return NULL;
}

void* _QLDestroyReservedBitmapContext(void) {
    if (verbose) puts("STUB: _QLDestroyReservedBitmapContext called");
    return NULL;
}

void* _QLDestroyVMBitmapContext(void) {
    if (verbose) puts("STUB: _QLDestroyVMBitmapContext called");
    return NULL;
}

void* _QLDictionaryBooleanValueForKey(void) {
    if (verbose) puts("STUB: _QLDictionaryBooleanValueForKey called");
    return NULL;
}

void* _QLDictionaryCGFloatValueForKey(void) {
    if (verbose) puts("STUB: _QLDictionaryCGFloatValueForKey called");
    return NULL;
}

void* _QLDictionaryCopyURLForKey(void) {
    if (verbose) puts("STUB: _QLDictionaryCopyURLForKey called");
    return NULL;
}

void* _QLDictionaryFloatValueForKey(void) {
    if (verbose) puts("STUB: _QLDictionaryFloatValueForKey called");
    return NULL;
}

void* _QLDictionaryGetPointValueForKey(void) {
    if (verbose) puts("STUB: _QLDictionaryGetPointValueForKey called");
    return NULL;
}

void* _QLDictionaryGetRectValueForKey(void) {
    if (verbose) puts("STUB: _QLDictionaryGetRectValueForKey called");
    return NULL;
}

void* _QLDictionaryGetSizeValueForKey(void) {
    if (verbose) puts("STUB: _QLDictionaryGetSizeValueForKey called");
    return NULL;
}

void* _QLDictionaryIntValueForKey(void) {
    if (verbose) puts("STUB: _QLDictionaryIntValueForKey called");
    return NULL;
}

void* _QLDictionarySInt32ValueForKey(void) {
    if (verbose) puts("STUB: _QLDictionarySInt32ValueForKey called");
    return NULL;
}

void* _QLDictionarySetBooleanValueForKey(void) {
    if (verbose) puts("STUB: _QLDictionarySetBooleanValueForKey called");
    return NULL;
}

void* _QLDictionarySetCGFloatValueForKey(void) {
    if (verbose) puts("STUB: _QLDictionarySetCGFloatValueForKey called");
    return NULL;
}

void* _QLDictionarySetFloatValueForKey(void) {
    if (verbose) puts("STUB: _QLDictionarySetFloatValueForKey called");
    return NULL;
}

void* _QLDictionarySetIntValueForKey(void) {
    if (verbose) puts("STUB: _QLDictionarySetIntValueForKey called");
    return NULL;
}

void* _QLDictionarySetPointValueForKey(void) {
    if (verbose) puts("STUB: _QLDictionarySetPointValueForKey called");
    return NULL;
}

void* _QLDictionarySetRectValueForKey(void) {
    if (verbose) puts("STUB: _QLDictionarySetRectValueForKey called");
    return NULL;
}

void* _QLDictionarySetSInt32ValueForKey(void) {
    if (verbose) puts("STUB: _QLDictionarySetSInt32ValueForKey called");
    return NULL;
}

void* _QLDictionarySetSizeValueForKey(void) {
    if (verbose) puts("STUB: _QLDictionarySetSizeValueForKey called");
    return NULL;
}

void* _QLDictionarySetURLForKey(void) {
    if (verbose) puts("STUB: _QLDictionarySetURLForKey called");
    return NULL;
}

void* _QLDictionarySetValueForKeyIfExists(void) {
    if (verbose) puts("STUB: _QLDictionarySetValueForKeyIfExists called");
    return NULL;
}

void* _QLDictionaryValueForContentType(void) {
    if (verbose) puts("STUB: _QLDictionaryValueForContentType called");
    return NULL;
}

void* _QLDictionaryValueForKey(void) {
    if (verbose) puts("STUB: _QLDictionaryValueForKey called");
    return NULL;
}

void* _QLDumpMachPortRights(void) {
    if (verbose) puts("STUB: _QLDumpMachPortRights called");
    return NULL;
}

void* _QLEnableLogs(void) {
    if (verbose) puts("STUB: _QLEnableLogs called");
    return NULL;
}

void* _QLFlushLogs(void) {
    if (verbose) puts("STUB: _QLFlushLogs called");
    return NULL;
}

void* _QLForcePluginRegistration(void) {
    if (verbose) puts("STUB: _QLForcePluginRegistration called");
    return NULL;
}

void* _QLFreeQLImageForMIG(void) {
    if (verbose) puts("STUB: _QLFreeQLImageForMIG called");
    return NULL;
}

void* _QLFreeQLPathForMIG(void) {
    if (verbose) puts("STUB: _QLFreeQLPathForMIG called");
    return NULL;
}

void* _QLFreeQLPathFromMIG(void) {
    if (verbose) puts("STUB: _QLFreeQLPathFromMIG called");
    return NULL;
}

void* _QLFreeQLStringForMIG(void) {
    if (verbose) puts("STUB: _QLFreeQLStringForMIG called");
    return NULL;
}

void* _QLFreeQLStringFromMIG(void) {
    if (verbose) puts("STUB: _QLFreeQLStringFromMIG called");
    return NULL;
}

void* _QLFreeQLURLForMIG(void) {
    if (verbose) puts("STUB: _QLFreeQLURLForMIG called");
    return NULL;
}

void* _QLFreeQLURLFromMIG(void) {
    if (verbose) puts("STUB: _QLFreeQLURLFromMIG called");
    return NULL;
}

void* _QLGeneratorAutoDisplayExtension(void) {
    if (verbose) puts("STUB: _QLGeneratorAutoDisplayExtension called");
    return NULL;
}

void* _QLGeneratorCopyAllIDs(void) {
    if (verbose) puts("STUB: _QLGeneratorCopyAllIDs called");
    return NULL;
}

void* _QLGeneratorCopyForcedGenerator(void) {
    if (verbose) puts("STUB: _QLGeneratorCopyForcedGenerator called");
    return NULL;
}

void* _QLGeneratorCopyGeneratorForContentType(void) {
    if (verbose) puts("STUB: _QLGeneratorCopyGeneratorForContentType called");
    return NULL;
}

void* _QLGeneratorCopyGeneratorForFile(void) {
    if (verbose) puts("STUB: _QLGeneratorCopyGeneratorForFile called");
    return NULL;
}

void* _QLGeneratorCopyGeneratorFromBundleURL(void) {
    if (verbose) puts("STUB: _QLGeneratorCopyGeneratorFromBundleURL called");
    return NULL;
}

void* _QLGeneratorCopyGeneratorFromID(void) {
    if (verbose) puts("STUB: _QLGeneratorCopyGeneratorFromID called");
    return NULL;
}

void* _QLGeneratorCopyGeneratorsLibrary(void) {
    if (verbose) puts("STUB: _QLGeneratorCopyGeneratorsLibrary called");
    return NULL;
}

void* _QLGeneratorCreate(void) {
    if (verbose) puts("STUB: _QLGeneratorCreate called");
    return NULL;
}

void* _QLGeneratorCreateBuiltin(void) {
    if (verbose) puts("STUB: _QLGeneratorCreateBuiltin called");
    return NULL;
}

void* _QLGeneratorDisableBaseGenerators(void) {
    if (verbose) puts("STUB: _QLGeneratorDisableBaseGenerators called");
    return NULL;
}

void* _QLGeneratorDisableUntrustedGenerators(void) {
    if (verbose) puts("STUB: _QLGeneratorDisableUntrustedGenerators called");
    return NULL;
}

void* _QLGeneratorGetDefaultFlavor(void) {
    if (verbose) puts("STUB: _QLGeneratorGetDefaultFlavor called");
    return NULL;
}

void* _QLGeneratorGetGeneratorID(void) {
    if (verbose) puts("STUB: _QLGeneratorGetGeneratorID called");
    return NULL;
}

void* _QLGeneratorGetNoGenerator(void) {
    if (verbose) puts("STUB: _QLGeneratorGetNoGenerator called");
    return NULL;
}

void* _QLGeneratorGetPluginURL(void) {
    if (verbose) puts("STUB: _QLGeneratorGetPluginURL called");
    return NULL;
}

void* _QLGeneratorGetPluginVersion(void) {
    if (verbose) puts("STUB: _QLGeneratorGetPluginVersion called");
    return NULL;
}

void* _QLGeneratorIsFolderGeneratorAllowed(void) {
    if (verbose) puts("STUB: _QLGeneratorIsFolderGeneratorAllowed called");
    return NULL;
}

void* _QLGeneratorNeedsAccessToExternalResources(void) {
    if (verbose) puts("STUB: _QLGeneratorNeedsAccessToExternalResources called");
    return NULL;
}

void* _QLGeneratorResetPluginCache(void) {
    if (verbose) puts("STUB: _QLGeneratorResetPluginCache called");
    return NULL;
}

void* _QLGeneratorScore(void) {
    if (verbose) puts("STUB: _QLGeneratorScore called");
    return NULL;
}

void* _QLGeneratorSetMaxGlobalWeight(void) {
    if (verbose) puts("STUB: _QLGeneratorSetMaxGlobalWeight called");
    return NULL;
}

void* _QLGeneratorSetPluginsWhiteList(void) {
    if (verbose) puts("STUB: _QLGeneratorSetPluginsWhiteList called");
    return NULL;
}

void* _QLGeneratorSupportsSandboxing(void) {
    if (verbose) puts("STUB: _QLGeneratorSupportsSandboxing called");
    return NULL;
}

void* _QLGlobalQuickLookPreferencesBooleanValueForKey(void) {
    if (verbose) puts("STUB: _QLGlobalQuickLookPreferencesBooleanValueForKey called");
    return NULL;
}

void* _QLIconCopyIconSetImageName(void) {
    if (verbose) puts("STUB: _QLIconCopyIconSetImageName called");
    return NULL;
}

void* _QLIsLoggingEnabled(void) {
    if (verbose) puts("STUB: _QLIsLoggingEnabled called");
    return NULL;
}

void* _QLLogUpdateProcName(void) {
    if (verbose) puts("STUB: _QLLogUpdateProcName called");
    return NULL;
}

void* _QLMachDispatchIncomingMessageAndReplyIfNecessary(void) {
    if (verbose) puts("STUB: _QLMachDispatchIncomingMessageAndReplyIfNecessary called");
    return NULL;
}

void* _QLMachGetMachPortCount(void) {
    if (verbose) puts("STUB: _QLMachGetMachPortCount called");
    return NULL;
}

void* _QLMachGetMinimumSequenceNumber(void) {
    if (verbose) puts("STUB: _QLMachGetMinimumSequenceNumber called");
    return NULL;
}

void* _QLMachRegisterForNoSenders(void) {
    if (verbose) puts("STUB: _QLMachRegisterForNoSenders called");
    return NULL;
}

void* _QLMachUnregisterForNoSenders(void) {
    if (verbose) puts("STUB: _QLMachUnregisterForNoSenders called");
    return NULL;
}

void* _QLMainLocalizedString(void) {
    if (verbose) puts("STUB: _QLMainLocalizedString called");
    return NULL;
}

void* _QLMiniCacheFolderCreateWithURL(void) {
    if (verbose) puts("STUB: _QLMiniCacheFolderCreateWithURL called");
    return NULL;
}

void* _QLPlugInLocalizedString(void) {
    if (verbose) puts("STUB: _QLPlugInLocalizedString called");
    return NULL;
}

void* _QLPostCacheInvalidationNotification(void) {
    if (verbose) puts("STUB: _QLPostCacheInvalidationNotification called");
    return NULL;
}

void* _QLPostGlobalChangeNotification(void) {
    if (verbose) puts("STUB: _QLPostGlobalChangeNotification called");
    return NULL;
}

void* _QLPrefferedParentUTI(void) {
    if (verbose) puts("STUB: _QLPrefferedParentUTI called");
    return NULL;
}

void* _QLPreviewCopyUTIIfPresent(void) {
    if (verbose) puts("STUB: _QLPreviewCopyUTIIfPresent called");
    return NULL;
}

void* _QLPreviewCreate(void) {
    if (verbose) puts("STUB: _QLPreviewCreate called");
    return NULL;
}

void* _QLPreviewReplyAppendData(void) {
    if (verbose) puts("STUB: _QLPreviewReplyAppendData called");
    return NULL;
}

void* _QLPreviewRequestCallGenerator(void) {
    if (verbose) puts("STUB: _QLPreviewRequestCallGenerator called");
    return NULL;
}

void* _QLPreviewRequestCreate(void) {
    if (verbose) puts("STUB: _QLPreviewRequestCreate called");
    return NULL;
}

void* _QLPreviewRequestCreateFromXPCRepresentation(void) {
    if (verbose) puts("STUB: _QLPreviewRequestCreateFromXPCRepresentation called");
    return NULL;
}

void* _QLPreviewRequestDispatch(void) {
    if (verbose) puts("STUB: _QLPreviewRequestDispatch called");
    return NULL;
}

void* _QLPreviewRequestFillXPCRepresentation(void) {
    if (verbose) puts("STUB: _QLPreviewRequestFillXPCRepresentation called");
    return NULL;
}

void* _QLPreviewRequestForward32(void) {
    if (verbose) puts("STUB: _QLPreviewRequestForward32 called");
    return NULL;
}

void* _QLPreviewRequestGotMoreData(void) {
    if (verbose) puts("STUB: _QLPreviewRequestGotMoreData called");
    return NULL;
}

void* _QLPreviewRequestReplyForInvalidRequest(void) {
    if (verbose) puts("STUB: _QLPreviewRequestReplyForInvalidRequest called");
    return NULL;
}

void* _QLPreviewRequestReplyWithContentTypeAndProperties(void) {
    if (verbose) puts("STUB: _QLPreviewRequestReplyWithContentTypeAndProperties called");
    return NULL;
}

void* _QLPreviewRequestSendMoreData(void) {
    if (verbose) puts("STUB: _QLPreviewRequestSendMoreData called");
    return NULL;
}

void* _QLPreviewRequestSendPendingReply(void) {
    if (verbose) puts("STUB: _QLPreviewRequestSendPendingReply called");
    return NULL;
}

void* _QLPreviewRequestSendReply(void) {
    if (verbose) puts("STUB: _QLPreviewRequestSendReply called");
    return NULL;
}

void* _QLPreviewRequestSendStartNewAttachment(void) {
    if (verbose) puts("STUB: _QLPreviewRequestSendStartNewAttachment called");
    return NULL;
}

void* _QLPreviewRequestSetDistantURLRepresentation(void) {
    if (verbose) puts("STUB: _QLPreviewRequestSetDistantURLRepresentation called");
    return NULL;
}

void* _QLPreviewRequestSetupGenerator(void) {
    if (verbose) puts("STUB: _QLPreviewRequestSetupGenerator called");
    return NULL;
}

void* _QLPreviewRequestStartNewAttachment(void) {
    if (verbose) puts("STUB: _QLPreviewRequestStartNewAttachment called");
    return NULL;
}

void* _QLPreviewRequestUpdateFromXPCRepresentation(void) {
    if (verbose) puts("STUB: _QLPreviewRequestUpdateFromXPCRepresentation called");
    return NULL;
}

void* _QLPreviewRequestWontCallGenerator(void) {
    if (verbose) puts("STUB: _QLPreviewRequestWontCallGenerator called");
    return NULL;
}

void* _QLRaiseAssert(void) {
    if (verbose) puts("STUB: _QLRaiseAssert called");
    return NULL;
}

void* _QLRegisterCacheInvalidationObserver(void) {
    if (verbose) puts("STUB: _QLRegisterCacheInvalidationObserver called");
    return NULL;
}

void* _QLRegisterForQuickLookGlobalNotification(void) {
    if (verbose) puts("STUB: _QLRegisterForQuickLookGlobalNotification called");
    return NULL;
}

void* _QLRegisterGlobalChangeObserver(void) {
    if (verbose) puts("STUB: _QLRegisterGlobalChangeObserver called");
    return NULL;
}

void* _QLRequestCancelGenerator(void) {
    if (verbose) puts("STUB: _QLRequestCancelGenerator called");
    return NULL;
}

void* _QLRequestCleanupAfterException(void) {
    if (verbose) puts("STUB: _QLRequestCleanupAfterException called");
    return NULL;
}

void* _QLRequestCreateFromXPCRepresentation(void) {
    if (verbose) puts("STUB: _QLRequestCreateFromXPCRepresentation called");
    return NULL;
}

void* _QLRequestFillXPCRepresentation(void) {
    if (verbose) puts("STUB: _QLRequestFillXPCRepresentation called");
    return NULL;
}

void* _QLRequestGeneratorShouldRunInSatellite(void) {
    if (verbose) puts("STUB: _QLRequestGeneratorShouldRunInSatellite called");
    return NULL;
}

void* _QLRequestHasGeneratorFunction(void) {
    if (verbose) puts("STUB: _QLRequestHasGeneratorFunction called");
    return NULL;
}

void* _QLRequestIsSandboxedGenerator(void) {
    if (verbose) puts("STUB: _QLRequestIsSandboxedGenerator called");
    return NULL;
}

void* _QLRequestNeedsAccessToExternalResources(void) {
    if (verbose) puts("STUB: _QLRequestNeedsAccessToExternalResources called");
    return NULL;
}

void* _QLRequestSendWillStart(void) {
    if (verbose) puts("STUB: _QLRequestSendWillStart called");
    return NULL;
}

void* _QLRequestUnmapData(void) {
    if (verbose) puts("STUB: _QLRequestUnmapData called");
    return NULL;
}

void* _QLRequestUpdateFromXPCRepresentation(void) {
    if (verbose) puts("STUB: _QLRequestUpdateFromXPCRepresentation called");
    return NULL;
}

void* _QLResetLogs(void) {
    if (verbose) puts("STUB: _QLResetLogs called");
    return NULL;
}

void* _QLResetServer(void) {
    if (verbose) puts("STUB: _QLResetServer called");
    return NULL;
}

void* _QLSeamlessClosingRequestFromPort(void) {
    if (verbose) puts("STUB: _QLSeamlessClosingRequestFromPort called");
    return NULL;
}

void* _QLSeamlessClosingRequestInvalidate(void) {
    if (verbose) puts("STUB: _QLSeamlessClosingRequestInvalidate called");
    return NULL;
}

void* _QLSeamlessOpeningCallCompletionHandlerAndInvalidate(void) {
    if (verbose) puts("STUB: _QLSeamlessOpeningCallCompletionHandlerAndInvalidate called");
    return NULL;
}

void* _QLSeamlessOpeningRequestFromPort(void) {
    if (verbose) puts("STUB: _QLSeamlessOpeningRequestFromPort called");
    return NULL;
}

void* _QLSeamlessOpeningRequestInvalidate(void) {
    if (verbose) puts("STUB: _QLSeamlessOpeningRequestInvalidate called");
    return NULL;
}

void* _QLSeamlessOpeningRequestRegister(void) {
    if (verbose) puts("STUB: _QLSeamlessOpeningRequestRegister called");
    return NULL;
}

void* _QLSeamlessOpeningRequestUnregister(void) {
    if (verbose) puts("STUB: _QLSeamlessOpeningRequestUnregister called");
    return NULL;
}

void* _QLSelectParentUTI(void) {
    if (verbose) puts("STUB: _QLSelectParentUTI called");
    return NULL;
}

void* _QLSetContainsContentType(void) {
    if (verbose) puts("STUB: _QLSetContainsContentType called");
    return NULL;
}

void* _QLSetThreadName(void) {
    if (verbose) puts("STUB: _QLSetThreadName called");
    return NULL;
}

void* _QLSimpleCrash(void) {
    if (verbose) puts("STUB: _QLSimpleCrash called");
    return NULL;
}

void* _QLStackReleaseFrame(void) {
    if (verbose) puts("STUB: _QLStackReleaseFrame called");
    return NULL;
}

void* _QLStopServerASAP(void) {
    if (verbose) puts("STUB: _QLStopServerASAP called");
    return NULL;
}

void* _QLThreadCopyName(void) {
    if (verbose) puts("STUB: _QLThreadCopyName called");
    return NULL;
}

void* _QLThumbnailComputeBaseline(void) {
    if (verbose) puts("STUB: _QLThumbnailComputeBaseline called");
    return NULL;
}

void* _QLThumbnailCopyCacheVersion(void) {
    if (verbose) puts("STUB: _QLThumbnailCopyCacheVersion called");
    return NULL;
}

void* _QLThumbnailCopyIDForSecureDelete(void) {
    if (verbose) puts("STUB: _QLThumbnailCopyIDForSecureDelete called");
    return NULL;
}

void* _QLThumbnailCopyUTIIfPresent(void) {
    if (verbose) puts("STUB: _QLThumbnailCopyUTIIfPresent called");
    return NULL;
}

void* _QLThumbnailGetFlavor(void) {
    if (verbose) puts("STUB: _QLThumbnailGetFlavor called");
    return NULL;
}

void* _QLThumbnailGetInlinePreviewMode(void) {
    if (verbose) puts("STUB: _QLThumbnailGetInlinePreviewMode called");
    return NULL;
}

void* _QLThumbnailImageCreateWithImageIO(void) {
    if (verbose) puts("STUB: _QLThumbnailImageCreateWithImageIO called");
    return NULL;
}

void* _QLThumbnailImageReplyGotContentTypeUTI(void) {
    if (verbose) puts("STUB: _QLThumbnailImageReplyGotContentTypeUTI called");
    return NULL;
}

void* _QLThumbnailNotifyWhenAdditionIsAvailable(void) {
    if (verbose) puts("STUB: _QLThumbnailNotifyWhenAdditionIsAvailable called");
    return NULL;
}

void* _QLThumbnailRequestCallGenerator(void) {
    if (verbose) puts("STUB: _QLThumbnailRequestCallGenerator called");
    return NULL;
}

void* _QLThumbnailRequestCleanUp(void) {
    if (verbose) puts("STUB: _QLThumbnailRequestCleanUp called");
    return NULL;
}

void* _QLThumbnailRequestCleanupAfterException(void) {
    if (verbose) puts("STUB: _QLThumbnailRequestCleanupAfterException called");
    return NULL;
}

void* _QLThumbnailRequestCreate(void) {
    if (verbose) puts("STUB: _QLThumbnailRequestCreate called");
    return NULL;
}

void* _QLThumbnailRequestCreateContext(void) {
    if (verbose) puts("STUB: _QLThumbnailRequestCreateContext called");
    return NULL;
}

void* _QLThumbnailRequestCreateFlavorNumber(void) {
    if (verbose) puts("STUB: _QLThumbnailRequestCreateFlavorNumber called");
    return NULL;
}

void* _QLThumbnailRequestCreateFromXPCRepresentation(void) {
    if (verbose) puts("STUB: _QLThumbnailRequestCreateFromXPCRepresentation called");
    return NULL;
}

void* _QLThumbnailRequestDispatch(void) {
    if (verbose) puts("STUB: _QLThumbnailRequestDispatch called");
    return NULL;
}

void* _QLThumbnailRequestFillXPCRepresentation(void) {
    if (verbose) puts("STUB: _QLThumbnailRequestFillXPCRepresentation called");
    return NULL;
}

void* _QLThumbnailRequestForward32(void) {
    if (verbose) puts("STUB: _QLThumbnailRequestForward32 called");
    return NULL;
}

void* _QLThumbnailRequestGetBitmapMaximumSizeForFlavor(void) {
    if (verbose) puts("STUB: _QLThumbnailRequestGetBitmapMaximumSizeForFlavor called");
    return NULL;
}

void* _QLThumbnailRequestGetCacheURL(void) {
    if (verbose) puts("STUB: _QLThumbnailRequestGetCacheURL called");
    return NULL;
}

void* _QLThumbnailRequestGetIconMode(void) {
    if (verbose) puts("STUB: _QLThumbnailRequestGetIconMode called");
    return NULL;
}

void* _QLThumbnailRequestGetMaximumSizeForFlavor(void) {
    if (verbose) puts("STUB: _QLThumbnailRequestGetMaximumSizeForFlavor called");
    return NULL;
}

void* _QLThumbnailRequestGetOptions(void) {
    if (verbose) puts("STUB: _QLThumbnailRequestGetOptions called");
    return NULL;
}

void* _QLThumbnailRequestGetRealMaximumBitmapDimension(void) {
    if (verbose) puts("STUB: _QLThumbnailRequestGetRealMaximumBitmapDimension called");
    return NULL;
}

void* _QLThumbnailRequestGetRealMaximumBitmapSize(void) {
    if (verbose) puts("STUB: _QLThumbnailRequestGetRealMaximumBitmapSize called");
    return NULL;
}

void* _QLThumbnailRequestGetRealMaximumSize(void) {
    if (verbose) puts("STUB: _QLThumbnailRequestGetRealMaximumSize called");
    return NULL;
}

void* _QLThumbnailRequestGetScaleFactorForContext(void) {
    if (verbose) puts("STUB: _QLThumbnailRequestGetScaleFactorForContext called");
    return NULL;
}

void* _QLThumbnailRequestHasCacheBeenTested(void) {
    if (verbose) puts("STUB: _QLThumbnailRequestHasCacheBeenTested called");
    return NULL;
}

void* _QLThumbnailRequestIsDistant(void) {
    if (verbose) puts("STUB: _QLThumbnailRequestIsDistant called");
    return NULL;
}

void* _QLThumbnailRequestIsTooSmallForGenerator(void) {
    if (verbose) puts("STUB: _QLThumbnailRequestIsTooSmallForGenerator called");
    return NULL;
}

void* _QLThumbnailRequestReplyForInvalidRequest(void) {
    if (verbose) puts("STUB: _QLThumbnailRequestReplyForInvalidRequest called");
    return NULL;
}

void* _QLThumbnailRequestSendPendingReply(void) {
    if (verbose) puts("STUB: _QLThumbnailRequestSendPendingReply called");
    return NULL;
}

void* _QLThumbnailRequestSendReply(void) {
    if (verbose) puts("STUB: _QLThumbnailRequestSendReply called");
    return NULL;
}

void* _QLThumbnailRequestSetCacheTested(void) {
    if (verbose) puts("STUB: _QLThumbnailRequestSetCacheTested called");
    return NULL;
}

void* _QLThumbnailRequestSetup(void) {
    if (verbose) puts("STUB: _QLThumbnailRequestSetup called");
    return NULL;
}

void* _QLThumbnailRequestShouldTryCache(void) {
    if (verbose) puts("STUB: _QLThumbnailRequestShouldTryCache called");
    return NULL;
}

void* _QLThumbnailRequestTryCacheRead(void) {
    if (verbose) puts("STUB: _QLThumbnailRequestTryCacheRead called");
    return NULL;
}

void* _QLThumbnailRequestUpdateFromXPCRepresentation(void) {
    if (verbose) puts("STUB: _QLThumbnailRequestUpdateFromXPCRepresentation called");
    return NULL;
}

void* _QLThumbnailRequestWontCallGenerator(void) {
    if (verbose) puts("STUB: _QLThumbnailRequestWontCallGenerator called");
    return NULL;
}

void* _QLThumbnailSetImage(void) {
    if (verbose) puts("STUB: _QLThumbnailSetImage called");
    return NULL;
}

void* _QLThumbnailSetThumbnailWithBitmapData(void) {
    if (verbose) puts("STUB: _QLThumbnailSetThumbnailWithBitmapData called");
    return NULL;
}

void* _QLThumbnailSizeIsSufficentForLowQuality(void) {
    if (verbose) puts("STUB: _QLThumbnailSizeIsSufficentForLowQuality called");
    return NULL;
}

void* _QLUIHelperUnregisterFromHelperDeath(void) {
    if (verbose) puts("STUB: _QLUIHelperUnregisterFromHelperDeath called");
    return NULL;
}

void* _QLUnregisterCacheInvalidationObserver(void) {
    if (verbose) puts("STUB: _QLUnregisterCacheInvalidationObserver called");
    return NULL;
}

void* _QLXPCCGFloatValueForKey(void) {
    if (verbose) puts("STUB: _QLXPCCGFloatValueForKey called");
    return NULL;
}

void* _QLXPCCopyCFObjectForKey(void) {
    if (verbose) puts("STUB: _QLXPCCopyCFObjectForKey called");
    return NULL;
}

void* _QLXPCCopyStringForKey(void) {
    if (verbose) puts("STUB: _QLXPCCopyStringForKey called");
    return NULL;
}

void* _QLXPCCopyURLForKey(void) {
    if (verbose) puts("STUB: _QLXPCCopyURLForKey called");
    return NULL;
}

void* _QLXPCFloatValueForKey(void) {
    if (verbose) puts("STUB: _QLXPCFloatValueForKey called");
    return NULL;
}

void* _QLXPCGetPointValueForKey(void) {
    if (verbose) puts("STUB: _QLXPCGetPointValueForKey called");
    return NULL;
}

void* _QLXPCGetRectValueForKey(void) {
    if (verbose) puts("STUB: _QLXPCGetRectValueForKey called");
    return NULL;
}

void* _QLXPCGetSizeValueForKey(void) {
    if (verbose) puts("STUB: _QLXPCGetSizeValueForKey called");
    return NULL;
}

void* _QLXPCIntValueForKey(void) {
    if (verbose) puts("STUB: _QLXPCIntValueForKey called");
    return NULL;
}

void* _QLXPCSInt32ValueForKey(void) {
    if (verbose) puts("STUB: _QLXPCSInt32ValueForKey called");
    return NULL;
}

void* _QLXPCSetCFObjectForKey(void) {
    if (verbose) puts("STUB: _QLXPCSetCFObjectForKey called");
    return NULL;
}

void* _QLXPCSetCGFloatValueForKey(void) {
    if (verbose) puts("STUB: _QLXPCSetCGFloatValueForKey called");
    return NULL;
}

void* _QLXPCSetFloatValueForKey(void) {
    if (verbose) puts("STUB: _QLXPCSetFloatValueForKey called");
    return NULL;
}

void* _QLXPCSetIntValueForKey(void) {
    if (verbose) puts("STUB: _QLXPCSetIntValueForKey called");
    return NULL;
}

void* _QLXPCSetPointValueForKey(void) {
    if (verbose) puts("STUB: _QLXPCSetPointValueForKey called");
    return NULL;
}

void* _QLXPCSetRectValueForKey(void) {
    if (verbose) puts("STUB: _QLXPCSetRectValueForKey called");
    return NULL;
}

void* _QLXPCSetSInt32ValueForKey(void) {
    if (verbose) puts("STUB: _QLXPCSetSInt32ValueForKey called");
    return NULL;
}

void* _QLXPCSetSizeValueForKey(void) {
    if (verbose) puts("STUB: _QLXPCSetSizeValueForKey called");
    return NULL;
}

void* _QLXPCSetStringForKey(void) {
    if (verbose) puts("STUB: _QLXPCSetStringForKey called");
    return NULL;
}

void* _QLXPCSetURLForKey(void) {
    if (verbose) puts("STUB: _QLXPCSetURLForKey called");
    return NULL;
}

void* _Z18QLTryCatchAndCrashU13block_pointerFvvE(void) {
    if (verbose) puts("STUB: _Z18QLTryCatchAndCrashU13block_pointerFvvE called");
    return NULL;
}

void* __QLLog_base(void) {
    if (verbose) puts("STUB: __QLLog_base called");
    return NULL;
}

void* __QLShouldLogForLevel(void) {
    if (verbose) puts("STUB: __QLShouldLogForLevel called");
    return NULL;
}

void* _geParenttFolderForFileURL(void) {
    if (verbose) puts("STUB: _geParenttFolderForFileURL called");
    return NULL;
}

void* _genericIconNameForMaxSize(void) {
    if (verbose) puts("STUB: _genericIconNameForMaxSize called");
    return NULL;
}

void* _getHelperManagerQueue(void) {
    if (verbose) puts("STUB: _getHelperManagerQueue called");
    return NULL;
}

void* logTestState(void) {
    if (verbose) puts("STUB: logTestState called");
    return NULL;
}

void* mt_log_quicklook_cache_size(void) {
    if (verbose) puts("STUB: mt_log_quicklook_cache_size called");
    return NULL;
}

void* ql_dispatch_async(void) {
    if (verbose) puts("STUB: ql_dispatch_async called");
    return NULL;
}

void* ql_dispatch_once(void) {
    if (verbose) puts("STUB: ql_dispatch_once called");
    return NULL;
}

void* ql_dispatch_source_set_cancel_handler(void) {
    if (verbose) puts("STUB: ql_dispatch_source_set_cancel_handler called");
    return NULL;
}

void* ql_dispatch_source_set_event_handler(void) {
    if (verbose) puts("STUB: ql_dispatch_source_set_event_handler called");
    return NULL;
}

void* ql_dispatch_sync(void) {
    if (verbose) puts("STUB: ql_dispatch_sync called");
    return NULL;
}

void* qlfuture_destroy(void) {
    if (verbose) puts("STUB: qlfuture_destroy called");
    return NULL;
}

void* qlfuture_dispatch(void) {
    if (verbose) puts("STUB: qlfuture_dispatch called");
    return NULL;
}

void* qlfuture_notify(void) {
    if (verbose) puts("STUB: qlfuture_notify called");
    return NULL;
}

void* qlfuture_wait(void) {
    if (verbose) puts("STUB: qlfuture_wait called");
    return NULL;
}

void* qlstaticqueue(void) {
    if (verbose) puts("STUB: qlstaticqueue called");
    return NULL;
}
