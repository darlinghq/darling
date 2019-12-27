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


#include <MapKit/MapKit.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* GEOGetMKIconManagerLog(void)
{
    if (verbose) puts("STUB: GEOGetMKIconManagerLog called");
    return NULL;
}

void* ImageForShieldDataSourceTypeShouldBeFlipped(void)
{
    if (verbose) puts("STUB: ImageForShieldDataSourceTypeShouldBeFlipped called");
    return NULL;
}

void* MKAddressDictionaryFromCLPlacemark(void)
{
    if (verbose) puts("STUB: MKAddressDictionaryFromCLPlacemark called");
    return NULL;
}

void* MKAddressDictionaryFromContactsDictionary(void)
{
    if (verbose) puts("STUB: MKAddressDictionaryFromContactsDictionary called");
    return NULL;
}

void* MKAddressDictionaryFromPostalAddress(void)
{
    if (verbose) puts("STUB: MKAddressDictionaryFromPostalAddress called");
    return NULL;
}

void* MKAddressStringForAddressDictionary(void)
{
    if (verbose) puts("STUB: MKAddressStringForAddressDictionary called");
    return NULL;
}

void* MKAnnotationViewIntersectionArea(void)
{
    if (verbose) puts("STUB: MKAnnotationViewIntersectionArea called");
    return NULL;
}

void* MKAnnotationViewsIntersectionArea(void)
{
    if (verbose) puts("STUB: MKAnnotationViewsIntersectionArea called");
    return NULL;
}

void* MKApplicationLayoutDirectionIsRightToLeft(void)
{
    if (verbose) puts("STUB: MKApplicationLayoutDirectionIsRightToLeft called");
    return NULL;
}

void* MKArrowAppendArrowheadToPathAndReturnMetrics(void)
{
    if (verbose) puts("STUB: MKArrowAppendArrowheadToPathAndReturnMetrics called");
    return NULL;
}

void* MKArrowAppendArrowheadToPathAndReturnMetricsWithStemAdjustment(void)
{
    if (verbose) puts("STUB: MKArrowAppendArrowheadToPathAndReturnMetricsWithStemAdjustment called");
    return NULL;
}

void* MKArrowAppendCurveToPath(void)
{
    if (verbose) puts("STUB: MKArrowAppendCurveToPath called");
    return NULL;
}

void* MKArrowAppendCurveToPathWithOptions(void)
{
    if (verbose) puts("STUB: MKArrowAppendCurveToPathWithOptions called");
    return NULL;
}

void* MKArrowAppendCurveToPathWithOptionsAndRadius(void)
{
    if (verbose) puts("STUB: MKArrowAppendCurveToPathWithOptionsAndRadius called");
    return NULL;
}

void* MKArrowAppendExitRoadLeftToPathInRect(void)
{
    if (verbose) puts("STUB: MKArrowAppendExitRoadLeftToPathInRect called");
    return NULL;
}

void* MKArrowAppendExitRoadRightToPathInRect(void)
{
    if (verbose) puts("STUB: MKArrowAppendExitRoadRightToPathInRect called");
    return NULL;
}

void* MKArrowAppendFinalSegmentToPath(void)
{
    if (verbose) puts("STUB: MKArrowAppendFinalSegmentToPath called");
    return NULL;
}

void* MKArrowAppendInitialSegmentToPath(void)
{
    if (verbose) puts("STUB: MKArrowAppendInitialSegmentToPath called");
    return NULL;
}

void* MKArrowAppendLeftTurnToPathInRect(void)
{
    if (verbose) puts("STUB: MKArrowAppendLeftTurnToPathInRect called");
    return NULL;
}

void* MKArrowAppendMergeLeftToPathInRect(void)
{
    if (verbose) puts("STUB: MKArrowAppendMergeLeftToPathInRect called");
    return NULL;
}

void* MKArrowAppendMergeRightToPathInRect(void)
{
    if (verbose) puts("STUB: MKArrowAppendMergeRightToPathInRect called");
    return NULL;
}

void* MKArrowAppendRightAngleCurveToPath(void)
{
    if (verbose) puts("STUB: MKArrowAppendRightAngleCurveToPath called");
    return NULL;
}

void* MKArrowAppendRightTurnToPathInRect(void)
{
    if (verbose) puts("STUB: MKArrowAppendRightTurnToPathInRect called");
    return NULL;
}

