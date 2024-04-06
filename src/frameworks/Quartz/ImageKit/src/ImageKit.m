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


#include <ImageKit/ImageKit.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;
__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}


void* const IKAnimationDelayKey = (void*)0;
void* const IKAnimationDurationKey = (void*)0;
void* const IKAnimationFrameRateKey = (void*)0;
void* const IKAnimationVelocityCurveKey = (void*)0;
void* IKCameraIrisStatusChangeNotification = (void*)0;
void* IKCameraStatusChangeNotification = (void*)0;
void* const IKDisableFontSmothing = (void*)0;
void* const IKFilterBrowserDefaultInputImage = (void*)0;
void* const IKFilterBrowserExcludeCategories = (void*)0;
void* const IKFilterBrowserExcludeFilters = (void*)0;
void* const IKFilterBrowserFilterDoubleClickNotification = (void*)0;
void* const IKFilterBrowserFilterSelectedNotification = (void*)0;
void* const IKFilterBrowserShowCategories = (void*)0;
void* const IKFilterBrowserShowPreview = (void*)0;
void* const IKFilterBrowserWillPreviewFilterNotification = (void*)0;
void* const IKFilterPboardType = (void*)0;
void* const IKIconCellDisableTwoLineTitlesKey = (void*)0;
void* const IKIconCellStatusProgress = (void*)0;
void* const IKIconCellStatusString = (void*)0;
void* const IKIconCellStatusTextAttributes = (void*)0;
void* const IKIconCellTextSizeKey = (void*)0;
void* const IKIconCellTitlesOnRightKey = (void*)0;
void* const IKImageBrowserBackgroundColorKey = (void*)0;
void* const IKImageBrowserCGImageRepresentationType = (void*)0;
void* const IKImageBrowserCGImageSourceRepresentationType = (void*)0;
void* const IKImageBrowserCIImageRepresentationType = (void*)0;
void* const IKImageBrowserCellBackgroundLayer = (void*)0;
void* const IKImageBrowserCellForegroundLayer = (void*)0;
void* const IKImageBrowserCellPlaceHolderLayer = (void*)0;
void* const IKImageBrowserCellSelectionLayer = (void*)0;
void* const IKImageBrowserCellsHighlightedTitleAttributesKey = (void*)0;
void* const IKImageBrowserCellsHighlightedUnfocusedTitleAttributesKey = (void*)0;
void* const IKImageBrowserCellsOutlineColorKey = (void*)0;
void* const IKImageBrowserCellsSubtitleAttributesKey = (void*)0;
void* const IKImageBrowserCellsTitleAttributesKey = (void*)0;
void* const IKImageBrowserDidFinishNicestRendering = (void*)0;
void* const IKImageBrowserDidStabilize = (void*)0;
void* const IKImageBrowserFlavorCellPartKey = (void*)0;
void* const IKImageBrowserFlavorGridSpacingKey = (void*)0;
void* const IKImageBrowserFlavorIconSizeKey = (void*)0;
void* const IKImageBrowserFlavorMaxIconSizeKey = (void*)0;
void* const IKImageBrowserFlavorShowItemInfoKey = (void*)0;
void* const IKImageBrowserFlavorTextAttributesKey = (void*)0;
void* const IKImageBrowserFlavorTextSizeKey = (void*)0;
void* const IKImageBrowserFlavorTitlesOnRightKey = (void*)0;
void* const IKImageBrowserGroupBackgroundColorKey = (void*)0;
void* const IKImageBrowserGroupFooterLayer = (void*)0;
void* const IKImageBrowserGroupHeaderLayer = (void*)0;
void* const IKImageBrowserGroupIdentifierKey = (void*)0;
void* const IKImageBrowserGroupRangeKey = (void*)0;
void* const IKImageBrowserGroupStyleKey = (void*)0;
void* const IKImageBrowserGroupTagImageKey = (void*)0;
void* const IKImageBrowserGroupTitleKey = (void*)0;
void* const IKImageBrowserIconRefPathRepresentationType = (void*)0;
void* const IKImageBrowserIconRefRepresentationType = (void*)0;
void* const IKImageBrowserNSBitmapImageRepresentationType = (void*)0;
void* const IKImageBrowserNSDataRepresentationType = (void*)0;
void* const IKImageBrowserNSImageRepresentationType = (void*)0;
void* const IKImageBrowserNSURLRepresentationType = (void*)0;
void* const IKImageBrowserOutlinesCellsBinding = (void*)0;
void* const IKImageBrowserPDFPageRepresentationType = (void*)0;
void* const IKImageBrowserPathRepresentationType = (void*)0;
void* const IKImageBrowserPromisedRepresentationType = (void*)0;
void* const IKImageBrowserQCCompositionPathRepresentationType = (void*)0;
void* const IKImageBrowserQCCompositionRepresentationType = (void*)0;
void* const IKImageBrowserQTMoviePathRepresentationType = (void*)0;
void* const IKImageBrowserQTMovieRepresentationType = (void*)0;
void* const IKImageBrowserQuickLookPathRepresentationType = (void*)0;
void* const IKImageBrowserSelectionColorKey = (void*)0;
void* const IKImageBrowserZoomValueBinding = (void*)0;
void* const IKImageCompletedInitialLoadingNotification = (void*)0;
void* const IKImageCompletedTileLoadNotification = (void*)0;
void* const IKImageFlowSubtitleAttributesKey = (void*)0;
void* const IKImageFlowTitleAttributesKey = (void*)0;
void* const IKImageViewLoadingDidFailNotification = (void*)0;
void* const IKOverlayTypeBackground = (void*)0;
void* const IKOverlayTypeImage = (void*)0;
void* const IKPictureTakerAllowsEditingKey = (void*)0;
void* const IKPictureTakerAllowsFileChoosingKey = (void*)0;
void* const IKPictureTakerAllowsVideoCaptureKey = (void*)0;
void* const IKPictureTakerCropAreaSizeKey = (void*)0;
void* const IKPictureTakerCustomSourcesKey = (void*)0;
void* const IKPictureTakerEnableFaceDetectionKey = (void*)0;
void* const IKPictureTakerImageTransformsKey = (void*)0;
void* const IKPictureTakerInformationalTextKey = (void*)0;
void* const IKPictureTakerOutputImageMaxSizeKey = (void*)0;
void* const IKPictureTakerRemainOpenAfterValidateKey = (void*)0;
void* const IKPictureTakerShowAddressBookPicture = (void*)0;
void* const IKPictureTakerShowAddressBookPictureKey = (void*)0;
void* const IKPictureTakerShowChatIconsKey = (void*)0;
void* const IKPictureTakerShowEffectsKey = (void*)0;
void* const IKPictureTakerShowEmptyPicture = (void*)0;
void* const IKPictureTakerShowEmptyPictureKey = (void*)0;
void* const IKPictureTakerShowRecentPictureKey = (void*)0;
void* const IKPictureTakerShowUserPicturesKey = (void*)0;
void* const IKPictureTakerSourceDefaults = (void*)0;
void* const IKPictureTakerSourceKey = (void*)0;
void* const IKPictureTakerSourceRecentPictures = (void*)0;
void* const IKPictureTakerUpdateRecentPictureKey = (void*)0;
void* const IKProfilePictureEditorCropSizeKey = (void*)0;
void* const IKProfilePictureEditorCustomSourcesKey = (void*)0;
void* const IKProfilePictureEditorHidePhotoStreamPictureKey = (void*)0;
void* const IKProfilePictureEditorHideRecentPicturesKey = (void*)0;
void* const IKProfilePictureEditorInputRectKey = (void*)0;
void* const IKProfilePictureEditorShowAddressBookPictureKey = (void*)0;
void* const IKProfilePictureEditorShowChatIconsKey = (void*)0;
void* const IKProfilePictureEditorShowEmptyPictureKey = (void*)0;
void* const IKProfilePictureEditorUpdateRecentPictureKey = (void*)0;
void* const IKProfilePictureEditorViewSizeKey = (void*)0;
void* const IKQuickLookContentRect = (void*)0;
void* const IKQuickLookProperties = (void*)0;
void* const IKSaveOptionsDefaultCompressionQuality = (void*)0;
void* const IKSizeHint = (void*)0;
void* const IKSlideshowAudioFile = (void*)0;
void* const IKSlideshowModeImages = (void*)0;
void* const IKSlideshowModeOther = (void*)0;
void* const IKSlideshowModePDF = (void*)0;
void* const IKSlideshowPDFDisplayBox = (void*)0;
void* const IKSlideshowPDFDisplayMode = (void*)0;
void* const IKSlideshowPDFDisplaysAsBook = (void*)0;
void* const IKSlideshowScreen = (void*)0;
void* const IKSlideshowStartIndex = (void*)0;
void* const IKSlideshowStartPaused = (void*)0;
void* const IKSlideshowWrapAround = (void*)0;
void* IKTaskManagerTaskDoneNotification = (void*)0;
void* const IKToolModeAnnotate = (void*)0;
void* const IKToolModeCrop = (void*)0;
void* const IKToolModeMove = (void*)0;
void* const IKToolModeNone = (void*)0;
void* const IKToolModeRotate = (void*)0;
void* const IKToolModeSelect = (void*)0;
void* const IKToolModeSelectEllipse = (void*)0;
void* const IKToolModeSelectLasso = (void*)0;
void* const IKToolModeSelectRect = (void*)0;
void* const IKToolModeSelectRectImageCapture = (void*)0;
void* IKUIFlavorAllowFallback = (void*)0;
void* IKUISizeFlavor = (void*)0;
void* IKUISizeMini = (void*)0;
void* IKUISizeRegular = (void*)0;
void* IKUISizeSmall = (void*)0;
void* IKUImaxSize = (void*)0;
void* const IK_ApertureBundleIdentifier = (void*)0;
void* const IK_MailBundleIdentifier = (void*)0;
void* const IK_PhotosBundleIdentifier = (void*)0;
void* const IK_iPhotoBundleIdentifier = (void*)0;
void* const kIKAllowLowPowerRendering = (void*)0;
void* const kIKColorSpace = (void*)0;
void* const kIKEnableImageSourceHardwareAcceleration = (void*)0;
void* const kIKGeometryChanged = (void*)0;
void* const kIKInitialBackingScale = (void*)0;
void* const kIKInitialCenter = (void*)0;
void* const kIKInitialZoomFactor = (void*)0;
void* const kIKKeepZoomToFitSticky = (void*)0;
void* const kIKOutputImageChangedNotification = (void*)0;
void* const kIKPictureTakenNotification = (void*)0;
void* const kIKTakePictureAbortedNotification = (void*)0;
void* const kIKThumbnailLowResFactor = (void*)0;
void* const kIKWebImageURL = (void*)0;
void* const kIKWebPageURL = (void*)0;
void* const kIKZoomToFitOnStart = (void*)0;
void* const kIKZoomToFitOnStartIgnoreWindowSize = (void*)0;

