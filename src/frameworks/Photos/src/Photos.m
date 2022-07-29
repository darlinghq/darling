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


#include <Photos/Photos.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* NSStringFromPHErrorCode(void)
{
    if (verbose) puts("STUB: NSStringFromPHErrorCode called");
    return NULL;
}

void* PHAdjustmentBaseVersionDescription(void)
{
    if (verbose) puts("STUB: PHAdjustmentBaseVersionDescription called");
    return NULL;
}

void* PHAssertPredicateContainsNoManagedObjects(void)
{
    if (verbose) puts("STUB: PHAssertPredicateContainsNoManagedObjects called");
    return NULL;
}

void* PHAssetCanPerformAdditionalProcessingOnAsset(void)
{
    if (verbose) puts("STUB: PHAssetCanPerformAdditionalProcessingOnAsset called");
    return NULL;
}

void* PHAssetDeferredProcessingNeededDescription(void)
{
    if (verbose) puts("STUB: PHAssetDeferredProcessingNeededDescription called");
    return NULL;
}

void* PHAssetExportRequestAssetAvailabilityDescription(void)
{
    if (verbose) puts("STUB: PHAssetExportRequestAssetAvailabilityDescription called");
    return NULL;
}

void* PHAssetExportRequestCopyFileURL(void)
{
    if (verbose) puts("STUB: PHAssetExportRequestCopyFileURL called");
    return NULL;
}

void* PHAssetExportRequestCreateOutputDirectory(void)
{
    if (verbose) puts("STUB: PHAssetExportRequestCreateOutputDirectory called");
    return NULL;
}

void* PHAssetExportRequestCustomAccessibilityLabelForAsset(void)
{
    if (verbose) puts("STUB: PHAssetExportRequestCustomAccessibilityLabelForAsset called");
    return NULL;
}

void* PHAssetExportRequestCustomDateForAssetWithOptions(void)
{
    if (verbose) puts("STUB: PHAssetExportRequestCustomDateForAssetWithOptions called");
    return NULL;
}

void* PHAssetExportRequestError(void)
{
    if (verbose) puts("STUB: PHAssetExportRequestError called");
    return NULL;
}

void* PHAssetExportRequestFilenameForAsset(void)
{
    if (verbose) puts("STUB: PHAssetExportRequestFilenameForAsset called");
    return NULL;
}

void* PHAssetExportRequestLocationMetadataOperationForAssetWithOptions(void)
{
    if (verbose) puts("STUB: PHAssetExportRequestLocationMetadataOperationForAssetWithOptions called");
    return NULL;
}

void* PHAssetExportRequestMediaFormatConversionManager(void)
{
    if (verbose) puts("STUB: PHAssetExportRequestMediaFormatConversionManager called");
    return NULL;
}

void* PHAssetExportRequestNeedsColorSpaceConversionForAssetWithOptions(void)
{
    if (verbose) puts("STUB: PHAssetExportRequestNeedsColorSpaceConversionForAssetWithOptions called");
    return NULL;
}

void* PHAssetExportRequestNeedsCustomAccessibilityLabelBakedInForAssetWithOptions(void)
{
    if (verbose) puts("STUB: PHAssetExportRequestNeedsCustomAccessibilityLabelBakedInForAssetWithOptions called");
    return NULL;
}

void* PHAssetExportRequestPerformAdditionalProcessingOnFile(void)
{
    if (verbose) puts("STUB: PHAssetExportRequestPerformAdditionalProcessingOnFile called");
    return NULL;
}

void* PHAssetExportRequestPerformMediaConversion(void)
{
    if (verbose) puts("STUB: PHAssetExportRequestPerformMediaConversion called");
    return NULL;
}

void* PHAssetExportRequestStateDebugDescription(void)
{
    if (verbose) puts("STUB: PHAssetExportRequestStateDebugDescription called");
    return NULL;
}

void* PHAssetLocationCoordinatesAreEqual(void)
{
    if (verbose) puts("STUB: PHAssetLocationCoordinatesAreEqual called");
    return NULL;
}

void* PHAssetMediaAnalysisStringWithSingleAudioClassification(void)
{
    if (verbose) puts("STUB: PHAssetMediaAnalysisStringWithSingleAudioClassification called");
    return NULL;
}

void* PHAssetMediaAnalysisStringsWithMultipleAudioClassifications(void)
{
    if (verbose) puts("STUB: PHAssetMediaAnalysisStringsWithMultipleAudioClassifications called");
    return NULL;
}

void* PHAssetMediaSubtypesDebugDescription(void)
{
    if (verbose) puts("STUB: PHAssetMediaSubtypesDebugDescription called");
    return NULL;
}