void* MKArrowAppendSegmentToPath(void)
{
    if (verbose) puts("STUB: MKArrowAppendSegmentToPath called");
    return NULL;
}

void* MKArrowAppendStraightArrowToPathInRect(void)
{
    if (verbose) puts("STUB: MKArrowAppendStraightArrowToPathInRect called");
    return NULL;
}

void* MKArrowAppendUTurnCurveToPath(void)
{
    if (verbose) puts("STUB: MKArrowAppendUTurnCurveToPath called");
    return NULL;
}

void* MKArrowAppendUTurnToPathInRect(void)
{
    if (verbose) puts("STUB: MKArrowAppendUTurnToPathInRect called");
    return NULL;
}

void* MKArrowCreateBezierPathsForArrivalWithSize(void)
{
    if (verbose) puts("STUB: MKArrowCreateBezierPathsForArrivalWithSize called");
    return NULL;
}

void* MKArrowCreateBezierPathsForArrivalWithSizeAndReturnCenter(void)
{
    if (verbose) puts("STUB: MKArrowCreateBezierPathsForArrivalWithSizeAndReturnCenter called");
    return NULL;
}

void* MKArrowCreateBezierPathsForEitherProceedToRouteOrArrivalWithSizeAndReturnCenter(void)
{
    if (verbose) puts("STUB: MKArrowCreateBezierPathsForEitherProceedToRouteOrArrivalWithSizeAndReturnCenter called");
    return NULL;
}

void* MKArrowCreateBezierPathsForProceedToRouteWithSize(void)
{
    if (verbose) puts("STUB: MKArrowCreateBezierPathsForProceedToRouteWithSize called");
    return NULL;
}

void* MKArrowCreateBezierPathsForProceedToRouteWithSizeAndReturnCenter(void)
{
    if (verbose) puts("STUB: MKArrowCreateBezierPathsForProceedToRouteWithSizeAndReturnCenter called");
    return NULL;
}

void* MKBearingFromCoordinateToCoordinate(void)
{
    if (verbose) puts("STUB: MKBearingFromCoordinateToCoordinate called");
    return NULL;
}

void* MKCalculateDistance(void)
{
    if (verbose) puts("STUB: MKCalculateDistance called");
    return NULL;
}

void* MKCenterWithLayerPosition(void)
{
    if (verbose) puts("STUB: MKCenterWithLayerPosition called");
    return NULL;
}

void* MKClosestMapPointInLineSegmentFromMapPoint(void)
{
    if (verbose) puts("STUB: MKClosestMapPointInLineSegmentFromMapPoint called");
    return NULL;
}

void* MKCompassPointFromLocationDirection(void)
{
    if (verbose) puts("STUB: MKCompassPointFromLocationDirection called");
    return NULL;
}

void* MKCoordinateForMapPoint(void)
{
    if (verbose) puts("STUB: MKCoordinateForMapPoint called");
    return NULL;
}

void* MKCoordinateForTilePoint(void)
{
    if (verbose) puts("STUB: MKCoordinateForTilePoint called");
    return NULL;
}

void* MKCoordinateRegionForMapRect(void)
{
    if (verbose) puts("STUB: MKCoordinateRegionForMapRect called");
    return NULL;
}

void* MKCoordinateRegionMakeWithDistance(void)
{
    if (verbose) puts("STUB: MKCoordinateRegionMakeWithDistance called");
    return NULL;
}

void* MKCoordinateRegionMakeWithZoomLevel(void)
{
    if (verbose) puts("STUB: MKCoordinateRegionMakeWithZoomLevel called");
    return NULL;
}

void* MKCoordinatesForRegion(void)
{
    if (verbose) puts("STUB: MKCoordinatesForRegion called");
    return NULL;
}

void* MKCurrentTransitConnectionFailureDiagnosis(void)
{
    if (verbose) puts("STUB: MKCurrentTransitConnectionFailureDiagnosis called");
    return NULL;
}

void* MKDefaultCoordinateRegion(void)
{
    if (verbose) puts("STUB: MKDefaultCoordinateRegion called");
    return NULL;
}

void* MKDefaultGuidanceManeuverMetrics(void)
{
    if (verbose) puts("STUB: MKDefaultGuidanceManeuverMetrics called");
    return NULL;
}