void *IKBestRepresentationFromNSImageForPhysicalSize(void) {
    if (verbose) puts("STUB: IKBestRepresentationFromNSImageForPhysicalSize called");
    return NULL;
}

void *IKCGBitmapContextRefFromCGImageRef(void) {
    if (verbose) puts("STUB: IKCGBitmapContextRefFromCGImageRef called");
    return NULL;
}

void *IKCGBitmapContextRefFromData(void) {
    if (verbose) puts("STUB: IKCGBitmapContextRefFromData called");
    return NULL;
}

void *IKCGBitmapContextRefFromNSBitmapImageRep(void) {
    if (verbose) puts("STUB: IKCGBitmapContextRefFromNSBitmapImageRep called");
    return NULL;
}

void *IKCGBitmapContextRefFromNSImage(void) {
    if (verbose) puts("STUB: IKCGBitmapContextRefFromNSImage called");
    return NULL;
}

void *IKCGImageFromCGBitmapContextRef(void) {
    if (verbose) puts("STUB: IKCGImageFromCGBitmapContextRef called");
    return NULL;
}

void *IKCGImageFromCGImageSourceRef(void) {
    if (verbose) puts("STUB: IKCGImageFromCGImageSourceRef called");
    return NULL;
}

void *IKCGImageFromData(void) {
    if (verbose) puts("STUB: IKCGImageFromData called");
    return NULL;
}