void* PHAssetMediaTypeDebugDescription(void)
{
    if (verbose) puts("STUB: PHAssetMediaTypeDebugDescription called");
    return NULL;
}

void* PHAssetOriginalCreationDateForAsset(void)
{
    if (verbose) puts("STUB: PHAssetOriginalCreationDateForAsset called");
    return NULL;
}

void* PHAssetOriginalLocationForAsset(void)
{
    if (verbose) puts("STUB: PHAssetOriginalLocationForAsset called");
    return NULL;
}

void* PHAssetPlaybackStyleDebugDescription(void)
{
    if (verbose) puts("STUB: PHAssetPlaybackStyleDebugDescription called");
    return NULL;
}

void* PHAssetPlaybackVariationDebugDescription(void)
{
    if (verbose) puts("STUB: PHAssetPlaybackVariationDebugDescription called");
    return NULL;
}

void* PHAssetResourceAnalysisTypeDescription(void)
{
    if (verbose) puts("STUB: PHAssetResourceAnalysisTypeDescription called");
    return NULL;
}

void* PHChooserListContinueEnumerating(void)
{
    if (verbose) puts("STUB: PHChooserListContinueEnumerating called");
    return NULL;
}

void* PHChooserListMoveFirst(void)
{
    if (verbose) puts("STUB: PHChooserListMoveFirst called");
    return NULL;
}

void* PHChooserListSetSortDescriptor(void)
{
    if (verbose) puts("STUB: PHChooserListSetSortDescriptor called");
    return NULL;
}

void* PHChoosingPolicyName(void)
{
    if (verbose) puts("STUB: PHChoosingPolicyName called");
    return NULL;
}

void* PHCollectionPresentationHintsStringFromQualityCategory(void)
{
    if (verbose) puts("STUB: PHCollectionPresentationHintsStringFromQualityCategory called");
    return NULL;
}

void* PHCollectionPresentationHintsStringFromTemporalRange(void)
{
    if (verbose) puts("STUB: PHCollectionPresentationHintsStringFromTemporalRange called");
    return NULL;
}

void* PHCollectionPresentationHintsStringFromTripType(void)
{
    if (verbose) puts("STUB: PHCollectionPresentationHintsStringFromTripType called");
    return NULL;
}

void* PHCollectionPresentationHintsStringsFromSocialRelationships(void)
{
    if (verbose) puts("STUB: PHCollectionPresentationHintsStringsFromSocialRelationships called");
    return NULL;
}

void* PHCopyUniformTypeIdentifierFromFileURL(void)
{
    if (verbose) puts("STUB: PHCopyUniformTypeIdentifierFromFileURL called");
    return NULL;
}

void* PHCopyUniformTypeIdentifierFromFilename(void)
{
    if (verbose) puts("STUB: PHCopyUniformTypeIdentifierFromFilename called");
    return NULL;
}

void* PHDecodeImageFromStoreForRequest(void)
{
    if (verbose) puts("STUB: PHDecodeImageFromStoreForRequest called");
    return NULL;
}

void* PHDecodeImageFromURLForRequest(void)
{
    if (verbose) puts("STUB: PHDecodeImageFromURLForRequest called");
    return NULL;
}

void* PHDescriptionForPhotosHighlightCategory(void)
{
    if (verbose) puts("STUB: PHDescriptionForPhotosHighlightCategory called");
    return NULL;
}

void* PHDescriptionForPhotosHighlightKind(void)
{
    if (verbose) puts("STUB: PHDescriptionForPhotosHighlightKind called");
    return NULL;
}

void* PHDescriptionForPhotosHighlightType(void)
{
    if (verbose) puts("STUB: PHDescriptionForPhotosHighlightType called");
    return NULL;
}

void* PHDescriptionsForResourceTypes(void)
{
    if (verbose) puts("STUB: PHDescriptionsForResourceTypes called");
    return NULL;
}

void* PHDeviceSupportsRAW(void)
{
    if (verbose) puts("STUB: PHDeviceSupportsRAW called");
    return NULL;
}

void* PHErrorFromPLError(void)
{
    if (verbose) puts("STUB: PHErrorFromPLError called");
    return NULL;
}

void* PHFetchDictionaryAccessingMutableCopy(void)
{
    if (verbose) puts("STUB: PHFetchDictionaryAccessingMutableCopy called");
    return NULL;
}

void* PHFullScreenSizeForScreen(void)
{
    if (verbose) puts("STUB: PHFullScreenSizeForScreen called");
    return NULL;
}

void* PHGenerateCropRectGivenNormalizedCropRect(void)
{
    if (verbose) puts("STUB: PHGenerateCropRectGivenNormalizedCropRect called");
    return NULL;
}

void* PHImageManagerRecordEnabled(void)
{
    if (verbose) puts("STUB: PHImageManagerRecordEnabled called");
    return NULL;
}