void* MKDirectionBetween(void)
{
    if (verbose) puts("STUB: MKDirectionBetween called");
    return NULL;
}

void* MKGetCenterForIntegralFrame(void)
{
    if (verbose) puts("STUB: MKGetCenterForIntegralFrame called");
    return NULL;
}

void* MKGetMKDefaultLog(void)
{
    if (verbose) puts("STUB: MKGetMKDefaultLog called");
    return NULL;
}

void* MKGetNextOrientation(void)
{
    if (verbose) puts("STUB: MKGetNextOrientation called");
    return NULL;
}

void* MKGuidanceManeuverArrowMetricsApplyScale(void)
{
    if (verbose) puts("STUB: MKGuidanceManeuverArrowMetricsApplyScale called");
    return NULL;
}

void* MKGuidanceManeuverArrowMetricsEqualToArrowMetrics(void)
{
    if (verbose) puts("STUB: MKGuidanceManeuverArrowMetricsEqualToArrowMetrics called");
    return NULL;
}

void* MKIntegerHash(void)
{
    if (verbose) puts("STUB: MKIntegerHash called");
    return NULL;
}

void* MKIntegralAnchorAdustedCenterForViewInView(void)
{
    if (verbose) puts("STUB: MKIntegralAnchorAdustedCenterForViewInView called");
    return NULL;
}

void* MKIntegralCenterForLayer(void)
{
    if (verbose) puts("STUB: MKIntegralCenterForLayer called");
    return NULL;
}

void* MKIntegralCenterForLayerWithScale(void)
{
    if (verbose) puts("STUB: MKIntegralCenterForLayerWithScale called");
    return NULL;
}

void* MKIntegralCenterForView(void)
{
    if (verbose) puts("STUB: MKIntegralCenterForView called");
    return NULL;
}

void* MKIntegralCenterForViewWithScale(void)
{
    if (verbose) puts("STUB: MKIntegralCenterForViewWithScale called");
    return NULL;
}

void* MKIntegralPoint(void)
{
    if (verbose) puts("STUB: MKIntegralPoint called");
    return NULL;
}

void* MKIsHostedInSiri(void)
{
    if (verbose) puts("STUB: MKIsHostedInSiri called");
    return NULL;
}

void* MKKeyForTransitArtwork(void)
{
    if (verbose) puts("STUB: MKKeyForTransitArtwork called");
    return NULL;
}

void* MKLayerPositionWithAnchoredPosition(void)
{
    if (verbose) puts("STUB: MKLayerPositionWithAnchoredPosition called");
    return NULL;
}

void* MKLayerPositionWithCenter(void)
{
    if (verbose) puts("STUB: MKLayerPositionWithCenter called");
    return NULL;
}

void* MKLayerTreeDescription(void)
{
    if (verbose) puts("STUB: MKLayerTreeDescription called");
    return NULL;
}

void* MKLineIntersectsOriginCircle(void)
{
    if (verbose) puts("STUB: MKLineIntersectsOriginCircle called");
    return NULL;
}

void* MKLocalSearchMetricsStatusForError(void)
{
    if (verbose) puts("STUB: MKLocalSearchMetricsStatusForError called");
    return NULL;
}

void* MKLocalSearchMetricsStatusToKeypressStatus(void)
{
    if (verbose) puts("STUB: MKLocalSearchMetricsStatusToKeypressStatus called");
    return NULL;
}

void* MKLocalizedAbbreviationForCompassPoint(void)
{
    if (verbose) puts("STUB: MKLocalizedAbbreviationForCompassPoint called");
    return NULL;
}

void* MKLocalizedStringForUnknownLocation(void)
{
    if (verbose) puts("STUB: MKLocalizedStringForUnknownLocation called");
    return NULL;
}

void* MKLongHash(void)
{
    if (verbose) puts("STUB: MKLongHash called");
    return NULL;
}

void* MKMapItemIdentifierFromGEOPBTransitLine(void)
{
    if (verbose) puts("STUB: MKMapItemIdentifierFromGEOPBTransitLine called");
    return NULL;
}

void* MKMapItemIdentifierFromGEOTransitLine(void)
{
    if (verbose) puts("STUB: MKMapItemIdentifierFromGEOTransitLine called");
    return NULL;
}

void* MKMapItemIdentifiersArrayFromGEOMapItemIdentifiersArray(void)
{
    if (verbose) puts("STUB: MKMapItemIdentifiersArrayFromGEOMapItemIdentifiersArray called");
    return NULL;
}