void *IKCGImageFromIconRef(void) {
    if (verbose) puts("STUB: IKCGImageFromIconRef called");
    return NULL;
}

void *IKCGImageFromNSBitmapImageRep(void) {
    if (verbose) puts("STUB: IKCGImageFromNSBitmapImageRep called");
    return NULL;
}

void *IKCGImageFromNSImage(void) {
    if (verbose) puts("STUB: IKCGImageFromNSImage called");
    return NULL;
}

void *IKCreateCGBitmapContextWithSize(void) {
    if (verbose) puts("STUB: IKCreateCGBitmapContextWithSize called");
    return NULL;
}

void *IKCreateCGContextRefFromCGImageRef(void) {
    if (verbose) puts("STUB: IKCreateCGContextRefFromCGImageRef called");
    return NULL;
}

void *IKCreateCGContextRefFromData(void) {
    if (verbose) puts("STUB: IKCreateCGContextRefFromData called");
    return NULL;
}

void *IKCreateCGContextRefFromNSBitmapImageRep(void) {
    if (verbose) puts("STUB: IKCreateCGContextRefFromNSBitmapImageRep called");
    return NULL;
}

void *IKCreateCGContextRefFromNSImage(void) {
    if (verbose) puts("STUB: IKCreateCGContextRefFromNSImage called");
    return NULL;
}