void* PHIsImageAssetResourceType(void)
{
    if (verbose) puts("STUB: PHIsImageAssetResourceType called");
    return NULL;
}

void* PHIsVideoAssetResourceType(void)
{
    if (verbose) puts("STUB: PHIsVideoAssetResourceType called");
    return NULL;
}

void* PHMemoryMoodForString(void)
{
    if (verbose) puts("STUB: PHMemoryMoodForString called");
    return NULL;
}

void* PHMemoryValidatedMemoryCategory(void)
{
    if (verbose) puts("STUB: PHMemoryValidatedMemoryCategory called");
    return NULL;
}

void* PHMemoryValidatedMemorySubcategory(void)
{
    if (verbose) puts("STUB: PHMemoryValidatedMemorySubcategory called");
    return NULL;
}

void* PHMomentAggregationScoreFromAggregationScoreType(void)
{
    if (verbose) puts("STUB: PHMomentAggregationScoreFromAggregationScoreType called");
    return NULL;
}

void* PHOriginalChoiceDebugDescription(void)
{
    if (verbose) puts("STUB: PHOriginalChoiceDebugDescription called");
    return NULL;
}

void* PHPhotosHighlightVisibilityStateIsVisibleInMonth(void)
{
    if (verbose) puts("STUB: PHPhotosHighlightVisibilityStateIsVisibleInMonth called");
    return NULL;
}

void* PHQueryTransformExpressionConstantValue(void)
{
    if (verbose) puts("STUB: PHQueryTransformExpressionConstantValue called");
    return NULL;
}

void* PHSanitizeNormalizedCropRect(void)
{
    if (verbose) puts("STUB: PHSanitizeNormalizedCropRect called");
    return NULL;
}

void* PHScaleImageFromURL(void)
{
    if (verbose) puts("STUB: PHScaleImageFromURL called");
    return NULL;
}

void* PHShortDescriptionForPhotosHighlightCategory(void)
{
    if (verbose) puts("STUB: PHShortDescriptionForPhotosHighlightCategory called");
    return NULL;
}

void* PHShortDescriptionForPhotosHighlightEnrichmentState(void)
{
    if (verbose) puts("STUB: PHShortDescriptionForPhotosHighlightEnrichmentState called");
    return NULL;
}

void* PHShortDescriptionForPhotosHighlightKind(void)
{
    if (verbose) puts("STUB: PHShortDescriptionForPhotosHighlightKind called");
    return NULL;
}

void* PHShortDescriptionForPhotosHighlightType(void)
{
    if (verbose) puts("STUB: PHShortDescriptionForPhotosHighlightType called");
    return NULL;
}

void* PHShortDescriptionForPhotosHighlightVisibilityState(void)
{
    if (verbose) puts("STUB: PHShortDescriptionForPhotosHighlightVisibilityState called");
    return NULL;
}

void* PHSignpostEventsEnabled(void)
{
    if (verbose) puts("STUB: PHSignpostEventsEnabled called");
    return NULL;
}

void* PHSizeScale(void)
{
    if (verbose) puts("STUB: PHSizeScale called");
    return NULL;
}

void* PHSizeScaleToFillSize(void)
{
    if (verbose) puts("STUB: PHSizeScaleToFillSize called");
    return NULL;
}

void* PHSizeScaleToFitSize(void)
{
    if (verbose) puts("STUB: PHSizeScaleToFitSize called");
    return NULL;
}

void* PHStringForMemoryMood(void)
{
    if (verbose) puts("STUB: PHStringForMemoryMood called");
    return NULL;
}

void* PHSuggestionNotificationStateStringWithNotificationState(void)
{
    if (verbose) puts("STUB: PHSuggestionNotificationStateStringWithNotificationState called");
    return NULL;
}

void* PHSuggestionStateStringWithState(void)
{
    if (verbose) puts("STUB: PHSuggestionStateStringWithState called");
    return NULL;
}

void* PHSuggestionStringWithSubtype(void)
{
    if (verbose) puts("STUB: PHSuggestionStringWithSubtype called");
    return NULL;
}

void* PHSuggestionStringWithType(void)
{
    if (verbose) puts("STUB: PHSuggestionStringWithType called");
    return NULL;
}

void* PHUniformTypeIdentifierConformsToResourceType(void)
{
    if (verbose) puts("STUB: PHUniformTypeIdentifierConformsToResourceType called");
    return NULL;
}

void* PHUniformTypeIdentifiersForResourceType(void)
{
    if (verbose) puts("STUB: PHUniformTypeIdentifiersForResourceType called");
    return NULL;
}

void* PHUnpackDownloadStatusNotification(void)
{
    if (verbose) puts("STUB: PHUnpackDownloadStatusNotification called");
    return NULL;
}