void* MKMapItemIdentifiersArrayToGEOMapItemIdentifiersArray(void)
{
    if (verbose) puts("STUB: MKMapItemIdentifiersArrayToGEOMapItemIdentifiersArray called");
    return NULL;
}

void* MKMapItemIdentifiersToGEOMapItemIdentifier(void)
{
    if (verbose) puts("STUB: MKMapItemIdentifiersToGEOMapItemIdentifier called");
    return NULL;
}

void* MKMapPointForCoordinate(void)
{
    if (verbose) puts("STUB: MKMapPointForCoordinate called");
    return NULL;
}

void* MKMapPointsPerMeterAtLatitude(void)
{
    if (verbose) puts("STUB: MKMapPointsPerMeterAtLatitude called");
    return NULL;
}

void* MKMapRectBoundingMapPoints(void)
{
    if (verbose) puts("STUB: MKMapRectBoundingMapPoints called");
    return NULL;
}

void* MKMapRectContainsPoint(void)
{
    if (verbose) puts("STUB: MKMapRectContainsPoint called");
    return NULL;
}

void* MKMapRectContainsRect(void)
{
    if (verbose) puts("STUB: MKMapRectContainsRect called");
    return NULL;
}

void* MKMapRectContainsRectWrappingMeridian(void)
{
    if (verbose) puts("STUB: MKMapRectContainsRectWrappingMeridian called");
    return NULL;
}

void* MKMapRectDivide(void)
{
    if (verbose) puts("STUB: MKMapRectDivide called");
    return NULL;
}

void* MKMapRectForCoordinateRegion(void)
{
    if (verbose) puts("STUB: MKMapRectForCoordinateRegion called");
    return NULL;
}

void* MKMapRectInset(void)
{
    if (verbose) puts("STUB: MKMapRectInset called");
    return NULL;
}

void* MKMapRectIntersection(void)
{
    if (verbose) puts("STUB: MKMapRectIntersection called");
    return NULL;
}

void* MKMapRectIntersectsMapRectWrappingMeridian(void)
{
    if (verbose) puts("STUB: MKMapRectIntersectsMapRectWrappingMeridian called");
    return NULL;
}

void* MKMapRectIntersectsRect(void)
{
    if (verbose) puts("STUB: MKMapRectIntersectsRect called");
    return NULL;
}

void* MKMapRectMakeWithRadialDistance(void)
{
    if (verbose) puts("STUB: MKMapRectMakeWithRadialDistance called");
    return NULL;
}

void* MKMapRectOffset(void)
{
    if (verbose) puts("STUB: MKMapRectOffset called");
    return NULL;
}

void* MKMapRectRemainder(void)
{
    if (verbose) puts("STUB: MKMapRectRemainder called");
    return NULL;
}

void* MKMapRectSpans180thMeridian(void)
{
    if (verbose) puts("STUB: MKMapRectSpans180thMeridian called");
    return NULL;
}

void* MKMapRectUnion(void)
{
    if (verbose) puts("STUB: MKMapRectUnion called");
    return NULL;
}

void* MKMappedABCNDictionary(void)
{
    if (verbose) puts("STUB: MKMappedABCNDictionary called");
    return NULL;
}

void* MKMetersBetweenMapPoints(void)
{
    if (verbose) puts("STUB: MKMetersBetweenMapPoints called");
    return NULL;
}

void* MKMetersPerMapPointAtLatitude(void)
{
    if (verbose) puts("STUB: MKMetersPerMapPointAtLatitude called");
    return NULL;
}

void* MKNormalizedPointForLayer(void)
{
    if (verbose) puts("STUB: MKNormalizedPointForLayer called");
    return NULL;
}

void* MKPrepareMultiLineLabelContainerForWidth(void)
{
    if (verbose) puts("STUB: MKPrepareMultiLineLabelContainerForWidth called");
    return NULL;
}

void* MKRectToLineSegments(void)
{
    if (verbose) puts("STUB: MKRectToLineSegments called");
    return NULL;
}

void* MKRectWithPoints(void)
{
    if (verbose) puts("STUB: MKRectWithPoints called");
    return NULL;
}

void* MKRoadWidthAtZoomScale(void)
{
    if (verbose) puts("STUB: MKRoadWidthAtZoomScale called");
    return NULL;
}