void *IKCreateIconRefFromPath(void) {
    if (verbose) puts("STUB: IKCreateIconRefFromPath called");
    return NULL;
}

void *IKCreateIconRefFromURL(void) {
    if (verbose) puts("STUB: IKCreateIconRefFromURL called");
    return NULL;
}

void *IKFastThumbnailImageFromPath(void) {
    if (verbose) puts("STUB: IKFastThumbnailImageFromPath called");
    return NULL;
}

void *IKFastThumbnailImageFromSourceRef(void) {
    if (verbose) puts("STUB: IKFastThumbnailImageFromSourceRef called");
    return NULL;
}

void *IKIconImageFromPath(void) {
    if (verbose) puts("STUB: IKIconImageFromPath called");
    return NULL;
}

void *IKIconImageFromURL(void) {
    if (verbose) puts("STUB: IKIconImageFromURL called");
    return NULL;
}

void *IKNSBitmapImageRepFromNSImage(void) {
    if (verbose) puts("STUB: IKNSBitmapImageRepFromNSImage called");
    return NULL;
}

void *IKNSImageFromCGBitmapContextRef(void) {
    if (verbose) puts("STUB: IKNSImageFromCGBitmapContextRef called");
    return NULL;
}

void *IKNSImageFromCGImage(void) {
    if (verbose) puts("STUB: IKNSImageFromCGImage called");
    return NULL;
}

void *IKNSImageFromCGImageSourceRef(void) {
    if (verbose) puts("STUB: IKNSImageFromCGImageSourceRef called");
    return NULL;
}

void *IKNSImageFromCIImage(void) {
    if (verbose) puts("STUB: IKNSImageFromCIImage called");
    return NULL;
}

void *IKNSImageFromCIImageWithSize(void) {
    if (verbose) puts("STUB: IKNSImageFromCIImageWithSize called");
    return NULL;
}

void *IKNSImageFromNSBitmapImageRep(void) {
    if (verbose) puts("STUB: IKNSImageFromNSBitmapImageRep called");
    return NULL;
}

void *IKSizeOfIcon(void) {
    if (verbose) puts("STUB: IKSizeOfIcon called");
    return NULL;
}

void *IKThumbnailImage(void) {
    if (verbose) puts("STUB: IKThumbnailImage called");
    return NULL;
}

void *IKThumbnailImageFromCGImage(void) {
    if (verbose) puts("STUB: IKThumbnailImageFromCGImage called");
    return NULL;
}

void *IKThumbnailImageFromSourceRef(void) {
    if (verbose) puts("STUB: IKThumbnailImageFromSourceRef called");
    return NULL;
}