void* PHVariationSuggestionStatesFromPLStatesAndType(void)
{
    if (verbose) puts("STUB: PHVariationSuggestionStatesFromPLStatesAndType called");
    return NULL;
}

void* PHVideoComplementVisibilityStateHasFullEditingSupport(void)
{
    if (verbose) puts("STUB: PHVideoComplementVisibilityStateHasFullEditingSupport called");
    return NULL;
}

void* PHVideoComplementVisibilityStateIsEvaluated(void)
{
    if (verbose) puts("STUB: PHVideoComplementVisibilityStateIsEvaluated called");
    return NULL;
}

void* PHVideoComplementVisibilityStateIsExplicit(void)
{
    if (verbose) puts("STUB: PHVideoComplementVisibilityStateIsExplicit called");
    return NULL;
}

void* PHVideoComplementVisibilityStateIsHidden(void)
{
    if (verbose) puts("STUB: PHVideoComplementVisibilityStateIsHidden called");
    return NULL;
}

void* PHVideoComplementVisibilityStateIsPlayable(void)
{
    if (verbose) puts("STUB: PHVideoComplementVisibilityStateIsPlayable called");
    return NULL;
}

void* PLImageManagerRecordEnabled(void)
{
    if (verbose) puts("STUB: PLImageManagerRecordEnabled called");
    return NULL;
}

void* PLObjectIDOrUUIDFromPHObject(void)
{
    if (verbose) puts("STUB: PLObjectIDOrUUIDFromPHObject called");
    return NULL;
}

void* PLVariationSuggestionTypeStatesFromPHStatesAndType(void)
{
    if (verbose) puts("STUB: PLVariationSuggestionTypeStatesFromPHStatesAndType called");
    return NULL;
}

void* _CreationDatesInLibraryAndOriginalMetadataAreSame(void)
{
    if (verbose) puts("STUB: _CreationDatesInLibraryAndOriginalMetadataAreSame called");
    return NULL;
}

void* _FastHashForPredicate(void)
{
    if (verbose) puts("STUB: _FastHashForPredicate called");
    return NULL;
}

void* _FastValueHashForExpression(void)
{
    if (verbose) puts("STUB: _FastValueHashForExpression called");
    return NULL;
}

void* _LocationsMatchUsingComparisonStrategy(void)
{
    if (verbose) puts("STUB: _LocationsMatchUsingComparisonStrategy called");
    return NULL;
}

void* _PHAllAssetMediaSubtypes(void)
{
    if (verbose) puts("STUB: _PHAllAssetMediaSubtypes called");
    return NULL;
}

void* _PHAssetResourceTypeDescription(void)
{
    if (verbose) puts("STUB: _PHAssetResourceTypeDescription called");
    return NULL;
}

void* _PHResourceDownloadRequestResourceAvailabilityDescription(void)
{
    if (verbose) puts("STUB: _PHResourceDownloadRequestResourceAvailabilityDescription called");
    return NULL;
}

void* _PHResourceDownloadRequestResourceTypeDescription(void)
{
    if (verbose) puts("STUB: _PHResourceDownloadRequestResourceTypeDescription called");
    return NULL;
}

void* _PrimaryPLResourceTypeForPrimaryPHResourceType(void)
{
    if (verbose) puts("STUB: _PrimaryPLResourceTypeForPrimaryPHResourceType called");
    return NULL;
}

void* _assetKindForMediaType(void)
{
    if (verbose) puts("STUB: _assetKindForMediaType called");
    return NULL;
}

void* _assetSubtypeForMediaSubtypes(void)
{
    if (verbose) puts("STUB: _assetSubtypeForMediaSubtypes called");
    return NULL;
}

void* _fetchNonHintResources(void)
{
    if (verbose) puts("STUB: _fetchNonHintResources called");
    return NULL;
}

void* _presentFullResourceAtIndex(void)
{
    if (verbose) puts("STUB: _presentFullResourceAtIndex called");
    return NULL;
}

void* _presentResourcesFromHintsUntilStop(void)
{
    if (verbose) puts("STUB: _presentResourcesFromHintsUntilStop called");
    return NULL;
}

void* _sortedResourcesFromResources(void)
{
    if (verbose) puts("STUB: _sortedResourcesFromResources called");
    return NULL;
}

void* assertOnQueue(void)
{
    if (verbose) puts("STUB: assertOnQueue called");
    return NULL;
}

void* dumpCGImage(void)
{
    if (verbose) puts("STUB: dumpCGImage called");
    return NULL;
}

void* importConcurrencyLimit(void)
{
    if (verbose) puts("STUB: importConcurrencyLimit called");
    return NULL;
}