void* MKTilePointForCoordinate(void)
{
    if (verbose) puts("STUB: MKTilePointForCoordinate called");
    return NULL;
}

void* MKTmpUserTrip(void)
{
    if (verbose) puts("STUB: MKTmpUserTrip called");
    return NULL;
}

void* MKTransitArtworkImageConstrainedToWidth(void)
{
    if (verbose) puts("STUB: MKTransitArtworkImageConstrainedToWidth called");
    return NULL;
}

void* MKTransitIncidentIcon(void)
{
    if (verbose) puts("STUB: MKTransitIncidentIcon called");
    return NULL;
}

void* MKTripRecording(void)
{
    if (verbose) puts("STUB: MKTripRecording called");
    return NULL;
}

void* MKUnNormalizedPointForLayer(void)
{
    if (verbose) puts("STUB: MKUnNormalizedPointForLayer called");
    return NULL;
}

void* MKZoomScaleForZoomLevelF(void)
{
    if (verbose) puts("STUB: MKZoomScaleForZoomLevelF called");
    return NULL;
}

void* _MKIsRunningInExtension(void)
{
    if (verbose) puts("STUB: _MKIsRunningInExtension called");
    return NULL;
}

void* _MKLinkedOnOrAfterReleaseSet(void)
{
    if (verbose) puts("STUB: _MKLinkedOnOrAfterReleaseSet called");
    return NULL;
}

void* _MKMapRectThatFits(void)
{
    if (verbose) puts("STUB: _MKMapRectThatFits called");
    return NULL;
}

void* _MKPlaceCardButtonsAreFullWidth(void)
{
    if (verbose) puts("STUB: _MKPlaceCardButtonsAreFullWidth called");
    return NULL;
}

void* _MKPlaceCardFormattedStringForTimeInterval(void)
{
    if (verbose) puts("STUB: _MKPlaceCardFormattedStringForTimeInterval called");
    return NULL;
}

void* _MKPlaceCardUseSmallerFont(void)
{
    if (verbose) puts("STUB: _MKPlaceCardUseSmallerFont called");
    return NULL;
}

void* _MKRAPIsAvailable(void)
{
    if (verbose) puts("STUB: _MKRAPIsAvailable called");
    return NULL;
}

void* _MKSiriLanguageIsRTLFn(void)
{
    if (verbose) puts("STUB: _MKSiriLanguageIsRTLFn called");
    return NULL;
}

void* _MKTransitIncidentImage(void)
{
    if (verbose) puts("STUB: _MKTransitIncidentImage called");
    return NULL;
}

void* _Z10_splitNodeP14MKQuadTrieNodeP10MKQuadTrie(void)
{
    if (verbose) puts("STUB: _Z10_splitNodeP14MKQuadTrieNodeP10MKQuadTrie called");
    return NULL;
}

void* _Z16MKMapRectLEQSize9MKMapRect9MKMapSize(void)
{
    if (verbose) puts("STUB: _Z16MKMapRectLEQSize9MKMapRect9MKMapSize called");
    return NULL;
}

void* _Z16_depthFirstApplyP14MKQuadTrieNodeU13block_pointerF28MKQuadTrieTraversalDirectiveS0_E(void)
{
    if (verbose) puts("STUB: _Z16_depthFirstApplyP14MKQuadTrieNodeU13block_pointerF28MKQuadTrieTraversalDirectiveS0_E called");
    return NULL;
}

void* _Z17__depthFirstApplyP14MKQuadTrieNodeU13block_pointerF28MKQuadTrieTraversalDirectiveS0_E(void)
{
    if (verbose) puts("STUB: _Z17__depthFirstApplyP14MKQuadTrieNodeU13block_pointerF28MKQuadTrieTraversalDirectiveS0_E called");
    return NULL;
}

void* _Z18MKQuadTrieNodeFreeP14MKQuadTrieNode(void)
{
    if (verbose) puts("STUB: _Z18MKQuadTrieNodeFreeP14MKQuadTrieNode called");
    return NULL;
}

void* _Z18MKQuadTrieNodeMake9MKMapRect(void)
{
    if (verbose) puts("STUB: _Z18MKQuadTrieNodeMake9MKMapRect called");
    return NULL;
}

void* _Z18_breadthFirstApplyP14MKQuadTrieNodeU13block_pointerF28MKQuadTrieTraversalDirectiveS0_E(void)
{
    if (verbose) puts("STUB: _Z18_breadthFirstApplyP14MKQuadTrieNodeU13block_pointerF28MKQuadTrieTraversalDirectiveS0_E called");
    return NULL;
}

void* _Z21_insertionNodeForItemPU23objcproto12MKAnnotation11objc_objectP14MKQuadTrieNode(void)
{
    if (verbose) puts("STUB: _Z21_insertionNodeForItemPU23objcproto12MKAnnotation11objc_objectP14MKQuadTrieNode called");
    return NULL;
}

void* _Z24MKStringFromQuadTrieNodeP14MKQuadTrieNode(void)
{
    if (verbose) puts("STUB: _Z24MKStringFromQuadTrieNodeP14MKQuadTrieNode called");
    return NULL;
}

void* _Z7_insertPU23objcproto12MKAnnotation11objc_objectP14MKQuadTrieNodeP10MKQuadTrie(void)
{
    if (verbose) puts("STUB: _Z7_insertPU23objcproto12MKAnnotation11objc_objectP14MKQuadTrieNodeP10MKQuadTrie called");
    return NULL;
}

void* _Z7_removePU23objcproto12MKAnnotation11objc_objectP14MKQuadTrieNode(void)
{
    if (verbose) puts("STUB: _Z7_removePU23objcproto12MKAnnotation11objc_objectP14MKQuadTrieNode called");
    return NULL;
}

void* _Z9_containsPU23objcproto12MKAnnotation11objc_objectP14MKQuadTrieNode(void)
{
    if (verbose) puts("STUB: _Z9_containsPU23objcproto12MKAnnotation11objc_objectP14MKQuadTrieNode called");
    return NULL;
}

void* _ZN3URS22shareSectionsForRoutesERKNSt3__16vectorIU8__strongP16GEOComposedRouteNS0_9allocatorIS4_EEEE(void)
{
    if (verbose) puts("STUB: _ZN3URS22shareSectionsForRoutesERKNSt3__16vectorIU8__strongP16GEOComposedRouteNS0_9allocatorIS4_EEEE called");
    return NULL;
}

void* _notifyCenterOffsetChanged(void)
{
    if (verbose) puts("STUB: _notifyCenterOffsetChanged called");
    return NULL;
}

void* _performOnMainThreadIfPossible(void)
{
    if (verbose) puts("STUB: _performOnMainThreadIfPossible called");
    return NULL;
}

void* _scaleForMapRect(void)
{
    if (verbose) puts("STUB: _scaleForMapRect called");
    return NULL;
}

void* mt_log_mapkit_usemap(void)
{
    if (verbose) puts("STUB: mt_log_mapkit_usemap called");
    return NULL;
}

void* mt_log_maps_bookmarked(void)
{
    if (verbose) puts("STUB: mt_log_maps_bookmarked called");
    return NULL;
}

void* mt_log_maps_gesture(void)
{
    if (verbose) puts("STUB: mt_log_maps_gesture called");
    return NULL;
}

void* mt_log_maps_mode_switch(void)
{
    if (verbose) puts("STUB: mt_log_maps_mode_switch called");
    return NULL;
}

void* mt_log_maps_mode_time(void)
{
    if (verbose) puts("STUB: mt_log_maps_mode_time called");
    return NULL;
}

void* mt_log_maps_opened(void)
{
    if (verbose) puts("STUB: mt_log_maps_opened called");
    return NULL;
}

void* mt_log_maps_printed(void)
{
    if (verbose) puts("STUB: mt_log_maps_printed called");
    return NULL;
}

void* mt_log_maps_reportproblem(void)
{
    if (verbose) puts("STUB: mt_log_maps_reportproblem called");
    return NULL;
}

void* mt_log_maps_shared_to(void)
{
    if (verbose) puts("STUB: mt_log_maps_shared_to called");
    return NULL;
}

void* mt_log_maps_usemapkitmap(void)
{
    if (verbose) puts("STUB: mt_log_maps_usemapkitmap called");
    return NULL;
}

void* mt_log_usedirections(void)
{
    if (verbose) puts("STUB: mt_log_usedirections called");
    return NULL;
}

void* mt_signature_for_map_type(void)
{
    if (verbose) puts("STUB: mt_signature_for_map_type called");
    return NULL;
}
